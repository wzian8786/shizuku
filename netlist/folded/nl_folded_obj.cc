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
void Port<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%port %s %s)", getName().str().c_str(),
                                isInput() ? "%input" :
                                isOutput() ? "%output" : "%inout");
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void Net<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%net %s", getName().str().c_str());
    if (!_upPorts.empty() || !_downPorts.empty() || !_pPorts.empty()) {
        if (indent) {
            fputc('\n', fp);
        }
    }
    for (const auto& p : _upPorts) {
        if (indent) {
            fprintf(fp, "%s", "        ");
        }
        fprintf(fp, "(%%upport %s)", p->getName().str().c_str());
        if (indent) {
            fprintf(fp, "\n");
        }
    }
    for (const auto& p : _downPorts) p->print(fp, indent);
    for (const auto& p : _pPorts) p->print(fp, indent);
    if (indent) {
        if (!_upPorts.empty() || !_downPorts.empty() || !_pPorts.empty()) {
            fprintf(fp, "%s", "    ");
        }
    }
    fputc(')', fp);
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void HierInst<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%hier %s %s)",getName().str().c_str(),
                getModule().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void PInst<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%pinst %s %s)",getName().str().c_str(),
                getProcess().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void DownPort<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "        ");
    }
    fprintf(fp, "(%%downport %s %s)", 
                getHierInst().getName().str().c_str(),
                getPort().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void PPort<Namespace>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "        ");
    }
    fprintf(fp, "(%%pport %s %s)",
                getPInst().getName().str().c_str(),
                getPort().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t Namespace>
void Module<Namespace>::print(FILE* fp, bool indent) const {
    fprintf(fp, "(%%module %s",getName().str().c_str());
    if (!_ports.empty() || !_nets.empty() ||
        !_hinsts.empty() || !_pinsts.empty()) {
        if (indent) {
            fputc('\n', fp);
        }
    }
    for (const auto& p : _ports) p->print(fp, indent);
    for (const auto& p : _nets) p->print(fp, indent);
    for (const auto& p : _hinsts) p->print(fp, indent);
    for (const auto& p : _pinsts) p->print(fp, indent);
    fputc(')', fp);
    if (indent) {
        fprintf(fp, "\n");
    }
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
    auto it = _hinstIndex.find(hinst->getName());
    if (it == _hinstIndex.end()) {
        _hinsts.emplace_back(hinst);
        _hinstIndex.emplace(hinst->getName(), hinst);
        return true;
    }
    return false;
}

template<uint32_t Namespace>
bool Module<Namespace>::addPInst(PInst<Namespace>* pinst) {
    auto it = _pinstIndex.find(pinst->getName());
    if (it == _pinstIndex.end()) {
        _pinsts.emplace_back(pinst);
        _pinstIndex.emplace(pinst->getName(), pinst);
        return true;
    }
    return false;
}

template<uint32_t Namespace>
bool Module<Namespace>::hasPort(Vid pname) const {
    return _portIndex.find(pname) != _portIndex.end();
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
bool Module<Namespace>::hasHierInst(Vid iname) const {
    return _hinstIndex.find(iname) != _hinstIndex.end();
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

template<uint32_t Namespace>
bool Module<Namespace>::hasPInst(Vid iname) const {
    return _pinstIndex.find(iname) != _pinstIndex.end();
}

template<uint32_t Namespace>
const PInst<Namespace>& Module<Namespace>::getPInst(Vid iname) const {
    auto it = _pinstIndex.find(iname);
    Assert(it != _pinstIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
PInst<Namespace>& Module<Namespace>::getPInst(Vid iname) {
    auto it = _pinstIndex.find(iname);
    Assert(it != _pinstIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
Process<Namespace>::Process(Vid name) :
            _name(name) {
}

template<uint32_t Namespace>
void Process<Namespace>::setType(Type type) {
    setFlag(1 << type, 0xe);
}

template<uint32_t Namespace>
bool Process<Namespace>::addPort(Port<Namespace>* port) {
    auto it = _portIndex.find(port->getName());
    if (it == _portIndex.end()) {
        _ports.emplace_back(port);
        _portIndex[port->getName()] = port;
        return true;
    }
    return false;
}

template<uint32_t Namespace>
bool Process<Namespace>::hasPort(Vid pname) const {
    return _portIndex.find(pname) != _portIndex.end();
}

template<uint32_t Namespace>
const Port<Namespace>& Process<Namespace>::getPort(Vid pname) const {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
Port<Namespace>& Process<Namespace>::getPort(Vid pname) {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t Namespace>
void Process<Namespace>::print(FILE* fp, bool indent) const {
    fprintf(fp, "(%%process %s %s",getName().str().c_str(),
                    isComb() ? "%comb" :
                    isSeq() ? "%seq" : "%call");
    if (!_ports.empty() && indent) {
        fputc('\n', fp);
    }
    for (const auto& p : _ports) p->print(fp, indent);
    fputc(')', fp);
    if (indent) {
        fprintf(fp, "\n");
    }
}

template class Port<NL_DEFAULT>;
template class DownPort<NL_DEFAULT>;
template class PPort<NL_DEFAULT>;
template class Net<NL_DEFAULT>;
template class HierInst<NL_DEFAULT>;
template class PInst<NL_DEFAULT>;
template class Module<NL_DEFAULT>;
template class Process<NL_DEFAULT>;
}

