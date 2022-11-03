#ifndef __KTIMING_H
#define __KTIMING_H

#include "kTypes.h"
#include "k_DllExport.h"
#include "time.h"
#include "kInterrupt.h"
#include "k_Regs.h"

K_REG_DEFINE(TM0CNT_L, u16, 0x04000100)
K_REG_DEFINE(TM1CNT_L, u16, 0x04000104)
K_REG_DEFINE(TM2CNT_L, u16, 0x04000108)
K_REG_DEFINE(TM3CNT_L, u16, 0x0400010C)
K_REG_DEFINE(TM0CNT_H, u16, 0x04000102)
K_REG_DEFINE(TM1CNT_H, u16, 0x04000106)
K_REG_DEFINE(TM2CNT_H, u16, 0x0400010A)
K_REG_DEFINE(TM3CNT_H, u16, 0x0400010E)

namespace k {
    enum TimerControlBit {
        TIMER_SCALE_1 = 0,
        TIMER_SCALE_64 = 1,
        TIMER_SCALE_256 = 2,
        TIMER_SCALE_1024 = 3,
        TIMER_CASCADE = 4,
        TIMER_IRQ_ENABLE = 64,
        TIMER_START = 128
    };

    enum TimerConstants {
        TIMER_SCALE_MUL_1 = 1,
        TIMER_SCALE_MUL_64 = 64,
        TIMER_SCALE_MUL_256 = 256,
        TIMER_SCALE_MUL_1024 = 1024,

        TIMER_MAX_VALUE = 0xFFFF,
    };

    struct TimerRegs {
        union {
            const volatile u16 Counter;
            volatile u16 Reload;
        };
        volatile u16 Control;
    };

    K_PUBLIC INLINE TimerRegs* GetTimerRegs(int timerIndex) {
        return K_REG_GET_ARRAY_NV(TimerRegs, 0x04000100, 4, timerIndex);
    }

    K_PUBLIC INLINE InterruptID TimerInterruptID(int timerIndex) {
        return (InterruptID) (TIMER_0 + timerIndex);
    }

    K_PUBLIC INLINE InterruptBit TimerInterruptBits(int timerIndex) {
        return (InterruptBit) (BIT_TIMER_0 << timerIndex);
    }

    K_PUBLIC INLINE u64 MsToClock(u64 ms) {
        return (ms * 33514LL) >> 6LL;
    }

    K_PUBLIC void GetTimerConfigForIntervalCycles(u32 cycles, volatile u16* pReload, volatile u16* pControl);

    K_PUBLIC INLINE void GetTimerConfigForIntervalMs(u32 ms, volatile u16* pReload, volatile u16* pControl) {
        GetTimerConfigForIntervalCycles(MsToClock(ms), pReload, pControl);
    }

    K_PUBLIC INLINE void WaitCycles(u32 cycles) {
        asm volatile("SWI 0x3");
    }

    K_PUBLIC INLINE void WaitMs(u32 ms) {
        WaitCycles(ms * 0x4174);
    }

    K_PUBLIC INLINE u64 SystemTick() {
        return (u64)clock();
    }

    K_PUBLIC INLINE u64 SystemTimeUs() {
        return SystemTick() * 64000 / 33514;
    }

    K_PUBLIC INLINE u64 SystemTimeMs() {
        return (SystemTick() << 6LL) / 33514;
    }

    struct TimeCounter {
    private:
        u64 m_Start;
        u64 m_End;

    public:
        K_PUBLIC INLINE void StartCycles(u64 timeoutCycles) {
            m_Start = SystemTick();
            m_End = m_Start + timeoutCycles;
        }
                
        K_PUBLIC INLINE void StartMs(u64 timeoutMs) {
            StartCycles(MsToClock(timeoutMs));
        }

        K_PUBLIC INLINE bool TimedOut() {
            return SystemTick() >= m_End;
        }
    };
}

#endif