"""
=============================================================================
CHAPTER 10: MODULES & PACKAGES
=============================================================================
Python's module system is how you organize code into reusable, maintainable units.
Understanding it deeply prevents import confusion, circular imports, and packaging issues.

WHY modules matter:
- Namespace isolation (avoid name collisions)
- Code reuse across projects
- Lazy loading (import what you need)
- Clear dependency graph

HOW Python finds modules (in order):
1. sys.modules (already imported modules cache)
2. Built-in modules (sys, os, etc.)
3. sys.path entries: current directory, PYTHONPATH, site-packages

=============================================================================
"""

# =============================================================================
# 10.1 IMPORTING — ALL THE WAYS
# =============================================================================
"""
WHAT: Different import syntaxes and their trade-offs.

IMPORT STYLES (from most to least recommended):
1. import module             → access as module.thing (most explicit)
2. from module import thing  → direct access to thing (selective)
3. from module import *      → import all public names (AVOID!)
4. import module as alias    → shorten long names
"""

# STYLE 1: import module — BEST for namespacing
import os
import sys
import json

# Clear where each function comes from:
path = os.path.join("/usr", "local")
data = json.loads('{"key": "value"}')

# STYLE 2: from module import specific_things
from pathlib import Path
from collections import defaultdict, Counter
from typing import Optional, TypeVar

# Acceptable when:
# - The name is distinctive enough (Path, Counter)
# - You use it many times (would be verbose with module prefix)

# STYLE 3: from module import * — AVOID!
# from os import *  # DON'T! Pollutes namespace, hides dependencies
# You won't know where names come from, and you might shadow builtins

# STYLE 4: import as — for long module names or conventions
import numpy as np               # Community convention
import pandas as pd              # Community convention
import matplotlib.pyplot as plt  # Community convention

# PYTHONIC IMPORT ORDER (PEP 8):
# 1. Standard library imports
# 2. Third-party library imports
# 3. Local application imports
# (blank line between each group)

# Example:
"""
import os
import sys
from pathlib import Path

import requests
import numpy as np
from fastapi import FastAPI

from myapp.models import User
from myapp.utils import validate_email
"""


# =============================================================================
# 10.2 __init__.py AND PACKAGE STRUCTURE
# =============================================================================
"""
WHAT: A package is a directory with __init__.py (or without — namespace package).
      __init__.py runs when the package is imported.

WHY __init__.py exists:
- Marks directory as a Python package
- Controls what's imported with `from package import *`
- Can re-export submodule contents for a clean public API
- Runs initialization code (connections, config loading)

PACKAGE STRUCTURE EXAMPLE:
    mypackage/
    ├── __init__.py        ← Package initialization
    ├── __main__.py        ← Enables `python -m mypackage`
    ├── core.py
    ├── utils.py
    ├── models/
    │   ├── __init__.py
    │   ├── user.py
    │   └── product.py
    └── services/
        ├── __init__.py
        ├── auth.py
        └── payment.py
"""

# __init__.py patterns:

# Pattern 1: Re-export for clean public API
# In mypackage/__init__.py:
"""
from mypackage.core import Engine
from mypackage.models.user import User
from mypackage.models.product import Product

__all__ = ["Engine", "User", "Product"]
"""
# Now users can do: from mypackage import User
# Instead of: from mypackage.models.user import User

# Pattern 2: Lazy imports for fast startup
"""
def get_heavy_module():
    from mypackage import heavy_module
    return heavy_module
"""

# Pattern 3: Version and metadata
"""
__version__ = "1.2.3"
__author__ = "Your Name"
"""


# =============================================================================
# 10.3 __all__ AND PUBLIC API
# =============================================================================
"""
WHAT: __all__ is a list of names that defines the public API of a module.
      It controls what `from module import *` exports.

WHY use __all__:
- Explicitly declares public API
- Prevents internal helpers from leaking
- Documentation: readers know what's intended for use
- IDE support: autocompletion shows public API

CONVENTION: Names starting with _ are "private" (convention, not enforced)
"""

# In a module file (e.g., utils.py):
__all__ = ["public_function", "PublicClass"]  # Only these are "public"

def public_function():
    """Part of the public API."""
    return _helper()

def _helper():
    """Private by convention (leading underscore). Not in __all__."""
    return "internal"

class PublicClass:
    """Exported."""
    pass

class _InternalClass:
    """Not exported (leading underscore convention)."""
    pass


# =============================================================================
# 10.4 RELATIVE IMPORTS
# =============================================================================
"""
WHAT: Import modules relative to current package position using dots.
      . = current package, .. = parent package, ... = grandparent, etc.

WHEN to use relative imports:
- Within a package, to reference sibling or sub-modules
- Keeps package self-contained (can be moved/renamed)
- Clear that it's an internal import (not third-party)

WHEN NOT to use:
- Top-level scripts (relative imports don't work in scripts run directly)
- When it makes the import harder to understand

NOTE: Relative imports only work within packages (not in standalone scripts).
"""

# Given package structure:
# mypackage/
# ├── __init__.py
# ├── module_a.py
# ├── module_b.py
# └── subpackage/
#     ├── __init__.py
#     └── module_c.py

