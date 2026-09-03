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

    foreach(NAME ${AUDIT_TESTS})
        add_executable(${NAME} ${NAME}.cpp)
        target_link_libraries(${NAME} PRIVATE fixedwide::fixedwide)
        target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
        target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
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
