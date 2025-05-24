#ifndef JARL_ENUM_HPP
#define JARL_ENUM_HPP

#include "detail.hpp"

#include <stdexcept>
#include <array>
#include <string_view>
#include <optional>

#define JARL_ENUMER(NAME, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
static constexpr auto NAME = create_enum(jarl::detail::adl<basic_enum<JARL_THIS_TYPE>>{}, JARL_CURRENT_INDEX(NAME){}); \
friend constexpr JARL_THIS_TYPE get_enum(JARL_ADL, JARL_CURRENT_INDEX(NAME)) noexcept { return NAME; } \
friend constexpr JARL_NAME_TYPE get_name(JARL_ADL, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr int get_value(JARL_ADL, JARL_CURRENT_INDEX(NAME)) noexcept { return (jarl::detail::assign_value(JARL_ADL{}, JARL_CURRENT_INDEX(NAME){}) __VA_OPT__(, __VA_ARGS__)); } \
friend constexpr JARL_NEXT_INDEX(NAME)

#define JARL_ENUM(ENUM_NAME, ENUM_BODY) \
struct ENUM_NAME : public jarl::detail::basic_enum<ENUM_NAME> \
{ \
    using basic_enum<ENUM_NAME>::basic_enum; \
    constexpr ENUM_NAME(basic_enum<ENUM_NAME> e) noexcept : basic_enum<ENUM_NAME>{e} {} \
    friend constexpr JARL_NAME_TYPE get_name(JARL_ADL) noexcept { return #ENUM_NAME; } \
    friend constexpr JARL_INDEX(0) \
    ENUM_BODY \
    get_size(JARL_ADL) noexcept { return {}; } \
    static_assert(decltype(get_size(JARL_ADL{})){} > 0, "cannot define an empty enum"); \
}

#define JE JARL_ENUMER

namespace jarl
{

namespace detail
{

struct enum_tag {};

template <typename E, size_type I>
constexpr int assign_value(adl<E>, JARL_INDEX(I)) noexcept
{
    if constexpr (I > 0)
        return get_value(adl<E>{}, JARL_INDEX(I - 1){}) + 1;
    return 0;
}

template <typename E>
struct enum_impl { enum class enum_t {}; };

template <typename E>
using enum_t = typename enum_impl<E>::enum_t;

template <typename E, size_type... Is>
class enum_traits
{
public:
    using enum_type = E;

    static constexpr name_type name() noexcept { return get_name(adl<E>{}); }
    static constexpr const auto& array() noexcept { return _array; }
    static constexpr const auto& names() noexcept { return _names; }
    static constexpr const auto& values() noexcept { return _values; }
    static constexpr size_type size() noexcept { return get_size(adl<E>{}); }

private:
    template <typename T>
    using array_t = std::array<const T, sizeof...(Is)>;

    static constexpr array_t<E> _array = { get_enum(adl<E>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<name_type> _names = { get_name(adl<E>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<int> _values = { get_value(adl<E>{}, JARL_INDEX(Is){})... };
};

template <typename E, typename>
struct make_enum_traits{};

template <typename E, size_type... Is>
struct make_enum_traits<E, std::index_sequence<Is...>> { using type = enum_traits<E, Is...>; };

template <typename E, std::size_t N>
using make_enum_traits_t = typename make_enum_traits<E, decltype(std::make_index_sequence<N>{})>::type;

}

template <typename T>
concept meta_enum = std::is_base_of_v<detail::enum_tag, T>;

template <meta_enum E>
using enum_traits = detail::make_enum_traits_t<typename std::decay_t<E>::JARL_THIS_TYPE, get_size(detail::adl<std::decay_t<E>>{})>;

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
constexpr std::optional<E> make_optional_enum(size_type index) noexcept
{
    if (index < enum_size<E>())
        return enum_array<E>()[index];
    return {};
}

// name -> enum
template <meta_enum E>
constexpr E enum_cast(name_type name)
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
constexpr std::optional<E> enum_optional_cast(name_type name) noexcept
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
constexpr E enum_cast(int value)
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
constexpr std::optional<E> enum_optional_cast(int value) noexcept
{
    constexpr const auto& values = enum_traits<E>::values();
    constexpr size_type size = enum_traits<E>::size();
    for (size_type i = 0; i < size; ++i)
        if (values[i] == value)
            return make_enum<E>(i);
    return {};
}

// enum -> string | enum -> value | enum -> enum
template <typename T, meta_enum E>
constexpr T enum_cast(E e) noexcept
{
    constexpr bool is_name = std::is_convertible_v<T, name_type>;
    constexpr bool is_value = std::is_same_v<T, int>;
    constexpr bool is_enum = std::is_convertible_v<T, E>;

    static_assert(is_name || is_value || is_enum, "bad enum_cast");

    if constexpr (is_name)
        return nameof(e);
    if constexpr (is_value)
        return valueof(e);
    if constexpr (is_enum)
        return e;
}

template <meta_enum E>
constexpr auto exactly(E e) noexcept { return typename E::JARL_THIS_TYPE{e}; }

namespace detail
{

template <typename E>
class basic_enum : public enum_tag
{
public:
    using JARL_THIS_TYPE = E;

    constexpr basic_enum() noexcept = default;
    explicit constexpr basic_enum(std::string_view name) { *this = enum_cast<E>(name); }

    constexpr operator enum_t<E>() const noexcept { return static_cast<enum_t<E>>(valueof(*this)); }

    template <std::convertible_to<int> T>
    explicit constexpr operator T() const noexcept { return static_cast<T>(valueof(*this)); }

    friend constexpr auto indexof(E e) noexcept { return e.basic_enum::_index; }
    friend constexpr auto nameof(E e) noexcept { return enum_names<E>()[indexof(e)]; }
    friend constexpr auto valueof(E e) noexcept { return enum_values<E>()[indexof(e)]; }

private:
    explicit constexpr basic_enum(size_type index) noexcept : _index{index} {}
    size_type _index = 0;

    friend constexpr auto create_enum(adl<basic_enum<E>>, size_type index) noexcept { return basic_enum{index}; } \
};

}

}

#endif //JARL_ENUM_HPP