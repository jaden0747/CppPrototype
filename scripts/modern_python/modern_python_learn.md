# Modern Python Features & Pythonic Best Practices

A comprehensive learning plan covering Python's features and idiomatic practices.

## File Index

| # | Topic | File |
|---|-------|------|
| 01 | Core Language Fundamentals | `01_core_fundamentals.py` |
| 02 | Data Structures | `02_data_structures.py` |
| 03 | Control Flow | `03_control_flow.py` |
| 04 | Functions | `04_functions.py` |
| 05 | Comprehensions & Generators | `05_comprehensions_generators.py` |
| 06 | Object-Oriented Programming | `06_oop.py` |
| 07 | Decorators & Metaprogramming | `07_decorators_metaprogramming.py` |
| 08 | Iterators & Itertools | `08_iterators_itertools.py` |
| 09 | Type Hints & Static Typing | `09_type_hints.py` |
| 10 | Modules & Packages | `10_modules_packages.py` |
| 11 | File I/O & Serialization | `11_file_io_serialization.py` |
| 12 | Error Handling & Debugging | `12_error_handling.py` |
| 13 | Concurrency & Parallelism | `13_concurrency.py` |
| 14 | Functional Programming | `14_functional_programming.py` |
| 15 | Standard Library Highlights | `15_standard_library.py` |
| 16 | Testing | `16_testing.py` |
| 17 | Pythonic Idioms & Best Practices | `17_pythonic_idioms.py` |
| 18 | Code Style & Tooling | `18_code_style_tooling.py` |
| 19 | Performance & Optimization | `19_performance.py` |
| 20 | Modern Python (3.8–3.13+) | `20_modern_python.py` |
| 21 | Design Patterns in Python | `21_design_patterns.py` |

---

## 1. Core Language Fundamentals

- 1.1 Variables and Dynamic Typing
- 1.2 Numeric Types (int, float, complex, decimal, fractions)
- 1.3 Strings (f-strings, raw strings, byte strings, multiline)
- 1.4 Boolean and None
- 1.5 Type Coercion and Truthiness
- 1.6 Operators (arithmetic, bitwise, walrus `:=`, unpacking `*` / `**`)
- 1.7 Comments and Docstrings

## 2. Data Structures

- 2.1 Lists (slicing, copying, sorting)
- 2.2 Tuples and Named Tuples
- 2.3 Dictionaries (merging `|`, dict comprehensions, defaultdict, OrderedDict)
- 2.4 Sets and Frozensets
- 2.5 Deque, Counter, ChainMap
- 2.6 Arrays and Bytearrays
- 2.7 Heaps and Priority Queues
- 2.8 Structural Pattern Matching with Data Structures

## 3. Control Flow

- 3.1 if/elif/else
- 3.2 Ternary (conditional) Expressions
- 3.3 for Loops and the `else` Clause
- 3.4 while Loops
- 3.5 Match/Case (Structural Pattern Matching, 3.10+)
- 3.6 Exception Handling (try/except/else/finally)
- 3.7 Exception Groups and `except*` (3.11+)
- 3.8 Context Managers (`with` statement)

## 4. Functions

- 4.1 Defining Functions (def, return)
- 4.2 Arguments (positional, keyword, default, `*args`, `**kwargs`)
- 4.3 Positional-Only `/` and Keyword-Only `*` Parameters
- 4.4 Lambda Functions
- 4.5 Closures and Nonlocal
- 4.6 Recursion and Tail-Call Patterns
- 4.7 Function Annotations and Type Hints
- 4.8 Docstrings and `help()`
- 4.9 First-Class Functions and Higher-Order Functions

## 5. Comprehensions & Generators

- 5.1 List Comprehensions
- 5.2 Dict Comprehensions
- 5.3 Set Comprehensions
- 5.4 Generator Expressions
- 5.5 Generator Functions (`yield`, `yield from`)
- 5.6 Coroutines with `send()` and `throw()`
- 5.7 Infinite Generators and Lazy Evaluation

