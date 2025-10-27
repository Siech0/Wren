
# Implemented at projects/libs/rhi/CMakeLists.txt
# Creates wren_rhi_opengl target.
wren_add_backend(opengl)

# Add source files and headers
target_sources(wren_rhi_opengl
    PRIVATE
      src/api.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            "${WREN_RHI_OPENGL_INCLUDEDIR}/wren/rhi/opengl/api.h"
)
