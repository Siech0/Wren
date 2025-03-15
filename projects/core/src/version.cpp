#include <core/version.hpp>

// NOLINTBEGIN
namespace core {
    // cppcheck-suppress unusedFunction
    auto version_major() -> unsigned int { return RENDERER_VERSION_MAJOR; }
    // cppcheck-suppress unusedFunction
    auto version_minor() -> unsigned int { return RENDERER_VERSION_MINOR; }
    // cppcheck-suppress unusedFunction
    auto version_patch() -> unsigned int { return RENDERER_VERSION_PATCH; }
    // cppcheck-suppress unusedFunction
    auto version() -> unsigned int { return RENDERER_VERSION; }

    #define STRINGIFY(x) #x
    #define EXP_STRINGIFY(x) STRINGIFY(x)
    // cppcheck-suppress unusedFunction
    auto version_string() -> char const * { return EXP_STRINGIFY(RENDERER_VERSION_MAJOR) "." EXP_STRINGIFY(RENDERER_VERSION_MINOR) "." EXP_STRINGIFY(RENDERER_VERSION_PATCH); }
    #undef STRINGIFY
    #undef EXP_STRINGIFY
}
// NOLINTEND
