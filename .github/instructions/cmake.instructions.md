---
applyTo: '**/{CMakeLists.txt,*.cmake,*.cmake.in}'
---

# CMake Instructions

<!-- Overview -->

Operate with Modern CMake (target-based) for strict, reproducible builds. Favor explicit, local target properties over directory- or cache-wide settings. Keep dependency graphs simple, usage requirements clear, and builds CI-friendly.

## Constraints

- **CMake version**: `4.2` minimum
- **Languages**: C++ only unless explicitly added
- **C++ standard**: **23**, **no compiler extensions**
- **Out-of-source builds only**
- **No global compile/link flags** (avoid `CMAKE_CXX_FLAGS*`)
- **No `file(GLOB ...)` for sources**
- **All policies set to `NEW`**
- **Reproducible builds**: forbid implicit network fetches unless explicitly enabled

Recommended top of root `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 4.2)

project(<PROJECT_NAME>
  VERSION 0.1.0
  DESCRIPTION "<TODO: short description>"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

## Expected Practices

### Targets & Features

- Define all build artifacts as proper targets (`add_library`, `add_executable`).
- Require standard via `target_compile_features(<tgt> PUBLIC cxx_std_23)` and disable extensions (`CXX_EXTENSIONS OFF`).
- Encapsulate usage requirements (`PUBLIC`, `PRIVATE`, `INTERFACE`).
- Use `target_sources()` with explicit lists.
- Support precompiled headers only when beneficial.

### Warnings & Errors

Centralize warnings:

```cmake
option(PROJECT_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

function(project_set_warnings tgt)
  if (MSVC)
    target_compile_options(${tgt} PRIVATE
      /W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc
      $<$<BOOL:${PROJECT_WARNINGS_AS_ERRORS}>:/WX>)
  else()
    target_compile_options(${tgt} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wconversion -Wsign-conversion -Wshadow
      -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual
      -Wnull-dereference -Wdouble-promotion -Wimplicit-fallthrough
      $<$<BOOL:${PROJECT_WARNINGS_AS_ERRORS}>:-Werror>)
  endif()
  set_target_properties(${tgt} PROPERTIES CXX_EXTENSIONS OFF)
  target_compile_features(${tgt} PUBLIC cxx_std_23)
endfunction()
```

### IPO / LTO

```cmake
function(project_enable_ipo tgt)
  if (CMAKE_BUILD_TYPE MATCHES "Rel" OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    set_property(TARGET ${tgt} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endfunction()
```

### Sanitizers (Opt-In)

```cmake
option(ENABLE_ASAN "AddressSanitizer" OFF)
option(ENABLE_UBSAN "UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "ThreadSanitizer" OFF)

function(project_enable_sanitizers tgt)
  if (MSVC)
    if (ENABLE_ASAN)
      target_compile_options(${tgt} PRIVATE /fsanitize=address)
      target_link_options(${tgt} PRIVATE /fsanitize=address)
    endif()
  else()
    set(san_opts "")
    if (ENABLE_ASAN) list(APPEND san_opts -fsanitize=address) endif()
    if (ENABLE_UBSAN) list(APPEND san_opts -fsanitize=undefined) endif()
    if (ENABLE_TSAN) list(APPEND san_opts -fsanitize=thread) endif()
    if (san_opts)
      target_compile_options(${tgt} PRIVATE ${san_opts})
      target_link_options(${tgt} PRIVATE ${san_opts})
    endif()
  endif()
endfunction()
```

### Dependencies

- Use `find_package()` with explicit versions.
- Vendored dependencies go in `third_party/`.
- Allow opt-in FetchContent with explicit toggle:

```cmake
option(PROJECT_ALLOW_FETCHCONTENT "Allow online dependency fetch" OFF)
if (PROJECT_ALLOW_FETCHCONTENT)
  include(FetchContent)
endif()
```

### Install & Export

Use `GNUInstallDirs` and proper export sets:

```cmake
install(TARGETS project_lib EXPORT projectTargets)
install(EXPORT projectTargets NAMESPACE project:: DESTINATION lib/cmake/project)
```

### Testing

Use CTest with structured names:

```cmake
include(CTest)
if (BUILD_TESTING)
  add_executable(project_test tests/foo_test.cpp)
  target_link_libraries(project_test PRIVATE project::lib GTest::gtest_main)
  add_test(NAME project.foo.test COMMAND project_test)
endif()
```

### Tooling

Expose these optional integrations:

- `CMAKE_CXX_CLANG_TIDY`
- `CMAKE_CXX_CPPCHECK`

### Presets

```jsonc
{
  "version": 6,
  "configurePresets": [
    { "name": "dev-unix", "generator": "Ninja", "binaryDir": "build/dev-unix",
      "cacheVariables": { "CMAKE_BUILD_TYPE":"Debug", "PROJECT_WARNINGS_AS_ERRORS":"ON" } },
    { "name": "dev-msvc", "generator": "Ninja Multi-Config", "binaryDir": "build/dev-msvc",
      "cacheVariables": { "CMAKE_CONFIGURATION_TYPES":"Debug;Release" } }
  ]
}
```

## Disallowed Practices

- Global compile flags or directory-wide includes
- Implicit `file(GLOB)` sources
- Linking raw library names instead of imported targets
- Compiler extensions or unversioned `find_package`
- Hidden network fetches
- Policy suppression without comment

## Target Templates

**Library**

```cmake
add_library(project_lib STATIC)
add_library(project::lib ALIAS project_lib)

target_sources(project_lib PRIVATE src/foo.cpp include/project/foo.hpp)
target_include_directories(project_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
```

**Executable**

```cmake
add_executable(project_tool src/main.cpp)
add_executable(project::tool ALIAS project_tool)

target_link_libraries(project_tool PRIVATE project::lib)
```

**Test**

```cmake
include(CTest)
if (BUILD_TESTING)
  add_executable(project_foo_test tests/foo_test.cpp)
  target_link_libraries(project_foo_test PRIVATE project::lib GTest::gtest_main)
  add_test(NAME project.foo.test COMMAND project_foo_test)
endif()
```

## Copilot User Commands

### 1. Add Library

**Trigger**: User requests: *add library `<name>`*
**Process**:

- Create a new `add_library()` target using the specified name.
- Add an alias target under the project namespace (`<project|component|module>::<name>`).
- Use `target_sources()` to explicitly list source and header files. Use file sets introduced in cmake 3.23 for headers or module files.
- Add include directories using `target_include_directories()` with build and install interfaces.
- Link any required internal or external dependencies if provided.
- Provide an example snippet for clarity.

### 2. Add Executable

**Trigger**: User requests: *add executable `<name>`*.
**Process**:

- Create a new `add_executable()` target using the specified name.
- Use `target_sources()` to explicitly list source files.
- Link the executable against relevant libraries.
- Apply consistent compiler and sanitizer settings to ensure reproducibility.
- Provide a short example block of how the executable target should look.

### 3. Add Test

**Trigger**: User requests: *add test for `<target|module|symbol>`*
**Process**:

- Ensure `RENDERER_BUILD_TESTS` is enabled and `CTest` is included.
- Ensure `tests/` subdirectory exists for project; Create if missing. Add tests subdirectory via `add_test_subdirectory(dir)`.
- Create a test executable target under `tests/` directory, use `add_test_executable()` to create the executable (reference: `tools/cmake/modules/utils.cmake` for details).
- Verify consistent naming and proper isolation of test dependencies.
- Write test code using Catch2. Check against c++ instructions for further details.

### 4. Add Benchmark

**Trigger**: User requests: *add benchmark for `<target|module|symbol>`*
**Process**:

- Ensure `RENDERER_BUILD_BENCHMARKS` option is enabled.
- Ensure `benchmarks/` subdirectory exists for project; Create if missing. Add benchmarks subdirectory via `add_benchmark_subdirectory(dir)`.
- Create a benchmark executable target under `benchmarks/` directory, use `add_benchmark_executable()` to create the executable (reference: `tools/cmake/modules/utils.cmake` for details).
- Verify consistent naming and proper isolation of benchmark dependencies.
- Write benchmark code using Google Benchmark framework. Check against c++ instructions for further details.

### 5. Add Install/Export Rules

**Trigger**: User requests: *add install/export rules for `<target>`*
**Process**:

- Add appropriate `install(TARGETS ...)` and `install(EXPORT ...)` commands for the given target.
- Define a namespace (e.g. `core::`) and destination paths following `GNUInstallDirs`.
- Ensure generated config and version files are properly placed under `cmake/`.
- Validate installation via `cmake --install . --prefix <path>`.
