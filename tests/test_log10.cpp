#include <fixedwide/log10.hpp>
#include <cfenv>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace fw = fixedwide;
using U = fw::wide::uint256;

namespace {
std::size_t checks = 0;

void require(bool condition, int line) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "log10 check failed at line %d\n", line);
        std::abort();
    }
}
#define CHECK(...) require((__VA_ARGS__), __LINE__)

// Independent base-10 oracle: long division using eight 32-bit digits.
// It shares neither the bit-width estimate nor the power table with the API.
constexpr U divide_by_ten(U value) {
    std::uint64_t remainder = 0;
    for (std::size_t i = 4; i != 0; --i) {
        const auto high = (remainder << 32) | (value.limbs[i - 1] >> 32);
        remainder = high % 10;
        const auto low = (remainder << 32) | (value.limbs[i - 1] & 0xffff'ffffULL);
        value.limbs[i - 1] = ((high / 10) << 32) | (low / 10);
        remainder = low % 10;
    }
    return value;
}

constexpr int reference(U value) {
    int exponent = -1;
    do {
        value = divide_by_ten(value);
        ++exponent;
    } while (!value.is_zero());
    return exponent;
}

template<class F>
constexpr F from_unsigned(U value) {
    typename F::raw_type raw{};
    if constexpr (F::bits <= 64) {
        raw = static_cast<typename F::raw_type>(value.limbs[0]);
    } else if constexpr (F::bits == 128) {
        raw.low = value.limbs[0];
        raw.high = value.limbs[1];
    } else {
        for (std::size_t i = 0; i < 4; ++i) raw.limbs[i] = value.limbs[i];
    }
    return F::from_raw(raw);
}

template<std::size_t Bits>
constexpr U maximum() {
    return (U(1) << static_cast<unsigned>(Bits - 1)) - U(1);
}

template<class F>
constexpr bool contract() {
    static_assert(std::is_same_v<decltype(fw::log10_floor(F{})), std::expected<int, fw::ArithmeticError>>);
    static_assert(noexcept(fw::log10_floor(F{})));
    const auto zero = fw::log10_floor(F{});
    const auto negative = fw::log10_floor(F::min());
    const auto one_raw = fw::log10_floor(F::from_raw(typename F::raw_type(1)));
    const auto max = fw::log10_floor(F::max());
    return !zero && zero.error() == fw::ArithmeticError::invalid_value && !negative &&
           negative.error() == fw::ArithmeticError::invalid_value && one_raw &&
           *one_raw == -static_cast<int>(F::fractional_digits) && max &&
           *max == reference(maximum<F::bits>()) - static_cast<int>(F::fractional_digits);
}

static_assert(contract<fw::Fixed8<2>>());
static_assert(contract<fw::Fixed16<4>>());
static_assert(contract<fw::Fixed32<9>>());
static_assert(contract<fw::Fixed64<18>>());
static_assert(contract<fw::Fixed128<38>>());
static_assert(contract<fw::Fixed256<76>>());
static_assert(*fw::log10_floor(fw::Fixed64<2>::from_raw(1)) == -2);
static_assert(*fw::log10_floor(fw::Fixed64<2>::from_raw(99)) == -1);
static_assert(*fw::log10_floor(fw::Fixed64<2>::from_raw(100)) == 0);
static_assert(*fw::log10_floor(fw::Fixed64<0>::from_raw(999'999'999'999'999'999LL)) == 17);

template<class T>
concept HasLog10Floor = requires(T value) { fw::log10_floor(value); };
static_assert(!HasLog10Floor<int> && !HasLog10Floor<double> && !HasLog10Floor<long double>);

template<class F>
void check(U raw, int exponent) {
    const auto result = fw::log10_floor(from_unsigned<F>(raw));
    CHECK(result && *result == exponent - static_cast<int>(F::fractional_digits));
}

template<std::size_t Bits, std::size_t... D>
void all_scales(U raw, std::index_sequence<D...>) {
    const int exponent = reference(raw);
    (check<fw::basic_fixed<Bits, static_cast<unsigned>(D)>>(raw, exponent), ...);
}

template<std::size_t Bits>
void boundary(U raw) {
    if (raw.is_zero() || raw > maximum<Bits>()) return;
    all_scales<Bits>(raw, std::make_index_sequence<fw::max_decimals_for_bits<Bits>() + 1>{});
}

std::uint64_t random_word(std::uint64_t& state) {
    state += 0x9e3779b97f4a7c15ULL;
    auto z = state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

template<std::size_t Bits>
void test_width() {
    using F = fw::basic_fixed<Bits, 0>;
    using Scaled = fw::basic_fixed<Bits, fw::max_decimals_for_bits<Bits>()>;
    CHECK(contract<F>());
    CHECK(contract<Scaled>());
    for (const auto value : {F{}, F::min(), F::from_raw(typename F::raw_type(-1))}) {
        const auto result = fw::log10_floor(value);
        CHECK(!result && result.error() == fw::ArithmeticError::invalid_value);
    }
    boundary<Bits>(maximum<Bits>());
    for (unsigned b = 0; b < Bits - 1; ++b) {
        const auto power = U(1) << b;
        boundary<Bits>(power - U(1));
        boundary<Bits>(power);
        boundary<Bits>(power + U(1));
    }
    // Includes the two sides of every bin used by the integer estimate.
    for (unsigned b = 1; b < Bits; ++b) boundary<Bits>((U(1) << b) - U(1));
    for (U power(1); power <= maximum<Bits>();) {
        boundary<Bits>(power - U(1));
        boundary<Bits>(power);
        boundary<Bits>(power + U(1));
        if (power > divide_by_ten(maximum<Bits>())) break;
        power = (power << 3) + (power << 1);
    }
    std::uint64_t state = 0x4c6f673130ULL + Bits;
    for (unsigned i = 0; i < 4000; ++i) {
        U raw;
        for (auto& limb : raw.limbs) limb = random_word(state);
        raw = raw & maximum<Bits>();
        if ((i & 1U) != 0) raw = raw >> static_cast<unsigned>(random_word(state) % (Bits - 1));
        if (raw.is_zero()) raw = U(1);
        const int expected = reference(raw);
        check<F>(raw, expected);
        check<Scaled>(raw, expected);
        const auto positive = from_unsigned<F>(raw);
        const auto negative = F::from_raw(static_cast<typename F::raw_type>(-positive.raw()));
        const auto result = fw::log10_floor(negative);
        CHECK(!result && result.error() == fw::ArithmeticError::invalid_value);
    }
}

template<std::size_t Bits>
void exhaustive() {
    for (std::uint64_t raw = 1; raw < (std::uint64_t{1} << (Bits - 1)); ++raw) boundary<Bits>(U(raw));
}

} // namespace

int main() {
    exhaustive<8>();
    exhaustive<16>();
    test_width<8>();
    test_width<16>();
    test_width<32>();
    test_width<64>();
    test_width<128>();
    test_width<256>();
    const int saved = std::fegetround();
    for (const int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        if (std::fesetround(mode) == 0) {
            volatile std::int64_t raw = 999'999'999'999'999'999LL;
            CHECK(*fw::log10_floor(fw::Fixed64<0>::from_raw(raw)) == 17);
        }
    }
    if (saved != -1) CHECK(std::fesetround(saved) == 0);
    std::printf("log10_floor: %zu checks OK\n", checks);
}
