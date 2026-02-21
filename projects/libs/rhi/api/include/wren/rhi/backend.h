#ifndef WREN_RHI_BACKEND_H
#define WREN_RHI_BACKEND_H

// -------------------------------------------------------------------------------------------------
// WrenRhiBackend — stable C ABI contract between the loader and backend DLLs.
//
// Rules:
//   - extern "C" linkage ensures no name-mangling on the exported factory
//   symbol.
//     The file itself is C++ and may use C++ features.
//   - Every backend DLL exports exactly one symbol with C linkage:
//       WrenRhiBackend* wren_rhi_create(void);
//   - The returned pointer must remain valid for the lifetime of the DLL.
//     Backends return a pointer to a static instance.
//   - All function pointers must be non-null after wren_rhi_create() returns.
//
// Versioning:
//   - k_backend_abi_version is incremented whenever WrenRhiBackend's layout
//   changes.
//     The loader rejects backends whose abi_version field does not match.
//     Both sides include this header, so mismatches only occur with
//     stale/cached DLLs.
//
// References:
//   Vulkan loader–ICD interface:
//     https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
//   WebGPU C header (same pattern):
//     https://github.com/webgpu-native/webgpu-headers/blob/main/webgpu.h
// -------------------------------------------------------------------------------------------------

#include <cstdint>

namespace wren::rhi {
// Increment when WrenRhiBackend's layout or any function pointer signature
// changes.
inline constexpr uint32_t k_backend_abi_version = 1;
} // namespace wren::rhi

extern "C" {

typedef struct WrenRhiBackend WrenRhiBackend;

struct WrenRhiBackend {
  uint32_t abi_version; // must equal wren::rhi::k_backend_abi_version

  // Returns the Backend enum value (wren::rhi::Backend cast to uint8_t).
  // Corresponds to the compile-time identity of this backend DLL.
  uint8_t (*backend_id)(void);
};

// Factory function type — resolved via dlsym / GetProcAddress.
typedef WrenRhiBackend *(*WrenRhiCreateFn)(void);

} // extern "C"

#endif // WREN_RHI_BACKEND_H
