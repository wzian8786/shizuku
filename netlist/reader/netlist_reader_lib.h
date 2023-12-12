#pragma once
#include "vid.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
namespace netlist {
namespace reader {
struct Context {
    std::vector<netlist::Port<netlist::NL_DEFAULT>*> ports;
    std::vector<netlist::Net<netlist::NL_DEFAULT>*> nets;
    std::vector<netlist::HierInst<netlist::NL_DEFAULT>*> hinsts;
    netlist::Port<netlist::NL_DEFAULT>::Direction direction;
    std::unordered_map<netlist::Vid, netlist::Module<netlist::NL_DEFAULT>*,
                       netlist::Vid::Hash> unresolvedModules;
    std::unordered_map<netlist::Vid, netlist::Module<netlist::NL_DEFAULT>*,
                       netlist::Vid::Hash> resolvedModules;

    Context() : direction(netlist::Port<netlist::NL_DEFAULT>::kPortInvalid) {}

    void clear() {
        ports.clear();
        nets.clear();
        hinsts.clear();
        direction = netlist::Port<netlist::NL_DEFAULT>::kPortInvalid;
    }
};
extern Context gCtx;
void sanityCheck();
}
}
