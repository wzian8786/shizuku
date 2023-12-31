#pragma once
#include <unordered_map>
#include "nl_def.h"
#include "szk_pool.h"
#include "vid.h"
namespace netlist {
template<uint32_t NS>
class PoolDB {
 public:
    PoolDB() {}
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
};
}
