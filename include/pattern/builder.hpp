#pragma once
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Builder Pattern
//
// Intent: Separate the construction of a complex object from its
// representation so that the same construction process can create different
// representations.
//
// Real-world analogy: Building a custom burger. The same "chef" (Director)
// follows the same assembly steps (add bun, add patty, add toppings) but a
// VeggieBuilder assembles a veggie burger while a MeatBuilder assembles a
// classic burger.
//
// Key C++ mechanics used:
//  - Step-by-step builder interface (pure virtual methods).
//  - Director class that knows the order of steps but not the details.
//  - Fluent builder variant (method chaining) shown as an alternative.
//  - Product is fully assembled and retrieved in one call (getResult).
//
// When to use:
//  - Object has many optional/configurable parts (HTTP request, SQL query,
//    game character, configuration object).
//  - You want to avoid a "telescoping constructor" anti-pattern.
//  - The same build steps should produce different representations.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Product -------------------------------------------------------

struct Burger
{
    std::string              bun;
    std::string              patty;
    std::vector<std::string> toppings;
    bool                     toasted = false;

    std::string describe() const
    {
        std::ostringstream ss;
        ss << "[" << bun << "] with " << patty << " patty";
        if (toasted)
            ss << " (toasted)";
        for (const auto& t : toppings)
            ss << ", " << t;
        return ss.str();
    }
};

// ---------- Abstract Builder ----------------------------------------------

class BurgerBuilder
{
public:
    virtual ~BurgerBuilder()                              = default;
    virtual void   setBun(const std::string& bun)         = 0;
    virtual void   setPatty(const std::string& patty)     = 0;
    virtual void   addTopping(const std::string& topping) = 0;
    virtual void   setToasted(bool toasted)               = 0;
    virtual Burger getResult()                            = 0;
};

// ---------- Concrete Builders ---------------------------------------------

class MeatBurgerBuilder : public BurgerBuilder
{
public:
    MeatBurgerBuilder()
    {
        reset();
    }

    void setBun(const std::string& bun) override
    {
        burger_.bun = bun;
    }
    void setPatty(const std::string& patty) override
    {
        burger_.patty = patty;
    }
    void addTopping(const std::string& topping) override
    {
        burger_.toppings.push_back(topping);
    }
    void setToasted(bool toasted) override
    {
        burger_.toasted = toasted;
    }

    Burger getResult() override
    {
        Burger result = burger_;
        reset();
        return result;
    }

private:
    void reset()
    {
        burger_       = Burger();
        burger_.bun   = "sesame";
        burger_.patty = "beef";
    }
    Burger burger_;
};

class VeggieBurgerBuilder : public BurgerBuilder
{
public:
    VeggieBurgerBuilder()
    {
        reset();
    }

    void setBun(const std::string& bun) override
    {
        burger_.bun = bun;
    }
    void setPatty(const std::string& patty) override
    {
        burger_.patty = patty;
    }
    void addTopping(const std::string& topping) override
    {
        burger_.toppings.push_back(topping);
    }
    void setToasted(bool toasted) override
    {
        burger_.toasted = toasted;
    }

    Burger getResult() override
    {
        Burger result = burger_;
        reset();
        return result;
    }

private:
    void reset()
    {
        burger_       = Burger();
        burger_.bun   = "whole-wheat";
        burger_.patty = "black-bean";
    }
    Burger burger_;
};

// ---------- Director (knows the recipes) ---------------------------------

class BurgerDirector
{
public:
    explicit BurgerDirector(BurgerBuilder* builder)
        : builder_(builder)
    {
    }

    void buildClassic()
    {
        builder_->setToasted(true);
        builder_->addTopping("lettuce");
        builder_->addTopping("tomato");
        builder_->addTopping("cheese");
    }

    void buildDeluxe()
    {
        builder_->setToasted(true);
        builder_->addTopping("lettuce");
        builder_->addTopping("tomato");
        builder_->addTopping("cheese");
        builder_->addTopping("pickles");
        builder_->addTopping("onion");
        builder_->addTopping("special sauce");
    }

private:
    BurgerBuilder* builder_;
};

// ---------- Fluent Builder (alternative style) ---------------------------

class FluentBurger
{
public:
    FluentBurger& withBun(const std::string& bun)
    {
        burger_.bun = bun;
        return *this;
    }
    FluentBurger& withPatty(const std::string& patty)
    {
        burger_.patty = patty;
        return *this;
    }
    FluentBurger& withTopping(const std::string& topping)
    {
        burger_.toppings.push_back(topping);
        return *this;
    }
    FluentBurger& toasted(bool v = true)
    {
        burger_.toasted = v;
        return *this;
    }
    Burger build()
    {
        return burger_;
    }

private:
    Burger burger_;
};

} // namespace pattern
