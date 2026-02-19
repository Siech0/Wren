#ifndef WREN_RHI_API_FEATURES_HPP
#define WREN_RHI_API_FEATURES_HPP

#include <type_traits>

#include <wren/foundation/enum_utils.hpp>

namespace wren::rhi {

// --- Feature bits ------------------------------------------------------------
//
// Feature is a bitset of "capabilities you can rely on". Query/enable them
// per-backend at device creation and cache the resulting mask.
//
// Scope/intent:
// - Only features that affect engine-level codegen/paths.
// - Each enumerator documents the rough mapping across APIs and links.
// - If a feature is "emulable but slow", the comment says so.
//
// NOTE: Some OpenGL extensions listed are vendor/ARB; always check runtime.
// NOTE: Metal often implements similar behavior under different names.
enum class Feature : std::uint64_t {
    None = 0,

    // ---------------- Pipeline stages / programmable stages -----------------

    Tessellation = 1ull << 0,
    // Vulkan: core feature bit `tessellationShader` (VkPhysicalDeviceFeatures)
    //   https://docs.vulkan.org/spec/latest/chapters/tessellation.html
    // OpenGL: core since 4.0 / ARB_tessellation_shader
    //   https://www.opengl.org/registry/doc/glspec40.core.20100311.pdf
    // D3D12: Hull/Domain shaders (FL11+)
    //   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    // Metal: Tessellation pipeline (Metal 2+)
    //   https://metalbyexample.com/tessellation/

    GeometryShader = 1ull << 1,
    // Vulkan: `geometryShader` feature bit
    //   https://docs.vulkan.org/spec/latest/chapters/geometry.html
    // OpenGL: core since 3.2
    // D3D12: Geometry shader stage available
    //   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    // Metal: NO geometry shaders; emulate with compute/mesh paths where needed.

    MeshShader = 1ull << 2,
    // Vulkan: VK_EXT_mesh_shader
    //   https://www.khronos.org/blog/mesh-shading-for-vulkan
    // OpenGL: NV/EXT_mesh_shader (vendor/EXT)
    // D3D12: Mesh/Amplification shaders (SM 6.5+)
    //   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_shader_visibility
    // Metal: Mesh shaders in Metal 3+ (Apple GPU families).
    // Caveat: Shader models & limits vary by vendor/OS.

    RayTracing = 1ull << 3,
    // Vulkan: VK_KHR_ray_tracing_pipeline (+ acceleration structures)
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_ray_tracing_pipeline.html
    // OpenGL: NV_ray_tracing (vendor)
    // D3D12: DXR 1.0/1.1
    //   https://learn.microsoft.com/windows/win32/direct3d12/directx-raytracing
    // Metal: Ray Tracing (Metal 3+)
    //   https://developer.apple.com/documentation/metal/ray_tracing

    // ---------------- Synchronization / submission --------------------------

    TimelineSemaphore = 1ull << 4,
    // Vulkan: VK_KHR_timeline_semaphore (core in 1.2; still requires feature enable)
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_timeline_semaphore.html
    // D3D12: ID3D12Fence is a 64-bit timeline fence
    //   https://learn.microsoft.com/windows/win32/api/d3d12/nn-d3d12-id3d12fence
    // Metal: MTLSharedEvent (timeline-like)
    //   https://developer.apple.com/documentation/metal/mtlsharedevent
    // OpenGL: No native timeline; emulate with sync objects/CPU waits.

    // ---------------- Resource binding / descriptors ------------------------

    DescriptorIndexing_Bindless = 1ull << 5,
    // Vulkan: VK_EXT_descriptor_indexing (non-uniform indexing, large arrays)
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_descriptor_indexing.html
    // OpenGL: ARB_bindless_texture (handle residency)
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_bindless_texture.txt
    // D3D12: Descriptor heaps + Resource Binding Tiers 1–3
    //   https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
    // Metal: Argument Buffers / Argument Tables
    //   https://developer.apple.com/documentation/metal/improving-cpu-performance-by-using-argument-buffers

