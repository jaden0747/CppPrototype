# Capstone Projects — C++ Mastery Series

> Complete these projects after finishing the lectures to solidify your
> understanding. Each project integrates multiple lectures' worth of concepts.

---

## Project 1: "Smart Container Library" (Lectures 01–05)

**Difficulty:** ★★★☆☆

Build a type-safe container library with:
- `SmartArray<T, N>` — fixed-size array with bounds checking, iterators
- `RingBuffer<T, N>` — circular buffer with move semantics
- `TypeErasedContainer` — holds any container via type erasure

**Requirements:**
- Use `auto`, range-for, `nullptr`, brace-init (Lecture 01)
- Lambdas for custom comparators and predicates (Lecture 02)
- Move semantics, Rule of Five for all containers (Lecture 03)
- `constexpr` size computations, `static_assert` for constraints (Lecture 04)
- Generic lambdas, `make_unique` (Lecture 05)

---

## Project 2: "JSON-Like Config System" (Lectures 06–08)

**Difficulty:** ★★★☆☆

Build a configuration management system:
- Parse "key = value" config files
- Values stored as `variant<int, double, bool, string>`
- Type-safe access with `get<T>(key)` returning `optional<T>`
- File watching using `<filesystem>`

**Requirements:**
- Structured bindings for iteration (Lecture 06)
- `optional`, `variant`, `string_view` (Lecture 07)
- `<filesystem>` for file operations, `if constexpr` for type dispatch (Lecture 08)

---

## Project 3: "Concept-Constrained ECS" (Lectures 09–10)

**Difficulty:** ★★★★☆

Build an Entity-Component-System framework:
- Define concepts: `Component`, `System`, `Queryable`
- Components stored in `SparseSet<T>` containers
- Systems query entities using range views + filter
- Compile-time validation of system requirements

**Requirements:**
- Custom concepts for Component/System interfaces (Lecture 09)
- Range views for entity queries and iteration (Lecture 10)
- Projections for sorting entities by component values

---

## Project 4: "Async Task Scheduler" (Lectures 11–12)

**Difficulty:** ★★★★★

Build a coroutine-based task scheduler:
- `Task<T>` — awaitable result type
- `Generator<T>` — lazy sequence type
- Thread pool with `jthread` + stop tokens
- Formatted logging with `source_location` + `format`
- Task priority using `<=>` on priority levels

**Requirements:**
- Coroutines: co_await, co_yield, co_return (Lecture 11)
- `jthread`, stop tokens for thread management (Lecture 12)
- `std::format` for log output (Lecture 12)
- `<=>` for priority comparison (Lecture 12)

---

## Project 5: "Modern C++23 Web Framework" (Lecture 13 + All)

**Difficulty:** ★★★★★

Build a minimal HTTP request handler:
- Route matching with `expected<Response, Error>` (no exceptions)
- Request parsing with `string_view` + ranges
- Middleware pipeline using deducing `this` for chaining
- Response formatting with `std::print`
- Configuration with `variant` + `optional`
- Collect results with `ranges::to`

**Requirements:**
- All C++23 features from Lecture 13
- Integration of concepts from Lectures 01–12

---

## Evaluation Criteria

For each project:

| Criterion | Weight |
|-----------|--------|
| Correctness | 30% |
| Modern C++ idioms used | 25% |
| Clean API design | 20% |
| Error handling | 15% |
| Performance considerations | 10% |

---

## Suggested Timeline

| Week | Activity |
|------|----------|
| 1–2 | Lectures 01–05 + Project 1 |
| 3–4 | Lectures 06–08 + Project 2 |
| 5–6 | Lectures 09–10 + Project 3 |
| 7–8 | Lectures 11–12 + Project 4 |
| 9–10 | Lecture 13 + Project 5 |
| 11–12 | Code review, optimization, presentation |

---

## Tips for Success

1. **Write tests first** — every component should have unit tests
2. **Use sanitizers** — compile with `-fsanitize=address,undefined`
3. **Read compiler errors carefully** — concept errors are much clearer than SFINAE
4. **Profile before optimizing** — measure, don't guess
5. **Review open-source C++20/23 code** — study how experts use these features
6. **Join the community** — r/cpp, cpplang Slack, C++ Conferences on YouTube
