// ============================================================================
// Template 09 — Exercises: Template Metaprogramming
// ============================================================================
#include <cassert>
#include <iostream>
#include <type_traits>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Implement compile-time `IsPrime<N>`:
//   IsPrime<2>::value == true
//   IsPrime<7>::value == true
//   IsPrime<9>::value == false
//   IsPrime<1>::value == false
// Use a helper template that tries divisors from 2 to sqrt(N).
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<int N> struct IsPrime { ... };

void exercise_is_prime()
{
    // static_assert(IsPrime<2>::value);
    // static_assert(IsPrime<7>::value);
    // static_assert(IsPrime<13>::value);
    // static_assert(!IsPrime<1>::value);
    // static_assert(!IsPrime<4>::value);
    // static_assert(!IsPrime<9>::value);
    std::cout << "  Exercise 1: IsPrime — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Implement a TypeList `IndexOf<TL, T>`:
//   IndexOf<TypeList<int, double, char>, double>::value == 1
//   IndexOf<TypeList<int, double, char>, float>::value == -1
// ──────────────────────────────────────────────────────────────────────────
// TODO: TypeList, IndexOf

void exercise_index_of()
{
    // using TL = TypeList<int, double, char, bool>;
    // static_assert(IndexOf<TL, int>::value == 0);
    // static_assert(IndexOf<TL, char>::value == 2);
    // static_assert(IndexOf<TL, float>::value == -1);
    std::cout << "  Exercise 2: IndexOf — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Implement `Reverse<TypeList<Ts...>>`:
//   Reverse<TypeList<int, double, char>>::type == TypeList<char, double, int>
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename TL> struct Reverse;

void exercise_reverse()
{
    // using TL = TypeList<int, double, char>;
    // using Rev = Reverse<TL>::type;
    // static_assert(std::is_same_v<Rev, TypeList<char, double, int>>);
    std::cout << "  Exercise 3: Reverse — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Implement `Transform<TL, MetaFunc>` that applies a
// meta-function to each type:
//   template<typename T> struct AddPointer { using type = T*; };
//   Transform<TypeList<int, double>, AddPointer>::type == TypeList<int*, double*>
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename TL, template<typename> class F> struct Transform;

void exercise_transform()
{
    // template<typename T> struct AddConst { using type = const T; };
    // using TL = TypeList<int, double>;
    // using Result = Transform<TL, AddConst>::type;
    // static_assert(std::is_same_v<Result, TypeList<const int, const double>>);
    std::cout << "  Exercise 4: Transform — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Implement `Unique<TypeList<Ts...>>` that
// removes duplicate types:
//   Unique<TypeList<int, double, int, char, double>>::type
//     == TypeList<int, double, char>
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename TL> struct Unique;

void exercise_unique()
{
    // using TL = TypeList<int, double, int, char, double, char>;
    // using U = Unique<TL>::type;
    // static_assert(TL_Size<U>::value == 3);
    // static_assert(std::is_same_v<U, TypeList<int, double, char>>);
    std::cout << "  Exercise 5: Unique — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 09 — Exercises: TMP                    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_is_prime();
    exercise_index_of();
    exercise_reverse();
    exercise_transform();
    exercise_unique();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
