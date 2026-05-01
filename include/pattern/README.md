# Design Patterns Library

A complete implementation of all **23 Gang of Four (GoF) design patterns** in
both **C++14** and **Python 3**, with unit tests for every pattern.

---

## Project layout

```
include/pattern/     ← C++ header-only implementations
test/pattern/        ← C++ GTest test files (one per pattern)
scripts/pattern/     ← Python implementations
scripts/test_pattern/← Python unittest test files
src/pattern/         ← CMakeLists.txt (builds all C++ test binaries)
```

---

## Building the C++ tests

```bash
# From the project root
cmake --preset debug          # or your preferred preset
cmake --build build/Debug --target test_singleton  # individual pattern
cmake --build build/Debug                           # all targets
ctest --test-dir build/Debug                        # run all tests
```

---

## Running the Python tests

```bash
# Individual pattern
python -m pytest scripts/test_pattern/test_singleton.py -v

# All pattern tests
python -m pytest scripts/test_pattern/ -v
```

---

## Pattern catalogue

### Creational Patterns

| # | Pattern | Intent (one sentence) | C++ file | Python file |
|---|---------|----------------------|----------|-------------|
| 1 | **Singleton** | Ensure a class has only one instance and provide a global access point to it. | [singleton.hpp](singleton.hpp) | [singleton.py](../../scripts/pattern/singleton.py) |
| 2 | **Factory Method** | Define an interface for creating an object, but let subclasses decide which class to instantiate. | [factory_method.hpp](factory_method.hpp) | [factory_method.py](../../scripts/pattern/factory_method.py) |
| 3 | **Abstract Factory** | Provide an interface for creating families of related objects without specifying concrete classes. | [abstract_factory.hpp](abstract_factory.hpp) | [abstract_factory.py](../../scripts/pattern/abstract_factory.py) |
| 4 | **Builder** | Separate the construction of a complex object from its representation. | [builder.hpp](builder.hpp) | [builder.py](../../scripts/pattern/builder.py) |
| 5 | **Prototype** | Specify the kinds of objects to create using a prototypical instance, and create new objects by copying it. | [prototype.hpp](prototype.hpp) | [prototype.py](../../scripts/pattern/prototype.py) |

### Structural Patterns

| # | Pattern | Intent (one sentence) | C++ file | Python file |
|---|---------|----------------------|----------|-------------|
| 6 | **Adapter** | Convert the interface of a class into another interface clients expect. | [adapter.hpp](adapter.hpp) | [adapter.py](../../scripts/pattern/adapter.py) |
| 7 | **Bridge** | Decouple an abstraction from its implementation so the two can vary independently. | [bridge.hpp](bridge.hpp) | [bridge.py](../../scripts/pattern/bridge.py) |
| 8 | **Composite** | Compose objects into tree structures to represent part-whole hierarchies. | [composite.hpp](composite.hpp) | [composite.py](../../scripts/pattern/composite.py) |
| 9 | **Decorator** | Attach additional responsibilities to an object dynamically. | [decorator.hpp](decorator.hpp) | [decorator.py](../../scripts/pattern/decorator.py) |
| 10 | **Facade** | Provide a simplified interface to a complex subsystem. | [facade.hpp](facade.hpp) | [facade.py](../../scripts/pattern/facade.py) |
| 11 | **Flyweight** | Use sharing to support a large number of fine-grained objects efficiently. | [flyweight.hpp](flyweight.hpp) | [flyweight.py](../../scripts/pattern/flyweight.py) |
| 12 | **Proxy** | Provide a surrogate or placeholder for another object to control access to it. | [proxy.hpp](proxy.hpp) | [proxy.py](../../scripts/pattern/proxy.py) |

### Behavioural Patterns

