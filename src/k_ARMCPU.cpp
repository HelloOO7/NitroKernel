#ifndef __ARMCPU_CPP
#define __ARMCPU_CPP

#include "k_ARMCPU.h"
#include "k_ARMAttributes.h"

namespace arm {

    ARMFUNC
    void* GetDTCMAddress() {
        void* dtcmAddress;
        asm volatile ("MRC p15, 0, %0, c9, c1, 0" : "=r" (dtcmAddress) ::);
        return dtcmAddress;
    }

    ARMFUNC
    int GetCPSR() {
        int ret;
        asm volatile("MRS %0, CPSR" : "=r" (ret) : :);
        return ret;
    }

    ARMFUNC
    void SetCPSR(int bits) {
        asm volatile(
            "MOVS R0, LR\n"
            "MSR CPSR_cxsf, %0\n"
            "BX R0"
             : : "r" (bits) :
        );
    }

    ARMFUNC
    void SetCPSR_c(int bits) {
        asm volatile(
            "MOVS R0, LR\n"
            "MSR CPSR_c, %0\n"
            "BX R0"
            : : "r" (bits) :
        );
    }

    ARMFUNC
    void SetCPUMode(CPUMode mode) {
        int cpsr = GetCPSR();
        SetCPSR_c(cpsr & ~arm::MASK_MODE | mode);
    }
}

#endif