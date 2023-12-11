#pragma once
#ifdef NDEBUG 
#define Assert(p)
#else
#include <cstdlib>
#include <cstdio>
#define ASSERT(p) \
    do { \
        if (__builtin_expect(!(p), 0)) { \
            fprintf(stderr, "Assertion failed at %s:%d\n", __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0); \

#define Assert(p) ASSERT(p)
#endif