#include "nl_netlist.h"
#include "nl_folded_obj.h"
namespace netlist {
template<uint32_t NS>
void Netlist<NS>::print(FILE* fp, bool indent) const {
    Module<NS>::foreach([fp, indent](const Module<NS>& module, size_t) {
        if (module.isRoot()) return;
        module.print(fp, indent);
    }, 1);
    Process<NS>::foreach([fp, indent](const Process<NS>& process, size_t) {
        process.print(fp, indent);
    }, 1);
}

template<uint32_t NS>
Netlist<NS> __attribute__((init_priority(300))) Netlist<NS>::gSingleton;

template class Netlist<NL_DEFAULT>;
}
