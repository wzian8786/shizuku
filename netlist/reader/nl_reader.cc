#include <stdio.h>
#include "nl_reader.h"
#include "nl_reader_lib.h"
#include "szk_assert.h"
extern int yyparse(void);
extern FILE* yyin;
extern "C" int yywrap(void) {
    return 1;
}
extern int yydebug;
namespace netlist {
namespace reader {
void NetlistReader::parse(FILE* fp) {
    Assert(fp);
    yyin = fp;
    yyparse();
}
void NetlistReader::finalize() {
    sanityCheck();
    resolveNets();
    buildTops();
}
}
}
