#include <core/vulkan/context.hpp>

#include <stdexcept>

namespace core::vk {
Context::Context() {
  // Load supported extensions
  if (!core::vk::loader::load_vulkan()) {
    throw std::runtime_error("Unable to load vulkan library.");
  }

  uint32_t extensions_count = 0;
  VkResult result = VK_SUCCESS;

  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count,
                                                  nullptr);
  if ((result != VK_SUCCESS) || (extensions_count == 0)) {
    throw std::runtime_error(
        "Unable to get the number of Instance extensions.");
  }

  m_extensionProperties.resize(static_cast<size_t>(extensions_count));

  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count,
                                                  m_extensionProperties.data());
  if ((result != VK_SUCCESS) || (extensions_count == 0)) {
    throw std::runtime_error(
        "Unable to get the number of Instance extensions.");
  }
}

Context::~Context() { core::vk::loader::unload_vulkan(); }
} // namespace core::vk
