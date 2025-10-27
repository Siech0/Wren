

# Generate cmake package files
include(CMakePackageConfigHelpers)

# Generate & install package version config
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/wren_config_version.cmake"
    VERSION ${WREN_VERSION}
    COMPATIBILITY SameMajorVersion
)
