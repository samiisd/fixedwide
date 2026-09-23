#include "check.hpp"
#include <fixedwide/literal.hpp>
#include <fixedwide/chars.hpp>
#include <array>
#include <string>
#include <string_view>
#include <type_traits>

namespace {
namespace fw = fixedwide;
using Money = fw::Fixed64<2>;

template<class T>
concept HasLiteral = requires { fw::literal<T>("1"); };
static_assert(HasLiteral<Money>);
static_assert(!HasLiteral<int> && !HasLiteral<double>);
static_assert(std::is_same_v<decltype(fw::literal<Money>("1.25")), Money>);
static_assert(noexcept(fw::literal<Money>("1.25")));
static_assert(fw::literal<Money>("1.25").raw() == 125);
static_assert(fw::literal<Money>("+001.2500").raw() == 125);
static_assert(fw::literal<Money>("125e-2").raw() == 125);
static_assert(fw::literal<Money>(".25").raw() == 25);
static_assert(fw::literal<Money>("1.").raw() == 100);
static_assert(fw::literal<Money>("-0.00") == Money{});
static_assert(fw::literal<Money>("0e999999999999999999999999") == Money{});
static_assert(fw::literal<Money>("0e-999999999999999999999999") == Money{});
constexpr char named[] = "-1.25";
static_assert(fw::literal<Money>(named).raw() == -125);

template<class T, std::size_t N, std::size_t M>
consteval bool boundaries(const char (&minimum)[N], const char (&maximum)[M]) {
    return fw::literal<T>(minimum) == T::min() && fw::literal<T>(maximum) == T::max();
}

static_assert(boundaries<fw::Fixed8<0>>("-128", "127"));
static_assert(boundaries<fw::Fixed8<2>>("-1.28", "1.27"));
static_assert(boundaries<fw::Fixed16<0>>("-32768", "32767"));
static_assert(boundaries<fw::Fixed16<4>>("-3.2768", "3.2767"));
static_assert(boundaries<fw::Fixed32<0>>("-2147483648", "2147483647"));
static_assert(boundaries<fw::Fixed32<9>>("-2.147483648", "2.147483647"));
static_assert(boundaries<fw::Fixed64<0>>("-9223372036854775808", "9223372036854775807"));
static_assert(boundaries<fw::Fixed64<8>>("-92233720368.54775808", "92233720368.54775807"));
static_assert(boundaries<fw::Fixed64<18>>("-9.223372036854775808", "9.223372036854775807"));
static_assert(boundaries<fw::Fixed128<0>>("-170141183460469231731687303715884105728",
                                        "170141183460469231731687303715884105727"));
static_assert(boundaries<fw::Fixed128<38>>("-1.70141183460469231731687303715884105728",
                                         "1.70141183460469231731687303715884105727"));
static_assert(boundaries<fw::Fixed256<0>>("-57896044618658097711785492504343953926634992332820282019728792003956564819968",
                                        "57896044618658097711785492504343953926634992332820282019728792003956564819967"));
static_assert(boundaries<fw::Fixed256<76>>("-5.7896044618658097711785492504343953926634992332820282019728792003956564819968",
                                         "5.7896044618658097711785492504343953926634992332820282019728792003956564819967"));

// Each initializer gets its own constant-evaluation budget, also on MSVC and
// Clang. Exercise every supported width/scale, not only the common aliases.
template<std::size_t Bits, unsigned D>
void all_scales() {
    using T = fw::basic_fixed<Bits, D>;
    constexpr auto one = fw::literal<T>("1");
    constexpr auto negative_one = fw::literal<T>("-1");
    static_assert(one.raw() == T::scale());
    static_assert(negative_one.raw() == -T::scale());
    static_assert(fw::literal<T>("0") == T{});
    const auto text = "17e-" + std::to_string(D);
    const auto expected = T::from_raw(static_cast<typename T::raw_type>(17));
    CHECK(fw::parse<T>(text) == expected);
    CHECK(fw::detail::parse_literal_exact<T>(text) == expected);
    if constexpr (D != 0) all_scales<Bits, D - 1>();
}

// Public literals build this table during translation. Runtime comparisons
// below use an independent raw-integer oracle, including both signed limits.
consteval auto small_grid() {
    std::array<fw::Fixed8<2>, 256> out{};
    for (int raw = -128; raw <= 127; ++raw) {
        const int mag = raw < 0 ? -raw : raw;
        const char text[]{raw < 0 ? '-' : '+', static_cast<char>('0' + mag / 100), '.',
                          static_cast<char>('0' + (mag / 10) % 10), static_cast<char>('0' + mag % 10), '\0'};
        out[static_cast<std::size_t>(raw + 128)] = fw::literal<fw::Fixed8<2>>(text);
    }
    return out;
}
constexpr auto grid = small_grid();

template<class T>
void compare(std::string_view text) {
    const auto constant_path = fw::detail::parse_literal_exact<T>(text);
    const auto runtime_path = fw::parse<T>(text, fw::Rounding::exact);
    CHECK(constant_path == runtime_path);
}

template<class T>
void differential() {
    constexpr std::string_view inputs[]{
        "", "+", "-", ".", "+.", "1.2.3", "1e", "1e+", "1e-", "1e2e3", "0x10", "nan", "inf",
        " 1", "1 ", "1\n", "1'000", "0", "-0", "+0.00", "00.000e9999999999999999999",
        "0e-99999999999999999999", "1", "-1", "+001.2500", ".5", "-.5", "1.", "1.e2", "1E+2",
        "1e99999999999999999999", "1e-99999999999999999999", "1e00000000000000000001",
        "127", "128", "-128", "-129", "1.27", "1.28", "-1.28", "-1.29", "1.271", "1.281",
        "9223372036854775807", "9223372036854775808", "-9223372036854775808", "-9223372036854775809",
        "170141183460469231731687303715884105727", "170141183460469231731687303715884105728",
        "-170141183460469231731687303715884105728", "-170141183460469231731687303715884105729",
        "57896044618658097711785492504343953926634992332820282019728792003956564819967",
        "57896044618658097711785492504343953926634992332820282019728792003956564819968",
        "-57896044618658097711785492504343953926634992332820282019728792003956564819968",
        "-57896044618658097711785492504343953926634992332820282019728792003956564819969"
    };
    for (auto text : inputs) compare<T>(text);
    compare<T>(std::string_view{"1\0junk", 6});
    compare<T>(std::string(4096, '0'));
    compare<T>(std::string(4097, '0'));
    compare<T>("1." + std::string(4094, '0'));
    compare<T>("1." + std::string(4093, '0') + "1");
    for (int raw = -30; raw <= 30; ++raw) {
        for (int exponent = -2; exponent <= 2; ++exponent) {
            const auto text = std::to_string(raw) + "e" +
                              std::to_string(exponent - static_cast<int>(T::fractional_digits));
            compare<T>(text);
        }
    }
}
} // namespace

int main() {
    all_scales<8, 2>();
    all_scales<16, 4>();
    all_scales<32, 9>();
    all_scales<64, 18>();
    all_scales<128, 38>();
    all_scales<256, 76>();
    for (int raw = -128; raw <= 127; ++raw) {
        CHECK(grid[static_cast<std::size_t>(raw + 128)].raw() == raw);
    }
    differential<fw::Fixed8<0>>();
    differential<fw::Fixed8<2>>();
    differential<fw::Fixed16<0>>();
    differential<fw::Fixed16<4>>();
    differential<fw::Fixed32<0>>();
    differential<fw::Fixed32<9>>();
    differential<fw::Fixed64<0>>();
    differential<fw::Fixed64<8>>();
    differential<fw::Fixed64<18>>();
    differential<fw::Fixed128<0>>();
    differential<fw::Fixed128<38>>();
    differential<fw::Fixed256<0>>();
    differential<fw::Fixed256<76>>();
    std::printf("literal: %llu checks passed\n", static_cast<unsigned long long>(checks));
}
