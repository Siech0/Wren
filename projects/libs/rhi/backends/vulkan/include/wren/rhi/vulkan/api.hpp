#ifndef WREN_RHI_VULKAN_API_HPP
#define WREN_RHI_VULKAN_API_HPP

#include <wren/rhi/backend.hpp>
#include <wren/rhi/vulkan/export.hpp>

extern "C" {
/// Factory entry-point. Returns a pointer to the static BackendVTable instance.
WREN_RHI_VULKAN_EXPORT wren::rhi::BackendVTable* wren_rhi_create();
} // extern "C"

#endif // WREN_RHI_VULKAN_API_HPP
