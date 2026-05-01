"""
=============================================================================
CHAPTER 18: CODE STYLE, TOOLING, AND PROJECT SETUP
=============================================================================
Modern Python development relies on a rich ecosystem of tools for
code quality, dependency management, and project structure.

This chapter covers the practical tooling every Python developer needs.

=============================================================================
"""

# =============================================================================
# 18.1 PEP 8 — STYLE GUIDE HIGHLIGHTS
# =============================================================================
"""
WHAT: PEP 8 is the official Python style guide.
      It defines conventions that all Python code should follow.

KEY RULES:
1. Indentation: 4 spaces (NEVER tabs)
2. Line length: 79 chars (strict), 99 chars (acceptable in modern projects)
3. Blank lines:
   - 2 blank lines around top-level functions/classes
   - 1 blank line between methods
4. Imports:
   - One per line
   - Grouped: stdlib → third-party → local (with blank lines between)
   - Absolute imports preferred over relative
5. Naming:
   - variables/functions: snake_case
   - classes: PascalCase
   - constants: UPPER_SNAKE_CASE
   - private: _leading_underscore
   - "internal": __double_leading (name mangling)
   - magic: __dunder__
6. Strings: consistency matters more than ' vs "
7. Trailing commas: use in multi-line collections/arguments
8. Whitespace:
   - After commas: f(a, b, c)
   - Around operators: x = 1 + 2
   - No space inside brackets: f(x) not f( x )
"""

# === IMPORT ORDERING EXAMPLE ===
# Standard library
import os
import sys
from pathlib import Path

# Third-party
# import requests
# import numpy as np
# from flask import Flask

# Local
# from mypackage import utils
# from mypackage.models import User


# === NAMING EXAMPLES ===
MAX_RETRIES = 3                     # Constant
DEFAULT_TIMEOUT = 30                # Constant

class HttpClient:                   # Class: PascalCase
    """An HTTP client."""

    _session = None                 # Private attribute

    def fetch_data(self, url: str): # Method: snake_case
        """Fetch data from URL."""
        pass

    def _validate_url(self, url):   # Private method
        pass


def calculate_tax(amount: float, rate: float = 0.1) -> float:
    """Calculate tax on amount."""   # Function: snake_case
    return amount * rate


# === TRAILING COMMAS ===
# Makes diffs cleaner (only changed line shows up)
config = {
    "host": "localhost",
    "port": 8080,
    "debug": True,        # ← trailing comma
}

def create_user(
    name: str,
    email: str,
    age: int,
    role: str = "user",   # ← trailing comma
) -> dict:
    pass


# =============================================================================
# 18.2 TYPE CHECKING: MYPY AND PYRIGHT
# =============================================================================
"""
WHAT: Static type checkers analyze your code WITHOUT running it.
      They catch type errors, None safety issues, and more.

WHY:
- Catch bugs before runtime
- Better IDE support (autocomplete, refactoring)
- Self-documenting code
- Safer refactoring

TOOLS:
- mypy: the original, most widely used
- pyright: faster, used by Pylance (VS Code)
- pytype: Google's type checker (more lenient)

HOW TO USE:
    $ pip install mypy
    $ mypy src/               # Check all files in src/
    $ mypy --strict src/      # Strict mode (recommended for new projects)

CONFIGURATION (pyproject.toml):
    [tool.mypy]
    python_version = "3.12"
    strict = true
    warn_return_any = true
    warn_unused_configs = true
    disallow_untyped_defs = true
"""

# Examples of what type checkers catch:
from typing import Optional

def greet(name: str) -> str:
    return f"Hello, {name}"

# greet(42)        # mypy error: Argument 1 has incompatible type "int"
# greet(None)      # mypy error: Argument 1 has incompatible type "None"

def find_user(user_id: int) -> Optional[dict]:
    if user_id == 1:
        return {"name": "Alice"}
    return None

user = find_user(1)
# print(user["name"])  # mypy error: "None" has no attribute "__getitem__"

# Fixed:
if user is not None:
    print(user["name"])  # OK — narrowed to dict


