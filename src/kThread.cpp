#ifndef __KTHREAD_CPP
#define __KTHREAD_CPP

#include "kTypes.h"
#include "kCriticalSection.h"
#include "k_ARMAttributes.h"
#include "k_ARMCPU.h"

#include "kThread.h"
#include "kTiming.h"
#include "kWaitObject.h"
#include "kMutex.h"
#include "kMathRegs.h"
#include "kInterrupt.h"
#include "kPrint.h"
#include "Heap/exl_OSAllocator.h"

#define K_THREAD_PRINTF(...) k::Printf(__VA_ARGS__)

namespace k {
    //Global state
    static int     g_RootThreadMemory[(sizeof(Thread) + 3) / 4];
    static Thread* g_CurrentThread;

    LOWLEVEL ARMFUNC
    void ThreadContext::SaveGP() {
        asm (
            "STM R0, {R0 - R12}\n"
            "BX LR\n"
        );
    }

    LOWLEVEL ARMFUNC
    void ThreadContext::LoadGP() {
        asm (
            "LDM R0, {R0 - R12}\n"
            "BX LR"
        );
    }

    LOWLEVEL ARMFUNC
    void ThreadContext::SaveMathRegs() {
        asm volatile (
            "PUSH {R4 - R9, LR}\n"
            "ADDS R0, R0, %0\n"
            "LDR R1, =%1\n"
            "LDM R1, {R2 - R5}\n"
            "ADDS R9, R1, %2\n"
            "LDM R9, {R6 - R7}\n"
            "LDRH R9, [R1, %3]\n"
            "AND R9, R9, #1\n"
            "LDRH R8, [R1, %4]\n"
            "AND R8, R8, #3\n"
            "ORRS R8, R8, R9, LSL#16\n"
            "STM R0, {R2 - R8}\n"
            "POP {R4 - R9, PC}\n"
            : : 
            "I" (offsetof(ThreadContext, m_DivNumer)),
            "X" (k::reg::DIV_NUMER),
            "I" ((const char*)k::reg::SQRT_PARAM - (const char*)k::reg::DIV_NUMER),
            "J" ((const char*)k::reg::SQRTCNT - (const char*)k::reg::DIV_NUMER),
            "J" ((const char*)k::reg::DIVCNT - (const char*)k::reg::DIV_NUMER)
            :
        );
    }

    LOWLEVEL ARMFUNC
    void ThreadContext::LoadMathRegs() {
        asm volatile (
            "PUSH {R4 - R8}\n"
            "ADDS R0, R0, %0\n"
            "LDR R1, =%1\n"
            "LDM R0, {R2 - R8}\n"
            "STM R1, {R2 - R5}\n"
            "STRH R8, [R1, %2]\n"
            "ADDS R2, R1, %3\n"
            "LSRS R8, R8, #16\n"
            "STM R2, {R6 - R7}\n"
            "STRH R8, [R1, %4]\n"
            "POP {R4 - R8}"
            : : 
            "I" (offsetof(ThreadContext, m_DivNumer)),
            "X" (k::reg::DIV_NUMER),
            "J" ((const char*)k::reg::DIVCNT - (const char*)k::reg::DIV_NUMER),
            "I" ((const char*)k::reg::SQRT_PARAM - (const char*)k::reg::DIV_NUMER),
            "J" ((const char*)k::reg::SQRTCNT - (const char*)k::reg::DIV_NUMER)
            :
        );
        asm ("BX LR");
    }

    INLINE
    void __kThreadInlineLinkRegDivWait() {
        asm volatile (
            "PUSH {R0}\n"
            "LDR LR, =%0\n"
            "__kThreadInlineLinkRegDivWaitLoop%=:"
            "LDR R0, [LR]\n"
            "ANDS R0, R0, %1\n"
            "BNE __kThreadInlineLinkRegDivWaitLoop%=\n"
            "POP {R0}\n"
            : :
            "X" (k::reg::DIVCNT),
            "I" ((int)k::DIV_BUSY)
            :
        );
    }

    LOWLEVEL ARMFUNC
    void __kThreadBranchLR() {
        //Used for properly exiting into ARM/Thumb state
        asm("BX LR");
    }

