#pragma once
#include "vid.h"
namespace netlist {
enum NetlistNamespace {
    NL_DEFAULT = 0,
};

struct NlPoolSpec {
    static constexpr uint32_t kLocalBits = 8;
    static constexpr uint32_t kBlockBits = 8;
    static constexpr uint32_t kPageIDBits = 16;
};

typedef vid::Vid<vid::VN_DEFAULT> Vid;

template<uint32_t> class Port;
template<uint32_t> class Net;
template<uint32_t> class HierInst;
template<uint32_t> class PInst;
template<uint32_t> class DownPort;
template<uint32_t> class PPort;
template<uint32_t> class Module;
template<uint32_t> class Model;
template<uint32_t> class Process;

class DataType;
class DataTypeDB;

}