# =============================================================================
# 18.3 LINTERS: RUFF, FLAKE8, PYLINT
# =============================================================================
"""
WHAT: Linters check code quality beyond type errors —
      style violations, potential bugs, complexity, etc.

RECOMMENDED (2024+): ruff
- Written in Rust (extremely fast)
- Replaces flake8 + isort + pyupgrade + many plugins
- Single tool for linting AND formatting

ALTERNATIVES:
- flake8: classic, many plugins (but slower)
- pylint: very thorough (but noisy, slow)

HOW TO USE RUFF:
    $ pip install ruff
    $ ruff check .           # Lint
    $ ruff check --fix .     # Lint + auto-fix
    $ ruff format .          # Format (like black)

CONFIGURATION (pyproject.toml):
    [tool.ruff]
    target-version = "py312"
    line-length = 99

    [tool.ruff.lint]
    select = ["E", "F", "W", "I", "N", "UP", "B", "A", "C4", "SIM"]
    ignore = ["E501"]  # line too long (handled by formatter)

    [tool.ruff.format]
    quote-style = "double"
    indent-style = "space"

RULE CATEGORIES:
    E = pycodestyle errors
    F = pyflakes (unused imports, undefined names)
    W = pycodestyle warnings
    I = isort (import ordering)
    N = naming conventions
    UP = pyupgrade (modernize syntax)
    B = bugbear (common bugs)
    SIM = simplify (code simplification)
"""


# =============================================================================
# 18.4 FORMATTERS: BLACK / RUFF FORMAT
# =============================================================================
"""
WHAT: Formatters automatically rewrite code to consistent style.
      Zero debate about formatting in code reviews.

WHY:
- Eliminates style discussions (the tool decides)
- Consistent codebase
- Smaller diffs (everyone's code looks the same)
- Less cognitive load (don't think about formatting)

TOOLS:
- black: the "uncompromising" formatter (de facto standard)
- ruff format: black-compatible but much faster
- yapf: Google's formatter (configurable)
- autopep8: minimal pep8 conformance

HOW:
    $ pip install black
    $ black .                # Format entire project
    $ black --check .        # Check without modifying
    $ black --diff .         # Show what would change

    # Or with ruff (faster):
    $ ruff format .

OPINIONATED DECISIONS (black/ruff format):
- Double quotes for strings
- Trailing commas in multi-line
- One expression per line for long function calls
- Magic trailing comma forces multi-line even if it fits

CONFIGURATION:
    [tool.black]
    line-length = 99
    target-version = ['py312']
"""


# =============================================================================
# 18.5 VIRTUAL ENVIRONMENTS
# =============================================================================
"""
WHAT: Isolated Python environments per project.
      Each project has its own set of installed packages.

WHY:
- Avoid dependency conflicts between projects
- Reproducible builds
- Don't pollute system Python
- Pin exact versions per project

TOOLS:
1. venv (built-in, recommended for most)
2. uv (fast, modern replacement by Astral)
3. conda (for scientific computing)
4. pyenv (manage Python versions)

HOW (venv):
    $ python -m venv .venv        # Create
    $ source .venv/bin/activate   # Activate (macOS/Linux)
    $ .venv\\Scripts\\activate      # Activate (Windows)
    $ deactivate                  # Deactivate
    $ pip install -r requirements.txt

HOW (uv — recommended 2024+):
    $ pip install uv
    $ uv venv                     # Create .venv
    $ uv pip install requests     # Install (10-100x faster than pip)
    $ uv pip install -r requirements.txt
    $ uv pip compile requirements.in -o requirements.txt

.gitignore:
    .venv/
    __pycache__/
    *.egg-info/
"""


# =============================================================================
# 18.6 DEPENDENCY MANAGEMENT
# =============================================================================
"""
WHAT: Track and reproduce exact dependencies for your project.

LEVELS:
1. requirements.txt — simple, pin exact versions
2. pyproject.toml — modern standard for project metadata + deps
3. poetry/pdm/hatch — full project managers
4. uv — fast, pip-compatible

=== requirements.txt ===
    requests==2.31.0
    flask>=3.0,<4.0
    numpy~=1.26.0

Generate:
    $ pip freeze > requirements.txt        # All packages
    $ pip-compile requirements.in          # From abstract deps

=== pyproject.toml (PEP 621) — THE MODERN WAY ===
    [project]
    name = "myproject"
    version = "1.0.0"
    requires-python = ">=3.11"
    dependencies = [
        "requests>=2.31",
        "pydantic>=2.0",
    ]

    [project.optional-dependencies]
    dev = [
        "pytest>=7.0",
        "ruff>=0.1",
        "mypy>=1.0",
    ]

Install:
    $ pip install .           # Install project
    $ pip install -e .        # Editable install (for development)
    $ pip install -e ".[dev]" # With dev dependencies

=== Lock files (exact reproducibility) ===
    $ uv pip compile pyproject.toml -o requirements.lock
    $ uv pip install -r requirements.lock
"""