    LOWLEVEL ARMFUNC
    int ThreadContext::SaveSYS() {
        register ThreadContext* ctx asm("r0"); //this
        asm ("PUSH {R0, LR}");
        ctx->SaveGP(); //saves general purpose registers
        asm ("POP {R0, LR}");
        ctx->m_CPSR = arm::GetCPSR_inl() & ~arm::MASK_THUMB; //since we'll return to __kThreadBranchLR which is an ARM function, make sure the Thumb bit is off
        ctx->m_PC = (int)__kThreadBranchLR;
        asm volatile("STM %0, {SP, LR}" : :"r" (&ctx->m_SP));
        //LR now points to the parent routine after the SaveSYS call
        //We need to indicate that it should not be called again - we'll set a magic value to R0 to make it skip the call next time
        ctx->m_GPRegs[0] = 42;
        //If loaded as thread state, this will return 42
        asm volatile ("MOVS R0, #0");
        asm ("BX LR");
    }

    LOWLEVEL ARMFUNC
    void ThreadContext::LoadIRQ() {
        register ThreadContext* ctx asm("r0"); //this
        ctx->LoadMathRegs();
        {
            //Restore SPSR
            asm volatile ("MSR SPSR_cxsf, %0" : : "r" (ctx->m_CPSR) :);

            //Set user stack pointer
            int irqCpsr = arm::GetCPSR_inl();
            arm::SetCPSR_inl(arm::MODE_SYS | arm::MASK_IRQ | arm::MASK_FIQ);
            asm volatile ("LDM %0, {SP, LR}" : : "r" (&ctx->m_SP) :);
            arm::SetCPSR_inl(irqCpsr);
        }
        asm volatile ("PUSH {%0}" : : "r" (ctx->m_PC));
        ctx->LoadGP();
        __kThreadInlineLinkRegDivWait();
        asm (
            "POP {LR}\n"
            "MOVS PC, LR"
        ); //will destroy LR_irq which is fine
    }

    LOWLEVEL ARMFUNC
    void ThreadContext::LoadSYS() {
        register ThreadContext* ctx asm("r0"); //this
        ctx->LoadMathRegs();
        asm volatile ("LDM %0, {SP, LR}" : : "r" (&ctx->m_SP) :);
        arm::SetCPSR_c_inl(arm::MODE_SVC | arm::MASK_IRQ | arm::MASK_FIQ);
        //We are now in SVC mode
        asm volatile ("MSR SPSR_cxsf, %0" : : "r" (ctx->m_CPSR) :);
        asm volatile ("PUSH {%0}" : : "r" (ctx->m_PC));
        ctx->LoadGP();
        __kThreadInlineLinkRegDivWait();
        asm (
            "POP {LR}\n"
            "MOVS PC, LR"
        ); //will destroy LR_svc (should be okay)
    }

    ThreadChain::ThreadChain(Thread* parent) {
        m_Parent = parent;
        m_Next = this;
        m_Prev = this;
    }

    void ThreadChain::Remove() {
        m_Prev->m_Next = m_Next;
        m_Next->m_Prev = m_Prev;
        m_Next = this;
        m_Prev = this;
    }

    void ThreadChain::Append(ThreadChain* next) {
        next->m_Prev = this;
        next->m_Next = m_Next;
        m_Next->m_Prev = next;
        m_Next = next;
    }

    void Thread::_InitClass::_Init(u32 timer, u32 threadQuantumCycles) {
        Thread::Scheduler::Init(timer, threadQuantumCycles);
    }

    void Thread::_InitClass::_Terminate() {
        
    }

    Thread::Thread(const char* name, void* stack, size_t stackSize) : 
    m_JoinThreads(this), 
    m_WaitChain(this),
    m_Stack {stack},
    m_StackSize {stackSize},
    m_StackTop {static_cast<char*>(stack) + stackSize}
    {
        m_Name = name;
        m_State = NEW;
        m_NextThread = nullptr;
        m_PrevThread = nullptr;

        m_Context.m_SP = ((int) m_StackTop) & 0xFFFFFFF8; //if needed, align down to 8-byte boundary
        m_Context.m_LR = (int)__kCurrentThreadExit;
        //The rest of the registers can be undefined
    }

    Thread::~Thread() {
        if (m_State != EXITED) {
            Stop();
        }
    }

    LOWLEVEL
    void Thread::Run() {
        asm ("BX LR");
    }

    u32 Thread::ActiveCount() {
        CriticalSection cs = CriticalSection::Enter();
        u32 count = 1;
        Thread* cur = g_CurrentThread;
        Thread* t = cur->m_NextThread;
        while (t != cur) {
            count++;
            t = t->m_NextThread;
        }
        cs.Leave();
        return count;
    }

