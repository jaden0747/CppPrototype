#pragma once
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Decorator Pattern
//
// Intent: Attach additional responsibilities to an object dynamically.
// Decorators provide a flexible alternative to subclassing for extending
// functionality.
//
// Real-world analogy: A coffee order. Start with a plain Espresso, then wrap
// it in a MilkDecorator (+$0.25), then a CaramelDecorator (+$0.50). Each
// wrapper adds its cost/description without changing the base class or any
// other decorators.
//
// Key C++ mechanics used:
//  - All decorators inherit from the same Component interface as the Concrete
//    Component — so they're interchangeable.
//  - BaseDecorator holds a std::unique_ptr<Coffee> to the wrapped component.
//  - Concrete decorators call wrappee_->cost() / description() and add their
//    own contribution.
//  - Stacking works because each decorator is itself a Coffee.
//
// When to use:
//  - Add behaviour to individual objects without affecting others.
//  - Behaviour can be added/removed at runtime.
//  - Extension by subclassing is impractical (combinatorial explosion).
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Component interface ------------------------------------------

class Coffee
{
public:
    virtual ~Coffee()                       = default;
    virtual double      cost() const        = 0;
    virtual std::string description() const = 0;
};

// ---------- Concrete Component -------------------------------------------

class Espresso : public Coffee
{
public:
    double cost() const override
    {
        return 1.00;
    }
    std::string description() const override
    {
        return "Espresso";
    }
};

class SimpleCoffee : public Coffee
{
public:
    double cost() const override
    {
        return 0.50;
    }
    std::string description() const override
    {
        return "Simple coffee";
    }
};

// ---------- Base Decorator -----------------------------------------------

class CoffeeDecorator : public Coffee
{
public:
    explicit CoffeeDecorator(std::unique_ptr<Coffee> wrappee)
        : wrappee_(std::move(wrappee))
    {
    }

    double cost() const override
    {
        return wrappee_->cost();
    }
    std::string description() const override
    {
        return wrappee_->description();
    }

protected:
    std::unique_ptr<Coffee> wrappee_;
};

// ---------- Concrete Decorators ------------------------------------------

class MilkDecorator : public CoffeeDecorator
{
public:
    explicit MilkDecorator(std::unique_ptr<Coffee> wrappee)
        : CoffeeDecorator(std::move(wrappee))
    {
    }

    double cost() const override
    {
        return wrappee_->cost() + 0.25;
    }
    std::string description() const override
    {
        return wrappee_->description() + ", Milk";
    }
};

class CaramelDecorator : public CoffeeDecorator
{
public:
    explicit CaramelDecorator(std::unique_ptr<Coffee> wrappee)
        : CoffeeDecorator(std::move(wrappee))
    {
    }

    double cost() const override
    {
        return wrappee_->cost() + 0.50;
    }
    std::string description() const override
    {
        return wrappee_->description() + ", Caramel";
    }
};

class WhipDecorator : public CoffeeDecorator
{
public:
    explicit WhipDecorator(std::unique_ptr<Coffee> wrappee)
        : CoffeeDecorator(std::move(wrappee))
    {
    }

    double cost() const override
    {
        return wrappee_->cost() + 0.30;
    }
    std::string description() const override
    {
        return wrappee_->description() + ", Whip";
    }
};

} // namespace pattern
