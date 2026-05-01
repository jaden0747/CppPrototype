#include <gtest/gtest.h>
#include "pattern/command.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// TextEditor receiver
// ---------------------------------------------------------------------------
TEST(Command, AppendText)
{
    TextEditor editor;
    editor.append("Hello");
    EXPECT_EQ("Hello", editor.content());
}

TEST(Command, DeleteLast)
{
    TextEditor editor;
    editor.append("Hello");
    editor.deleteLast(2);
    EXPECT_EQ("Hel", editor.content());
}

TEST(Command, DeleteMoreThanContentClearsAll)
{
    TextEditor editor;
    editor.append("Hi");
    editor.deleteLast(100);
    EXPECT_TRUE(editor.content().empty());
}

// ---------------------------------------------------------------------------
// AppendCommand execute / undo
// ---------------------------------------------------------------------------
TEST(Command, AppendCommandExecute)
{
    TextEditor editor;
    AppendCommand cmd(editor, " World");
    editor.append("Hello");
    cmd.execute();
    EXPECT_EQ("Hello World", editor.content());
}

TEST(Command, AppendCommandUndo)
{
    TextEditor editor;
    editor.append("Hello");
    AppendCommand cmd(editor, " World");
    cmd.execute();
    cmd.undo();
    EXPECT_EQ("Hello", editor.content());
}

// ---------------------------------------------------------------------------
// DeleteLastCommand execute / undo
// ---------------------------------------------------------------------------
TEST(Command, DeleteLastCommandExecute)
{
    TextEditor editor;
    editor.append("Hello");
    DeleteLastCommand cmd(editor, 3);
    cmd.execute();
    EXPECT_EQ("He", editor.content());
}

TEST(Command, DeleteLastCommandUndo)
{
    TextEditor editor;
    editor.append("Hello");
    DeleteLastCommand cmd(editor, 3);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ("Hello", editor.content());
}

// ---------------------------------------------------------------------------
// CommandHistory: execute / undo / redo
// ---------------------------------------------------------------------------
TEST(Command, HistoryExecuteUpdatesContent)
{
    TextEditor editor;
    CommandHistory history;
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "Hi")));
    EXPECT_EQ("Hi", editor.content());
}

TEST(Command, HistoryUndoRevertsContent)
{
    TextEditor editor;
    CommandHistory history;
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "Hi")));
    history.undo();
    EXPECT_TRUE(editor.content().empty());
}

TEST(Command, HistoryRedoReappliesContent)
{
    TextEditor editor;
    CommandHistory history;
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "Hi")));
    history.undo();
    history.redo();
    EXPECT_EQ("Hi", editor.content());
}

TEST(Command, HistoryUndoOnEmptyReturnsFalse)
{
    CommandHistory history;
    EXPECT_FALSE(history.undo());
}

TEST(Command, HistoryNewActionClearsRedo)
{
    TextEditor editor;
    CommandHistory history;
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "A")));
    history.undo();
    EXPECT_TRUE(history.canRedo());
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "B")));
    EXPECT_FALSE(history.canRedo());
}

TEST(Command, MultipleUndoRedo)
{
    TextEditor editor;
    CommandHistory history;
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, "Hello")));
    history.execute(std::unique_ptr<Command>(new AppendCommand(editor, " World")));
    history.undo();  // removes " World"
    EXPECT_EQ("Hello", editor.content());
    history.redo();  // re-adds " World"
    EXPECT_EQ("Hello World", editor.content());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
