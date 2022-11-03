#ifndef __KCRITITICALSECTION_H
#define __KCRITITICALSECTION_H

#include "kTypes.h"
#include "k_DllExport.h"
#include "k_ARMAttributes.h"

namespace k {
    class CriticalSection {
    private:
        int m_IRQBackup;

    public:
        K_PUBLIC static CriticalSection Enter();
        K_PUBLIC void Leave();
    };
}

#endif