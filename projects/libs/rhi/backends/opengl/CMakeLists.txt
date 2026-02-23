
# Implemented at projects/libs/rhi/CMakeLists.txt
# Creates wren.rhi.opengl target.
wren_add_backend(opengl)

# Add source files and headers
target_sources(wren.rhi.opengl
    PRIVATE
      src/api.cpp
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_RHI_OPENGL_INCLUDEDIR}" FILES
            "${WREN_RHI_OPENGL_INCLUDEDIR}/wren/rhi/opengl/api.hpp"
)
