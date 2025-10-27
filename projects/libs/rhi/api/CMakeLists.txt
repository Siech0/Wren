


# Variables are required for subcomponents
set(WREN_RHI_API_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

# Prepare root level target
add_library(wren_rhi_api INTERFACE)
add_library(wren::rhi_api ALIAS wren_rhi_api)
target_include_directories(wren_rhi_api INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${WREN_RHI_API_INCLUDEDIR}>
)
target_compile_features(wren_rhi_api INTERFACE cxx_std_23)


# Install target
include(GNUInstallDirs)
install(TARGETS wren_rhi_api
    EXPORT wren_rhi_api_targets
    COMPONENT development
)
install(EXPORT wren_rhi_api_targets
    FILE wren_rhi_api_targets.cmake
    NAMESPACE wren::
    DESTINATION lib/cmake/wren
    COMPONENT development
)
install(DIRECTORY "${RHI_API_INCLUDEDIR}/"
    DESTINATION include/wren/rhi_api
    COMPONENT development
)
