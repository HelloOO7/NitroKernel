#ifndef __KSPINLOCK_H
#define __KSPINLOCK_H

#include "k_DllExport.h"
#include "kTypes.h"

namespace k {
    class SpinLock {
    private:
        enum State {
            AVAILABLE,
            LOCKED
        };

        int m_Lock;
    
    public:
        K_PUBLIC void Acquire(int spinPeriod);

        K_PUBLIC INLINE void Acquire() {
            Acquire(0);
        }

        K_PUBLIC void Release();
    };
}

#endif