#pragma once
#include <mutex>

// ---------------------------------------------------------------------------
// Singleton Pattern
//
// Intent: Ensure a class has only one instance and provide a global access
// point to it.
//
// Real-world analogy: A government has only one official president at a time.
// Any citizen asking "who is the president?" gets the same person.
//
// Key C++ mechanics used:
//  - Static local variable (Meyers Singleton) — guaranteed thread-safe init
//    since C++11 (§6.7 of the standard).
//  - Deleted copy constructor and copy-assignment to prevent duplication.
//  - Private constructor to prevent external instantiation.
//
// When to use:
//  - Logger, configuration manager, thread pool, hardware interface driver,
//    or any resource that must exist exactly once.
//
// Trade-offs:
//  - Introduces global state — makes unit-testing harder.
//  - Tight coupling: clients depend on the concrete Singleton class.
//  - Hidden dependencies are hard to reason about.
// ---------------------------------------------------------------------------

namespace pattern
{

// A Meyers Singleton that carries a simple integer counter as example state.
// Replace `int counter_` with whatever shared resource you need.
class Singleton
{
public:
    // Returns the sole instance. Thread-safe since C++11.
    static Singleton& instance()
    {
        static Singleton inst;
        return inst;
    }

    // --- example interface ------------------------------------------------
    void increment()
    {
        ++counter_;
    }
    void reset()
    {
        counter_ = 0;
    }
    int value() const
    {
        return counter_;
    }
    // ----------------------------------------------------------------------

    // Non-copyable, non-movable
    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&)                 = delete;
    Singleton& operator=(Singleton&&)      = delete;

private:
    Singleton()
        : counter_(0)
    {
    }
    int counter_;
};

} // namespace pattern
