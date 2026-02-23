#ifndef WREN_RHI_BACKEND_HPP
#define WREN_RHI_BACKEND_HPP

// -------------------------------------------------------------------------------------------------
// wren::rhi::BackendVTable — stable ABI contract between the loader and backend DLLs.
//
// Rules:
//   - Every backend DLL exports exactly one symbol with C linkage:
//       wren::rhi::BackendVTable* wren_rhi_create();
//   - The returned pointer must remain valid for the lifetime of the DLL.
//     Backends return a pointer to a static instance.
//   - All function pointers in BackendVTable must be non-null on return.
//
// Versioning:
//   - k_backend_abi_version is incremented whenever BackendVTable's layout
//     or any function-pointer signature changes.
//   - The loader rejects backends whose abi_version field does not match.
//   - Both sides include this header, so mismatches only occur with
//     stale / cached DLLs.
//
// References:
//   Vulkan loader–ICD interface:
//     https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
//   WebGPU C header (same pattern):
//     https://github.com/webgpu-native/webgpu-headers/blob/main/webgpu.h
// -------------------------------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>

#include <wren/rhi/api/features.hpp>

namespace wren::rhi {

/// Increment when BackendVTable's layout or any function-pointer signature changes.
inline constexpr uint32_t k_backend_abi_version = 1;

/// Forward declaration of the per-backend opaque device state.
/// Each backend defines this struct in its own translation unit.
struct DeviceState;

/// Opaque handle to a live device owned by the backend DLL.
/// Obtained from BackendVTable::create_device; released with BackendVTable::destroy_device.
using DeviceHandle = DeviceState*;

/// Stable vtable exchanged between the loader and a backend DLL.
struct BackendVTable {
    /// Must equal k_backend_abi_version; checked by the loader on load.
    uint32_t abi_version;

    /// Returns the Backend enum value cast to uint8_t.
    /// Identifies the compile-time API of this backend DLL.
    uint8_t (*backend_id)();

    // -----------------------------------------------------------------
    // Device lifecycle
    // -----------------------------------------------------------------

    /// Creates a logical device satisfying @p desc.
    ///
    /// @param desc     Creation parameters; pointer only needs to be valid
    ///                 for the duration of this call.
    /// @param err_buf  Receives a null-terminated error message on failure
    ///                 (up to @p err_len bytes including the null terminator).
    /// @param err_len  Byte capacity of err_buf.
    /// @returns        Opaque device handle on success, nullptr on failure.
    DeviceHandle (*create_device)(DeviceDesc const* desc, char* err_buf, std::size_t err_len);

    /// Destroys a device previously returned by create_device.
    /// Passing nullptr is a no-op.
    void (*destroy_device)(DeviceHandle device);

    // -----------------------------------------------------------------
    // Queries (safe to call from any thread after device creation)
    // -----------------------------------------------------------------

    /// Copies the post-creation capability snapshot into @p out.
    void (*get_capabilities)(DeviceHandle device, Capabilities* out);
};

/// Factory function type — resolved by the loader via dlsym / GetProcAddress.
using BackendFactoryFn = BackendVTable*(*)();

} // namespace wren::rhi

#endif // WREN_RHI_BACKEND_HPP
