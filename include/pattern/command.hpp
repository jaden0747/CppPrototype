#pragma once
#include <memory>
#include <string>
#include <vector>
#include <stack>

// ---------------------------------------------------------------------------
// Command Pattern
//
// Intent: Encapsulate a request as an object, thereby allowing parameterisation
// of clients with different requests, queueing, logging, and undo/redo.
//
// Real-world analogy: A text editor. Each user action (type text, delete,
// bold) is wrapped in a Command object. The editor keeps a history stack —
// undo pops and reverts, redo re-executes.
//
// Key C++ mechanics used:
//  - Command interface with execute() and undo().
//  - Receiver (TextEditor) knows how to do the actual work.
//  - Invoker (CommandHistory) manages execute/undo/redo stacks.
//  - std::stack<std::unique_ptr<Command>> for LIFO history.
//
// When to use:
//  - Parameterise objects with operations.
//  - Queue operations, schedule their execution, or execute them remotely.
//  - Implement reversible operations (undo/redo).
//  - Implement transactional behaviour (commit/rollback).
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Receiver -----------------------------------------------------

class TextEditor
{
public:
    void append(const std::string& text) { content_ += text; }
    void deleteLast(std::size_t n)
    {
        if (n >= content_.size()) content_.clear();
        else content_.erase(content_.size() - n, n);
    }
    const std::string& content() const { return content_; }

private:
    std::string content_;
};

// ---------- Command interface --------------------------------------------

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
    virtual std::string name() const = 0;
};

// ---------- Concrete Commands --------------------------------------------

class AppendCommand : public Command
{
public:
    AppendCommand(TextEditor& editor, const std::string& text)
        : editor_(editor), text_(text) {}

    void execute() override { editor_.append(text_); }
    void undo()    override { editor_.deleteLast(text_.size()); }
    std::string name() const override { return "Append(" + text_ + ")"; }

private:
    TextEditor& editor_;
    std::string text_;
};

class DeleteLastCommand : public Command
{
public:
    DeleteLastCommand(TextEditor& editor, std::size_t n)
        : editor_(editor), n_(n), deleted_("") {}

    void execute() override
    {
        const std::string& c = editor_.content();
        std::size_t actual = (n_ <= c.size()) ? n_ : c.size();
        deleted_ = c.substr(c.size() - actual, actual);
        editor_.deleteLast(n_);
    }

    void undo() override { editor_.append(deleted_); }
    std::string name() const override { return "DeleteLast(" + std::to_string(n_) + ")"; }

private:
    TextEditor& editor_;
    std::size_t n_;
    std::string deleted_;
};

// ---------- Invoker (history with undo/redo) -----------------------------

class CommandHistory
{
public:
    void execute(std::unique_ptr<Command> cmd)
    {
        cmd->execute();
        history_.push(std::move(cmd));
        // New action clears redo stack
        while (!redo_.empty()) redo_.pop();
    }

    bool undo()
    {
        if (history_.empty()) return false;
        history_.top()->undo();
        redo_.push(std::move(history_.top()));
        history_.pop();
        return true;
    }

    bool redo()
    {
        if (redo_.empty()) return false;
        redo_.top()->execute();
        history_.push(std::move(redo_.top()));
        redo_.pop();
        return true;
    }

    bool canUndo() const { return !history_.empty(); }
    bool canRedo() const { return !redo_.empty(); }

    std::size_t historySize() const { return history_.size(); }

private:
    std::stack<std::unique_ptr<Command>> history_;
    std::stack<std::unique_ptr<Command>> redo_;
};

} // namespace pattern
