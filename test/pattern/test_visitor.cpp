#include <gtest/gtest.h>
#include <cmath>
#include "pattern/visitor.hpp"

using namespace pattern;

namespace {
constexpr double PI = 3.14159265358979;
constexpr double EPS = 1e-6;
} // namespace

// ---------------------------------------------------------------------------
// AreaVisitor
// ---------------------------------------------------------------------------
TEST(Visitor, CircleArea)
{
    Circle c(5.0);
    AreaVisitor av;
    c.accept(av);
    EXPECT_NEAR(PI * 25.0, av.total(), EPS);
}

TEST(Visitor, RectangleArea)
{
    Rectangle r(4.0, 3.0);
    AreaVisitor av;
    r.accept(av);
    EXPECT_NEAR(12.0, av.total(), EPS);
}

TEST(Visitor, TriangleArea3_4_5)
{
    Triangle t(3.0, 4.0, 5.0);
    AreaVisitor av;
    t.accept(av);
    EXPECT_NEAR(6.0, av.total(), EPS);  // right triangle
}

TEST(Visitor, AreaVisitorAccumulates)
{
    AreaVisitor av;
    Rectangle r(2.0, 3.0);
    Circle    c(1.0);
    r.accept(av);
    c.accept(av);
    EXPECT_NEAR(6.0 + PI, av.total(), EPS);
}

TEST(Visitor, AreaVisitorReset)
{
    AreaVisitor av;
    Rectangle r(2.0, 3.0);
    r.accept(av);
    av.reset();
    EXPECT_NEAR(0.0, av.total(), EPS);
}

// ---------------------------------------------------------------------------
// PerimeterVisitor
// ---------------------------------------------------------------------------
TEST(Visitor, CirclePerimeter)
{
    Circle c(5.0);
    PerimeterVisitor pv;
    c.accept(pv);
    EXPECT_NEAR(2.0 * PI * 5.0, pv.total(), EPS);
}

TEST(Visitor, RectanglePerimeter)
{
    Rectangle r(4.0, 3.0);
    PerimeterVisitor pv;
    r.accept(pv);
    EXPECT_NEAR(14.0, pv.total(), EPS);
}

TEST(Visitor, TrianglePerimeter)
{
    Triangle t(3.0, 4.0, 5.0);
    PerimeterVisitor pv;
    t.accept(pv);
    EXPECT_NEAR(12.0, pv.total(), EPS);
}

// ---------------------------------------------------------------------------
// NameCollectorVisitor
// ---------------------------------------------------------------------------
TEST(Visitor, NameCollectorOrder)
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::unique_ptr<Shape>(new Circle(1.0)));
    shapes.push_back(std::unique_ptr<Shape>(new Rectangle(2.0, 3.0)));
    shapes.push_back(std::unique_ptr<Shape>(new Triangle(3.0, 4.0, 5.0)));

    NameCollectorVisitor nc;
    for (auto& s : shapes) s->accept(nc);

    ASSERT_EQ(3u, nc.names().size());
    EXPECT_EQ("Circle",    nc.names()[0]);
    EXPECT_EQ("Rectangle", nc.names()[1]);
    EXPECT_EQ("Triangle",  nc.names()[2]);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
