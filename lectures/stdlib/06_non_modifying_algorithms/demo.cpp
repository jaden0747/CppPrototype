// ============================================================================
// Stdlib 06 — Demo: Non-Modifying Algorithms
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. find, find_if, find_if_not
// ──────────────────────────────────────────────────────────────────────────
void demo_find()
{
    std::cout << "=== 1. find / find_if / find_if_not ===\n";

    std::vector<int> v{10, 20, 30, 40, 50};

    // find exact value
    auto it = std::find(v.begin(), v.end(), 30);
    std::cout << "  find(30): index " << std::distance(v.begin(), it) << "\n";

    // find_if with predicate
    auto it2 = std::find_if(v.begin(), v.end(), [](int x) { return x > 25; });
    std::cout << "  find_if(>25): " << *it2 << "\n";

    // Ranges version
    auto it3 = std::ranges::find(v, 40);
    std::cout << "  ranges::find(40): " << *it3 << "\n";

    // find_if_not
    auto it4 = std::find_if_not(v.begin(), v.end(), [](int x) { return x < 35; });
    std::cout << "  find_if_not(<35): " << *it4 << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. count, count_if
// ──────────────────────────────────────────────────────────────────────────
void demo_count()
{
    std::cout << "=== 2. count / count_if ===\n";

    std::vector<int> v{1, 2, 3, 2, 4, 2, 5, 2};

    auto n = std::count(v.begin(), v.end(), 2);
    std::cout << "  count(2) = " << n << "\n";

    auto even = std::ranges::count_if(v, [](int x) { return x % 2 == 0; });
    std::cout << "  count_if(even) = " << even << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. all_of, any_of, none_of
// ──────────────────────────────────────────────────────────────────────────
void demo_predicates()
{
    std::cout << "=== 3. all_of / any_of / none_of ===\n";

    std::vector<int> v{2, 4, 6, 8, 10};

    bool all_even = std::ranges::all_of(v, [](int x) { return x % 2 == 0; });
    std::cout << "  all_of(even)? " << std::boolalpha << all_even << "\n";

    bool any_gt5 = std::ranges::any_of(v, [](int x) { return x > 5; });
    std::cout << "  any_of(>5)? " << any_gt5 << "\n";

    bool none_neg = std::ranges::none_of(v, [](int x) { return x < 0; });
    std::cout << "  none_of(<0)? " << none_neg << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. for_each
// ──────────────────────────────────────────────────────────────────────────
void demo_for_each()
{
    std::cout << "=== 4. for_each ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // for_each — apply function to each element
    int sum = 0;
    std::for_each(v.begin(), v.end(), [&sum](int x) { sum += x; });
    std::cout << "  sum via for_each = " << sum << "\n";

    // for_each_n — first n elements only
    std::cout << "  first 3: ";
    std::for_each_n(v.begin(), 3, [](int x) { std::cout << x << " "; });
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. mismatch, equal
// ──────────────────────────────────────────────────────────────────────────
void demo_mismatch_equal()
{
    std::cout << "=== 5. mismatch / equal ===\n";

    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{1, 2, 9, 4, 5};

    // mismatch
    auto [it_a, it_b] = std::mismatch(a.begin(), a.end(), b.begin());
    std::cout << "  mismatch at index " << std::distance(a.begin(), it_a) << ": " << *it_a << " vs " << *it_b << "\n";

    // equal
    std::vector<int> c{1, 2, 3, 4, 5};
    bool             eq = std::ranges::equal(a, c);
    std::cout << "  a == c? " << std::boolalpha << eq << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. search, adjacent_find
// ──────────────────────────────────────────────────────────────────────────
void demo_search()
{
    std::cout << "=== 6. search / adjacent_find ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> pattern{3, 4, 5};

    // search — find subsequence
    auto it = std::search(v.begin(), v.end(), pattern.begin(), pattern.end());
    if (it != v.end())
    {
        std::cout << "  pattern found at index " << std::distance(v.begin(), it) << "\n";
    }

    // adjacent_find — first pair of equal neighbors
    std::vector<int> v2{1, 2, 3, 3, 4, 5};
    auto             adj = std::adjacent_find(v2.begin(), v2.end());
    if (adj != v2.end())
    {
        std::cout << "  adjacent pair: " << *adj << ", " << *(adj + 1) << " at index " << std::distance(v2.begin(), adj)
                  << "\n";
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. min/max element
// ──────────────────────────────────────────────────────────────────────────
void demo_minmax()
{
    std::cout << "=== 7. min_element / max_element / minmax_element ===\n";

    std::vector<int> v{42, 17, 8, 99, 3, 55, 21};

    auto mn = std::ranges::min_element(v);
    auto mx = std::ranges::max_element(v);
    std::cout << "  min = " << *mn << " at index " << std::distance(v.begin(), mn) << "\n";
    std::cout << "  max = " << *mx << " at index " << std::distance(v.begin(), mx) << "\n";

    auto [lo, hi] = std::ranges::minmax_element(v);
    std::cout << "  minmax = [" << *lo << ", " << *hi << "]\n";

    // With projection
    struct Item
    {
        std::string name;
        int         price;
    };
    std::vector<Item> items{{"A", 30}, {"B", 10}, {"C", 50}};
    auto              cheapest = std::ranges::min_element(items, {}, &Item::price);
    std::cout << "  cheapest item: " << cheapest->name << " ($" << cheapest->price << ")\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 06 — Non-Modifying Algorithms            ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_find();
    demo_count();
    demo_predicates();
    demo_for_each();
    demo_mismatch_equal();
    demo_search();
    demo_minmax();

    std::cout << "All demos complete.\n";
    return 0;
}
