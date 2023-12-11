#pragma once
#include "netlist_def.h"
namespace netlist {
class Base {
 public:
    enum FlagIndex {
        kIndexValid = 0,
        kIndexForDerived,
    };
    constexpr static size_t kNextOffset = sizeof(uint32_t);
    static_assert(kNextOffset == 4, "size unmatch");

    Base() : _flags(1 << kIndexValid) {}
    ~Base() { _flags = 0; }

    operator bool() const { return _flags & (1 << kIndexValid); }
    void setFlag(uint32_t flag, uint32_t mask) {
        _flags &= ~mask;
        _flags |= flag;
    }
    bool testFlag(int shift) const { return _flags & (1 << shift); }

    // there are
 private:
    uint32_t            _flags;       
};

}