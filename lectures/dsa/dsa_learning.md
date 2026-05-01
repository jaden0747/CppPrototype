# Data Structures & Algorithms in C++

## A Course for Junior Engineers Starting from Scratch

> **Who is this for?** Someone who knows basic programming (variables, loops, if-else) but has little experience with computer science theory. We'll build everything from the ground up, one brick at a time.

---

## How This Course Works

Each module follows the same rhythm:

1. **The "Why"** — A real-world story that shows why you need this concept
2. **The "What"** — The concept explained in plain language
3. **The "How"** — C++ code you can run and play with
4. **Practice Problems** — Ranked from warm-up to challenge

**Estimated total duration:** 10–12 weeks (2–3 topics per week, ~1.5 hours per topic)

---

## Module 0: Setting the Stage

### 0.1 — What Are Data Structures and Algorithms?

**The restaurant analogy:** Imagine you run a restaurant kitchen. *Data structures* are how you organize your ingredients — do you throw everything in one big pile, or do you use labeled shelves, a spice rack, and a fridge with separate drawers? *Algorithms* are your recipes — step-by-step instructions to turn those ingredients into a dish.

A bad kitchen layout (bad data structure) makes even a simple recipe painfully slow. A great layout (good data structure) makes complex recipes feel effortless.

**Key takeaway:** Choosing the right data structure is often more important than writing clever code.

### 0.2 — C++ Refresher (Just What You Need)

Before we dive in, make sure you're comfortable with:

- **Variables and types** — `int`, `double`, `char`, `string`, `bool`
- **Control flow** — `if/else`, `for`, `while`
- **Functions** — writing a function, passing arguments, returning values
- **Pointers and references** — what `*` and `&` mean, how to use `new` and `delete`
- **Structs and classes** — grouping related data together
- **Basic I/O** — `cin`, `cout`

If any of these feel shaky, spend a day or two brushing up before moving on. The rest of the course builds on them heavily.

### 0.3 — Big O Notation: How We Measure Speed

**The delivery driver analogy:** Suppose you need to deliver packages. If you have 10 packages and it takes 10 minutes, what happens with 100 packages?

- **O(1) — Constant:** You have a magical teleporter. Whether it's 1 package or a million, it takes 1 second. Example: looking up a value by index in an array.
- **O(n) — Linear:** You drive to each house one by one. 10 packages = 10 minutes, 100 packages = 100 minutes. Example: searching for a name by scanning every item in a list.
- **O(n²) — Quadratic:** For every package, you compare it against every other package to decide delivery order. 10 packages = 100 comparisons, 100 packages = 10,000 comparisons. Example: bubble sort.
- **O(log n) — Logarithmic:** You use a GPS that cuts the search area in half every step. 1,000 houses? Only ~10 steps. Example: binary search.

We'll keep revisiting Big O throughout the course — don't worry about memorizing a formula. Focus on the *intuition*.

**Practice:**
- Identify the Big O of: looping through an array, nested for-loop over same array, accessing `arr[5]`
- Rank these from fastest to slowest: O(n²), O(1), O(n log n), O(n), O(log n)

---

## Module 1: Arrays & Strings

### 1.1 — Arrays: Your First Data Structure

**The parking lot analogy:** An array is like a row of numbered parking spaces. Space #0, space #1, space #2, and so on. Every space is the same size. If you know the number, you can walk directly to that space — no searching needed.

**What makes arrays special:**

- **Fast access by position:** Getting `arr[42]` is instant — you just jump to slot 42. This is O(1).
- **Fixed size (in classic C arrays):** Once you build a parking lot with 100 spaces, you can't magically add space #101 without rebuilding.
- **Elements sit side-by-side in memory:** This makes them very cache-friendly (your CPU loves this).

**What arrays struggle with:**

- **Inserting in the middle:** If you want to squeeze a car between spaces #3 and #4, you need to push every car from #4 onward down by one. That's O(n).
- **Deleting from the middle:** Same problem in reverse — a gap that needs filling.

**C++ code to explore:**

```cpp
#include <iostream>
using namespace std;

int main() {
    // Classic C-style array (fixed size)
    int scores[5] = {90, 85, 78, 92, 88};

    // Access by index — instant
    cout << "Third score: " << scores[2] << endl;  // 78

    // Loop through all elements
    for (int i = 0; i < 5; i++) {
        cout << "Score " << i << ": " << scores[i] << endl;
    }

    return 0;
}
```

### 1.2 — Dynamic Arrays (`std::vector`)

**The expandable suitcase analogy:** A `vector` is like a suitcase that magically grows when you stuff in more clothes. Behind the scenes, when it runs out of room, it buys a bigger suitcase (usually double the size), moves everything over, and throws away the old one.

