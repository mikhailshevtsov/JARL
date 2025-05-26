#ifndef JARL_STRUCT_HPP
#define JARL_STRUCT_HPP

#include "detail.hpp"

#include <stdexcept>
#include <array>
#include <variant>

#define JARL_FIELD(TYPE, NAME, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
TYPE NAME{ __VA_OPT__(__VA_ARGS__) }; \
friend constexpr JARL_NAME_TYPE get_type_hint(JARL_ADL, JARL_CURRENT_INDEX(NAME)) noexcept { return #TYPE; } \
friend constexpr JARL_NAME_TYPE get_name(JARL_ADL, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr const auto& get_field(const JARL_THIS_TYPE& s, JARL_CURRENT_INDEX(NAME)) noexcept { return s.NAME; } \
friend constexpr auto& get_field(JARL_THIS_TYPE& s, JARL_CURRENT_INDEX(NAME)) noexcept { return s.NAME; } \
friend constexpr TYPE get_type(JARL_ADL, JARL_CURRENT_INDEX(NAME)); \
friend constexpr JARL_NEXT_INDEX(NAME)

#define JARL_DEFINE_STRUCT(STRUCT_NAME, ...) \
using JARL_THIS_TYPE = STRUCT_NAME; \
friend constexpr void is_struct(JARL_ADL) noexcept {} \
friend constexpr JARL_NAME_TYPE get_name(JARL_ADL) noexcept { return #STRUCT_NAME; } \
friend constexpr JARL_INDEX(0) \
__VA_ARGS__ \
get_size(JARL_ADL) noexcept { return {}; } \

#define JARL_STRUCT(STRUCT_NAME, STRUCT_BODY) \
struct STRUCT_NAME \
{ \
    JARL_DEFINE_STRUCT(STRUCT_NAME, STRUCT_BODY) \
}

#define JF JARL_FIELD

namespace jarl
{

namespace detail
{

template <typename S, size_type... Is>
class struct_traits
{
public:
    using struct_type = S;

    template <size_type I>
    using field_type = std::tuple_element<I, std::tuple<decltype(get_type(adl<S>{}, JARL_INDEX(Is){}))...>>;

    static constexpr name_type name() noexcept { return get_name(adl<S>{}); }
    static constexpr const auto& type_hints() noexcept { return _type_hints; }
    static constexpr const auto& names() noexcept { return _names; }
    static constexpr size_type size() noexcept { return get_size(adl<S>{}); }

    static constexpr auto make_tuple(const S& s) noexcept { return std::make_tuple(std::cref(get_field(s, JARL_INDEX(Is){}))...); }
    static constexpr auto make_tuple(S& s) noexcept { return std::make_tuple(std::ref(get_field(s, JARL_INDEX(Is){}))...); }
    static constexpr auto make_tuple(S&& s) noexcept { return std::make_tuple(std::move(get_field(s, JARL_INDEX(Is){}))...); }

    static constexpr auto get(const S& s, size_type index)
    {
        assert_index(index);
        return _const_getters[index](s);
    }

    static constexpr auto get(S& s, size_type index)
    {
        assert_index(index);
        return _getters[index](s);
    }

    static constexpr auto get(S&& s, size_type index)
    {
        assert_index(index);
        return _move_getters[index](s);
    }

private:
    static constexpr void assert_index(size_type index)
    {
        if (index >= size())
            throw std::out_of_range("struct index is out range");
    }

private:
    template <typename T>
    using array_t = std::array<const T, sizeof...(Is)>;

    static constexpr array_t<name_type> _type_hints = { get_type_hint(adl<S>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<name_type> _names = { get_name(adl<S>{}, JARL_INDEX(Is){})... };

    using const_variant_t = std::variant<std::reference_wrapper<const decltype(get_type(adl<S>{}, JARL_INDEX(Is){}))>...>;
    using variant_t = std::variant<std::reference_wrapper<decltype(get_type(adl<S>{}, JARL_INDEX(Is){}))>...>;
    using move_variant_t = std::variant<decltype(get_type(adl<S>{}, JARL_INDEX(Is){}))...>;

    static constexpr array_t<const_variant_t(*)(const S&)> _const_getters = { [](const S& s) -> const_variant_t { return std::get<Is>(make_tuple(s)); }... };
    static constexpr array_t<variant_t(*)(S&)> _getters = { [](S& s) -> variant_t { return std::get<Is>(make_tuple(s)); }... };
    static constexpr array_t<move_variant_t(*)(S&&)> _move_getters = { [](S&& s) -> move_variant_t { return std::get<Is>(make_tuple(std::move(s))); }... };
};

template <typename S, typename>
struct make_struct_traits{};

template <typename S, size_type... Is>
struct make_struct_traits<S, std::index_sequence<Is...>> { using type = struct_traits<S, Is...>; };

template <typename S, std::size_t N>
using make_struct_traits_t = typename make_struct_traits<S, decltype(std::make_index_sequence<N>{})>::type;

}

template <typename T>
concept meta_struct = requires{ is_struct(jarl::detail::adl<std::decay_t<T>>{}); };

template <meta_struct S>
using struct_traits = detail::make_struct_traits_t<std::decay_t<S>, get_size(detail::adl<std::decay_t<S>>{})>;

template <size_type I, meta_struct S>
using field_type_t = typename struct_traits<S>::template field_type<I>;

template <meta_struct S>
constexpr name_type struct_name() noexcept { return struct_traits<S>::name(); }

template <meta_struct S>
constexpr const auto& struct_type_hints() noexcept { return struct_traits<S>::type_hints(); }

template <meta_struct S>
constexpr const auto& struct_names() noexcept { return struct_traits<S>::names(); }

template <meta_struct S>
constexpr size_type struct_size() noexcept { return struct_traits<S>::size(); }

template <meta_struct S>
constexpr auto make_tuple(S&& s) { return struct_traits<S>::make_tuple(std::forward<S>(s)); }

template <size_type I, meta_struct S>
constexpr decltype(auto) get(S&& s) noexcept { return std::get<I>(make_tuple(std::forward<S>(s))); }

template <meta_struct S>
constexpr auto get(S&& s, size_type index) { return struct_traits<S>::get(std::forward<S>(s), index); }

template <meta_struct S>
constexpr auto get(S&& s, name_type name)
{
    size_type index = 0;
    for (; index < struct_size<S>(); ++index)
        if (name == struct_names<S>()[index])
            break;
    return get(std::forward<S>(s), index);
}

template <typename T, typename... Ts>
constexpr T as(std::variant<std::reference_wrapper<Ts>...> var) { return std::get<std::reference_wrapper<std::remove_reference_t<T>>>(var); }

}

#endif //JARL_STRUCT_HPP