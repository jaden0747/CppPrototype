#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <iterator>

// ---------------------------------------------------------------------------
// Iterator Pattern
//
// Intent: Provide a way to sequentially access elements of an aggregate object
// without exposing its underlying representation.
//
// Real-world analogy: A TV remote's "next/prev channel" buttons — you iterate
// through channels without knowing how the channel list is stored.
//
// Two versions shown:
//   1. Custom Iterator class (manual forward iterator over a tree-like list)
//   2. STL-compatible iterator (implements the iterator concept so range-for
//      and std algorithms work out of the box)
//
// Key C++ mechanics used:
//  - Iterator interface with hasNext() / next() (pattern classic form).
//  - STL-compatible iterator nested inside the container, exposing
//    begin() / end(), operator++, operator*, and operator!=.
//
// When to use:
//  - You want to hide the internal structure of a collection.
//  - You want to provide multiple traversal orders without changing the
//    aggregate.
//  - Multiple iterators over the same collection simultaneously.
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- 1. Custom iterator over a flat list --------------------------

class WordCollection
{
public:
    void add(const std::string& word) { words_.push_back(word); }
    std::size_t size() const { return words_.size(); }

    // Forward iterator (manual)
    class ForwardIterator
    {
    public:
        explicit ForwardIterator(const WordCollection& col)
            : col_(col), index_(0) {}

        bool        hasNext() const { return index_ < col_.words_.size(); }
        const std::string& next()
        {
            if (!hasNext()) throw std::out_of_range("Iterator exhausted");
            return col_.words_[index_++];
        }

    private:
        const WordCollection& col_;
        std::size_t           index_;
    };

    // Reverse iterator
    class ReverseIterator
    {
    public:
        explicit ReverseIterator(const WordCollection& col)
            : col_(col), index_(col.words_.size()) {}

        bool hasNext() const { return index_ > 0; }
        const std::string& next()
        {
            if (!hasNext()) throw std::out_of_range("Iterator exhausted");
            return col_.words_[--index_];
        }

    private:
        const WordCollection& col_;
        std::size_t           index_;
    };

    ForwardIterator forwardIterator() const { return ForwardIterator(*this); }
    ReverseIterator reverseIterator() const { return ReverseIterator(*this); }

private:
    std::vector<std::string> words_;
};

// ---------- 2. STL-compatible iterator -----------------------------------

class NumberRange
{
public:
    NumberRange(int from, int to) : from_(from), to_(to) {}

    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = int;
        using difference_type   = int;
        using pointer           = const int*;
        using reference         = const int&;

        int current;
        explicit Iterator(int v) : current(v) {}
        int       operator*()  const  { return current; }
        Iterator& operator++()        { ++current; return *this; }
        Iterator  operator++(int)     { Iterator tmp(*this); ++current; return tmp; }
        bool      operator==(const Iterator& o) const { return current == o.current; }
        bool      operator!=(const Iterator& o) const { return current != o.current; }
    };

    Iterator begin() const { return Iterator(from_); }
    Iterator end()   const { return Iterator(to_ + 1); }

private:
    int from_, to_;
};

} // namespace pattern
