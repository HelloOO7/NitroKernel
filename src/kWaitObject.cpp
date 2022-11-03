#ifndef __KWAITOBJECT_CPP
#define __KWAITOBJECT_CPP

#include "kTypes.h"
#include "kThread.h"
#include "kWaitObject.h"
#include "kCriticalSection.h"

namespace k {

    bool WaitObject::Notify() {
        bool ret = false;
        CriticalSection cs = CriticalSection::Enter();
        ThreadChain* next = m_WaitThreads.m_Next;
        if (next != &m_WaitThreads) {
            Thread* container = next->m_Parent;
            container->m_State = RUNNING;
            next->Remove();
            ret = true;
        }
        cs.Leave();
        return ret;
    }

    bool WaitObject::NotifyAll() {
        CriticalSection cs = CriticalSection::Enter();
        bool ret = Notify(); //at least one
        if (ret) {
            while (Notify()) {
                ;
            }
        }
        cs.Leave();
        return ret;
    }
}

#endif