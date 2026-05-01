# Template 10 — Expression Templates & Tag Dispatch

> **Goal:** Advanced template patterns — expression templates for lazy
> evaluation and eliminating temporaries, tag dispatch for compile-time
> routing, and policy-based design for flexible library architecture.

---

## Table of Contents

1. [Expression Templates](#1-expression-templates)
2. [Tag Dispatch](#2-tag-dispatch)
3. [Policy-Based Design](#3-policy-based-design)
4. [The Detection Idiom](#4-the-detection-idiom)
5. [When to Use These Patterns](#5-when-to-use-these-patterns)
6. [Exercises](#6-exercises)

---

## 1. Expression Templates

### The problem: unnecessary temporaries

Consider a simple vector math library:

```cpp
Vec c = a + b + d;
```

With naive operator overloading, this creates **two temporaries**:

```
Step 1: temp1 = a + b       ← allocates and fills a temporary Vec
Step 2: temp2 = temp1 + d   ← allocates and fills ANOTHER temporary Vec
Step 3: c = temp2            ← copies to c
```

For vectors with millions of elements, those temporaries are expensive.

### The solution: defer evaluation

Expression templates represent the **expression itself** as a lightweight
type, and only compute the result when actually needed (usually at assignment):

```cpp
Vec c = a + b + d;
// Instead of computing immediately:
// 1. operator+ returns AddExpr<Vec, Vec> (no computation!)
// 2. operator+ returns AddExpr<AddExpr<Vec, Vec>, Vec> (still no computation!)
// 3. operator= iterates once: c[i] = a[i] + b[i] + d[i]  ← single loop!
```

### How it works — step by step

**Step 1: Expression node types**

```cpp
template<typename E>
struct VecExpr {
    double operator[](size_t i) const {
        return static_cast<const E&>(*this)[i];
    }
    size_t size() const {
        return static_cast<const E&>(*this).size();
    }
};
```

**Step 2: The concrete vector (a leaf node)**

```cpp
class Vec : public VecExpr<Vec> {
    std::vector<double> data_;
public:
    Vec(size_t n) : data_(n) {}
    double operator[](size_t i) const { return data_[i]; }
    double& operator[](size_t i) { return data_[i]; }
    size_t size() const { return data_.size(); }

    // Assignment from ANY expression — this is where computation happens
    template<typename E>
    Vec& operator=(const VecExpr<E>& expr) {
        const E& e = static_cast<const E&>(expr);
        data_.resize(e.size());
        for (size_t i = 0; i < data_.size(); ++i)
            data_[i] = e[i];  // evaluates the expression tree per-element
        return *this;
    }
};
```

**Step 3: Addition expression (an internal node)**

```cpp
template<typename L, typename R>
class AddExpr : public VecExpr<AddExpr<L, R>> {
    const L& lhs_;
    const R& rhs_;
public:
    AddExpr(const L& l, const R& r) : lhs_(l), rhs_(r) {}
    double operator[](size_t i) const { return lhs_[i] + rhs_[i]; }
    size_t size() const { return lhs_.size(); }
};

template<typename L, typename R>
AddExpr<L, R> operator+(const VecExpr<L>& l, const VecExpr<R>& r) {
    return AddExpr<L, R>(static_cast<const L&>(l), static_cast<const R&>(r));
}
```

**Step 4: Usage**

```cpp
Vec a(1000), b(1000), d(1000);
// ... fill a, b, d ...

Vec c(1000);
c = a + b + d;
// Type of "a + b + d": AddExpr<AddExpr<Vec, Vec>, Vec>
// Only ONE loop, ZERO temporaries
// Equivalent to: for(i) c[i] = a[i] + b[i] + d[i];
```

### Where expression templates are used

- **Eigen** (linear algebra library)
- **Blaze** (math library)
- **Boost.uBLAS**
- Any domain where operator chains create unnecessary temporaries

---

## 2. Tag Dispatch

### What is tag dispatch?

Use **empty tag types** to select different function overloads at compile
time:

```cpp
struct fast_tag {};
struct safe_tag {};

template<typename T>
void sort_impl(std::vector<T>& v, fast_tag) {
    // Quicksort — fast but unstable
    std::sort(v.begin(), v.end());
}

template<typename T>
void sort_impl(std::vector<T>& v, safe_tag) {
    // Mergesort — stable
    std::stable_sort(v.begin(), v.end());
}

template<typename T, typename Tag>
void sort(std::vector<T>& v, Tag tag = fast_tag{}) {
    sort_impl(v, tag);
}
```

### Standard library example: iterator dispatch

The STL uses iterator category tags to select optimal algorithms:

```cpp
// std::advance has different implementations based on iterator type:
template<typename Iter>
void advance_impl(Iter& it, int n, std::input_iterator_tag) {
    while (n-- > 0) ++it;  // O(n) — can only go forward
}

template<typename Iter>
void advance_impl(Iter& it, int n, std::random_access_iterator_tag) {
    it += n;  // O(1) — can jump directly
}

template<typename Iter>
void advance(Iter& it, int n) {
    advance_impl(it, n,
        typename std::iterator_traits<Iter>::iterator_category{});
}
```

### Tag dispatch vs `if constexpr` vs concepts

| Technique | C++ version | Best for |
|-----------|-------------|----------|
| Tag dispatch | C++98 | Classic approach, works everywhere |
| `if constexpr` | C++17 | Single function body with branches |
| Concepts | C++20 | Modern, readable constraints |

```cpp
// Tag dispatch (C++98)
void f(int, fast_tag) { }
void f(int, safe_tag) { }

// if constexpr (C++17)
template<bool Fast>
void f(int x) {
    if constexpr (Fast) { /* fast path */ }
    else { /* safe path */ }
}

// Concepts (C++20)
void f(std::integral auto x) { /* integral path */ }
void f(std::floating_point auto x) { /* float path */ }
```

---

## 3. Policy-Based Design

### What is it?

Template parameters as **interchangeable behavior modules**. The user picks
which "policies" to plug in:

```cpp
template<typename T,
         typename StoragePolicy = HeapStorage<T>,
         typename ThreadPolicy = SingleThreaded>
class Container : private StoragePolicy, private ThreadPolicy {
public:
    void add(const T& val) {
        ThreadPolicy::lock();
        StoragePolicy::store(val);
        ThreadPolicy::unlock();
    }
};
```

### Example policies

```cpp
// Storage policies
template<typename T>
struct HeapStorage {
    std::vector<T> data_;
    void store(const T& val) { data_.push_back(val); }
};

template<typename T>
struct StackStorage {
    std::array<T, 1024> data_;
    size_t size_ = 0;
    void store(const T& val) { data_[size_++] = val; }
};

// Threading policies
struct SingleThreaded {
    void lock() {}
    void unlock() {}
};

struct MultiThreaded {
    std::mutex mtx_;
    void lock() { mtx_.lock(); }
    void unlock() { mtx_.unlock(); }
};

// Mix and match:
Container<int, HeapStorage<int>, SingleThreaded> fast_container;
Container<int, HeapStorage<int>, MultiThreaded> safe_container;
Container<int, StackStorage<int>, SingleThreaded> stack_container;
```

### Why policy-based design?

- **No runtime overhead** — policies are resolved at compile time
- **Combinatorial flexibility** — N storage × M threading = N×M combinations
- **Open for extension** — users can write new policies

### Famous example: `std::allocator`

```cpp
template<typename T, typename Allocator = std::allocator<T>>
class vector { /* ... */ };

// Custom allocator policy:
vector<int, MyPoolAllocator<int>> pool_vec;
```

---

## 4. The Detection Idiom

### What is it?

A standardized SFINAE pattern (pre-concepts) for checking if a type has
a specific feature:

```cpp
// Primary template: detection fails → false
template<typename, template<typename...> class Op, typename... Args>
struct detector : std::false_type {};

// Specialization: detection succeeds → true
template<template<typename...> class Op, typename... Args>
struct detector<std::void_t<Op<Args...>>, Op, Args...> : std::true_type {};

template<template<typename...> class Op, typename... Args>
constexpr bool is_detected_v = detector<void, Op, Args...>::value;
```

### Using it

```cpp
// Define what you're detecting as an alias
template<typename T>
using has_push_back_t = decltype(std::declval<T>().push_back(
    std::declval<typename T::value_type>()));

template<typename T>
using has_size_t = decltype(std::declval<T>().size());

// Check:
static_assert(is_detected_v<has_push_back_t, std::vector<int>>);  // true
static_assert(!is_detected_v<has_push_back_t, std::array<int, 5>>);  // false (no push_back)
static_assert(is_detected_v<has_size_t, std::vector<int>>);  // true
```

### Modern replacement: concepts

```cpp
// Detection idiom (pre-C++20)
template<typename T>
using has_size_t = decltype(std::declval<T>().size());
constexpr bool has_size = is_detected_v<has_size_t, T>;

// Concepts (C++20) — much cleaner
template<typename T>
concept HasSize = requires(T t) { t.size(); };
```

---

## 5. When to Use These Patterns

| Pattern | Use when | Avoid when |
|---------|----------|------------|
| Expression templates | Math-heavy code with operator chains | Simple operations, readability matters |
| Tag dispatch | Pre-C++17 code, need overload resolution | `if constexpr` or concepts available |
| Policy-based design | Library code needing combinatorial flexibility | Simple single-purpose classes |
| Detection idiom | Pre-C++20 SFINAE checks | Concepts available (C++20) |

---

## 6. Exercises

See `exercises.cpp`.

---

**Next lecture:** Perfect Forwarding & Reference Collapsing.
