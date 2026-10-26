#include "core/capture_metadata.h"
#include "save.h"
#include <raw_capture_storage.h>
bool rf_raw_save(const RawRecording &recorded) {
    FS *fs = nullptr;
    if (!getFsStorage(fs) || fs == nullptr) {
        displayError("No space left on device", true);
        return false;
    }

    char filename[32];
    int index = 0;

    if (!fs->exists("/BruceRF")) {
        if (!fs->mkdir("/BruceRF")) {
            displayError("Error creating directory", true);
            return false;
        }
    }

    do { snprintf(filename, sizeof(filename), "/BruceRF/raw_%d.sub", index++); } while (fs->exists(filename));

    if (!saveRawCapture(*fs, filename, recorded)) {
        displayError("Capture save failed", true);
        return false;
    }
#ifdef BRUCE_TEMBED_BASE
    if (!annotateCapture(*fs, filename, uint32_t(recorded.frequency * 1000000), "RAW/unknown"))
        displayWarning("Saved; metadata unavailable");
#endif
    displaySuccess(filename, true);
    return true;
}