    Thread* Thread::CurrentThread() {
        return g_CurrentThread;
    }

    void Thread::Yield() {
        g_CurrentThread->_Yield();
    }

    void Thread::Yield(Thread* yieldTo, bool scheduleReturn) {
        g_CurrentThread->_Yield(yieldTo, scheduleReturn);
    }

    void Thread::Sleep(u64 millis) {
        g_CurrentThread->_Sleep(millis);
    }

    void Thread::WaitOn(WaitObject* waitObj, u64 timeout) {
        g_CurrentThread->_WaitOn(waitObj, timeout);
    }

    void Thread::Exit(int exitCode) {
        g_CurrentThread->_Exit(exitCode);
    }

    void Thread::Start() {
        CriticalSection cs = CriticalSection::Enter();

        int funcPtr = (int) k::Thread::_RunProc;
        m_Context.m_GPRegs[0] = (int) this;
        m_Context.m_PC = funcPtr;
        m_Context.m_CPSR = arm::MODE_SYS | ((funcPtr & 1) ? arm::MASK_THUMB : 0);

        if (g_CurrentThread != nullptr) {
            AppendTo(g_CurrentThread);
        }
        else {
            g_CurrentThread = this;
            m_NextThread = this;
            m_PrevThread = this;
        }

        cs.Leave();
    }

    void Thread::Stop() {
        CriticalSection cs = CriticalSection::Enter();
        Unlink();
        ThreadChain* chain = m_JoinThreads.m_Next;
        while (chain != &m_JoinThreads) {
            Thread* cont = m_WaitChain.m_Parent;
            cont->m_State = RUNNING;
            chain = chain->m_Next;
            Printf("Resuming joined thread %s\n", cont->m_Name);
        }
        m_State = EXITED;
        cs.Leave();
    }

    void Thread::Join(u64 timeout) {
        g_CurrentThread->_Join(this, timeout);
    }

    void Thread::_Yield() {
        if (Scheduler::IsSingleThread() || this != g_CurrentThread) {
            return;
        }
        k::CriticalSection cs = CriticalSection::Enter();
        if (m_Context.SaveSYS() == 0) {
            Scheduler::SwitchThread();
            Thread* next = g_CurrentThread;
            if (next != this) {
                this->m_Context.SaveMathRegs();
                //K_THREAD_PRINTF("Thread %s yielded to %s\n", this->m_Name, next->m_Name);
                next->m_Context.LoadSYS(); //this will leave the critical section
            }
        }
        cs.Leave();
        //if this check failed, we returned after a context switch. go back to where we yielded
    }

    void Thread::_RunProc(Thread* _this) {
        _this->Run();
    }

    void Thread::_Yield(Thread* yieldTo, bool scheduleReturn) {
        if (yieldTo == nullptr) {
            _Yield();
        }
        if (this == yieldTo || this != g_CurrentThread) {
            return;
        }
        k::CriticalSection cs = CriticalSection::Enter();
        if (m_Context.SaveSYS() == 0) {
            if (scheduleReturn) {
                Scheduler::ManualReschedule(this, yieldTo);
            }
            g_CurrentThread = yieldTo;
            this->m_Context.SaveMathRegs();
            yieldTo->m_Context.LoadSYS();
        }
        cs.Leave();
    }

    void Thread::_Sleep(u64 millis) {
        CriticalSection cs = CriticalSection::Enter();
        K_THREAD_PRINTF("Sleeping thread %s for %lld\n", m_Name, millis);
        m_WaitTimeout.StartMs(millis);
        m_State = ASLEEP;
        Yield();
        K_THREAD_PRINTF("Thread %s woke up!\n", m_Name);
        cs.Leave();
    }

    void Thread::_WaitOn(WaitObject* waitObj, u64 timeout) {
        CriticalSection cs = CriticalSection::Enter();
        waitObj->m_WaitThreads.m_Prev->Append(&this->m_WaitChain);
        SetWaitState(timeout);
        _Yield();
        cs.Leave();
    }

    void Thread::_Exit(int exitCode) {
        if (this != reinterpret_cast<Thread*>(g_RootThreadMemory)) {
            CriticalSection cs = CriticalSection::Enter();

            K_THREAD_PRINTF("Thread %s exited with code %d.\n", this->m_Name, exitCode);

            Stop();

            Yield(); //this will never return as the thread context is never reloaded
            cs.Leave();
        }
        else {
            K_THREAD_PRINTF("Warn: Can not exit main thread!\n");
        }
    }

