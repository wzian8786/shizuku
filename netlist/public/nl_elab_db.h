#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
#include "nl_def.h"
namespace netlist {
template<uint32_t NS>
class ElabDB {
 public:
    ElabDB();

    void elab();

    uint64_t getCellOffset(const MInst<NS>& inst) const;
    uint64_t getCellOffset(const PInst<NS>& inst) const;

    void printFlatten(FILE* fp) const;

 private:
    void resetWeights();

    // generate weights like DFS, number of cells
    void genWeights(const std::vector<Module<NS>*>& topo);

    // generate index to MInst ID from DFS;
    void genIndex();
    void visitInst(const MInst<NS>& inst, size_t dfs);

 private:
    std::vector<size_t>             _dfs;
    std::vector<size_t>             _dfsOffset;
    std::vector<size_t>             _cellNum;
    std::vector<size_t>             _cellMInstOffset;
    std::vector<size_t>             _cellPInstOffset;
    std::vector<uint32_t>           _index;
};
}
