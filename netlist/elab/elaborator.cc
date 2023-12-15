#include "elaborator.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_vertex.h"
namespace netlist {
template<uint32_t NS>
Elaborator<NS>::Elaborator() :
    _dfs(Module<NS>::Pool::get().getMaxSize()),
    _cellNum(Module<NS>::Pool::get().getMaxSize()) {}

template<uint32_t NS>
void Elaborator<NS>::genWeights() {
    Netlist<NS>& netlist = Netlist<NS>::get();
    std::vector<const Module<NS>*> todo;
    netlist.bottomUp(todo);

    for (auto mod : todo) {
        const typename Module<NS>::HierInstHolder& insts = mod->getHierInsts();
        const typename Module<NS>::PInstHolder& pinsts = mod->getPInsts();
        size_t dfs = 1;
        size_t cellNum = 0;
        for (auto it = pinsts.begin(); it != pinsts.end(); ++it) {
            const PInst<NS>& pinst = **it;
            const Process<NS>& process = pinst.getProcess();
            cellNum += Cell::getNumCell(process.getNumInput(), process.getNumOutput());
        }

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            const HierInst<NS>& cinst = **it;
            const Module<NS>& cmod = cinst.getModule();
            Assert(_dfs[cmod.getID()]);
            dfs += _dfs[cmod.getID()];
            cellNum += _cellNum[cmod.getID()];
        }
        _dfs[mod.getID()] = dfs;
        _cellNum[mod.getID()] = cellNum;
    }
}
}
