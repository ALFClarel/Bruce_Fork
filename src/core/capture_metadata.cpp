#ifdef BRUCE_TEMBED_BASE
#include "capture_metadata.h"
#include <ArduinoJson.h>
#include <globals.h>

bool isCapturePath(const String &path) {
    return path.endsWith(".sub") || path.endsWith(".ir") || path.endsWith(".rfid");
}

bool annotateCapture(FS &fs, const String &path, uint32_t frequencyHz, const String &protocol) {
    File capture = fs.open(path, FILE_READ);
    if (!capture) return false;
    String guess = protocol;
    // Inspect only a bounded prefix of the saved file, using the existing format fields.
    // Never allocate a potentially huge RAW_Data line just to extract a protocol name.
    char buffer[128];
    size_t used = 0;
    for (unsigned bytes = 0; capture.available() && bytes < 8192; ++bytes) {
        char c = char(capture.read());
        if (c == '\n') {
            buffer[used] = 0;
            String line(buffer);
            int split = line.indexOf(':');
            if (split > 0) {
                String key = line.substring(0, split);
                String value = line.substring(split + 1); value.trim(); key.toLowerCase();
                if (guess.isEmpty() && (key == "protocol" || key == "device type")) guess = value;
                if (!frequencyHz && key == "frequency") frequencyHz = strtoul(value.c_str(), nullptr, 10);
                if (guess.isEmpty() && key == "type" && value == "raw") guess = "RAW/unknown";
            }
            used = 0;
        } else if (used < sizeof(buffer) - 1) buffer[used++] = c;
    }
    const size_t captureBytes = capture.size();
    capture.close();
    if (guess.isEmpty()) guess = "unknown";
    JsonDocument meta;
    meta["filename"] = path;
    meta["bytes"] = captureBytes;
    if (frequencyHz) meta["frequency_hz"] = frequencyHz;
    else meta["frequency_hz"] = nullptr;
    meta["protocol_guess"] = guess;
    if (clock_set) meta["timestamp"] = rtc.getEpoch();
    else meta["timestamp"] = nullptr;
    meta["timestamp_valid"] = clock_set;
    meta["uptime_ms"] = millis();
    meta["qr_payload"] = path; // rendered by the existing QR encoder in the SD manager
    String destination = path + ".meta.json";
    String temporary = destination + ".tmp";
    File sidecar = fs.open(temporary, FILE_WRITE);
    if (!sidecar) return false;
    size_t written = serializeJsonPretty(meta, sidecar);
    sidecar.close();
    if (!written) { fs.remove(temporary); return false; }
    if (fs.exists(destination)) fs.remove(destination);
    if (!fs.rename(temporary, destination)) { fs.remove(temporary); return false; }
    return true;
}
#endif
