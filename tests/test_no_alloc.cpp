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
    (void)s; (void)m; (void)d; (void)md;

    // 2. Wide arithmetic
    Fixed256<18> w1 = *from_integer<Fixed256<18>>(1234567);
    Fixed256<18> w2 = *from_integer<Fixed256<18>>(7654321);
    auto ws = *add(w1, w2);
    auto wm = *mul(w1, w2);
    auto wd = *div(wm, w1);
    (void)ws; (void)wm; (void)wd;

    // 3. Mixed arithmetic and casting
    Fixed32<4> m1 = *from_integer<Fixed32<4>>(123);
    auto mc = *fixed_cast<Fixed128<12>>(m1);
    auto m_mul = *mul_to<Fixed128<12>>(m1, a);
    (void)mc; (void)m_mul;

    // 4. Text parsing and formatting to stack buffer
    char buf[128];
    auto fmt_res = *to_chars(buf, sizeof(buf), a);
    auto parse_res = *from_chars<Fixed64<12>>(buf, buf + fmt_res);
    (void)parse_res;

    // 5. Binary serialization
    auto bytes = to_bytes(a);
    auto bin_res = *from_bytes<Fixed64<12>>(bytes);
    (void)bin_res;

    // Assert that NO allocations occurred
    int total_allocations = g_alloc_count.load();
    std::cout << "Total heap allocations during fixedwide operations: " << total_allocations << "\n";
    assert(total_allocations == 0);

    std::cout << "ZERO HEAP ALLOCATION VERIFIED!\n";
    return 0;
}
