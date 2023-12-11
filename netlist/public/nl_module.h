#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "netlist_def.h"
#include "nl_datatype.h"
#include "nl_base.h"
#include "szk_pool.h"
#include "vid.h"
namespace netlist {
template<uint32_t Namespace>
class Port : public Base {
 public:
    typedef util::Pool<Port, Namespace, NlPoolSpec> Pool;

    enum Direction {
        kPortInvalid = 0,
        kPortInput = kIndexForDerived,
        kPortOutput,
        kPortInout,
    };

    Port(Vid name, Direction dir, const DataType* dt);
    ~Port() { Base::~Base(); }

    Vid getName() const { return _name; }

    static void* operator new(std::size_t count);
    static void operator delete(void* p) {}

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
class Module : public Base {
 public:
    typedef util::Pool<Module, Namespace, NlPoolSpec> Pool;

    explicit Module(Vid name) : _name(name) {}
    ~Module() { Base::~Base(); }

    Vid getName() const { return _name; }

    static void* operator new(std::size_t count);
    static void operator delete(void* p) {}

    bool addPort(Port<Namespace>* port);

 private:
    typedef std::unique_ptr<Port<Namespace>> PortPtr;
    typedef std::vector<PortPtr> PortHolder;
    typedef std::unordered_map<Vid, Port<Namespace>*, Vid::Hash> PortIndex;

 private:
    uint32_t                _pad;
    Vid                     _name;
    PortHolder              _ports;
    PortIndex               _portIndex;
};
}
