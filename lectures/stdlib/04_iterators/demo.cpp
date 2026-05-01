// ============================================================================
// Stdlib 04 — Demo: Iterators
// ============================================================================
#include <algorithm>
#include <cassert>
#include <deque>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// Helper
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
// 1. Iterator categories
// ──────────────────────────────────────────────────────────────────────────
void demo_categories()
{
    std::cout << "=== 1. Iterator Categories ===\n";

    // Random access: vector, deque, array
    std::vector<int> v{1, 2, 3, 4, 5};
    auto             it = v.begin();
    it += 3; // random access
    std::cout << "  vector[3] via iterator: " << *it << "\n";

    // Bidirectional: list, set, map
    std::list<int> l{10, 20, 30, 40};
    auto           lit = l.end();
    --lit; // bidirectional
    std::cout << "  list.back via --end(): " << *lit << "\n";

    // Forward only: forward_list
    std::forward_list<int> fl{1, 2, 3};
    auto                   flit = fl.begin();
    ++flit; // forward only — no --
    std::cout << "  forward_list[1]: " << *flit << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Range access functions
// ──────────────────────────────────────────────────────────────────────────
void demo_range_access()
{
    std::cout << "=== 2. Range Access ===\n";

    int arr[] = {10, 20, 30, 40, 50};

    // std::begin / std::end work on C arrays too
    std::cout << "  C array via std::begin/end: ";
    for (auto it = std::begin(arr); it != std::end(arr); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // std::rbegin / std::rend — reverse iteration
    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "  reverse: ";
    for (auto it = std::rbegin(v); it != std::rend(v); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // std::ssize (C++20) — signed size
    std::cout << "  ssize: " << std::ssize(v) << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. advance, distance, next, prev
// ──────────────────────────────────────────────────────────────────────────
void demo_advance_distance()
{
    std::cout << "=== 3. advance / distance / next / prev ===\n";

    std::list<int> l{10, 20, 30, 40, 50};

    // std::advance — moves iterator in-place
    auto it = l.begin();
    std::advance(it, 3);
    std::cout << "  advance(begin, 3) = " << *it << "\n";

    // std::distance
    std::cout << "  distance(begin, it) = " << std::distance(l.begin(), it) << "\n";

    // std::next / std::prev — returns new iterator
    auto n = std::next(l.begin(), 2);
    std::cout << "  next(begin, 2) = " << *n << "\n";

    auto p = std::prev(l.end());
    std::cout << "  prev(end) = " << *p << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Insert iterators
// ──────────────────────────────────────────────────────────────────────────
void demo_insert_iterators()
{
    std::cout << "=== 4. Insert Iterators ===\n";

    std::vector<int> src{1, 2, 3, 4, 5};

    // back_inserter
    std::vector<int> dest;
    std::copy(src.begin(), src.end(), std::back_inserter(dest));
    print("back_inserter", dest);

    // front_inserter (deque/list)
    std::deque<int> dq;
    std::copy(src.begin(), src.end(), std::front_inserter(dq));
    print("front_inserter", dq); // reversed!

    // inserter — insert at specific position
    std::list<int> lst{10, 20, 30};
    std::copy(src.begin(), src.end(), std::inserter(lst, std::next(lst.begin())));
    print("inserter after 10", lst);

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Stream iterators
// ──────────────────────────────────────────────────────────────────────────
void demo_stream_iterators()
{
    std::cout << "=== 5. Stream Iterators ===\n";

    // istream_iterator — read from stream
    std::istringstream iss("10 20 30 40 50");
    std::vector<int>   v{std::istream_iterator<int>(iss), std::istream_iterator<int>()};
    print("from stream", v);

    // ostream_iterator — write to stream
    std::cout << "  to stream: ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";

    // Transform and output
    std::cout << "  doubled: ";
    std::transform(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "), [](int x) { return x * 2; });
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. iterator_traits
// ──────────────────────────────────────────────────────────────────────────
template <typename It>
void show_category([[maybe_unused]] It it)
{
    using category = typename std::iterator_traits<It>::iterator_category;

    if constexpr (std::is_same_v<category, std::random_access_iterator_tag>)
        std::cout << "  random_access_iterator\n";
    else if constexpr (std::is_same_v<category, std::bidirectional_iterator_tag>)
        std::cout << "  bidirectional_iterator\n";
    else if constexpr (std::is_same_v<category, std::forward_iterator_tag>)
        std::cout << "  forward_iterator\n";
    else if constexpr (std::is_same_v<category, std::input_iterator_tag>)
        std::cout << "  input_iterator\n";
    else
        std::cout << "  output_iterator\n";
}

void demo_iterator_traits()
{
    std::cout << "=== 6. iterator_traits ===\n";

    std::vector<int> v;
    show_category(v.begin()); // random_access

    std::list<int> l;
    show_category(l.begin()); // bidirectional

    std::forward_list<int> fl;
    show_category(fl.begin()); // forward

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. Custom iterator — range of integers
// ──────────────────────────────────────────────────────────────────────────
class IntRange
{
    int start_, end_;

public:
    IntRange(int s, int e)
        : start_(s)
        , end_(e)
    {
    }

    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = int;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const int*;
        using reference         = int;

        int current;

        int operator*() const
        {
            return current;
        }
        Iterator& operator++()
        {
            ++current;
            return *this;
        }
        Iterator operator++(int)
        {
            auto tmp = *this;
            ++current;
            return tmp;
        }
        bool operator==(const Iterator& o) const
        {
            return current == o.current;
        }
        bool operator!=(const Iterator& o) const
        {
            return current != o.current;
        }
    };

    Iterator begin() const
    {
        return {start_};
    }
    Iterator end() const
    {
        return {end_};
    }
};

void demo_custom_iterator()
{
    std::cout << "=== 7. Custom Iterator (IntRange) ===\n";

    IntRange range(1, 11);
    std::cout << "  IntRange(1,11): ";
    for (int x : range)
        std::cout << x << " ";
    std::cout << "\n";

    // Works with standard algorithms!
    std::vector<int> v;
    std::copy(range.begin(), range.end(), std::back_inserter(v));
    std::cout << "  copied to vector, sum = " << std::accumulate(v.begin(), v.end(), 0) << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 04 — Iterators                           ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_categories();
    demo_range_access();
    demo_advance_distance();
    demo_insert_iterators();
    demo_stream_iterators();
    demo_iterator_traits();
    demo_custom_iterator();

    std::cout << "All demos complete.\n";
    return 0;
}
