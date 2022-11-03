#ifndef __KINTERRUPT_CPP
#define __KINTERRUPT_CPP

#include "kInterrupt.h"
#include "kCriticalSection.h"
#include "k_ARMAttributes.h"
#include "k_Regs.h"
#include "k_ARMCPU.h"

namespace k {
    static InterruptFunc g_IntrVec[32];
    static InterruptFunc g_SysIntrFunc;
    static InterruptFunc* g_SysIntrVec;

    static InterruptFunc* GetIrqHandlerAddress(void* dtcmAddress) {
        InterruptFunc* irqHandlerPtrAddr = reinterpret_cast<InterruptFunc*>(static_cast<char*>(dtcmAddress) + 0x3FFC);
        return irqHandlerPtrAddr;
    }

    ARMFUNC
    void Interrupt::_InitClass::_Init() {
        k::CriticalSection cs = k::CriticalSection::Enter();
        void* dtcm = arm::GetDTCMAddress();
        InterruptFunc* irqHandlerPtrAddr = GetIrqHandlerAddress(dtcm);
        g_SysIntrVec = reinterpret_cast<InterruptFunc*>(static_cast<char*>(dtcm) + 0x20);
        g_SysIntrFunc = *irqHandlerPtrAddr;
        *irqHandlerPtrAddr = Interrupt::HandleInterrupt;
        memset(g_IntrVec, 0, sizeof(g_IntrVec));
        cs.Leave();
    }

    ARMFUNC
    void Interrupt::_InitClass::_Terminate() {
        k::CriticalSection cs = k::CriticalSection::Enter();
        InterruptFunc* irqHandlerPtrAddr = GetIrqHandlerAddress(arm::GetDTCMAddress());
        *irqHandlerPtrAddr = g_SysIntrFunc;
        cs.Leave();
    }
    
    ARMFUNC LOWLEVEL IRQFUNC
    void Interrupt::HandleInterrupt() {
        //Ensure we are only using the BIOS-saved registers
        //Index MUST be in R0 for the branch to g_SysIntrFunc to work
        register unsigned int index asm("r0");
        unsigned int irq;
        InterruptFunc func;
        register int clz asm("r12");
        if (K_REG_GET(IME)) {
            irq = K_REG_GET(IE) & K_REG_GET(IF);
            if (irq != 0) {
                index = 32;
                while (irq != 0) {
                    asm volatile ("CLZ %0, %1" : "=r" (clz) : "r" (irq));
                    clz++;
                    index -= clz;
                    irq <<= clz;
                }
                K_REG_SET(IF, (1 << index));
                func = g_IntrVec[index];
                if (!func) {
                    asm volatile ("PUSH {LR}");
                    //This executes the epilogue of the system interrupt
                    //The offset is hardcoded, but less hardcoded than loading the interrupt vector ourselves
                    func = (InterruptFunc)((char*)g_SysIntrFunc + 0x40);
                }
                if (func) {
                    //Ensure branch that keeps LR intact
                    asm volatile (
                        "BX %0\n"
                        : : "r" (func)
                    );    
                }
            }
        }
        asm ("BX LR");
    }

    void Interrupt::Set(InterruptBit intrBits, InterruptFunc func) {
        k::CriticalSection cs = k::CriticalSection::Enter();
        unsigned int bitsInt = intrBits;
        for (int index = 0; bitsInt != 0; index++) {
            if ((intrBits & 1) != 0) {
                g_IntrVec[index] = func;
            }
            bitsInt >>= 1;
        }
        cs.Leave();
    }

    void Interrupt::Set(InterruptID intrId, InterruptFunc func) {
        k::CriticalSection cs = k::CriticalSection::Enter();
        g_IntrVec[intrId] = func;
        cs.Leave();
    }

    void Interrupt::Enable(InterruptBit mask) {
        K_REG_GET(IE) |= mask;
    }

    void Interrupt::Disable(InterruptBit mask) {
        K_REG_GET(IE) &= ~mask;
        Interrupt::Set(mask, nullptr);
    }

    bool Interrupt::IsEnabled(InterruptBit mask) {
        return (K_REG_GET(IE) & mask) != 0;
    }
};

#endif