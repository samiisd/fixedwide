#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstdint>

inline std::uint64_t checks = 0;
inline void check(bool condition, const char* expression, const char* file, int line) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "%s:%d: FAILED: %s\n", file, line, expression);
        std::abort();
    }
}
#define CHECK(...) check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__, __FILE__, __LINE__)
