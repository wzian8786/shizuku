#include "szk_log.h"
#include <stdarg.h>
#include <cstdlib>
#include <cstring>
namespace util {
Logger __attribute__((init_priority(200))) Logger::gLogger;
Logger::Logger() : _level(kInfo), _out(stdout), _error(stderr) {
    const char* env = getenv("SHIZUKU_LOG_LEVEL");
    if (env) {
        if (!strcmp(env, "debug")) {
            _level = kDebug;
        } else if (!strcmp(env, "note")) {
            _level = kNote;
        } else if (!strcmp(env, "info")) {
            _level = kInfo;
        } else if (!strcmp(env, "warning")) {
            _level = kWarning;
        }
    }
}
                   
#ifndef NDEBUG
void Logger::debug(const char* fmt, ...) {
    if (gLogger._level == kDebug) {
        fprintf(gLogger._out, "%s", "[ DEBUG ]: ");
        va_list ap;
        va_start(ap, fmt);
        vfprintf(gLogger._out, fmt, ap);
        va_end(ap);
        fputc('\n', gLogger._out);
    }
}
#endif

void Logger::note(const char* fmt, ...) {
    if (gLogger._level <= kNote) {
        fprintf(gLogger._out, "%s", "[ Note  ]: ");
        va_list ap;
        va_start(ap, fmt);
        vfprintf(gLogger._out, fmt, ap);
        va_end(ap);
        fputc('\n', gLogger._out);
    }
}

void Logger::info(const char* fmt, ...) {
    if (gLogger._level <= kInfo) {
        fprintf(gLogger._out, "%s", "[ INFO  ]: ");
        va_list ap;
        va_start(ap, fmt);
        vfprintf(gLogger._out, fmt, ap);
        va_end(ap);
        fputc('\n', gLogger._out);
    }
}

void Logger::warning(const char* fmt, ...) {
    if (gLogger._level <= kWarning) {
        fprintf(gLogger._error, "%s", "[Warning]: ");
        va_list ap;
        va_start(ap, fmt);
        vfprintf(gLogger._error, fmt, ap);
        va_end(ap);
        fputc('\n', gLogger._error);
    }
}

void Logger::error(const char* fmt, ...) {
    fprintf(gLogger._error, "%s", "[ ERROR ]: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(gLogger._error, fmt, ap);
    va_end(ap);
    fputc('\n', gLogger._error);
}

void Logger::fatal(const char* fmt, ...) {
    fprintf(gLogger._error, "%s", "[ FATAL ]: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(gLogger._error, fmt, ap);
    va_end(ap);
    fputc('\n', gLogger._error);
    exit(-1);
}

}
