#include "nl_datatype.h"
namespace netlist {
bool DataType::Equal::operator ()(const DataType* a, const DataType* b) const {
    DataTypeID id = a->getClassID();
    if (id == b->getClassID()) {
        switch (id) {
        case kDtScalar2S:
        case kDtScalar4S:
        case kDtScalarStr:
        case kDtReal:
            return true;
        case kDtPackedArray:
            return *a->cast<PackedArray>() == *b->cast<PackedArray>();
        case kDtUnpackedArray:
            return *a->cast<UnpackedArray>() == *b->cast<UnpackedArray>();
        default:
            Assert(0);
        }
    }
    return false;
}

PackedArray::PackedArray(int msb, int lsb, bool sign) :
        _msb(msb),
        _lsb(lsb),
        _signed(sign) {}

UnpackedArray::UnpackedArray(int msb, int lsb,
                             bool sign, const DataType* child) :
        _msb(msb),
        _lsb(lsb),
        _signed(sign),
        _child(child) {}

}
