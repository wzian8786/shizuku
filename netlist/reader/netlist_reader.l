%{
#include <stdlib.h>
#include "vid.h"
#include "netlist_reader_def.h"
#include "szk_log.h"
#define YY_NO_UNPUT
#define YY_NO_INPUT
typedef vid::VID VID;
extern int yylineno;
%}
SPACE       [ \t\b\f\r]
NEWLINE     \n

SIMPLE_ID   [a-zA-Z_][a-zA-Z0-9$_]*
ESCAPED_ID  \\[^ \t\b\f\r\n]+[ \t\b\f\r\n]
ID          {ESCAPED_ID}|{SIMPLE_ID}
NUMBER      [1-9][0-9]*|0

%option yylineno

%%

";"[^\n]*   { ; }
"%module"   { return T_MODULE; }
"%minst"    { return T_MINST; }
"%pinst"    { return T_PINST; }
"%port"     { return T_PORT; }
"%net"      { return T_NET; }
"%input"    { return T_INPUT; }
"%output"   { return T_OUTPUT; }
"%inout"    { return T_INOUT; }
"%mport"    { return T_MPORT; }
"%iport"    { return T_IPORT; }
"%pport"    { return T_PPORT; }
"%process"  { return T_PROCESS; }
"%comb"     { return T_COMB; }
"%seq"      { return T_SEQ; }
"%call"     { return T_CALL; }
{ID}        { yylval.vid = VID(yytext); return T_ID; }
{NUMBER}    { yylval.num = strtol(yytext, nullptr, 0); return T_NUMBER; }
{SPACE}     { ; }
{NEWLINE}   { ; }
"("         { return '('; }
")"         { return ')'; }
.           { util::Logger::fatal("Oops! syntax error at line %d.", yylineno); }

