#ifndef JARL_ENUM_HPP
#define JARL_ENUM_HPP

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <array>
#include <string_view>
#include <optional>

#define JARL_INTEGRAL_CONSTANT(INDEX) std::integral_constant<jarl::size_type, INDEX>

#define JARL_ENUMER(NAME, ...) \
NAME ##_index{}; \
static constexpr auto NAME = create_basic_enum(basic_enum<enum_type>{}, NAME ##_index); \
friend constexpr enum_type get_enum(enum_type, JARL_INTEGRAL_CONSTANT(NAME ##_index)) noexcept { return NAME; } \
friend constexpr jarl::name_type get_name(enum_type, JARL_INTEGRAL_CONSTANT(NAME ##_index)) noexcept { return #NAME; } \
friend constexpr value_type get_value(enum_type, JARL_INTEGRAL_CONSTANT(NAME ##_index)) noexcept { return (jarl::detail::assign_value<enum_type, NAME ##_index>() __VA_OPT__(, __VA_ARGS__)); } \
static constexpr JARL_INTEGRAL_CONSTANT(NAME ##_index + 1) 

#define JARL_ENUM(ENUM_NAME, ENUM_BODY) \
struct ENUM_NAME : public jarl::detail::basic_enum<ENUM_NAME> \
{ \
    using basic_enum<ENUM_NAME>::basic_enum; \
    constexpr ENUM_NAME(basic_enum<ENUM_NAME> e) noexcept : basic_enum<ENUM_NAME>{e} {} \
    friend constexpr jarl::name_type get_name(enum_type) noexcept { return #ENUM_NAME; } \
    static constexpr JARL_INTEGRAL_CONSTANT(0) \
    ENUM_BODY \
    _size{}; \
    static_assert(_size > 0, "cannot define an empty enum"); \
}

namespace jarl
{

using size_type = std::size_t;
using name_type = std::string_view;

namespace detail
{
struct enum_tag {};

template <typename E, size_type I>
constexpr typename E::value_type assign_value() noexcept
{
    if constexpr (I > 0)
        return get_value(E{}, JARL_INTEGRAL_CONSTANT(I - 1){}) + 1;
    return 0;
}

template <typename E>
struct enum_impl
{
    enum class enum_t {};
};

template <typename E>
using enum_t = typename enum_impl<E>::enum_t;

template <typename E, size_type... Is>
class enum_traits
{
public:
    using enum_type = E;
    using value_type = typename E::value_type;

    static constexpr name_type name() noexcept { return get_name(E{}); }
    static constexpr const auto& array() noexcept { return _array; }
    static constexpr const auto& names() noexcept { return _names; }
    static constexpr const auto& values() noexcept { return _values; }
    static constexpr size_type size() noexcept { return E::enum_type::_size; }

private:
    template <typename T>
    using array_t = std::array<const T, sizeof...(Is)>;

    static constexpr array_t<E> _array = { get_enum(E{}, JARL_INTEGRAL_CONSTANT(Is){})... };
    static constexpr array_t<name_type> _names = { get_name(E{}, JARL_INTEGRAL_CONSTANT(Is){})... };
    static constexpr array_t<value_type> _values = { get_value(E{}, JARL_INTEGRAL_CONSTANT(Is){})... };
};

template <typename E, size_type I, size_type... Is>
struct make_enum_traits
{
    using type = typename make_enum_traits<E, I - 1, I, Is...>::type;
};

template <typename E, size_type... Is>
struct make_enum_traits<E, 0, Is...>
{
    using type = enum_traits<E, 0, Is...>;
};

template <typename E, size_type... Is>
using make_enum_traits_t = typename make_enum_traits<E, Is...>::type;

template <typename E>
using enum_type_t = typename E::enum_type;

}

template <typename T>
struct is_enum : std::bool_constant<std::is_base_of_v<detail::enum_tag, T>> {};

template <typename T>
constexpr bool is_enum_v = is_enum<T>::value;

template <typename T>
concept meta_enum = is_enum_v<T>;

template <meta_enum E>
using enum_traits = detail::make_enum_traits_t<detail::enum_type_t<E>, detail::enum_type_t<E>::_size - 1>;

template <meta_enum E>
constexpr name_type enum_name() noexcept { return enum_traits<E>::name(); }

template <meta_enum E>
constexpr const auto& enum_array() noexcept { return enum_traits<E>::array(); }

template <meta_enum E>
constexpr const auto& enum_names() noexcept { return enum_traits<E>::names(); }

template <meta_enum E>
constexpr const auto& enum_values() noexcept { return enum_traits<E>::values(); }

template <meta_enum E>
constexpr size_type enum_size() noexcept { return enum_traits<E>::size(); }

template <meta_enum E>
using value_type_t = typename enum_traits<E>::value_type;

struct bad_enum_cast : std::bad_cast
{
    const char* what() const noexcept override { return "bad enum_cast"; }
};

template <meta_enum E>
constexpr E make_enum(size_type index)
{
    if (index < enum_size<E>())
        return enum_array<E>()[index];
    throw std::out_of_range("enum index is out of range");
}

template <meta_enum E>
constexpr std::optional<E> make_optional_enum(size_type index)
{
    if (index < enum_size<E>())
        return enum_array<E>()[index];
    return {};
}

// name -> enum
template <meta_enum E>
constexpr E enum_cast(std::string_view name)
{
    constexpr const auto& names = enum_traits<E>::names();
    constexpr size_type size = enum_traits<E>::size();
    for (size_type i = 0; i < size; ++i)
        if (names[i] == name)
            return make_enum<E>(i);
    throw bad_enum_cast{};
}

// name -> optional enum
template <meta_enum E>
constexpr std::optional<E> enum_optional_cast(std::string_view name)
{
    constexpr const auto& names = enum_traits<E>::names();
    constexpr size_type size = enum_traits<E>::size();
    for (size_type i = 0; i < size; ++i)
        if (names[i] == name)
            return make_enum<E>(i);
    return {};
}

// value -> enum
template <meta_enum E>
constexpr E enum_cast(value_type_t<E> value)
{
    constexpr const auto& values = enum_traits<E>::values();
    constexpr size_type size = enum_traits<E>::size();
    for (size_type i = 0; i < size; ++i)
        if (values[i] == value)
            return make_enum<E>(i);
    throw bad_enum_cast{};
}

// value -> optional enum
template <meta_enum E>
constexpr std::optional<E> enum_optional_cast(value_type_t<E> value)
{
    constexpr const auto& values = enum_traits<E>::values();
    constexpr size_type size = enum_traits<E>::size();
    for (size_type i = 0; i < size; ++i)
        if (values[i] == value)
            return make_enum<E>(i);
    return {};
}

// enum -> string | enum -> value
template <typename T, meta_enum E>
constexpr T enum_cast(E e) noexcept
{
    constexpr bool is_name = std::is_convertible_v<T, std::string_view>;
    constexpr bool is_value = std::is_same_v<T, value_type_t<E>>;
    constexpr bool is_enum = std::is_convertible_v<T, E>;

    static_assert(is_name || is_value || is_enum, "bad enum_cast");

    if constexpr (is_name)
        return nameof(e);
    if constexpr (is_value)
        return valueof(e);
    if constexpr (is_enum)
        return e;
}

namespace detail
{

template <typename E>
class basic_enum : public enum_tag
{
public:
    using enum_type = E;
    using value_type = std::underlying_type_t<enum_t<E>>;

public:
    constexpr basic_enum() noexcept = default;
    explicit constexpr basic_enum(std::string_view name) noexcept { *this = enum_cast<E>(name); }

    constexpr operator detail::enum_t<E>() const noexcept { return static_cast<enum_t<E>>(valueof(*this)); }

    template <std::convertible_to<value_type> T>
    explicit constexpr operator T() const noexcept { return static_cast<T>(valueof(*this)); }

    friend constexpr auto indexof(E e) noexcept { return e._index; }
    friend constexpr auto nameof(E e) noexcept { return enum_names<E>()[indexof(e)]; }
    friend constexpr auto valueof(E e) noexcept { return enum_values<E>()[indexof(e)]; }

protected:
    explicit constexpr basic_enum(size_type index) noexcept : _index{index} {}
    size_type _index = 0;

    friend constexpr auto create_basic_enum(basic_enum e, size_type index) noexcept { return basic_enum{index}; }
};

}

}

#endif //JARL_ENUM_HPP