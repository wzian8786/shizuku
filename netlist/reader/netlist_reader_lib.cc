#include "netlist_reader_lib.h"
#include <cstdlib>
#include "szk_log.h"
namespace netlist {
namespace reader {
using util::Logger;
Context gCtx;
std::unordered_map<Module<NL_DEFAULT>*, std::vector<std::pair<Net<NL_DEFAULT>*, NetContext>>> gNets;
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

void resolveNets() {
    Module<NL_DEFAULT>::foreach([](Module<NL_DEFAULT>& module, size_t) {
        auto it = gNets.find(&module);
        Assert(it != gNets.end());
        for (auto nit : it->second) {
            Net<NL_DEFAULT>* net = nit.first;
            const NetContext& nc = nit.second;
            module.addNet(net);
            net->setModule(&module);
            for (Vid pname : nc.upports) {
                Port<NL_DEFAULT>* port = &module.getPort(pname);
                net->addUpPort(port);
            }
            for (auto p : nc.downports) {
                Vid iname = p.first;
                Vid pname = p.second;
                HierInst<NL_DEFAULT>* hinst = &module.getHierInst(iname);
                Module<NL_DEFAULT>& cmodule = hinst->getModule(); 
                Assert(hinst->getParent() == &module);
                Port<NL_DEFAULT>* cport = &cmodule.getPort(pname);
                net->addDownPort(new DownPort<NL_DEFAULT>(hinst, cport));
            }
        }
    }, 0);
}
}
}
