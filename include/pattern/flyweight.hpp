#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Flyweight Pattern
//
// Intent: Use sharing to support a large number of fine-grained objects
// efficiently. Separate intrinsic state (shared, immutable) from extrinsic
// state (unique per context, stored outside the flyweight).
//
// Real-world analogy: A forest with a million trees. Each tree has a unique
// position/colour (extrinsic), but only ~10 tree *types* (texture, mesh).
// Store one TreeType object per species; every Tree instance just references
// a shared TreeType.
//
// Key C++ mechanics used:
//  - Flyweight (TreeType) stores intrinsic state; constructor is private/
//    restricted to the factory.
//  - FlyweightFactory (TreeTypeFactory) maps a key → shared_ptr<TreeType>.
//  - Context (Tree) stores extrinsic state and a shared_ptr to the flyweight.
//  - std::unordered_map for O(1) lookup in the factory.
//
// When to use:
//  - Application uses a huge number of objects.
//  - Memory cost is high because of the sheer quantity.
//  - Most object state can be made extrinsic.
//  - Many groups of objects can share extrinsic state once removed.
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Flyweight (intrinsic state) ----------------------------------

class TreeType
{
public:
    TreeType(const std::string& name,
             const std::string& colour,
             const std::string& texture)
        : name_(name), colour_(colour), texture_(texture) {}

    const std::string& name()    const { return name_; }
    const std::string& colour()  const { return colour_; }
    const std::string& texture() const { return texture_; }

    // Simulates rendering a tree with extrinsic (x, y) coordinates
    std::string render(int x, int y) const
    {
        return "Draw " + name_ + " [" + colour_ + "/" + texture_ + "] at ("
               + std::to_string(x) + "," + std::to_string(y) + ")";
    }

private:
    std::string name_;
    std::string colour_;
    std::string texture_;
};

// ---------- Flyweight Factory --------------------------------------------

class TreeTypeFactory
{
public:
    std::shared_ptr<TreeType> getTreeType(const std::string& name,
                                          const std::string& colour,
                                          const std::string& texture)
    {
        std::string key = name + "|" + colour + "|" + texture;
        auto it = cache_.find(key);
        if (it != cache_.end())
            return it->second;

        auto type = std::make_shared<TreeType>(name, colour, texture);
        cache_[key] = type;
        return type;
    }

    std::size_t cacheSize() const { return cache_.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<TreeType>> cache_;
};

// ---------- Context (extrinsic state + reference to flyweight) -----------

struct Tree
{
    int x;
    int y;
    std::shared_ptr<TreeType> type;

    std::string render() const { return type->render(x, y); }
};

// ---------- Forest (collection of contexts) ------------------------------

class Forest
{
public:
    void plantTree(int x, int y,
                   const std::string& name,
                   const std::string& colour,
                   const std::string& texture)
    {
        auto type = factory_.getTreeType(name, colour, texture);
        trees_.push_back({x, y, type});
    }

    std::size_t treeCount()      const { return trees_.size(); }
    std::size_t uniqueTypeCount() const { return factory_.cacheSize(); }

    const Tree& treeAt(std::size_t idx) const { return trees_[idx]; }

private:
    TreeTypeFactory        factory_;
    std::vector<Tree>      trees_;
};

} // namespace pattern
