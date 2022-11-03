#ifndef __KTIMING_CPP
#define __KTIMING_CPP

#include "kTypes.h"
#include "kTiming.h"

namespace k {
    void GetTimerConfigForIntervalCycles(u32 cycles, volatile u16* pReload, volatile u16* pControl) {
        TimerControlBit scale = TIMER_SCALE_1;
        if (cycles > TIMER_MAX_VALUE * TIMER_SCALE_MUL_256) {
            scale = TIMER_SCALE_1024;
            cycles >>= 10;
        }
        else if (cycles >= TIMER_MAX_VALUE * TIMER_SCALE_MUL_64) {
            scale = TIMER_SCALE_256;
            cycles >>= 8;
        }
        else if (cycles >= TIMER_MAX_VALUE * TIMER_SCALE_MUL_1) {
            scale = TIMER_SCALE_64;
            cycles >>= 6;
        }
        else {
            scale = TIMER_SCALE_1;
        }
        *pReload = TIMER_MAX_VALUE - cycles;
        *pControl = TIMER_IRQ_ENABLE | TIMER_START | scale;
    }
}

#endif