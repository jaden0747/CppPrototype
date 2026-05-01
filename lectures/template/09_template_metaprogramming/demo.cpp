// ============================================================================
// Template 09 — Demo: Template Metaprogramming
// ============================================================================
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// ──────────────────────────────────────────────────────────────────────────
// 1. Compile-Time Value Computation
// ──────────────────────────────────────────────────────────────────────────
template <int N>
struct Factorial
{
    static constexpr long long value = N * Factorial<N - 1>::value;
};
template <>
struct Factorial<0>
{
    static constexpr long long value = 1;
};

template <int N>
struct Fibonacci
{
    static constexpr long long value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};
template <>
struct Fibonacci<0>
{
    static constexpr long long value = 0;
};
template <>
struct Fibonacci<1>
{
    static constexpr long long value = 1;
};

// Power: Base^Exp
template <int Base, int Exp>
struct Power
{
    static constexpr long long value = Base * Power<Base, Exp - 1>::value;
};
template <int Base>
struct Power<Base, 0>
{
    static constexpr long long value = 1;
};

void demo_compile_time_values()
{
    std::cout << "=== 1. Compile-Time Value Computation ===\n";

    static_assert(Factorial<0>::value == 1);
    static_assert(Factorial<5>::value == 120);
    static_assert(Factorial<10>::value == 3628800);
    std::cout << "  Factorial<10> = " << Factorial<10>::value << "\n";

    static_assert(Fibonacci<0>::value == 0);
    static_assert(Fibonacci<10>::value == 55);
    std::cout << "  Fibonacci<10> = " << Fibonacci<10>::value << "\n";

    static_assert(Power<2, 10>::value == 1024);
    std::cout << "  Power<2,10> = " << Power<2, 10>::value << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Recursive Type Manipulation
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct PointerDepth
{
    static constexpr int value = 0;
};
template <typename T>
struct PointerDepth<T*>
{
    static constexpr int value = 1 + PointerDepth<T>::value;
};

// Strip all pointers
template <typename T>
struct RemoveAllPointers
{
    using type = T;
};
template <typename T>
struct RemoveAllPointers<T*>
{
    using type = typename RemoveAllPointers<T>::type;
};

template <typename T>
using remove_all_pointers_t = typename RemoveAllPointers<T>::type;

// Count array rank
template <typename T>
struct ArrayRank
{
    static constexpr size_t value = 0;
};
template <typename T, size_t N>
struct ArrayRank<T[N]>
{
    static constexpr size_t value = 1 + ArrayRank<T>::value;
};

void demo_type_manipulation()
{
    std::cout << "=== 2. Recursive Type Manipulation ===\n";

    static_assert(PointerDepth<int>::value == 0);
    static_assert(PointerDepth<int*>::value == 1);
    static_assert(PointerDepth<int***>::value == 3);
    std::cout << "  PointerDepth<int***> = " << PointerDepth<int***>::value << "\n";

    static_assert(std::is_same_v<remove_all_pointers_t<int***>, int>);
    static_assert(std::is_same_v<remove_all_pointers_t<double>, double>);
    std::cout << "  RemoveAllPointers<int***> → int ✓\n";

    static_assert(ArrayRank<int>::value == 0);
    static_assert(ArrayRank<int[3]>::value == 1);
    static_assert(ArrayRank<int[3][4][5]>::value == 3);
    std::cout << "  ArrayRank<int[3][4][5]> = " << ArrayRank<int[3][4][5]>::value << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Type Lists
// ──────────────────────────────────────────────────────────────────────────
template <typename... Ts>
struct TypeList
{
};

// Size
template <typename TL>
struct TL_Size;
template <typename... Ts>
struct TL_Size<TypeList<Ts...>>
{
    static constexpr size_t value = sizeof...(Ts);
};

// Head
template <typename TL>
struct TL_Head;
template <typename T, typename... Rest>
struct TL_Head<TypeList<T, Rest...>>
{
    using type = T;
};

// Tail
template <typename TL>
struct TL_Tail;
template <typename T, typename... Rest>
struct TL_Tail<TypeList<T, Rest...>>
{
    using type = TypeList<Rest...>;
};

// Append
template <typename TL, typename T>
struct TL_Append;
template <typename... Ts, typename T>
struct TL_Append<TypeList<Ts...>, T>
{
    using type = TypeList<Ts..., T>;
};

// Contains
template <typename TL, typename T>
struct TL_Contains;
template <typename T>
struct TL_Contains<TypeList<>, T> : std::false_type
{
};
template <typename T, typename... Rest>
struct TL_Contains<TypeList<T, Rest...>, T> : std::true_type
{
};
template <typename Head, typename... Rest, typename T>
struct TL_Contains<TypeList<Head, Rest...>, T> : TL_Contains<TypeList<Rest...>, T>
{
};

void demo_type_lists()
{
    std::cout << "=== 3. Type Lists ===\n";

    using MyTypes = TypeList<int, double, std::string>;

    static_assert(TL_Size<MyTypes>::value == 3);
    std::cout << "  Size = " << TL_Size<MyTypes>::value << "\n";

    static_assert(std::is_same_v<TL_Head<MyTypes>::type, int>);
    std::cout << "  Head = int ✓\n";

    using Tail = TL_Tail<MyTypes>::type;
    static_assert(TL_Size<Tail>::value == 2);
    std::cout << "  Tail size = " << TL_Size<Tail>::value << "\n";

    using Extended = TL_Append<MyTypes, char>::type;
    static_assert(TL_Size<Extended>::value == 4);
    std::cout << "  Append<char> size = " << TL_Size<Extended>::value << "\n";

    static_assert(TL_Contains<MyTypes, int>::value);
    static_assert(!TL_Contains<MyTypes, char>::value);
    std::cout << "  Contains<int> = true ✓\n";
    std::cout << "  Contains<char> = false ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. integral_constant & bool_constant
// ──────────────────────────────────────────────────────────────────────────
template <int N>
using int_c = std::integral_constant<int, N>;

// Compile-time arithmetic via types
template <typename A, typename B>
using add_c = int_c<A::value + B::value>;

template <typename A, typename B>
using mul_c = int_c<A::value * B::value>;

// Conditional on types
template <bool Cond, typename Then, typename Else>
using if_c = std::conditional_t<Cond, Then, Else>;

void demo_integral_constant()
{
    std::cout << "=== 4. integral_constant ===\n";

    using three  = int_c<3>;
    using four   = int_c<4>;
    using seven  = add_c<three, four>;
    using twelve = mul_c<three, four>;

    static_assert(seven::value == 7);
    static_assert(twelve::value == 12);
    std::cout << "  3 + 4 = " << seven::value << "\n";
    std::cout << "  3 * 4 = " << twelve::value << "\n";

    using result = if_c<(sizeof(int) == 4), int_c<32>, int_c<64>>;
    std::cout << "  int bits = " << result::value << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. constexpr vs TMP
// ──────────────────────────────────────────────────────────────────────────

// TMP style
template <int N>
struct GCD_TMP
{
    template <int A, int B>
    struct Impl
    {
        static constexpr int value = Impl<B, A % B>::value;
    };
    template <int A>
    struct Impl<A, 0>
    {
        static constexpr int value = A;
    };
};

// constexpr style (much cleaner)
constexpr int gcd_constexpr(int a, int b)
{
    while (b != 0)
    {
        int t = b;
        b     = a % b;
        a     = t;
    }
    return a;
}

void demo_constexpr_vs_tmp()
{
    std::cout << "=== 5. constexpr vs TMP ===\n";

    // TMP
    constexpr int g1 = GCD_TMP<0>::Impl<48, 18>::value;
    static_assert(g1 == 6);
    std::cout << "  TMP: gcd(48, 18) = " << g1 << "\n";

    // constexpr
    constexpr int g2 = gcd_constexpr(48, 18);
    static_assert(g2 == 6);
    std::cout << "  constexpr: gcd(48, 18) = " << g2 << "\n";
    std::cout << "  (Same result, much cleaner code!)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 09 — Template Metaprogramming          ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_compile_time_values();
    demo_type_manipulation();
    demo_type_lists();
    demo_integral_constant();
    demo_constexpr_vs_tmp();

    std::cout << "All demos complete.\n";
    return 0;
}
