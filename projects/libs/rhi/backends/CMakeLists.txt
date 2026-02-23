option(WREN_BUILD_OPENGL_BACKEND "Build the OpenGL backend for the RHI library" ON)
option(WREN_BUILD_VULKAN_BACKEND "Build the Vulkan backend for the RHI library" ON)

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
# Controls the compile-time spdlog level (SPDLOG_ACTIVE_LEVEL) for backends.
# Log calls below the active level expand to no-ops; zero runtime cost.
# Each backend exposes its own override that falls back to this global.
#
# Valid values: trace | debug | info | warn | error | critical | off
# ---------------------------------------------------------------------------
set(WREN_RHI_LOG_LEVEL "warn" CACHE STRING
    "Default compile-time log level for all RHI backends.")
set_property(CACHE WREN_RHI_LOG_LEVEL PROPERTY STRINGS
    trace debug info warn error critical off)

# ---------------------------------------------------------------------------
# wren_add_backend(<name>)
#
# Scaffolds a standard RHI backend target named wren.rhi.<name>.
#
# After calling this function the backend's own CMakeLists.txt is responsible
# for populating sources, dependencies, and any additional compile definitions.
#
# Variables set in the calling scope:
#   WREN_RHI_<NAME>_INCLUDEDIR:         path to the backend's public headers
#   WREN_RHI_<NAME>_EXPORT_INCLUDEDIR:  path to the generated export header
# ---------------------------------------------------------------------------
function(wren_add_backend backend_name)
    string(TOLOWER ${backend_name} backend_name_lc)
    string(TOUPPER ${backend_name} backend_name_uc)

    set(WREN_RHI_${backend_name_uc}_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")
    set(WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/${backend_name_lc}/include")

    # Propagate to the calling scope so the backend CMakeLists can reference them.
    set(WREN_RHI_${backend_name_uc}_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include" PARENT_SCOPE)
    set(WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/${backend_name_lc}/include" PARENT_SCOPE)

    # Backends are always shared. They are loaded as runtime modules.
    add_library(wren.rhi.${backend_name_lc} MODULE)
    add_library(wren::rhi.${backend_name_lc} ALIAS wren.rhi.${backend_name_lc})

    target_link_libraries(wren.rhi.${backend_name_lc}
        PUBLIC wren::rhi.api
    )
    target_include_directories(wren.rhi.${backend_name_lc}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${WREN_RHI_${backend_name_uc}_INCLUDEDIR}>
            $<BUILD_INTERFACE:${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}>
    )
    target_compile_features(wren.rhi.${backend_name_lc} PUBLIC cxx_std_23)

    set_target_properties(wren.rhi.${backend_name_lc}
        PROPERTIES
            CXX_VISIBILITY_PRESET     hidden
            VISIBILITY_INLINES_HIDDEN YES
            VERSION                   ${PROJECT_VERSION}
            SOVERSION                 ${PROJECT_VERSION_MAJOR}
            EXPORT_NAME               rhi.${backend_name_lc}
            OUTPUT_NAME               "wren_rhi_${backend_name_lc}"
            DEBUG_POSTFIX             "d"
            BUILD_RPATH               "$ORIGIN"
            INSTALL_RPATH             "$ORIGIN"
            POSITION_INDEPENDENT_CODE ON
            LINK_WHAT_YOU_USE         OFF
            # MODULE DLLs use LIBRARY_OUTPUT_DIRECTORY (not RUNTIME_OUTPUT_DIRECTORY).
            # Mirror the runtime output dir so the DLL lands next to the exe and the
            # loader can find it by bare filename at runtime.
            LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )

    # Generate the export header.
    include(GenerateExportHeader)
    file(MAKE_DIRECTORY
        ${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc})
    generate_export_header(wren.rhi.${backend_name_lc}
        BASE_NAME        WREN_RHI_${backend_name_uc}
        EXPORT_FILE_NAME
            ${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc}/export.hpp
    )
    target_sources(wren.rhi.${backend_name_lc}
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS "${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}"
                FILES
                    "${WREN_RHI_${backend_name_uc}_EXPORT_INCLUDEDIR}/wren/rhi/${backend_name_lc}/export.hpp"
    )

    # Install rules.
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
    install(TARGETS wren.rhi.${backend_name_lc}
        EXPORT wren_rhi_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  COMPONENT runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  COMPONENT runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  COMPONENT development
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development
    )
    install(EXPORT wren_rhi_targets
        FILE      wren_rhi_targets.cmake
        NAMESPACE wren::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wren
        COMPONENT development
    )

    # Record which backends were built.
    set(WREN_RHI_BACKENDS_BUILT
        ${WREN_RHI_BACKENDS_BUILT}
        ${backend_name_lc}
        CACHE INTERNAL "List of built RHI backends"
    )

    # -----------------------------------------------------------------------
    # Per-backend compile-time log level.
    # Defaults to WREN_RHI_LOG_LEVEL but can be overridden per backend.
    # -----------------------------------------------------------------------
    set(WREN_RHI_${backend_name_uc}_LOG_LEVEL "${WREN_RHI_LOG_LEVEL}" CACHE STRING
        "Compile-time log level for the ${backend_name_lc} RHI backend.")
    set_property(CACHE WREN_RHI_${backend_name_uc}_LOG_LEVEL PROPERTY STRINGS
        trace debug info warn error critical off)

    string(TOLOWER "${WREN_RHI_${backend_name_uc}_LOG_LEVEL}" _log_level)
    if(_log_level STREQUAL "trace")
        set(_spdlog_level "SPDLOG_LEVEL_TRACE")
    elseif(_log_level STREQUAL "debug")
        set(_spdlog_level "SPDLOG_LEVEL_DEBUG")
    elseif(_log_level STREQUAL "info")
        set(_spdlog_level "SPDLOG_LEVEL_INFO")
    elseif(_log_level STREQUAL "warn")
        set(_spdlog_level "SPDLOG_LEVEL_WARN")
    elseif(_log_level STREQUAL "error")
        set(_spdlog_level "SPDLOG_LEVEL_ERROR")
    elseif(_log_level STREQUAL "critical")
        set(_spdlog_level "SPDLOG_LEVEL_CRITICAL")
    else()
        set(_spdlog_level "SPDLOG_LEVEL_OFF")
    endif()

    target_compile_definitions(wren.rhi.${backend_name_lc}
        PRIVATE SPDLOG_ACTIVE_LEVEL=${_spdlog_level}
    )
    message(STATUS "${backend_name_lc} backend log level: "
        "${WREN_RHI_${backend_name_uc}_LOG_LEVEL} (${_spdlog_level})")
endfunction()

if(WREN_BUILD_OPENGL_BACKEND)
    add_subdirectory(opengl)
endif()

if(WREN_BUILD_VULKAN_BACKEND)
    add_subdirectory(vulkan)
endif()
