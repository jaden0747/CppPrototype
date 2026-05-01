#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------------------------
// Interpreter Pattern
//
// Intent: Given a language, define a representation for its grammar along with
// an interpreter that uses the representation to interpret sentences.
//
// Example domain: A simple Boolean expression evaluator.
//   Grammar:
//     expr   ::= var | 'true' | 'false'
//              | '!' expr
//              | '(' expr 'AND' expr ')'
//              | '(' expr 'OR'  expr ')'
//
//   Context: a symbol table mapping variable names → bool values.
//
// Key C++ mechanics used:
//  - Abstract Expression with virtual `interpret(context)`.
//  - Terminal Expressions (literals, variables) are leaves.
//  - Non-Terminal Expressions (AND, OR, NOT) compose sub-expressions.
//  - Context is passed by reference to all interpret() calls.
//
// When to use:
//  - A grammar is simple and performance is not critical.
//  - You need to interpret sentences in a language (SQL, regex, math
//    expressions, configuration mini-languages).
// ---------------------------------------------------------------------------

namespace pattern {

using Context = std::unordered_map<std::string, bool>;

// ---------- Abstract Expression ------------------------------------------

class BoolExpr
{
public:
    virtual ~BoolExpr() = default;
    virtual bool interpret(const Context& ctx) const = 0;
};

// ---------- Terminal Expressions -----------------------------------------

class Literal : public BoolExpr
{
public:
    explicit Literal(bool value) : value_(value) {}
    bool interpret(const Context&) const override { return value_; }
private:
    bool value_;
};

class Variable : public BoolExpr
{
public:
    explicit Variable(const std::string& name) : name_(name) {}
    bool interpret(const Context& ctx) const override
    {
        auto it = ctx.find(name_);
        if (it == ctx.end())
            throw std::runtime_error("Undefined variable: " + name_);
        return it->second;
    }
private:
    std::string name_;
};

// ---------- Non-Terminal Expressions -------------------------------------

class NotExpr : public BoolExpr
{
public:
    explicit NotExpr(std::unique_ptr<BoolExpr> operand)
        : operand_(std::move(operand)) {}
    bool interpret(const Context& ctx) const override
    {
        return !operand_->interpret(ctx);
    }
private:
    std::unique_ptr<BoolExpr> operand_;
};

class AndExpr : public BoolExpr
{
public:
    AndExpr(std::unique_ptr<BoolExpr> left, std::unique_ptr<BoolExpr> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    bool interpret(const Context& ctx) const override
    {
        return left_->interpret(ctx) && right_->interpret(ctx);
    }
private:
    std::unique_ptr<BoolExpr> left_;
    std::unique_ptr<BoolExpr> right_;
};

class OrExpr : public BoolExpr
{
public:
    OrExpr(std::unique_ptr<BoolExpr> left, std::unique_ptr<BoolExpr> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    bool interpret(const Context& ctx) const override
    {
        return left_->interpret(ctx) || right_->interpret(ctx);
    }
private:
    std::unique_ptr<BoolExpr> left_;
    std::unique_ptr<BoolExpr> right_;
};

} // namespace pattern