    DescriptorBuffer = 1ull << 6,
    // Vulkan: VK_EXT_descriptor_buffer (descriptors stored in GPU-visible memory)
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_descriptor_buffer.html
    // D3D12/GL/Metal: No direct equivalent; often similar patterns via heaps/argument buffers.

    BufferDeviceAddress = 1ull << 7,
    // Vulkan: VK_KHR_buffer_device_address
    // D3D12: GPU virtual addresses on buffers are standard
    // Metal: Pointers via argument buffers/indirect addressing (no explicit "BDA" toggle)
    // OpenGL: No direct, emulate via bindless handles/SSBO indices.

    // ---------------- Draw/dispatch & indirect ------------------------------

    MultiDrawIndirect = 1ull << 8,
    // Vulkan: vkCmdDraw*Indirect + VK_KHR_draw_indirect_count
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_draw_indirect_count.html
    // OpenGL: ARB_multi_draw_indirect / ARB_indirect_parameters
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_multi_draw_indirect.txt
    // D3D12: ExecuteIndirect (different model)
    //   https://learn.microsoft.com/windows/win32/direct3d12/executeindirect
    // Metal: Indirect Command Buffers
    //   https://developer.apple.com/documentation/metal/accelerating_draw_calls_with_indirect_command_buffers

    // ---------------- Shading features / data types -------------------------

    Subgroup_WaveOps = 1ull << 9,
    // Vulkan: subgroup operations (core 1.1) + VK_EXT_subgroup_size_control
    // OpenGL: KHR_shader_subgroup (where available)
    // D3D12: HLSL Wave intrinsics (Shader Model 6+)
    //   https://learn.microsoft.com/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
    // Metal: SIMD-group functions in MSL

    ShaderFloat16_Int8 = 1ull << 10,
    // Vulkan: VK_KHR_shader_float16_int8
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_shader_float16_int8.html
    // plus 8/16-bit storage: VK_KHR_{8,16}bit_storage
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_8bit_storage.html
    // OpenGL: various vendor/EXT 16-bit types; not universal
    // D3D12: SM 6.x supports min-precision types; check target
    // Metal: `half` types widely supported

    ShaderInt64 = 1ull << 11,
    // Vulkan: 64-bit ints available in core (shaderInt64 feature)
    // OpenGL: ARB_gpu_shader_int64
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_gpu_shader_int64.txt
    // D3D12: 64-bit integer support in HLSL (SM 6+), but atomics vary
    // Metal: limited; no 64-bit image formats; use two 32-bit halves if needed

    ImageLoadStore_UAV = 1ull << 12,
    // Vulkan: storage images / UAV-equivalents
    //   (format capabilities: VkFormatFeature* STORAGE_IMAGE)
    // OpenGL: ARB_shader_image_load_store
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_shader_image_load_store.txt
    // D3D12: UAVs
    // Metal: readWrite textures/buffers in compute/fragment

    // ---------------- Rasterization & sampling ------------------------------

    VariableRateShading = 1ull << 13,
    // Vulkan: VK_KHR_fragment_shading_rate
    //   https://docs.vulkan.org/samples/latest/samples/extensions/fragment_shading_rate_dynamic/README.html
    // OpenGL: NV_shading_rate_image (vendor)
    // D3D12: VRS (Options6 / Tier 1–3)
    //   https://learn.microsoft.com/windows/win32/direct3d12/vrs
    // Metal: Rasterization Rate Maps (tile shading)
    //   https://developer.apple.com/documentation/metal/rasterization_rate_maps

    ConservativeRaster = 1ull << 14,
    // Vulkan: VK_EXT_conservative_rasterization
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_conservative_rasterization.html
    // OpenGL: NV_conservative_raster
    // D3D12: ConservativeRasterizationTier (1–3)
    //   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_conservative_rasterization_tier
    // Metal: not directly; some hardware-specific approximations

