
# Implemented at projects/libs/rhi/CMakeLists.txt
# Creates wren.rhi.vulkan target.
wren_add_backend(vulkan)

# Add source files and headers
target_sources(wren.rhi.vulkan
    PRIVATE
      src/api.cpp
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_RHI_VULKAN_INCLUDEDIR}" FILES
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/api.h"
)
