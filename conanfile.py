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
        # std::expected is the library's error channel; without it nothing here
        # compiles, and a confusing template error is worse than this message.
        minimum = {"gcc": "14", "clang": "18", "apple-clang": "15", "msvc": "193"}
        compiler = str(self.settings.compiler)
        required = minimum.get(compiler)
        if required and Version(self.settings.compiler.version) < required:
            raise ConanInvalidConfiguration(
                f"fixedwide needs {compiler} >= {required} for C++23 std::expected"
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
