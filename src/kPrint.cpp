#ifndef __KUTIL_PRINT_CPP
#define __KUTIL_PRINT_CPP

#include <cstdarg>
#include "kPrint.h"
#include "Util/exl_Print.h"

namespace k {

    static Printer* g_kSystemPrinter = nullptr;

    extern "C" void kPrintSetSystemPrinter(Printer* printer) {
        g_kSystemPrinter = printer;
    }

    void Print(const char* str) {
        exlPrint(str);
        if (g_kSystemPrinter) {
            g_kSystemPrinter->Print(str);
        }
    }

    void Printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        char outBuffer[256];
        vsnprintf(outBuffer, 256, format, args);
        exlPrint(outBuffer);
        if (g_kSystemPrinter) {
            g_kSystemPrinter->Printf(format, args);
        }
        va_end(args);
    }
}

#endif