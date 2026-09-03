#include "check.hpp"
#include <fixedwide/all.hpp>
#include <type_traits>

using namespace fixedwide;

int main() {
    // Check storage size and alignment guarantees
    static_assert(sizeof(Fixed8<0>) == 1 && sizeof(Fixed8<2>) == 1);
    static_assert(alignof(Fixed8<2>) == 1);

    static_assert(sizeof(Fixed16<0>) == 2 && sizeof(Fixed16<4>) == 2);
    static_assert(alignof(Fixed16<4>) == 2);

    static_assert(sizeof(Fixed32<0>) == 4 && sizeof(Fixed32<9>) == 4);
    static_assert(alignof(Fixed32<9>) == 4);

    static_assert(sizeof(Fixed64<0>) == 8 && sizeof(Fixed64<18>) == 8);
    static_assert(alignof(Fixed64<18>) == 8);

    static_assert(sizeof(Fixed128<0>) == 16 && sizeof(Fixed128<38>) == 16);
    static_assert(alignof(Fixed128<38>) == 8);

    static_assert(sizeof(Fixed256<0>) == 32 && sizeof(Fixed256<76>) == 32);
    static_assert(alignof(Fixed256<76>) == 8);

    // Wide integers
    static_assert(sizeof(wide::uint128) == 16 && alignof(wide::uint128) == 8);
    static_assert(sizeof(wide::int128) == 16 && alignof(wide::int128) == 8);
    static_assert(sizeof(wide::uint256) == 32 && alignof(wide::uint256) == 8);
    static_assert(sizeof(wide::int256) == 32 && alignof(wide::int256) == 8);

    // Trivially copyable and standard layout
    static_assert(std::is_trivially_copyable_v<Fixed8<2>>);
    static_assert(std::is_trivially_copyable_v<Fixed16<4>>);
    static_assert(std::is_trivially_copyable_v<Fixed32<9>>);
    static_assert(std::is_trivially_copyable_v<Fixed64<12>>);
    static_assert(std::is_trivially_copyable_v<Fixed128<12>>);
    static_assert(std::is_trivially_copyable_v<Fixed256<18>>);

    static_assert(std::is_standard_layout_v<Fixed8<2>>);
    static_assert(std::is_standard_layout_v<Fixed16<4>>);
    static_assert(std::is_standard_layout_v<Fixed32<9>>);
    static_assert(std::is_standard_layout_v<Fixed64<12>>);
    static_assert(std::is_standard_layout_v<Fixed128<12>>);
    static_assert(std::is_standard_layout_v<Fixed256<18>>);

    // No implicit conversions
    static_assert(!std::is_convertible_v<int, Fixed64<12>>);
    static_assert(!std::is_convertible_v<double, Fixed128<12>>);
    static_assert(!std::is_convertible_v<Fixed64<12>, Fixed128<12>>);

    CHECK(sizeof(Fixed256<18>) == 32);
    std::printf("test_storage passed (%lu checks)\n", checks);
    return 0;
}
