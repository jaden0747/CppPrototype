// ============================================================================
// Lecture 08 — Exercises: C++17 Templates & Filesystem
// ============================================================================
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Universal serializer with if constexpr
// Write serialize<T>(T value) -> std::string that handles:
//   - integral types: "int:42"
//   - floating point: "float:3.14"
//   - std::string: "str:hello"
//   - bool: "bool:true" / "bool:false"
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> std::string serialize(T value) { ... }

void exercise_serialize()
{
    // assert(serialize(42) == "int:42");
    // assert(serialize(3.14) == "float:3.140000");
    // assert(serialize(true) == "bool:true");
    // assert(serialize(std::string("hi")) == "str:hi");
    std::cout << "  Exercise 1: serialize — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: template<auto> compile-time array
// Create a compile-time fixed array:
//   template<auto... Values> struct ConstArray { ... };
// with:
//   static constexpr size_t size = sizeof...(Values)
//   static constexpr auto get(size_t i) -> ???
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<auto... Values> struct ConstArray { ... };

void exercise_const_array()
{
    // static_assert(ConstArray<1,2,3,4,5>::size == 5);
    // static_assert(ConstArray<10,20,30>::values[0] == 10);
    // static_assert(ConstArray<10,20,30>::values[2] == 30);
    std::cout << "  Exercise 2: ConstArray — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Constrained variadic with conjunction
// Write a function `sum_integers(args...)` that only compiles if ALL
// arguments are integral types. Use std::conjunction.
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename... Args,
//          typename = std::enable_if_t<std::conjunction_v<std::is_integral<Args>...>>>
// auto sum_integers(Args... args) { ... }

void exercise_conjunction()
{
    // assert(sum_integers(1, 2, 3) == 6);
    // assert(sum_integers(1L, 2LL, 3) == 6);
    // The following should NOT compile:
    // sum_integers(1, 2.0, 3);  // contains double!
    std::cout << "  Exercise 3: conjunction — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Filesystem — directory statistics
// Write a function that takes a path and returns a tuple:
//   {total_files, total_dirs, total_bytes}
// Use recursive_directory_iterator.
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// std::tuple<int,int,uintmax_t> dir_stats(const fs::path& dir) { ... }

void exercise_filesystem()
{
    // Create test structure
    fs::path tmp = fs::temp_directory_path() / "lecture08_exercise";
    fs::create_directories(tmp / "sub1");
    fs::create_directories(tmp / "sub2");
    {
        std::ofstream(tmp / "a.txt") << "hello";
    }
    {
        std::ofstream(tmp / "sub1" / "b.txt") << "world!";
    }
    {
        std::ofstream(tmp / "sub2" / "c.txt") << "test data here";
    }

    // auto [files, dirs, bytes] = dir_stats(tmp);
    // assert(files == 3);
    // assert(dirs == 2);
    // assert(bytes > 0);

    fs::remove_all(tmp);
    std::cout << "  Exercise 4: filesystem stats — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: std::apply — tuple-based dispatch
// Given a vector of tuple<string, int, double>, use std::apply to print
// each tuple as a formatted string: "name: N (score: D)"
// ──────────────────────────────────────────────────────────────────────────
// TODO: std::string format_entry(const std::tuple<std::string, int, double>& t) { ... }

void exercise_apply()
{
    // using Entry = std::tuple<std::string, int, double>;
    // Entry e{"Alice", 1, 95.5};
    // auto s = format_entry(e);
    // assert(s.find("Alice") != std::string::npos);
    // assert(s.find("95.5") != std::string::npos || s.find("95.50") != std::string::npos);
    std::cout << "  Exercise 5: std::apply — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): File extension counter
// Build a program that:
//   1. Takes a directory path
//   2. Recursively finds all files
//   3. Groups by extension
//   4. Returns a map<string, pair<int, uintmax_t>> : ext -> {count, total_bytes}
// Use filesystem + if constexpr for handling symlinks vs regular files.
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement extension_stats(path) -> map<string, pair<int, uintmax_t>>

void exercise_ext_counter()
{
    // fs::path tmp = fs::temp_directory_path() / "lecture08_ext";
    // fs::create_directories(tmp);
    // { std::ofstream(tmp / "a.cpp") << "int main(){}"; }
    // { std::ofstream(tmp / "b.cpp") << "void foo(){}"; }
    // { std::ofstream(tmp / "c.h") << "#pragma once"; }
    // auto stats = extension_stats(tmp);
    // assert(stats[".cpp"].first == 2);
    // assert(stats[".h"].first == 1);
    // fs::remove_all(tmp);
    std::cout << "  Exercise 6: extension counter — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 08 — Exercises: Templates & Filesystem  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_serialize();
    exercise_const_array();
    exercise_conjunction();
    exercise_filesystem();
    exercise_apply();
    exercise_ext_counter();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
