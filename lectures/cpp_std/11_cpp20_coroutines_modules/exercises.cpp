// ============================================================================
// Lecture 11 — Exercises: C++20 Coroutines & Modules
// ============================================================================
#include <coroutine>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Provided: Basic Generator<T> (use this for exercises)
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct Generator
{
    struct promise_type
    {
        T         current_value;
        Generator get_return_object()
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend()
        {
            return {};
        }
        std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        std::suspend_always yield_value(T value)
        {
            current_value = std::move(value);
            return {};
        }
        void return_void()
        {
        }
        void unhandled_exception()
        {
            std::terminate();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type handle_;
    explicit Generator(handle_type h)
        : handle_(h)
    {
    }
    ~Generator()
    {
        if (handle_)
            handle_.destroy();
    }
    Generator(Generator&& o) noexcept
        : handle_(std::exchange(o.handle_, {}))
    {
    }
    Generator(const Generator&) = delete;

    bool next()
    {
        handle_.resume();
        return !handle_.done();
    }
    T value() const
    {
        return handle_.promise().current_value;
    }
};

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write generators
// a) powers_of_two() — yields 1, 2, 4, 8, 16, ... infinitely
// b) countdown(n) — yields n, n-1, ..., 1, 0
// c) cycle(items) — yields items[0], items[1], ..., items[n-1], items[0], ...
// ──────────────────────────────────────────────────────────────────────────
// TODO: Generator<int> powers_of_two() { ... }
// TODO: Generator<int> countdown(int n) { ... }
// TODO: Generator<std::string> cycle(std::vector<std::string> items) { ... }

void exercise_generators()
{
    // auto pw = powers_of_two();
    // pw.next(); assert(pw.value() == 1);
    // pw.next(); assert(pw.value() == 2);
    // pw.next(); assert(pw.value() == 4);
    // pw.next(); assert(pw.value() == 8);

    // auto cd = countdown(3);
    // cd.next(); assert(cd.value() == 3);
    // cd.next(); assert(cd.value() == 2);
    // cd.next(); assert(cd.value() == 1);
    // cd.next(); assert(cd.value() == 0);
    // assert(!cd.next());  // done

    std::cout << "  Exercise 1: generators — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Implement Task<T> from scratch
// A coroutine type that eagerly starts and stores a single result via co_return.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> struct Task { ... };

// TODO: Task<int> add_async(int a, int b) { co_return a + b; }
// TODO: Task<std::string> concat_async(std::string a, std::string b) { co_return a + b; }

void exercise_task()
{
    // auto t1 = add_async(10, 20);
    // assert(t1.result() == 30);
    // auto t2 = concat_async("hello ", "world");
    // assert(t2.result() == "hello world");
    std::cout << "  Exercise 2: Task<T> — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Coroutine-based tokenizer
// Write a generator that tokenizes a string by whitespace:
//   tokenize("hello world foo") yields "hello", "world", "foo"
// ──────────────────────────────────────────────────────────────────────────
// TODO: Generator<std::string> tokenize(std::string input) { ... }

void exercise_tokenizer()
{
    // auto tokens = tokenize("hello world foo bar");
    // tokens.next(); assert(tokens.value() == "hello");
    // tokens.next(); assert(tokens.value() == "world");
    // tokens.next(); assert(tokens.value() == "foo");
    // tokens.next(); assert(tokens.value() == "bar");
    // assert(!tokens.next());
    std::cout << "  Exercise 3: tokenizer — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Stateful coroutine (game turns)
// Write a coroutine that simulates a turn-based game:
//   - Alternates between "Player 1" and "Player 2"
//   - Yields the current player name + turn number
//   Example: "Player 1: turn 1", "Player 2: turn 2", ...
// ──────────────────────────────────────────────────────────────────────────
// TODO: Generator<std::string> game_turns(int max_turns) { ... }

void exercise_game_turns()
{
    // auto game = game_turns(4);
    // game.next(); assert(game.value() == "Player 1: turn 1");
    // game.next(); assert(game.value() == "Player 2: turn 2");
    // game.next(); assert(game.value() == "Player 1: turn 3");
    // game.next(); assert(game.value() == "Player 2: turn 4");
    // assert(!game.next());
    std::cout << "  Exercise 4: game turns — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Generator composition
// Write a function that "chains" two generators:
//   chain(gen1, gen2) yields all of gen1, then all of gen2
// ──────────────────────────────────────────────────────────────────────────
// TODO: Generator<int> chain(Generator<int> g1, Generator<int> g2) { ... }

void exercise_chain()
{
    // auto g1 = range(1, 4);   // 1, 2, 3
    // auto g2 = range(10, 13); // 10, 11, 12
    // auto combined = chain(std::move(g1), std::move(g2));
    // std::vector<int> result;
    // while (combined.next()) result.push_back(combined.value());
    // assert(result == std::vector<int>({1,2,3,10,11,12}));
    std::cout << "  Exercise 5: chain — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Build an async scheduler
// Implement:
//   - A simple Scheduler class that stores coroutine handles
//   - An Awaiter that suspends the current coroutine and adds it to scheduler
//   - scheduler.run() resumes all ready coroutines in round-robin
// ──────────────────────────────────────────────────────────────────────────

void exercise_scheduler()
{
    // Scheduler sched;
    // sched.spawn(task_a());  // task that prints "A1", yields, prints "A2"
    // sched.spawn(task_b());  // task that prints "B1", yields, prints "B2"
    // sched.run();
    // Expected output: A1 B1 A2 B2 (interleaved)
    std::cout << "  Exercise 6: scheduler — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 11 — Exercises: Coroutines & Modules    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_generators();
    exercise_task();
    exercise_tokenizer();
    exercise_game_turns();
    exercise_chain();
    exercise_scheduler();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
