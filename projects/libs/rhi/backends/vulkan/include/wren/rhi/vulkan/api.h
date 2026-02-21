#ifndef WREN_RHI_VULKAN_API_H
#define WREN_RHI_VULKAN_API_H

#include <wren/rhi/backend.h>
#include <wren/rhi/vulkan/export.hpp>

#ifdef __cplusplus
extern "C" {
#endif

/// Factory entry-point. Returns a pointer to the static backend descriptor.
WREN_RHI_VULKAN_EXPORT WrenRhiBackend *wren_rhi_create(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WREN_RHI_VULKAN_API_H
