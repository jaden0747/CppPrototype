// ============================================================================
// Lecture 10 — Demo: C++20 Ranges
// ============================================================================
#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Basic range algorithms
// ──────────────────────────────────────────────────────────────────────────
void demo_range_algorithms()
{
    std::cout << "=== 1. Range Algorithms ===\n";

    std::vector<int> v{5, 2, 8, 1, 9, 3, 7, 4, 6};

    // Sort
    std::ranges::sort(v);
    std::cout << "  sorted: ";
    for (int x : v)
        std::cout << x << " ";
    std::cout << "\n";

    // Find
    auto it = std::ranges::find(v, 7);
    std::cout << "  find(7): " << *it << " at pos " << std::distance(v.begin(), it) << "\n";

    // Count
    auto cnt = std::ranges::count_if(v, [](int x) { return x > 5; });
    std::cout << "  count(>5): " << cnt << "\n";

    // Min/Max
    auto [mn, mx] = std::ranges::minmax(v);
    std::cout << "  min=" << mn << " max=" << mx << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Views and pipes
// ──────────────────────────────────────────────────────────────────────────
void demo_views()
{
    std::cout << "=== 2. Views & Pipes ===\n";

    // iota: generate a range
    std::cout << "  iota(1,11): ";
    for (int x : std::views::iota(1, 11))
        std::cout << x << " ";
    std::cout << "\n";

    // Filter + transform
    auto evens_squared = std::views::iota(1, 11) | std::views::filter([](int x) { return x % 2 == 0; }) |
                         std::views::transform([](int x) { return x * x; });

    std::cout << "  evens squared: ";
    for (int x : evens_squared)
        std::cout << x << " ";
    std::cout << "\n";

    // take + drop
    auto middle = std::views::iota(1, 20) | std::views::drop(5) | std::views::take(5);
    std::cout << "  drop(5)|take(5): ";
    for (int x : middle)
        std::cout << x << " ";
    std::cout << "\n";

    // reverse
    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "  reversed: ";
    for (int x : v | std::views::reverse)
        std::cout << x << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Projections
// ──────────────────────────────────────────────────────────────────────────
struct Person
{
    std::string name;
    int         age;
};

void demo_projections()
{
    std::cout << "=== 3. Projections ===\n";

    std::vector<Person> people{{"Alice", 30}, {"Bob", 25}, {"Carol", 35}, {"Dave", 28}};

    // Sort by age
    std::ranges::sort(people, {}, &Person::age);
    std::cout << "  Sorted by age:\n";
    for (const auto& p : people)
        std::cout << "    " << p.name << " (" << p.age << ")\n";

    // Find by name
    auto it = std::ranges::find(people, "Carol", &Person::name);
    if (it != people.end())
        std::cout << "  Found Carol, age " << it->age << "\n";

    // Max by age
    auto oldest = std::ranges::max(people, {}, &Person::age);
    std::cout << "  Oldest: " << oldest.name << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Composing complex pipelines
// ──────────────────────────────────────────────────────────────────────────
void demo_complex_pipeline()
{
    std::cout << "=== 4. Complex Pipeline ===\n";

    // Generate first 10 fibonacci-like numbers using views
    // (Not actual fibonacci — demonstrating composition)
    auto squares_of_odds = std::views::iota(1, 50) | std::views::filter([](int x) { return x % 2 != 0; }) |
                           std::views::transform([](int x) { return x * x; }) | std::views::take(10);

    std::cout << "  First 10 odd squares: ";
    for (int x : squares_of_odds)
        std::cout << x << " ";
    std::cout << "\n";

    // Chaining multiple operations on data
    std::vector<std::string> words{"hello", "world", "foo", "bar", "baz", "qux"};

    auto long_words_upper = words | std::views::filter([](const std::string& s) { return s.size() > 3; }) |
                            std::views::transform(
                                [](std::string s)
                                {
                                    std::ranges::transform(s, s.begin(), ::toupper);
                                    return s;
                                });

    std::cout << "  Long words uppercased: ";
    for (const auto& w : long_words_upper)
        std::cout << w << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. keys/values views
// ──────────────────────────────────────────────────────────────────────────
void demo_elements_view()
{
    std::cout << "=== 5. keys/values Views ===\n";

    std::vector<std::pair<std::string, int>> scores{{"Alice", 95}, {"Bob", 87}, {"Carol", 92}};

    std::cout << "  keys: ";
    for (const auto& k : scores | std::views::keys)
        std::cout << k << " ";
    std::cout << "\n";

    std::cout << "  values: ";
    for (int v : scores | std::views::values)
        std::cout << v << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Laziness demonstration
// ──────────────────────────────────────────────────────────────────────────
void demo_laziness()
{
    std::cout << "=== 6. Laziness ===\n";
    std::cout << "  Processing (filter+transform, take 3):\n";

    auto pipeline = std::views::iota(1, 100) |
                    std::views::filter(
                        [](int x)
                        {
                            std::cout << "    filter(" << x << ")\n";
                            return x % 3 == 0;
                        }) |
                    std::views::transform(
                        [](int x)
                        {
                            std::cout << "    transform(" << x << ")\n";
                            return x * 10;
                        }) |
                    std::views::take(3);

    std::cout << "  Results: ";
    for (int x : pipeline)
        std::cout << x << " ";
    std::cout << "\n  (Notice: only evaluated as needed!)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 10 — C++20 Ranges                       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_range_algorithms();
    demo_views();
    demo_projections();
    demo_complex_pipeline();
    demo_elements_view();
    demo_laziness();

    std::cout << "All demos complete.\n";
    return 0;
}
