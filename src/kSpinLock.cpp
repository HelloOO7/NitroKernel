#ifndef __KSPINLOCK_CPP
#define __KSPINLOCK_CPP

#include "kSpinLock.h"
#include "kTiming.h"
#include "k_ARMCPU.h"

namespace k {

    ARMFUNC
    void SpinLock::Acquire(int spinPeriod) {
        void* semaphoreAddr = &m_Lock;
        int lock;
        while (true) {
            asm volatile("SWP %0, %1, [%2]" : "=&r" (lock) : "r" (State::LOCKED), "r" (semaphoreAddr) : "memory");
            if (lock == State::AVAILABLE) {
                break;
            }
            k::WaitCycles(spinPeriod);
        }
    }

    void SpinLock::Release() {
        m_Lock = State::AVAILABLE;
    }
}

#endif