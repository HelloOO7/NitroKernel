#ifndef __KUTIL_PRINT_CPP
#define __KUTIL_PRINT_CPP

#include <cstdarg>
#include "kPrint.h"
#include "Util/exl_Print.h"

namespace k {
    void Print(const char* str) {
        exlPrint(str);
    }

    void Printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        char outBuffer[1024];
        vsnprintf(outBuffer, 1024, format, args);
        exlPrint(outBuffer);
        va_end(args);
    }
}

#endif