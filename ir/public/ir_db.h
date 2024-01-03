#pragma once
#include <cstdio>
#include "ir_def.h"
namespace ir {
template<uint32_t NS>
class IRDB {
 public:
    static void printFolded(FILE* fp, bool indent);
    static void elab();
};
}
