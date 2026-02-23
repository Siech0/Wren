// Internal — not part of the public API.
// Maps Vulkan physical device queries to engine-level wren::rhi types.

#include "vk_capabilities.hpp"

#include <algorithm>
#include <cstring>

// The bitwise operators for wren::rhi flag enums are imported in features.hpp.

namespace wren::rhi::vulkan::detail {

// -------------------------------------------------------------------------------------------------
AdapterKind to_adapter_kind(vk::PhysicalDeviceType type) noexcept {
    switch (type) {
        case vk::PhysicalDeviceType::eIntegratedGpu: return AdapterKind::Integrated;
        case vk::PhysicalDeviceType::eDiscreteGpu:   return AdapterKind::Discrete;
        case vk::PhysicalDeviceType::eVirtualGpu:    return AdapterKind::Virtual;
        case vk::PhysicalDeviceType::eCpu:           return AdapterKind::CPU;
        default:                                      return AdapterKind::Other;
    }
}

// -------------------------------------------------------------------------------------------------
uint64_t device_local_heap_bytes(vk::PhysicalDeviceMemoryProperties const& mem_props) noexcept {
    uint64_t best = 0;
    for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i) {
        auto const& heap = mem_props.memoryHeaps[i];
        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
            best = std::max(best, static_cast<uint64_t>(heap.size));
    }
    return best;
}

