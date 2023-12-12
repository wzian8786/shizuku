#pragma once
#include "vid.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
namespace netlist {
namespace reader {
struct NetContext {
    std::vector<Vid> upports;
    std::vector<std::pair<Vid, Vid> > downports;
};

struct Context {
    std::vector<Port<NL_DEFAULT>*> ports;
    std::vector<std::pair<Net<NL_DEFAULT>*, NetContext> > nets;
    std::vector<HierInst<NL_DEFAULT>*> hinsts;
    Port<NL_DEFAULT>::Direction direction;
    std::unordered_map<Vid, Module<NL_DEFAULT>*,
                       Vid::Hash> unresolvedModules;
    std::unordered_map<Vid, Module<NL_DEFAULT>*,
                       Vid::Hash> resolvedModules;

    Context() : direction(Port<NL_DEFAULT>::kPortInvalid) {}

    void clear() {
        ports.clear();
        nets.clear();
        hinsts.clear();
        direction = Port<NL_DEFAULT>::kPortInvalid;
    }
};

extern Context gCtx;
extern std::unordered_map<Module<NL_DEFAULT>*, 
    std::vector<std::pair<Net<NL_DEFAULT>*, NetContext>>> gNets;
void resolveNets();
void sanityCheck();
}
}
