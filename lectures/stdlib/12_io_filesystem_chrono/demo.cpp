// ============================================================================
// Stdlib 12 — Demo: I/O, Filesystem & Chrono
// ============================================================================
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// ──────────────────────────────────────────────────────────────────────────
// 1. Stream manipulators
// ──────────────────────────────────────────────────────────────────────────
void demo_stream_format()
{
    std::cout << "=== 1. Stream Manipulators ===\n";

    // Width & alignment
    std::cout << "  " << std::setw(10) << std::left << "Name" << std::setw(8) << std::right << "Score" << "\n";
    std::cout << "  " << std::setw(10) << std::left << "Alice" << std::setw(8) << std::right << 95 << "\n";
    std::cout << "  " << std::setw(10) << std::left << "Bob" << std::setw(8) << std::right << 87 << "\n";

    // Number formatting
    double pi = 3.14159265358979;
    std::cout << "  fixed(3): " << std::fixed << std::setprecision(3) << pi << "\n";
    std::cout << "  scientific: " << std::scientific << pi << "\n";
    std::cout << std::defaultfloat; // reset

    // Integer bases
    int n = 255;
    std::cout << "  dec: " << std::dec << n << "  hex: " << std::hex << n << "  oct: " << std::oct << n << "\n";
    std::cout << std::dec; // reset

    // Bool
    std::cout << "  boolalpha: " << std::boolalpha << true << " " << false << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. stringstream
// ──────────────────────────────────────────────────────────────────────────
void demo_stringstream()
{
    std::cout << "=== 2. stringstream ===\n";

    // Build a string
    std::ostringstream oss;
    oss << "Value: " << 42 << ", Pi: " << std::fixed << std::setprecision(2) << 3.14159;
    std::cout << "  built: " << oss.str() << "\n";

    // Parse from string
    std::istringstream iss("10 20 30 40 50");
    int                val;
    std::cout << "  parsed: ";
    while (iss >> val)
        std::cout << val << " ";
    std::cout << "\n";

    // getline with delimiter
    std::istringstream csv("alpha,beta,gamma");
    std::string        field;
    std::cout << "  csv fields: ";
    while (std::getline(csv, field, ','))
        std::cout << "[" << field << "] ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. File I/O
// ──────────────────────────────────────────────────────────────────────────
void demo_file_io()
{
    std::cout << "=== 3. File I/O ===\n";

    const std::string filename = "/tmp/stdlib12_demo.txt";

    // Write
    {
        std::ofstream out(filename);
        out << "Line 1: Hello, file!\n";
        out << "Line 2: C++ I/O is easy\n";
        out << "Line 3: " << 42 << " " << 3.14 << "\n";
        std::cout << "  wrote to " << filename << "\n";
    }

    // Read line by line
    {
        std::ifstream in(filename);
        std::string   line;
        int           linenum = 0;
        while (std::getline(in, line))
        {
            std::cout << "  [" << ++linenum << "] " << line << "\n";
        }
    }

    // Append
    {
        std::ofstream out(filename, std::ios::app);
        out << "Line 4: Appended!\n";
    }

    // Read entire file into string
    {
        std::ifstream in(filename);
        std::string   content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::cout << "  total chars: " << content.size() << "\n";
    }

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Filesystem (C++17)
// ──────────────────────────────────────────────────────────────────────────
void demo_filesystem()
{
    std::cout << "=== 4. Filesystem (C++17) ===\n";

    // Path operations
    fs::path p = "/usr/local/bin/app";
    std::cout << "  path: " << p << "\n";
    std::cout << "  parent: " << p.parent_path() << "\n";
    std::cout << "  filename: " << p.filename() << "\n";
    std::cout << "  stem: " << p.stem() << "\n";
    std::cout << "  extension: " << p.extension() << "\n";

    // Path composition
    fs::path dir  = "/tmp";
    fs::path file = dir / "subdir" / "file.txt";
    std::cout << "  composed: " << file << "\n";

    // Current directory
    std::cout << "  current_path: " << fs::current_path() << "\n";

    // File status
    fs::path demo_file = "/tmp/stdlib12_demo.txt";
    if (fs::exists(demo_file))
    {
        std::cout << "  " << demo_file << " exists\n";
        std::cout << "  is_regular_file: " << std::boolalpha << fs::is_regular_file(demo_file) << "\n";
        std::cout << "  file_size: " << fs::file_size(demo_file) << " bytes\n";
    }

    // Directory iteration
    std::cout << "  /tmp entries (first 5):\n";
    int count = 0;
    for (const auto& entry : fs::directory_iterator("/tmp"))
    {
        if (++count > 5)
            break;
        std::cout << "    " << (entry.is_directory() ? "[DIR] " : "      ") << entry.path().filename() << "\n";
    }

    // Create & remove temp directory
    fs::path tmpdir = "/tmp/stdlib12_test_dir";
    fs::create_directories(tmpdir / "sub1" / "sub2");
    std::cout << "  created: " << tmpdir << "\n";
    fs::remove_all(tmpdir);
    std::cout << "  removed: " << tmpdir << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Chrono — clocks & durations
// ──────────────────────────────────────────────────────────────────────────
void demo_chrono()
{
    std::cout << "=== 5. Chrono ===\n";

    // Duration literals
    auto ms    = 500ms;
    auto sec   = 2s;
    auto total = ms + sec;
    std::cout << "  500ms + 2s = " << std::chrono::duration_cast<std::chrono::milliseconds>(total).count() << "ms\n";

    // Steady clock — for benchmarking
    auto start = std::chrono::steady_clock::now();

    // Simulate work
    volatile double sum = 0;
    for (int i = 0; i < 1000000; ++i)
        sum += std::sqrt(static_cast<double>(i));

    auto end     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "  sqrt loop took " << elapsed.count() << " us\n";

    // Duration conversions
    auto dur  = 3661s; // 1 hour, 1 minute, 1 second
    auto hrs  = std::chrono::duration_cast<std::chrono::hours>(dur);
    auto mins = std::chrono::duration_cast<std::chrono::minutes>(dur % 1h);
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur % 1min);
    std::cout << "  3661s = " << hrs.count() << "h " << mins.count() << "m " << secs.count() << "s\n";

    // System clock — wall time
    auto now        = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::cout << "  current time: " << std::ctime(&time_t_now);

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Stopwatch utility
// ──────────────────────────────────────────────────────────────────────────
class Stopwatch
{
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_;

public:
    Stopwatch()
        : start_(Clock::now())
    {
    }

    void reset()
    {
        start_ = Clock::now();
    }

    double elapsed_ms() const
    {
        auto now = Clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }
};

void demo_stopwatch()
{
    std::cout << "=== 6. Stopwatch Utility ===\n";

    Stopwatch sw;

    // Benchmark vector fill
    std::vector<int> v(1000000);
    for (int i = 0; i < 1000000; ++i)
        v[i] = i;

    std::cout << "  vector fill: " << sw.elapsed_ms() << " ms\n";

    sw.reset();
    std::sort(v.begin(), v.end(), std::greater<>());
    std::cout << "  sort desc: " << sw.elapsed_ms() << " ms\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 12 — I/O, Filesystem & Chrono            ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_stream_format();
    demo_stringstream();
    demo_file_io();
    demo_filesystem();
    demo_chrono();
    demo_stopwatch();

    std::cout << "All demos complete.\n";
    return 0;
}
