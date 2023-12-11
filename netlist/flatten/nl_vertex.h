#pragma once
#include <cstdint>
#include "netlist_def.h"
#include "szk_assert.h"
#include "szk_object.h"
#include "vid.h"
namespace netlist {
typedef util::Object<40, false> addr_t;

enum FlagIndex {
    kIndexValid = 0,
    kIndexHead,
    kIndexForRent,
};

inline uint64_t encode40(uint8_t h, uint32_t l) {
    return (((uint64_t)h) << 32) | l;
}

class __attribute__((packed)) Head {
 public:
    Head() : _flags{(1 << kIndexValid) |
                    (1 << kIndexHead)},
             _forrent{0},
             _dfsh{0},
             _modelh{0},
             _dfs{0},
             _model{0},
             _name{0} {}

    operator bool() const { return _flags & (1 << kIndexValid); }
    bool isHead() const { return _flags & (1 << kIndexHead); }

    Vid getVid() const { return Vid(_name); }
    void setVid(Vid name) { _name = *name; }

    uint64_t getDFS() const { return encode40(_dfsh, _dfs); }
    void setDFS(uint64_t dfs) {
        _dfs = dfs;
        dfs >>= 32;
        _dfsh = dfs;
    }

    uint64_t getModel() const { return encode40(_modelh, _model); }
    void setModel(uint64_t model) {
        _model = model;
        model >>= 32;
        _modelh = model;
    }
    
 private:
    uint8_t             _flags;
    uint8_t             _forrent;
    uint8_t             _dfsh;
    uint8_t             _modelh;
    uint32_t            _dfs;
    uint32_t            _model;
    uint64_t            _name;
};

class Cell {
 public:
    Cell() : _flags{1 << kIndexValid},
             _inputh{0, 0, 0},
             _offset{0},
             _input{0, 0, 0} {}

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

    addr_t getInput(size_t index) const {
        Assert(index < 3);
        return addr_t(encode40(_inputh[index], _input[index]));
    }


 private:
    uint8_t             _flags;
    uint8_t             _inputh[3];
    uint32_t            _offset;
    uint32_t            _input[3];
};

static_assert(sizeof(Cell) == sizeof(Head), "unexpected Cell size");

template<uint32_t Namespace>
class Vertex : public Head {
 public:
    //explicit Vertex(const Model&);

 private:
    Cell                _cell[1];
};

static_assert(sizeof(Vertex<NL_DEFAULT>) == 2 * sizeof(Cell));

}
