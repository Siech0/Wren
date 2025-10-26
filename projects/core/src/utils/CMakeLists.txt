add_library(core-utils INTERFACE)
add_library(core::utils ALIAS core-utils)
target_sources(
  core-utils
  INTERFACE FILE_SET
            public_headers
            TYPE
            HEADERS
            BASE_DIRS
            ${CORE_INCLUDEDIR}
            FILES
            "${CORE_INCLUDEDIR}/core/utils/scope_exit.hpp"
            "${CORE_INCLUDEDIR}/core/utils/unique_resource.hpp"
)
target_link_libraries(core-utils INTERFACE core::export)
