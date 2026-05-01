"""
=============================================================================
CHAPTER 12: ERROR HANDLING & DEBUGGING
=============================================================================
Python's exception system is a core part of the language — not just for errors,
but as a control flow mechanism (EAFP principle).

WHY error handling is critical:
- Programs interact with unpredictable external systems
- Good error handling makes failures diagnosable
- Bad error handling hides bugs and corrupts data
- EAFP is the Pythonic way (try/except over if/else checks)

PRINCIPLE: Exceptions should be exceptional but EXPECTED.
           Handle what you can, let the rest propagate.

=============================================================================
"""

# =============================================================================
# 12.1 EXCEPTION HIERARCHY
# =============================================================================
"""
WHAT: Python's exception classes form a hierarchy. Catching a parent
      class catches all its children.

KEY HIERARCHY:
BaseException
├── SystemExit          ← sys.exit() — don't catch!
├── KeyboardInterrupt   ← Ctrl+C — don't catch!
├── GeneratorExit       ← generator.close() — don't catch!
└── Exception           ← catch THIS as the broadest level
    ├── StopIteration
    ├── ArithmeticError
    │   ├── ZeroDivisionError
    │   └── OverflowError
    ├── LookupError
    │   ├── IndexError
    │   └── KeyError
    ├── OSError
    │   ├── FileNotFoundError
    │   ├── PermissionError
    │   └── ConnectionError
    ├── ValueError
    ├── TypeError
    ├── AttributeError
    ├── RuntimeError
    └── ...

RULE: Never catch BaseException. Catch Exception at most.
      Prefer catching specific exceptions.
"""

# Demonstrating hierarchy
try:
    d = {}
    d["missing_key"]
except LookupError as e:
    # Catches both KeyError and IndexError
    print(f"Lookup failed: {type(e).__name__}: {e}")


# =============================================================================
# 12.2 CUSTOM EXCEPTIONS
# =============================================================================
"""
WHAT: Create your own exception classes for domain-specific errors.

WHY custom exceptions:
- More meaningful error messages
- Callers can catch specific errors vs broad ones
- Add context and data to exceptions
- Document what can go wrong in your API

BEST PRACTICES:
- Inherit from Exception (or a more specific built-in)
- End class name with "Error"
- Keep them simple — minimal __init__
- Create a base exception for your library
"""

# Base exception for your application/library
class AppError(Exception):
    """Base exception for all application errors."""
    pass

class ValidationError(AppError):
    """Raised when input validation fails."""

    def __init__(self, field: str, message: str, value=None):
        self.field = field
        self.message = message
        self.value = value
        super().__init__(f"Validation error on '{field}': {message}")

class NotFoundError(AppError):
    """Raised when a requested resource doesn't exist."""

    def __init__(self, resource_type: str, identifier):
        self.resource_type = resource_type
        self.identifier = identifier
        super().__init__(f"{resource_type} with id={identifier} not found")

class AuthenticationError(AppError):
    """Raised when authentication fails."""
    pass

# Usage
def get_user(user_id: int):
    users = {1: "Alice", 2: "Bob"}
    if user_id not in users:
        raise NotFoundError("User", user_id)
    return users[user_id]

def validate_age(age):
    if not isinstance(age, int):
        raise ValidationError("age", "must be an integer", age)
    if age < 0 or age > 150:
        raise ValidationError("age", "must be between 0 and 150", age)

# Catching custom exceptions
try:
    get_user(999)
except NotFoundError as e:
    print(f"Not found: {e.resource_type} #{e.identifier}")
except AppError as e:
    # Catch any app error as fallback
    print(f"App error: {e}")


# =============================================================================
# 12.3 EXCEPTION CHAINING
# =============================================================================
"""
WHAT: `raise ... from ...` links exceptions, showing the root cause.

WHY exception chaining:
- Preserves the original exception context
- Shows the CAUSE of the error (not just the symptoms)
- Two types:
  - Explicit: `raise NewError() from original` (intentional wrapping)
  - Implicit: raise inside except block (automatic __context__)

WHEN to use:
- Wrapping low-level errors into domain errors
- Adding context while preserving the original traceback
"""

import json

def load_config(path: str) -> dict:
    """Load config with proper exception chaining."""
    try:
        with open(path, "r") as f:
            return json.load(f)
    except FileNotFoundError as e:
        # Explicit chain: shows this was caused by file not found
        raise AppError(f"Config file missing: {path}") from e
    except json.JSONDecodeError as e:
        raise AppError(f"Invalid JSON in config: {path}") from e

