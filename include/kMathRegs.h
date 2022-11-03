#ifndef __KMATHREGS_H
#define __KMATHREGS_H

#include "k_Regs.h"
#include "kTypes.h"

K_REG_DEFINE(DIVCNT, u16, 0x04000280)
K_REG_DEFINE(DIV_NUMER, s64, 0x04000290)
K_REG_DEFINE(DIV_DENOM, s64, 0x04000298)
K_REG_DEFINE(DIV_RESULT, s64, 0x040002A0)
K_REG_DEFINE(DIVREM_RESULT, s64, 0x040002A8)
K_REG_DEFINE(SQRTCNT, u16, 0x040002B0)
K_REG_DEFINE(SQRT_RESULT, u32, 0x040002B4)
K_REG_DEFINE(SQRT_PARAM, u64, 0x040002B8)

namespace k {
    struct DivRegs {
        volatile u16 Control;
        const u64 _dummy;
        volatile s64 Numer;
        volatile s64 Denom;
        const volatile s64 Result;
    };

    struct SqrtRegs {
        volatile u16 Control;
        const volatile u32 Result;
        volatile u64 Param;
    };

    static_assert(offsetof(DivRegs, Numer) == 0x10);
    static_assert(offsetof(SqrtRegs, Param) == 0x8);

    enum DivControlBit {
        /**
         * @brief 32-bit numerator / 32-bit denominator -> 32-bit result, remainder (18 cycles)
         */
        DIV_MODE_32N_32D = 0,
        /**
         * @brief 64-bit numerator / 32-bit denominator -> 64-bit result, 32-bit remainder (34 cycles)
         */
        DIV_MODE_64N_32D = 1,
        /**
         * @brief 64-bit numerator / 64-bit denominator -> 32-bit result, remainder (34 cycles)
         */
        DIV_MODE_64N_64D = 2,
        /**
         * @brief Raised by the divider if the denominator is zero.
         */
        DIV_BY_ZERO_ERROR = (1 << 14),
        /**
         * @brief Set while the division is pending.
         */
        DIV_BUSY = (1 << 15)
    };

    INLINE void DivWait() {
        while (K_REG_GET(DIVCNT) & DivControlBit::DIV_BUSY) {
            ;
        }
    }
}

K_REG_DEFINE_STRUCT(DIVREGS, k::DivRegs, 0x04000280)
K_REG_DEFINE_STRUCT(SQRTREGS, k::SqrtRegs, 0x040002B0)

#endif