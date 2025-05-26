#ifndef JARL_DETAIL_HPP
#define JARL_DETAIL_HPP

#include <cstddef>
#include <string_view>

#define JARL_SIZE_TYPE jarl::size_type
#define JARL_NAME_TYPE jarl::name_type
#define JARL_THIS_TYPE this_type
#define JARL_DEFAULT_ENUM_TYPE void

#define JARL_INDEX(INDEX) std::integral_constant<JARL_SIZE_TYPE, INDEX>
#define JARL_ADL jarl::detail::adl<JARL_THIS_TYPE>
#define JARL_DEFINE_CURRENT_INDEX(NAME) NAME ##_index(JARL_ADL);
#define JARL_CURRENT_INDEX(NAME) decltype(NAME ##_index(JARL_ADL{}))
#define JARL_NEXT_INDEX(NAME) JARL_INDEX(JARL_CURRENT_INDEX(NAME){} + 1)

#define JARL_TYPE(...) __VA_ARGS__
#define JT JARL_TYPE

namespace jarl
{

using size_type = std::size_t;
using name_type = std::string_view;

namespace detail
{

template <typename>
struct adl {};

}

}

#endif //JARL_DETAIL_HPP