"""Conan recipe for fixedwide.

Deliberately narrow: a package build compiles the library and nothing else.
Tests, examples, benchmarks and the fuzzer are all forced off, so building the
package never needs Boost, a fuzzing-capable Clang, or a network fetch.
"""

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load
from conan.tools.scm import Version

import os
import re


class FixedwideConan(ConanFile):
    name = "fixedwide"
    license = "MIT"
    url = "https://github.com/Samiisd/fixedwide"
    homepage = "https://github.com/Samiisd/fixedwide"
    description = (
        "Checked fixed-point decimal arithmetic and portable fixed-width wide "
        "integers (128/256-bit) for C++23. No heap allocation, no virtual "
        "dispatch, no exceptions: every fallible operation returns "
        "std::expected."
    )
    topics = ("fixed-point", "decimal", "int128", "int256", "checked-arithmetic", "cpp23")

    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        # Mirrors FIXEDWIDE_FORCE_PORTABLE. It changes the compiled backend, so
        # it is an option rather than a build detail: a consumer must get the
        # same answer from the package it would get from a source build.
        "force_portable": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "force_portable": False,
    }

    implements = ["auto_shared_fpic"]

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
        "LICENSE",
    )

    def set_version(self):
        # One source of truth for the version: the header the library itself
        # reports at runtime. A recipe that hard-coded it would drift.
        if self.version:
            return
        header = load(self, os.path.join(self.recipe_folder, "include", "fixedwide", "version.hpp"))
        match = re.search(r'FIXEDWIDE_VERSION_STRING\s+"([^"]+)"', header)
        if not match:
            raise ConanInvalidConfiguration("cannot read FIXEDWIDE_VERSION_STRING from version.hpp")
        self.version = match.group(1)

    def layout(self):
        cmake_layout(self)

    def validate(self):
        check_min_cppstd(self, 23)

        # Both of these are measured, not guessed, and both produce a wall of
        # template errors if they are left to fail on their own.
        #
        # gcc 14: libstdc++ did not put frexpl and ldexpl in namespace std
        # until then, and src/floating.cpp needs them.
        # clang 18: fine, but only with libc++ -- see below.
        minimum = {"gcc": "14", "clang": "18", "apple-clang": "15", "msvc": "193"}
        compiler = str(self.settings.compiler)
        required = minimum.get(compiler)
        if required and Version(self.settings.compiler.version) < required:
            raise ConanInvalidConfiguration(
                f"fixedwide needs {compiler} >= {required} for C++23 std::expected"
            )

        # Clang 17 and 18 report __cpp_concepts as 201907; libstdc++ gates
        # <expected> on 202002, so std::expected does not exist for them with
        # that standard library. Nothing this library can do works around it.
        libcxx = self.settings.get_safe("compiler.libcxx")
        if (compiler == "clang" and Version(self.settings.compiler.version) < "19"
                and libcxx and "libstdc++" in str(libcxx)):
            raise ConanInvalidConfiguration(
                "clang < 19 cannot see std::expected with libstdc++ (it reports "
                "__cpp_concepts=201907, and libstdc++ gates <expected> on 202002). "
                "Use -s compiler.libcxx=libc++, or clang 19 or newer."
            )

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["FIXEDWIDE_BUILD_TESTS"] = False
        toolchain.cache_variables["FIXEDWIDE_BUILD_ORACLE_TESTS"] = False
        toolchain.cache_variables["FIXEDWIDE_BUILD_BENCHMARKS"] = False
        toolchain.cache_variables["FIXEDWIDE_BUILD_EXAMPLES"] = False
        toolchain.cache_variables["FIXEDWIDE_BUILD_FUZZER"] = False
        toolchain.cache_variables["FIXEDWIDE_FORCE_PORTABLE"] = bool(self.options.force_portable)
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["fixedwide"]
        # A Conan consumer and a find_package consumer must write the identical
        # two lines of CMake: find_package(fixedwide) and fixedwide::fixedwide.
        self.cpp_info.set_property("cmake_file_name", "fixedwide")
        self.cpp_info.set_property("cmake_target_name", "fixedwide::fixedwide")

        # FIXEDWIDE_FORCE_PORTABLE is PUBLIC in CMakeLists.txt because it
        # changes which code path the headers take. A consumer of a portable
        # package must compile against the same headers the library did.
        if self.options.force_portable:
            self.cpp_info.defines = ["FIXEDWIDE_FORCE_PORTABLE"]

        self.cpp_info.set_property("cmake_config_version_compat", "SameMajorVersion")
