add_library(core-display STATIC)
add_library(core::display ALIAS core-display)

target_sources(core-display PRIVATE test.cpp)

target_link_libraries(core-display PUBLIC core::common-inc)
