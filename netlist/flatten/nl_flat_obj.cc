#include "nl_flat_obj.h"
#include <sstream>
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_vertex.h"
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

template<uint32_t NS>
Vid FPInst<NS>::getName() const {
    return Vertex<NS>::get(_addr).getName();
}

template<uint32_t NS>
FMInst<NS> FPInst<NS>::getParent() const {
    return FMInst<NS>(Vertex<NS>::get(_addr).getDFS());
}

template<uint32_t NS>
std::string FPInst<NS>::getPath(unsigned char delimiter) const {
    std::stringstream ss;
    ss << getParent().getPath() << delimiter << getName().str();
    return ss.str();
}

template<uint32_t NS>
FPInst<NS>* FPInst<NS>::Builder::operator() (Cell<NS>& cell, size_t id, size_t& size) {
    size = 1;
    Vertex<NS>& vertex = *(Vertex<NS>*)&cell;
    inst = vertex ? FPInst(id) : FPInst();
    return &inst;
}

template<uint32_t NS>
Vid FDPort<NS>::getName() const {
    uint64_t offset = Vertex<NS>::getOffset(_addr);
    const Vertex<NS>& vertex = Vertex<NS>::get(_addr-offset);
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    Assert(process);
    std::stringstream ss;
    size_t numOutput = process.getNumOfOutput();
    if (offset > numOutput) {
        ss << "io" << (offset-numOutput);
    } else {
        ss << "o" << (offset-1);
    }
    return Vid(ss.str());
}

template<uint32_t NS>
std::string FDPort<NS>::getPath(unsigned char delimiter) const {
    std::stringstream ss;
    ss << getParent().getPath() << delimiter << getName().str();
    return ss.str();
}

template<uint32_t NS>
FPInst<NS> FDPort<NS>::getParent() const {
    return FPInst<NS>(_addr - Vertex<NS>::getOffset(_addr));
}

template<uint32_t NS>
bool FDPort<NS>::isIO() const {
    const Vertex<NS>& vertex = Vertex<NS>::get(_addr);
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    Assert(process);
    return Vertex<NS>::getOffset(_addr) > process.getNumOfOutput();
}

template<uint32_t NS>
FDPort<NS>* FDPort<NS>::Builder::operator() (Cell<NS>& cell, size_t id, size_t& size) {
    Vertex<NS>& vertex = *(Vertex<NS>*)&cell;
    if (!vertex) {
        size = 0;
        return nullptr;
    } 
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    size = process.getNumOfOutput() + process.getNumOfInout();
    if (size) {
        ports.resize(size);
        for (size_t i = 0; i < size; ++i) {
            ports[i] = FDPort(id+1+i);
        }
        return &ports[0];
    }
    return nullptr;
}

template<uint32_t NS>
Vid FRPort<NS>::getName() const {
    const Vertex<NS>& vertex = Vertex<NS>::get(_addr);
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    Assert(process);
    std::stringstream ss;
    size_t numInput = process.getNumOfInput();
    if (_port >= numInput) {
        ss << "io" << (_port-numInput);
    } else {
        ss << "i" << _port;
    }
    return Vid(ss.str());
}

template<uint32_t NS>
std::string FRPort<NS>::getPath(unsigned char delimiter) const {
    std::stringstream ss;
    ss << getParent().getPath() << delimiter << getName().str();
    return ss.str();
}

template<uint32_t NS>
FPInst<NS> FRPort<NS>::getParent() const {
    return FPInst<NS>(_addr);
}

template<uint32_t NS>
bool FRPort<NS>::isIO() const {
    const Vertex<NS>& vertex = Vertex<NS>::get(_addr);
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    Assert(process);
    return _port >= process.getNumOfInput();
}

template<uint32_t NS>
FDPort<NS> FRPort<NS>::getDriver() const {
    return FDPort<NS>(Vertex<NS>::get(_addr).getDriver(_port));
}

template<uint32_t NS>
FRPort<NS>* FRPort<NS>::Builder::operator() (Cell<NS>& cell, size_t id, size_t& size) {
    Vertex<NS>& vertex = *(Vertex<NS>*)&cell;
    if (!vertex) {
        size = 0;
        return nullptr;
    } 
    const Process<NS>& process = Process<NS>::Pool::get()[vertex.getProcessID()];
    size = process.getNumOfInput() + process.getNumOfInout();
    if (size) {
        ports.resize(size);
        for (size_t i = 0; i < size; ++i) {
            ports[i] = FRPort(id, i);
        }
        return &ports[0];
    }
    return nullptr;
}

template class FMInst<NL_DEFAULT>;
template class FPInst<NL_DEFAULT>;
template class FDPort<NL_DEFAULT>;
template class FRPort<NL_DEFAULT>;

}
