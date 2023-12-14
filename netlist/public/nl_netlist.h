#pragma once
#include "nl_pool_db.h"
#include "nl_datatype_db.h"
#include "nl_vid_db.h"
#include "nl_top_db.h"
namespace netlist {
template<uint32_t NS>
class Netlist : public PreVidDB,
                public PoolDB<NS>,
                public TopDB<NS>,
                public DataTypeDB {
 public:
    static Netlist& get() { return gSingleton; }
    void print(FILE* fp, bool indent) const;

 private:
    static Netlist gSingleton;
};
}
