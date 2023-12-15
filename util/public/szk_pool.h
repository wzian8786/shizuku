#pragma once
#include <sys/mman.h>
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <memory>
#include "szk_assert.h"
#include "szk_parray.h"
template <typename P>
class JPoolTester;
namespace util {
template<class T, uint32_t Namespace, class Spec>
class Pool {
 public:
    constexpr static uint32_t kLocalBits  = Spec::kLocalBits;
    static_assert(kLocalBits, "Spec::kLocalBits can't be 0");
    constexpr static uint64_t kLocalSize  = 1ul << kLocalBits;

    constexpr static uint32_t kPageBits   = Spec::kLocalBits + Spec::kBlockBits;
    static_assert(kPageBits < 24, "Create huge page is not allowed");
    constexpr static uint64_t kPageSize   = 1ul << kPageBits;
    constexpr static uint64_t kPageMask   = (kPageSize - 1);

    constexpr static uint32_t kPageIDBits = Spec::kPageIDBits;
    constexpr static uint64_t kMaxPageNum = 1ul << kPageIDBits;

    typedef T  Element;
    typedef T* PagePtr;
    friend ::JPoolTester<Pool>;

    Pool() : _pt(kMaxPageNum), _mapped(((uint64_t)-1) & (~kPageMask)),
             _addr(0), _ts(0) {
        Assert(!gNoMore);
        gNoMore = true;
    }

    ~Pool() {
        clear();
    }

    void clear() {
        uint64_t pages = (_addr.load(std::memory_order_relaxed) + kPageMask) >> kPageBits;
        for (size_t i = 0; i < pages; ++i) {
            munmap(_pt[i], sizeof(T) << kPageBits);
            _pt[i] = nullptr;
        }
        _mapped.store(((uint64_t)-1) & (~kPageMask), std::memory_order_relaxed);
        _addr.store(0, std::memory_order_relaxed);
        _ts += 1;
    }

    T& operator[](uint64_t id) {
        Assert(id < _addr.load(std::memory_order_relaxed));
        return _pt[(id & (~kPageMask)) >> kPageBits][id & kPageMask];
    }

    const T& operator[](uint64_t id) const {
        Assert(id < _addr.load(std::memory_order_relaxed));
        return _pt[(id & (~kPageMask)) >> kPageBits][id & kPageMask];
    }

    uint64_t New(size_t num=1) {
        return tAlloc.New(num);
    }

    uint64_t RNew(size_t num=1) {
        return tAlloc.RNew(num);
    }

    static Pool& get() { return gSingleton; }
    uint64_t getMaxSize() { return _addr.load(std::memory_order_relaxed); }

 private:
    uint64_t newBlock(size_t blocks) {
        if (!blocks) return _addr.load(std::memory_order_relaxed);

        // get the staring address
        uint64_t addr = _addr.fetch_add(blocks << kLocalBits);

        // calulate pages need to be allocated
        uint64_t pages = (((addr - 1) & kPageMask) + (blocks << kLocalBits)) >> kPageBits;

        if (pages) {
            uint64_t prev = ((addr - 1) & (~kPageMask));

            // waiting previous page address is updated to _mapped
            while (_mapped.load(std::memory_order_acquire) != prev) {}

            for (size_t i = 0; i < pages; ++i) {
                size_t index = (prev + (i + 1) * kPageSize) >> kPageBits;
                Assert(index < _pt.size());
                Assert(!_pt[index]);
                T* p = (T*)mmap(nullptr, sizeof(T) << kPageBits, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                Assert(p != MAP_FAILED);
                _pt[index] = p;
            }
            _mapped.store(prev + pages * kPageSize, std::memory_order_release);
        } else {
            int64_t cur = addr & (~kPageMask);

            // make sure the current page is available
            while ((int64_t)_mapped.load(std::memory_order_acquire) < cur) {}
        }
        return addr;
    }

    uint64_t getTs() const { return _ts.load(); }

    class Alloc {
     public:
        Alloc() : _pool(gSingleton), _cur(0),
                  _avail(0), _free(0), _ts(gSingleton.getTs()) {}

        void reset() {
            _cur = 0;
            _avail = 0;
            _free = 0;
        }

        uint64_t RNew(size_t num) {
            // check if the Alloc is out of date
            uint64_t ts = _pool.getTs();
            if (ts != _ts) {
                reset();
                _ts = ts;
            }
            uint64_t ret = _cur;
            // if free list is used and required number of elements is 1
            // use free list first
            if (num == 1 && _free) {
                Element& e = _pool[_free];
                uint64_t* pn = (uint64_t*)((char*)&e + Element::kNextOffset);
                ret = _free;
                _free = *pn;
                return ret;
            }
            if (num <= _avail) {
                _cur += num;
                _avail -= num;
            } else {
                size_t blocks = (num + kLocalSize - 1) >> kLocalBits;
                Assert(blocks);
                ret = _pool.newBlock(blocks);
                if (ret == _cur + _avail) {
                    ret = _cur;
                } else {
                    // if the available buffer is not large enough,
                    // and the newly allocated space is contiguos, recycle the
                    // current available buffer using free list.
                    for (size_t i = 0; i < _avail; ++i) {
                        Element& e = _pool[_cur+i];
                        uint64_t* pn = (uint64_t*)((char*)&e + Element::kNextOffset);
                        *pn = _free;
                        _free = _cur + i;
                    }
                    _avail = 0;
                }
                _cur = ret + num;
                _avail = _avail + (blocks << kLocalBits) - num;
            }
            return ret;
        }

        uint64_t New(size_t num) {
            // check if the Alloc is out of date
            uint64_t ts = _pool.getTs();
            if (ts != _ts) {
                reset();
                _ts = ts;
            }
            uint64_t ret = _cur;
            if (num <= _avail) {
                _cur += num;
                _avail -= num;
            } else {
                size_t blocks = (num + kLocalSize - 1) >> kLocalBits;
                Assert(blocks);
                ret = _pool.newBlock(blocks);
                if (ret == _cur + _avail) {
                    ret = _cur;
                } else {
                    _avail = 0;
                }
                _cur = ret + num;
                _avail = _avail + (blocks << kLocalBits) - num;
            }
            return ret;
        }

     private:
        Pool&                   _pool;
        uint64_t                _cur;
        uint64_t                _avail;
        uint64_t                _free;
        uint64_t                _ts;
    };

 private:
    PArray<T*, kPageIDBits>     _pt;
    std::atomic<uint64_t>       _mapped;
    std::atomic<uint64_t>       _addr;
    std::atomic<uint64_t>       _ts;
    static Pool                 gSingleton;
    static bool                 gNoMore;
    static thread_local Alloc   tAlloc;
};

template<class T, uint32_t Namespace, class Spec>
Pool<T, Namespace, Spec> __attribute__((init_priority(200))) Pool<T, Namespace, Spec>::gSingleton;
template<class T, uint32_t Namespace, class Spec>
bool Pool<T, Namespace, Spec>::gNoMore = false;
template<class T, uint32_t Namespace, class Spec>
thread_local typename Pool<T, Namespace, Spec>::Alloc Pool<T, Namespace, Spec>::tAlloc;
}
