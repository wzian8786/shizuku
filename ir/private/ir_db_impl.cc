#include "ir_db_impl.h"
#include <algorithm>
#include "ir_folded_obj.h"
namespace ir {
template<uint32_t NS>
void IRDBImpl<NS>::topDown(std::vector<Module<NS>*>& modules) {
    std::vector<uint32_t> visited(Module<NS>::Pool::getMaxSize());
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
        };
        i++;
    }
}

template<uint32_t NS>
void IRDBImpl<NS>::bottomUp(std::vector<Module<NS>*>& modules) {
    topDown(modules);
    std::reverse(modules.begin(), modules.end());
}

template<uint32_t NS>
IRDBImpl<NS> __attribute__((init_priority(300))) IRDBImpl<NS>::gSingleton;

template class IRDBImpl<IR_DEFAULT>;
}
