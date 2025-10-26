#pragma once

#include <core/export.hpp>

// NOLINTBEGIN

#define RENDERER_VERSION_MAJOR 0
#define RENDERER_VERSION_MINOR 1
#define RENDERER_VERSION_PATCH 0

#define RENDERER_VERSION                                                       \
RENDERER_VERSION_MAJOR * 10000u + RENDERER_VERSION_MINOR * 100u +              \
RENDERER_VERSION_PATCH

namespace core {
    auto CORE_EXPORT version_major() -> unsigned int;
    auto CORE_EXPORT version_minor() -> unsigned int;
    auto CORE_EXPORT version_patch() -> unsigned int;
    auto CORE_EXPORT version() -> unsigned int;
    auto CORE_EXPORT version_string() -> char const *;
}

// NOLINTEND
