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
    constexpr static size_t kNextOffset = 8;

    // _id is not initialized for purpose, it's actually set in
    // operator new
    Base() : _flags(1 << kIndexValid) {}
    ~Base() { _flags = 0; }

    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;

    operator bool() const { return _flags & (1 << kIndexValid); }
    void clearFlag(size_t flag) { _flags &= ~(1ull << flag); }
    void setFlag(size_t flag) { _flags |= (1ull << flag); }
    bool testFlag(size_t shift) const { return _flags & (1 << shift); }

    uint32_t getID() const { return _id; }
    void setID(uint32_t id) { _id = id; }

    // there are
 private:
    uint32_t            _flags;       
    uint32_t            _id;
};
static_assert(sizeof(Base) == Base::kNextOffset, "size mismatch");

#define ManagerByPool(T) \
    typedef util::Pool<T, NS, NlPoolSpec> Pool; \
    static void* operator new(std::size_t count) { \
        Assert(count == sizeof(T)); \
        uint32_t id = Pool::get().New(); \
        T* m = &Pool::get()[id]; \
        m->setID(id); \
        return m; \
    } \
    static void operator delete(void* p) {} \
    ~T() { Base::~Base(); } \
    template<typename Func> \
    static void foreach(Func func, size_t threads) { \
        util::foreach<typename T::Pool, util::TransBuilder<T>, util::ValidFilter<T>>(func, threads); \
    }

template<uint32_t NS>
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
    const DataType* getDataType() const { return _dt; }

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

template<uint32_t NS>
class Net : public Base {
 public:
    ManagerByPool(Net);

    Net(Vid name, const DataType* dt) :
        _name(name), _dt(dt), _module(nullptr) {}

    Vid getName() const { return _name; }
    const DataType* getDataType() const { return _dt; }

    void setModule(Module<NS>* module) { _module = module; }
    const Module<NS>& getModule() const {
        Assert(_module);
        return *_module;
    }

    void addUpPort(Port<NS>* port) { _upPorts.emplace_back(port); }
    void addDownPort(DownPort<NS>* dp) { _downPorts.emplace_back(dp); }
    void addPPort(PPort<NS>* pp) { _pPorts.emplace_back(pp); }

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::vector<Port<NS>*> UpPortVec;

    typedef std::unique_ptr<DownPort<NS>> DownPortPtr;
    typedef std::vector<DownPortPtr> DownPortHolder;

    typedef std::unique_ptr<PPort<NS>> PPortPtr;
    typedef std::vector<PPortPtr> PPortHolder;

 private:
    Vid                 _name;
    const DataType*     _dt;
    Module<NS>*         _module;
    UpPortVec           _upPorts;
    DownPortHolder      _downPorts;
    PPortHolder         _pPorts;
};

template<uint32_t NS>
class DownPort : public Base {
 public:
    ManagerByPool(DownPort);

    DownPort(HierInst<NS>* inst, Port<NS>* port) :
            _inst(inst), _port(port) {}

    const HierInst<NS>& getHierInst() const { return *_inst; }
    const Port<NS>& getPort() const { return *_port; }

    void print(FILE* fp, bool indent) const;

 private:
    HierInst<NS>*           _inst;
    Port<NS>*               _port;
};

template<uint32_t NS>
class PPort : public Base {
 public:
    ManagerByPool(PPort);

    PPort(PInst<NS>* inst, Port<NS>* port) :
            _inst(inst), _port(port) {}

    const PInst<NS>& getPInst() const { return *_inst; }
    const Port<NS>& getPort() const { return *_port; }

    void print(FILE* fp, bool indent) const;

 private:
    PInst<NS>*              _inst;
    Port<NS>*               _port;
};

template<uint32_t NS>
class HierInst : public Base {
 public:
    ManagerByPool(HierInst);

    HierInst(Vid name, Module<NS>* module) :
        _name(name), _module(module), _parent(nullptr) {}

    Vid getName() const { return _name; }
    const Module<NS>& getModule() const { return *_module; }
    Module<NS>& getModule() { return *_module; }
    const Module<NS>* getParent() const { return _parent; }
    void setParent(Module<NS>* parent) { _parent = parent; }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Module<NS>*             _module;
    Module<NS>*             _parent;
};

