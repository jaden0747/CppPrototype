// ============================================================================
// Stdlib 02 — Demo: Associative Containers
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>

// ──────────────────────────────────────────────────────────────────────────
// 1. std::set basics
// ──────────────────────────────────────────────────────────────────────────
void demo_set()
{
    std::cout << "=== 1. std::set ===\n";

    std::set<int> s{5, 3, 1, 4, 2, 2, 3}; // duplicates ignored
    std::cout << "  size (no dups): " << s.size() << "\n";

    // Iteration is sorted
    std::cout << "  elements: ";
    for (int x : s)
        std::cout << x << " ";
    std::cout << "\n";

    // Insert & erase
    auto [it, inserted] = s.insert(6);
    std::cout << "  insert(6): " << (inserted ? "new" : "existed") << "\n";

    s.erase(3);
    std::cout << "  after erase(3): ";
    for (int x : s)
        std::cout << x << " ";
    std::cout << "\n";

    // contains (C++20)
    std::cout << "  contains(4)? " << std::boolalpha << s.contains(4) << "\n";

    // lower_bound / upper_bound
    auto lb = s.lower_bound(3);
    std::cout << "  lower_bound(3) = " << *lb << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::multiset
// ──────────────────────────────────────────────────────────────────────────
void demo_multiset()
{
    std::cout << "=== 2. std::multiset ===\n";

    std::multiset<int> ms{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    std::cout << "  size (with dups): " << ms.size() << "\n";
    std::cout << "  count(5) = " << ms.count(5) << "\n";

    // equal_range
    auto [lo, hi] = ms.equal_range(5);
    std::cout << "  equal_range(5): ";
    for (auto it = lo; it != hi; ++it)
        std::cout << *it << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::map
// ──────────────────────────────────────────────────────────────────────────
void demo_map()
{
    std::cout << "=== 3. std::map ===\n";

    std::map<std::string, int> m;
    m["apple"]  = 3;
    m["banana"] = 5;
    m["cherry"] = 2;

    // emplace
    m.emplace("date", 7);

    // Iteration (sorted by key)
    std::cout << "  contents:\n";
    for (const auto& [key, val] : m)
    {
        std::cout << "    " << key << " -> " << val << "\n";
    }

    // operator[] default-constructs if missing
    int& x = m["elderberry"]; // inserts with value 0
    std::cout << "  m[\"elderberry\"] (auto-inserted) = " << x << "\n";

    // at() throws if missing
    try
    {
        [[maybe_unused]] int y = m.at("fig");
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "  m.at(\"fig\"): " << e.what() << "\n";
    }

    // contains (C++20)
    std::cout << "  contains(\"apple\")? " << std::boolalpha << m.contains("apple") << "\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. try_emplace & insert_or_assign (C++17)
// ──────────────────────────────────────────────────────────────────────────
void demo_try_emplace()
{
    std::cout << "=== 4. try_emplace & insert_or_assign (C++17) ===\n";

    std::map<std::string, std::string> m;

    // try_emplace: only constructs value if key is NEW
    auto [it1, ok1] = m.try_emplace("key1", "first value");
    std::cout << "  try_emplace(key1): " << (ok1 ? "inserted" : "skipped") << "\n";

    auto [it2, ok2] = m.try_emplace("key1", "second value");
    std::cout << "  try_emplace(key1) again: " << (ok2 ? "inserted" : "skipped") << "\n";
    std::cout << "  key1 = \"" << m["key1"] << "\"\n";

    // insert_or_assign: insert or OVERWRITE
    m.insert_or_assign("key1", "overwritten!");
    std::cout << "  after insert_or_assign: key1 = \"" << m["key1"] << "\"\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Node extraction & merging (C++17)
// ──────────────────────────────────────────────────────────────────────────
void demo_node_extract()
{
    std::cout << "=== 5. Node Extraction & Merging (C++17) ===\n";

    std::map<int, std::string> m1{{1, "one"}, {2, "two"}, {3, "three"}};
    std::map<int, std::string> m2{{4, "four"}, {5, "five"}};

    // Extract and modify a node's key
    auto node = m1.extract(2);
    if (!node.empty())
    {
        std::cout << "  extracted key=2, value=\"" << node.mapped() << "\"\n";
        node.key() = 20; // Change key without copy!
        m1.insert(std::move(node));
    }

    std::cout << "  m1 after key change: ";
    for (const auto& [k, v] : m1)
        std::cout << k << ":" << v << " ";
    std::cout << "\n";

    // Merge: move all nodes from m2 into m1
    m1.merge(m2);
    std::cout << "  m1 after merge: ";
    for (const auto& [k, v] : m1)
        std::cout << k << ":" << v << " ";
    std::cout << "\n";
    std::cout << "  m2 remaining: " << m2.size() << " elements\n";

    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Custom comparators
// ──────────────────────────────────────────────────────────────────────────
void demo_custom_comparator()
{
    std::cout << "=== 6. Custom Comparators ===\n";

    // Case-insensitive string set
    auto ci_less = [](const std::string& a, const std::string& b)
    {
        return std::lexicographical_compare(
            a.begin(),
            a.end(),
            b.begin(),
            b.end(),
            [](char ca, char cb) { return std::tolower(ca) < std::tolower(cb); });
    };

    std::set<std::string, decltype(ci_less)> s(ci_less);
    s.insert("Apple");
    s.insert("APPLE"); // considered duplicate
    s.insert("banana");

    std::cout << "  case-insensitive set: ";
    for (const auto& e : s)
        std::cout << "\"" << e << "\" ";
    std::cout << "\n";
    std::cout << "  size = " << s.size() << " (Apple == APPLE)\n";

    // Descending order map
    std::map<int, std::string, std::greater<>> desc;
    desc[1] = "one";
    desc[2] = "two";
    desc[3] = "three";
    std::cout << "  descending map: ";
    for (const auto& [k, v] : desc)
        std::cout << k << ":" << v << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. std::multimap
// ──────────────────────────────────────────────────────────────────────────
void demo_multimap()
{
    std::cout << "=== 7. std::multimap ===\n";

    std::multimap<std::string, int> mm;
    mm.emplace("math", 95);
    mm.emplace("math", 88);
    mm.emplace("english", 72);
    mm.emplace("math", 91);
    mm.emplace("english", 85);

    // Group by key
    std::cout << "  all entries:\n";
    for (const auto& [subject, score] : mm)
    {
        std::cout << "    " << subject << ": " << score << "\n";
    }

    // equal_range for one key
    auto [lo, hi] = mm.equal_range("math");
    std::cout << "  math scores: ";
    for (auto it = lo; it != hi; ++it)
        std::cout << it->second << " ";
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 02 — Associative Containers              ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_set();
    demo_multiset();
    demo_map();
    demo_try_emplace();
    demo_node_extract();
    demo_custom_comparator();
    demo_multimap();

    std::cout << "All demos complete.\n";
    return 0;
}
