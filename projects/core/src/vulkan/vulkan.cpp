#include <core/vulkan/vulkan.hpp>

#include <iostream>
#include <mutex>

#if defined(_WIN32)

#define WINDOWS_LEAN_AND_MEAN
#include "windows.h"
#define load_function GetProcAddress // NOLINT
using library_type = HMODULE;

#elif defined(__linux)

#include <dlfcn.h>
#define load_function dlsym // NOLINT
using library_type = void *;

#endif

// NOLINTNEXTLINE
#define EXPORTED_VULKAN_FUNCTION(name) PFN_##name name;
// NOLINTNEXTLINE
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
// NOLINTNEXTLINE
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
// NOLINTNEXTLINE
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  PFN_##name name;
// NOLINTNEXTLINE
#define DEVICE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
// NOLINTNEXTLINE
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  PFN_##name name;

#include <core/vulkan/vulkan_functions.inl>

#undef EXPORTED_VULKAN_FUNCTION
#undef GLOBAL_LEVEL_VULKAN_FUNCTION
#undef INSTANCE_LEVEL_VULKAN_FUNCTION
#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#undef DEVICE_LEVEL_VULKAN_FUNCTION
#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

namespace core::vk::loader {

namespace {
// We should never expect to see this being called more than once, but just in
// case lets add some
//   low-cost safety.
std::mutex initialization_lock;        // NOLINT
bool vulkan_initialized = false;       // NOLINT
library_type vulkan_library = nullptr; // NOLINT

} // namespace

auto load_vulkan() noexcept -> bool {
  const std::lock_guard<std::mutex> lock(initialization_lock);
  if (vulkan_initialized) {
    return true;
  }

#if defined _WIN32
  vulkan_library = LoadLibrary("vulkan-1.dll");
#elif defined __linux
  vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);
#endif

// NOLINTNEXTLINE
#define EXPORTED_VULKAN_FUNCTION(name)                                         \
  name = reinterpret_cast<PFN_##name>(load_function(vulkan_library, #name));   \
  if ((name) == nullptr) {                                                     \
    std::cerr << "Could not load exported Vulkan function named: " << #name    \
              << std::endl;                                                    \
    return false;                                                              \
  }

// NOLINTNEXTLINE
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(nullptr, #name));  \
  if ((name) == nullptr) {                                                     \
    std::cerr << "Could not load global-level function named: " << #name       \
              << std::endl;                                                    \
    return false;                                                              \
  }

#include <core/vulkan/vulkan_functions.inl>

  vulkan_initialized = true;
  return true;
}

auto unload_vulkan() noexcept -> bool {
  const std::lock_guard<std::mutex> lock(initialization_lock);
  if (!vulkan_initialized) {
    return true;
  }

#if defined _WIN32
  FreeLibrary(vulkan_library);
#elif defined __linux
  dlclose(vulkan_library);
#endif

  vulkan_initialized = false;
  return true;
}

} // namespace core::vk::loader
