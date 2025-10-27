
# Variables are required for subcomponents
set(WREN_FOUNDATION_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

# Prepare subcomponents
add_subdirectory(src)

# Prepare root level target
add_library(wren_foundation INTERFACE)
add_library(wren::foundation ALIAS wren_foundation)
target_include_directories(wren_foundation INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${WREN_FOUNDATION_INCLUDEDIR}>
)

target_sources(wren_foundation
    INTERFACE
        FILE_SET HEADERS BASE_DIRS "${WREN_FOUNDATION_INCLUDEDIR}" FILES
            "${WREN_FOUNDATION_INCLUDEDIR}/wren/foundation/utility/scope_exit.hpp"
            "${WREN_FOUNDATION_INCLUDEDIR}/wren/foundation/utility/unique_resource.hpp"
)

# Add tests and benchmarks
add_test_subdirectory(test)
add_benchmark_subdirectory(bench)

# Install target
include(GNUInstallDirs)
install(TARGETS wren_foundation
    EXPORT wren_foundation_targets
    COMPONENT development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT wren_foundation_targets
    FILE wren_foundation_targets.cmake
    NAMESPACE wren::
    DESTINATION lib/cmake/wren
    COMPONENT development
)
