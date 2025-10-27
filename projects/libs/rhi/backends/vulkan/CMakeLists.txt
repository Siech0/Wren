
# Implemented at projects/libs/rhi/CMakeLists.txt
# Creates wren_rhi_vulkan target.
wren_add_backend(vulkan)

# Add source files and headers
target_sources(wren_rhi_vulkan
    PRIVATE
      src/api.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/api.h"
)
