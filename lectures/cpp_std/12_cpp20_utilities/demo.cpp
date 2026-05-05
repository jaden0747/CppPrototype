// ============================================================================
// Lecture 12 — Demo: C++20 Utilities
// ============================================================================
#include <algorithm>
#include <compare>
#include <iostream>
#include <source_location>
#include <span>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

// Note: std::format may need <format> header and compiler support.
// If your compiler doesn't support <format>, see the fallback section.
#if __has_include(<format>)
#include <format>
#define HAS_FORMAT 1
#else
#define HAS_FORMAT 0
#endif

// ──────────────────────────────────────────────────────────────────────────
// 1. Three-way comparison (spaceship operator)
// ──────────────────────────────────────────────────────────────────────────
struct Point3D
{
    double x, y, z;
    auto   operator<=>(const Point3D&) const = default;
};

struct Version
{
    int maj, min, patch;
    // Explicit spaceship: demonstrates <=> while giving IntelliSense a visible body
    std::strong_ordering operator<=>(const Version& o) const
    {
        if (auto c = maj <=> o.maj; c != 0)
            return c;
        if (auto c = min <=> o.min; c != 0)
            return c;
        return patch <=> o.patch;
    }
    bool operator==(const Version& o) const
    {
        return maj == o.maj && min == o.min && patch == o.patch;
    }
    bool operator<(const Version& o) const
    {
        return std::tie(maj, min, patch) < std::tie(o.maj, o.min, o.patch);
    }
};

struct CaseInsensitive
{
    std::string data;

    std::weak_ordering operator<=>(const CaseInsensitive& other) const
    {
        auto lower = [](std::string s)
        {
            std::ranges::transform(s, s.begin(), ::tolower);
            return s;
        };
        return lower(data) <=> lower(other.data);
    }
    bool operator==(const CaseInsensitive& other) const
    {
        return (*this <=> other) == std::weak_ordering::equivalent;
    }
};

void demo_spaceship()
{
    std::cout << "=== 1. Spaceship Operator <=> ===\n";

    Point3D a{1, 2, 3}, b{1, 2, 4};
    std::cout << "  Point3D(1,2,3) < (1,2,4): " << std::boolalpha << (a < b) << "\n";
    std::cout << "  Point3D(1,2,3) == (1,2,3): " << (a == Point3D{1, 2, 3}) << "\n";

    Version v1{2, 1, 0}, v2{2, 0, 9};
    std::cout << "  Version 2.1.0 > 2.0.9: " << (v1 > v2) << "\n";

    // Sort versions — std::sort uses operator<, which is driven by our <=>
    std::vector<Version> versions{{1, 0, 0}, {2, 1, 0}, {1, 9, 9}, {2, 0, 1}};
    std::sort(versions.begin(), versions.end());
    std::cout << "  Sorted versions: ";
    for (auto& v : versions)
        std::cout << v.maj << "." << v.min << "." << v.patch << " ";
    std::cout << "\n";

    CaseInsensitive s1{"Hello"}, s2{"hello"};
    std::cout << "  CaseInsensitive: \"Hello\" == \"hello\": " << (s1 == s2) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::format
// ──────────────────────────────────────────────────────────────────────────
void demo_format()
{
    std::cout << "=== 2. std::format ===\n";

#if HAS_FORMAT
    std::cout << "  " << std::format("Hello, {}!", "world") << "\n";
    std::cout << "  " << std::format("{:>10}", 42) << "\n";
    std::cout << "  " << std::format("{:.2f}", 3.14159) << "\n";
    std::cout << "  " << std::format("{:#x}", 255) << "\n";
    std::cout << "  " << std::format("{:*^20}", "center") << "\n";

    // Table formatting
    std::cout << "  " << std::format("{:<10} {:>5} {:>8}\n", "Name", "Age", "Score");
    std::cout << "  " << std::format("{:<10} {:>5} {:>8.1f}\n", "Alice", 30, 95.5);
    std::cout << "  " << std::format("{:<10} {:>5} {:>8.1f}\n", "Bob", 25, 87.3);
#else
    std::cout << "  <format> not available on this compiler.\n";
    std::cout << "  Install fmt library or use GCC 13+ / Clang 17+ / MSVC 2022.\n";
#endif
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::span
// ──────────────────────────────────────────────────────────────────────────
void print_span(std::span<const int> data)
{
    std::cout << "  [";
    for (size_t i = 0; i < data.size(); ++i)
    {
        if (i > 0)
            std::cout << ", ";
        std::cout << data[i];
    }
    std::cout << "] (size=" << data.size() << ")\n";
}

double average(std::span<const double> values)
{
    double sum = 0;
    for (double v : values)
        sum += v;
    return values.empty() ? 0 : sum / values.size();
}

void demo_span()
{
    std::cout << "=== 3. std::span ===\n";

    // From vector
    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "  full: ";
    print_span(vec);

    // Subspan
    std::span<const int> s(vec);
    std::cout << "  first(3): ";
    print_span(s.first(3));
    std::cout << "  last(3): ";
    print_span(s.last(3));
    std::cout << "  subspan(2,4): ";
    print_span(s.subspan(2, 4));

    // From C-array
    int arr[] = {10, 20, 30};
    std::cout << "  C-array: ";
    print_span(arr);

    // Average
    std::vector<double> grades{85.5, 92.0, 78.5, 95.0, 88.5};
    std::cout << "  average: " << average(grades) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::jthread & stop tokens
// ──────────────────────────────────────────────────────────────────────────
void demo_jthread()
{
    std::cout << "=== 4. std::jthread & Stop Tokens ===\n";

    // Basic jthread (auto-joins)
    {
        std::jthread t([] { std::cout << "  Worker ran!\n"; });
        // ~jthread auto-joins here
    }
    std::cout << "  (jthread auto-joined)\n";

    // With stop token
    {
        std::jthread t(
            [](std::stop_token st)
            {
                int count = 0;
                while (!st.stop_requested() && count < 5)
                {
                    ++count;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                std::cout << "  Worker did " << count << " iterations before stop\n";
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        t.request_stop();
    }

    // Stop callback
    {
        std::jthread t(
            [](std::stop_token st)
            {
                std::stop_callback cb(st, [] { std::cout << "  Stop callback fired!\n"; });
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            });
        t.request_stop();
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. std::source_location
// ──────────────────────────────────────────────────────────────────────────
void log_message(std::string_view msg, std::source_location loc = std::source_location::current())
{
    std::cout << "  [" << loc.file_name() << ":" << loc.line() << " " << loc.function_name() << "] " << msg << "\n";
}

void demo_source_location()
{
    std::cout << "=== 5. std::source_location ===\n";
    log_message("This is a log message");
    log_message("Another from here");

    auto loc = std::source_location::current();
    std::cout << "  Current: " << loc.file_name() << ":" << loc.line() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 12 — C++20 Utilities                    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_spaceship();
    demo_format();
    demo_span();
    demo_jthread();
    demo_source_location();

    std::cout << "All demos complete.\n";
    return 0;
}
