#ifndef JARL_DETAIL_HPP
#define JARL_DETAIL_HPP

#include <cstddef>

#define JARL_TAG jarl::tag<this_type>
#define JARL_INDEX(INDEX) std::integral_constant<jarl::size_type, INDEX>
#define JARL_DEFINE_CURRENT_INDEX(NAME) NAME ##_index(JARL_TAG);
#define JARL_CURRENT_INDEX(NAME) decltype(NAME ##_index(JARL_TAG{}))
#define JARL_DECLARE_NEXT_INDEX(NAME) JARL_INDEX(JARL_CURRENT_INDEX(NAME){} + 1)
#define JARL_STATIC_STRING(STRING) []() { using namespace jarl::detail; return #STRING ##_s; }()
//#define JARL_STATIC_STRING(STRING) []<std::size_t... Is>(std::index_sequence<Is...>) -> jarl::detail::static_string<#STRING[Is]...> { return {}; } (std::make_index_sequence<std::size(#STRING) - 1>{})

/// @brief Identity macro that expands to its arguments unchanged.
/// 
/// Can be used to force macro expansion or to improve readability.
/// @param ... Any arguments.
/// @return The same arguments passed in.
#define JARL_TYPE(...) __VA_ARGS__

/// @brief Alias for `JARL_TYPE` macro.
/// @param ... Any arguments.
/// @return The same arguments passed in.
#define JT JARL_TYPE

namespace jarl
{

namespace detail
{

template <typename CharT, CharT... Cs>
struct static_string
{
    template <CharT... OtherCs>
    constexpr static_string<CharT, Cs..., OtherCs...> operator+(static_string<CharT, OtherCs...>) const noexcept { return {}; }

    constexpr operator const char*() const noexcept { return data; }

    static constexpr const char data[] {Cs..., '\0'};
};

template <typename CharT, CharT... Cs>
constexpr static_string<CharT, Cs...> operator""_s() noexcept { return {}; }

}

/// @brief Alias for the unsigned integer type used for sizes and counts..
using size_type = std::size_t;

template <typename>
struct tag {};

}

#endif //JARL_DETAIL_HPP