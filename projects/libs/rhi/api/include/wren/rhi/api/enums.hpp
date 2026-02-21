
#ifndef WREN_RHI_API_ENUMS_HPP
#define WREN_RHI_API_ENUMS_HPP

#include <cstdint>

#include <wren/foundation/utility/enum_utils.hpp>

namespace wren::rhi {
    // ===================================================================================
// Backend metadata (no spec needed; this is just for your RIL bookkeeping)
// ===================================================================================
enum class Backend : std::uint8_t {
    OpenGL,      // GL core profile 3.3–4.6
    Vulkan,      // Khronos Vulkan
    D3D12,       // Direct3D 12
    Metal,       // Apple Metal
    None         // No backend (null device)
};

// ===================================================================================
// Queue / Command types
//   VK: VkQueueFlagBits — https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFlagBits.html
//   D3D12: D3D12_COMMAND_LIST_TYPE — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type
//   Metal: Encoders per pass — https://developer.apple.com/documentation/metal/command-encoder-factory-methods
//   GL: no explicit queues; work is serialized per context (glFlush/glFinish)
//       https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFinish.xhtml
// ===================================================================================
enum class QueueType : std::uint8_t {
    Graphics,   // VK: GRAPHICS_BIT | D3D12: DIRECT | Metal: render encoder | GL: draw calls
    Compute,    // VK: COMPUTE_BIT  | D3D12: COMPUTE | Metal: compute encoder | GL: compute dispatch (4.3+)
    Transfer,   // VK: TRANSFER_BIT | D3D12: COPY    | Metal: blit encoder   | GL: buffer/tex copy cmd
    Present     // VK: presentation-capable queue family | D3D12: swapchain on DIRECT | Metal: present drawable | GL: SwapBuffers
};

// ===================================================================================
// Shader stages (bitmask)
//   VK: VkShaderStageFlagBits — https://docs.vulkan.org/refpages/latest/refpages/source/VkShaderStageFlagBits.html
//   GL: classic stages; mesh via NV/EXT extensions
//       https://registry.khronos.org/OpenGL/extensions/EXT/EXT_mesh_shader.txt
//   D3D12: classic + Mesh/Amplification — https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html
//   Metal: vertex/fragment/compute; mesh shaders on Apple platforms — https://developer.apple.com/videos/play/wwdc2022/10162/
// ===================================================================================
enum class ShaderStage : std::uint32_t {
    None            = 0,
    Vertex          = 1u << 0,  // VK: VERTEX_BIT | GL: vertex | D3D12: VS | Metal: vertex
    TessControl     = 1u << 1,  // VK: TESSELLATION_CONTROL_BIT | GL: tess control | D3D12: HS | Metal: n/a (tess via compute)
    TessEval        = 1u << 2,  // VK: TESSELLATION_EVALUATION_BIT | GL: tess eval | D3D12: DS
    Geometry        = 1u << 3,  // VK: GEOMETRY_BIT | GL: geometry | D3D12: GS | Metal: n/a
    Fragment        = 1u << 4,  // VK: FRAGMENT_BIT | GL: fragment | D3D12: PS | Metal: fragment
    Compute         = 1u << 5,  // VK: COMPUTE_BIT | GL: compute | D3D12: CS | Metal: kernel/compute
    // Modern/optional:
    Task            = 1u << 6,  // VK: TASK_BIT_EXT | GL: TASK (NV/EXT) | D3D12: Amplification | Metal: object/mesh (platforms supporting)
    Mesh            = 1u << 7,  // VK: MESH_BIT_EXT | GL: MESH (NV/EXT)  | D3D12: Mesh       | Metal: mesh (iOS/macOS recent)
    // Ray tracing:
    RayGen          = 1u << 8,  // VK: RAYGEN_BIT_KHR | D3D12: Raytracing - DXIL library | Metal: n/a
    AnyHit          = 1u << 9,
    ClosestHit      = 1u << 10,
    Miss            = 1u << 11,
    Intersection    = 1u << 12,
    Callable        = 1u << 13
};
template<> struct wren::foundation::enable_flags<ShaderStage> : std::true_type {};

// ===================================================================================
// Primitive Topology
//   VK: VkPrimitiveTopology — https://docs.vulkan.org/refpages/latest/refpages/source/VkPrimitiveTopology.html
//   GL: glDraw* mode — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
//   D3D12: D3D_PRIMITIVE_TOPOLOGY — https://learn.microsoft.com/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_primitive_topology
//   Metal: MTLPrimitiveType — https://developer.apple.com/documentation/metal/mtlprimitivetype
// ===================================================================================
enum class PrimitiveTopology : std::uint8_t {
    PointList,        // VK_POINT_LIST | GL_POINTS | D3D: POINTLIST | MTLPrimitiveTypePoint
    LineList,         // VK_LINE_LIST  | GL_LINES  | D3D: LINELIST  | MTLPrimitiveTypeLine
    LineStrip,        // VK_LINE_STRIP | GL_LINE_STRIP | D3D: LINESTRIP | MTLPrimitiveTypeLineStrip
    TriangleList,     // VK_TRIANGLE_LIST | GL_TRIANGLES | D3D: TRIANGLELIST | MTLPrimitiveTypeTriangle
    TriangleStrip,    // VK_TRIANGLE_STRIP| GL_TRIANGLE_STRIP | D3D: TRIANGLESTRIP | MTLPrimitiveTypeTriangleStrip
    TriangleFan,      // VK_TRIANGLE_FAN | GL_TRIANGLE_FAN | D3D: (no fan) | Metal: (no fan)
    PatchList         // VK_PATCH_LIST | GL_PATCHES | D3D: PATCH control-points | Metal: use tess pipeline/compute emulation
};

// ===================================================================================
// Raster state: cull & winding
//   VK FrontFace/CullMode — https://docs.vulkan.org/refpages/latest/refpages/source/VkFrontFace.html
//   GL glFrontFace/glCullFace — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glFrontFace.xhtml
//   D3D12 rasterizer — https://learn.microsoft.com/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc
//   Metal winding/cull — https://developer.apple.com/documentation/metal/mtlwinding , https://developer.apple.com/documentation/metal/mtlcullmode
// ===================================================================================
enum class CullMode : std::uint8_t {
    None,           // VK_CULL_MODE_NONE | GL: disable CULL_FACE | D3D12: NONE | Metal: none
    Front,          // VK_CULL_MODE_FRONT_BIT | GL_FRONT | D3D12: FRONT | Metal: front
    Back,           // VK_CULL_MODE_BACK_BIT  | GL_BACK  | D3D12: BACK  | Metal: back
    FrontAndBack    // VK_CULL_MODE_FRONT_AND_BACK | GL_FRONT_AND_BACK | D3D12/Metal: not supported for raster draw
};

enum class FrontFace : std::uint8_t {
    CCW,      // VK_FRONT_FACE_COUNTER_CLOCKWISE | GL_CCW (default) | D3D12: FrontCounterClockwise=TRUE | Metal: counterClockwise
    CW        // VK_FRONT_FACE_CLOCKWISE         | GL_CW             | D3D12: FrontCounterClockwise=FALSE | Metal: clockwise
};

// ===================================================================================
// Multisample (MSAA) sample counts
//   VK: VkSampleCountFlagBits — https://docs.vulkan.org/refpages/latest/refpages/source/VkSampleCountFlagBits.html
//   GL: glRenderbufferStorageMultisample — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glRenderbufferStorageMultisample.xhtml
//   D3D12: DXGI_SAMPLE_DESC — https://learn.microsoft.com/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_sample_desc
//   Metal: render pipeline rasterSampleCount — https://developer.apple.com/documentation/metal/mtlrenderpipelinedescriptor/rastersamplecount
// ===================================================================================
enum class SampleCount : std::uint8_t {
    C1  = 1,  // VK_SAMPLE_COUNT_1_BIT | GL: default | D3D12: Count=1 | Metal: 1
    C2  = 2,  // 2x
    C4  = 4,  // 4x
    C8  = 8,  // 8x
    C16 = 16, // 16x
    C32 = 32  // 32x (rare)
};

// ===================================================================================
// Depth/stencil tests & ops
//   Compare: VkCompareOp — https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompareOp.html
//            glDepthFunc — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
//            D3D12_COMPARISON_FUNC — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_comparison_func
//            MTLCompareFunction — https://developer.apple.com/documentation/metal/mtlcomparefunction
//   Stencil: VkStencilOp — https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOp.html
//            glStencilOp — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glStencilOp.xhtml
//            D3D12_STENCIL_OP — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_stencil_op
//            MTLStencilOperation — https://developer.apple.com/documentation/metal/mtlstenciloperation
// ===================================================================================
enum class CompareOp : std::uint8_t {
    Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always
    // Maps directly across VK/GL/D3D12/Metal
};

enum class StencilOp : std::uint8_t {
    Keep,            // Keep existing value
    Zero,            // Write 0
    Replace,         // Write ref
    IncrementClamp,  // VK: INCREMENT_AND_CLAMP | GL: GL_INCR | D3D12: INCR_SAT | Metal: incrementClamp
    DecrementClamp,  // VK: DECREMENT_AND_CLAMP | GL: GL_DECR | D3D12: DECR_SAT | Metal: decrementClamp
    Invert,          // Bitwise invert
    IncrementWrap,   // VK: INCREMENT_AND_WRAP | GL: INCR_WRAP | D3D12: INCR | Metal: incrementWrap
    DecrementWrap    // VK: DECREMENT_AND_WRAP | GL: DECR_WRAP | D3D12: DECR | Metal: decrementWrap
};

// ===================================================================================
// Blending
//   Factors: VkBlendFactor — https://docs.vulkan.org/refpages/latest/refpages/source/VkBlendFactor.html
//            glBlendFunc — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBlendFunc.xhtml
//            D3D12_BLEND — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_blend
//            MTLBlendFactor — https://developer.apple.com/documentation/metal/mtlblendfactor
//   Ops:     VkBlendOp — https://docs.vulkan.org/refpages/latest/refpages/source/VkBlendOp.html
//            glBlendEquation — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBlendEquation.xhtml
//            D3D12_BLEND_OP — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_blend_op
//            MTLBlendOperation — https://developer.apple.com/documentation/metal/mtlblendoperation
// ===================================================================================
enum class BlendFactor : std::uint8_t {
    Zero, One,
    SrcColor, OneMinusSrcColor,
    DstColor, OneMinusDstColor,
    SrcAlpha, OneMinusSrcAlpha,
    DstAlpha, OneMinusDstAlpha,
    ConstantColor, OneMinusConstantColor,
    ConstantAlpha, OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color, OneMinusSrc1Color,   // Dual-source blending (VK/D3D12; GL with extensions; not Metal)
    Src1Alpha, OneMinusSrc1Alpha
};

enum class BlendOp : std::uint8_t {
    Add, Subtract, ReverseSubtract, Min, Max
};

enum class ColorWriteMask : std::uint8_t {
    None = 0,       // VK: 0 | GL: glColorMask(false,...) | D3D12: 0 | Metal: 0
    R = 1 << 0,     // VK_COLOR_COMPONENT_R_BIT | D3D12_COLOR_WRITE_ENABLE_RED | MTLColorWriteMaskRed
    G = 1 << 1,     // ...
    B = 1 << 2,
    A = 1 << 3,
    All = R | G | B | A
};
template<> struct wren::foundation::enable_flags<ColorWriteMask> : std::true_type {};

// ===================================================================================
// Sampler state
//   Filter: VkFilter/VkSamplerMipmapMode — https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerMipmapMode.html
//           GL: GL_TEXTURE_MIN/MAG_FILTER — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
//           D3D12_FILTER — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_filter
//           Metal: MTLSamplerMinMagFilter/MipFilter — https://developer.apple.com/documentation/metal/mtlsamplerminmagfilter
//   Address: VkSamplerAddressMode — https://docs.vulkan.org/refpages/latest/refpages/source/VkSamplerAddressMode.html
//            GL: GL_TEXTURE_WRAP_* — glTexParameter (above)
//            D3D12_TEXTURE_ADDRESS_MODE — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
//            MTLSamplerAddressMode — https://developer.apple.com/documentation/metal/mtlsampleraddressmode
//   Border:  VkBorderColor — https://registry.khronos.org/vulkan/specs/latest/man/html/VkBorderColor.html
//            GL: GL_TEXTURE_BORDER_COLOR (arbitrary vec4 via glSamplerParameterfv — no discrete enum)
//            D3D12_STATIC_BORDER_COLOR — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_static_border_color
//            MTLSamplerBorderColor — https://developer.apple.com/documentation/metal/mtlsamplerbordercolor (iOS 14+, macOS 10.12+, tvOS 16+)
// ===================================================================================
enum class Filter : std::uint8_t { Nearest, Linear };
enum class MipmapMode : std::uint8_t { Nearest, Linear };

enum class AddressMode : std::uint8_t {
    Repeat,             // VK_REPEAT | GL_REPEAT | D3D12_WRAP | Metal: repeat
    MirroredRepeat,     // VK_MIRRORED_REPEAT | GL_MIRRORED_REPEAT | D3D12_MIRROR | Metal: mirrorRepeat
    ClampToEdge,        // VK_CLAMP_TO_EDGE | GL_CLAMP_TO_EDGE | D3D12_CLAMP | Metal: clampToEdge
    ClampToBorder,      // VK_CLAMP_TO_BORDER | GL_CLAMP_TO_BORDER | D3D12_BORDER | Metal: clampToBorderColor (iOS 13+/macOS 10.12+)
    MirrorClampToEdge   // VK_MIRROR_CLAMP_TO_EDGE (1.2) | GL_MIRROR_CLAMP_TO_EDGE | D3D12: MIRROR_ONCE | Metal: mirrorClampToEdge (macOS 10.11+, iOS 14+, tvOS 16+)
};

enum class BorderColor : std::uint8_t {
    TransparentBlack,   // VK: VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK / INT_TRANSPARENT_BLACK | GL: {0,0,0,0} | D3D12: TRANSPARENT_BLACK | Metal: transparentBlack
    OpaqueBlack,        // VK: VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK / INT_OPAQUE_BLACK         | GL: {0,0,0,1} | D3D12: OPAQUE_BLACK       | Metal: opaqueBlack
    OpaqueWhite         // VK: VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE / INT_OPAQUE_WHITE         | GL: {1,1,1,1} | D3D12: OPAQUE_WHITE       | Metal: opaqueWhite
};

// ===================================================================================
// Vertex attribute formats (keep a practical subset)
//   VK formats — https://docs.vulkan.org/spec/latest/chapters/formats.html
//   GL vertex attrib format — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribFormat.xhtml
//   D3D: DXGI_FORMAT — https://learn.microsoft.com/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
//   Metal: MTLVertexFormat — https://developer.apple.com/documentation/metal/mtlvertexformat
// ===================================================================================
enum class VertexFormat : std::uint16_t {
    // 32-bit floats:
    R32_Float,              // VK_FORMAT_R32_SFLOAT          | GL: GL_FLOAT (x1)       | DXGI_FORMAT_R32_FLOAT          | MTLVertexFormatFloat
    RG32_Float,             // VK_FORMAT_R32G32_SFLOAT       | GL: GL_FLOAT (x2)       | DXGI_FORMAT_R32G32_FLOAT       | MTLVertexFormatFloat2
    RGB32_Float,            // VK_FORMAT_R32G32B32_SFLOAT    | GL: GL_FLOAT (x3)       | DXGI_FORMAT_R32G32B32_FLOAT    | MTLVertexFormatFloat3
    RGBA32_Float,           // VK_FORMAT_R32G32B32A32_SFLOAT | GL: GL_FLOAT (x4)       | DXGI_FORMAT_R32G32B32A32_FLOAT | MTLVertexFormatFloat4
    // UNorm 8-bit:
    R8_UNorm,               // VK_FORMAT_R8_UNORM            | GL: GL_UNSIGNED_BYTE (n, x1) | DXGI_FORMAT_R8_UNORM            | MTLVertexFormatUCharNormalized
    RG8_UNorm,              // VK_FORMAT_R8G8_UNORM          | GL: GL_UNSIGNED_BYTE (n, x2) | DXGI_FORMAT_R8G8_UNORM          | MTLVertexFormatUChar2Normalized
    RGBA8_UNorm,            // VK_FORMAT_R8G8B8A8_UNORM      | GL: GL_UNSIGNED_BYTE (n, x4) | DXGI_FORMAT_R8G8B8A8_UNORM     | MTLVertexFormatUChar4Normalized
    BGRA8_UNorm,            // VK_FORMAT_B8G8R8A8_UNORM      | GL: GL_UNSIGNED_BYTE BGRA ext | DXGI_FORMAT_B8G8R8A8_UNORM    | MTLVertexFormatUChar4Normalized_BGRA
    // SNorm 8-bit:
    RGBA8_SNorm,            // VK_FORMAT_R8G8B8A8_SNORM      | GL: GL_BYTE (n, x4)     | DXGI_FORMAT_R8G8B8A8_SNORM     | MTLVertexFormatChar4Normalized
    // Packed:
    RGB10A2_UNorm,          // VK_FORMAT_A2B10G10R10_UNORM_PACK32 | GL: GL_UNSIGNED_INT_2_10_10_10_REV (n) | DXGI_FORMAT_R10G10B10A2_UNORM | MTLVertexFormatUInt1010102Normalized
    R11G11B10_Float,        // VK_FORMAT_B10G11R11_UFLOAT_PACK32  | GL: GL_UNSIGNED_INT_10F_11F_11F_REV    | DXGI_FORMAT_R11G11B10_FLOAT   | MTLVertexFormatFloatRG11B10
    // UInt 16-bit:
    R16_UInt,               // VK_FORMAT_R16_UINT            | GL: GL_UNSIGNED_SHORT (x1) | DXGI_FORMAT_R16_UINT           | MTLVertexFormatUShort
    RG16_UInt,              // VK_FORMAT_R16G16_UINT         | GL: GL_UNSIGNED_SHORT (x2) | DXGI_FORMAT_R16G16_UINT        | MTLVertexFormatUShort2
    RGBA16_UInt,            // VK_FORMAT_R16G16B16A16_UINT   | GL: GL_UNSIGNED_SHORT (x4) | DXGI_FORMAT_R16G16B16A16_UINT  | MTLVertexFormatUShort4
    // UInt / SInt 32-bit:
    R32_UInt,               // VK_FORMAT_R32_UINT            | GL: GL_UNSIGNED_INT (x1)   | DXGI_FORMAT_R32_UINT           | MTLVertexFormatUInt
    RG32_UInt,              // VK_FORMAT_R32G32_UINT         | GL: GL_UNSIGNED_INT (x2)   | DXGI_FORMAT_R32G32_UINT        | MTLVertexFormatUInt2
    RGBA32_UInt,            // VK_FORMAT_R32G32B32A32_UINT   | GL: GL_UNSIGNED_INT (x4)   | DXGI_FORMAT_R32G32B32A32_UINT  | MTLVertexFormatUInt4
    R32_SInt,               // VK_FORMAT_R32_SINT            | GL: GL_INT (x1)            | DXGI_FORMAT_R32_SINT           | MTLVertexFormatInt
    RG32_SInt,              // VK_FORMAT_R32G32_SINT         | GL: GL_INT (x2)            | DXGI_FORMAT_R32G32_SINT        | MTLVertexFormatInt2
    RGBA32_SInt             // VK_FORMAT_R32G32B32A32_SINT   | GL: GL_INT (x4)            | DXGI_FORMAT_R32G32B32A32_SINT  | MTLVertexFormatInt4
};

// ===================================================================================
// Index type
//   VK: VkIndexType — https://docs.vulkan.org/refpages/latest/refpages/source/VkIndexType.html
//   GL: glDrawElements 'type' — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
//   D3D12: DXGI_FORMAT_R16/R32_UINT in index buffer view — https://learn.microsoft.com/windows/win32/api/d3d12/ns-d3d12-d3d12_index_buffer_view
//   Metal: MTLIndexType — https://developer.apple.com/documentation/metal/mtlindextype
// ===================================================================================
enum class IndexType : std::uint8_t {
    Uint16,  // VK: VK_INDEX_TYPE_UINT16 | GL: GL_UNSIGNED_SHORT | D3D12: DXGI_FORMAT_R16_UINT | Metal: MTLIndexTypeUInt16
    Uint32,  // VK: VK_INDEX_TYPE_UINT32 | GL: GL_UNSIGNED_INT   | D3D12: DXGI_FORMAT_R32_UINT | Metal: MTLIndexTypeUInt32
    Uint8    // GL: GL_UNSIGNED_BYTE | VK: VK_INDEX_TYPE_UINT8 (core VK 1.4; ext: VK_EXT_index_type_uint8) | D3D12/Metal: not supported
};

// ===================================================================================
// Texture & buffer usage flags (unified for barriers/creation)
//   Texture: VkImageUsageFlagBits — https://docs.vulkan.org/refpages/latest/refpages/source/VkImageUsageFlagBits.html
//            GL: no explicit usage flags — usage is implicit (FBO attachment, sampler binding, etc.)
//            D3D12_RESOURCE_FLAGS — https://learn.microsoft.com/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
//            MTLTextureUsage — https://developer.apple.com/documentation/metal/mtltextureusage
//   Buffer:  VkBufferUsageFlagBits — https://docs.vulkan.org/refpages/latest/refpages/source/VkBufferUsageFlagBits.html
//            GL: no explicit usage flags — target (ARRAY_BUFFER, etc.) determines use at bind time
//            D3D12: no buffer resource flags; usage is implicit via view type and resource state
//            Metal: no MTLBuffer usage flags; any buffer can fill any role
// ===================================================================================
enum class TextureUsage : std::uint32_t {
    None            = 0,
    Sampled         = 1u << 0,   // VK_IMAGE_USAGE_SAMPLED_BIT          | GL: implicit (texture unit bind) | D3D12: no flag (SRV; state NON_PIXEL_SHADER_RESOURCE / PIXEL_SHADER_RESOURCE) | Metal: shaderRead
    Storage         = 1u << 1,   // VK_IMAGE_USAGE_STORAGE_BIT          | GL: image unit (glBindImageTexture) | D3D12: ALLOW_UNORDERED_ACCESS (UAV) | Metal: shaderWrite
    ColorAttachment = 1u << 2,   // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | GL: FBO color attachment (glFramebufferTexture2D) | D3D12: ALLOW_RENDER_TARGET | Metal: renderTarget
    DepthStencilAtt = 1u << 3,   // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | GL: FBO depth/stencil attachment | D3D12: ALLOW_DEPTH_STENCIL | Metal: renderTarget
    TransferSrc     = 1u << 4,   // VK_IMAGE_USAGE_TRANSFER_SRC_BIT     | GL: implicit (glBlitFramebuffer / glCopyImageSubData src) | D3D12: no flag (state COPY_SOURCE) | Metal: no flag (any texture is blit-able)
    TransferDst     = 1u << 5    // VK_IMAGE_USAGE_TRANSFER_DST_BIT     | GL: implicit (copy/blit dst) | D3D12: no flag (state COPY_DEST) | Metal: no flag (any texture is blit-able)
};
template<> struct wren::foundation::enable_flags<TextureUsage> : std::true_type {};

enum class BufferUsage : std::uint32_t {
    None        = 0,
    Vertex      = 1u << 0,  // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT   | GL: GL_ARRAY_BUFFER target          | D3D12: no flag (bound via VBV)  | Metal: no flag (setVertexBuffer)
    Index       = 1u << 1,  // VK_BUFFER_USAGE_INDEX_BUFFER_BIT    | GL: GL_ELEMENT_ARRAY_BUFFER target  | D3D12: no flag (bound via IBV)  | Metal: no flag (setIndexBuffer)
    Uniform     = 1u << 2,  // VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT  | GL: GL_UNIFORM_BUFFER target        | D3D12: no flag (bound via CBV)  | Metal: no flag (setBytes / setBuffer)
    Storage     = 1u << 3,  // VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | GL: GL_SHADER_STORAGE_BUFFER target | D3D12: ALLOW_UNORDERED_ACCESS   | Metal: no flag (read/write in shader)
    Indirect    = 1u << 4,  // VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | GL: GL_DRAW_INDIRECT_BUFFER target  | D3D12: no flag (ExecuteIndirect) | Metal: no flag (drawPrimitives indirect)
    TransferSrc = 1u << 5,  // VK_BUFFER_USAGE_TRANSFER_SRC_BIT    | GL: implicit copy src               | D3D12: no flag (state COPY_SOURCE) | Metal: no flag
    TransferDst = 1u << 6   // VK_BUFFER_USAGE_TRANSFER_DST_BIT    | GL: implicit copy dst               | D3D12: no flag (state COPY_DEST)   | Metal: no flag
};
template<> struct wren::foundation::enable_flags<BufferUsage> : std::true_type {};

// ===================================================================================
// Texture dimension & common pixel formats (compact set)
//   VK: VkFormat — https://docs.vulkan.org/spec/latest/chapters/formats.html
//   GL: sized internal formats — https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
//   D3D: DXGI_FORMAT — https://learn.microsoft.com/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
//   Metal: MTLPixelFormat — https://developer.apple.com/documentation/metal/mtlpixelformat
// ===================================================================================
enum class TextureDimension : std::uint8_t { Tex1D, Tex2D, Tex3D, Cube };

enum class TextureFormat : std::uint16_t {
    // Color (UNORM, sRGB):
    RGBA8_UNorm,         // VK_FORMAT_R8G8B8A8_UNORM        | GL_RGBA8             | DXGI_FORMAT_R8G8B8A8_UNORM        | MTLPixelFormatRGBA8Unorm
    BGRA8_UNorm,         // VK_FORMAT_B8G8R8A8_UNORM        | n/a (no GL_BGRA8 internal format; BGRA is upload-only) | DXGI_FORMAT_B8G8R8A8_UNORM | MTLPixelFormatBGRA8Unorm
    RGBA8_sRGB,          // VK_FORMAT_R8G8B8A8_SRGB         | GL_SRGB8_ALPHA8      | DXGI_FORMAT_R8G8B8A8_UNORM_SRGB   | MTLPixelFormatRGBA8Unorm_sRGB
    BGRA8_sRGB,          // VK_FORMAT_B8G8R8A8_SRGB         | n/a (see BGRA8_UNorm note) | DXGI_FORMAT_B8G8R8A8_UNORM_SRGB | MTLPixelFormatBGRA8Unorm_sRGB
    // HDR/float:
    RG16_Float,          // VK_FORMAT_R16G16_SFLOAT         | GL_RG16F             | DXGI_FORMAT_R16G16_FLOAT           | MTLPixelFormatRG16Float
    RGBA16_Float,        // VK_FORMAT_R16G16B16A16_SFLOAT   | GL_RGBA16F           | DXGI_FORMAT_R16G16B16A16_FLOAT     | MTLPixelFormatRGBA16Float
    RGBA32_Float,        // VK_FORMAT_R32G32B32A32_SFLOAT   | GL_RGBA32F           | DXGI_FORMAT_R32G32B32A32_FLOAT     | MTLPixelFormatRGBA32Float
    R11G11B10_Float,     // VK_FORMAT_B10G11R11_UFLOAT_PACK32 | GL_R11F_G11F_B10F  | DXGI_FORMAT_R11G11B10_FLOAT       | MTLPixelFormatRG11B10Float
    RGB10A2_UNorm,       // VK_FORMAT_A2B10G10R10_UNORM_PACK32 | GL_RGB10_A2       | DXGI_FORMAT_R10G10B10A2_UNORM     | MTLPixelFormatRGB10A2Unorm
    // Depth/stencil:
    D24S8,               // VK_FORMAT_D24_UNORM_S8_UINT     | GL_DEPTH24_STENCIL8  | DXGI_FORMAT_D24_UNORM_S8_UINT     | MTLPixelFormatDepth24Unorm_Stencil8 (macOS only)
    D32,                 // VK_FORMAT_D32_SFLOAT            | GL_DEPTH_COMPONENT32F | DXGI_FORMAT_D32_FLOAT            | MTLPixelFormatDepth32Float
    D32S8,               // VK_FORMAT_D32_SFLOAT_S8_UINT    | GL_DEPTH32F_STENCIL8 | DXGI_FORMAT_D32_FLOAT_S8X24_UINT  | MTLPixelFormatDepth32Float_Stencil8
};

[[nodiscard]] inline const char *to_string(Backend b) {
  switch (b) {
    case Backend::OpenGL: return "OpenGL";
    case Backend::Vulkan: return "Vulkan";
    case Backend::D3D12:  return "D3D12";
    case Backend::Metal:  return "Metal";
    case Backend::None:   return "None";
  }
  return "Unknown";
}

} // namespace wren::rhi

#endif // WREN_RHI_API_ENUMS_HPP
