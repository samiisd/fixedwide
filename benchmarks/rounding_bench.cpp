// Paired rounding-mode benchmark with an independent, untimed Boost oracle.
// Small-value and chain fixtures derive from benchmarks/numeric_bench.cpp;
// wide fixtures derive from benchmarks/benchmark.cpp in that release.
// Unlike the original harness, the two rounding modes alternate within each
// timed pair. They execute the SAME loop body and compiled library functions.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>
#include <boost/multiprecision/cpp_int.hpp> // independent, UNTImED oracle only
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace fw = fixedwide;
using Big = boost::multiprecision::cpp_int;
using Mode = fw::Rounding;
constexpr std::array modes{Mode::toward_zero, Mode::nearest_even};
constexpr std::size_t data_size = 4096;
std::size_t iterations = 1048576;
unsigned repetitions = 9;
std::uint64_t order_seed = 1;
std::string filter;
std::uint64_t oracle_checks = 0;
std::mt19937_64 order_rng;

enum class Op { mul, div, mul_div, quantize4 };
const char* mode_name(Mode m) { return m == Mode::toward_zero ? "toward_zero" : "nearest_even"; }
const char* op_name(Op op) {
    switch (op) { case Op::mul: return "mul"; case Op::div: return "div";
    case Op::mul_div: return "mul_div"; case Op::quantize4: return "quantize4"; }
    std::abort();
}
[[noreturn]] void fail(const std::string& why) { std::fprintf(stderr, "FAILED: %s\n", why.c_str()); std::abort(); }
bool selected(const std::string& name) { return filter.empty() || name.find(filter) != std::string::npos; }
inline void escape(std::int64_t value) { __asm__ __volatile__("" : : "r"(value) : "memory"); }
inline void escape(fw::i128 value) {
    __asm__ __volatile__("" : : "r"(static_cast<std::uint64_t>(value)),
        "r"(static_cast<std::uint64_t>(static_cast<fw::u128>(value) >> 64)) : "memory");
}
inline void escape(fw::FP64 v) { escape(v.raw()); }
inline void escape(fw::FP128 v) { escape(v.raw()); }
template<class T, class E> T require(const std::expected<T, E>& result) {
    if (!result) fail("unexpected error in valid fixture");
    return *result;
}
// GNU noinline ensures the SAME loop address runs for both mode arguments;
// no specialization by the timing wrapper or code placement differences.
struct Timing { double wall_ns; double cpu_ns; };
std::uint64_t thread_cpu_ns() {
    timespec value{};
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &value) != 0) fail("thread CPU clock unavailable");
    return static_cast<std::uint64_t>(value.tv_sec)*1000000000ULL + static_cast<std::uint64_t>(value.tv_nsec);
}
template<class Loop> [[gnu::noinline]] Timing timed(Loop& loop, Mode mode, std::size_t count) {
    const auto cpu_start = thread_cpu_ns();
    const auto start = std::chrono::steady_clock::now();
    loop(count, mode);
    const auto end = std::chrono::steady_clock::now();
    const auto cpu_end = thread_cpu_ns();
    return {std::chrono::duration<double, std::nano>(end - start).count() / static_cast<double>(count),
            static_cast<double>(cpu_end - cpu_start) / static_cast<double>(count)};
}
template<class Loop> void measure_pair(const std::string& name, Loop loop) {
    if (!selected(name)) return;
    loop(8192, modes[0]); loop(8192, modes[1]);
    for (unsigned repeat = 0; repeat < repetitions; ++repeat) {
        // Balanced AB / BA, with the first order randomized per workload.
        const unsigned first = static_cast<unsigned>((order_seed + repeat) & 1u);
        for (unsigned slot = 0; slot < 2; ++slot) {
            const unsigned index = first ^ slot;
            const auto timing = timed(loop, modes[index], iterations);
            std::printf("%u,%s,%u,%u,%s,%zu,%.9f,%.9f\n", fw::fractional_digits, name.c_str(),
                        repeat, slot, mode_name(modes[index]), iterations, timing.wall_ns, timing.cpu_ns);
        }
    }
    order_seed ^= order_rng();
    std::fflush(stdout);
}
Big big(fw::i128 v) {
    const bool negative = v < 0;
    const fw::u128 mag = negative ? fw::u128{0} - static_cast<fw::u128>(v) : static_cast<fw::u128>(v);
    Big out = static_cast<std::uint64_t>(mag >> 64);
    out <<= 64; out += static_cast<std::uint64_t>(mag);
    return negative ? -out : out;
}
Big rational(Big numerator, Big denominator, Mode rounding) {
    const bool negative = (numerator < 0) != (denominator < 0);
    if (numerator < 0) numerator = -numerator;
    if (denominator < 0) denominator = -denominator;
    Big q = numerator / denominator;
    const Big r = numerator % denominator;
    // Oracle intentionally uses arbitrary-width 2*r, not the library's d-r.
    if (rounding == Mode::nearest_even && (2*r > denominator || (2*r == denominator && (q & 1) != 0))) ++q;
    return negative ? -q : q;
}
template<Op op, class T> T calculate(T a, T b, T c, Mode m) {
    if constexpr (op == Op::mul) return require(fw::mul(a, b, m));
    if constexpr (op == Op::div) return require(fw::div(a, b, m));
    if constexpr (op == Op::mul_div) return require(fw::mul_div(a, b, c, m));
    if constexpr (op == Op::quantize4) return require(fw::quantize(a, 4, m));
}
template<class T> const char* type_name() { return sizeof(T) == 8 ? "FP64" : "FP128"; }
template<class T> T from_raw(fw::i128 raw) { return T::from_raw(static_cast<decltype(T{}.raw())>(raw)); }
template<class T> struct Inputs {
    std::array<T, data_size> a, b, c;
};
template<class T> Inputs<T> typical() {
    Inputs<T> in;
    std::mt19937_64 rng(0x826041);
    for (std::size_t i = 0; i < data_size; ++i) {
        auto raw = static_cast<std::int64_t>(rng() % (1000 * fw::scale)) + fw::scale;
        if (i % 4 == 0) raw = -raw;
        in.a[i] = from_raw<T>(raw);
        in.b[i] = from_raw<T>(static_cast<std::int64_t>(rng() % (10 * fw::scale)) + fw::scale);
        in.c[i] = from_raw<T>(static_cast<std::int64_t>(rng() % (10 * fw::scale)) + fw::scale);
    }
    return in;
}
template<Op op, class T> Inputs<T> exact_or_ties(bool ties) {
    Inputs<T> in;
    std::mt19937_64 rng(0x300889);
    for (std::size_t i = 0; i < data_size; ++i) {
        fw::i128 q = static_cast<fw::i128>(rng() % (1000 * fw::scale)) + fw::scale;
        fw::i128 n = 2*q + static_cast<unsigned>(ties);
        if (i & 1) n = -n;
        // All fixtures have raw result n/2: exact if even, halfway if odd.
        in.a[i] = from_raw<T>(n);
        if constexpr (op == Op::mul) { in.b[i] = from_raw<T>(fw::scale/2); in.c[i] = from_raw<T>(fw::scale); }
        if constexpr (op == Op::div) { in.b[i] = from_raw<T>(2*fw::scale); in.c[i] = from_raw<T>(fw::scale); }
        if constexpr (op == Op::mul_div) {
            const auto raw = fw::scale + static_cast<std::int64_t>(rng() % fw::scale);
            in.b[i] = from_raw<T>(raw); in.c[i] = from_raw<T>(2*raw);
        }
    }
    return in;
}
template<Op op, class T> Big expected(T a, T b, T c, Mode m) {
    if constexpr (op == Op::mul) return rational(big(a.raw()) * big(b.raw()), Big(fw::scale), m);
    if constexpr (op == Op::div) return rational(big(a.raw()) * fw::scale, big(b.raw()), m);
    if constexpr (op == Op::mul_div) return rational(big(a.raw()) * big(b.raw()), big(c.raw()), m);
    if constexpr (op == Op::quantize4) {
        std::int64_t divisor = fw::scale / 10000;
        return rational(big(a.raw()), Big(divisor), m) * divisor;
    }
}
template<Op op, class T> void throughput(const char* group, const Inputs<T>& inputs, std::size_t size=data_size) {
    const auto name = std::string(group) + "." + type_name<T>() + "." + op_name(op);
    if (!selected(name)) return;
    std::size_t changed = 0, exact_count = 0, tie_count = 0;
    for (std::size_t j=0; j<size; ++j) {
        const auto& a=inputs.a[j]; const auto& b=inputs.b[j]; const auto& c=inputs.c[j];
        for (Mode mode : modes) {
            const auto value = calculate<op>(a,b,c,mode);
            if (big(value.raw()) != expected<op>(a,b,c,mode)) fail(name + " oracle mismatch");
            ++oracle_checks;
        }
        if (calculate<op>(a,b,c,modes[0]) != calculate<op>(a,b,c,modes[1])) ++changed;
        Big n, d;
        if constexpr (op == Op::mul) { n=big(a.raw())*big(b.raw()); d=fw::scale; }
        if constexpr (op == Op::div) { n=big(a.raw())*fw::scale; d=big(b.raw()); }
        if constexpr (op == Op::mul_div) { n=big(a.raw())*big(b.raw()); d=big(c.raw()); }
        if constexpr (op == Op::quantize4) { n=big(a.raw()); d=fw::scale/10000; }
        if (n<0) n=-n; if (d<0) d=-d;
        const Big r=n%d;
        exact_count += r==0; tie_count += 2*r==d;
    }
    std::fprintf(stderr,"fixture=%s rows=%zu exact=%zu ties=%zu mode_changed=%zu\n", name.c_str(), size, exact_count, tie_count, changed);
    measure_pair(name, [&](std::size_t count, Mode mode) {
        for (std::size_t i=0; i<count; ++i) {
            const auto j = i & (size-1);
            escape(calculate<op>(inputs.a[j],inputs.b[j],inputs.c[j],mode));
        }
    });
}
template<Op op, class T> void chain(bool exact) {
    const auto name = std::string(exact ? "exact_chain." : "inexact_chain.") + type_name<T>() + "." + op_name(op);
    if (!selected(name)) return;
    constexpr std::size_t chain_size=256;
    std::array<T, chain_size> factors{}, denominators{};
    std::mt19937_64 rng(0x3001208);
    for (std::size_t i=0; i<chain_size; i+=2) {
        if (exact) {
            factors[i]=from_raw<T>(2*fw::scale); factors[i+1]=from_raw<T>(fw::scale/2);
            denominators[i]=denominators[i+1]=from_raw<T>(fw::scale);
        } else {
            const auto f=fw::scale + static_cast<std::int64_t>(rng() % (fw::scale/100)) + 1;
            if constexpr (op == Op::mul_div) {
                const auto d=fw::scale + static_cast<std::int64_t>(rng() % (fw::scale/100)) + 1;
                factors[i]=from_raw<T>(f); denominators[i]=from_raw<T>(d);
                factors[i+1]=from_raw<T>(d); denominators[i+1]=from_raw<T>(f);
            } else {
                const auto inv=static_cast<std::int64_t>((fw::i128{fw::scale}*fw::scale)/f);
                factors[i]=from_raw<T>(f); factors[i+1]=from_raw<T>(inv);
                denominators[i]=denominators[i+1]=from_raw<T>(fw::scale);
            }
        }
    }
    for (Mode mode : modes) {
        T state=from_raw<T>(fw::scale+fw::scale/4);
        const auto start=state;
        std::size_t exact_steps=0,tie_steps=0;
        for (std::size_t i=0; i<iterations; ++i) {
            const auto j=i&(chain_size-1);
            // First 4096 steps checked independently; EVERY step stays nonzero
            // and within a broad bounded range over the full timed chain length.
            Big want;
            if (i<4096) {
                want=expected<op>(state,factors[j],denominators[j],mode);
                Big n=big(state.raw())*(op==Op::div ? Big(fw::scale) : big(factors[j].raw()));
                Big d=op==Op::mul ? Big(fw::scale) : op==Op::div ? big(factors[j].raw()) : big(denominators[j].raw());
                const Big r=n%d;
                exact_steps+=r==0; tie_steps+=2*r==d;
            }
            state=calculate<op>(state,factors[j],denominators[j],mode);
            if (i<4096) { if (big(state.raw())!=want) fail(name+" chain oracle mismatch"); ++oracle_checks; }
            if (state.raw() < fw::scale/4 || state.raw() > 4*fw::scale) fail(name+" unbounded chain");
        }
        std::fprintf(stderr,"fixture=%s mode=%s first4096_exact=%zu first4096_ties=%zu start_raw=%lld final_raw=%lld steps=%zu\n",
            name.c_str(),mode_name(mode),exact_steps,tie_steps,static_cast<long long>(start.raw()),static_cast<long long>(state.raw()),iterations);
    }
    measure_pair(name,[&](std::size_t count,Mode mode) {
        T state=from_raw<T>(fw::scale+fw::scale/4);
        for (std::size_t i=0; i<count; ++i) {
            const auto j=i&(chain_size-1);
            state=calculate<op>(state,factors[j],denominators[j],mode);
            escape(state);
        }
    });
}
template<class T> void text() {
    const auto input=typical<T>();
    std::array<std::string, data_size> texts;
    for (std::size_t i=0; i<data_size; ++i) {
        char buf[64]; auto n=require(fw::to_chars(buf,sizeof buf,input.a[i]));
        texts[i]=std::string(buf,n)+static_cast<char>('0'+(i%10));
    }
    const auto parse_name=std::string("parse_extra_digit.")+type_name<T>();
    const auto format_name=std::string("format_2digits.")+type_name<T>();
    if (selected(parse_name)) {
        for (std::size_t i=0; i<data_size; ++i) for (Mode mode:modes) {
            T parsed;
            if constexpr(sizeof(T)==8) parsed=require(fw::parse64(texts[i],mode));
            else parsed=require(fw::parse128(texts[i],mode));
            Big n=big(input.a[i].raw())*10;
            if(n<0) n-=i%10; else n+=i%10;
            if(big(parsed.raw())!=rational(n,Big(10),mode)) fail(parse_name+" oracle mismatch");
            ++oracle_checks;
        }
        measure_pair(parse_name,[&](std::size_t count, Mode mode) {
            for(std::size_t i=0;i<count;++i) {
                const auto j=i&(data_size-1);
                if constexpr(sizeof(T)==8) escape(require(fw::parse64(texts[j],mode)));
                else escape(require(fw::parse128(texts[j],mode)));
            }
        });
    }
    if (selected(format_name)) {
        for(std::size_t i=0;i<data_size;++i) for(Mode mode:modes) {
            char buf[64];
            auto n=require(fw::to_chars(buf,sizeof buf,input.a[i],{.digits=2,.rounding=mode}));
            T parsed;
            if constexpr(sizeof(T)==8) parsed=require(fw::parse64(std::string_view(buf,n)));
            else parsed=require(fw::parse128(std::string_view(buf,n)));
            const auto divisor=fw::scale/100;
            if(big(parsed.raw())!=rational(big(input.a[i].raw()),Big(divisor),mode)*divisor) fail(format_name+" oracle mismatch");
            ++oracle_checks;
        }
        measure_pair(format_name,[&](std::size_t count,Mode mode) {
            for(std::size_t i=0;i<count;++i) {
                char buf[64];
                auto n=require(fw::to_chars(buf,sizeof buf,input.a[i&(data_size-1)],{.digits=2,.rounding=mode}));
                __asm__ __volatile__("" : : "m"(buf),"r"(n) : "memory");
            }
        });
    }
}
template<Op op,class T> void small_arithmetic() {
    const auto inputs=typical<T>();
    throughput<op>("throughput4096",inputs);
    throughput<op>("throughput256",inputs,256);
    throughput<op>("exact_results",exact_or_ties<op,T>(false));
    throughput<op>("halfway_ties",exact_or_ties<op,T>(true));
    chain<op,T>(true); chain<op,T>(false);
}
void wide_arithmetic() {
    Inputs<fw::FP128> mul,native64,native128,wide64,wide128,md;
    std::mt19937_64 rng(0x4b209cd5);
    for(std::size_t i=0;i<data_size;++i) {
        const fw::i128 a=(fw::i128{1}<<84)+rng(), b=(fw::i128{1}<<64)+rng();
        const fw::i128 large=(fw::i128{1}<<120)+rng(), d=(fw::i128{1}<<96)+rng();
        const fw::i128 small=fw::scale+static_cast<std::int64_t>(rng()%(10*fw::scale));
        const auto set=[&](auto& in,fw::i128 x,fw::i128 y,fw::i128 z) {
            in.a[i]=from_raw<fw::FP128>(x); in.b[i]=from_raw<fw::FP128>(y); in.c[i]=from_raw<fw::FP128>(z);
        };
        set(mul,a,b,fw::scale); set(native64,a,small,fw::scale); set(native128,a,d,fw::scale);
        set(wide64,large,small,fw::scale); set(wide128,large,d,fw::scale); set(md,a,b,d);
    }
    throughput<Op::mul>("wide_product",mul);
    throughput<Op::div>("native_by64",native64);
    throughput<Op::div>("native_by128",native128);
    throughput<Op::div>("wide_by64",wide64);
    throughput<Op::div>("wide_by128",wide128);
    throughput<Op::mul_div>("wide_product",md);
}
void wide_output_product(bool full_range) {
    const std::string name = full_range ? "fullrange.FP64.mul_wide" : "throughput4096.FP64.mul_wide";
    if (!selected(name)) return;
    auto inputs = typical<fw::FP64>();
    if (full_range) {
        std::mt19937_64 rng(0x48283);
        for (std::size_t i=0; i<data_size; ++i) {
            inputs.a[i] = fw::FP64::from_raw(static_cast<std::int64_t>(rng()));
            inputs.b[i] = fw::FP64::from_raw(static_cast<std::int64_t>(rng()));
        }
    }
    for (std::size_t i=0; i<data_size; ++i) for (Mode mode : modes) {
        const auto value = require(fw::mul_wide(inputs.a[i], inputs.b[i], mode));
        if (big(value.raw()) != expected<Op::mul>(inputs.a[i], inputs.b[i], inputs.c[i], mode))
            fail(name + " oracle mismatch");
        ++oracle_checks;
    }
    measure_pair(name, [&](std::size_t count, Mode mode) {
        for (std::size_t i=0; i<count; ++i) {
            const auto j = i & (data_size-1);
            escape(require(fw::mul_wide(inputs.a[j], inputs.b[j], mode)));
        }
    });
}
int main(int argc,char** argv) {
    for(int i=1;i<argc;++i) {
        const std::string_view a=argv[i];
        if(a=="--filter"&&i+1<argc) { filter=argv[++i]; continue; }
        if(i+1>=argc) return 2;
        std::uint64_t value=0; const std::string_view s=argv[++i];
        auto [p,ec]=std::from_chars(s.data(),s.data()+s.size(),value);
        if(ec!=std::errc{}||p!=s.data()+s.size()) return 2;
        if(a=="--iterations") iterations=value;
        else if(a=="--repetitions"&&value<=1000) repetitions=static_cast<unsigned>(value);
        else if(a=="--seed") order_seed=value;
        else return 2;
    }
    if(iterations<4096||iterations>16777216||repetitions<3) return 2;
    order_rng.seed(order_seed);
    std::fprintf(stderr,"paired_rounding_benchmark;compiler=%s;digits=%u;iterations=%zu;pairs=%u;seed=%llu\n",
        __VERSION__,fw::fractional_digits,iterations,repetitions,static_cast<unsigned long long>(order_seed));
    std::puts("digits,workload,pair_index,order_slot,mode,iterations,ns_per_op,cpu_ns_per_op");
    small_arithmetic<Op::mul,fw::FP64>(); small_arithmetic<Op::div,fw::FP64>(); small_arithmetic<Op::mul_div,fw::FP64>();
    small_arithmetic<Op::mul,fw::FP128>(); small_arithmetic<Op::div,fw::FP128>(); small_arithmetic<Op::mul_div,fw::FP128>();
    wide_arithmetic();
    wide_output_product(false); wide_output_product(true);
    throughput<Op::quantize4>("throughput4096",typical<fw::FP64>());
    throughput<Op::quantize4>("throughput4096",typical<fw::FP128>());
    text<fw::FP64>(); text<fw::FP128>();
    std::fprintf(stderr,"PASSED oracle_checks=%llu\n",static_cast<unsigned long long>(oracle_checks));
}