| # | Pattern | Intent (one sentence) | C++ file | Python file |
|---|---------|----------------------|----------|-------------|
| 13 | **Chain of Responsibility** | Pass a request along a chain of handlers, each deciding to handle or pass it on. | [chain_of_responsibility.hpp](chain_of_responsibility.hpp) | [chain_of_responsibility.py](../../scripts/pattern/chain_of_responsibility.py) |
| 14 | **Command** | Encapsulate a request as an object, allowing undo/redo, queuing, and logging. | [command.hpp](command.hpp) | [command.py](../../scripts/pattern/command.py) |
| 15 | **Interpreter** | Define a grammar for a language and provide an interpreter to deal with that grammar. | [interpreter.hpp](interpreter.hpp) | [interpreter.py](../../scripts/pattern/interpreter.py) |
| 16 | **Iterator** | Provide a way to access elements of an aggregate sequentially without exposing its representation. | [iterator.hpp](iterator.hpp) | [iterator.py](../../scripts/pattern/iterator.py) |
| 17 | **Mediator** | Define an object that encapsulates how a set of objects interact, reducing direct coupling. | [mediator.hpp](mediator.hpp) | [mediator.py](../../scripts/pattern/mediator.py) |
| 18 | **Memento** | Without violating encapsulation, capture an object's state so it can be restored later. | [memento.hpp](memento.hpp) | [memento.py](../../scripts/pattern/memento.py) |
| 19 | **Observer** | Define a one-to-many dependency so that when one object changes state, dependents are notified. | [observer.hpp](observer.hpp) | [observer.py](../../scripts/pattern/observer.py) |
| 20 | **State** | Allow an object to alter its behaviour when its internal state changes. | [state.hpp](state.hpp) | [state.py](../../scripts/pattern/state.py) |
| 21 | **Strategy** | Define a family of algorithms, encapsulate each one, and make them interchangeable. | [strategy.hpp](strategy.hpp) | [strategy.py](../../scripts/pattern/strategy.py) |
| 22 | **Template Method** | Define the skeleton of an algorithm in a base class, deferring some steps to subclasses. | [template_method.hpp](template_method.hpp) | [template_method.py](../../scripts/pattern/template_method.py) |
| 23 | **Visitor** | Represent an operation on elements of an object structure without changing their classes. | [visitor.hpp](visitor.hpp) | [visitor.py](../../scripts/pattern/visitor.py) |

---

## Pattern deep-dives

### 1 · Singleton
**Analogy:** A country has exactly one government.
**Key C++ feature:** Meyers' singleton — local `static` is initialised once,
thread-safe since C++11. Copy and move constructors are `= delete`.
**Snippet:**
```cpp
Counter& c = Counter::instance();
c.increment();
```

---

### 2 · Factory Method
**Analogy:** A logistics company that started with trucks now needs ships.
Adding ships doesn't change the order-processing workflow.
**Key C++ feature:** Covariant return types on virtual `create()` — a derived
factory can return a derived product pointer.
**Snippet:**
```cpp
auto logistics = makeLogistics("ship");
auto transport = logistics->createTransport();
transport->deliver();
```

---

### 3 · Abstract Factory
**Analogy:** IKEA sells furniture in consistent style collections (Modern,
Victorian). You pick a collection; all pieces match.
**Key C++ feature:** Factory method returns `unique_ptr<IProduct>` — ownership
is transferred to the caller with no raw-pointer leaks.
**Snippet:**
```cpp
auto factory = makeFurnitureFactory("modern");
auto chair = factory->createChair();
```

---

### 4 · Builder
**Analogy:** A restaurant burger menu — same kitchen, different assembly order
produces a MeatBurger or VeggieBurger.
**Key C++ feature:** Fluent interface (method chaining via `return *this`) as
an alternative to a Director class.
**Snippet:**
```cpp
auto burger = FluentBurger{}.withBun("sesame").withPatty("beef").build();
```

---

### 5 · Prototype
**Analogy:** A photocopier. You don't re-type the document; you clone the
existing one and edit the copy.
**Key C++ feature:** `clone()` via copy constructor wrapped in `unique_ptr`;
`ShapeRegistry` maps string keys to prototype shapes.
**Snippet:**
```cpp
ShapeRegistry reg;
auto c = std::make_unique<Circle>(5.0);
reg.registerShape("big_circle", std::move(c));
auto copy = reg.create("big_circle");
```

---

### 6 · Adapter
**Analogy:** A power-socket adapter lets a US plug fit a European socket.
**Key C++ feature:** Two flavours — *object adapter* (composition) and
*class adapter* (multiple inheritance). Both demonstrated.
**Snippet:**
```cpp
SquarePeg sp(10.0);
SquarePegAdapter adapter(sp);
RoundHole hole(7.07);
hole.fits(adapter);   // true — sqrt(2)/2 * 10 ≈ 7.07
```

---

