#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Prototype Pattern
//
// Intent: Specify the kinds of objects to create using a prototypical instance,
// and create new objects by copying (cloning) this prototype.
//
// Real-world analogy: A cell divides by copying itself rather than building a
// new cell from scratch. A game spawner holds one "template" enemy and clones
// it whenever a new enemy is needed — no expensive construction required.
//
// Key C++ mechanics used:
//  - Virtual `clone()` method returning std::unique_ptr<Base>.
//  - Copy constructor is used internally by the clone implementation.
//  - Prototype Registry: a map of named prototypes for lookup by key.
//  - Deep copy semantics: vectors/strings are copied by value.
//
// When to use:
//  - Object creation is more expensive than copying (e.g. DB round-trip).
//  - You need many objects that differ only slightly from a reference.
//  - Avoid subclassing the creator (prefer cloning over factory hierarchy).
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Shape hierarchy (example domain) ----------------------------

class Shape
{
public:
    explicit Shape(const std::string& color)
        : color_(color)
    {
    }
    virtual ~Shape() = default;

    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual std::string            type() const  = 0;

    const std::string& color() const
    {
        return color_;
    }
    void setColor(const std::string& c)
    {
        color_ = c;
    }

protected:
    std::string color_;
};

class Circle : public Shape
{
public:
    Circle(const std::string& color, double radius)
        : Shape(color)
        , radius_(radius)
    {
    }

    std::unique_ptr<Shape> clone() const override
    {
        return std::unique_ptr<Shape>(new Circle(*this));
    }

    std::string type() const override
    {
        return "Circle";
    }
    double radius() const
    {
        return radius_;
    }
    void setRadius(double r)
    {
        radius_ = r;
    }

private:
    double radius_;
};

class Rectangle : public Shape
{
public:
    Rectangle(const std::string& color, double w, double h)
        : Shape(color)
        , width_(w)
        , height_(h)
    {
    }

    std::unique_ptr<Shape> clone() const override
    {
        return std::unique_ptr<Shape>(new Rectangle(*this));
    }

    std::string type() const override
    {
        return "Rectangle";
    }
    double width() const
    {
        return width_;
    }
    double height() const
    {
        return height_;
    }

private:
    double width_, height_;
};

// ---------- Prototype Registry -------------------------------------------

class ShapeRegistry
{
public:
    void add(const std::string& key, std::unique_ptr<Shape> proto)
    {
        registry_[key] = std::move(proto);
    }

    std::unique_ptr<Shape> get(const std::string& key) const
    {
        auto it = registry_.find(key);
        if (it == registry_.end())
            throw std::out_of_range("Prototype not found: " + key);
        return it->second->clone();
    }

    bool has(const std::string& key) const
    {
        return registry_.find(key) != registry_.end();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Shape>> registry_;
};

} // namespace pattern
