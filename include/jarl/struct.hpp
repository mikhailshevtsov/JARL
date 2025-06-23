#ifndef JARL_STRUCT_HPP
#define JARL_STRUCT_HPP

#include "detail.hpp"

#include <stdexcept>
#include <array>
#include <variant>

#define JARL_FIELD(TYPE, NAME, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
TYPE NAME{ __VA_OPT__(__VA_ARGS__) }; \
friend constexpr auto get_type_hint(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return #TYPE; } \
friend constexpr auto get_name(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr const auto& get_field(const this_type& s, JARL_CURRENT_INDEX(NAME)) noexcept { return s.NAME; } \
friend constexpr auto& get_field(this_type& s, JARL_CURRENT_INDEX(NAME)) noexcept { return s.NAME; } \
friend constexpr TYPE get_type(JARL_TAG, JARL_CURRENT_INDEX(NAME)); \
friend constexpr auto get_full_name(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return get_static_name(JARL_TAG{}) + JARL_STATIC_STRING(::NAME); } \
friend constexpr JARL_DECLARE_NEXT_INDEX(NAME)

#define JARL_DEFINE_STRUCT(STRUCT_NAME, ...) \
using this_type = STRUCT_NAME; \
friend constexpr void is_struct(JARL_TAG) noexcept {} \
friend constexpr auto get_name(JARL_TAG) noexcept { return #STRUCT_NAME; } \
friend constexpr auto get_static_name(JARL_TAG) noexcept { return JARL_STATIC_STRING(STRUCT_NAME); } \
friend constexpr JARL_INDEX(0) \
__VA_ARGS__ \
get_size(JARL_TAG) noexcept { return {}; } \

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

template <typename StructT, size_type... Is>
class struct_traits
{
public:
    using struct_type = StructT;

    template <size_type I>
    using field_type = std::tuple_element<I, std::tuple<decltype(get_type(tag<StructT>{}, JARL_INDEX(Is){}))...>>;

    static constexpr auto name() noexcept { return get_name(tag<StructT>{}); }
    static constexpr const auto& type_hints() noexcept { return _type_hints; }
    static constexpr const auto& names() noexcept { return _names; }
    static constexpr const auto& full_names() noexcept { return _full_names; }
    static constexpr auto size() noexcept { return get_size(tag<StructT>{}); }

    static constexpr auto make_tuple(const StructT& s) noexcept { return std::make_tuple(std::cref(get_field(s, JARL_INDEX(Is){}))...); }
    static constexpr auto make_tuple(StructT& s) noexcept { return std::make_tuple(std::ref(get_field(s, JARL_INDEX(Is){}))...); }
    static constexpr auto make_tuple(StructT&& s) noexcept { return std::make_tuple(std::move(get_field(s, JARL_INDEX(Is){}))...); }

    static constexpr auto get(const StructT& s, size_type index)
    {
        assert_index(index);
        return _const_getters[index](s);
    }

    static constexpr auto get(StructT& s, size_type index)
    {
        assert_index(index);
        return _getters[index](s);
    }

    static constexpr auto get(StructT&& s, size_type index)
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

    static constexpr array_t<const char*> _type_hints = { get_type_hint(tag<StructT>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<const char*> _names = { get_name(tag<StructT>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<const char*> _full_names = { get_full_name(tag<StructT>{}, JARL_INDEX(Is){})... };

    using const_variant_t = std::variant<std::reference_wrapper<const decltype(get_type(tag<StructT>{}, JARL_INDEX(Is){}))>...>;
    using variant_t = std::variant<std::reference_wrapper<decltype(get_type(tag<StructT>{}, JARL_INDEX(Is){}))>...>;
    using move_variant_t = std::variant<decltype(get_type(tag<StructT>{}, JARL_INDEX(Is){}))...>;

    static constexpr array_t<const_variant_t(*)(const StructT&)> _const_getters = { [](const StructT& s) -> const_variant_t { return std::get<Is>(make_tuple(s)); }... };
    static constexpr array_t<variant_t(*)(StructT&)> _getters = { [](StructT& s) -> variant_t { return std::get<Is>(make_tuple(s)); }... };
    static constexpr array_t<move_variant_t(*)(StructT&&)> _move_getters = { [](StructT&& s) -> move_variant_t { return std::get<Is>(make_tuple(std::move(s))); }... };
};

template <typename StructT, typename>
struct make_struct_traits{};

template <typename StructT, size_type... Is>
struct make_struct_traits<StructT, std::index_sequence<Is...>> { using type = struct_traits<StructT, Is...>; };

template <typename StructT, std::size_t N>
using make_struct_traits_t = typename make_struct_traits<StructT, decltype(std::make_index_sequence<N>{})>::type;

}

template <typename T>
concept meta_struct = requires{ is_struct(jarl::tag<std::decay_t<T>>{}); };

template <meta_struct StructT>
using struct_traits = detail::make_struct_traits_t<std::decay_t<StructT>, get_size(tag<std::decay_t<StructT>>{})>;

template <size_type I, meta_struct StructT>
using field_type_t = typename struct_traits<StructT>::template field_type<I>;

template <meta_struct StructT>
constexpr auto struct_name() noexcept { return struct_traits<StructT>::name(); }

template <meta_struct StructT>
constexpr const auto& struct_type_hints() noexcept { return struct_traits<StructT>::type_hints(); }

template <meta_struct StructT>
constexpr const auto& struct_names() noexcept { return struct_traits<StructT>::names(); }

template <meta_struct StructT>
constexpr const auto& struct_full_names() noexcept { return struct_traits<StructT>::full_names(); }

template <meta_struct StructT>
constexpr auto struct_size() noexcept { return struct_traits<StructT>::size(); }

template <meta_struct StructT>
constexpr auto make_tuple(StructT&& s) { return struct_traits<StructT>::make_tuple(std::forward<StructT>(s)); }

template <size_type I, meta_struct StructT>
constexpr decltype(auto) get(StructT&& s) noexcept { return std::get<I>(make_tuple(std::forward<StructT>(s))); }

template <meta_struct StructT>
constexpr auto get(StructT&& s, size_type index) { return struct_traits<StructT>::get(std::forward<StructT>(s), index); }

template <meta_struct StructT>
constexpr auto get(StructT&& s, std::string_view name)
{
    size_type index = 0;
    for (; index < struct_size<StructT>(); ++index)
        if (name == struct_names<StructT>()[index])
            break;
    return get(std::forward<StructT>(s), index);
}

template <typename T, typename... Ts>
constexpr T as(std::variant<std::reference_wrapper<Ts>...> var) { return std::get<std::reference_wrapper<std::remove_reference_t<T>>>(var); }

}

#endif //JARL_STRUCT_HPP