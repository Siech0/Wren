

option(WREN_BUILD_SHARED_PLATFORM "Build Platform library as shared library" ON)

# Variables are required for subcomponents
set(WREN_PLATFORM_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(WREN_PLATFORM_EXPORT_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/include")

# Create library target
if(WREN_BUILD_SHARED_PLATFORM)
    add_library(wren.platform SHARED)
else()
    add_library(wren.platform STATIC)
endif()
add_library(wren::platform ALIAS wren.platform)

# Prepare library target
target_include_directories(wren.platform
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${WREN_PLATFORM_INCLUDEDIR}>
        $<BUILD_INTERFACE:${WREN_PLATFORM_EXPORT_INCLUDEDIR}>
)
target_compile_features(wren.platform
    PUBLIC cxx_std_23
)
target_sources(wren.platform
    PRIVATE
        "src/window.cpp"
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_PLATFORM_INCLUDEDIR}" FILES
            "${WREN_PLATFORM_INCLUDEDIR}/wren/platform/window.hpp"
)

# Dependency management
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(wren.platform
    PRIVATE
        glfw
)

# Set visibility and shared object information
set_target_properties(wren.platform
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "wren_platform"
        DEBUG_POSTFIX "d"
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
        POSITION_INDEPENDENT_CODE ON
        LINK_WHAT_YOU_USE OFF
)

# Prepare export headers
include(GenerateExportHeader)
file(MAKE_DIRECTORY ${WREN_PLATFORM_EXPORT_INCLUDEDIR}/wren/platform) # Ensure directory exists
generate_export_header(wren.platform
    BASE_NAME WREN_PLATFORM
    EXPORT_FILE_NAME ${WREN_PLATFORM_EXPORT_INCLUDEDIR}/wren/platform/export.hpp
)
target_sources(wren.platform
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_PLATFORM_EXPORT_INCLUDEDIR}" FILES
            "${WREN_PLATFORM_EXPORT_INCLUDEDIR}/wren/platform/export.hpp"
)

# Install targets
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
# Install library target
install(TARGETS wren.platform
    EXPORT wren_platform_targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development
)
# Install export set
install(EXPORT wren_platform_targets
  FILE wren_platform_targets.cmake
  NAMESPACE wren::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wren
  COMPONENT development
)
