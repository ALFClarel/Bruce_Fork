#include "tembed_policy.h"
#include <cassert>
#include <climits>
int main() {
    using namespace tembed;
    assert(tuneKhz(433920, -1) == 433930);
    assert(tuneKhz(348000, -1) == 387000);
    assert(tuneKhz(387000, 1) == 348000);
    assert(tuneKhz(464000, -1) == 779000);
    assert(tuneKhz(779000, 1) == 464000);
    assert(tuneKhz(300000, INT_MAX) == 300000);
    assert(tuneKhz(928000, INT_MIN) == 928000);
    assert(signalPercent(-120, -100, -30) == 0);
    assert(signalPercent(-30, -100, -30) == 100);
    assert(signalPercent(-65, -100, -30) == 50);
    assert(signalPercent(4, 5, 5) == 0);
    BatteryPolicy battery;
    assert(!battery.observe(65535, 65535, false));
    for (int i = 0; i < 2; ++i) battery.observe(15, 3700, false);
    assert(battery.level == PowerLevel::Normal);
    battery.observe(15, 3700, false);
    assert(battery.level == PowerLevel::Low);
    for (int i = 0; i < 4; ++i) battery.observe(18, 3700, false);
    assert(battery.level == PowerLevel::Low); // hysteresis prevents flicker
    for (int i = 0; i < 3; ++i) battery.observe(20, 3700, false);
    assert(battery.level == PowerLevel::Normal);
    for (int i = 0; i < 3; ++i) battery.observe(0, 3250, false);
    assert(battery.level == PowerLevel::Critical);
    for (int i = 0; i < 3; ++i) battery.observe(9, 3400, false);
    assert(battery.level == PowerLevel::Critical);
    for (int i = 0; i < 3; ++i) battery.observe(0, 3400, true);
    assert(battery.level == PowerLevel::Normal);
}
