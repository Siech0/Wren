#pragma once

#include <string_view>

#include <GLFW/glfw3.h>

#include <core/export.hpp>

namespace core::display {
    class CORE_EXPORT window  {
    public:

        window(int width, int height, std::string_view title);
        window(const window&) = delete;
        auto operator=(const window&) -> window& = delete;
        window(window&& other) noexcept;
        auto operator=(window&& other) noexcept -> window&;

        virtual ~window();

        static void init_system();
        static void deinit_system();

        [[nodiscard]]
        static auto is_system_initialized() -> bool;

        [[nodiscard]]
        auto should_close() const noexcept -> bool;

    private:
        static inline bool _system_initialized = false;
        GLFWwindow *_window;

        void release();
    };
}
