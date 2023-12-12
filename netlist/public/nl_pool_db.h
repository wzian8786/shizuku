#pragma once
#include "nl_def.h"
#include "szk_pool.h"
namespace netlist {
template <uint32_t Namespace>
class PoolDB {
 public:
    PoolDB() {}
    ~PoolDB();
    typedef util::Pool<Port<Namespace>, Namespace, NlPoolSpec> PortPool;
    typedef util::Pool<Module<Namespace>, Namespace, NlPoolSpec> ModulePool;
};
}
