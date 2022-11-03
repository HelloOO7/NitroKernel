#ifndef __KGUARD_CPP
#define __KGUARD_CPP

#include "k_ARMAttributes.h"
#include "kTypes.h"
#include "kSpinLock.h"
#include "kThread.h"
#include "kGuard.h"
#include "kCriticalSection.h"

namespace k {

    void Guard::Acquire() {
        CriticalSection cs = CriticalSection::Enter();
        bool first = true;
        while (m_Owner != nullptr) {
            Thread::Yield(m_Owner, first);
            first = false;
        }
        cs.Leave();
    }

    void Guard::Release() {
        CriticalSection cs = CriticalSection::Enter();
        if (m_Owner == Thread::CurrentThread()) {
            m_Owner = nullptr;
        }
        cs.Leave();
    }
}

#endif