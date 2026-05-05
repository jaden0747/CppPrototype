#pragma once
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Observer Pattern
//
// Intent: Define a one-to-many dependency between objects so that when one
// object changes state, all its dependents are notified and updated
// automatically.
//
// Real-world analogy: Newspaper subscriptions. Subscribers are notified
// whenever a new edition is published; they can subscribe/unsubscribe at
// any time without the publisher knowing who they are.
//
// Key C++ mechanics used:
//  - Pure-virtual IObserver interface keeps observers decoupled from subjects.
//  - ISubject interface formalises attach/detach/notify contract.
//  - EventEmitter<T> is a generic, type-safe subject backed by std::function,
//    useful when you do not need a class hierarchy on the observer side.
//
// When to use:
//  - A change to one object requires updating others, and you don't know how
//    many objects need to change.
//  - An object should be able to notify other objects without making
//    assumptions about who those objects are.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Observer interface -------------------------------------------

struct Event
{
    std::string type;
    std::string data;
};

class IObserver
{
public:
    virtual ~IObserver()                 = default;
    virtual void onEvent(const Event& e) = 0;
};

// ---------- Subject interface --------------------------------------------

class ISubject
{
public:
    virtual ~ISubject()                 = default;
    virtual void attach(IObserver* o)   = 0;
    virtual void detach(IObserver* o)   = 0;
    virtual void notify(const Event& e) = 0;
};

// ---------- Concrete Subject (Stock ticker) ------------------------------

class StockMarket : public ISubject
{
public:
    void attach(IObserver* o) override
    {
        observers_.push_back(o);
    }

    void detach(IObserver* o) override
    {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), o), observers_.end());
    }

    void notify(const Event& e) override
    {
        for (IObserver* obs : observers_)
            obs->onEvent(e);
    }

    void setPrice(const std::string& ticker, double price)
    {
        lastTicker_ = ticker;
        lastPrice_  = price;
        notify(Event{ticker, std::to_string(price)});
    }

    const std::string& lastTicker() const
    {
        return lastTicker_;
    }
    double lastPrice() const
    {
        return lastPrice_;
    }

    std::size_t subscriberCount() const
    {
        return observers_.size();
    }

private:
    std::vector<IObserver*> observers_;
    std::string             lastTicker_;
    double                  lastPrice_{0.0};
};

// ---------- Concrete Observers -------------------------------------------

class Logger : public IObserver
{
public:
    void onEvent(const Event& e) override
    {
        log_.push_back("[" + e.type + "] " + e.data);
    }
    const std::vector<std::string>& log() const
    {
        return log_;
    }

private:
    std::vector<std::string> log_;
};

class AlertMonitor : public IObserver
{
public:
    explicit AlertMonitor(double threshold)
        : threshold_(threshold)
    {
    }

    void onEvent(const Event& e) override
    {
        double price = std::stod(e.data);
        if (price > threshold_)
            alerts_.push_back(e.type + " exceeded threshold: " + e.data);
    }
    const std::vector<std::string>& alerts() const
    {
        return alerts_;
    }

private:
    double                   threshold_;
    std::vector<std::string> alerts_;
};

// ---------- Generic EventEmitter (callback-based) -----------------------

template <typename T>
class EventEmitter
{
public:
    using Handler = std::function<void(const T&)>;

    int subscribe(Handler h)
    {
        int id = nextId_++;
        handlers_.push_back({id, std::move(h)});
        return id;
    }

    void unsubscribe(int id)
    {
        handlers_.erase(
            std::remove_if(handlers_.begin(), handlers_.end(), [id](const Entry& e) { return e.id == id; }),
            handlers_.end());
    }

    void emit(const T& value)
    {
        for (auto& entry : handlers_)
            entry.handler(value);
    }

    std::size_t subscriberCount() const
    {
        return handlers_.size();
    }

private:
    struct Entry
    {
        int     id;
        Handler handler;
    };
    std::vector<Entry> handlers_;
    int                nextId_{0};
};

} // namespace pattern
