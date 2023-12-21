#pragma once
#include <atomic>
#include <cstdio>
#include <vector>
#include <memory>
#include <unordered_map>
#include "nl_def.h"
#include "nl_datatype.h"
#include "szk_pool.h"
#include "szk_foreach.h"
#include "szk_deletable_vec.h"
#include "vid.h"
namespace netlist {
class Base {
 public:
    enum FlagIndex {
        kIndexValid = 0,
        kIndexForDerived,
    };
    constexpr static size_t kNextOffset = 8;

    explicit Base(uint32_t id) : _flags(1 << kIndexValid), _id(id) {}
    ~Base() { _flags = 0; }

    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;

    operator bool() const { return _flags & (1 << kIndexValid); }
    void clearFlag(size_t flag) { _flags &= ~(1ull << flag); }
    void setFlag(size_t flag) { _flags |= (1ull << flag); }
    bool testFlag(size_t shift) const { return _flags & (1 << shift); }

    uint32_t getID() const { return _id; }

    // there are
 private:
    uint32_t            _flags;       
    uint32_t            _id;
};
static_assert(sizeof(Base) == Base::kNextOffset, "size mismatch");

#define ManagerByPool(T) \
    typedef util::Pool<T, NS, NlPoolSpec, true> Pool; \
    static void* operator new(std::size_t count, uint32_t& id) { \
        Assert(count == sizeof(T)); \
        id = Pool::get().New(); \
        return &Pool::get()[id]; \
    } \
    static void operator delete(void* p) {} \
    template<typename Func> \
    static void foreach(Func func, size_t threads=0) { \
        util::foreach<typename T::Pool, util::TransBuilder<T>, util::ValidFilter<T>>(func, threads); \
    } \
    static std::atomic<size_t>   gDeleted; \
    ~T() { gDeleted++; } 

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

    Port(uint32_t id, Vid name, Direction dir, const DataType& dt);
    Port(uint32_t id, uint32_t index, Direction dir, const DataType& dt);
    const DataType& getDataType() const { return _dt; }

    Vid getName() const { return _name; }
    uint32_t getIndex() const { return _index.i; }
    uint32_t getDirIndex() const { return _index.d; }
    void setDirIndex(uint32_t d) { _index.d = d; }

    bool isInput() const { return testFlag(kPortInput); }
    bool isOutput() const { return testFlag(kPortOutput); }
    bool isInout() const { return testFlag(kPortInout); }

    Direction getDirection() const {
        return isInput() ? kPortInput : isOutput() ? kPortOutput : kPortInout;
    }

    void print(FILE* fp, bool indent, bool isModule) const;

 private:
    const DataType&     _dt;
    union {
        Vid             _name;
        struct {
            uint32_t    i;
            uint32_t    d;
        } _index;
    };
};

template<uint32_t NS>
class Net : public Base {
 public:
    ManagerByPool(Net);

    Net(uint32_t id, Vid name, Module<NS>& module, const DataType& dt) :
        Base(id), _name(name), _dt(dt), _module(module) {}

    Vid getName() const { return _name; }
    const DataType& getDataType() const { return _dt; }

    const Module<NS>& getModule() const { return _module; }

    void addMPort(Port<NS>* port) { _mPorts.emplace_back(port); }
    void addIPort(IPort<NS>* dp) { _iPorts.emplace_back(dp); }
    void addPPort(PPort<NS>* pp) { _pPorts.emplace_back(pp); }

    void transferMPort(size_t index, Net<NS>& dst);
    void transferIPort(size_t index, Net<NS>& dst);
    void transferPPort(size_t idnex, Net<NS>& dst);

    void print(FILE* fp, bool indent) const;

 public:
    typedef util::DeletableVec<Port<NS>*> MPortVec;

    typedef std::unique_ptr<IPort<NS>> IPortPtr;
    typedef util::DeletableVec<IPortPtr> IPortHolder;

    typedef std::unique_ptr<PPort<NS>> PPortPtr;
    typedef util::DeletableVec<PPortPtr> PPortHolder;

    const MPortVec& getMPorts() const { return _mPorts; }
    MPortVec& getMPorts() { return _mPorts; }
    const IPortHolder& getIPorts() const { return _iPorts; }
    IPortHolder& getIPorts() { return _iPorts; }
    const PPortHolder& getPPorts() const { return _pPorts; }
    PPortHolder& getPPorts() { return _pPorts; }

 private:
    Vid                 _name;
    const DataType&     _dt;
    Module<NS>&         _module;
    MPortVec            _mPorts;
    IPortHolder         _iPorts;
    PPortHolder         _pPorts;
};

template<uint32_t NS>
class IPort : public Base {
 public:
    ManagerByPool(IPort);

    IPort(uint32_t id, MInst<NS>& inst, Port<NS>& port) :
            Base(id), _inst(inst), _port(port) {}

    const MInst<NS>& getMInst() const { return _inst; }
    MInst<NS>& getMInst() { return _inst; }
    const Port<NS>& getPort() const { return _port; }
    Port<NS>& getPort() { return _port; }

    void print(FILE* fp, bool indent) const;

 private:
    MInst<NS>&              _inst;
    Port<NS>&               _port;
};

template<uint32_t NS>
class PPort : public Base {
 public:
    ManagerByPool(PPort);

    PPort(uint32_t id, PInst<NS>& inst, Port<NS>& port) :
            Base(id), _inst(inst), _port(port) {}

