add_library_target(
  core-display
  STATIC
  ALIAS_AS
  core::display
  PRIVATE
  SOURCES
  test.cpp
)
target_link_libraries(core-display PUBLIC core::common-inc)