## 6. Object-Oriented Programming

- 6.1 Classes and Instances
- 6.2 `__init__`, `__new__`, `__del__`
- 6.3 Instance, Class, and Static Methods
- 6.4 Properties (`@property`, getters/setters)
- 6.5 Inheritance and MRO (Method Resolution Order)
- 6.6 Multiple Inheritance and Mixins
- 6.7 Abstract Base Classes (ABC)
- 6.8 Dunder/Magic Methods (`__repr__`, `__str__`, `__eq__`, `__hash__`, etc.)
- 6.9 Operator Overloading
- 6.10 Slots (`__slots__`)
- 6.11 Dataclasses (`@dataclass`)
- 6.12 Enums (`enum.Enum`, `IntEnum`, `StrEnum`)
- 6.13 Protocols and Structural Subtyping (3.8+)

## 7. Decorators & Metaprogramming

- 7.1 Function Decorators
- 7.2 Class Decorators
- 7.3 Decorators with Arguments
- 7.4 `functools.wraps`
- 7.5 Built-in Decorators (`@staticmethod`, `@classmethod`, `@property`, `@abstractmethod`)
- 7.6 `functools.lru_cache` / `functools.cache`
- 7.7 Metaclasses (`type`, `__init_subclass__`)
- 7.8 Descriptors (`__get__`, `__set__`, `__delete__`)
- 7.9 `__class_getitem__` and Generic Classes

## 8. Iterators & Itertools

- 8.1 Iterator Protocol (`__iter__`, `__next__`)
- 8.2 Custom Iterators
- 8.3 `itertools` (chain, product, permutations, combinations, groupby, starmap, etc.)
- 8.4 `zip`, `zip_longest`, `enumerate`
- 8.5 `map`, `filter`, `reduce`
- 8.6 `sorted` with key functions
- 8.7 `any()`, `all()`
- 8.8 Infinite Iterators (count, cycle, repeat)

## 9. Type Hints & Static Typing

- 9.1 Basic Type Annotations (int, str, list, dict)
- 9.2 `Optional`, `Union`, `|` syntax (3.10+)
- 9.3 `TypeAlias` and `type` statement (3.12+)
- 9.4 Generics (`TypeVar`, `Generic`, `ParamSpec`)
- 9.5 `Literal`, `Final`, `ClassVar`
- 9.6 `TypedDict`
- 9.7 `Protocol` for Structural Typing
- 9.8 `Annotated`
- 9.9 `TypeGuard` and `TypeNarrowing`
- 9.10 `Self` type (3.11+)
- 9.11 Variance (`covariant`, `contravariant`)
- 9.12 Using mypy / pyright

## 10. Modules & Packages

- 10.1 Importing (import, from, as, relative imports)
- 10.2 `__init__.py` and Package Structure
- 10.3 `__all__` and Public API
- 10.4 Namespace Packages
- 10.5 `importlib` and Dynamic Imports
- 10.6 `sys.path` and Module Resolution
- 10.7 Circular Imports and Solutions
- 10.8 `__main__.py` and `-m` flag

## 11. File I/O & Serialization

- 11.1 Reading/Writing Text Files
- 11.2 Reading/Writing Binary Files
- 11.3 `pathlib.Path` (modern file paths)
- 11.4 `os` and `shutil` for File Operations
- 11.5 JSON (`json` module)
- 11.6 CSV (`csv` module)
- 11.7 TOML (`tomllib`, 3.11+)
- 11.8 Pickle (serialization/deserialization)
- 11.9 `struct` for Binary Data
- 11.10 `tempfile` and Temporary Files

## 12. Error Handling & Debugging

- 12.1 Exception Hierarchy
- 12.2 Custom Exceptions
- 12.3 Exception Chaining (`from`)
- 12.4 `traceback` Module
- 12.5 `warnings` Module
- 12.6 Assertions and `__debug__`
- 12.7 `logging` Module
- 12.8 `pdb` and Debugging
- 12.9 `breakpoint()` (3.7+)

