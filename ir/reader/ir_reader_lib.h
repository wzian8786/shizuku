#pragma once
#include "vid.h"
#include "ir_folded_obj.h"
#include "ir_db_impl.h"
namespace ir {
namespace reader {
struct NetContext {
    std::vector<Vid> upports;
    std::vector<std::pair<Vid, Vid> > downports;
    std::vector<std::pair<Vid, int64_t> > pports;
};

struct Context {
    Module<IR_DEFAULT>* module;
    Process<IR_DEFAULT>* process;
    size_t portId;
    Port<IR_DEFAULT>::Direction direction;
    std::vector<std::pair<Net<IR_DEFAULT>*, NetContext> > nets;

    std::unordered_map<Vid, Module<IR_DEFAULT>*,
                       Vid::Hash> unresolvedModules;
    std::unordered_map<Vid, Module<IR_DEFAULT>*,
                       Vid::Hash> resolvedModules;

    std::unordered_map<Vid, Process<IR_DEFAULT>*,
                       Vid::Hash> unresolvedProcesses;
    std::unordered_map<Vid, Process<IR_DEFAULT>*,
                       Vid::Hash> resolvedProcesses;

    Context() : module(nullptr), 
                process(nullptr),
                portId(0),
                direction(Port<IR_DEFAULT>::kPortInvalid) {}

    void clear() {
        module = nullptr;
        direction = Port<IR_DEFAULT>::kPortInvalid;
        nets.clear();
    }
};

extern Context gCtx;
extern std::unordered_map<Module<IR_DEFAULT>*, 
    std::vector<std::pair<Net<IR_DEFAULT>*, NetContext>>> gNets;
void resolveNets();
void buildTops();
void sanityCheck();
}
}
