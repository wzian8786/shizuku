#include "nl_pool_db.h"
#include <ios>
#include <sstream>
#include "nl_vid_db.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
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
    }, 0); 
    Process<NS>::foreach([](Process<NS>& p, size_t i) {
        p.~Process();
    }, 0);
}

template<uint32_t NS>
template<typename T>
void PoolDB<NS>::reclaim() {
    constexpr size_t kTimesGC = 4;
    constexpr size_t kThresholdGC = 0x1000000;
    size_t d = T::gDeleted;
    size_t m = T::Pool::get().getMaxSize();
    if ((d * kTimesGC > m) || (d + kThresholdGC > m)) {
        T::foreach([](const T& e, size_t i) {
            if (!e) {
                T::Pool::get().reclaim(i, 1);
            }
        });
    }
    T::gDeleted = 0;
}

template<uint32_t NS>
void PoolDB<NS>::gc() {
    reclaim<Module<NS>>();
    reclaim<Process<NS>>();
    reclaim<Port<NS>>();
    reclaim<Net<NS>>();
    reclaim<MInst<NS>>();
    reclaim<PInst<NS>>();
    reclaim<IPort<NS>>();
    reclaim<PPort<NS>>();
}

template<uint32_t NS>
void PoolDB<NS>::print(FILE* fp, bool indent) const {
    Module<NS>::foreach([fp, indent](const Module<NS>& module, size_t) {
        if (module.isRoot()) return;
        module.print(fp, indent);
    }, 1);
    std::vector<Vid> names(Process<NS>::Pool::get().getMaxSize());
    for (const auto& p : _processIndex) {
        names[p.second] = p.first;
    }
    Process<NS>::foreach([fp, indent, &names](const Process<NS>& process, size_t id) {
        process.print(fp, indent, names[id]);
    }, 1);
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
        process = new (id) Process<NS>(id);
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

template<uint32_t NS>
Vid PoolDB<NS>::createNetResolver(size_t input, size_t io,
                                  const DataType& dt) {
    std::stringstream ss;
    ss << "S$N" << std::hex << input;
    if (io) {
        ss << "_" << io;
    }
    if (Netlist<NS>::get().getTypeScalar() != dt) {
        ss << "#(" << dt.toString() << ")";
    }
    Vid name(ss.str(), true);
    if (!hasProcess(name)) {
        Process<NS>* process = createProcess(name);
        Assert(process);
        size_t index = 0;
        uint32_t id;
        for (size_t i = 0; i < input; ++i) {
            Port<NS>* port = new (id) Port<NS>(id, ++index,
                    Port<NS>::kPortInput, dt);
            process->addPort(port);
        }
        for (size_t i = 0; i < io; ++i) {
            Port<NS>* port = new (id) Port<NS>(id, ++index,
                    Port<NS>::kPortInout, dt);
            process->addPort(port);
        }
        // add output port
        process->addPort(new (id) Port<NS>(id, ++index,
                Port<NS>::kPortOutput, dt));
    }
    return name;
}

template class PoolDB<NL_DEFAULT>;
}

namespace util {
template class Pool<netlist::Port<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::Net<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::Module<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::MInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::PInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::IPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::PPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
template class Pool<netlist::Process<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec, kGC1>;
}
