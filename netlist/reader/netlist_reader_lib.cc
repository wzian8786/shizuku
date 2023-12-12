#include "netlist_reader_lib.h"
#include <cstdlib>
#include "szk_log.h"
namespace netlist {
namespace reader {
using Netlist = netlist::Netlist<netlist::NL_DEFAULT>;
using Port = netlist::Port<netlist::NL_DEFAULT>;
using Net = netlist::Net<netlist::NL_DEFAULT>;
using HierInst = netlist::HierInst<netlist::NL_DEFAULT>;
using Module = netlist::Module<netlist::NL_DEFAULT>;
using Vid = netlist::Vid;
using util::Logger;
Context gCtx;
void sanityCheck() {
    bool error = false;
    if (!gCtx.unresolvedModules.empty()) {
        for (auto it : gCtx.unresolvedModules) {
            Logger::error("Unresolved module '%s'", it.first.str().c_str());
            error = true;
        }
    }
    if (error) {
        exit(-1);
    }
}
}
}