### 7 · Bridge
**Analogy:** A TV remote and a radio remote share the same button layout but
talk to completely different devices underneath.
**Key C++ feature:** The abstraction (`RemoteControl`) holds a pointer to the
implementation (`IDevice`) — changing one side doesn't recompile the other.
**Snippet:**
```cpp
TV tv;
RemoteControl remote(&tv);
remote.togglePower();
remote.setVolume(50);
```

---

### 8 · Composite
**Analogy:** A file system — files and directories are treated uniformly.
A directory can contain other directories or files.
**Key C++ feature:** `unique_ptr` children in the composite node — ownership
is expressed in the type system; no manual `delete` needed.
**Snippet:**
```cpp
auto root = std::make_unique<Directory>("root");
root->add(std::make_unique<File>("readme.txt", 100));
root->size();  // recursive
```

---

### 9 · Decorator
**Analogy:** A coffee shop — start with espresso, add milk, caramel, whip.
Each addition wraps the previous one; the bill accumulates.
**Key C++ feature:** Each decorator wraps an `IComponent` by `unique_ptr`,
enabling unlimited stacking with proper ownership.
**Snippet:**
```cpp
auto coffee = std::make_unique<Espresso>();
coffee = std::make_unique<Milk>(std::move(coffee));
coffee = std::make_unique<Caramel>(std::move(coffee));
coffee->cost();  // 1.0 + 0.25 + 0.50
```

---

### 10 · Facade
**Analogy:** A home-theatre "watch movie" button — one press activates
projector, dimmer, sound system, and popcorn machine.
**Key C++ feature:** Facade composes several subsystem objects and exposes a
single high-level method; subsystems remain accessible for advanced use.
**Snippet:**
```cpp
VideoConverter vc;
std::string out = vc.convert("movie.mp4", "avi");
```

---

### 11 · Flyweight
**Analogy:** A forest rendering engine. Thousands of trees share the same
`TreeType` (texture, colour) — only position differs per tree.
**Key C++ feature:** `unordered_map` cache in `TreeTypeFactory` ensures only
one `TreeType` per unique (name, colour, texture) combination.
**Snippet:**
```cpp
Forest forest;
forest.plantTree(0, 0, "Oak", "green", "oak_tex");
forest.plantTree(1, 2, "Oak", "green", "oak_tex");  // reuses same TreeType
```

---

### 12 · Proxy
**Analogy:** A bank card is a proxy for your bank account — it controls
access, adds logging, and caches balance checks.
**Three flavours shown:** virtual proxy (lazy init), protection proxy
(role-based access), caching proxy (result memoisation).
**Snippet:**
```cpp
ProtectionProxy proxy("admin_password");
proxy.setRole(Role::Admin);
proxy.fetchData();  // allowed
```

---

### 13 · Chain of Responsibility
**Analogy:** IT support tiers — Tier 1 handles basic issues; escalates to
Tier 2 for software problems, Tier 3 for hardware.
**Key C++ feature:** Each handler holds a `unique_ptr` to the next handler —
the chain owns itself and cleans up automatically.
**Snippet:**
```cpp
auto chain = buildDefaultChain();
chain->handle(SupportRequest{Priority::High, "kernel panic"});
```

---

### 14 · Command
**Analogy:** A text editor with undo/redo — every keystroke is a Command
object stored in a history stack.
**Key C++ feature:** `std::stack<unique_ptr<ICommand>>` in `CommandHistory`
for LIFO undo; a separate redo stack for redo.
**Snippet:**
```cpp
CommandHistory history;
history.execute(std::make_unique<AppendCommand>(editor, "Hello"));
history.undo();
```

---

### 15 · Interpreter
**Analogy:** A SQL `WHERE` clause parser — `AND`, `OR`, `NOT` compose into an
expression tree evaluated against a row.
**Key C++ feature:** Recursive `evaluate()` on expression tree nodes;
`Context = unordered_map<string,bool>` for variable bindings.
**Snippet:**
```cpp
Context ctx{{"x", true}, {"y", false}};
auto expr = std::make_unique<And>(
    std::make_unique<Variable>("x"),
    std::make_unique<Not>(std::make_unique<Variable>("y")));
expr->evaluate(ctx);  // true
```

---

