#include "nl_elab_db.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_vertex.h"
#include "nl_vid_db.h"
namespace netlist {
template<uint32_t NS>
ElabDB<NS>::ElabDB() {}

template<uint32_t NS>
void ElabDB<NS>::elab() {
    reset();
    genWeights();
    genIndex();
}

template<uint32_t NS>
void ElabDB<NS>::reset() {
    _dfs.clear();
    _dfsOffset.clear();
    _cellNum.clear();
    _cellOffset.clear();
    _dfs.resize(Module<NS>::Pool::get().getMaxSize());
    _dfsOffset.resize(MInst<NS>::Pool::get().getMaxSize());
    _cellNum.resize(Module<NS>::Pool::get().getMaxSize());
    _cellOffset.resize(MInst<NS>::Pool::get().getMaxSize());
}

template<uint32_t NS>
void ElabDB<NS>::genWeights() {
    Netlist<NS>& netlist = Netlist<NS>::get();
    std::vector<Module<NS>*> todo;
    netlist.bottomUp(todo);

    for (auto mod : todo) {
        const typename Module<NS>::MInstHolder& insts = mod->getMInsts();
        const typename Module<NS>::PInstHolder& pinsts = mod->getPInsts();
        size_t dfs = 1;
        size_t cellNum = 0;
        for (auto it = pinsts.begin(); it != pinsts.end(); ++it) {
            const PInst<NS>& pinst = **it;
            const Process<NS>& process = pinst.getProcess();
            cellNum += Cell::getNumCell(process.getNumOfInput(), process.getNumOfOutput());
        }

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            const MInst<NS>& cinst = **it;
            const Module<NS>& cmod = cinst.getModule();
            _dfsOffset[cinst.getID()] = dfs;
            _cellOffset[cinst.getID()] = cellNum;

            Assert(_dfs[cmod.getID()]);
            dfs += _dfs[cmod.getID()];
            cellNum += _cellNum[cmod.getID()];
        }
        _dfs[mod->getID()] = dfs;
        _cellNum[mod->getID()] = cellNum;
    }
}

template<uint32_t NS>
void ElabDB<NS>::genIndex() {
    const Module<NS>& root = Netlist<NS>::get().getRoot();
    size_t maxDfs = _dfs[root.getID()];
    _index.resize(maxDfs);
    const typename Module<NS>::MInstHolder& insts = root.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, _dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::visitInst(const MInst<NS>& inst, size_t dfs) {
    _index[dfs] = inst.getID();
    const Module<NS>& module = inst.getModule();
    const typename Module<NS>::MInstHolder& insts = module.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, dfs+_dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::printFlatten(FILE* fp) const {
    printf("#0 %s(%s)\n", Vid(kVidSRoot).str().c_str(),
                          Vid(kVidSRoot).str().c_str());
    for (size_t i = 1; i < _index.size(); ++i) {
        Assert(i < MInst<NS>::Pool::get().getMaxSize());
        const MInst<NS>& minst = MInst<NS>::Pool::get()[_index[i]];
        Assert(minst);
        const Module<NS>& module = minst.getModule();
        printf("#%lu %s(%s)\n", i, minst.getName().str().c_str(),
                                   module.getName().str().c_str());
    }
}

template class ElabDB<NL_DEFAULT>;
}
