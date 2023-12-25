#pragma once
#include "vid.h"
namespace netlist {
typedef vid::Vid<vid::VN_DEFAULT> Vid;

enum NetlistNamespace {
    NL_DEFAULT = 0,
};

struct NlPoolSpec {
    static constexpr uint32_t kLocalBits = 8;
    static constexpr uint32_t kBlockBits = 8;
    static constexpr uint32_t kPageIDBits = 16;
};

struct NlFPoolSpec {
    static constexpr uint32_t kLocalBits = 8;
    static constexpr uint32_t kBlockBits = 8;
    static constexpr uint32_t kPageIDBits = 24;
};

template<uint32_t> class Port;
template<uint32_t> class Net;
template<uint32_t> class MInst;
template<uint32_t> class PInst;
template<uint32_t> class IPort;
template<uint32_t> class PPort;
template<uint32_t> class Module;
template<uint32_t> class Process;

template<uint32_t> class Cell;
template<uint32_t> class FMInst;
template<uint32_t> class FPInst;
template<uint32_t> class FDPort;
template<uint32_t> class FRPort;

class DataType;
class DataTypeDB;
class PreVidDB;
}
