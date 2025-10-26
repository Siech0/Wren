---
applyTo: '**/*.{h,hpp,hxx,hh,ipp,cpp,cc,cxx,ixx,cppm,tpp}'
---

# C++ Instructions

All C++ in this repository must follow **C++23** with compiler extensions **disabled**. Favor safety, clarity, and maintainability over micro-optimization.

## Constraints

- **Standard**: `C++23`
- **Extensions**: Disabled (`-pedantic-errors`, `/permissive-`)
- **Warnings**: All on, treated as errors in CI
- **No raw memory management** unless encapsulated
- **Header self-containment** enforced

## Expected Practices

### Language & Safety

- RAII everywhere — no naked `new`/`delete`.
- Value semantics first; explicit ownership via `std::unique_ptr`.
- Use `std::span`, `std::string_view`, `std::optional`, `std::expected`.
- Mark significant returns `[[nodiscard]]`.
- Prefer `constexpr` and `noexcept`.
- Avoid implicit narrowing; prefer brace initialization.
- Use `enum class` instead of plain enums.
- Use standard `<chrono>` and `<filesystem>` types.
- Avoid shared mutable state; concurrency via `std::jthread` + `stop_token`.

### Design & Interfaces

- Headers must be self-contained.
- Avoid transitive includes; include what you use.
- Keep APIs narrow and type-safe.
- No `using namespace` in headers.
- Pass by value if cheap; else `const&`.
- Return `std::optional` or `std::expected` instead of out-params.
- Document preconditions and complexity.

### Modules

Prefer modules for new code:

**Interface**

```cpp
export module project.math;

export namespace project::math {
  double lerp(double a, double b, double t) noexcept;
}
```

**Implementation**

```cpp
module project.math;

namespace project::math {
  double lerp(double a, double b, double t) noexcept {
    return a + (b - a) * t;
  }
}
```

### Error Handling

- Use `std::expected` for recoverable errors.
- Exceptions are fine for truly exceptional paths only.
- Never throw from destructors.
- `assert` for developer assumptions.

### Naming & Style

- **Types / Concepts**: `PascalCase`
- **Functions / Variables / Namespaces**: `snake_case`
- **Constants / Enums**: `SCREAMING_SNAKE_CASE`
- **Files**: `.hpp` for headers, `.cpp` for impl, `.ixx`/`.cppm` for modules
- **Include guards**: `#pragma once`
- **Max line length**: 120 chars

### Testing

- Every public API should have at least one unit test.
- Deterministic test behavior (no random seeds/time).
- Avoid global/static state in tests.

### Performance

- Prefer STL algorithms/ranges over manual loops.
- Use `reserve()` where possible.
- Avoid redundant `std::move` on returns.
- Benchmark before optimizing.

### Documentation

```cpp
/// Parses configuration from text.
/// Strong exception guarantee. O(n).
auto parse_config(std::string_view)
  -> std::expected<config, parse_error>;
```

## Disallowed Practices

- Raw memory management
- Global `using namespace`
- Non-constexpr global state
- Macro-based metaprogramming
- `reinterpret_cast` without strong justification
- `catch (...)` except at top-level boundaries
- Throwing exceptions across ABI boundaries

## Idioms & Snippets

**Header skeleton**

```cpp
#pragma once
#include <expected>
#include <string_view>

namespace project {

class widget {
public:
  [[nodiscard]] static auto make(std::string_view name)
    -> std::expected<widget, std::string>;

  [[nodiscard]] auto id() const noexcept -> std::uint32_t;

private:
  explicit widget(std::uint32_t id) noexcept : id_{id} {}
  std::uint32_t id_{};
};

} // namespace project
```

**Ranges**

```cpp
#include <ranges>
#include <vector>

auto evens(std::vector<int> const& xs) {
  return xs | std::views::filter([](int x){ return x % 2 == 0; });
}
```

**Concurrency**

```cpp
#include <stop_token>
#include <thread>

void run_worker(std::stop_token st) {
  while (!st.stop_requested()) {
    // work
  }
}

void start() {
  std::jthread t(run_worker);
  // stop via t.request_stop();
}
```

## Copilot User Commands

### 1. Add Unit Test

**Trigger**: User requests: *add unit test for `<symbol>`*
**Process**:

- Create a test source file under `tests/` matching the symbol’s area or module.
- Use the standard testing framework (e.g., GTest or Catch2).
- Write deterministic tests covering success, error, and edge cases.
- Follow naming convention `project.<area>.<symbol>.<behavior>`.
- Add the new test target to CMake and register it via `add_test()`.

### 2. Explain Warning

**Trigger**: User requests: *explain warning `<code>`*
**Process**:

- Look up the compiler diagnostic by warning ID or message.
- Explain the root cause in C++23 terms (e.g., lifetime issue, narrowing, UB risk).
- Suggest the best-practice fix—prefer code changes over disabling warnings.
- If relevant, show before/after code examples to illustrate the correct form.
