# Google Benchmark 1.9.4 uses __COUNTER__ in its public header. New Clang
# versions diagnose it under the dependency's -pedantic-errors setting.
# Keep the pinned dependency and all other diagnostics; suppress only this
# supported warning on the benchmark target and its header consumers.
#
# Deferring also permits CMAKE_PROJECT_INCLUDE to apply the identical reviewed
# compatibility rule to an older arithmetic checkout without editing its CMake.
function(fixedwide_google_benchmark_compat)
    if(NOT TARGET benchmark::benchmark OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        return()
    endif()
    get_target_property(_target benchmark::benchmark ALIASED_TARGET)
    if(NOT _target)
        set(_target benchmark::benchmark)
    endif()
    get_target_property(_applied ${_target} FIXEDWIDE_C2Y_COMPAT_APPLIED)
    if(_applied)
        return()
    endif()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-Wno-c2y-extensions" FIXEDWIDE_HAS_WNO_C2Y_EXTENSIONS)
    if(FIXEDWIDE_HAS_WNO_C2Y_EXTENSIONS)
        get_target_property(_imported ${_target} IMPORTED)
        if(_imported)
            target_compile_options(${_target} INTERFACE -Wno-c2y-extensions)
        else()
            target_compile_options(${_target} PUBLIC -Wno-c2y-extensions)
        endif()
        set_property(TARGET ${_target} PROPERTY FIXEDWIDE_C2Y_COMPAT_APPLIED TRUE)
    endif()
endfunction()

cmake_language(DEFER CALL fixedwide_google_benchmark_compat)
