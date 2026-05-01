#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cmath>

// ---------------------------------------------------------------------------
// Visitor Pattern
//
// Intent: Represent an operation to be performed on elements of an object
// structure. Visitor lets you define a new operation without changing the
// classes of the elements on which it operates.
//
// Real-world analogy: An accountant who visits every department in a company.
// Each department type (R&D, Sales, Marketing) is visited and processed
// differently, but neither the accountant nor the departments need to know
// about all other department types.
//
// Key C++ mechanics used:
//  - Double dispatch: accept(Visitor&) on each element calls
//    visitor.visit(*this), binding both the element type and the visitor
//    type at runtime.
//  - Separate IVisitor interface per "operation family" — adding a new
//    operation is just a new class that implements IVisitor, with no changes
//    to the element hierarchy.
//
// When to use:
//  - You need to perform many distinct and unrelated operations on an object
//    structure, and you don't want to pollute their classes with these ops.
//  - The object structure classes rarely change, but you often add new
//    operations on the structure.
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Forward declarations -----------------------------------------

class Circle;
class Rectangle;
class Triangle;

// ---------- Visitor interface -------------------------------------------

class IShapeVisitor
{
public:
    virtual ~IShapeVisitor() = default;
    virtual void visit(Circle&    c) = 0;
    virtual void visit(Rectangle& r) = 0;
    virtual void visit(Triangle&  t) = 0;
};

// ---------- Element interface -------------------------------------------

class Shape
{
public:
    virtual ~Shape() = default;
    virtual void accept(IShapeVisitor& v) = 0;
    virtual std::string name() const = 0;
};

// ---------- Concrete elements --------------------------------------------

class Circle : public Shape
{
public:
    explicit Circle(double radius) : radius_(radius) {}
    void accept(IShapeVisitor& v) override { v.visit(*this); }
    std::string name() const override { return "Circle"; }
    double radius() const { return radius_; }
private:
    double radius_;
};

class Rectangle : public Shape
{
public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    void accept(IShapeVisitor& v) override { v.visit(*this); }
    std::string name() const override { return "Rectangle"; }
    double width()  const { return width_; }
    double height() const { return height_; }
private:
    double width_, height_;
};

class Triangle : public Shape
{
public:
    Triangle(double a, double b, double c) : a_(a), b_(b), c_(c) {}
    void accept(IShapeVisitor& v) override { v.visit(*this); }
    std::string name() const override { return "Triangle"; }
    double a() const { return a_; }
    double b() const { return b_; }
    double c() const { return c_; }
private:
    double a_, b_, c_;
};

// ---------- Concrete Visitors -------------------------------------------

class AreaVisitor : public IShapeVisitor
{
public:
    double total() const { return total_; }
    void reset()         { total_ = 0.0; }

    void visit(Circle& c) override
    {
        total_ += 3.14159265358979 * c.radius() * c.radius();
    }
    void visit(Rectangle& r) override
    {
        total_ += r.width() * r.height();
    }
    void visit(Triangle& t) override
    {
        // Heron's formula
        double s = (t.a() + t.b() + t.c()) / 2.0;
        total_ += std::sqrt(s * (s - t.a()) * (s - t.b()) * (s - t.c()));
    }

private:
    double total_{0.0};
};

class PerimeterVisitor : public IShapeVisitor
{
public:
    double total() const { return total_; }
    void reset()         { total_ = 0.0; }

    void visit(Circle& c) override
    {
        total_ += 2.0 * 3.14159265358979 * c.radius();
    }
    void visit(Rectangle& r) override
    {
        total_ += 2.0 * (r.width() + r.height());
    }
    void visit(Triangle& t) override
    {
        total_ += t.a() + t.b() + t.c();
    }

private:
    double total_{0.0};
};

class NameCollectorVisitor : public IShapeVisitor
{
public:
    const std::vector<std::string>& names() const { return names_; }

    void visit(Circle&    c) override { names_.push_back(c.name()); }
    void visit(Rectangle& r) override { names_.push_back(r.name()); }
    void visit(Triangle&  t) override { names_.push_back(t.name()); }

private:
    std::vector<std::string> names_;
};

} // namespace pattern