## 13. Concurrency & Parallelism

- 13.1 Threading (`threading` module)
- 13.2 The GIL (Global Interpreter Lock)
- 13.3 Multiprocessing (`multiprocessing` module)
- 13.4 `concurrent.futures` (ThreadPoolExecutor, ProcessPoolExecutor)
- 13.5 `asyncio` Basics (async/await)
- 13.6 Async Generators and Async Comprehensions
- 13.7 `asyncio.gather`, `asyncio.wait`, `TaskGroup` (3.11+)
- 13.8 Async Context Managers and Iterators
- 13.9 Synchronization Primitives (Lock, Event, Semaphore, Queue)
- 13.10 `subprocess` Module
- 13.11 Free-threaded Python / No-GIL (3.13+)

## 14. Functional Programming

- 14.1 First-Class Functions
- 14.2 `map`, `filter`, `reduce`
- 14.3 `functools` (partial, reduce, wraps, singledispatch, total_ordering)
- 14.4 `operator` Module
- 14.5 Immutability Patterns
- 14.6 Pure Functions and Side Effects
- 14.7 Function Composition
- 14.8 Pattern Matching as Functional Decomposition

## 15. Standard Library Highlights

- 15.1 `collections` (namedtuple, deque, defaultdict, Counter, OrderedDict, ChainMap)
- 15.2 `dataclasses`
- 15.3 `datetime`, `zoneinfo`, `calendar`
- 15.4 `re` (Regular Expressions)
- 15.5 `math`, `statistics`, `random`
- 15.6 `hashlib`, `secrets`, `hmac`
- 15.7 `copy` (shallow/deep copy)
- 15.8 `contextlib` (contextmanager, suppress, redirect_stdout, ExitStack)
- 15.9 `abc` (Abstract Base Classes)
- 15.10 `typing` and `typing_extensions`
- 15.11 `argparse` and CLI argument parsing
- 15.12 `unittest`, `doctest`
- 15.13 `http.server`, `urllib`, `email`
- 15.14 `sqlite3`
- 15.15 `socket` and Networking Basics
- 15.16 `weakref`
- 15.17 `inspect`
- 15.18 `dis` (bytecode disassembly)

## 16. Testing

- 16.1 `unittest` Framework
- 16.2 `pytest` (fixtures, parametrize, markers)
- 16.3 Mocking (`unittest.mock`, `patch`, `MagicMock`)
- 16.4 `doctest`
- 16.5 Property-Based Testing (`hypothesis`)
- 16.6 Coverage (`coverage.py`)
- 16.7 Test Organization and Best Practices
- 16.8 TDD Workflow

## 17. Pythonic Idioms & Best Practices

- 17.1 The Zen of Python (`import this`)
- 17.2 EAFP vs LBYL
- 17.3 Unpacking and Starred Assignments
- 17.4 Using `enumerate()` Instead of Range + Index
- 17.5 Using `zip()` for Parallel Iteration
- 17.6 Dictionary `.get()` and `.setdefault()`
- 17.7 Truthiness and Falsy Values
- 17.8 Avoiding Mutable Default Arguments
- 17.9 Context Managers for Resource Management
- 17.10 List Comprehensions Over `map`/`filter`
- 17.11 Generator Expressions for Memory Efficiency
- 17.12 Using `collections.defaultdict` and `Counter`
- 17.13 String Joining (`''.join()` over concatenation)
- 17.14 f-strings Over `.format()` and `%`
- 17.15 `pathlib` Over `os.path`
- 17.16 Walrus Operator `:=` for Assignment Expressions
- 17.17 Using `__all__` for Public API
- 17.18 Flat is Better Than Nested
- 17.19 Duck Typing and Protocols
- 17.20 Single Responsibility Functions

## 18. Code Style & Tooling