    const PInst<NS>& getPInst() const { return _inst; }
    const Port<NS>& getPort() const { return _port; }

    void print(FILE* fp, bool indent) const;

 private:
    PInst<NS>&              _inst;
    Port<NS>&               _port;
};

template<uint32_t NS>
class MInst : public Base {
 public:
    ManagerByPool(MInst);

    MInst(uint32_t id, Vid name, Module<NS>& module) :
            Base(id), _name(name), _module(module), _parent(nullptr) {}

    Vid getName() const { return _name; }
    const Module<NS>& getModule() const { return _module; }
    Module<NS>& getModule() { return _module; }
    const Module<NS>* getParent() const { return _parent; }
    void setParent(Module<NS>* parent) { _parent = parent; }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Module<NS>&             _module;
    Module<NS>*             _parent;
};

template<uint32_t NS>
class PInst : public Base {
 public:
    ManagerByPool(PInst);

    PInst(uint32_t id, Vid name, Vid procName, Module<NS>& parent, Process<NS>& proc) :
        Base(id), _name(name), _procName(procName), _proc(proc), _parent(parent) {}

    Vid getName() const { return _name; }
    Vid getProcessName() const { return _procName; }
    const Process<NS>& getProcess() const { return _proc; }
    Process<NS>& getProcess() { return _proc; }

    const Module<NS>& getParent() const {
        return _parent;
    }

    void print(FILE* fp, bool indent) const;

 private:
    Vid                     _name;
    Vid                     _procName;
    Process<NS>&            _proc;
    Module<NS>&             _parent;
};

template<uint32_t NS>
class Module : public Base {
 public:
    ManagerByPool(Module);

    Module(uint32_t id, Vid name) :
            Base(id), _name(name) {}

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
    bool addMInst(MInst<NS>* inst);
    bool addPInst(PInst<NS>* inst);

    bool hasPort(Vid) const;
    const Port<NS>& getPort(Vid pname) const;
    Port<NS>& getPort(Vid pname);

    bool hasMInst(Vid) const;
    const MInst<NS>& getMInst(Vid iname) const;
    MInst<NS>& getMInst(Vid iname);

    bool hasPInst(Vid) const;
    const PInst<NS>& getPInst(Vid iname) const;
    PInst<NS>& getPInst(Vid iname);

    void print(FILE* fp, bool indent) const;

 private:
    typedef std::unordered_map<Vid, Port<NS>*, Vid::Hash> PortIndex;
    typedef std::unordered_map<Vid, Net<NS>*, Vid::Hash> NetIndex;
    typedef std::unordered_map<Vid, MInst<NS>*, Vid::Hash> MInstIndex;
    typedef std::unordered_map<Vid, PInst<NS>*, Vid::Hash> PInstIndex;

 public:
    typedef std::unique_ptr<Port<NS>> PortPtr;
    typedef util::DeletableVec<PortPtr> PortHolder;

    typedef std::unique_ptr<Net<NS>> NetPtr;
    typedef util::DeletableVec<NetPtr> NetHolder;

    typedef std::unique_ptr<MInst<NS>> MInstPtr;
    typedef util::DeletableVec<MInstPtr> MInstHolder;

    typedef std::unique_ptr<PInst<NS>> PInstPtr;
    typedef util::DeletableVec<PInstPtr> PInstHolder;

    const PortHolder& getPorts() const { return _ports; }
    const NetHolder& getNets() const { return _nets; }
    const MInstHolder& getMInsts() const { return _minsts; }
    const PInstHolder& getPInsts() const { return _pinsts; }

 private:
    Vid                     _name;
    PortHolder              _ports;
    PortIndex               _portIndex;
    NetHolder               _nets;
    NetIndex                _netIndex;
    MInstHolder             _minsts;
    MInstIndex              _minstIndex;
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

    struct Hash {
        size_t operator()(const Process& p) const {
            return (size_t) &p;
        }
    };

    Process(uint32_t id);

    uint32_t getNumOfInput() const { return _inputIndex.validSize(); }
    uint32_t getNumOfOutput() const { return _outputIndex.validSize(); }
    uint32_t getNumOfInout() const { return _inoutIndex.validSize(); }
    uint32_t getNumOfPorts() const { return _ports.validSize(); }

    void setType(Type type);
    bool isComb() const { return testFlag(kTypeComb); }
    bool isSeq() const { return testFlag(kTypeSeq); }
    bool isCall() const { return testFlag(kTypeCall); }

    bool addPort(Port<NS>* port);
    const Port<NS>& getPort(size_t id) const;
    const Port<NS>& getPort(size_t id, typename Port<NS>::Direction dir) const;
    Port<NS>& getPort(size_t id);
    Port<NS>& getPort(size_t id, typename Port<NS>::Direction dir);

    void print(FILE* fp, bool indent, Vid name) const;

    typedef std::unique_ptr<Port<NS>> PortPtr;
    typedef util::DeletableVec<PortPtr> PortHolder;
    const PortHolder& getPorts() const { return _ports; }
    PortHolder& getPorts() { return _ports; }

    typedef util::DeletableVec<uint32_t> IndexVec;

 private:
    IndexVec                _inputIndex;
    IndexVec                _outputIndex;
    IndexVec                _inoutIndex;
    PortHolder              _ports;
};
}
