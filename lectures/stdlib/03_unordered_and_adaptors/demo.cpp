// ============================================================================
// Stdlib 03 — Demo: Unordered Containers & Adaptors
// ============================================================================
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <queue>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. std::unordered_set
// ──────────────────────────────────────────────────────────────────────────
void demo_unordered_set()
{
    std::cout << "=== 1. std::unordered_set ===\n";

    std::unordered_set<int> s{5, 3, 1, 4, 2, 2, 3};
    std::cout << "  size (no dups): " << s.size() << "\n";

    // Insert & find
    s.insert(6);
    std::cout << "  contains(4)? " << std::boolalpha << s.contains(4) << "\n";
    std::cout << "  contains(99)? " << s.contains(99) << "\n";

    // Iteration order is NOT sorted
    std::cout << "  elements (arbitrary order): ";
    for (int x : s)
        std::cout << x << " ";
    std::cout << "\n";

    // Bucket info
    std::cout << "  bucket_count: " << s.bucket_count() << "\n";
    std::cout << "  load_factor: " << s.load_factor() << "\n";
    std::cout << "  max_load_factor: " << s.max_load_factor() << "\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::unordered_map
// ──────────────────────────────────────────────────────────────────────────
void demo_unordered_map()
{
    std::cout << "=== 2. std::unordered_map ===\n";

    std::unordered_map<std::string, int> m;
    m["apple"]  = 3;
    m["banana"] = 5;
    m["cherry"] = 2;
    m.emplace("date", 7);

    for (const auto& [k, v] : m)
    {
        std::cout << "  " << k << " -> " << v << "\n";
    }

    // try_emplace (C++17) — only construct value if key missing
    auto [it, ok] = m.try_emplace("apple", 99);
    std::cout << "  try_emplace(apple,99): " << (ok ? "inserted" : "skipped") << " val=" << it->second << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Custom hash for user type
// ──────────────────────────────────────────────────────────────────────────
struct Point
{
    int  x, y;
    bool operator==(const Point&) const = default;
};

struct PointHash
{
    std::size_t operator()(const Point& p) const
    {
        auto h1 = std::hash<int>{}(p.x);
        auto h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1); // simple combine
    }
};

void demo_custom_hash()
{
    std::cout << "=== 3. Custom Hash ===\n";

    std::unordered_set<Point, PointHash> points;
    points.insert({1, 2});
    points.insert({3, 4});
    points.insert({1, 2}); // duplicate

    std::cout << "  points.size() = " << points.size() << " (1,2 deduped)\n";
    std::cout << "  contains({3,4})? " << std::boolalpha << points.contains({3, 4}) << "\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::stack
// ──────────────────────────────────────────────────────────────────────────
void demo_stack()
{
    std::cout << "=== 4. std::stack ===\n";

    std::stack<int> st;
    st.push(1);
    st.push(2);
    st.push(3);

    std::cout << "  top: " << st.top() << "\n";
    st.pop();
    std::cout << "  after pop, top: " << st.top() << "\n";
    std::cout << "  size: " << st.size() << "\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. std::queue
// ──────────────────────────────────────────────────────────────────────────
void demo_queue()
{
    std::cout << "=== 5. std::queue ===\n";

    std::queue<std::string> q;
    q.push("first");
    q.push("second");
    q.push("third");

    std::cout << "  front: " << q.front() << "  back: " << q.back() << "\n";
    q.pop();
    std::cout << "  after pop, front: " << q.front() << "\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. std::priority_queue
// ──────────────────────────────────────────────────────────────────────────
void demo_priority_queue()
{
    std::cout << "=== 6. std::priority_queue ===\n";

    // Max-heap by default
    std::priority_queue<int> pq;
    pq.push(3);
    pq.push(1);
    pq.push(4);
    pq.push(1);
    pq.push(5);

    std::cout << "  max-heap drain: ";
    while (!pq.empty())
    {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n";

    // Min-heap
    std::priority_queue<int, std::vector<int>, std::greater<>> min_pq;
    for (int x : {3, 1, 4, 1, 5})
        min_pq.push(x);

    std::cout << "  min-heap drain: ";
    while (!min_pq.empty())
    {
        std::cout << min_pq.top() << " ";
        min_pq.pop();
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. std::span (C++20)
// ──────────────────────────────────────────────────────────────────────────
void print_span(std::span<const int> data)
{
    std::cout << "  span contents: [";
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (i > 0)
            std::cout << ", ";
        std::cout << data[i];
    }
    std::cout << "] (size=" << data.size() << ")\n";
}

void demo_span()
{
    std::cout << "=== 7. std::span (C++20) ===\n";

    // span from vector
    std::vector<int> v{1, 2, 3, 4, 5};
    print_span(v);

    // span from C array
    int arr[] = {10, 20, 30};
    print_span(arr);

    // subspan
    std::span<const int> full(v);
    auto                 sub = full.subspan(1, 3); // elements 2,3,4
    print_span(sub);

    // first / last
    print_span(full.first(2));
    print_span(full.last(2));

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 03 — Unordered Containers & Adaptors     ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_unordered_set();
    demo_unordered_map();
    demo_custom_hash();
    demo_stack();
    demo_queue();
    demo_priority_queue();
    demo_span();

    std::cout << "All demos complete.\n";
    return 0;
}