template<uint32_t NS>
class PInst : public Base {
 public:
    ManagerByPool(PInst);

    PInst(Vid name, Process<NS>* proc) :
        _name(name), _proc(proc) {}

    Vid getName() const { return _name; }
    const Process<NS>& getProcess() const { return *_proc; }
    Process<NS>& getProcess() { return *_proc; }

    const Module<NS>& getParent() const {
        Assert(_parent);
        return *_parent;
    }
    void setParent(Module<NS>* parent) { _parent = parent; }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Process<NS>*            _proc;
    Module<NS>*             _parent;
};

template<uint32_t NS>
class Module : public Base {
 public:
    ManagerByPool(Module);

    explicit Module(Vid name) : _name(name) {}

    enum Flag {
        kFlagRoot = kIndexForDerived,
        kFlagTop,
    };

    bool isRoot() const { return testFlag(kFlagRoot); }
    bool isTop() const { return testFlag(kFlagTop); }
    void setRoot() { setFlag(kFlagRoot); }
    void setTop() { setFlag(kFlagTop); }
    void clearTop() { clearFlag(kFlagTop); }
    
    Vid getName() const { return _name; }

    bool addPort(Port<NS>* port);
    bool addNet(Net<NS>* net);
    bool addHierInst(HierInst<NS>* inst);
    bool addPInst(PInst<NS>* inst);

    bool hasPort(Vid) const;
    const Port<NS>& getPort(Vid pname) const;
    Port<NS>& getPort(Vid pname);

    bool hasHierInst(Vid) const;
    const HierInst<NS>& getHierInst(Vid iname) const;
    HierInst<NS>& getHierInst(Vid iname);

    bool hasPInst(Vid) const;
    const PInst<NS>& getPInst(Vid iname) const;
    PInst<NS>& getPInst(Vid iname);

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::unordered_map<Vid, Port<NS>*, Vid::Hash> PortIndex;
    typedef std::unordered_map<Vid, Net<NS>*, Vid::Hash> NetIndex;
    typedef std::unordered_map<Vid, HierInst<NS>*, Vid::Hash> HierInstIndex;
    typedef std::unordered_map<Vid, PInst<NS>*, Vid::Hash> PInstIndex;

 public:
    typedef std::unique_ptr<Port<NS>> PortPtr;
    typedef std::vector<PortPtr> PortHolder;

    typedef std::unique_ptr<Net<NS>> NetPtr;
    typedef std::vector<NetPtr> NetHolder;

    typedef std::unique_ptr<HierInst<NS>> HierInstPtr;
    typedef std::vector<HierInstPtr> HierInstHolder;

    typedef std::unique_ptr<PInst<NS>> PInstPtr;
    typedef std::vector<PInstPtr> PInstHolder;

    const PortHolder& getPorts() const { return _ports; }
    const NetHolder& getNets() const { return _nets; }
    const HierInstHolder& getHierInsts() const { return _hinsts; }
    const PInstHolder& getPInsts() const { return _pinsts; }

 private:
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

template<uint32_t NS>
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

    uint32_t getNumOfInput() const { return _numInput; }
    uint32_t getNumOfOutput() const { return _numOutput; }

    void setType(Type type);
    bool isComb() const { return testFlag(kTypeComb); }
    bool isSeq() const { return testFlag(kTypeSeq); }
    bool isCall() const { return testFlag(kTypeCall); }

    bool addPort(Port<NS>* port);
    bool hasPort(Vid) const;
    const Port<NS>& getPort(Vid pname) const;
    Port<NS>& getPort(Vid pname);

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::unique_ptr<Port<NS>> PortPtr;
    typedef std::vector<PortPtr> PortHolder;
    typedef std::unordered_map<Vid, Port<NS>*, Vid::Hash> PortIndex;

 private:
    Vid                     _name;
    uint32_t                _numInput;
    uint32_t                _numOutput;
    PortHolder              _ports;
    PortIndex               _portIndex;
};
}
