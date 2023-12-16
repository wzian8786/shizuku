#include "nl_elab_db.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_vertex.h"
#include "nl_vid_db.h"
namespace netlist {
template<uint32_t NS>
ElabDB<NS>::ElabDB() {}

template<uint32_t NS>
void ElabDB<NS>::elab() {
    std::vector<Module<NS>*> topo;
    Netlist<NS>::get().bottomUp(topo);
    Annotation portAnno(Port<NS>::Pool::get().getMaxSize());
    Annotation netAnno(Net<NS>::Pool::get().getMaxSize());

    reset();

    genWeights(topo);
    genIndex();

    annotate(topo, portAnno, netAnno); 
}

template<uint32_t NS>
void ElabDB<NS>::reset() {
    _dfs.clear();
    _dfsOffset.clear();
    _cellNum.clear();
    _cellOffset.clear();
    _dfs.resize(Module<NS>::Pool::get().getMaxSize());
    _dfsOffset.resize(MInst<NS>::Pool::get().getMaxSize());
    _cellNum.resize(Module<NS>::Pool::get().getMaxSize());
    _cellOffset.resize(MInst<NS>::Pool::get().getMaxSize());
}

template<uint32_t NS>
void ElabDB<NS>::genWeights(const std::vector<Module<NS>*>& topo) {
    for (auto mod : topo) {
        const typename Module<NS>::MInstHolder& insts = mod->getMInsts();
        const typename Module<NS>::PInstHolder& pinsts = mod->getPInsts();
        size_t dfs = 1;
        size_t cellNum = kReservedCell;
        for (auto it = pinsts.begin(); it != pinsts.end(); ++it) {
            const PInst<NS>& pinst = **it;
            const Process<NS>& process = pinst.getProcess();
            cellNum += Cell::getNumCell(process.getNumOfInput(), process.getNumOfOutput());
        }

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            const MInst<NS>& cinst = **it;
            const Module<NS>& cmod = cinst.getModule();
            _dfsOffset[cinst.getID()] = dfs;
            _cellOffset[cinst.getID()] = cellNum;

            Assert(_dfs[cmod.getID()]);
            dfs += _dfs[cmod.getID()];
            cellNum += _cellNum[cmod.getID()];
        }
        _dfs[mod->getID()] = dfs;
        _cellNum[mod->getID()] = cellNum;
    }
}

