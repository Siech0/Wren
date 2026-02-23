
# Target generated for basic operations like retrieving version info
add_library(wren.version_info INTERFACE)
add_library(wren::version_info ALIAS wren.version_info)
target_include_directories(wren.version_info INTERFACE
    "$<BUILD_INTERFACE:${WREN_VERSION_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:include>")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS wren.version_info
    EXPORT wren_targets
    FILE_SET HEADERS
    COMPONENT development
)

# Main build
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(tools)
