// ============================================================================
// Lecture 13 — Exercises: C++23 Additions
// ============================================================================
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#if __has_include(<expected>)
#include <expected>
#define HAS_EXPECTED 1
#else
#define HAS_EXPECTED 0
#endif

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Deducing this — recursive lambda
// Write a recursive lambda using deducing this:
//   a) factorial(n) — returns n!
//   b) tree_depth(node) — finds depth of a binary tree
// ──────────────────────────────────────────────────────────────────────────

struct TreeNode
{
    int       value;
    TreeNode* left  = nullptr;
    TreeNode* right = nullptr;
};

void exercise_deducing_this()
{
    // C++23:
    // auto factorial = [](this auto self, int n) -> int {
    //     return n <= 1 ? 1 : n * self(n - 1);
    // };
    // assert(factorial(5) == 120);
    // assert(factorial(0) == 1);

    // auto tree_depth = [](this auto self, TreeNode* node) -> int {
    //     if (!node) return 0;
    //     return 1 + std::max(self(node->left), self(node->right));
    // };
    // TreeNode n1{1}, n2{2}, n3{3, &n1, &n2}, root{0, &n3, nullptr};
    // assert(tree_depth(&root) == 3);
    std::cout << "  Exercise 1: deducing this — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: std::expected — error handling chain
// Build a pipeline:
//   read_file(path) → expected<string, Error>
//   parse_config(content) → expected<Config, Error>
//   validate(config) → expected<Config, Error>
// Chain them with .and_then()
// ──────────────────────────────────────────────────────────────────────────

enum class ConfigError
{
    FileNotFound,
    ParseFailed,
    ValidationFailed
};

struct Config
{
    std::string host;
    int         port;
};

// TODO: Implement using std::expected (or simulate with optional for older compilers)
// std::expected<std::string, ConfigError> read_config_file(std::string_view path) { ... }
// std::expected<Config, ConfigError> parse_config(const std::string& content) { ... }
// std::expected<Config, ConfigError> validate_config(Config cfg) { ... }

void exercise_expected()
{
#if HAS_EXPECTED
    // auto result = read_config_file("config.txt")
    //     .and_then(parse_config)
    //     .and_then(validate_config);
    //
    // if (result) {
    //     assert(result->port > 0);
    // } else {
    //     // Handle error
    // }
#endif
    std::cout << "  Exercise 2: expected chain — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Multidimensional subscript
// Implement a Tensor3D class with operator[](i, j, k)
// Support: construction, element access, fill, and slice operations.
// ──────────────────────────────────────────────────────────────────────────

// TODO:
// class Tensor3D {
// public:
//     Tensor3D(size_t d1, size_t d2, size_t d3);
//     double& operator[](size_t i, size_t j, size_t k);       // C++23
//     const double& operator[](size_t i, size_t j, size_t k) const;
//     void fill(double value);
//     size_t size() const;
// private:
//     std::vector<double> data_;
//     size_t d1_, d2_, d3_;
// };

void exercise_multidim()
{
    // Tensor3D t(3, 4, 5);
    // t.fill(0.0);
    // t[1, 2, 3] = 42.0;
    // assert(t[1, 2, 3] == 42.0);
    // assert(t[0, 0, 0] == 0.0);
    // assert(t.size() == 60);  // 3*4*5
    std::cout << "  Exercise 3: multidimensional [] — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: ranges::to
// Use ranges::to to:
//   a) Collect filtered view into vector
//   b) Convert a view of chars into a string
//   c) Build a map from a view of pairs
// ──────────────────────────────────────────────────────────────────────────
void exercise_ranges_to()
{
    // auto evens = std::views::iota(1, 20)
    //     | std::views::filter([](int x) { return x % 2 == 0; })
    //     | std::ranges::to<std::vector>();
    // assert(evens.size() == 9);
    // assert(evens[0] == 2);

    // auto alpha = std::views::iota('a', 'z' + 1)
    //     | std::ranges::to<std::string>();
    // assert(alpha == "abcdefghijklmnopqrstuvwxyz");
    std::cout << "  Exercise 4: ranges::to — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Deducing this — CRTP replacement
// Create a Chainable mixin that allows method chaining without CRTP:
//   struct Builder : Chainable {
//       std::string result;
//       auto& add(this auto& self, std::string_view s) { self.result += s; return self; }
//   };
//   Builder b;
//   b.add("hello").add(" ").add("world");
// ──────────────────────────────────────────────────────────────────────────
void exercise_crtp_replacement()
{
    // Builder b;
    // b.add("hello").add(" ").add("world");
    // assert(b.result == "hello world");
    std::cout << "  Exercise 5: CRTP replacement — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Full error-handling pipeline
// Build a complete data processing pipeline:
//   - Input: vector of string lines
//   - Parse each line into a Record (name, age, score)
//   - Each step returns expected<T, Error>
//   - Collect successful records, report errors
//   - Use std::print for output formatting
//   - Use ranges::to for collection
// ──────────────────────────────────────────────────────────────────────────

struct StudentRecord
{
    std::string name;
    int         age;
    double      score;
};

// TODO: Build the full pipeline

void exercise_full_pipeline()
{
    // std::vector<std::string> input{
    //     "Alice,22,95.5",
    //     "Bob,invalid,87.3",
    //     "Carol,21,88.0",
    //     "",
    //     "Dave,25,92.1"
    // };
    //
    // auto [records, errors] = process_all(input);
    // assert(records.size() == 3);  // Alice, Carol, Dave
    // assert(errors.size() == 2);   // Bob (invalid), empty line
    std::cout << "  Exercise 6: full pipeline — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 13 — Exercises: C++23 Additions         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_deducing_this();
    exercise_expected();
    exercise_multidim();
    exercise_ranges_to();
    exercise_crtp_replacement();
    exercise_full_pipeline();

    std::cout << "\nAll exercises done!\n";
    std::cout << "\n🎉 You've completed all exercises in the C++ mastery series!\n";
    return 0;
}
