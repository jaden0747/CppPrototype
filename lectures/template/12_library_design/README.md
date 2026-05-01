# Template 12 — Library Design Patterns

> **Goal:** Apply templates to real-world library design — type erasure,
> compile-time strings, template-based serialization, and understanding
> how `std::ranges` works internally. Learn **why** each pattern exists,
> **when** to use it, and **how** to implement it correctly.

---

## Table of Contents

1. [Type Erasure](#1-type-erasure)
2. [Compile-Time String Processing](#2-compile-time-string-processing)
3. [Template-Based Serialization](#3-template-based-serialization)
4. [How Ranges Work Internally](#4-how-ranges-work-internally)
5. [Library Design Guidelines](#5-library-design-guidelines)
6. [Exercises](#6-exercises)

---

## 1. Type Erasure

### The problem

Templates give you compile-time polymorphism, but you lose the ability to
store different types in the same container:

```cpp
template<typename T>
void draw(const T& shape);

// ✅ Works with any shape type
draw(Circle{5});
draw(Rectangle{3, 4});

// ❌ But you can't store them together!
std::vector<???> shapes;  // What type goes here?
```

Virtual functions let you store different types together, but they require
inheritance and vtables.

### Type erasure: best of both worlds

Type erasure gives you a **value-semantic** type that can hold any object
satisfying a concept, **without** requiring inheritance in the public API:

```cpp
AnyDrawable shape1 = Circle{5};      // stores a Circle
AnyDrawable shape2 = Rectangle{3,4}; // stores a Rectangle
std::vector<AnyDrawable> shapes = {shape1, shape2};  // ✅ Works!
```

### How it works — the three parts

1. **Concept** (internal base class with virtual functions)
2. **Model** (template that wraps any concrete type)
3. **Wrapper** (public value type that owns a Concept pointer)

```cpp
class AnyCallable {
    // 1. CONCEPT — internal interface (hidden from users)
    struct Concept {
        virtual ~Concept() = default;
        virtual int call(int) = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    // 2. MODEL — wraps any type that satisfies the interface
    template<typename F>
    struct Model : Concept {
        F func_;
        Model(F f) : func_(std::move(f)) {}
        int call(int x) override { return func_(x); }
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model>(func_);
        }
    };

    // 3. WRAPPER — value-semantic public type
    std::unique_ptr<Concept> impl_;

public:
    // Constructor: accepts ANY callable (lambda, function pointer, functor)
    template<typename F>
    AnyCallable(F f) : impl_(std::make_unique<Model<F>>(std::move(f))) {}

    // Copy constructor
    AnyCallable(const AnyCallable& other)
        : impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

    // Call operator
    int operator()(int x) { return impl_->call(x); }
};

// Usage:
AnyCallable f = [](int x) { return x * 2; };
AnyCallable g = [](int x) { return x + 10; };
std::cout << f(5) << "\n";  // 10
std::cout << g(5) << "\n";  // 15
```

### Where type erasure is used in the standard library

| Type | Erases |
|------|--------|
| `std::function<R(Args...)>` | Any callable with signature R(Args...) |
| `std::any` | Any copyable type |
| `std::move_only_function` | Any move-only callable (C++23) |
| `std::format_args` | Format arguments of any type |

### Small buffer optimization (SBO)

Real implementations avoid heap allocation for small objects:

```cpp
class AnyCallable {
    static constexpr size_t BufSize = 32;
    alignas(std::max_align_t) char buffer_[BufSize];
    Concept* impl_;  // points into buffer_ for small objects, heap for large

    template<typename F>
    void construct(F&& f) {
        if constexpr (sizeof(Model<F>) <= BufSize) {
            impl_ = new (buffer_) Model<F>(std::forward<F>(f));  // placement new
        } else {
            impl_ = new Model<F>(std::forward<F>(f));  // heap allocation
        }
    }
};
```

---

## 2. Compile-Time String Processing

### Fixed strings as template parameters (C++20)

Before C++20, you couldn't pass strings as template arguments. Now you can
with a structural type:

```cpp
template<size_t N>
struct FixedString {
    char data[N]{};

    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    constexpr auto operator<=>(const FixedString&) const = default;
};

// Deduction guide: FixedString("hello") → FixedString<6>
template<size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;
```

### Using fixed strings as non-type template parameters

```cpp
template<FixedString Name>
struct NamedValue {
    static void print() { std::cout << Name.data << "\n"; }
};

NamedValue<"hello">::print();  // prints "hello"
NamedValue<"world">::print();  // prints "world"
```

### Use case: compile-time format strings

```cpp
template<FixedString Fmt>
void checked_print(auto... args) {
    // Count % in format string at compile time
    constexpr size_t expected = /* count '%' in Fmt */;
    static_assert(sizeof...(args) == expected,
                  "Wrong number of format arguments");
    // ...
}
```

### Use case: named struct fields

```cpp
template<FixedString Name, typename T>
struct Field {
    static constexpr auto name = Name;
    T value;
};

auto f = Field<"age", int>{.value = 25};
std::cout << f.name.data << " = " << f.value << "\n";  // "age = 25"
```

---

## 3. Template-Based Serialization

### The pattern: specialize a Serializer for each type

```cpp
// Primary template (unimplemented — forces specialization)
template<typename T>
struct Serializer {
    static_assert(sizeof(T) == 0, "No serializer for this type");
};

// Specialization for int
template<>
struct Serializer<int> {
    static void write(std::ostream& os, int v) {
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }
    static int read(std::istream& is) {
        int v;
        is.read(reinterpret_cast<char*>(&v), sizeof(v));
        return v;
    }
};

// Specialization for std::string
template<>
struct Serializer<std::string> {
    static void write(std::ostream& os, const std::string& s) {
        auto size = static_cast<uint32_t>(s.size());
        os.write(reinterpret_cast<const char*>(&size), sizeof(size));
        os.write(s.data(), s.size());
    }
    static std::string read(std::istream& is) {
        uint32_t size;
        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::string s(size, '\0');
        is.read(s.data(), size);
        return s;
    }
};
```

### Generic serialize/deserialize functions

```cpp
template<typename T>
void serialize(std::ostream& os, const T& value) {
    Serializer<T>::write(os, value);
}

template<typename T>
T deserialize(std::istream& is) {
    return Serializer<T>::read(is);
}

// Usage:
serialize(file, 42);
serialize(file, std::string("hello"));
auto x = deserialize<int>(file);
auto s = deserialize<std::string>(file);
```

### Extending for containers

```cpp
template<typename T>
struct Serializer<std::vector<T>> {
    static void write(std::ostream& os, const std::vector<T>& v) {
        auto size = static_cast<uint32_t>(v.size());
        Serializer<uint32_t>::write(os, size);
        for (const auto& elem : v)
            Serializer<T>::write(os, elem);
    }
    static std::vector<T> read(std::istream& is) {
        auto size = Serializer<uint32_t>::read(is);
        std::vector<T> v;
        v.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
            v.push_back(Serializer<T>::read(is));
        return v;
    }
};
```

---

## 4. How Ranges Work Internally

### The architecture

Ranges are built from several layered template patterns:

### 1. Concepts define requirements

```cpp
template<typename T>
concept range = requires(T& t) {
    std::ranges::begin(t);
    std::ranges::end(t);
};

template<typename T>
concept view = range<T> && std::movable<T> && /* lightweight */;
```

### 2. View adaptors are lazy template types

```cpp
// views::transform returns a transform_view
auto result = v | std::views::transform([](int x) { return x * 2; });
// Type: transform_view<ref_view<vector<int>>, lambda>
// NO computation happens yet!
```

### 3. Pipe operator chains adaptors

```cpp
// Each | creates a new view wrapping the previous one
auto pipeline = v
    | std::views::filter([](int x) { return x > 0; })
    | std::views::transform([](int x) { return x * 2; })
    | std::views::take(5);

// Type: take_view<transform_view<filter_view<ref_view<vector<int>>, ...>, ...>, ...>
// Still no computation! Just a chain of lightweight view objects.

// Computation happens when you iterate:
for (int x : pipeline) { }  // NOW elements are processed one at a time
```

### 4. `view_interface` uses CRTP

```cpp
template<typename Derived>
class view_interface {
public:
    bool empty() requires requires { std::ranges::begin(derived()); } {
        return std::ranges::begin(derived()) == std::ranges::end(derived());
    }
    auto front() { return *std::ranges::begin(derived()); }
    // ... more convenience methods
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
};
```

---

## 5. Library Design Guidelines

### When to use each pattern

| Pattern | Best for |
|---------|----------|
| Type erasure | Value-semantic polymorphism, avoiding inheritance in public API |
| Compile-time strings | Configuration, named types, format validation |
| Template serialization | Extensible binary formats, protocol buffers |
| Range-style views | Lazy, composable data pipelines |

### General principles

1. **Prefer value semantics** — type erasure over raw pointers/inheritance
2. **Make illegal states unrepresentable** — use concepts/static_assert
3. **Lazy is better** — defer computation (expression templates, ranges)
4. **Extension by specialization** — let users add support for their types
5. **Don't over-template** — only parameterize what actually varies

---

## 6. Exercises

See `exercises.cpp`.

---

**Next lecture:** Debugging & Best Practices.
