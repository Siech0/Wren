set(BUILD_CORE RENDERER_BUILD_ALL OR RENDERER_BUILD_RESOURCE_PROCESSOR)
if(BUILD_CORE)
  add_subdirectory(core)
endif()

if(RENDERER_BUILD_ALL OR RENDERER_BUILD_RESOURCE_PROCESSOR)
  add_subdirectory(resource-processor)
endif()

if(RENDERER_BUILD_ALL OR RENDERER_BUILD_RENDERER)
  add_subdirectory(renderer)
endif()
