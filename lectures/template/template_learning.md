# C++ Templates — Complete Learning Plan

A structured roadmap from basic templates to advanced template metaprogramming. Each phase builds on the previous one.

---

## How to Use This Plan

- ✅ Check off topics as you complete them
- 🕐 Estimated total: 10–14 weeks at ~1–2 hrs/day
- 💡 Write and compile code for every topic — templates are best learned by doing
- 🔁 Templates are cumulative — revisit earlier phases often
- ⚠️ Compiler errors in template code can be long and cryptic. Learn to read them from the bottom up.

---

## Phase 1 — Foundations (2 weeks)

> If you can write a function and a class, you can start writing templates.

### Week 1 — Function & Class Templates

| # | Topic | Key Idea |
|---|-------|----------|
| 1 | Function templates | Write one function that works with any type |
| 2 | Template argument deduction | Compiler figures out `T` from the arguments you pass |
| 3 | Explicit template arguments | `max<double>(1, 2.5)` — when deduction isn't enough |
| 4 | Class templates | Parameterize an entire class over types |
| 5 | Member function templates | A class's method can have its own template parameters |
| 6 | Default template arguments | `template<typename T = int>` |

**Example — function template:**

```cpp
template<typename T>
T max_of(T a, T b) {
    return (a > b) ? a : b;
}
// Usage: max_of(3, 7);  max_of(1.5, 2.3);
```

**Example — class template:**

```cpp
template<typename T, int N>
struct FixedArray {
    T data[N];
    T& operator[](int i) { return data[i]; }
    constexpr int size() const { return N; }
};
// Usage: FixedArray<double, 10> arr;
```

**Practice project:** Implement a generic `Stack<T>` class with `push`, `pop`, and `top`.

---

### Week 2 — Non-Type Parameters & Specialization

| # | Topic | Key Idea |
|---|-------|----------|
| 7 | Non-type template parameters | `template<int N>` — values as template arguments |
| 8 | Template specialization (full) | Provide a completely different implementation for a specific type |
| 9 | Partial specialization | Specialize on patterns (e.g., all pointer types) |
| 10 | Function template overloading | Overloading interacts with specialization — know the rules |

**Example — specialization:**

```cpp
// Primary template
template<typename T>
struct Serializer {
    static std::string to_string(const T& val) { return std::to_string(val); }
};

// Full specialization for std::string
template<>
struct Serializer<std::string> {
    static std::string to_string(const std::string& val) { return val; }
};

// Partial specialization for all pointer types
template<typename T>
struct Serializer<T*> {
    static std::string to_string(T* ptr) {
        return ptr ? Serializer<T>::to_string(*ptr) : "null";
    }
};
```

**Practice project:** Write a `Formatter<T>` class that specializes output for `int`, `double`, `std::string`, and pointer types.

---

## Phase 2 — Intermediate Techniques (3 weeks)

> This is where templates start feeling powerful. You'll use these techniques in real libraries.

### Week 3 — Variadic Templates

| # | Topic | Key Idea |
|---|-------|----------|
| 11 | Parameter packs (`typename... Args`) | Accept any number of template arguments |
| 12 | Pack expansion (`args...`) | Expand a pack into function arguments, initializer lists, etc. |
| 13 | Recursive variadic functions | Process one argument at a time, recurse on the rest |
| 14 | Fold expressions (C++17) | `(args + ...)` — collapse a pack with an operator |
| 15 | `sizeof...` operator | Get the number of elements in a pack |

**Example — variadic print:**

```cpp
// C++17 fold expression version
template<typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}
// Usage: print("hello", 42, 3.14);

// Recursive version (C++11 compatible)
void print_r() {}  // base case

template<typename T, typename... Rest>
void print_r(T first, Rest... rest) {
    std::cout << first << " ";
    print_r(rest...);
}
```

**Practice project:** Implement a type-safe `make_vector(args...)` function that creates a vector from its arguments.

---

### Week 4 — SFINAE & Type Traits

| # | Topic | Key Idea |
|---|-------|----------|
| 16 | `std::enable_if` | Conditionally enable/disable a template based on a type property |
| 17 | SFINAE (Substitution Failure Is Not An Error) | Failed substitution silently removes a candidate — not an error |
| 18 | Type traits (`<type_traits>`) | Query properties of types at compile time |
| 19 | `std::is_integral`, `std::is_floating_point`, etc. | Built-in type checks |
| 20 | `std::conditional` | Choose a type based on a compile-time condition |
| 21 | `std::decay`, `std::remove_reference` | Strip qualifiers for clean type manipulation |
| 22 | `decltype` and `declval` | Deduce the type of an expression without evaluating it |

**Example — SFINAE with enable_if:**

