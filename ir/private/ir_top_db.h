#pragma once
#include "ir_def.h"
#include <vector>
namespace ir {
template<uint32_t NS>
class TopDB {
 public:
    TopDB();

    typedef std::vector<Module<NS>*> TopVec;

    Module<NS>& getRoot() { return *_root; }
    const Module<NS>& getRoot() const { return *_root; }

    void getTops(TopVec& tops) const;

 private:
    Module<NS>*              _root;
};
}
