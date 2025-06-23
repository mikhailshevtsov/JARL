#ifndef JARL_ENUM_HPP
#define JARL_ENUM_HPP

#include "detail.hpp"

#include <stdexcept>
#include <array>
#include <string_view>
#include <optional>
#include <type_traits>

/// @brief Defines an enumerator within an enum class with optional custom value.
/// 
/// This macro defines a named enumerator `NAME` with optional explicit value.
/// It generates helper friend functions to retrieve the enumerator, its name,
/// value, and fully qualified name used internally by the enum system.
/// 
/// @param NAME Enumerator identifier.
/// @param ... Optional explicit value or additional arguments for value assignment.
#define JARL_ENUMER(NAME, ...) \
JARL_DEFINE_CURRENT_INDEX(NAME) \
static constexpr auto NAME = create_enum(jarl::tag<basic_enum>{}, JARL_CURRENT_INDEX(NAME){}); \
friend constexpr auto get_enum(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return NAME; } \
friend constexpr auto get_name(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return #NAME; } \
friend constexpr auto get_value(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return jarl::detail::assign_value(JARL_TAG{}, JARL_CURRENT_INDEX(NAME){} __VA_OPT__(, __VA_ARGS__)); } \
friend constexpr auto get_full_name(JARL_TAG, JARL_CURRENT_INDEX(NAME)) noexcept { return get_static_name(JARL_TAG{}) + JARL_STATIC_STRING(::NAME); } \
friend constexpr JARL_DECLARE_NEXT_INDEX(NAME)


/// @brief Defines a strongly-typed enum class with custom value and index types.
/// 
/// This macro declares a struct `ENUM_NAME` inheriting from the internal `basic_enum`
/// template with specified underlying value and index types. It provides static
/// functions to access enum meta-information and enforces non-empty enumerations.
/// 
/// @param ENUM_NAME Name of the enum class.
/// @param VALUE_TYPE Integral type used as underlying enum value.
/// @param INDEX_TYPE Integral type used as enumerator index.
/// @param ... Enumerator definitions and additional enum body code.
#define JARL_ENUM_VI(ENUM_NAME, VALUE_TYPE, INDEX_TYPE, ...) \
struct ENUM_NAME : public jarl::basic_enum<ENUM_NAME, VALUE_TYPE, INDEX_TYPE> \
{ \
    using basic_enum<ENUM_NAME, VALUE_TYPE, INDEX_TYPE>::basic_enum; \
    constexpr ENUM_NAME(basic_enum e) noexcept : basic_enum{e} {} \
    friend constexpr auto get_name(JARL_TAG) noexcept { return #ENUM_NAME; } \
    friend constexpr auto get_static_name(JARL_TAG) noexcept { return JARL_STATIC_STRING(ENUM_NAME); } \
    friend constexpr JARL_INDEX(0) \
    __VA_ARGS__ \
    get_size(JARL_TAG) noexcept { return {}; } \
    static_assert(decltype(get_size(JARL_TAG{})){} > 0, "cannot define an empty enum"); \
}

/// @brief Defines a strongly-typed enum class with a custom underlying value type and default index type.
/// 
/// Shortcut macro using default index type.
/// 
/// @param ENUM_NAME Name of the enum class.
/// @param VALUE_TYPE Integral underlying value type.
/// @param ENUM_BODY Enumerator definitions and additional enum body code.
#define JARL_ENUM_V(ENUM_NAME, VALUE_TYPE, ENUM_BODY) JARL_ENUM_VI(ENUM_NAME, VALUE_TYPE, jarl::default_index_type_t, ENUM_BODY)

/// @brief Defines a strongly-typed enum class with a custom index type and default value type.
/// 
/// Shortcut macro using default underlying value type.
/// 
/// @param ENUM_NAME Name of the enum class.
/// @param INDEX_TYPE Integral enumerator index type.
/// @param ENUM_BODY Enumerator definitions and additional enum body code.
#define JARL_ENUM_I(ENUM_NAME, INDEX_TYPE, ENUM_BODY) JARL_ENUM_VI(ENUM_NAME, jarl::default_value_type_t, INDEX_TYPE, ENUM_BODY)

/// @brief Defines a strongly-typed enum class with default underlying value and index types.
/// 
/// Shortcut macro using default underlying value and index types.
/// 
/// @param ENUM_NAME Name of the enum class.
/// @param ENUM_BODY Enumerator definitions and additional enum body code.
#define JARL_ENUM(ENUM_NAME, ENUM_BODY) JARL_ENUM_VI(ENUM_NAME, jarl::default_value_type_t, jarl::default_index_type_t, ENUM_BODY)

/// @brief Alias for `JARL_ENUMER`.
#define JE JARL_ENUMER

namespace jarl
{

namespace detail
{

struct enum_base {};

enum class default_enum {};

template <typename EnumT, std::integral T>
struct enum_impl { enum class enum_t : T {}; };

template <typename EnumT, std::integral T>
using enum_t = typename enum_impl<EnumT, T>::enum_t;

template <typename EnumT, size_type I>
constexpr typename EnumT::value_type assign_value(tag<EnumT>, JARL_INDEX(I), typename EnumT::value_type value) noexcept { return value; }

template <typename EnumT, size_type I>
constexpr typename EnumT::value_type assign_value(tag<EnumT>, JARL_INDEX(I)) noexcept
{
    if constexpr (I > 0)
        return get_value(tag<EnumT>{}, JARL_INDEX(I - 1){}) + 1;
    return 0;
}

}

/// @brief Concept satisfied by enums supporting meta-information.
template <typename T>
concept meta_enum = std::is_base_of_v<detail::enum_base, T>;

/// @brief Underlying integer type of the default enum.
using default_value_type_t = std::underlying_type_t<detail::default_enum>;

/// @brief Default index type used for enumerator indexing.
using default_index_type_t = unsigned char;

template <meta_enum EnumT, size_type... Is>
class enum_traits_impl
{
public:
    /// @brief Alias for the enumeration type.
    using enum_type = EnumT;

    /// @brief Underlying value type of the enumeration.
    using value_type = typename EnumT::value_type;

    /// @brief Returns the name of the enumeration.
    /// @return The enum's name as a string.
    static constexpr auto name() noexcept { return get_name(tag<EnumT>{}); }

    /// @brief Returns an array of all enumerators.
    /// @return Compile-time array of enumerator values.
    static constexpr const auto& array() noexcept { return _array; }

    /// @brief Returns an array of enumerator names.
    /// @return Compile-time array of short names of enumerators.
    static constexpr const auto& names() noexcept { return _names; }

    /// @brief Returns an array of enumerator values.
    /// @return Compile-time array of underlying integral values.
    static constexpr const auto& values() noexcept { return _values; }

    /// @brief Returns an array of fully qualified enumerator names.
    /// @return Compile-time array of strings like `EnumName::EnumeratorName`.
    static constexpr const auto& full_names() noexcept { return _full_names; }

    /// @brief Returns the number of enumerators.
    /// @return Number of enumerators in the enumeration.
    static constexpr auto size() noexcept { return get_size(tag<EnumT>{}); }

private:
    template <typename T>
    using array_t = std::array<const T, sizeof...(Is)>;

    static constexpr array_t<EnumT> _array = { get_enum(tag<EnumT>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<const char*> _names = { get_name(tag<EnumT>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<value_type> _values = { get_value(tag<EnumT>{}, JARL_INDEX(Is){})... };
    static constexpr array_t<const char*> _full_names = { get_full_name(tag<EnumT>{}, JARL_INDEX(Is){})... };
};

template <meta_enum EnumT, typename>
struct make_enum_traits{};

template <meta_enum EnumT, size_type... Is>
struct make_enum_traits<EnumT, std::index_sequence<Is...>> { using type = enum_traits_impl<EnumT, Is...>; };

template <meta_enum EnumT, size_type N>
using make_enum_traits_t = typename make_enum_traits<EnumT, decltype(std::make_index_sequence<N>{})>::type;

/// @brief Provides meta-information for the given enumeration type.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
template <meta_enum EnumT>
using enum_traits = make_enum_traits_t<typename std::decay_t<EnumT>::this_type, get_size(tag<typename std::decay_t<EnumT>::this_type>{})>;

/// @brief Underlying value type of the enumeration.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
template <meta_enum EnumT>
using value_type_t = typename enum_traits<EnumT>::value_type;

/// @brief Returns the name of the enumeration type.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Name of the enumeration as a string.
template <meta_enum EnumT>
constexpr auto enum_name() noexcept { return enum_traits<EnumT>::name(); }

/// @brief Returns all enumerators as an array.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Array of all enumerators.
template <meta_enum EnumT>
constexpr const auto& enum_array() noexcept { return enum_traits<EnumT>::array(); }

/// @brief Returns all enumerator names as an array of strings.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Array of short names of all enumerators.
template <meta_enum EnumT>
constexpr const auto& enum_names() noexcept { return enum_traits<EnumT>::names(); }

/// @brief Returns all enumerator values as an array.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Array of integral values of all enumerators.
template <meta_enum EnumT>
constexpr const auto& enum_values() noexcept { return enum_traits<EnumT>::values(); }

/// @brief Returns all enumerator names in fully qualified form.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Array of strings in the format `EnumName::EnumeratorName`.
template <meta_enum EnumT>
constexpr const auto& enum_full_names() noexcept { return enum_traits<EnumT>::full_names(); }

/// @brief Returns the number of enumerators.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @return Number of enumerators.
template <meta_enum EnumT>
constexpr auto enum_size() noexcept { return enum_traits<EnumT>::size(); }

/// @brief Exception thrown when enum_cast fails due to invalid input.
struct bad_enum_cast : std::bad_cast
{
    const char* what() const noexcept override { return "bad enum_cast"; }
};

/// @brief Returns the enumerator at the given index.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param index Index of the enumerator.
/// @return Enumerator at the given index.
/// @throws std::out_of_range If the index is out of bounds.
template <meta_enum EnumT>
constexpr EnumT make_enum(size_type index)
{
    if (index < enum_size<EnumT>())
        return enum_array<EnumT>()[index];
    throw std::out_of_range("enum index is out of range");
}

/// @brief Returns the enumerator at the given index or std::nullopt if out of bounds.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param index Index of the enumerator.
/// @return Optional enumerator at the given index.
template <meta_enum EnumT>
constexpr std::optional<EnumT> make_optional_enum(size_type index) noexcept
{
    if (index < enum_size<EnumT>())
        return enum_array<EnumT>()[index];
    return {};
}

/// @brief Converts a string to an enum or throws bad_enum_cast.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param name Short or full string name of the enum constant.
/// @return Corresponding enum value.
/// @throws bad_enum_cast If the string does not match any enum name.
template <meta_enum EnumT>
constexpr EnumT enum_cast(std::string_view name)
{
    constexpr const auto& names = enum_traits<EnumT>::names();
    constexpr size_type size = enum_traits<EnumT>::size();
    for (size_type i = 0; i < size; ++i)
        if (names[i] == name)
            return make_enum<EnumT>(i);

    constexpr const auto& full_names = enum_traits<EnumT>::full_names();
    for (size_type i = 0; i < size; ++i)
        if (full_names[i] == name)
            return make_enum<EnumT>(i);

    throw bad_enum_cast{};
}

/// @brief Converts a string to an enum or returns empty optional on failure.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param name Short or full string name of the enum constant.
/// @return Optional enum value, empty if name is not found.
template <meta_enum EnumT>
constexpr std::optional<EnumT> enum_optional_cast(std::string_view name) noexcept
{
    constexpr const auto& names = enum_traits<EnumT>::names();
    constexpr size_type size = enum_traits<EnumT>::size();
    for (size_type i = 0; i < size; ++i)
        if (names[i] == name)
            return make_enum<EnumT>(i);

    constexpr const auto& full_names = enum_traits<EnumT>::full_names();
    for (size_type i = 0; i < size; ++i)
        if (full_names[i] == name)
            return make_enum<EnumT>(i);

    return {};
}

/// @brief Converts a value to an enum or throws bad_enum_cast.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param value Underlying integer value of the enum.
/// @return Corresponding enum value.
/// @throws bad_enum_cast If the value is not defined in the enum.
template <meta_enum EnumT>
constexpr EnumT enum_cast(value_type_t<EnumT> value)
{
    constexpr const auto& values = enum_traits<EnumT>::values();
    constexpr size_type size = enum_traits<EnumT>::size();
    for (size_type i = 0; i < size; ++i)
        if (values[i] == value)
            return make_enum<EnumT>(i);
    throw bad_enum_cast{};
}

/// @brief Converts a value to an enum or returns empty optional on failure.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param value Underlying integer value of the enum.
/// @return Optional enum value, empty if value is not found.
template <meta_enum EnumT>
constexpr std::optional<EnumT> enum_optional_cast(value_type_t<EnumT> value) noexcept
{
    constexpr const auto& values = enum_traits<EnumT>::values();
    constexpr size_type size = enum_traits<EnumT>::size();
    for (size_type i = 0; i < size; ++i)
        if (values[i] == value)
            return make_enum<EnumT>(i);
    return {};
}

/// @brief Converts an enum to string, value, or another enum type.
/// @tparam T Target type: string, integral, or another enum.
/// @tparam EnumT Enumeration type satisfying the meta_enum concept.
/// @param enumer Enum value to convert.
/// @return Converted result as type T.
template <typename T, meta_enum EnumT>
constexpr T enum_cast(EnumT enumer) noexcept
{
    constexpr bool is_name = std::is_convertible_v<T, std::string>;
    constexpr bool is_value = std::is_same_v<T, value_type_t<EnumT>>;
    constexpr bool is_enum = std::is_convertible_v<T, EnumT>;

    static_assert(is_name || is_value || is_enum, "bad enum_cast");

    if constexpr (is_name)
        return T{nameof(enumer)};
    if constexpr (is_value)
        return T{valueof(enumer)};
    if constexpr (is_enum)
        return T{enumer};
}

/// @brief CRTP base class providing core functionality for enum wrapper types.
/// 
/// This class implements common interface and operations for enum classes
/// inheriting from it via CRTP. It should not be used directly by users.
/// 
/// @tparam EnumT The derived enumeration type.
/// @tparam ValueT Integral type representing the underlying enum value.
/// @tparam IndexT Integral type used to index enumerators internally.
template <typename EnumT, std::integral ValueT, std::integral IndexT>
class basic_enum : public detail::enum_base
{
public:
    /// @brief Alias for the derived enum type.
    using this_type = EnumT;

    /// @brief Underlying integral type of the enum value.
    using value_type = std::underlying_type_t<detail::enum_t<EnumT, ValueT>>;

    /// @brief Default constructor.
    constexpr basic_enum() noexcept = default;

    /// @brief Constructs enum from its string representation.
    /// @param name The enumerator name (short or full).
    explicit constexpr basic_enum(std::string_view name) { *this = enum_cast<EnumT>(name); }

    /// @brief Conversion operator to the underlying enum type.
    constexpr operator detail::enum_t<EnumT, ValueT>() const noexcept { return static_cast<detail::enum_t<EnumT, ValueT>>(valueof(*this)); }

    /// @brief Explicit conversion to any integral type convertible from ValueT.
    /// @tparam U Target integral type.
    template <std::convertible_to<ValueT> U>
    explicit constexpr operator U() const noexcept { return static_cast<U>(valueof(*this)); }

    /// @brief Three-way comparison operator based on underlying enum value.
    constexpr bool operator<=>(const basic_enum& other) const noexcept { return valueof(*this) <=> valueof(other); }

    /// @brief Returns the internal index of the enumerator.
    /// @param enumer Instance of the enum.
    /// @return Enumerator index.
    friend constexpr auto indexof(EnumT enumer) noexcept { return enumer.basic_enum::_index; }

    /// @brief Returns the short name of the enumerator.
    /// @param enumer Instance of the enum.
    /// @return Enumerator name as string_view.
    friend constexpr auto nameof(EnumT enumer) noexcept { return enum_names<EnumT>()[indexof(enumer)]; }
    
    /// @brief Returns the underlying value of the enumerator.
    /// @param enumer Instance of the enum.
    /// @return Enumerator underlying integral value.
    friend constexpr auto valueof(EnumT enumer) noexcept { return enum_values<EnumT>()[indexof(enumer)]; }
    
    /// @brief Returns the fully qualified name of the enumerator.
    /// @param enumer Instance of the enum.
    /// @return Fully qualified enumerator name, e.g. `EnumName::EnumeratorName`.
    friend constexpr auto fullnameof(EnumT enumer) noexcept { return enum_full_names<EnumT>()[indexof(enumer)]; }
    
    /// @brief Returns the exact enum instance of derived type.
    /// @param enumer Instance of the enum.
    /// @return Enum instance of type `EnumT`.
    friend constexpr auto exactly(EnumT enumer) noexcept { return typename EnumT::this_type{enumer}; }

private:
    explicit constexpr basic_enum(IndexT index) noexcept : _index{index} {}
    IndexT _index = 0;

    friend constexpr auto create_enum(tag<basic_enum>, IndexT index) noexcept { return basic_enum{index}; } \
};

}

#endif //JARL_ENUM_HPP