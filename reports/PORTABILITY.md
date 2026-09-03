# Portability & Platform Support: fixedwide 0.5.0-alpha.3

## 1. Supported Compilers & Standards
- **C++ Standard**: C++23 (`std::expected`, `<stdfloat>`, `std::countl_zero`)
- **Compilers**:
  - Clang 18+ (tested Clang 22.1.8)
  - GCC 13+ (tested GCC 14.2.0)
  - MSVC 2022+ (guarded `/W4 /permissive-` and `/fsanitize=address`)

## 2. Architectures & Portability Modes
1. **Hardware-Accelerated x86_64**:
   - Uses `unsigned __int128`, `imulq`, `idivq`, `shldq`, and Clang `unsigned _BitInt(256)`.
2. **Forced-Portable Mode (`-DFIXEDWIDE_FORCE_PORTABLE=ON`)**:
   - Multi-limb software arithmetic using 64-bit limbs.
   - Verified on pure standard C++23 with 23/23 tests passing.
3. **Embedded & Freestanding Suitability**:
   - Zero heap allocations (`test_no_alloc` passes).
   - Clean compilation under `-fno-exceptions` (`build_no_eh` passes 13/13 tests).
