# Stdlib 09 — Strings & Text

> **Goal:** Master `std::string`, `std::string_view`, number conversions,
> `std::format`, and text processing utilities. Understand when to use each
> and how they differ in ownership, performance, and safety.

---

## Table of Contents

1. [std::string — The Basics](#1-stdstring--the-basics)
2. [Small String Optimization (SSO)](#2-small-string-optimization-sso)
3. [std::string_view (C++17) — Non-Owning View](#3-stdstring_view-c17)
4. [String Operations Reference](#4-string-operations-reference)
5. [Number ↔ String Conversions](#5-number--string-conversions)
6. [std::format (C++20)](#6-stdformat-c20)
7. [std::charconv (C++17) — Fast Conversions](#7-stdcharconv-c17)
8. [std::regex — Regular Expressions](#8-stdregex)
9. [Common Pitfalls](#9-common-pitfalls)
10. [Exercises](#10-exercises)

---

## 1. `std::string` — The Basics

### What is it?

`std::string` is a dynamic, owning array of characters. It manages its own
memory (allocates on the heap, grows as needed, frees on destruction).

```cpp
#include <string>

std::string s1 = "hello";              // from C-string literal
std::string s2("world");               // constructor
std::string s3(5, 'x');                // "xxxxx" — 5 copies of 'x'
std::string s4 = s1 + " " + s2;       // concatenation: "hello world"
```

### Why not C strings (`char*`)?

| Feature | `char*` / `char[]` | `std::string` |
|---------|--------------------|---------------|
| Memory management | Manual (`malloc`/`free`) | Automatic (RAII) |
| Size tracking | Manual (`strlen` is O(n)) | `.size()` is O(1) |
| Concatenation | `strcat` (buffer overflow risk) | `+` operator (safe) |
| Comparison | `strcmp` (returns int) | `==`, `<` (natural) |
| Copy | `strcpy` (buffer overflow risk) | Assignment `=` (safe) |
| Null terminator | Must manage manually | Handled automatically |

### Key operations

```cpp
s.size();       // number of characters (same as s.length())
s.empty();      // true if size == 0
s.c_str();      // get null-terminated C string (for C APIs)
s.data();       // same as c_str() since C++17
s.substr(3, 5); // substring starting at index 3, length 5
s.find("llo");  // find substring, returns index or npos
s.rfind("l");   // find from end
s.starts_with("he"); // C++20
s.ends_with("lo");   // C++20
s.contains("ell");   // C++23
```

---

## 2. Small String Optimization (SSO)

### What is it?

Most string implementations store **short strings inside the object itself**
(on the stack), avoiding heap allocation entirely.

```
std::string short_str = "hi";   // Stored INSIDE the object (no heap alloc)
std::string long_str = "this is a long string that exceeds the buffer";
                                // Stored on the HEAP
```

### Why does it matter?

- Short strings (typically < 22 bytes on 64-bit) are **extremely fast**
  to create, copy, and destroy — no `new`/`delete` calls
- This is why `std::string` is often faster than you'd expect

### How to check

```cpp
// Implementation-defined, but typically:
// sizeof(std::string) is 32 bytes on most implementations
// SSO buffer is about 15-22 characters
std::cout << sizeof(std::string);  // usually 32
```

---

## 3. `std::string_view` (C++17)

### What is it?

A **non-owning, read-only view** into a string. It's a pointer + length — no
copying, no allocation. Think of it as a "window" into someone else's string.

### Why use it?

```cpp
// Problem: this copies the string every time!
void process(const std::string& s);
process("hello"); // constructs a temporary std::string from "hello"!

// Solution: string_view avoids the copy
void process(std::string_view s);  // just a pointer + length
process("hello");  // NO copy — views the string literal directly
```

### Basic usage

```cpp
#include <string_view>

std::string_view sv = "hello world";
sv.substr(0, 5);     // "hello" — returns another string_view (no copy!)
sv.remove_prefix(6); // sv is now "world" (no copy — just adjust pointer)
sv.find("world");    // 0
sv.size();           // 5
```

### The critical rule: `string_view` does NOT own the data!

```cpp
std::string_view dangerous() {
    std::string s = "hello";
    return s;  // 💥 DANGLING! s is destroyed, view points to garbage
}

// Safe uses:
void safe(std::string_view sv);        // OK: caller owns the string
std::string_view lit = "hello world"; // OK: string literals live forever
```

### When to use `string_view` vs `const string&`

| Parameter type | When to use |
|---------------|-------------|
| `std::string_view` | Read-only access, no need to store. Avoids copies from `char*`. |
| `const std::string&` | When you need to pass to APIs requiring `std::string` |
| `std::string` (by value) | When you'll modify or store a copy anyway |

---

## 4. String Operations Reference

### Searching

```cpp
std::string s = "hello world hello";

s.find("hello");           // 0 — first occurrence
s.rfind("hello");          // 12 — last occurrence
s.find_first_of("aeiou");  // 1 — first vowel ('e')
s.find_last_of("aeiou");   // 15 — last vowel ('o')
s.find_first_not_of("helo "); // 6 — first char not in set ('w')
```

All return `std::string::npos` (typically `size_t(-1)`) if not found.

### Modifying

```cpp
s.append(" again");          // add to end
s.insert(5, " beautiful");   // insert at position
s.erase(5, 10);             // erase 10 chars starting at position 5
s.replace(0, 5, "Hi");      // replace "hello" with "Hi"
s.clear();                   // empty the string
s.reserve(100);              // pre-allocate capacity (avoid reallocations)
```

### Comparing

```cpp
s1 == s2;                    // content equality
s1 < s2;                     // lexicographic comparison
s1.compare(s2);              // returns <0, 0, or >0 (like strcmp)
```

---

## 5. Number ↔ String Conversions

### Number to string: `std::to_string`

```cpp
std::string s1 = std::to_string(42);      // "42"
std::string s2 = std::to_string(3.14);    // "3.140000" (6 decimal places!)
```

**Limitation:** No control over formatting. Use `std::format` for precision.

### String to number: `stoi`, `stol`, `stod`

```cpp
int i = std::stoi("42");        // 42
long l = std::stol("123456789");
double d = std::stod("3.14");   // 3.14
```

**These throw `std::invalid_argument` if the string is not a number,
and `std::out_of_range` if the value doesn't fit.**

---

## 6. `std::format` (C++20)

### What is it?

Python-style string formatting for C++. Replaces `sprintf`, `stringstream`,
and `operator<<` chaining.

### Basic usage

```cpp
#include <format>

auto s = std::format("Hello, {}!", "world");        // "Hello, world!"
auto s2 = std::format("{} + {} = {}", 1, 2, 3);     // "1 + 2 = 3"
```

### Formatting options

```cpp
// Integers
std::format("{:d}", 42);       // "42" (decimal)
std::format("{:x}", 255);      // "ff" (hex)
std::format("{:o}", 8);        // "10" (octal)
std::format("{:b}", 10);       // "1010" (binary)
std::format("{:08x}", 255);    // "000000ff" (zero-padded, 8 chars)

// Floating point
std::format("{:.2f}", 3.14159); // "3.14" (2 decimal places)
std::format("{:e}", 12345.0);   // "1.234500e+04" (scientific)

// Width and alignment
std::format("{:>10}", "hi");   // "        hi" (right-align, width 10)
std::format("{:<10}", "hi");   // "hi        " (left-align)
std::format("{:^10}", "hi");   // "    hi    " (center)
std::format("{:*^10}", "hi");  // "****hi****" (fill with *)
```

### Why prefer `std::format`?

| Old way | New way |
|---------|---------|
| `sprintf(buf, "%d", 42)` — buffer overflow risk! | `std::format("{}", 42)` — safe |
| `std::stringstream ss; ss << std::setw(10) << x;` — verbose | `std::format("{:10}", x)` |
| `std::to_string(x) + " items"` — allocates twice | `std::format("{} items", x)` |

### `std::print` (C++23)

```cpp
std::print("Hello, {}!\n", "world"); // like format but writes to stdout
```

---

## 7. `std::charconv` (C++17) — Fast Conversions

### What is it?

The **fastest** way to convert between numbers and strings. No allocations,
no locale, no exceptions.

### `to_chars` — number to string (into a buffer)

```cpp
#include <charconv>

char buf[20];
auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), 42);
// ptr points past the last character written
// ec is std::errc{} on success
std::string_view result(buf, ptr - buf); // "42"
```

### `from_chars` — string to number (from a buffer)

```cpp
int value;
auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
if (ec == std::errc{}) {
    // success! value contains the parsed number
}
```

### Why use `charconv`?

| Method | Speed | Allocates? | Locale-dependent? |
|--------|-------|-----------|-------------------|
| `std::to_string` | Slow | Yes | Yes |
| `std::stoi` | Medium | Yes | Yes |
| `std::format` | Medium | Yes | Optional |
| `std::to_chars`/`from_chars` | **Fastest** | **No** | **No** |

Use `charconv` in performance-critical code (parsers, serializers).

---

## 8. `std::regex` — Regular Expressions

### Basic usage

```cpp
#include <regex>

std::string text = "Phone: 123-456-7890";
std::regex pattern(R"(\d{3}-\d{3}-\d{4})");

// Search
std::smatch match;
if (std::regex_search(text, match, pattern)) {
    std::cout << match[0] << "\n"; // "123-456-7890"
}

// Match entire string
bool is_phone = std::regex_match("123-456-7890", pattern); // true

// Replace
auto result = std::regex_replace(text, pattern, "***-***-****");
// "Phone: ***-***-****"
```

### Performance warning

`std::regex` is notoriously **slow** in most implementations. For hot paths,
consider alternatives:
- Simple `string::find` / `string_view` operations
- `std::from_chars` for number parsing
- Third-party libraries like CTRE (compile-time regex) or RE2

---

## 9. Common Pitfalls

### 1. `string_view` dangling reference

```cpp
std::string_view get_name() {
    std::string s = "temporary";
    return s;  // 💥 s destroyed, view is dangling!
}
```

### 2. Mixing signed/unsigned with `string::find`

```cpp
std::string s = "hello";
if (s.find("xyz") == -1) { ... }  // ❌ Wrong! npos is not -1
if (s.find("xyz") == std::string::npos) { ... } // ✅ Correct
```

### 3. `to_string` precision for doubles

```cpp
std::to_string(0.1);  // "0.100000" — always 6 decimal places
// Use std::format("{:.2f}", 0.1) for "0.10"
```

---

## 10. Exercises

See `exercises.cpp`.

---

**Next lecture:** Memory Management.
