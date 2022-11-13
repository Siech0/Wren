from conan import ConanFile, tools
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
import re

class ConanRecipe(ConanFile):

    # Package Description
    name = "renderer"
    license = "MIT"
    author = "siech0"

    # Package Options
    settings = "os", "compiler", "build_type", "arch"

    # Package exports/imports/requires
    requires = (
        "glm/1.0.1",
        "vulkan-memory-allocator/3.0.1",
        "glfw/3.4"
    )

    build_requires = (
        "gtest/1.16.0",
        "benchmark/1.9.1"
    )

    def set_version(self):
        filepath = "projects/core/include/core/version.hpp"
        try:
            data = tools.load(filepath)
            major_version = re.search(r"RENDERER_VERSION_MAJOR\s+(\d+)", data).group(1)
            minor_version = re.search(r"RENDERER_VERSION_MINOR\s+(\d+)", data).group(1)
            patch_version = re.search(r"RENDERER_VERSION_PATCH\s+(\d+)", data).group(1)
            version_string = "{}.{}.{}".format(major_version, minor_version, patch_version)
            self.version = version_string
        except Exception as e:
            self.version = None

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
