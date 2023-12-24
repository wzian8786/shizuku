#include "nl_elab_db.h"
#include <memory>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_vertex.h"
#include "nl_vid_db.h"
#include "nl_vertex.h"
#include "szk_log.h"
namespace netlist {
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
// existence of their siblings. The net of such case may believe it already knows
// all the driver & readers connected together, but the fact is it only finds
// driver & readers in its scope, not in its siblings' scope. 
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
class ElabAnnotator {
 public:
    explicit ElabAnnotator(const ElabDB<NS>& db) :
        _portAnno(Port<NS>::Pool::getMaxSize()),
        _netAnno(Net<NS>::Pool::getMaxSize()),
        _portNexuses(Module<NS>::Pool::getMaxSize()),
        _netNexuses(Module<NS>::Pool::getMaxSize()),
        _db(db) {}

 public:
    struct Nexus {
        Nexus() : hasSibling(false), driver(0) {}
        ~Nexus() {}
        bool                            hasSibling;
        uint64_t                        driver;
        std::vector<std::pair<
            uint64_t, uint64_t>>        readers;
    };
    typedef std::vector<std::shared_ptr<Nexus>> Annotation;
    const Annotation& getAnnotation() const { return _netAnno; }

 private:
    struct PairHash {
        size_t operator()(const std::pair<Nexus*, uint32_t>& p) const {
            return std::hash<size_t>()((size_t)p.first) ^
                   std::hash<size_t>()((size_t)p.second);
        }
    };

    typedef std::shared_ptr<Nexus> NexusPtr;
    struct IPortNexus;
    struct NetNexus;
    struct NetNexus {
        NetNexus(NexusPtr nx, const Net<NS>& n) :
                 nexus(nx), net(n) {}
        NexusPtr                        nexus;
        const Net<NS>&                  net;
        std::vector<const IPortNexus*>  ports;
    };
    struct IPortNexus {
        explicit IPortNexus(Nexus* nx) :
                   nexus(nx) {}
        Nexus*                          nexus;
        std::vector<const IPort<NS>*>   ports;
        std::vector<NetNexus*>          nets;
    };

    typedef std::vector<std::vector<std::unique_ptr<NetNexus>>> NetNexusVec;
    typedef std::vector<std::vector<std::unique_ptr<IPortNexus>>> IPortNexusVec;
    typedef std::unordered_map<std::pair<Nexus*, uint32_t>,
                IPortNexus*, PairHash> IPortNexusIndex;
    
    void annotateNet(const Net<NS>& net, IPortNexusIndex& portNexusIndex) {
        util::Logger::debug("(elab) annotating net %s(%u)",
                            net.getName().str().c_str(), net.getID());
        NexusPtr nexus(new Nexus());
        NetNexus* netNexus = nullptr;
        bool hasSibling = false;
        const typename Net<NS>::IPortHolder& iports = net.getIPorts();
        uint32_t moduleID = net.getModule().getID();
        for (auto it = iports.begin(); it != iports.end(); ++it) {
            const IPort<NS>& iport = **it;
            Assert(iport.getPort().getID() < _portAnno.size());
            NexusPtr pnexus = _portAnno[iport.getPort().getID()];
            // Port may not connected to any net, then is not annotated
            if (!pnexus) continue;
            if (pnexus->hasSibling) {
                if (!netNexus) {
                    netNexus = new NetNexus(nexus, net);
                    _netNexuses[moduleID].emplace_back(netNexus);
                }
                std::pair<Nexus*, uint32_t> key { pnexus.get(), iport.getMInst().getID() };
                auto dit = portNexusIndex.find(key);
                if (dit == portNexusIndex.end()) {
                    IPortNexus* portNexus = new IPortNexus(pnexus.get());
                    _portNexuses[moduleID].emplace_back(portNexus);
                    dit = portNexusIndex.emplace(key, portNexus).first;
                }
                IPortNexus* portNexus = dit->second;
                portNexus->ports.emplace_back(&iport);
                portNexus->nets.emplace_back(netNexus);
                netNexus->ports.emplace_back(portNexus);
                hasSibling = true;
            } else {
                mergeNexus(*nexus, *pnexus, iport);
            }
        }

        const typename Net<NS>::PPortHolder& pports = net.getPPorts();
        for (auto it = pports.begin(); it != pports.end(); ++it) {
            const PPort<NS>& pport = **it;
            const PInst<NS>& pinst = pport.getPInst();
            typename Port<NS>::Direction dir = pport.getPort().getDirection();
            const Process<NS>& process = pinst.getProcess();
            uint64_t offset = _db.getCellOffset(pinst);
            uint32_t dirIndex = pport.getPort().getDirIndex();
            switch (dir) {
            case Port<NS>::kPortInput:
                nexus->readers.emplace_back(offset, dirIndex);
                break;
            case Port<NS>::kPortOutput:
                Assert(!nexus->driver);
                nexus->driver = offset + dirIndex;
                break;
            case Port<NS>::kPortInout:
                Assert(!nexus->driver);
                nexus->driver = offset + process.getNumOfOutput() + dirIndex;
                nexus->readers.emplace_back(offset, dirIndex);
                break;
            default:
                ASSERT(0);
            }
        }

        if (!hasSibling) {
            const typename Net<NS>::MPortVec& mports = net.getMPorts();
            if (mports.empty()) {
                _netAnno[net.getID()] = nexus;
            } else {
                nexus->hasSibling = mports.size() > 1;
                for (auto& ptr : mports) {
                    _portAnno[ptr->getID()] = nexus;
                    util::Logger::debug("(elab) annotating port %s(%u)",
                                ptr->getName().str().c_str(), ptr->getID());
                } 
            }
        }
    }

