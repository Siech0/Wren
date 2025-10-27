
option(WREN_BUILD_OPENGL_BACKEND "Build the OpenGL backend for the RHI library" OFF) # Not implemented
option(WREN_BUILD_VULKAN_BACKEND "Build the Vulkan backend for the RHI library" OFF) # Not implemented
option(WREN_BUILD_SHARED_BACKENDS "Build RHI backends as shared libraries" ON)

add_subdirectory(api)


function(wren_add_backend backend_name)
    # Prepare RHI backend target
    # - Creates wren_rhi_<backend_name> target
    # - Defines alias target wren::rhi_<backend_name>
    # - Sets up include directories and export headers
    # - Installs target and export headers
    # - Defines WREN_RHI_<BACKEND_NAME>_INCLUDEDIR and WREN_RHI_<BACKEND_NAME>_EXPORT_INCLUDEDIR variables

    # This function assumes that callers will further populate the target sources, dependencies, etc.

    # Set uppercased and lowercased names
    string(TOLOWER ${backend_name} backend_name_lc)
    string(TOUPPER ${backend_name} backend_name_uc)

    set(WREN_RHI_${backend_name_uc}_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/${backend_name_lc}/include")
    set(WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/${backend_name_lc}/include")

    # Create library target
    if(WREN_BUILD_SHARED_BACKENDS)
        add_library(wren_rhi_${backend_name_lc} SHARED)
    else()
        add_library(wren_rhi_${backend_name_lc} STATIC)
    endif()
    add_library(wren::rhi_${backend_name_lc} ALIAS wren_rhi_${backend_name_lc})

    # Prepare library target
    target_link_libraries(wren_rhi_${backend_name_lc}
        PUBLIC wren::rhi_api
    )
    target_include_directories(wren_rhi_${backend_name_lc}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${WREN_RHI_${backend_name_uc}_INCLUDEDIR}>
            $<BUILD_INTERFACE:${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}>
    )
    target_compile_features(wren_rhi_${backend_name_lc} PUBLIC cxx_std_23)

    # Set visibility and shared object information
    set_target_properties(wren_rhi_${backend_name_lc}
      PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME rhi_${backend_name_lc}
        OUTPUT_NAME "wren_rhi_${backend_name_lc}"
        DEBUG_POSTFIX "d"
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
        POSITION_INDEPENDENT_CODE ON
        LINK_WHAT_YOU_USE OFF
    )

    # Prepare export headers
    include(GenerateExportHeader)
    file(MAKE_DIRECTORY ${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc}) # Ensure directory exists
    generate_export_header(wren_rhi_${backend_name_lc}
        BASE_NAME WREN_RHI_${backend_name_uc}
        EXPORT_FILE_NAME ${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc}/export.hpp
    )
    target_sources(wren_rhi_${backend_name_lc}
        PUBLIC
            FILE_SET HEADERS BASE_DIRS "${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}" FILES
                "${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc}/export.hpp"
    )

    # Install targets
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    # Install library target
    install(TARGETS wren_rhi_${backend_name_lc}
        EXPORT wren_rhi_targets
        COMPONENT runtime
    )
    install(TARGETS wren_rhi_${backend_name_lc}
        EXPORT wren_rhi_targets
        FILE_SET HEADERS
        COMPONENT development
    )

    # Append CMake Metadata
    set(WREN_RHI_BACKENDS_BUILT
        ${WREN_RHI_BACKENDS_BUILT}
        ${backend_name_lc}
        CACHE INTERNAL "List of built RHI backends"
    )
endfunction()

if(WREN_BUILD_OPENGL_BACKEND)
    add_subdirectory(backends/opengl)
endif()

if(WREN_BUILD_VULKAN_BACKEND)
    add_subdirectory(backends/vulkan)
endif()
