#pragma once
#include <cstdint>
#include "ir_def.h"
#include "szk_pool.h"
#include "szk_assert.h"
#include "ir_flat_obj.h"
#include "vid.h"
namespace ir {
static constexpr uint64_t kReservedCell = 1;
enum FlagIndex {
    kIndexValid = 0,
    kIndexHead,
    kIndexForRent,
};

inline uint64_t encode40(uint8_t h, uint32_t l) {
    return (((uint64_t)h) << 32) | l;
}

inline void decode40(uint64_t v, uint8_t& h, uint32_t& l) {
    l = v;
    h = (v >> 32);
}

class Links {
 public:
    Links() {}

    void init(size_t offset) {
        memset(this, 0, sizeof(*this));
        _flags = 1 << kIndexValid;
        _offset = offset;
    }

    constexpr static size_t kInputPerLinks = 3;

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    uint32_t getOffset() const { return _offset; }

    uint64_t getDriver(size_t index) const {
        Assert(index < kInputPerLinks);
        return encode40(_inputh[index], _input[index]);
    }

    void setDriver(uint64_t driver, size_t index) {
        decode40(driver, _inputh[index], _input[index]);
    }

 private:
    uint8_t             _flags;
    uint8_t             _inputh[kInputPerLinks];
    uint32_t            _input[kInputPerLinks];
    uint32_t            _offset:24;
    uint32_t            _origh:8;
    uint32_t            _orig;
};

class Head {
 public:
    Head() {}

    void init(uint64_t dfs, uint64_t proc, Vid name) {
        _flags = (1 << kIndexValid) | (1 << kIndexHead);
        _pad = 0;
        decode40(dfs, _dfsh, _dfs);
        decode40(proc, _proch, _proc);
        _name = name;
    }

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    Vid getName() const { return _name; }
    void setName(Vid name) { _name = name; }

    uint64_t getDFS() const { return encode40(_dfsh, _dfs); }
    void setDFS(uint64_t dfs) {
        _dfs = dfs;
        dfs >>= 32;
        _dfsh = dfs;
    }

    uint64_t getProcess() const { return encode40(_proch, _proc); }
    void setProcess(uint64_t proc) {
        _proc = proc;
        proc >>= 32;
        _proch = proc;
    }

 private:
    uint8_t             _flags;
    uint8_t             _pad;
    uint8_t             _dfsh;
    // not really used
    uint8_t             _proch;
    uint32_t            _dfs;
    uint32_t            _proc;
    Vid                 _name;
};

static_assert(sizeof(Cell<IR_DEFAULT>) == sizeof(Head), "unexpected Cell size");
static_assert(sizeof(Cell<IR_DEFAULT>) == sizeof(Links), "unexpected Cell size");

template<uint32_t NS>
class Vertex {
 public:
    Vertex(uint64_t dfs, uint64_t proc, Vid name, size_t size) {
        init(dfs, proc, name, size);
    }

    operator bool() const { return _head && _head.isHead(); }

    typedef util::Pool<Cell<NS>, NS, NlFPoolSpec, util::kGC2> Pool;
    static void* operator new(size_t count, const Process<NS>& p, size_t& size);
    static void operator delete(void* p) {}
    static Vertex& get(uint64_t addr);
    static uint64_t getOffset(uint64_t addr);

    void init(uint64_t dfs, uint64_t proc, Vid name, size_t size);
    void setDriver(uint64_t driver, size_t iid);
    uint64_t getDriver(size_t iid) const;

    Vid getName() const { return _head.getName(); }
    uint64_t getDFS() const { return _head.getDFS(); }
    uint64_t getProcessID() const { return _head.getProcess(); }

    static size_t getNumCell(size_t in, size_t out) {
        return 1 + std::max(out, (in+Links::kInputPerLinks-1) /
                                Links::kInputPerLinks);
    }

 private:
    Head                _head;
    Links               _links[1];
};

static_assert(sizeof(Vertex<IR_DEFAULT>) == 2 * sizeof(Cell<IR_DEFAULT>), "unexpected Vertex size");
}
