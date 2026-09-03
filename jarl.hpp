#ifndef JARL_HPP
#define JARL_HPP

#include <array>
#include <tuple>
#include <string_view>

#define JARL_STATIC_INDEX(INDEX) std::integral_constant<std::size_t, INDEX>
#define JARL_STATIC_STRING(STRING) decltype([]<std::size_t... Is>(std::index_sequence<Is...>) { return jarl::static_string<#STRING[Is]...>{}; }(std::make_index_sequence<sizeof(#STRING)>{}))

#define JARL_DEFINE_CURRENT_INDEX(NAME) NAME##_index(jarl::tag<this_type>);
#define JARL_CURRENT_INDEX(NAME) decltype(NAME##_index(jarl::tag<this_type>{}))
#define JARL_DECLARE_NEXT_INDEX(NAME) JARL_STATIC_INDEX(JARL_CURRENT_INDEX(NAME){} + 1)

#define JARL_MACRO(...) __VA_ARGS__

#define JARL_FIELD(NAME, TYPE, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
\
TYPE NAME{ __VA_OPT__(__VA_ARGS__) }; \
\
friend constexpr TYPE get_type(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)); \
friend constexpr auto get_type_name(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return #TYPE; } \
friend constexpr auto get_name(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr auto get_member(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return &this_type::NAME; } \
friend constexpr auto get_index(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return JARL_CURRENT_INDEX(NAME){}; } \
friend constexpr auto get_static_name(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return JARL_STATIC_STRING(NAME){}; } \
friend constexpr auto get_index(jarl::tag<this_type>, JARL_STATIC_STRING(NAME)) noexcept { return JARL_CURRENT_INDEX(NAME){}; } \
\
friend constexpr TYPE& get_value(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME), this_type& object) noexcept { return object.NAME; } \
friend constexpr const TYPE& get_value(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME), const this_type& object) noexcept { return object.NAME; } \
friend constexpr TYPE&& get_value(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME), this_type&& object) noexcept { return std::move(object).NAME; } \
\
friend constexpr JARL_DECLARE_NEXT_INDEX(NAME)

