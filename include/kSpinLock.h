#ifndef __KSPINLOCK_H
#define __KSPINLOCK_H

#include "k_DllExport.h"
#include "kTypes.h"

namespace k {
    class SpinLock {
    public:
        enum State {
            AVAILABLE,
            LOCKED
        };
    private:
        int m_Lock;
    
    public:
        K_PUBLIC INLINE SpinLock() {
            m_Lock = AVAILABLE;
        }

        K_PUBLIC void AcquireBusy(int spinPeriod);

        K_PUBLIC INLINE void AcquireBusy() {
            AcquireBusy(0);
        }

        K_PUBLIC void Acquire();

        K_PUBLIC void Release();

        K_PUBLIC State TryLock();

        K_PUBLIC INLINE State TryLock_inl() {
            State lock;
            asm volatile("SWP %0, %1, [%2]" : "=&r" (lock) : "r" (State::LOCKED), "r" (&m_Lock) : "memory");
            return lock;
        }
    };
}

#endif