```cpp
// Only enabled for integral types
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
safe_divide(T a, T b) {
    if (b == 0) throw std::runtime_error("division by zero");
    return a / b;
}

// Only enabled for floating-point types
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>
safe_divide(T a, T b) {
    return a / b;  // floating-point handles inf
}
```

**Practice project:** Write a `to_string<T>` function that uses SFINAE to choose different implementations for integers, floats, strings, and containers.

---

### Week 5 — Template Aliases, Variables & Lambdas

| # | Topic | Key Idea |
|---|-------|----------|
| 23 | Template aliases (`template<T> using ...`) | Simplify complex type expressions |
| 24 | Variable templates (C++14) | `template<T> constexpr T pi = ...` |
| 25 | Generic lambdas as templates (C++14/20) | `auto` params make lambdas into templates |
| 26 | `if constexpr` (C++17) | Compile-time branching inside templates |
| 27 | Template lambdas with explicit params (C++20) | `[]<typename T>(T x) { ... }` |

**Example — if constexpr replacing SFINAE:**

```cpp
template<typename T>
std::string stringify(const T& val) {
    if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(val);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return val;
    } else {
        return "[unknown type]";
    }
}
```

**Practice project:** Create a `TypeInfo<T>` alias template and variable template that stores type name strings and sizes.

---

## Phase 3 — Concepts & Constraints (C++20) (2 weeks)

> Concepts replace SFINAE with readable, named constraints. This is the modern way.

### Week 6 — Defining & Using Concepts

| # | Topic | Key Idea |
|---|-------|----------|
| 28 | `concept` keyword | Name a set of requirements on a type |
| 29 | `requires` clause | Attach a concept to a template |
| 30 | `requires` expression | Test if expressions are valid for a type |
| 31 | Compound requirements | Check return type and noexcept in one expression |
| 32 | Nested requirements | Combine multiple constraints |

**Example — custom concept:**

```cpp
template<typename T>
concept Printable = requires(T val, std::ostream& os) {
    { os << val } -> std::same_as<std::ostream&>;
};

template<Printable T>
void log(const T& val) {
    std::cout << "[LOG] " << val << "\n";
}
```

---

### Week 7 — Standard Concepts & Constrained Templates

| # | Topic | Key Idea |
|---|-------|----------|
| 33 | Standard library concepts (`<concepts>`) | `std::integral`, `std::copyable`, `std::invocable`, etc. |
| 34 | Concept subsumption & overload resolution | More constrained overloads are preferred |
| 35 | Abbreviated function templates | `void f(std::integral auto x)` — shorthand |
| 36 | Constraining class templates | Apply concepts to entire classes |
| 37 | Concepts vs SFINAE — migration patterns | How to modernize old SFINAE code |

**Practice project:** Refactor your Phase 2 SFINAE code to use concepts. Write a `Container` concept that requires `begin()`, `end()`, `size()`, and `push_back()`.

---

## Phase 4 — Advanced Patterns (3–4 weeks)

> Real-world libraries (Boost, ranges, Eigen) use these patterns heavily.

### Week 8 — CRTP & Static Polymorphism

| # | Topic | Key Idea |
|---|-------|----------|
| 38 | CRTP (Curiously Recurring Template Pattern) | A class derives from a template instantiated with itself |
| 39 | Static polymorphism vs virtual functions | Compile-time dispatch — zero overhead |
| 40 | Mixin pattern | Add reusable behavior via templates |
| 41 | Deducing `this` (C++23) | Replaces many CRTP use cases with cleaner syntax |

**Example — CRTP:**

```cpp
template<typename Derived>
struct Comparable {
    bool operator!=(const Derived& other) const {
        return !(static_cast<const Derived&>(*this) == other);
    }
    bool operator>(const Derived& other) const {
        return other < static_cast<const Derived&>(*this);
    }
};

struct Point : Comparable<Point> {
    int x, y;
    bool operator==(const Point& o) const { return x==o.x && y==o.y; }
    bool operator<(const Point& o) const { return x<o.x || (x==o.x && y<o.y); }
};
// Point automatically gets != and > from CRTP base
```

---

### Week 9 — Template Metaprogramming (TMP)

| # | Topic | Key Idea |
|---|-------|----------|
| 42 | Compile-time computation with templates | Templates as a functional language |
| 43 | Recursive type computations | `Factorial<N>::value` pattern |
| 44 | Type lists | `TypeList<int, double, char>` — manipulate lists of types |
| 45 | `std::integral_constant` | Wrap compile-time values as types |
| 46 | `constexpr` vs TMP | When to use modern constexpr instead of template tricks |

**Example — compile-time Fibonacci:**

