#ifndef __ARMCPU_H
#define __ARMCPU_H

#include "k_ARMAttributes.h"
#include "kTypes.h"

namespace arm {
    enum CPUMode {
        MODE_USER = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_MON = 0x16,
        MODE_ABT = 0x17,
        MODE_HYP = 0x1A,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum CPSRMask {
        MASK_MODE = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQ = 0x40,
        MASK_IRQ = 0x80
    };

    void* GetDTCMAddress();
    
    int GetCPSR();

    void SetCPSR(int bits);

    void SetCPSR_c(int bits);

    void SetCPUMode(CPUMode mode);

    INLINE int GetCPSR_inl() {
        int ret;
        asm volatile("MRS %0, CPSR" : "=r" (ret) : :);
        return ret;
    }

    INLINE void SetCPSR_inl(int bits) {
        asm volatile(
            "MSR CPSR_cxsf, %0\n" : : "r" (bits) :
        );
    }

    INLINE void SetCPSR_c_inl(int bits) {
        asm volatile(
            "MSR CPSR_c, %0\n" : : "r" (bits) :
        );
    }

    INLINE void* GetSP_inl() {
        void* sp;
        asm volatile (
            "MOVS %0, SP"
            : "=r" (sp) ::
        );
        return sp;
    }
}

#endif