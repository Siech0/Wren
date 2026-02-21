


# Variables are required for subcomponents
set(WREN_RHI_API_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

# Prepare root level target
add_library(wren.rhi.api INTERFACE)
add_library(wren::rhi.api ALIAS wren.rhi.api)
target_include_directories(wren.rhi.api INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${WREN_RHI_API_INCLUDEDIR}>
)
target_compile_features(wren.rhi.api INTERFACE cxx_std_23)
target_sources(wren.rhi.api INTERFACE
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_RHI_API_INCLUDEDIR}" FILES
            "${WREN_RHI_API_INCLUDEDIR}/wren/rhi/api/api.hpp"
            "${WREN_RHI_API_INCLUDEDIR}/wren/rhi/api/features.hpp"
            "${WREN_RHI_API_INCLUDEDIR}/wren/rhi/api/enums.hpp"
            "${WREN_RHI_API_INCLUDEDIR}/wren/rhi/backend.h"
)
target_link_libraries(wren.rhi.api INTERFACE wren::foundation)

# Install target
include(GNUInstallDirs)
install(TARGETS wren.rhi.api
    EXPORT wren_rhi_api_targets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development
)
install(EXPORT wren_rhi_api_targets
    FILE wren_rhi_api_targets.cmake
    NAMESPACE wren::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wren
    COMPONENT development
)
