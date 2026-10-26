#pragma once
#include <stdint.h>

namespace tembed {
// CC1101 usable bands; arithmetic is in integer kHz to avoid float drift.
inline int32_t tuneKhz(int32_t current, int32_t steps, int32_t step = 10) {
    if (!steps) return current;
    int64_t next = int64_t(current) - int64_t(steps) * step;
    if (next < 300000) return 300000;
    if (next > 928000) return 928000;
    if (next > 348000 && next < 387000) return steps < 0 ? 387000 : 348000;
    if (next > 464000 && next < 779000) return steps < 0 ? 779000 : 464000;
    return int32_t(next);
}

inline uint8_t signalPercent(int value, int floor, int ceiling) {
    if (ceiling <= floor || value <= floor) return 0;
    if (value >= ceiling) return 100;
    return uint8_t((value - floor) * 100 / (ceiling - floor));
}
} // namespace tembed
