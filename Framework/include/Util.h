#pragma once

namespace Util {

inline float lerp(const float a, const float b, const float t) {
    return a + t * (b - a);
}
}