- 18.1 PEP 8 (Style Guide)
- 18.2 PEP 257 (Docstring Conventions)
- 18.3 Linters (`ruff`, `flake8`, `pylint`)
- 18.4 Formatters (`black`, `ruff format`)
- 18.5 Type Checkers (`mypy`, `pyright`)
- 18.6 `pre-commit` Hooks
- 18.7 Virtual Environments (`venv`, `virtualenv`)
- 18.8 Dependency Management (`pip`, `poetry`, `uv`, `pdm`)
- 18.9 `pyproject.toml` Configuration
- 18.10 Packaging and Distribution (setuptools, wheel, twine)

## 19. Performance & Optimization

- 19.1 Profiling (`cProfile`, `timeit`, `line_profiler`)
- 19.2 Memory Profiling (`tracemalloc`, `memory_profiler`)
- 19.3 `__slots__` for Memory Optimization
- 19.4 `array` vs `list`
- 19.5 String Interning
- 19.6 Caching (`lru_cache`, `cache`)
- 19.7 NumPy for Numerical Performance
- 19.8 C Extensions and `ctypes`
- 19.9 `Cython` and `mypyc`
- 19.10 Lazy Imports and Deferred Computation

## 20. Modern Python (3.8 – 3.13+)

- 20.1 Walrus Operator `:=` (3.8)
- 20.2 Positional-Only Parameters (3.8)
- 20.3 `TypedDict` and `Protocol` (3.8)
- 20.4 Dictionary Merge `|` Operator (3.9)
- 20.5 `str.removeprefix()` / `str.removesuffix()` (3.9)
- 20.6 Type Hinting Generics in Standard Collections (3.9)
- 20.7 Structural Pattern Matching `match/case` (3.10)
- 20.8 Union Type `X | Y` Syntax (3.10)
- 20.9 `ParamSpec` and `Concatenate` (3.10)
- 20.10 Exception Groups and `except*` (3.11)
- 20.11 `Self` Type (3.11)
- 20.12 `tomllib` (3.11)
- 20.13 `TaskGroup` for Structured Concurrency (3.11)
- 20.14 `type` Statement for Type Aliases (3.12)
- 20.15 Generic Syntax `class Foo[T]:` (3.12)
- 20.16 Improved f-string Parsing (3.12)
- 20.17 `override` Decorator (3.12)
- 20.18 Per-Interpreter GIL (3.12)
- 20.19 Free-Threaded CPython (3.13)
- 20.20 Improved Error Messages (3.10–3.13)

## 21. Design Patterns in Python

- 21.1 Singleton
- 21.2 Factory / Abstract Factory
- 21.3 Builder
- 21.4 Strategy (using first-class functions)
- 21.5 Observer
- 21.6 Decorator (structural pattern, not Python decorator)
- 21.7 Iterator
- 21.8 Command
- 21.9 State
- 21.10 Template Method
- 21.11 Dependency Injection
- 21.12 Repository Pattern

## 22. Networking & Web

- 22.1 `requests` / `httpx`
- 22.2 REST API Consumption
- 22.3 `fastapi` Basics
- 22.4 `flask` Basics
- 22.5 WebSockets (`websockets`, `aiohttp`)
- 22.6 GraphQL Clients
- 22.7 `aiohttp` for Async HTTP

## 23. Data & Science Libraries

- 23.1 NumPy Fundamentals
- 23.2 Pandas DataFrames
- 23.3 Polars (modern alternative)
- 23.4 Matplotlib / Seaborn / Plotly
- 23.5 SQLAlchemy ORM
- 23.6 Pydantic (data validation)

## 24. Security Best Practices

- 24.1 Input Validation and Sanitization
- 24.2 Avoiding `eval()` and `exec()`
- 24.3 `secrets` Over `random` for Crypto
- 24.4 Secure Password Hashing (`bcrypt`, `argon2`)
- 24.5 Environment Variables for Secrets
- 24.6 SQL Injection Prevention (parameterized queries)
- 24.7 Dependency Auditing (`pip-audit`, `safety`)

---

> Each section above is a stub to be expanded with examples, explanations, and exercises.
