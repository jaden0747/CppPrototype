// ============================================================================
// Lecture 11 — Demo: C++20 Coroutines & Modules
// ============================================================================
// Note: Modules demo is conceptual (requires specific build system support).
// This file focuses on coroutines which work with standard -std=c++20.
// ============================================================================
#include <coroutine>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Generator<T> — A basic lazy sequence coroutine
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct Generator
{
    struct promise_type
    {
        T                  current_value;
        std::exception_ptr exception = nullptr;

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
            exception = std::current_exception();
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

    Generator(Generator&& other) noexcept
        : handle_(std::exchange(other.handle_, {}))
    {
    }
    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other)
        {
            if (handle_)
                handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }
    Generator(const Generator&)            = delete;
    Generator& operator=(const Generator&) = delete;

    bool next()
    {
        handle_.resume();
        if (handle_.promise().exception)
            std::rethrow_exception(handle_.promise().exception);
        return !handle_.done();
    }

    T value() const
    {
        return handle_.promise().current_value;
    }

    // Iterator support for range-for
    struct Iterator
    {
        handle_type handle;
        bool        done;

        Iterator& operator++()
        {
            handle.resume();
            done = handle.done();
            return *this;
        }
        T operator*() const
        {
            return handle.promise().current_value;
        }
        bool operator==(std::default_sentinel_t) const
        {
            return done;
        }
    };

    Iterator begin()
    {
        handle_.resume();
        return Iterator{handle_, handle_.done()};
    }
    std::default_sentinel_t end()
    {
        return {};
    }
};

// ──────────────────────────────────────────────────────────────────────────
// 1. Basic generators
// ──────────────────────────────────────────────────────────────────────────
Generator<int> range(int start, int end)
{
    for (int i = start; i < end; ++i)
        co_yield i;
}

Generator<int> fibonacci()
{
    int a = 0, b = 1;
    while (true)
    {
        co_yield a;
        int next = a + b;
        a        = b;
        b        = next;
    }
}

Generator<int> primes()
{
    co_yield 2;
    for (int n = 3;; n += 2)
    {
        bool is_prime = true;
        for (int d = 3; d * d <= n; d += 2)
        {
            if (n % d == 0)
            {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
            co_yield n;
    }
}

void demo_generators()
{
    std::cout << "=== 1. Generators (co_yield) ===\n";

    // range generator
    std::cout << "  range(1,6): ";
    for (int x : range(1, 6))
        std::cout << x << " ";
    std::cout << "\n";

    // fibonacci (take first 10)
    std::cout << "  fibonacci (first 10): ";
    auto fib = fibonacci();
    for (int i = 0; i < 10 && fib.next(); ++i)
    {
        std::cout << fib.value() << " ";
    }
    std::cout << "\n";

    // primes (first 10)
    std::cout << "  primes (first 10): ";
    auto p = primes();
    for (int i = 0; i < 10 && p.next(); ++i)
    {
        std::cout << p.value() << " ";
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Generator with filtering logic
// ──────────────────────────────────────────────────────────────────────────
Generator<int> even_numbers(int limit)
{
    for (int i = 0; i < limit; i += 2)
        co_yield i;
}

Generator<std::string> string_generator()
{
    co_yield "Hello";
    co_yield "from";
    co_yield "a";
    co_yield "coroutine!";
}

void demo_typed_generators()
{
    std::cout << "=== 2. Typed Generators ===\n";

    std::cout << "  evens: ";
    for (int x : even_numbers(20))
        std::cout << x << " ";
    std::cout << "\n";

    std::cout << "  strings: ";
    for (const auto& s : string_generator())
        std::cout << s << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Simple Task<T> (co_return)
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
struct Task
{
    struct promise_type
    {
        std::optional<T> result;

        Task get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend()
        {
            return {};
        } // eager start
        std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        void return_value(T value)
        {
            result = std::move(value);
        }
        void unhandled_exception()
        {
            std::terminate();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type handle_;

    explicit Task(handle_type h)
        : handle_(h)
    {
    }
    ~Task()
    {
        if (handle_)
            handle_.destroy();
    }
    Task(Task&& o) noexcept
        : handle_(std::exchange(o.handle_, {}))
    {
    }
    Task(const Task&) = delete;

    T result() const
    {
        return *handle_.promise().result;
    }
};

Task<int> compute(int x, int y)
{
    co_return x + y;
}

Task<std::string> greet(std::string name)
{
    co_return "Hello, " + name + "!";
}

void demo_task()
{
    std::cout << "=== 3. Task<T> (co_return) ===\n";

    auto t1 = compute(3, 4);
    std::cout << "  compute(3,4) = " << t1.result() << "\n";

    auto t2 = greet("World");
    std::cout << "  greet(\"World\") = " << t2.result() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Coroutine state machine
// ──────────────────────────────────────────────────────────────────────────
Generator<std::string> traffic_light()
{
    while (true)
    {
        co_yield "RED";
        co_yield "RED+YELLOW";
        co_yield "GREEN";
        co_yield "YELLOW";
    }
}

void demo_state_machine()
{
    std::cout << "=== 4. Coroutine State Machine ===\n";
    std::cout << "  Traffic light sequence: ";
    auto light = traffic_light();
    for (int i = 0; i < 8 && light.next(); ++i)
    {
        std::cout << light.value();
        if (i < 7)
            std::cout << " → ";
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 11 — C++20 Coroutines                   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_generators();
    demo_typed_generators();
    demo_task();
    demo_state_machine();

    std::cout << "All demos complete.\n";
    return 0;
}
