%{
#include <string.h>
#include <cstdint>
#include <vector>
#include "vid.h"
#include "nl_module.h"
#include "nl_netlist.h"
extern int yylex(void);
void yyerror(const char* s);
#define YYCOPY(Dst, Src, Count)                 \
    do {                                        \
        YYPTRDIFF_T yyi;                        \
        for (yyi = 0; yyi < (Count); yyi++) {   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
    } while (0);
using Netlist = netlist::Netlist<netlist::NL_DEFAULT>;
using Module = netlist::Module<netlist::NL_DEFAULT>;
using Port = netlist::Port<netlist::NL_DEFAULT>;

static std::vector<Port*> gPorts;
static Port::Direction gDirection = Port::kPortInvalid;
%}

%union yyu {
    yyu() { num = 0; }
    yyu& operator= (const yyu& a) { num = a.num; return *this; }
    int64_t  num;
    vid::VID vid;
}

%token T_MODULE;
%token T_HIERINST;
%token T_LEAFINST;
%token T_PORTREF;
%token T_PORT;
%token T_NET;
%token T_INPUT;
%token T_OUTPUT;
%token T_INOUT;
%token T_ID;
%token T_NUMBER;

%type <vid> T_ID;

%start source_text;

%%

source_text 
    : modules
    |
    ;

modules
    : module
    | modules module
    ;

module
    : '(' T_MODULE T_ID module_items ')' {
        Module* module = new Module($3);
        for (Port* port : gPorts) {
            module->addPort(port);
        }
        gPorts.clear();
    }
    ;

module_items 
    : module_item
    | module_items module_item
    |
    ;

module_item 
    : port
    | net
    | hier_inst
    | leaf_inst
    ;

direction 
    : T_INPUT {
        gDirection = Port::kPortInput;
    }
    | T_OUTPUT {
        gDirection = Port::kPortOutput;
    }
    | T_INOUT {
        gDirection = Port::kPortInout;
    }
    ;

port
    : '(' T_PORT direction T_ID ')' {
        Port* port = new Port($4, gDirection, Netlist::get().getTypeScalar());
        gPorts.push_back(port);
    }
    ;

net 
    : '(' T_NET T_ID ')' 
    ;

port_ref 
    : '(' T_PORTREF T_ID T_ID ')'
    ;

port_refs 
    : port_ref
    | port_refs port_ref
    |
    ;

hier_inst
    : '(' T_HIERINST T_ID T_ID port_refs ')'
    ;

leaf_inst
    : '(' T_LEAFINST T_ID T_ID port_refs ')'
    ;


%%

void yyerror(const char* s) {
    fprintf(stderr, "--- Oops, %s\n", s);
}
