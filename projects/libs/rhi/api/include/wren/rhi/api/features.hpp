#ifndef WREN_RHI_API_FEATURES_HPP
#define WREN_RHI_API_FEATURES_HPP

#include <cstdint>
#include <type_traits>

#include <wren/foundation/utility/enum_utils.hpp>
#include <wren/rhi/api/enums.hpp>

namespace wren::rhi {

// Import bitwise-operator templates from wren::foundation so they are
// available when operating on Feature and DeviceFlag inside this namespace.
using wren::foundation::operator|;
using wren::foundation::operator|=;
using wren::foundation::operator&;
using wren::foundation::operator&=;
using wren::foundation::underlying;

/// Bitset of optional hardware capabilities.
///
/// Query and enable features per-backend at device creation; cache the
/// resulting mask in @ref Capabilities::features.
///
/// @note Only features that influence engine-level code paths or shader variants
///       are included here.
/// @note If a feature is "emulable but slow", the enumerator comment says so.
/// @note Some OpenGL extensions listed are vendor/ARB; always check at runtime.
/// @note Metal often implements similar behaviour under different names.
enum class Feature : std::uint64_t {
    None = 0,

    /// @name Pipeline stages / programmable stages
    /// @{

    /// Hardware tessellation stage.
    ///
    /// - **Vulkan** – core feature bit `tessellationShader` (`VkPhysicalDeviceFeatures`)
    ///   https://docs.vulkan.org/spec/latest/chapters/tessellation.html
    /// - **OpenGL** – core since 4.0 / `ARB_tessellation_shader`
    ///   https://www.opengl.org/registry/doc/glspec40.core.20100311.pdf
    /// - **D3D12** – Hull/Domain shaders (FL11+)
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    /// - **Metal** – Tessellation pipeline (Metal 2+)
    ///   https://metalbyexample.com/tessellation/
    Tessellation = 1ull << 0,

    /// Geometry shader stage.
    ///
    /// - **Vulkan** – `geometryShader` feature bit
    ///   https://docs.vulkan.org/spec/latest/chapters/geometry.html
    /// - **OpenGL** – core since 3.2
    /// - **D3D12** – Geometry shader stage available
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    /// - **Metal** – No geometry shaders; emulate with compute/mesh paths where needed.
    GeometryShader = 1ull << 1,

    /// Mesh/amplification (task) shader stage.
    ///
    /// - **Vulkan** – `VK_EXT_mesh_shader`
    ///   https://www.khronos.org/blog/mesh-shading-for-vulkan
    /// - **OpenGL** – `NV/EXT_mesh_shader` (vendor/EXT)
    /// - **D3D12** – Mesh/Amplification shaders (SM 6.5+)
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    /// - **Metal** – Mesh shaders in Metal 3+ (Apple GPU families).
    /// @note Shader model limits vary by vendor/OS.
    MeshShader = 1ull << 2,

    /// Hardware ray-tracing pipeline.
    ///
    /// - **Vulkan** – `VK_KHR_ray_tracing_pipeline` (+ acceleration structures)
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_ray_tracing_pipeline.html
    /// - **OpenGL** – `NV_ray_tracing` (vendor)
    /// - **D3D12** – DXR 1.0/1.1
    ///   https://learn.microsoft.com/windows/win32/direct3d12/directx-raytracing
    /// - **Metal** – Ray Tracing (Metal 3+)
    ///   https://developer.apple.com/documentation/metal/ray_tracing
    RayTracing = 1ull << 3,

    /// @}
    /// @name Synchronisation / submission
    /// @{

    /// 64-bit timeline semaphore / fence.
    ///
    /// - **Vulkan** – `VK_KHR_timeline_semaphore` (core in 1.2; feature enable still required)
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_timeline_semaphore.html
    /// - **D3D12** – `ID3D12Fence` is a 64-bit timeline fence
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/nn-d3d12-id3d12fence
    /// - **Metal** – `MTLSharedEvent` (timeline-like)
    ///   https://developer.apple.com/documentation/metal/mtlsharedevent
    /// - **OpenGL** – No native timeline; emulate with sync objects/CPU waits.
    TimelineSemaphore = 1ull << 4,

    /// @}
    /// @name Resource binding / descriptors
    /// @{

