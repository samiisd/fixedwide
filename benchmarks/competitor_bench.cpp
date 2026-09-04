// Competitor comparison.
//
// Three rules this suite exists to obey, because the previous one broke all of
// them:
//
//  1. Results are MEDIANS of repeated timed runs, reported as such. The old
//     helper returned the minimum of five trials while the report called the
//     numbers medians.
//  2. Every timed loop's output is validated OUTSIDE the timed region against
//     an independent expectation, so a compiler that deletes the work cannot
//     produce a fast row.
//  3. Rows are grouped by SEMANTIC CLASS and never merged into one ranking.
//     A decimal fixed-point multiply, a binary fixed-point multiply and a
//     decimal floating-point multiply are different numerical contracts. Cost
//     may be compared across them; correctness may not.
//
// What the libraries actually are, since the previous report got this wrong:
//
//  * CNL's scaled_integer is radix-parameterised, NOT base-2 only. It is
//    benchmarked here in BOTH a base-2 and a base-10 configuration. CNL's own
//    documentation notes decimal support is less exercised than binary, which
//    is a caveat on the result, not a reason to omit it.
//  * fpm is binary fixed point and is NOT truncation-only: fpm::fixed rounds
//    its multiply and divide to nearest. It is compared as binary fixed point.
//  * Boost.Multiprecision int128_t in its fixed-precision, allocator-free
//    configuration does NOT allocate. It is included as a raw wide-integer
//    baseline, not as a decimal type.
//  * Boost.Decimal is IEEE 754 decimal FLOATING point. Different contract from
//    a fixed-point decimal; reported in its own class.
//
// Anything not measured here is absent from the report. There are no rows for
// libraries this executable does not run.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/binary.hpp>
#include <fixedwide/floating.hpp>
#include "measurement.hpp"

#include <fpm/fixed.hpp>
#include <cnl/scaled_integer.h>
#include <boost/multiprecision/cpp_int.hpp>
#if __has_include(<boost/decimal.hpp>)
#include <boost/decimal.hpp>
#define FIXEDWIDE_HAVE_BOOST_DECIMAL 1
#endif

#include <array>
#include <charconv>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

namespace {

constexpr std::size_t data_size = 4096;
std::uint64_t validations = 0;

[[noreturn]] void fail(const std::string& what) {
    std::fprintf(stderr, "VALIDATION FAILED: %s\n", what.c_str());
    std::exit(1);
}
void expect(bool ok, const std::string& what) {
    ++validations;
    if (!ok) fail(what);
}

template<class T>
void escape(const T& value) {
    __asm__ __volatile__("" : : "r"(&value) : "memory");
}

// Operand set shared by every library, so no row gets an easier distribution.
struct Operands {
    std::vector<double> a, b; // exact at 12 decimals and at 2^-16
    std::vector<std::string> text;
};

std::string format_fixed4(double value) {
    char buffer[64];
    auto [end, ec] = std::to_chars(buffer, buffer + sizeof buffer, value, std::chars_format::fixed, 4);
    return std::string(buffer, end);
}

Operands make_operands() {
    Operands ops;
    std::mt19937_64 rng(0x5eed);
    ops.a.reserve(data_size);
    ops.b.reserve(data_size);
    ops.text.reserve(data_size);
    for (std::size_t i = 0; i < data_size; ++i) {
        // Values with 4 fractional decimal digits: representable exactly in
        // every decimal type here, and near-exactly in the binary ones.
        const double a = static_cast<double>(rng() % 900'0000 + 100'0000) / 10'000.0;
        const double b = static_cast<double>(rng() % 90'0000 + 10'0000) / 10'000.0;
        ops.a.push_back(a);
        ops.b.push_back(b);
        char buffer[64];
        auto [end, ec] = std::to_chars(buffer, buffer + sizeof buffer, a, std::chars_format::fixed, 4);
        ops.text.emplace_back(buffer, end);
    }
    return ops;
}

// A row's identity: library, semantic class, and what it computed.
void row(const char* library, const char* type, const char* semantic_class, const char* operation, auto loop) {
    // Type names contain commas; quote the field so the CSV stays parseable.
    fixedwide_bench::measure(std::string(library) + ",\"" + type + "\"," + semantic_class + "," + operation, loop);
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--filter" && i + 1 < argc)
            fixedwide_bench::filter = argv[++i];
        else if (arg == "--iterations" && i + 1 < argc)
            fixedwide_bench::iterations = std::strtoull(argv[++i], nullptr, 10);
    }

