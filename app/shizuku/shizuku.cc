#include "szk_option.h"
#include "netlist_reader.h"
static void printBanner() {
    printf("\n");
    printf("                      Welcome\n\n");
    printf("  This project is still in development. Thera are lots\n");
    printf("  of features not finished yet. If anyone is interested\n");
    printf("  in this project, please visit:\n\n");
    printf("    https://github.com/wzian8786/shisuku\n\n");
    printf("  More information is available there.\n");
    printf("\n");
}

static void parseOption(int argc, const char* argv[]) {
    util::OptionDB& db = util::OptionDB::get();
    db.addOption(new util::BoolOption("-h", "--help", false));
    db.parse(argc, argv);
}

static void parseNetlist() {
    const util::OptionDB::ArgVec& args = util::OptionDB::get().getArgs();
    for (const std::string& arg : args) {
        FILE* fp = fopen(arg.c_str(),  "r");
        netlist::reader::NetlistReader::parse(fp);
        fclose(fp);
    }
    netlist::reader::NetlistReader::finalize();
}

int main(int argc, const char* argv[]) {
    printBanner();
    parseOption(argc, argv);
    parseNetlist();
    return 0;
}
