#ifndef __KCRITITICALSECTION_H
#define __KCRITITICALSECTION_H

#include "kTypes.h"
#include "k_DllExport.h"

namespace k {
    class CriticalSection {
    private:
        int m_IRQBackup;

        K_PUBLIC static CriticalSection Enter();
        K_PUBLIC static void Leave(CriticalSection criticalSection);
    };
}

#endif