```cpp
// Old TMP style
template<int N>
struct Fib {
    static constexpr int value = Fib<N-1>::value + Fib<N-2>::value;
};
template<> struct Fib<0> { static constexpr int value = 0; };
template<> struct Fib<1> { static constexpr int value = 1; };

// Modern constexpr (preferred)
constexpr int fib(int n) {
    if (n < 2) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) { int t = a+b; a = b; b = t; }
    return b;
}
```

---

### Week 10 — Expression Templates & Tag Dispatch

| # | Topic | Key Idea |
|---|-------|----------|
| 47 | Expression templates | Build expression trees at compile time — defer evaluation |
| 48 | Tag dispatch | Use empty tag types to select overloads |
| 49 | Policy-based design | Parameterize behavior with template policies |
| 50 | Detecting idiom (`std::void_t`, `is_detected`) | Generic way to check if a type has a member |

**Example — tag dispatch:**

```cpp
struct random_access_tag {};
struct forward_tag {};

template<typename Iter>
void advance_impl(Iter& it, int n, random_access_tag) {
    it += n;  // O(1)
}

template<typename Iter>
void advance_impl(Iter& it, int n, forward_tag) {
    for (int i = 0; i < n; ++i) ++it;  // O(n)
}
```

**Practice project:** Implement a small linear algebra `Vec<T, N>` class that uses expression templates to avoid temporary objects in `a + b + c`.

---

### Week 11 — Perfect Forwarding & Reference Collapsing

| # | Topic | Key Idea |
|---|-------|----------|
| 51 | Forwarding references (`T&&` in templates) | Not an rvalue reference — it's universal |
| 52 | Reference collapsing rules | `T& &&` → `T&`, `T&& &&` → `T&&` |
| 53 | `std::forward<T>` | Pass lvalues as lvalues, rvalues as rvalues |
| 54 | Writing factory functions (`make_unique`, `emplace`) | Perfect forwarding in practice |
| 55 | Common pitfalls (auto&& vs T&&, dangling refs) | Know the traps |

**Example — perfect forwarding:**

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
// Forwards each argument as lvalue or rvalue correctly
```

---

## Phase 5 — Real-World Application (2 weeks)

> Put it all together. Build things that use multiple patterns.

### Week 12 — Library Design Patterns

| # | Topic | Key Idea |
|---|-------|----------|
| 56 | Type erasure (`std::function`, `std::any`) | Hide template types behind a uniform interface |
| 57 | Compile-time string processing | `constexpr` + templates for format strings |
| 58 | Template-based serialization | Generic read/write for any struct |
| 59 | Ranges and views internals | How ranges use templates, concepts, and CRTP |

---

### Week 13 — Debugging & Best Practices

| # | Topic | Key Idea |
|---|-------|----------|
| 60 | Reading template error messages | Strategies for GCC, Clang, and MSVC |
| 61 | Reducing compile times | Extern templates, explicit instantiation |
| 62 | Testing template code | Static asserts, concept checks, type-level unit tests |
| 63 | When NOT to use templates | Simpler alternatives (virtual functions, std::variant, std::function) |

---

## Capstone Projects

| After Phase | Project |
|-------------|---------|
| 1 — Foundations | Generic `HashMap<K, V>` with customizable hash policy |
| 2 — Intermediate | A type-safe `printf` using variadic templates and SFINAE |
| 3 — Concepts | A `Serializable` concept + generic JSON serializer |
| 4 — Advanced | A small expression-template matrix math library |
| 5 — Real-World | A plugin system using type erasure and policy-based design |

---

## Recommended Resources

| Resource | Best For |
|----------|----------|
| *C++ Templates: The Complete Guide* — Vandevoorde, Josuttis, Gregor | The definitive template book (2nd edition covers C++17) |
| *Template Metaprogramming with C++* — Marius Bancila | Modern TMP with C++20 concepts |
| [cppreference.com — Templates](https://en.cppreference.com/w/cpp/language/templates) | Authoritative reference |
| [Compiler Explorer (godbolt.org)](https://godbolt.org) | See template instantiations and assembly |
| [C++ Weekly — Jason Turner](https://www.youtube.com/@cppweekly) | Bite-sized template topics |
| [CppCon talks on YouTube](https://www.youtube.com/@CppCon) | Deep dives from template experts |

---

## Quick Reference — Templates Evolution by Standard

| Standard | Template Features Added |
|----------|------------------------|
| C++98/03 | Function/class templates, specialization, SFINAE |
| C++11 | Variadic templates, extern templates, template aliases, decltype |
| C++14 | Variable templates, generic lambdas, return type deduction |
| C++17 | CTAD, fold expressions, if constexpr, std::void_t |
| C++20 | Concepts, requires, abbreviated templates, constexpr containers |
| C++23 | Deducing this, static operator(), explicit object parameters |

---

*Generated with Claude · Master the templates, master C++! 🔧*