// -------------------------------------------------------------------------------------------------
DeviceLimits extract_limits(vk::PhysicalDeviceLimits const& lim) noexcept {
    DeviceLimits out{};

    out.maxImageDimension1D = lim.maxImageDimension1D;
    out.maxImageDimension2D = lim.maxImageDimension2D;
    out.maxImageDimension3D = lim.maxImageDimension3D;
    out.maxCubeDimension    = lim.maxImageDimensionCube;
    out.maxMipLevels        = 32u; // log2(maxImageDimension2D) upper bound; Vulkan doesn't expose this directly
    out.maxArrayLayers      = lim.maxImageArrayLayers;

    out.maxPerStageSamplers        = lim.maxPerStageDescriptorSamplers;
    out.maxPerStageSampledImages   = lim.maxPerStageDescriptorSampledImages;
    out.maxPerStageStorageImages   = lim.maxPerStageDescriptorStorageImages;
    out.maxPerStageUniformBuffers  = lim.maxPerStageDescriptorUniformBuffers;
    out.maxPerStageStorageBuffers  = lim.maxPerStageDescriptorStorageBuffers;

    out.maxColorAttachments = lim.maxColorAttachments;

    out.maxVertexInputBindings   = lim.maxVertexInputBindings;
    out.maxVertexInputAttributes = lim.maxVertexInputAttributes;

    // maxMSAASamples — report the highest color+depth sample count both support.
    {
        auto shared = lim.framebufferColorSampleCounts & lim.framebufferDepthSampleCounts;
        if      (shared & vk::SampleCountFlagBits::e64) out.maxMSAASamples = 64;
        else if (shared & vk::SampleCountFlagBits::e32) out.maxMSAASamples = 32;
        else if (shared & vk::SampleCountFlagBits::e16) out.maxMSAASamples = 16;
        else if (shared & vk::SampleCountFlagBits::e8)  out.maxMSAASamples = 8;
        else if (shared & vk::SampleCountFlagBits::e4)  out.maxMSAASamples = 4;
        else if (shared & vk::SampleCountFlagBits::e2)  out.maxMSAASamples = 2;
        else                                             out.maxMSAASamples = 1;
    }

    out.uniformBufferAlignment = static_cast<uint32_t>(lim.minUniformBufferOffsetAlignment);
    out.storageBufferAlignment = static_cast<uint32_t>(lim.minStorageBufferOffsetAlignment);

    out.maxComputeWorkGroupSizeX        = lim.maxComputeWorkGroupSize[0];
    out.maxComputeWorkGroupSizeY        = lim.maxComputeWorkGroupSize[1];
    out.maxComputeWorkGroupSizeZ        = lim.maxComputeWorkGroupSize[2];
    out.maxComputeWorkGroupInvocations  = lim.maxComputeWorkGroupInvocations;

    // Vulkan timestamp precision is reported as timestampPeriod (nanoseconds per tick).
    // Convert to ticks/second: freq = 1e9 / period.
    out.timelineTickFrequency =
        lim.timestampPeriod > 0.0f
            ? static_cast<uint64_t>(1'000'000'000.0 / static_cast<double>(lim.timestampPeriod))
            : 0u;

    return out;
}

// -------------------------------------------------------------------------------------------------
Feature extract_features(
    vk::PhysicalDeviceFeatures               const& feats,
    vk::PhysicalDeviceVulkan12Features       const& feats12,
    vk::PhysicalDeviceVulkan13Features       const& feats13,
    std::span<vk::ExtensionProperties const>        exts) noexcept
{
    Feature caps = Feature::None;

    auto set = [&](Feature f, bool cond) noexcept {
        if (cond) caps = caps | f;
    };

    // --- Pipeline stages --------------------------------------------------------
    set(Feature::Tessellation,   feats.tessellationShader == VK_TRUE);
    set(Feature::GeometryShader, feats.geometryShader     == VK_TRUE);
    set(Feature::MeshShader,     has_extension(exts, "VK_EXT_mesh_shader"));
    set(Feature::RayTracing,     has_extension(exts, "VK_KHR_ray_tracing_pipeline") &&
                                 has_extension(exts, "VK_KHR_acceleration_structure"));

    // --- Synchronization --------------------------------------------------------
    // Timeline semaphores are Vulkan 1.2 core (feats12.timelineSemaphore).
    set(Feature::TimelineSemaphore, feats12.timelineSemaphore == VK_TRUE);

    // --- Resource binding -------------------------------------------------------
    // Bindless: needs both partial binding and runtime-sized arrays.
    set(Feature::DescriptorIndexing_Bindless,
        feats12.descriptorBindingPartiallyBound  == VK_TRUE &&
        feats12.runtimeDescriptorArray           == VK_TRUE);
    set(Feature::DescriptorBuffer,   has_extension(exts, "VK_EXT_descriptor_buffer"));
    set(Feature::BufferDeviceAddress, feats12.bufferDeviceAddress == VK_TRUE);

    // --- Draw / dispatch --------------------------------------------------------
    set(Feature::MultiDrawIndirect,
        feats.multiDrawIndirect == VK_TRUE);

    // --- Shader capabilities ----------------------------------------------------
    // Subgroup (wave ops): always present in Vulkan 1.1+ (we require 1.3).
    caps = caps | Feature::Subgroup_WaveOps;

    set(Feature::ShaderFloat16_Int8,
        feats12.shaderFloat16 == VK_TRUE || feats12.shaderInt8 == VK_TRUE);
    set(Feature::ShaderInt64,         feats.shaderInt64    == VK_TRUE);
    set(Feature::ImageLoadStore_UAV,
        feats.fragmentStoresAndAtomics       == VK_TRUE ||
        feats.vertexPipelineStoresAndAtomics == VK_TRUE);

    // --- Rasterisation & sampling -----------------------------------------------
    set(Feature::VariableRateShading, has_extension(exts, "VK_KHR_fragment_shading_rate"));
    set(Feature::ConservativeRaster,  has_extension(exts, "VK_EXT_conservative_rasterization"));
    set(Feature::FragmentInterlock_ROV, has_extension(exts, "VK_EXT_fragment_shader_interlock"));
    set(Feature::SampleRateShading,   feats.sampleRateShading  == VK_TRUE);
    set(Feature::AnisotropicFiltering, feats.samplerAnisotropy  == VK_TRUE);
    set(Feature::DepthClamp,          feats.depthClamp          == VK_TRUE);
    set(Feature::DualSourceBlending,  feats.dualSrcBlend        == VK_TRUE);

    // Mirror clamp to edge: promoted to Vulkan 1.2 core.
    set(Feature::MirrorClampToEdge,   feats12.samplerMirrorClampToEdge == VK_TRUE);

    set(Feature::NonSolidFill,        feats.fillModeNonSolid == VK_TRUE);
    set(Feature::DepthBoundsTest,     feats.depthBounds      == VK_TRUE);

    // --- Multi-view / memory ----------------------------------------------------
    // Multiview: core in Vulkan 1.1; always available at our 1.3 minimum.
    caps = caps | Feature::Multiview;
    // Persistent mapped buffers: always available via HOST_COHERENT + HOST_VISIBLE heaps.
    caps = caps | Feature::PersistentMappedBuffers;

    set(Feature::SparseResources, feats.sparseBinding == VK_TRUE);

    // --- Dynamic rendering ------------------------------------------------------
    // Core in Vulkan 1.3 (feats13.dynamicRendering).
    set(Feature::DynamicRendering, feats13.dynamicRendering == VK_TRUE);

    // --- Presentation -----------------------------------------------------------
    // VK_KHR_swapchain must be available (checked at device extension level).
    set(Feature::Presentation, has_extension(exts, "VK_KHR_swapchain"));

    // --- Texture compression ----------------------------------------------------
    set(Feature::TexCompression_BC,       feats.textureCompressionBC       == VK_TRUE);
    set(Feature::TexCompression_ETC2,     feats.textureCompressionETC2     == VK_TRUE);
    set(Feature::TexCompression_ASTC_LDR, feats.textureCompressionASTC_LDR == VK_TRUE);

    // --- Debug ------------------------------------------------------------------
    set(Feature::DebugMarkers_Labels, has_extension(exts, "VK_EXT_debug_utils"));

    return caps;
}

// -------------------------------------------------------------------------------------------------
AdapterInfo make_adapter_info(uint32_t index, vk::raii::PhysicalDevice const& phys) {
    // Properties chain: base + driver properties.
    auto props_chain = phys.getProperties2<
        vk::PhysicalDeviceProperties2,
        vk::PhysicalDeviceDriverProperties>();
    auto const& props   = props_chain.get<vk::PhysicalDeviceProperties2>().properties;

    // Features chain: base + Vulkan 1.2 + 1.3 promoted features.
    auto feats_chain = phys.getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan12Features,
        vk::PhysicalDeviceVulkan13Features>();
    auto const& feats   = feats_chain.get<vk::PhysicalDeviceFeatures2>().features;
    auto const& feats12 = feats_chain.get<vk::PhysicalDeviceVulkan12Features>();
    auto const& feats13 = feats_chain.get<vk::PhysicalDeviceVulkan13Features>();

    // Extension list.
    auto const extensions = phys.enumerateDeviceExtensionProperties();

    // Memory info for VRAM estimate.
    auto const mem_props = phys.getMemoryProperties();

    // Build capabilities.
    Capabilities caps{};
    caps.backend         = wren::rhi::Backend::Vulkan;
    caps.apiVersionMajor = VK_API_VERSION_MAJOR(props.apiVersion);
    caps.apiVersionMinor = VK_API_VERSION_MINOR(props.apiVersion);
    caps.features        = extract_features(feats, feats12, feats13, extensions);
    caps.limits          = extract_limits(props.limits);

    AdapterInfo info{};
    info.index              = index;
    info.name               = std::string{props.deviceName.data()};
    info.kind               = to_adapter_kind(props.deviceType);
    info.video_memory_bytes = device_local_heap_bytes(mem_props);
    info.driver_version     = props.driverVersion;
    info.api_version_major  = caps.apiVersionMajor;
    info.api_version_minor  = caps.apiVersionMinor;
    info.capabilities       = std::move(caps);

    return info;
}

} // namespace wren::rhi::vulkan::detail
