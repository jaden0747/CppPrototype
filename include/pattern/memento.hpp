#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Memento Pattern
//
// Intent: Without violating encapsulation, capture and externalise an object's
// internal state so the object can be restored to that state later.
//
// Real-world analogy: A text editor's "undo" history. Each keystroke creates
// a snapshot (Memento) of the document state. Undo restores the previous one.
//
// Key C++ mechanics used:
//  - Memento is a value type (or private inner class) — it stores state but
//    has no behaviour.
//  - Originator creates and restores Mementos; it owns its own state.
//  - Caretaker manages a stack of Mementos (does not inspect their contents).
//  - Nested class pattern: Originator's inner Memento class hides state from
//    external code.
//
// When to use:
//  - You need snapshots of an object's state to restore it later.
//  - A direct interface to obtaining the state would expose implementation
//    details and break the object's encapsulation.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Originator ---------------------------------------------------

class Editor
{
public:
    // Opaque snapshot — external code cannot read the state inside
    class Memento
    {
    public:
        const std::string& content() const
        {
            return content_;
        }
        const std::string& label() const
        {
            return label_;
        }

    private:
        friend class Editor;
        Memento(const std::string& content, const std::string& lbl)
            : content_(content)
            , label_(lbl)
        {
        }
        std::string content_;
        std::string label_;
    };

    void type(const std::string& text)
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

    Memento save(const std::string& label = "") const
    {
        return Memento(content_, label);
    }

    void restore(const Memento& m)
    {
        content_ = m.content_;
    }

private:
    std::string content_;
};

// ---------- Caretaker ----------------------------------------------------

class History
{
public:
    void push(const Editor::Memento& m)
    {
        snapshots_.push_back(m);
    }

    Editor::Memento pop()
    {
        if (snapshots_.empty())
            throw std::out_of_range("History is empty");
        Editor::Memento m = snapshots_.back();
        snapshots_.pop_back();
        return m;
    }

    bool empty() const
    {
        return snapshots_.empty();
    }
    std::size_t size() const
    {
        return snapshots_.size();
    }

    const Editor::Memento& top() const
    {
        if (snapshots_.empty())
            throw std::out_of_range("History is empty");
        return snapshots_.back();
    }

private:
    std::vector<Editor::Memento> snapshots_;
};

} // namespace pattern
