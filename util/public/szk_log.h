#pragma once
#include <cstdio>
namespace util {
class Logger {
 public:
    enum Level {
        kDebug = 0,
        kNote,
        kInfo,
        kWarning,
        kNoWarning,
    };

    Logger() : _level(kInfo), _out(stdout), _error(stderr) {}
    ~Logger() { fflush(_out); }

    static void setLevel(Level level) { gLogger._level = level; }
    static void setOutFd(FILE* out) { gLogger._out = out; }
    static void setErrorFd(FILE* error) { gLogger._error = error; }

    static void debug(const char*, ...);
    static void note(const char*, ...);
    static void info(const char*, ...);
    static void warning(const char*, ...);
    static void error(const char*, ...);
    static void fatal(const char*, ...);

 private:
    static Logger           gLogger;
    Level                   _level;
    FILE*                   _out;
    FILE*                   _error;
};
}
