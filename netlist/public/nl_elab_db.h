#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "nl_def.h"
namespace netlist {
template<uint32_t NS>
class ElabDB {
 public:
    ElabDB();

    void elab();

    void printFlatten(FILE* fp) const;

 private:
    struct Nexus {
        Nexus() : needMerge(false), driver(0) {}
        ~Nexus() {}
        bool                    needMerge;
        uint64_t                driver;
        std::vector<uint64_t>   readers;
    };
    typedef std::shared_ptr<Nexus> NexusPtr;
    typedef std::vector<NexusPtr> Annotation;
    typedef std::unordered_set<uint32_t> VisitedIPorts;
    typedef std::vector<std::pair<NexusPtr, const IPort<NS>*>> NetNexus;
    typedef std::vector<std::pair<NexusPtr, const Net<NS>*>> IPortNexus;
    typedef std::pair<const Net<NS>*, NetNexus> NetNexusPair;
    typedef std::pair<const IPort<NS>*, IPortNexus> IPortNexusPair;
    typedef std::unordered_map<const Nexus*, NetNexusPair> NetNexusIndex;
    typedef std::unordered_map<const Nexus*, IPortNexusPair> IPortNexusIndex;

    void reset();

    // generate weights like DFS, number of cells
    void genWeights(const std::vector<Module<NS>*>& topo);

    // generate index to MInst ID from DFS;
    void genIndex();
    void visitInst(const MInst<NS>& inst, size_t dfs);

    // annotate port infomation
    void annotate(const std::vector<Module<NS>*>& topo,
                  Annotation& portAnno,
                  Annotation& netAnno);

    void annotateNet(const Net<NS>& net,
                     Annotation& portAnno,
                     Annotation& netAnno,
                     NetNexusIndex& netIndex,
                     IPortNexusIndex& portIndex,
                     VisitedIPorts& visitedIPorts);

    void mergeLink(Annotation& portAnno,
                   Annotation& netAnno,
                   NetNexusIndex& netIndex,
                   IPortNexusIndex& portIndex);

 private:
    std::vector<size_t>             _dfs;
    std::vector<size_t>             _dfsOffset;
    std::vector<size_t>             _cellNum;
    std::vector<size_t>             _cellOffset;
    std::vector<uint32_t>           _index;
};
}
