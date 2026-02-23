#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <string>

#include <wren/rhi/api/features.hpp>
#include <wren/rhi/api/status.hpp>
#include <wren/rhi/vulkan/adapter.hpp>
#include <wren/rhi/vulkan/export.hpp>

#include <vulkan/vulkan_raii.hpp>

namespace wren::rhi::vulkan {

// -------------------------------------------------------------------------------------------------
// Indices and family info for the logical device queues.
//
// Vulkan exposes work submission through queue families. We pick:
//   - graphics_family : supports VK_QUEUE_GRAPHICS_BIT (+ implicitly transfer)
//   - compute_family  : supports VK_QUEUE_COMPUTE_BIT
//                       Prefer a dedicated async-compute family when available;
//                       fall back to the graphics family otherwise.
//   - transfer_family : supports VK_QUEUE_TRANSFER_BIT
//                       Prefer a dedicated transfer-only family (DMA engine);
//                       fall back to graphics family otherwise.
//
// All three indices are valid after successful device creation.
// -------------------------------------------------------------------------------------------------
struct QueueFamilyIndices {
    uint32_t graphics = UINT32_MAX;
    uint32_t compute  = UINT32_MAX;
    uint32_t transfer = UINT32_MAX;

    [[nodiscard]] bool is_complete() const noexcept {
        return graphics != UINT32_MAX && compute != UINT32_MAX && transfer != UINT32_MAX;
    }
};

// -------------------------------------------------------------------------------------------------
// Fine-grained error returned from VulkanDevice::create().
// -------------------------------------------------------------------------------------------------
struct DeviceCreateError {
    Status      status;
    std::string message;
};

// -------------------------------------------------------------------------------------------------
// VulkanDevice — owns a VkDevice and the selected physical device.
//
// Responsibilities:
//   - Selects the best physical device honouring DeviceDesc::preferredAdapterIndex
//     and DeviceFeatureRequest (required vs. preferred features).
//   - Creates queue families for graphics, compute, and transfer work.
//   - Enables the minimal set of VkDevice extensions needed by the resolved
//     feature set (e.g., VK_KHR_dynamic_rendering, VK_KHR_timeline_semaphore).
//   - Queries and stores the final Capabilities so callers have a single
//     authoritative post-creation snapshot.
//
// Surface / presentation decoupling:
//   VulkanDevice does NOT create a VkSurfaceKHR. Presentation capability is
//   indicated through Feature::Presentation in the resolved capabilities, which
//   reflects whether VK_KHR_swapchain is available on the selected device.
//   The surface and swapchain are created by a higher-level presentation layer
//   that may depend on the platform (Win32, Xlib, Wayland, etc.).
//
// Thread-safety:
//   Construction and destruction must happen on a single thread.
//   Query methods (capabilities(), queue_family_indices()) are const and
//   safe to call from any thread.
// -------------------------------------------------------------------------------------------------
class WREN_RHI_VULKAN_EXPORT VulkanDevice {
public:
    ~VulkanDevice();

    VulkanDevice(VulkanDevice&&) noexcept;
    VulkanDevice& operator=(VulkanDevice&&) noexcept;

    VulkanDevice(VulkanDevice const&)             = delete;
    VulkanDevice& operator=(VulkanDevice const&)  = delete;

    // -----------------------------------------------------------------
    // Factory
    // -----------------------------------------------------------------

    /// Selects a physical device, creates a logical device, and resolves the
    /// feature set described by @p desc.
    ///
    /// Selection priority:
    ///   1. desc.preferredAdapterIndex if non-zero and valid.
    ///   2. First discrete GPU whose required features are fully supported.
    ///   3. First adapter (any kind) whose required features are fully supported.
    ///
    /// Returns DeviceCreateError with Status::MissingRequiredFeature when no
    /// physical device satisfies desc.featureRequest.required.
    ///
    /// @param instance  The vk::raii::Instance used to enumerate physical devices.
    /// @param desc      Creation parameters (feature request, flags, adapter hint).
    [[nodiscard]] static auto create(vk::raii::Instance const& instance, DeviceDesc const& desc)
        -> std::expected<VulkanDevice, DeviceCreateError>;

    // -----------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------

    /// Returns the resolved capabilities of the selected physical device.
    /// Features in featureRequest.preferred that were unavailable are absent
    /// from capabilities().features.
    [[nodiscard]] auto capabilities() const noexcept -> Capabilities const&;

    /// Returns the queue family indices selected during device creation.
    [[nodiscard]] auto queue_family_indices() const noexcept -> QueueFamilyIndices const&;

    // -----------------------------------------------------------------
    // Internal (used by higher-level RHI objects built on top)
    // -----------------------------------------------------------------
    struct Impl;
    [[nodiscard]] Impl&       impl() noexcept       { return *impl_; }
    [[nodiscard]] Impl const& impl() const noexcept { return *impl_; }

private:
    explicit VulkanDevice(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> impl_;
};

} // namespace wren::rhi::vulkan
