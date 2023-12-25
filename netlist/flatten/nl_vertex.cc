#include "nl_vertex.h"
#include "nl_folded_obj.h"
namespace netlist {
template<uint32_t NS>
void* Vertex<NS>::operator new(size_t count, const Process<NS>& p, size_t& size) {
    size = getNumCell(p.getNumOfInput() + p.getNumOfInout(),
                      p.getNumOfOutput() + p.getNumOfInout());
    uint64_t id = Vertex<NS>::Pool::get().New(size);
    return &Vertex<NS>::Pool::get()[id];
}

template<uint32_t NS>
void Vertex<NS>::init(uint64_t dfs, uint64_t proc, Vid name, size_t size) {
    Assert(size > 1);
    _head.init(dfs, proc, name);
    for (size_t i = 1; i < size; ++i) {
        _cell[i-1].init(i);
    }
}

template<uint32_t NS>
void Vertex<NS>::setDriver(uint64_t driver, size_t iid) {
    size_t offset = iid / Links::kInputPerLinks;
    size_t index = iid % Links::kInputPerLinks;
    _cell[offset].setDriver(driver, index);
}

template<uint32_t NS>
uint64_t Vertex<NS>::getDriver(size_t iid) const {
    size_t offset = iid / Links::kInputPerLinks;
    size_t index = iid % Links::kInputPerLinks;
    return _cell[offset].getDriver(index);
}

template<uint32_t NS>
Vertex<NS>& Vertex<NS>::get(uint64_t addr) {
    Vertex<NS>& vertex = *(Vertex<NS>*)&Pool::get()[addr];
    Assert(vertex);
    return vertex;
}

template<uint32_t NS>
uint64_t Vertex<NS>::getOffset(uint64_t addr) {
    Links* links = (Links*)&Pool::get()[addr];
    Assert(!links->isHead());
    return links->getOffset();
}

template class Vertex<NL_DEFAULT>;
}

namespace util {
template class Pool<netlist::Cell<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlFPoolSpec, kGC2>;
}
