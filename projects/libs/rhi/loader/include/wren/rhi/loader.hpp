#pragma once

#include <expected>
#include <string>

#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/backend.h>

namespace wren::rhi {

// -------------------------------------------------------------------------------------------------
// BackendLibrary — RAII owner of a loaded backend DLL.
//
// Usage:
//   auto result = BackendLibrary::load(Backend::Vulkan);
//   if (!result) { /* result.error() contains the reason */ }
//   auto& lib = *result;
//   std::println("Loaded backend id: {}", static_cast<int>(lib.backend_id()));
//
// The DLL is unloaded when the BackendLibrary is destroyed.
// BackendLibrary is move-only; do not copy.
//
// DLL naming convention (resolved relative to the executable):
//   Windows  : wren_rhi_<backend>.dll
//   Linux    : libwren_rhi_<backend>.so
//   macOS    : libwren_rhi_<backend>.dylib
// -------------------------------------------------------------------------------------------------
class BackendLibrary {
public:
    /// Load a backend DLL for the given Backend enum value.
    /// Returns an error string on failure.
    [[nodiscard]] static auto load(Backend which)
        -> std::expected<BackendLibrary, std::string>;

    [[nodiscard]] Backend backend_id() const noexcept {
        return static_cast<Backend>(backend_->backend_id());
    }

    BackendLibrary(BackendLibrary&& other) noexcept;
    BackendLibrary& operator=(BackendLibrary&& other) noexcept;
    ~BackendLibrary();

    BackendLibrary(const BackendLibrary&)            = delete;
    BackendLibrary& operator=(const BackendLibrary&) = delete;

private:
    explicit BackendLibrary(WrenRhiBackend* backend, void* lib_handle) noexcept
        : backend_(backend), lib_handle_(lib_handle) {}

    WrenRhiBackend* backend_    = nullptr;
    void*           lib_handle_ = nullptr;
};

} // namespace wren::rhi