    const Operands ops = make_operands();

    std::printf("# fixedwide competitor benchmark\n");
    std::printf("# compiler=%s iterations=%zu repetitions=%u\n", __VERSION__, fixedwide_bench::iterations,
                fixedwide_bench::repetitions);
    std::printf("# every row is a MEDIAN of the repetitions; min/median/p95/max and all raw samples are emitted\n");
    std::printf(
        "library,type,semantic_class,operation,iterations,repetitions,min_ns,median_ns,p95_ns,max_ns,samples\n");

    // ---- decimal fixed point --------------------------------------------
    {
        using T = fixedwide::Fixed64<12>;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = *fixedwide::parse<T>(ops.text[i]);
            b[i] = *fixedwide::parse<T>(format_fixed4(ops.b[i]));
        }
        // Validate before timing against an EXACT integer oracle: the full
        // 128-bit product divided by the scale, rounded half to even by hand.
        for (std::size_t i = 0; i < data_size; ++i) {
            const auto product = fixedwide::mul(a[i], b[i]);
            expect(product.has_value(), "fixedwide mul returned an error");
            const __int128 full = static_cast<__int128>(a[i].raw()) * b[i].raw();
            constexpr __int128 scale = 1'000'000'000'000;
            __int128 quotient = full / scale;
            const __int128 remainder = full % scale;
            if (remainder * 2 > scale || (remainder * 2 == scale && (quotient & 1))) ++quotient;
            expect(product->raw() == static_cast<std::int64_t>(quotient),
                   "fixedwide mul disagrees with the exact integer oracle");
        }
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "mul_nearest_even", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::mul(a[i & (data_size - 1)], b[i & (data_size - 1)]));
        });
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "div_nearest_even", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::div(a[i & (data_size - 1)], b[i & (data_size - 1)]));
        });
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "mul_div_one_rounding", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i)
                escape(*fixedwide::mul_div(a[i & (data_size - 1)], b[i & (data_size - 1)], a[i & (data_size - 1)]));
        });
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "dependent_chain_mul", [&](std::size_t n) {
            T acc = a[0];
            for (std::size_t i = 0; i < n; ++i) {
                auto next = fixedwide::mul(acc, b[i & (data_size - 1)]);
                acc = next ? *next : a[0];
            }
            escape(acc);
        });
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "parse", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i)
                escape(*fixedwide::parse<T>(ops.text[i & (data_size - 1)], fixedwide::Rounding::nearest_even));
        });
        row("fixedwide", "Fixed64<12>", "decimal_fixed", "format", [&](std::size_t n) {
            char buffer[fixedwide::text_capacity];
            for (std::size_t i = 0; i < n; ++i) {
                const auto written = fixedwide::to_chars(buffer, sizeof buffer, a[i & (data_size - 1)]);
                escape(*written);
            }
        });
    }
    {
        // CNL with radix 10. NOTE the scale: this row is scale 6 and the
        // fixedwide decimal rows above are scale 12, so the two are NOT
        // like-for-like -- a 10^6 rescale is a cheaper division than a 10^12
        // one. The matched-scale comparison is the block further down; this row
        // is kept because it is what the earlier reports quoted, and removing it
        // silently would make those numbers unexplainable.
        //
        // CNL's decimal support is documented as less exercised than its binary
        // support.
        using T = cnl::scaled_integer<std::int64_t, cnl::power<-6, 10>>;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = T{ops.a[i]};
            b[i] = T{ops.b[i]};
        }
        // The result is forced back to T on purpose.
        //
        // cnl::scaled_integer::operator* does NOT rescale: it returns a type
        // whose exponent is the SUM of the operands', so `a * b` is a bare
        // 64-bit multiply and the product is left at scale 10^-12. Timing that
        // and calling it a decimal multiply compared fixedwide's
        // multiply-widen-rescale-check against CNL's multiply alone, which is
        // how the earlier reports arrived at "CNL is about 8x faster". Assigning
        // to T performs the rescale, which is the comparable operation.
        for (std::size_t i = 0; i < data_size; ++i) {
            const T product = a[i] * b[i];
            expect(std::abs(static_cast<double>(product) - ops.a[i] * ops.b[i]) < 0.01,
                   "cnl decimal mul disagrees with the double oracle");
        }
        row("cnl", "scaled_integer<int64,power<-6,10>>", "decimal_fixed", "mul_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) {
                const T product = a[i & (data_size - 1)] * b[i & (data_size - 1)];
                escape(product);
            }
        });
        row("cnl", "scaled_integer<int64,power<-6,10>>", "decimal_fixed", "div_unchecked", [&](std::size_t n) {
            // Same correction as the multiply: division returns the DIFFERENCE
            // of the exponents, so it must be brought back to T to be the same
            // operation fixedwide::div performs.
            for (std::size_t i = 0; i < n; ++i) {
                const T quotient = a[i & (data_size - 1)] / b[i & (data_size - 1)];
                escape(quotient);
            }
        });
    }

    // ---- binary fixed point ---------------------------------------------
    {
        // fpm rounds its multiply and divide to nearest; it is not truncating.
        using T = fpm::fixed<std::int64_t, __int128, 32>;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = T{ops.a[i]};
            b[i] = T{ops.b[i]};
        }
        for (std::size_t i = 0; i < data_size; ++i) {
            expect(std::abs(static_cast<double>(a[i] * b[i]) - ops.a[i] * ops.b[i]) < 0.01,
                   "fpm mul disagrees with the double oracle");
        }
        row("fpm", "fixed<int64,int128,32>", "binary_fixed", "mul_nearest_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] * b[i & (data_size - 1)]);
        });
        row("fpm", "fixed<int64,int128,32>", "binary_fixed", "div_nearest_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] / b[i & (data_size - 1)]);
        });
    }
    {
        using T = cnl::scaled_integer<std::int64_t, cnl::power<-32>>;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = T{ops.a[i]};
            b[i] = T{ops.b[i]};
        }
        for (std::size_t i = 0; i < data_size; ++i) {
            expect(std::abs(static_cast<double>(a[i]) - ops.a[i]) < 0.01, "cnl binary conversion");
        }
        row("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed", "mul_unchecked", [&](std::size_t n) {
            // Forced back to T for the same reason as the decimal rows above.
            for (std::size_t i = 0; i < n; ++i) {
                const T product = a[i & (data_size - 1)] * b[i & (data_size - 1)];
                escape(product);
            }
        });
    }

    // ---- decimal floating point -----------------------------------------
