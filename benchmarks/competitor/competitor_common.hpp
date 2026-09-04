#pragma once

#include "measurement.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fixedwide_competitor {

inline constexpr std::size_t data_size = 4096;
static_assert((data_size & (data_size - 1)) == 0);

extern std::uint64_t validations;

[[noreturn]] void fail(std::string_view what);
void expect(bool condition, std::string_view what);

// Keep a result observable without adding work to the measured operation.
template<class T>
inline void consume(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" : : "g"(&value) : "memory");
#else
    volatile const T* sink = &value;
    (void)sink;
#endif
}

// Make the bytes written by a caller-buffer formatting API observable too.
inline void consume_bytes(const char* data, std::size_t size) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" : : "r"(data), "r"(size) : "memory");
#else
    volatile const char* sink = data;
    if (size != 0) (void)sink[size - 1];
#endif
}

constexpr std::int64_t pow10_i64(unsigned decimals) {
    std::int64_t value = 1;
    for (unsigned i = 0; i < decimals; ++i) value *= 10;
    return value;
}

std::string fixed_text(std::int64_t raw, unsigned decimals);

struct OpCase {
    std::int64_t lhs_raw{};
    std::int64_t rhs_raw{};
    std::int64_t expected_raw{};
    std::string lhs_text;
    std::string rhs_text;
    std::string expected_text;
};

struct TextCase {
    std::int64_t raw{};
    std::string text;
};

struct Fixtures {
    unsigned decimals{};
    std::int64_t scale{};
    std::vector<OpCase> add;
    std::vector<OpCase> mul;
    std::vector<OpCase> div;
    std::vector<TextCase> text;
};

Fixtures make_scale4_fixtures();
Fixtures make_scale12_fixtures();

template<class F>
void row(const char* library, const char* type, const char* semantic_class, const char* operation, F&& loop) {
    fixedwide_bench::measure(std::string(library) + ",\"" + type + "\"," + semantic_class + "," + operation,
                             std::forward<F>(loop));
}

void benchmark_fixed_scale4(const Fixtures& fixtures);
void benchmark_fixed_scale12(const Fixtures& fixtures);
void benchmark_decimal_float(const Fixtures& fixtures);
void benchmark_adjacent_types(const Fixtures& fixtures);
void benchmark_hardware_floors(const Fixtures& fixtures);
void benchmark_serialization(const Fixtures& fixtures);

} // namespace fixedwide_competitor
