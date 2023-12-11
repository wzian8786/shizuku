#include "nl_module.h"
#include "szk_pool.h"
#include "szk_assert.h"
namespace netlist {
template<uint32_t Namespace>
Port<Namespace>::Port(Vid name, Direction dir, const DataType* dt) :
            _dir(dir), _name(name), _dt(dt) {}

template<uint32_t Namespace>
void* Port<Namespace>::operator new(std::size_t count) {
    Assert(count == sizeof(Port));
    uint32_t id = Pool::get().New();
    Port* p = &Pool::get()[id];
    p->setID(id);
    return p;
}

template<uint32_t Namespace>
void Port<Namespace>::operator delete(void* p) {
    Port* port = static_cast<Port*>(p);
    port->~Port();
    port->setID(0);
}

template<uint32_t Namespace>
void* Module<Namespace>::operator new(std::size_t count) {
    Assert(count == sizeof(Module));
    uint32_t id = Pool::get().New();
    Module* m = &Pool::get()[id];
    m->setID(id);
    return m;
}

template<uint32_t Namespace>
void Module<Namespace>::operator delete(void* p) {
    Module* m = static_cast<Module*>(p);
    m->~Module();
    // we still own the memory, it is OK to call setID
    m->setID(0);
}

template class Port<NL_DEFAULT>;
template class Module<NL_DEFAULT>;
}

