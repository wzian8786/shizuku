#pragma once
#include <sys/mman.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include "szk_assert.h"
namespace util {
// PArray (Persist Array) is an array with persist virtual memory 
// address.
//
// The virtual memory pages are allocated when the PArray is creatoed,
// but only the used pages are mapped to physical memory. So PArray
// introduce no significant memory penalty.  
//
// The purpose of PArray is completely eliminating the base point 
// reallocation. The reasons are listed below:
// 1. Reallocation causes points & references to the array element 
//    became invalid. 
// 2. Index can be used as alternative of point & reference to fix
//    the previous issue, but still it has limitation.
//    If one thread is resizing the array while other threads are 
//    reading from it, there is race condition that the reading threads 
//    get old base address, and the points & references derived from
//    this old base address are still invalid.
template<typename T, uint32_t Bits>
class PArray {
 public:
    static constexpr uint64_t kPageBits = 12;
    static constexpr uint64_t kPageSize = 1ul << kPageBits;
    static constexpr uint64_t kPageMask = kPageSize - 1;
    static constexpr uint32_t kBits = Bits;
    static_assert(Bits >= kPageBits, "Bits must larger than 12");
    // so the virtual memory size is not too huge
    static_assert(Bits <= 32, "Create huge PArray is not allowed.");

    struct iterator {
        iterator(PArray& a, uint64_t i) : index(i), array(a) {}

        T& operator*() const { return array[index]; }
        uint64_t operator++() { return ++index; }
        bool operator!=(const iterator& rhs) {
            return (index != rhs.index) ||
                   (&array != &rhs.array);
        }
        
        uint64_t    index;
        PArray&    array;
    };

    struct const_iterator {
        const_iterator(const PArray& a, uint64_t i) : index(i), array(a) {}

        const T& operator*() const { return array[index]; }
        uint64_t operator++() { return ++index; }
        bool operator!=(const const_iterator& rhs) {
            return (index != rhs.index) ||
                   (&array != &rhs.array);
        }
        
        uint64_t        index;
        const PArray&  array;
    };

    PArray() : _size(0),
                _base((T*)mmap(nullptr, sizeof(T) << Bits, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) {
        Assert(_base != MAP_FAILED);
        madvise(_base, sizeof(T) << kBits, MADV_DONTNEED);
    }

    explicit PArray(uint64_t size) : _size(size),
                _base((T*)mmap(nullptr, sizeof(T) << Bits, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) {
        Assert(_base != MAP_FAILED);
        madvise(_base, sizeof(T) << Bits, MADV_DONTNEED);
    }

    ~PArray() {
        for (size_t i = 0; i < _size; ++i) {
            _base[i].~T();
        }
        munmap(_base, sizeof(T) << Bits);
    }

    void push_back(const T& v) {
        _base[_size++] = v;
        Assert(_size <= (1ull << Bits));
    }

    void push_back(T&& v) {
        _base[_size++] = std::forward<T>(v);
        Assert(_size <= (1ull << Bits));
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        push_back(T(std::forward<Args>(args)...));
    }

    void resize(uint64_t size) {
        for (size_t i = _size; i < size; ++i) {
            _base[i] = T();
        }
        for (size_t i = size; i < _size; ++i) {
            _base[i].~T();
        }
        _size = size;
        Assert(_size <= (1ull << Bits));
    }

    void clear() {
        resize(0);
    }

    T& operator[](uint64_t index) {
        Assert(index < _size);
        return _base[index];
    }

    const T& operator[](uint64_t index) const {
        Assert(index < _size);
        return _base[index];
    }

    size_t size() const { return _size; }
    size_t cap() const { return (_size + kPageBits) & (~kPageBits); }

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, _size); }

    const_iterator begin() const { return iterator(*this, 0); }
    const_iterator end() const { return iterator(*this, _size); }

 private:
    uint64_t                _size;
    T* const                _base;
};
}
