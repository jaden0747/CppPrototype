// ============================================================================
// Template 02 — Demo: Non-Type Parameters & Specialization
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Non-type template parameters
// ──────────────────────────────────────────────────────────────────────────
template <typename T, int N>
struct FixedArray
{
    T             data[N] = {};
    constexpr int size() const
    {
        return N;
    }
    T& operator[](int i)
    {
        return data[i];
    }
    const T& operator[](int i) const
    {
        return data[i];
    }
};

// template<auto> — C++17
template <auto Value>
struct Constant
{
    static constexpr auto value = Value;
    using type                  = decltype(Value);
};

// Non-type for compile-time configuration
template <int Width, int Height>
struct Screen
{
    static constexpr int pixels = Width * Height;
    void                 info() const
    {
        std::cout << "  Screen " << Width << "x" << Height << " = " << pixels << " pixels\n";
    }
};

void demo_nontype()
{
    std::cout << "=== 1. Non-Type Template Parameters ===\n";

    FixedArray<int, 5> arr;
    for (int i = 0; i < arr.size(); ++i)
        arr[i] = i * 10;
    std::cout << "  FixedArray<int,5>: ";
    for (int i = 0; i < arr.size(); ++i)
        std::cout << arr[i] << " ";
    std::cout << "\n";

    std::cout << "  Constant<42>::value = " << Constant<42>::value << "\n";
    std::cout << "  Constant<'X'>::value = " << Constant<'X'>::value << "\n";
    std::cout << "  Constant<true>::value = " << std::boolalpha << Constant<true>::value << "\n";

    Screen<1920, 1080> hd;
    hd.info();
    Screen<3840, 2160> uhd;
    uhd.info();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Full specialization
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct TypeName
{
    static const char* get()
    {
        return "unknown";
    }
};

template <>
struct TypeName<int>
{
    static const char* get()
    {
        return "int";
    }
};
template <>
struct TypeName<double>
{
    static const char* get()
    {
        return "double";
    }
};
template <>
struct TypeName<char>
{
    static const char* get()
    {
        return "char";
    }
};
template <>
struct TypeName<bool>
{
    static const char* get()
    {
        return "bool";
    }
};
template <>
struct TypeName<std::string>
{
    static const char* get()
    {
        return "std::string";
    }
};

void demo_full_specialization()
{
    std::cout << "=== 2. Full Specialization ===\n";
    std::cout << "  TypeName<int>: " << TypeName<int>::get() << "\n";
    std::cout << "  TypeName<double>: " << TypeName<double>::get() << "\n";
    std::cout << "  TypeName<char>: " << TypeName<char>::get() << "\n";
    std::cout << "  TypeName<float>: " << TypeName<float>::get() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Partial specialization
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct Serializer
{
    static std::string to_string(const T& val)
    {
        return std::to_string(val);
    }
};

// Full specialization for string
template <>
struct Serializer<std::string>
{
    static std::string to_string(const std::string& val)
    {
        return "\"" + val + "\"";
    }
};

// Partial: pointer types
template <typename T>
struct Serializer<T*>
{
    static std::string to_string(T* ptr)
    {
        return ptr ? Serializer<T>::to_string(*ptr) : "null";
    }
};

// Partial: vectors
template <typename T>
struct Serializer<std::vector<T>>
{
    static std::string to_string(const std::vector<T>& v)
    {
        std::string result = "[";
        for (size_t i = 0; i < v.size(); ++i)
        {
            if (i)
                result += ", ";
            result += Serializer<T>::to_string(v[i]);
        }
        return result + "]";
    }
};

// Partial: pairs
template <typename A, typename B>
struct Serializer<std::pair<A, B>>
{
    static std::string to_string(const std::pair<A, B>& p)
    {
        return "(" + Serializer<A>::to_string(p.first) + ", " + Serializer<B>::to_string(p.second) + ")";
    }
};

void demo_partial_specialization()
{
    std::cout << "=== 3. Partial Specialization ===\n";

    std::cout << "  int: " << Serializer<int>::to_string(42) << "\n";
    std::cout << "  string: " << Serializer<std::string>::to_string("hello") << "\n";

    int  x  = 7;
    int* px = &x;
    std::cout << "  int*: " << Serializer<int*>::to_string(px) << "\n";
    int* null = nullptr;
    std::cout << "  null: " << Serializer<int*>::to_string(null) << "\n";

    std::vector<int> v{1, 2, 3};
    std::cout << "  vector<int>: " << Serializer<std::vector<int>>::to_string(v) << "\n";

    std::pair<int, std::string> p{42, "hi"};
    std::cout << "  pair: " << Serializer<std::pair<int, std::string>>::to_string(p) << "\n";

    // Nested: vector of pairs
    std::vector<std::pair<int, int>> vp{{1, 2}, {3, 4}};
    std::cout << "  vector<pair>: " << Serializer<std::vector<std::pair<int, int>>>::to_string(vp) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Function overloading vs specialization
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
std::string describe(T val)
{
    return "generic: " + std::to_string(val);
}

// Overload for string (preferred over specialization)
std::string describe(const std::string& val)
{
    return "string: " + val;
}

// Overload for pointers
template <typename T>
std::string describe(T* ptr)
{
    return ptr ? "ptr → " + describe(*ptr) : "null";
}

void demo_overloading()
{
    std::cout << "=== 4. Function Overloading ===\n";
    std::cout << "  " << describe(42) << "\n";
    std::cout << "  " << describe(3.14) << "\n";
    std::cout << "  " << describe(std::string("hello")) << "\n";
    int x = 99;
    std::cout << "  " << describe(&x) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 02 — Non-Type Params & Specialization  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_nontype();
    demo_full_specialization();
    demo_partial_specialization();
    demo_overloading();

    std::cout << "All demos complete.\n";
    return 0;
}
