// ============================================================================
// Template 10 — Exercises: Expression Templates & Tag Dispatch
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Extend the Vec expression template with operator- (subtraction).
// Write VecSub expression and test: a - b gives element-wise subtraction.
// ──────────────────────────────────────────────────────────────────────────
// TODO: struct VecSub : VecExpr<VecSub<L,R>> { ... };
// TODO: operator-

void exercise_vec_sub()
{
    // Vec a{5.0, 10.0, 15.0};
    // Vec b{1.0, 2.0, 3.0};
    // Vec result(3);
    // result = a - b;
    // assert(result[0] == 4.0 && result[1] == 8.0 && result[2] == 12.0);
    std::cout << "  Exercise 1: VecSub — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a tag-dispatch based `allocate<T>(n)` that:
//   - stack_tag → uses alloca or a local array
//   - heap_tag → uses new T[n]
//   - pool_tag → uses a simple static pool
// ──────────────────────────────────────────────────────────────────────────
// TODO: tag types and allocate overloads

void exercise_allocator()
{
    // auto* p = allocate<int, heap_tag>(10);
    // p[0] = 42;
    // assert(p[0] == 42);
    // deallocate<heap_tag>(p);
    std::cout << "  Exercise 2: allocator tags — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a policy-based Logger class with:
//   - OutputPolicy: ConsoleOutput, FileOutput, NullOutput
//   - FormatPolicy: PlainFormat, TimestampFormat
// Logger<ConsoleOutput, TimestampFormat>::log("message")
// ──────────────────────────────────────────────────────────────────────────
// TODO: policies and Logger class

void exercise_logger()
{
    // Logger<ConsoleOutput, PlainFormat> log;
    // log.log("hello");  // outputs: hello
    // Logger<NullOutput, PlainFormat> null_log;
    // null_log.log("silent");  // no output
    std::cout << "  Exercise 3: Logger — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Use the detecting idiom to write:
//   has_reserve<T> — true if T has a .reserve(size_t) method
//   has_operator_plus<T> — true if T + T is valid
// Then write a function that conditionally calls reserve.
// ──────────────────────────────────────────────────────────────────────────
// TODO: detectors and conditional function

void exercise_detect()
{
    // static_assert(has_reserve<std::vector<int>>);
    // static_assert(!has_reserve<std::array<int, 5>>);
    // static_assert(has_operator_plus<int>);
    // static_assert(has_operator_plus<std::string>);
    std::cout << "  Exercise 4: detecting idiom — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a policy-based SmartPointer<T, Ownership, Threading>:
//   - Ownership: UniqueOwnership, SharedOwnership (ref-counted)
//   - Threading: SingleThread, MultiThread (atomic ref count)
// ──────────────────────────────────────────────────────────────────────────
// TODO: policies and SmartPointer class

void exercise_smart_ptr()
{
    // SmartPointer<int, UniqueOwnership, SingleThread> p(new int(42));
    // assert(*p == 42);
    // SmartPointer<int, SharedOwnership, SingleThread> sp(new int(10));
    // auto sp2 = sp;  // shared ref count
    std::cout << "  Exercise 5: SmartPointer — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 10 — Exercises: Expr Templates & Tags  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_vec_sub();
    exercise_allocator();
    exercise_logger();
    exercise_detect();
    exercise_smart_ptr();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
