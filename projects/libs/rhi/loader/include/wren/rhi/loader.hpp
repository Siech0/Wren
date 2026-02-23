#pragma once

#include <expected>
#include <string>

#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/api/features.hpp>
#include <wren/rhi/backend.hpp>

namespace wren::rhi {

// -------------------------------------------------------------------------------------------------
// BackendDevice — RAII owner of a live device created inside a backend DLL.
//
// Obtained from BackendLibrary::create_device().
// Calls WrenRhiBackend::destroy_device() on destruction.
// Move-only; the backend DLL must outlive all devices it created.
// -------------------------------------------------------------------------------------------------
class BackendDevice {
public:
    /// Post-creation capability snapshot (features, limits, API version).
    [[nodiscard]] Capabilities const& capabilities() const noexcept {
        return capabilities_;
    }

    /// Raw opaque handle (for use by higher-level RHI wrappers that need to
    /// call additional backend entry points).
    [[nodiscard]] DeviceHandle handle() const noexcept { return handle_; }

    BackendDevice(BackendDevice&& other) noexcept;
    BackendDevice& operator=(BackendDevice&& other) noexcept;
    ~BackendDevice();

    BackendDevice(BackendDevice const&)            = delete;
    BackendDevice& operator=(BackendDevice const&) = delete;

private:
    friend class BackendLibrary;
    explicit BackendDevice(BackendVTable* backend,
                           DeviceHandle handle,
                           Capabilities caps) noexcept
        : backend_(backend), handle_(handle), capabilities_(caps) {}

    BackendVTable* backend_      = nullptr;
    DeviceHandle   handle_       = nullptr;
    Capabilities        capabilities_ = {};
};

// -------------------------------------------------------------------------------------------------
// BackendLibrary — RAII owner of a loaded backend DLL.
//
// Usage:
//   auto lib = BackendLibrary::load(Backend::Vulkan);
//   if (!lib) { /* lib.error() */ }
//
//   auto device = lib->create_device(desc);
//   if (!device) { /* device.error() */ }
//
// The DLL is unloaded when the BackendLibrary is destroyed.
// All BackendDevice instances obtained from a library must be destroyed
// before the BackendLibrary itself is destroyed.
// BackendLibrary is move-only; do not copy.
//
// DLL naming convention (resolved relative to the executable):
//   Windows  : wren_rhi_<backend>[d].dll
//   Linux    : libwren_rhi_<backend>[d].so
//   macOS    : libwren_rhi_<backend>[d].dylib
// -------------------------------------------------------------------------------------------------
class BackendLibrary {
public:
    /// Load a backend DLL for the given Backend enum value.
    /// Returns an error string on failure.
    [[nodiscard]] static auto load(Backend which)
        -> std::expected<BackendLibrary, std::string>;

    /// Creates a logical device satisfying @p desc.
    /// Returns an error string if device creation fails.
    [[nodiscard]] auto create_device(DeviceDesc const& desc)
        -> std::expected<BackendDevice, std::string>;

    [[nodiscard]] Backend backend_id() const noexcept {
        return static_cast<Backend>(backend_->backend_id());
    }

    BackendLibrary(BackendLibrary&& other) noexcept;
    BackendLibrary& operator=(BackendLibrary&& other) noexcept;
    ~BackendLibrary();

    BackendLibrary(const BackendLibrary&)            = delete;
    BackendLibrary& operator=(const BackendLibrary&) = delete;

private:
    explicit BackendLibrary(BackendVTable* backend, void* lib_handle) noexcept
        : backend_(backend), lib_handle_(lib_handle) {}

    BackendVTable* backend_    = nullptr;
    void*           lib_handle_ = nullptr;
};

} // namespace wren::rhi
