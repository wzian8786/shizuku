%{
#include <stdlib.h>
#include "vid.h"
#include "netlist_reader_def.h"
#include "szk_log.h"
#define YY_NO_UNPUT
#define YY_NO_INPUT
typedef vid::VID VID;
%}
MODULE      module
HIERINST    hier
LEAFINST    leaf
PORTREF     portref
PORT        port
NET         net
INPUT       input
OUTPUT      output
INOUT       inout
SPACE       [ \t\b\f\r]
NEWLINE     \n

SIMPLE_ID   [a-zA-Z_][a-zA-Z0-9$_]*
ESCAPED_ID  \\[^ \t\b\f\r\n]+[ \t\b\f\r\n]
ID          {ESCAPED_ID}|{SIMPLE_ID}
NUMBER      [1-9][0-9]*

%%

";"[^\n]*   { ; }
{MODULE}    { return T_MODULE; }
{HIERINST}  { return T_HIERINST; }
{LEAFINST}  { return T_LEAFINST; }
{PORTREF}   { return T_PORTREF; }
{PORT}      { return T_PORT; }
{NET}       { return T_NET; }
{INPUT}     { return T_INPUT; }
{OUTPUT}    { return T_OUTPUT; }
{INOUT}     { return T_INOUT; }
{ID}        { yylval.vid = VID(yytext); return T_ID; }
{NUMBER}    { yylval.num = strtol(yytext, nullptr, 0); return T_NUMBER; }
{SPACE}     { ; }
{NEWLINE}   { ; }
"("         { return '('; }
")"         { return ')'; }
.           { util::Logger::fatal("Oops, syntax error."); }

