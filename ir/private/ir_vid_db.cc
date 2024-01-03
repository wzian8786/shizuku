#include "ir_vid_db.h"
#include <vector>
#include "ir_def.h"
#include "vid.h"
namespace ir {
PreVidDB::PreVidDB() {
    static std::vector<std::pair<int, std::string> > pd = {
        { kVidSRoot, "S$Root" },
    };
    Vid::predef(pd);
}
}
