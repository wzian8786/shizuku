#pragma once
#include <cstdint>
#include <atomic>
#include "szk_assert.h"
namespace util {
// Wsqueue (Work stealing queue) is a lock-free implementation of 
// work-stealing queue.
// The queue has 2 points, which point to the top & bottom of contiguos
// range. The work stealing is actually updating the bottom of this
// rnage.
class Wsqueue {
 public:
    static constexpr uint64_t kReservedTasks = 4;
    Wsqueue() : _pt(0) {}

    Wsqueue(const Wsqueue& q) : _pt(q._pt.load()) {}

    void reset(uint32_t top, uint32_t bottom) {
        _pt.store(getPt(top, bottom));
        _victim.clear();
    }

    void addVictim(Wsqueue* v) {
        _victim.emplace_back(v);
    }

    uint32_t pop() {
        uint64_t pt = _pt.load(std::memory_order_relaxed);
        uint32_t top = getTop(pt);
        if (!top) return 0;

        uint32_t bottom = getBottom(pt);
        if (top < bottom) {
            return getTop(_pt.fetch_add(1, std::memory_order_acquire));
        } else {
            _pt.store(0, std::memory_order_release);
        }
        return 0;
    }

    bool steal() {
        for (size_t i = 0; i < _victim.size(); ++i) {
            Wsqueue* victim = _victim[i];
            uint64_t pt = victim->_pt.load(std::memory_order_acquire);
            uint32_t top = getTop(pt);
            uint32_t bottom = getBottom(pt);
            Assert(bottom >= top);
            uint64_t numTask = bottom - top;
            while (numTask > kReservedTasks) {
                uint32_t newTop = top + (numTask >> 1);
                uint64_t newPt = getPt(newTop, bottom);
                if (victim->_pt.compare_exchange_strong(pt, newPt, std::memory_order_seq_cst)) {
                    _pt.store(getPt(top, newTop), std::memory_order_release);
                    return true;
                } else {
                    top = getTop(pt);
                    bottom = getBottom(pt);
                    numTask = bottom - top;
                }
            }
        }
        return false;
    }

 private:
    typedef union {
        uint64_t        u64;
        struct {
            uint32_t    u32_1;
            uint32_t    u32_2;
        } u32;
    } U;

    inline static uint64_t getPt(uint32_t top, uint32_t bottom) {
        return U{.u32{top, bottom}}.u64;
    }

    inline static uint32_t getTop(uint64_t pt) {
        return U{pt}.u32.u32_1;
    }

    inline static uint32_t getBottom(uint64_t pt) {
        return U{pt}.u32.u32_2;
    }

 private:
    std::atomic<uint64_t>                   _pt;
    std::vector<Wsqueue*>                   _victim;
};
}
