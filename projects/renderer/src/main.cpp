#include <cstring>
#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <core/vulkan/context.hpp>
#include <core/vulkan/vulkan.hpp>

auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc; // Mark unused
    (void)argv; // Mark unused
    core::vk::Context ctx;

  } catch (const std::exception &e) {
    std::cerr << "Critical Error: " << e.what() << '\n';
  }
}
