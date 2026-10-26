#pragma once
#include <cstdlib>
#include <cstdint>
#include <vector>

// Own every stored copy, independently of which UI path exits the recorder.
template <class Symbol, void (*Release)(void *) = std::free> struct OwnedRawRecording {
    float frequency = 0;
    std::vector<Symbol *> codes;
    std::vector<uint16_t> codeLengths;
    std::vector<uint16_t> gaps;

    OwnedRawRecording() = default;
    OwnedRawRecording(const OwnedRawRecording &) = delete;
    OwnedRawRecording &operator=(const OwnedRawRecording &) = delete;
    ~OwnedRawRecording() { clear(); }
    void clear() {
        for (auto *code : codes) Release(code);
        codes.clear();
        codeLengths.clear();
        gaps.clear();
        frequency = 0;
    }
};
