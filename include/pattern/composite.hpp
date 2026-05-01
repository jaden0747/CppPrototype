#pragma once
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Composite Pattern
//
// Intent: Compose objects into tree structures to represent part-whole
// hierarchies. Composite lets clients treat individual objects and
// compositions of objects uniformly.
//
// Real-world analogy: A file system. A Directory contains Files and other
// Directories. Whether you call `size()` on a single File or an entire
// Directory tree, the call is identical — the structure is transparent.
//
// Key C++ mechanics used:
//  - Common Component interface with virtual operations.
//  - Leaf implements operations directly.
//  - Composite holds a vector of children (std::unique_ptr<Component>).
//  - Recursive `size()` and `print()` demonstrate tree traversal.
//
// When to use:
//  - You want clients to treat leaf and composite objects uniformly.
//  - The structure is naturally hierarchical (menus, UI trees, AST, scene
//    graphs, org charts, file systems).
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Component (abstract) -----------------------------------------

class FileSystemNode
{
public:
    explicit FileSystemNode(const std::string& name) : name_(name) {}
    virtual ~FileSystemNode() = default;

    virtual long long size()                           const = 0;
    virtual void      print(const std::string& indent) const = 0;
    virtual bool      isDirectory()                    const = 0;

    const std::string& name() const { return name_; }

protected:
    std::string name_;
};

// ---------- Leaf ----------------------------------------------------------

class File : public FileSystemNode
{
public:
    File(const std::string& name, long long bytes)
        : FileSystemNode(name), bytes_(bytes) {}

    long long size()                           const override { return bytes_; }
    bool      isDirectory()                    const override { return false; }
    void      print(const std::string& indent) const override
    {
        // Simple output for demonstration — not using std::cout in tests
        (void)indent;
    }

private:
    long long bytes_;
};

// ---------- Composite -----------------------------------------------------

class Directory : public FileSystemNode
{
public:
    explicit Directory(const std::string& name) : FileSystemNode(name) {}

    void add(std::unique_ptr<FileSystemNode> child)
    {
        children_.push_back(std::move(child));
    }

    long long size() const override
    {
        long long total = 0;
        for (const auto& child : children_)
            total += child->size();
        return total;
    }

    bool isDirectory() const override { return true; }

    void print(const std::string& indent) const override
    {
        (void)indent;
        for (const auto& child : children_)
            child->print(indent + "  ");
    }

    std::size_t childCount() const { return children_.size(); }

    const FileSystemNode* childAt(std::size_t index) const
    {
        if (index >= children_.size())
            throw std::out_of_range("Index out of range");
        return children_[index].get();
    }

private:
    std::vector<std::unique_ptr<FileSystemNode>> children_;
};

} // namespace pattern
