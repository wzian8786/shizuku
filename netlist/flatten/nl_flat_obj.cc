#include "nl_flat_obj.h"
#include <sstream>
#include "nl_folded_obj.h"
#include "nl_netlist.h"
namespace netlist {
template<uint32_t NS>
typename FMInst<NS>::Pool FMInst<NS>::Pool::gSingleton;

template<uint32_t NS>
Vid FMInst<NS>::getName() const {
    uint32_t instId = Netlist<NS>::get().getMInstID(_dfs);
    const MInst<NS>& inst = MInst<NS>::Pool::get()[instId];
    Assert(inst);
    return inst.getName();
}

template<uint32_t NS>
std::string FMInst<NS>::getPath(unsigned char delimiter) const {
    const Netlist<NS>& netlist = Netlist<NS>::get();
    uint32_t dfs = _dfs;
    std::vector<Vid> names;
    while (dfs) {
        names.emplace_back(FMInst<NS>(dfs).getName());
        uint32_t instId = netlist.getMInstID(dfs);
        dfs -= netlist.getDFSOffset(instId);
    }
    std::stringstream ss;
    for (std::vector<Vid>::reverse_iterator it = names.rbegin();
         it != names.rend(); ++it) {
        if (it != names.rbegin()) {
            ss << delimiter;
        }
        ss << it->str();
    }
    return ss.str();
}

template<uint32_t NS>
FMInst<NS> FMInst<NS>::getParent() const {
    const Netlist<NS>& netlist = Netlist<NS>::get();
    uint32_t instId = netlist.getMInstID(_dfs);
    uint64_t dfs = _dfs - netlist.getDFSOffset(instId);
    return FMInst<NS>(dfs);
}

template<uint32_t NS>
size_t FMInst<NS>::Pool::getMaxSize() {
    const Netlist<NS>& netlist = Netlist<NS>::get();
    return netlist.getTotalNumInst(netlist.getRoot().getID());
}

template class FMInst<NL_DEFAULT>;

}