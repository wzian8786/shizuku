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
using netlist::reader::NetContext;
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
%token T_UPPORT;
%token T_DOWNPORT;
%token T_LEAFPORT;
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
        netlist::reader::resolveNets();
        netlist::reader::sanityCheck();
    }
    |
    ;

modules
    : module
    | modules module
    ;

module
    : '(' T_MODULE T_ID {
        Vid name = $3;
        gCtx.clear();
        if (gCtx.resolvedModules.find(name) != gCtx.resolvedModules.end()) {
            Logger::fatal("Duplicate module '%s'", name.str().c_str());
        }
        auto it = gCtx.unresolvedModules.find(name);
        if (it != gCtx.unresolvedModules.end()) {
            gCtx.module = it->second;
            gCtx.unresolvedModules.erase(name);
        } else {
            gCtx.module = new Module(name);
        }
        gCtx.resolvedModules.emplace(name, gCtx.module);
    } module_items ')' {
        netlist::reader::gNets.emplace(gCtx.module, std::move(gCtx.nets));
    }
    | '(' T_MODULE T_ID ')' {
        Vid name = $3;
        gCtx.clear();
        if (gCtx.resolvedModules.find(name) != gCtx.resolvedModules.end()) {
            Logger::fatal("Duplicate module '%s'", name.str().c_str());
        }
        auto it = gCtx.unresolvedModules.find(name);
        if (it != gCtx.unresolvedModules.end()) {
            gCtx.module = it->second;
            gCtx.unresolvedModules.erase(name);
        } else {
            gCtx.module = new Module(name);
        }
        netlist::reader::gNets.emplace(gCtx.module, std::move(gCtx.nets));
        gCtx.resolvedModules.emplace(name, gCtx.module);
    }
    ;

module_items 
    : module_item
    | module_items module_item
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
        if (!gCtx.module->addPort(port)) {
            Logger::fatal("Duplicate port '%s'", port->getName().str().c_str());
        }
    }
    ;

upport
    : '(' T_UPPORT T_ID ')' {
        Assert(!gCtx.nets.empty());
        NetContext& ctx = gCtx.nets.back().second;
        ctx.upports.emplace_back($3);
    }
    ;

downport
    : '(' T_DOWNPORT T_ID T_ID ')' {
        Assert(!gCtx.nets.empty());
        NetContext& ctx = gCtx.nets.back().second;
        ctx.downports.emplace_back($3, $4);
    }
    ;

leafport
    : '(' T_LEAFPORT T_ID T_ID ')'
    ;

conn_item
    : upport
    | downport
    | leafport
    ;

conn_items
    : conn_item
    | conn_items conn_item
    ;

net 
    : '(' T_NET T_ID {
        Net* net = new Net($3, Netlist::get().getTypeScalar());
        gCtx.nets.emplace_back(net, NetContext());
    } conn_items ')'
    | '(' T_NET T_ID ')' {
        Net* net = new Net($3, Netlist::get().getTypeScalar());
        gCtx.nets.emplace_back(net, NetContext());
    }
    ;

hier_inst
    : '(' T_HIERINST T_ID T_ID ')' {
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
        if (!gCtx.module->addHierInst(hinst)) {
            Logger::fatal("Duplicate hierarchical instance '%s'", hinst->getName().str().c_str());
        }
        hinst->setParent(gCtx.module);
    }
    ;

leaf_inst
    : '(' T_LEAFINST T_ID T_ID ')'
    ;


%%

void yyerror(const char* s) {
    Logger::fatal("Oops, %s", s);
}
