# ⭐ JARL (Just Another Reflection Library)
**Header-only C++20 library for compile-time reflection**

## ✨ Features
- **Header-only** - no dependencies beyond the standard library.
- **Compile-time reflection** - retrieve field names, types and member pointers at compile time.
- **Structured access** - access struct fields by index or iterate over field metadata.
- **Simple syntax** - define structs and fields with convenient macros.

## ⚙️ Installation
Simply include the header in your project:
```cpp
#include "jarl.hpp"
```
No build steps or additional dependencies are required.

## ✅ Usage
Use **JARL_STRUCT** and **JARL_FIELD** to define structs with reflection metadata:
```cpp
#include "jarl.hpp"
#include <iostream>

JARL_STRUCT(Person, 
    JARL_FIELD(std::string, name)
    JARL_FIELD(int, age)
);

int main() {
    Person p{"Alice", 30};

    // Access field by index
    std::cout << jarl::get<0>(p) << "\n"; // prints "Alice"
    std::cout << jarl::get<1>(p) << "\n"; // prints 30

    // Access metadata
    constexpr auto size = jarl::meta<Person>::size();
    constexpr auto names = jarl::meta<Person>::field_names();
    constexpr auto types = jarl::meta<Person>::field_type_names();

    for (std::size_t i = 0; i < size; ++i)
        std::cout << types[i] << " " << names[i] << "\n";
}
```
Output:
```bash
Alice
30
std::string name
int age
```

## 🔥 JSON Serialization Example
You can easily serialize a **JARL_STRUCT** struct to JSON using fold expressions:
```cpp
#include <iostream>
#include <string>
#include <array>
#include <concepts>
#include <sstream>

#include "jarl.hpp"

template <typename T>
concept String =
    std::convertible_to<T, std::string_view> ||
    std::same_as<std::remove_cvref_t<T>, std::string> ||
    std::same_as<std::remove_cvref_t<T>, const char*> ||
    std::same_as<std::remove_cvref_t<T>, char*>;

template <typename T>
concept Boolean = std::same_as<std::remove_cvref_t<T>, bool>;

template <typename T>
concept Number = std::is_arithmetic_v<std::remove_cvref_t<T>> && !Boolean<T>;

template <typename T>
concept Array = requires(T a) {
    { std::begin(a) } -> std::input_iterator;
    { std::end(a) };
} && !String<T>;

template <typename T>
concept Object = jarl::meta_struct<T>;

template <typename T>
concept Json = String<T> || Number<T> || Boolean<T> || Array<T> || Object<T>;


template <String T>
void build_json(const T& str, std::ostringstream& oss, std::string& indent)
{
    oss << "\"" << str << "\"";
}

template <Boolean T>
void build_json(T val, std::ostringstream& oss, std::string& indent)
{
    oss << (val ? "true" : "false");
}

template <Number T>
void build_json(T num, std::ostringstream& oss, std::string& indent)
{
    oss << num;
}

template <Array T>
void build_json(const T& arr, std::ostringstream& oss, std::string& indent)
{
    oss << "[";
    auto last = std::prev(std::cend(arr));
    for (auto it = std::cbegin(arr); it < last; ++it)
    {
        build_json(*it, oss, indent);
        oss << ", ";
    }
    build_json(*last, oss, indent);
    oss << "]";
}

template <Object T>
void build_json(const T& obj, std::ostringstream& oss, std::string& indent)
{
    oss << "{\n";
    indent += "  ";

    [&]<std::size_t... Is>(std::index_sequence<Is...>)
    {
        ([&]()
        {
            oss << indent << "\"" << jarl::field<T, Is>::name() << "\": ";
            build_json(jarl::get<Is>(obj), oss, indent);
            if (Is < jarl::meta<T>::size() - 1)
                oss << ",";
            oss << "\n";
        }(), ...);
    }(std::make_index_sequence<jarl::meta<T>::size()>{});

    indent.erase(indent.size() - 2);
    oss << indent << "}";
}

template <Json T>
std::string to_json(const T& obj)
{
    std::ostringstream oss;
    std::string indent;
    indent.reserve(16);

    build_json(obj, oss, indent);

    return oss.str();
}

JARL_STRUCT(
    Nested,
    JARL_FIELD(int, a, 42)
    JARL_FIELD(bool, b, false)
);

JARL_STRUCT(
    Test,
    JARL_FIELD(std::string, str, "Vova")
    JARL_FIELD(int, num, 100)
    JARL_FIELD(JARL_MACRO(std::array<int, 3>), arr, {1, 2, 3})
    JARL_FIELD(Nested, obj)
);

int main()
{
    std::cout << to_json(Test{}) << "\n";
}
```
Output:
```bash
{
  "str": "Vova",
  "num": 100,
  "arr": [1, 2, 3],
  "obj": {
    "a": 42,
    "b": false
  }
}
```

## 📜 License
No License
