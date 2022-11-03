#ifndef __KMUTEX_H
#define __KMUTEX_H

#include "kSpinLock.h"
#include "kThread.h"
#include "kGuard.h"
#include "kWaitObject.h"

namespace k {
    class Mutex : protected WaitObject {
    private:
        Guard   m_Guard;
        Thread* m_Owner;
        u32     m_LockCount;

    public:
        K_PUBLIC void Init();
        K_PUBLIC void Acquire();
        K_PUBLIC void Release();
    };
}

#endif