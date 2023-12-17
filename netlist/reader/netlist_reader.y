%{
#include <string.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "vid.h"
#include "nl_folded_obj.h"
#include "nl_netlist.h"
#include "nl_reader_lib.h"
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
using MInst = netlist::MInst<netlist::NL_DEFAULT>;
using PInst = netlist::PInst<netlist::NL_DEFAULT>;
using Module = netlist::Module<netlist::NL_DEFAULT>;
using Process = netlist::Process<netlist::NL_DEFAULT>;
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
%token T_MINST;
%token T_PINST;
%token T_MPORT;
%token T_IPORT;
%token T_PPORT;
%token T_PORT;
%token T_NET;
%token T_INPUT;
%token T_OUTPUT;
%token T_INOUT;
%token T_ID;
%token T_NUMBER;
%token T_PROCESS;
%token T_COMB;
%token T_SEQ;
%token T_CALL;

%type <vid> T_ID;

%start source_text;

%%

source_text 
    : blocks
    |
    ;

blocks
    : block
    | blocks block
    ;

block
    : module
    | process 
    ;

module
    : '(' T_MODULE T_ID {
        Vid name = $3;
        if (gCtx.resolvedModules.find(name) != gCtx.resolvedModules.end()) {
            Logger::fatal("Duplicate module '%s'", name.str().c_str());
        }
        auto it = gCtx.unresolvedModules.find(name);
        if (it != gCtx.unresolvedModules.end()) {
            gCtx.module = it->second;
            gCtx.unresolvedModules.erase(name);
        } else {
            gCtx.module = Netlist::get().createModule(name);
        }
        gCtx.resolvedModules.emplace(name, gCtx.module);
    } module_items ')' {
        netlist::reader::gNets.emplace(gCtx.module, std::move(gCtx.nets));
        gCtx.clear();
    }
    | '(' T_MODULE T_ID ')' {
        Vid name = $3;
        if (gCtx.resolvedModules.find(name) != gCtx.resolvedModules.end()) {
            Logger::fatal("Duplicate module '%s'", name.str().c_str());
        }
        auto it = gCtx.unresolvedModules.find(name);
        if (it != gCtx.unresolvedModules.end()) {
            gCtx.module = it->second;
            gCtx.unresolvedModules.erase(name);
        } else {
            gCtx.module = Netlist::get().createModule(name);
        }
        netlist::reader::gNets.emplace(gCtx.module, std::move(gCtx.nets));
        gCtx.resolvedModules.emplace(name, gCtx.module);
        gCtx.clear();
    }
    ;

module_items 
    : module_item
    | module_items module_item
    ;

module_item 
    : port
    | net
    | minst
    | pinst
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
    : '(' T_PORT T_ID direction')' {
        uint32_t id;
        Port* port = new (id) Port(id, $3, gCtx.direction, Netlist::get().getTypeScalar());
        if (gCtx.module) {
            Assert(!gCtx.process);
            if (!gCtx.module->addPort(port)) {
                Logger::fatal("Duplicate port '%s'", port->getName().str().c_str());
            }
        } else {
            Assert(gCtx.process);
            if (!gCtx.process->addPort(port)) {
                Logger::fatal("Duplicate port '%s'", port->getName().str().c_str());
            }
        }
    }
    ;

upport
    : '(' T_MPORT T_ID ')' {
        Assert(!gCtx.nets.empty());
        NetContext& ctx = gCtx.nets.back().second;
        ctx.upports.emplace_back($3);
    }
    ;

downport
    : '(' T_IPORT T_ID T_ID ')' {
        Assert(!gCtx.nets.empty());
        NetContext& ctx = gCtx.nets.back().second;
        ctx.downports.emplace_back($3, $4);
    }
    ;

pport
    : '(' T_PPORT T_ID T_ID ')' {
        Assert(!gCtx.nets.empty());
        NetContext& ctx = gCtx.nets.back().second;
        ctx.pports.emplace_back($3, $4);
    }
    ;

conn_item
    : upport
    | downport
    | pport
    ;

conn_items
    : conn_item
    | conn_items conn_item
    ;

net 
    : '(' T_NET T_ID {
        uint32_t id;
        Net* net = new (id) Net(id, $3, Netlist::get().getTypeScalar());
        gCtx.nets.emplace_back(net, NetContext());
    } conn_items ')'
    | '(' T_NET T_ID ')' {
        uint32_t id;
        Net* net = new (id) Net(id, $3, Netlist::get().getTypeScalar());
        gCtx.nets.emplace_back(net, NetContext());
    }
    ;

minst
    : '(' T_MINST T_ID T_ID ')' {
        Vid modName = $4;
        Module* module = nullptr;
        auto it1 = gCtx.resolvedModules.find(modName);
        if (it1 != gCtx.resolvedModules.end()) {
            module = it1->second;
        } else {
            auto it2 = gCtx.unresolvedModules.find(modName);
            if (it2 != gCtx.unresolvedModules.end()) {
                module = it2->second;
            } else {
                module = Netlist::get().createModule(modName);
                gCtx.unresolvedModules.emplace(modName, module);
            }
        }
        uint32_t id;
        MInst* minst = new (id) MInst(id, $3, module); 
        if (!gCtx.module->addMInst(minst)) {
            Logger::fatal("Duplicate module instance '%s'", minst->getName().str().c_str());
        }
        minst->setParent(gCtx.module);
    }
    ;

pinst
    : '(' T_PINST T_ID T_ID ')' {
        Vid procName = $4;
        Process* process = nullptr;
        auto it1 = gCtx.resolvedProcesses.find(procName);
        if (it1 != gCtx.resolvedProcesses.end()) {
            process = it1->second;
        } else {
            auto it2 = gCtx.unresolvedProcesses.find(procName);
            if (it2 != gCtx.unresolvedProcesses.end()) {
                process = it2->second;
            } else {
                process = Netlist::get().createProcess(procName);
                gCtx.unresolvedProcesses.emplace(procName, process);
            }
        }
        uint32_t id;
        PInst* pinst = new (id) PInst(id, $3, process);
        if (!gCtx.module->addPInst(pinst)) {
            Logger::fatal("Duplicate process instance '%s'", pinst->getName().str().c_str());
        }
        pinst->setParent(gCtx.module);
    }
    ;

process
    : '(' T_PROCESS T_ID {
        Vid name = $3;
        if (gCtx.resolvedProcesses.find(name) != gCtx.resolvedProcesses.end()) {
            Logger::fatal("Duplicate process '%s'", name.str().c_str());
        }
        auto it = gCtx.unresolvedProcesses.find(name);
        if (it != gCtx.unresolvedProcesses.end()) {
            gCtx.process = it->second;
            gCtx.unresolvedProcesses.erase(name);
        } else {
            gCtx.process = Netlist::get().createProcess(name);
        }
        gCtx.resolvedProcesses.emplace(name, gCtx.process);
    } process_type process_items')' {
        Assert(gCtx.process->isComb() || gCtx.process->isSeq() || gCtx.process->isCall());
        gCtx.process = nullptr;
    }
    ;

process_items
    : process_item
    | process_items process_item
    ;

process_item
    : port
    ;

process_type
    : T_COMB {
        gCtx.process->setType(Process::kTypeComb);
    }
    | T_SEQ {
        gCtx.process->setType(Process::kTypeSeq);
    }
    | T_CALL {
        gCtx.process->setType(Process::kTypeCall);
    }
    ;

%%

void yyerror(const char* s) {
    Logger::fatal("Oops, %s", s);
}
