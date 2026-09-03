// Resident mmap access only: no file I/O, msync, interprocess publication or
// first-page-fault cost. Identical generated loops receive different byte offsets.
#include <fixedwide/arithmetic.hpp>
#include "measurement.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

namespace fw = fixedwide;
namespace bench = fixedwide_bench;
[[gnu::noinline]] void load_loop(const std::byte* bytes, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        fw::FP128 value;
        std::memcpy(&value, bytes, sizeof(value));
        const auto raw = static_cast<fw::u128>(value.raw());
        __asm__ __volatile__("" : : "r"(static_cast<std::uint64_t>(raw)),
            "r"(static_cast<std::uint64_t>(raw >> 64)) : "memory");
    }
}
[[gnu::noinline]] void update_loop(std::byte* bytes, std::size_t count) {
    const std::array factors{fw::FP128::from_raw(2 * fw::scale), fw::FP128::from_raw(fw::scale / 2)};
    for (std::size_t i = 0; i < count; ++i) {
        fw::FP128 value;
        std::memcpy(&value, bytes, sizeof(value));
        const auto product = fw::mul(value, factors[i & 1], fw::Rounding::toward_zero);
        if (!product) std::abort();
        std::memcpy(bytes, &*product, sizeof(value));
        __asm__ __volatile__("" : : "r"(bytes) : "memory");
    }
}
int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--iterations" && i+1 < argc) bench::iterations = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--repetitions" && i+1 < argc) bench::repetitions = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
        else return 2;
    }
    if (bench::iterations < 4096 || bench::repetitions < 3 || bench::repetitions > 1000) return 2;
    const auto page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 128 || sysconf(_SC_LEVEL1_DCACHE_LINESIZE) != 64) return 2;
    void* mapping = mmap(nullptr, static_cast<std::size_t>(page_size), PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED) { std::perror("mmap"); return 1; }
    std::fprintf(stderr, "compiler=%s; digits=%u; cache_line=64; anonymous resident mmap; FP128 size/alignment=%zu/%zu\n",
        __VERSION__, fw::fractional_digits, sizeof(fw::FP128), alignof(fw::FP128));
    std::puts("name,iterations,repetitions,min_ns,median_ns,p95_batch_ns,max_ns,samples_ns");
    const auto initial = fw::FP128::from_raw(fw::scale + fw::scale / 4);
    for (unsigned offset : {0U, 8U, 1U, 56U, 63U}) {
        auto* bytes = static_cast<std::byte*>(mapping) + offset;
        std::memcpy(bytes, &initial, sizeof(initial)); // Initialize and fault pages outside timing.
        bench::measure("mmap.load.offset" + std::to_string(offset), [&](std::size_t n) { load_loop(bytes, n); });
        bench::measure("mmap.mul_chain.offset" + std::to_string(offset), [&](std::size_t n) { update_loop(bytes, n); });
    }
    return munmap(mapping, static_cast<std::size_t>(page_size)) == 0 ? 0 : 1;
}
