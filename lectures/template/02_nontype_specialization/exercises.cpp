// ============================================================================
// Template 02 — Exercises: Non-Type Parameters & Specialization
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Create a `StaticString<N>` that holds a fixed-size char array.
// Support: constructor from const char*, size(), operator[], c_str().
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<int N> class StaticString { ... };

void exercise_static_string()
{
    // StaticString<16> s("hello");
    // assert(s.size() == 5);
    // assert(s[0] == 'h');
    std::cout << "  Exercise 1: StaticString — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Create a compile-time `Factorial<N>` using template
// specialization (not constexpr). Base case: Factorial<0> = 1.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<int N> struct Factorial { ... };
// TODO: template<> struct Factorial<0> { ... };

void exercise_factorial()
{
    // static_assert(Factorial<0>::value == 1);
    // static_assert(Factorial<1>::value == 1);
    // static_assert(Factorial<5>::value == 120);
    // static_assert(Factorial<10>::value == 3628800);
    std::cout << "  Exercise 2: Factorial — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a `TypeInfo<T>` trait with full specializations for:
//   int, double, float, char, bool, std::string
// Each provides: name (const char*), is_numeric (bool), size (bytes)
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> struct TypeInfo;

void exercise_typeinfo()
{
    // assert(std::string(TypeInfo<int>::name) == "int");
    // assert(TypeInfo<int>::is_numeric == true);
    // assert(TypeInfo<int>::size == 4);
    // assert(std::string(TypeInfo<std::string>::name) == "std::string");
    // assert(TypeInfo<std::string>::is_numeric == false);
    std::cout << "  Exercise 3: TypeInfo — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write partial specializations:
//   IsPointer<T>   — has value = false
//   IsPointer<T*>  — has value = true
//   RemovePointer<T>  — has type = T
//   RemovePointer<T*> — has type = T
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> struct IsPointer { ... };
// TODO: template<typename T> struct RemovePointer { ... };

void exercise_pointer_traits()
{
    // static_assert(!IsPointer<int>::value);
    // static_assert(IsPointer<int*>::value);
    // static_assert(IsPointer<double**>::value);
    // static_assert(std::is_same_v<RemovePointer<int*>::type, int>);
    // static_assert(std::is_same_v<RemovePointer<double>::type, double>);
    std::cout << "  Exercise 4: IsPointer/RemovePointer — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a `Formatter<T>` class template that:
// - Primary: uses std::to_string
// - Specializes for std::string (quotes it)
// - Partial for T* (shows *ptr or "null")
// - Partial for std::vector<T> (shows [a, b, c])
// - Partial for std::pair<A,B> (shows (a, b))
// Then write a free function format_value(T) that dispatches to Formatter.
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement Formatter and format_value

void exercise_formatter()
{
    // assert(format_value(42) == "42");
    // assert(format_value(std::string("hi")) == "\"hi\"");
    // int x = 5;
    // assert(format_value(&x) == "5");
    // assert(format_value(std::vector<int>{1,2,3}) == "[1, 2, 3]");
    // assert(format_value(std::make_pair(1, 2)) == "(1, 2)");
    std::cout << "  Exercise 5: Formatter — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 02 — Exercises: Non-Type & Special.    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_static_string();
    exercise_factorial();
    exercise_typeinfo();
    exercise_pointer_traits();
    exercise_formatter();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
