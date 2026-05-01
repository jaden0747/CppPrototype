# C++ Modern Features Learning Plan (C++11 → C++23)

You're a excellent teacher who can break down complex topics into simple, digestible steps. Here's a comprehensive learning plan to master modern C++ features from C++11 to C++23. Each phase focuses on key features introduced in that standard, with practical projects to reinforce your understanding.

For each topics, you can refer to the official documentation on [cppreference.com](https://cppreference.com) for detailed explanations and examples.

To help learner understand the topics, you also need to create some exercises for each topic. The exercises should be simple and practical, allowing learners to apply the concepts they've learned in a hands-on way.

The concept should be explained in a simple and easy-to-understand way, using analogies and examples where appropriate. The exercises should be designed to reinforce the concepts and help learners build confidence in their understanding.

Hints should be given for each exercise, guiding learners through the problem-solving process without giving away the solution. The hints should encourage critical thinking and problem-solving skills.

Each topics should contains as much excercises as possible, covering different aspects of the topic to ensure a comprehensive understanding. Hints should be provided for each exercise, helping learners to overcome challenges and deepen their understanding of the topic.

A structured, step-by-step roadmap to master modern C++. Each phase builds on the previous one.

---

## How to Use This Plan

- ✅ Check off features as you learn them
- 🕐 Estimated time per phase is for focused daily practice (~1–2 hrs/day)
- 💡 Write real code for each feature — don't just read about it
- 🔁 Review previous phases briefly before starting a new one

---

## Phase 1 — C++11 Essentials (3–4 weeks)

> The biggest leap in C++ history. These features appear in virtually every modern codebase.

### Week 1 — Daily Vocabulary

These are things you'll use every single day. Learn them first.

| #   | Feature                | Why It Matters                 |
| --- | ---------------------- | ------------------------------ |
| 1   | `auto` type deduction  | Less boilerplate, cleaner code |
| 2   | Range-based `for` loop | Safer, cleaner iteration       |
| 3   | `nullptr`              | Type-safe null pointer         |
| 4   | Initializer lists `{}` | Uniform initialization syntax  |
| 5   | `enum class`           | Scoped, strongly-typed enums   |

**Practice project:** Write a program that reads a list of numbers, stores them in a container, and prints only the even ones — using all 5 features above.

---

### Week 2 — Functions & Lambdas

| #   | Feature                         | Why It Matters                               |
| --- | ------------------------------- | -------------------------------------------- |
| 6   | Lambda expressions              | Inline functions, callbacks, STL algorithms  |
| 7   | `default` & `deleted` functions | Control what the compiler generates          |
| 8   | Delegating constructors         | Reduce constructor duplication               |
| 9   | Type aliases (`using`)          | Cleaner than `typedef`, works with templates |

**Practice project:** Use `std::sort` with a lambda comparator. Create a class with deleted copy constructor.

---

### Week 3 — Memory & Ownership

| #   | Feature                                   | Why It Matters                   |
| --- | ----------------------------------------- | -------------------------------- |
| 10  | Move semantics & rvalue references (`&&`) | Core performance feature         |
| 11  | `std::unique_ptr`                         | Single-owner resource management |
| 12  | `std::shared_ptr`                         | Shared ownership                 |
| 13  | `std::weak_ptr`                           | Break circular references        |

**Practice project:** Refactor a class that uses raw `new`/`delete` to use smart pointers. Observe move semantics with a class that logs its constructor/destructor calls.

---

### Week 4 — Compile-Time & Templates

| #   | Feature                                         | Why It Matters                                 |
| --- | ----------------------------------------------- | ---------------------------------------------- |
| 14  | `constexpr`                                     | Compute values at compile time                 |
| 15  | `static_assert`                                 | Catch bugs at compile time                     |
| 16  | Variadic templates                              | Foundation of `std::tuple`, `std::make_unique` |
| 17  | Threading library (`std::thread`, `std::mutex`) | Portable concurrency                           |

**Practice project:** Write a `constexpr` function for Fibonacci. Write a variadic `print()` function that prints any number of arguments.

---

## Phase 2 — C++14 Refinements (1 week)

> C++14 is a polish release. These additions are small but frequently useful.

| #   | Feature                                     | Why It Matters                                |
| --- | ------------------------------------------- | --------------------------------------------- |
| 18  | Generic lambdas (`auto` params)             | Lambdas that work with any type               |
| 19  | `constexpr` relaxation (`if`, `for` inside) | More powerful compile-time logic              |
| 20  | Return type deduction for functions         | Less ceremony for simple functions            |
| 21  | Variable templates                          | Template-based constants (e.g., `pi<double>`) |
| 22  | `std::make_unique`                          | Missing from C++11, now complete              |
| 23  | Binary literals & digit separators          | `0b1010'1100`, `1'000'000`                    |

**Practice project:** Rewrite Phase 1 lambdas as generic lambdas. Create a `pi<T>` variable template.

---

## Phase 3 — C++17 Power Features (3–4 weeks)

> C++17 introduced features that are now standard vocabulary. High ROI.

### Week 1 — Ergonomics

| #   | Feature                                  | Why It Matters                            |
| --- | ---------------------------------------- | ----------------------------------------- |
| 24  | Structured bindings                      | Unpack pairs, tuples, structs cleanly     |
| 25  | Class template argument deduction (CTAD) | `std::pair p(1, 2.5)` — no angle brackets |
| 26  | Inline variables                         | Headers with variables — no ODR issues    |
| 27  | `if` / `switch` with initializers        | Scoped temporary in condition             |

---

### Week 2 — Vocabulary Types

| #   | Feature              | Why It Matters                    |
| --- | -------------------- | --------------------------------- |
| 28  | `std::optional<T>`   | Express "value or nothing" safely |
| 29  | `std::variant<T...>` | Type-safe union                   |
| 30  | `std::any`           | Type-erased value container       |
| 31  | `std::string_view`   | Zero-copy string handling         |

**Practice project:** Write a config parser that returns `std::optional<std::string>`. Model a shape as `std::variant<Circle, Rectangle, Triangle>` and use `std::visit`.

---

### Week 3 — Templates & Algorithms

| #   | Feature                                     | Why It Matters                       |
| --- | ------------------------------------------- | ------------------------------------ |
| 32  | Fold expressions                            | Compact variadic template expansions |
| 33  | `if constexpr`                              | Compile-time branching in templates  |
| 34  | Parallel algorithms (`std::execution::par`) | Easy parallelism for STL algorithms  |
| 35  | `std::filesystem`                           | Portable file/directory operations   |

**Practice project:** Write a function that uses `if constexpr` to handle integral vs floating-point types differently. Use `std::filesystem` to list files in a directory.

---

## Phase 4 — C++20 Revolution (6–8 weeks)

> C++20 is as big as C++11. Take your time — each pillar is a deep topic.

### Weeks 1–2 — Concepts

| #   | Feature                                                         | Why It Matters                           |
| --- | --------------------------------------------------------------- | ---------------------------------------- |
| 36  | Concepts (defining & using)                                     | Named, readable constraints on templates |
| 37  | Standard library concepts (`std::integral`, `std::range`, etc.) | Pre-built constraints                    |
| 38  | Abbreviated function templates (`auto` params)                  | Shorthand for constrained templates      |

**Practice project:** Write a generic `clamp<T>` function constrained to `std::totally_ordered`. Replace raw `typename T` in a prior project with concepts.

---

### Weeks 3–4 — Ranges

| #   | Feature                                                  | Why It Matters                          |
| --- | -------------------------------------------------------- | --------------------------------------- |
| 39  | Ranges & views (`std::views::filter`, `transform`, etc.) | Lazy, composable sequences              |
| 40  | Range adaptors pipeline (`                               | ` operator)                             | Readable data transformation chains |
| 41  | New range views: `zip`, `take`, `drop`, `reverse`        | Common operations without raw iterators |

**Practice project:** Replace a loop-heavy data transformation with a ranges pipeline. Parse a CSV and filter/map rows using `std::views`.

---

### Weeks 5–6 — Coroutines & Modules

| #   | Feature                                          | Why It Matters                        |
| --- | ------------------------------------------------ | ------------------------------------- |
| 42  | Coroutines (`co_yield`, `co_await`, `co_return`) | Generators, async I/O, lazy sequences |
| 43  | Modules (`import`, `export module`)              | Faster builds, encapsulation          |

> ⚠️ Coroutines require a supporting library (e.g., cppcoro or C++23's `std::generator`). Start with a simple generator before async coroutines.

---

### Week 7 — Ergonomics & Utilities

| #   | Feature                                            | Why It Matters                            |
| --- | -------------------------------------------------- | ----------------------------------------- |
| 44  | Three-way comparison (`<=>`)                       | Auto-generate all comparison operators    |
| 45  | `std::span<T>`                                     | Non-owning view over arrays               |
| 46  | `std::format`                                      | Python-style, type-safe string formatting |
| 47  | Designated initializers                            | Initialize struct fields by name          |
| 48  | `[[likely]]` / `[[unlikely]]`                      | Branch prediction hints                   |
| 49  | `constexpr` improvements (vector, string, virtual) | More power at compile time                |

---

## Phase 5 — C++23 Additions (1–2 weeks)

> C++23 refines and fills gaps. Many features are immediately practical.

| #   | Feature                                                 | Why It Matters                             |
| --- | ------------------------------------------------------- | ------------------------------------------ |
| 50  | `std::expected<T, E>`                                   | Error handling without exceptions          |
| 51  | `std::print` / `std::println`                           | Simpler output built on `std::format`      |
| 52  | Deducing `this`                                         | Explicit self parameter; recursive lambdas |
| 53  | `std::flat_map` / `std::flat_set`                       | Cache-friendly sorted containers           |
| 54  | `std::mdspan`                                           | Multidimensional array views               |
| 55  | Ranges improvements (`chunk`, `slide`, `stride`, `zip`) | More views out of the box                  |
| 56  | `if consteval`                                          | Branch on compile-time vs runtime context  |
| 57  | `std::stacktrace`                                       | Capture stack traces at runtime            |
| 58  | Multidimensional `operator[]`                           | `m[i, j]` for matrix types                 |

**Practice project:** Replace exception-based error handling in a prior project with `std::expected`. Write a recursive lambda using deducing `this`.

---

## Capstone Projects by Phase

| After Phase | Project Idea                                                                 |
| ----------- | ---------------------------------------------------------------------------- |
| 1 (C++11)   | A type-safe event system using smart pointers and lambdas                    |
| 2 (C++14)   | A compile-time unit system using variable templates                          |
| 3 (C++17)   | A mini expression parser returning `std::optional` or `std::variant`         |
| 4 (C++20)   | A lazy pipeline that reads a file, parses it with ranges, and formats output |
| 5 (C++23)   | A small CLI tool with `std::expected` error handling and `std::print` output |

---

## Recommended Resources

| Resource                                                                  | Best For                                  |
| ------------------------------------------------------------------------- | ----------------------------------------- |
| [cppreference.com](https://cppreference.com)                              | Authoritative reference for every feature |
| *A Tour of C++* — Bjarne Stroustrup                                       | Fast, dense overview by the creator       |
| *C++ Core Guidelines* — isocpp.github.io                                  | Best practices for all modern features    |
| [Compiler Explorer (godbolt.org)](https://godbolt.org)                    | See exactly what your code compiles to    |
| [C++ Weekly — Jason Turner (YouTube)](https://www.youtube.com/@cppweekly) | Short, practical video walkthroughs       |

---

## Quick Reference: Feature → Standard

| Standard | Key Features                                                                          |
| -------- | ------------------------------------------------------------------------------------- |
| C++11    | auto, lambdas, move semantics, smart pointers, threads, constexpr, variadic templates |
| C++14    | generic lambdas, make_unique, constexpr relaxation, return type deduction             |
| C++17    | structured bindings, optional, variant, string_view, if constexpr, filesystem         |
| C++20    | concepts, ranges, coroutines, modules, spaceship operator, std::format                |
| C++23    | expected, print/println, deducing this, flat_map, mdspan                              |

---

*Generated with Claude · Happy coding! 🚀*