#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include "nl_def.h"
#include "szk_foreach.h"
#include "szk_pool.h"
namespace netlist {
template<uint32_t NS>
class Cell {
 public:
    constexpr static size_t kNextOffset = 8;
    operator bool() const { return _a1 & 0x1; }

 private:
    uint8_t         _a1;
    uint8_t         _a2;
    uint8_t         _a3;
    uint8_t         _a4;
    uint32_t        _a8;
    uint32_t        _a12;
    uint32_t        _a16;
    uint32_t        _a20;
    uint32_t        _a24;
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
