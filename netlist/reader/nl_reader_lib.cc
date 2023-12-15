#include "nl_reader_lib.h"
#include <cstdlib>
#include "szk_log.h"
#include "nl_netlist.h"
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
    if (!gCtx.unresolvedProcesses.empty()) {
        for (auto it : gCtx.unresolvedProcesses) {
            Logger::error("Unresolved process '%s'", it.first.str().c_str());
            error = true;
        }
    }
    if (error) {
        exit(-1);
    }
}

void buildTops() {
    std::vector<uint8_t> notTop(Module<NL_DEFAULT>::Pool::get().getMaxSize());
    MInst<NL_DEFAULT>::foreach([&notTop](const MInst<NL_DEFAULT>& minst, size_t i) {
        notTop[minst.getModule().getID()] = 1;
    }, 0);
    Module<NL_DEFAULT>::foreach([&notTop](Module<NL_DEFAULT>& module, size_t i) {
        if (module.isRoot()) return;
        if (notTop[i]) return;
        MInst<NL_DEFAULT>* minst = new MInst<NL_DEFAULT>(module.getName(), &module);
        module.setTop();
        bool suc = Netlist<NL_DEFAULT>::get().getRoot().addMInst(minst);
        // there is no module whose name is identical, so addMInst must success
        Assert(suc);
    }, 0);
}

void resolveNets() {
    bool error = false;
    Module<NL_DEFAULT>::foreach([&error](Module<NL_DEFAULT>& module, size_t) {
        if (module.isRoot()) return;
        auto it = gNets.find(&module);
        Assert(it != gNets.end());
        for (auto nit : it->second) {
            Net<NL_DEFAULT>* net = nit.first;
            const NetContext& nc = nit.second;
            module.addNet(net);
            net->setModule(&module);
            const DataType* dt = net->getDataType();
            for (Vid pname : nc.upports) {
                if (!module.hasPort(pname)) {
                    Logger::error("Module '%s' doesn't have port '%s'",
                                   module.getName().str().c_str(), pname.str().c_str());
                    error = true;
                    continue;
                }
                Port<NL_DEFAULT>* port = &module.getPort(pname);
                if (port->getDataType() != dt) {
                    Logger::error("DataType mismatch net '%s', port '%s'",
                                   net->getName().str().c_str(),
                                   port->getName().str().c_str());
                    error = true;
                    continue;
                }
                net->addMPort(port);
            }
            for (auto p : nc.downports) {
                Vid iname = p.first;
                Vid pname = p.second;

                if (!module.hasMInst(iname)) {
                    Logger::error("Module '%s' doesn't have child instance '%s'",
                                   module.getName().str().c_str(), iname.str().c_str());
                    error = true;
                    continue;
                }
                MInst<NL_DEFAULT>* minst = &module.getMInst(iname);

                Module<NL_DEFAULT>& cmodule = minst->getModule(); 
                Assert(minst->getParent() == &module);

                if (!cmodule.hasPort(pname)) {
                    Logger::error("Module '%s' doesn't have port '%s'",
                                   cmodule.getName().str().c_str(), pname.str().c_str());
                    error = true;
                    continue;
                }
                Port<NL_DEFAULT>* cport = &cmodule.getPort(pname);
                if (cport->getDataType() != dt) {
                    Logger::error("DataType mismatch net '%s', instance '%s', port '%s'",
                                   net->getName().str().c_str(),
                                   iname.str().c_str(),
                                   cport->getName().str().c_str());
                    error = true;
                    continue;
                }
                net->addIPort(new IPort<NL_DEFAULT>(minst, cport));
            }

            for (auto p : nc.pports) {
                Vid iname = p.first;
                Vid pname = p.second;

                if (!module.hasPInst(iname)) {
                    Logger::error("Module '%s' doesn't have process instance '%s'",
                                   module.getName().str().c_str(), iname.str().c_str());
                    error = true;
                    continue;
                }
                PInst<NL_DEFAULT>* pinst = &module.getPInst(iname);
                Assert(&pinst->getParent() == &module);

                Process<NL_DEFAULT>& process = pinst->getProcess();
                if (!process.hasPort(pname)) {
                    Logger::error("Process '%s' doesn't have port '%s'",
                                   process.getName().str().c_str(), pname.str().c_str());
                    error = true;
                    continue;
                }
                Port<NL_DEFAULT>* pport = &process.getPort(pname);
                if (pport->getDataType() != dt) {
                    Logger::error("DataType mismatch net '%s', process '%s', port '%s'",
                                   net->getName().str().c_str(),
                                   pname.str().c_str(),
                                   pport->getName().str().c_str());
                    error = true;
                    continue;
                }
                net->addPPort(new PPort<NL_DEFAULT>(pinst, pport));
            }
        }
    }, 0);
    if (error) exit(-1);
}
}
}
