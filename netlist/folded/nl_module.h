#pragma once
#include <vector>
#include <memory>
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
        kPortInput = 0,
        kPortOutput,
        kPortInout,
    };

    Port(Vid name, Direction dir, const DataType* dt);

    Vid getName() const { return _name; }

    static void* operator new(std::size_t count);
    static void operator delete(void* p);

 private:
    Direction           _dir;
    Vid                 _name;
    const DataType*     _dt;
};

template<uint32_t Namespace>
class Module : public Base {
 public:
    typedef util::Pool<Module, Namespace, NlPoolSpec> Pool;

    explicit Module(Vid name) : _name(name) {}

    Vid getName() const { return _name; }

    static void* operator new(std::size_t count);
    static void operator delete(void* p);

 private:
    typedef std::unique_ptr<Port<Namespace>> PortPtr;

 private:
    uint32_t                _pad;
    Vid                     _name;
    std::vector<PortPtr>    _ports;
};
}
