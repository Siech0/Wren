
# Implemented at projects/libs/rhi/CMakeLists.txt
# Creates wren.rhi.vulkan target.
wren_add_backend(vulkan)

# ---------------------------------------------------------------------------
# Vulkan SDK — use the canonical CMake imported targets.
#   Vulkan::Vulkan  — loader + headers (link target)
#   Vulkan::Headers — headers only   (vulkan.hpp / vulkan_raii.hpp)
# vulkan-hpp is pulled from vcpkg and provides vulkan/vulkan_raii.hpp.
# ---------------------------------------------------------------------------
find_package(Vulkan REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
target_sources(wren.rhi.vulkan
    PRIVATE
        src/api.cpp
        src/vk_capabilities.cpp
        src/instance.cpp
        src/device.cpp
    PUBLIC
        FILE_SET HEADERS BASE_DIRS "${WREN_RHI_VULKAN_INCLUDEDIR}" FILES
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/api.hpp"
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/adapter.hpp"
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/instance.hpp"
            "${WREN_RHI_VULKAN_INCLUDEDIR}/wren/rhi/vulkan/device.hpp"
)

# ---------------------------------------------------------------------------
# Dependencies
#   vulkan-hpp header-only defines are PRIVATE (implementation detail).
#   Callers only see the wren::rhi types; they never link Vulkan directly.
# ---------------------------------------------------------------------------
target_link_libraries(wren.rhi.vulkan
    PRIVATE
        Vulkan::Vulkan
        spdlog::spdlog
)

# Use the vk::raii:: RAII wrapper from vulkan_raii.hpp.
# Exceptions remain enabled internally — we catch them at public API
# boundaries and convert to std::expected.  Do NOT leak these defines
# to consumers (PRIVATE only).