    void Thread::_Join(Thread* parent, u64 timeout) {
        if (parent->m_State != EXITED) {
            CriticalSection cs = CriticalSection::Enter();
            parent->m_JoinThreads.m_Prev->Append(&this->m_WaitChain); //append to tail
            SetWaitState(timeout);
            _Yield();
            cs.Leave();
        }
    }

    void Thread::Scheduler::Init(u32 timer, u32 threadQuantumCycles) {
        CriticalSection cs = CriticalSection::Enter();

        g_CurrentThread = nullptr;
        Thread* rootThread = reinterpret_cast<Thread*>(g_RootThreadMemory);

        rootThread->m_Name = "MainThread";
        rootThread->m_NextThread = nullptr;
        rootThread->m_PrevThread = nullptr;
        rootThread->m_State = RUNNING;
        //The thread is currently running without any switching
        //If a switch is fired, the rest of its variables will be initialized on context save
        rootThread->Start();
    
        //Set up timer interrupts
        Interrupt::Set(TimerInterruptID(timer), Thread::Scheduler::_Irq);
        Interrupt::Enable(TimerInterruptBits(timer));

        TimerRegs* regs = GetTimerRegs(timer);
        GetTimerConfigForIntervalCycles(threadQuantumCycles, &regs->Reload, &regs->Control);

        cs.Leave();
    }

    bool Thread::Scheduler::IsSingleThread() {
        return g_CurrentThread->m_NextThread == g_CurrentThread;
    }

    void Thread::Scheduler::SwitchThread() {
        if (!g_CurrentThread) {
            k::Print("ERROR: CurrentThread null!");
            while (true) {
                ;
            }
        }
        do {
            g_CurrentThread = g_CurrentThread->m_NextThread;
            g_CurrentThread->Update();
        }
        while (g_CurrentThread->IsWaiting());
    }

    void Thread::Scheduler::ManualReschedule(Thread* thread, Thread* desiredPrevious) {
        thread->Swap(desiredPrevious);
    }

    ARMFUNC NOINLINE
    void Thread::Scheduler::_SaveCurThreadContext() {
        g_CurrentThread->m_Context.SaveGP();
    }

    LOWLEVEL IRQFUNC ARMFUNC
    void Thread::Scheduler::_Irq() {
        //STACK CODE EXECUTED UNTIL THIS POINT:
        //PUSH {R0-R3,R12,LR} (in ARM9 BIOS)
        //PUSH {LR} (in system interrupt handler, which we do NOT use)
        //Branch here
        //This means the stack currently contains:
        //--STACK TOP
        //LR - to where the exception occured
        //R12 backup
        //R3 backup
        //R2 backup
        //R1 backup
        //R0 backup
        //LR - to BIOS interrupt handler - ADDED BY OUR CODE BELOW

        //This should all be within a critical section, which (hopefully) the default IRQ handler ensures
        //TODO verify if this is actually the case
        asm ("PUSH {LR}");
        if (IsSingleThread()) {
            asm ("POP {PC}"); //return back to the IRQ handler
        }
        
        //This shall not tamper with any registers beyond R3, so we call another function
        //that will backup used regs on the stack if needed
        _SaveCurThreadContext();
        asm volatile ("NOP"); //force GCC to load g_CurrentThread address after __kThreadStateSave
        Thread* lastThread = g_CurrentThread;
        SwitchThread();
        if (lastThread == g_CurrentThread) {
            lastThread->m_Context.LoadGP();
            asm ("POP {PC}"); //no thread switched - return
        }

        //Only save math regs when thread switch is imminent
        lastThread->m_Context.SaveMathRegs();

        asm volatile ("ADDS SP, #0x4"); //discard backup link register

        //First we will fix up our thread state from the IRQ stack
        asm volatile (
            "MOVS R11, %0\n"
            "POP {R0 - R3, R12, LR}\n"
            "STM R11, {R0 - R3}\n"
            "ADDS R11, R11, %1\n"
            "STR R12, [R11]\n"
            "ADDS R11, R11, %2\n"
            //(http://osnet.cs.nchu.edu.tw/powpoint/Embedded94_1/Chapter%207%20ARM%20Exceptions.pdf - slide 27)
            "SUBS LR, LR, #4\n"
            "STR LR, [R11]\n"
            //iiit's clobbering time
            :: "r" (&lastThread->m_Context), "I" (offsetof(ThreadContext, m_GPRegs[12])), "I" (offsetof(ThreadContext, m_PC) - offsetof(ThreadContext, m_GPRegs[12])) :
        );

        //Save SPSR
        {
            int spsr;
            asm volatile ("MRS %0, SPSR" : "=r" (spsr) : :);
            lastThread->m_Context.m_CPSR = spsr;
        }

        //Now save SP_sys and LR_sys
        {
            //Peek into privileged user mode for stack pointer
            int irqCpsr = arm::GetCPSR_inl();
            arm::SetCPSR_inl(arm::MODE_SYS | arm::MASK_IRQ | arm::MASK_FIQ);
            asm volatile ("STM %0, {SP, LR}" : : "r" (&lastThread->m_Context.m_SP) :);
            arm::SetCPSR_inl(irqCpsr);
        }
        
        #ifdef DEBUG
        K_THREAD_PRINTF("Switching from thread %s to %s\n", lastThread->m_Name, g_CurrentThread->m_Name);
        g_CurrentThread->DumpThreadState();
        #endif

        g_CurrentThread->m_Context.LoadIRQ();

        asm ("BX LR"); //for disassemblers
    }

