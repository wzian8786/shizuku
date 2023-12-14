#include "elaborator.h"
#include "nl_folded_obj.h"
namespace netlist {
template<uint32_t NS>
Elaborator<NS>::Elaborator() :
    _dfs(Module<NS>::Pool::get().getMaxSize()),
    _cellNum(Module<NS>::Pool::get().getMaxSize()) {}

template<uint32_t NS>
void genWeight() {
    size_t end = Module<NS>::Pool::get().getMaxSize();
    for (size_t i = 0; i < end; ++i) {
        const Module<NS>& mod = Module<NS>::Pool::get()[i];
        if (!mod) continue;
    }
}
}
