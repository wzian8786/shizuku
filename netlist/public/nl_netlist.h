#pragma once
#include "nl_pool_db.h"
#include "nl_datatype_db.h"
namespace netlist {
template<uint32_t Namespace>
class Netlist : public PoolDB<Namespace>,
                public DataTypeDB {
 public:
    static Netlist& get() { return gSingleton; }

 private:
    static Netlist gSingleton;
};
}