#define JARL_DEFINE_STRUCT(STRUCT_NAME, ...) \
using this_type = STRUCT_NAME; \
friend constexpr void is_meta_struct(jarl::tag<this_type>) noexcept {} \
friend constexpr auto get_name(jarl::tag<this_type>) noexcept { return #STRUCT_NAME; } \
friend constexpr JARL_STATIC_INDEX(0) \
__VA_OPT__(__VA_ARGS__) \
get_size(jarl::tag<this_type>) noexcept { return {}; } \

#define JARL_STRUCT(STRUCT_NAME, ...) \
struct STRUCT_NAME { JARL_DEFINE_STRUCT(STRUCT_NAME, __VA_OPT__(__VA_ARGS__)) }

namespace jarl
{

template <char... Cs>
struct static_string
{
    static constexpr const auto size = sizeof...(Cs);
    static constexpr const char data[size] = { Cs... };
};

template <typename>
struct tag {};

template <typename T>
concept meta_struct = requires{ is_meta_struct(tag<std::remove_cvref_t<T>>{}); };

namespace impl
{

template <meta_struct T, std::size_t I>
struct field
{
    using struct_type = T;
    using type = decltype(get_type(tag<T>{}, JARL_STATIC_INDEX(I){}));
    static constexpr auto type_name() noexcept { return get_type_name(tag<T>{}, JARL_STATIC_INDEX(I){}); }
    static constexpr auto name() noexcept { return get_name(tag<T>{}, JARL_STATIC_INDEX(I){}); }
    static constexpr auto member() noexcept { return get_member(tag<T>{}, JARL_STATIC_INDEX(I){}); }
    static constexpr auto index() noexcept { return get_index(tag<T>{}, JARL_STATIC_INDEX(I){}); }
    static constexpr auto static_name() noexcept { return get_static_name(tag<T>{}, JARL_STATIC_INDEX(I){}); }
};

template <typename T, std::size_t... Is>
class meta
{
public:
    using type = T;

    static constexpr auto name() noexcept { return get_name(tag<T>{}); }
    static constexpr auto size() noexcept { return get_size(tag<T>{}); }

    static constexpr const auto& field_type_names() noexcept { return _field_type_names; }
    static constexpr const auto& field_names() noexcept { return _field_names; }

private:
    static constexpr std::array<std::string_view, sizeof...(Is)> _field_type_names = { field<T, Is>::type_name()... };
    static constexpr std::array<std::string_view, sizeof...(Is)> _field_names = { field<T, Is>::name()... };
};

template <typename T, typename>
struct make_meta{};

template <typename T, std::size_t... Is>
struct make_meta<T, std::index_sequence<Is...>> { using type = meta<T, Is...>; };

template <typename T, std::size_t N>
using make_meta_t = typename make_meta<T, decltype(std::make_index_sequence<N>{})>::type;

}

template <meta_struct T>
using meta = impl::make_meta_t<std::remove_cvref_t<T>, get_size(tag<std::remove_cvref_t<T>>{})>;

template <meta_struct T, std::size_t I>
using field = impl::field<std::remove_cvref_t<T>, I>;

template <meta_struct T, std::size_t I>
using field_type_t = typename field<T, I>::type;

template <std::size_t I, meta_struct T>
constexpr decltype(auto) get(T&& object) noexcept { return get_value(tag<std::remove_cvref_t<T>>{}, JARL_STATIC_INDEX(I){}, std::forward<T>(object)); }

template <std::size_t I, meta_struct T>
constexpr decltype(auto) get(T&& object, field<T, I>) noexcept { return get<I>(std::forward<T>(object)); }

template <meta_struct T, char... Cs>
constexpr decltype(auto) get(T&& object, static_string<Cs...> s = {}) noexcept
{
    constexpr auto I = get_index(tag<std::remove_cvref_t<T>>{}, s);
    return get<I>(std::forward<T>(object));
}

template <meta_struct T, typename V>
constexpr auto visit(V&& visitor, T&& object, std::size_t index)
{
    return [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        using R = std::common_type_t<decltype(std::forward<V>(visitor)(get<Is>(std::forward<T>(object))))...>;
        using Visitor = R(*)(V&&, T&&);
        static constexpr Visitor visitors[]
        {
            [](V&& visitor, T&& object) -> R
            {
                return std::forward<V>(visitor)(get<Is>(std::forward<T>(object)));
            }...
        };
        return visitors[index](std::forward<V>(visitor), std::forward<T>(object));
    }(std::make_index_sequence<meta<T>::size()>{});
}

template <meta_struct T, typename V>
constexpr auto visit(V&& visitor, T&& object, std::string_view name)
{
    const auto& field_names = meta<T>::field_names();
    for (std::size_t index = 0; index < std::size(field_names); ++index)
        if (field_names[index] == name)
            return visit(std::forward<V>(visitor), std::forward<T>(object), index);
}

struct void_t {};

namespace impl
{

template <std::size_t I, meta_struct T, typename V>
constexpr auto for_each_helper(V&& visitor, T&& object)
{
    if constexpr (std::is_void_v<decltype(std::forward<V>(visitor)(get<I>(std::forward<T>(object))))>)
    { 
        std::forward<V>(visitor)(get<I>(std::forward<T>(object)));
        return void_t{};
    }
    else
        return std::forward<V>(visitor)(get<I>(std::forward<T>(object)));
}

template <std::size_t I, meta_struct T, typename V>
constexpr auto for_each_field_helper(V&& visitor, field<T, I> field_)
{
    if constexpr (std::is_void_v<decltype(std::forward<V>(visitor)(field_))>)
    { 
        std::forward<V>(visitor)(field_);
        return void_t{};
    }
    else
        return std::forward<V>(visitor)(field_);
}

}

template <meta_struct T, typename V, std::size_t... Is>
constexpr auto for_each(V&& visitor, T&& object, std::index_sequence<Is...>)
{
    return std::make_tuple(impl::for_each_helper<Is>(std::forward<V>(visitor), std::forward<T>(object))...);
}

template <meta_struct T, typename V>
constexpr auto for_each(V&& visitor, T&& object)
{
    return for_each(std::forward<V>(visitor), std::forward<T>(object), std::make_index_sequence<meta<T>::size()>{});
}

template <meta_struct T, typename V, std::size_t... Is>
constexpr auto for_each_field(V&& visitor, std::index_sequence<Is...>)
{
    return std::make_tuple(impl::for_each_field_helper<Is, T>(std::forward<V>(visitor), field<T, Is>{})...);
}

template <meta_struct T, typename V>
constexpr auto for_each_field(V&& visitor)
{
    return for_each_field<T>(std::forward<V>(visitor), std::make_index_sequence<meta<T>::size()>{});
}

}

#endif //JARL_HPP