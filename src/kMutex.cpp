#ifndef __KMUTEX_CPP
#define __KMUTEX_CPP

#include "kSpinLock.h"
#include "kThread.h"
#include "kMutex.h"
#include "kCriticalSection.h"

namespace k {

    void Mutex::Init() {
        Reset();
        m_Guard = Guard();
        m_LockCount = 0;
        m_Owner = nullptr;
    }

    void Mutex::Acquire() {
        TryAcquire:
        m_Guard.Acquire();
        Thread* curThread = Thread::CurrentThread();
        if (m_Owner) {
            if (m_Owner == curThread) {
                m_LockCount++;
            }
        }
        else {
            m_Owner = curThread;
            m_LockCount = 1;
        }
        bool wait = m_Owner != curThread;
        //Enter a critical section so that the mutex is not released by the owner
        //before we start waiting on it (otherwise we would be waiting on an available mutex forever)
        CriticalSection cs = CriticalSection::Enter();
        m_Guard.Release();
        if (wait) {
            Thread::WaitOn(this, 0);
            cs.Leave();
            goto TryAcquire; //wait finished, try to claim ownership of mutex again
        }
        else {
            cs.Leave();
        }
    }

    void Mutex::Release() {
        m_Guard.Acquire();
        //Critical section to prevent thread switching, which could
        //Needlessly invoke the newly notified thread which would immediatelly stall
        //since the guard is not yet released
        CriticalSection cs = CriticalSection::Enter();
        if (Thread::CurrentThread() == m_Owner) {
            m_LockCount--;
            if (m_LockCount == 0) {
                m_Owner = nullptr;
                Notify(); //notify only one thread as only one can access the mutex
            }
        }
        m_Guard.Release();
        cs.Leave();
    }
}

#endif