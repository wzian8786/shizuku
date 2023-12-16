#pragma once
#include <cstdint>
#include "nl_def.h"
#include "szk_assert.h"
#include "szk_object.h"
#include "vid.h"
namespace netlist {
static constexpr uint64_t kReservedCell = 1;

enum FlagIndex {
    kIndexValid = 0,
    kIndexHead,
    kIndexForRent,
};

inline uint64_t encode40(uint8_t h, uint32_t l) {
    return (((uint64_t)h) << 32) | l;
}

class Head {
 public:
    Head() : _flags{(1 << kIndexValid) |
                    (1 << kIndexHead)},
             _pad{0},
             _dfsh{0},
             _proch{0},
             _dfs{0},
             _proc{0},
             _name{0} {}

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    Vid getVid() const { return _name; }
    void setVid(Vid name) { _name = name; }

    uint64_t getDFS() const { return encode40(_dfsh, _dfs); }
    void setDFS(uint64_t dfs) {
        _dfs = dfs;
        dfs >>= 32;
        _dfsh = dfs;
    }

    uint64_t getProcess() const { return encode40(_proch, _proc); }
    void setProcess(uint64_t proc) {
        _proc = proc;
        proc >>= 32;
        _proch = proc;
    }

 private:
    uint8_t             _flags;
    uint8_t             _pad;
    uint8_t             _dfsh;
    uint8_t             _proch;
    uint32_t            _dfs;
    uint32_t            _proc;
    Vid                 _name;
};

class Cell {
 public:
    Cell() : _flags{1 << kIndexValid},
             _inputh{0, 0, 0},
             _input{0, 0, 0},
             _offset{0},
             _origh{0},
             _orig{0} {}

    constexpr static size_t kInputPerCell = 3;

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    uint32_t getOffset() const { return _offset; }

    Head& asHead() {
        Assert(isHead());
        return *(Head*)this;
    }

    const Head& asHead() const {
        Assert(isHead());
        return *(const Head*)this;
    }

    uint64_t getInput(size_t index) const {
        Assert(index < kInputPerCell);
        return encode40(_inputh[index], _input[index]);
    }

    static size_t getNumCell(size_t in, size_t out) {
        Assert(out || in);
        return std::max(out, (in+kInputPerCell-1)/kInputPerCell);
    }

 private:
    uint8_t             _flags;
    uint8_t             _inputh[kInputPerCell];
    uint32_t            _input[kInputPerCell];
    uint32_t            _offset:24;
    uint32_t            _origh:8;
    uint32_t            _orig;
};

static_assert(sizeof(Cell) == sizeof(Head), "unexpected Cell size");

template<uint32_t NS>
class Vertex {
 public:

 private:
    Head                _head;
    Cell                _cell[1];
};

static_assert(sizeof(Vertex<NL_DEFAULT>) == 2 * sizeof(Cell), "unexpected Vertex size");
}
