#include <gtest/gtest.h>
#include "pattern/memento.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------
TEST(Memento, TypeAppendsText)
{
    Editor e;
    e.type("Hello");
    EXPECT_EQ("Hello", e.content());
}

TEST(Memento, SaveCapturesCurrentContent)
{
    Editor e;
    e.type("Hello");
    auto m = e.save();
    EXPECT_EQ("Hello", m.content());
}

TEST(Memento, RestoreReverts)
{
    Editor e;
    e.type("Hello");
    auto m = e.save();
    e.type(" World");
    e.restore(m);
    EXPECT_EQ("Hello", e.content());
}

TEST(Memento, MementoLabel)
{
    Editor e;
    auto m = e.save("initial");
    EXPECT_EQ("initial", m.label());
}

// ---------------------------------------------------------------------------
// History (Caretaker)
// ---------------------------------------------------------------------------
TEST(Memento, HistoryIsEmptyInitially)
{
    History h;
    EXPECT_TRUE(h.empty());
}

TEST(Memento, HistoryPushIncreasesSize)
{
    Editor e;
    History h;
    h.push(e.save());
    EXPECT_EQ(1u, h.size());
}

TEST(Memento, HistoryPopReturnsLastSnapshot)
{
    Editor e;
    History h;
    e.type("A");
    h.push(e.save());
    e.type("B");
    h.push(e.save());
    auto m = h.pop();
    EXPECT_EQ("AB", m.content());
}

TEST(Memento, HistoryPopEmptyThrows)
{
    History h;
    EXPECT_THROW(h.pop(), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Undo workflow
// ---------------------------------------------------------------------------
TEST(Memento, UndoRestoresPreviousState)
{
    Editor e;
    History h;

    e.type("Hello");
    h.push(e.save());

    e.type(" World");
    h.push(e.save());

    // Undo " World"
    h.pop();             // discard current
    e.restore(h.top());  // restore previous
    EXPECT_EQ("Hello", e.content());
}

TEST(Memento, MultipleUndos)
{
    Editor e;
    History h;

    h.push(e.save("empty"));
    e.type("A");
    h.push(e.save("A"));
    e.type("B");
    h.push(e.save("AB"));

    h.pop();             // discard AB
    e.restore(h.top()); // restore A
    EXPECT_EQ("A", e.content());

    h.pop();              // discard A
    e.restore(h.top()); // restore empty
    EXPECT_TRUE(e.content().empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
