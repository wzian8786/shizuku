#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "nl_def.h"
#include "nl_datatype.h"
#include "szk_pool.h"
#include "szk_foreach.h"
#include "vid.h"
namespace netlist {
#define ManagerByPool(T) \
    typedef util::Pool<T, Namespace, NlPoolSpec> Pool; \
    static void* operator new(std::size_t count) { \
        Assert(count == sizeof(T)); \
        uint32_t id = Pool::get().New(); \
        T* m = &Pool::get()[id]; \
        return m; \
    } \
    static void operator delete(void* p) {} \
    ~T() { Base::~Base(); } \
    template<typename Func> \
    static void foreach(Func func, size_t threads) { \
        util::foreach<typename T::Pool, util::TransBuilder<T>, util::ValidFilter<T>>(func, threads); \
    }

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

 private:
    Vid                 _name;
    const DataType*     _dt;
};

template<uint32_t Namespace>
class Net : public Base {
 public:
    ManagerByPool(Net);

    Net(Vid name, const DataType* dt) :
        _name(name), _dt(dt) {}

    Vid getName() const { return _name; }

 private:
    Vid                 _name;
    const DataType*     _dt;
};

template<uint32_t Namespace>
class HierInst : public Base {
 public:
    ManagerByPool(HierInst);

    HierInst(Vid name, Module<Namespace>* module) :
        _name(name), _module(module) {}

    Vid getName() const { return _name; }
    const Module<Namespace>& getModule() const { return *_module; }

 private:
    Vid                     _name;
    Module<Namespace>*      _module;
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

 private:
    uint32_t                _pad;
    Vid                     _name;
    PortHolder              _ports;
    PortIndex               _portIndex;
    NetHolder               _nets;
    NetIndex                _netIndex;
    HierInstHolder          _hinsts;
    HierInstIndex           _hinstIndex;
};

static_assert(sizeof(Base) == 4, "size unmatch");
}
