// ============================================================================
// Stdlib 07 — Demo: Modifying & Sorting Algorithms
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <string>
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
// 1. copy, copy_if, copy_n
// ──────────────────────────────────────────────────────────────────────────
void demo_copy()
{
    std::cout << "=== 1. copy / copy_if / copy_n ===\n";

    std::vector<int> src{1, 2, 3, 4, 5, 6, 7, 8};

    // copy
    std::vector<int> dst;
    std::copy(src.begin(), src.end(), std::back_inserter(dst));
    print("copy", dst);

    // copy_if — only even
    std::vector<int> evens;
    std::copy_if(src.begin(), src.end(), std::back_inserter(evens), [](int x) { return x % 2 == 0; });
    print("copy_if(even)", evens);

    // copy_n — first 3
    std::vector<int> first3;
    std::copy_n(src.begin(), 3, std::back_inserter(first3));
    print("copy_n(3)", first3);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. transform
// ──────────────────────────────────────────────────────────────────────────
void demo_transform()
{
    std::cout << "=== 2. transform ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // Unary transform
    std::vector<int> squared;
    std::transform(v.begin(), v.end(), std::back_inserter(squared), [](int x) { return x * x; });
    print("squared", squared);

    // Binary transform — element-wise addition
    std::vector<int> a{10, 20, 30};
    std::vector<int> b{1, 2, 3};
    std::vector<int> sum;
    std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(sum), std::plus<>{});
    print("a + b", sum);

    // Ranges transform — in-place
    std::ranges::transform(v, v.begin(), [](int x) { return x * 2; });
    print("doubled in-place", v);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. fill, generate, iota
// ──────────────────────────────────────────────────────────────────────────
void demo_fill_generate()
{
    std::cout << "=== 3. fill / generate / iota ===\n";

    // fill
    std::vector<int> v(5);
    std::fill(v.begin(), v.end(), 42);
    print("fill(42)", v);

    // generate
    int counter = 0;
    std::generate(v.begin(), v.end(), [&counter]() { return counter++ * 10; });
    print("generate(n*10)", v);

    // iota
    std::vector<int> seq(8);
    std::iota(seq.begin(), seq.end(), 1);
    print("iota(1..)", seq);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. remove, remove_if, erase
// ──────────────────────────────────────────────────────────────────────────
void demo_remove()
{
    std::cout << "=== 4. remove / erase-remove idiom / C++20 erase ===\n";

    // Erase-remove idiom (pre-C++20)
    std::vector<int> v{1, 2, 3, 2, 4, 2, 5};
    print("before", v);
    v.erase(std::remove(v.begin(), v.end(), 2), v.end());
    print("erase-remove(2)", v);

    // C++20 std::erase / std::erase_if
    std::vector<int> v2{1, 2, 3, 4, 5, 6, 7, 8};
    std::erase_if(v2, [](int x) { return x % 2 == 0; });
    print("erase_if(even)", v2);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. replace
// ──────────────────────────────────────────────────────────────────────────
void demo_replace()
{
    std::cout << "=== 5. replace / replace_if ===\n";

    std::vector<int> v{1, 2, 3, 2, 4, 2, 5};
    std::replace(v.begin(), v.end(), 2, 99);
    print("replace(2->99)", v);

    std::replace_if(v.begin(), v.end(), [](int x) { return x > 50; }, 0);
    print("replace_if(>50->0)", v);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. reverse, rotate, shuffle
// ──────────────────────────────────────────────────────────────────────────
void demo_reorder()
{
    std::cout << "=== 6. reverse / rotate / shuffle ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    std::ranges::reverse(v);
    print("reversed", v);

    // rotate: make element at index 2 the new front
    std::ranges::reverse(v); // restore
    std::rotate(v.begin(), v.begin() + 2, v.end());
    print("rotate(+2)", v);

    // shuffle
    std::mt19937 rng(42);
    std::shuffle(v.begin(), v.end(), rng);
    print("shuffled", v);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. unique
// ──────────────────────────────────────────────────────────────────────────
void demo_unique()
{
    std::cout << "=== 7. unique ===\n";

    std::vector<int> v{1, 1, 2, 2, 3, 3, 3, 4, 5, 5};
    print("before", v);

    // unique removes consecutive duplicates
    v.erase(std::unique(v.begin(), v.end()), v.end());
    print("unique", v);

    // For non-sorted: sort first, then unique
    std::vector<int> v2{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    std::ranges::sort(v2);
    v2.erase(std::unique(v2.begin(), v2.end()), v2.end());
    print("sort+unique", v2);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 8. Sorting algorithms
// ──────────────────────────────────────────────────────────────────────────
void demo_sorting()
{
    std::cout << "=== 8. Sorting Algorithms ===\n";

    // sort
    std::vector<int> v{5, 3, 1, 4, 2, 8, 7, 6};
    std::ranges::sort(v);
    print("sort", v);

    // sort descending
    std::ranges::sort(v, std::greater{});
    print("sort desc", v);

    // partial_sort — sort only first 3
    std::vector<int> v2{9, 5, 3, 7, 1, 8, 2, 4, 6};
    std::partial_sort(v2.begin(), v2.begin() + 3, v2.end());
    print("partial_sort(3)", v2);

    // nth_element — O(n), partition around nth
    std::vector<int> v3{9, 5, 3, 7, 1, 8, 2, 4, 6};
    std::nth_element(v3.begin(), v3.begin() + 4, v3.end());
    std::cout << "  nth_element(4): median=" << v3[4] << "\n";
    print("after nth_element", v3);

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 9. Partitioning
// ──────────────────────────────────────────────────────────────────────────
void demo_partition()
{
    std::cout << "=== 9. partition / stable_partition ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
    auto             mid = std::partition(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
    print("partition(even)", v);
    std::cout << "  partition point at index " << std::distance(v.begin(), mid) << "\n";

    // stable_partition preserves relative order
    std::vector<int> v2{1, 2, 3, 4, 5, 6, 7, 8};
    std::stable_partition(v2.begin(), v2.end(), [](int x) { return x % 2 == 0; });
    print("stable_partition(even)", v2);

    // is_partitioned / partition_point
    bool part = std::is_partitioned(v2.begin(), v2.end(), [](int x) { return x % 2 == 0; });
    std::cout << "  is_partitioned? " << std::boolalpha << part << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 07 — Modifying & Sorting Algorithms      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_copy();
    demo_transform();
    demo_fill_generate();
    demo_remove();
    demo_replace();
    demo_reorder();
    demo_unique();
    demo_sorting();
    demo_partition();

    std::cout << "All demos complete.\n";
    return 0;
}
