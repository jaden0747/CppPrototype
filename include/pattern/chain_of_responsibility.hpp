#pragma once
#include <functional>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Chain of Responsibility Pattern
//
// Intent: Avoid coupling the sender of a request to its receiver by giving
// more than one object a chance to handle the request. Chain the receiving
// objects and pass the request along the chain until an object handles it.
//
// Real-world analogy: A technical support escalation system.
// A Tier-1 agent handles basic questions; if stumped, escalates to Tier-2;
// if still unsolved, escalates to Tier-3 engineering.
//
// Key C++ mechanics used:
//  - Abstract Handler with a `setNext()` and `handle()` interface.
//  - Concrete handlers decide whether to process or pass along.
//  - Chaining via unique_ptr; handler owns its successor.
//  - Request passed as a value type (struct with severity level).
//
// When to use:
//  - More than one object may handle a request, and the handler isn't known
//    a priori.
//  - You want to issue a request without specifying the receiver explicitly.
//  - The set of objects that can handle the request should be specified
//    dynamically.
// ---------------------------------------------------------------------------

namespace pattern
{

struct SupportRequest
{
    int         level; // 1 = basic, 2 = intermediate, 3 = advanced
    std::string description;
};

// ---------- Abstract Handler ---------------------------------------------

class SupportHandler
{
public:
    virtual ~SupportHandler() = default;

    SupportHandler* setNext(std::unique_ptr<SupportHandler> next)
    {
        next_ = std::move(next);
        return next_.get();
    }

    virtual std::string handle(const SupportRequest& req) = 0;

protected:
    std::string passToNext(const SupportRequest& req)
    {
        if (next_)
            return next_->handle(req);
        return "Unhandled: " + req.description;
    }

    std::unique_ptr<SupportHandler> next_;
};

// ---------- Concrete Handlers --------------------------------------------

class Tier1Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 1)
            return "Tier1 handled: " + req.description;
        return passToNext(req);
    }
};

class Tier2Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 2)
            return "Tier2 handled: " + req.description;
        return passToNext(req);
    }
};

class Tier3Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 3)
            return "Tier3 handled: " + req.description;
        return passToNext(req);
    }
};

// ---------- Helper: build a default chain --------------------------------
// Returns the head (Tier1 → Tier2 → Tier3)
inline std::unique_ptr<SupportHandler> buildDefaultChain()
{
    auto t1 = std::unique_ptr<SupportHandler>(new Tier1Handler());
    auto t2 = std::unique_ptr<SupportHandler>(new Tier2Handler());
    auto t3 = std::unique_ptr<SupportHandler>(new Tier3Handler());
    t2->setNext(std::move(t3));
    t1->setNext(std::move(t2));
    return t1;
}

} // namespace pattern