    /// Non-uniform descriptor indexing / large bindless arrays.
    ///
    /// - **Vulkan** – `VK_EXT_descriptor_indexing`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_descriptor_indexing.html
    /// - **OpenGL** – `ARB_bindless_texture` (handle residency)
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_bindless_texture.txt
    /// - **D3D12** – Descriptor heaps + Resource Binding Tiers 1–3
    ///   https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
    /// - **Metal** – Argument Buffers / Argument Tables
    ///   https://developer.apple.com/documentation/metal/improving-cpu-performance-by-using-argument-buffers
    DescriptorIndexing_Bindless = 1ull << 5,

    /// Descriptors stored directly in GPU-visible memory (no descriptor pool).
    ///
    /// - **Vulkan** – `VK_EXT_descriptor_buffer`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_descriptor_buffer.html
    /// - **D3D12 / GL / Metal** – No direct equivalent; comparable patterns via heaps/argument buffers.
    DescriptorBuffer = 1ull << 6,

    /// Raw GPU virtual addresses for buffer resources.
    ///
    /// - **Vulkan** – `VK_KHR_buffer_device_address`
    /// - **D3D12** – GPU virtual addresses are standard on all buffers.
    /// - **Metal** – Pointers via argument buffers / indirect addressing (no explicit toggle).
    /// - **OpenGL** – No direct equivalent; emulate via bindless handles/SSBO indices.
    BufferDeviceAddress = 1ull << 7,

    /// @}
    /// @name Draw/dispatch & indirect
    /// @{

    /// Multi-draw indirect + indirect draw count from a GPU buffer.
    ///
    /// - **Vulkan** – `vkCmdDraw*Indirect` + `VK_KHR_draw_indirect_count`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_draw_indirect_count.html
    /// - **OpenGL** – `ARB_multi_draw_indirect` / `ARB_indirect_parameters`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_multi_draw_indirect.txt
    /// - **D3D12** – `ExecuteIndirect` (different model)
    ///   https://learn.microsoft.com/windows/win32/direct3d12/executeindirect
    /// - **Metal** – Indirect Command Buffers
    ///   https://developer.apple.com/documentation/metal/accelerating_draw_calls_with_indirect_command_buffers
    MultiDrawIndirect = 1ull << 8,

    /// @}
    /// @name Shading features / data types
    /// @{

    /// Subgroup / wave-level intrinsics.
    ///
    /// - **Vulkan** – Subgroup operations (core 1.1) + `VK_EXT_subgroup_size_control`
    /// - **OpenGL** – `KHR_shader_subgroup` (where available)
    /// - **D3D12** – HLSL Wave intrinsics (Shader Model 6+)
    ///   https://learn.microsoft.com/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
    /// - **Metal** – SIMD-group functions in MSL
    Subgroup_WaveOps = 1ull << 9,

    /// 16-bit float and 8-bit integer types in shaders.
    ///
    /// - **Vulkan** – `VK_KHR_shader_float16_int8`
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_shader_float16_int8.html
    ///   plus 8/16-bit storage: `VK_KHR_8bit_storage` / `VK_KHR_16bit_storage`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_8bit_storage.html
    /// - **OpenGL** – Various vendor/EXT 16-bit type extensions; not universal.
    /// - **D3D12** – SM 6.x min-precision types; check target hardware.
    /// - **Metal** – `half` types widely supported.
    ShaderFloat16_Int8 = 1ull << 10,

    /// 64-bit integer types in shaders.
    ///
    /// - **Vulkan** – `shaderInt64` feature (core)
    /// - **OpenGL** – `ARB_gpu_shader_int64`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_gpu_shader_int64.txt
    /// - **D3D12** – 64-bit integer support in HLSL (SM 6+); atomics vary.
    /// - **Metal** – Limited; no 64-bit image formats; use two 32-bit halves if needed.
    ShaderInt64 = 1ull << 11,

    /// Storage images / Unordered Access Views (UAVs).
    ///
    /// - **Vulkan** – Storage images (`VkFormatFeature*::STORAGE_IMAGE`)
    /// - **OpenGL** – `ARB_shader_image_load_store`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_shader_image_load_store.txt
    /// - **D3D12** – UAVs
    /// - **Metal** – readWrite textures/buffers in compute/fragment shaders
    ImageLoadStore_UAV = 1ull << 12,

    /// @}
    /// @name Rasterisation & sampling
    /// @{

