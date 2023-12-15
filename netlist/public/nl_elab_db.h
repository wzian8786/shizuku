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

 private:
    void reset();
    // generate weights like DFS, number of cells
    void genWeights();
    // generate index to MInst ID from DFS;
    void genHIndex();

    void visitInst(const MInst<NS>& inst, size_t dfs);

 private:
    std::vector<size_t>             _dfs;
    std::vector<size_t>             _dfsOffset;
    std::vector<size_t>             _cellNum;
    std::vector<size_t>             _cellOffset;
    std::vector<uint32_t>           _hindex;
};
}
