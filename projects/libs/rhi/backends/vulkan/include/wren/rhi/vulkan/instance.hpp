#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <wren/rhi/vulkan/adapter.hpp>
#include <wren/rhi/vulkan/export.hpp>

namespace wren::rhi::vulkan {

// -------------------------------------------------------------------------------------------------
// Options for Vulkan instance creation.
//
// No surface or presentation extensions are requested here — those are the
// responsibility of the platform/presentation layer, keeping this factory
// usable in headless / off-screen workloads.
// -------------------------------------------------------------------------------------------------
struct InstanceConfig {
    std::string_view application_name    = "wren";
    uint32_t         application_version = 0;      ///< Pack with VK_MAKE_VERSION(maj,min,patch).
    bool             enable_debug        = false;  ///< Request validation layers + debug utils extension.
};

// -------------------------------------------------------------------------------------------------
// Factory for vk::raii::Instance.
//
// @param ctx  Context that owns the Vulkan dispatch table.  Must outlive the
//             returned Instance (vk::raii::Instance borrows the dispatcher).
// @param cfg  See InstanceConfig.
//
// Validation layers are activated when cfg.enable_debug is true and
// VK_LAYER_KHRONOS_validation is available; absence is logged but non-fatal.
//
// @returns The instance on success, or a human-readable error string.
// -------------------------------------------------------------------------------------------------
[[nodiscard]] WREN_RHI_VULKAN_EXPORT
auto create_instance(vk::raii::Context& ctx, InstanceConfig const& cfg)
    -> std::expected<vk::raii::Instance, std::string>;

// -------------------------------------------------------------------------------------------------
// Attaches VK_EXT_debug_utils validation output.
//
// Call only when cfg.enable_debug was true.  The returned object must be
// kept alive alongside the instance to receive validation messages.
// -------------------------------------------------------------------------------------------------
[[nodiscard]] WREN_RHI_VULKAN_EXPORT
auto create_debug_messenger(vk::raii::Instance& instance)
    -> vk::raii::DebugUtilsMessengerEXT;

// -------------------------------------------------------------------------------------------------
// Capability snapshots for every physical device.
//
// Intended to be called before VulkanDevice::create() so the application
// can inspect and choose a GPU without committing to a logical device.
// -------------------------------------------------------------------------------------------------
[[nodiscard]] WREN_RHI_VULKAN_EXPORT
auto enumerate_adapters(vk::raii::Instance const& instance)
    -> std::vector<AdapterInfo>;

} // namespace wren::rhi::vulkan
