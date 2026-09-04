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
        target_include_directories(${NAME} PRIVATE ${Boost_INCLUDE_DIRS})
        add_test(NAME fixedwide.audit.${NAME} COMMAND ${NAME})
    endforeach()

    # Negative compilation tests (must fail to compile)
    set(NEGATIVE_TESTS
        ambiguous_mixed_mul
        invalid_bits
        invalid_scale
        runtime_scale
    )

    foreach(NEG ${NEGATIVE_TESTS})
        add_library(neg_${NEG} EXCLUDE_FROM_ALL negative/${NEG}.cpp)
        target_link_libraries(neg_${NEG} PRIVATE fixedwide::fixedwide)
        add_test(NAME fixedwide.negative.${NEG}
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target neg_${NEG}
        )
        set_tests_properties(fixedwide.negative.${NEG} PROPERTIES WILL_FAIL TRUE)
    endforeach()
endif()

if(FIXEDWIDE_BUILD_ORACLE_TESTS)
    find_package(Boost REQUIRED)
    add_executable(test_oracle test_oracle.cpp)
    target_link_libraries(test_oracle PRIVATE fixedwide::fixedwide)
    target_include_directories(test_oracle PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME fixedwide.oracle COMMAND test_oracle)
endif()
