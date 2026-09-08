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

    jarl::for_each_field<T>([&](auto field)
    {
        oss << indent << "\"" << field.name() << "\": ";
        build_json(jarl::get(obj, field), oss, indent);
        if (field.index() < jarl::meta<T>::size() - 1)
            oss << ",";
        oss << "\n";
    });

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
    JARL_FIELD(a, int, 42)
    JARL_FIELD(b, bool, false)
);

JARL_STRUCT(
    Test,
    JARL_FIELD(str, std::string, "Vova")
    JARL_FIELD(num, int, 100)
    JARL_FIELD(arr, JARL_MACRO(std::array<int, 3>), {1, 2, 3})
    JARL_FIELD(obj, Nested)
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

## 🔥 Streaming Object Example
```cpp
#include <iostream>
#include <string>
#include <sstream>

#include "jarl.hpp"

template <jarl::meta_struct T>
std::ostream& operator<<(std::ostream& out, const T& object)
{
    jarl::for_each_field<T>([&](auto field)
    {
        out << field.name() << "=" << jarl::get(object, field) << (field.index() + 1 < jarl::size(object) ? "\n" : "");
    });
    return out;
}

template <jarl::meta_struct T>
std::istream& operator>>(std::istream& in, T& object)
{
    std::string line;
    while (std::getline(in, line) && !line.empty())
    {
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (name.empty() || value.empty())
                continue;
            jarl::visit([&](auto& field_value)
            {
                std::istringstream iss(value);
                iss >> field_value;
            }, object, name);
        }
    }
    return in;
}

JARL_STRUCT(
    Person,
    JARL_FIELD(name, std::string)
    JARL_FIELD(age, int)
);

int main()
{
    std::string input = "name=John Doe\nage=30\n";
    std::istringstream iss(input);
    Person person;
    iss >> person;
    std::cout << person << "\n";
}
```
Output:
```bash
name=John
age=30
```

## 🔥 Copying Subset of Properties Example
```cpp
JARL_STRUCT(
    Small,
    JARL_FIELD(name, std::string)
    JARL_FIELD(age, int)
    JARL_FIELD(email, std::string)
);

JARL_STRUCT(
    Large,
    JARL_FIELD(email, std::string)
    JARL_FIELD(address, std::string)
    JARL_FIELD(name, std::string)
    JARL_FIELD(phone, std::string)
    JARL_FIELD(age, int)
);

int main()
{
    Large large;
    large.address = "123 Main St";
    large.name = "John Doe";
    large.phone = "555-1234";
    large.age = 30;
    large.email = "john.doe@example.com";

    Small small;

    jarl::for_each_field<Small>([&](auto field)
    {
        jarl::get(small, field) = jarl::get(large, field.static_name());
    });

    std::cout << small << "\n";
}
```
Output:
```bash
name=John Doe
age=30
email=john.doe@example.com
```

## 📜 License
No License
