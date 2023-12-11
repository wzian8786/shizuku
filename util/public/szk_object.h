#pragma once
#include <cstdint>
#include <type_traits>
#include <cstddef>
namespace util {

constexpr static size_t kRttiBits = 8;

template<size_t CapBits, bool Rtti=false>
class Object {};

template<size_t CapBits>
class Object<CapBits, false> {
 public:
    typedef typename std::conditional<CapBits <= 32, uint32_t, uint64_t>::type RawType;
    static_assert(CapBits <= 64, "CapBits too large.");

 public:
    bool operator !() const { return !_addr; }
    operator bool() const { return _addr; }
    RawType operator *() const { return _addr; }
    bool operator <(Object rhs) const { return _addr < rhs._addr; }
    bool operator ==(Object rhs) const { return _addr == rhs._addr; }
    bool operator !=(Object rhs) const { return _addr != rhs._addr; }
    void operator =(Object rhs) { _addr = rhs._addr; }
    bool valid() const { return _addr; }

    Object() : _addr(0), _pad(0) {}
    explicit Object(RawType a) : _addr(a), _pad(0) {}
    Object(const Object& rhs) : _addr(rhs._addr), _pad(rhs._pad) {}
    RawType getAddr() const { return _addr; }

 private:
    RawType                     _addr : CapBits;
    RawType                     _pad : (sizeof(RawType) << 3) - CapBits;
};

template<>
class Object<32, false> {
 public:
    constexpr static size_t kBits = 32;
    typedef uint32_t RawType;

 public:
    bool operator !() const { return !_addr; }
    operator bool() const { return _addr; }
    uint32_t operator *() const { return _addr; }
    bool operator <(Object rhs) const { return _addr < rhs._addr; }
    bool operator ==(Object rhs) const { return _addr == rhs._addr; }
    bool operator !=(Object rhs) const { return _addr != rhs._addr; }
    void operator =(Object rhs) { _addr = rhs._addr; }
    bool valid() const { return _addr; }

    Object() : _addr(0) {} 
    explicit Object(uint32_t a) : _addr(a) {}
    Object(const Object& rhs) : _addr(rhs._addr) {}
    uint32_t getAddr() const { return _addr; }

 private:
    uint32_t                    _addr;
};

template<>
class Object<64, false> {
 public:
    constexpr static size_t kBits = 64;
    typedef uint64_t RawType;

 public:
    bool operator !() const { return !_addr; }
    operator bool() const { return _addr; }
    uint64_t operator *() const { return _addr; }
    bool operator <(Object rhs) const { return _addr < rhs._addr; }
    bool operator ==(Object rhs) const { return _addr == rhs._addr; }
    bool operator !=(Object rhs) const { return _addr != rhs._addr; }
    void operator =(Object rhs) { _addr = rhs._addr; }
    bool valid() const { return _addr; }

    Object() : _addr(0) {} 
    explicit Object(uint64_t a) : _addr(a) {}
    Object(const Object& rhs) : _addr(rhs._addr) {}
    uint64_t getAddr() const { return _addr; }

 private:
    uint64_t                    _addr;
};

template<size_t CapBits>
class Object<CapBits, true> {
 public:
    constexpr static size_t kBits = CapBits + kRttiBits;
    typedef typename std::conditional<kBits <= 32, uint32_t, uint64_t>::type RawType;
    static_assert(CapBits <= 56, "CapBits too large.");

 public:
    bool operator !() const { return !_id.id; }
    operator bool() const { return _id.id; }
    RawType operator *() const { return _id.id; }
    bool operator <(Object rhs) const { return _id.id < rhs._id.id; }
    bool operator ==(Object rhs) const { return _id.id == rhs._id.id; }
    bool operator !=(Object rhs) const { return _id.id != rhs._id.id; }
    void operator =(Object rhs) { _id.id = rhs._id.id; }
    bool valid() const { return _id.id; }

    Object() : _id{0} {}
    explicit Object(RawType v) : _id{v} {}
    Object(RawType r, RawType a) : _id{.s=S{a, 0, r}} {}
    Object(const Object& rhs) : _id{rhs._id.id} {}
    RawType getAddr() const { return _id.s.addr; }
    RawType getRtti() const { return _id.s.rtti; }
    
 private:
    typedef struct {
        RawType addr : CapBits;
        RawType pad  : (sizeof(RawType) << 3) - CapBits - kRttiBits;
        RawType rtti : kRttiBits;
    } S;
    typedef union {
        RawType id;
        S       s;
    } U;

    U                           _id;
};

template<>
class Object<24, true> {
 public:
    constexpr static size_t kBits = 32;
    typedef uint32_t RawType;

 public:
    bool operator !() const { return !_id.id; }
    operator bool() const { return _id.id; }
    uint32_t operator *() const { return _id.id; }
    bool operator <(Object rhs) const { return _id.id < rhs._id.id; }
    bool operator ==(Object rhs) const { return _id.id == rhs._id.id; }
    bool operator !=(Object rhs) const { return _id.id != rhs._id.id; }
    void operator =(Object rhs) { _id.id = rhs._id.id; }
    bool valid() const { return _id.id; }

    Object() : _id{0} {}
    explicit Object(uint32_t v) : _id{v} {}
    Object(uint32_t r, uint32_t a) : _id{.s=S{a, r}} {}
    Object(const Object& rhs) : _id{rhs._id.id} {}
    uint32_t getAddr() const { return _id.s.addr; }
    uint32_t getRtti() const { return _id.s.rtti; }
    
 private:
    typedef struct {
        uint32_t addr : 24;
        uint32_t rtti : 8;
    } S;
    typedef union {
        uint32_t id;
        S       s;
    } U;

    U                           _id;
};

template<>
class Object<56, true> {
 public:
    constexpr static size_t kBits = 64;
    typedef uint64_t RawType;

 public:
    bool operator !() const { return !_id.id; }
    operator bool() const { return _id.id; }
    uint64_t operator *() const { return _id.id; }
    bool operator <(Object rhs) const { return _id.id < rhs._id.id; }
    bool operator ==(Object rhs) const { return _id.id == rhs._id.id; }
    bool operator !=(Object rhs) const { return _id.id != rhs._id.id; }
    void operator =(Object rhs) { _id.id = rhs._id.id; }
    bool valid() const { return _id.id; }

    Object() : _id{0} {}
    explicit Object(uint64_t v) : _id{v} {}
    Object(uint64_t r, uint64_t a) : _id{.s=S{a, r}} {}
    Object(const Object& rhs) : _id{rhs._id.id} {}
    uint64_t getAddr() const { return _id.s.addr; }
    uint64_t getRtti() const { return _id.s.rtti; }
    
 private:
    typedef struct {
        uint64_t addr : 56;
        uint64_t rtti : 8;
    } S;
    typedef union {
        uint64_t id;
        S       s;
    } U;

    U                           _id;
};
}
