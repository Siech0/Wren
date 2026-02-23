#include <wren/rhi/opengl/api.hpp>

#include <wren/foundation/system/platform.hpp>
#include <wren/rhi/api/enums.hpp>
#include <wren/rhi/api/features.hpp>

#include <cstring>

static uint8_t gl_backend_id() noexcept {
    return static_cast<uint8_t>(wren::rhi::Backend::OpenGL);
}

static wren::rhi::DeviceHandle gl_create_device(
    wren::rhi::DeviceDesc const* /*desc*/,
    char*       err_buf,
    std::size_t err_len) noexcept
{
    static constexpr const char k_msg[] = "OpenGL backend: device creation not yet implemented";
    if (err_buf && err_len > 0) {
#ifdef WREN_COMPILER_MSVC_ABI
        strncpy_s(err_buf, err_len, k_msg, err_len - 1);
#else
        std::strncpy(err_buf, k_msg, err_len - 1);
        err_buf[err_len - 1] = '\0';
#endif
    }
    return nullptr;
}

static void gl_destroy_device(wren::rhi::DeviceHandle /*device*/) noexcept {}

static void gl_get_capabilities(
    wren::rhi::DeviceHandle  /*device*/,
    wren::rhi::Capabilities*  /*out*/) noexcept {}

static wren::rhi::BackendVTable s_opengl_backend{
    .abi_version      = wren::rhi::k_backend_abi_version,
    .backend_id       = gl_backend_id,
    .create_device    = gl_create_device,
    .destroy_device   = gl_destroy_device,
    .get_capabilities = gl_get_capabilities,
};

extern "C" WREN_RHI_OPENGL_EXPORT wren::rhi::BackendVTable* wren_rhi_create() {
    return &s_opengl_backend;
}
