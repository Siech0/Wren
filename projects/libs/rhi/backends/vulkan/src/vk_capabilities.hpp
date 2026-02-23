#pragma once

// Internal header — not installed, not part of the public API.
// Translates VkPhysicalDevice properties/features/extensions into the
// engine-level wren::rhi::Capabilities aggregate.

#include <span>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include <wren/rhi/api/features.hpp>
#include <wren/rhi/vulkan/adapter.hpp>

namespace wren::rhi::vulkan::detail {

// -------------------------------------------------------------------------------------------------
// Extension helpers
// -------------------------------------------------------------------------------------------------

/// Returns true when @p name appears in @p extensions.
[[nodiscard]] inline bool has_extension(
    std::span<vk::ExtensionProperties const> extensions,
    std::string_view                         name) noexcept
{
    for (auto const& ext : extensions) {
        if (std::string_view{ext.extensionName.data()} == name)
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------
// Feature flag extraction
//
// We use Vulkan 1.2+ feature chains (VkPhysicalDeviceVulkan11Features etc.)
// when querying device capabilities so the mapping stays independent of which
// extensions are individually promoted.
// -------------------------------------------------------------------------------------------------

/// Derives a wren::rhi::Feature bitmask from the set of runtime Vulkan
/// properties, core features, and available device extensions.
///
/// @param props2     VkPhysicalDeviceProperties2 chain (device type, limits, …)
/// @param feats      VkPhysicalDeviceFeatures (base Vulkan 1.0 feature set)
/// @param feats12    VkPhysicalDeviceVulkan12Features (promoted 1.2 features)
/// @param feats13    VkPhysicalDeviceVulkan13Features (promoted 1.3 features)
/// @param extensions All device-level extension property names.
[[nodiscard]] Feature extract_features(
    vk::PhysicalDeviceFeatures               const& feats,
    vk::PhysicalDeviceVulkan12Features       const& feats12,
    vk::PhysicalDeviceVulkan13Features       const& feats13,
    std::span<vk::ExtensionProperties const>        extensions) noexcept;

/// Fills a DeviceLimits struct from a VkPhysicalDeviceLimits (part of
/// VkPhysicalDeviceProperties).
[[nodiscard]] DeviceLimits extract_limits(
    vk::PhysicalDeviceLimits const& vk_limits) noexcept;

/// Determines the approximate size of the largest DEVICE_LOCAL heap
/// (proxy for dedicated GPU VRAM).
[[nodiscard]] uint64_t device_local_heap_bytes(
    vk::PhysicalDeviceMemoryProperties const& mem_props) noexcept;

/// Converts VkPhysicalDeviceType to AdapterKind.
[[nodiscard]] AdapterKind to_adapter_kind(vk::PhysicalDeviceType type) noexcept;

/// Builds a complete AdapterInfo from a physical device. Queries all
/// relevant properties chain in a single vkGetPhysicalDeviceProperties2 call.
[[nodiscard]] AdapterInfo make_adapter_info(
    uint32_t                     index,
    vk::raii::PhysicalDevice const& phys_device);

} // namespace wren::rhi::vulkan::detail
