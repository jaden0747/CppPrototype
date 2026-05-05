// ============================================================================
// Stdlib 05 — Demo: Ranges (C++20)
// ============================================================================
#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Range-based algorithms vs classic
// ──────────────────────────────────────────────────────────────────────────
void demo_range_algorithms()
{
    std::cout << "=== 1. Range Algorithms ===\n";

    std::vector<int> v{5, 3, 1, 4, 2};

    // Classic: std::sort(v.begin(), v.end());
    // Range:
    std::ranges::sort(v);
    std::cout << "  sorted: ";
    for (int x : v)
        std::cout << x << " ";
    std::cout << "\n";

    // ranges::find
    auto it = std::ranges::find(v, 3);
    std::cout << "  find(3): " << (it != v.end() ? "found" : "not found") << "\n";

    // ranges::count_if
    auto cnt = std::ranges::count_if(v, [](int x) { return x > 2; });
    std::cout << "  count(>2): " << cnt << "\n";

    // ranges::reverse
    std::ranges::reverse(v);
    std::cout << "  reversed: ";
    for (int x : v)
        std::cout << x << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Projections
// ──────────────────────────────────────────────────────────────────────────
struct Student
{
    std::string name;
    int         grade;
};

void demo_projections()
{
    std::cout << "=== 2. Projections ===\n";

    std::vector<Student> students{{"Alice", 92}, {"Bob", 87}, {"Charlie", 95}, {"Diana", 88}};

    // Sort by grade (descending) using projection
    std::ranges::sort(students, std::greater{}, &Student::grade);

    std::cout << "  sorted by grade (desc):\n";
    for (const auto& s : students)
    {
        std::cout << "    " << s.name << ": " << s.grade << "\n";
    }

    // Find by name
    auto it = std::ranges::find(students, "Bob", &Student::name);
    if (it != students.end())
    {
        std::cout << "  found Bob, grade=" << it->grade << "\n";
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Views — filter, transform, take, drop
// ──────────────────────────────────────────────────────────────────────────
void demo_views()
{
    std::cout << "=== 3. Views ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // filter: keep only even
    std::cout << "  even: ";
    for (int x : v | std::views::filter([](int x) { return x % 2 == 0; }))
        std::cout << x << " ";
    std::cout << "\n";

    // transform: square each
    std::cout << "  squared: ";
    for (int x : v | std::views::transform([](int x) { return x * x; }))
        std::cout << x << " ";
    std::cout << "\n";

    // take: first 3
    std::cout << "  first 3: ";
    for (int x : v | std::views::take(3))
        std::cout << x << " ";
    std::cout << "\n";

    // drop: skip first 7
    std::cout << "  last 3: ";
    for (int x : v | std::views::drop(7))
        std::cout << x << " ";
    std::cout << "\n";

    // reverse
    std::cout << "  reversed: ";
    for (int x : v | std::views::reverse)
        std::cout << x << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Pipe composition
// ──────────────────────────────────────────────────────────────────────────
void demo_pipe_composition()
{
    std::cout << "=== 4. Pipe Composition ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Chain: even numbers, squared, first 3
    auto pipeline = v | std::views::filter([](int x) { return x % 2 == 0; }) |
                    std::views::transform([](int x) { return x * x; }) | std::views::take(3);

    std::cout << "  even|squared|take(3): ";
    for (int x : pipeline)
        std::cout << x << " ";
    std::cout << "\n";

    // Views are lazy — nothing computed until iterated
    // We can store the pipeline and iterate multiple times
    std::cout << "  iterate again: ";
    for (int x : pipeline)
        std::cout << x << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. iota view — lazy integer sequences
// ──────────────────────────────────────────────────────────────────────────
void demo_iota()
{
    std::cout << "=== 5. std::views::iota ===\n";

    // Bounded
    std::cout << "  iota(1,11): ";
    for (int x : std::views::iota(1, 11))
        std::cout << x << " ";
    std::cout << "\n";

    // Unbounded + take
    std::cout << "  iota(100)|take(5): ";
    for (int x : std::views::iota(100) | std::views::take(5))
        std::cout << x << " ";
    std::cout << "\n";

    // Combine: sum of first 100 squares
    int sum = 0;
    for (int x : std::views::iota(1, 101) | std::views::transform([](int x) { return x * x; }))
        sum += x;
    std::cout << "  sum of 1..100 squared = " << sum << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. split & join views
// ──────────────────────────────────────────────────────────────────────────
void demo_split_join()
{
    std::cout << "=== 6. split view ===\n";

    std::string csv = "Alice,95,A";

    std::cout << "  split(\"" << csv << "\", ','): ";
    for (auto field : csv | std::views::split(','))
    {
        // NOTE: in C++20 the inner range from views::split is only a forward
        // range whose end() returns std::default_sentinel_t — neither the
        // string_view iterator-pair constructor (requires contiguous_iterator)
        // nor std::string's (begin, end) constructor accept it. Materialize
        // the field via std::ranges::copy. C++23's std::string_view range
        // constructor / std::ranges::to would let us avoid the copy.
        std::string s;
        std::ranges::copy(field, std::back_inserter(s));
        std::cout << "[" << s << "] ";
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. keys / values views for maps
// ──────────────────────────────────────────────────────────────────────────
void demo_keys_values()
{
    std::cout << "=== 7. keys / values / elements ===\n";

    std::vector<std::pair<std::string, int>> data{{"alpha", 1}, {"beta", 2}, {"gamma", 3}};

    std::cout << "  keys: ";
    for (const auto& k : data | std::views::keys)
        std::cout << k << " ";
    std::cout << "\n";

    std::cout << "  values: ";
    for (int v : data | std::views::values)
        std::cout << v << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 05 — Ranges (C++20)                      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_range_algorithms();
    demo_projections();
    demo_views();
    demo_pipe_composition();
    demo_iota();
    demo_split_join();
    demo_keys_values();

    std::cout << "All demos complete.\n";
    return 0;
}
