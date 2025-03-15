#ifndef CORE_VERSION_HPP
#define CORE_VERSION_HPP

// NOLINTBEGIN

#define RENDERER_VERSION_MAJOR 0
#define RENDERER_VERSION_MINOR 1
#define RENDERER_VERSION_PATCH 0

#define RENDERER_VERSION                                                       \
RENDERER_VERSION_MAJOR * 10000u + RENDERER_VERSION_MINOR * 100u +              \
RENDERER_VERSION_PATCH

namespace core {
    auto version_major() -> unsigned int;
    auto version_minor() -> unsigned int;
    auto version_patch() -> unsigned int;
    auto version() -> unsigned int;
    auto version_string() -> char const *;
}

// NOLINTEND

#endif // CORE_VERSION_HPP
