#pragma once
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>

// Publish a capture only after ALL writes and the persisted length agree.
// FS/File are template parameters so storage failures can be tested on the host.
template <class Filesystem, class Recording>
bool saveRawCapture(Filesystem &fs, const char *path, const Recording &recorded) {
    if (!std::isfinite(recorded.frequency) || recorded.frequency <= 0 || recorded.codes.empty() ||
        recorded.codes.size() != recorded.codeLengths.size() ||
        recorded.gaps.size() + 1 < recorded.codes.size() || fs.exists(path)) return false;
    for (size_t i = 0; i < recorded.codes.size(); ++i)
        if (!recorded.codes[i] || !recorded.codeLengths[i]) return false;

    char temporary[96];
    int n = std::snprintf(temporary, sizeof(temporary), "%s.part", path);
    if (n < 0 || size_t(n) >= sizeof(temporary)) return false;
    auto file = fs.open(temporary, "w");
    if (!file) return false;
    size_t expected = 0;
    bool ok = true;
    auto write = [&](const char *text) {
        if (!ok) return;
        size_t count = std::strlen(text);
        ok = file.write(reinterpret_cast<const uint8_t *>(text), count) == count;
        expected += count;
    };
    write("Filetype: Bruce SubGhz File\nVersion: 1\n");
    char line[64];
    std::snprintf(line, sizeof(line), "Frequency: %u\n",
                  unsigned(std::lround(recorded.frequency * 1000000.0)));
    write(line);
    write("Preset: 0\nProtocol: RAW\nRAW_Data: ");
    unsigned values = 0;
    auto value = [&](int duration) {
        if (!duration) return;
        if (values && values % 512 == 0) write("\nRAW_Data: ");
        std::snprintf(line, sizeof(line), "%d ", duration);
        write(line);
        ++values;
    };
    for (size_t i = 0; ok && i < recorded.codes.size(); ++i) {
        for (size_t j = 0; ok && j < recorded.codeLengths[i]; ++j) {
            const auto &code = recorded.codes[i][j];
            value(code.level0 ? int(code.duration0) : -int(code.duration0));
            value(code.level1 ? int(code.duration1) : -int(code.duration1));
        }
        if (i + 1 < recorded.codes.size()) value(-int(recorded.gaps[i]) * 1000);
    }
    write("\n");
    file.flush();
    ok = ok && !file.getWriteError() && file.size() == expected;
    file.close();
    if (ok) {
        auto verify = fs.open(temporary, "r");
        ok = bool(verify) && verify.size() == expected;
        verify.close();
    }
    if (ok && fs.rename(temporary, path)) return true;
    fs.remove(temporary);
    return false;
}
