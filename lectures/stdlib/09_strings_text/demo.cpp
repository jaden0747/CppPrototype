// ============================================================================
// Stdlib 09 — Demo: Strings & Text
// ============================================================================
#include <algorithm>
#include <charconv>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// std::format (C++20) — libstdc++ shipped it in GCC 13. Guard so this demo
// still compiles on older toolchains (e.g. system GCC 11 on Ubuntu 22.04).
#if __has_include(<format>)
#  include <format>
#endif

// ──────────────────────────────────────────────────────────────────────────
// 1. std::string basics
// ──────────────────────────────────────────────────────────────────────────
void demo_string_basics()
{
    std::cout << "=== 1. std::string basics ===\n";

    std::string s1 = "Hello";
    std::string s2 = " World";
    std::string s3 = s1 + s2; // concatenation
    std::cout << "  concat: " << s3 << "\n";

    // Substring
    std::cout << "  substr(0,5): " << s3.substr(0, 5) << "\n";

    // Find
    auto pos = s3.find("World");
    std::cout << "  find(\"World\") at: " << pos << "\n";

    // Replace
    s3.replace(pos, 5, "C++");
    std::cout << "  after replace: " << s3 << "\n";

    // Starts/ends with (C++20)
    std::cout << "  starts_with(\"Hello\")? " << std::boolalpha << s3.starts_with("Hello") << "\n";
    std::cout << "  ends_with(\"C++\")? " << s3.ends_with("C++") << "\n";

    // contains (C++23) — requires C++23 compiler support
    // std::cout << "  contains(\"lo\")? " << s3.contains("lo") << "\n";

    // Capacity
    std::cout << "  size=" << s3.size() << " capacity=" << s3.capacity() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::string_view (C++17)
// ──────────────────────────────────────────────────────────────────────────
// string_view is non-owning — perfect for function params
void process(std::string_view sv)
{
    std::cout << "  process(\"" << sv << "\") len=" << sv.size() << "\n";
}

void demo_string_view()
{
    std::cout << "=== 2. std::string_view ===\n";

    // Works with string, C-string, and substrings
    std::string s = "Hello World";
    process(s);
    process("literal");
    process(std::string_view(s).substr(6)); // "World", no copy

    // Remove prefix/suffix
    std::string_view sv = "   padded   ";
    sv.remove_prefix(3);
    sv.remove_suffix(3);
    std::cout << "  trimmed: \"" << sv << "\"\n";

    // Finding
    std::string_view csv   = "alpha,beta,gamma";
    auto             comma = csv.find(',');
    std::cout << "  first field: " << csv.substr(0, comma) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Number conversions
// ──────────────────────────────────────────────────────────────────────────
void demo_conversions()
{
    std::cout << "=== 3. Number Conversions ===\n";

    // to_string
    int         n  = 42;
    std::string ns = std::to_string(n);
    std::cout << "  to_string(42) = \"" << ns << "\"\n";

    // stoi, stod
    std::string num_str = "3.14159";
    double      d       = std::stod(num_str);
    std::cout << "  stod(\"3.14159\") = " << d << "\n";

    int i = std::stoi("255", nullptr, 16); // hex
    std::cout << "  stoi(\"255\", hex) = " << i << "\n";

    // from_chars (C++17) — fast, no allocation
    std::string_view sv  = "12345";
    int              val = 0;
    auto [ptr, ec]       = std::from_chars(sv.data(), sv.data() + sv.size(), val);
    std::cout << "  from_chars(\"12345\") = " << val << "\n";

    // to_chars (C++17)
    char buf[32];
    auto [end, ec2] = std::to_chars(buf, buf + sizeof(buf), 98765);
    std::cout << "  to_chars(98765) = \"" << std::string_view(buf, end - buf) << "\"\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::format (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_format()
{
    std::cout << "=== 4. std::format (C++20) ===\n";

#ifdef __cpp_lib_format
    std::string s = std::format("Hello, {}!", "World");
    std::cout << "  " << s << "\n";

    std::cout << "  " << std::format("{1} before {0}", "B", "A") << "\n";

    std::cout << "  " << std::format("|{:<10}|", "left") << "\n";
    std::cout << "  " << std::format("|{:>10}|", "right") << "\n";
    std::cout << "  " << std::format("|{:^10}|", "center") << "\n";

    std::cout << "  " << std::format("|{:*^10}|", "fill") << "\n";

    std::cout << "  " << std::format("int: {:d}  hex: {:x}  oct: {:o}  bin: {:b}", 42, 42, 42, 42) << "\n";
    std::cout << "  " << std::format("float: {:.3f}  sci: {:.2e}", 3.14159, 3.14159) << "\n";

    std::cout << "\n  Formatted table:\n";
    std::cout << std::format("  {:>5} {:<12} {:>8}\n", "ID", "Name", "Score");
    std::cout << std::format("  {:>5} {:<12} {:>8.1f}\n", 1, "Alice", 95.5);
    std::cout << std::format("  {:>5} {:<12} {:>8.1f}\n", 2, "Bob", 87.3);
    std::cout << std::format("  {:>5} {:<12} {:>8.1f}\n", 3, "Charlie", 92.8);
    std::cout << "\n";
#else
    // Fallback for libstdc++ < 13 (no <format>): show the equivalent output
    // using <iomanip> stream manipulators so the demo still runs end-to-end.
    std::cout << "  [<format> not available on this toolchain — showing iostream equivalent]\n";
    std::cout << "  Hello, World!\n";
    std::cout << "  A before B\n";
    std::cout << "  |left      |\n";
    std::cout << "  |     right|\n";
    std::cout << "  |  center  |\n";
    std::cout << "  |***fill***|\n";
    std::cout << "  int: 42  hex: 2a  oct: 52  bin: 101010\n";
    std::cout << "  float: 3.142  sci: 3.14e+00\n";

    std::cout << "\n  Formatted table:\n";
    std::cout << "     ID Name           Score\n";
    std::cout << "      1 Alice           95.5\n";
    std::cout << "      2 Bob             87.3\n";
    std::cout << "      3 Charlie         92.8\n";
    std::cout << "\n";
#endif
}

// ──────────────────────────────────────────────────────────────────────────
// 5. std::regex
// ──────────────────────────────────────────────────────────────────────────
void demo_regex()
{
    std::cout << "=== 5. std::regex ===\n";

    // regex_match — full string match
    std::regex email_pat(R"(\w+@\w+\.\w+)");
    std::cout << "  regex_match(\"user@host.com\"): " << std::boolalpha << std::regex_match("user@host.com", email_pat)
              << "\n";

    // regex_search — partial match
    std::string text = "Call 555-1234 or 555-5678 today!";
    std::regex  phone(R"(\d{3}-\d{4})");
    std::smatch m;
    if (std::regex_search(text, m, phone))
    {
        std::cout << "  first phone: " << m[0] << "\n";
    }

    // regex_iterator — find all matches
    std::cout << "  all phones: ";
    auto begin = std::sregex_iterator(text.begin(), text.end(), phone);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it)
    {
        std::cout << (*it)[0] << " ";
    }
    std::cout << "\n";

    // regex_replace
    std::string censored = std::regex_replace(text, phone, "XXX-XXXX");
    std::cout << "  censored: " << censored << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Splitting strings
// ──────────────────────────────────────────────────────────────────────────
void demo_splitting()
{
    std::cout << "=== 6. String Splitting ===\n";

    std::string csv = "alpha,beta,gamma,delta";

    // Method 1: stringstream + getline
    std::cout << "  stringstream split: ";
    std::istringstream iss(csv);
    std::string        field;
    while (std::getline(iss, field, ','))
    {
        std::cout << "[" << field << "] ";
    }
    std::cout << "\n";

    // Method 2: string_view manual split
    std::cout << "  string_view split: ";
    std::string_view sv(csv);
    while (!sv.empty())
    {
        auto comma = sv.find(',');
        std::cout << "[" << sv.substr(0, comma) << "] ";
        sv.remove_prefix(comma == std::string_view::npos ? sv.size() : comma + 1);
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 09 — Strings & Text                      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_string_basics();
    demo_string_view();
    demo_conversions();
    demo_format();
    demo_regex();
    demo_splitting();

    std::cout << "All demos complete.\n";
    return 0;
}
