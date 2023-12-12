#include "nl_folded_obj.h"
#include "szk_pool.h"
#include "szk_assert.h"
namespace netlist {
template<uint32_t Namespace>
Port<Namespace>::Port(Vid name, Direction dir, const DataType* dt) :
            _name(name), _dt(dt) {
    setFlag(1 << dir, (1 << kPortInput) | (1 << kPortOutput) | (1 << kPortInout));
}

template<uint32_t Namespace>
bool Module<Namespace>::addPort(Port<Namespace>* port) {
    auto it = _portIndex.find(port->getName());
    if (it == _portIndex.end()) {
        _ports.emplace_back(port);
        _portIndex[port->getName()] = port;
        return true;
    }
    return false;
}

template<uint32_t Namespace>
bool Module<Namespace>::addNet(Net<Namespace>* net) {
    auto it = _netIndex.find(net->getName());
    if (it == _netIndex.end()) {
        _nets.emplace_back(net);
        _netIndex.emplace(net->getName(), net);
        return true;
    }
    return false;
}

template<uint32_t Namespace>
bool Module<Namespace>::addHierInst(HierInst<Namespace>* hinst) {
    Assert(hinst->getModule() == *this);
    auto it = _hinstIndex.find(hinst->getName());
    if (it == _hinstIndex.end()) {
        _hinsts.emplace_back(hinst);
        _hinstIndex.emplace(hinst->getName(), hinst);
        return true;
    }
    return false;
}

template<uint32_t Namespace>
const Port<Namespace>& Module<Namespace>::getPort(Vid pname) const {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
Port<Namespace>& Module<Namespace>::getPort(Vid pname) {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
const HierInst<Namespace>& Module<Namespace>::getHierInst(Vid iname) const {
    auto it = _hinstIndex.find(iname);
    Assert(it != _hinstIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
HierInst<Namespace>& Module<Namespace>::getHierInst(Vid iname) {
    auto it = _hinstIndex.find(iname);
    Assert(it != _hinstIndex.end());
    return *it->second;
}

template class Port<NL_DEFAULT>;
template class DownPort<NL_DEFAULT>;
template class Net<NL_DEFAULT>;
template class HierInst<NL_DEFAULT>;
template class Module<NL_DEFAULT>;
}