template<uint32_t NS>
void ElabDB<NS>::genIndex() {
    const Module<NS>& root = Netlist<NS>::get().getRoot();
    size_t maxDfs = _dfs[root.getID()];
    _index.resize(maxDfs);
    const typename Module<NS>::MInstHolder& insts = root.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, _dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::visitInst(const MInst<NS>& inst, size_t dfs) {
    _index[dfs] = inst.getID();
    const Module<NS>& module = inst.getModule();
    const typename Module<NS>::MInstHolder& insts = module.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, dfs+_dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::printFlatten(FILE* fp) const {
    printf("#0 %s(%s)\n", Vid(kVidSRoot).str().c_str(),
                          Vid(kVidSRoot).str().c_str());
    for (size_t i = 1; i < _index.size(); ++i) {
        Assert(i < MInst<NS>::Pool::get().getMaxSize());
        const MInst<NS>& minst = MInst<NS>::Pool::get()[_index[i]];
        Assert(minst);
        const Module<NS>& module = minst.getModule();
        printf("#%lu %s(%s)\n", i, minst.getName().str().c_str(),
                                   module.getName().str().c_str());
    }
}

template<uint32_t NS>
void ElabDB<NS>::mergeLink(Annotation& portAnno,
                                 Annotation& netAnno,
                                 NetNexusIndex& netIndex,
                                 IPortNexusIndex& portIndex) {
    std::unordered_set<const Nexus*> visited;
    for (auto it = netIndex.begin(); it != netIndex.end(); ++it) {
        if (visited.find(it->first) != visited.end()) continue;
        std::vector<const Nexus*> todo = { it->first };
        std::vector<const Net<NS>*> nets;
        std::vector<const Port<NS>*> mports;
        while (!todo.empty()) {
            const Nexus* nexus = todo.back();
            todo.pop_back();
            visited.emplace(nexus);
            Assert(visited.find(nexus) != visited.end());

            auto nit = netIndex.find(nexus);
            Assert(nit != netIndex.end());
            const Net<NS>& net = *nit->second.first;
            nets.emplace_back(&net);

            const typename Net<NS>::MPortVec& mportVec = net.getMPorts();
            for (const auto& ptr : mportVec) {
                mports.emplace_back(ptr);
            }

            const NetNexus& netNexus = nit->second.second;
            for (const auto& p : netNexus) {
                const Nexus* pnexus = p.first.get();
                auto pit = portIndex.find(pnexus);
                Assert(pit != portIndex.end());

                const IPortNexus& portNexus = pit->second.second;
                for (const auto& n : portNexus) {
                    todo.emplace_back(n.first.get());
                }
            }
        }
    }
}

template<uint32_t NS>
void ElabDB<NS>::annotateNet(const Net<NS>& net,
                             Annotation& portAnno,
                             Annotation& netAnno,
                             NetNexusIndex& netIndex,
                             IPortNexusIndex& portIndex,
                             VisitedIPorts& visitedIPorts) {
    NexusPtr nexus(new Nexus());

    bool needMerge = false;
    const typename Net<NS>::IPortHolder& iports = net.getIPorts();
    NetNexus ninfo;
    for (auto& ptr : iports) {
        const IPort<NS>& iport = *ptr;
        Assert(iport.getPort().getID() < portAnno.size());
        NexusPtr pnexus = portAnno[iport.getID()];
        visitedIPorts.insert(iport.getID());
        if (pnexus->needMerge) {
            auto dit = portIndex.find(pnexus.get());
            if (dit == portIndex.end()) {
                dit = portIndex.emplace(pnexus.get(), std::make_pair(&iport, IPortNexus())).first;
            }
            dit->second.second.emplace_back(nexus, &net);
            needMerge = true;
            ninfo.emplace_back(pnexus, &iport);
        }
    }

    if (!needMerge) {
        const typename Net<NS>::MPortVec& mports = net.getMPorts();
        if (mports.empty()) {
            netAnno[net.getID()] = nexus;
        } else {
            nexus->needMerge = mports.size() > 1;
            for (auto& ptr : mports) {
                portAnno[ptr->getID()] = nexus;
            } 
        }
    } else {
        netIndex.emplace(nexus.get(), std::make_pair(&net, std::move(ninfo)));
    }
}

// The basic idea of the algorithms is that:
// 1. For each net, if it doesn't connect to any MPort(module port), means 
//    all the driver & readers must be in the sub-tree of current module.
//    And the cell offset can be calculated.
//    This information is annotated on the net, no matter where the current
//    module is instantiated, this information can always be reused.
// 2. If a net connects to any module port, means it's impossible to find all
//    the driver & readers within the subtree of current module. Still we collect
//    the driver & readers already known, and annotate the information on the
//    module port. So upper module can use this information.
//
// The annotation actually is done in folded view, and elaboration uses the
// annotation to create driver & readers connection is flatten view. In this way
// it is not necessary to searching driver & readers agian & again when elaboration 
// transverses the hierarchical tree. And this algorithms works perfectly as it 
// benefits both memory usage and elaboration performance.
// 
// But there is an exception.
// If a net is connected to more than one module ports. And in the upper module,
// The nets connected to these ports can't see each other, they do not know the
// existence of their sibling. The net is such case may believe it already knows
// all the driver & readers connected together, but the fact is it only finds
// driver & readers in its scope, not its siblings' scope. 
// 
// So we have to fix this issue manually.
// 1. We use class Nexus to describe driver & readers connected. 
// 2. And the are 2 kinds of Nexus:
//    a. IPortNexus is used to describe driver & readers connected to a bunch of 
//       IPorts which actually connected together. Please note IPort with the
//       same module are always merged into one IPortNexus.
//    b. NetNexus is used to describe driver & readers connected to one Net. As
//       mentioned Net can't see its sibling directly, so NetNexuses in one module
//       which actually connected together are not merged.
//    
// The fix works as below:
// 1. For each IPortNexus, find all the NetNexuses it connected to.
// 2. For each NetNexus, find all the IPortNexuses it connected to.
// 3. Use the information of (1) (2), find all the IPortNexuses & NetNexuses 
//    connected together. The number of both IPortNexus & NetNexus may be more than
//    One. This is because: 
//    a. NetNexus can't see it's sibling, its easy to understand.
//    b. IPortNexus is always merged together in one sub instance, but IPortNexuses
//       from different sub instance are not.
//    These IPortNexuses & NetNexuses are merged together, and called a link.
// 4. For each link, it works just as normal Nexus. If it connected to module ports,
//    annotate it on module ports, otherwise annotate it on the nets of this link. 
template<uint32_t NS>
void ElabDB<NS>::annotate(const std::vector<Module<NS>*>& topo,
                          Annotation& portAnno,
                          Annotation& netAnno) {
    for (auto module : topo) {
        const typename Module<NS>::NetHolder& nets = module->getNets();
        VisitedIPorts visitedIPorts;
        NetNexusIndex netIndex;
        IPortNexusIndex portIndex;
        for (auto& itn : nets) {
            annotateNet(*itn, portAnno, netAnno, netIndex, portIndex, visitedIPorts);
        }
        mergeLink(portAnno, netAnno, netIndex, portIndex);

        const typename Module<NS>::MInstHolder& insts = module->getMInsts();
        for (auto& it : insts) {
            MInst<NS>& inst = *it;
            (void) inst;
        }
    }
}

template class ElabDB<NL_DEFAULT>;
}
