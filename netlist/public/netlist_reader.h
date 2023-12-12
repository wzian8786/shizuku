#pragma once
#include <cstdio>
namespace netlist {
namespace reader {
class NetlistReader {
 public:
    static void parse(FILE* fp);
};
}
}
