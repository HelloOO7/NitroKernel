#ifndef __KWAITOBJECT_H
#define __KWAITOBJECT_H

namespace k {
    class WaitObject;
}

#include "kThread.h"
#include "k_DllExport.h"

namespace k {
    class WaitObject {
    private:
        ThreadChain m_WaitThreads;

    friend class Thread;

    public:
        INLINE WaitObject() : m_WaitThreads(nullptr) {
        }

        K_PUBLIC bool Notify();
        K_PUBLIC bool NotifyAll();
    protected:
        INLINE void Reset() {
            m_WaitThreads.m_Next = &m_WaitThreads;
            m_WaitThreads.m_Prev = &m_WaitThreads;
        }
    };
}

#endif