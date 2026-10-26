#ifdef BRUCE_TEMBED_BASE
#include "nfc_macro.h"
#include "core/display.h"
#include "core/powerSave.h"
#include "modules/rf/rf_send.h"
#include "modules/ir/custom_ir.h"
#include <SD.h>

bool dispatchNfcMacro(String uid) {
    if (powerBatteryCritical()) {
        displayWarning("Battery too low for macro");
        return false;
    }
    uid.replace(" ", "");
    uid.toUpperCase();
    File mapping = SD.open("/BruceNFC/macros.tsv", FILE_READ);
    if (!mapping || mapping.size() > 8192) return false;
    // TSV: UID<TAB>sub|ir<TAB>/absolute/capture/path. Bounded, no JSON allocation.
    char buffer[257];
    for (unsigned entry = 0; mapping.available() && entry < 64; ++entry) {
        size_t size = mapping.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        if (size == sizeof(buffer) - 1) return false; // malformed/oversized record
        buffer[size] = 0;
        String line(buffer);
        line.trim();
        if (line.isEmpty() || line.startsWith("#")) continue;
        int first = line.indexOf('\t');
        int second = line.indexOf('\t', first + 1);
        if (first < 0 || second < 0) continue;
        String key = line.substring(0, first);
        key.replace(" ", "");
        key.toUpperCase();
        if (key != uid) continue;
        String type = line.substring(first + 1, second);
        String path = line.substring(second + 1);
        if (!path.startsWith("/") || path.indexOf("..") >= 0) return false;
        File capture = SD.open(path, FILE_READ);
        if (!capture || capture.isDirectory() || capture.size() > 65536) return false;
        capture.close();
        mapping.close();
        if (type == "sub" && path.endsWith(".sub")) {
            RfCodes code{};
            return readSubFile(&SD, path, code) && txSubFile(code, true);
        }
        if (type == "ir" && path.endsWith(".ir")) return txIrFile(&SD, path, true);
        return false;
    }
    return false;
}
#endif
