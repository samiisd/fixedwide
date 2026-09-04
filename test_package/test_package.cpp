// The smallest program that proves the package works: parse, compute across
// scales, format, and check the exact answer.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/mixed.hpp>
#include <fixedwide/version.hpp>
#include <cstdio>
#include <cstring>

int main() {
    using Price = fixedwide::Fixed64<4>;
    using Rate  = fixedwide::Fixed64<8>;
    using Money = fixedwide::Fixed128<2>;

    const auto price = fixedwide::parse<Price>("19.9900");
    const auto rate  = fixedwide::parse<Rate>("1.07500000");
    if (!price || !rate) { std::puts("parse failed"); return 1; }

    const auto gross = fixedwide::mul_to<Money>(*price, *rate);
    if (!gross) { std::puts("multiply failed"); return 1; }

    char buffer[fixedwide::text_capacity];
    const auto written = fixedwide::to_chars(buffer, sizeof buffer, *gross);
    if (!written) { std::puts("format failed"); return 1; }

    std::printf("fixedwide %s: 19.99 * 1.075 = %.*s\n",
                FIXEDWIDE_VERSION_STRING, static_cast<int>(*written), buffer);

    if (*written != 5 || std::memcmp(buffer, "21.49", 5) != 0) {
        std::puts("wrong answer");
        return 1;
    }
    return 0;
}
