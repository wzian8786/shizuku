#include "nl_pool_db.h"
#include "nl_folded_obj.h"
#include "szk_foreach.h"
namespace netlist {
template <uint32_t Namespace>
PoolDB<Namespace>::~PoolDB() {
    util::foreach<ModulePool,
                  util::TransBuilder<Module<Namespace>>,
                  util::ValidFilter<Module<Namespace>>>([](Module<Namespace>& mod, size_t i) {
        mod.~Module();
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
