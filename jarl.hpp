#ifndef JARL_HPP
#define JARL_HPP

#include <array>
#include <tuple>

#define JARL_INDEX(INDEX) std::integral_constant<std::size_t, INDEX>

#define JARL_DEFINE_CURRENT_INDEX(NAME) NAME ##_index(jarl::tag<this_type>);
#define JARL_CURRENT_INDEX(NAME) decltype(NAME ##_index(jarl::tag<this_type>{}))
#define JARL_DECLARE_NEXT_INDEX(NAME) JARL_INDEX(JARL_CURRENT_INDEX(NAME){} + 1)

#define JARL_TYPE(...) __VA_ARGS__

#ifdef JARL_SHORTCUTS
#define JS JARL_STRUCT
#define JF JARL_FIELD
#define JT JARL_TYPE
#endif

#define JARL_FIELD(TYPE, NAME, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
TYPE NAME{ __VA_OPT__(__VA_ARGS__) }; \
friend constexpr const char* get_type_name(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return #TYPE; } \
friend constexpr const char* get_name(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr const TYPE& get_value(const this_type& object, JARL_CURRENT_INDEX(NAME)) noexcept { return object.NAME; } \
friend constexpr TYPE& get_value(this_type& object, JARL_CURRENT_INDEX(NAME)) noexcept { return object.NAME; } \
friend constexpr TYPE&& get_value(this_type&& object, JARL_CURRENT_INDEX(NAME)) noexcept { return std::move(object.NAME); } \
friend constexpr TYPE this_type::* get_member(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)) noexcept { return &this_type::NAME; } \
friend constexpr TYPE get_type(jarl::tag<this_type>, JARL_CURRENT_INDEX(NAME)); \
friend constexpr JARL_DECLARE_NEXT_INDEX(NAME)

#define JARL_DEFINE_STRUCT(STRUCT_NAME, ...) \
using this_type = STRUCT_NAME; \
friend constexpr void is_meta_struct(jarl::tag<this_type>) noexcept {} \
friend constexpr const char* get_name(jarl::tag<this_type>) noexcept { return #STRUCT_NAME; } \
friend constexpr JARL_INDEX(0) \
__VA_ARGS__ \
get_size(jarl::tag<this_type>) noexcept { return {}; } \

#define JARL_STRUCT(STRUCT_NAME, STRUCT_BODY) \
struct STRUCT_NAME { JARL_DEFINE_STRUCT(STRUCT_NAME, STRUCT_BODY) }

namespace jarl
{

template <typename>
struct tag {};

template <typename T>
concept meta_struct = requires{ is_meta_struct(tag<std::decay_t<T>>{}); };

template <meta_struct T, std::size_t I>
struct field
{
    using struct_type = T;
    using type = decltype(get_type(tag<T>{}, JARL_INDEX(I){}));

    static constexpr std::size_t index() noexcept { return I; }
    static constexpr const char* type_name() noexcept { return get_type_name(tag<T>{}, JARL_INDEX(I){}); }
    static constexpr const char* name() noexcept { return get_name(tag<T>{}, JARL_INDEX(I){}); }
    static constexpr type T::* member() noexcept { return get_member(tag<T>{}, JARL_INDEX(I){}); }
    constexpr type T::* operator*() const noexcept { return member(); }
};

namespace impl
{

template <typename T, std::size_t... Is>
class meta
{
public:
    using type = T;

    static constexpr const char* name() noexcept { return get_name(tag<T>{}); }
    static constexpr const std::array<const char* const, sizeof...(Is)>& field_type_names() noexcept { return _field_type_names; }
    static constexpr const std::array<const char* const, sizeof...(Is)>& field_names() noexcept { return _field_names; }
    static constexpr std::size_t size() noexcept { return get_size(tag<T>{}); }

private:
    static constexpr std::array<const char* const, sizeof...(Is)> _field_type_names = { field<T, Is>::type_name()... };
    static constexpr std::array<const char* const, sizeof...(Is)> _field_names = { field<T, Is>::name()... };
};

template <typename T, typename>
struct make_meta{};

template <typename T, std::size_t... Is>
struct make_meta<T, std::index_sequence<Is...>> { using type = meta<T, Is...>; };

template <typename T, std::size_t N>
using make_meta_t = typename make_meta<T, decltype(std::make_index_sequence<N>{})>::type;

}

template <meta_struct T>
using meta = impl::make_meta_t<std::decay_t<T>, get_size(tag<std::decay_t<T>>{})>;

template <meta_struct T, std::size_t I>
using field_type_t = typename field<T, I>::type;

template <std::size_t I, meta_struct T>
constexpr decltype(auto) get(T&& object) noexcept { return get_value(std::forward<T>(object), JARL_INDEX(I){}); }

template <meta_struct T, typename... Args> requires std::is_constructible_v<T, Args...>
constexpr T from_tuple(const std::tuple<Args...>& tup)
{
    return [&tup]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return T{std::forward<Args>(std::get<Is>(tup))...};
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}

template <meta_struct T, typename... Args> requires std::is_constructible_v<T, Args...>
constexpr T from_tuple(std::tuple<Args...>& tup)
{
    return [&tup]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return T{std::forward<Args>(std::get<Is>(tup))...};
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}

template <meta_struct T, typename... Args> requires std::is_constructible_v<T, Args...>
constexpr T from_tuple(std::tuple<Args...>&& tup)
{
    return [tup = std::move(tup)]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return T{std::move(std::get<Is>(tup))...};
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}

template <meta_struct T>
constexpr auto make_tuple(T&& object) noexcept
{
    return [object = std::forward<T>(object)]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return std::make_tuple(get<Is>(std::forward<T>(object))...);
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}


template <meta_struct T>
constexpr auto tie(T& object) noexcept
{
    return [&object]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return std::tie(get<Is>(object)...);
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}

template <meta_struct T>
constexpr auto forward_as_tuple(T&& object) noexcept
{
    return [object = std::forward<T>(object)]<std::size_t... Is>(std::index_sequence<Is...>) mutable
    {
        return std::forward_as_tuple(get<Is>(std::forward<T>(object))...);
    }
    (std::make_index_sequence<meta<std::remove_cvref_t<T>>::size()>{});
}

}

#endif //JARL_HPP