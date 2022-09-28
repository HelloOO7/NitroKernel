#ifndef __KUTIL_PRINT_H
#define __KUTIL_PRINT_H

#include "k_DllExport.h"
#include "Util/exl_Print.h"

namespace k {
    K_PUBLIC void Print(const char* str);

    K_PUBLIC void Printf(const char* format, ...);
}

#endif