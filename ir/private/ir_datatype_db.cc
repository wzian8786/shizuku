#include "ir_datatype_db.h"
namespace ir {
const DataType& DataTypeDB::getTypeScalar() {
    if (!_scalar) {
        Scalar s;
        _scalar = &persist(&s);
    }
    return *_scalar;
}

const DataType& DataTypeDB::getTypeReal() {
    if (!_real) {
        Real s;
        _real = &persist(&s);
    }
    return *_real;
}

const DataType& DataTypeDB::persist(DataType* dt) {
    auto it = _dedup.find(dt);
    if (it != _dedup.end()) {
        return **it;
    }

    DataType* toinsert = nullptr;
    switch (dt->getClassID()) {
    case kDtScalar:
        toinsert = new Scalar();
        break;
    case kDtReal:
        toinsert = new Real();
        break;
    case kDtPackedArray:
        toinsert = new PackedArray(*dt->cast<PackedArray>());
        break;
    case kDtUnpackedArray:
        toinsert = new UnpackedArray(*dt->cast<UnpackedArray>());
        break;
    default:
        Assert(0);
    }
        
    _holder.emplace_back(toinsert);
    _dedup.insert(toinsert); 

    return *toinsert;
}
}