# =============================================================================
# 18.7 PROJECT STRUCTURE
# =============================================================================
"""
MODERN PROJECT LAYOUT (src layout):

    myproject/
    ├── pyproject.toml          ← Project config (replaces setup.py)
    ├── README.md
    ├── LICENSE
    ├── .gitignore
    ├── .python-version         ← Pin Python version
    ├── src/
    │   └── mypackage/
    │       ├── __init__.py
    │       ├── core.py
    │       ├── models.py
    │       └── utils.py
    ├── tests/
    │   ├── conftest.py
    │   ├── test_core.py
    │   └── test_models.py
    └── docs/
        └── ...

WHY src/ layout:
- Prevents accidental import of local package (must install)
- Forces you to test the installed version
- Cleaner separation of source and project files
- Standard in modern Python packaging

MINIMAL pyproject.toml:
    [build-system]
    requires = ["hatchling"]
    build-backend = "hatchling.build"

    [project]
    name = "mypackage"
    version = "0.1.0"
    requires-python = ">=3.11"
    dependencies = []

    [tool.pytest.ini_options]
    testpaths = ["tests"]

    [tool.ruff]
    target-version = "py312"
    line-length = 99

    [tool.mypy]
    strict = true
"""


# =============================================================================
# 18.8 PRE-COMMIT HOOKS
# =============================================================================
"""
WHAT: Run checks automatically before each git commit.
      Prevents committing broken/ugly code.

WHY:
- Automated quality gate
- No more "forgot to format" commits
- Consistent across team
- Fast feedback (before CI)

HOW:
    $ pip install pre-commit
    $ pre-commit install

.pre-commit-config.yaml:
    repos:
    - repo: https://github.com/astral-sh/ruff-pre-commit
      rev: v0.4.0
      hooks:
      - id: ruff
        args: [--fix]
      - id: ruff-format

    - repo: https://github.com/pre-commit/mirrors-mypy
      rev: v1.10.0
      hooks:
      - id: mypy
        additional_dependencies: [types-requests]

    - repo: https://github.com/pre-commit/pre-commit-hooks
      rev: v4.6.0
      hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files
"""


# =============================================================================
# 18.9 CI/CD WITH GITHUB ACTIONS
# =============================================================================
"""
WHAT: Automated testing and deployment on every push/PR.

MINIMAL .github/workflows/ci.yml:

    name: CI
    on: [push, pull_request]

    jobs:
      test:
        runs-on: ubuntu-latest
        strategy:
          matrix:
            python-version: ["3.11", "3.12", "3.13"]

        steps:
        - uses: actions/checkout@v4
        - uses: actions/setup-python@v5
          with:
            python-version: ${{ matrix.python-version }}

        - name: Install dependencies
          run: |
            pip install -e ".[dev]"

        - name: Lint
          run: ruff check .

        - name: Type check
          run: mypy src/

        - name: Test
          run: pytest --cov=src/ --cov-report=term-missing
"""


# =============================================================================
# 18.10 COMPLETE TOOLING SETUP (RECOMMENDED 2024)
# =============================================================================
"""
THE MODERN PYTHON TOOLSTACK:

1. Python version manager: pyenv or mise
2. Package manager: uv (fast) or pip
3. Virtual env: uv venv or python -m venv
4. Linter + Formatter: ruff (replaces black + flake8 + isort)
5. Type checker: mypy or pyright
6. Testing: pytest
7. Pre-commit: pre-commit (with ruff + mypy hooks)
8. CI: GitHub Actions
9. Documentation: mkdocs-material or Sphinx
10. Editor: VS Code with Pylance + Ruff extension

QUICK START:
    $ pyenv install 3.12
    $ pyenv local 3.12
    $ pip install uv
    $ uv venv
    $ source .venv/bin/activate
    $ uv pip install ruff mypy pytest pre-commit
    $ ruff check .          # Lint
    $ ruff format .         # Format
    $ mypy src/             # Type check
    $ pytest                # Test
"""


# =============================================================================
# SUMMARY
# =============================================================================
"""
1. Follow PEP 8 (but let tools enforce it, don't memorize)
2. Use a formatter (ruff format/black) — zero formatting debates
3. Use a linter (ruff) — catch bugs and style issues
4. Use a type checker (mypy/pyright) — catch type errors statically
5. Use virtual environments — ALWAYS (one per project)
6. Use pyproject.toml — modern project configuration
7. Use pre-commit — automated quality gates
8. Use CI — test on every push
9. Use src/ layout — proper package structure
10. Automate everything — formatting, linting, testing, deployment
"""
