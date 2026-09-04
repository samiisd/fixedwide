# Every example is also a test. `ctest -R fixedwide.example` compiles and RUNS
# all of them, and each one prints "OK" only after checking its own output, so
# an example that silently stops being correct fails the build.
set(EXAMPLE_NAMES
    01_quick_start
    02_rounding_modes
    03_error_handling
    04_mixed_scales
    05_text_io
    06_binary_roundtrip
    07_money_ledger
    08_constexpr
)

foreach(NAME ${EXAMPLE_NAMES})
    add_executable(example_${NAME} ${NAME}.cpp)
    target_link_libraries(example_${NAME} PRIVATE fixedwide::fixedwide)
    add_test(NAME fixedwide.example.${NAME} COMMAND example_${NAME})
    set_tests_properties(fixedwide.example.${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "OK")
endforeach()
