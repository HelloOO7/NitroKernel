#ifndef __KGUARD_H
#define __KGUARD_H

#include "k_DllExport.h"
#include "kTypes.h"
#include "kSpinLock.h"
#include "kThread.h"

namespace k {
    class Guard {
        private:
            Thread*  m_Owner;
        
        public:
            INLINE Guard() {
                m_Owner = nullptr;
            }

            void Acquire();
            void Release();
    };
}

#endif