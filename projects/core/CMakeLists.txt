add_test_subdirectory(test)
add_benchmark_subdirectory(bench)

# Top level core library
set(CORE_EXPORT_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(CORE_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

include(GenerateExportHeader)
add_library(core SHARED)
add_library(core::core ALIAS core)
generate_export_header(core EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/core/export.hpp")

# Set default visibility
set_target_properties(
  core
  PROPERTIES CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
             LINK_WHAT_YOU_USE OFF
)

# Link sub-libraries privately - their objects will be in the DLL Use WHOLE_ARCHIVE to ensure all symbols from static
# libs are included
target_link_libraries(
  core PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,core::display> $<LINK_LIBRARY:WHOLE_ARCHIVE,core::gfx-vulkan>
               $<LINK_LIBRARY:WHOLE_ARCHIVE,core::utils>
)
# if TARGET_PROPERTY 'TYPE' is NOT equal to 'SHARED_LIBRARY' add 'CORE_STATIC_DEFINE' to compile definitions
target_compile_definitions(
  core PRIVATE "$<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>>:CORE_STATIC_DEFINE>"
)

# Generate the export header for the core library and create a target we can link against for static subcomponents
add_library(core-export INTERFACE)
add_library(core::export ALIAS core-export)
target_compile_definitions(core-export INTERFACE "core_EXPORTS")
target_sources(
  core-export
  INTERFACE FILE_SET
            public_headers
            TYPE
            HEADERS
            BASE_DIRS
            ${CORE_EXPORT_INCLUDEDIR}
            FILES
            "${CORE_EXPORT_INCLUDEDIR}/core/export.hpp"
)

# Main build
add_subdirectory(src)
