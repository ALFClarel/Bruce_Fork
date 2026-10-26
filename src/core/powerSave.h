#pragma once
#include "display.h"
#include <globals.h>

void checkPowerSaveTime();

void sleepModeOn();

void sleepModeOff();

void fadeOutScreen(int startValue);

#ifdef BRUCE_TEMBED_BASE
void observeBattery(unsigned percent, unsigned millivolts, bool charging);
uint8_t powerBrightnessLimit(uint8_t requested);
uint16_t powerScanWindow(uint16_t requested);
void powerScanYield();
bool powerBatteryCritical();
#else
inline uint8_t powerBrightnessLimit(uint8_t requested) { return requested; }
inline uint16_t powerScanWindow(uint16_t requested) { return requested; }
inline void powerScanYield() {}
inline bool powerBatteryCritical() { return false; }
#endif
