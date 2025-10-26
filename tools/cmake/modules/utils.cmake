include("${CMAKE_CURRENT_LIST_DIR}/target_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake_utils.cmake")

# Add directory as test-only directory
function(add_test_subdirectory dir)
  if(RENDERER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(${dir})
  endif()
endfunction()

# Add directory as test-only directory
function(add_benchmark_subdirectory dir)
  if(RENDERER_BUILD_BENCHMARKS)
    add_subdirectory(${dir})
  endif()
endfunction()
