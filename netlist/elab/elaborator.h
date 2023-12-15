#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
namespace netlist {
template<uint32_t Namespace>
class Elaborator {
 public:
    Elaborator();

    void elab();

 private:
    void genWeights();

 private:
    std::vector<size_t>             _dfs;
    std::vector<size_t>             _cellNum;
};
}
