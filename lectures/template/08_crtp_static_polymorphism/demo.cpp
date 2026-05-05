// ============================================================================
// Template 08 — Demo: CRTP & Static Polymorphism
// ============================================================================
#define _USE_MATH_DEFINES   // expose M_PI from <cmath> on MSVC / clang-cl
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Basic CRTP Pattern
// ──────────────────────────────────────────────────────────────────────────
template <typename Derived>
struct Shape
{
    double area() const
    {
        return static_cast<const Derived*>(this)->area_impl();
    }
    double perimeter() const
    {
        return static_cast<const Derived*>(this)->perimeter_impl();
    }
    void describe() const
    {
        std::cout << "  " << static_cast<const Derived*>(this)->name() << ": area=" << area()
                  << " perimeter=" << perimeter() << "\n";
    }
};

struct Circle : Shape<Circle>
{
    double radius;
    explicit Circle(double r)
        : radius(r)
    {
    }
    double area_impl() const
    {
        return M_PI * radius * radius;
    }
    double perimeter_impl() const
    {
        return 2.0 * M_PI * radius;
    }
    const char* name() const
    {
        return "Circle";
    }
};

struct Rectangle : Shape<Rectangle>
{
    double w, h;
    Rectangle(double w, double h)
        : w(w)
        , h(h)
    {
    }
    double area_impl() const
    {
        return w * h;
    }
    double perimeter_impl() const
    {
        return 2.0 * (w + h);
    }
    const char* name() const
    {
        return "Rectangle";
    }
};

void demo_basic_crtp()
{
    std::cout << "=== 1. Basic CRTP ===\n";
    Circle c(5.0);
    c.describe();
    Rectangle r(3.0, 4.0);
    r.describe();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Static vs Virtual Polymorphism
// ──────────────────────────────────────────────────────────────────────────

// Virtual version for comparison
struct VShape
{
    virtual ~VShape()                = default;
    virtual double      area() const = 0;
    virtual const char* name() const = 0;
};

struct VCircle : VShape
{
    double radius;
    explicit VCircle(double r)
        : radius(r)
    {
    }
    double area() const override
    {
        return M_PI * radius * radius;
    }
    const char* name() const override
    {
        return "VCircle";
    }
};

// CRTP dispatch function — works at compile time
template <typename Derived>
void print_shape(const Shape<Derived>& s)
{
    s.describe(); // no virtual call!
}

void demo_comparison()
{
    std::cout << "=== 2. Static vs Virtual ===\n";

    // CRTP: resolved at compile time
    Circle c(3.0);
    print_shape(c);

    // Virtual: resolved at runtime
    VCircle vc(3.0);
    VShape& ref = vc;
    std::cout << "  " << ref.name() << " area=" << ref.area() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Mixin Classes via CRTP
// ──────────────────────────────────────────────────────────────────────────

// Mixin: adds print() capability
template <typename Derived>
struct Printable
{
    void print(std::ostream& os = std::cout) const
    {
        os << static_cast<const Derived*>(this)->to_string();
    }
};

// Mixin: adds comparison operators
template <typename Derived>
struct EqualityComparable
{
    bool operator!=(const Derived& other) const
    {
        return !(static_cast<const Derived*>(this)->operator==(other));
    }
};

// Mixin: adds clone()
template <typename Derived>
struct Cloneable
{
    Derived clone() const
    {
        return Derived(static_cast<const Derived&>(*this));
    }
};

// A class using multiple CRTP mixins
struct Point : Printable<Point>, EqualityComparable<Point>, Cloneable<Point>
{
    double x, y;
    Point(double x, double y)
        : x(x)
        , y(y)
    {
    }

    std::string to_string() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }
};

void demo_mixins()
{
    std::cout << "=== 3. Mixin Classes ===\n";

    Point p(1.0, 2.0);
    std::cout << "  Point: ";
    p.print();
    std::cout << "\n";

    auto p2 = p.clone();
    std::cout << "  Clone: ";
    p2.print();
    std::cout << "\n";

    std::cout << "  p == p2: " << std::boolalpha << (p == p2) << "\n";
    std::cout << "  p != p2: " << (p != p2) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. CRTP for Static Interface Enforcement
// ──────────────────────────────────────────────────────────────────────────
template <typename Derived>
struct Counter
{
    static int count;

    Counter()
    {
        ++count;
    }
    ~Counter()
    {
        --count;
    }
    Counter(const Counter&)
    {
        ++count;
    }

    static int alive()
    {
        return count;
    }
};

template <typename Derived>
int Counter<Derived>::count = 0;

struct Widget : Counter<Widget>
{
    std::string name;
    Widget(std::string n = "unnamed")
        : name(std::move(n))
    {
    }
};

struct Gadget : Counter<Gadget>
{
    int id;
    Gadget(int id = 0)
        : id(id)
    {
    }
};

void demo_counter()
{
    std::cout << "=== 4. CRTP Counter ===\n";
    {
        Widget w1("foo"), w2("bar");
        Gadget g1(1);
        std::cout << "  Widgets alive: " << Widget::alive() << "\n"; // 2
        std::cout << "  Gadgets alive: " << Gadget::alive() << "\n"; // 1
    }
    std::cout << "  After scope — Widgets: " << Widget::alive() << " Gadgets: " << Gadget::alive() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Deducing This (C++23 preview)
// ──────────────────────────────────────────────────────────────────────────
// Note: requires C++23 compiler support. Shown as concept, may not compile
// on all platforms yet.
#if __cpp_explicit_this_parameter >= 202110L
struct Base23
{
    template <typename Self>
    void greet(this Self&& self)
    {
        std::cout << "  Hello from " << self.name() << "\n";
    }
};

struct Derived23 : Base23
{
    std::string name() const
    {
        return "Derived23";
    }
};
#endif

void demo_deducing_this()
{
    std::cout << "=== 5. Deducing This (C++23) ===\n";
#if __cpp_explicit_this_parameter >= 202110L
    Derived23 d;
    d.greet();
#else
    std::cout << "  (C++23 deducing this not available on this compiler)\n";
#endif
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 08 — CRTP & Static Polymorphism        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_basic_crtp();
    demo_comparison();
    demo_mixins();
    demo_counter();
    demo_deducing_this();

    std::cout << "All demos complete.\n";
    return 0;
}
