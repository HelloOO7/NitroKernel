#ifndef __KTHREAD_H
#define __KTHREAD_H

namespace k {
    class Thread;

    typedef void (*ThreadFunc)();
    template<typename T>
    using ThreadFuncEx = void (*)(T);

    struct ThreadChain {
        Thread*      m_Parent;
        ThreadChain* m_Next;
        ThreadChain* m_Prev;

        ThreadChain(Thread* parent);

        void Append(ThreadChain* next);
        void Remove();
    };

    struct ThreadContext {
        int m_GPRegs[13];
        int m_SP;
        int m_LR;
        int m_PC;
        int m_CPSR;

        u64 m_DivNumer;
        u64 m_DivDenom;
        u64 m_SqrtParam;
        u16 m_DivCnt;
        u16 m_SqrtCnt;

        void SaveGP();
        void LoadGP();
        void SaveMathRegs();
        void LoadMathRegs();

        int SaveSYS();
        void LoadIRQ();
        void LoadSYS();
    };

    enum ThreadState {
        NEW,
        RUNNING,
        ASLEEP,
        WAITING,
        WAITING_TIMED,
        EXITED
    };
}

#include "kTypes.h"
#include "k_Init.h"
#include "k_DllExport.h"
#include "k_ARMAttributes.h"
#include "kTiming.h"
#include "kWaitObject.h"

namespace k {

    class Thread {
        friend class WaitObject;
    public:
        class _InitClass {
        private:
            static void _Init(u32 timer, u32 threadQuantumCycles);
            static void _Terminate();
        
        friend class k::KInitializer;
        };
    private:
        class Scheduler {
        private:
            static void Init(u32 timer, u32 threadQuantumCycles);
            static bool IsSingleThread();
            static void SwitchThread();
            static void ManualReschedule(Thread* thread, Thread* desiredPrevious);

            static void _SaveCurThreadContext();
            static void _Irq();

            friend class _InitClass;
            friend class Thread;
        };
    
    protected:
        ThreadContext m_Context;

        ThreadState m_State;

        Thread*     m_NextThread;
        Thread*     m_PrevThread;

    protected:
        void* const     m_Stack;
        void* const     m_StackTop;
        const size_t    m_StackSize;

    private:
        const char* m_Name;

        ThreadChain m_JoinThreads;
        
        ThreadChain m_WaitChain;
        TimeCounter m_WaitTimeout;
        Thread*     m_NextWaitThread;

    public:
        K_PUBLIC Thread(const char* name, void* stack, size_t stackSize);
        K_PUBLIC ~Thread();

        K_PUBLIC virtual void Run();
    
    public:
        K_PUBLIC static u32 ActiveCount();
        K_PUBLIC static Thread* CurrentThread();
        K_PUBLIC static void Yield();
        K_PUBLIC static void Yield(Thread* yieldTo, bool scheduleReturn);
        K_PUBLIC static void Sleep(u64 millis);
        K_PUBLIC static void WaitOn(WaitObject* waitObj, u64 timeout);
        K_PUBLIC static void Exit(int exitCode);
    
    public:
        K_PUBLIC void Start();
        K_PUBLIC void Stop();
        K_PUBLIC void Join(u64 timeout);
        K_PUBLIC INLINE void Join() {
            Join(0);
        }
    
    private:
        static void _RunProc(Thread* _this);
        void _Yield();
        void _Yield(Thread* yieldTo, bool scheduleReturn);
        void _Sleep(u64 millis);
        void _WaitOn(WaitObject* waitObj, u64 timeout);
        void _Exit(int exitCode);
        void _Join(Thread* joiner, u64 timeout);

    public:
        K_PUBLIC INLINE const char* GetName() {
            return m_Name;
        }
        
        K_PUBLIC INLINE ThreadState GetState() {
            return m_State;
        }
    
    private:
        void SetWaitState(u64 timeout);
        bool ResumeOnTimeout();
        bool IsWaiting();
        void Update();
        void DumpThreadState();
    private:
        void Unlink();
        void AppendTo(Thread* prev);
        void Swap(Thread* newPredecessor);

    private:
        static void __kCurrentThreadExit();
    };

    class ProcThread : public Thread {
    private:
        ThreadFuncEx<void*> m_Proc;
        void*               m_Arg;
    public:
        K_PUBLIC ProcThread(const char* name, void* stack, size_t stackSize, ThreadFuncEx<void*> proc, void* arg);
        
        template<typename T>
        K_PUBLIC INLINE
        ProcThread(const char* name, void* stack, size_t stackSize, ThreadFuncEx<T> proc, T arg) : ProcThread(name, stack, stackSize, (ThreadFuncEx<void*>) proc, arg) {
        
        }
        
        K_PUBLIC INLINE 
        ProcThread(const char* name, void* stack, size_t stackSize, ThreadFunc proc) : ProcThread(name, stack, stackSize, (ThreadFuncEx<nullptr_t>)proc, nullptr) {

        }

        K_PUBLIC void Run() override;
    };
}

#endif