    /// Per-primitive or per-region shading rate control.
    ///
    /// - **Vulkan** – `VK_KHR_fragment_shading_rate`
    ///   https://docs.vulkan.org/samples/latest/samples/extensions/fragment_shading_rate_dynamic/README.html
    /// - **OpenGL** – `NV_shading_rate_image` (vendor)
    /// - **D3D12** – VRS (Options6 / Tier 1–3)
    ///   https://learn.microsoft.com/windows/win32/direct3d12/vrs
    /// - **Metal** – Rasterisation Rate Maps (tile shading)
    ///   https://developer.apple.com/documentation/metal/rasterization_rate_maps
    VariableRateShading = 1ull << 13,

    /// Conservative rasterisation (inner/outer coverage).
    ///
    /// - **Vulkan** – `VK_EXT_conservative_rasterization`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_conservative_rasterization.html
    /// - **OpenGL** – `NV_conservative_raster`
    /// - **D3D12** – `ConservativeRasterizationTier` (1–3)
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_conservative_rasterization_tier
    /// - **Metal** – No direct equivalent; some hardware-specific approximations.
    ConservativeRaster = 1ull << 14,

    /// Fragment shader interlock / Rasteriser Ordered Views.
    ///
    /// - **Vulkan** – `VK_EXT_fragment_shader_interlock`
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_fragment_shader_interlock.html
    /// - **OpenGL** – `ARB_fragment_shader_interlock`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_shader_interlock.txt
    /// - **D3D12** – Rasteriser Ordered Views (ROVs)
    ///   https://learn.microsoft.com/windows/win32/direct3d12/rasterizer-order-views
    /// - **Metal** – Raster Order Groups (`areRasterOrderGroupsSupported`)
    ///   https://developer.apple.com/documentation/metal/mtldevice/arerasterordergroupssupported
    FragmentInterlock_ROV = 1ull << 15,

    /// Per-sample frequency shading.
    ///
    /// - **Vulkan** – `sampleRateShading` feature bit
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    /// - **OpenGL** – `ARB_sample_shading`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_sample_shading.txt
    /// - **D3D12** – Per-sample PS frequency path (`SV_SampleIndex`) + VRS alternatives
    /// - **Metal** – Behaviour varies; MoltenVK caveats for full supersampling.
    SampleRateShading = 1ull << 16,

    /// Anisotropic texture filtering.
    ///
    /// - **Vulkan** – `samplerAnisotropy` feature
    /// - **OpenGL** – `EXT_texture_filter_anisotropic`
    /// - **D3D12** – `SamplerDesc.MaxAnisotropy`
    /// - **Metal** – `MTLSamplerDescriptor.maxAnisotropy`
    AnisotropicFiltering = 1ull << 17,

    /// Depth clamp (prevent near/far clip; clamp instead).
    ///
    /// - **Vulkan** – `depthClamp` feature / `depthClampEnable`
    /// - **OpenGL** – `GL_DEPTH_CLAMP`
    /// - **D3D12** – `RasterizerState.DepthClipEnable = false` (note: clip disable, not clamp)
    /// - **Metal** – `MTLDepthClipMode::Clamp` (platform support varies)
    DepthClamp = 1ull << 18,

    /// Dual-source colour blending (two outputs from a single fragment slot).
    ///
    /// - **Vulkan** – `dualSrcBlend` feature
    /// - **OpenGL** – `ARB_blend_func_extended`
    /// - **D3D12** – Dual-source colour blend supported.
    /// - **Metal** – Not supported; emulate with MRT in many cases.
    DualSourceBlending = 1ull << 19,

    /// Mirror-clamp-to-edge sampler address mode.
    ///
    /// - **Vulkan** – `VK_KHR_sampler_mirror_clamp_to_edge`
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_sampler_mirror_clamp_to_edge.html
    /// - **OpenGL** – `ARB_texture_mirror_clamp_to_edge`
    ///   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_mirror_clamp_to_edge.txt
    /// - **D3D12** – Mirror + Clamp address mode variants
    ///   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
    /// - **Metal** – `MTLSamplerAddressMode::mirrorClampToEdge`
    MirrorClampToEdge = 1ull << 20,

