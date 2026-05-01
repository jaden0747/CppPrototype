#include <gtest/gtest.h>
#include "pattern/interpreter.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Terminal expressions
// ---------------------------------------------------------------------------
TEST(Interpreter, LiteralTrue)
{
    Context ctx;
    EXPECT_TRUE(Literal(true).interpret(ctx));
}

TEST(Interpreter, LiteralFalse)
{
    Context ctx;
    EXPECT_FALSE(Literal(false).interpret(ctx));
}

TEST(Interpreter, VariableLookupTrue)
{
    Context ctx{{"x", true}};
    EXPECT_TRUE(Variable("x").interpret(ctx));
}

TEST(Interpreter, VariableLookupFalse)
{
    Context ctx{{"x", false}};
    EXPECT_FALSE(Variable("x").interpret(ctx));
}

TEST(Interpreter, UndefinedVariableThrows)
{
    Context ctx;
    EXPECT_THROW(Variable("missing").interpret(ctx), std::runtime_error);
}

// ---------------------------------------------------------------------------
// NOT
// ---------------------------------------------------------------------------
TEST(Interpreter, NotTrue)
{
    Context ctx;
    NotExpr n(std::unique_ptr<BoolExpr>(new Literal(true)));
    EXPECT_FALSE(n.interpret(ctx));
}

TEST(Interpreter, NotFalse)
{
    Context ctx;
    NotExpr n(std::unique_ptr<BoolExpr>(new Literal(false)));
    EXPECT_TRUE(n.interpret(ctx));
}

// ---------------------------------------------------------------------------
// AND
// ---------------------------------------------------------------------------
TEST(Interpreter, AndTrueTrue)
{
    Context ctx;
    AndExpr a(std::unique_ptr<BoolExpr>(new Literal(true)),
              std::unique_ptr<BoolExpr>(new Literal(true)));
    EXPECT_TRUE(a.interpret(ctx));
}

TEST(Interpreter, AndTrueFalse)
{
    Context ctx;
    AndExpr a(std::unique_ptr<BoolExpr>(new Literal(true)),
              std::unique_ptr<BoolExpr>(new Literal(false)));
    EXPECT_FALSE(a.interpret(ctx));
}

// ---------------------------------------------------------------------------
// OR
// ---------------------------------------------------------------------------
TEST(Interpreter, OrFalseTrue)
{
    Context ctx;
    OrExpr o(std::unique_ptr<BoolExpr>(new Literal(false)),
             std::unique_ptr<BoolExpr>(new Literal(true)));
    EXPECT_TRUE(o.interpret(ctx));
}

TEST(Interpreter, OrFalseFalse)
{
    Context ctx;
    OrExpr o(std::unique_ptr<BoolExpr>(new Literal(false)),
             std::unique_ptr<BoolExpr>(new Literal(false)));
    EXPECT_FALSE(o.interpret(ctx));
}

// ---------------------------------------------------------------------------
// Composite expression: (x AND y) OR (NOT z)
// ---------------------------------------------------------------------------
TEST(Interpreter, CompoundExpression)
{
    // (x AND y) OR (NOT z)
    Context ctx{{"x", true}, {"y", false}, {"z", false}};

    auto xVar = std::unique_ptr<BoolExpr>(new Variable("x"));
    auto yVar = std::unique_ptr<BoolExpr>(new Variable("y"));
    auto zVar = std::unique_ptr<BoolExpr>(new Variable("z"));

    auto xAndY = std::unique_ptr<BoolExpr>(new AndExpr(std::move(xVar), std::move(yVar)));
    auto notZ  = std::unique_ptr<BoolExpr>(new NotExpr(std::move(zVar)));
    OrExpr expr(std::move(xAndY), std::move(notZ));

    // x=T, y=F: xAndY=F; z=F, notZ=T → F OR T = true
    EXPECT_TRUE(expr.interpret(ctx));
}

TEST(Interpreter, CompoundExpressionAllFalse)
{
    Context ctx{{"x", false}, {"y", false}, {"z", true}};

    auto xAndY = std::unique_ptr<BoolExpr>(
        new AndExpr(std::unique_ptr<BoolExpr>(new Variable("x")),
                    std::unique_ptr<BoolExpr>(new Variable("y"))));
    auto notZ = std::unique_ptr<BoolExpr>(
        new NotExpr(std::unique_ptr<BoolExpr>(new Variable("z"))));
    OrExpr expr(std::move(xAndY), std::move(notZ));

    // F AND F = F; NOT T = F → F OR F = false
    EXPECT_FALSE(expr.interpret(ctx));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
