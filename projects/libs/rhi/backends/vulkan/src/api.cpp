#include <wren/rhi/vulkan/api.h>
#include <wren/rhi/api/enums.hpp>

static uint8_t vk_backend_id() noexcept {
    return static_cast<uint8_t>(wren::rhi::Backend::Vulkan);
}

static WrenRhiBackend s_vulkan_backend{
    .abi_version = wren::rhi::k_backend_abi_version,
    .backend_id  = vk_backend_id,
};

extern "C" WREN_RHI_VULKAN_EXPORT WrenRhiBackend* wren_rhi_create(void) {
    return &s_vulkan_backend;
}
