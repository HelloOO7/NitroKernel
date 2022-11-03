#ifndef __KSPINLOCK_CPP
#define __KSPINLOCK_CPP

#include "kTiming.h"
#include "kThread.h"
#include "k_ARMCPU.h"
#include "k_ARMAttributes.h"

#include "kSpinLock.h"

namespace k {

    ARMFUNC
    void SpinLock::AcquireBusy(int spinPeriod) {
        while (TryLock_inl() != State::AVAILABLE) {
            WaitCycles(spinPeriod);
        }
    }

    ARMFUNC
    void SpinLock::Acquire() {
        while (TryLock_inl() != State::AVAILABLE) {
            Thread::Yield();
        }
    }

    void SpinLock::Release() {
        m_Lock = State::AVAILABLE;
    }

    ARMFUNC
    SpinLock::State SpinLock::TryLock() {
        State lock;
        asm volatile("SWP %0, %1, [%2]" : "=&r" (lock) : "r" (State::LOCKED), "r" (&m_Lock) : "memory");
        return lock;
    }
}

#endif