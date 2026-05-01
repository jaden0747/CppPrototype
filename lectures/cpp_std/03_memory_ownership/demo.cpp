// ============================================================================
// Lecture 03 — Demo: Memory & Ownership (C++11)
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Move semantics with a custom Buffer class
// ──────────────────────────────────────────────────────────────────────────
class Buffer
{
public:
    explicit Buffer(std::size_t size)
        : data_(new int[size])
        , size_(size)
    {
        std::fill(data_, data_ + size_, 0);
        std::cout << "    [Buffer] Constructed, size=" << size_ << "\n";
    }

    ~Buffer()
    {
        delete[] data_;
        std::cout << "    [Buffer] Destroyed, size was=" << size_ << "\n";
    }

    // Copy constructor
    Buffer(const Buffer& other)
        : data_(new int[other.size_])
        , size_(other.size_)
    {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "    [Buffer] COPIED, size=" << size_ << "\n";
    }

    // Move constructor
    Buffer(Buffer&& other) noexcept
        : data_(other.data_)
        , size_(other.size_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "    [Buffer] MOVED, size=" << size_ << "\n";
    }

    // Copy assignment
    Buffer& operator=(const Buffer& other)
    {
        if (this != &other)
        {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
            std::cout << "    [Buffer] Copy-assigned, size=" << size_ << "\n";
        }
        return *this;
    }

    // Move assignment
    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data_;
            data_       = other.data_;
            size_       = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
            std::cout << "    [Buffer] Move-assigned, size=" << size_ << "\n";
        }
        return *this;
    }

    std::size_t size() const
    {
        return size_;
    }
    int& operator[](std::size_t i)
    {
        return data_[i];
    }

private:
    int*        data_;
    std::size_t size_;
};

void demo_move_semantics()
{
    std::cout << "=== 1. Move Semantics ===\n";

    Buffer a(5);
    a[0] = 42;

    std::cout << "  Copy construction:\n";
    Buffer b = a; // copy
    assert(b[0] == 42);

    std::cout << "  Move construction:\n";
    Buffer c = std::move(a); // move
    assert(c[0] == 42);
    assert(a.size() == 0); // a is empty

    std::cout << "  Move assignment:\n";
    Buffer d(3);
    d = std::move(c);
    assert(d.size() == 5);
    assert(c.size() == 0);

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. unique_ptr
// ──────────────────────────────────────────────────────────────────────────
class Shape
{
public:
    virtual ~Shape()                 = default;
    virtual std::string name() const = 0;
    virtual double      area() const = 0;
};

class Circle : public Shape
{
    double r_;

public:
    explicit Circle(double r)
        : r_(r)
    {
    }
    std::string name() const override
    {
        return "Circle";
    }
    double area() const override
    {
        return 3.14159 * r_ * r_;
    }
};

class Rect : public Shape
{
    double w_, h_;

public:
    Rect(double w, double h)
        : w_(w)
        , h_(h)
    {
    }
    std::string name() const override
    {
        return "Rect";
    }
    double area() const override
    {
        return w_ * h_;
    }
};

std::unique_ptr<Shape> createShape(const std::string& type)
{
    if (type == "circle")
        return std::unique_ptr<Shape>(new Circle(5.0));
    if (type == "rect")
        return std::unique_ptr<Shape>(new Rect(3.0, 4.0));
    return nullptr;
}

void demo_unique_ptr()
{
    std::cout << "=== 2. unique_ptr ===\n";

    auto s1 = createShape("circle");
    auto s2 = createShape("rect");

    std::cout << "  " << s1->name() << " area = " << s1->area() << "\n";
    std::cout << "  " << s2->name() << " area = " << s2->area() << "\n";

    // Transfer ownership
    auto s3 = std::move(s1);
    assert(s1 == nullptr);
    std::cout << "  After move: s1 is null, s3 owns " << s3->name() << "\n";

    // Container of unique_ptrs
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(createShape("circle"));
    shapes.push_back(createShape("rect"));
    std::cout << "  Vector has " << shapes.size() << " shapes\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. shared_ptr and weak_ptr
// ──────────────────────────────────────────────────────────────────────────
struct Node
{
    std::string           name;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev; // weak to avoid circular ref!

    Node(const std::string& n)
        : name(n)
    {
        std::cout << "    Node(" << n << ") created\n";
    }
    ~Node()
    {
        std::cout << "    ~Node(" << name << ") destroyed\n";
    }
};

void demo_shared_weak_ptr()
{
    std::cout << "=== 3. shared_ptr & weak_ptr ===\n";

    auto a = std::make_shared<Node>("A");
    auto b = std::make_shared<Node>("B");

    std::cout << "  ref count A: " << a.use_count() << "\n"; // 1

    a->next = b; // A → B  (shared)
    b->prev = a; // B → A  (weak — no ref count increase!)

    std::cout << "  ref count A after link: " << a.use_count() << "\n"; // still 1!
    std::cout << "  ref count B after link: " << b.use_count() << "\n"; // 2 (a->next + b)

    // Access via weak_ptr
    if (auto locked = b->prev.lock())
    {
        std::cout << "  B's prev is: " << locked->name << "\n";
    }

    std::cout << "  Resetting...\n";
    a.reset();
    b.reset();
    std::cout << "  Done — no leaks!\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 03 — Memory & Ownership (C++11)         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_move_semantics();
    demo_unique_ptr();
    demo_shared_weak_ptr();

    std::cout << "All demos complete.\n";
    return 0;
}
