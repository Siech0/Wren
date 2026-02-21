# C++ Style Guide {#cpp_style}

This document is the authoritative reference for C++ style in this repository.
All rules are enforced by tooling; `.clang-format` for layout and `.clang-tidy`
for static analysis. Where the tools cannot catch something, this document defines
the convention.

______________________________________________________________________

## Table of Contents

1. [Language Standard & Compiler Flags](#1-language-standard--compiler-flags)
1. [Naming Conventions](#2-naming-conventions)
1. [Formatting](#3-formatting)
1. [File & Header Organisation](#4-file--header-organisation)
1. [Includes](#5-includes)
1. [Namespaces](#6-namespaces)
1. [Types & Classes](#7-types--classes)
1. [Functions](#8-functions)
1. [Error Handling](#9-error-handling)
1. [Comments & Documentation](#10-comments--documentation)
1. [Control Flow](#11-control-flow)
1. [Static Analysis: clang-tidy](#12-static-analysis-clang-tidy)

______________________________________________________________________

## 1. Language Standard & Compiler Flags

| Setting    | Value                                              |
| ---------- | -------------------------------------------------- |
| Standard   | **C++23**                                          |
| Extensions | **Disabled** (`-pedantic-errors` / `/permissive-`) |
| Warnings   | All on, treated as errors in CI                    |

Prefer modules for new library code. Use the `.ixx` / `.cppm` extension for
module interfaces and plain `.cpp` for module implementation units.

______________________________________________________________________

## 2. Naming Conventions

### Summary Table

| Construct                          | Convention                                 | Example                                    |
| ---------------------------------- | ------------------------------------------ | ------------------------------------------ |
| Namespaces                         | `snake_case`                               | `wren::rhi`, `wren::foundation`            |
| Types; class, struct, concept      | `PascalCase`                               | `BackendLibrary`, `DeviceLimits`           |
| Type aliases                       | `PascalCase`                               | `WrenRhiCreateFn`                          |
| `enum class` values                | `PascalCase`                               | `Backend::Vulkan`, `Feature::None`         |
| Functions (free & member)          | `snake_case`                               | `backend_id()`, `platform_load()`          |
| Local variables                    | `snake_case`                               | `lib_handle`, `dll_name`                   |
| Function parameters                | `snake_case`                               | `which`, `name`, `size`                    |
| Private/protected member variables | `snake_case_` *(trailing underscore)*      | `backend_`, `lib_handle_`                  |
| `constexpr` constants (any scope)  | `k_snake_case`                             | `k_backend_abi_version`, `k_build_suffix`  |
| Macros (last resort only)          | `SCREAMING_SNAKE_CASE` with project prefix | `WREN_RHI_VULKAN_EXPORT`                   |
| Header guard macros                | `SCREAMING_SNAKE_CASE`                     | `WREN_RHI_API_FEATURES_HPP`                |
| Template parameters                | `PascalCase`                               | `template<Enum E>`, `template<typename T>` |

### Notable Points

**`k_` prefix for constants**. All `constexpr` variables use the `k_` prefix,
regardless of scope.

```cpp
inline constexpr uint32_t k_backend_abi_version = 1;

static constexpr const char k_build_suffix[] = "d";
```

**`s_` prefix for statics**. All `static` variables use the `s_` prefix,
regardless of scope.

**Trailing underscore for member variables**. Only private/protected members
carry the underscore. Public data members of plain aggregates (`struct`) do not:

```cpp
// Class: trailing underscore
class BackendLibrary {
    WrenRhiBackend* backend_    = nullptr;
    void*           lib_handle_ = nullptr;
};

// Plain aggregate: no trailing underscore
struct DeviceDesc {
    void*    nativeWindowHandle    = nullptr;
    uint32_t preferredAdapterIndex = 0;
};
```

**`enum class` values are `PascalCase`**, not `SCREAMING_SNAKE_CASE`. Using
`enum class` already scopes every value; all-caps would be redundant:

```cpp
enum class Backend : std::uint8_t { OpenGL, Vulkan, D3D12, Metal, None };
enum class Feature  : std::uint64_t { None = 0, Tessellation = 1ull << 0, … };
```

**C ABI types follow C conventions**. Structs and typedefs in `extern "C"`
blocks use `PascalCase` with the `Wren` project prefix to avoid collisions
with C names in the global namespace:

```cpp
extern "C" {
struct WrenRhiBackend { … };
typedef WrenRhiBackend* (*WrenRhiCreateFn)(void);
}
```

______________________________________________________________________

## 3. Formatting

All formatting is enforced by `.clang-format`. Run it before every commit
(pre-commit does this automatically). The key decisions are:

| Rule                        | Value                                        |
| --------------------------- | -------------------------------------------- |
| Indent width                | **2 spaces**                                 |
| Tab characters              | **Never**                                    |
| Column limit                | **80**                                       |
| Pointer/reference alignment | **Right** (`int *p`, `int &r`)               |
| Brace style                 | **Attach** (K&R. Opening brace on same line) |
| Short functions on one line | **Allowed**                                  |
| Short `if` on one line      | **Never**                                    |
| Short loops on one line     | **Never**                                    |
| Constructor initialisers    | **Break before colon**                       |
| Access modifier offset      | **-2** (dedented 2 spaces from class body)   |
| Includes                    | **Sorted, case-sensitive, preserve blocks**  |

### Indentation & Braces

```cpp
// Correct. Braces attach, 2-space indent
class Foo {
public:
  void bar() {
    if (condition) {
      do_something();
    } else {
      do_other();
    }
  }
};
```

```cpp
// Correct. Constructor initialiser list
BackendLibrary::BackendLibrary(WrenRhiBackend* backend, void* lib_handle) noexcept
    : backend_(backend),
      lib_handle_(lib_handle) {}
```

### Pointer & Reference Syntax

```cpp
int *ptr;        // right-aligned: * binds to name
const char *msg;
void *handle;

auto &ref = value;   // & also right-aligned
```

### Line Length

The hard limit is **80 characters**. clang-format wraps automatically. For
long function signatures, break before each parameter when they do not fit on
one line:

```cpp
[[nodiscard]] static auto load(Backend which)
    -> std::expected<BackendLibrary, std::string>;
```

______________________________________________________________________

## 4. File & Header Organisation

| File type                        | Extension |
| -------------------------------- | --------- |
| C++ header                       | `.hpp`    |
| C header (C ABI / shared with C) | `.h`      |
| C++ implementation               | `.cpp`    |

**Header guards**; Use `#ifndef` + `#define` guards (not `#pragma once`)
for headers that may be included from C. For pure C++ headers, `#pragma once`
is acceptable. The guard macro must match the full include path, uppercased
with underscores:

```cpp
// File: include/wren/rhi/api/features.hpp
#ifndef WREN_RHI_API_FEATURES_HPP
#define WREN_RHI_API_FEATURES_HPP
// …
#endif // WREN_RHI_API_FEATURES_HPP
```

**Headers must be self-contained**. Every header must compile in isolation.
Never rely on a header being included before yours.

______________________________________________________________________

## 5. Includes

Order includes in the following groups, separated by a blank line and sorted
case-sensitively within each group (clang-format enforces this):

1. Corresponding header (in `.cpp` files)
1. Project headers (e.g `<wren/…>` )
1. Third-party library headers
1. C++ standard library headers
1. C standard library headers (`<cstdint>`, `<cstring>`, etc.)

```cpp
#include <wren/rhi/loader.hpp>            // 1 own header first

#include <wren/rhi/api/enums.hpp>         // 2 project headers
#include <wren/rhi/backend.h>

#include <glfw/glfw.h>                    // 3 third-party (none here)

#include <expected>                       // 4 C++ stdlib
#include <string>

#include <cassert>                        // 5 C stdlib wrappers
#include <cstdint>
#include <cstring>
```

Platform-specific headers (`<windows.h>`, `<linux.h>`) go in their own
`#ifdef` block, after the project includes:

```cpp
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#else
#  include <linux.h>
#endif
```

Rules:

- **Include what you use**; do not rely on transitive includes.
- **No `using namespace` in headers**. Ever.
- Prefer `<cstdint>` over `<stdint.h>`.

______________________________________________________________________

## 6. Namespaces

Namespace names are `snake_case`. Nest them using the C++17 `::` syntax:

```cpp
namespace wren::rhi {
// content
} // namespace wren::rhi
```

Always close with a `// namespace <name>` comment. Never indent namespace
contents.

**Anonymous namespaces vs `static`** for translation-unit-local (file-scope)
functions, either `static` or anonymous namespace is acceptable. The codebase
currently uses `static` for file-scope helpers:

```cpp
static void* platform_load(const char* name) noexcept { … }
static void  platform_unload(void* handle)   noexcept { … }
```

`using namespace` is forbidden in headers. In `.cpp` files it is permitted but
not encouraged. Prefer explicit qualification.

______________________________________________________________________

## 7. Types & Classes

### Aggregates vs Classes

Use a plain `struct` (no access specifiers, no invariants) for pure data
carriers; descriptors, limit structs, POD options:

```cpp
struct DeviceDesc {
    void*                nativeWindowHandle    = nullptr;
    uint32_t             preferredAdapterIndex = 0;
    DeviceFlag           flags                 = DeviceFlag::None;
    DeviceFeatureRequest featureRequest{};
};
```

Use `class` when the type has invariants, hidden state, or non-trivial
lifetime management.

### Member Ordering

Within a `class`, order sections as:

1. `public:`. interface (constructors, factories, methods)
1. `protected:`. extension points (if any)
1. `private:`. implementation details and member variables

Access modifier keyword is dedented by 2 from the class body (`AccessModifierOffset: -2`):

```cpp
class BackendLibrary {
public:
  [[nodiscard]] static auto load(Backend which)
      -> std::expected<BackendLibrary, std::string>;

  [[nodiscard]] Backend backend_id() const noexcept;

  BackendLibrary(BackendLibrary&& other) noexcept;
  BackendLibrary& operator=(BackendLibrary&& other) noexcept;
  ~BackendLibrary();

  BackendLibrary(const BackendLibrary&)            = delete;
  BackendLibrary& operator=(const BackendLibrary&) = delete;

private:
  explicit BackendLibrary(WrenRhiBackend* backend, void* lib_handle) noexcept;

  WrenRhiBackend* backend_    = nullptr;
  void*           lib_handle_ = nullptr;
};
```

### Rules of Five / Zero

Explicitly `= delete` or `= default` all five special member functions whenever
you define any one of them (enforced by `cppcoreguidelines-special-member-functions`).

Move-only types must explicitly delete the copy constructor and copy
assignment operator.

### Enum Classes

Always use `enum class`. Always specify an explicit underlying integral type.
Prefer the narrowest type that fits all values:

```cpp
enum class Backend    : std::uint8_t  { … };
enum class ShaderStage: std::uint32_t { None = 0, Vertex = 1u << 0, … };
enum class Feature    : std::uint64_t { None = 0, Tessellation = 1ull << 0, … };
```

For flag enums, opt in to bitwise operators via the `enable_flags` mechanism
from `<wren/foundation/utility/enum_utils.hpp>`. Put the specialisation
immediately after the enum definition:

```cpp
enum class ShaderStage : std::uint32_t { … };
template<> struct wren::foundation::enable_flags<ShaderStage> : std::true_type {};
```

### Concepts & Templates

Concept names are `PascalCase`. Template type parameter names are `PascalCase`.
Prefer named concepts over raw `typename` constraints:

```cpp
template<class E>
concept Enum = std::is_enum_v<E>;

template<Enum E>
constexpr auto underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
```

______________________________________________________________________

## 8. Functions

### Attributes

Mark every function that returns a value the caller must not silently discard
with `[[nodiscard]]`. This includes factories, error-returning functions,
and pure query functions:

```cpp
[[nodiscard]] static auto load(Backend which)
    -> std::expected<BackendLibrary, std::string>;

[[nodiscard]] Backend backend_id() const noexcept;
```

Mark functions that cannot throw `noexcept`. This includes platform helpers,
destructors, getters over trivial state, and move constructors:

```cpp
static void platform_unload(void* handle) noexcept;
BackendLibrary(BackendLibrary&&) noexcept;
```

### Return Type Syntax

Prefer the trailing return type (`-> T`) for non-trivial return types,
especially when returning `std::expected` or template-dependent types. Use
the leading return type only for simple cases where it aids readability:

```cpp
// preferred for complex returns
[[nodiscard]] auto load(Backend which)
    -> std::expected<BackendLibrary, std::string>;

// fine for simple cases
[[nodiscard]] Backend backend_id() const noexcept;
```

### Parameter Passing

| Scenario                                  | Convention                      |
| ----------------------------------------- | ------------------------------- |
| Cheap to copy (`int`, `Backend`, pointer) | Pass by value                   |
| Read-only string                          | `std::string_view`              |
| Read-only object                          | `const T&`                      |
| Output (avoid where possible)             | Return value or `std::expected` |
| Polymorphic callback                      | `std::function<…>` or template  |

Avoid output parameters. Return values or `std::expected` instead.

### `constexpr` & `inline`

Prefer `constexpr` over `inline`. A `constexpr` function is implicitly
`inline`. Prefer `constexpr` over macros for constants:

```cpp
// Correct
inline constexpr uint32_t k_backend_abi_version = 1;

// Wrong
#define WREN_RHI_BACKEND_ABI_VERSION 1
```

______________________________________________________________________

## 9. Error Handling

Use `std::expected<T, E>` for recoverable errors in any function that can
fail for legitimate reasons. `E` is typically `std::string` for human-readable
messages or a dedicated error type for richer context:

```cpp
[[nodiscard]] static auto load(Backend which)
    -> std::expected<BackendLibrary, std::string>;

// Failure path
if (!handle) {
    return std::unexpected{"Failed to load '" + name + "': " + platform_last_error()};
}
```

Use `assert` or `static_assert` for developer preconditions that indicate programmer error;
things that should never happen if the code is correct. `assert`s are stripped
in release builds and must not have side-effects:

```cpp
assert(backend != nullptr);
```

**Exceptions** are permitted for truly exceptional (unrecoverable) situations.
Never throw across ABI boundaries. Never throw from destructors.

`catch (...)` is only acceptable at top-level boundary handlers (e.g., `main`
or a plugin host boundary).

______________________________________________________________________

## 10. Comments & Documentation

### File-Scope Section Banners

Use dashed banner comments to divide a `.cpp` or `.hpp` file into logical
sections. Use 97-character width (fills to column 99 with the `// `):

```cpp
// -------------------------------------------------------------------------------------------------
// Platform helpers
// -------------------------------------------------------------------------------------------------
```

### Inline Comments

Trailing comments are aligned by clang-format. Place them one space after the
code:

```cpp
uint32_t abi_version; // must equal wren::rhi::k_backend_abi_version
```

### Documentation Comments

Use `///` triple-slash for Doxygen/documentation-generating comments on
declarations. Use `//` for explanatory implementation comments:

```cpp
/// Load a backend DLL for the given Backend enum value.
/// Returns an error string on failure.
[[nodiscard]] static auto load(Backend which)
    -> std::expected<BackendLibrary, std::string>;
```

Document preconditions, complexity, and exception guarantees where non-obvious:

```cpp
/// Parses configuration from text.
/// Strong exception guarantee. O(n).
auto parse_config(std::string_view input)
    -> std::expected<Config, ParseError>;
```

### Cross-API Annotations in Headers

For enums and types that map to multiple graphics API concepts, annotate each
enumerator with the per-API equivalent and a link:

```cpp
// ===================================================================================
// Depth/stencil tests & ops
//   Compare: VkCompareOp — https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompareOp.html
//            glDepthFunc — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
//            D3D12_COMPARISON_FUNC — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_comparison_func
//            MTLCompareFunction — https://developer.apple.com/documentation/metal/mtlcomparefunction
//   Stencil: VkStencilOp — https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOp.html
//            glStencilOp — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilOp.xhtml
//            D3D12_STENCIL_OP — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_stencil_op
//            MTLStencilOperation — https://developer.apple.com/documentation/metal/mtlstenciloperation
// ===================================================================================
enum class CompareOp : std::uint8_t {
    Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always
    // Maps directly across VK/GL/D3D12/Metal
};

enum class StencilOp : std::uint8_t {
    Keep,            // Keep existing value
    Zero,            // Write 0
    Replace,         // Write ref
    IncrementClamp,  // VK: INCREMENT_AND_CLAMP | GL: GL_INCR | D3D12: INCR_SAT | Metal: incrementClamp
    DecrementClamp,  // VK: DECREMENT_AND_CLAMP | GL: GL_DECR | D3D12: DECR_SAT | Metal: decrementClamp
    Invert,          // Bitwise invert
    IncrementWrap,   // VK: INCREMENT_AND_WRAP | GL: INCR_WRAP | D3D12: INCR | Metal: incrementWrap
    DecrementWrap    // VK: DECREMENT_AND_WRAP | GL: DECR_WRAP | D3D12: DECR | Metal: decrementWrap
};

```

______________________________________________________________________

## 11. Control Flow

### Early Return / Guard Clauses

Prefer early returns over deeply nested `if`/`else` trees. This reduces
visual nesting and makes the happy path obvious:

```cpp
// Correct guard at the top
auto BackendLibrary::load(Backend which) -> std::expected<BackendLibrary, std::string> {
    const auto name = dll_name(which);
    if (name.empty()) {
        return std::unexpected{"Backend::None cannot be loaded"};
    }

    void* handle = platform_load(name.c_str());
    if (!handle) {
        return std::unexpected{"Failed to load '" + name + "': " + platform_last_error()};
    }
    // … happy path continues here
}
```

The `readability-else-after-return` clang-tidy check enforces that `else`
is not used after a `return` (or `throw`/`continue`/`break`).

### `switch` Statements

Every `switch` on an `enum class` must handle all enumerators or have an
explicit `default`. Clang-tidy's `hicpp-multiway-paths-covered` enforces
exhaustiveness.

______________________________________________________________________

## 12. Static Analysis: Clang-tidy

The `.clang-tidy` configuration enables a broad set of checks. Key enabled
groups:

| Group                            | What it catches                                                                      |
| -------------------------------- | ------------------------------------------------------------------------------------ |
| `bugprone-*`                     | Common bugs: suspicious string compare, misplaced widening casts, etc.               |
| `cert-*`                         | CERT secure-coding rules                                                             |
| `cppcoreguidelines-*` (selected) | Narrowing conversions, `goto`, `malloc`, non-const globals, special member functions |
| `modernize-*`                    | C++11–23 modernisation opportunities                                                 |
| `performance-*`                  | Unnecessary copies, inefficient moves, etc.                                          |
| `readability-*`                  | Identifier naming, else-after-return, boolean simplification, magic numbers          |
| `google-*` (selected)            | No `using namespace`, explicit constructors, casting style                           |
| `misc-*`                         | General miscellany; `misc-include-cleaner` is disabled (too noisy)                   |

Notable check options enabled:

- `bugprone-argument-comment.CommentBoolLiterals`: flag bare `true`/`false`
  arguments; prefer `enum class` with two values when it aids readability.
- `cppcoreguidelines-narrowing-conversions.PedanticMode`: flag all narrowing,
  including `ptrdiff_t` → `int`.
- `readability-qualified-auto.AddConstToQualified`: `auto` pointers from
  `const` containers become `const auto*`.

Run the full check set locally with:

```bash
cmake --preset dev-msvc   # configure
ninja -C build clang-tidy # or: run-clang-tidy.py -p build
```