    FragmentInterlock_ROV = 1ull << 15,
    // Vulkan: VK_EXT_fragment_shader_interlock
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_fragment_shader_interlock.html
    // OpenGL: ARB_fragment_shader_interlock
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_shader_interlock.txt
    // D3D12: Rasterizer Ordered Views (ROVs)
    //   https://learn.microsoft.com/windows/win32/direct3d12/rasterizer-order-views
    // Metal: Raster Order Groups (areRasterOrderGroupsSupported)
    //   https://developer.apple.com/documentation/metal/mtldevice/arerasterordergroupssupported

    SampleRateShading = 1ull << 16,
    // Vulkan: `sampleRateShading` feature bit
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    // OpenGL: ARB_sample_shading
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_sample_shading.txt
    // D3D12: per-sample PS frequency path (SV_SampleIndex) + VRS alternatives
    // Metal: behavior varies; MoltenVK caveats for full supersampling

    AnisotropicFiltering = 1ull << 17,
    // Vulkan: `samplerAnisotropy` feature
    // OpenGL: EXT_texture_filter_anisotropic
    // D3D12: SamplerDesc.MaxAnisotropy
    // Metal: MTLSamplerDescriptor.maxAnisotropy

    DepthClamp = 1ull << 18,
    // Vulkan: `depthClamp` feature / depthClampEnable
    // OpenGL: GL_DEPTH_CLAMP
    // D3D12: RasterizerState.DepthClipEnable=false (not exactly clamp; beware)
    // Metal: MTLDepthClipMode::Clamp (platform support varies)

    DualSourceBlending = 1ull << 19,
    // Vulkan: `dualSrcBlend` feature
    // OpenGL: ARB_blend_func_extended
    // D3D12: Dual-source color blend supported
    // Metal: not supported (emulate with MRT in many cases)

    MirrorClampToEdge = 1ull << 20,
    // Vulkan: VK_KHR_sampler_mirror_clamp_to_edge
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_sampler_mirror_clamp_to_edge.html
    // OpenGL: ARB_texture_mirror_clamp_to_edge
    //   https://registry.khronos.org/OpenGL/extensions/ARB/ARB_texture_mirror_clamp_to_edge.txt
    // D3D12: Address mode variants (Mirror + Clamp) achieve similar behavior
    //   https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
    // Metal: MTLSamplerAddressMode::mirrorClampToEdge

    NonSolidFill = 1ull << 21,
    // Vulkan: `fillModeNonSolid` feature (wireframe/points)
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    // OpenGL: Polygon mode LINE/POINT
    // D3D12: FillMode_WIREFRAME
    // Metal: wireframe is limited (macOS); not on all Apple GPUs

    DepthBoundsTest = 1ull << 22,
    // Vulkan: `depthBounds` feature
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    // OpenGL: EXT_depth_bounds_test (vendor/EXT)
    // D3D12/Metal: no direct; emulate in-shader if needed

    // ---------------- Multiview / XR ---------------------------------------

    Multiview = 1ull << 23,
    // Vulkan: VK_KHR_multiview
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_multiview.html
    // OpenGL/ES: OVR_multiview2
    //   https://registry.khronos.org/OpenGL/extensions/OVR/OVR_multiview2.txt
    // D3D12: View Instancing (SV_ViewID)
    //   https://microsoft.github.io/DirectX-Specs/d3d/ViewInstancing.html
    // Metal: multiple viewports/view selection (not identical to VK multiview)

    // ---------------- Memory / residency -----------------------------------

    PersistentMappedBuffers = 1ull << 24,
    // OpenGL: ARB_buffer_storage (persistent/coherent mapping)
    // Vulkan: HOST_VISIBLE/HOST_COHERENT memory (not identical lifetime semantics)
    // D3D12: Upload/Readback heaps (persistently CPU-mapped)
    // Metal: Shared storage mode buffers

    SparseResources = 1ull << 25,
    // Vulkan: sparse binding/aliased residency (core+extensions)
    // D3D12: Tiled Resources
    //   https://learn.microsoft.com/windows/win32/direct3d12/tiled-resources
    // OpenGL: ARB_sparse_texture (+ variants)
    // Metal: Sparse textures/heaps

    // ---------------- Presentation / render-passes --------------------------

