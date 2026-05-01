// ============================================================================
// Stdlib 10 — Demo: Memory Management
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. std::unique_ptr
// ──────────────────────────────────────────────────────────────────────────
struct Widget
{
    int id;
    Widget(int i)
        : id(i)
    {
        std::cout << "    Widget(" << id << ") constructed\n";
    }
    ~Widget()
    {
        std::cout << "    Widget(" << id << ") destroyed\n";
    }
    void greet() const
    {
        std::cout << "    Widget " << id << " says hello\n";
    }
};

void demo_unique_ptr()
{
    std::cout << "=== 1. std::unique_ptr ===\n";

    // Create with make_unique (preferred)
    auto w1 = std::make_unique<Widget>(1);
    w1->greet();

    // Transfer ownership
    auto w2 = std::move(w1);
    assert(!w1); // w1 is null
    std::cout << "  w1 is null after move: " << std::boolalpha << (w1 == nullptr) << "\n";
    w2->greet();

    // unique_ptr with array
    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i)
        arr[i] = i * 10;
    std::cout << "  unique_ptr array[2] = " << arr[2] << "\n";

    // Custom deleter
    auto file_closer = [](FILE* f)
    {
        if (f)
        {
            std::fclose(f);
            std::cout << "    FILE closed\n";
        }
    };
    // unique_ptr with custom deleter for C resources
    // std::unique_ptr<FILE, decltype(file_closer)> fp(fopen("/dev/null", "r"), file_closer);

    std::cout << "  (w2 destroyed at scope end)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::shared_ptr
// ──────────────────────────────────────────────────────────────────────────
void demo_shared_ptr()
{
    std::cout << "=== 2. std::shared_ptr ===\n";

    auto s1 = std::make_shared<Widget>(10);
    std::cout << "  use_count: " << s1.use_count() << "\n";

    {
        auto s2 = s1; // share ownership
        std::cout << "  use_count after copy: " << s1.use_count() << "\n";
        s2->greet();
    }
    std::cout << "  use_count after s2 scope: " << s1.use_count() << "\n";

    // Reset
    s1.reset();
    std::cout << "  after reset, s1 is null: " << std::boolalpha << (s1 == nullptr) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::weak_ptr — breaking cycles
// ──────────────────────────────────────────────────────────────────────────
struct Node
{
    std::string           name;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev; // weak to break cycle!

    Node(std::string n)
        : name(std::move(n))
    {
        std::cout << "    Node(" << name << ") created\n";
    }
    ~Node()
    {
        std::cout << "    Node(" << name << ") destroyed\n";
    }
};

void demo_weak_ptr()
{
    std::cout << "=== 3. std::weak_ptr ===\n";

    auto a = std::make_shared<Node>("A");
    auto b = std::make_shared<Node>("B");

    a->next = b;
    b->prev = a; // weak_ptr — doesn't prevent destruction

    std::cout << "  a use_count: " << a.use_count() << "\n"; // 1, not 2
    std::cout << "  b use_count: " << b.use_count() << "\n"; // 2 (a->next)

    // Lock weak_ptr to get shared_ptr
    if (auto prev = b->prev.lock())
    {
        std::cout << "  b->prev = " << prev->name << "\n";
    }

    // expired check
    std::weak_ptr<Node> weak_a = a;
    std::cout << "  weak_a expired? " << std::boolalpha << weak_a.expired() << "\n";

    std::cout << "  (both nodes destroyed at scope end)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. make_shared vs new — single allocation
// ──────────────────────────────────────────────────────────────────────────
void demo_make_shared_efficiency()
{
    std::cout << "=== 4. make_shared Efficiency ===\n";

    // make_shared: 1 allocation (object + control block together)
    auto s1 = std::make_shared<Widget>(20);

    // Two separate allocations (less efficient):
    // auto s2 = std::shared_ptr<Widget>(new Widget(21));

    std::cout << "  make_shared uses 1 allocation for obj+control block\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. enable_shared_from_this
// ──────────────────────────────────────────────────────────────────────────
struct Tracker : std::enable_shared_from_this<Tracker>
{
    std::string name;
    Tracker(std::string n)
        : name(std::move(n))
    {
    }

    std::shared_ptr<Tracker> get_self()
    {
        return shared_from_this();
    }
};

void demo_enable_shared()
{
    std::cout << "=== 5. enable_shared_from_this ===\n";

    auto t  = std::make_shared<Tracker>("T1");
    auto t2 = t->get_self();
    std::cout << "  same object? " << std::boolalpha << (t.get() == t2.get()) << "\n";
    std::cout << "  use_count: " << t.use_count() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. PMR — Polymorphic Memory Resources
// ──────────────────────────────────────────────────────────────────────────
void demo_pmr()
{
    std::cout << "=== 6. PMR (Polymorphic Memory Resources) ===\n";

    // Stack-based buffer with monotonic_buffer_resource
    char                                buffer[4096];
    std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

    // pmr::vector uses the pool allocator
    std::pmr::vector<int> v(&pool);
    for (int i = 0; i < 20; ++i)
        v.push_back(i);

    std::cout << "  pmr::vector size: " << v.size() << "\n";
    std::cout << "  allocated from stack buffer (no heap alloc!)\n";

    // pmr::string
    std::pmr::string s("Hello PMR World!", &pool);
    std::cout << "  pmr::string: " << s << "\n";

    // Nested containers
    std::pmr::vector<std::pmr::string> names(&pool);
    names.emplace_back("Alice");
    names.emplace_back("Bob");
    std::cout << "  names[0]: " << names[0] << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. construct_at / destroy_at (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_construct_destroy()
{
    std::cout << "=== 7. construct_at / destroy_at (C++20) ===\n";

    // Allocate raw memory
    alignas(Widget) char storage[sizeof(Widget)];

    // Construct in-place
    Widget* w = std::construct_at(reinterpret_cast<Widget*>(storage), 99);
    w->greet();

    // Destroy without deallocating
    std::destroy_at(w);
    std::cout << "  (destroyed via destroy_at, no deallocation)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 10 — Memory Management                   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_unique_ptr();
    demo_shared_ptr();
    demo_weak_ptr();
    demo_make_shared_efficiency();
    demo_enable_shared();
    demo_pmr();
    demo_construct_destroy();

    std::cout << "All demos complete.\n";
    return 0;
}
