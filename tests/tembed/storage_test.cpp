#include <raw_recording.h>
#include <raw_capture_storage.h>
#include <cassert>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <limits>

struct Symbol { unsigned duration0, level0, duration1, level1; };
static unsigned allocated = 0;
void release(void *p) { assert(allocated > 0); --allocated; std::free(p); }
using Recording = OwnedRawRecording<Symbol, release>;
static_assert(!std::is_copy_constructible<Recording>::value, "Capture buffers must have one owner");
static_assert(!std::is_copy_assignable<Recording>::value, "No shallow ownership copies");
struct FakeFS {
    std::map<std::string, std::shared_ptr<std::string>> files;
    size_t budget = std::numeric_limits<size_t>::max();
    bool failOpen = false, failVerify = false, failRename = false, truncateFlush = false;
    unsigned writeError = 0;
    struct File {
        FakeFS *fs = nullptr;
        std::shared_ptr<std::string> bytes;
        explicit operator bool() const { return bool(bytes); }
        size_t write(const uint8_t *data, size_t count) {
            size_t n = std::min(count, fs->budget);
            bytes->append(reinterpret_cast<const char *>(data), n);
            fs->budget -= n;
            return n;
        }
        void flush() { if (fs->truncateFlush && !bytes->empty()) bytes->pop_back(); }
        unsigned getWriteError() { return fs->writeError; }
        size_t size() { return bytes ? bytes->size() : 0; }
        void close() { bytes.reset(); }
    };
    bool exists(const char *path) { return files.count(path); }
    File open(const char *path, const char *mode) {
        if (failOpen || (mode[0] == 'r' && failVerify)) return {};
        if (mode[0] == 'w') files[path] = std::make_shared<std::string>();
        auto i = files.find(path);
        return i == files.end() ? File{} : File{this, i->second};
    }
    bool rename(const char *from, const char *to) {
        if (failRename || !exists(from) || exists(to)) return false;
        files[to] = files.at(from); files.erase(from); return true;
    }
    bool remove(const char *path) { return files.erase(path); }
};
void populate(Recording &r, unsigned count = 1) {
    r.frequency = 433.92f;
    for (unsigned i = 0; i < count; ++i) {
        auto *symbols = static_cast<Symbol *>(std::malloc(2 * sizeof(Symbol)));
        assert(symbols);
        ++allocated;
        symbols[0] = {100, 1, 200, 0}; symbols[1] = {300, 1, 400, 0};
        r.codes.push_back(symbols); r.codeLengths.push_back(2);
        if (i) r.gaps.push_back(20);
    }
}
void earlyReturn() { Recording r; populate(r, 12); return; }
int main() {
    for (int i = 0; i < 1000; ++i) { earlyReturn(); assert(allocated == 0); }
    Recording r; populate(r, 2);
    FakeFS fs;
    assert(saveRawCapture(fs, "/capture.sub", r));
    const std::string bytes = *fs.files.at("/capture.sub");
    assert(bytes.find("Version: 1\n") != std::string::npos);
    assert(bytes.find('\0') == std::string::npos);
    assert(bytes.find("100 -200 300 -400 -20000 100 -200 300 -400") != std::string::npos);
    assert(!fs.exists("/capture.sub.part"));
    for (size_t limit = 0; limit < bytes.size(); ++limit) {
        FakeFS broken; broken.budget = limit;
        assert(!saveRawCapture(broken, "/capture.sub", r));
        assert(broken.files.empty()); // no partial capture or side effects advertised as success.
    }
    for (int fault = 0; fault < 5; ++fault) {
        FakeFS broken;
        broken.failOpen = fault == 0; broken.failVerify = fault == 1;
        broken.failRename = fault == 2; broken.truncateFlush = fault == 3;
        broken.writeError = fault == 4;
        assert(!saveRawCapture(broken, "/capture.sub", r));
        assert(broken.files.empty());
    }
    assert(!saveRawCapture(fs, "/capture.sub", r));
    assert(*fs.files.at("/capture.sub") == bytes); // never replace an existing capture.
    r.clear(); assert(allocated == 0); assert(r.codes.empty() && r.gaps.empty() && r.frequency == 0);
    FakeFS empty;
    assert(!saveRawCapture(empty, "/empty.sub", r));
    populate(r); r.codeLengths.clear();
    assert(!saveRawCapture(empty, "/invalid.sub", r));
}
