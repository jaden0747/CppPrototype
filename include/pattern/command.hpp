#pragma once
#include <memory>
#include <stack>
#include <string>
#include <vector>

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

namespace pattern
{

// ---------- Receiver -----------------------------------------------------

class TextEditor
{
public:
    void append(const std::string& text)
    {
        content_ += text;
    }
    void deleteLast(std::size_t n)
    {
        if (n >= content_.size())
            content_.clear();
        else
            content_.erase(content_.size() - n, n);
    }
    const std::string& content() const
    {
        return content_;
    }

private:
    std::string content_;
};

// ---------- Command interface --------------------------------------------

class Command
{
public:
    virtual ~Command()               = default;
    virtual void        execute()    = 0;
    virtual void        undo()       = 0;
    virtual std::string name() const = 0;
};

// ---------- Concrete Commands --------------------------------------------

class AppendCommand : public Command
{
public:
    AppendCommand(TextEditor& editor, const std::string& text)
        : editor_(editor)
        , text_(text)
    {
    }

    void execute() override
    {
        editor_.append(text_);
    }
    void undo() override
    {
        editor_.deleteLast(text_.size());
    }
    std::string name() const override
    {
        return "Append(" + text_ + ")";
    }

private:
    TextEditor& editor_;
    std::string text_;
};

class DeleteLastCommand : public Command
{
public:
    DeleteLastCommand(TextEditor& editor, std::size_t n)
        : editor_(editor)
        , n_(n)
        , deleted_("")
    {
    }

    void execute() override
    {
        const std::string& c      = editor_.content();
        std::size_t        actual = (n_ <= c.size()) ? n_ : c.size();
        deleted_                  = c.substr(c.size() - actual, actual);
        editor_.deleteLast(n_);
    }

    void undo() override
    {
        editor_.append(deleted_);
    }
    std::string name() const override
    {
        return "DeleteLast(" + std::to_string(n_) + ")";
    }

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
        while (!redo_.empty())
            redo_.pop();
    }

    bool undo()
    {
        if (history_.empty())
            return false;
        history_.top()->undo();
        redo_.push(std::move(history_.top()));
        history_.pop();
        return true;
    }

    bool redo()
    {
        if (redo_.empty())
            return false;
        redo_.top()->execute();
        history_.push(std::move(redo_.top()));
        redo_.pop();
        return true;
    }

    bool canUndo() const
    {
        return !history_.empty();
    }
    bool canRedo() const
    {
        return !redo_.empty();
    }

    std::size_t historySize() const
    {
        return history_.size();
    }

private:
    std::stack<std::unique_ptr<Command>> history_;
    std::stack<std::unique_ptr<Command>> redo_;
};

class ICommand
{
public:
    virtual ~ICommand()    = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;

    // Optional: for replay systems, commands can report
    // whether they're deterministic (safe to replay)
    virtual bool isDeterministic() const
    {
        return true;
    }
};

class Player
{
public:
    int getX() const
    {
        return 0;
    }
    int getY() const
    {
        return 0;
    }
    void move(int dx, int dy)
    {
    }
    void setPosition(int x, int y)
    {
    }
    void takeDamage(int damage)
    {
    }
    void restoreHealth(int health)
    {
    }
    void resurrect()
    {
    }
};

class Entity
{
public:
    bool isAlive() const
    {
        return true;
    }
    void takeDamage(int damage)
    {
    }
    void restoreHealth(int health)
    {
    }
    void resurrect()
    {
    }
    int getHealth() const
    {
        return 0;
    }
    int getMaxHealth() const
    {
        return 0;
    }
    int getX() const
    {
        return 0;
    }
    int getY() const
    {
        return 0;
    }
    void move(int dx, int dy)
    {
    }
    void setPosition(int x, int y)
    {
    }
};

class MoveCommand : public ICommand
{
public:
    MoveCommand(Player* player, int dx, int dy)
        : m_player(player)
        , m_dx(dx)
        , m_dy(dy)
    {
    }

    void execute() override
    {
        // Store previous position BEFORE moving — needed for undo
        m_prevX = m_player->getX();
        m_prevY = m_player->getY();
        m_player->move(m_dx, m_dy);
    }

    void undo() override
    {
        m_player->setPosition(m_prevX, m_prevY);
    }

private:
    Player* m_player;
    int     m_dx, m_dy;
    int     m_prevX = 0, m_prevY = 0; // captured on execute()
};

// AttackCommand.h — stores enough state to fully reverse the action
class AttackCommand : public ICommand
{
public:
    AttackCommand(Entity* attacker, Entity* target, int damage)
        : m_attacker(attacker)
        , m_target(target)
        , m_damage(damage)
    {
    }

    void execute() override
    {
        m_targetWasAlive = m_target->isAlive();
        m_target->takeDamage(m_damage);
    }

    void undo() override
    {
        m_target->restoreHealth(m_damage);
        if (!m_targetWasAlive)
            m_target->resurrect();
    }

private:
    Entity* m_attacker;
    Entity* m_target;
    int     m_damage;
    bool    m_targetWasAlive = true;
};

// MacroCommand — composite: executes a list of commands as one unit
class MacroCommand : public ICommand
{
public:
    void add(std::unique_ptr<ICommand> cmd)
    {
        m_commands.push_back(std::move(cmd));
    }

    void execute() override
    {
        for (auto& cmd : m_commands)
            cmd->execute();
    }

    void undo() override
    {
        // Undo in reverse order
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it)
            (*it)->undo();
    }

private:
    std::vector<std::unique_ptr<ICommand>> m_commands;
};

class CommandManager
{
public:
    void execute(std::unique_ptr<ICommand> cmd)
    {
        cmd->execute();
        m_history.push_back(std::move(cmd));

        // Branching redo: executing a new command clears the redo stack
        m_redoStack.clear();
    }

    void undo()
    {
        if (m_history.empty())
            return;
        auto cmd = std::move(m_history.back());
        m_history.pop_back();
        cmd->undo();
        m_redoStack.push_back(std::move(cmd));
    }

    void redo()
    {
        if (m_redoStack.empty())
            return;
        auto cmd = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        cmd->execute();
        m_history.push_back(std::move(cmd));
    }

    // Collapse last N commands into one undoable macro (e.g. "place building")
    void collapseToMacro(size_t count)
    {
        if (count > m_history.size())
            return;
        auto macro = std::make_unique<MacroCommand>();
        auto start = m_history.end() - count;
        for (auto it = start; it != m_history.end(); ++it)
            macro->add(std::move(*it));
        m_history.erase(start, m_history.end());
        m_history.push_back(std::move(macro));
    }

    bool canUndo() const
    {
        return !m_history.empty();
    }
    bool canRedo() const
    {
        return !m_redoStack.empty();
    }

private:
    std::vector<std::unique_ptr<ICommand>> m_history;
    std::vector<std::unique_ptr<ICommand>> m_redoStack;
};

} // namespace pattern
