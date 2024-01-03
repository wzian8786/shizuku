#include <stdio.h>
#include "ir_reader.h"
#include "ir_reader_lib.h"
#include "szk_assert.h"
extern int yyparse(void);
extern FILE* yyin;
extern "C" int yywrap(void) {
    return 1;
}
extern int yydebug;
namespace ir {
namespace reader {
void IRReader::parse(FILE* fp) {
    Assert(fp);
    yyin = fp;
    yyparse();
}
void IRReader::finalize() {
    sanityCheck();
    resolveNets();
    buildTops();
}
}
}
