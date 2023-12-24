#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
#include "nl_def.h"
namespace netlist {
template<uint32_t NS>
class ElabAnnotator;

template<uint32_t NS>
class ElabDB {
 public:
    ElabDB();

    void elab();

    uint64_t getCellOffset(const MInst<NS>& inst) const;
    uint64_t getCellOffset(const PInst<NS>& inst) const;

    void debugPrint() const;

    // _dfs;
    uint64_t getTotalNumInst(uint32_t moduleID) const;

    // _dfsOFfset;
    uint64_t getDFSOffset(uint32_t instID) const;

    // _index
    uint32_t getMInstID(uint64_t dfs) const;
 private:
    void resetWeights();

    // generate weights like DFS, number of cells
    void genWeights(const std::vector<Module<NS>*>& topo);

    // generate index to MInst ID from DFS;
    void genIndex();
    void visitInst(const MInst<NS>& inst, uint64_t dfs);

    void createVertex(const ElabAnnotator<NS>& a);

 private:
    std::vector<uint64_t>           _dfs;
    std::vector<uint64_t>           _dfsOffset;
    std::vector<uint64_t>           _cellNum;
    std::vector<uint64_t>           _cellMInstOffset;
    std::vector<uint64_t>           _cellPInstOffset;
    std::vector<uint32_t>           _index;
};
}
