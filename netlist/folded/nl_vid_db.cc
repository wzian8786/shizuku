#include "nl_vid_db.h"
#include <vector>
#include "nl_def.h"
#include "vid.h"
namespace netlist {
PreVidDB::PreVidDB() {
    static std::vector<std::pair<int, std::string> > pd = {
        { kVidSRoot, "S$Root" },
    };
    Vid::predef(pd);
}
}
