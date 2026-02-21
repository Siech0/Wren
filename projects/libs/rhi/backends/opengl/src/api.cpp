#include <wren/rhi/opengl/api.h>
#include <wren/rhi/api/enums.hpp>

static uint8_t gl_backend_id() noexcept {
    return static_cast<uint8_t>(wren::rhi::Backend::OpenGL);
}

static WrenRhiBackend s_opengl_backend{
    .abi_version = wren::rhi::k_backend_abi_version,
    .backend_id  = gl_backend_id,
};

extern "C" WREN_RHI_OPENGL_EXPORT WrenRhiBackend* wren_rhi_create(void) {
    return &s_opengl_backend;
}
