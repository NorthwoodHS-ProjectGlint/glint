#include "glint/glint.h"

#include <iostream>
#include <cstdarg>

void ioDebugPrint(const char *format, ...)
{
    #if DEBUG



    #endif

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}