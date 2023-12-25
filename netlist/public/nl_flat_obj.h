#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include "nl_def.h"
#include "szk_foreach.h"
#include "szk_pool.h"
namespace netlist {
// Cell is nothing but the smallest unit of memory allocation
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

        FMInst* operator() (uint64_t p, uint64_t, size_t& size) {
            size = 1;
            dfs = FMInst(p);
            return &dfs;
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

        FPInst* operator() (Cell<NS>& cell, uint64_t id, size_t& size);
        FPInst inst;
    };

    template<typename Func>
    static void foreach(Func func, size_t threads=0) {
        util::foreach<Pool, Builder, util::ValidFilter<FPInst>>(func, threads);
    }

 private:
    uint64_t                _addr;
};

template<uint32_t NS>
class FDPort {
 public:
    FDPort() : _addr(0) {}
    explicit FDPort(uint64_t addr) : _addr(addr) {}
    FDPort(const FDPort& a) : _addr(a._addr) {}
    FDPort& operator =(FDPort a) { _addr = a._addr; return *this; } 

    Vid getName() const;
    std::string getPath(unsigned char delimiter='.') const;

    FPInst<NS> getParent() const;
    operator bool() const { return _addr; }

    bool isIO() const;

    struct Builder {
        typedef FDPort OT;
        Builder() {}
        Builder(const Builder& a) {}

        FDPort* operator() (Cell<NS>& cell, uint64_t id, size_t& size); 

        std::vector<FDPort<NS>> ports;
    };

    typedef util::Pool<Cell<NS>, NS, NlFPoolSpec, util::kGC2> Pool;

    template<typename Func>
    static void foreach(Func func, size_t threads=0) {
        util::foreach<Pool, Builder, util::ValidFilter<FDPort>>(func, threads);
    }

 private:
    uint64_t                _addr;
};

template<uint32_t NS>
class FRPort {
 public:
    FRPort() : _addr(0), _port(0) {}
    FRPort(uint64_t addr, uint64_t port) : _addr(addr), _port(port) {}
    FRPort(const FRPort& a) : _addr(a._addr), _port(a._port) {}
    FRPort& operator =(FRPort a) { _addr = a._addr; _port = a._port; return *this; } 

    Vid getName() const;
    std::string getPath(unsigned char delimiter='.') const;

    FPInst<NS> getParent() const;
    operator bool() const { return _addr; }

    bool isIO() const;

    FDPort<NS> getDriver() const;

    struct Builder {
        typedef FRPort OT;
        Builder() {}
        Builder(const Builder& a) {}

        FRPort* operator() (Cell<NS>& cell, uint64_t id, size_t& size); 

        std::vector<FRPort<NS>> ports;
    };

    typedef util::Pool<Cell<NS>, NS, NlFPoolSpec, util::kGC2> Pool;

    template<typename Func>
    static void foreach(Func func, size_t threads=0) {
        util::foreach<Pool, Builder, util::ValidFilter<FRPort>>(func, threads);
    }

 private:
    uint64_t                _addr:40;
    uint64_t                _port:24;
};

}