    /// Wireframe / point fill-mode (non-solid polygon fill).
    ///
    /// - **Vulkan** – `fillModeNonSolid` feature
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    /// - **OpenGL** – Polygon mode `LINE` / `POINT`
    /// - **D3D12** – `FillMode_WIREFRAME`
    /// - **Metal** – Wireframe is limited on macOS; not available on all Apple GPUs.
    NonSolidFill = 1ull << 21,

    /// Depth bounds test (discard fragments outside a [min, max] depth range).
    ///
    /// - **Vulkan** – `depthBounds` feature
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    /// - **OpenGL** – `EXT_depth_bounds_test` (vendor/EXT)
    /// - **D3D12 / Metal** – No direct equivalent; emulate in-shader if needed.
    DepthBoundsTest = 1ull << 22,

    /// @}
    /// @name Multiview / XR
    /// @{

    /// Simultaneous rendering to multiple views in a single pass.
    ///
    /// - **Vulkan** – `VK_KHR_multiview`
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_multiview.html
    /// - **OpenGL/ES** – `OVR_multiview2`
    ///   https://registry.khronos.org/OpenGL/extensions/OVR/OVR_multiview2.txt
    /// - **D3D12** – View Instancing (`SV_ViewID`)
    ///   https://microsoft.github.io/DirectX-Specs/d3d/ViewInstancing.html
    /// - **Metal** – Multiple viewports/view selection (not identical to Vulkan multiview).
    Multiview = 1ull << 23,

    /// @}
    /// @name Memory / residency
    /// @{

    /// Persistently CPU-mapped GPU buffers (no map/unmap per frame).
    ///
    /// - **OpenGL** – `ARB_buffer_storage` (persistent + coherent mapping)
    /// - **Vulkan** – `HOST_VISIBLE` / `HOST_COHERENT` memory (different lifetime semantics)
    /// - **D3D12** – Upload/Readback heaps (persistently CPU-mapped)
    /// - **Metal** – Shared storage mode buffers
    PersistentMappedBuffers = 1ull << 24,

    /// Sparse/tiled resources (partially resident textures & buffers).
    ///
    /// - **Vulkan** – Sparse binding / aliased residency (core + extensions)
    /// - **D3D12** – Tiled Resources
    ///   https://learn.microsoft.com/windows/win32/direct3d12/tiled-resources
    /// - **OpenGL** – `ARB_sparse_texture` (+ variants)
    /// - **Metal** – Sparse textures / heaps
    SparseResources = 1ull << 25,

    /// @}
    /// @name Presentation / render passes
    /// @{

    /// Render passes without persistent framebuffer/render-pass objects.
    ///
    /// - **Vulkan** – `VK_KHR_dynamic_rendering` (core in 1.3)
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_dynamic_rendering.html
    /// - **D3D12** – Optional RenderPass API; D3D12 has no required render-pass objects.
    /// - **OpenGL** – FBOs (no explicit render-pass objects).
    /// - **Metal** – Render pass descriptors are the native model.
    DynamicRendering = 1ull << 26,

    /// Swapchain / window-system presentation.
    ///
    /// - **Vulkan** – `VK_KHR_swapchain`
    ///   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_swapchain.html
    /// - **OpenGL** – Platform WSI (WGL/GLX/EGL); no explicit swapchain object.
    /// - **D3D12** – DXGI swap chains
    /// - **Metal** – `CAMetalLayer` drawable presentation
    Presentation = 1ull << 27,

    /// @}
    /// @name Texture compression families
    /// @{

    /// BC (DXT/S3TC) block compression.
    ///
    /// - **Vulkan** – BC formats (mandatory on desktop)
    ///   https://docs.vulkan.org/spec/latest/chapters/formats.html
    /// - **OpenGL** – `EXT_texture_compression_s3tc` (S3TC/DXT)
    ///   https://developer.download.nvidia.com/opengl/specs/GL_EXT_texture_compression_s3tc.txt
    /// - **D3D12** – BC natively supported.
    /// - **Metal** – BC on macOS; not on iOS.
    TexCompression_BC = 1ull << 28,

    /// ETC2/EAC block compression.
    ///
    /// - **Vulkan** – ETC2/EAC formats (opt-in feature)
    /// - **OpenGL ES 3.0+** – ETC2 mandatory
    /// - **D3D12** – Generally not supported; decode paths exist on some stacks.
    /// - **Metal** – Supported on iOS/tile-GPUs; check feature sets.
    TexCompression_ETC2 = 1ull << 29,

