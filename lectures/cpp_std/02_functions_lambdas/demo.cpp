// ============================================================================
// Lecture 02 — Demo: Functions & Lambdas (C++11)
// ============================================================================
// Build:  cmake --build build/Debug --target lecture02_demo
// Run:    ./build/Debug/lecture02_demo
// ============================================================================

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Lambda expressions
// ──────────────────────────────────────────────────────────────────────────
void demo_lambdas()
{
    std::cout << "=== 1. Lambda Expressions ===\n";

    // Simplest lambda
    auto greet = [] { std::cout << "  Hello from a lambda!\n"; };
    greet();

    // Lambda with parameters
    auto add = [](int a, int b) -> int { return a + b; };
    std::cout << "  add(3, 4) = " << add(3, 4) << "\n";

    // Lambda with algorithms
    std::vector<int> nums{5, 2, 8, 1, 9, 3, 7};
    std::sort(nums.begin(), nums.end(), [](int a, int b) { return a > b; });
    std::cout << "  Sorted desc: ";
    for (const auto& n : nums)
        std::cout << n << " ";
    std::cout << "\n";

    // count_if
    auto count = std::count_if(nums.begin(), nums.end(), [](int x) { return x > 5; });
    std::cout << "  Count > 5: " << count << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Captures
// ──────────────────────────────────────────────────────────────────────────
void demo_captures()
{
    std::cout << "=== 2. Captures ===\n";

    int              threshold = 5;
    std::vector<int> v{1, 2, 3, 6, 7, 8};

    // Capture by value
    auto above = [threshold](int x) { return x > threshold; };
    auto cnt   = std::count_if(v.begin(), v.end(), above);
    std::cout << "  Above " << threshold << ": " << cnt << " elements\n";

    // Capture by reference — modifies outer variable
    int sum = 0;
    std::for_each(v.begin(), v.end(), [&sum](int x) { sum += x; });
    std::cout << "  Sum (by-ref capture): " << sum << "\n";

    // Mutable capture — modifies local copy
    int  counter = 0;
    auto inc     = [counter]() mutable { return ++counter; };
    std::cout << "  inc() = " << inc() << ", inc() = " << inc() << "\n";
    std::cout << "  Original counter still = " << counter << "\n";

    // Capture all by reference
    int  a = 10, b = 20;
    auto swap_ab = [&] { std::swap(a, b); };
    swap_ab();
    std::cout << "  After [&] swap: a=" << a << " b=" << b << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. = default and = delete
// ──────────────────────────────────────────────────────────────────────────
class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(int value)
        : value_(value)
    {
    }

    NonCopyable(const NonCopyable&)            = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    // Move is allowed
    NonCopyable(NonCopyable&& other) noexcept
        : value_(other.value_)
    {
        other.value_ = 0;
    }
    NonCopyable& operator=(NonCopyable&& other) noexcept
    {
        value_       = other.value_;
        other.value_ = 0;
        return *this;
    }

    int value() const
    {
        return value_;
    }

private:
    int value_ = 0;
};

// Delete a specific overload to prevent implicit conversions
void processInt(int x)
{
    std::cout << "  processInt(" << x << ")\n";
}
void processInt(double) = delete; // Compile error if double passed

void demo_default_delete()
{
    std::cout << "=== 3. = default / = delete ===\n";

    NonCopyable a{42};
    // NonCopyable b = a;       // ERROR: copy deleted
    NonCopyable b = std::move(a); // OK: move allowed
    std::cout << "  After move: a=" << a.value() << " b=" << b.value() << "\n";

    processInt(7);
    // processInt(3.14);  // Would fail: deleted overload
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Delegating Constructors
// ──────────────────────────────────────────────────────────────────────────
class Server
{
public:
    Server(const std::string& host, int port, int maxConn)
        : host_(host)
        , port_(port)
        , maxConn_(maxConn)
    {
        std::cout << "  Server(" << host_ << ":" << port_ << ", max=" << maxConn_ << ")\n";
    }

    // Delegates to 3-arg ctor
    Server(const std::string& host, int port)
        : Server(host, port, 100)
    {
    }

    // Delegates further
    Server()
        : Server("localhost", 8080)
    {
    }

    std::string info() const
    {
        return host_ + ":" + std::to_string(port_) + " [max=" + std::to_string(maxConn_) + "]";
    }

private:
    std::string host_;
    int         port_;
    int         maxConn_;
};

void demo_delegating_ctors()
{
    std::cout << "=== 4. Delegating Constructors ===\n";
    Server s1;
    Server s2("192.168.1.1", 9090);
    Server s3("10.0.0.1", 443, 500);
    std::cout << "  s1: " << s1.info() << "\n";
    std::cout << "  s2: " << s2.info() << "\n";
    std::cout << "  s3: " << s3.info() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Type aliases with using
// ──────────────────────────────────────────────────────────────────────────
using Callback  = std::function<void(const std::string&)>;
using StringVec = std::vector<std::string>;

template <typename T>
using Vec = std::vector<T>;

void demo_using_aliases()
{
    std::cout << "=== 5. Type Aliases (using) ===\n";

    Callback cb = [](const std::string& msg) { std::cout << "  CB: " << msg << "\n"; };
    cb("Hello via Callback alias");

    StringVec names{"Alice", "Bob"};
    std::cout << "  StringVec size: " << names.size() << "\n";

    Vec<double> coords{1.0, 2.5, 3.7};
    std::cout << "  Vec<double> size: " << coords.size() << "\n";

    // Function pointer alias
    using MathOp    = double (*)(double, double);
    MathOp multiply = [](double a, double b) -> double { return a * b; };
    std::cout << "  multiply(3, 4) = " << multiply(3, 4) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. std::function — type-erased callable
// ──────────────────────────────────────────────────────────────────────────
void demo_std_function()
{
    std::cout << "=== 6. std::function ===\n";

    // Stores any callable with signature int(int, int)
    std::function<int(int, int)> op;

    op = [](int a, int b) { return a + b; };
    std::cout << "  add: " << op(10, 5) << "\n";

    op = [](int a, int b) { return a * b; };
    std::cout << "  mul: " << op(10, 5) << "\n";

    // Vector of callbacks
    std::vector<std::function<void()>> hooks;
    hooks.push_back([] { std::cout << "  hook 1\n"; });
    hooks.push_back([] { std::cout << "  hook 2\n"; });
    for (auto& h : hooks)
        h();

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔════════════════════════════════════════════════╗\n"
              << "║  Lecture 02 — Functions & Lambdas (C++11)      ║\n"
              << "╚════════════════════════════════════════════════╝\n\n";

    demo_lambdas();
    demo_captures();
    demo_default_delete();
    demo_delegating_ctors();
    demo_using_aliases();
    demo_std_function();

    std::cout << "All demos complete.\n";
    return 0;
}