    DynamicRendering = 1ull << 26,
    // Vulkan: VK_KHR_dynamic_rendering (core in 1.3)
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_dynamic_rendering.html
    // D3D12: RenderPass API optional; D3D12 has no render-pass objects (N/A)
    // OpenGL: FBOs (no explicit render pass objects)
    // Metal: Render pass descriptors are the native model

    Presentation = 1ull << 27,
    // Vulkan: VK_KHR_swapchain
    //   https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_swapchain.html
    // OpenGL: platform WSI (WGL/GLX/EGL) swap interval controls; no "swapchain" object
    // D3D12: DXGI swap chains
    // Metal: CAMetalLayer drawable presentation

    // ---------------- Texture compression families -------------------------

    TexCompression_BC = 1ull << 28,
    // Vulkan: BC formats (mandatory on desktop)
    //   https://docs.vulkan.org/spec/latest/chapters/formats.html
    // OpenGL: EXT_texture_compression_s3tc (S3TC/DXT)
    //   https://developer.download.nvidia.com/opengl/specs/GL_EXT_texture_compression_s3tc.txt
    // D3D12: BC natively supported
    // Metal: BC on macOS; not on iOS

    TexCompression_ETC2 = 1ull << 29,
    // Vulkan: ETC2/EAC formats per feature
    // OpenGL ES 3.0+: ETC2 mandatory
    // D3D12: generally not supported (decode paths exist in some stacks)
    // Metal: supported on iOS/tile-GPUs; check feature sets

    TexCompression_ASTC_LDR = 1ull << 30,
    // Vulkan: `textureCompressionASTC_LDR` feature
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
    // OpenGL/ES: KHR_texture_compression_astc_ldr
    // D3D12: historically not supported; limited decode on some HW/OS
    // Metal: ASTC widely supported on Apple GPUs (iOS) & modern Macs

    // ---------------- Debug / tooling --------------------------------------

    DebugMarkers_Labels = 1ull << 31,
    // Vulkan: VK_EXT_debug_utils / debug markers
    //   https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_debug_utils.html
    // OpenGL: KHR_debug
    //   https://registry.khronos.org/OpenGL/extensions/KHR/KHR_debug.txt
    // D3D12: ID3DUserDefinedAnnotation / PIX markers
    //   https://learn.microsoft.com/windows/win32/direct3d11/user-defined-annotations
    // Metal: push/pop debug groups, object labels

    // (add new bits above; keep <= 64 or split into multiple enums)
};
template<> struct wren::foundation::enable_flags<Feature> : std::true_type {};

// Small utility helpers
[[nodiscard]] constexpr bool has_all(Feature set, Feature bits) {
  return (underlying(set & bits) == underlying(bits));
}
[[nodiscard]] constexpr bool has_any(Feature set, Feature bits) {
  return underlying(set & bits) != 0;
}

// -------------------------------------------------------------------------------------------------
// Device limits
//  - A curated subset of limits you’ll routinely validate at object creation.
//  - Backends fill this from the native API once (e.g., VkPhysicalDeviceLimits / D3D12 feature queries / Metal feature sets / GL queries).
//
//  References:
//   Vulkan limits:   VkPhysicalDeviceLimits
//   D3D12 limits:    D3D12_FEATURE_DATA_D3D12_OPTIONS*, D3D12_FEATURE_DATA_FORMAT_SUPPORT
//   Metal limits:    Metal Feature Set Tables (OS & GPU family specific)
//   OpenGL limits:   glGet* caps (e.g., GL_MAX_* queries)
// -------------------------------------------------------------------------------------------------
struct DeviceLimits {
  // Textures & images
  uint32_t maxImageDimension1D = 16384;
  uint32_t maxImageDimension2D = 16384;
  uint32_t maxImageDimension3D = 2048;
  uint32_t maxCubeDimension    = 16384;
  uint32_t maxMipLevels        = 15;
  uint32_t maxArrayLayers      = 2048;

  // Samplers & descriptors per stage (approx. across APIs)
  uint32_t maxPerStageSamplers        = 16;
  uint32_t maxPerStageSampledImages   = 128;
  uint32_t maxPerStageStorageImages   = 8;
  uint32_t maxPerStageUniformBuffers  = 12;
  uint32_t maxPerStageStorageBuffers  = 8;

