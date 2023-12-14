#include "nl_top_db.h"
#include "nl_folded_obj.h"
#include "nl_vid_db.h"
namespace netlist {
template<uint32_t NS>
TopDB<NS>::TopDB() :
    _root(new Module<NS>(Vid(kVidSRoot))) {
    _root->setRoot();
}

template<uint32_t NS>
void TopDB<NS>::addTop(Module<NS>* top) {
}

template class TopDB<NL_DEFAULT>;
}
