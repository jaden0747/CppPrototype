#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Strategy Pattern
//
// Intent: Define a family of algorithms, encapsulate each one, and make
// them interchangeable. Strategy lets the algorithm vary independently from
// the clients that use it.
//
// Real-world analogy: Navigation apps (Google Maps, Waze). The route-finding
// algorithm (fastest / shortest / avoid tolls) can be swapped at runtime
// while the app's UI and data remain the same.
//
// Key C++ mechanics used:
//  - IStrategy interface / pure-virtual base
//  - Context owns a strategy by pointer (can swap at runtime)
//  - std::function variant (FunctionalSorter) when you don't need a class
//    hierarchy — a lambda or free function is enough.
//
// When to use:
//  - You want to define a class that will have one behaviour from a family
//    of behaviours, and the behaviour can be chosen at runtime.
//  - You need different variants of an algorithm.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Strategy interface -------------------------------------------

class ISortStrategy
{
public:
    virtual ~ISortStrategy()                               = default;
    virtual void        sort(std::vector<int>& data) const = 0;
    virtual std::string name() const                       = 0;
};

// ---------- Concrete Strategies ------------------------------------------

class BubbleSort : public ISortStrategy
{
public:
    void sort(std::vector<int>& data) const override
    {
        for (std::size_t i = 0; i + 1 < data.size(); ++i)
            for (std::size_t j = 0; j + 1 < data.size() - i; ++j)
                if (data[j] > data[j + 1])
                    std::swap(data[j], data[j + 1]);
    }
    std::string name() const override
    {
        return "BubbleSort";
    }
};

class QuickSort : public ISortStrategy
{
public:
    void sort(std::vector<int>& data) const override
    {
        std::sort(data.begin(), data.end());
    }
    std::string name() const override
    {
        return "QuickSort";
    }
};

class ReverseSort : public ISortStrategy
{
public:
    void sort(std::vector<int>& data) const override
    {
        std::sort(data.rbegin(), data.rend());
    }
    std::string name() const override
    {
        return "ReverseSort";
    }
};

// ---------- Context -------------------------------------------------------

class Sorter
{
public:
    explicit Sorter(std::unique_ptr<ISortStrategy> strategy)
        : strategy_(std::move(strategy))
    {
    }

    void setStrategy(std::unique_ptr<ISortStrategy> strategy)
    {
        strategy_ = std::move(strategy);
    }

    void sort(std::vector<int>& data) const
    {
        strategy_->sort(data);
    }

    std::string strategyName() const
    {
        return strategy_->name();
    }

private:
    std::unique_ptr<ISortStrategy> strategy_;
};

// ---------- Functional variant (std::function) ---------------------------

class FunctionalSorter
{
public:
    using Strategy = std::function<void(std::vector<int>&)>;

    explicit FunctionalSorter(Strategy s)
        : strategy_(std::move(s))
    {
    }

    void setStrategy(Strategy s)
    {
        strategy_ = std::move(s);
    }
    void sort(std::vector<int>& data) const
    {
        strategy_(data);
    }

private:
    Strategy strategy_;
};

// ---------- Convenience factory helpers ----------------------------------

inline std::unique_ptr<ISortStrategy> makeBubbleSort()
{
    return std::unique_ptr<ISortStrategy>(new BubbleSort());
}
inline std::unique_ptr<ISortStrategy> makeQuickSort()
{
    return std::unique_ptr<ISortStrategy>(new QuickSort());
}
inline std::unique_ptr<ISortStrategy> makeReverseSort()
{
    return std::unique_ptr<ISortStrategy>(new ReverseSort());
}

} // namespace pattern
