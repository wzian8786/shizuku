#include "nl_folded_obj.h"
#include "szk_pool.h"
#include "szk_assert.h"
#include "nl_vid_db.h"
namespace netlist {
template<uint32_t NS>
Port<NS>::Port(Vid name, Direction dir, const DataType* dt) :
            _name(name), _dt(dt) {
    setFlag(dir);
}

template<uint32_t NS>
void Port<NS>::print(FILE* fp, bool indent) const {
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

template<uint32_t NS>
void Net<NS>::print(FILE* fp, bool indent) const {
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

template<uint32_t NS>
void HierInst<NS>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%hier %s %s)",getName().str().c_str(),
                getModule().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void PInst<NS>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%pinst %s %s)",getName().str().c_str(),
                getProcess().getName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void DownPort<NS>::print(FILE* fp, bool indent) const {
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

template<uint32_t NS>
void PPort<NS>::print(FILE* fp, bool indent) const {
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

template<uint32_t NS>
void Module<NS>::print(FILE* fp, bool indent) const {
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

template<uint32_t NS>
bool Module<NS>::addPort(Port<NS>* port) {
    auto it = _portIndex.find(port->getName());
    if (it == _portIndex.end()) {
        _ports.emplace_back(port);
        _portIndex[port->getName()] = port;
        return true;
    }
    return false;
}

template<uint32_t NS>
bool Module<NS>::addNet(Net<NS>* net) {
    auto it = _netIndex.find(net->getName());
    if (it == _netIndex.end()) {
        _nets.emplace_back(net);
        _netIndex.emplace(net->getName(), net);
        return true;
    }
    return false;
}

template<uint32_t NS>
bool Module<NS>::addHierInst(HierInst<NS>* hinst) {
    auto it = _hinstIndex.find(hinst->getName());
    if (it == _hinstIndex.end()) {
        _hinsts.emplace_back(hinst);
        _hinstIndex.emplace(hinst->getName(), hinst);
        return true;
    }
    return false;
}

template<uint32_t NS>
bool Module<NS>::addPInst(PInst<NS>* pinst) {
    auto it = _pinstIndex.find(pinst->getName());
    if (it == _pinstIndex.end()) {
        _pinsts.emplace_back(pinst);
        _pinstIndex.emplace(pinst->getName(), pinst);
        return true;
    }
    return false;
}

template<uint32_t NS>
bool Module<NS>::hasPort(Vid pname) const {
    return _portIndex.find(pname) != _portIndex.end();
}

template<uint32_t NS>
const Port<NS>& Module<NS>::getPort(Vid pname) const {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t NS>
Port<NS>& Module<NS>::getPort(Vid pname) {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t NS>
bool Module<NS>::hasHierInst(Vid iname) const {
    return _hinstIndex.find(iname) != _hinstIndex.end();
}

template<uint32_t NS>
const HierInst<NS>& Module<NS>::getHierInst(Vid iname) const {
    auto it = _hinstIndex.find(iname);
    Assert(it != _hinstIndex.end());
    return *it->second;
}

template<uint32_t NS>
HierInst<NS>& Module<NS>::getHierInst(Vid iname) {
    auto it = _hinstIndex.find(iname);
    Assert(it != _hinstIndex.end());
    return *it->second;
}

template<uint32_t NS>
bool Module<NS>::hasPInst(Vid iname) const {
    return _pinstIndex.find(iname) != _pinstIndex.end();
}

template<uint32_t NS>
const PInst<NS>& Module<NS>::getPInst(Vid iname) const {
    auto it = _pinstIndex.find(iname);
    Assert(it != _pinstIndex.end());
    return *it->second;
}

template<uint32_t NS>
PInst<NS>& Module<NS>::getPInst(Vid iname) {
    auto it = _pinstIndex.find(iname);
    Assert(it != _pinstIndex.end());
    return *it->second;
}

template<uint32_t NS>
Process<NS>::Process(Vid name) :
            _name(name) {
}

template<uint32_t NS>
void Process<NS>::setType(Type type) {
    setFlag(type);
}

template<uint32_t NS>
bool Process<NS>::addPort(Port<NS>* port) {
    auto it = _portIndex.find(port->getName());
    if (it == _portIndex.end()) {
        _ports.emplace_back(port);
        _portIndex[port->getName()] = port;
        return true;
    }
    return false;
}

template<uint32_t NS>
bool Process<NS>::hasPort(Vid pname) const {
    return _portIndex.find(pname) != _portIndex.end();
}

template<uint32_t NS>
const Port<NS>& Process<NS>::getPort(Vid pname) const {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t NS>
Port<NS>& Process<NS>::getPort(Vid pname) {
    auto it = _portIndex.find(pname);
    Assert(it != _portIndex.end());
    return *it->second;
}

template<uint32_t NS>
void Process<NS>::print(FILE* fp, bool indent) const {
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

