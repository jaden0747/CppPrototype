# Lecture 08 — C++17 Templates & Filesystem

> **Goal:** Master advanced C++17 template features (`if constexpr`,
> `template<auto>`, type trait upgrades) and the `<filesystem>` library for
> portable file operations. Understand **how** each feature eliminates old
> patterns, **when** to prefer one approach over another, and the common
> pitfalls to avoid.

---

## Table of Contents

1. [`if constexpr` — Deep Dive](#1-if-constexpr--deep-dive)
2. [`template<auto>` — Non-Type Template Parameters](#2-templateauto--non-type-template-parameters)
3. [Type Traits Upgrades](#3-type-traits-upgrades)
4. [`std::filesystem`](#4-stdfilesystem)
5. [`std::apply` and `std::invoke`](#5-stdapply-and-stdinvoke)
6. [Exercises](#6-exercises)

---

## 1. `if constexpr` — Deep Dive

### What exactly happens at compile time?

When the compiler sees `if constexpr (condition)`:
1. Evaluate `condition` as a compile-time boolean
2. **Discard** the false branch entirely — it won't even be type-checked
3. The true branch becomes the only code that exists

This is fundamentally different from runtime `if`: both branches of a regular
`if` must compile for every template instantiation.

### Pattern 1: Recursive variadic processing (replaces recursion base case)

```cpp
// Before C++17: needed a base case overload
template<typename T>
void print_all(T last) { std::cout << last; }

template<typename T, typename... Rest>
void print_all(T first, Rest... rest) {
    std::cout << first << ", ";
    print_all(rest...);
}

// C++17: single function, no base case overload needed
template<typename T, typename... Rest>
void print_all(T first, Rest... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0) {
        std::cout << ", ";
        print_all(rest...);  // only instantiated when rest is non-empty
    }
}
```

**Why is this better?** One fewer function overload to maintain. The logic is
in one place. You can read the algorithm top-to-bottom.

### Pattern 2: SFINAE replacement

```cpp
// Before: two overloads + SFINAE
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string to_str(T val) { return std::to_string(val); }

template<typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
std::string to_str(T val) { return std::string(val); }

// After: one function, clear branching
template<typename T>
std::string to_str(T val) {
    if constexpr (std::is_integral_v<T>)
        return std::to_string(val);
    else if constexpr (std::is_floating_point_v<T>)
        return std::to_string(val);
    else
        return std::string(val);
}
```

### Pattern 3: Tag dispatch elimination

```cpp
// OLD: separate function per tag
template<typename Iter>
void advance_impl(Iter& it, int n, std::random_access_iterator_tag) { it += n; }
template<typename Iter>
void advance_impl(Iter& it, int n, std::input_iterator_tag) {
    while(n--) ++it;
}
template<typename Iter>
void advance(Iter& it, int n) {
    advance_impl(it, n, typename std::iterator_traits<Iter>::iterator_category{});
}

// NEW: single function, constexpr if
template<typename Iter>
void advance(Iter& it, int n) {
    using category = typename std::iterator_traits<Iter>::iterator_category;
    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>)
        it += n;
    else if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, category>) {
        if (n > 0) while (n--) ++it;
        else while (n++) --it;
    } else
        while (n--) ++it;
}
```

### Common mistake: condition must be dependent

```cpp
template<typename T>
void foo() {
    if constexpr (sizeof(int) == 4) { /* always true on most platforms */ }
    // This is fine — condition CAN be non-dependent (just unusual)

    if constexpr (false) {
        static_assert(false);  // ❌ Still ill-formed! static_assert(false) is always checked
    }
    if constexpr (false) {
        static_assert(sizeof(T) == 0);  // ✅ OK: depends on T, discarded
    }
}
```

---

## 2. `template<auto>` — Non-Type Template Parameters

### What is it?

Let the compiler deduce BOTH the type and value of a non-type template parameter:

```cpp
template<auto Value>
struct Constant {
    static constexpr auto value = Value;
    using type = decltype(Value);
};

Constant<42> ci;       // type = int, value = 42
Constant<'x'> cc;     // type = char, value = 'x'
Constant<true> cb;     // type = bool, value = true
Constant<nullptr> cn;  // type = std::nullptr_t
```

### Why does this matter?

Before C++17, you had to specify the type explicitly:

```cpp
// C++14: must write the type even though it's redundant
template<int N> struct OldConstant {};
template<char C> struct OldChar {};
// Can't write a SINGLE template that works with any non-type parameter

// C++17: one template handles all value types
template<auto V> struct Constant {};
```

### Use case: function pointers as template parameters

```cpp
template<auto Func>
decltype(auto) call() { return Func(); }

int answer() { return 42; }
double pi() { return 3.14; }

auto r1 = call<answer>();  // int 42
auto r2 = call<pi>();      // double 3.14
```

### Use case: heterogeneous compile-time values

```cpp
template<auto... Values>
struct ValueList {};

using mixed = ValueList<1, 'a', true, 42L>;
// Each value can be a different type!
```

### Use case: compile-time counters and indices

```cpp
template<auto N>
constexpr auto doubled = N * 2;

static_assert(doubled<21> == 42);
static_assert(doubled<3.14> == 6.28);  // works with double too!
```

---

## 3. Type Traits Upgrades

### The `_v` suffix: less boilerplate

C++17 adds `_v` variable templates for ALL type traits:

```cpp
// Before (C++11/14): verbose
if (std::is_integral<T>::value && std::is_same<T, U>::value) { ... }

// After (C++17): concise
if (std::is_integral_v<T> && std::is_same_v<T, U>) { ... }
```

### New traits in C++17

| Trait | What it checks |
|-------|---------------|
| `std::is_aggregate_v<T>` | Is T an aggregate type? |
| `std::has_unique_object_representations_v<T>` | Can T be memcmp'd safely? |
| `std::is_invocable_v<F, Args...>` | Can F be called with Args? |
| `std::is_invocable_r_v<R, F, Args...>` | Same + returns R? |
| `std::invoke_result_t<F, Args...>` | What does F(Args...) return? |
| `std::conjunction_v<Traits...>` | Short-circuit AND |
| `std::disjunction_v<Traits...>` | Short-circuit OR |
| `std::negation_v<Trait>` | Logical NOT |

### `std::conjunction` / `std::disjunction` — short-circuit logic

```cpp
// WITHOUT conjunction: all traits instantiated regardless
template<typename... Args>
std::enable_if_t<(std::is_integral_v<Args> && ...)>
ints_only(Args... args);
// With 100 args, all 100 is_integral are evaluated even if first fails

// WITH conjunction: stops at first failure
template<typename... Args>
std::enable_if_t<std::conjunction_v<std::is_integral<Args>...>>
ints_only(Args... args);
// Stops instantiating traits at the first non-integral type
```

### `std::invoke_result_t` replaces `std::result_of`

```cpp
// Deprecated (C++17):
typename std::result_of<F(Args...)>::type

// Use instead:
std::invoke_result_t<F, Args...>
```

---

## 4. `std::filesystem`

### What is it?

A portable library for file and directory operations that works across
Windows, Linux, and macOS without platform-specific code.

### Why was it needed?

Before C++17:
- `<dirent.h>` on POSIX, `FindFirstFile` on Windows
- Path separators differ (`/` vs `\`)
- Boost.Filesystem was the only portable option

### Paths — the foundation

```cpp
#include <filesystem>
namespace fs = std::filesystem;

fs::path p = "/usr/local/bin/app.exe";
p.filename();     // "app.exe"
p.stem();         // "app"
p.extension();    // ".exe"
p.parent_path();  // "/usr/local/bin"
p.root_path();    // "/"
p.is_absolute();  // true
p.is_relative();  // false
```

### Path composition with `/` operator

```cpp
fs::path base = "/home/user";
auto full = base / "documents" / "file.txt";
// → "/home/user/documents/file.txt"

// Works on Windows too:
fs::path win = "C:\\Users";
auto winFull = win / "phong" / "Desktop";
// → "C:\Users\phong\Desktop"
```

### File queries

```cpp
fs::exists(p);              // does it exist?
fs::is_regular_file(p);     // is it a file?
fs::is_directory(p);        // is it a directory?
fs::is_symlink(p);          // is it a symlink?
fs::file_size(p);           // size in bytes (uintmax_t)
fs::last_write_time(p);     // modification time
```

### Directory iteration

```cpp
// Non-recursive: just immediate children
for (const auto& entry : fs::directory_iterator("/tmp")) {
    std::cout << entry.path().filename() << "\n";
}

// Recursive: all descendants
for (const auto& entry : fs::recursive_directory_iterator(".")) {
    if (entry.is_regular_file() && entry.path().extension() == ".cpp")
        std::cout << entry.path() << "\n";
}
```

### File manipulation

```cpp
fs::create_directory("output");           // one level
fs::create_directories("a/b/c/d");        // all levels
fs::copy("src.txt", "dst.txt");           // copy file
fs::copy("dir1", "dir2", fs::copy_options::recursive); // copy tree
fs::rename("old.txt", "new.txt");         // rename/move
fs::remove("file.txt");                   // delete file
fs::remove_all("temp_dir");              // delete directory tree
```

### Error handling: two styles

```cpp
// Style 1: exceptions (default)
try {
    auto size = fs::file_size("nonexistent.txt");
} catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << "\n";
}

// Style 2: error codes (non-throwing)
std::error_code ec;
auto size = fs::file_size("nonexistent.txt", ec);
if (ec) {
    std::cerr << ec.message() << "\n";
}
```

**When to use which?** Use error codes in performance-sensitive code or when
file-not-found is expected. Use exceptions when errors are truly exceptional.

---

## 5. `std::apply` and `std::invoke`

### `std::apply` — unpack a tuple into function arguments

```cpp
#include <tuple>

auto args = std::make_tuple(1, 2.0, "hello");
auto result = std::apply([](int a, double b, const char* c) {
    return std::string(c) + " " + std::to_string(a);
}, args);
// result = "hello 1"
```

**Why?** Without `std::apply`, unpacking a tuple requires index tricks:

```cpp
// Manual approach (ugly!):
template<typename F, typename Tuple, std::size_t... I>
auto apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return f(std::get<I>(std::forward<Tuple>(t))...);
}
```

### `std::invoke` — uniformly call anything

The problem: calling member functions requires different syntax than free functions:

```cpp
struct Foo {
    int x;
    int bar(int y) { return x + y; }
};
Foo f{10};

// Regular function: func(args)
// Member function: (obj.*func)(args)  or  (ptr->*func)(args)
// Member variable: obj.*var

// std::invoke handles ALL of these uniformly:
std::invoke(&Foo::bar, f, 5);   // 15 (member function)
std::invoke(&Foo::x, f);        // 10 (member variable access)
std::invoke([](int a) { return a * 2; }, 21); // 42 (lambda)
std::invoke(std::plus{}, 3, 4); // 7 (functor)
```

### Real-world use: generic callback storage

```cpp
template<typename F, typename... Args>
auto delayed_call(F&& f, Args&&... args) {
    return [f = std::forward<F>(f),
            ...args = std::forward<Args>(args)]() mutable {
        return std::invoke(f, args...);
    };
}

auto callback = delayed_call(&Foo::bar, f, 5);
callback();  // 15
```

---

## 6. Exercises

See `exercises.cpp`:

1. Use `if constexpr` to write a universal `serialize()` function
2. Create a `Constant<auto>` template and demonstrate type deduction
3. Use `std::conjunction` to constrain a variadic function
4. Walk a directory tree and collect statistics (file count, total size)
5. Use `std::apply` to forward tuple contents to a function
6. Implement a generic callback system using `std::invoke`

---

**Next lecture:** C++20 Concepts — constraining templates elegantly.
