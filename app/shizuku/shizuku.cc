#include "szk_option.h"
#include "szk_log.h"
#include "netlist_reader.h"
#include "nl_folded_obj.h"
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
        if (!fp) {
            util::Logger::fatal("Failed to open file '%s'", arg.c_str());
        }
        netlist::reader::NetlistReader::parse(fp);
        fclose(fp);
    }
    netlist::reader::NetlistReader::finalize();
}

int main(int argc, const char* argv[]) {
    printBanner();
    parseOption(argc, argv);
    parseNetlist();
    netlist::Module<netlist::NL_DEFAULT>::foreach([](const netlist::Module<netlist::NL_DEFAULT>& module, size_t) {
        module.print(stdout, false);
    }, 1);
    netlist::Process<netlist::NL_DEFAULT>::foreach([](const netlist::Process<netlist::NL_DEFAULT>& process, size_t) {
        process.print(stdout, false);
    }, 1);
    return 0;
}
