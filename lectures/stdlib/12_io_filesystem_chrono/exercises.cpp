// ============================================================================
// Stdlib 12 — Exercises: I/O, Filesystem & Chrono
// ============================================================================
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// ── Exercise 1: Binary file I/O ──────────────────────────────────────────
void ex1_binary_io()
{
    std::cout << "Exercise 1: Binary I/O\n";

    // TODO: Write a vector<double> to a binary file using ofstream::write
    // TODO: Read it back using ifstream::read
    // TODO: Verify the data matches

    std::cout << "\n";
}

// ── Exercise 2: Stream manipulator table ─────────────────────────────────
void ex2_formatted_table()
{
    std::cout << "Exercise 2: Formatted table\n";

    struct Student
    {
        std::string name;
        int         age;
        double      gpa;
    };

    std::vector<Student> students{{"Alice", 20, 3.92}, {"Bob", 22, 3.45}, {"Charlie", 21, 3.78}, {"Diana", 20, 3.99}};

    // TODO: Print a formatted table using setw, left, right, fixed, setprecision
    //   Name (left, 15) | Age (right, 5) | GPA (right, 8, 2 decimals)

    std::cout << "\n";
}

// ── Exercise 3: Config file parser ───────────────────────────────────────
void ex3_config_parser()
{
    std::cout << "Exercise 3: Config file parser\n";

    // TODO: Write a simple config file with key=value pairs to /tmp/stdlib12_config.txt
    // TODO: Parse it back into a map<string, string>
    // TODO: Print all key-value pairs

    std::cout << "\n";
}

// ── Exercise 4: Directory tree printer ───────────────────────────────────
void ex4_dir_tree()
{
    std::cout << "Exercise 4: Directory tree\n";

    // TODO: Write a function that prints a directory tree (like `tree` command)
    //        using recursive_directory_iterator
    // TODO: Show indentation based on depth
    // TODO: Test on the current directory (limit depth to 2)

    std::cout << "\n";
}

// ── Exercise 5: File extension counter ───────────────────────────────────
void ex5_extension_counter()
{
    std::cout << "Exercise 5: Extension counter\n";

    // TODO: Scan a directory (e.g., current dir) recursively
    // TODO: Count files by extension using map<string, int>
    // TODO: Print extension counts sorted by count (descending)

    std::cout << "\n";
}

// ── Exercise 6: Stopwatch class ──────────────────────────────────────────
void ex6_stopwatch()
{
    std::cout << "Exercise 6: Stopwatch\n";

    // TODO: Implement a Stopwatch class with:
    //   - start(), stop(), reset()
    //   - elapsed() returning duration in desired unit
    //   - lap() returning split times
    // TODO: Use it to benchmark sorting 1M integers

    std::cout << "\n";
}

// ── Exercise 7: Duration converter ───────────────────────────────────────
void ex7_duration_convert()
{
    std::cout << "Exercise 7: Duration converter\n";

    // TODO: Write a function format_duration(seconds) that converts
    //        a number of seconds to "Xh Ym Zs" format using chrono
    // TODO: Test with: 3661, 7200, 90, 86400

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — file backup tool ──────────────────────────
void ex8_backup_tool()
{
    std::cout << "Exercise 8: File backup tool\n";

    // TODO: Implement a backup function that:
    //   1. Takes a source directory and backup directory
    //   2. Copies all files that are newer than their backup copy
    //   3. Uses last_write_time to compare
    //   4. Creates directories as needed
    //   5. Prints a summary of files copied/skipped
    // Test with /tmp/stdlib12_src and /tmp/stdlib12_backup

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 12 — Exercises: I/O, FS & Chrono         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_binary_io();
    ex2_formatted_table();
    ex3_config_parser();
    ex4_dir_tree();
    ex5_extension_counter();
    ex6_stopwatch();
    ex7_duration_convert();
    ex8_backup_tool();

    std::cout << "All exercises complete.\n";
    return 0;
}