    void mergeSiblings(const Module<NS>& module) {
        const std::vector<std::unique_ptr<NetNexus>>& netNexuses =
                _netNexuses[module.getID()];
        std::unordered_set<const NetNexus*> visited;
        std::unordered_set<const IPortNexus*> pvisited;
        for (const auto& it : netNexuses) {
            const NetNexus* netNexus = it.get();
            if (visited.find(netNexus) != visited.end()) continue;
            NexusPtr link(new Nexus());
            std::vector<const NetNexus*> todo = { netNexus };
            std::vector<const Net<NS>*> nets;
            std::vector<const Port<NS>*> mports;
            util::Logger::debug("(elab) annotating create link");
            util::Logger::debug("(elab)   module: %s", module.getName().str().c_str());
            while (!todo.empty()) {
                netNexus = todo.back();
                todo.pop_back();
                Assert(visited.find(netNexus) == visited.end());
                visited.emplace(netNexus);
    
                const Net<NS>& net = netNexus->net;
                nets.emplace_back(&net);
                util::Logger::debug("(elab)     net: %s", net.getName().str().c_str());
                mergeNexus(*link, *netNexus->nexus);
    
                const typename Net<NS>::MPortVec& mportVec = net.getMPorts();
                for (auto it = mportVec.begin(); it != mportVec.end(); ++it) {
                    mports.emplace_back(*it);
                }
    
                for (auto portNexus : netNexus->ports) {
                    if (pvisited.find(portNexus) != pvisited.end()) {
                        continue;
                    }
                    pvisited.emplace(portNexus);
                    Assert(!portNexus->ports.empty());
                    for (auto iport : portNexus->ports) {
                        util::Logger::debug("(elab)     inst %s, port: %s",
                            iport->getMInst().getName().str().c_str(),
                            iport->getPort().getName().str().c_str());
                    }
                    mergeNexus(*link, *portNexus->nexus, *portNexus->ports.back());
                    for (const auto& n : portNexus->nets) {
                        if (visited.find(n) == visited.end()) {
                            todo.emplace_back(n);
                        }
                    }
                }
            }
            if (mports.empty()) {
                for (auto net : nets) {
                    _netAnno[net->getID()] = link;
                }
            } else {
                link->hasSibling = mports.size() > 1;
                for (auto port : mports) {
                    _portAnno[port->getID()] = link;
                }
            }
        }
    }

    void mergeNexus(Nexus& dst, Nexus& src, const IPort<NS>& port) {
        Assert(!dst.driver || !src.driver);
        const MInst<NS>& inst = port.getMInst();
        uint64_t offset = _db.getCellOffset(inst);
        if (src.driver) {
            dst.driver = offset + src.driver;
        }
        for (const auto& p : src.readers) {
            dst.readers.emplace_back(p.first + offset, p.second);
        }
    }

    void mergeNexus(Nexus& dst, Nexus& src) {
        Assert(!dst.driver || !src.driver);
        dst.driver = src.driver;
        for (const auto& p : src.readers) {
            dst.readers.emplace_back(p);
        }
    }

