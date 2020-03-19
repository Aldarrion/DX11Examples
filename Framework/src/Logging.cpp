#include "Logging.h"

#include <stdio.h>
#include <stdarg.h>

#define COM_NO_WINDOWS_H
#include <d3d11_1.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


namespace ex {
void log(LogLevel level, const char* formatString, ...) {
    va_list args;
    va_start(args, formatString);

    constexpr size_t buffSize = 2 << 13;
    char buffer[buffSize];

    const char* prefix = nullptr;

    switch (level) {
        case LogLevel::Info:
            prefix = "Info:    ";
            break;
        case LogLevel::Warning:
            prefix = "Warning: ";
            break;
        case LogLevel::Error:
            prefix = "Error:   ";
            break;
    }

    snprintf(buffer, buffSize - 1, prefix);

    const size_t prefixSize = strlen(prefix);
    const int len = vsnprintf(buffer + prefixSize, buffSize - 1 - prefixSize, formatString, args);
    assert(len >= 0 && "Logging failed, check the format string or exceeded length.");

    buffer[prefixSize + len] = '\n';
    buffer[prefixSize + len + 1] = '\0';

    OutputDebugStringA(buffer);
}

void beginEvent(ID3DUserDefinedAnnotation* perf, const wchar_t* name) {
    if (perf) {
        perf->BeginEvent(name);
    }
}

void endEvent(ID3DUserDefinedAnnotation* perf) {
    if (perf) {
        perf->EndEvent();
    }
}

}
