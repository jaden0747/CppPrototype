// ============================================================================
// Template 04 — Demo: SFINAE & Type Traits
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Basic SFINAE
// ──────────────────────────────────────────────────────────────────────────
// This overload only participates if T has ::value_type
template <typename T>
auto get_first(const T& container) -> typename T::value_type
{
    return *container.begin();
}

// Fallback for non-containers
template <typename T>
auto get_first(const T& val) -> std::enable_if_t<!std::is_class_v<T>, T>
{
    return val;
}

void demo_basic_sfinae()
{
    std::cout << "=== 1. Basic SFINAE ===\n";

    std::vector<int> v{10, 20, 30};
    std::cout << "  get_first(vector) = " << get_first(v) << "\n";
    std::cout << "  get_first(42) = " << get_first(42) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. enable_if — controlling overload resolution
// ──────────────────────────────────────────────────────────────────────────

// In return type
template <typename T>
std::enable_if_t<std::is_integral_v<T>, std::string> describe(T val)
{
    return "integer: " + std::to_string(val);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::string> describe(T val)
{
    return "floating: " + std::to_string(val);
}

// In template parameter (cleaner)
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T safe_negate(T val)
{
    // Handle unsigned types
    if constexpr (std::is_unsigned_v<T>)
        return static_cast<T>(0);
    else
        return -val;
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
T safe_negate(T val)
{
    return -val;
}

void demo_enable_if()
{
    std::cout << "=== 2. enable_if ===\n";
    std::cout << "  " << describe(42) << "\n";
    std::cout << "  " << describe(3.14) << "\n";

    std::cout << "  safe_negate(42) = " << safe_negate(42) << "\n";
    std::cout << "  safe_negate(3.14) = " << safe_negate(3.14) << "\n";
    std::cout << "  safe_negate(5u) = " << safe_negate(5u) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Type Traits
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
void inspect_type(const char* label)
{
    std::cout << "  " << label << ":\n";
    std::cout << "    is_integral     = " << std::is_integral_v<T> << "\n";
    std::cout << "    is_floating     = " << std::is_floating_point_v<T> << "\n";
    std::cout << "    is_arithmetic   = " << std::is_arithmetic_v<T> << "\n";
    std::cout << "    is_pointer      = " << std::is_pointer_v<T> << "\n";
    std::cout << "    is_class        = " << std::is_class_v<T> << "\n";
    std::cout << "    is_const        = " << std::is_const_v<T> << "\n";
    std::cout << "    is_reference    = " << std::is_reference_v<T> << "\n";
    std::cout << "    is_signed       = " << std::is_signed_v<T> << "\n";
    std::cout << "    sizeof          = " << sizeof(T) << "\n";
}

void demo_traits()
{
    std::cout << "=== 3. Type Traits ===\n";
    inspect_type<int>("int");
    inspect_type<const double>("const double");
    inspect_type<std::string>("std::string");
    inspect_type<int*>("int*");
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Type Transformations
// ──────────────────────────────────────────────────────────────────────────
void demo_transformations()
{
    std::cout << "=== 4. Type Transformations ===\n";

    // remove_const
    static_assert(std::is_same_v<std::remove_const_t<const int>, int>);
    std::cout << "  remove_const<const int> → int ✓\n";

    // remove_reference
    static_assert(std::is_same_v<std::remove_reference_t<int&>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int&&>, int>);
    std::cout << "  remove_reference<int&> → int ✓\n";

    // add_pointer
    static_assert(std::is_same_v<std::add_pointer_t<int>, int*>);
    std::cout << "  add_pointer<int> → int* ✓\n";

    // decay (like passing by value)
    static_assert(std::is_same_v<std::decay_t<const int&>, int>);
    static_assert(std::is_same_v<std::decay_t<int[5]>, int*>);
    std::cout << "  decay<const int&> → int ✓\n";
    std::cout << "  decay<int[5]> → int* ✓\n";

    // conditional
    static_assert(std::is_same_v<std::conditional_t<true, int, double>, int>);
    static_assert(std::is_same_v<std::conditional_t<false, int, double>, double>);
    std::cout << "  conditional<true, int, double> → int ✓\n";

    // common_type
    static_assert(std::is_same_v<std::common_type_t<int, double>, double>);
    std::cout << "  common_type<int, double> → double ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. decltype & declval
// ──────────────────────────────────────────────────────────────────────────

// declval lets us reason about types without constructing them
struct NonDefault
{
    NonDefault() = delete;
    int foo() const
    {
        return 42;
    }
};

// decltype(expr) gives us the type of the expression
template <typename T, typename U>
using add_result_t = decltype(std::declval<T>() + std::declval<U>());

// SFINAE check: does T have a .size() method?
template <typename T, typename = void>
struct has_size : std::false_type
{
};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type
{
};

void demo_decltype()
{
    std::cout << "=== 5. decltype & declval ===\n";

    // declval to get return type without construction
    using ret = decltype(std::declval<NonDefault>().foo());
    static_assert(std::is_same_v<ret, int>);
    std::cout << "  NonDefault::foo() returns int ✓\n";

    // add_result_t
    static_assert(std::is_same_v<add_result_t<int, double>, double>);
    std::cout << "  int + double → double ✓\n";

    // has_size detector
    static_assert(has_size<std::vector<int>>::value);
    static_assert(has_size<std::string>::value);
    static_assert(!has_size<int>::value);
    std::cout << "  vector has size: true ✓\n";
    std::cout << "  int has size: false ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 04 — SFINAE & Type Traits              ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_basic_sfinae();
    demo_enable_if();
    demo_traits();
    demo_transformations();
    demo_decltype();

    std::cout << "All demos complete.\n";
    return 0;
}
