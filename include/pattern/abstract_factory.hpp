#pragma once
#include <memory>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Abstract Factory Pattern
//
// Intent: Provide an interface for creating families of related or dependent
// objects without specifying their concrete classes.
//
// Real-world analogy: A furniture shop sells Modern and Victorian style sets.
// Each set has a Chair, Sofa, and Coffee Table. You pick a style (factory)
// and get a consistent family of furniture — you never mix a Modern chair
// with a Victorian sofa.
//
// Key C++ mechanics used:
//  - Abstract base class (interface) for each product family member.
//  - Abstract factory interface with one creation method per product.
//  - Concrete factories implement all creation methods for one style.
//  - std::unique_ptr for ownership — client code never calls new directly.
//
// When to use:
//  - System must be independent of how its products are created.
//  - System should work with multiple families of products.
//  - You want to enforce constraints across related product types
//    (e.g. all products must belong to the same theme/platform).
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Product interfaces -------------------------------------------

class Chair
{
public:
    virtual ~Chair()                  = default;
    virtual std::string sitOn() const = 0;
    virtual std::string style() const = 0;
};

class Sofa
{
public:
    virtual ~Sofa()                   = default;
    virtual std::string lieOn() const = 0;
    virtual std::string style() const = 0;
};

class CoffeeTable
{
public:
    virtual ~CoffeeTable()            = default;
    virtual std::string putOn() const = 0;
    virtual std::string style() const = 0;
};

// ---------- Modern family ------------------------------------------------

class ModernChair : public Chair
{
public:
    std::string sitOn() const override
    {
        return "Sitting on a sleek modern chair";
    }
    std::string style() const override
    {
        return "Modern";
    }
};

class ModernSofa : public Sofa
{
public:
    std::string lieOn() const override
    {
        return "Lying on a minimalist modern sofa";
    }
    std::string style() const override
    {
        return "Modern";
    }
};

class ModernCoffeeTable : public CoffeeTable
{
public:
    std::string putOn() const override
    {
        return "Placing items on a glass modern table";
    }
    std::string style() const override
    {
        return "Modern";
    }
};

// ---------- Victorian family ---------------------------------------------

class VictorianChair : public Chair
{
public:
    std::string sitOn() const override
    {
        return "Sitting on an ornate Victorian chair";
    }
    std::string style() const override
    {
        return "Victorian";
    }
};

class VictorianSofa : public Sofa
{
public:
    std::string lieOn() const override
    {
        return "Lying on a velvet Victorian sofa";
    }
    std::string style() const override
    {
        return "Victorian";
    }
};

class VictorianCoffeeTable : public CoffeeTable
{
public:
    std::string putOn() const override
    {
        return "Placing items on a carved Victorian table";
    }
    std::string style() const override
    {
        return "Victorian";
    }
};

// ---------- Abstract factory interface -----------------------------------

class FurnitureFactory
{
public:
    virtual ~FurnitureFactory()                                    = default;
    virtual std::unique_ptr<Chair>       createChair() const       = 0;
    virtual std::unique_ptr<Sofa>        createSofa() const        = 0;
    virtual std::unique_ptr<CoffeeTable> createCoffeeTable() const = 0;
};

// ---------- Concrete factories -------------------------------------------

class ModernFurnitureFactory : public FurnitureFactory
{
public:
    std::unique_ptr<Chair> createChair() const override
    {
        return std::unique_ptr<Chair>(new ModernChair());
    }
    std::unique_ptr<Sofa> createSofa() const override
    {
        return std::unique_ptr<Sofa>(new ModernSofa());
    }
    std::unique_ptr<CoffeeTable> createCoffeeTable() const override
    {
        return std::unique_ptr<CoffeeTable>(new ModernCoffeeTable());
    }
};

class VictorianFurnitureFactory : public FurnitureFactory
{
public:
    std::unique_ptr<Chair> createChair() const override
    {
        return std::unique_ptr<Chair>(new VictorianChair());
    }
    std::unique_ptr<Sofa> createSofa() const override
    {
        return std::unique_ptr<Sofa>(new VictorianSofa());
    }
    std::unique_ptr<CoffeeTable> createCoffeeTable() const override
    {
        return std::unique_ptr<CoffeeTable>(new VictorianCoffeeTable());
    }
};

// ---------- Helper -------------------------------------------------------

inline std::unique_ptr<FurnitureFactory> makeFurnitureFactory(const std::string& style)
{
    if (style == "modern")
        return std::unique_ptr<FurnitureFactory>(new ModernFurnitureFactory());
    if (style == "victorian")
        return std::unique_ptr<FurnitureFactory>(new VictorianFurnitureFactory());
    throw std::invalid_argument("Unknown furniture style: " + style);
}

} // namespace pattern
