
set(WREN_RHI_LOADER_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(wren.rhi.loader STATIC)
add_library(wren::rhi.loader ALIAS wren.rhi.loader)

target_sources(wren.rhi.loader
    PRIVATE
        src/loader.cpp
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_RHI_LOADER_INCLUDEDIR}" FILES
            "${WREN_RHI_LOADER_INCLUDEDIR}/wren/rhi/loader.hpp"
)

target_include_directories(wren.rhi.loader
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${WREN_RHI_LOADER_INCLUDEDIR}>
)

target_link_libraries(wren.rhi.loader
    PUBLIC wren::rhi.api
)

target_compile_features(wren.rhi.loader PUBLIC cxx_std_23)

# dlopen / dlclose require -ldl on Linux/BSD
if(NOT WIN32)
    target_link_libraries(wren.rhi.loader PRIVATE ${CMAKE_DL_LIBS})
endif()

set_target_properties(wren.rhi.loader PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    EXPORT_NAME rhi.loader
    DEBUG_POSTFIX "d"
)

# Install
include(GNUInstallDirs)
install(TARGETS wren.rhi.loader
    EXPORT wren_rhi_targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT development
)
