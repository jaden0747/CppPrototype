// ============================================================================
// Stdlib 03 — Exercises: Unordered Containers & Adaptors
// ============================================================================
#include <cassert>
#include <functional>
#include <iostream>
#include <queue>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ── Exercise 1: Anagram grouping ─────────────────────────────────────────
void ex1_anagram_groups()
{
    std::cout << "Exercise 1: Anagram grouping\n";
    std::vector<std::string> words{"eat", "tea", "tan", "ate", "nat", "bat"};

    // TODO: Group anagrams together using unordered_map<string, vector<string>>
    //        Hint: sort each word to get the anagram key
    // TODO: Print each group

    std::cout << "\n";
}

// ── Exercise 2: Two-sum with unordered_map ───────────────────────────────
void ex2_two_sum()
{
    std::cout << "Exercise 2: Two-sum\n";
    std::vector<int> nums{2, 7, 11, 15};
    int              target = 9;

    // TODO: Find two indices i,j such that nums[i]+nums[j]==target
    //        Use unordered_map<int,int> to map value -> index
    // TODO: Print the pair of indices

    std::cout << "\n";
}

// ── Exercise 3: Custom hash for a struct ─────────────────────────────────
struct Employee
{
    std::string name;
    int         id;
    bool        operator==(const Employee&) const = default;
};

// TODO: Write a hash function for Employee (combine name and id hashes)

void ex3_custom_hash()
{
    std::cout << "Exercise 3: Custom hash\n";

    // TODO: Create unordered_set<Employee, EmployeeHash>
    // TODO: Add some employees, try adding duplicates
    // TODO: Print the set size

    std::cout << "\n";
}

// ── Exercise 4: Balanced parentheses (stack) ─────────────────────────────
void ex4_balanced_parens()
{
    std::cout << "Exercise 4: Balanced parentheses\n";

    std::vector<std::string> tests{"(())", "(()", "()[]{}", "([)]", "{[]}"};

    // TODO: For each string, use a stack to check if brackets are balanced
    // TODO: Print each string and whether it's balanced

    std::cout << "\n";
}

// ── Exercise 5: BFS with queue ───────────────────────────────────────────
void ex5_bfs()
{
    std::cout << "Exercise 5: BFS shortest path\n";

    // Simple graph as adjacency list
    std::unordered_map<int, std::vector<int>> graph{
        {0, {1, 2}}, {1, {0, 3}}, {2, {0, 3, 4}}, {3, {1, 2, 5}}, {4, {2}}, {5, {3}}};

    // TODO: Use std::queue to perform BFS from node 0 to node 5
    // TODO: Track visited nodes with unordered_set
    // TODO: Print the shortest path length

    std::cout << "\n";
}

// ── Exercise 6: Top-K elements (priority_queue) ──────────────────────────
void ex6_top_k()
{
    std::cout << "Exercise 6: Top-K elements\n";
    std::vector<int> data{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    int              k = 3;

    // TODO: Find the top-k largest elements using a min-heap (priority_queue
    //        with std::greater<>). Keep the heap size at most k.
    // TODO: Print the top-k elements

    std::cout << "\n";
}

// ── Exercise 7: span utility function ────────────────────────────────────
// TODO: Write a function that takes span<const int> and returns the mean
//       double compute_mean(std::span<const int> data);

void ex7_span()
{
    std::cout << "Exercise 7: span utility\n";

    std::vector<int> v{10, 20, 30, 40, 50};
    int              arr[] = {1, 2, 3, 4, 5};

    // TODO: Call compute_mean with vector, array, and a subspan
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — Simple expression evaluator ───────────────
void ex8_expression_eval()
{
    std::cout << "Exercise 8: Expression evaluator\n";

    // TODO: Evaluate a simple infix expression like "3 + 5 * 2 - 8 / 4"
    //        Use two stacks: one for operators, one for operands
    //        Handle operator precedence (* / before + -)
    // Hint: Shunting-yard algorithm (simplified)

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 03 — Exercises: Unordered & Adaptors     ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_anagram_groups();
    ex2_two_sum();
    ex3_custom_hash();
    ex4_balanced_parens();
    ex5_bfs();
    ex6_top_k();
    ex7_span();
    ex8_expression_eval();

    std::cout << "All exercises complete.\n";
    return 0;
}