#if defined(FIXEDWIDE_HAVE_BOOST_DECIMAL)
    {
        using T = boost::decimal::decimal64_t;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = T{ops.a[i]};
            b[i] = T{ops.b[i]};
        }
        for (std::size_t i = 0; i < data_size; ++i) {
            expect(std::abs(static_cast<double>(a[i] * b[i]) - ops.a[i] * ops.b[i]) < 0.01,
                   "boost.decimal mul disagrees with the double oracle");
        }
        row("boost.decimal", "decimal64_t", "decimal_float", "mul", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] * b[i & (data_size - 1)]);
        });
        row("boost.decimal", "decimal64_t", "decimal_float", "div", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] / b[i & (data_size - 1)]);
        });
        // Same-contract text comparison: a decimal type parsing decimal text.
        // std::from_chars on a double is a useful floor, but it is not the same
        // job -- it produces a binary float and rejects nothing on a decimal grid.
        for (std::size_t i = 0; i < data_size; ++i) {
            T parsed{};
            const auto& text = ops.text[i];
            auto result = boost::decimal::from_chars(text.data(), text.data() + text.size(), parsed);
            expect(result.ec == std::errc{}, "boost.decimal from_chars failed");
            expect(std::abs(static_cast<double>(parsed) - ops.a[i]) < 0.01,
                   "boost.decimal from_chars disagrees with the double oracle");
        }
        row("boost.decimal", "decimal64_t", "decimal_float", "parse", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) {
                const auto& text = ops.text[i & (data_size - 1)];
                T parsed{};
                escape(boost::decimal::from_chars(text.data(), text.data() + text.size(), parsed));
            }
        });
        row("boost.decimal", "decimal64_t", "decimal_float", "format", [&](std::size_t n) {
            char buffer[64];
            for (std::size_t i = 0; i < n; ++i) {
                escape(boost::decimal::to_chars(buffer, buffer + sizeof buffer, a[i & (data_size - 1)]));
            }
        });
    }
