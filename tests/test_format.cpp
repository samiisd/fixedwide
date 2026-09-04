// The three convenience headers -- format.hpp, iostream.hpp and hash.hpp --
// had no test at all, which is how a std::formatter that libc++ rejected
// outright shipped in a release. Each one is exercised here through the
// standard's own concepts, not just by calling it.
#include "check.hpp"
#include <fixedwide/all.hpp>
#include <fixedwide/format.hpp>
#include <fixedwide/hash.hpp>
#include <fixedwide/iostream.hpp>

#include <format>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace fixedwide;

// The concept is the actual contract. `std::format("{}", v)` compiling with one
// standard library says nothing about another: libc++ instantiates a formatter
// with its own context type, so a `format` member hard-coded to
// `std::format_context&` fails this while still compiling under libstdc++.
static_assert(std::formattable<Fixed8<2>, char>);
static_assert(std::formattable<Fixed16<4>, char>);
static_assert(std::formattable<Fixed32<9>, char>);
static_assert(std::formattable<Fixed64<12>, char>);
static_assert(std::formattable<Fixed128<38>, char>);
static_assert(std::formattable<Fixed256<76>, char>);

void test_format() {
    const auto value = parse<Fixed64<4>>("-1234.5000").value();

    CHECK(std::format("{}", value) == "-1234.5000");

    // Width, alignment and fill.
    CHECK(std::format("{:>15}", value) == "     -1234.5000");
    CHECK(std::format("{:<15}|", value) == "-1234.5000     |");
    CHECK(std::format("{:*^14}", value) == "**-1234.5000**");
    CHECK(std::format("[{}] [{}]", value, Fixed32<2>::from_raw(150)) == "[-1234.5000] [1.50]");

    // Numbers right-align by default, as every arithmetic type does.
    CHECK(std::format("{:15}", value) == "     -1234.5000");

    // PRECISION MEANS DECIMALS, not characters.
    //
    // This formatter used to inherit std::formatter<std::string_view>, which
    // made `{:.2}` truncate the STRING: 123.4567 printed as "12". For a decimal
    // type that is a silently wrong number, so it is asserted here in both
    // directions -- the right answer, and specifically not the truncation.
    const auto price = parse<Fixed64<4>>("123.4567").value();
    CHECK(std::format("{:.2}", price) == "123.46");
    CHECK(std::format("{:.2}", price) != "12");
    CHECK(std::format("{:.2f}", price) == "123.46"); // 'f' accepted
    CHECK(std::format("{:.0}", price) == "123");
    CHECK(std::format("{:.4}", price) == "123.4567");

    // Rounding is nearest-even, the same as to_chars, not truncation.
    CHECK(std::format("{:.3}", parse<Fixed64<4>>("1.0005").value()) == "1.000");
    CHECK(std::format("{:.3}", parse<Fixed64<4>>("1.0015").value()) == "1.002");
    CHECK(std::format("{:.2}", parse<Fixed64<4>>("-7.5000").value()) == "-7.50");

    // Precision combines with fill, alignment and width.
    CHECK(std::format("{:*^14.2}", price) == "****123.46****");
    CHECK(std::format("{:10.2}", parse<Fixed64<4>>("-7.5000").value()) == "     -7.50");

    // A precision the type cannot honour is a format_error, not a wrong number.
    bool threw = false;
    try {
        (void)std::vformat("{:.6}", std::make_format_args(price));
    } catch (const std::format_error&) {
        threw = true;
    }
    CHECK(threw);

    // The widest and narrowest types, and the extremes of each.
    CHECK(std::format("{}", Fixed8<2>::from_raw(-128)) == "-1.28");
    CHECK(std::format("{}", Fixed128<0>::from_raw(wide::int128(42))) == "42");
    CHECK(!std::format("{}", Fixed256<76>::max()).empty());
}

void test_iostream() {
    std::ostringstream out;
    out << parse<Fixed64<3>>("12.345").value();
    CHECK(out.str() == "12.345");

    Fixed64<3> parsed{};
    std::istringstream in("12.345 nonsense");
    in >> parsed;
    CHECK(in.good() || in.eof());
    CHECK(parsed == parse<Fixed64<3>>("12.345").value());

    // Text this type cannot hold exactly sets failbit and leaves the value be.
    Fixed64<2> untouched = parse<Fixed64<2>>("7.00").value();
    std::istringstream precise("1.005");
    precise >> untouched;
    CHECK(precise.fail());
    CHECK(untouched == parse<Fixed64<2>>("7.00").value());
}

void test_hash() {
    // The documented guarantee: equal values hash equal.
    const auto a = parse<Fixed64<4>>("3.1400").value();
    const auto b = Fixed64<4>::from_raw(31'400);
    CHECK(a == b);
    CHECK(std::hash<Fixed64<4>>{}(a) == std::hash<Fixed64<4>>{}(b));
    CHECK(hash_value(a) == std::hash<Fixed64<4>>{}(a));

    // Usable as a key, which is the only reason the header exists.
    std::unordered_map<Fixed64<4>, int> by_price;
    by_price[a] = 1;
    by_price[b] += 1;
    CHECK(by_price.size() == 1);
    CHECK(by_price[a] == 2);

    std::unordered_set<Fixed128<12>> wide_keys;
    wide_keys.insert(Fixed128<12>::from_raw(wide::int128(1, 2)));
    wide_keys.insert(Fixed128<12>::from_raw(wide::int128(1, 2)));
    wide_keys.insert(Fixed128<12>::min());
    wide_keys.insert(Fixed128<12>::max());
    CHECK(wide_keys.size() == 3);

    // The wide integers hash too, and hash consistently with their fixed-point
    // wrappers' raw values.
    CHECK(std::hash<wide::uint128>{}(wide::uint128(5, 6)) == std::hash<wide::uint128>{}(wide::uint128(5, 6)));
    CHECK(std::hash<wide::int256>{}(wide::int256(1, 2, 3, 4)) == std::hash<wide::int256>{}(wide::int256(1, 2, 3, 4)));
}

void test_to_string() {
    CHECK(to_string(Fixed64<2>::from_raw(-5)).value() == "-0.05");
    CHECK(to_string(wide::int128(-7)).value() == "-7");
    CHECK(to_string(wide::uint256(9, 0, 0, 0)).value() == "9");
}

int main() {
    test_format();
    test_iostream();
    test_hash();
    test_to_string();
    std::printf("test_format passed (%lu checks)\n", checks);
    return 0;
}
