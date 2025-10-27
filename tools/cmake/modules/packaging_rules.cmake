# ------------------------------------------------------------------------------
# CPack configuration for Wren project
# ------------------------------------------------------------------------------

# Basic metadata ---------------------------------------------------------------
set(CPACK_PACKAGE_NAME "wren")
set(CPACK_PACKAGE_VENDOR "Gavin Dunlap")
set(CPACK_PACKAGE_CONTACT "gavin.r.dunlap@gmail.com")

# Version info from your versioning.cmake
set(CPACK_PACKAGE_VERSION_MAJOR "${WREN_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${WREN_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${WREN_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${WREN_VERSION_STRING}")

# Use full extended version string for package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${WREN_VERSION_STRING}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Wren Engine: modular graphics and rendering framework")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/gavin-dunlap/wren")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# ------------------------------------------------------------------------------
# Install components (runtime, development, docs, tools)
# ------------------------------------------------------------------------------

set(CPACK_COMPONENTS_ALL runtime development docs tools)

# Friendly display names
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")
set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "C++ Headers and CMake Configs")
set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation (HTML/PDF)")
set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Command-line Tools")

# Descriptions
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Shared libraries and executables required to run Wren applications.")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "C++ headers and CMake targets to develop with Wren.")
set(CPACK_COMPONENT_DOCS_DESCRIPTION "API documentation and user manuals generated via Doxygen.")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "Auxiliary build and resource processing tools.")

# Group components for installers
set(CPACK_COMPONENT_GROUP_WREN_DISPLAY_NAME "Wren Engine")
set(CPACK_COMPONENT_GROUP_WREN_DESCRIPTION "Core engine, development headers, and tools for the Wren engine.")
set(CPACK_COMPONENT_RUNTIME_GROUP WREN)
set(CPACK_COMPONENT_DEVELOPMENT_GROUP WREN)
set(CPACK_COMPONENT_DOCS_GROUP WREN)
set(CPACK_COMPONENT_TOOLS_GROUP WREN)

# ------------------------------------------------------------------------------
# Packaging formats and defaults
# ------------------------------------------------------------------------------

# Common archive formats
set(CPACK_GENERATOR "ZIP;TGZ")

# Windows NSIS / macOS pkg
if(WIN32)
    list(APPEND CPACK_GENERATOR "NSIS")
elseif(APPLE)
    list(APPEND CPACK_GENERATOR "DragNDrop")
endif()

# Installer options (for NSIS, DMG, etc.)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "wren-${CPACK_PACKAGE_VERSION}")
set(CPACK_NSIS_DISPLAY_NAME "Wren Engine ${CPACK_PACKAGE_VERSION}")
set(CPACK_NSIS_PACKAGE_NAME "Wren Engine")
set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
set(CPACK_NSIS_MODIFY_PATH ON)

# ------------------------------------------------------------------------------
# Output directories
# ------------------------------------------------------------------------------

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")

# Include CPack last
include(CPack)
