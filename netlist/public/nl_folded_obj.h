#pragma once
#include <cstdio>
#include <vector>
#include <memory>
#include <unordered_map>
#include "nl_def.h"
#include "nl_datatype.h"
#include "szk_pool.h"
#include "szk_foreach.h"
#include "vid.h"
namespace netlist {
class Base {
 public:
    enum FlagIndex {
        kIndexValid = 0,
        kIndexForDerived,
    };
    constexpr static size_t kNextOffset = sizeof(uint32_t);

    Base() : _flags(1 << kIndexValid) {}
    ~Base() { _flags = 0; }

    operator bool() const { return _flags & (1 << kIndexValid); }
    void setFlag(uint32_t flag, uint32_t mask) {
        _flags &= ~mask;
        _flags |= flag;
    }
    bool testFlag(int shift) const { return _flags & (1 << shift); }

    // there are
 private:
    uint32_t            _flags;       
};

template<uint32_t Namespace>
class Port : public Base {
 public:
    ManagerByPool(Port);

    enum Direction {
        kPortInvalid = 0,
        kPortInput = kIndexForDerived,
        kPortOutput,
        kPortInout,
    };

    Port(Vid name, Direction dir, const DataType* dt);

    Vid getName() const { return _name; }

    bool isInput() const { return testFlag(kPortInput); }
    bool isOutput() const { return testFlag(kPortOutput); }
    bool isInout() const { return testFlag(kPortInout); }

    Direction getDirection() const {
        return isInput() ? kPortInput : isOutput() ? kPortOutput : kPortInout;
    }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                 _name;
    const DataType*     _dt;
};

template<uint32_t Namespace>
class Net : public Base {
 public:
    ManagerByPool(Net);

    Net(Vid name, const DataType* dt) :
        _name(name), _dt(dt), _module(nullptr) {}

    Vid getName() const { return _name; }

    void setModule(Module<Namespace>* module) { _module = module; }
    const Module<Namespace>& getModule() const {
        Assert(_module);
        return *_module;
    }

    void addUpPort(Port<Namespace>* port) { _upPorts.emplace_back(port); }
    void addDownPort(DownPort<Namespace>* dp) { _downPorts.emplace_back(dp); }
    void addPPort(PPort<Namespace>* pp) { _pPorts.emplace_back(pp); }

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::vector<Port<Namespace>*> UpPortVec;

    typedef std::unique_ptr<DownPort<Namespace>> DownPortPtr;
    typedef std::vector<DownPortPtr> DownPortHolder;

    typedef std::unique_ptr<PPort<Namespace>> PPortPtr;
    typedef std::vector<PPortPtr> PPortHolder;

 private:
    Vid                 _name;
    const DataType*     _dt;
    Module<Namespace>*  _module;
    UpPortVec           _upPorts;
    DownPortHolder      _downPorts;
    PPortHolder         _pPorts;
};

template<uint32_t Namespace>
class DownPort : public Base {
 public:
    ManagerByPool(DownPort);

    DownPort(HierInst<Namespace>* inst, Port<Namespace>* port) :
            _inst(inst), _port(port) {}

    const HierInst<Namespace>& getHierInst() const { return *_inst; }
    const Port<Namespace>& getPort() const { return *_port; }

    void print(FILE* fp, bool indent) const;

 private:
    HierInst<Namespace>*    _inst;
    Port<Namespace>*        _port;
};

template<uint32_t Namespace>
class PPort : public Base {
 public:
    ManagerByPool(PPort);

    PPort(PInst<Namespace>* inst, Port<Namespace>* port) :
            _inst(inst), _port(port) {}

    const PInst<Namespace>& getPInst() const { return *_inst; }
    const Port<Namespace>& getPort() const { return *_port; }

    void print(FILE* fp, bool indent) const;

 private:
    PInst<Namespace>*       _inst;
    Port<Namespace>*        _port;
};

template<uint32_t Namespace>
class HierInst : public Base {
 public:
    ManagerByPool(HierInst);

    HierInst(Vid name, Module<Namespace>* module) :
        _name(name), _module(module), _parent(nullptr) {}

