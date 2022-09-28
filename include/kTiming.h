#ifndef __KTIMING_H
#define __KTIMING_H

#include "kTypes.h"
#include "k_DllExport.h"
#include "time.h"

namespace k {
    K_PUBLIC INLINE void WaitCycles(int cycles) {
        asm volatile("SWI 0x3");
    }

    K_PUBLIC INLINE void WaitMs(int ms) {
        WaitCycles(ms * 0x4174);
    }

    K_PUBLIC INLINE u64 SystemTimeUs() {
        return (u64)clock() * 64000 / 33514;
    }

    K_PUBLIC INLINE u64 SystemTimeMs() {
        return ((u64)clock() << 6LL) / 33514;
    }
}

#endif