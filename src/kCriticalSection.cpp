#ifndef __KCRITITICALSECTION_CPP
#define __KCRITITICALSECTION_CPP

#include "kTypes.h"
#include "k_ARMCPU.h"
#include "k_ARMAttributes.h"

#include "kCriticalSection.h"

namespace k {
    ARMFUNC
    CriticalSection CriticalSection::Enter() {
        CriticalSection cs;
        int cpsr;
        asm volatile("MRS %0, CPSR" : "=r" (cpsr) : :);
        asm volatile("MSR CPSR_c, %0" : : "r" (cpsr | arm::MASK_IRQ) :);
        cs.m_IRQBackup = cpsr & arm::MASK_IRQ;
        return cs;
    }

    ARMFUNC
    void CriticalSection::Leave() {
        int cpsr;
        asm volatile("MRS %0, CPSR" : "=r" (cpsr) : :);
        asm volatile("MSR CPSR_c, %0" : : "r" (cpsr & ~arm::MASK_IRQ | m_IRQBackup) :);
    }
}

#endif