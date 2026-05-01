# Lecture 03 — Memory & Ownership: Move Semantics and Smart Pointers (C++11)

> **Goal:** Understand value categories (lvalue/rvalue), move semantics,
> `std::unique_ptr`, `std::shared_ptr`, and `std::weak_ptr`. After this lecture
> you'll never write `new`/`delete` in application code again.

---

## Table of Contents

1. [The Copy Problem](#1-the-copy-problem)
2. [Value Categories: lvalue vs rvalue](#2-value-categories-lvalue-vs-rvalue)
3. [Move Semantics](#3-move-semantics)
4. [`std::move` — The Cast](#4-stdmove--the-cast)
5. [Rule of Five](#5-rule-of-five)
6. [`std::unique_ptr`](#6-stdunique_ptr)
7. [`std::shared_ptr` and `std::weak_ptr`](#7-stdshared_ptr-and-stdweak_ptr)
8. [Common Pitfalls](#8-common-pitfalls)
9. [Exercises](#9-exercises)
10. [Capstone Mini-Project](#10-capstone-mini-project)
11. [Further Reading](#11-further-reading)

---

## 1. The Copy Problem

Consider a function that returns a large vector:

```cpp
std::vector<int> generate() {
    std::vector<int> result(1'000'000, 42);
    return result;  // Copy 4MB of data? That's slow!
}
```

Before C++11, the compiler might elide the copy (NRVO), but it wasn't
guaranteed. With move semantics, even if elision doesn't kick in, the vector
is **moved** (pointer swap) instead of copied — O(1) instead of O(n).

---

## 2. Value Categories: lvalue vs rvalue

| Category | Definition | Example |
|----------|-----------|---------|
| **lvalue** | Has identity, you can take its address | `x`, `arr[0]`, `*ptr` |
| **rvalue** | Temporary, about to die, can't take its address | `42`, `x + y`, `std::move(x)` |

Why it matters: if something is about to die, we can **steal** its resources
instead of copying them. That's the entire point of move semantics.

```cpp
std::string a = "hello";      // a is an lvalue
std::string b = a;            // copy (a still exists)
std::string c = std::move(a); // move (a is now empty, c owns the data)
```

---

## 3. Move Semantics

A **move constructor** and **move assignment operator** transfer resources:

```cpp
class Buffer {
    int* data_;
    size_t size_;
public:
    // Move constructor — steal the guts
    Buffer(Buffer&& other) noexcept
        : data_(other.data_), size_(other.size_)
    {
        other.data_ = nullptr;  // leave source in valid empty state
        other.size_ = 0;
    }

    // Move assignment
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;            // free old
            data_ = other.data_;       // steal new
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
};
```

Key rules:
- Move constructors/assignments take `T&&` (rvalue reference)
- Always mark them `noexcept` — containers (like `vector`) only use move if
  it's `noexcept`
- Leave the moved-from object in a **valid but unspecified** state

---

## 4. `std::move` — The Cast

`std::move` doesn't move anything. It's just a cast from lvalue to rvalue:

```cpp
template<typename T>
decltype(auto) move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}
```

It says: "I'm done with this object — you may steal from it."

### When to use `std::move`

- Passing a local to a function that can benefit from moving:
  ```cpp
  std::vector<int> data = createData();
  processAndConsume(std::move(data));  // data is empty after this
  ```
- Returning a named local (though NRVO usually handles this)
- Moving into containers: `vec.push_back(std::move(expensive_obj));`

### When NOT to use `std::move`

- On `const` objects — `const T&&` is useless; the move ctor can't steal
  from something it can't modify
- On return values — `return std::move(x)` **prevents** NRVO! Just `return x;`
- After the move — accessing a moved-from object is usually a bug

---

## 5. Rule of Five

If you define **any** of these, define (or `= default` / `= delete`) **all five**:

1. Destructor
2. Copy constructor
3. Copy assignment operator
4. Move constructor
5. Move assignment operator

```cpp
class Resource {
public:
    Resource();
    ~Resource();
    Resource(const Resource& other);             // copy ctor
    Resource& operator=(const Resource& other);  // copy assign
    Resource(Resource&& other) noexcept;         // move ctor
    Resource& operator=(Resource&& other) noexcept; // move assign
};
```

**Rule of Zero:** If your class only holds value types and smart pointers,
you need NONE of the five — the compiler-generated ones do the right thing.

---

## 6. `std::unique_ptr`

### What is it?

An owning pointer that **cannot be copied**, only moved. When it goes out of
scope, it deletes the object.

```cpp
#include <memory>

auto p = std::make_unique<Widget>(42);  // C++14
// or: std::unique_ptr<Widget> p(new Widget(42));  // C++11

p->doWork();         // use like raw pointer
Widget& ref = *p;    // dereference

auto q = std::move(p);  // transfer ownership
// p is now nullptr
```

### Factory pattern with unique_ptr

```cpp
std::unique_ptr<Shape> createShape(const std::string& type) {
    if (type == "circle") return std::make_unique<Circle>(5.0);
    if (type == "rect")   return std::make_unique<Rectangle>(3.0, 4.0);
    return nullptr;
}
```

### Custom deleters

```cpp
auto file = std::unique_ptr<FILE, decltype(&fclose)>(
    fopen("data.txt", "r"), &fclose);
```

---

## 7. `std::shared_ptr` and `std::weak_ptr`

### `shared_ptr` — reference-counted ownership

Multiple `shared_ptr` instances can own the same object. The object is
destroyed when the last `shared_ptr` goes away.

```cpp
auto sp1 = std::make_shared<Widget>(42);  // ref count = 1
auto sp2 = sp1;                           // ref count = 2
sp1.reset();                              // ref count = 1
// sp2 still valid
```

### `weak_ptr` — non-owning observer

Breaks circular references. Doesn't keep the object alive.

```cpp
std::weak_ptr<Widget> wp = sp2;     // doesn't increment ref count
if (auto locked = wp.lock()) {      // try to get shared_ptr
    locked->doWork();               // safe — object is alive
}
```

### When to use what

| Pointer type | Ownership | Copy? | Use case |
|-------------|-----------|-------|----------|
| `unique_ptr` | Exclusive | No (move only) | Default choice. Factories, containers. |
| `shared_ptr` | Shared | Yes | Multiple owners. Caches, graphs. |
| `weak_ptr` | None (observer) | Yes | Break cycles, optional back-references. |
| Raw pointer (`T*`) | None (non-owning) | Yes | Function params that don't own. |

---

## 8. Common Pitfalls

### 1. Using `std::move` on const

```cpp
const std::string s = "hello";
auto t = std::move(s);  // This COPIES! Can't steal from const.
```

### 2. Using object after move

```cpp
auto v = std::vector<int>{1, 2, 3};
auto w = std::move(v);
v.size();  // Technically valid (guaranteed to be in a valid state) but v is empty
v[0];      // UB if v is empty!
```

### 3. shared_ptr cycles

```cpp
struct Node {
    std::shared_ptr<Node> next;  // If A→B and B→A → memory leak!
    // Fix: std::weak_ptr<Node> next;
};
```

### 4. make_shared vs new

```cpp
// BAD: two allocations (object + control block)
std::shared_ptr<Widget> p(new Widget(42));

// GOOD: one allocation (object + control block together)
auto p = std::make_shared<Widget>(42);
```

---

## 9. Exercises

See `exercises.cpp` in this folder:

1. Implement a `Buffer` class with proper move semantics (Rule of Five)
2. Write a factory that returns `unique_ptr` to polymorphic objects
3. Build a `shared_ptr`-based tree and verify reference counts
4. Detect and fix a circular reference bug using `weak_ptr`
5. Challenge: Implement a simple `unique_ptr` from scratch

---

## 10. Capstone Mini-Project

**"Resource Pool"**

Build a generic object pool (`Pool<T>`) that:
- Pre-allocates N objects
- `acquire()` returns a `unique_ptr` with a custom deleter that returns
  the object to the pool instead of destroying it
- `shared_acquire()` returns a `shared_ptr` with the same custom deleter
- Demonstrates zero copies — all transfers via move
- Thread-safe bonus: protect the pool with a mutex

---

## 11. Further Reading

| Topic | Resource |
|-------|----------|
| Move semantics deep dive | Howard Hinnant's "Everything about move semantics" |
| Value categories | cppreference.com/w/cpp/language/value_category |
| Smart pointers | Herb Sutter's GotW #89, #90, #91 |
| Custom deleters | *Effective Modern C++* Item 18-22 |

**Next lecture:** Compile-Time Power — constexpr, static_assert, variadic
templates, and threading fundamentals.
