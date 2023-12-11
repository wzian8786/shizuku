#include "nl_netlist.h"
namespace netlist {
template<uint32_t Namespace>
Netlist<Namespace> Netlist<Namespace>::gSingleton;

template class Netlist<NL_DEFAULT>;
}
