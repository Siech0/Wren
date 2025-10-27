
option(WREN_BUILD_RESOURCE_PROCESSOR "Build the resource processor application" ON)

if(WREN_BUILD_RESOURCE_PROCESSOR)
  add_subdirectory(resource_processor)
endif()
