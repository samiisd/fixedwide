if(FIXEDWIDE_BUILD_TESTS)
    set(TEST_NAMES
        storage
        wide
        rounding
        core
        mixed
        chars
        binary
        floating
        no_alloc
        signed_min
        overflow
        constexpr
        mixed_native
        format
    )

    foreach(NAME ${TEST_NAMES})
        add_executable(test_${NAME} test_${NAME}.cpp)
        target_link_libraries(test_${NAME} PRIVATE fixedwide::fixedwide)
        target_include_directories(test_${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(test_${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
        add_test(NAME fixedwide.${NAME} COMMAND test_${NAME})
    endforeach()

    set(AUDIT_TESTS
        audit_same_domain
        audit_mixed
        audit_known
        audit_floating
        audit_io
        audit_text_targeted
        audit_portable_edges
        audit_portable_div128
    )

    # Two of the audit suites differential-test against Boost.Multiprecision.
    # Boost is header-only here and never a runtime dependency; when it is
    # absent (a cross build without a Boost sysroot, say) those two are skipped
    # and the rest of the matrix still runs.
    # Host Boost headers are not valid for a cross build, so do not reach for
    # them there: the two Boost-backed suites are skipped and the remaining 21
    # still run on the target.
    if(NOT CMAKE_CROSSCOMPILING)
        find_package(Boost QUIET)
    endif()
    foreach(NAME ${AUDIT_TESTS})
        if(NAME MATCHES "audit_(same_domain|mixed)" AND NOT Boost_FOUND)
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
    set(NEGATIVE_TESTS
        ambiguous_mixed_mul "call to deleted function 'mul'|use of deleted function"
        invalid_bits        "Bits must be one of"
        invalid_scale       "Decimals exceeds maximum capacity"
        runtime_scale       "scale_member"
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
