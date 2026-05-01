// ============================================================================
// Stdlib 08 — Demo: Numeric, Set & Heap Algorithms
// ============================================================================
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

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
// 1. accumulate, reduce, transform_reduce
// ──────────────────────────────────────────────────────────────────────────
void demo_accumulate()
{
    std::cout << "=== 1. accumulate / reduce / transform_reduce ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // accumulate — fold left
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "  accumulate sum = " << sum << "\n";

    // product
    int prod = std::accumulate(v.begin(), v.end(), 1, std::multiplies<>{});
    std::cout << "  accumulate product = " << prod << "\n";

    // reduce (C++17) — unordered, parallelisable
    int rsum = std::reduce(v.begin(), v.end(), 0);
    std::cout << "  reduce sum = " << rsum << "\n";

    // transform_reduce — dot product
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    int              dot = std::transform_reduce(a.begin(), a.end(), b.begin(), 0);
    std::cout << "  dot product = " << dot << " (1*4 + 2*5 + 3*6 = 32)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. partial_sum, inclusive_scan, exclusive_scan
// ──────────────────────────────────────────────────────────────────────────
void demo_partial_sum()
{
    std::cout << "=== 2. partial_sum / inclusive_scan / exclusive_scan ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // partial_sum — running total
    std::vector<int> ps;
    std::partial_sum(v.begin(), v.end(), std::back_inserter(ps));
    print("partial_sum", ps);

    // inclusive_scan (C++17)
    std::vector<int> is;
    std::inclusive_scan(v.begin(), v.end(), std::back_inserter(is));
    print("inclusive_scan", is);

    // exclusive_scan (C++17) — starts with init value
    std::vector<int> es;
    std::exclusive_scan(v.begin(), v.end(), std::back_inserter(es), 0);
    print("exclusive_scan(init=0)", es);

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. adjacent_difference
// ──────────────────────────────────────────────────────────────────────────
void demo_adjacent_diff()
{
    std::cout << "=== 3. adjacent_difference ===\n";

    std::vector<int> v{1, 3, 6, 10, 15};
    std::vector<int> diff;
    std::adjacent_difference(v.begin(), v.end(), std::back_inserter(diff));
    print("adjacent_difference", diff); // [1, 2, 3, 4, 5]
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. gcd, lcm, midpoint, lerp
// ──────────────────────────────────────────────────────────────────────────
void demo_math_utils()
{
    std::cout << "=== 4. gcd / lcm / midpoint / lerp ===\n";

    std::cout << "  gcd(12, 8) = " << std::gcd(12, 8) << "\n";
    std::cout << "  lcm(12, 8) = " << std::lcm(12, 8) << "\n";
    std::cout << "  midpoint(10, 20) = " << std::midpoint(10, 20) << "\n";
    std::cout << "  lerp(0.0, 10.0, 0.3) = " << std::lerp(0.0, 10.0, 0.3) << "\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Set operations (on sorted ranges)
// ──────────────────────────────────────────────────────────────────────────
void demo_set_ops()
{
    std::cout << "=== 5. Set Operations ===\n";

    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{3, 4, 5, 6, 7};

    std::vector<int> result;

    // union
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    print("union", result);

    // intersection
    result.clear();
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    print("intersection", result);

    // difference (a - b)
    result.clear();
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    print("difference(a-b)", result);

    // symmetric difference
    result.clear();
    std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    print("symmetric_diff", result);

    // includes
    std::vector<int> sub{2, 3, 4};
    bool             inc = std::includes(a.begin(), a.end(), sub.begin(), sub.end());
    std::cout << "  a includes {2,3,4}? " << std::boolalpha << inc << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Heap operations
// ──────────────────────────────────────────────────────────────────────────
void demo_heap()
{
    std::cout << "=== 6. Heap Operations ===\n";

    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};

    // make_heap — turn into max-heap
    std::make_heap(v.begin(), v.end());
    print("make_heap", v);
    std::cout << "  heap top (max) = " << v.front() << "\n";

    // push_heap — add element
    v.push_back(8);
    std::push_heap(v.begin(), v.end());
    std::cout << "  after push(8), top = " << v.front() << "\n";

    // pop_heap — remove max
    std::pop_heap(v.begin(), v.end());
    int popped = v.back();
    v.pop_back();
    std::cout << "  popped " << popped << ", new top = " << v.front() << "\n";

    // sort_heap — sort in ascending order
    std::sort_heap(v.begin(), v.end());
    print("sort_heap", v);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. Binary search
// ──────────────────────────────────────────────────────────────────────────
void demo_binary_search()
{
    std::cout << "=== 7. Binary Search ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    bool found = std::binary_search(v.begin(), v.end(), 7);
    std::cout << "  binary_search(7)? " << std::boolalpha << found << "\n";

    auto lb = std::lower_bound(v.begin(), v.end(), 5);
    std::cout << "  lower_bound(5) at index " << std::distance(v.begin(), lb) << "\n";

    auto ub = std::upper_bound(v.begin(), v.end(), 5);
    std::cout << "  upper_bound(5) at index " << std::distance(v.begin(), ub) << "\n";

    auto [lo, hi] = std::equal_range(v.begin(), v.end(), 5);
    std::cout << "  equal_range(5): [" << std::distance(v.begin(), lo) << ", " << std::distance(v.begin(), hi)
              << ")\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 8. <bit> utilities (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_bit()
{
    std::cout << "=== 8. <bit> Utilities (C++20) ===\n";

    uint32_t x = 0b0010'1100;
    std::cout << "  x = " << x << " (0b00101100)\n";
    std::cout << "  popcount = " << std::popcount(x) << "\n";
    std::cout << "  countl_zero = " << std::countl_zero(x) << "\n";
    std::cout << "  countr_zero = " << std::countr_zero(x) << "\n";
    std::cout << "  has_single_bit(8) = " << std::boolalpha << std::has_single_bit(8u) << "\n";
    std::cout << "  has_single_bit(7) = " << std::has_single_bit(7u) << "\n";
    std::cout << "  bit_ceil(5) = " << std::bit_ceil(5u) << "\n";
    std::cout << "  bit_floor(5) = " << std::bit_floor(5u) << "\n";

    // bit_cast — safe reinterpretation
    float f    = 1.0f;
    auto  bits = std::bit_cast<uint32_t>(f);
    std::cout << "  bit_cast<uint32_t>(1.0f) = 0x" << std::hex << bits << std::dec << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 9. <random>
// ──────────────────────────────────────────────────────────────────────────
void demo_random()
{
    std::cout << "=== 9. <random> ===\n";

    std::mt19937 rng(42); // seeded Mersenne Twister

    // Uniform int [1, 6]
    std::uniform_int_distribution<int> dice(1, 6);
    std::cout << "  dice rolls: ";
    for (int i = 0; i < 10; ++i)
        std::cout << dice(rng) << " ";
    std::cout << "\n";

    // Uniform real [0.0, 1.0)
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::cout << "  uniform real: ";
    for (int i = 0; i < 5; ++i)
        std::cout << unit(rng) << " ";
    std::cout << "\n";

    // Normal distribution
    std::normal_distribution<double> norm(100.0, 15.0);
    std::cout << "  normal(100,15): ";
    for (int i = 0; i < 5; ++i)
        std::cout << static_cast<int>(norm(rng)) << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 08 — Numeric, Set & Heap Algorithms      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_accumulate();
    demo_partial_sum();
    demo_adjacent_diff();
    demo_math_utils();
    demo_set_ops();
    demo_heap();
    demo_binary_search();
    demo_bit();
    demo_random();

    std::cout << "All demos complete.\n";
    return 0;
}
