#pragma once
#include <cstdint>
#include <string>
#include "nl_def.h"
#include "szk_foreach.h"
namespace netlist {
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

        FMInst& operator() (uint64_t p) {
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
}
