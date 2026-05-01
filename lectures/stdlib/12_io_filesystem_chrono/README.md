# Stdlib 12 — I/O, Filesystem & Chrono

> **Goal:** Master I/O streams for reading/writing, the filesystem library
> for portable path manipulation and file operations, and the chrono library
> for measuring time, handling durations, and working with calendar dates.

---

## Table of Contents

1. [I/O Streams Overview](#1-io-streams-overview)
2. [File I/O: `ifstream`, `ofstream`, `fstream`](#2-file-io)
3. [`stringstream` — String-Backed Streams](#3-stringstream)
4. [Stream Manipulators](#4-stream-manipulators)
5. [Filesystem: `path` and Path Operations](#5-filesystem-path)
6. [Filesystem: Querying and Iterating](#6-filesystem-querying)
7. [Filesystem: Modifying Operations](#7-filesystem-modifying)
8. [Chrono: Clocks and Time Points](#8-chrono-clocks)
9. [Chrono: Durations](#9-chrono-durations)
10. [Chrono: Calendar and Time Zones (C++20)](#10-chrono-calendar)
11. [Exercises](#11-exercises)

---

## 1. I/O Streams Overview

### The stream model

C++ I/O is based on the **stream abstraction**: data flows as a sequence
of characters, either **in** (from a source) or **out** (to a sink).

```
stdin  ──→ cin  ──→  your program  ──→ cout ──→ stdout
                                   ──→ cerr ──→ stderr
file   ──→ ifstream                ──→ ofstream ──→ file
string ──→ istringstream           ──→ ostringstream ──→ string
```

### The standard streams

```cpp
#include <iostream>

std::cout << "normal output\n";    // buffered stdout
std::cerr << "error output\n";     // unbuffered stderr (errors)
std::clog << "log output\n";       // buffered stderr (logging)

int x;
std::cin >> x;                     // read from stdin
```

### Reading a whole line

```cpp
std::string line;
std::getline(std::cin, line);      // reads until newline

// Common pitfall: mixing >> and getline
int n;
std::cin >> n;                     // reads number, leaves '\n' in buffer
std::cin.ignore();                 // ← MUST consume the leftover '\n'
std::getline(std::cin, line);      // now reads correctly
```

---

## 2. File I/O

### Reading from a file

```cpp
#include <fstream>

std::ifstream file("data.txt");
if (!file.is_open()) {
    std::cerr << "Failed to open file\n";
    return;
}

// Read word by word:
std::string word;
while (file >> word) {
    std::cout << word << "\n";
}

// Read line by line:
std::string line;
while (std::getline(file, line)) {
    std::cout << line << "\n";
}

// File is automatically closed when `file` goes out of scope (RAII)
```

### Writing to a file

```cpp
std::ofstream out("output.txt");
out << "Hello, file!\n";
out << 42 << "\n";
out << std::fixed << std::setprecision(2) << 3.14159 << "\n";
// File closed automatically
```

### Opening modes

```cpp
std::ofstream out("log.txt", std::ios::app);    // append mode
std::fstream f("data.bin", std::ios::binary | std::ios::in | std::ios::out);
```

| Mode | Meaning |
|------|---------|
| `ios::in` | Open for reading |
| `ios::out` | Open for writing (truncates by default) |
| `ios::app` | Append to end |
| `ios::binary` | Binary mode (no text translation) |
| `ios::trunc` | Truncate file on open |
| `ios::ate` | Seek to end after opening |

### Binary file I/O

```cpp
// Write binary data
std::ofstream out("data.bin", std::ios::binary);
int values[] = {1, 2, 3, 4, 5};
out.write(reinterpret_cast<char*>(values), sizeof(values));

// Read binary data
std::ifstream in("data.bin", std::ios::binary);
int buf[5];
in.read(reinterpret_cast<char*>(buf), sizeof(buf));
```

---

## 3. `stringstream` — String-Backed Streams

### What is it?

A stream that reads from / writes to a `std::string` instead of a file
or the console. Useful for parsing and building strings.

### Building a string (like a StringBuilder)

```cpp
#include <sstream>

std::ostringstream oss;
oss << "Name: " << name << ", Age: " << age << ", Score: " << score;
std::string result = oss.str();
```

### Parsing a string

```cpp
std::istringstream iss("42 3.14 hello");
int i;
double d;
std::string s;
iss >> i >> d >> s;
// i = 42, d = 3.14, s = "hello"
```

### Splitting a CSV line

```cpp
std::string line = "Alice,25,90.5";
std::istringstream iss(line);
std::string token;
while (std::getline(iss, token, ',')) {
    std::cout << token << "\n";
}
// Prints: Alice, 25, 90.5
```

**Note:** In modern C++20+ code, prefer `std::format` over `ostringstream`
for building formatted strings.

---

## 4. Stream Manipulators

```cpp
#include <iomanip>

// Width and fill
std::cout << std::setw(10) << std::setfill('0') << 42;  // "0000000042"

// Floating point precision
std::cout << std::fixed << std::setprecision(2) << 3.14159; // "3.14"
std::cout << std::scientific << 12345.0;                     // "1.23e+04"

// Integer bases
std::cout << std::hex << 255;    // "ff"
std::cout << std::oct << 8;     // "10"
std::cout << std::dec << 42;    // "42" (back to decimal)

// Boolean as text
std::cout << std::boolalpha << true;   // "true" (not "1")
```

---

## 5. Filesystem: `path` and Path Operations

### What is `std::filesystem::path`?

A **portable representation** of a file path. Handles OS differences
(`/` on Unix, `\` on Windows) automatically.

```cpp
#include <filesystem>
namespace fs = std::filesystem;

fs::path p = "/home/user/docs/report.pdf";

p.filename();     // "report.pdf"
p.stem();         // "report"
p.extension();    // ".pdf"
p.parent_path();  // "/home/user/docs"
p.root_path();    // "/"
p.is_absolute();  // true
p.is_relative();  // false
```

### Path manipulation

```cpp
fs::path dir = "/home/user";
fs::path file = dir / "docs" / "report.pdf";  // operator/ joins paths
// file = "/home/user/docs/report.pdf"

file.replace_extension(".txt");  // "/home/user/docs/report.txt"
file.replace_filename("notes.md"); // "/home/user/docs/notes.md"
```

### Why use `fs::path` instead of `std::string`?

- **Portable:** `/` works everywhere (auto-converted to `\` on Windows)
- **Rich API:** `stem()`, `extension()`, `parent_path()` etc.
- **Type safety:** Functions taking `path` vs `string` are clearer
- **Iteration:** Can iterate over path components

```cpp
for (const auto& component : fs::path("/home/user/docs/file.txt"))
    std::cout << component << "\n";
// Prints: "/" "home" "user" "docs" "file.txt"
```

---

## 6. Filesystem: Querying

```cpp
namespace fs = std::filesystem;

fs::exists("file.txt");            // does it exist?
fs::is_regular_file("file.txt");   // is it a regular file?
fs::is_directory("mydir");         // is it a directory?
fs::file_size("file.txt");         // size in bytes
fs::last_write_time("file.txt");   // last modification time
```

### Listing a directory

```cpp
// Non-recursive:
for (const auto& entry : fs::directory_iterator("/home/user/docs")) {
    std::cout << entry.path() << "\n";
}

// Recursive (all subdirectories):
for (const auto& entry : fs::recursive_directory_iterator("/home/user")) {
    if (entry.is_regular_file() && entry.path().extension() == ".cpp")
        std::cout << entry.path() << "\n";
}
```

### Space information

```cpp
auto info = fs::space("/home");
info.capacity;   // total disk space
info.free;       // free space
info.available;  // available to non-privileged users
```

---

## 7. Filesystem: Modifying Operations

```cpp
fs::create_directory("new_dir");              // create one directory
fs::create_directories("a/b/c/d");            // create nested directories
fs::copy("src.txt", "dst.txt");               // copy file
fs::copy("dir1", "dir2", fs::copy_options::recursive); // copy directory tree
fs::rename("old.txt", "new.txt");             // rename/move
fs::remove("file.txt");                       // delete file
fs::remove_all("dir_to_delete");              // delete directory tree
```

### Error handling

```cpp
// Two styles:
// 1. Throws filesystem_error on failure:
fs::copy("src", "dst");

// 2. Returns error code (no exception):
std::error_code ec;
fs::copy("src", "dst", ec);
if (ec) std::cerr << ec.message() << "\n";
```

---

## 8. Chrono: Clocks and Time Points

### The three main clocks

```cpp
#include <chrono>
namespace ch = std::chrono;

// system_clock: wall clock time (can go backwards if adjusted!)
auto now = ch::system_clock::now();

// steady_clock: monotonic (NEVER goes backwards) — for benchmarking
auto start = ch::steady_clock::now();
do_work();
auto end = ch::steady_clock::now();

// high_resolution_clock: alias for the highest resolution clock available
```

### Measuring elapsed time

```cpp
auto start = ch::steady_clock::now();
do_expensive_work();
auto end = ch::steady_clock::now();

auto elapsed = ch::duration_cast<ch::milliseconds>(end - start);
std::cout << "Took " << elapsed.count() << " ms\n";

// Or with C++20 floating point duration:
ch::duration<double> secs = end - start;
std::cout << "Took " << secs.count() << " seconds\n";
```

---

## 9. Chrono: Durations

### What is a duration?

A `duration` represents a time interval with a specific unit.
It's **type-safe** — you can't accidentally mix milliseconds and seconds.

```cpp
ch::seconds s(5);           // 5 seconds
ch::milliseconds ms(500);   // 500 milliseconds
ch::microseconds us(1000);  // 1000 microseconds
ch::minutes m(2);           // 2 minutes
ch::hours h(1);             // 1 hour
```

### Duration literals (C++14)

```cpp
using namespace std::chrono_literals;

auto d = 5s;     // 5 seconds
auto d2 = 500ms; // 500 milliseconds
auto d3 = 2h;    // 2 hours
auto d4 = 30min; // 30 minutes
```

### Duration arithmetic

```cpp
auto total = 2h + 30min + 15s;  // 2 hours, 30 minutes, 15 seconds
auto ms = ch::duration_cast<ch::milliseconds>(total);
std::cout << ms.count() << " ms\n";  // 9015000 ms
```

### Sleeping

```cpp
#include <thread>
std::this_thread::sleep_for(500ms);
std::this_thread::sleep_until(ch::steady_clock::now() + 2s);
```

---

## 10. Chrono: Calendar and Time Zones (C++20)

### Calendar dates

```cpp
using namespace std::chrono;

year_month_day date = 2024y/January/15;  // Jan 15, 2024
date.year();   // 2024y
date.month();  // January
date.day();    // 15d

// Check validity
year_month_day bad = 2024y/February/30;
bad.ok();  // false — Feb 30 doesn't exist!
```

### Time zone conversions

```cpp
auto utc = system_clock::now();
auto local = zoned_time{"America/New_York", utc};
auto tokyo = zoned_time{"Asia/Tokyo", utc};
```

---

## 11. Exercises

See `exercises.cpp`.

---

**Next lecture:** Concurrency.
