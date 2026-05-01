// ============================================================================
// Lecture 07 — Demo: C++17 Vocabulary Types
// ============================================================================
#include <any>
#include <cassert>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. std::optional
// ──────────────────────────────────────────────────────────────────────────
std::optional<int> find_index(const std::vector<int>& v, int target)
{
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == target)
            return static_cast<int>(i);
    return std::nullopt;
}

void demo_optional()
{
    std::cout << "=== 1. std::optional ===\n";

    std::vector<int> data{10, 20, 30, 40, 50};

    if (auto idx = find_index(data, 30))
    {
        std::cout << "  Found 30 at index " << *idx << "\n";
    }

    auto missing = find_index(data, 99);
    std::cout << "  Looking for 99: " << (missing.has_value() ? "found" : "not found") << "\n";
    std::cout << "  value_or(-1): " << missing.value_or(-1) << "\n";

    // Monadic operations (C++23 has transform/and_then/or_else, but value_or is C++17)
    std::optional<std::string> name = "Alice";
    std::optional<std::string> empty;
    std::cout << "  name: " << name.value_or("(anonymous)") << "\n";
    std::cout << "  empty: " << empty.value_or("(anonymous)") << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::variant
// ──────────────────────────────────────────────────────────────────────────

// Overloaded visitor helper
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

using Value = std::variant<int, double, std::string>;

void demo_variant()
{
    std::cout << "=== 2. std::variant ===\n";

    Value v = 42;
    std::cout << "  Initially int: " << std::get<int>(v) << "\n";

    v = 3.14;
    std::cout << "  Now double: " << std::get<double>(v) << "\n";
    std::cout << "  Index: " << v.index() << "\n";

    v = std::string("hello");
    std::cout << "  Now string: " << std::get<std::string>(v) << "\n";
    std::cout << "  holds_alternative<string>: " << std::boolalpha << std::holds_alternative<std::string>(v) << "\n";

    // Visit with overloaded lambdas
    std::vector<Value> values{42, 2.718, std::string("world")};
    std::cout << "  Visiting values:\n";
    for (const auto& val : values)
    {
        std::visit(
            overloaded{
                [](int i) { std::cout << "    int: " << i << "\n"; },
                [](double d) { std::cout << "    dbl: " << d << "\n"; },
                [](const std::string& s) { std::cout << "    str: " << s << "\n"; },
            },
            val);
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::any
// ──────────────────────────────────────────────────────────────────────────
void demo_any()
{
    std::cout << "=== 3. std::any ===\n";

    std::any a = 42;
    std::cout << "  int: " << std::any_cast<int>(a) << "\n";

    a = std::string("hello any");
    std::cout << "  string: " << std::any_cast<std::string>(a) << "\n";

    a = 3.14;
    std::cout << "  type name: " << a.type().name() << "\n";

    // Safe cast with pointer
    if (auto* p = std::any_cast<double>(&a))
    {
        std::cout << "  double via pointer: " << *p << "\n";
    }

    // Wrong type returns nullptr
    if (auto* p = std::any_cast<int>(&a))
    {
        std::cout << "  This won't print\n";
    }
    else
    {
        std::cout << "  any_cast<int> returned nullptr (correct)\n";
    }

    // Property bag pattern
    std::map<std::string, std::any> props;
    props["width"] = 1920;
    props["title"] = std::string("My Window");
    props["ratio"] = 16.0 / 9.0;
    std::cout << "  props[width] = " << std::any_cast<int>(props["width"]) << "\n";
    std::cout << "  props[title] = " << std::any_cast<std::string>(props["title"]) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::string_view
// ──────────────────────────────────────────────────────────────────────────
void print_trimmed(std::string_view sv)
{
    // Trim leading spaces
    auto start = sv.find_first_not_of(' ');
    if (start == std::string_view::npos)
    {
        std::cout << "    (empty)\n";
        return;
    }
    auto end = sv.find_last_not_of(' ');
    std::cout << "    \"" << sv.substr(start, end - start + 1) << "\"\n";
}

bool starts_with(std::string_view sv, std::string_view prefix)
{
    return sv.size() >= prefix.size() && sv.substr(0, prefix.size()) == prefix;
}

void demo_string_view()
{
    std::cout << "=== 4. std::string_view ===\n";

    // No allocation from string literal
    std::string_view greeting = "Hello, C++17!";
    std::cout << "  literal view: " << greeting << "\n";
    std::cout << "  substr(0,5): " << greeting.substr(0, 5) << "\n";

    // From std::string (no copy)
    std::string s = "  padded string  ";
    print_trimmed(s);

    // Useful for parsing without allocation
    std::string_view csv = "apple,banana,cherry";
    std::cout << "  Splitting CSV:\n";
    size_t pos = 0;
    while (pos < csv.size())
    {
        auto comma = csv.find(',', pos);
        if (comma == std::string_view::npos)
            comma = csv.size();
        std::cout << "    " << csv.substr(pos, comma - pos) << "\n";
        pos = comma + 1;
    }

    // starts_with (manual in C++17, built-in in C++20)
    std::cout << "  starts_with(\"Hello\"): " << std::boolalpha << starts_with(greeting, "Hello") << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 07 — C++17 Vocabulary Types              ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_optional();
    demo_variant();
    demo_any();
    demo_string_view();

    std::cout << "All demos complete.\n";
    return 0;
}