 public:
    void annotate(const std::vector<Module<NS>*>& topo) {
        for (auto module : topo) {
            const typename Module<NS>::NetHolder& nets = module->getNets();
            util::Logger::debug("(elab) annotating module %s(%u)",
                    module->getName().str().c_str(), module->getID());
            IPortNexusIndex portNexusIndex;
            for (auto it = nets.begin(); it != nets.end(); ++it) {
                annotateNet(**it, portNexusIndex);
            }
            mergeSiblings(*module);
        }
    }

 private:
    Annotation                          _portAnno;
    Annotation                          _netAnno;
    IPortNexusVec                       _portNexuses;
    NetNexusVec                         _netNexuses;
    const ElabDB<NS>&                   _db;
};

template<uint32_t NS>
class NetResolverCreator {
 public:
    NetResolverCreator() {}

    void createNetResolverForNet(Module<NS>& module, Net<NS>& net) {
        const typename Net<NS>::IPortHolder& iports = net.getIPorts();
        const typename Net<NS>::MPortVec& mports = net.getMPorts();
        const typename Net<NS>::PPortHolder& pports = net.getPPorts();
        std::unordered_set<size_t> diports;
        std::unordered_set<size_t> dmports;
        std::unordered_set<size_t> dpports;
        // 1. search for drivers
        size_t io  = 0;
        for (auto it = iports.begin(); it != iports.end(); ++it) {
            const IPort<NS>& port = **it;
            if (port.getPort().isOutput() || port.getPort().isInout()) {
                diports.emplace(it.index);
                if (port.getPort().isInout()) io++;
            }
        }
        for (auto it = mports.begin(); it != mports.end(); ++it) {
            const Port<NS>& port = **it;
            if (port.isInput() || port.isInout()) {
                dmports.emplace(it.index);
                if (port.isInout()) io++;
            }
        };
        for (auto it = pports.begin(); it != pports.end(); ++it) {
            const PPort<NS>& port = **it;
            if (port.getPort().isOutput() || port.getPort().isInout()) {
                dpports.emplace(it.index);
                if (port.getPort().isInout()) io++;
            }
        };

        // 2. do nothing if drive <= 1
        size_t drive = diports.size() + dmports.size() + dpports.size();
        if (drive <= 1) return;

        // 3. get multDrive processs
        Vid procName = Netlist<NS>::get().createNetResolver(drive - io, io, net.getDataType());
        Process<NS>& process = Netlist<NS>::get().getProcess(procName);

        // 4. create multDrive processs inst
        uint32_t id;
        PInst<NS>* pinst = new (id) PInst<NS>(id, Vid("NR").derive(), procName, module, process);
        util::Logger::debug("(elab) creating mult-drive instance %s(%u)",
                pinst->getName().str().c_str(), pinst->getID());
        module.addPInst(pinst);

        // 5. connect drivers to the input port of multiDrive instance
        size_t inputPortId = 0;
        size_t inoutPortId = 0;
        for (size_t index: diports) {
            const Port<NS>& port = iports[index]->getPort();
            Net<NS>* dnet = new (id) Net<NS>(id, net.getName().derive(), module, net.getDataType());
            net.transferIPort(index, *dnet);

            size_t portId = port.isOutput() ? inputPortId++ : inoutPortId++;
            typename Port<NS>::Direction dir = port.isOutput() ? Port<NS>::kPortInput : Port<NS>::kPortOutput;
            PPort<NS>* pport = new (id) PPort<NS>(id, *pinst, process.getPort(portId, dir));
            dnet->addPPort(pport);
            module.addNet(dnet);
        }

        for (size_t index: dmports) {
            const Port<NS>* port = mports[index];
            Net<NS>* dnet = new (id) Net<NS>(id, net.getName().derive(), module, net.getDataType());
            net.transferMPort(index, *dnet);

            size_t portId = port->isInput() ? inputPortId++ : inoutPortId++;
            typename Port<NS>::Direction dir = port->isInput() ? Port<NS>::kPortInput : Port<NS>::kPortOutput;
            PPort<NS>* pport = new (id) PPort<NS>(id, *pinst, process.getPort(portId, dir));
            dnet->addPPort(pport);
            module.addNet(dnet);
        }
        for (size_t index: dpports) {
            const Port<NS>& port = pports[index]->getPort();
            Net<NS>* dnet = new (id) Net<NS>(id, net.getName().derive(), module, net.getDataType());
            net.transferPPort(index, *dnet);

            size_t portId = port.isOutput() ? inputPortId++ : inoutPortId++;
            typename Port<NS>::Direction dir = port.isOutput() ? Port<NS>::kPortInput : Port<NS>::kPortOutput;
            PPort<NS>* pport = new (id) PPort<NS>(id, *pinst, process.getPort(portId, dir));
            dnet->addPPort(pport);
            module.addNet(dnet);
        }
        uint32_t id2;
        PPort<NS>* pport = new (id2) PPort<NS>(id2, *pinst, process.getPort(0, Port<NS>::kPortOutput));
        net.addPPort(pport);
    }

