%{
#include <string.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "vid.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "netlist_reader_lib.h"
#include "szk_log.h"
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
using Port = netlist::Port<netlist::NL_DEFAULT>;
using Net = netlist::Net<netlist::NL_DEFAULT>;
using HierInst = netlist::HierInst<netlist::NL_DEFAULT>;
using Module = netlist::Module<netlist::NL_DEFAULT>;
using Vid = netlist::Vid;
using util::Logger;
using netlist::reader::gCtx;
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
    : modules {
        netlist::reader::sanityCheck();
    }
    |
    ;

modules
    : module
    | modules module
    ;

module
    : '(' T_MODULE T_ID module_items ')' {
        Vid name = $3;
        auto it = gCtx.unresolvedModules.find(name);
        Module* module = nullptr;
        if (gCtx.resolvedModules.find(name) != gCtx.resolvedModules.end()) {
            Logger::fatal("Duplicate module '%s'", name.str().c_str());
        }
        if (it != gCtx.unresolvedModules.end()) {
            module = it->second;
            gCtx.unresolvedModules.erase(name);
        } else {
            module = new Module(name);
        }
        for (Port* port : gCtx.ports) {
            if (!module->addPort(port)) {
                Logger::fatal("Duplicate port '%s'", port->getName().str().c_str());
            }
        }
        for (Net* net : gCtx.nets) {
            if (!module->addNet(net)) {
                Logger::fatal("Duplicate net '%s'", net->getName().str().c_str());
            }
        }
        for (HierInst* inst : gCtx.hinsts) {
            if (!module->addHierInst(inst)) {
                Logger::fatal("Duplicate hierarchical instance '%s'", inst->getName().str().c_str());
            }
        }
        gCtx.clear();
        gCtx.resolvedModules.emplace(name, module);
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
        gCtx.direction = Port::kPortInput;
    }
    | T_OUTPUT {
        gCtx.direction = Port::kPortOutput;
    }
    | T_INOUT {
        gCtx.direction = Port::kPortInout;
    }
    ;

port
    : '(' T_PORT direction T_ID ')' {
        Port* port = new Port($4, gCtx.direction, Netlist::get().getTypeScalar());
        gCtx.ports.emplace_back(port);
    }
    ;

net 
    : '(' T_NET T_ID ')' {
        Net* net = new Net($3, Netlist::get().getTypeScalar());
        gCtx.nets.emplace_back(net);
    }
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
    : '(' T_HIERINST T_ID T_ID port_refs ')' {
        Vid modName = $3;
        Module* module = nullptr;
        auto it1 = gCtx.resolvedModules.find(modName);
        if (it1 != gCtx.resolvedModules.end()) {
            module = it1->second;
        } else {
            auto it2 = gCtx.unresolvedModules.find(modName);
            if (it2 != gCtx.unresolvedModules.end()) {
                module = it2->second;
            } else {
                module = new Module(modName);
                gCtx.unresolvedModules.emplace(modName, module);
            }
        }
        HierInst* hinst = new HierInst($4, module); 
        gCtx.hinsts.emplace_back(hinst);
    }
    ;

leaf_inst
    : '(' T_LEAFINST T_ID T_ID port_refs ')'
    ;


%%

void yyerror(const char* s) {
    fprintf(stderr, "--- Oops, %s\n", s);
}
