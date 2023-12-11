#include <stdio.h>
#include "netlist_reader.h"
extern int yyparse(void);
extern FILE* yyin;
extern "C" int yywrap(void) {
    return 1;
}
extern int yydebug;
namespace netlist {
namespace reader {
void NetlistReader::parse(FILE* fp) {
    yyin = fp;
    yyparse();
}
}
}
