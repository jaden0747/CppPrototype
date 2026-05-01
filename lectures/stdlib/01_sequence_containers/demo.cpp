// ============================================================================
// Stdlib 01 — Demo: Sequence Containers
// ============================================================================
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

// Helper to print any iterable
template <typename C>
void print(const char* label, const C& c)
{
    std::cout << "  " << label << ": [";
    bool first = true;
    for (const auto& e : c)
    {
        if (!first)
            std::cout << ", ";
        std::cout << e;
        first = false;
    }
    std::cout << "]\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 1. std::vector
// ──────────────────────────────────────────────────────────────────────────
void demo_vector()
{
    std::cout << "=== 1. std::vector ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};
    print("initial", v);

    // push_back vs emplace_back
    v.push_back(6);
    v.emplace_back(7); // constructs in-place
    print("after push", v);

    // reserve vs resize
    std::cout << "  capacity before reserve: " << v.capacity() << "\n";
    v.reserve(100);
    std::cout << "  capacity after reserve(100): " << v.capacity() << "\n";
    std::cout << "  size unchanged: " << v.size() << "\n";

    v.resize(10, 0); // extend with zeros
    print("after resize(10,0)", v);

    // data() — raw pointer for C interop
    int* raw = v.data();
    std::cout << "  data()[0] = " << raw[0] << "\n";

    // Erase-remove idiom (remove all zeros)
    v.erase(std::remove(v.begin(), v.end(), 0), v.end());
    print("after erase zeros", v);

    // C++20 std::erase
    std::erase(v, 3);
    print("after std::erase(3)", v);

    // shrink_to_fit
    v.shrink_to_fit();
    std::cout << "  capacity after shrink: " << v.capacity() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::array
// ──────────────────────────────────────────────────────────────────────────
void demo_array()
{
    std::cout << "=== 2. std::array ===\n";

    std::array<int, 5> a{10, 20, 30, 40, 50};
    print("initial", a);

    std::cout << "  size (constexpr): " << a.size() << "\n";
    std::cout << "  front: " << a.front() << "  back: " << a.back() << "\n";

    // fill
    std::array<double, 4> b;
    b.fill(3.14);
    print("filled", b);

    // Sorting works like any range
    std::array<int, 6> c{5, 3, 1, 4, 2, 6};
    std::sort(c.begin(), c.end());
    print("sorted", c);

    // constexpr usage
    constexpr std::array<int, 3> cx{1, 2, 3};
    static_assert(cx[0] == 1 && cx.size() == 3);
    std::cout << "  constexpr array OK ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::deque
// ──────────────────────────────────────────────────────────────────────────
void demo_deque()
{
    std::cout << "=== 3. std::deque ===\n";

    std::deque<int> d;
    d.push_back(2);
    d.push_back(3);
    d.push_front(1);
    d.push_front(0);
    print("after push front+back", d);

    // Random access
    std::cout << "  d[2] = " << d[2] << "\n";

    d.pop_front();
    d.pop_back();
    print("after pop front+back", d);

    // Insert in middle
    d.insert(d.begin() + 1, 99);
    print("after insert(1, 99)", d);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::list & std::forward_list
// ──────────────────────────────────────────────────────────────────────────
void demo_list()
{
    std::cout << "=== 4. std::list ===\n";

    std::list<int> l{5, 3, 1, 4, 2};
    print("initial", l);

    // Member sort (std::sort requires random access)
    l.sort();
    print("sorted", l);

    // Remove
    l.remove(3);
    print("remove(3)", l);

    // Unique (removes consecutive duplicates — sort first)
    std::list<int> l2{1, 1, 2, 2, 3, 3, 3};
    l2.unique();
    print("unique", l2);

    // Splice — O(1) transfer from another list
    std::list<int> other{10, 20, 30};
    l.splice(l.end(), other);
    print("after splice", l);
    std::cout << "  other is now empty: " << std::boolalpha << other.empty() << "\n";

    // forward_list (singly linked)
    std::cout << "\n=== std::forward_list ===\n";
    std::forward_list<int> fl{5, 3, 1, 4, 2};
    fl.sort();
    print("sorted", fl);

    fl.push_front(0);
    fl.insert_after(fl.begin(), 99);
    print("after insert", fl);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Iterator invalidation demonstration
// ──────────────────────────────────────────────────────────────────────────
void demo_invalidation()
{
    std::cout << "=== 5. Iterator Invalidation ===\n";

    // vector: push_back can invalidate
    std::vector<int> v{1, 2, 3};
    v.reserve(10); // Prevent reallocation
    auto it = v.begin();
    v.push_back(4);
    // After reserve, this is safe:
    std::cout << "  *it after push (reserved) = " << *it << " ✓\n";

    // list: iterators never invalidated (except erased)
    std::list<int> l{1, 2, 3};
    auto           lit = std::next(l.begin()); // points to 2
    l.push_back(4);
    l.push_front(0);
    std::cout << "  list *it after push = " << *lit << " ✓ (still 2)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Performance comparison hint
// ──────────────────────────────────────────────────────────────────────────
void demo_comparison()
{
    std::cout << "=== 6. Container Comparison ===\n";

    constexpr int N = 100000;

    // vector: push_back
    std::vector<int> v;
    v.reserve(N);
    for (int i = 0; i < N; ++i)
        v.push_back(i);
    std::cout << "  vector push_back x" << N << ": size=" << v.size() << "\n";

    // deque: push_front + push_back
    std::deque<int> d;
    for (int i = 0; i < N; ++i)
    {
        if (i % 2 == 0)
            d.push_back(i);
        else
            d.push_front(i);
    }
    std::cout << "  deque mixed push x" << N << ": size=" << d.size() << "\n";

    // list: push_back
    std::list<int> l;
    for (int i = 0; i < N; ++i)
        l.push_back(i);
    std::cout << "  list push_back x" << N << ": size=" << l.size() << "\n";

    std::cout << "  (Benchmark these with chrono for real numbers!)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 01 — Sequence Containers                 ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_vector();
    demo_array();
    demo_deque();
    demo_list();
    demo_invalidation();
    demo_comparison();

    std::cout << "All demos complete.\n";
    return 0;
}
