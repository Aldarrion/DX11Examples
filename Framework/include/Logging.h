#pragma once

#include <cassert>

namespace ex {
enum class LogLevel {
    Info,
    Warning,
    Error
};

void log(LogLevel level, const char* formatString, ...);
}

