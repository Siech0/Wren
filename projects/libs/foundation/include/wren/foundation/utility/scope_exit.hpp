#pragma once

#include <concepts>
#include <functional>

namespace wren::foundation::utility {
    class scope_exit {
    public:
        using function_type = std::move_only_function<void()>;

        explicit scope_exit(function_type &&f) noexcept : _f(std::move(f)), _should_call(true) {}

        scope_exit(scope_exit &&other) noexcept : _f(std::move(other._f)), _should_call(other._should_call) {
            other.release();
        }

        auto operator=(scope_exit &&other) noexcept -> scope_exit & {
            reset();
            _f = std::move(other._f);
            _should_call = other._should_call;
            other.release();
            return *this;
        }

        ~scope_exit() {
            reset();
        }

        void reset() {
            if (_should_call) {
                _should_call = false;
                _f();
            }
        }

        auto release() noexcept -> function_type&& {
            _should_call = false;
            return std::move(_f);
        }

    private:
        function_type _f;
        bool _should_call;

        scope_exit &operator=(scope_exit const&) = delete;
        scope_exit(scope_exit const&) = delete;
    };

} // namespace wren::foundation::utility