# The traceback will show:
# FileNotFoundError: [Errno 2] No such file or directory: 'missing.json'
#
# The above exception was the direct cause of the following exception:
#
# AppError: Config file missing: missing.json

# Suppress chaining (when original exception is irrelevant):
# raise NewError("message") from None


# =============================================================================
# 12.4 EXCEPTION HANDLING PATTERNS
# =============================================================================
"""
Common patterns for handling exceptions in real applications.
"""

# Pattern 1: Try/except/else/finally
def read_and_process(filepath):
    """Full try statement demonstrating all clauses."""
    try:
        f = open(filepath, "r", encoding="utf-8")
        data = f.read()
    except FileNotFoundError:
        print(f"File {filepath} not found, using defaults")
        data = "{}"
    except PermissionError:
        print(f"No permission to read {filepath}")
        data = "{}"
    else:
        # ONLY runs if NO exception occurred
        # Good place for success-path logic
        print(f"Successfully read {len(data)} characters")
    finally:
        # ALWAYS runs — cleanup
        # Note: better to use `with` statement for files
        if 'f' in locals() and not f.closed:
            f.close()

# Pattern 2: EAFP (Easier to Ask Forgiveness than Permission)
"""
PYTHONIC approach: just try it, handle failure.
Better than LBYL (Look Before You Leap): check everything first.
"""
# LBYL (non-Pythonic, also has race conditions):
import os
# if os.path.exists(filepath):
#     if os.access(filepath, os.R_OK):
#         with open(filepath) as f:
#             data = f.read()

# EAFP (Pythonic):
try:
    with open("config.json", "r") as f:
        config = json.load(f)
except (FileNotFoundError, json.JSONDecodeError):
    config = {}  # Default config

# Pattern 3: Specific exception handling
def convert_to_int(value):
    """Handle specific cases differently."""
    try:
        return int(value)
    except (TypeError, ValueError) as e:
        # Handle multiple exception types
        print(f"Cannot convert {value!r}: {e}")
        return None

# Pattern 4: Re-raise after logging
import logging

logger = logging.getLogger(__name__)

def critical_operation():
    """Log the error, then let it propagate."""
    try:
        result = 1 / 0
    except ZeroDivisionError:
        logger.exception("Critical operation failed!")  # Logs traceback
        raise  # Re-raise the SAME exception (preserves traceback)

# Pattern 5: Context manager for error handling
from contextlib import suppress

# Ignore specific exceptions cleanly:
with suppress(FileNotFoundError):
    os.remove("might_not_exist.txt")
# Equivalent to:
# try:
#     os.remove("might_not_exist.txt")
# except FileNotFoundError:
#     pass


# =============================================================================
# 12.5 LOGGING
# =============================================================================
"""
WHAT: The logging module provides structured, leveled output for production code.

WHY logging over print():
- Levels: DEBUG, INFO, WARNING, ERROR, CRITICAL
- Configurable output (file, console, network)
- Timestamps, source info, formatting
- Can be disabled without removing code
- Thread-safe

WHEN to use each level:
- DEBUG: Detailed information for diagnosing problems
- INFO: Confirmation that things work as expected
- WARNING: Something unexpected but not an error
- ERROR: Something failed, but application continues
- CRITICAL: Application cannot continue
"""

import logging

# Basic configuration
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)

# Create logger for this module
logger = logging.getLogger(__name__)

# Usage
logger.debug("Detailed diagnostic info")
logger.info("Operation completed successfully")
logger.warning("Disk space running low")
logger.error("Failed to connect to database")
logger.critical("Application shutting down!")

# Logging exceptions (includes traceback)
try:
    1 / 0
except ZeroDivisionError:
    logger.exception("Math error occurred")
    # This logs the full traceback at ERROR level

# Lazy formatting (don't format string if level is disabled)
# GOOD — string formatting only happens if DEBUG is enabled:
logger.debug("Processing item %s of %d", "widget", 100)
# BAD — always formats, even if DEBUG is off:
# logger.debug(f"Processing item {'widget'} of {100}")

# Structured logging with extra data
logger.info("User login", extra={"user_id": 123, "ip": "192.168.1.1"})


# =============================================================================
# 12.6 ASSERTIONS AND __debug__
# =============================================================================
"""
WHAT: `assert` checks assumptions during development.
      Disabled with `python -O` (optimize flag) in production!

WHEN to use assert:
- Internal invariants (things that should NEVER happen if code is correct)
- Development-time sanity checks
- Documentation of assumptions

WHEN NOT to use assert:
- Input validation (user data) — use exceptions instead!
- Security checks — can be disabled!
- Production error handling — use proper exceptions
"""

