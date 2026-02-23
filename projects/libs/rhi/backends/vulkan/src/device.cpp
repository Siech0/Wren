#include <wren/rhi/vulkan/device.hpp>

#include <algorithm>
#include <cstring>
#include <format>
#include <vector>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_raii.hpp>

#include "vk_capabilities.hpp"

// The bitwise operators for wren::rhi flag enums are available via features.hpp.

namespace wren::rhi::vulkan {

// -------------------------------------------------------------------------------------------------
// Impl
// -------------------------------------------------------------------------------------------------
struct VulkanDevice::Impl {
    vk::raii::PhysicalDevice phys_device;
    vk::raii::Device         device;
    QueueFamilyIndices       queue_indices;
    Capabilities             capabilities;

    Impl(vk::raii::PhysicalDevice phys, vk::raii::Device dev,
         QueueFamilyIndices qi, Capabilities caps)
        : phys_device{std::move(phys)}
        , device{std::move(dev)}
        , queue_indices{qi}
        , capabilities{std::move(caps)}
    {}
};

// -------------------------------------------------------------------------------------------------
// Internal helpers
// -------------------------------------------------------------------------------------------------
namespace {

// -----------------------------------------------------------------
// Queue family selection
// -----------------------------------------------------------------
[[nodiscard]] QueueFamilyIndices select_queue_families(
    vk::raii::PhysicalDevice const& phys) noexcept
{
    auto const families = phys.getQueueFamilyProperties();
    QueueFamilyIndices result;

    // 1. Graphics queue (required).
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); ++i) {
        if (families[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            result.graphics = i;
            break;
        }
    }

    // 2. Prefer a dedicated async-compute queue (no graphics bit).
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); ++i) {
        if ((families[i].queueFlags & vk::QueueFlagBits::eCompute) &&
            !(families[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
            result.compute = i;
            break;
        }
    }
    if (result.compute == UINT32_MAX)
        result.compute = result.graphics; // fallback: share the graphics queue

    // 3. Prefer a dedicated DMA transfer queue (no graphics or compute bits).
    for (uint32_t i = 0; i < static_cast<uint32_t>(families.size()); ++i) {
        if ((families[i].queueFlags & vk::QueueFlagBits::eTransfer) &&
            !(families[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            !(families[i].queueFlags & vk::QueueFlagBits::eCompute)) {
            result.transfer = i;
            break;
        }
    }
    if (result.transfer == UINT32_MAX)
        result.transfer = result.graphics; // fallback: share the graphics queue

    return result;
}

// -----------------------------------------------------------------
// Feature-to-extension mapping
// Builds the minimal set of device extensions needed to support the
// requested feature mask, intersected with what is actually available.
// -----------------------------------------------------------------
[[nodiscard]] std::vector<const char*> resolve_extensions(
    Feature                                  requested,
    std::vector<vk::ExtensionProperties> const& available) noexcept
{
    using detail::has_extension;
    std::vector<const char*> out;
    std::span<vk::ExtensionProperties const> avail_span{available};

    auto try_add = [&](const char* name) {
        if (has_extension(avail_span, name))
            out.push_back(name);
    };

    // Swapchain: requested whenever Presentation is in the feature mask.
    if (has_any(requested, Feature::Presentation))
        try_add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Mesh shaders.
    if (has_any(requested, Feature::MeshShader))
        try_add("VK_EXT_mesh_shader");

    // Ray tracing: pipeline + acceleration structure + deferred host ops.
    if (has_any(requested, Feature::RayTracing)) {
        try_add("VK_KHR_ray_tracing_pipeline");
        try_add("VK_KHR_acceleration_structure");
        try_add("VK_KHR_deferred_host_operations"); // required by accel structure
    }

    // Descriptor buffer.
    if (has_any(requested, Feature::DescriptorBuffer))
        try_add("VK_EXT_descriptor_buffer");

    // Variable rate shading.
    if (has_any(requested, Feature::VariableRateShading))
        try_add("VK_KHR_fragment_shading_rate");

    // Conservative rasterisation.
    if (has_any(requested, Feature::ConservativeRaster))
        try_add("VK_EXT_conservative_rasterization");

    // Fragment interlock / ROV.
    if (has_any(requested, Feature::FragmentInterlock_ROV))
        try_add("VK_EXT_fragment_shader_interlock");

    // Debug labels.
    if (has_any(requested, Feature::DebugMarkers_Labels))
        try_add("VK_EXT_debug_utils");

    return out;
}

// -----------------------------------------------------------------
// Physical device scoring — higher is better.
// Used when preferredAdapterIndex is 0 (no explicit preference).
// -----------------------------------------------------------------
[[nodiscard]] int32_t score_physical_device(
    vk::raii::PhysicalDevice const& phys,
    Feature                         required_features)
{
    auto const info = detail::make_adapter_info(0, phys);

    // Can it satisfy our hard requirements?
    if (!has_all(info.capabilities.features, required_features))
        return -1;

    // Prefer discrete > integrated > others.
    int32_t score = 0;
    switch (info.kind) {
        case AdapterKind::Discrete:   score += 10000; break;
        case AdapterKind::Integrated: score += 1000;  break;
        default:                                      break;
    }

    // Bonus for more GPU memory.
    score += static_cast<int32_t>(info.video_memory_bytes / (256ull * 1024 * 1024));

    return score;
}

// -----------------------------------------------------------------
// Feature structures needed for device-level feature negotiation.
// We enable only what the resolved feature mask requests.
// -----------------------------------------------------------------

// Flat aggregate holding the feature structs we pass to VkDeviceCreateInfo.
// The pNext chain is fully wired in build_feature_chain() before the call.
struct DeviceFeatureChain {
    vk::PhysicalDeviceFeatures2          features2{};
    vk::PhysicalDeviceVulkan11Features   vk11{};
    vk::PhysicalDeviceVulkan12Features   vk12{};
    vk::PhysicalDeviceVulkan13Features   vk13{};

    // Extension features — enabled only when the matching extension is active.
    vk::PhysicalDeviceMeshShaderFeaturesEXT              mesh_shader{};
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR      ray_tracing{};
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR   accel_struct{};
    vk::PhysicalDeviceDescriptorBufferFeaturesEXT        descriptor_buffer{};
    vk::PhysicalDeviceFragmentShadingRateFeaturesKHR     fsr{};
    vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT interlock{};
};

// Build the pNext feature query chain, query physical device, then mask
// down to only what's in `resolved`.  Returns a chain ready for VkDeviceCreateInfo.
[[nodiscard]] DeviceFeatureChain build_feature_chain(
    vk::raii::PhysicalDevice const&  phys,
    Feature                          resolved,
    std::vector<const char*> const&  extensions)
{
    auto ext_active = [&](const char* name) noexcept {
        return std::ranges::any_of(extensions, [name](const char* e) {
            return std::strcmp(e, name) == 0;
        });
    };

    DeviceFeatureChain c;

    // Wire the base 1.1/1.2/1.3 chain (always present on Vulkan 1.3).
    c.features2.pNext = &c.vk11;
    c.vk11.pNext      = &c.vk12;
    c.vk12.pNext      = &c.vk13;
    void** tail = reinterpret_cast<void**>(&c.vk13.pNext);

    // Append extension structs.
    auto append = [&](auto* s) {
        *tail = s;
        tail  = reinterpret_cast<void**>(&s->pNext);
    };

    if (ext_active("VK_EXT_mesh_shader"))              append(&c.mesh_shader);
    if (ext_active("VK_KHR_ray_tracing_pipeline")) {   append(&c.ray_tracing);
                                                        append(&c.accel_struct); }
    if (ext_active("VK_EXT_descriptor_buffer"))        append(&c.descriptor_buffer);
    if (ext_active("VK_KHR_fragment_shading_rate"))    append(&c.fsr);
    if (ext_active("VK_EXT_fragment_shader_interlock")) append(&c.interlock);
    *tail = nullptr;

    // Query all supported values at once.
    phys.getDispatcher()->vkGetPhysicalDeviceFeatures2(
        static_cast<VkPhysicalDevice>(*phys),
        reinterpret_cast<VkPhysicalDeviceFeatures2*>(&c.features2));

    // Mask out features not in `resolved` so we don't enable what wasn't requested.
    auto& f   = c.features2.features;
    auto& f12 = c.vk12;
    auto& f13 = c.vk13;

    if (!has_any(resolved, Feature::Tessellation))        f.tessellationShader = VK_FALSE;
    if (!has_any(resolved, Feature::GeometryShader))      f.geometryShader     = VK_FALSE;
    if (!has_any(resolved, Feature::MultiDrawIndirect))   f.multiDrawIndirect  = VK_FALSE;
    if (!has_any(resolved, Feature::SampleRateShading))   f.sampleRateShading  = VK_FALSE;
    if (!has_any(resolved, Feature::AnisotropicFiltering)) f.samplerAnisotropy = VK_FALSE;
    if (!has_any(resolved, Feature::DepthClamp))          f.depthClamp         = VK_FALSE;
    if (!has_any(resolved, Feature::DualSourceBlending))  f.dualSrcBlend       = VK_FALSE;
    if (!has_any(resolved, Feature::NonSolidFill))        f.fillModeNonSolid   = VK_FALSE;
    if (!has_any(resolved, Feature::DepthBoundsTest))     f.depthBounds        = VK_FALSE;
    if (!has_any(resolved, Feature::SparseResources))     f.sparseBinding      = VK_FALSE;
    if (!has_any(resolved, Feature::ShaderInt64))         f.shaderInt64        = VK_FALSE;

    if (!has_any(resolved, Feature::TimelineSemaphore))
        f12.timelineSemaphore = VK_FALSE;
    if (!has_any(resolved, Feature::BufferDeviceAddress))
        f12.bufferDeviceAddress = VK_FALSE;
    if (!has_any(resolved, Feature::DescriptorIndexing_Bindless)) {
        f12.descriptorBindingPartiallyBound = VK_FALSE;
        f12.runtimeDescriptorArray          = VK_FALSE;
    }
    if (!has_any(resolved, Feature::ShaderFloat16_Int8)) {
        f12.shaderFloat16 = VK_FALSE;
        f12.shaderInt8    = VK_FALSE;
    }
    if (!has_any(resolved, Feature::MirrorClampToEdge))
        f12.samplerMirrorClampToEdge = VK_FALSE;

    if (!has_any(resolved, Feature::DynamicRendering))
        f13.dynamicRendering = VK_FALSE;

    return c;
}

} // anonymous namespace

// -------------------------------------------------------------------------------------------------
// VulkanDevice lifecycle
// -------------------------------------------------------------------------------------------------
VulkanDevice::VulkanDevice(std::unique_ptr<Impl> impl) noexcept
    : impl_{std::move(impl)}
{}

VulkanDevice::~VulkanDevice() = default;

VulkanDevice::VulkanDevice(VulkanDevice&&) noexcept            = default;
VulkanDevice& VulkanDevice::operator=(VulkanDevice&&) noexcept = default;

// -------------------------------------------------------------------------------------------------
// Queries
// -------------------------------------------------------------------------------------------------
auto VulkanDevice::capabilities() const noexcept -> Capabilities const& {
    return impl_->capabilities;
}

auto VulkanDevice::queue_family_indices() const noexcept -> QueueFamilyIndices const& {
    return impl_->queue_indices;
}

// -------------------------------------------------------------------------------------------------
// Factory
// -------------------------------------------------------------------------------------------------
auto VulkanDevice::create(vk::raii::Instance const& instance, DeviceDesc const& desc)
    -> std::expected<VulkanDevice, DeviceCreateError>
{
    try {
        // ------------------------------------------------------------------
        // 1. Enumerate physical devices.
        // ------------------------------------------------------------------
        auto const phys_devices = instance.enumeratePhysicalDevices();
        if (phys_devices.empty()) {
            return std::unexpected{DeviceCreateError{
                Status::InternalError, "No Vulkan-capable physical devices found."}};
        }

        Feature const required  = desc.featureRequest.required;
        Feature const preferred = desc.featureRequest.preferred;

        // ------------------------------------------------------------------
        // 2. Select physical device.
        //
        //   Priority:
        //     a) desc.preferredAdapterIndex if it satisfies required features.
        //     b) Best-scored device that satisfies required features.
        // ------------------------------------------------------------------
        int32_t chosen_idx = -1;

        // (a) User-specified index.
        if (desc.preferredAdapterIndex < static_cast<uint32_t>(phys_devices.size())) {
            auto const info = detail::make_adapter_info(
                desc.preferredAdapterIndex, phys_devices[desc.preferredAdapterIndex]);
            if (has_all(info.capabilities.features, required))
                chosen_idx = static_cast<int32_t>(desc.preferredAdapterIndex);
        }

        // (b) Score-based selection.
        if (chosen_idx < 0) {
            int32_t best_score = -1;
            for (uint32_t i = 0; i < static_cast<uint32_t>(phys_devices.size()); ++i) {
                int32_t s = score_physical_device(phys_devices[i], required);
                if (s > best_score) {
                    best_score = s;
                    chosen_idx = static_cast<int32_t>(i);
                }
            }
        }

        if (chosen_idx < 0) {
            return std::unexpected{DeviceCreateError{
                Status::MissingRequiredFeature,
                "No physical device satisfies the required feature set."}};
        }

        auto const& phys = phys_devices[static_cast<uint32_t>(chosen_idx)];
        auto const adapter_info = detail::make_adapter_info(
            static_cast<uint32_t>(chosen_idx), phys);

        // ------------------------------------------------------------------
        // 3. Resolve feature set: required + available subset of preferred.
        // ------------------------------------------------------------------
        Feature const available = adapter_info.capabilities.features;
        Feature const resolved  = required | (preferred & available);

        // Log downgraded preferred features (best-effort).
        // Compute which preferred features were unavailable.
        Feature const missing_preferred = static_cast<Feature>(
            static_cast<uint64_t>(preferred) & ~static_cast<uint64_t>(available));
        if (auto bits = static_cast<uint64_t>(missing_preferred); bits != 0) {
            SPDLOG_WARN("[wren/rhi/vulkan] Selected adapter '{}': "
                        "some preferred features unavailable (mask={:#x}). "
                        "Continuing with reduced feature set.",
                        adapter_info.name, bits);
        }

        // ------------------------------------------------------------------
        // 4. Queue families.
        // ------------------------------------------------------------------
        auto const qi = select_queue_families(phys);
        if (!qi.is_complete()) {
            return std::unexpected{DeviceCreateError{
                Status::UnsupportedQueueType,
                std::format("Adapter '{}' does not expose a graphics queue.",
                            adapter_info.name)}};
        }

        // ------------------------------------------------------------------
        // 5. Device extensions — resolved against the feature set.
        // ------------------------------------------------------------------
        auto const avail_exts = phys.enumerateDeviceExtensionProperties();
        auto const extensions = resolve_extensions(resolved, avail_exts);

        // ------------------------------------------------------------------
        // 6. Build the feature chain (enables only resolved features).
        // ------------------------------------------------------------------
        auto feat_chain = build_feature_chain(phys, resolved, extensions);

        // ------------------------------------------------------------------
        // 7. Queue create infos.
        //    Deduplicate family indices — we still only create one queue
        //    per unique family.
        // ------------------------------------------------------------------
        static constexpr float k_queue_priority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queue_cis;
        std::vector<uint32_t> unique_families;

        for (uint32_t f : {qi.graphics, qi.compute, qi.transfer}) {
            if (std::ranges::find(unique_families, f) == unique_families.end()) {
                unique_families.push_back(f);
                queue_cis.push_back(
                    vk::DeviceQueueCreateInfo{}
                        .setQueueFamilyIndex(f)
                        .setQueueCount(1)
                        .setPQueuePriorities(&k_queue_priority));
            }
        }

        // ------------------------------------------------------------------
        // 8. Create the logical device.
        // ------------------------------------------------------------------
        bool const want_debug = (static_cast<uint32_t>(desc.flags) & static_cast<uint32_t>(DeviceFlag::Debug)) != 0;
        std::vector<const char*> layers;
        if (want_debug)
            layers.push_back("VK_LAYER_KHRONOS_validation");

        auto device_ci = vk::DeviceCreateInfo{}
            .setPNext(&feat_chain.features2)
            .setQueueCreateInfos(queue_cis)
            .setPEnabledLayerNames(layers)
            .setPEnabledExtensionNames(extensions);
            // pEnabledFeatures is null — everything routed through pNext chain.

        auto vk_device = phys.createDevice(device_ci);

        // ------------------------------------------------------------------
        // 9. Build final Capabilities snapshot for this logical device.
        //    Re-derive from the physical device using the resolved feature mask.
        // ------------------------------------------------------------------
        Capabilities final_caps = adapter_info.capabilities;
        final_caps.features = resolved; // only what we actually enabled

        // ------------------------------------------------------------------
        // 10. Construct and return.
        // ------------------------------------------------------------------
        auto impl = std::make_unique<Impl>(
            phys,                // vk::raii::PhysicalDevice is copyable (ref-counted handle)
            std::move(vk_device),
            qi,
            std::move(final_caps));

        return VulkanDevice{std::move(impl)};

    } catch (vk::SystemError const& err) {
        return std::unexpected{DeviceCreateError{
            Status::InternalError,
            std::format("Vulkan device creation failed: {}", err.what())}};
    } catch (std::exception const& err) {
        return std::unexpected{DeviceCreateError{
            Status::InternalError,
            std::format("Device creation error: {}", err.what())}};
    }
}

} // namespace wren::rhi::vulkan
