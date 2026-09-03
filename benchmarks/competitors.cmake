# Competitor libraries, fetched at pinned tags so the comparison is reproducible
# from a clean checkout with no vendored directories and no local setup.
#
# The previous suite only built if competitors/fpm/include and competitors/cnl/include
# happened to exist on the machine. Those directories were not in the archive, so
# nothing in the competitor report could be regenerated from it.
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(fpm
    GIT_REPOSITORY https://github.com/MikeLankamp/fpm.git
    GIT_TAG        v1.1.0
    GIT_SHALLOW    TRUE)
FetchContent_Declare(cnl
    GIT_REPOSITORY https://github.com/johnmcfarlane/cnl.git
    GIT_TAG        v1.1.7
    GIT_SHALLOW    TRUE)

FetchContent_Populate(fpm)
FetchContent_Populate(cnl)

add_library(fixedwide_competitor_deps INTERFACE)
target_include_directories(fixedwide_competitor_deps SYSTEM INTERFACE
    ${fpm_SOURCE_DIR}/include
    ${cnl_SOURCE_DIR}/include)

# Record exactly what was resolved, so a result file can name its dependencies.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/competitor_versions.txt
"fpm v1.1.0 ${fpm_SOURCE_DIR}\ncnl v1.1.7 ${cnl_SOURCE_DIR}\nboost ${Boost_VERSION}\n")
