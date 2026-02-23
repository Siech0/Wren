#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <wren/rhi/api/features.hpp>
#include <wren/rhi/vulkan/export.hpp>

namespace wren::rhi::vulkan {

// -------------------------------------------------------------------------------------------------
// AdapterKind: physical device category.
//
//  Maps directly to VkPhysicalDeviceType:
//    VK_PHYSICAL_DEVICE_TYPE_OTHER          -> Other
//    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU -> Integrated
//    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU   -> Discrete
//    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU    -> Virtual
//    VK_PHYSICAL_DEVICE_TYPE_CPU            -> CPU
// -------------------------------------------------------------------------------------------------
enum class AdapterKind : uint8_t {
    Other      = 0,
    Integrated = 1,
    Discrete   = 2,
    Virtual    = 3,
    CPU        = 4,
};

[[nodiscard]] inline const char* to_string(AdapterKind k) noexcept {
    switch (k) {
        case AdapterKind::Integrated: return "Integrated";
        case AdapterKind::Discrete:   return "Discrete";
        case AdapterKind::Virtual:    return "Virtual";
        case AdapterKind::CPU:        return "CPU";
        default:                      return "Other";
    }
}

// -------------------------------------------------------------------------------------------------
// AdapterInfo: read-only snapshot of a physical GPU's identity and capabilities.
//
// Populated by VulkanInstance::enumerate_adapters() before any logical device
// is created. Use this to:
//   - Present a GPU picker UI.
//   - Select the best adapter for a given DeviceFeatureRequest.
//   - Validate that required features are present before calling device creation.
//
// video_memory_bytes approximates dedicated device-local heap size.
// For integrated GPUs sharing system RAM this value may be zero or reflect
// the shared aperture, not total system RAM.
//
// -------------------------------------------------------------------------------------------------
struct WREN_RHI_VULKAN_EXPORT AdapterInfo {
    uint32_t     index;               ///< Zero-based index in the instance's physical device list.
    std::string  name;                ///< VkPhysicalDeviceProperties::deviceName
    AdapterKind  kind;                ///< Integrated / Discrete / Virtual / CPU / Other
    uint64_t     video_memory_bytes;  ///< Approximate DEVICE_LOCAL heap size in bytes.
    uint32_t     driver_version;      ///< Raw VkPhysicalDeviceProperties::driverVersion.
    uint32_t     api_version_major;   ///< Supported Vulkan API major version.
    uint32_t     api_version_minor;   ///< Supported Vulkan API minor version.
    Capabilities capabilities;        ///< Feature flags + numeric limits snapshot.
};

} // namespace wren::rhi::vulkan
