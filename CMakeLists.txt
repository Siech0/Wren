cmake_minimum_required(VERSION 3.31)

# Project Setup
include(tools/cmake/extract-version.cmake)
extract_version("${CMAKE_CURRENT_LIST_DIR}/projects/core/include/core/version.hpp" RENDERER)

project(
  renderer
  VERSION ${RENDERER_VERSION}
  LANGUAGES CXX
)
message(STATUS "Renderer project version: ${RENDERER_VERSION}")

include(tools/cmake/utils.cmake)
include(GNUInstallDirs)
include(GenerateExportHeader)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Renderer setting build type to 'Release'.")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Release or Debug" FORCE
  )
endif()

# Options ####

# Build all
option(RENDERER_BUILD_ALL "Build all artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build tests
option(RENDERER_BUILD_TESTS "Build test artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build benchmarks
option(RENDERER_BUILD_BENCHMARKS "Bench benchmark artifacts." ${PROJECT_IS_TOP_LEVEL})

# Build resource processor
option(RENDERER_BUILD_RESOURCE_PROCESSOR "Build resource processor artifacts." ${PROJECT_IS_TOP_LEVEL})

# Process resource files
if(RENDERER_BUILD_RESOURCE_PROCESSOR OR RENDERER_BUILD_ALL)
  option(RENDERER_PROCESS_RESOURCES "Process resource files." ${PROJECT_IS_TOP_LEVEL})
endif()

# Build renderer
option(RENDERER_BUILD_RENDERER "Build primary renderer artifacts." ${PROJECT_IS_TOP_LEVEL})

# Embed resources
option(RENDERER_EMBED_RESOURCES "Embed resource files." ON)

# Generate format targets
option(RENDERER_USE_FORMAT "Generate formatting targets." ${PROJECT_IS_TOP_LEVEL})

# Generate spell check targets
option(RENDERER_USE_SPELLCHECK "Generate spellcheck targets." ${PROJECT_IS_TOP_LEVEL})

# Generate documentation
option(RENDERER_USE_DOCS "Generate documentation targets." ${PROJECT_IS_TOP_LEVEL})

# Generate install targets
option(RENDERER_INSTALL "Generate install targets." ${PROJECT_IS_TOP_LEVEL})

# Dependency Management #####
if(RENDERER_BUILD_TESTS)
  enable_testing()
  find_package(GTest)
endif()

if(RENDERER_BUILD_BENCHMARKS)
  find_package(benchmark)
endif()

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

# Project Breakout #####
add_subdirectory(projects)

# Installation #####
if(RENDERER_USE_FORMAT)
  message(STATUS "Renderer generating format targets (format-check, format-fix).")
  include(tools/cmake/format-targets.cmake)
endif()

if(RENDERER_USE_SPELLCHECK)
  message(STATUS "Renderer generating spellcheck targets (spell-check, spell-fix).")
  include(tools/cmake/spell-targets.cmake)
endif()

if(RENDERER_USE_DOCS)
  message(STATUS "Renderer generating documentation targets (docs-build).")
  include(tools/cmake/docs-targets.cmake)
endif()

if(RENDERER_INSTALL)
  message(STATUS "Renderer generating install targets.")
  include(tools/cmake/install-rules.cmake)
endif()
