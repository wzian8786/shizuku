#pragma once
#include <unordered_map>
#include <atomic>
#include "nl_def.h"
#include "szk_pool.h"
#include "vid.h"
namespace netlist {
template<uint32_t NS>
class PoolDB {
 public:
    PoolDB() : _deletedModule(0),
               _deletedProcess(0),
               _deletedPort(0),
               _deletedNet(0),
               _deletedMInst(0),
               _deletedPInst(0),
               _deletedIPort(0),
               _deletedPPort(0) {}
    ~PoolDB();

    Module<NS>* createModule(Vid name);
    bool hasModule(Vid name) const;
    Module<NS>& getModule(Vid name);
    const Module<NS>& getModule(Vid name) const;

    Process<NS>* createProcess(Vid name);
    bool hasProcess(Vid name) const;
    Process<NS>& getProcess(Vid name);
    const Process<NS>& getProcess(Vid name) const;

    Vid createNetResolver(size_t input, size_t io, const DataType& dt);

    void clear();
    void gc();
    void print(FILE* fp, bool indent) const;

 private:
    template<typename T>
    void reclaim();

 private:
    std::unordered_map<Vid, uint32_t, Vid::Hash>    _processIndex;
    std::unordered_map<Vid, uint32_t, Vid::Hash>    _moduleIndex;
    std::atomic<size_t>                             _deletedModule;
    std::atomic<size_t>                             _deletedProcess;
    std::atomic<size_t>                             _deletedPort;
    std::atomic<size_t>                             _deletedNet;
    std::atomic<size_t>                             _deletedMInst;
    std::atomic<size_t>                             _deletedPInst;
    std::atomic<size_t>                             _deletedIPort;
    std::atomic<size_t>                             _deletedPPort;
};
}
