include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(WREN_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/wren")

# Create a minimal config that includes sub-target exports (and finds deps)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/wrenConfig.cmake.in"
  "${CMAKE_BINARY_DIR}/wrenConfig.cmake"
  INSTALL_DESTINATION "${WREN_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/wrenConfigVersion.cmake"
  VERSION "${WREN_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_BINARY_DIR}/wrenConfig.cmake"
  "${CMAKE_BINARY_DIR}/wrenConfigVersion.cmake"
  DESTINATION "${WREN_INSTALL_CMAKEDIR}"
  COMPONENT development
)
