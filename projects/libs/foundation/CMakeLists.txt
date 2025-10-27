
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

# Add tests and benchmarks
add_test_subdirectory(test)
add_benchmark_subdirectory(bench)

# Install target
include(GNUInstallDirs)
install(TARGETS wren_foundation
    EXPORT wren_foundation_targets
    COMPONENT development
)
install(EXPORT wren_foundation_targets
    FILE wren_foundation_targets.cmake
    NAMESPACE wren::
    DESTINATION lib/cmake/wren_foundation
    COMPONENT development
)
install(DIRECTORY "${WREN_FOUNDATION_INCLUDEDIR}/"
    DESTINATION include/wren/foundation
    COMPONENT development
)
