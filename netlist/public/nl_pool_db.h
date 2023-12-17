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

    void clear();

 private:
    std::unordered_map<Vid, uint32_t, Vid::Hash>    _processIndex;
    std::unordered_map<Vid, uint32_t, Vid::Hash>    _moduleIndex;
};
}
