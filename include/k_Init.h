#ifndef __K_INIT_H
#define __K_INIT_H

namespace k {
    class KInitializer;
}

#include "RPM_Api.h"
#include "kDLL.h"

namespace k {
    class KInitializer {
    public:
        static void DllMainImpl(RPM_DLLAPI_DLLMAIN_PARAMS);
    };

    RPM_DLLAPI_DLLMAIN_DECLARE;
}

#endif