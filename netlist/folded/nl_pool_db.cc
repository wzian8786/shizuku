#include "nl_pool_db.h"
#include "nl_folded_obj.h"
#include "szk_foreach.h"
namespace netlist {
template <uint32_t Namespace>
PoolDB<Namespace>::~PoolDB() {
    Module<Namespace>::foreach([](Module<Namespace>& mod, size_t i) {
        mod.~Module();
    }, 0/*thread_num*/); 
    Process<Namespace>::foreach([](Process<Namespace>& p, size_t i) {
        p.~Process();
    }, 0);
}

template class PoolDB<NL_DEFAULT>;
}

namespace util {
template class Pool<netlist::Port<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Net<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Module<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::HierInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::PInst<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::DownPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::PPort<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
template class Pool<netlist::Process<netlist::NL_DEFAULT>,
                    netlist::NL_DEFAULT, netlist::NlPoolSpec>;
}
