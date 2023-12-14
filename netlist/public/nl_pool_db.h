#pragma once
#include "nl_def.h"
#include "szk_pool.h"
namespace netlist {
template <uint32_t NS>
class PoolDB {
 public:
    PoolDB() {}
    ~PoolDB();

    void clear();
};
}
