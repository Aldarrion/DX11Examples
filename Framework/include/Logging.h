#pragma once

#include <cassert>

struct ID3DUserDefinedAnnotation;

namespace ex {
enum class LogLevel {
    Info,
    Warning,
    Error
};

void log(LogLevel level, const char* formatString, ...);

void beginEvent(ID3DUserDefinedAnnotation* perf, const wchar_t* name);
void endEvent(ID3DUserDefinedAnnotation* perf);

}

