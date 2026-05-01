// ============================================================================
// Lecture 04 — Demo: Compile-Time Power & Threads (C++11)
// ============================================================================
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. constexpr — compile-time computation
// ──────────────────────────────────────────────────────────────────────────
constexpr int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int fibonacci(int n)
{
    return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

void demo_constexpr()
{
    std::cout << "=== 1. constexpr ===\n";

    // All computed at compile time:
    constexpr int f5    = factorial(5);
    constexpr int f10   = factorial(10);
    constexpr int fib10 = fibonacci(10);

    static_assert(f5 == 120, "");
    static_assert(f10 == 3628800, "");
    static_assert(fib10 == 55, "");

    std::cout << "  factorial(5)  = " << f5 << "\n";
    std::cout << "  factorial(10) = " << f10 << "\n";
    std::cout << "  fibonacci(10) = " << fib10 << "\n";

    // constexpr array
    constexpr int TABLE[] = {factorial(1), factorial(2), factorial(3), factorial(4), factorial(5)};
    std::cout << "  Table: ";
    for (auto v : TABLE)
        std::cout << v << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. static_assert
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
class TypeSafeBuffer
{
    static_assert(std::is_trivially_copyable<T>::value, "TypeSafeBuffer only works with trivially copyable types");
    static_assert(sizeof(T) <= 64, "TypeSafeBuffer elements must be <= 64 bytes");
    T data_[256];

public:
    T& operator[](std::size_t i)
    {
        return data_[i];
    }
};

void demo_static_assert()
{
    std::cout << "=== 2. static_assert ===\n";

    static_assert(sizeof(int) >= 4, "Need at least 32-bit ints");
    static_assert(sizeof(void*) >= 4, "Need at least 32-bit pointers");

    TypeSafeBuffer<int> buf; // OK — int is trivially copyable
    buf[0] = 42;
    // TypeSafeBuffer<std::string> bad;  // FAILS: string not trivially copyable

    std::cout << "  All static_asserts passed at compile time!\n";
    std::cout << "  buf[0] = " << buf[0] << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Variadic templates
// ──────────────────────────────────────────────────────────────────────────

// Type-safe print
void typesafe_print(std::ostream& os)
{
    os << "\n";
}

template <typename T, typename... Args>
void typesafe_print(std::ostream& os, const T& first, const Args&... rest)
{
    os << first;
    if (sizeof...(rest) > 0)
        os << ", ";
    typesafe_print(os, rest...);
}

// Variadic sum
template <typename T>
constexpr T sum(T value)
{
    return value;
}

template <typename T, typename... Args>
constexpr T sum(T first, Args... rest)
{
    return first + sum(rest...);
}

// make_unique emulation (C++11 style)
template <typename T, typename... Args>
std::unique_ptr<T> my_make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void demo_variadic()
{
    std::cout << "=== 3. Variadic Templates ===\n";

    std::cout << "  print: ";
    typesafe_print(std::cout, 42, " hello", 3.14, " world");

    constexpr auto s = sum(1, 2, 3, 4, 5);
    static_assert(s == 15, "");
    std::cout << "  sum(1..5) = " << s << "\n";

    // sizeof... works on a parameter pack name, not a type directly
    auto count_types = []<typename... Ts>() { return sizeof...(Ts); };
    std::cout << "  sizeof... demo (int,double,char): " << count_types.template operator()<int, double, char>()
              << " type(s)\n";

    auto p = my_make_unique<std::string>("hello from make_unique");
    std::cout << "  my_make_unique: " << *p << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Threads & mutex
// ──────────────────────────────────────────────────────────────────────────
void demo_threads()
{
    std::cout << "=== 4. Threads & Mutex ===\n";

    std::mutex mtx;
    int        counter = 0;
    const int  N       = 10000;

    auto worker = [&](int id)
    {
        for (int i = 0; i < N; ++i)
        {
            std::lock_guard<std::mutex> lock(mtx);
            ++counter;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker, i);
    for (auto& t : threads)
        t.join();

    std::cout << "  4 threads x " << N << " increments = " << counter << "\n";
    std::cout << "  Expected: " << 4 * N << (counter == 4 * N ? " ✓" : " ✗") << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Condition variable & async
// ──────────────────────────────────────────────────────────────────────────
void demo_async()
{
    std::cout << "=== 5. std::async ===\n";

    auto heavy = [](int n) -> long long
    {
        long long sum = 0;
        for (int i = 0; i < n; ++i)
            sum += i;
        return sum;
    };

    auto f1 = std::async(std::launch::async, heavy, 1000000);
    auto f2 = std::async(std::launch::async, heavy, 2000000);

    std::cout << "  Computing in parallel...\n";
    auto r1 = f1.get();
    auto r2 = f2.get();
    std::cout << "  sum(0..1M) = " << r1 << "\n";
    std::cout << "  sum(0..2M) = " << r2 << "\n\n";
}

void demo_producer_consumer()
{
    std::cout << "=== 5b. Producer/Consumer ===\n";

    std::queue<int>         q;
    std::mutex              mtx;
    std::condition_variable cv;
    bool                    done = false;

    // Producer
    std::thread producer(
        [&]
        {
            for (int i = 0; i < 5; ++i)
            {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    q.push(i);
                }
                cv.notify_one();
            }
            {
                std::lock_guard<std::mutex> lock(mtx);
                done = true;
            }
            cv.notify_one();
        });

    // Consumer
    std::thread consumer(
        [&]
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] { return !q.empty() || done; });
                while (!q.empty())
                {
                    std::cout << "  Consumed: " << q.front() << "\n";
                    q.pop();
                }
                if (done)
                    break;
            }
        });

    producer.join();
    consumer.join();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 04 — Compile-Time Power & Threads       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_constexpr();
    demo_static_assert();
    demo_variadic();
    demo_threads();
    demo_async();
    demo_producer_consumer();

    std::cout << "All demos complete.\n";
    return 0;
}
