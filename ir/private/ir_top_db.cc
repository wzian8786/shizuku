#include "ir_top_db.h"
#include "ir_folded_obj.h"
#include "ir_vid_db.h"
namespace ir {
template<uint32_t NS>
TopDB<NS>::TopDB() {
    uint32_t id;
    _root = new (id) Module<NS>(id, Vid(kVidSRoot));
    _root->setRoot();
}

template class TopDB<IR_DEFAULT>;
}
