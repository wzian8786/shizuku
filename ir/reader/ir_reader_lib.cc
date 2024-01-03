#include "ir_reader_lib.h"
#include <cstdlib>
#include "szk_log.h"
#include "ir_db_impl.h"
namespace ir {
namespace reader {
using util::Logger;
Context gCtx;
std::unordered_map<Module<IR_DEFAULT>*, std::vector<std::pair<Net<IR_DEFAULT>*, NetContext>>> gNets;
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
    std::vector<uint8_t> notTop(Module<IR_DEFAULT>::Pool::getMaxSize());
    MInst<IR_DEFAULT>::foreach([&notTop](const MInst<IR_DEFAULT>& minst, size_t i) {
        notTop[minst.getModule().getID()] = 1;
    }, 0);
    Module<IR_DEFAULT>::foreach([&notTop](Module<IR_DEFAULT>& module, size_t i) {
        if (module.isRoot()) return;
        if (notTop[i]) return;
        uint32_t id;
        MInst<IR_DEFAULT>* minst = new (id) MInst<IR_DEFAULT>(id, module.getName(), module);
        module.setTop();
        bool suc = IRDBImpl<IR_DEFAULT>::get().getRoot().addMInst(minst);
        // there is no module whose name is identical, so addMInst must success
        Assert(suc);
    }, 0);
}

void resolveNets() {
    bool error = false;
    Module<IR_DEFAULT>::foreach([&error](Module<IR_DEFAULT>& module, size_t) {
        if (module.isRoot()) return;
        auto it = gNets.find(&module);
        Assert(it != gNets.end());
        for (auto nit : it->second) {
            Net<IR_DEFAULT>* net = nit.first;
            const NetContext& nc = nit.second;
            if (!module.addNet(net)) {
                Logger::error("Duplicate net '%s' in module '%s'",
                               net->getName().str().c_str(),
                               module.getName().str().c_str());
                error = true;
                continue;
            }
            const DataType& dt = net->getDataType();
            for (Vid pname : nc.upports) {
                if (!module.hasPort(pname)) {
                    Logger::error("Module '%s' doesn't have port '%s'",
                                   module.getName().str().c_str(), pname.str().c_str());
                    error = true;
                    continue;
                }
                Port<IR_DEFAULT>* port = &module.getPort(pname);
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
                MInst<IR_DEFAULT>* minst = &module.getMInst(iname);

                Module<IR_DEFAULT>& cmodule = minst->getModule(); 
                Assert(minst->getParent() == &module);

                if (!cmodule.hasPort(pname)) {
                    Logger::error("Module '%s' doesn't have port '%s'",
                                   cmodule.getName().str().c_str(), pname.str().c_str());
                    error = true;
                    continue;
                }
                Port<IR_DEFAULT>* cport = &cmodule.getPort(pname);
                if (cport->getDataType() != dt) {
                    Logger::error("DataType mismatch net '%s', instance '%s', port '%s'",
                                   net->getName().str().c_str(),
                                   iname.str().c_str(),
                                   cport->getName().str().c_str());
                    error = true;
                    continue;
                }
                uint32_t id;
                net->addIPort(new (id) IPort<IR_DEFAULT>(id, *minst, *cport));
            }

            for (auto p : nc.pports) {
                Vid iname = p.first;
                size_t index = (size_t)p.second;

                if (!module.hasPInst(iname)) {
                    Logger::error("Module '%s' doesn't have process instance '%s'",
                                   module.getName().str().c_str(), iname.str().c_str());
                    error = true;
                    continue;
                }
                PInst<IR_DEFAULT>* pinst = &module.getPInst(iname);
                Assert(&pinst->getParent() == &module);

                Process<IR_DEFAULT>& process = pinst->getProcess();
                if (!index) {
                    Logger::error("Invalid process port index '0', the index starts from 1");
                    error = true;
                    continue;
                }
                if (index > process.getNumOfPorts()) {
                    Logger::error("Process '%s' doesn't have the '%lu'th port",
                                   pinst->getProcessName().str().c_str(), index);
                    error = true;
                    continue;
                }
                Port<IR_DEFAULT>* pport = &process.getPort(index);
                if (pport->getDataType() != dt) {
                    Logger::error("DataType mismatch net '%s', process '%s', index 'lu'",
                                   net->getName().str().c_str(),
                                   pinst->getProcessName().str().c_str(), index);
                    error = true;
                    continue;
                }
                uint32_t id;
                net->addPPort(new (id) PPort<IR_DEFAULT>(id, *pinst, *pport));
            }
        }
    }, 0);
    if (error) exit(-1);
}
}
}
