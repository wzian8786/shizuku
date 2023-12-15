#include "nl_netlist.h"
#include <algorithm>
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
void Netlist<NS>::topDown(std::vector<Module<NS>*>& modules) {
    std::vector<uint32_t> visited(Module<NS>::Pool::get().getMaxSize());
    Module<NS>* root = &this->getRoot();
    modules.emplace_back(root);
    size_t i = 0;
    while (i != modules.size()) {
        Module<NS>& cur = *modules[i];
        const typename Module<NS>::MInstHolder& minsts = cur.getMInsts();
        for (auto it = minsts.begin(); it != minsts.end(); ++it) {
            MInst<NS>& minst = **it;
            Module<NS>& module = minst.getModule();
            if (!visited[module.getID()]) {
                visited[module.getID()] = 1;
                modules.push_back(&module);
            }
            i++;
        }
    }
}

template<uint32_t NS>
void Netlist<NS>::bottomUp(std::vector<Module<NS>*>& modules) {
    topDown(modules);
    std::reverse(modules.begin(), modules.end());
}

template<uint32_t NS>
Netlist<NS> __attribute__((init_priority(300))) Netlist<NS>::gSingleton;

template class Netlist<NL_DEFAULT>;
}
