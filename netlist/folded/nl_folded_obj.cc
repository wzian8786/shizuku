#include "nl_folded_obj.h"
#include "szk_pool.h"
#include "szk_assert.h"
#include "nl_vid_db.h"
namespace netlist {
template<uint32_t NS>
Port<NS>::Port(uint32_t id, Vid name, Direction dir, const DataType& dt) :
            Base(id), _dt(dt), _name(name) {
    setFlag(dir);
}

template<uint32_t NS>
Port<NS>::Port(uint32_t id, uint32_t index, Direction dir, const DataType& dt) :
            Base(id), _dt(dt), _index({index, 0}) {
    setFlag(dir);
}

template<uint32_t NS>
void Port<NS>::print(FILE* fp, bool indent, bool isModule) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%port ");
    if (isModule) {
        fprintf(fp, "%s ", getName().str().c_str());
    }
    fprintf(fp, "%s)", isInput() ? "%input" :
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
    if (!_mPorts.empty() || !_iPorts.empty() || !_pPorts.empty()) {
        if (indent) {
            fputc('\n', fp);
        }
    }
    for (const auto& p : _mPorts) {
        if (indent) {
            fprintf(fp, "%s", "        ");
        }
        fprintf(fp, "(%%mport %s)", p->getName().str().c_str());
        if (indent) {
            fprintf(fp, "\n");
        }
    }
    for (const auto& p : _iPorts) p->print(fp, indent);
    for (const auto& p : _pPorts) p->print(fp, indent);
    if (indent) {
        if (!_mPorts.empty() || !_iPorts.empty() || !_pPorts.empty()) {
            fprintf(fp, "%s", "    ");
        }
    }
    fputc(')', fp);
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void MInst<NS>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "    ");
    }
    fprintf(fp, "(%%minst %s %s)",getName().str().c_str(),
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
                getProcessName().str().c_str());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void IPort<NS>::print(FILE* fp, bool indent) const {
    if (indent) {
        fprintf(fp, "%s", "        ");
    }
    fprintf(fp, "(%%iport %s %s)", 
                getMInst().getName().str().c_str(),
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
    fprintf(fp, "(%%pport %s %u)",
                getPInst().getName().str().c_str(),
                getPort().getIndex());
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void Module<NS>::print(FILE* fp, bool indent) const {
    fprintf(fp, "(%%module %s",getName().str().c_str());
    if (!_ports.empty() || !_nets.empty() ||
        !_minsts.empty() || !_pinsts.empty()) {
        if (indent) {
            fputc('\n', fp);
        }
    }
    for (const auto& p : _ports) p->print(fp, indent, true);
    for (const auto& p : _nets) p->print(fp, indent);
    for (const auto& p : _minsts) p->print(fp, indent);
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
bool Module<NS>::addMInst(MInst<NS>* minst) {
    auto it = _minstIndex.find(minst->getName());
    if (it == _minstIndex.end()) {
        _minsts.emplace_back(minst);
        _minstIndex.emplace(minst->getName(), minst);
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
bool Module<NS>::hasMInst(Vid iname) const {
    return _minstIndex.find(iname) != _minstIndex.end();
}

template<uint32_t NS>
const MInst<NS>& Module<NS>::getMInst(Vid iname) const {
    auto it = _minstIndex.find(iname);
    Assert(it != _minstIndex.end());
    return *it->second;
}

template<uint32_t NS>
MInst<NS>& Module<NS>::getMInst(Vid iname) {
    auto it = _minstIndex.find(iname);
    Assert(it != _minstIndex.end());
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
Process<NS>::Process(uint32_t id) :
            Base(id),
            _ports(1) {
}

template<uint32_t NS>
void Process<NS>::setType(Type type) {
    setFlag(type);
}

template<uint32_t NS>
bool Process<NS>::addPort(Port<NS>* port) {
    size_t index = _ports.size();
    _ports.emplace_back(port);
    if (port->isInput()) {
        port->setDirIndex(_inputIndex.size());
        _inputIndex.emplace_back(index);
    } else if (port->isOutput()) {
        port->setDirIndex(_outputIndex.size());
        _outputIndex.emplace_back(index);
    } else if (port->isInout()) {
        port->setDirIndex(_inoutIndex.size());
        _inoutIndex.emplace_back(index);
    } else {
        Assert(0);
    }
    return true;
}

template<uint32_t NS>
const Port<NS>& Process<NS>::getPort(size_t id) const {
    Assert(id && id < _ports.size());
    Assert(_ports[id]);
    return *_ports[id];
}

template<uint32_t NS>
const Port<NS>& Process<NS>::getPort(size_t id, typename Port<NS>::Direction dir) const {
    size_t index = 0;
    switch (dir) {
    case Port<NS>::kPortInput:
        Assert(id < _inputIndex.size());
        index = _inputIndex[id];
        break;
    case Port<NS>::kPortOutput:
        Assert(id < _outputIndex.size());
        index = _outputIndex[id];
        break;
    case Port<NS>::kPortInout:
        Assert(id < _inoutIndex.size());
        index = _inoutIndex[id];
        break;
    default:
        ASSERT(0);
    }
    Assert(index && index < _ports.size());
    return *_ports[index];
}

template<uint32_t NS>
Port<NS>& Process<NS>::getPort(size_t id, typename Port<NS>::Direction dir) {
    size_t index = 0;
    switch (dir) {
    case Port<NS>::kPortInput:
        Assert(id < _inputIndex.size());
        index = _inputIndex[id];
        break;
    case Port<NS>::kPortOutput:
        Assert(id < _outputIndex.size());
        index = _outputIndex[id];
        break;
    case Port<NS>::kPortInout:
        Assert(id < _inoutIndex.size());
        index = _inoutIndex[id];
        break;
    default:
        ASSERT(0);
    }
    Assert(index && index < _ports.size());
    return *_ports[index];
}

template<uint32_t NS>
Port<NS>& Process<NS>::getPort(size_t id) {
    Assert(id && id < _ports.size());
    Assert(_ports[id]);
    return *_ports[id];
}

template<uint32_t NS>
void Process<NS>::print(FILE* fp, bool indent, Vid name) const {
    fprintf(fp, "(%%process %s %s", name.str().c_str(),
                    isComb() ? "%comb" :
                    isSeq() ? "%seq" : "%call");
    if (!_ports.empty() && indent) {
        fputc('\n', fp);
    }
    for (const auto& p : _ports) p->print(fp, indent, false);
    fputc(')', fp);
    if (indent) {
        fprintf(fp, "\n");
    }
}

template<uint32_t NS>
void Net<NS>::transferMPort(size_t index, Net<NS>& dst) {
    Assert(index < _mPorts.size());
    dst._mPorts.emplace_back(_mPorts.del(index));
}

template<uint32_t NS>
void Net<NS>::transferIPort(size_t index, Net<NS>& dst) {
    Assert(index < _iPorts.size());
    dst._iPorts.emplace_back(_iPorts.del(index));
}

template<uint32_t NS>
void Net<NS>::transferPPort(size_t index, Net<NS>& dst) {
    Assert(index < _pPorts.size());
    dst._pPorts.emplace_back(_pPorts.del(index));
}

template<uint32_t NS>
std::atomic<size_t> Port<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> IPort<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> PPort<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> Net<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> MInst<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> PInst<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> Module<NS>::gDeleted(0);
template<uint32_t NS>
std::atomic<size_t> Process<NS>::gDeleted(0);

template class Port<NL_DEFAULT>;
template class IPort<NL_DEFAULT>;
template class PPort<NL_DEFAULT>;
template class Net<NL_DEFAULT>;
template class MInst<NL_DEFAULT>;
template class PInst<NL_DEFAULT>;
template class Module<NL_DEFAULT>;
template class Process<NL_DEFAULT>;
}

