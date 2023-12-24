#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include "nl_def.h"
#include "szk_foreach.h"
#include "szk_pool.h"
namespace netlist {
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

template<uint32_t NS>
class Cell {
 public:
    Cell() { init(); }

    void init() {
        memset(this, 0, sizeof(*this));
        _flags = 1 << kIndexValid;
    }

    constexpr static size_t kNextOffset = 8;
    constexpr static size_t kInputPerCell = 3;

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    uint32_t getOffset() const { return _offset; }

    uint64_t getInput(size_t index) const {
        Assert(index < kInputPerCell);
        return encode40(_inputh[index], _input[index]);
    }

    static size_t getNumCell(size_t in, size_t out) {
        return 1 + std::max(out, (in+kInputPerCell-1)/kInputPerCell);
    }

    void setDriver(uint64_t driver, size_t index) {
        decode40(driver, _inputh[index], _input[index]);
    }

 private:
    uint8_t             _flags;
    uint8_t             _inputh[kInputPerCell];
    uint32_t            _input[kInputPerCell];
    uint32_t            _offset:24;
    uint32_t            _origh:8;
    uint32_t            _orig;
};

template<uint32_t NS>
class FMInst {
 public:
    FMInst() : _dfs(0) {}
    explicit FMInst(uint64_t dfs) : _dfs(dfs) {}
    FMInst(const FMInst& a) : _dfs(a._dfs) {}
    FMInst& operator =(FMInst a) { _dfs = a._dfs; return *this; } 

    Vid getName() const;
    std::string getPath(unsigned char delimiter='.') const;

    FMInst getParent() const;

    operator bool() const { return _dfs; }

    struct Pool {
        typedef uint64_t Element;
        static const Pool& get() { return gSingleton; }
        static size_t getMaxSize();
        uint64_t operator[](size_t i) const { return i; }
        static Pool gSingleton;
    };

    struct Builder {
        typedef FMInst OT;
        Builder() {}
        Builder(const Builder& a) {}

        FMInst& operator() (uint64_t p, uint64_t) {
            dfs = FMInst(p);
            return dfs;
        }

        FMInst dfs;
    };

    template<typename Func>
    static void foreach(Func func, size_t threads=0) {
        util::foreach<Pool, Builder, util::ValidFilter<FMInst>>(func, threads);
    }

 private:
    uint64_t                _dfs;
};

template<uint32_t NS>
class FPInst {
 public:
    FPInst() : _addr(0) {}
    explicit FPInst(uint64_t addr) : _addr(addr) {}
    FPInst(const FPInst& a) : _addr(a._addr) {}
    FPInst& operator =(FPInst a) { _addr = a._addr; return *this; } 

    Vid getName() const;
    std::string getPath(unsigned char delimiter='.') const;

    FMInst<NS> getParent() const;
    operator bool() const { return _addr; }

    typedef util::Pool<Cell<NS>, NS, NlFPoolSpec, util::kGC2> Pool;

    struct Builder {
        typedef FPInst OT;
        Builder() {}
        Builder(const Builder& a) {}

        FPInst& operator() (Cell<NS>& cell, uint64_t id);
        FPInst inst;
    };

    template<typename Func>
    static void foreach(Func func, size_t threads=0) {
        util::foreach<Pool, Builder, util::ValidFilter<FPInst>>(func, threads);
    }

 private:
    uint64_t                _addr;
};
}
