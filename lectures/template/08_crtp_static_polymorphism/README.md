# Template 08 — CRTP & Static Polymorphism

> **Goal:** Use the Curiously Recurring Template Pattern for compile-time
> polymorphism — zero-overhead interfaces, mixins, and C++23 deducing this.
> Understand **why** CRTP exists, **when** to choose it over virtual dispatch,
> and **how** modern C++ is replacing some of its use cases.

---

## Table of Contents

1. [What Is CRTP?](#1-what-is-crtp)
2. [How CRTP Works](#2-how-crtp-works)
3. [Static vs Virtual Polymorphism](#3-static-vs-virtual-polymorphism)
4. [Mixin Classes](#4-mixin-classes)
5. [CRTP for Static Interfaces](#5-crtp-for-static-interfaces)
6. [Deducing This (C++23)](#6-deducing-this-c23)
7. [When to Use CRTP](#7-when-to-use-crtp)
8. [Exercises](#8-exercises)

---

## 1. What Is CRTP?

### The name

**C**uriously **R**ecurring **T**emplate **P**attern — a class derives from
a base class template, passing **itself** as the template argument:

```cpp
template<typename Derived>
struct Base { };

struct MyClass : Base<MyClass> { };  // ← MyClass passes itself to Base
```

### Why is it "curious"?

Because the base class "knows" the exact derived type at compile time, even
though the derived class hasn't been fully defined yet when the base is
instantiated. This gives the base class **compile-time access** to the
derived class's members.

### The core idea

```cpp
template<typename Derived>
struct Base {
    void interface() {
        // Cast this to the derived type and call its method
        static_cast<Derived*>(this)->implementation();
    }
};

struct Concrete : Base<Concrete> {
    void implementation() { std::cout << "Concrete!\n"; }
};

Concrete c;
c.interface();  // prints "Concrete!" — dispatched at compile time
```

### How is this different from virtual functions?

```cpp
// Virtual: runtime dispatch via vtable pointer
struct VirtualBase {
    virtual void impl() = 0;  // resolved at runtime
};

// CRTP: compile-time dispatch via template
template<typename D>
struct CRTPBase {
    void impl() { static_cast<D*>(this)->impl(); }  // resolved at compile time
};
```

---

## 2. How CRTP Works

### Step by step

1. `Base<Derived>` is a template — the compiler generates a **unique base class**
   for each derived class
2. Inside `Base`, `Derived` is a **complete type name** — you can cast to it
3. `static_cast<Derived*>(this)` is safe because `this` IS a `Derived*`
   (since `Derived` inherits from `Base<Derived>`)
4. The call to `implementation()` is resolved **at compile time** — no vtable

### The cast is safe because...

```
Derived : Base<Derived>
    ↑
    this (in Base) is really pointing to a Derived object
    so static_cast<Derived*>(this) is valid
```

### What happens if you make a mistake?

```cpp
struct A : Base<A> { void impl() { } };
struct B : Base<A> { void impl() { } };  // ❌ Bug! B inherits Base<A>, not Base<B>
// B::interface() would call A::implementation, not B's!
```

This is a classic CRTP pitfall. C++23's deducing this eliminates it.

---

## 3. Static vs Virtual Polymorphism

### Comparison table

| Feature | Virtual (runtime) | CRTP (compile-time) |
|---------|-------------------|---------------------|
| **Dispatch mechanism** | vtable pointer + indirection | `static_cast` + inline |
| **Overhead per object** | 8 bytes (vtable ptr) | 0 bytes |
| **Call overhead** | Indirect branch (cache miss possible) | Direct call (inlineable) |
| **Heterogeneous containers** | ✅ `vector<Base*>` works | ❌ Each derived is a different type |
| **Adding new types** | Open set (any new class can derive) | Closed set (known at compile time) |
| **Binary compatibility** | Stable ABI | Recompile on change |
| **Error messages** | Clear (wrong virtual override) | Cryptic (template errors) |

### When does the performance difference matter?

- **Hot loops** calling polymorphic methods millions of times → CRTP wins
- **Plugin systems** where types are loaded at runtime → virtual wins
- **Small objects** where 8 bytes vtable overhead is significant → CRTP wins
- **Casual code** where simplicity matters → virtual wins

### Benchmark perspective

```cpp
// Virtual: ~2-5ns per call (indirect branch, possible cache miss)
for (auto& shape : shapes)
    shape->area();  // virtual dispatch each time

// CRTP: ~0.5-1ns per call (inlined)
for (auto& shape : shapes)
    shape.area();   // resolved at compile time, likely inlined
```

---

## 4. Mixin Classes

### What are mixins?

CRTP bases that **add functionality** to any derived class:

```cpp
// Mixin: adds print() to any class with to_string()
template<typename Derived>
struct Printable {
    void print() const {
        std::cout << static_cast<const Derived*>(this)->to_string() << "\n";
    }
};

// Mixin: adds operator!= from operator==
template<typename Derived>
struct EqualityComparable {
    bool operator!=(const Derived& other) const {
        return !(static_cast<const Derived*>(this)->operator==(other));
    }
};

// Use: inherit from multiple mixins
struct Point : Printable<Point>, EqualityComparable<Point> {
    int x, y;
    std::string to_string() const { return "(" + std::to_string(x) + "," + std::to_string(y) + ")"; }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

Point p{1, 2};
p.print();           // "prints (1,2)" — from Printable mixin
p != Point{3, 4};    // true — from EqualityComparable mixin
```

### Mixin for counting instances

```cpp
template<typename Derived>
struct InstanceCounter {
    static inline int count = 0;
    InstanceCounter()  { ++count; }
    ~InstanceCounter() { --count; }
    static int instance_count() { return count; }
};

struct Widget : InstanceCounter<Widget> { };
struct Gadget : InstanceCounter<Gadget> { };

Widget w1, w2;
Gadget g1;
Widget::instance_count();  // 2
Gadget::instance_count();  // 1
// Each derived class gets its OWN counter (different template instantiation)
```

### Mixin for fluent interfaces (method chaining)

```cpp
template<typename Derived>
struct FluentBuilder {
    Derived& set_name(std::string n) {
        static_cast<Derived*>(this)->name_ = std::move(n);
        return *static_cast<Derived*>(this);
    }
    Derived& set_value(int v) {
        static_cast<Derived*>(this)->value_ = v;
        return *static_cast<Derived*>(this);
    }
};

struct Config : FluentBuilder<Config> {
    std::string name_;
    int value_ = 0;
};

Config c;
c.set_name("test").set_value(42);  // method chaining returns Config&
```

---

## 5. CRTP for Static Interfaces

### Enforcing an interface at compile time

```cpp
template<typename Derived>
struct Shape {
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
    double perimeter() const {
        return static_cast<const Derived*>(this)->perimeter_impl();
    }
    void draw() const {
        static_cast<const Derived*>(this)->draw_impl();
    }
};

struct Circle : Shape<Circle> {
    double radius;
    double area_impl() const { return 3.14159 * radius * radius; }
    double perimeter_impl() const { return 2 * 3.14159 * radius; }
    void draw_impl() const { std::cout << "Drawing circle\n"; }
};

struct Rectangle : Shape<Rectangle> {
    double width, height;
    double area_impl() const { return width * height; }
    double perimeter_impl() const { return 2 * (width + height); }
    void draw_impl() const { std::cout << "Drawing rectangle\n"; }
};
```

### Using with templates (static dispatch)

```cpp
template<typename T>
void print_shape_info(const Shape<T>& shape) {
    std::cout << "Area: " << shape.area()
              << ", Perimeter: " << shape.perimeter() << "\n";
}

Circle c{5.0};
Rectangle r{3.0, 4.0};
print_shape_info(c);  // Area: 78.5398, Perimeter: 31.4159
print_shape_info(r);  // Area: 12, Perimeter: 14
```

---

## 6. Deducing This (C++23)

### The problem with CRTP

CRTP requires the **derived class to pass itself** as a template argument.
This is error-prone and verbose:

```cpp
struct A : Base<A> { };   // Must remember to pass A
struct B : Base<B> { };   // Must remember to pass B
struct C : Base<A> { };   // ❌ Bug! Should be Base<C>
```

### C++23 "deducing this" replaces CRTP for many use cases

```cpp
struct Base {
    template<typename Self>
    void print(this Self&& self) {  // "this Self&&" is deducing this
        std::cout << self.name() << "\n";
    }
};

struct Derived : Base {
    std::string name() const { return "Derived"; }
};

Derived d;
d.print();  // prints "Derived" — Self is deduced as Derived&
```

### Mixin without CRTP

```cpp
// Old CRTP way
template<typename Derived>
struct OldPrintable {
    void print() const {
        std::cout << static_cast<const Derived*>(this)->to_string();
    }
};
struct OldWidget : OldPrintable<OldWidget> { /* ... */ };

// New deducing-this way (C++23)
struct NewPrintable {
    void print(this auto const& self) {
        std::cout << self.to_string();
    }
};
struct NewWidget : NewPrintable { /* ... */ };
// No template argument needed! Cleaner and less error-prone.
```

### When to still use CRTP

- Pre-C++23 code
- When you need the Derived type at class-definition time (for static members)
- When you need different base class instantiations per derived type

---

## 7. When to Use CRTP

| Use CRTP when... | Don't use CRTP when... |
|------------------|----------------------|
| You need zero-overhead polymorphism | Virtual dispatch is fine for your use case |
| In performance-critical hot paths | Code simplicity matters more |
| Building mixin libraries | A simple base class would work |
| You know all types at compile time | Types are determined at runtime |
| Pre-C++23 static polymorphism | C++23 deducing this is available |

---

## 8. Exercises

See `exercises.cpp`.

---

**Next lecture:** Template Metaprogramming.