    void createNetResolver(const std::vector<Module<NS>*>& topo) {
        for (auto module : topo) {
            const typename Module<NS>::NetHolder& nets = module->getNets();
            util::Logger::debug("(elab) creating mult-drive if any for module %s(%u)",
                    module->getName().str().c_str(), module->getID());
            for (auto it = nets.begin(); it != nets.end(); ++it) {
                createNetResolverForNet(*module, **it);
            }
        }
    }
};

template<uint32_t NS>
class DanglingNetCreator {
 private:
    struct PairHash {
        size_t operator()(const std::pair<uint32_t, uint32_t>& p) const {
            static_assert(sizeof(std::pair<uint32_t, uint32_t>) == sizeof(size_t));
            union {
                std::pair<uint32_t, uint32_t> p;
                size_t s;
            } u {p};
            return std::hash<size_t>()(u.s);
        }
    };

 public:
    void createDanglingNet() {
        Module<NS>::foreach([](Module<NS>& module, size_t id) {
            std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> visitedIPorts;
            std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> visitedPPorts;
            const typename Module<NS>::NetHolder& nets = module.getNets();
            for (auto nit = nets.begin(); nit != nets.end(); ++nit) {
                const typename Net<NS>::IPortHolder& iports = nit->getIPorts();
                for (auto it = iports.begin(); it != iports.end(); ++it) {
                    visitedIPorts.emplace(it->getMInst().getID(),
                                          it->getPort().getID());
                }
                const typename Net<NS>::PPortHolder& pports = nit->getPPorts();
                for (auto it = pports.begin(); it != pports.end(); ++it) {
                    visitedPPorts.emplace(it->getPInst().getID(),
                                          it->getPort().getID());
                }
            }
            const typename Module<NS>::MInstHolder& minsts = module.getMInsts();
            for (auto mit = minsts.begin(); mit != minsts.end(); ++mit) {
                const Module<NS>& master = mit->getModule();
                const typename Module<NS>::PortHolder& ports = master.getPorts();
                for (auto it = ports.begin(); it != ports.end(); ++it) {
                    std::pair<uint32_t, uint32_t> key { mit->getID(), it->getID() };
                    if (visitedIPorts.find(key) == visitedIPorts.end()) {
                        uint32_t id1;
                        Net<NS>* nnet = new (id1) Net<NS>(id1, Vid("Z").derive(),
                                                module, it->getDataType());
                        IPort<NS>* iport = new (id1) IPort<NS>(id1, **mit, **it);
                        nnet->addIPort(iport);
                        module.addNet(nnet);
                    }
                }
            }
            const typename Module<NS>::PInstHolder& pinsts = module.getPInsts();
            for (auto pit = pinsts.begin(); pit != pinsts.end(); ++pit) {
                const Process<NS>& process = pit->getProcess();
                const typename Process<NS>::PortHolder& ports = process.getPorts();
                for(auto it = ports.begin(); it != ports.end(); ++it) {
                    std::pair<uint32_t, uint32_t> key { pit->getID(), it->getID() };
                    if (visitedPPorts.find(key) == visitedPPorts.end()) {
                        uint32_t id1;
                        Net<NS>* nnet = new (id1) Net<NS>(id1, Vid("Z").derive(),
                                                module, it->getDataType());
                        PPort<NS>* pport = new (id1) PPort<NS>(id1, **pit, **it);
                        nnet->addPPort(pport);
                        module.addNet(nnet);
                    }
                }
            }
        });
    }
};

template<uint32_t NS>
ElabDB<NS>::ElabDB() {}

template<uint32_t NS>
void ElabDB<NS>::elab() {
    // 1. topologic sort modules from bottom to top
    std::vector<Module<NS>*> topo;
    Netlist<NS>::get().bottomUp(topo);

    // 2. create nets for dangling port
    DanglingNetCreator<NS>().createDanglingNet();

    // 3. create multiple drive
    NetResolverCreator<NS>().createNetResolver(topo);

    // 4. calculate the weights
    genWeights(topo);

    // 5. generate index for DFS
    genIndex();

    // 6. annotate the design
    ElabAnnotator<NS> anno(*this);
    anno.annotate(topo);

    // 7. Allocate Vertex
    createVertex(anno);
}

template<uint32_t NS>
uint64_t ElabDB<NS>::getCellOffset(const MInst<NS>& inst) const {
    Assert(inst.getID() < _cellMInstOffset.size());
    return _cellMInstOffset[inst.getID()];
}

template<uint32_t NS>
uint64_t ElabDB<NS>::getCellOffset(const PInst<NS>& inst) const {
    Assert(inst.getID() < _cellPInstOffset.size());
    return _cellPInstOffset[inst.getID()];
}

template<uint32_t NS>
void ElabDB<NS>::resetWeights() {
    _dfs.clear();
    _dfsOffset.clear();
    _cellNum.clear();
    _cellMInstOffset.clear();
    _cellPInstOffset.clear();
    _dfs.resize(Module<NS>::Pool::getMaxSize());
    _dfsOffset.resize(MInst<NS>::Pool::getMaxSize());
    _cellNum.resize(Module<NS>::Pool::getMaxSize());
    _cellMInstOffset.resize(MInst<NS>::Pool::getMaxSize());
    _cellPInstOffset.resize(PInst<NS>::Pool::getMaxSize());
}

template<uint32_t NS>
void ElabDB<NS>::createVertex(const ElabAnnotator<NS>& a) {
    const Module<NS>& root = Netlist<NS>::get().getRoot();
    size_t toAlloc = _cellNum[root.getID()] + kReservedCell;
    Vertex<NS>::Pool::get().New(toAlloc);

    const typename ElabAnnotator<NS>::Annotation& anno = a.getAnnotation();
    std::deque<const Module<NS>*> todo = { &root };
    
    uint64_t dfs = 0;
    uint64_t addr = kReservedCell;
    while (!todo.empty()) {
        const Module<NS>& module = *todo.front();
        todo.pop_front();
        const typename Module<NS>::PInstHolder& pinsts = module.getPInsts();
        for (const auto& p : pinsts) {
            const PInst<NS>& inst = *p;
            const Process<NS>& process = inst.getProcess();
            uint64_t vaddr = addr + _cellPInstOffset[inst.getID()];
            size_t size = Cell<NS>::getNumCell(process.getNumOfInput() + process.getNumOfInout(),
                                               process.getNumOfOutput() + process.getNumOfInout());
            Vertex<NS>& vertex = Vertex<NS>::get(vaddr);
            vertex.init(dfs, process.getID(), inst.getName(), size);
        }

        const typename Module<NS>::MInstHolder& minsts = module.getMInsts();
        for (const auto& p : minsts) {
            const MInst<NS>& inst = *p;
            dfs += _dfsOffset[inst.getID()];
            addr += _cellMInstOffset[inst.getID()];
            todo.emplace_back(&inst.getModule());
        }
    }

    addr = kReservedCell;
    todo = { &root };
    while (!todo.empty()) {
        const Module<NS>& module = *todo.front();
        todo.pop_front();

        const typename Module<NS>::NetHolder& nets = module.getNets();
        for (const auto& p : nets) {
            const Net<NS>& net = *p;
            Assert(net.getID() <= anno.size());
            const typename ElabAnnotator<NS>::Nexus* nexus = anno[net.getID()].get();
            if (!nexus) continue;
            for (const auto& p : nexus->readers) {
                uint64_t vaddr = addr + p.first;
                Vertex<NS>& vertex = Vertex<NS>::get(vaddr);
                vertex.setDriver(nexus->driver, p.second);
            }
        }

        const typename Module<NS>::MInstHolder& minsts = module.getMInsts();
        for (const auto& p : minsts) {
            const MInst<NS>& inst = *p;
            addr += _cellMInstOffset[inst.getID()];
            todo.emplace_back(&inst.getModule());
        }
    }
}

template<uint32_t NS>
void ElabDB<NS>::genWeights(const std::vector<Module<NS>*>& topo) {
    resetWeights();
    for (auto mod : topo) {
        const typename Module<NS>::MInstHolder& insts = mod->getMInsts();
        const typename Module<NS>::PInstHolder& pinsts = mod->getPInsts();
        uint64_t dfs = 1;
        uint64_t cellNum = 0;
        for (auto it = pinsts.begin(); it != pinsts.end(); ++it) {
            const PInst<NS>& pinst = **it;
            const Process<NS>& process = pinst.getProcess();
            _cellPInstOffset[pinst.getID()] = cellNum;
            cellNum += Cell<NS>::getNumCell(process.getNumOfInput() + process.getNumOfInout(),
                                        process.getNumOfOutput() + process.getNumOfInout());
        };

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            const MInst<NS>& cinst = **it;
            const Module<NS>& cmod = cinst.getModule();
            _dfsOffset[cinst.getID()] = dfs;
            _cellMInstOffset[cinst.getID()] = cellNum;

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
    uint64_t maxDfs = _dfs[root.getID()];
    _index.resize(maxDfs);
    const typename Module<NS>::MInstHolder& insts = root.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, _dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::visitInst(const MInst<NS>& inst, uint64_t dfs) {
    _index[dfs] = inst.getID();
    const Module<NS>& module = inst.getModule();
    const typename Module<NS>::MInstHolder& insts = module.getMInsts();
    for (auto it = insts.begin(); it != insts.end(); ++it) {
        const MInst<NS>& cinst = **it;
        visitInst(cinst, dfs+_dfsOffset[cinst.getID()]);
    }
}

template<uint32_t NS>
void ElabDB<NS>::debugPrint() const {
    Assert(_dfs.size() == _cellNum.size());
    printf("%s", "------------- Module -----------------\n");
    for (size_t i = 0; i < _dfs.size(); ++i) {
        const Module<NS>& module = Module<NS>::Pool::get()[i];
        if (module) {
            printf("%s(%u) children %lu, cell %lu\n",
                    module.getName().str().c_str(),
                    module.getID(), _dfs[i], _cellNum[i]);
        }
    }

    Assert(_cellMInstOffset.size() == _dfsOffset.size());
    printf("%s", "-------------- MInst -----------------\n");
    for (size_t i = 0; i < _cellMInstOffset.size(); ++i) {
        const MInst<NS>& minst = MInst<NS>::Pool::get()[i];
        if (minst) {
            printf("%s(%u) module %s, dfs offset %lu, cell offset %lu\n",
                    minst.getName().str().c_str(),
                    minst.getID(),
                    minst.getModule().getName().str().c_str(),
                    _dfsOffset[i], _cellMInstOffset[i]);
        }
    }

    printf("%s", "-------------- PInst -----------------\n");
    for (size_t i = 0; i < _cellPInstOffset.size(); ++i) {
        const PInst<NS>& pinst = PInst<NS>::Pool::get()[i];
        if (pinst) {
            printf("%s(%u) process %s, cell offset %lu\n",
                    pinst.getName().str().c_str(),
                    pinst.getID(),
                    pinst.getProcessName().str().c_str(),
                    _cellPInstOffset[i]);
        }
    }
    /*printf("#0 %s(%s)\n", Vid(kVidSRoot).str().c_str(),
                          Vid(kVidSRoot).str().c_str());
    for (size_t i = 1; i < _index.size(); ++i) {
        Assert(i < MInst<NS>::Pool::getMaxSize());
        const MInst<NS>& minst = MInst<NS>::Pool::get()[_index[i]];
        Assert(minst);
        const Module<NS>& module = minst.getModule();
        printf("#%lu %s(%s)\n", i, minst.getName().str().c_str(),
                                   module.getName().str().c_str());
    }*/
}

template<uint32_t NS>
uint32_t ElabDB<NS>::getMInstID(uint64_t dfs) const {
    Assert(dfs < _index.size());
    return _index[dfs];
}

template<uint32_t NS>
uint64_t ElabDB<NS>::getDFSOffset(uint32_t instID) const {
    Assert(instID < _dfsOffset.size());
    return _dfsOffset[instID];
}

template<uint32_t NS>
uint64_t ElabDB<NS>::getTotalNumInst(uint32_t moduleID) const {
    Assert(moduleID < _dfs.size());
    return _dfs[moduleID];
}

template class ElabDB<NL_DEFAULT>;
}
