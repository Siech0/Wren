#ifndef WREN_FOUNDATION_UTILITY_ENUM_UTILS_HPP
#define WREN_FOUNDATION_UTILITY_ENUM_UTILS_HPP

#include <type_traits>

namespace wren::foundation {


template<class E>
concept Enum = std::is_enum_v<E>;

template<Enum E>
constexpr auto underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

template <Enum E>
struct enable_flags : std::false_type {};
// Define `template<> struct enable_flags<E> : std::true_type {}` to enable bitwise operators for enum E.

template<Enum E> requires enable_flags<E>::value
constexpr E operator|(E a, E b) { return static_cast<E>(underlying(a) | underlying(b)); }

template<Enum E> requires enable_flags<E>::value
constexpr E& operator|=(E& a, E b) { a = a | b; return a; }

template<Enum E> requires enable_flags<E>::value
constexpr E operator&(E a, E b) { return static_cast<E>(underlying(a) & underlying(b)); }

template<Enum E> requires enable_flags<E>::value
constexpr E& operator&=(E& a, E b) { a = a & b; return a; }

} // namespace wren::foundation

#endif // WREN_FOUNDATION_UTILITY_ENUM_UTILS_HPP
