#pragma once
#include "nl_def.h"
#include <vector>
namespace netlist {
template<uint32_t NS>
class TopDB {
 public:
    TopDB();

    typedef std::vector<Module<NS>*> TopVec;

    Module<NS>& getRoot() { return *_root; }
    const Module<NS>& getRoot() const { return *_root; }

    void addTop(Module<NS>* top);
    void getTops(TopVec& tops) const;

 private:
    Module<NS>*              _root;
};
}