  // Attachments
  uint32_t maxColorAttachments = 8;

  // Vertex I/O
  uint32_t maxVertexInputBindings   = 16;
  uint32_t maxVertexInputAttributes = 16;

  // MSAA
  uint32_t maxMSAASamples = 8; // query per-format in native APIs

  // Alignment constraints (bytes)
  uint32_t uniformBufferAlignment = 256;  // D3D12 CBV: 256; Vulkan often 256; GL typically 256 or 64 depending on impl
  uint32_t storageBufferAlignment = 256;  // varies; choose conservative

  // Compute
  uint32_t maxComputeWorkGroupSizeX = 1024;
  uint32_t maxComputeWorkGroupSizeY = 1024;
  uint32_t maxComputeWorkGroupSizeZ = 64;
  uint32_t maxComputeWorkGroupInvocations = 1024;

  // Timeline counter granularity (if emulated, this is 1)
  uint64_t timelineTickFrequency = 1;
};

// -------------------------------------------------------------------------------------------------
// Capabilities: the snapshot your backend returns at device creation.
// -------------------------------------------------------------------------------------------------
struct Capabilities {
  Backend    backend           = Backend::None;
  uint32_t   apiVersionMajor   = 0;
  uint32_t   apiVersionMinor   = 0;
  Feature    features          = Feature::None;   // optional capabilities
  DeviceLimits     limits{};                            // concrete numeric limits
};

// -------------------------------------------------------------------------------------------------
// Device feature negotiation
//
// Request model:
//   - 'required'  → must be present or creation fails (e.g., MeshShaders for a tool).
//   - 'preferred' → nice-to-have; backend proceeds with fallbacks if missing.
// Backend must:
//   - Probe once, fill Capabilities.
//   - If (required ∧ !supported) → return MissingRequiredFeature with a helpful message.
//   - Otherwise succeed and log which preferred features were downgraded.
// -------------------------------------------------------------------------------------------------
struct DeviceFeatureRequest {
  Feature required  = Feature::None;   // must be present or creation fails
  Feature preferred = Feature::None;   // nice-to-have; fallback if missing
};

// -------------------------------------------------------------------------------------------------
// Common device flags (debug, headless, etc.)
//
// Debug       → request validation layers / KHR_debug / D3D12 debug / Metal validation
// Headless    → no swapchain / presentation (use offscreen targets)
// HighPriority→ hint for high-priority queue if platform supports it
// -------------------------------------------------------------------------------------------------
enum class DeviceFlag : uint32_t {
  None         = 0,
  Debug        = 1u << 0,
  Headless     = 1u << 1,
  HighPriority = 1u << 2
};
template<> struct enable_flags<DeviceFlag> : std::true_type {};

// -------------------------------------------------------------------------------------------------
// Device creation descriptor
//
// nativeWindowHandle     → window/view handle when you intend to present;
//                          leave null for headless devices.
// preferredAdapterIndex  → hint for multi-GPU systems (0 = default).
// flags                  → see DeviceFlag.
// featureRequest         → required/preferred feature negotiation.
//
// Platform notes:
//   - Vulkan: window handle is used by the platform layer (e.g., GLFW) to pick a surface.
//   - OpenGL: window handle (or a GLFW handle) is used to create the GL context.
//   - D3D12: used to create a swapchain (IDXGISwapChain*) if presenting.
//   - Metal: maps to CAMetalLayer hosting when presenting.
// -------------------------------------------------------------------------------------------------
struct DeviceDesc {
  void*     nativeWindowHandle   = nullptr;   // HWND / NSView* / GLFWwindow* / etc.
  uint32_t  preferredAdapterIndex = 0;
  DeviceFlag flags               = DeviceFlag::None;

  DeviceFeatureRequest featureRequest{};
};


// -------------------------------------------------------------------------------------------------
// Utilities
// -------------------------------------------------------------------------------------------------




} // namespace wren::rhi

#endif // WREN_RHI_API_FEATURES_HPP
