// ============================================================================
// Template 01 — Demo: Function & Class Templates
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Function Templates
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
T max_of(T a, T b)
{
    return (a > b) ? a : b;
}

template <typename T, typename U>
auto add(T a, U b) // return type deduced (C++14)
{
    return a + b;
}

void demo_function_templates()
{
    std::cout << "=== 1. Function Templates ===\n";
    std::cout << "  max_of(3, 7) = " << max_of(3, 7) << "\n";
    std::cout << "  max_of(1.5, 2.3) = " << max_of(1.5, 2.3) << "\n";
    std::cout << "  max_of('a', 'z') = " << max_of('a', 'z') << "\n";

    std::cout << "  add(1, 2.5) = " << add(1, 2.5) << "\n";
    std::cout << "  add(string, string) = " << add(std::string("hello"), std::string(" world")) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Template Argument Deduction
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
void show_type(T value)
{
    // __PRETTY_FUNCTION__ shows the deduced type (GCC/Clang)
    std::cout << "  show_type(" << value << "): " << __PRETTY_FUNCTION__ << "\n";
}

template <typename T>
void show_ref(const T& value)
{
    std::cout << "  show_ref: " << __PRETTY_FUNCTION__ << "\n";
}

void demo_deduction()
{
    std::cout << "=== 2. Template Argument Deduction ===\n";
    show_type(42);
    show_type(3.14);
    show_type("hello");
    show_type(std::string("world"));

    show_ref(42);      // T = int
    show_ref("hello"); // T = char[6]
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Explicit Template Arguments
// ──────────────────────────────────────────────────────────────────────────
template <typename R, typename T>
R convert(T value)
{
    return static_cast<R>(value);
}

void demo_explicit()
{
    std::cout << "=== 3. Explicit Template Arguments ===\n";
    // max_of(1, 2.5) won't compile — T deduced as both int and double
    // Fix: explicit
    std::cout << "  max_of<double>(1, 2.5) = " << max_of<double>(1, 2.5) << "\n";

    auto x = convert<double>(42);
    std::cout << "  convert<double>(42) = " << x << "\n";

    auto y = convert<int>(3.99);
    std::cout << "  convert<int>(3.99) = " << y << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Class Templates
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
class Stack
{
    std::vector<T> data_;

public:
    void push(const T& val)
    {
        data_.push_back(val);
    }
    void pop()
    {
        data_.pop_back();
    }
    const T& top() const
    {
        return data_.back();
    }
    bool empty() const
    {
        return data_.empty();
    }
    size_t size() const
    {
        return data_.size();
    }

    // Iterate (for demo purposes)
    auto begin() const
    {
        return data_.begin();
    }
    auto end() const
    {
        return data_.end();
    }
};

void demo_class_template()
{
    std::cout << "=== 4. Class Templates ===\n";

    Stack<int> si;
    si.push(10);
    si.push(20);
    si.push(30);
    std::cout << "  Stack<int> top: " << si.top() << " size: " << si.size() << "\n";
    std::cout << "  Contents: ";
    for (int x : si)
        std::cout << x << " ";
    std::cout << "\n";

    Stack<std::string> ss;
    ss.push("hello");
    ss.push("world");
    std::cout << "  Stack<string> top: " << ss.top() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Member Function Templates
// ──────────────────────────────────────────────────────────────────────────
class Printer
{
public:
    template <typename T>
    void print(const T& value) const
    {
        std::cout << "  Printer: " << value << "\n";
    }
};

// Class template with a member template
template <typename T>
class Box
{
    T value_;

public:
    explicit Box(T v)
        : value_(std::move(v))
    {
    }
    const T& get() const
    {
        return value_;
    }

    // Convert to a Box of a different type
    template <typename U>
    Box<U> as() const
    {
        return Box<U>(static_cast<U>(value_));
    }
};

void demo_member_templates()
{
    std::cout << "=== 5. Member Function Templates ===\n";

    Printer p;
    p.print(42);
    p.print(3.14);
    p.print("hello");

    Box<int>    bi(42);
    Box<double> bd = bi.as<double>();
    std::cout << "  Box<int>(42).as<double>() = " << bd.get() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Default Template Arguments
// ──────────────────────────────────────────────────────────────────────────
template <typename T = double>
T zero()
{
    return T{};
}

template <typename T = int, size_t Cap = 100>
class FixedBuffer
{
    T      data_[Cap] = {};
    size_t size_      = 0;

public:
    void add(T val)
    {
        if (size_ < Cap)
            data_[size_++] = val;
    }
    size_t size() const
    {
        return size_;
    }
    size_t capacity() const
    {
        return Cap;
    }
    T operator[](size_t i) const
    {
        return data_[i];
    }
};

void demo_defaults()
{
    std::cout << "=== 6. Default Template Arguments ===\n";
    std::cout << "  zero() = " << zero() << " (double)\n";
    std::cout << "  zero<int>() = " << zero<int>() << "\n";

    FixedBuffer<> buf1; // int, capacity 100
    buf1.add(42);
    std::cout << "  FixedBuffer<> capacity=" << buf1.capacity() << " [0]=" << buf1[0] << "\n";

    FixedBuffer<double, 5> buf2;
    buf2.add(3.14);
    std::cout << "  FixedBuffer<double,5> capacity=" << buf2.capacity() << " [0]=" << buf2[0] << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 01 — Function & Class Templates        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_function_templates();
    demo_deduction();
    demo_explicit();
    demo_class_template();
    demo_member_templates();
    demo_defaults();

    std::cout << "All demos complete.\n";
    return 0;
}
