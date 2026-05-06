# Design Pattern Recognition Cheat-Sheet

A single-page reference. For each pattern: **what it solves**, **the one thing that makes it unique**, and **the recognition trigger** (the phrase or code smell that tells you "this is that pattern").

---

## Creational — *how objects are created*

| Pattern              | Solves                                        | Unique mechanic                                    | Recognize it when…                                                            |
| -------------------- | --------------------------------------------- | -------------------------------------------------- | ----------------------------------------------------------------------------- |
| **Singleton**        | One and only one instance                     | `static` local variable / private ctor             | You see "only one X should ever exist" or global-but-thread-safe access       |
| **Factory Method**   | Subclass decides which product to create      | Virtual `create()` in base class                   | Base class calls its own abstract method to make an object                    |
| **Abstract Factory** | Create *families* of related objects          | Factory of factories                               | You need Product A and Product B to always match a theme/style                |
| **Builder**          | Construct complex objects step by step        | Separate `Builder` with fluent setters + `build()` | Object needs many optional parts assembled in a specific order                |
| **Prototype**        | Clone objects without coupling to their class | `clone()` / copy constructor                       | You need a copy of an existing runtime object whose exact type you don't know |

---

## Structural — *how objects are composed*

| Pattern       | Solves                                      | Unique mechanic                                                 | Recognize it when…                                                         |
| ------------- | ------------------------------------------- | --------------------------------------------------------------- | -------------------------------------------------------------------------- |
| **Adapter**   | Incompatible interface mismatch             | Wrapper that translates one interface to another                | You have a class that *almost* fits but the interface is wrong             |
| **Bridge**    | Avoid explosion of subclass combinations    | Abstraction holds a *pointer* to implementation                 | Two independent axes of variation (e.g. shape × renderer)                  |
| **Composite** | Treat single items and groups uniformly     | Tree of nodes all sharing one interface                         | You build a tree/hierarchy where leaf and container are interchangeable    |
| **Decorator** | Add behavior at runtime without subclassing | Wraps an object of the *same interface*                         | You want to stack optional responsibilities (logging, caching, validation) |
| **Facade**    | Simplify a complex subsystem                | One class with high-level methods hiding many subsystem calls   | You want a "one button does it all" entry point into a messy subsystem     |
| **Flyweight** | Efficiently share many fine-grained objects | Splits *intrinsic* (shared) vs *extrinsic* (per-instance) state | You have thousands of similar objects consuming too much memory            |
| **Proxy**     | Control access to another object            | Same interface as the real object; delegates to it              | You need lazy init, access control, caching, or logging around an object   |

---

## Behavioural — *how objects communicate*

| Pattern                     | Solves                                                    | Unique mechanic                                                       | Recognize it when…                                                             |
| --------------------------- | --------------------------------------------------------- | --------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| **Chain of Responsibility** | Decouple sender from receiver                             | Each handler either handles or passes to next                         | Multiple handlers might process a request; you don't know which one upfront    |
| **Command**                 | Encapsulate a request as an object                        | `execute()` / `undo()` on a command object                            | You need undo/redo, request queuing, or macro recording                        |
| **Interpreter**             | Evaluate sentences in a language                          | Recursive expression tree with `evaluate()`                           | You're parsing/evaluating a small grammar or expression (DSL, config)          |
| **Iterator**                | Traverse a collection without exposing internals          | `hasNext()` / `next()` or C++ `begin()`/`end()`                       | You want `for (auto x : collection)` for a custom container                    |
| **Mediator**                | Reduce direct coupling between many objects               | Central hub object; components only know the mediator                 | Many objects talk to each other — replacing the mesh with a star topology      |
| **Memento**                 | Save and restore an object's internal state               | Opaque snapshot object; only the originator can read it               | You need undo history without breaking encapsulation                           |
| **Observer**                | Notify many dependents when state changes                 | Subject keeps a list of observers; calls `update()` on them           | Event system, pub/sub, "when X changes, Y and Z should react"                  |
| **State**                   | Object changes behavior when its internal state changes   | State object replaces itself in the context                           | A class has big `if/switch` blocks on an internal mode/status flag             |
| **Strategy**                | Swap algorithms or behaviors at runtime                   | Interface injected into a context; context delegates to it            | You pick one of several interchangeable algorithms at construction or runtime  |
| **Template Method**         | Fix an algorithm's skeleton; let subclasses fill in steps | `public final` method calls `protected virtual` hooks                 | Base class defines order of steps; subclasses customize individual steps       |
| **Visitor**                 | Add operations to objects without modifying them          | Double-dispatch: `element.accept(visitor)` → `visitor.visit(element)` | You want to add new operations to a stable class hierarchy without touching it |

---

## Quick confusion guide — patterns that look alike

| Confused pair                          | Key difference                                                                                              |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| **Adapter vs Decorator**               | Adapter *changes* the interface; Decorator *keeps* the same interface                                       |
| **Decorator vs Proxy**                 | Decorator adds behavior; Proxy controls access (lazy, auth, cache)                                          |
| **Strategy vs State**                  | Strategy is chosen externally and rarely changes; State transitions itself internally                       |
| **Strategy vs Template Method**        | Strategy uses composition (inject the algorithm); Template Method uses inheritance (override a step)        |
| **Factory Method vs Abstract Factory** | Factory Method makes *one* product type; Abstract Factory makes *a family* of coordinated products          |
| **Composite vs Decorator**             | Composite is a tree with multiple children; Decorator wraps exactly *one* object                            |
| **Observer vs Mediator**               | Observer is pub/sub (subjects don't know observers); Mediator has a central hub that knows all participants |
| **Command vs Strategy**                | Command captures *what to do once* (with undo); Strategy captures *how to do it always*                     |
| **Bridge vs Adapter**                  | Bridge is designed upfront to separate two axes; Adapter is a retrofit to fix an existing mismatch          |
| **Proxy vs Facade**                    | Proxy has the *same interface* as the real object; Facade introduces a *new, simpler* interface             |

---

## One-word memory hooks

```
Singleton    → UNIQUE
Factory Mth  → DELEGATE-CREATE
Abst Factory → FAMILY
Builder      → STEP-BY-STEP
Prototype    → CLONE

Adapter      → TRANSLATE
Bridge       → SPLIT-AXIS
Composite    → TREE
Decorator    → WRAP-SAME
Facade       → SIMPLIFY
Flyweight    → SHARE
Proxy        → GUARD

Chain        → PASS-ALONG
Command      → UNDO
Interpreter  → GRAMMAR
Iterator     → TRAVERSE
Mediator     → HUB
Memento      → SNAPSHOT
Observer     → NOTIFY
State        → MODE-SWITCH
Strategy     → SWAP-ALGO
Template Mth → SKELETON
Visitor      → NEW-OP
```