**Why you'll use vectors 90% of the time:**

- You don't need to decide the size upfront
- Pushing to the end is *usually* O(1) (amortized — occasionally it needs to resize, but averaged over many pushes, it's constant)
- Still has O(1) access by index, just like a plain array

```cpp
#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<int> grades;  // starts empty

    grades.push_back(95);  // add to end
    grades.push_back(87);
    grades.push_back(91);

    cout << "Size: " << grades.size() << endl;         // 3
    cout << "First grade: " << grades[0] << endl;       // 95
    cout << "Last grade: " << grades.back() << endl;    // 91

    grades.pop_back();  // remove last element
    cout << "Size after pop: " << grades.size() << endl; // 2

    // Range-based for loop (modern C++)
    for (int g : grades) {
        cout << g << " ";
    }
    cout << endl;

    return 0;
}
```

### 1.3 — Strings as Arrays of Characters

**The necklace analogy:** A string is like a necklace of letter beads. You can examine each bead one at a time, count them, rearrange them, or cut the necklace and re-string it.

In C++, `std::string` behaves a lot like a `vector<char>` — you can access characters by index, loop through them, and resize.

```cpp
#include <iostream>
#include <string>
using namespace std;

int main() {
    string word = "hello";

    // Access characters like an array
    cout << word[0] << endl;  // 'h'

    // Reverse a string (classic interview question)
    int left = 0, right = word.size() - 1;
    while (left < right) {
        swap(word[left], word[right]);
        left++;
        right--;
    }
    cout << word << endl;  // "olleh"

    return 0;
}
```

**Practice Problems:**

- Find the maximum element in an array
- Reverse an array in-place (without creating a new array)
- Check if a string is a palindrome (reads the same forwards and backwards)
- Merge two sorted arrays into one sorted array
- Given an array of integers, find two numbers that add up to a target value

---

## Module 2: Linked Lists

### 2.1 — Singly Linked List

**The treasure hunt analogy:** Imagine a treasure hunt where each clue card says two things: a piece of the treasure, and *where the next clue is hidden*. You start at clue #1, which points you to clue #2, which points you to clue #3, and so on. There's no master map — you can only follow the chain.

That's a linked list. Each "clue card" is a **node**, containing:

- **data** — the value stored
- **next** — a pointer to the next node (or `nullptr` if it's the last one)

**Why not just use arrays?**

- **Inserting/deleting is cheap:** To insert a new clue between #2 and #3, you just update two pointers. No shifting needed. O(1) if you already have a pointer to the spot.
- **No wasted space:** Each node is created on demand — no reserving 1000 spaces "just in case."

**The trade-off:**

- **No random access:** Want the 50th item? You have to walk through all 49 items before it. That's O(n). Arrays win here.

```cpp
#include <iostream>
using namespace std;

// Define what a "clue card" looks like
struct Node {
    int data;
    Node* next;

    Node(int val) : data(val), next(nullptr) {}
};

// A simple linked list manager
class LinkedList {
public:
    Node* head;

    LinkedList() : head(nullptr) {}

    // Add a new node to the front — O(1)
    void pushFront(int val) {
        Node* newNode = new Node(val);
        newNode->next = head;
        head = newNode;
    }

    // Print all elements — O(n)
    void print() {
        Node* current = head;
        while (current != nullptr) {
            cout << current->data << " -> ";
            current = current->next;
        }
        cout << "NULL" << endl;
    }

    // Don't forget to free memory in real code!
    ~LinkedList() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

int main() {
    LinkedList list;
    list.pushFront(3);
    list.pushFront(2);
    list.pushFront(1);
    list.print();  // 1 -> 2 -> 3 -> NULL

    return 0;
}
```

### 2.2 — Doubly Linked List

**Upgrade the treasure hunt:** Now each clue card also tells you where the *previous* clue is. You can walk forward *or* backward through the chain. This is a doubly linked list — each node has both `next` and `prev` pointers.

**When to use it:** When you need to traverse in both directions, or when you need to quickly delete a node when you already have a pointer to it (you can update both neighbors without searching).

### 2.3 — When to Use Linked Lists vs. Arrays

| Feature             | Array / Vector             | Linked List                            |
| ------------------- | -------------------------- | -------------------------------------- |
| Access by index     | O(1) — instant             | O(n) — must walk                       |
| Insert at beginning | O(n) — shift everything    | O(1) — update one pointer              |
| Insert at end       | O(1) amortized (vector)    | O(1) if you track the tail             |
| Insert in middle    | O(n) — shift elements      | O(1) if you have a pointer to the spot |
| Memory              | Contiguous, cache-friendly | Scattered, more overhead per node      |

**Rule of thumb:** Use `vector` by default. Use a linked list when you do lots of insertions/deletions at arbitrary positions and rarely need random access.

**Practice Problems:**

- Reverse a singly linked list
- Detect if a linked list has a cycle (hint: two pointers, one fast, one slow)
- Find the middle node of a linked list in one pass
- Merge two sorted linked lists into one sorted list

---

## Module 3: Stacks & Queues

### 3.1 — Stack: Last In, First Out (LIFO)

**The stack of plates analogy:** In a cafeteria, plates are stacked on top of each other. You always take the plate on *top* (the last one added), and you always add new plates on *top*. You never pull from the bottom — that would collapse the whole stack.

**Three operations, all O(1):**

- `push(x)` — put `x` on top
- `pop()` — remove and return the top item
- `top()` / `peek()` — look at the top item without removing it

**Where stacks show up in real life:**

- The "undo" button in your text editor (each action is pushed; undo pops the most recent)
- Your browser's "back" button
- Function calls in your program (the call stack!)
- Matching parentheses — every `(` is pushed, every `)` pops

```cpp
#include <iostream>
#include <stack>
using namespace std;

// Classic interview problem: check if parentheses are balanced
bool isBalanced(string s) {
    stack<char> st;

    for (char c : s) {
        if (c == '(' || c == '{' || c == '[') {
            st.push(c);
        } else {
            if (st.empty()) return false;

            char top = st.top();
            st.pop();

            if (c == ')' && top != '(') return false;
            if (c == '}' && top != '{') return false;
            if (c == ']' && top != '[') return false;
        }
    }

    return st.empty();
}

int main() {
    cout << isBalanced("({[]})") << endl;  // 1 (true)
    cout << isBalanced("({[})") << endl;   // 0 (false)
    return 0;
}
```

### 3.2 — Queue: First In, First Out (FIFO)

**The line at a coffee shop analogy:** The first person in line gets served first. New people join at the back. Nobody cuts in line.

**Core operations, all O(1):**

- `enqueue(x)` / `push(x)` — join the back of the line
- `dequeue()` / `pop()` — serve (remove) the person at the front
- `front()` — see who's at the front without removing them

**Where queues show up:**

- Print job scheduling (first document sent prints first)
- Breadth-first search in graphs (we'll cover this later!)
- Message queues in web applications

```cpp
#include <iostream>
#include <queue>
using namespace std;

int main() {
    queue<string> coffeeOrders;

    coffeeOrders.push("Latte");
    coffeeOrders.push("Cappuccino");
    coffeeOrders.push("Americano");

    // Serve customers in order
    while (!coffeeOrders.empty()) {
        cout << "Now serving: " << coffeeOrders.front() << endl;
        coffeeOrders.pop();
    }
    // Output:
    // Now serving: Latte
    // Now serving: Cappuccino
    // Now serving: Americano

    return 0;
}
```

### 3.3 — Deque (Double-Ended Queue)

A deque lets you add and remove from *both* ends in O(1). Think of it as a combination of a stack and a queue. In C++, `std::deque` is the underlying container for both `stack` and `queue`.

**Practice Problems:**

- Implement a stack using two queues
- Implement a queue using two stacks
- Evaluate a postfix expression using a stack (e.g., `3 4 + 2 *` = 14)
- Implement a "Min Stack" that supports push, pop, top, and retrieving the minimum element, all in O(1)

---

## Module 4: Hash Tables

### 4.1 — The Idea: From Name to Location in One Step

**The library card catalog analogy:** Imagine a library where every book is assigned a shelf number using a magic formula based on the book's title. Want "Harry Potter"? Run the title through the formula, get shelf #42, walk straight there. No searching. That "magic formula" is called a **hash function**.

**How it works:**

1. You have a value (a key), like the string `"Alice"`
2. The hash function converts it to a number: `hash("Alice")` → `7`
3. You store Alice's data in slot #7 of an internal array
4. When you look up Alice later, the same formula gives you #7 again — direct access

**The collision problem:** What if `hash("Alice")` and `hash("Bob")` both give 7? This is called a **collision**. Common solutions include *chaining* (each slot holds a small linked list) and *open addressing* (try the next available slot).

**Performance:**

- Average case: O(1) for insert, lookup, and delete
- Worst case: O(n) if everything hashes to the same slot (rare with good hash functions)

### 4.2 — `unordered_map` and `unordered_set` in C++

C++ gives you hash tables out of the box:

- `unordered_map<Key, Value>` — stores key-value pairs (like a dictionary)
- `unordered_set<Key>` — stores unique keys (like a mathematical set)

```cpp
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

// Classic problem: count how often each word appears
int main() {
    vector<string> words = {"apple", "banana", "apple", "cherry", "banana", "apple"};

    unordered_map<string, int> wordCount;

    for (const string& w : words) {
        wordCount[w]++;  // auto-creates entry if key doesn't exist
    }

    for (auto& [word, count] : wordCount) {
        cout << word << ": " << count << endl;
    }
    // Possible output (order not guaranteed):
    // cherry: 1
    // banana: 2
    // apple: 3

    return 0;
}
```

```cpp
// "Two Sum" — find two numbers that add to a target
// This is the #1 most asked interview question
vector<int> twoSum(vector<int>& nums, int target) {
    unordered_map<int, int> seen;  // value -> index

    for (int i = 0; i < nums.size(); i++) {
        int complement = target - nums[i];

        if (seen.count(complement)) {
            return {seen[complement], i};
        }
        seen[nums[i]] = i;
    }

    return {};  // no solution found
}
```

**Practice Problems:**

- Check if an array has any duplicate values
- Find the first non-repeating character in a string
- Group anagrams together (e.g., "eat", "tea", "ate" are all anagrams)
- Given two arrays, find their intersection (elements that appear in both)

---

## Module 5: Trees

### 5.1 — What Is a Tree?

**The family tree analogy:** A tree data structure looks exactly like a family tree. There's one ancestor at the top (the **root**). Each person can have children. People at the bottom with no children are called **leaves**. The connection between a parent and child is called an **edge**.

**Key vocabulary:**

- **Root** — the topmost node (no parent)
- **Leaf** — a node with no children
- **Height** — the longest path from root to any leaf
- **Depth** — the distance from the root to a specific node
- **Subtree** — any node and all its descendants form a smaller tree

### 5.2 — Binary Tree

A **binary tree** is a tree where every node has *at most two children*, called **left** and **right**.

```cpp
struct TreeNode {
    int data;
    TreeNode* left;
    TreeNode* right;

    TreeNode(int val) : data(val), left(nullptr), right(nullptr) {}
};
```

### 5.3 — Binary Search Tree (BST)

**The guessing game analogy:** Think of the game where someone picks a number and you guess. After each guess, they say "higher" or "lower." A BST organizes data exactly like this:

- Everything in the **left** subtree is **smaller** than the current node
- Everything in the **right** subtree is **larger** than the current node

This means searching is like binary search — you cut the possibilities in half at every step. Average case: O(log n). Worst case (if the tree becomes a straight line): O(n).

```cpp
#include <iostream>
using namespace std;

struct TreeNode {
    int data;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int val) : data(val), left(nullptr), right(nullptr) {}
};

// Insert into BST
TreeNode* insert(TreeNode* root, int val) {
    if (!root) return new TreeNode(val);

    if (val < root->data)
        root->left = insert(root->left, val);
    else
        root->right = insert(root->right, val);

    return root;
}

// Search in BST
bool search(TreeNode* root, int val) {
    if (!root) return false;
    if (val == root->data) return true;

    if (val < root->data)
        return search(root->left, val);
    else
        return search(root->right, val);
}

// In-order traversal: prints elements in sorted order!
void inOrder(TreeNode* root) {
    if (!root) return;
    inOrder(root->left);
    cout << root->data << " ";
    inOrder(root->right);
}

int main() {
    TreeNode* root = nullptr;
    for (int val : {50, 30, 70, 20, 40, 60, 80}) {
        root = insert(root, val);
    }

    inOrder(root);  // 20 30 40 50 60 70 80
    cout << endl;

    cout << "Search 40: " << search(root, 40) << endl;  // 1 (true)
    cout << "Search 45: " << search(root, 45) << endl;  // 0 (false)

    return 0;
}
```

### 5.4 — Tree Traversals

There are four classic ways to "visit" every node in a tree. Think of them as different routes through the same city:

- **In-order (Left → Root → Right):** For a BST, this gives you elements in sorted order. Like reading a book left to right.
- **Pre-order (Root → Left → Right):** Visit yourself first, then your children. Useful for copying a tree.
- **Post-order (Left → Right → Root):** Visit children first, then yourself. Useful for deleting a tree (delete children before the parent).
- **Level-order (Breadth-First):** Visit all nodes on level 0, then level 1, then level 2, etc. Uses a queue.

### 5.5 — Balanced Trees (Brief Overview)

If you insert sorted data into a BST (1, 2, 3, 4, 5...), it degenerates into a linked list and all operations become O(n). **Self-balancing trees** like AVL trees and Red-Black trees automatically rearrange themselves to stay balanced, guaranteeing O(log n) operations.

In C++, `std::map` and `std::set` are implemented as Red-Black trees under the hood — you get O(log n) operations with sorted order for free.

**Practice Problems:**

- Find the maximum depth (height) of a binary tree
- Check if two binary trees are identical
- Check if a binary tree is a valid BST
- Find the lowest common ancestor of two nodes
- Convert a sorted array into a balanced BST

---

## Module 6: Heaps & Priority Queues

### 6.1 — The Heap

**The hospital emergency room analogy:** Patients don't get treated in the order they arrive — the most critical patient is always treated first. A **heap** is a data structure that always gives you the most important (smallest or largest) element instantly.

**Key properties of a (min) heap:**

- It's a complete binary tree (every level is full, except possibly the last, which fills left to right)
- Every parent is smaller than or equal to its children
- The root is always the smallest element

**Operations:**

- **Get minimum:** O(1) — just look at the root
- **Insert:** O(log n) — add at the bottom, then "bubble up" to restore the heap property
- **Remove minimum:** O(log n) — remove root, move last element to root, then "bubble down"

### 6.2 — `priority_queue` in C++

```cpp
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int main() {
    // Max-heap by default in C++
    priority_queue<int> maxHeap;
    maxHeap.push(30);
    maxHeap.push(10);
    maxHeap.push(50);
    maxHeap.push(20);

    cout << "Max: " << maxHeap.top() << endl;  // 50

    // Min-heap: use greater<int>
    priority_queue<int, vector<int>, greater<int>> minHeap;
    minHeap.push(30);
    minHeap.push(10);
    minHeap.push(50);
    minHeap.push(20);

    cout << "Min: " << minHeap.top() << endl;  // 10

    // Process in priority order
    while (!minHeap.empty()) {
        cout << minHeap.top() << " ";
        minHeap.pop();
    }
    // Output: 10 20 30 50

    return 0;
}
```

**Practice Problems:**

- Find the K-th largest element in an array
- Merge K sorted linked lists
- Find the median of a data stream (hint: use two heaps)

---

## Module 7: Graphs

### 7.1 — What Is a Graph?

**The social network analogy:** Think of Facebook. Every person is a dot (**node** or **vertex**). Every friendship is a line connecting two dots (**edge**). That's a graph. Unlike trees, graphs can have cycles (A is friends with B, B with C, C with A — a circle), and there's no single "root."

**Types of graphs:**

- **Undirected:** Friendships — if A knows B, then B knows A
- **Directed:** Twitter follows — A follows B doesn't mean B follows A
- **Weighted:** Road maps — edges have distances or costs
- **Unweighted:** All connections are "equal"

### 7.2 — How to Represent a Graph in Code

**Adjacency List (most common):** For each node, store a list of its neighbors.

```cpp
#include <iostream>
#include <vector>
using namespace std;

int main() {
    int n = 5;  // 5 nodes: 0, 1, 2, 3, 4
    vector<vector<int>> adj(n);

    // Add edges (undirected)
    auto addEdge = [&](int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    };

    addEdge(0, 1);
    addEdge(0, 4);
    addEdge(1, 2);
    addEdge(1, 3);
    addEdge(3, 4);

    // Print neighbors
    for (int i = 0; i < n; i++) {
        cout << "Node " << i << " connects to: ";
        for (int neighbor : adj[i]) {
            cout << neighbor << " ";
        }
        cout << endl;
    }

    return 0;
}
```

**Adjacency Matrix:** A 2D grid where `matrix[i][j] = 1` means there's an edge from i to j. Simple but uses O(n²) memory — wasteful for sparse graphs.

### 7.3 — BFS (Breadth-First Search)

**The ripple in a pond analogy:** Drop a stone in a pond. The ripple spreads outward in all directions, reaching nearby points first, then farther ones. BFS explores a graph the same way — it visits all neighbors first, then neighbors' neighbors, and so on.

**Uses a queue.** BFS finds the shortest path in unweighted graphs.

```cpp
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

void bfs(int start, vector<vector<int>>& adj, int n) {
    vector<bool> visited(n, false);
    queue<int> q;

    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        cout << node << " ";

        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }
    cout << endl;
}
```

### 7.4 — DFS (Depth-First Search)

**The maze explorer analogy:** You enter a maze and always go as deep as possible down one path. When you hit a dead end, you backtrack to the last intersection and try a different path. DFS works the same way.

**Uses a stack (or recursion, which uses the call stack).**

```cpp
void dfs(int node, vector<vector<int>>& adj, vector<bool>& visited) {
    visited[node] = true;
    cout << node << " ";

    for (int neighbor : adj[node]) {
        if (!visited[neighbor]) {
            dfs(neighbor, adj, visited);
        }
    }
}
```

### 7.5 — When to Use BFS vs. DFS

| Scenario                         | Use          |
| -------------------------------- | ------------ |
| Shortest path (unweighted)       | BFS          |
| Check if a path exists           | Either       |
| Explore all connected components | Either       |
| Topological sort                 | DFS          |
| Detect cycles                    | DFS (easier) |
| Level-by-level processing        | BFS          |

**Practice Problems:**

- Find if a path exists between two nodes
- Count the number of connected components (groups of friends who all know each other)
- Find the shortest path between two nodes in an unweighted graph
- Detect a cycle in a directed graph
- Topological sort of a directed acyclic graph (DAG)

---

## Module 8: Sorting Algorithms

### 8.1 — Why Learn Sorting?

Sorting is the backbone of computer science. Once data is sorted, you can binary search it (O(log n) instead of O(n)), find duplicates easily, and solve many problems more efficiently. Plus, understanding sorting algorithms teaches you fundamental techniques like divide-and-conquer and partitioning.

### 8.2 — Bubble Sort — The Slow but Intuitive One

**The "tallest in the back" analogy:** Like lining up students by height — you compare neighbors and swap if they're out of order, making passes until everyone is in place. The biggest "bubbles up" to the end on each pass.

- **Time:** O(n²)
- **When to use:** Almost never in practice. Great for learning.

### 8.3 — Merge Sort — The Divide-and-Conquer Hero

**The "sort two halves" analogy:** Imagine sorting a deck of cards. Split the deck in half. Sort each half (by splitting *those* in half, and so on, until you have piles of one card — which are trivially sorted). Then merge the sorted halves back together by comparing the top card of each pile.

- **Time:** O(n log n) — always, guaranteed
- **Space:** O(n) — needs extra space for merging
- **When to use:** When you need guaranteed performance and stability (equal elements stay in their original order)

```cpp
#include <vector>
using namespace std;

void merge(vector<int>& arr, int left, int mid, int right) {
    vector<int> temp;
    int i = left, j = mid + 1;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j])
            temp.push_back(arr[i++]);
        else
            temp.push_back(arr[j++]);
    }

    while (i <= mid) temp.push_back(arr[i++]);
    while (j <= right) temp.push_back(arr[j++]);

    for (int k = 0; k < temp.size(); k++)
        arr[left + k] = temp[k];
}

void mergeSort(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}
```

### 8.4 — Quick Sort — The Practical Champion

**The "pick a pivot" analogy:** Choose one card from the deck (the pivot). Put all cards smaller than the pivot in a "left" pile, all larger ones in a "right" pile. Now recursively sort each pile. No merging step needed — the piles are already in the right relative order.

- **Time:** O(n log n) average, O(n²) worst case (rare with good pivot selection)
- **Space:** O(log n) — sorts in-place
- **When to use:** The go-to in practice. C++ `std::sort` uses a hybrid that includes quicksort.

### 8.5 — Sorting Comparison Cheat Sheet

| Algorithm      | Average Time | Worst Time | Space    | Stable? |
| -------------- | ------------ | ---------- | -------- | ------- |
| Bubble Sort    | O(n²)        | O(n²)      | O(1)     | Yes     |
| Selection Sort | O(n²)        | O(n²)      | O(1)     | No      |
| Insertion Sort | O(n²)        | O(n²)      | O(1)     | Yes     |
| Merge Sort     | O(n log n)   | O(n log n) | O(n)     | Yes     |
| Quick Sort     | O(n log n)   | O(n²)      | O(log n) | No      |
| Heap Sort      | O(n log n)   | O(n log n) | O(1)     | No      |

**"Stable"** means: if two elements are equal, a stable sort keeps them in their original relative order.

**Practice Problems:**

- Implement merge sort and quick sort from scratch
- Sort an array of strings by length, then alphabetically for same-length strings
- Sort an array containing only 0s, 1s, and 2s (Dutch National Flag problem)
- Find the K-th smallest element using quickselect

---

## Module 9: Searching Algorithms

### 9.1 — Linear Search — The Brute Force

Walk through every element, one by one, until you find what you're looking for. Simple, works on unsorted data, but slow: O(n).

### 9.2 — Binary Search — The Power of Sorted Data

**The dictionary analogy:** When you look up a word in a physical dictionary, you don't start at page 1. You open to the middle, check if your word comes before or after, and flip to the right half. Then repeat. Each step cuts the remaining pages in half.

**Requirement:** The data must be sorted.
**Time:** O(log n) — searching 1 billion elements takes only ~30 steps!

```cpp
#include <vector>
using namespace std;

int binarySearch(vector<int>& arr, int target) {
    int left = 0, right = arr.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;  // avoids overflow

        if (arr[mid] == target)
            return mid;           // found it!
        else if (arr[mid] < target)
            left = mid + 1;       // target is in the right half
        else
            right = mid - 1;      // target is in the left half
    }

    return -1;  // not found
}
```

**Common variations (important for interviews):**

- Find the *first* occurrence of a value
- Find the *last* occurrence of a value
- Find the insertion point (where a value *should* go)
- Search in a rotated sorted array

**Practice Problems:**

- Implement binary search (iterative and recursive)
- Find the square root of a number using binary search (integer version)
- Search in a rotated sorted array
- Find the peak element in an array

---

## Module 10: Dynamic Programming (DP)

### 10.1 — What Is Dynamic Programming?

**The staircase analogy:** You're climbing a staircase with `n` steps. Each time, you can take 1 step or 2 steps. How many different ways can you reach the top?

The naive approach: try every possible combination (exponential time — very slow).

The DP insight: **If I already know how many ways to reach step 3 and step 4, then the number of ways to reach step 5 is just `ways(3) + ways(4)`.** Instead of re-calculating the same thing millions of times, I calculate each sub-result *once* and store it. That's DP.

**Two core ideas:**

1. **Overlapping subproblems:** The same smaller problem gets solved many times
2. **Optimal substructure:** The best solution to the big problem can be built from the best solutions to smaller problems

### 10.2 — The Fibonacci Example

```cpp
#include <iostream>
#include <vector>
using namespace std;

// BAD: exponential time — recalculates everything
int fibSlow(int n) {
    if (n <= 1) return n;
    return fibSlow(n - 1) + fibSlow(n - 2);
}

// GOOD: O(n) time, O(n) space — stores results in a table
int fibDP(int n) {
    if (n <= 1) return n;

    vector<int> dp(n + 1);
    dp[0] = 0;
    dp[1] = 1;

    for (int i = 2; i <= n; i++) {
        dp[i] = dp[i-1] + dp[i-2];
    }

    return dp[n];
}

// BEST: O(n) time, O(1) space — only need the last two values
int fibOptimal(int n) {
    if (n <= 1) return n;

    int prev2 = 0, prev1 = 1;
    for (int i = 2; i <= n; i++) {
        int curr = prev1 + prev2;
        prev2 = prev1;
        prev1 = curr;
    }

    return prev1;
}
```

### 10.3 — The DP Approach (Step by Step)

1. **Define the state:** What does `dp[i]` represent? (e.g., "the number of ways to climb to step i")
2. **Find the recurrence:** How does `dp[i]` relate to smaller values? (e.g., `dp[i] = dp[i-1] + dp[i-2]`)
3. **Set the base case:** What do you know for sure? (e.g., `dp[0] = 1, dp[1] = 1`)
4. **Decide the direction:** Fill the table bottom-up (iterative) or top-down (recursive with memoization)
5. **Optimize space if possible:** Often you only need the last 1-2 rows, not the whole table

### 10.4 — Classic DP Problems to Study

- **Climbing stairs** — how many ways to climb n steps (1 or 2 at a time)
- **Coin change** — fewest coins to make a target amount
- **Longest common subsequence** — longest sequence that appears in both strings
- **0/1 Knapsack** — maximize value without exceeding weight capacity
- **Longest increasing subsequence** — longest sequence of numbers where each is bigger than the last
- **Edit distance** — minimum operations to transform one string into another

---

## Module 11: Recursion & Backtracking

### 11.1 — Recursion: Functions That Call Themselves

**The Russian nesting doll analogy:** Open a doll, and there's a smaller doll inside. Open that one, and there's an even smaller one. You keep going until you reach the tiniest doll (the **base case**). Then you close them all back up in reverse order.

**Every recursive function needs:**

1. **A base case:** When to stop (the tiniest doll)
2. **A recursive step:** How to break the problem into a smaller version of itself

```cpp
// Classic example: factorial
// 5! = 5 * 4 * 3 * 2 * 1
int factorial(int n) {
    if (n <= 1) return 1;          // base case: the tiniest doll
    return n * factorial(n - 1);   // recursive step: open the next doll
}
```

### 11.2 — Backtracking: Try, Fail, Undo, Try Again

**The maze analogy (again, but deeper):** You enter a maze and pick a path. If it leads to a dead end, you walk back to the last fork and try a different direction. If *that* fails too, you go back further. You systematically try every possibility, undoing your choices when they don't work out.

**The pattern:**

```
function backtrack(current_state):
    if current_state is a solution:
        record it
        return

    for each possible choice:
        make the choice
        backtrack(updated_state)
        undo the choice         ← THIS IS THE KEY STEP
```

**Classic backtracking problems:**

- Generate all permutations of a set
- N-Queens problem (place N queens on an N×N chessboard so none attack each other)
- Sudoku solver
- Subset sum — find all subsets that add up to a target

---

## Module 12: Greedy Algorithms

### 12.1 — The Greedy Idea

**The greedy cashier analogy:** A cashier giving change uses the largest coin possible at each step. Need to give 67 cents? Use a quarter (42 left), another quarter (17 left), a dime (7 left), a nickel (2 left), two pennies (done). At each step, you make the *locally optimal* choice without looking ahead.

**When greedy works:** When making the best local choice at each step guarantees the best global result. This doesn't always work (the coin change problem with weird denominations is a counterexample), but when it does, greedy algorithms are simple and fast.

**Classic greedy problems:**

- **Activity selection:** Given a set of activities with start/end times, find the maximum number of non-overlapping activities. Strategy: always pick the activity that ends earliest.
- **Huffman coding:** Build an optimal encoding tree for data compression.
- **Fractional knapsack:** Like 0/1 knapsack, but you can take fractions of items. Strategy: take the highest value-per-weight first.

---

## Module 13: Advanced Data Structures (Overview)

These are topics you'll encounter as you grow. You don't need to implement them from scratch, but knowing *when and why* to use them is valuable.

### 13.1 — Trie (Prefix Tree)

**The autocomplete analogy:** When you type "app" into a search bar, it suggests "apple," "application," "appreciate." A trie stores words character by character in a tree, where shared prefixes share the same path. Looking up a word takes O(length of word) time, regardless of how many words are stored.

**Use when:** Autocomplete, spell-checking, IP routing.

### 13.2 — Disjoint Set (Union-Find)

**The friend groups analogy:** You're at a party and need to figure out friend groups. Every time two people say they're friends, you merge their groups. Later, you can instantly check if two people are in the same group. With path compression and union by rank, both operations are nearly O(1).

**Use when:** Connected components, Kruskal's minimum spanning tree, network connectivity.

### 13.3 — Segment Tree / Binary Indexed Tree (Fenwick Tree)

**Use when:** You need to answer range queries (e.g., "what's the sum of elements from index 3 to 7?") and also update individual elements — both in O(log n).

### 13.4 — Graphs: Shortest Path Algorithms

- **Dijkstra's algorithm:** Find the shortest path from one node to all others in a weighted graph (no negative edges). Uses a priority queue.
- **Bellman-Ford:** Handles negative edges but is slower — O(V × E).
- **Floyd-Warshall:** Find shortest paths between *all pairs* of nodes — O(V³).

---

## Recommended Study Schedule

| Week | Topics                                            | Focus                          |
| ---- | ------------------------------------------------- | ------------------------------ |
| 1    | Module 0 (Big O), Module 1 (Arrays & Strings)     | Build strong foundations       |
| 2    | Module 2 (Linked Lists)                           | Pointer manipulation           |
| 3    | Module 3 (Stacks & Queues)                        | LIFO and FIFO patterns         |
| 4    | Module 4 (Hash Tables)                            | The most useful data structure |
| 5    | Module 5 (Trees, BST)                             | Recursive thinking             |
| 6    | Module 6 (Heaps)                                  | Priority-based problems        |
| 7    | Module 7 (Graphs, BFS, DFS)                       | Graph traversal                |
| 8    | Module 8 (Sorting)                                | Divide and conquer             |
| 9    | Module 9 (Searching)                              | Binary search mastery          |
| 10   | Module 10 (Dynamic Programming)                   | The hardest and most rewarding |
| 11   | Module 11 (Recursion & Backtracking)              | Systematic exploration         |
| 12   | Module 12 (Greedy), Module 13 (Advanced overview) | Wrap up and look ahead         |

---

## Tips for Success

**"Don't read about swimming — get in the pool."** For every topic, write code. Run it. Break it. Fix it. Change the inputs and see what happens.

**Start with brute force.** For every problem, first write the simplest, slowest solution that works. Then ask: "Where is the repeated work? What data structure could eliminate it?"

**Draw pictures.** Trees, linked lists, and graphs make much more sense when you sketch them on paper (or a whiteboard). Trace through algorithms step by step with a small example.

**Learn to recognize patterns.** Most problems map to a handful of core patterns: sliding window, two pointers, BFS/DFS, DP table, hash map lookup, divide and conquer. The more problems you solve, the faster you'll spot these.

**Resources to pair with this course:**

- LeetCode (start with the "Easy" problems in each topic)
- Visualgo.net (animated visualizations of algorithms)
- "Introduction to Algorithms" by Cormen et al. (the classic textbook, use as reference)
- C++ Reference at cppreference.com

---

*This course plan is designed to be followed sequentially. Each module builds on concepts from previous ones. The person working through this should spend roughly equal time reading explanations and solving practice problems.*