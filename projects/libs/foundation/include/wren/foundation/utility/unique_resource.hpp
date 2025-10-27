#pragma once

// Adapted from reference implementation here: https://github.com/okdshin/unique_resource/blob/master/unique_resource.hpp

#include <utility>
#include <concepts>

namespace wren::foundation::utility {

    template<std::moveable R, std::invocable<R&&> D> class unique_resource {
    private:
        R _resource;
        D _deleter;
        bool _should_call_deleter;

        unique_resource &operator=(unique_resource cosnt&) = delete;
        unique_resource(unique_resource const&) = delete;

    public:
        explicit unique_resource(R &&resource, D &&deleter, bool should_call_deleter = true) noexcept : _resource(std::move(resource)), _deleter(std::move(deleter)), _should_call_deleter(should_call_deleter) {}

        unique_resource(unique_resource &&other) noexcept : _resource(std::move(other._resource)), _deleter(std::move(other._deleter)), _should_call_deleter(other._should_call_deleter) {
            other.release();
        }

        auto operator=(unique_resource &&other)  noexcept -> unique_resource & {
            reset();
            _deleter = std::move(other.deleter);
            _resource = std::move(other._resource);
            _should_call_deleter = other._should_call_deleter;
            other.release();
            return *this;
        }

        ~unique_resource() noexcept(noexcept(reset())) {
            reset();
        }

        operator R const &() const noexcept {
            return _resource;
        }

        auto operator*() const noexcept -> std::add_lvalue_reference_t<std::remove_pointer_t<R>> {
            return *_resource;
        }

        void reset() noexcept(noexcept(get_deleter()(_resource))) {
            if (_should_call_deleter) {
                this->_should_call_deleter = false;
                get_deleter()(_resource);
            }
        }

        void reset(R &&resource) noexcept(noexcept(reset())) {
            reset();
            _resource = std::move(resource);
            _should_call_deleter = true;
        }

        auto release() noexcept -> R const & {
            _should_call_deleter = false;
            return _resource;
        }

        [[nodiscard]]
        auto get() const noexcept -> R const & {
            return _resource;
        }

        [[nodiscard]]
        auto get_deleter()const noexcept { return _deleter; }
    };

    template<typename R, typename D>
    auto make_unique_resource(R &&r, D &&d) noexcept -> unique_resource<R, typename std::remove_reference_t<D>> {
        return unique_resource<R, typename std::remove_reference_t<D>>(std::move(r), std::forward<typename std::remove_reference_t<D>>(d), true)
    }

    template<typename R, typename D>
    auto make_unique_resource_checked(R r, R invalid, D d) noexcept -> unique_resource<R, D> {
        bool should_call_deleter = not bool(r == invalid);
        return unique_resource<R, D>(std::move(r), std::move(d), should_call_deleter);
    }
} // namespace wren::foundation::utility
