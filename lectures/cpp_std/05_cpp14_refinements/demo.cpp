// ============================================================================
// Lecture 05 — Demo: C++14 Refinements
// ============================================================================
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Generic Lambdas
// ──────────────────────────────────────────────────────────────────────────
void demo_generic_lambdas()
{
    std::cout << "=== 1. Generic Lambdas ===\n";

    // auto parameters → template operator()
    auto add = [](auto a, auto b) { return a + b; };
    std::cout << "  add(3, 4) = " << add(3, 4) << "\n";
    std::cout << "  add(1.5, 2.5) = " << add(1.5, 2.5) << "\n";
    std::cout << "  add(\"hi \", \"there\") = " << add(std::string("hi "), std::string("there")) << "\n";

    // Generic lambda for printing any container
    auto print_all = [](const auto& container, const std::string& label)
    {
        std::cout << "  " << label << ": ";
        for (const auto& elem : container)
            std::cout << elem << " ";
        std::cout << "\n";
    };

    std::vector<int>         ints{1, 2, 3};
    std::vector<std::string> strs{"a", "b", "c"};
    print_all(ints, "ints");
    print_all(strs, "strs");
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Relaxed constexpr
// ──────────────────────────────────────────────────────────────────────────
constexpr int factorial(int n)
{
    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    return result;
}

constexpr int gcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b        = a % b;
        a        = temp;
    }
    return a;
}

constexpr bool is_prime(int n)
{
    if (n < 2)
        return false;
    for (int i = 2; i * i <= n; ++i)
        if (n % i == 0)
            return false;
    return true;
}

void demo_relaxed_constexpr()
{
    std::cout << "=== 2. Relaxed constexpr ===\n";

    static_assert(factorial(6) == 720, "");
    static_assert(gcd(48, 18) == 6, "");
    static_assert(is_prime(17), "");
    static_assert(!is_prime(15), "");

    std::cout << "  factorial(6) = " << factorial(6) << "\n";
    std::cout << "  gcd(48, 18) = " << gcd(48, 18) << "\n";
    std::cout << "  is_prime(17) = " << std::boolalpha << is_prime(17) << "\n";
    std::cout << "  is_prime(15) = " << is_prime(15) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::make_unique
// ──────────────────────────────────────────────────────────────────────────
struct Gadget
{
    int         id;
    std::string name;
    Gadget(int id, std::string name)
        : id(id)
        , name(std::move(name))
    {
        std::cout << "    Gadget(" << this->id << ", " << this->name << ") created\n";
    }
    ~Gadget()
    {
        std::cout << "    ~Gadget(" << id << ")\n";
    }
};

void demo_make_unique()
{
    std::cout << "=== 3. std::make_unique ===\n";

    auto g1 = std::make_unique<Gadget>(1, "Widget");
    auto g2 = std::make_unique<Gadget>(2, "Gizmo");

    // Array form
    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i)
        arr[i] = i * 10;
    std::cout << "  Array: ";
    for (int i = 0; i < 5; ++i)
        std::cout << arr[i] << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Variable templates
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
constexpr T pi = T(3.14159265358979323846L);

template <typename T>
constexpr T e = T(2.71828182845904523536L);

template <typename T>
constexpr bool is_floating_v = std::is_floating_point<T>::value;

void demo_variable_templates()
{
    std::cout << "=== 4. Variable Templates ===\n";

    std::cout << "  pi<float>  = " << pi<float> << "\n";
    std::cout << "  pi<double> = " << pi<double> << "\n";
    std::cout << "  e<double>  = " << e<double> << "\n";

    static_assert(is_floating_v<double>, "");
    static_assert(!is_floating_v<int>, "");
    std::cout << "  is_floating_v<double> = " << std::boolalpha << is_floating_v<double> << "\n";
    std::cout << "  is_floating_v<int> = " << is_floating_v<int> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Return type deduction & decltype(auto)
// ──────────────────────────────────────────────────────────────────────────
auto multiply(int a, int b)
{
    return a * b;
}

auto makeVec()
{
    return std::vector<int>{1, 2, 3};
}

static int     global_val = 42;
decltype(auto) getRef()
{
    return (global_val);
} // returns int&

void demo_return_deduction()
{
    std::cout << "=== 5. Return Type Deduction ===\n";

    std::cout << "  multiply(6, 7) = " << multiply(6, 7) << "\n";

    auto v = makeVec();
    std::cout << "  makeVec() size = " << v.size() << "\n";

    decltype(auto) ref = getRef();
    ref                = 100;
    std::cout << "  global_val after ref=100: " << global_val << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Binary literals & digit separators
// ──────────────────────────────────────────────────────────────────────────
void demo_literals()
{
    std::cout << "=== 6. Binary Literals & Digit Separators ===\n";

    int    mask    = 0b1111'0000;
    long   big     = 1'000'000;
    double precise = 3.14159'26535'89793;

    std::cout << "  0b1111'0000 = " << mask << "\n";
    std::cout << "  1'000'000 = " << big << "\n";
    std::cout << "  pi = " << precise << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. [[deprecated]]
// ──────────────────────────────────────────────────────────────────────────
[[deprecated("Use newApi() instead")]]
void oldApi()
{
    std::cout << "  oldApi called\n";
}

void newApi()
{
    std::cout << "  newApi called\n";
}

void demo_deprecated()
{
    std::cout << "=== 7. [[deprecated]] ===\n";
    // oldApi();  // Uncomment to see compiler warning
    newApi();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 05 — C++14 Refinements                  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_generic_lambdas();
    demo_relaxed_constexpr();
    demo_make_unique();
    demo_variable_templates();
    demo_return_deduction();
    demo_literals();
    demo_deprecated();

    std::cout << "All demos complete.\n";
    return 0;
}
