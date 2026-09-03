// Downstream consumer used by the packaging check: parse, compute, format,
// with nothing on the include path but the installed package.
#include <fixedwide/arithmetic.hpp>
#include <fixedwide/chars.hpp>
#include <fixedwide/mixed.hpp>
#include <cstdio>

int main() {
    using Price = fixedwide::Fixed64<8>;
    using Rate  = fixedwide::Fixed64<12>;
    using Money = fixedwide::Fixed128<12>;

    const auto price = fixedwide::parse<Price>("123.45678901");
    const auto rate  = fixedwide::parse<Rate>("1.012345678901");
    if (!price || !rate) { std::puts("parse failed"); return 1; }

    // Mixed widths and scales require an explicit destination domain.
    const auto notional = fixedwide::mul_to<Money>(*price, *rate);
    if (!notional) { std::puts("multiply failed"); return 1; }

    char buffer[fixedwide::text_capacity];
    const auto written = fixedwide::to_chars(buffer, sizeof buffer, *notional);
    if (!written) { std::puts("format failed"); return 1; }

    std::printf("%.*s\n", static_cast<int>(*written), buffer);
    return 0;
}