# GOOD: Assert internal invariants
def binary_search(sorted_list, target):
    """Assert precondition (documentation + debug check)."""
    assert sorted_list == sorted(sorted_list), "List must be sorted!"
    # ... implementation

# GOOD: Assert postconditions
def calculate_discount(price, percent):
    result = price * (1 - percent / 100)
    assert result >= 0, f"Discount produced negative price: {result}"
    return result

# BAD: Don't use assert for input validation!
# assert user_input > 0, "Must be positive"  # DISABLED with -O flag!
# GOOD:
# if user_input <= 0:
#     raise ValueError("Must be positive")

# BAD: Don't use assert for security checks!
# assert user.is_admin, "Not authorized"  # CAN BE BYPASSED!
# GOOD:
# if not user.is_admin:
#     raise PermissionError("Not authorized")


# =============================================================================
# 12.7 DEBUGGING WITH PDB AND BREAKPOINT()
# =============================================================================
"""
WHAT: pdb is Python's built-in debugger. breakpoint() (3.7+) is the
      modern way to invoke it.

WHY use pdb:
- Inspect variables at any point in execution
- Step through code line by line
- Evaluate expressions in context
- Set conditional breakpoints

HOW to use:
1. Add `breakpoint()` where you want to pause
2. Run your script normally
3. Use pdb commands to navigate

COMMON PDB COMMANDS:
- n (next): execute next line
- s (step): step into function call
- c (continue): run until next breakpoint
- p expr (print): print expression value
- pp expr: pretty-print
- l (list): show surrounding code
- w (where): show call stack
- q (quit): exit debugger
- b line (break): set breakpoint at line
- h (help): show commands
"""

def debug_example():
    """Example showing breakpoint usage."""
    data = [1, 2, 3, 4, 5]
    total = 0

    for item in data:
        total += item
        # Uncomment to debug:
        # breakpoint()  # Execution pauses here
        # You can then inspect: p total, p item, p data

    return total

# Conditional breakpoint
def process_items(items):
    for i, item in enumerate(items):
        # Only break on item 42 or index 100:
        # if item == 42 or i == 100:
        #     breakpoint()
        pass

# Environment variable to disable breakpoints:
# PYTHONBREAKPOINT=0 python script.py  → breakpoints are no-ops
# PYTHONBREAKPOINT=ipdb.set_trace python script.py  → use ipdb instead


# =============================================================================
# 12.8 WARNINGS MODULE
# =============================================================================
"""
WHAT: Warnings are for non-fatal issues that should be noticed but don't
      stop execution (deprecations, potential problems).

WHY warnings instead of exceptions:
- Don't want to break existing code
- Alert developers about deprecated usage
- Signal potential issues (resource leaks, performance)

WHEN to use:
- Deprecating functions/APIs
- Potential misconfiguration
- Performance issues that aren't errors
"""

import warnings

# Issue a warning
def old_function():
    """Deprecated function."""
    warnings.warn(
        "old_function() is deprecated, use new_function() instead",
        DeprecationWarning,
        stacklevel=2,  # Show caller's location, not this line
    )
    return "old result"

# Control warning behavior
# warnings.filterwarnings("ignore", category=DeprecationWarning)  # Suppress
# warnings.filterwarnings("error", category=UserWarning)  # Turn into exception

# In tests, assert warnings are raised:
# with warnings.catch_warnings(record=True) as w:
#     warnings.simplefilter("always")
#     old_function()
#     assert len(w) == 1
#     assert issubclass(w[0].category, DeprecationWarning)


# =============================================================================
# SUMMARY: ERROR HANDLING BEST PRACTICES
# =============================================================================
"""
1. Catch SPECIFIC exceptions (never bare `except:` or `except Exception:`)
2. Use EAFP over LBYL (try/except over precondition checks)
3. Create custom exception hierarchies for your application
4. Use exception chaining (`raise ... from ...`) to preserve context
5. Use `else` clause for success-path code (only runs if no exception)
6. Use `finally` or context managers for cleanup (prefer context managers)
7. Use logging, not print(), for production error reporting
8. Use assert for internal invariants ONLY (never for input validation)
9. Use breakpoint() for debugging (not print-debugging)
10. Use warnings for non-fatal issues (deprecation, potential problems)
11. Let exceptions propagate unless you can meaningfully handle them
12. Always include enough context in error messages to diagnose the issue
"""
