// Supplementary scale-8/12 performance matrix. The historical instruction-count
// driver and baseline stay untouched. This SAME source is built against both
// revisions. Constants specialize rounding; fixtures distinguish exact/inexact
// results and positive/mixed-sign inputs. The independent preflight is untimed.
#include <fixedwide/arithmetic.hpp>

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <string_view>

namespace {
namespace fw = fixedwide;
using I = __int128;
using U = unsigned __int128;
using Mode = fw::Rounding;
enum class Operation { mul, div, mul_div };
constexpr std::size_t fixture_size = 256;
constexpr std::size_t fixture_mask = fixture_size - 1;
volatile std::uint64_t sink;
std::uint64_t checks = 0;

[[noreturn]] void fail(const char* message) {
    std::fprintf(stderr, "FAILED: %s\n", message);
    std::exit(1);
}
std::uint64_t next(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}
std::uint64_t number(std::string_view text) {
    std::uint64_t result = 0;
    auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), result);
    if (ec != std::errc{} || end != text.data() + text.size() || result == 0) fail("expected a positive integer");
    return result;
}
std::uint64_t clock_ns() {
    timespec t{};
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) != 0) fail("thread CPU clock");
    return static_cast<std::uint64_t>(t.tv_sec) * 1'000'000'000ULL + static_cast<std::uint64_t>(t.tv_nsec);
}
std::int64_t oracle(I n, I d, Mode mode) {
    if (d == 0) fail("zero divisor in fixture");
    I q = n / d;
    I r = n % d;
    const U rem = static_cast<U>(r < 0 ? -r : r);
    const U den = static_cast<U>(d < 0 ? -d : d);
    if (mode == Mode::nearest_even && (rem > den - rem || (rem == den - rem && (q % 2) != 0)))
        q += (n < 0) != (d < 0) ? -1 : 1;
    if (q < INT64_MIN || q > INT64_MAX) fail("fixture result does not fit int64");
    return static_cast<std::int64_t>(q);
}

template<unsigned D, Operation Op>
struct Fixture {
    using F = fw::Fixed64<D>;
    std::array<F, fixture_size> a{}, b{}, c{};
    std::array<std::int64_t, fixture_size> expected{};
    explicit Fixture(bool exact, bool mixed, Mode mode) {
        std::uint64_t state = 0x5d38'21a4'86fe'730bULL;
        constexpr std::int64_t scale = F::scale();
        for (std::size_t i = 0; i < fixture_size; ++i) {
            auto value = [&] { return scale + static_cast<std::int64_t>(next(state) % scale); };
            std::int64_t x, y, z;
            I n, d;
            do {
                x = value();
                y = value();
                z = value();
                if (exact) {
                    if constexpr (Op == Operation::mul) {
                        x = (1 + static_cast<std::int64_t>(next(state) % 31)) * scale;
                    } else if constexpr (Op == Operation::div) {
                        x = (1 + static_cast<std::int64_t>(next(state) % 31)) * y;
                    } else {
                        x = (1 + static_cast<std::int64_t>(next(state) % 31)) * z;
                    }
                }
                if (mixed) {
                    if ((next(state) & 1) != 0) x = -x;
                    if ((next(state) & 1) != 0) y = -y;
                    if ((next(state) & 1) != 0) z = -z;
                }
                // Every raw magnitude is below 64 * 10^12, so these exact
                // products fit signed 128 bits independently of fixedwide.
                n = Op == Operation::div ? I{x} * scale : I{x} * y;
                d = Op == Operation::mul ? scale : Op == Operation::div ? y : z;
            } while (!exact && n % d == 0);
            if ((n % d == 0) != exact) fail("incorrect exact/inexact fixture category");
            a[i] = F::from_raw(x);
            b[i] = F::from_raw(y);
            c[i] = F::from_raw(z);
            expected[i] = oracle(n, d, mode);
        }
    }
};

