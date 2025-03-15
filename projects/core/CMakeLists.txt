add_test_subdirectory(test)
add_benchmark_subdirectory(bench)

add_subdirectory(src)

# Top level core library
add_library(core STATIC src/version.cpp)
add_library(core::core ALIAS core)

set_target_properties(
  core
  PROPERTIES VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             EXPORT_NAME core
             OUTPUT_NAME core
)

# This definition is required if we want to be able to compile as a static library
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(core PUBLIC CORE_STATIC_DEFINE)
endif()

# This target is used to link against the generated export header
add_library(core-common-inc INTERFACE)
add_library(core::common-inc ALIAS core-common-inc)
target_include_directories(
  core-common-inc INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
                            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(core PUBLIC core::common-inc core::gfx core::display)
