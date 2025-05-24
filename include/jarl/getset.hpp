#ifndef JARL_GETSET_HPP
#define JARL_GETSET_HPP

#define JARL_BODY(...) __VA_ARGS__

#define JARL_GETTER(TYPE, NAME, BODY) \
constexpr decltype(auto) get ##NAME() const { BODY } \

#define JARL_SETTER(TYPE, NAME, BODY) \
constexpr void set ##NAME(const TYPE& value) { BODY } \

#define JARL_FORWARD_SETTER(TYPE, NAME, BODY) \
constexpr void set ##NAME(std::convertible_to<TYPE> auto&& value) { using value_type = decltype(value); BODY } \

#define JARL_MOVE_SETTER(TYPE, NAME, BODY) \
constexpr void set ##NAME(TYPE&& value) { BODY }

#endif //JARL_GETSET_HPP