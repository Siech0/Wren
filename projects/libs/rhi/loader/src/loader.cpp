#include <wren/rhi/loader.hpp>

#include <wren/foundation/system/platform.hpp>

#ifdef WREN_PLATFORM_WINDOWS
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
#ifdef WREN_PLATFORM_WINDOWS
    return static_cast<void*>(LoadLibraryA(name));
#else
    return dlopen(name, RTLD_NOW | RTLD_LOCAL);
#endif
}

static void platform_unload(void* handle) noexcept {
#ifdef WREN_PLATFORM_WINDOWS
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

static void* platform_symbol(void* handle, const char* name) noexcept {
#ifdef WREN_PLATFORM_WINDOWS
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name));
#else
    return dlsym(handle, name);
#endif
}

static std::string platform_last_error() {
#ifdef WREN_PLATFORM_WINDOWS
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

    // Match CMake's DEBUG_POSTFIX "d" — appended in debug builds (WREN_BUILD_DEBUG).
#ifdef WREN_BUILD_RELEASE
    static constexpr const char k_build_suffix[] = "";
#else
    static constexpr const char k_build_suffix[] = "d";
#endif

#ifdef WREN_PLATFORM_WINDOWS
    return std::string("wren_rhi_") + suffix + k_build_suffix + ".dll";
#elif defined(WREN_PLATFORM_APPLE)
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

    auto* create = reinterpret_cast<BackendFactoryFn>(platform_symbol(handle, "wren_rhi_create"));
    if (!create) {
        platform_unload(handle);
        return std::unexpected{"Symbol 'wren_rhi_create' not found in '" + name + "': " + platform_last_error()};
    }

    BackendVTable* backend = create();
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

    if (!backend->create_device || !backend->destroy_device || !backend->get_capabilities) {
        platform_unload(handle);
        return std::unexpected{"Backend '" + name + "' has null device function pointer(s)"};
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

// -------------------------------------------------------------------------------------------------
// BackendLibrary — device creation
// -------------------------------------------------------------------------------------------------

auto BackendLibrary::create_device(DeviceDesc const& desc)
    -> std::expected<BackendDevice, std::string>
{
    char err_buf[512]{};
    DeviceHandle handle =
        backend_->create_device(&desc, err_buf, sizeof(err_buf));

    if (!handle) {
        return std::unexpected{
            err_buf[0] != '\0' ? std::string{err_buf} : std::string{"create_device returned null"}
        };
    }

    Capabilities caps{};
    backend_->get_capabilities(handle, &caps);
    return BackendDevice{backend_, handle, caps};
}

// -------------------------------------------------------------------------------------------------
// BackendDevice — special members
// -------------------------------------------------------------------------------------------------

BackendDevice::BackendDevice(BackendDevice&& other) noexcept
    : backend_(other.backend_)
    , handle_(other.handle_)
    , capabilities_(other.capabilities_)
{
    other.backend_ = nullptr;
    other.handle_  = nullptr;
}

BackendDevice& BackendDevice::operator=(BackendDevice&& other) noexcept {
    if (this != &other) {
        this->~BackendDevice();
        backend_      = other.backend_;
        handle_       = other.handle_;
        capabilities_ = other.capabilities_;
        other.backend_ = nullptr;
        other.handle_  = nullptr;
    }
    return *this;
}

BackendDevice::~BackendDevice() {
    if (!handle_) return;
    backend_->destroy_device(handle_);
    handle_  = nullptr;
    backend_ = nullptr;
}

} // namespace wren::rhi
