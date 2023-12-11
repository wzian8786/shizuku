#include "nl_netlist.h"
namespace netlist {
template<uint32_t Namespace>
Netlist<Namespace> __attribute__((init_priority(102))) Netlist<Namespace>::gSingleton;

template class Netlist<NL_DEFAULT>;
}
