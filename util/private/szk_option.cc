#include "szk_option.h"
#include <cstdlib>
#include "szk_assert.h"
#include "szk_log.h"
namespace util {
OptionDB OptionDB::gSingleton;
void Option::check() {
    Assert(_short.length() == 2 && _long.length() > 2);
    Assert(_short[0] == '-');
    Assert(_long[0] == '-' && _long[1] == '-');
}

bool Option::needArg() const {
    Assert(0);
    return false;
}

bool Option::enabled() const {
    Assert(0);
    return false;
}

long Option::getLong() const {
    Assert(0);
    return 0;
}

unsigned long Option::getUlong() const {
    Assert(0);
    return 0;
}

int Option::getInt() const {
    Assert(0);
    return 0;
}

unsigned int Option::getUint() const {
    Assert(0);
    return 0;
}

const std::string& Option::getString() const {
    Assert(0);
    static std::string whatever;
    return whatever;
}

double Option::getReal() const {
    Assert(0);
    return 0;
}

void OptionDB::addOption(Option* opt){
    _index.emplace(opt->shortName(), opt);
    _index.emplace(opt->LongName(), opt);
    _holder.emplace_back(opt);
}

void OptionDB::parse(int argc, const char* argv[]) {
    Assert(argc > 0);
    for (int i = 1; i < argc; ++i) {
        const char* cur = argv[i];
        if (cur[0] == '-') {
            auto it = _index.find(std::string(cur));
            if (it == _index.end()) {
                Logger::fatal("Unknown option '%s'", cur);
            }
            Option* opt = it->second;
            const char* next = nullptr;
            if (opt->needArg()) {
                if (i+1 == argc || argv[i+1][0] == '-') {
                    Logger::fatal("No argument follows option '%s'", cur);
                }
                next = argv[++i];
            }
            opt->process(next);
        } else {
            _args.emplace_back(cur);
        }
    }
}

void LongOption::process(const char* arg) {
    _v = strtol(arg, nullptr, 0);
    set();
}

void UlongOption::process(const char* arg) {
    _v = strtoul(arg, nullptr, 0);
    set();
}

void IntOption::process(const char* arg) {
    _v = strtol(arg, nullptr, 0);
    set();
}

void UintOption::process(const char* arg) {
    _v = strtoul(arg, nullptr, 0);
    set();
}

void StringOption::process(const char* arg) {
    _v = std::string(arg);
    set();
}

void RealOption::process(const char* arg) {
    _v = atof(arg);
    set();
}
}