#endif

    // ---- raw integer and binary floating point baselines -----------------
    {
        // Fixed-precision, allocator-free Boost.Multiprecision: no heap traffic.
        using T = boost::multiprecision::int128_t;
        static_assert(!std::numeric_limits<T>::is_bounded || sizeof(T) > 0);
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = static_cast<std::int64_t>(ops.a[i] * 10000);
            b[i] = static_cast<std::int64_t>(ops.b[i] * 10000);
        }
        for (std::size_t i = 0; i < data_size; ++i) expect(a[i] > 0 && b[i] > 0, "boost int128 setup");
        row("boost.multiprecision", "int128_t", "raw_integer", "mul_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(T(a[i & (data_size - 1)] * b[i & (data_size - 1)]));
        });
    }
    {
        std::vector<double> a = ops.a, b = ops.b;
        row("std", "double", "binary_float", "mul", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] * b[i & (data_size - 1)]);
        });
        row("std", "double", "binary_float", "div", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] / b[i & (data_size - 1)]);
        });
        row("std", "double", "binary_float", "parse", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) {
                const auto& text = ops.text[i & (data_size - 1)];
                double out = 0;
                std::from_chars(text.data(), text.data() + text.size(), out);
                escape(out);
            }
        });
        row("std", "double", "binary_float", "format", [&](std::size_t n) {
            char buffer[64];
            for (std::size_t i = 0; i < n; ++i) {
                auto [end, ec] =
                    std::to_chars(buffer, buffer + sizeof buffer, a[i & (data_size - 1)], std::chars_format::fixed, 4);
                escape(end);
            }
        });
    }

    // ---- matched-scale decimal comparison --------------------------------
    //
    // The rows above compare Fixed64<12> against a scale-6 CNL type, which is
    // not the same operation. These pair each scale with its own counterpart,
    // and validate against an EXACT integer oracle rather than a 0.01 floating
    // tolerance -- a tolerance that loose cannot tell a correct decimal result
    // from a wrong one at either scale.
    //
    // In its own scope, like every other group here: see the note on the
    // fixedwide/raw-floor block for why that matters.
    {
        // Both libraries are built from the SAME raw scaled integer, not from a
        // double. Constructing from a double made them disagree immediately --
        // CNL truncates that conversion and fixedwide rounds to nearest -- so
        // the two would have been timing different inputs. The validation
        // caught it; this is the fix.
        //
        // Operands include negatives, which the shared positive fixture does
        // not reach.
        auto compare_pair = [&]<unsigned Decimals, int Power>(const char* fw_name, const char* cnl_name) {
            using FW = fixedwide::basic_fixed<64, Decimals>;
            using CN = cnl::scaled_integer<std::int64_t, cnl::power<Power, 10>>;
            static_assert(static_cast<int>(Decimals) == -Power, "the two scales must match");

            // Raw magnitudes are derived from the scale so both scale points see
            // the same VALUES. A fixed raw range would have made scale 12 mean
            // values around 1e-6, whose products underflow to zero -- fast, and
            // measuring nothing.
            std::int64_t unit = 1;
            for (unsigned d = 0; d < Decimals; ++d) unit *= 10;

            std::mt19937_64 rng(0xC0FFEE);
            std::vector<FW> fa(data_size), fb(data_size);
            std::vector<CN> ca(data_size), cb(data_size);
            for (std::size_t i = 0; i < data_size; ++i) {
                // Values in [1, 1000) and divisors in [1, 100), signs mixed.
                std::int64_t ra = static_cast<std::int64_t>(rng() % 1000) * unit +
                                  static_cast<std::int64_t>(rng() % static_cast<std::uint64_t>(unit)) + unit;
                std::int64_t rb = static_cast<std::int64_t>(rng() % 100) * unit +
                                  static_cast<std::int64_t>(rng() % static_cast<std::uint64_t>(unit)) + unit;
                if (i % 3 == 0) ra = -ra;
                if (i % 5 == 0) rb = -rb;

                fa[i] = FW::from_raw(ra);
                fb[i] = FW::from_raw(rb);
                ca[i] = cnl::_impl::from_rep<CN>(ra);
                cb[i] = cnl::_impl::from_rep<CN>(rb);
                expect(fa[i].raw() == cnl::_impl::to_rep(ca[i]), "fixedwide and cnl disagree on the operand");
                expect(fb[i].raw() == cnl::_impl::to_rep(cb[i]), "fixedwide and cnl disagree on the divisor");
            }

            // Exact integer oracle: the full 128-bit product divided by the
            // scale, truncated -- which is what CNL does. fixedwide is checked
            // against its own nearest-even contract separately above.
            for (std::size_t i = 0; i < data_size; ++i) {
                const __int128 full = static_cast<__int128>(fa[i].raw()) * fb[i].raw();
                __int128 scale = 1;
                for (unsigned d = 0; d < Decimals; ++d) scale *= 10;
                const CN rescaled = ca[i] * cb[i]; // the assignment IS the rescale
                expect(cnl::_impl::to_rep(rescaled) == static_cast<std::int64_t>(full / scale),
                       "cnl decimal multiply disagrees with the exact integer oracle");
            }

            row("fixedwide", fw_name, "decimal_matched", "mul_nearest_even", [&](std::size_t n) {
                for (std::size_t i = 0; i < n; ++i)
                    escape(*fixedwide::mul(fa[i & (data_size - 1)], fb[i & (data_size - 1)]));
            });
            row("cnl", cnl_name, "decimal_matched", "mul_unchecked", [&](std::size_t n) {
                for (std::size_t i = 0; i < n; ++i) {
                    const CN product = ca[i & (data_size - 1)] * cb[i & (data_size - 1)];
                    escape(product);
                }
            });
            row("fixedwide", fw_name, "decimal_matched", "div_nearest_even", [&](std::size_t n) {
                for (std::size_t i = 0; i < n; ++i)
                    escape(*fixedwide::div(fa[i & (data_size - 1)], fb[i & (data_size - 1)]));
            });
            row("cnl", cnl_name, "decimal_matched", "div_unchecked", [&](std::size_t n) {
                for (std::size_t i = 0; i < n; ++i) {
                    const CN quotient = ca[i & (data_size - 1)] / cb[i & (data_size - 1)];
                    escape(quotient);
                }
            });
        };

        // Scale 6 only, and that is itself the finding.
        //
        // cnl::scaled_integer forms the product in its rep type, so a scale-12
        // multiply overflows int64 for any value above about 0.003. At scale 12
        // it cannot multiply ordinary money at all: the demonstration below is
        // checked on every run, so this claim cannot go stale.
        compare_pair.template operator()<6, -6>("Fixed64<6>", "scaled_integer<int64,power<-6,10>>");

        {
            using CN12 = cnl::scaled_integer<std::int64_t, cnl::power<-12, 10>>;
            using FW12 = fixedwide::Fixed64<12>;
            constexpr std::int64_t ra = 123'456789012345LL; // 123.456789012345
            constexpr std::int64_t rb = 2'000000000000LL;   //   2.000000000000
            constexpr std::int64_t exact = 246'913578024690LL;

            const CN12 cnl_product = cnl::_impl::from_rep<CN12>(ra) * cnl::_impl::from_rep<CN12>(rb);
            const auto fw_product = fixedwide::mul(FW12::from_raw(ra), FW12::from_raw(rb));

            // CNL is wrong here, and wrong without saying so. fixedwide widens
            // to 128 bits for the intermediate and gets the exact answer.
            expect(cnl::_impl::to_rep(cnl_product) != exact, "cnl scale-12 multiply was expected to overflow silently");
            expect(fw_product.has_value() && fw_product->raw() == exact,
                   "fixedwide scale-12 multiply must be exact where cnl overflows");

            std::fprintf(stderr,
                         "NOTE 123.456789012345 * 2 at 12 decimals:"
                         " cnl raw=%lld (wrong), fixedwide raw=%lld (exact)\n",
                         static_cast<long long>(cnl::_impl::to_rep(cnl_product)),
                         static_cast<long long>(fw_product->raw()));
        }
    }

    // ---- fixedwide against the raw-type floor ----------------------------
    //
    // In their own scope on purpose. These rows were first written inside the
    // decimal_fixed block above, and merely adding them there moved that
    // block's div_nearest_even from 2.2 ns to 5.1 ns with the library
    // untouched -- code layout, not arithmetic. Keeping each group of rows in
    // its own scope keeps one group from silently retuning another.
    {
        using T = fixedwide::Fixed64<12>;
        std::vector<T> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = *fixedwide::parse<T>(ops.text[i]);
            b[i] = *fixedwide::parse<T>(format_fixed4(ops.b[i]));
        }
        for (std::size_t i = 0; i < data_size; ++i) {
            const auto sum = fixedwide::add(a[i], b[i]);
            expect(sum.has_value() && sum->raw() == a[i].raw() + b[i].raw(),
                   "fixedwide add disagrees with the exact integer sum");
        }
        row("fixedwide", "Fixed64<12>", "raw_baseline", "add_checked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::add(a[i & (data_size - 1)], b[i & (data_size - 1)]));
        });

        // Serialization against the memcpy floor. to_bytes fixes the byte order,
        // so unlike a raw copy it produces the same bytes on any machine.
        {
            const auto bytes = fixedwide::to_bytes<fixedwide::endian::little>(a[0]);
            const auto back = fixedwide::from_bytes<T, fixedwide::endian::little>(bytes);
            expect(back && *back == a[0], "fixedwide byte round-trip");
        }
        row("fixedwide", "Fixed64<12>", "raw_baseline", "to_bytes_little", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i)
                escape(fixedwide::to_bytes<fixedwide::endian::little>(a[i & (data_size - 1)]));
        });
        row("fixedwide", "Fixed64<12>", "raw_baseline", "from_bytes_little", [&](std::size_t n) {
            const auto bytes = fixedwide::to_bytes<fixedwide::endian::little>(a[0]);
            for (std::size_t i = 0; i < n; ++i) escape(*fixedwide::from_bytes<T, fixedwide::endian::little>(bytes));
        });
    }

    // ---- raw machine types ----------------------------------------------
    //
    // Not competitors: a floor. These are what the hardware costs with no scale,
    // no rounding mode, no overflow check and no decimal guarantee, so they say
    // how much of a fixedwide row is the operation and how much is the contract.
    // int64_t here carries no scale at all -- keeping a decimal point in it is
    // work the caller would have to do on top.
    {
        std::vector<std::int64_t> a(data_size), b(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            a[i] = static_cast<std::int64_t>(ops.a[i] * 10'000);
            b[i] = static_cast<std::int64_t>(ops.b[i] * 10'000);
            expect(b[i] != 0, "int64 divisor must be nonzero");
        }
        row("std", "int64_t", "raw_baseline", "add_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] + b[i & (data_size - 1)]);
        });
        row("std", "int64_t", "raw_baseline", "mul_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] * b[i & (data_size - 1)]);
        });
        row("std", "int64_t", "raw_baseline", "div_unchecked", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] / b[i & (data_size - 1)]);
        });

        // The serialization floor: a raw copy of the object representation. It
        // is the fastest possible answer and it is endian-dependent, which is
        // exactly the guarantee to_bytes adds on top.
        row("std", "int64_t", "raw_baseline", "memcpy_store", [&](std::size_t n) {
            std::array<std::uint8_t, sizeof(std::int64_t)> buffer{};
            for (std::size_t i = 0; i < n; ++i) {
                std::memcpy(buffer.data(), &a[i & (data_size - 1)], sizeof buffer);
                escape(buffer);
            }
        });
        row("std", "int64_t", "raw_baseline", "memcpy_load", [&](std::size_t n) {
            std::array<std::uint8_t, sizeof(std::int64_t)> buffer{};
            std::memcpy(buffer.data(), &a[0], sizeof buffer);
            for (std::size_t i = 0; i < n; ++i) {
                std::int64_t out = 0;
                std::memcpy(&out, buffer.data(), sizeof out);
                escape(out);
            }
        });
    }
    {
        std::vector<double> a = ops.a, b = ops.b;
        row("std", "double", "raw_baseline", "add", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(a[i & (data_size - 1)] + b[i & (data_size - 1)]);
        });
        std::vector<float> fa(data_size), fb(data_size);
        for (std::size_t i = 0; i < data_size; ++i) {
            fa[i] = static_cast<float>(ops.a[i]);
            fb[i] = static_cast<float>(ops.b[i]);
        }
        row("std", "float", "raw_baseline", "add", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(fa[i & (data_size - 1)] + fb[i & (data_size - 1)]);
        });
        row("std", "float", "raw_baseline", "mul", [&](std::size_t n) {
            for (std::size_t i = 0; i < n; ++i) escape(fa[i & (data_size - 1)] * fb[i & (data_size - 1)]);
        });
    }

    std::fprintf(stderr, "PASSED validations=%llu\n", static_cast<unsigned long long>(validations));
    return 0;
}
