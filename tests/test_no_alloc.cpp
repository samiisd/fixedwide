#include <fixedwide/all.hpp>
#include <new>
#include <cstdlib>
#include <atomic>
#include <cassert>
#include <iostream>

static std::atomic<int> g_alloc_count{0};

void* operator new(std::size_t size) {
    g_alloc_count.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size);
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    std::free(ptr);
}

void* operator new[](std::size_t size) {
    g_alloc_count.fetch_add(1, std::memory_order_relaxed);
    return std::malloc(size);
}

void operator delete[](void* ptr) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    std::free(ptr);
}

int main() {
    using namespace fixedwide;

    g_alloc_count.store(0);

    // 1. Same-domain arithmetic
    Fixed64<12> a = *from_integer<Fixed64<12>>(15);
    Fixed64<12> b = *from_integer<Fixed64<12>>(20);
    auto s = *add(a, b);
    auto m = *mul(a, b);
    auto d = *div(m, a);
    auto md = *mul_div(a, b, d);
    (void)s;
    (void)m;
    (void)d;
    (void)md;

    // 2. Wide arithmetic
    Fixed256<18> w1 = *from_integer<Fixed256<18>>(1234567);
    Fixed256<18> w2 = *from_integer<Fixed256<18>>(7654321);
    auto ws = *add(w1, w2);
    auto wm = *mul(w1, w2);
    auto wd = *div(wm, w1);
    (void)ws;
    (void)wm;
    (void)wd;

    // 3. Mixed arithmetic and casting
    Fixed32<4> m1 = *from_integer<Fixed32<4>>(123);
    auto mc = *fixed_cast<Fixed128<12>>(m1);
    auto m_mul = *mul_to<Fixed128<12>>(m1, a);
    (void)mc;
    (void)m_mul;

    // 4. Text parsing and formatting to stack buffer
    char buf[128];
    auto fmt_res = *to_chars(buf, sizeof(buf), a);
    auto parse_res = *from_chars<Fixed64<12>>(buf, buf + fmt_res);
    (void)parse_res;

    // 5. Binary serialization
    auto bytes = to_bytes(a);
    auto bin_res = *from_bytes<Fixed64<12>>(bytes);
    (void)bin_res;

    // 6. The inline floating conversion, including a 128-bit destination.
    volatile double input = 1.25;
    const auto f8 = from_float<Fixed64<8>>(input);
    const auto f10 = from_float<Fixed64<10>>(input);
    const auto f12 = from_float<Fixed64<12>>(input);
    const auto f128 = from_float<Fixed128<32>>(input);
    if (!f8 || !f10 || !f12 || !f128) return 1;

    volatile float input_float = 1.25f;
    volatile long double input_long_double = 1.25L;
    if (!from_float<Fixed64<12>>(input_float) || !from_float<Fixed128<38>>(input_float) ||
        !from_float<Fixed64<12>>(input_long_double) || !from_float<Fixed128<38>>(input_long_double))
        return 1;

    volatile float out_float = to_float<float>(*f128);
    volatile double out_double = to_double(*f128);
    volatile long double out_long_double = to_float<long double>(*f128);
    if (out_float != 1.25f || out_double != 1.25 || out_long_double != 1.25L) return 1;

    // 7. Accumulated success and sticky-error paths, including zero placeholders.
    Checked checked;
    const auto rate = checked.from_float<Fixed64<8>>(input);
    const auto product = checked.mul_to<Fixed128<12>>(a, rate);
    const auto sum = checked.add(product, Fixed128<12>{b});
    if (!checked.result(sum)) return 1;
    const auto failed = checked.add(Fixed64<12>::max(), Fixed64<12>::from_raw(1));
    const auto later = checked.div(a, failed);
    if (later != Fixed64<12>{} || checked.result(later).error() != ArithmeticError::overflow) return 1;

    // 8. Batch planning, independent blocks, a wide fallback and an accepted
    // prefix failure all retain the allocation-free arithmetic contract.
    using Money = Fixed128<12>;
    std::array<Money, 9> amounts{};
    amounts.fill(Money::max());
    std::array<Money, 9> converted{};
    const auto multiplier = batch::prepare_mul_to<Money>(Money{"0.1"}, Rounding::nearest_even);
    if (!batch::transform(std::span{converted}, multiplier, std::span{amounts})) return 1;
    std::array<Fixed256<24>, 9> wide_products{};
    if (!batch::mul_to<Fixed256<24>>(std::span{wide_products}, std::span{amounts}, Money{"100"}, Rounding::exact))
        return 1;
    const auto batch_failure = batch::add(std::span{converted}, std::span{amounts}, Money::from_raw(1));
    if (batch_failure || batch_failure.error().index != 0 || batch_failure.error().error != ArithmeticError::overflow)
        return 1;
    std::array<Money, 9> ones{};
    ones.fill(Money{"1"});
    const auto batch_sum = batch::reduce_add(std::span{ones});
    if (!batch_sum || *batch_sum != Money{"9"}) return 1;
    const auto exact_plan = batch::prepare_mul_to<Money>(Fixed64<10>::from_raw(37), Rounding::exact);
    if (!batch::transform<4>(std::span{converted}, exact_plan, std::span{ones})) return 1;
    Money total{};
    const auto committed = batch::for_each<4>(
        ones.size(), exact_plan,
        [&total](std::size_t, Money value) noexcept -> std::expected<void, ArithmeticError> {
            const auto next = add(total, value);
            if (!next) return std::unexpected(next.error());
            total = *next;
            return {};
        },
        std::span{ones});
    if (!committed || total != Money::from_raw(wide::int128{33'300})) return 1;
    ones[2] = Money::from_raw(wide::int128{1});
    const auto simd_failure = batch::transform<4>(std::span{converted}, exact_plan, std::span{ones});
    if (simd_failure || simd_failure.error().index != 2 || simd_failure.error().error != ArithmeticError::inexact)
        return 1;

    // Wide-grid products, including AVX2 rejection, prepared fallback and
    // public scalar recovery, must not introduce a persistent coefficient copy
    // or heap scratch. Keep the quote runtime-dependent in optimized builds.
    volatile std::int64_t quote_raw = 37;
    const auto quote = Fixed64<10>::from_raw(quote_raw);
    const wide::int128 coefficient = (wide::int128{1} << 90) + wide::int128{1};
    ones.fill(Money::from_raw(coefficient * wide::int128{10'000'000'000ULL}));
    const auto full_plan = batch::prepare_mul_to<Money>(quote, Rounding::exact);
    if (!batch::transform<4>(std::span{converted}, full_plan, std::span{ones})) return 1;
    const auto expected_wide = Money::from_raw(coefficient * wide::int128{quote.raw()});
    for (const auto value : converted)
        if (value != expected_wide) return 1;
    const auto full_scalar = mul_to<Money>(ones[0], quote, Rounding::exact);
    if (!full_scalar || *full_scalar != expected_wide) return 1;
    const wide::int128 largest_coefficient = Money::max().raw() / 10'000'000'000ULL;
    const auto largest_grid = Money::from_raw(largest_coefficient * wide::int128{10'000'000'000ULL});
    const auto wide_overflow = mul_to<Money>(largest_grid, Fixed64<10>{"2"}, Rounding::exact);
    if (wide_overflow || wide_overflow.error() != ArithmeticError::overflow) return 1;

    // Assert that NO allocations occurred
    int total_allocations = g_alloc_count.load();
    std::cout << "Total heap allocations during fixedwide operations: " << total_allocations << "\n";
    if (total_allocations != 0) {
        std::cerr << "FAIL: heap allocations occurred: " << total_allocations << "\n";
        return 1;
    }

    std::cout << "ZERO HEAP ALLOCATION VERIFIED!\n";
    return 0;
}
