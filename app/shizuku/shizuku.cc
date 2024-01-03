#include "szk_option.h"
#include "szk_log.h"
#include "ir_reader.h"
#include "ir_db_impl.h"
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

enum {
    kOptHelp = 0,
    kOptDumpPostElabNL,
    kOptDumpPreElabNL,
};

static void parseOption(int argc, const char* argv[]) {
    util::OptionDB& db = util::OptionDB::get();
    db.addOption(kOptHelp, new util::BoolOption("-h", "--help", false));
    db.addOption(kOptDumpPostElabNL, new util::BoolOption("", "--dump-post-elab-nl", false));
    db.addOption(kOptDumpPreElabNL, new util::BoolOption("", "--dump-pre-elab-nl", false));
    db.parse(argc, argv);
}

static void parseIR() {
    const util::OptionDB::ArgVec& args = util::OptionDB::get().getArgs();
    for (const std::string& arg : args) {
        FILE* fp = fopen(arg.c_str(),  "r");
        if (!fp) {
            util::Logger::fatal("Failed to open file '%s'", arg.c_str());
        }
        ir::reader::IRReader::parse(fp);
        fclose(fp);
    }
    ir::reader::IRReader::finalize();
}

int main(int argc, const char* argv[]) {
    printBanner();
    parseOption(argc, argv);
    parseIR();
    ir::IRDBImpl<ir::IR_DEFAULT>& nl =
            ir::IRDBImpl<ir::IR_DEFAULT>::get();
    if (util::OptionDB::getOption(kOptDumpPreElabNL).enabled()) {
        FILE* fp = fopen("pre-elab.nl", "w");
        nl.print(fp, true);
        fclose(fp);
    }
    nl.elab();
    if (util::OptionDB::getOption(kOptDumpPostElabNL).enabled()) {
        FILE* fp = fopen("post-elab.nl", "w");
        nl.print(fp, true);
        fclose(fp);
    }
    nl.debugPrint();
    return 0;
}
