# Stdlib 10 — Memory Management

> **Goal:** Master smart pointers (`unique_ptr`, `shared_ptr`, `weak_ptr`),
> understand ownership semantics, learn custom deleters, and get an
> introduction to allocators and PMR.

---

## Table of Contents

1. [Why Smart Pointers?](#1-why-smart-pointers)
2. [`std::unique_ptr` — Exclusive Ownership](#2-stdunique_ptr--exclusive-ownership)
3. [`std::shared_ptr` — Shared Ownership](#3-stdshared_ptr--shared-ownership)
4. [`std::weak_ptr` — Non-Owning Observer](#4-stdweak_ptr--non-owning-observer)
5. [`make_unique` / `make_shared`](#5-make_unique--make_shared)
6. [Custom Deleters](#6-custom-deleters)
7. [Ownership Guidelines](#7-ownership-guidelines)
8. [Allocators and PMR](#8-allocators-and-pmr)
9. [Common Pitfalls](#9-common-pitfalls)
10. [Exercises](#10-exercises)

---

## 1. Why Smart Pointers?

### The problem with raw pointers

```cpp
void leaky() {
    int* p = new int(42);
    do_something(p);  // what if this throws?
    delete p;         // never reached if exception thrown!
}

void double_delete() {
    int* p = new int(42);
    int* q = p;       // two pointers to same memory
    delete p;
    delete q;         // 💥 undefined behavior!
}
```

Raw pointers have no concept of **ownership**. Who is responsible for
deleting the memory? When multiple pointers reference the same object,
who deletes it? These questions lead to:
- **Memory leaks** (forgot to delete)
- **Double-free** (deleted twice)
- **Use-after-free** (used after delete)
- **Exception-unsafe code** (delete never reached)

### The solution: RAII with smart pointers

Smart pointers **automatically** free memory when they go out of scope.
They embody the **RAII** principle (Resource Acquisition Is Initialization).

---

## 2. `std::unique_ptr` — Exclusive Ownership

### What is it?

`unique_ptr` is a smart pointer that **owns** the object **exclusively**.
When the `unique_ptr` is destroyed, the object is deleted. There can only
be **one** `unique_ptr` pointing to a given object.

### Basic usage

```cpp
#include <memory>

auto p = std::make_unique<int>(42);  // create on heap, p owns it
std::cout << *p;    // 42 — dereference like raw pointer
// p goes out of scope here → memory is freed automatically
```

### Key properties

```cpp
auto p = std::make_unique<int>(42);

// Cannot copy (unique ownership!)
// auto p2 = p;  // ❌ compile error!

// Can move (transfer ownership)
auto p2 = std::move(p);
// p is now nullptr, p2 owns the object

// Check if it owns something
if (p)  std::cout << "p has value\n";    // not printed
if (p2) std::cout << "p2 has value\n";   // printed

// Release ownership (get raw pointer, stop managing)
int* raw = p2.release(); // p2 is now nullptr, YOU must delete raw
delete raw;

// Reset (delete current object, optionally take new one)
auto p3 = std::make_unique<int>(10);
p3.reset();              // deletes 10, p3 is nullptr
p3.reset(new int(20));   // p3 now owns 20
```

### Zero overhead

`unique_ptr` has **zero overhead** compared to a raw pointer (same size,
same speed). The `delete` call is inlined by the compiler. **There is no
reason to ever use `new`/`delete` directly** — always use `unique_ptr`.

### Arrays

```cpp
auto arr = std::make_unique<int[]>(10);  // array of 10 ints
arr[3] = 42;  // operator[] works
// Automatically calls delete[] (not delete)
```

### Polymorphism

```cpp
class Base { public: virtual ~Base() = default; };
class Derived : public Base {};

std::unique_ptr<Base> p = std::make_unique<Derived>();
// Works! Calls Derived destructor through virtual ~Base()
```

---

## 3. `std::shared_ptr` — Shared Ownership

### What is it?

`shared_ptr` allows **multiple pointers to share ownership** of an object.
It keeps a **reference count**. When the last `shared_ptr` is destroyed,
the object is deleted.

### Basic usage

```cpp
auto p1 = std::make_shared<int>(42);
auto p2 = p1;  // copy: both own the object, refcount = 2
auto p3 = p1;  // copy: refcount = 3

std::cout << p1.use_count();  // 3

p2.reset();  // refcount drops to 2
p3.reset();  // refcount drops to 1
// p1 goes out of scope → refcount drops to 0 → object deleted
```

### How the reference count works

```
[Control Block]        [Object]
| refcount: 2  |  →    | 42 |
| weak_count: 0|

shared_ptr p1 ──→ [Control Block]
shared_ptr p2 ──↗
```

Each `shared_ptr` points to a **control block** that holds:
- The strong reference count (number of `shared_ptr`s)
- The weak reference count (number of `weak_ptr`s)
- The deleter function

### When to use `shared_ptr`

- Multiple objects need to reference the same resource
- You don't know at compile time which pointer will be the last one alive
- Graph structures, caches, observer patterns

### Performance cost

`shared_ptr` is NOT zero overhead:
- **Extra allocation** for the control block (unless using `make_shared`)
- **Atomic increment/decrement** of reference count (thread-safe but slower)
- **Larger** than a raw pointer (two pointers: to object + to control block)

---

## 4. `std::weak_ptr` — Non-Owning Observer

### The problem: cyclic references

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;  // 💥 cycle! Memory never freed!
};

auto a = std::make_shared<Node>();
auto b = std::make_shared<Node>();
a->next = b;
b->prev = a;  // a → b → a → b → ... cycle!
// Neither refcount ever reaches 0 → memory leak!
```

### The solution: `weak_ptr` breaks cycles

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;   // ✅ weak_ptr doesn't increase refcount
};
```

### Using `weak_ptr`

```cpp
auto sp = std::make_shared<int>(42);
std::weak_ptr<int> wp = sp;  // doesn't increase refcount

// You can't dereference a weak_ptr directly
// *wp;  // ❌ doesn't compile

// Convert to shared_ptr first (checks if object still alive):
if (auto locked = wp.lock()) {
    std::cout << *locked;  // safe! object is alive
} else {
    std::cout << "object was deleted\n";
}

// Check if the object is still alive:
wp.expired();  // true if object was deleted

sp.reset();    // object deleted
wp.expired();  // true
wp.lock();     // returns empty shared_ptr
```

---

## 5. `make_unique` / `make_shared`

### Why prefer `make_unique` over `new`?

```cpp
// Bad: two separate steps
std::unique_ptr<Widget> p(new Widget(args));

// Good: one expression, exception-safe
auto p = std::make_unique<Widget>(args);
```

`make_unique` is:
1. **Exception-safe** — no leak if another argument throws
2. **No `new` keyword** — clear intent, harder to misuse
3. **DRY** — don't repeat the type

### Why prefer `make_shared`?

```cpp
// This does TWO heap allocations:
std::shared_ptr<Widget> p(new Widget(args));
// 1. new Widget (the object)
// 2. new ControlBlock (the reference count)

// This does ONE heap allocation:
auto p = std::make_shared<Widget>(args);
// Object and control block in one contiguous allocation!
```

**Rule:** Always use `make_unique` and `make_shared` unless you need a
custom deleter.

---

## 6. Custom Deleters

### What if you're not managing `new`'d memory?

```cpp
// File handle — need to call fclose, not delete
auto file = std::unique_ptr<FILE, decltype(&fclose)>(
    fopen("data.txt", "r"),
    fclose
);
// When file goes out of scope, fclose is called automatically

// C library resource
auto handle = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
    SDL_CreateWindow(...),
    SDL_DestroyWindow
);
```

### Lambda deleter

```cpp
auto p = std::unique_ptr<int, std::function<void(int*)>>(
    new int(42),
    [](int* p) {
        std::cout << "Deleting " << *p << "\n";
        delete p;
    }
);
```

---

## 7. Ownership Guidelines

### The ownership hierarchy

```
Raw pointer (T*)        → "I'm just borrowing, I don't own this"
unique_ptr<T>           → "I own this exclusively"
shared_ptr<T>           → "We share ownership"
weak_ptr<T>             → "I'm observing, I don't own this"
```

### Function parameter guidelines

```cpp
void takes_ownership(std::unique_ptr<Widget> w);     // "Give me ownership"
void shares_ownership(std::shared_ptr<Widget> w);    // "I'll keep a copy"
void just_uses(Widget& w);                           // "I just need to use it"
void just_uses(Widget* w);                           // "I just need to use it (nullable)"
void observes(const std::shared_ptr<Widget>& w);     // ❌ Avoid! Just use Widget&
```

### Decision tree

1. Does only **one** thing own it? → `unique_ptr`
2. Do **multiple** things share it? → `shared_ptr`
3. Do you need to **observe** without owning? → `weak_ptr` or raw pointer/reference
4. Do you need a **nullable** non-owning reference? → raw pointer
5. Do you need a **non-nullable** non-owning reference? → reference (`&`)

---

## 8. Allocators and PMR

### What are allocators?

Allocators control **where** memory comes from. By default, containers use
`std::allocator<T>` which calls `new`/`delete`. Custom allocators can use
memory pools, stack memory, shared memory, etc.

### Polymorphic Memory Resources (PMR) — C++17

PMR provides **runtime-polymorphic** allocators (unlike classic allocators
which are compile-time):

```cpp
#include <memory_resource>

// Stack-based buffer (no heap allocation!)
char buffer[1024];
std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

std::pmr::vector<int> v(&pool);
v.push_back(1);  // allocates from buffer, not from heap!
v.push_back(2);
```

### Common PMR resources

| Resource | Behavior |
|----------|---------|
| `monotonic_buffer_resource` | Fast (never deallocates individually) |
| `unsynchronized_pool_resource` | Pool allocator (single-threaded) |
| `synchronized_pool_resource` | Pool allocator (thread-safe) |
| `null_memory_resource` | Always throws (useful for testing) |

---

## 9. Common Pitfalls

### 1. Creating `shared_ptr` from raw pointer twice

```cpp
int* raw = new int(42);
std::shared_ptr<int> p1(raw);
std::shared_ptr<int> p2(raw);  // 💥 Two control blocks! Double-free!
```

### 2. Returning `unique_ptr` by value is fine (move semantics)

```cpp
std::unique_ptr<Widget> create() {
    return std::make_unique<Widget>();  // ✅ implicit move
}
```

### 3. Don't use `shared_ptr` when `unique_ptr` suffices

`shared_ptr` has overhead. If only one owner exists, use `unique_ptr`.
You can always convert `unique_ptr` → `shared_ptr` later if needed.

---

## 10. Exercises

See `exercises.cpp`.

---

**Next lecture:** Utilities & Type Support.
