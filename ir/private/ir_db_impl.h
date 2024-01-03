#pragma once
#include "ir_pool_db.h"
#include "ir_datatype_db.h"
#include "ir_vid_db.h"
#include "ir_top_db.h"
#include "ir_elab_db.h"
namespace ir {
template<uint32_t NS>
class IRDBImpl: public PreVidDB,
                public PoolDB<NS>,
                public TopDB<NS>,
                public ElabDB<NS>,
                public DataTypeDB {
 public:
    static IRDBImpl& get() { return gSingleton; }

    void topDown(std::vector<Module<NS>*>& modules);
    void bottomUp(std::vector<Module<NS>*>& modules);

 private:
    static IRDBImpl gSingleton;
};
}
