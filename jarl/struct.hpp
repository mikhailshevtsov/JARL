#ifndef JARL_STRUCT_HPP
#define JARL_STRUCT_HPP

#include <stdexcept>
#include <array>
#include <variant>

#define JARL_INDEX(INDEX) std::integral_constant<std::size_t, INDEX>

#define JARL_DEFINE_CURRENT_INDEX(NAME) NAME ##_index(jarl::tag<this_type>);
#define JARL_CURRENT_INDEX(NAME) decltype(NAME ##_index(jarl::tag<this_type>{}))
#define JARL_DECLARE_NEXT_INDEX(NAME) JARL_INDEX(JARL_CURRENT_INDEX(NAME){} + 1)

#define JARL_TYPE(...) __VA_ARGS__
#define JT JARL_TYPE

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
friend constexpr void is_struct(jarl::tag<this_type>) noexcept {} \
friend constexpr const char* get_name(jarl::tag<this_type>) noexcept { return #STRUCT_NAME; } \
friend constexpr JARL_INDEX(0) \
__VA_ARGS__ \
get_size(jarl::tag<this_type>) noexcept { return {}; } \

#define JARL_STRUCT(STRUCT_NAME, STRUCT_BODY) \
struct STRUCT_NAME { JARL_DEFINE_STRUCT(STRUCT_NAME, STRUCT_BODY) }

#define JF JARL_FIELD

namespace jarl
{

template <typename>
struct tag {};

template <typename T>
concept structure = requires{ is_struct(tag<std::decay_t<T>>{}); };

template <structure StructT, std::size_t I>
struct field
{
    using struct_type = StructT;
    using type = decltype(get_type(tag<StructT>{}, JARL_INDEX(I){}));

    static constexpr std::size_t index() noexcept { return I; }
    static constexpr const char* type_name() noexcept { return get_type_name(tag<StructT>{}, JARL_INDEX(I){}); }
    static constexpr const char* name() noexcept { return get_name(tag<StructT>{}, JARL_INDEX(I){}); }
    static constexpr type StructT::* member() noexcept { return get_member(tag<StructT>{}, JARL_INDEX(I){}); }
};

namespace impl
{

template <typename StructT, std::size_t... Is>
class struct_traits
{
public:
    using struct_type = StructT;

    static constexpr const char* name() noexcept { return get_name(tag<StructT>{}); }
    static constexpr const std::array<const char* const, sizeof...(Is)>& field_type_names() noexcept { return _field_type_names; }
    static constexpr const std::array<const char* const, sizeof...(Is)>& field_names() noexcept { return _field_names; }
    static constexpr std::size_t size() noexcept { return get_size(tag<StructT>{}); }

private:
    static constexpr std::array<const char* const, sizeof...(Is)> _field_type_names = { field<StructT, Is>::type_name()... };
    static constexpr std::array<const char* const, sizeof...(Is)> _field_names = { field<StructT, Is>::name()... };
};

template <typename StructT, typename>
struct make_struct_traits{};

template <typename StructT, std::size_t... Is>
struct make_struct_traits<StructT, std::index_sequence<Is...>> { using type = struct_traits<StructT, Is...>; };

template <typename StructT, std::size_t N>
using make_struct_traits_t = typename make_struct_traits<StructT, decltype(std::make_index_sequence<N>{})>::type;

}

template <structure StructT>
using struct_traits = impl::make_struct_traits_t<std::decay_t<StructT>, get_size(tag<std::decay_t<StructT>>{})>;

template <structure StructT>
using struct_type_t = typename struct_traits<StructT>::struct_type;

template <structure StructT>
constexpr const char* struct_name() noexcept { return struct_traits<StructT>::name(); }

template <structure StructT>
constexpr const auto& struct_field_type_names() noexcept { return struct_traits<StructT>::field_type_names(); }

template <structure StructT>
constexpr const auto& struct_field_names() noexcept { return struct_traits<StructT>::field_names(); }

template <structure StructT>
constexpr std::size_t struct_size() noexcept { return struct_traits<StructT>::size(); }

template <std::size_t I, structure StructT>
constexpr decltype(auto) get(StructT&& object) noexcept { return get_value(std::forward<StructT>(object), JARL_INDEX(I){}); }

}

#endif //JARL_STRUCT_HPP