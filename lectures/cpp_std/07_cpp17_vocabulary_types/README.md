# Lecture 07 — C++17 Vocabulary Types

> **Goal:** Master `std::optional`, `std::variant`, `std::any`, and
> `std::string_view` — the four vocabulary types that eliminate raw pointers,
> unions, void*, and unnecessary string copies. Understand **why** each
> exists, **when** to use it, and **how** to avoid common pitfalls.

---

## Table of Contents

1. [`std::optional<T>`](#1-stdoptionalt)
2. [`std::variant<Types...>`](#2-stdvarianttypes)
3. [`std::any`](#3-stdany)
4. [`std::string_view`](#4-stdstring_view)
5. [Choosing the Right Type](#5-choosing-the-right-type)
6. [Exercises](#6-exercises)

---

## 1. `std::optional<T>`

### What is it?

A type that either holds a value of type `T` or holds **nothing** (empty).

### What problem does it solve?

Before `optional`, signaling "no value" was error-prone:

```cpp
// ❌ Sentinel value: what if -1 IS a valid result?
int find_index(const std::vector<int>& v, int target) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;
    return -1;  // magic sentinel
}

// ❌ Output parameter: confusing API, easy to forget the bool check
bool find_index(const std::vector<int>& v, int target, size_t& out);

// ❌ Pointer: who owns the memory? can it be null?
int* find_value(const std::vector<int>& v, int target);
```

### The solution

```cpp
#include <optional>

std::optional<size_t> find_index(const std::vector<int>& v, int target) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;  // implicitly wraps in optional
    return std::nullopt;                // "no value"
}

auto idx = find_index(vec, 42);
if (idx) {                    // or idx.has_value()
    std::cout << *idx;        // dereference to get value
}
int val = idx.value_or(-1);   // safe default
```

### Key operations

| Operation | Description | Notes |
|-----------|-------------|-------|
| `opt.has_value()` | Returns true if contains value | Same as `bool(opt)` |
| `*opt` | Access value directly | **UB if empty!** |
| `opt.value()` | Access value with check | Throws `bad_optional_access` if empty |
| `opt.value_or(def)` | Value or fallback | Returns copy, not reference |
| `opt.emplace(args...)` | Construct in-place | Destroys previous value first |
| `opt.reset()` | Make empty | Calls T destructor |
| `opt = std::nullopt` | Clear | Same as reset() |

### Monadic operations (C++23)

```cpp
// C++23 adds functional-style chaining:
auto result = find_index(vec, 42)
    .transform([](size_t i) { return i * 2; })     // map the value
    .and_then([](size_t i) -> std::optional<std::string> {  // flatmap
        if (i < 100) return std::to_string(i);
        return std::nullopt;
    })
    .or_else([] { return std::optional<std::string>("default"); });
```

### When to use optional

| Use optional | Don't use optional |
|--------------|-------------------|
| Function may not return a value | Value is always present |
| Replace sentinel values (-1, nullptr) | Performance-critical tight loops |
| Optional configuration parameters | Large objects (optional adds overhead) |

---

## 2. `std::variant<Types...>`

### What is it?

A **type-safe union** that holds exactly one value from a set of types.

### What problem does it solve?

```cpp
// ❌ C union: no type safety, no destructors called
union Bad { int i; double d; std::string s; };  // UB with string!

// ❌ void*: no type information, no safety
void* data = new int(42);  // what type is it? who knows!
```

### The solution

```cpp
#include <variant>

std::variant<int, double, std::string> v;
v = 42;         // holds int
v = 3.14;       // now holds double
v = "hello"s;   // now holds string
```

### Accessing the value

```cpp
// 1. std::get<Type> — throws if wrong type
std::string s = std::get<std::string>(v);  // OK
// std::get<int>(v);  // throws bad_variant_access!

// 2. std::get<Index> — by position
auto& s = std::get<2>(v);  // index 2 = string

// 3. std::get_if — returns pointer (nullptr if wrong)
if (auto* p = std::get_if<std::string>(&v)) {
    std::cout << *p;  // safe!
}

// 4. Check which type is active
v.index();                           // 0, 1, or 2
std::holds_alternative<int>(v);      // false (currently string)
```

### `std::visit` — the recommended way to use variant

```cpp
// Generic visitor — auto deduces the active type
std::visit([](auto&& arg) {
    std::cout << arg << "\n";
}, v);
```

### The "overloaded" pattern — type-specific handling

```cpp
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::visit(overloaded{
    [](int i)            { std::cout << "int: " << i << "\n"; },
    [](double d)         { std::cout << "dbl: " << d << "\n"; },
    [](const std::string& s) { std::cout << "str: " << s << "\n"; },
}, v);
```

### Real-world use case: expression tree

```cpp
struct Literal { double value; };
struct Add;
struct Multiply;

using Expr = std::variant<
    Literal,
    std::unique_ptr<Add>,
    std::unique_ptr<Multiply>
>;

struct Add      { Expr left, right; };
struct Multiply { Expr left, right; };
```

### `valueless_by_exception`

A variant can become empty if assignment throws:

```cpp
struct ThrowOnCopy {
    ThrowOnCopy(const ThrowOnCopy&) { throw std::runtime_error("oops"); }
};
std::variant<int, ThrowOnCopy> v = 42;
try { v = ThrowOnCopy{}; } catch(...) {}
v.valueless_by_exception();  // true — variant is in invalid state
```

---

## 3. `std::any`

### What is it?

A type-safe container for **any** single copyable value. Like `void*` but safe.

```cpp
#include <any>

std::any a = 42;
a = std::string("hello");
a = 3.14;

double d = std::any_cast<double>(a);   // OK
// int i = std::any_cast<int>(a);      // throws bad_any_cast!

if (a.type() == typeid(double)) { /* ... */ }
a.has_value();  // true
a.reset();      // clear
```

### When to use `any` (rarely!)

| Use `any` | Use `variant` instead |
|-----------|----------------------|
| Plugin systems | Known set of types |
| Completely dynamic property bags | Config with known types |
| Interop with scripting languages | Application-level unions |

**Rule of thumb:** If you know the possible types, use `variant`. Only use
`any` when types are truly open-ended.

---

## 4. `std::string_view`

### What is it?

A **non-owning** view into a contiguous character sequence. Zero-copy
read access to any string-like data.

### The problem it solves

```cpp
// This copies the string just to read it!
void process(const std::string& s);
process("hello");  // creates a temporary std::string from the literal

// With string_view — zero copies:
void process(std::string_view sv);
process("hello");                    // no allocation!
process(std::string("world"));      // no copy!
process(std::string_view("data", 4)); // from raw buffer
```

### What does string_view contain?

Just two things: a **pointer** and a **length**. No heap allocation, no
ownership.

```
string_view = { const char* data, size_t length }
```

### Available operations

Most `std::string` read-only operations work:

```cpp
std::string_view sv = "Hello, World!";
sv.size();            // 13
sv.empty();           // false
sv[0];                // 'H'
sv.substr(7, 5);      // "World" (returns another string_view!)
sv.find("World");     // 7
sv.starts_with("He"); // true (C++20)
sv.ends_with("!");    // true (C++20)
sv.remove_prefix(7);  // sv is now "World!"
sv.remove_suffix(1);  // sv is now "World"
```

### Lifetime danger!

`string_view` does **not** own the data. If the source dies, the view dangles:

```cpp
// ❌ DANGLING — string dies at semicolon
std::string_view bad() {
    std::string s = "temporary";
    return s;  // s destroyed here! view dangles!
}

// ❌ DANGLING — temporary dies at semicolon
std::string_view sv = std::string("temp");
// The temporary string is destroyed. sv is dangling!

// ✅ OK — string literal has static lifetime
std::string_view ok = "hello";  // string literals live forever
```

### Best practices

| Do | Don't |
|----|-------|
| Use as function parameters | Return from functions (return `string` instead) |
| View string literals | Store longer than the source lives |
| View existing `string` objects | Create from temporary strings |
| Replace `const char*` parameters | Use where ownership transfer is needed |

---

## 5. Choosing the Right Type

| Need | Use | Why |
|------|-----|-----|
| Maybe no value | `optional<T>` | Explicit "empty" state |
| One of several known types | `variant<Ts...>` | Type-safe, zero overhead |
| Any type at all | `any` | Fully dynamic (use sparingly) |
| Read-only string access | `string_view` | Zero-copy, fast |
| Nullable pointer | `optional<T&>` or raw `T*` | `optional<T&>` not in standard yet; use pointer |

---

## 6. Exercises

See `exercises.cpp`:

1. Rewrite functions using `optional` instead of sentinel values
2. Build a `variant`-based expression tree (int | double | string)
3. Implement a property bag using `std::map<string, any>`
4. Replace `const std::string&` parameters with `string_view`
5. Implement the overloaded visitor pattern from scratch
6. Chain optional operations using monadic style (C++23)

---

**Next lecture:** C++17 Templates & Filesystem — `if constexpr`, template
auto, and the `<filesystem>` library.
