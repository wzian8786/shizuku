#pragma once
#include <cstdio>
namespace ir {
namespace reader {
class IRReader {
 public:
    static void parse(FILE* fp);
    static void finalize();
};
}
}
