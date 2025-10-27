
option(WREN_BUILD_RENDERER "Build the renderer application" ON)

if(WREN_BUILD_RENDERER)
  add_subdirectory(renderer)
endif()
