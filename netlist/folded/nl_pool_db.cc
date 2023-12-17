#include "nl_pool_db.h"
#include "nl_folded_obj.h"
#include "szk_foreach.h"
namespace netlist {
template<uint32_t NS>
PoolDB<NS>::~PoolDB() {
    clear();
}

template<uint32_t NS>
void PoolDB<NS>::clear() {
    _processIndex.clear();
    _moduleIndex.clear();
    Module<NS>::foreach([](Module<NS>& mod, size_t i) {
        mod.~Module();
    }, 0/*thread_num*/); 
    Process<NS>::foreach([](Process<NS>& p, size_t i) {
        p.~Process();
    }, 0);
}

template<uint32_t NS>
Module<NS>* PoolDB<NS>::createModule(Vid name) {
    Module<NS>* module = nullptr;
    if (_moduleIndex.find(name) == _moduleIndex.end()) {
        uint32_t id;
        module = new (id) Module<NS>(id, name);
        _moduleIndex.emplace(name, module->getID());
    }
    return module;
}

template<uint32_t NS>
bool PoolDB<NS>::hasModule(Vid name) const {
    return _moduleIndex.find(name) != _moduleIndex.end();
}

template<uint32_t NS>
Module<NS>& PoolDB<NS>::getModule(Vid name) {
    auto it = _moduleIndex.find(name);
    Assert(it != _moduleIndex.end());
    return Module<NS>::Pool::get()[it->second];
}

template<uint32_t NS>
const Module<NS>& PoolDB<NS>::getModule(Vid name) const {
    auto it = _moduleIndex.find(name);
    Assert(it != _moduleIndex.end());
    return Module<NS>::Pool::get()[it->second];
}

template<uint32_t NS>
Process<NS>* PoolDB<NS>::createProcess(Vid name) {
    Process<NS>* process = nullptr;
    if (_processIndex.find(name) == _processIndex.end()) {
        uint32_t id;
        process = new (id) Process<NS>(id, name);
        _processIndex.emplace(name, process->getID());
    }
    return process;
}

template<uint32_t NS>
bool PoolDB<NS>::hasProcess(Vid name) const {
    return _processIndex.find(name) != _processIndex.end();
}

template<uint32_t NS>
Process<NS>& PoolDB<NS>::getProcess(Vid name) {
    auto it = _processIndex.find(name);
    Assert(it != _processIndex.end());
    return Process<NS>::Pool::get()[it->second];
}

template<uint32_t NS>
const Process<NS>& PoolDB<NS>::getProcess(Vid name) const {
    auto it = _processIndex.find(name);
    Assert(it != _processIndex.end());
    return Process<NS>::Pool::get()[it->second];
}

template class PoolDB<NL_DEFAULT>;
}

namespace util {
template class Pool<netlist::Port<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Net<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Module<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::MInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::PInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::IPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::PPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Process<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
}
