# Stdlib 11 — Utilities & Type Support

> **Goal:** Master vocabulary types (`optional`, `variant`, `any`, `expected`),
> `tuple`/`pair`, type traits, and functional utilities. These are the
> building blocks you'll use in almost every C++ program.

---

## Table of Contents

1. [`std::optional` — Nullable Value](#1-stdoptional--nullable-value)
2. [`std::variant` — Type-Safe Union](#2-stdvariant--type-safe-union)
3. [`std::any` — Type-Erased Value](#3-stdany--type-erased-value)
4. [`std::expected` (C++23) — Value or Error](#4-stdexpected-c23--value-or-error)
5. [`std::pair` and `std::tuple`](#5-stdpair-and-stdtuple)
6. [Structured Bindings](#6-structured-bindings)
7. [Type Traits](#7-type-traits)
8. [Functional Utilities](#8-functional-utilities)
9. [Exercises](#9-exercises)

---

## 1. `std::optional` — Nullable Value

### What is it?

`std::optional<T>` is a wrapper that either contains a value of type `T` or
is empty (contains nothing). It's a **type-safe replacement** for using
sentinel values, output parameters, or raw pointers to indicate "no value."

### The problem it solves

```cpp
// Bad: using -1 as "not found" — what if -1 is a valid value?
int find_index(const std::vector<int>& v, int target) {
    for (int i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;
    return -1;  // magic sentinel value
}

// Bad: output parameter — ugly and error-prone
bool find_index(const std::vector<int>& v, int target, int& out) {
    for (int i = 0; i < v.size(); ++i)
        if (v[i] == target) { out = i; return true; }
    return false;
}
```

### The solution

```cpp
#include <optional>

std::optional<int> find_index(const std::vector<int>& v, int target) {
    for (int i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;     // ← wraps in optional
    return std::nullopt;                  // ← "no value"
}
```

### Using optional

```cpp
auto result = find_index(v, 42);

// Method 1: Check with has_value() or bool conversion
if (result.has_value())
    std::cout << "Found at " << result.value() << "\n";

if (result)                    // same as has_value()
    std::cout << "Found at " << *result << "\n";  // dereference

// Method 2: value_or() — default if empty
int idx = result.value_or(-1); // -1 if not found

// Method 3: value() throws std::bad_optional_access if empty
try { auto v = result.value(); }
catch (const std::bad_optional_access&) { ... }
```

### When to use `optional`

- Function return type: "this might not have a result"
- Struct member: "this field might not be set yet"
- **NOT** for pointers: `optional<T*>` is almost always wrong — use `T*` (nullptr is already "no value")

---

## 2. `std::variant` — Type-Safe Union

### What is it?

`std::variant<T1, T2, ...>` holds **exactly one** value from a fixed set
of types. It's a **type-safe union** — unlike C unions, it tracks which
type is currently active and prevents invalid access.

### Basic usage

```cpp
#include <variant>

std::variant<int, double, std::string> v;
v = 42;                    // holds int
v = 3.14;                  // now holds double
v = "hello"s;              // now holds string

// Get the value (throws bad_variant_access if wrong type):
std::get<double>(v);       // ❌ throws! v holds string
std::get<std::string>(v);  // "hello"

// Safe access:
auto* p = std::get_if<int>(&v);  // nullptr if v doesn't hold int
if (p) std::cout << *p;

// Check which type is active:
v.index();  // 2 (0=int, 1=double, 2=string)
std::holds_alternative<std::string>(v);  // true
```

### `std::visit` — the power of variant

```cpp
std::variant<int, double, std::string> v = 42;

// Visit with a visitor (overloaded function):
std::visit([](auto&& arg) {
    std::cout << arg << "\n";
}, v);

// Visit with overloaded lambdas (using overload pattern):
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

std::visit(overloaded{
    [](int i)                { std::cout << "int: " << i << "\n"; },
    [](double d)             { std::cout << "double: " << d << "\n"; },
    [](const std::string& s) { std::cout << "string: " << s << "\n"; },
}, v);
```

### When to use `variant`

- **Closed set of types** known at compile time
- Replacement for `enum` + `union` patterns
- **State machines** (each state has different data)
- **AST nodes** (expression = literal | binary_op | unary_op)
- Alternative to inheritance when you don't need extensibility

### `variant` vs inheritance

| Feature | `variant` | Inheritance |
|---------|----------|-------------|
| Types known at compile time? | ✅ Required | ❌ Extensible |
| No heap allocation? | ✅ Stack-allocated | ❌ Usually heap (`unique_ptr`) |
| Adding new types? | ❌ Must update all visitors | ✅ Just add new subclass |
| Adding new operations? | ✅ Just add new visitor | ❌ Must update all classes |
| Performance | ✅ No vtable, no indirection | ❌ Virtual dispatch overhead |

---

## 3. `std::any` — Type-Erased Value

### What is it?

`std::any` can hold a value of **any type**. It's like `void*` but type-safe.

```cpp
#include <any>

std::any a = 42;
std::any b = std::string("hello");
std::any c = 3.14;

// Retrieve with any_cast (throws bad_any_cast if wrong type):
int val = std::any_cast<int>(a);       // 42
auto& s = std::any_cast<std::string&>(b); // "hello"

a.type().name();  // implementation-defined type name
a.has_value();    // true
a.reset();        // clear
```

### When to use `any`

- Plugin systems where types are not known at compile time
- Property maps (`map<string, any>`)
- **Use sparingly** — `variant` is almost always better when the types are known

---

## 4. `std::expected` (C++23) — Value or Error

### What is it?

`std::expected<T, E>` holds either a **success value** of type `T` or an
**error value** of type `E`. It's the modern C++ way to handle errors
without exceptions.

### The problem it solves

```cpp
// Bad: how to return an error?
int parse_int(std::string_view s) {
    // ... what if s is invalid? throw? return -1? output param?
}
```

### The solution

```cpp
#include <expected>

std::expected<int, std::string> parse_int(std::string_view s) {
    int value;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{})
        return std::unexpected("invalid number");
    return value;
}

// Usage:
auto result = parse_int("42");
if (result) {
    std::cout << result.value() << "\n";  // 42
} else {
    std::cerr << result.error() << "\n";  // error message
}

// Or with value_or:
int val = result.value_or(0);
```

### `expected` vs exceptions vs error codes

| Approach | Pros | Cons |
|----------|------|------|
| Exceptions | Automatic propagation | Invisible control flow, slow path |
| Error codes | Fast, explicit | Easy to ignore, no type safety |
| `expected<T,E>` | Fast, explicit, type-safe | Must check, no auto-propagation |
| `optional<T>` | Simple | No error info (just "failed") |

---

## 5. `std::pair` and `std::tuple`

### `std::pair` — two values bundled together

```cpp
#include <utility>

std::pair<std::string, int> p{"Alice", 25};
p.first;   // "Alice"
p.second;  // 25

// Make a pair:
auto p2 = std::make_pair("Bob", 30);
```

### `std::tuple` — N values bundled together

```cpp
#include <tuple>

std::tuple<int, double, std::string> t{1, 3.14, "hello"};
std::get<0>(t);  // 1
std::get<1>(t);  // 3.14
std::get<2>(t);  // "hello"

auto t2 = std::make_tuple(42, "world");
```

---

## 6. Structured Bindings

C++17 lets you unpack `pair`, `tuple`, and structs:

```cpp
// Pair
auto [name, age] = std::make_pair("Alice", 25);

// Tuple
auto [x, y, z] = std::make_tuple(1, 2.0, "three");

// Map iteration
std::map<std::string, int> m{{"a", 1}, {"b", 2}};
for (auto& [key, value] : m)
    std::cout << key << ": " << value << "\n";

// Struct
struct Point { int x; int y; };
Point p{10, 20};
auto [px, py] = p;  // px = 10, py = 20
```

---

## 7. Type Traits

### What are they?

Type traits are **compile-time queries** about types. They live in `<type_traits>`.

### Common type traits

```cpp
#include <type_traits>

// Check properties:
std::is_integral_v<int>;          // true
std::is_floating_point_v<double>; // true
std::is_same_v<int, int>;        // true
std::is_same_v<int, long>;       // false
std::is_pointer_v<int*>;         // true
std::is_const_v<const int>;      // true
std::is_reference_v<int&>;       // true

// Transform types:
std::remove_const_t<const int>;     // int
std::remove_reference_t<int&>;      // int
std::decay_t<const int&>;           // int (removes ref + const + array→ptr)
std::conditional_t<true, int, double>; // int (compile-time if/else)
std::common_type_t<int, double>;    // double
```

### Why use type traits?

1. **Constrain templates** (before concepts):
   ```cpp
   template<typename T>
   std::enable_if_t<std::is_integral_v<T>, T> square(T x) { return x * x; }
   ```

2. **Static assertions**:
   ```cpp
   static_assert(std::is_trivially_copyable_v<MyStruct>,
                 "MyStruct must be trivially copyable for memcpy");
   ```

3. **`if constexpr` branching**:
   ```cpp
   template<typename T>
   void process(T val) {
       if constexpr (std::is_integral_v<T>)
           std::cout << "integer: " << val;
       else
           std::cout << "other: " << val;
   }
   ```

---

## 8. Functional Utilities

### `std::function` — type-erased callable wrapper

```cpp
#include <functional>

std::function<int(int, int)> op;
op = [](int a, int b) { return a + b; };   // lambda
op(3, 4);  // 7

op = std::minus<>{};  // function object
op(10, 3); // 7
```

**When to use:** Callbacks, strategy pattern, storing heterogeneous callables.
**Performance note:** `std::function` may allocate heap memory. For templates,
prefer `auto` or concepts.

### `std::invoke` — call anything uniformly

```cpp
std::invoke(func, args...);           // regular function
std::invoke(&Class::method, obj, args...); // member function
std::invoke(&Class::member, obj);     // member variable access
```

### `std::bind_front` (C++20) — partial application

```cpp
auto add5 = std::bind_front(std::plus<>{}, 5);
add5(3);  // 8

// Replaces the awkward std::bind:
// auto old = std::bind(std::plus<>{}, 5, std::placeholders::_1);
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** I/O, Filesystem & Chrono.
