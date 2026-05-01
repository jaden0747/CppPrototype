# Template 01 — Function & Class Templates

> **Goal:** Write your first templates — functions and classes parameterized
> over types. Understand argument deduction, explicit arguments, and defaults.
> Learn **why** templates exist and **how** the compiler turns them into real code.

---

## Table of Contents

1. [What Are Templates and Why Do They Exist?](#1-what-are-templates-and-why-do-they-exist)
2. [Function Templates](#2-function-templates)
3. [Template Argument Deduction](#3-template-argument-deduction)
4. [Explicit Template Arguments](#4-explicit-template-arguments)
5. [Class Templates](#5-class-templates)
6. [Class Template Argument Deduction (CTAD)](#6-class-template-argument-deduction-ctad)
7. [Member Function Templates](#7-member-function-templates)
8. [Default Template Arguments](#8-default-template-arguments)
9. [Where Do Templates Go? (Headers)](#9-where-do-templates-go)
10. [Common Mistakes](#10-common-mistakes)
11. [Exercises](#11-exercises)

---

## 1. What Are Templates and Why Do They Exist?

### The problem: code duplication

Without templates, you'd write the same logic for every type:

```cpp
int    max_int(int a, int b)       { return (a > b) ? a : b; }
double max_double(double a, double b) { return (a > b) ? a : b; }
float  max_float(float a, float b)  { return (a > b) ? a : b; }
// ... for every type that supports >
```

That's the **same logic** copy-pasted with different types. If you find a bug,
you must fix it in every copy.

### The solution: templates

Templates let you write the logic **once** and let the compiler fill in the type:

```cpp
template<typename T>
T max_of(T a, T b) {
    return (a > b) ? a : b;
}

max_of(3, 7);       // compiler generates max_of<int>
max_of(1.5, 2.3);   // compiler generates max_of<double>
max_of('a', 'z');    // compiler generates max_of<char>
```

### The mental model: templates are blueprints

Think of a template as a **cookie cutter**. The template is the shape;
the type you pass is the dough. The compiler stamps out a new function
(or class) for each type you use.

```
Template:  max_of<T>(T a, T b)
                 ↓
            ┌─────────────────────┐
    T=int   │ max_of<int>(int, int)      │
    T=double│ max_of<double>(double, double)│
    T=string│ max_of<string>(string, string)│
            └─────────────────────┘
```

Each generated function is called an **instantiation**.

### Key insight: templates are NOT compiled until used

The template itself is just a pattern. The compiler only generates code
when you actually call `max_of<int>()`. If you never use a particular type,
no code is generated for it.

---

## 2. Function Templates

### Syntax

```cpp
template<typename T>   // T is the template parameter
T max_of(T a, T b) {  // T is used as a type
    return (a > b) ? a : b;
}
```

- `template<typename T>` declares a template with one type parameter
- `typename` and `class` are interchangeable here (but `typename` is preferred)
- `T` is a placeholder — it can be any valid type name

### Multiple template parameters

```cpp
template<typename T, typename U>
auto add(T a, U b) {
    return a + b;  // return type deduced by compiler (C++14)
}

add(1, 2.5);    // T=int, U=double, returns double
add(1.0f, 2L);  // T=float, U=long, returns float? Actually: double
```

### How the compiler generates code

When you write `max_of(3, 7)`:

1. Compiler sees `max_of` is a template
2. Deduces `T = int` from the arguments
3. Generates `int max_of(int a, int b) { return (a > b) ? a : b; }`
4. Compiles the generated function

Each unique `T` produces a **separate function** in the binary. This is
called **code bloat** if you use many types — but it's usually worth it
because the functions are fully optimized for each type.

---

## 3. Template Argument Deduction

### The compiler is smart

Usually you don't need to specify `<T>` — the compiler deduces it:

```cpp
template<typename T>
void show(T value);

show(42);          // T = int (deduced from 42)
show(3.14);        // T = double
show("hello");     // T = const char* (string literal decays to pointer)
show(std::string("hello")); // T = std::string
```

### Deduction with references

How the parameter is declared affects deduction:

```cpp
template<typename T>
void f1(T x);         // T deduced as value type (copies argument)

template<typename T>
void f2(T& x);        // T deduced without reference (x is T&)

template<typename T>
void f3(const T& x);  // T deduced without const/ref (x is const T&)

int i = 42;
f1(i);   // T = int, x is a copy
f2(i);   // T = int, x is int& (reference to i)
f3(i);   // T = int, x is const int& (read-only reference)
```

### When deduction fails

```cpp
template<typename T>
T max_of(T a, T b);

max_of(1, 2.5);  // ❌ ERROR: T deduced as both int and double!
// Fix 1: explicit template argument
max_of<double>(1, 2.5);

// Fix 2: use two template parameters
template<typename T, typename U>
auto max_of(T a, U b) { return (a > b) ? a : b; }
```

---

## 4. Explicit Template Arguments

### When deduction can't work

Sometimes the return type can't be deduced from arguments:

```cpp
template<typename R, typename T>
R convert(T value) {
    return static_cast<R>(value);
}

auto x = convert<double>(42);  // R=double (explicit), T=int (deduced)
```

### Specifying all arguments

```cpp
auto result = max_of<double>(1, 2.5);  // force T=double
```

---

## 5. Class Templates

### What is it?

A class template parameterizes an entire class — all members use the type parameter:

```cpp
template<typename T>
class Stack {
    std::vector<T> data_;
public:
    void push(const T& val) { data_.push_back(val); }
    void pop()              { data_.pop_back(); }
    const T& top() const    { return data_.back(); }
    bool empty() const      { return data_.empty(); }
    size_t size() const     { return data_.size(); }
};

Stack<int> si;            // Stack of ints
si.push(1);
si.push(2);

Stack<std::string> ss;    // Stack of strings
ss.push("hello");
```

### Multiple template parameters

```cpp
template<typename K, typename V>
class Pair {
public:
    K first;
    V second;
    Pair(K k, V v) : first(std::move(k)), second(std::move(v)) {}
};

Pair<std::string, int> p("Alice", 25);
```

### How class templates differ from function templates

| Feature | Function template | Class template |
|---------|------------------|---------------|
| Deduction | Automatic from arguments | Requires CTAD (C++17) or explicit |
| Specialization | Full only | Full and partial |
| Where to define | Usually header | Usually header |

---

## 6. Class Template Argument Deduction (CTAD)

### Before C++17: must specify types

```cpp
std::pair<int, double> p(1, 2.5);    // verbose!
std::vector<int> v{1, 2, 3};          // must specify int
```

### C++17: compiler deduces from constructor arguments

```cpp
std::pair p(1, 2.5);                  // deduces pair<int, double>
std::vector v{1, 2, 3};              // deduces vector<int>
std::tuple t(1, 2.0, "hi");          // deduces tuple<int, double, const char*>
```

### CTAD for your own classes

```cpp
template<typename T>
class Stack {
    std::vector<T> data_;
public:
    Stack() = default;
    Stack(std::initializer_list<T> init) : data_(init) {}
};

Stack s{1, 2, 3};  // deduces Stack<int> — requires deduction guide or
                    // the constructor to make it obvious
```

### Writing deduction guides

```cpp
template<typename T>
Stack(std::initializer_list<T>) -> Stack<T>;
// Now Stack{1.0, 2.0} deduces Stack<double>
```

---

## 7. Member Function Templates

### Non-template class with template member

```cpp
class Printer {
public:
    template<typename T>
    void print(const T& value) {
        std::cout << value << "\n";
    }
};

Printer p;
p.print(42);       // instantiates print<int>
p.print("hello");  // instantiates print<const char*>
```

### Class template with additional template members

```cpp
template<typename T>
class Container {
    std::vector<T> data_;
public:
    // Conversion constructor: Container<int> from Container<double>
    template<typename U>
    Container(const Container<U>& other) {
        for (const auto& elem : other)
            data_.push_back(static_cast<T>(elem));
    }
};
```

---

## 8. Default Template Arguments

### For class templates

```cpp
template<typename T = int, typename Allocator = std::allocator<T>>
class MyVector { /* ... */ };

MyVector<> v1;                   // T=int, default allocator
MyVector<double> v2;             // T=double, default allocator
MyVector<int, MyAlloc<int>> v3;  // both explicit
```

### For function templates (C++11)

```cpp
template<typename T = double>
T zero() { return T{}; }

zero();        // T=double, returns 0.0
zero<int>();   // T=int, returns 0
```

---

## 9. Where Do Templates Go?

### The header-only rule

Template code must be **visible to the compiler at every use site**. This
means templates almost always go in **header files** (`.h`/`.hpp`):

```cpp
// mystack.hpp
template<typename T>
class Stack {
    std::vector<T> data_;
public:
    void push(const T& val) { data_.push_back(val); }
    // ... all definitions here in the header
};
```

### Why?

When `main.cpp` writes `Stack<int>`, the compiler needs to see the full
`Stack` definition to generate the `int` version. If the definition is in
a `.cpp` file, the compiler can't see it → **linker error**.

### Exceptions

- **Explicit instantiation** (`template class Stack<int>;` in a .cpp) forces
  the compiler to generate code there — but you must predict all needed types
- **Modules** (C++20) may change this in the future

---

## 10. Common Mistakes

### 1. Forgetting that templates are in headers

```
// stack.cpp — ❌ WRONG
template<typename T>
void Stack<T>::push(const T& val) { ... }
// Linker error: "undefined reference to Stack<int>::push"
```

### 2. Mixing `typename` and `class` (no difference)

Both `template<typename T>` and `template<class T>` mean exactly the same thing.
Convention: prefer `typename`.

### 3. Deduction ambiguity

```cpp
template<typename T>
T max_of(T a, T b);
max_of(1, 2.5);  // Error: T can't be both int and double
```

---

## 11. Exercises

See `exercises.cpp`.

zero();       // returns 0.0 (double)
zero<int>();  // returns 0 (int)
```

---

## 7. Exercises

See `exercises.cpp`.

---

## 8. Capstone Mini-Project

**"Generic Stack with Iterator"**

Build a `Stack<T>` that:
- Has push, pop, top, empty, size
- Supports iteration (begin/end) for range-for
- Has a `transform(Func)` member template that returns `Stack<U>` where
  `U` is the return type of `Func(T)`
- Default template argument for an optional size limit

---

**Next lecture:** Non-Type Parameters & Specialization.
