// Deterministic per-operation instruction counter.
//
// Wall-clock timing on a shared CI runner is too noisy to gate on: a 3% real
// regression is invisible under 15% of neighbour noise. Retired instructions
// are not noisy -- under Valgrind the same binary on the same input executes
// exactly the same number of them, run after run and machine after machine.
//
// Usage: icount <workload> <iterations>
//
// The harness is deliberately dumb. It runs the workload exactly `iterations`
// times and prints one number; scripts/icount.sh runs it twice, at n and 2n,
// and subtracts. That subtraction cancels process startup, page faults, the
// fixture setup and the print, so what is left is the marginal cost of one
// operation and nothing else. Anything here that does NOT scale with
// `iterations` is therefore free, and anything that does must be the operation
// under test -- which is why there is no timing, no oracle and no I/O in the
// loop.

#include <fixedwide/arithmetic.hpp>
#include <fixedwide/binary.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/mixed.hpp>

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>

namespace fw = fixedwide;

namespace {

// A power of two so the index is a mask, not a division: a modulo would put a
// division in every measured iteration and swamp the cheap operations.
constexpr std::size_t fixture_size = 256;
static_assert((fixture_size & (fixture_size - 1)) == 0, "fixture_size must be a power of two");
constexpr std::size_t fixture_mask = fixture_size - 1;

// A small deterministic generator, so a baseline recorded on one machine is
// reproducible on another. std::mt19937_64 would work too but pulls in a header
// whose constructor cost varies between standard libraries.
struct Splitmix {
    std::uint64_t state;
    constexpr std::uint64_t operator()() noexcept {
        state += 0x9E37'79B9'7F4A'7C15ULL;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58'476D'1CE4'E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D0'49BB'1331'11EBULL;
        return z ^ (z >> 31);
    }
};

// The sink every workload feeds. volatile, so the compiler must materialise the
// result of each operation and cannot hoist the loop out.
volatile std::uint64_t sink = 0;

std::uint64_t digest(std::int64_t v) { return static_cast<std::uint64_t>(v); }
std::uint64_t digest(fw::wide::int128 v) { return v.low ^ v.high; }
std::uint64_t digest(fw::wide::int256 v) {
    return v.limbs[0] ^ v.limbs[1] ^ v.limbs[2] ^ v.limbs[3];
}

template<typename Fixed>
std::uint64_t digest(Fixed v) { return digest(v.raw()); }

// An expected<T, E>: fold both the value and the error into the sink, so the
// error path is not optimised away either.
template<typename T, typename E>
std::uint64_t digest(const std::expected<T, E>& r) {
    return r ? digest(*r) : 0xFFFF'FFFF'FFFF'FFFFULL - static_cast<std::uint64_t>(r.error());
}

// Fill a fixture with nonzero values that stay well inside the type's range, so
// no workload spends its iterations on the overflow early-out.
template<typename Fixed>
std::array<Fixed, fixture_size> make_fixture(std::uint64_t seed, unsigned raw_bits) {
    Splitmix rng{seed};
    std::array<Fixed, fixture_size> out{};
    for (auto& element : out) {
        using Raw = typename Fixed::raw_type;
        std::uint64_t bits = rng() >> (64 - raw_bits);
        if (bits == 0) bits = 1;
        if constexpr (sizeof(Raw) <= 8) {
            element = Fixed::from_raw(static_cast<Raw>(bits));
        } else {
            element = Fixed::from_raw(Raw(bits));
        }
    }
    return out;
}

// Plain int64 operands, for the raw-type floor rows. Divisors are forced
// nonzero so the division rows measure division and not an early exit.
std::array<std::int64_t, fixture_size> raw_int64_fixture(std::uint64_t seed) {
    Splitmix rng{seed};
    std::array<std::int64_t, fixture_size> out{};
    for (auto& element : out) {
        std::uint64_t bits = rng() >> (64 - 26);
        if (bits == 0) bits = 1;
        element = static_cast<std::int64_t>(bits);
    }
    return out;
}

// Every workload has this shape: two fixtures and a binary operation. The mask
// index and the sink are identical across workloads, so their cost is common to
// every row and mostly cancels between rows as well.
template<typename FixtureA, typename FixtureB, typename Op>
void run(std::uint64_t iterations, const FixtureA& a, const FixtureB& b, Op op) {
    std::uint64_t accumulator = 0;
    for (std::uint64_t i = 0; i < iterations; ++i) {
        const std::size_t index = static_cast<std::size_t>(i) & fixture_mask;
        accumulator ^= digest(op(a[index], b[index]));
    }
    sink = accumulator;
}

template<typename Fixture, typename Op>
void run_unary(std::uint64_t iterations, const Fixture& a, Op op) {
    std::uint64_t accumulator = 0;
    for (std::uint64_t i = 0; i < iterations; ++i) {
        const std::size_t index = static_cast<std::size_t>(i) & fixture_mask;
        accumulator ^= digest(op(a[index]));
    }
    sink = accumulator;
}

// Formatting writes into a caller's buffer, so it does not fit run()'s shape.
template<typename Fixture>
void run_format(std::uint64_t iterations, const Fixture& values) {
    std::uint64_t accumulator = 0;
    char buffer[fw::text_capacity];
    for (std::uint64_t i = 0; i < iterations; ++i) {
        const std::size_t index = static_cast<std::size_t>(i) & fixture_mask;
        const auto written = fw::to_chars(buffer, sizeof buffer, values[index]);
        accumulator ^= written ? *written : 0u;
        accumulator ^= static_cast<unsigned char>(buffer[0]);
    }
    sink = accumulator;
}

using Money64  = fw::Fixed64<12>;
using Money128 = fw::Fixed128<18>;
using Money256 = fw::Fixed256<38>;
using Price    = fw::Fixed64<4>;
using Rate     = fw::Fixed64<8>;
// Six decimals keeps Price x Rate inside the native mixed path; eighteen does
// not, and that is the point of having both.
using MixedFast = fw::Fixed128<6>;

constexpr auto nearest = fw::Rounding::nearest_even;

// ---------------------------------------------------------------------------
// The workload table. Adding a row here and re-recording the baseline is the
// whole procedure for putting a new operation under the regression gate.
// ---------------------------------------------------------------------------
bool dispatch(std::string_view name, std::uint64_t n) {
    // 26 raw bits keeps a 12-decimal product inside 64 bits.
    static const auto a64  = make_fixture<Money64>(1, 26);
    static const auto b64  = make_fixture<Money64>(2, 26);
    static const auto a128 = make_fixture<Money128>(3, 56);
    static const auto b128 = make_fixture<Money128>(4, 56);
    static const auto a256 = make_fixture<Money256>(5, 60);
    static const auto b256 = make_fixture<Money256>(6, 60);
    static const auto price = make_fixture<Price>(7, 30);
    static const auto rate  = make_fixture<Rate>(8, 30);

    if (name == "add.Fixed64")      { run(n, a64, b64, [](auto x, auto y) { return fw::add(x, y); }); return true; }
    if (name == "sub.Fixed64")      { run(n, a64, b64, [](auto x, auto y) { return fw::sub(x, y); }); return true; }
    if (name == "mul.Fixed64")      { run(n, a64, b64, [](auto x, auto y) { return fw::mul(x, y, nearest); }); return true; }
    if (name == "div.Fixed64")      { run(n, a64, b64, [](auto x, auto y) { return fw::div(x, y, nearest); }); return true; }
    if (name == "mul_div.Fixed64")  { run(n, a64, b64, [](auto x, auto y) { return fw::mul_div(x, y, y, nearest); }); return true; }
    if (name == "quantize.Fixed64") { run_unary(n, a64, [](auto x) { return fw::quantize(x, 4, nearest); }); return true; }
    if (name == "remainder.Fixed64"){ run(n, a64, b64, [](auto x, auto y) { return fw::remainder(x, y); }); return true; }

    if (name == "add.Fixed128")     { run(n, a128, b128, [](auto x, auto y) { return fw::add(x, y); }); return true; }
    if (name == "mul.Fixed128")     { run(n, a128, b128, [](auto x, auto y) { return fw::mul(x, y, nearest); }); return true; }
    if (name == "div.Fixed128")     { run(n, a128, b128, [](auto x, auto y) { return fw::div(x, y, nearest); }); return true; }
    if (name == "mul_div.Fixed128") { run(n, a128, b128, [](auto x, auto y) { return fw::mul_div(x, y, y, nearest); }); return true; }
    if (name == "quantize.Fixed128"){ run_unary(n, a128, [](auto x) { return fw::quantize(x, 6, nearest); }); return true; }

    if (name == "mul.Fixed256")     { run(n, a256, b256, [](auto x, auto y) { return fw::mul(x, y, nearest); }); return true; }
    if (name == "div.Fixed256")     { run(n, a256, b256, [](auto x, auto y) { return fw::div(x, y, nearest); }); return true; }
    if (name == "quantize.Fixed256"){ run_unary(n, a256, [](auto x) { return fw::quantize(x, 12, nearest); }); return true; }

    // Cross-scale. These come in pairs on purpose.
    //
    // detail::mixed_native takes a mixed operation only when the aligned
    // intermediate fits 126 bits; outside that it falls back to a general
    // kernel that works in 1024-bit limbs. The two differ by roughly 500x, and
    // which one you get is decided at compile time by the widths and scales.
    // Both are measured, so neither can regress unnoticed and the cliff itself
    // stays visible. docs/benchmarks.md explains where the edge is.
    if (name == "mul_to.native")  { run(n, price, rate, [](auto x, auto y) { return fw::mul_to<MixedFast>(x, y, nearest); }); return true; }
    if (name == "div_to.native")  { run(n, price, rate, [](auto x, auto y) { return fw::div_to<MixedFast>(x, y, nearest); }); return true; }
    if (name == "add_to.native")  { run(n, price, rate, [](auto x, auto y) { return fw::add_to<MixedFast>(x, y, nearest); }); return true; }
    if (name == "mul_to.general") { run(n, price, rate, [](auto x, auto y) { return fw::mul_to<Money128>(x, y, nearest); }); return true; }
    if (name == "div_to.general") { run(n, price, rate, [](auto x, auto y) { return fw::div_to<Money128>(x, y, nearest); }); return true; }
    if (name == "compare.Price.Rate") { run(n, price, rate, [](auto x, auto y) -> std::int64_t { return (x <=> y) == std::strong_ordering::less; }); return true; }
    if (name == "fixed_cast.Price.MixedFast") {
        run_unary(n, price, [](auto x) { return fw::fixed_cast<MixedFast>(x, nearest); }); return true;
    }

    // Text. The parse fixture is strings, so it does not go through run().
    if (name == "parse.Fixed64") {
        static const auto texts = [] {
            std::array<std::array<char, 24>, fixture_size> out{};
            Splitmix rng{9};
            for (auto& text : out) {
                std::snprintf(text.data(), text.size(), "%llu.%06llu",
                              static_cast<unsigned long long>(rng() % 1'000'000ULL),
                              static_cast<unsigned long long>(rng() % 1'000'000ULL));
            }
            return out;
        }();
        std::uint64_t accumulator = 0;
        for (std::uint64_t i = 0; i < n; ++i) {
            const std::size_t index = static_cast<std::size_t>(i) & fixture_mask;
            accumulator ^= digest(fw::parse<Money64>(std::string_view(texts[index].data()), nearest));
        }
        sink = accumulator;
        return true;
    }
    // The which-type test is resolved before the loop, not inside it. A
    // string_view comparison per iteration scales with n, so it survives the
    // two-point subtraction and would be counted as part of to_chars.
    if (name == "to_chars.Fixed64") { run_format(n, a64); return true; }
    if (name == "to_chars.Fixed128") { run_format(n, a128); return true; }

    // Raw machine types, as a floor. Wall-clock cannot separate these -- they
    // are all a fraction of a nanosecond, below the resolution of a timed loop
    // -- but instruction counts can, exactly. The difference between
    // add.Fixed64 and add.int64_raw is precisely what the overflow check costs.
    if (name == "add.int64_raw") {
        static const auto x = raw_int64_fixture(11), y = raw_int64_fixture(12);
        run(n, x, y, [](std::int64_t p, std::int64_t q) { return p + q; });
        return true;
    }
    if (name == "mul.int64_raw") {
        static const auto x = raw_int64_fixture(13), y = raw_int64_fixture(14);
        run(n, x, y, [](std::int64_t p, std::int64_t q) { return p * q; });
        return true;
    }
    if (name == "div.int64_raw") {
        static const auto x = raw_int64_fixture(15), y = raw_int64_fixture(16);
        run(n, x, y, [](std::int64_t p, std::int64_t q) { return p / q; });
        return true;
    }
    // Serialization against the raw-copy floor. to_bytes pins the byte order;
    // memcpy of the object representation does not, and is the fastest possible
    // answer, so the gap is what a defined wire format costs.
    if (name == "to_bytes.Fixed64") {
        run_unary(n, a64, [](Money64 v) {
            const auto bytes = fw::to_bytes<fw::endian::little>(v);
            std::uint64_t folded = 0;
            for (auto byte : bytes) folded = (folded << 8) ^ byte;
            return static_cast<std::int64_t>(folded);
        });
        return true;
    }
    if (name == "memcpy.int64_raw") {
        static const auto x = raw_int64_fixture(17);
        run_unary(n, x, [](std::int64_t v) {
            std::array<std::uint8_t, sizeof(std::int64_t)> bytes{};
            std::memcpy(bytes.data(), &v, sizeof bytes);
            std::uint64_t folded = 0;
            for (auto byte : bytes) folded = (folded << 8) ^ byte;
            return static_cast<std::int64_t>(folded);
        });
        return true;
    }

    // The empty workload. Its count is the cost of the loop, the mask and the
    // sink -- the floor every other row sits on, recorded so a change in the
    // harness itself is visible instead of being blamed on the library.
    if (name == "baseline.empty") {
        std::uint64_t accumulator = 0;
        for (std::uint64_t i = 0; i < n; ++i) {
            accumulator ^= static_cast<std::uint64_t>(i) & fixture_mask;
        }
        sink = accumulator;
        return true;
    }

    return false;
}

constexpr const char* workloads[] = {
    "baseline.empty",
    "add.Fixed64", "sub.Fixed64", "mul.Fixed64", "div.Fixed64", "mul_div.Fixed64",
    "quantize.Fixed64", "remainder.Fixed64",
    "add.Fixed128", "mul.Fixed128", "div.Fixed128", "mul_div.Fixed128", "quantize.Fixed128",
    "mul.Fixed256", "div.Fixed256", "quantize.Fixed256",
    "mul_to.native", "div_to.native", "add_to.native",
    "mul_to.general", "div_to.general",
    "compare.Price.Rate", "fixed_cast.Price.MixedFast",
    "parse.Fixed64", "to_chars.Fixed64", "to_chars.Fixed128",
    "add.int64_raw", "mul.int64_raw", "div.int64_raw",
    "to_bytes.Fixed64", "memcpy.int64_raw",
};

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "--list") == 0) {
        for (const char* name : workloads) std::puts(name);
        return 0;
    }
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s <workload> <iterations>\n       %s --list\n",
                     argv[0], argv[0]);
        return 2;
    }
    const std::uint64_t iterations = std::strtoull(argv[2], nullptr, 10);
    if (!dispatch(argv[1], iterations)) {
        std::fprintf(stderr, "unknown workload: %s\n", argv[1]);
        return 2;
    }
    // Printed so the sink is observably used; the value itself is not checked
    // here, because correctness is the test suite's job, not the counter's.
    std::printf("%llu\n", static_cast<unsigned long long>(sink));
    return 0;
}
