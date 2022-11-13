#pragma once

#include <core/vulkan/vulkan.hpp>
#include <vector>

namespace core::vk {
class Context {
public:
  Context();
  ~Context();
  Context(const Context &other) = delete;
  auto operator=(const Context &other) -> Context & = delete;
  Context(Context &&other) = default;
  auto operator=(Context &&other) -> Context & = default;

private:
  std::vector<VkExtensionProperties> m_extensionProperties;
  std::vector<char *> m_enabledExtensions;
};
} // namespace core::vk
