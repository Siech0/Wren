cmake_minimum_required(VERSION 4.2)

# Project Setup
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake/modules")
include(versioning)

set(WREN_VERSION_INCLUDEDIR "${CMAKE_BINARY_DIR}/include")
wren_extract_version(
    VERSION_FILE "${CMAKE_CURRENT_LIST_DIR}/project.json"
    STATE_FILE "${CMAKE_BINARY_DIR}/last_build.json"
    HEADER_OUT "${WREN_VERSION_INCLUDEDIR}/wren/version.hpp"
)
message(STATUS "Wren version: ${WREN_VERSION_STRING} (${WREN_VERSION}, build: ${WREN_VERSION_BUILD})")

project(
    renderer
    VERSION ${WREN_VERSION}
    LANGUAGES CXX
)
message(STATUS "Wren project version: ${WREN_VERSION_STRING}")

include(utils)

# Development: Place all DLLs and executables in common directory for easy testing
include(GNUInstallDirs)
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

###### Options ######

# Build all
option(WREN_BUILD_ALL "Build all artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build tests
option(WREN_BUILD_TESTS "Build test artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build benchmarks
option(WREN_BUILD_BENCHMARKS "Bench benchmark artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build docs
option(WREN_USE_DOCS "Generate documentation targets." ${PROJECT_IS_TOP_LEVEL})

# Generate install targets
option(WREN_INSTALL "Generate install targets." ${PROJECT_IS_TOP_LEVEL})

# Dependency Management #####
if(WREN_BUILD_TESTS)
    enable_testing()
    find_package(GTest)
endif()

if(WREN_BUILD_BENCHMARKS)
    find_package(benchmark)
endif()

# Project Breakout #####
add_subdirectory(projects)

# Installation #####
if(WREN_USE_DOCS)
    message(STATUS "Renderer generating documentation targets (docs-build).")
    include(docs_targets)
endif()

if(WREN_INSTALL)
    message(STATUS "Renderer generating install targets.")
    include(install_rules)
    include(packaging_rules)
endif()
