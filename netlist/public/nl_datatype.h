#pragma once
#include <cstdlib>
#include "szk_assert.h"
#include "nl_def.h"
namespace netlist {
enum DataTypeID {
    kDtInvalidID = 0, 
    kDtScalar,
    kDtReal,
    kDtPackedArray,
    kDtUnpackedArray,
};

class DataType {
 public:
    DataType() {}
    virtual ~DataType() {}

    virtual DataTypeID getClassID() const = 0;

    struct Hash {
        size_t operator ()(const DataType* a) const {
            return a->hash();
        }
    };

    struct Equal {
        bool operator ()(const DataType* a, const DataType* b) const;
    };

    virtual size_t hash() const = 0;

    virtual bool isPrimitive() const { return false; }

    virtual bool is2Status()   const { return false; }
    virtual bool is4Status()   const { return false; }
    virtual bool isStrength()  const { return false; }

    virtual bool isReal()      const { return false; }
    virtual bool isInteger()   const { return false; }
    virtual bool isSigned()    const { return false; }

    virtual bool isIndexable() const { return false; }
    virtual bool isPacked()    const { return false; }
    virtual bool isUnpacked()  const { return false; }

    virtual bool isArray()     const { return false; }
    virtual bool isStruct()    const { return false; }
    virtual bool isUnion()     const { return false; }
    virtual bool isInterface() const { return false; }

    virtual int getMSB()    const { Assert(0); return 0; }
    virtual int getLSB()    const { Assert(0); return 0; }
    virtual size_t width()  const { Assert(0); return 0; }

    template<typename T>
    const T* cast() const {
        Assert(T::kDataType == getClassID());
        return static_cast<const T*>(this);
    }

    template<typename T>
    T* cast() {
        Assert(T::kDataType == getClassID());
        return static_cast<T*>(this);
    }

    DataType* getMember(Vid vid)    const { Assert(0); return nullptr; }
    DataType* getIndex(long index)  const { Assert(0); return nullptr; }
};

class Scalar final : public DataType {
 public:
    constexpr static DataTypeID kDataType = kDtScalar;
    DataTypeID getClassID() const final { return kDtScalar; }

    size_t hash()       const final { return (size_t)kDataType; }

    bool isPrimitive()  const final { return true; }
    bool isInteger()    const final { return true; }
    bool is2Status()    const final { return true; }
    size_t width()      const final { return 1; }
};

class Real final : public DataType {
 public:
    constexpr static DataTypeID kDataType = kDtReal;
    DataTypeID getClassID() const final { return kDtReal; }

    size_t hash()       const final { return (size_t)kDataType; }

    bool isPrimitive()  const final { return true; }
    bool isReal()       const final { return true; }
};

class PackedArray final : public DataType {
 public:
    PackedArray(int msb, int lsb, bool sign);

    constexpr static DataTypeID kDataType = kDtPackedArray;
    DataTypeID getClassID() const final { return kDtPackedArray; }

    size_t hash()       const final {
        return std::hash<size_t>()((((size_t)kDataType) << 32) + (size_t)_signed) ^
               std::hash<size_t>()((((size_t)_msb) << 32) + (size_t)_lsb);
    }

    bool operator ==(const PackedArray& a) const {
        return _msb == a._msb && _lsb == a._lsb && _signed == a._signed;
    }

    bool isInteger()    const final { return true; } 
    bool isIndexable()  const final { return true; }
    bool isPacked()     const final { return true; }
    bool isArray()      const final { return true; }
    bool isSigned()     const final { return _signed; }
    int  getMSB()       const final { return _msb; }
    int  getLSB()       const final { return _lsb; }
    size_t width()      const final { return abs(_msb - _lsb) + 1; }

 private:
    int                     _msb;
    int                     _lsb;
    bool                    _signed;
};

class UnpackedArray final : public DataType {
 public:
    UnpackedArray(int msb, int lsb, bool sign, const DataType* child);

    constexpr static DataTypeID kDataType = kDtUnpackedArray;
    DataTypeID getClassID() const final { return kDtUnpackedArray; }

    size_t hash()       const final {
        return std::hash<size_t>()((((size_t)kDataType) << 32) + (size_t)_signed) ^
               std::hash<size_t>()((((size_t)_msb) << 32) + (size_t)_lsb);
    }

    bool operator ==(const UnpackedArray& a) const {
        return _msb == a._msb && _lsb == a._lsb &&
               _signed == a._signed && _child == a._child;
    }

    bool isIndexable()  const final { return true; }
    bool isArray()      const final { return true; }
    bool isSigned()     const final { return _signed; }
    int  getMSB()       const final { return _msb; }
    int  getLSB()       const final { return _lsb; }
    size_t width()      const final { return abs(_msb - _lsb) + 1; }

 private:
    int                     _msb;
    int                     _lsb;
    bool                    _signed;
    const DataType*         _child;
};
}
