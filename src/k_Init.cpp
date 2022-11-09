#include "k_Init.h"

#include "kDLL.h"
#include "kThread.h"
#include "kCriticalSection.h"
#include "kInterrupt.h"

namespace k {
    RPM_DLLAPI_DLLMAIN_DEFINE {
        KInitializer::DllMainImpl(mgr, module, reason);
        return rpm::DllMainReturnCode::OK;
    }

    void KInitializer::DllMainImpl(RPM_DLLAPI_DLLMAIN_PARAMS) {
        k::CriticalSection cs = k::CriticalSection::Enter(); //wouldn't wanna get interrupted when this is unloading/unloaded
        switch (reason) {
            case rpm::MODULE_LOAD:
                k::Interrupt::_InitClass::_Init();
                k::dll::_InitClass::_SysInit(mgr);
                k::Thread::_InitClass::_Init(3, 65536);
                break;
            case rpm::MODULE_UNLOAD:
                k::dll::_InitClass::_SysTerminate();
                k::Thread::_InitClass::_Terminate();
                k::Interrupt::_InitClass::_Terminate();
                break;
        }
        cs.Leave();
    }
}