#include <wren/rhi/loader.hpp>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#include <cassert>
#include <cstring>

namespace wren::rhi {

// -------------------------------------------------------------------------------------------------
// Platform helpers
// -------------------------------------------------------------------------------------------------

static void* platform_load(const char* name) noexcept {
#ifdef _WIN32
    return static_cast<void*>(LoadLibraryA(name));
#else
    return dlopen(name, RTLD_NOW | RTLD_LOCAL);
#endif
}

static void platform_unload(void* handle) noexcept {
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

static void* platform_symbol(void* handle, const char* name) noexcept {
#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name));
#else
    return dlsym(handle, name);
#endif
}

static std::string platform_last_error() {
#ifdef _WIN32
    DWORD code = GetLastError();
    char buf[256]{};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, code, 0, buf, sizeof(buf), nullptr);
    return buf;
#else
    const char* msg = dlerror();
    return msg ? msg : "(unknown error)";
#endif
}

// -------------------------------------------------------------------------------------------------
// DLL filename resolution
// -------------------------------------------------------------------------------------------------

/// Returns the platform DLL filename for the given backend, or empty string for Backend::None.
static std::string dll_name(Backend b) {
    const char* suffix = nullptr;
    switch (b) {
        case Backend::Vulkan: suffix = "vulkan"; break;
        case Backend::OpenGL: suffix = "opengl"; break;
        case Backend::D3D12:  suffix = "d3d12";  break;
        case Backend::Metal:  suffix = "metal";  break;
        case Backend::None:   return {};
    }

    // Match CMake's DEBUG_POSTFIX "d" — appended when NDEBUG is not defined (Debug builds).
#ifdef NDEBUG
    static constexpr const char k_build_suffix[] = "";
#else
    static constexpr const char k_build_suffix[] = "d";
#endif

#ifdef _WIN32
    return std::string("wren_rhi_") + suffix + k_build_suffix + ".dll";
#elif defined(__APPLE__)
    return std::string("libwren_rhi_") + suffix + k_build_suffix + ".dylib";
#else
    return std::string("libwren_rhi_") + suffix + k_build_suffix + ".so";
#endif
}

// -------------------------------------------------------------------------------------------------
// BackendLibrary — static members
// -------------------------------------------------------------------------------------------------

auto BackendLibrary::load(Backend which) -> std::expected<BackendLibrary, std::string> {
    const auto name = dll_name(which);
    if (name.empty()) {
        return std::unexpected{"Backend::None cannot be loaded"};
    }

    void* handle = platform_load(name.c_str());
    if (!handle) {
        return std::unexpected{"Failed to load '" + name + "': " + platform_last_error()};
    }

    auto* create = reinterpret_cast<WrenRhiCreateFn>(platform_symbol(handle, "wren_rhi_create"));
    if (!create) {
        platform_unload(handle);
        return std::unexpected{"Symbol 'wren_rhi_create' not found in '" + name + "': " + platform_last_error()};
    }

    WrenRhiBackend* backend = create();
    if (!backend) {
        platform_unload(handle);
        return std::unexpected{"wren_rhi_create() returned null for '" + name + "'"};
    }

    if (backend->abi_version != wren::rhi::k_backend_abi_version) {
        platform_unload(handle);
        return std::unexpected{
            "ABI version mismatch for '" + name + "': "
            "expected " + std::to_string(wren::rhi::k_backend_abi_version) +
            ", got "    + std::to_string(backend->abi_version)
        };
    }

    if (!backend->backend_id) {
        platform_unload(handle);
        return std::unexpected{"Backend '" + name + "' has null backend_id function pointer"};
    }

    return BackendLibrary{backend, handle};
}

// -------------------------------------------------------------------------------------------------
// BackendLibrary — special members
// -------------------------------------------------------------------------------------------------

BackendLibrary::BackendLibrary(BackendLibrary&& other) noexcept
    : backend_(other.backend_), lib_handle_(other.lib_handle_) {
    other.backend_    = nullptr;
    other.lib_handle_ = nullptr;
}

BackendLibrary& BackendLibrary::operator=(BackendLibrary&& other) noexcept {
    if (this != &other) {
        this->~BackendLibrary();
        backend_          = other.backend_;
        lib_handle_       = other.lib_handle_;
        other.backend_    = nullptr;
        other.lib_handle_ = nullptr;
    }
    return *this;
}

BackendLibrary::~BackendLibrary() {
    if (!lib_handle_) return;
    platform_unload(lib_handle_);
    lib_handle_ = nullptr;
    backend_    = nullptr;
}

} // namespace wren::rhi
