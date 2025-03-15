# Documentation Generation #####
find_package(Doxygen)
if(DOXYGEN_FOUND)

  # Allow Output directory to be changed
  set(DOXYGEN_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/docs"
      CACHE PATH "Path for the generated Doxygen documentation."
  )
  mark_as_advanced(DOXYGEN_OUTPUT_DIRECTORY)

  # Prepare input and output Doxyfile locations
  set(DOXYGEN_IN "${PROJECT_SOURCE_DIR}/docs/Doxyfile.in")
  set(DOXYGEN_OUT "${PROJECT_BINARY_DIR}/Doxyfile")

  # Configure the Doxyfile
  configure_file("${DOXYGEN_IN}" "${DOXYGEN_OUT}" @ONLY)

  # Create the doxygen documentation generation target
  message(STATUS "Generating Doxygen target.")
  add_custom_target(
    docs-build
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_OUT}"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Building documentation."
    VERBATIM
  )

else()
  message("Doxygen needs to be installed on the system for documentation to be generated.")
endif()
