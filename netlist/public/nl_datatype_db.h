#pragma once
#include <unordered_set>
#include <vector>
#include <memory>
#include "nl_datatype.h"
namespace netlist {
class DataTypeDB {
 public:
    DataTypeDB() : _s2s(nullptr),
                   _s4s(nullptr),
                   _sstr(nullptr),
                   _real(nullptr) {}

    static DataTypeDB& get() { return gSingleton; }

    // shortcut for primitive types
    const DataType* getScalar2S();
    const DataType* getScalar4S();
    const DataType* getScalarStr();
    const DataType* getReal();

    const DataType* persist(DataType* dt);

 private:
    static DataTypeDB gSingleton; 

    typedef std::unordered_set<DataType*, DataType::Hash,
                               DataType::Equal> DataTypeSet;
    typedef std::vector<std::unique_ptr<DataType>> DataTypeVec;
    
    DataTypeVec                     _holder;
    DataTypeSet                     _dedup;

    // caches for primitve types
    const DataType*                 _s2s; 
    const DataType*                 _s4s;
    const DataType*                 _sstr;
    const DataType*                 _real;
};
}
