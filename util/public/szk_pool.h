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
enum GCPolicy {
    kNoGC = 0,
    kGC1,
    kGC2
};

template<class Pool>
class GC2Alloc {
 public:
    GC2Alloc() : _pool(Pool::get()), _cur(0),
              _avail(0), _free2(0), _free3(0), _ts(_pool.getTs()) {}
    void reset() {
        _cur = 0;
        _avail = 0;
        _free2 = 0;
        _free3 = 0;
    }

    uint64_t New(size_t num) {
        // check if the Alloc is out of date
        uint64_t ts = _pool.getTs();
        if (ts != _ts) {
            reset();
            _ts = ts;
        }
        uint64_t ret = _cur;
        // if free list is used and required number of elements is 1
        // use free list first
        if (num == 2) {
            if (_free2) { 
                typename Pool::Element& e = _pool[_free2];
                uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
                ret = _free2;
                _free2 = *pn;
                return ret;
            } else if (_free3) {
                typename Pool::Element& e = _pool[_free3];
                uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
                ret = _free3;
                _free3 = *pn;
                return ret;
            }
        } else if (num == 3 && _free3) {
            typename Pool::Element& e = _pool[_free3];
            uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
            ret = _free3;
            _free3 = *pn;
            return ret;
        }
        if (num <= _avail) {
            _cur += num;
            _avail -= num;
        } else {
            size_t blocks = (num + Pool::kLocalSize - 1) >> Pool::kLocalBits;
            Assert(blocks);
            ret = _pool.newBlock(blocks);
            if (ret == _cur + _avail) {
                ret = _cur;
            } else {
                _avail = 0;
            }
            _cur = ret + num;
            _avail = _avail + (blocks << Pool::kLocalBits) - num;
        }
        return ret;
    }

    void reclaim(uint64_t id, size_t size) {
        typename Pool::Element& e = _pool[id];
        Assert(!e);
        uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
        if (size == 2) {
            *pn = _free2;
            _free2 = id;
        } else if (size == 3) {
            *pn = _free3;
            _free3 = id;
        } else {
            Assert(0);
        }
    }

  private:
    Pool&                   _pool;
    uint64_t                _cur;
    uint64_t                _avail;
    uint64_t                _free2;
    uint64_t                _free3;
    uint64_t                _ts;
};

template<class Pool>
class GC1Alloc {
 public: 
    GC1Alloc() : _pool(Pool::get()), _cur(0),
              _avail(0), _free(0), _ts(_pool.getTs()) {}
    
    void reset() {
        _cur = 0;
        _avail = 0;
        _free = 0;
    }

    uint64_t New(size_t num) {
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
            typename Pool::Element& e = _pool[_free];
            uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
            ret = _free;
            _free = *pn;
            return ret;
        }
        if (num <= _avail) {
            _cur += num;
            _avail -= num;
        } else {
            size_t blocks = (num + Pool::kLocalSize - 1) >> Pool::kLocalBits;
            Assert(blocks);
            ret = _pool.newBlock(blocks);
            if (ret == _cur + _avail) {
                ret = _cur;
            } else {
                // if the available buffer is not large enough,
                // and the newly allocated space is contiguos, recycle the
                // current available buffer using free list.
                for (size_t i = 0; i < _avail; ++i) {
                    typename Pool::Element& e = _pool[_cur+i];
                    uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
                    *pn = _free;
                    _free = _cur + i;
                }
                _avail = 0;
            }
            _cur = ret + num;
            _avail = _avail + (blocks << Pool::kLocalBits) - num;
        }
        return ret;
    }

    void reclaim(uint64_t id, size_t size) {
        Assert(size == 1);
        typename Pool::Element& e = _pool[id];
        Assert(!e);
        uint64_t* pn = (uint64_t*)((char*)&e + Pool::Element::kNextOffset);
        *pn = _free;
        _free = id;
    }
 
  private:
    Pool&                   _pool;
    uint64_t                _cur;
    uint64_t                _avail;
    uint64_t                _free;
    uint64_t                _ts;
};

template<class Pool>
class GC0Alloc {
 public:
    GC0Alloc() : _pool(Pool::get()), _cur(0),
              _avail(0), _free(0), _ts(_pool.getTs()) {}
 
    void reset() {
        _cur = 0;
        _avail = 0;
        _free = 0;
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
            size_t blocks = (num + Pool::kLocalSize - 1) >> Pool::kLocalBits;
            Assert(blocks);
            ret = _pool.newBlock(blocks);
            if (ret == _cur + _avail) {
                ret = _cur;
            } else {
                _avail = 0;
            }
            _cur = ret + num;
            _avail = _avail + (blocks << Pool::kLocalBits) - num;
        }
        return ret;
    }
 
    void reclaim(uint64_t id, size_t size) {}

 private:
    Pool&                   _pool;
    uint64_t                _cur;
    uint64_t                _avail;
    uint64_t                _free;
    uint64_t                _ts;
};

template<class T, uint32_t Namespace, class Spec, GCPolicy GC=kNoGC>
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
    typedef typename std::conditional<GC==kNoGC, GC0Alloc<Pool>,
            typename std::conditional<GC==kGC1, GC1Alloc<Pool>, GC2Alloc<Pool>>::type>::type Alloc;
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

    void reclaim(uint64_t id, size_t size) {
        return tAlloc.reclaim(id, size);
    }

    static Pool& get() { return gSingleton; }
    static uint64_t getMaxSize() { return gSingleton._addr.load(std::memory_order_relaxed); }

    uint64_t getTs() const { return _ts.load(); }

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

 private:
    PArray<T*, kPageIDBits>     _pt;
    std::atomic<uint64_t>       _mapped;
    std::atomic<uint64_t>       _addr;
    std::atomic<uint64_t>       _ts;
    static Pool                 gSingleton;
    static bool                 gNoMore;
    static thread_local Alloc   tAlloc;
};

template<class T, uint32_t Namespace, class Spec, GCPolicy GC>
Pool<T, Namespace, Spec, GC> __attribute__((init_priority(200))) Pool<T, Namespace, Spec, GC>::gSingleton;
template<class T, uint32_t Namespace, class Spec, GCPolicy GC>
bool Pool<T, Namespace, Spec, GC>::gNoMore = false;
template<class T, uint32_t Namespace, class Spec, GCPolicy GC>
thread_local typename Pool<T, Namespace, Spec, GC>::Alloc Pool<T, Namespace, Spec, GC>::tAlloc;
}
