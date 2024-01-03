#include "ir_db.h"
#include "ir_db_impl.h"
namespace ir {
template<uint32_t NS>
void IRDB<NS>::elab() {
    IRDBImpl<NS>::get().elab();
}

template<uint32_t NS>
void IRDB<NS>::printFolded(FILE* fp, bool indent) {
    IRDBImpl<NS>::get().print(fp, indent);
}

template class IRDB<IR_DEFAULT>;
}