    /// ASTC LDR block compression.
    ///
    /// - **Vulkan** – `textureCompressionASTC_LDR` feature
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    /// - **OpenGL/ES** – `KHR_texture_compression_astc_ldr`
    /// - **D3D12** – Historically not supported; limited decode on some HW/OS.
    /// - **Metal** – ASTC widely supported on Apple GPUs (iOS) and modern Macs.
    TexCompression_ASTC_LDR = 1ull << 30,

    /// @}
    /// @name Debug / tooling
    /// @{

    /// GPU debug markers, object labels, and validation messaging.
    ///
    /// - **Vulkan** – `VK_EXT_debug_utils` / debug markers
    ///   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_debug_utils.html
    /// - **OpenGL** – `KHR_debug`
    ///   https://registry.khronos.org/OpenGL/extensions/KHR/KHR_debug.txt
    /// - **D3D12** – `ID3DUserDefinedAnnotation` / PIX markers
    ///   https://learn.microsoft.com/windows/win32/direct3d11/user-defined-annotations
    /// - **Metal** – Push/pop debug groups, object labels.
    DebugMarkers_Labels = 1ull << 31,

    /// @}
    // Add new bits above; keep <= 64 bits or split into Feature2.
};

} // namespace wren::rhi
namespace wren::foundation { template<> struct enable_flags<wren::rhi::Feature> : std::true_type {}; }
namespace wren::rhi {

/// Returns `true` if all bits in @p bits are set in @p set.
[[nodiscard]] constexpr bool has_all(Feature set, Feature bits) noexcept {
    return (underlying(set & bits) == underlying(bits));
}
/// Returns `true` if any bit in @p bits is set in @p set.
[[nodiscard]] constexpr bool has_any(Feature set, Feature bits) noexcept {
    return underlying(set & bits) != 0;
}

/// Curated subset of hardware limits queried once at device creation.
///
/// Each backend populates this from its native API:
///   - Vulkan: `VkPhysicalDeviceLimits` (via `vkGetPhysicalDeviceProperties`)
///   - D3D12: `D3D12_FEATURE_DATA_D3D12_OPTIONS*`, `D3D12_FEATURE_DATA_FORMAT_SUPPORT`
///   - Metal: Metal Feature Set Tables (OS- and GPU-family-specific)
///   - OpenGL: `glGetIntegerv` / `glGetInteger64v` (`GL_MAX_*` caps)
///
/// Only values that are routinely validated at object-creation time are
/// included.  Per-format capabilities (e.g., per-format MSAA support) should
/// be queried through dedicated format-info APIs rather than stored here.
struct DeviceLimits {
    /// @name Textures & images
    /// @{
    uint32_t maxImageDimension1D;   ///< Maximum width of a 1-D texture.
    uint32_t maxImageDimension2D;   ///< Maximum width/height of a 2-D texture.
    uint32_t maxImageDimension3D;   ///< Maximum width/height/depth of a 3-D texture.
    uint32_t maxCubeDimension;      ///< Maximum width/height of a cube-map face.
    uint32_t maxMipLevels;          ///< Maximum number of mip levels in a texture.
    uint32_t maxArrayLayers;        ///< Maximum number of layers in an array texture.
    /// @}

    /// @name Descriptors per stage
    /// Approximate cross-API limits; precise per-stage vs. total rules vary.
    /// @{
    uint32_t maxPerStageSamplers;         ///< Samplers visible to a single shader stage.
    uint32_t maxPerStageSampledImages;    ///< Sampled images (SRVs) per stage.
    uint32_t maxPerStageStorageImages;    ///< Storage images (UAVs) per stage.
    uint32_t maxPerStageUniformBuffers;   ///< Uniform / constant buffers per stage.
    uint32_t maxPerStageStorageBuffers;   ///< Storage / structured buffers per stage.
    /// @}

    /// @name Attachments
    /// @{
    uint32_t maxColorAttachments;   ///< Maximum simultaneous colour render targets.
    /// @}

    /// @name Vertex I/O
    /// @{
    uint32_t maxVertexInputBindings;    ///< Maximum vertex buffer bindings.
    uint32_t maxVertexInputAttributes;  ///< Maximum vertex input attributes.
    /// @}

    /// @name MSAA
    /// @{
    /// Maximum MSAA sample count across all formats.
    /// Per-format maximum must be queried separately via the format-info API.
    uint32_t maxMSAASamples;
    /// @}

