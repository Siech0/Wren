#pragma once

#ifndef EXPORTED_VULKAN_FUNCTION
#define EXPORTED_VULKAN_FUNCTION(function)
#endif
#undef EXPORTED_VULKAN_FUNCTION
//
#ifndef GLOBAL_LEVEL_VULKAN_FUNCTION
#define GLOBAL_LEVEL_VULKAN_FUNCTION(function)
#endif
#undef GLOBAL_LEVEL_VULKAN_FUNCTION
//
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION
#define INSTANCE_LEVEL_VULKAN_FUNCTION(function)
#endif
#undef INSTANCE_LEVEL_VULKAN_FUNCTION
//
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(function, extension)
#endif
#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
//
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION
#define DEVICE_LEVEL_VULKAN_FUNCTION(function)
#endif
#undef DEVICE_LEVEL_VULKAN_FUNCTION
//
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(function, extension)
#endif
#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

#include <export/render_core_export.hpp>

#include <utility>
#include <vulkan/vulkan.h>

// NOLINTNEXTLINE
#define EXPORTED_VULKAN_FUNCTION(name) extern PFN_##name name;
// NOLINTNEXTLINE
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
// NOLINTNEXTLINE
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
// NOLINTNEXTLINE
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  extern PFN_##name name;
// NOLINTNEXTLINE
#define DEVICE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
// NOLINTNEXTLINE
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  extern PFN_##name name;

#include <core/vulkan/vulkan_functions.inl>

#undef GLOBAL_LEVEL_VULKAN_FUNCTION
#undef EXPORTED_VULKAN_FUNCTION
#undef INSTANCE_LEVEL_VULKAN_FUNCTION
#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#undef DEVICE_LEVEL_VULKAN_FUNCTION
#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

namespace core::vk::loader {

/**
 * @brief Loads the Vulkan library.
 * @return True on successful load, false otherwise.
 */
auto load_vulkan() noexcept -> bool;

/**
 * @brief Unloads the Bulkan library.
 */
auto unload_vulkan() noexcept -> bool;

} // namespace core::vk::loader
