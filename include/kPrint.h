#ifndef __KUTIL_PRINT_H
#define __KUTIL_PRINT_H

#include <cstdarg>
#include "k_DllExport.h"

namespace k {
    class Printer {
    public:
        virtual void Print(const char* str) = 0;
        virtual void Printf(const char* format, va_list va) = 0;
    };

    extern "C" K_PUBLIC void kPrintSetSystemPrinter(Printer* printer);

    K_PUBLIC void Print(const char* str);

    K_PUBLIC void Printf(const char* format, ...);
}

#endif