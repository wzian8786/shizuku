#pragma once
#include "vid.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
namespace netlist {
namespace reader {
struct NetContext {
    std::vector<Vid> upports;
    std::vector<std::pair<Vid, Vid> > downports;
    std::vector<std::pair<Vid, Vid> > pports;
};

struct Context {
    Module<NL_DEFAULT>* module;
    Process<NL_DEFAULT>* process;
    Port<NL_DEFAULT>::Direction direction;
    std::vector<std::pair<Net<NL_DEFAULT>*, NetContext> > nets;

    std::unordered_map<Vid, Module<NL_DEFAULT>*,
                       Vid::Hash> unresolvedModules;
    std::unordered_map<Vid, Module<NL_DEFAULT>*,
                       Vid::Hash> resolvedModules;

    std::unordered_map<Vid, Process<NL_DEFAULT>*,
                       Vid::Hash> unresolvedProcesses;
    std::unordered_map<Vid, Process<NL_DEFAULT>*,
                       Vid::Hash> resolvedProcesses;

    Context() : module(nullptr), 
                process(nullptr),
                direction(Port<NL_DEFAULT>::kPortInvalid) {}

    void clear() {
        module = nullptr;
        direction = Port<NL_DEFAULT>::kPortInvalid;
        nets.clear();
    }
};

extern Context gCtx;
extern std::unordered_map<Module<NL_DEFAULT>*, 
    std::vector<std::pair<Net<NL_DEFAULT>*, NetContext>>> gNets;
void resolveNets();
void buildTops();
void sanityCheck();
}
}
