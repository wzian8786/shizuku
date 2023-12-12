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
void* Port<Namespace>::operator new(std::size_t count) {
    Assert(count == sizeof(Port));
    uint32_t id = Pool::get().New();
    Port* p = &Pool::get()[id];
    return p;
}

template<uint32_t Namespace>
void* Module<Namespace>::operator new(std::size_t count) {
    Assert(count == sizeof(Module));
    uint32_t id = Pool::get().New();
    Module* m = &Pool::get()[id];
    return m;
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

template class Port<NL_DEFAULT>;
template class Module<NL_DEFAULT>;
}

