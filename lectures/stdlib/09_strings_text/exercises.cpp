// ============================================================================
// Stdlib 09 — Exercises: Strings & Text
// ============================================================================
#include <charconv>
#include <format>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// ── Exercise 1: String view splitter ─────────────────────────────────────
void ex1_sv_split()
{
    std::cout << "Exercise 1: string_view splitter\n";

    // TODO: Write a function split(string_view, char delimiter) -> vector<string_view>
    //        that splits without any allocations (zero-copy)
    // TODO: Test with "one:two:three:four" split on ':'
    // TODO: Print each field

    std::cout << "\n";
}

// ── Exercise 2: CSV parser with string_view ──────────────────────────────
void ex2_csv_parser()
{
    std::cout << "Exercise 2: CSV parser\n";
    std::string csv = R"(Name,Age,City
Alice,30,New York
Bob,25,London
Charlie,35,Tokyo)";

    // TODO: Parse CSV into vector<vector<string>>
    // TODO: Use string_view for zero-copy field access
    // TODO: Print as a formatted table using std::format

    std::cout << "\n";
}

// ── Exercise 3: from_chars benchmark ─────────────────────────────────────
void ex3_charconv()
{
    std::cout << "Exercise 3: charconv parsing\n";

    std::vector<std::string> numbers{"123", "456", "789", "1024", "65535"};

    // TODO: Parse each string to int using from_chars
    // TODO: Also parse with stoi for comparison
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 4: Format a report ──────────────────────────────────────────
void ex4_format_report()
{
    std::cout << "Exercise 4: Formatted report\n";

    struct Product
    {
        std::string name;
        double      price;
        int         quantity;
    };

    std::vector<Product> products{
        {"Laptop", 999.99, 5},
        {"Mouse", 29.99, 50},
        {"Keyboard", 79.99, 30},
        {"Monitor", 499.99, 10},
        {"USB Cable", 9.99, 200}};

    // TODO: Print a formatted table with headers using std::format
    //        Columns: Name (left, 15), Price (right, 10, 2 decimal), Qty (right, 5)
    // TODO: Print a total row at the bottom

    std::cout << "\n";
}

// ── Exercise 5: Regex email validator ────────────────────────────────────
void ex5_email_validator()
{
    std::cout << "Exercise 5: Email validator\n";

    std::vector<std::string> emails{
        "user@example.com", "bad@", "test.user@domain.org", "@missing.com", "user@.com", "good+tag@mail.co.uk"};

    // TODO: Write a regex to validate email addresses
    // TODO: Print each email and whether it's valid

    std::cout << "\n";
}

// ── Exercise 6: Regex find & replace ─────────────────────────────────────
void ex6_regex_replace()
{
    std::cout << "Exercise 6: Regex find & replace\n";
    std::string text = "Today is 2024-01-15 and tomorrow is 2024-01-16";

    // TODO: Use regex to find all dates in YYYY-MM-DD format
    // TODO: Replace them with DD/MM/YYYY format using backreferences
    // TODO: Print original and modified text

    std::cout << "\n";
}

// ── Exercise 7: String builder ───────────────────────────────────────────
void ex7_string_builder()
{
    std::cout << "Exercise 7: Efficient string building\n";

    // TODO: Build a large string by concatenating 1000 elements
    // Method 1: string += in a loop
    // Method 2: ostringstream
    // Method 3: reserve + append
    // TODO: Print the size of each result (should be equal)

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — text search tool ──────────────────────────
void ex8_text_search()
{
    std::cout << "Exercise 8: Text search tool\n";

    std::string text = R"(The quick brown fox jumps over the lazy dog.
The fox is clever and fast.
Dogs are loyal and friendly.
The brown dog chased the fox.)";

    // TODO: Implement a mini grep: given a search term, find all lines containing it
    // TODO: Use string_view to avoid copying lines
    // TODO: Print matching lines with line numbers
    // TODO: Highlight the match (e.g., surround with >> <<)
    // Test with search term "fox"

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 09 — Exercises: Strings & Text           ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_sv_split();
    ex2_csv_parser();
    ex3_charconv();
    ex4_format_report();
    ex5_email_validator();
    ex6_regex_replace();
    ex7_string_builder();
    ex8_text_search();

    std::cout << "All exercises complete.\n";
    return 0;
}