    void Thread::DumpThreadState() {
        k::Printf(
            "Thread state dump for %s\n"
            "R0  = %08x\n"
            "R1  = %08x\n"
            "R2  = %08x\n"
            "R3  = %08x\n"
            "R4  = %08x\n"
            "R5  = %08x\n"
            "R6  = %08x\n"
            "R7  = %08x\n"
            "R8  = %08x\n"
            "R9  = %08x\n"
            "R10 = %08x\n"
            "R11 = %08x\n"
            "R12 = %08x\n"
            "SP  = %08x\n"
            "LR  = %08x\n"
            "PC  = %08x\n"
            "CPSR  = %08x\n",
            this->m_Name,
            m_Context.m_GPRegs[0], m_Context.m_GPRegs[1], m_Context.m_GPRegs[2], m_Context.m_GPRegs[3],
            m_Context.m_GPRegs[4], m_Context.m_GPRegs[5], m_Context.m_GPRegs[6], m_Context.m_GPRegs[7],
            m_Context.m_GPRegs[8], m_Context.m_GPRegs[9], m_Context.m_GPRegs[10], m_Context.m_GPRegs[11],
            m_Context.m_GPRegs[12], m_Context.m_SP, m_Context.m_LR, m_Context.m_PC, m_Context.m_CPSR
        );
    }

    void Thread::SetWaitState(u64 timeout) {
        if (timeout != 0) {
            m_State = WAITING_TIMED;
            m_WaitTimeout.StartMs(timeout);
        }
        else {
            m_State = WAITING;
        }
    }

    bool Thread::IsWaiting() {
        return m_State != RUNNING;
    }

    bool Thread::ResumeOnTimeout() {
        if (m_WaitTimeout.TimedOut()) {
            m_State = RUNNING;
            return true;
        }
        return false;
    }

    void Thread::Update() {
        switch (m_State) {
            case NEW:
                m_State = RUNNING;
                break;
            case ASLEEP:
                ResumeOnTimeout();
                break;
            case WAITING_TIMED:
                if (ResumeOnTimeout()) {
                    m_WaitChain.Remove();
                }
                break;
        }
    }

    void Thread::Unlink() {
        m_NextThread->m_PrevThread = m_PrevThread;
        m_PrevThread->m_NextThread = m_NextThread;
    }

    void Thread::AppendTo(Thread* prev) {
        this->m_PrevThread = prev;
        this->m_NextThread = prev->m_NextThread;
        prev->m_NextThread->m_PrevThread = this;
        prev->m_NextThread = this;
    }

    void Thread::Swap(Thread* newPredecessor) {
        Unlink();
        AppendTo(newPredecessor);
    }

    void Thread::__kCurrentThreadExit() {
        Exit(0);
    }

    ProcThread::ProcThread(const char* name, void* stack, size_t stackSize, ThreadFuncEx<void*> proc, void* arg) : Thread(name, stack, stackSize) {
        m_Proc = proc;
        m_Arg = arg;
    }

    void ProcThread::Run() {
        m_Proc(m_Arg);
    }
}

#endif