# In module_b.py:
# from . import module_a           # Import sibling module
# from .module_a import some_func  # Import specific name from sibling

# In subpackage/module_c.py:
# from .. import module_a          # Import from parent package
# from ..module_b import helper    # Import from parent's sibling


# =============================================================================
# 10.5 DYNAMIC IMPORTS AND IMPORTLIB
# =============================================================================
"""
WHAT: Import modules programmatically at runtime (by string name).

WHEN to use:
- Plugin systems (load modules discovered at runtime)
- Optional dependencies (import if available, fallback otherwise)
- Testing (mock imports)
- Configuration-driven behavior
"""

import importlib

# Import a module by string name
module_name = "json"
json_module = importlib.import_module(module_name)
print(json_module.dumps({"hello": "world"}))

# Import submodule
os_path = importlib.import_module("os.path")
print(os_path.exists("/tmp"))

# Optional dependency pattern
try:
    import ujson as json_lib  # Fast JSON if available
except ImportError:
    import json as json_lib   # Fallback to stdlib

# Plugin loading pattern
def load_plugin(plugin_name: str):
    """Dynamically load a plugin module."""
    try:
        module = importlib.import_module(f"plugins.{plugin_name}")
        return module.Plugin()  # Convention: each plugin exposes Plugin class
    except ImportError as e:
        raise RuntimeError(f"Plugin {plugin_name} not found: {e}")


# =============================================================================
# 10.6 sys.path AND MODULE RESOLUTION
# =============================================================================
"""
WHAT: sys.path is the list of directories Python searches for modules.

HOW Python resolves imports (in order):
1. Check sys.modules (already imported — cached)
2. Check built-in modules
3. Search directories in sys.path:
   - Directory of the running script (or current dir for interactive)
   - PYTHONPATH environment variable entries
   - Standard library directories
   - site-packages (pip installed packages)

MODIFYING sys.path (use sparingly):
"""

import sys
print(f"Python path: {sys.path[:3]}...")  # Show first 3 entries

# Add a directory to path (e.g., for project structure)
# sys.path.insert(0, "/path/to/my/modules")

# BETTER: Use proper package installation (pip install -e .)
# or PYTHONPATH environment variable
# or pyproject.toml with build system


# =============================================================================
# 10.7 CIRCULAR IMPORTS AND SOLUTIONS
# =============================================================================
"""
WHAT: Module A imports Module B, and Module B imports Module A.
      This causes ImportError or partially initialized modules.

WHY circular imports happen:
- Poor module boundaries
- Two modules that are too tightly coupled
- "God module" that everything depends on

SOLUTIONS (in order of preference):
1. Restructure: Move shared code to a third module
2. Import at function level (deferred import)
3. Import the module, not names (import a vs from a import thing)
4. Move import to bottom of file (rarely appropriate)
"""

# Problem:
# models.py: from services import validate
# services.py: from models import User
# → Circular!

# Solution 1: Extract shared code
# common.py: shared types/interfaces
# models.py: from common import ...
# services.py: from common import ...

# Solution 2: Deferred import (import inside function)
"""
# services.py
def get_user():
    from models import User  # Import when needed, not at module level
    return User(...)
"""

# Solution 3: Import module, not names
"""
# Instead of: from models import User
import models
# Use as: models.User
"""

# Solution 4: TYPE_CHECKING block (for type hints only)
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    # Only imported during static analysis, NOT at runtime
    from models import User  # type: ignore

def process_user(user: "User") -> None:  # Forward reference (string)
    pass


# =============================================================================
# 10.8 __main__.py AND -m FLAG
# =============================================================================
"""
WHAT:
- __main__.py: entry point when running a package with `python -m package`
- if __name__ == "__main__": guard for code that runs only when script is executed directly

WHY __main__.py:
- Makes packages executable: `python -m mypackage`
- Cleaner than having a separate CLI script
- Works with installed packages

WHY the `if __name__ == "__main__":` guard:
- Prevents code from running when module is imported (only on direct execution)
- Essential for multiprocessing (Windows needs it)
- Good practice for any module that can be both imported and run
"""

# Typical __main__.py for a package:
"""
# mypackage/__main__.py
from mypackage.cli import main

if __name__ == "__main__":
    main()
"""

# Then run with: python -m mypackage

# The __name__ == "__main__" pattern:
def main():
    """Application entry point."""
    print("Running as main program")

if __name__ == "__main__":
    # Only executes when this file is run directly
    # NOT when imported as a module
    main()


# =============================================================================
# SUMMARY: MODULES & PACKAGES BEST PRACTICES
# =============================================================================
"""
1. Import order: stdlib → third-party → local (PEP 8)
2. Prefer `import module` over `from module import *`
3. Use `from module import name` for frequently used, distinctive names
4. Define __all__ in all public modules
5. Use leading underscore for private names (_helper, _internal_class)
6. Avoid circular imports — restructure code or use deferred imports
7. Use TYPE_CHECKING for type-hint-only imports
8. Always have `if __name__ == "__main__":` guard in executable modules
9. Use relative imports within packages (.module, ..parent)
10. Install your package in development mode: pip install -e .
11. Prefer importlib over __import__ for dynamic imports
12. Keep __init__.py simple — re-export public API, minimal logic
"""
