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
enum class PowerLevel : uint8_t { Normal, Low, Critical };
struct BatteryPolicy {
    PowerLevel level = PowerLevel::Normal;
    PowerLevel candidate = PowerLevel::Normal;
    uint8_t samples = 0;
    bool observe(unsigned percent, unsigned millivolts, bool charging) {
        if (percent > 100 || millivolts < 2500 || millivolts > 5000) return false;
        PowerLevel next = PowerLevel::Normal;
        if (!charging) {
            if (percent <= 5 || millivolts <= 3300 ||
                (level == PowerLevel::Critical && (percent < 8 || millivolts < 3450)))
                next = PowerLevel::Critical;
            else if (percent <= 15 || millivolts <= 3500 ||
                     (level == PowerLevel::Low && (percent < 20 || millivolts < 3600)))
                next = PowerLevel::Low;
        }
        if (next != candidate) { candidate = next; samples = 0; }
        if (samples < 3) ++samples;
        if (samples == 3) level = candidate;
        return true;
    }
};
} // namespace tembed
