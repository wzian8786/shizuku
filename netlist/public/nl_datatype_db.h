#pragma once
#include <unordered_set>
#include <vector>
#include <memory>
#include "nl_datatype.h"
namespace netlist {
class DataTypeDB {
 public:
    DataTypeDB() : _scalar(nullptr),
                   _real(nullptr) {}

    // shortcut for primitive types
    const DataType& getTypeScalar();
    const DataType& getTypeReal();

    const DataType& persist(DataType* dt);

 private:
    typedef std::unordered_set<DataType*, DataType::Hash,
                               DataType::Equal> DataTypeSet;
    typedef std::vector<std::unique_ptr<DataType>> DataTypeVec;
    
    DataTypeVec                     _holder;
    DataTypeSet                     _dedup;

    // caches for primitve types
    const DataType*                 _scalar;
    const DataType*                 _real;
};
}
