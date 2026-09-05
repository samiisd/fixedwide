#pragma once
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>
namespace fixedwide_bench {
inline std::size_t iterations = 1048576;
inline unsigned repetitions = 11;
inline std::string filter;
template<class F>
void measure(const std::string& name, F loop) {
    if (!filter.empty() && name.find(filter) == std::string::npos) return;
    loop(4096);
    std::vector<double> samples;
    for (unsigned repeat = 0; repeat < repetitions; ++repeat) {
        const auto start = std::chrono::steady_clock::now();
        loop(iterations);
        const auto end = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::nano>(end - start).count() /
                          static_cast<double>(iterations));
    }
    auto sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    std::printf("%s,%zu,%u,%.6f,%.6f,%.6f,%.6f,\"", name.c_str(), iterations, repetitions, sorted.front(),
                sorted[sorted.size() / 2], sorted[(sorted.size() - 1) * 95 / 100], sorted.back());
    for (unsigned i = 0; i < samples.size(); ++i) std::printf("%s%.6f", i ? ";" : "", samples[i]);
    std::puts("\"");
    std::fflush(stdout);
}

} // namespace fixedwide_bench
