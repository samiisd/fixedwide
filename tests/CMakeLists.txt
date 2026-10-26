if(FIXEDWIDE_BUILD_TESTS)
    set(TEST_NAMES
        storage
        wide
        rounding
        core
        unchecked
        checked
        midpoint
        log10
        mixed
        chars
        literal
        binary
        floating
        no_alloc
        signed_min
        overflow
        constexpr
        mixed_native
        format
        readme
    )

    foreach(NAME ${TEST_NAMES})
        add_executable(test_${NAME} test_${NAME}.cpp)
        target_link_libraries(test_${NAME} PRIVATE fixedwide::fixedwide)
        target_include_directories(test_${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(test_${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
        add_test(NAME fixedwide.${NAME} COMMAND test_${NAME})
    endforeach()

    if(NOT MSVC)
        target_compile_options(test_checked PRIVATE -fno-exceptions)
    endif()

    # Literal-only consumers need neither the compiled library nor exceptions.
    # Do not link fixedwide here: that would hide accidental runtime references.
    add_executable(test_literal_header_only test_literal_header_only.cpp)
    target_compile_features(test_literal_header_only PRIVATE cxx_std_23)
    target_include_directories(test_literal_header_only PRIVATE ${PROJECT_SOURCE_DIR}/include)
    if(FIXEDWIDE_FORCE_PORTABLE)
        target_compile_definitions(test_literal_header_only PRIVATE FIXEDWIDE_FORCE_PORTABLE)
    endif()
    if(NOT MSVC)
        target_compile_options(test_literal_header_only PRIVATE -fno-exceptions)
    endif()
    add_test(NAME fixedwide.literal_header_only COMMAND test_literal_header_only)

    # Prove the supported floating conversions have no compiled-kernel dependency,
    # even without exceptions and under aggressive consumer FP optimizations.
    # Compile flags only: do not contaminate the generic differential oracle or
    # introduce fast-math startup code that changes the process FP environment.
    add_executable(test_floating_header_only test_floating_header_only.cpp)
    target_compile_features(test_floating_header_only PRIVATE cxx_std_23)
    target_include_directories(test_floating_header_only PRIVATE ${PROJECT_SOURCE_DIR}/include)
    if(FIXEDWIDE_FORCE_PORTABLE)
        target_compile_definitions(test_floating_header_only PRIVATE FIXEDWIDE_FORCE_PORTABLE)
    endif()
    if(MSVC)
        target_compile_options(test_floating_header_only PRIVATE /fp:fast)
    else()
        target_compile_options(test_floating_header_only PRIVATE -fno-exceptions -ffast-math -ffp-contract=fast)
    endif()
    if(FIXEDWIDE_SANITIZE AND NOT MSVC)
        target_compile_options(test_floating_header_only PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(test_floating_header_only PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME fixedwide.floating_header_only COMMAND test_floating_header_only)

    add_executable(test_to_floating_header_only test_to_floating_header_only.cpp)
    target_compile_features(test_to_floating_header_only PRIVATE cxx_std_23)
    target_include_directories(test_to_floating_header_only PRIVATE ${PROJECT_SOURCE_DIR}/include)
    if(FIXEDWIDE_FORCE_PORTABLE)
        target_compile_definitions(test_to_floating_header_only PRIVATE FIXEDWIDE_FORCE_PORTABLE)
    endif()
    if(MSVC)
        target_compile_options(test_to_floating_header_only PRIVATE /fp:fast)
    else()
        target_compile_options(test_to_floating_header_only PRIVATE -fno-exceptions -ffast-math -ffp-contract=fast)
    endif()
    if(FIXEDWIDE_SANITIZE AND NOT MSVC)
        target_compile_options(test_to_floating_header_only PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(test_to_floating_header_only PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME fixedwide.to_floating_header_only COMMAND test_to_floating_header_only)

    # test_format asserts that an impossible precision throws std::format_error,
    # which is what the standard requires formatter::parse to do. Throwing and
    # catching that exception aborts under AddressSanitizer with libc++, and it
    # is not this library's bug -- this whole program reproduces it:
    #
    #     #include <format>
    #     int main() {
    #         try { throw std::format_error("boom"); }
    #         catch (const std::format_error&) {}
    #     }
    #
    #     AddressSanitizer: alloc-dealloc-mismatch (operator new vs free)
    #       in std::range_error::~range_error()  [libc++abi.so.1]
    #
    # libc++ constructs the exception through an inline header path that uses
    # the program's operator new, and destroys it through libc++abi, which
    # frees. Only that one check is turned off, only for this one test; every
    # other AddressSanitizer check stays on here, and every other test keeps all
    # of them.
    set_tests_properties(fixedwide.format PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=alloc_dealloc_mismatch=0")

    set(AUDIT_TESTS
        audit_same_domain
        audit_unchecked
        audit_midpoint
        audit_mixed
        audit_known
        audit_floating
        audit_io
        audit_text_targeted
        audit_portable_edges
        audit_portable_div128
        audit_fastpath_rounding
    )

    # The same-domain, mixed, midpoint and unchecked audits use Boost.Multiprecision.
    # Boost is header-only here and never a runtime dependency; when it is
    # absent (a cross build without a Boost sysroot, say) they are skipped
    # and the rest of the matrix still runs.
    # Host Boost headers are not valid for a cross build, so do not reach for
    # them there: the Boost-backed suites are skipped; the rest still run on
    # the target, including the dependency-free midpoint tests.
    if(NOT CMAKE_CROSSCOMPILING)
        find_package(Boost QUIET)
    endif()
    foreach(NAME ${AUDIT_TESTS})
        if(NAME MATCHES "audit_(same_domain|mixed|midpoint|unchecked)" AND NOT Boost_FOUND)
            message(STATUS "fixedwide: skipping ${NAME} (Boost headers not found)")
            continue()
        endif()
        add_executable(${NAME} ${NAME}.cpp)
        target_link_libraries(${NAME} PRIVATE fixedwide::fixedwide)
        target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
        target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
        # Only when Boost was actually found: an unset Boost_INCLUDE_DIRS
        # expands to Boost_INCLUDE_DIR-NOTFOUND, which CMake reports as an error
        # at generate time. That is what broke the Windows jobs, where Boost is
        # not installed and the two Boost-backed suites are skipped anyway.
        if(Boost_FOUND)
            target_include_directories(${NAME} PRIVATE ${Boost_INCLUDE_DIRS})
        endif()
        add_test(NAME fixedwide.audit.${NAME} COMMAND ${NAME})
    endforeach()

    # Negative compilation tests (must fail to compile)
    # Each negative test names the diagnostic it expects, not merely that the
    # build failed. WILL_FAIL alone accepted ANY compile error, so a typo, a
    # renamed header or a missing include would have passed these silently while
    # the rule they are meant to protect had stopped being enforced.
    #
    # PASS_REGULAR_EXPRESSION makes ctest ignore the exit code and pass only if
    # the output matches, which asserts both that it failed and why. A build that
    # unexpectedly SUCCEEDS produces no such message and therefore fails.
    # Literal diagnostics deliberately differ from their source filenames, so
    # a successful build's progress output cannot satisfy their expressions.
    set(NEGATIVE_TESTS
        # Three compilers, three phrasings for the same diagnostic:
        #   clang: call to deleted function 'mul'
        #   gcc:   use of deleted function
        #   msvc:  attempting to reference a deleted function
        ambiguous_mixed_mul "deleted function"
        invalid_bits        "Bits must be one of"
        invalid_scale       "Decimals exceeds maximum capacity"
        runtime_scale       "scale_member"
        literal_empty       "literal_error_empty"
        literal_invalid     "literal_error_invalid"
        literal_too_precise  "literal_error_too_precise"
        literal_overflow    "literal_error_overflow"
        literal_embedded_nul "literal_error_invalid"
        literal_unterminated "literal_error_invalid"
        literal_runtime     "constant expression|constant-expression|did not evaluate to a constant"
        constructor_empty       "literal_error_empty"
        constructor_invalid     "literal_error_invalid"
        constructor_too_precise "literal_error_too_precise"
        constructor_overflow    "literal_error_overflow"
        constructor_embedded_nul "literal_error_invalid"
        constructor_unterminated "literal_error_invalid"
        constructor_runtime     "constant expression|constant-expression|did not evaluate to a constant"
        constructor_template_inexact "literal_error_too_precise"
        constructor_template_runtime "constant expression|constant-expression|did not evaluate to a constant"
        operator_runtime        "constant expression|constant-expression|did not evaluate to a constant"
        operator_inexact        "literal_error_too_precise"
    )

    list(LENGTH NEGATIVE_TESTS _neg_count)
    math(EXPR _neg_last "${_neg_count} - 1")
    foreach(_i RANGE 0 ${_neg_last} 2)
        list(GET NEGATIVE_TESTS ${_i} NEG)
        math(EXPR _j "${_i} + 1")
        list(GET NEGATIVE_TESTS ${_j} NEG_DIAGNOSTIC)

        add_library(neg_${NEG} EXCLUDE_FROM_ALL negative/${NEG}.cpp)
        target_link_libraries(neg_${NEG} PRIVATE fixedwide::fixedwide)
        add_test(NAME fixedwide.negative.${NEG}
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target neg_${NEG}
        )
        set_tests_properties(fixedwide.negative.${NEG} PROPERTIES
            PASS_REGULAR_EXPRESSION "${NEG_DIAGNOSTIC}")
    endforeach()
endif()

if(FIXEDWIDE_BUILD_ORACLE_TESTS)
    find_package(Boost REQUIRED)
    add_executable(test_oracle test_oracle.cpp)
    target_link_libraries(test_oracle PRIVATE fixedwide::fixedwide)
    target_include_directories(test_oracle PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME fixedwide.oracle COMMAND test_oracle)
endif()

# Independent, focused coverage regressions supplement the large differential
# suites. Boost is a test-only dependency and never propagates to consumers.
if(Boost_FOUND AND (FIXEDWIDE_BUILD_TESTS OR FIXEDWIDE_BUILD_ORACLE_TESTS))
    add_executable(test_coverage
        test_coverage.cpp
        test_coverage_wide.cpp
        test_coverage_text.cpp
        test_coverage_arithmetic.cpp
        test_coverage_adapters.cpp
        test_coverage_mixed.cpp
        test_coverage_checked.cpp)
    target_link_libraries(test_coverage PRIVATE fixedwide::fixedwide)
    target_include_directories(test_coverage PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
    # These tests share a source-coverage map. A single unity translation unit
    # avoids competing unused/instantiated mappings for the same header code.
    set_target_properties(test_coverage PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 0)
    foreach(SUITE wide text arithmetic adapters format_adapters mixed checked)
        add_test(NAME fixedwide.coverage_${SUITE} COMMAND test_coverage ${SUITE})
    endforeach()
    # The existing fixedwide.format test documents the libc++abi format_error
    # allocation mismatch. Isolate the same exception checks here, leaving the
    # arithmetic, floating and other coverage suites fully checked by ASan.
    set_tests_properties(fixedwide.coverage_format_adapters PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=alloc_dealloc_mismatch=0")
endif()

if(Boost_FOUND AND FIXEDWIDE_BUILD_TESTS)
    add_executable(test_floating_formats test_floating_formats.cpp)
    target_link_libraries(test_floating_formats PRIVATE fixedwide::fixedwide)
    target_include_directories(test_floating_formats PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME fixedwide.floating_formats COMMAND test_floating_formats)
endif()

if(Boost_FOUND AND FIXEDWIDE_BUILD_TESTS)
    add_executable(test_to_floating test_to_floating.cpp)
    target_link_libraries(test_to_floating PRIVATE fixedwide::fixedwide)
    target_include_directories(test_to_floating PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME fixedwide.to_floating COMMAND test_to_floating)
endif()

# GCC can exercise the other two long-double formats on an x86 host. These
# targets intentionally do NOT link fixedwide or call long-double libc routines:
# those have the host ABI, which the compiler option would make incompatible.
# The rational oracle takes its input from test-generated integer fields.
if(FIXEDWIDE_BUILD_TESTS AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
   CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$" AND NOT CMAKE_CROSSCOMPILING)
    foreach(WIDTH 64 128)
        set(SOURCES floating_header_only to_floating_header_only)
        if(Boost_FOUND)
            list(APPEND SOURCES floating_formats to_floating)
        endif()
        foreach(SOURCE ${SOURCES})
            set(NAME test_${SOURCE}_ld${WIDTH})
            add_executable(${NAME} test_${SOURCE}.cpp)
            target_compile_features(${NAME} PRIVATE cxx_std_23)
            target_include_directories(${NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
            if(Boost_FOUND)
                target_include_directories(${NAME} PRIVATE ${Boost_INCLUDE_DIRS})
            endif()
            target_compile_definitions(${NAME} PRIVATE FIXEDWIDE_TEST_ALTERNATE_LDBL)
            target_compile_options(${NAME} PRIVATE -mlong-double-${WIDTH})
            if(FIXEDWIDE_FORCE_PORTABLE)
                target_compile_definitions(${NAME} PRIVATE FIXEDWIDE_FORCE_PORTABLE)
            endif()
            if(SOURCE MATCHES "header_only$")
                target_compile_options(${NAME} PRIVATE -fno-exceptions -ffast-math -ffp-contract=fast)
            endif()
            if(FIXEDWIDE_SANITIZE)
                target_compile_options(${NAME} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
                target_link_options(${NAME} PRIVATE -fsanitize=address,undefined)
            endif()
            add_test(NAME fixedwide.${SOURCE}_ld${WIDTH} COMMAND ${NAME})
        endforeach()
    endforeach()
endif()