### 16 · Iterator
**Analogy:** A playlist remote — next/prev buttons iterate songs without
exposing the underlying linked list.
**Two flavours shown:** custom bidirectional iterator nested in a collection;
STL-compatible iterator enabling range-for.
**Snippet:**
```cpp
NumberRange range(1, 5);
for (int n : range) std::cout << n << ' ';  // 1 2 3 4 5
```

---

### 17 · Mediator
**Analogy:** An air traffic control tower — planes don't talk to each other
directly; all communication goes through the tower.
**Key C++ feature:** `Component` base stores a raw (non-owning) pointer to the
mediator; the mediator (chat room) owns nothing — users do.
**Snippet:**
```cpp
ChatRoom room;
ChatUser alice{"Alice"}, bob{"Bob"};
room.addUser(&alice); room.addUser(&bob);
alice.send("Hello!");   // bob receives, alice does not
```

---

### 18 · Memento
**Analogy:** A text editor's "Undo" history. Each save point captures the
document state; undo restores a previous save point.
**Key C++ feature:** `Memento` is a nested class of `Editor` — its private
constructor is accessible only to `Editor` (friend), maintaining encapsulation.
**Snippet:**
```cpp
Editor e; History h;
e.type("Hello"); h.push(e.save());
e.type(" World");
h.pop(); e.restore(h.top());  // back to "Hello"
```

---

### 19 · Observer
**Analogy:** A newspaper subscription — subscribers receive new editions
automatically; they can subscribe/unsubscribe freely.
**Two flavours shown:** class-based `IObserver`/`ISubject` hierarchy;
generic `EventEmitter<T>` backed by `std::function` for lightweight callbacks.
**Snippet:**
```cpp
StockMarket market;
Logger logger;
market.attach(&logger);
market.setPrice("AAPL", 150.0);  // logger.log() now has one entry
```

---

### 20 · State
**Analogy:** A vending machine — inserting a coin, selecting a product, and
dispensing are valid only in the correct sequence.
**Key C++ feature:** Each state transition is implemented by the state itself
calling `vm.setState(new NextState())` — the context delegates unconditionally.
**Snippet:**
```cpp
VendingMachine vm(3);
vm.insertCoin();
vm.selectProduct();
vm.dispense();          // stock--; back to Idle
vm.insertCoin();        // throws if vm is OutOfStock
```

---

### 21 · Strategy
**Analogy:** A navigation app — "fastest", "shortest", "avoid tolls" are
interchangeable routing strategies.
**Two flavours shown:** `ISortStrategy` class hierarchy; `FunctionalSorter`
accepting `std::function` / lambdas for lightweight swaps.
**Snippet:**
```cpp
Sorter sorter(makeBubbleSort());
sorter.sort(data);
sorter.setStrategy(makeReverseSort());
sorter.sort(data);
```

---

### 22 · Template Method
**Analogy:** A data-mining report pipeline. The steps (open, extract, parse,
analyse, build report) are fixed; concrete miners fill in the format-specific steps.
**Key C++ feature:** NVI (Non-Virtual Interface) idiom — `mine()` is `public
final`; the hook methods are `protected virtual`, preventing external callers
from breaking the algorithm skeleton.
**Snippet:**
```cpp
CsvMiner miner;
std::string report = miner.mine("row1;row2;row3");
```

---

### 23 · Visitor
**Analogy:** An accountant visiting every department. Each department type
is processed differently, but the accountant adds new calculations without
modifying any department class.
**Key C++ feature:** Double dispatch — `shape.accept(visitor)` calls
`visitor.visit(*this)`, binding both types at runtime without `dynamic_cast`.
**Snippet:**
```cpp
AreaVisitor av;
for (auto& shape : shapes) shape->accept(av);
double total = av.total();
```

---

## C++ vs Python cheat-sheet

| Concept | C++ idiom | Python idiom |
|---------|-----------|--------------|
| Prevent copy | `= delete` on copy ctor/assign | `__copy__ = None` or raise |
| Ownership | `unique_ptr<T>` | GC / `weakref` |
| Interface | Pure-virtual base class | `ABC` + `@abstractmethod` |
| Null-safety | Pointer + assert | `Optional[T]` + None check |
| Callbacks | `std::function<void(T)>` | `Callable[[T], None]` |
| Immutable snapshot | `const` member / nested class | `@dataclass(frozen=True)` |
| Double dispatch | `accept()`/`visit()` virtual calls | `visit_TYPE()` naming + `accept()` |
