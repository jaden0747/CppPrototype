#pragma once
#include <cmath>
#include <sstream>
#include <string>

// ---------------------------------------------------------------------------
// Adapter Pattern
//
// Intent: Convert the interface of a class into another interface that clients
// expect. Adapter lets classes work together that couldn't otherwise because of
// incompatible interfaces.
//
// Real-world analogy: A European power outlet provides 220V round pins.
// An American traveller has a 110V flat-pin plug. A travel adapter sits between
// them — it looks like a round-pin socket to the wall and a flat-pin socket to
// the device.
//
// Two forms shown:
//   1. Object Adapter (composition) — wraps an instance; works with subclasses.
//   2. Class Adapter (multiple inheritance) — C++ supports this; less flexible.
//
// Key C++ mechanics used:
//  - Composition (object adapter) vs. multiple inheritance (class adapter).
//  - Adapter implements the target interface and delegates to the adaptee.
//  - No modification to existing (legacy) code required.
//
// When to use:
//  - You want to use an existing class but its interface doesn't match.
//  - You can't modify the source class (third-party, legacy).
//  - You need a reusable class that cooperates with unrelated classes.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Target interface (what the client expects) -------------------

class RoundHole
{
public:
    explicit RoundHole(double radius)
        : radius_(radius)
    {
    }

    bool fits(double peg_radius) const
    {
        return peg_radius <= radius_;
    }
    double radius() const
    {
        return radius_;
    }

private:
    double radius_;
};

class RoundPeg
{
public:
    explicit RoundPeg(double radius)
        : radius_(radius)
    {
    }
    virtual ~RoundPeg() = default;
    virtual double radius() const
    {
        return radius_;
    }

private:
    double radius_;
};

// ---------- Incompatible legacy class (Adaptee) --------------------------

class SquarePeg
{
public:
    explicit SquarePeg(double width)
        : width_(width)
    {
    }
    double width() const
    {
        return width_;
    }

    std::string describe() const
    {
        std::ostringstream ss;
        ss << "SquarePeg(width=" << width_ << ")";
        return ss.str();
    }

private:
    double width_;
};

// ---------- Object Adapter -----------------------------------------------
// Wraps a SquarePeg and exposes the RoundPeg interface.
// Math: the radius of the smallest circle enclosing a square of side `w`
// is w * sqrt(2) / 2.

class SquarePegAdapter : public RoundPeg
{
public:
    explicit SquarePegAdapter(const SquarePeg& peg)
        : RoundPeg(0.0)
        , // base radius unused
        adaptee_(peg)
    {
    }

    double radius() const override
    {
        return adaptee_.width() * 1.41421356 / 2.0;
    }

private:
    const SquarePeg& adaptee_;
};

// ---------- Class Adapter (MI variant) ------------------------------------
// Inherits publicly from RoundPeg (target) and SquarePeg (adaptee).

class SquarePegClassAdapter : public RoundPeg, public SquarePeg
{
public:
    explicit SquarePegClassAdapter(double width)
        : RoundPeg(0.0)
        , SquarePeg(width)
    {
    }

    double radius() const override
    {
        return SquarePeg::width() * 1.41421356 / 2.0;
    }
};

} // namespace pattern
