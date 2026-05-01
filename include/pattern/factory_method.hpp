#pragma once
#include <memory>
#include <string>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Factory Method Pattern
//
// Intent: Define an interface for creating an object, but let subclasses
// decide which class to instantiate. Factory Method lets a class defer
// instantiation to subclasses.
//
// Real-world analogy: A logistics company (Creator) needs to deliver goods.
// Road Logistics creates Trucks; Sea Logistics creates Ships. The delivery
// algorithm stays the same — only the transport object changes.
//
// Key C++ mechanics used:
//  - Pure virtual factory method in abstract Creator.
//  - std::unique_ptr for ownership — no raw new/delete at call sites.
//  - Covariant return types (C++11): concrete factories may return a more
//    derived pointer type (not used here for simplicity, but valid).
//
// When to use:
//  - When you don't know ahead of time which class you need to instantiate.
//  - When you want subclasses to specify the objects they create.
//  - Frameworks that call library code from application code (inversion).
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Product hierarchy --------------------------------------------

class Transport
{
public:
    virtual ~Transport() = default;
    virtual std::string deliver() const = 0;
    virtual std::string type()    const = 0;
};

class Truck : public Transport
{
public:
    std::string deliver() const override { return "Delivering by land in a truck"; }
    std::string type()    const override { return "Truck"; }
};

class Ship : public Transport
{
public:
    std::string deliver() const override { return "Delivering by sea in a ship"; }
    std::string type()    const override { return "Ship"; }
};

class Plane : public Transport
{
public:
    std::string deliver() const override { return "Delivering by air in a plane"; }
    std::string type()    const override { return "Plane"; }
};

// ---------- Creator hierarchy --------------------------------------------

class Logistics
{
public:
    virtual ~Logistics() = default;

    // The factory method. Subclasses override this.
    virtual std::unique_ptr<Transport> createTransport() const = 0;

    // Template method that uses the factory product.
    // Note: this method is the same for all logistics providers.
    std::string planDelivery() const
    {
        auto t = createTransport();
        return "[" + t->type() + "] " + t->deliver();
    }
};

class RoadLogistics : public Logistics
{
public:
    std::unique_ptr<Transport> createTransport() const override
    {
        return std::unique_ptr<Transport>(new Truck());
    }
};

class SeaLogistics : public Logistics
{
public:
    std::unique_ptr<Transport> createTransport() const override
    {
        return std::unique_ptr<Transport>(new Ship());
    }
};

class AirLogistics : public Logistics
{
public:
    std::unique_ptr<Transport> createTransport() const override
    {
        return std::unique_ptr<Transport>(new Plane());
    }
};

// ---------- Helper: create logistics by name (optional) ------------------

inline std::unique_ptr<Logistics> makeLogistics(const std::string& kind)
{
    if (kind == "road") return std::unique_ptr<Logistics>(new RoadLogistics());
    if (kind == "sea")  return std::unique_ptr<Logistics>(new SeaLogistics());
    if (kind == "air")  return std::unique_ptr<Logistics>(new AirLogistics());
    throw std::invalid_argument("Unknown logistics kind: " + kind);
}

} // namespace pattern
