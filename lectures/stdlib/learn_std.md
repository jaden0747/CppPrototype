# C++ Standard Library Learning Plan

A structured roadmap to master the C++ Standard Library — from everyday containers to concurrency, filesystems, and beyond.

> **Prerequisites:** Comfortable with modern C++ syntax (C++11+).

---

## How to Use This Plan

- Work through phases in order — later phases build on earlier ones
- 🕐 Time estimates assume ~1–2 focused hours/day
- For every header you learn, check [cppreference.com](https://cppreference.com) — it is the authoritative reference
- Write code for every topic; don't just read

---

## Phase 1 — Containers (3–4 weeks)

> The heart of the STL. You'll use these every day.

### Week 1 — Sequence Containers

These store elements in a linear order.

| Container | Header | When to Use |
|-----------|--------|-------------|
| `std::vector<T>` | `<vector>` | Default choice for dynamic arrays |
| `std::array<T, N>` | `<array>` | Fixed-size stack-allocated array |
| `std::deque<T>` | `<deque>` | Fast insert/erase at both ends |
| `std::list<T>` | `<list>` | Frequent mid-sequence insert/erase |
| `std::forward_list<T>` | `<forward_list>` | Memory-constrained singly-linked list |

**Key concepts to master:**
- Iterator invalidation rules (especially for `vector`)
- `reserve()` vs `resize()` on `vector`
- When `list` beats `vector` (hint: rarely)
- `emplace_back()` vs `push_back()`

**Practice:** Build a task queue using `deque`. Benchmark `vector` vs `list` for 10,000 insertions in the middle.

---

### Week 2 — Associative Containers

Ordered (tree-based) containers — O(log n) operations.

| Container | Header | When to Use |
|-----------|--------|-------------|
| `std::map<K, V>` | `<map>` | Key→value with sorted keys |
| `std::set<T>` | `<set>` | Sorted unique values |
| `std::multimap<K, V>` | `<map>` | Multiple values per key |
| `std::multiset<T>` | `<set>` | Sorted values with duplicates |

**Key concepts to master:**
- `find()` vs `count()` vs `contains()` (C++20)
- `lower_bound()` / `upper_bound()` / `equal_range()`
- Custom comparators
- Structured bindings with map iteration: `for (auto& [k, v] : m)`

**Practice:** Build a word frequency counter with `map`. Implement a leaderboard with `multimap`.

---

### Week 3 — Unordered Containers

Hash-based containers — O(1) average operations.

| Container | Header | When to Use |
|-----------|--------|-------------|
| `std::unordered_map<K, V>` | `<unordered_map>` | Fast key→value lookup |
| `std::unordered_set<T>` | `<unordered_set>` | Fast membership test |
| `std::unordered_multimap` | `<unordered_map>` | Multi-value fast lookup |
| `std::unordered_multiset` | `<unordered_set>` | Multi-value fast membership |

**Key concepts to master:**
- Hash collisions and load factor
- Writing a custom `std::hash<T>` specialization
- When to prefer `unordered_map` over `map` (and when not to)
- Rehashing and `reserve()`

**Practice:** Re-implement the word frequency counter from Week 2 using `unordered_map`. Measure the speedup.

---

### Week 4 — Container Adaptors & C++17/23 Additions

| Container | Header | Notes |
|-----------|--------|-------|
| `std::stack<T>` | `<stack>` | LIFO adaptor over deque/vector |
| `std::queue<T>` | `<queue>` | FIFO adaptor |
| `std::priority_queue<T>` | `<queue>` | Heap-based max/min queue |
| `std::span<T>` *(C++20)* | `<span>` | Non-owning view over contiguous data |
| `std::flat_map<K,V>` *(C++23)* | `<flat_map>` | Sorted vector-backed map; cache-friendly |
| `std::flat_set<T>` *(C++23)* | `<flat_set>` | Sorted vector-backed set |
| `std::mdspan<T,...>` *(C++23)* | `<mdspan>` | Multidimensional array view |

**Practice:** Implement Dijkstra's algorithm using `priority_queue`. Write a function that accepts `std::span<int>` instead of raw pointer + size.

---

## Phase 2 — Iterators & Ranges (2 weeks)

> Iterators are the glue between containers and algorithms. Ranges are their modern replacement.

### Week 1 — Iterators

| Concept | Header | Notes |
|---------|--------|-------|
| Iterator categories (input, forward, bidirectional, random access, contiguous) | `<iterator>` | Determines which algorithms work |
| `std::begin` / `std::end` | `<iterator>` | Generic range access |
| `std::advance`, `std::distance`, `std::next`, `std::prev` | `<iterator>` | Iterator arithmetic |
| `std::back_inserter`, `std::front_inserter`, `std::inserter` | `<iterator>` | Output iterator adaptors |
| `std::reverse_iterator` | `<iterator>` | Iterate backwards |
| `std::move_iterator` | `<iterator>` | Move elements instead of copying |
| `std::istream_iterator` / `std::ostream_iterator` | `<iterator>` | Stream-based iteration |

**Practice:** Read integers from `std::cin` into a `vector` using `istream_iterator`. Copy a vector in reverse using `reverse_iterator`.

---

### Week 2 — Ranges (C++20)

| Concept | Header | Notes |
|---------|--------|-------|
| `std::ranges::range` concept | `<ranges>` | What makes something a range |
| Views: `filter`, `transform`, `take`, `drop`, `reverse` | `<ranges>` | Lazy adaptors |
| Views: `zip`, `chunk`, `stride`, `slide` *(C++23)* | `<ranges>` | Newer combinators |
| `std::views::iota` | `<ranges>` | Integer range generator |
| `std::views::split` / `std::views::join` | `<ranges>` | String/range splitting |
| `std::ranges::to<Container>` *(C++23)* | `<ranges>` | Materialise a range into a container |
| Range algorithms (`std::ranges::sort`, `find`, `copy`, etc.) | `<algorithm>` | Constraint-checked, no iterator pairs |

**Practice:** Pipeline: read a file of words → filter length > 4 → transform to uppercase → collect into a `vector` — using only ranges.

---

## Phase 3 — Algorithms (2–3 weeks)

> The STL provides ~100 algorithms. Learn them by category.

### Week 1 — Non-modifying Algorithms (`<algorithm>`)

| Algorithm | Notes |
|-----------|-------|
| `find`, `find_if`, `find_if_not` | Linear search |
| `count`, `count_if` | Count matching elements |
| `all_of`, `any_of`, `none_of` | Range predicates |
| `for_each` | Apply function to each element |
| `search`, `search_n` | Subsequence search |
| `mismatch`, `equal` | Range comparison |
| `min_element`, `max_element`, `minmax_element` | Extremes |

---

### Week 2 — Modifying & Sorting Algorithms

| Algorithm | Notes |
|-----------|-------|
| `copy`, `copy_if`, `copy_n` | Range copying |
| `move`, `move_backward` | Range moving |
| `transform` | Apply transformation to range |
| `fill`, `fill_n`, `generate`, `generate_n` | Range filling |
| `remove`, `remove_if` + `erase` (erase-remove idiom) | Element removal |
| `replace`, `replace_if` | Element replacement |
| `sort`, `stable_sort`, `partial_sort` | Sorting |
| `nth_element` | Partial ordering |
| `unique`, `unique_copy` | Remove consecutive duplicates |
| `reverse`, `rotate` | Reordering |
| `shuffle` | Random reordering |

**Key concept:** The **erase-remove idiom** — `v.erase(std::remove_if(v.begin(), v.end(), pred), v.end())` — and its C++20 simplification: `std::erase_if(v, pred)`.

---

### Week 3 — Numeric, Heap & Set Algorithms

| Algorithm | Header | Notes |
|-----------|--------|-------|
| `accumulate` | `<numeric>` | Fold/reduce over a range |
| `reduce` *(C++17)* | `<numeric>` | Parallelisable accumulate |
| `transform_reduce` *(C++17)* | `<numeric>` | Map-reduce in one step |
| `partial_sum`, `inclusive_scan`, `exclusive_scan` | `<numeric>` | Prefix sums |
| `iota` | `<numeric>` | Fill with incrementing values |
| `inner_product` | `<numeric>` | Dot product |
| `binary_search`, `lower_bound`, `upper_bound` | `<algorithm>` | Sorted-range search |
| `merge`, `inplace_merge` | `<algorithm>` | Sorted range merging |
| `set_union`, `set_intersection`, `set_difference` | `<algorithm>` | Set operations on sorted ranges |
| `make_heap`, `push_heap`, `pop_heap`, `sort_heap` | `<algorithm>` | Manual heap management |
| `next_permutation`, `prev_permutation` | `<algorithm>` | Combinatorial enumeration |

**Practice:** Compute a dot product using `inner_product`. Use `set_intersection` to find common elements between two sorted vectors. Implement a top-K finder using `partial_sort`.

---

## Phase 4 — Strings & Text (1–2 weeks)

| Feature | Header | Notes |
|---------|--------|-------|
| `std::string` | `<string>` | Mutable, owning string |
| `std::string_view` *(C++17)* | `<string_view>` | Non-owning string reference |
| `std::wstring`, `std::u8string`, `std::u16string`, `std::u32string` | `<string>` | Wide and Unicode strings |
| `std::to_string` | `<string>` | Number → string |
| `std::stoi`, `std::stod`, etc. | `<string>` | String → number |
| `std::format` *(C++20)* | `<format>` | Python-style formatting |
| `std::print` / `std::println` *(C++23)* | `<print>` | Formatted output |
| `std::regex` | `<regex>` | Regular expressions |
| `std::charconv` (`from_chars`, `to_chars`) *(C++17)* | `<charconv>` | Fast, locale-independent conversions |

**Key concepts:**
- Why `string_view` beats `const string&` for function parameters
- `from_chars` / `to_chars` for high-performance number parsing (no allocation, no locale)
- `std::format` vs `printf` vs `stringstream`

**Practice:** Write a CSV parser using `string_view` for zero-copy field access. Format a table of data using `std::format` with width and precision specifiers.

---

## Phase 5 — Memory Management (1–2 weeks)

| Feature | Header | Notes |
|---------|--------|-------|
| `std::unique_ptr<T>` | `<memory>` | Single-owner smart pointer |
| `std::shared_ptr<T>` | `<memory>` | Reference-counted ownership |
| `std::weak_ptr<T>` | `<memory>` | Non-owning observer of shared_ptr |
| `std::make_unique` / `std::make_shared` | `<memory>` | Preferred factory functions |
| `std::allocator<T>` | `<memory>` | Default allocator; basis for custom ones |
| `std::pmr::polymorphic_allocator` *(C++17)* | `<memory_resource>` | Runtime-switchable allocators |
| `std::pmr::monotonic_buffer_resource` | `<memory_resource>` | Arena/bump allocator |
| `std::pmr::unsynchronized_pool_resource` | `<memory_resource>` | Pool allocator |
| `std::align` | `<memory>` | Aligned pointer adjustment |
| `std::destroy_at`, `std::construct_at` *(C++17/20)* | `<memory>` | Explicit lifetime management |

**Key concepts:**
- `shared_ptr` control block and the cost of `make_shared` vs separate allocation
- Breaking `shared_ptr` cycles with `weak_ptr`
- PMR (Polymorphic Memory Resources) for performance-critical code

**Practice:** Replace all raw `new`/`delete` in a project with smart pointers. Implement a simple arena allocator using `monotonic_buffer_resource`.

---

## Phase 6 — Utilities & Type Support (1–2 weeks)

### Vocabulary Types

| Feature | Header | Notes |
|---------|--------|-------|
| `std::optional<T>` *(C++17)* | `<optional>` | Value or nothing |
| `std::variant<T...>` *(C++17)* | `<variant>` | Type-safe union |
| `std::any` *(C++17)* | `<any>` | Type-erased value |
| `std::expected<T,E>` *(C++23)* | `<expected>` | Value or error — no exceptions |
| `std::pair<A,B>` | `<utility>` | Two-element tuple |
| `std::tuple<T...>` | `<tuple>` | N-element heterogeneous container |

### Type Traits & Metaprogramming

| Feature | Header | Notes |
|---------|--------|-------|
| `std::is_integral_v<T>`, `std::is_floating_point_v<T>`, etc. | `<type_traits>` | Compile-time type queries |
| `std::is_same_v<A, B>` | `<type_traits>` | Type equality |
| `std::enable_if` / `std::void_t` | `<type_traits>` | SFINAE helpers (pre-C++20) |
| `std::conditional_t<B,T,F>` | `<type_traits>` | Compile-time type selection |
| `std::decay_t`, `std::remove_cv_t`, `std::remove_reference_t` | `<type_traits>` | Type transformations |
| `std::invoke` / `std::apply` | `<functional>` | Call any callable / unpack tuple as args |
| `std::function<Sig>` | `<functional>` | Type-erased callable |
| `std::bind_front` *(C++20)* | `<functional>` | Partial application |

**Practice:** Write a `Result<T>` type using `std::expected`. Use `std::visit` to implement a polymorphic printer over a `std::variant<int, double, std::string>`.

---

## Phase 7 — I/O & Filesystem (1–2 weeks)

### I/O Streams

| Feature | Header | Notes |
|---------|--------|-------|
| `std::cin` / `std::cout` / `std::cerr` | `<iostream>` | Standard streams |
| `std::ifstream` / `std::ofstream` / `std::fstream` | `<fstream>` | File streams |
| `std::stringstream` / `std::istringstream` | `<sstream>` | String-backed streams |
| `std::iomanip` manipulators (`setw`, `setprecision`, `hex`, etc.) | `<iomanip>` | Stream formatting |
| `std::sync_with_stdio` | `<ios>` | C/C++ stream sync control |

### Filesystem (C++17)

| Feature | Notes |
|---------|-------|
| `std::filesystem::path` | Portable path manipulation |
| `std::filesystem::exists`, `is_regular_file`, `is_directory` | File status queries |
| `std::filesystem::directory_iterator` | Iterate directory contents |
| `std::filesystem::recursive_directory_iterator` | Recursive traversal |
| `std::filesystem::copy`, `rename`, `remove`, `create_directories` | File operations |
| `std::filesystem::file_size`, `last_write_time` | File metadata |

**Practice:** Write a tool that scans a directory tree, groups files by extension, and prints a summary report.

---

## Phase 8 — Concurrency (2–3 weeks)

> One of the most important and complex parts of the standard library. Take your time.

### Week 1 — Threads & Synchronisation

| Feature | Header | Notes |
|---------|--------|-------|
| `std::thread` | `<thread>` | OS thread abstraction |
| `std::jthread` *(C++20)* | `<thread>` | Joinable thread with stop token |
| `std::mutex` | `<mutex>` | Mutual exclusion |
| `std::recursive_mutex` | `<mutex>` | Re-entrant mutex |
| `std::lock_guard<M>` | `<mutex>` | RAII mutex lock (single mutex) |
| `std::unique_lock<M>` | `<mutex>` | Flexible RAII lock (can unlock early) |
| `std::scoped_lock<M...>` *(C++17)* | `<mutex>` | Deadlock-safe multi-mutex lock |
| `std::condition_variable` | `<condition_variable>` | Thread notification |
| `std::once_flag` + `std::call_once` | `<mutex>` | One-time initialisation |

---

### Week 2 — Atomics & Lock-Free Programming

| Feature | Header | Notes |
|---------|--------|-------|
| `std::atomic<T>` | `<atomic>` | Lock-free atomic operations |
| `std::atomic_flag` | `<atomic>` | Simplest lock-free boolean |
| Memory orders: `relaxed`, `acquire`, `release`, `seq_cst` | `<atomic>` | Control synchronisation guarantees |
| `std::atomic_ref<T>` *(C++20)* | `<atomic>` | Atomic access to non-atomic objects |

---

### Week 3 — Async & Futures

| Feature | Header | Notes |
|---------|--------|-------|
| `std::future<T>` | `<future>` | Retrieve async result |
| `std::promise<T>` | `<future>` | Set async result |
| `std::async` | `<future>` | Launch async task |
| `std::packaged_task<F>` | `<future>` | Wrap callable as future-based task |
| `std::shared_future<T>` | `<future>` | Multiple consumers of one result |
| `std::latch` *(C++20)* | `<latch>` | Single-use countdown synchroniser |
| `std::barrier` *(C++20)* | `<barrier>` | Reusable phase barrier |
| `std::semaphore` *(C++20)* | `<semaphore>` | Counting semaphore |

**Practice:** Implement a thread pool. Write a parallel map using `std::async`. Implement a producer-consumer queue using `mutex` + `condition_variable`.

---

## Phase 9 — Numerics, Math & Random (1 week)

| Feature | Header | Notes |
|---------|--------|-------|
| `std::numeric_limits<T>` | `<limits>` | Type min/max, epsilon, infinity |
| `std::abs`, `std::sqrt`, `std::pow`, `std::log`, etc. | `<cmath>` | Standard math functions |
| `std::complex<T>` | `<complex>` | Complex number arithmetic |
| `std::valarray<T>` | `<valarray>` | SIMD-friendly numeric array |
| `std::random_device` | `<random>` | Non-deterministic seed source |
| `std::mt19937` / `std::mt19937_64` | `<random>` | Mersenne Twister PRNG |
| `std::uniform_int_distribution` | `<random>` | Uniform integer distribution |
| `std::uniform_real_distribution` | `<random>` | Uniform float distribution |
| `std::normal_distribution` | `<random>` | Gaussian distribution |
| `std::gcd`, `std::lcm` *(C++17)* | `<numeric>` | Greatest common divisor / LCM |
| `std::midpoint`, `std::lerp` *(C++20)* | `<numeric>` | Safe midpoint and linear interpolation |
| `std::bit_cast<T>` *(C++20)* | `<bit>` | Reinterpret bits safely |
| `std::popcount`, `std::countl_zero`, etc. *(C++20)* | `<bit>` | Bit manipulation |

**Practice:** Write a dice simulator using `mt19937` + `uniform_int_distribution`. Use `std::bit_cast` to inspect the bit layout of a `float`.

---

## Phase 10 — Time & Chrono (1 week)

| Feature | Header | Notes |
|---------|--------|-------|
| `std::chrono::system_clock` | `<chrono>` | Wall clock time |
| `std::chrono::steady_clock` | `<chrono>` | Monotonic clock (for benchmarking) |
| `std::chrono::high_resolution_clock` | `<chrono>` | Highest precision available |
| `std::chrono::duration<R, P>` | `<chrono>` | Typed time duration |
| Duration literals: `1s`, `100ms`, `2min`, `3h` | `<chrono>` | Readable duration construction |
| `std::chrono::time_point<C, D>` | `<chrono>` | Point in time on a clock |
| `std::chrono::year_month_day` *(C++20)* | `<chrono>` | Calendar date |
| `std::chrono::zoned_time` *(C++20)* | `<chrono>` | Timezone-aware time |
| `std::this_thread::sleep_for` / `sleep_until` | `<thread>` | Timed thread sleep |

**Practice:** Write a `Stopwatch` class using `steady_clock`. Use `year_month_day` to compute the number of days between two dates.

---

## Capstone Projects

| After Phase | Project |
|-------------|---------|
| Containers + Algorithms | Implement a mini `grep` — search files for a pattern, collect matches into sorted output |
| Strings | A CSV parser that handles quoted fields using `string_view` and `charconv` |
| Memory | A simple object pool using PMR `monotonic_buffer_resource` |
| Concurrency | A parallel file word-count tool using `jthread` and `atomic` |
| Full stdlib | A command-line task manager: JSON-ish persistence with `fstream`, sorted display with `map`, deadline tracking with `chrono` |

---

## Quick Reference: Headers by Category

| Category | Key Headers |
|----------|------------|
| Containers | `<vector>` `<array>` `<deque>` `<list>` `<map>` `<set>` `<unordered_map>` `<unordered_set>` `<stack>` `<queue>` `<span>` |
| Iterators & Ranges | `<iterator>` `<ranges>` |
| Algorithms | `<algorithm>` `<numeric>` |
| Strings | `<string>` `<string_view>` `<format>` `<regex>` `<charconv>` |
| Memory | `<memory>` `<memory_resource>` |
| Utilities | `<optional>` `<variant>` `<any>` `<expected>` `<tuple>` `<utility>` `<functional>` `<type_traits>` |
| I/O | `<iostream>` `<fstream>` `<sstream>` `<iomanip>` |
| Filesystem | `<filesystem>` |
| Concurrency | `<thread>` `<mutex>` `<condition_variable>` `<atomic>` `<future>` `<latch>` `<barrier>` `<semaphore>` |
| Numerics | `<cmath>` `<complex>` `<random>` `<numeric>` `<limits>` `<bit>` |
| Time | `<chrono>` |

---

## Recommended Resources

| Resource | Best For |
|----------|---------|
| [cppreference.com](https://cppreference.com) | Complete, accurate reference for every header and class |
| *The C++ Standard Library* — Nicolai Josuttis | The definitive book on the stdlib |
| [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) | When and how to use stdlib features correctly |
| [Compiler Explorer — godbolt.org](https://godbolt.org) | See generated code; experiment with algorithms |
| [Quick C++ Benchmarks — quick-bench.com](https://quick-bench.com) | Benchmark container and algorithm choices |
| [C++ Weekly — Jason Turner (YouTube)](https://www.youtube.com/@cppweekly) | Short practical videos on stdlib features |

---

*Total estimated time: ~16–22 weeks at 1–2 hours/day*
*Generated with Claude · Happy coding! 🚀*