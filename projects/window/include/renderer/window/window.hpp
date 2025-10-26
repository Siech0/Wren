#pragma once

#include <string_view>

#include <GLFW/glfw3.h>

namespace renderer::window {

class Window {
public:
  Window(int width, int height, std::string_view title);
  Window(const Window &) = delete;
  auto operator=(const Window &) -> Window & = delete;
  Window(Window &&other) noexcept;
  auto operator=(Window &&other) noexcept -> Window &;

  ~Window();

  static void init_system();
  static void deinit_system();

  [[nodiscard]]
  static auto is_system_initialized() -> bool;

  [[nodiscard]]
  auto should_close() const noexcept -> bool;

  [[nodiscard]]
  auto native_handle() const noexcept -> GLFWwindow * { return window_; }

private:
  static inline bool system_initialized_ = false;
  GLFWwindow *window_;

  void release();
};

} // namespace renderer::window