template<Operation Op, Mode M, class F>
auto calculate(F a, F b, F c) {
    if constexpr (Op == Operation::mul)
        return fw::mul(a, b, M);
    else if constexpr (Op == Operation::div)
        return fw::div(a, b, M);
    else
        return fw::mul_div(a, b, c, M);
}
// Independent operations: the checksum depends on the results, but no result
// feeds another arithmetic input. Never label these rows dependency latency.
template<unsigned D, Operation Op, Mode M>
[[gnu::noinline]] std::uint64_t loop(const Fixture<D, Op>& f, std::uint64_t iterations) {
    std::uint64_t sum = 0;
    for (std::uint64_t i = 0; i < iterations; ++i) {
        const auto j = static_cast<std::size_t>(i) & fixture_mask;
        const auto result = calculate<Op, M>(f.a[j], f.b[j], f.c[j]);
        auto bits = result ? static_cast<std::uint64_t>(result->raw()) : UINT64_MAX;
        __asm__ volatile("" : "+r"(bits) : : "memory");
        sum += bits;
    }
    sink = sum;
    return sum;
}
struct Options {
    bool list = false;
    bool timing = false;
    std::string_view selected;
    std::uint64_t iterations = 262144;
    unsigned repetitions = 3;
};

template<unsigned D, Operation Op, Mode M>
bool visit(const Options& options) {
    bool found = false;
    const char* op = Op == Operation::mul ? "mul" : Op == Operation::div ? "div" : "mul_div";
    const char* mode = M == Mode::nearest_even ? "nearest_even" : "toward_zero";
    for (bool exact : {true, false})
        for (bool mixed : {false, true}) {
            const std::string name = std::string(op) + ".Fixed64_" + std::to_string(D) + "." + mode +
                                     (exact ? ".exact" : ".inexact") + (mixed ? ".mixed" : ".positive");
            if (options.list) {
                std::puts(name.c_str());
                found = true;
                continue;
            }
            if (!options.selected.empty() && options.selected != name) continue;
            found = true;
            const Fixture<D, Op> f(exact, mixed, M);
            for (std::size_t i = 0; i < fixture_size; ++i) {
                const auto result = calculate<Op, M>(f.a[i], f.b[i], f.c[i]);
                if (!result || result->raw() != f.expected[i]) fail(name.c_str());
                ++checks;
            }
            if (!options.timing) {
                std::printf("%llu\n", static_cast<unsigned long long>(loop<D, Op, M>(f, options.iterations)));
                continue;
            }
            loop<D, Op, M>(f, 4096);
            for (unsigned repeat = 0; repeat < options.repetitions; ++repeat) {
                const auto start = clock_ns();
                const auto checksum = loop<D, Op, M>(f, options.iterations);
                const auto elapsed = clock_ns() - start;
                std::printf("%u,%s,%s,%u,%llu,%.9f,%llu\n", D, name.c_str(), mode, repeat,
                            static_cast<unsigned long long>(options.iterations),
                            static_cast<double>(elapsed) / static_cast<double>(options.iterations),
                            static_cast<unsigned long long>(checksum));
            }
            std::fflush(stdout);
        }
    return found;
}
template<unsigned D>
bool visit_scale(const Options& o) {
    bool found = false;
    found |= visit<D, Operation::mul, Mode::toward_zero>(o);
    found |= visit<D, Operation::mul, Mode::nearest_even>(o);
    found |= visit<D, Operation::div, Mode::toward_zero>(o);
    found |= visit<D, Operation::div, Mode::nearest_even>(o);
    found |= visit<D, Operation::mul_div, Mode::toward_zero>(o);
    found |= visit<D, Operation::mul_div, Mode::nearest_even>(o);
    return found;
}
} // namespace

int main(int argc, char** argv) {
    Options o;
    if (argc == 2 && std::string_view(argv[1]) == "--list")
        o.list = true;
    else if (argc == 3 && std::string_view(argv[1]) != "--timing") {
        o.selected = argv[1];
        o.iterations = number(argv[2]);
    } else {
        if (argc < 2 || std::string_view(argv[1]) != "--timing")
            fail("use --list, <workload> <iterations>, or --timing [iterations] [repetitions]");
        o.timing = true;
        if (argc > 2) o.iterations = number(argv[2]);
        if (argc > 3) {
            auto n = number(argv[3]);
            if (n > 100) fail("too many repetitions");
            o.repetitions = static_cast<unsigned>(n);
        }
        if (argc > 4) fail("unexpected argument");
        std::puts("digits,workload,mode,repeat,iterations,cpu_ns_per_op,checksum");
    }
    const bool found8 = visit_scale<8>(o);
    const bool found12 = visit_scale<12>(o);
    if (!found8 && !found12) fail("unknown workload");
    if (!o.list) std::fprintf(stderr, "PASSED oracle_checks=%llu\n", static_cast<unsigned long long>(checks));
}
