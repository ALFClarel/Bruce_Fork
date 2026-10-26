#pragma once
#include <Arduino.h>
#include <FS.h>
#ifdef BRUCE_TEMBED_BASE
bool isCapturePath(const String &path);
bool annotateCapture(FS &fs, const String &path, uint32_t frequencyHz = 0, const String &protocol = "");
#endif