    Vid getName() const { return _name; }
    const Module<Namespace>& getModule() const { return *_module; }
    Module<Namespace>& getModule() { return *_module; }
    const Module<Namespace>* getParent() const { return _parent; }
    void setParent(Module<Namespace>* parent) { _parent = parent; }
    bool isTop() const { return !_parent; }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Module<Namespace>*      _module;
    Module<Namespace>*      _parent;
};

template<uint32_t Namespace>
class PInst : public Base {
 public:
    ManagerByPool(PInst);

    PInst(Vid name, Process<Namespace>* proc) :
        _name(name), _proc(proc) {}

    Vid getName() const { return _name; }
    const Process<Namespace>& getProcess() const { return *_proc; }
    Process<Namespace>& getProcess() { return *_proc; }

    const Module<Namespace>& getParent() const {
        Assert(_parent);
        return *_parent;
    }
    void setParent(Module<Namespace>* parent) { _parent = parent; }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Process<Namespace>*     _proc;
    Module<Namespace>*      _parent;
};

template<uint32_t Namespace>
class Module : public Base {
 public:
    ManagerByPool(Module);

    explicit Module(Vid name) : _name(name) {}

    Vid getName() const { return _name; }

    bool addPort(Port<Namespace>* port);
    bool addNet(Net<Namespace>* net);
    bool addHierInst(HierInst<Namespace>* inst);
    bool addPInst(PInst<Namespace>* inst);

    bool hasPort(Vid) const;
    const Port<Namespace>& getPort(Vid pname) const;
    Port<Namespace>& getPort(Vid pname);

    bool hasHierInst(Vid) const;
    const HierInst<Namespace>& getHierInst(Vid iname) const;
    HierInst<Namespace>& getHierInst(Vid iname);

    bool hasPInst(Vid) const;
    const PInst<Namespace>& getPInst(Vid iname) const;
    PInst<Namespace>& getPInst(Vid iname);

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::unique_ptr<Port<Namespace>> PortPtr;
    typedef std::vector<PortPtr> PortHolder;
    typedef std::unordered_map<Vid, Port<Namespace>*, Vid::Hash> PortIndex;

    typedef std::unique_ptr<Net<Namespace>> NetPtr;
    typedef std::vector<NetPtr> NetHolder;
    typedef std::unordered_map<Vid, Net<Namespace>*, Vid::Hash> NetIndex;

    typedef std::unique_ptr<HierInst<Namespace>> HierInstPtr;
    typedef std::vector<HierInstPtr> HierInstHolder;
    typedef std::unordered_map<Vid, HierInst<Namespace>*, Vid::Hash> HierInstIndex;

    typedef std::unique_ptr<PInst<Namespace>> PInstPtr;
    typedef std::vector<PInstPtr> PInstHolder;
    typedef std::unordered_map<Vid, PInst<Namespace>*, Vid::Hash> PInstIndex;

 private:
    uint32_t                _pad;
    Vid                     _name;
    PortHolder              _ports;
    PortIndex               _portIndex;
    NetHolder               _nets;
    NetIndex                _netIndex;
    HierInstHolder          _hinsts;
    HierInstIndex           _hinstIndex;
    PInstHolder             _pinsts;
    PInstIndex              _pinstIndex;
};

template<uint32_t Namespace>
class Process : public Base{
 public:
    ManagerByPool(Process);

    enum Type {
        kTypeInvalid = 0,
        kTypeComb,
        kTypeSeq,
        kTypeCall,
    };

    Process(Vid name);

    Vid getName() const { return _name; }

    void setType(Type type);
    bool isComb() const { return testFlag(kTypeComb); }
    bool isSeq() const { return testFlag(kTypeSeq); }
    bool isCall() const { return testFlag(kTypeCall); }

    bool addPort(Port<Namespace>* port);
    bool hasPort(Vid) const;
    const Port<Namespace>& getPort(Vid pname) const;
    Port<Namespace>& getPort(Vid pname);

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::unique_ptr<Port<Namespace>> PortPtr;
    typedef std::vector<PortPtr> PortHolder;
    typedef std::unordered_map<Vid, Port<Namespace>*, Vid::Hash> PortIndex;

 private:
    Vid                     _name;
    PortHolder              _ports;
    PortIndex               _portIndex;
};

static_assert(sizeof(Base) == 4, "size unmatch");
}
