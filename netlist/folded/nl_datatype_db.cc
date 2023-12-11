#include "nl_datatype_db.h"
namespace netlist {
DataTypeDB DataTypeDB::gSingleton;
const DataType* DataTypeDB::getScalar2S() {
    if (!_s2s) {
        Scalar2S s;
        _s2s = persist(&s);
    }
    return _s2s;
}

const DataType* DataTypeDB::getScalar4S() {
    if (!_s4s) {
        Scalar4S s;
        _s4s = persist(&s);
    }
    return _s4s;
}

const DataType* DataTypeDB::getScalarStr() {
    if (!_sstr) {
        ScalarStr s;
        _sstr = persist(&s);
    }
    return _sstr;
}

const DataType* DataTypeDB::getReal() {
    if (!_real) {
        Real s;
        _real = persist(&s);
    }
    return _real;
}

const DataType* DataTypeDB::persist(DataType* dt) {
    auto it = _dedup.find(dt);
    if (it != _dedup.end()) {
        return *it;
    }

    DataType* toinsert = nullptr;
    switch (dt->getClassID()) {
    case kDtScalar2S:
        toinsert = new Scalar2S();
        break;
    case kDtScalar4S:
        toinsert = new Scalar4S();
        break;
    case kDtScalarStr:
        toinsert = new ScalarStr();
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

    return toinsert;
}
}
