#include "nl_pool_db.h"
#include "nl_module.h"
#include "szk_foreach.h"
namespace netlist {
// initialization priority of Pool is 101 while PoolDB is 102, so the deconstructor
// of PoolDB is always executed before Pool.
template <uint32_t Namespace>
PoolDB<Namespace> __attribute__((init_priority(102))) PoolDB<Namespace>::gSingleton;

template <uint32_t Namespace>
PoolDB<Namespace>::~PoolDB() {
    util::foreach<ModulePool>([](Module<Namespace>& mod, size_t i) {
        if (mod) {
            mod.~Module();
        }
    }); 
}

template class PoolDB<NL_DEFAULT>;
}

namespace util {
template class Pool<netlist::Port<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Module<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
}