    /// @name Alignment constraints (bytes)
    /// @{
    /// Minimum required alignment for uniform / constant buffer offsets.
    /// Typical values: 256 (D3D12 CBV, Vulkan, most GL), 64 (some GL drivers).
    uint32_t uniformBufferAlignment;
    /// Minimum required alignment for storage / structured buffer offsets.
    /// Use the most conservative value across target platforms.
    uint32_t storageBufferAlignment;
    /// @}

    /// @name Compute
    /// @{
    uint32_t maxComputeWorkGroupSizeX;          ///< Max local work-group size along X.
    uint32_t maxComputeWorkGroupSizeY;          ///< Max local work-group size along Y.
    uint32_t maxComputeWorkGroupSizeZ;          ///< Max local work-group size along Z.
    uint32_t maxComputeWorkGroupInvocations;    ///< Max total invocations per work-group (X × Y × Z).
    /// @}

    /// @name Timing
    /// @{
    /// Ticks per second of the device timestamp counter.
    /// Set to 1 when timestamps are emulated or unavailable.
    uint64_t timelineTickFrequency;
    /// @}
};

/// Capability snapshot returned by a backend at device creation.
struct Capabilities {
    Backend      backend         = Backend::None;  ///< Identifying backend API.
    uint32_t     apiVersionMajor = 0;              ///< Major component of the reported API version.
    uint32_t     apiVersionMinor = 0;              ///< Minor component of the reported API version.
    Feature      features        = Feature::None;  ///< Optional capabilities present on this device.
    DeviceLimits limits{};                         ///< Concrete numeric hardware limits.
};

/// Feature negotiation request attached to a @ref DeviceDesc.
///
/// Request model:
/// - `required`  – Must be present or device creation fails.
/// - `preferred` – Nice-to-have; backend proceeds with fallbacks if missing.
///
/// The backend must:
/// -# Probe the hardware once and fill @ref Capabilities.
/// -# If `required && !supported` → return an error with a descriptive message.
/// -# Otherwise succeed and log which preferred features were downgraded.
struct DeviceFeatureRequest {
    Feature required  = Feature::None;   ///< Must be present or creation fails.
    Feature preferred = Feature::None;   ///< Nice-to-have; falls back gracefully if missing.
};

/// Flags that control global device behaviour.
///
/// - `Debug`        – Enable API validation layers / `KHR_debug` / D3D12 debug layer / Metal validation.
/// - `Headless`     – No swapchain / presentation; use off-screen render targets.
/// - `HighPriority` – Hint for a high-priority queue if the platform supports it.
enum class DeviceFlag : uint32_t {
    None         = 0,
    Debug        = 1u << 0,   ///< Enable API validation / debug layers.
    Headless     = 1u << 1,   ///< Off-screen only; no presentation.
    HighPriority = 1u << 2,   ///< Prefer high-priority / compute queue.
};

[[nodiscard]] constexpr bool has_any(DeviceFlag set, DeviceFlag bits) noexcept {
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(bits)) != 0;
}

/// Descriptor passed to the backend at device creation.
///
/// @par Platform notes
/// - **Vulkan** – `nativeWindowHandle` is forwarded to the platform layer
///   (e.g. GLFW) to create a `VkSurfaceKHR`.
/// - **OpenGL** – `nativeWindowHandle` (or a GLFW window) is used to create
///   the GL context.
/// - **D3D12** – Used to create an `IDXGISwapChain*` when presenting.
/// - **Metal** – Maps to a `CAMetalLayer` host window when presenting.
struct DeviceDesc {
    void*                nativeWindowHandle    = nullptr;           ///< Window/view handle; null for headless.
    uint32_t             preferredAdapterIndex = 0;                 ///< Adapter hint for multi-GPU systems (0 = default).
    DeviceFlag           flags                 = DeviceFlag::None;  ///< Behaviour flags.
    DeviceFeatureRequest featureRequest{};                          ///< Required/preferred feature negotiation.
};



} // namespace wren::rhi

// enable_flags specialisations live outside wren::rhi so the template is
// specialised in its own (wren::foundation) enclosing namespace.
namespace wren::foundation {
template<> struct enable_flags<wren::rhi::DeviceFlag> : std::true_type {};
} // namespace wren::foundation

#endif // WREN_RHI_API_FEATURES_HPP
