"""
=============================================================================
CHAPTER 16: TESTING
=============================================================================
Testing is essential for maintainable Python code.
Python has excellent testing tools, from built-in unittest to the
de facto standard pytest.

WHY test:
- Catch bugs before users do
- Enable refactoring with confidence
- Document expected behavior (tests as executable specs)
- Faster development (catch issues early, not in production)

TESTING PYRAMID:
- Unit tests (many): test individual functions/methods in isolation
- Integration tests (some): test components working together
- End-to-end tests (few): test full workflows

=============================================================================
"""

# =============================================================================
# 16.1 PYTEST — THE DE FACTO STANDARD
# =============================================================================
"""
WHAT: pytest is the most popular Python testing framework.
      Simple to start, powerful when you need advanced features.

WHY pytest over unittest:
- Simple assert statements (no self.assertEqual, etc.)
- Automatic test discovery (finds test_*.py files)
- Powerful fixtures (dependency injection for tests)
- Parametrize (run same test with different inputs)
- Rich plugins ecosystem
- Better error messages

HOW TO RUN:
    $ pytest                    # Run all tests
    $ pytest test_module.py     # Run specific file
    $ pytest -k "test_name"    # Run tests matching name
    $ pytest -v                # Verbose output
    $ pytest -x                # Stop on first failure
    $ pytest --tb=short        # Shorter tracebacks
"""

# === BASIC TESTS ===
# File: test_calculator.py

def add(a, b):
    return a + b

def divide(a, b):
    if b == 0:
        raise ValueError("Cannot divide by zero")
    return a / b

# Simple test functions (just prefix with test_)
def test_add():
    assert add(2, 3) == 5

def test_add_negative():
    assert add(-1, 1) == 0

def test_add_floats():
    import math
    assert math.isclose(add(0.1, 0.2), 0.3)

# Testing exceptions
import pytest

def test_divide_by_zero():
    """Test that proper exception is raised."""
    with pytest.raises(ValueError, match="Cannot divide by zero"):
        divide(10, 0)

# Testing approximate values
def test_divide_result():
    assert divide(10, 3) == pytest.approx(3.333, rel=1e-3)


# =============================================================================
# 16.2 FIXTURES
# =============================================================================
"""
WHAT: Fixtures provide test dependencies via dependency injection.
      They set up (and tear down) test preconditions.

WHY fixtures:
- DRY: shared setup code for multiple tests
- Composable: fixtures can depend on other fixtures
- Scoping: control lifetime (function, class, module, session)
- Cleanup: automatic teardown via yield

HOW: Decorate a function with @pytest.fixture, then use its name
     as a test parameter. pytest injects it automatically.
"""

@pytest.fixture
def sample_users():
    """Fixture providing test data."""
    return [
        {"name": "Alice", "age": 30, "role": "admin"},
        {"name": "Bob", "age": 25, "role": "user"},
        {"name": "Charlie", "age": 35, "role": "user"},
    ]

def test_user_count(sample_users):
    """Fixture injected by name."""
    assert len(sample_users) == 3

def test_admin_exists(sample_users):
    admins = [u for u in sample_users if u["role"] == "admin"]
    assert len(admins) == 1

# Fixture with setup AND teardown (using yield)
@pytest.fixture
def temp_database():
    """Create temp DB, yield it, then clean up."""
    import tempfile, os
    db_path = tempfile.mktemp(suffix=".db")
    # Setup
    db = {"path": db_path, "connection": "mock_connection"}
    yield db
    # Teardown (runs after test completes)
    if os.path.exists(db_path):
        os.remove(db_path)

def test_database_operations(temp_database):
    assert temp_database["connection"] == "mock_connection"

# Fixture scopes
@pytest.fixture(scope="module")
def expensive_resource():
    """Created once per module (shared across tests in module)."""
    print("\nCreating expensive resource...")
    return {"data": "expensive"}

# Fixture parametrization
@pytest.fixture(params=["sqlite", "postgres", "mysql"])
def database_engine(request):
    """Run tests with each database engine."""
    return request.param

def test_connection(database_engine):
    """This test runs 3 times (once per database engine)."""
    assert database_engine in ["sqlite", "postgres", "mysql"]


# =============================================================================
# 16.3 PARAMETRIZE
# =============================================================================
"""
WHAT: Run the same test with different inputs/expected outputs.
      Eliminates duplicate test code.

WHY: One test function, many test cases. Each case reported separately.
"""

@pytest.mark.parametrize("input,expected", [
    (1, 1),
    (2, 4),
    (3, 9),
    (4, 16),
    (-1, 1),
    (0, 0),
])
def test_square(input, expected):
    assert input ** 2 == expected

# Multiple parameters
@pytest.mark.parametrize("a,b,expected", [
    (1, 2, 3),
    (0, 0, 0),
    (-1, 1, 0),
    (100, 200, 300),
])
def test_add_parametrized(a, b, expected):
    assert add(a, b) == expected

# Parametrize with IDs for readability
@pytest.mark.parametrize("email,valid", [
    ("user@example.com", True),
    ("invalid-email", False),
    ("@missing-user.com", False),
    ("user@.com", False),
], ids=["valid_email", "no_at_sign", "no_username", "no_domain"])
def test_email_validation(email, valid):
    # assert validate_email(email) == valid
    pass


# =============================================================================
# 16.4 MOCKING
# =============================================================================
"""
WHAT: Replace real objects with controlled fakes during testing.
      Isolate the code under test from its dependencies.

WHY mock:
- Test without external dependencies (DB, API, filesystem)
- Control behavior (simulate errors, specific responses)
- Verify interactions (assert functions were called correctly)
- Speed (avoid slow operations)

WHEN to mock:
- External services (APIs, databases)
- Time-dependent code (datetime.now)
- Random values
- File system operations in unit tests

WHEN NOT to mock:
- Simple data transformations
- Pure functions
- When integration testing
"""

from unittest.mock import Mock, patch, MagicMock, call

# Basic Mock
mock_api = Mock()
mock_api.get_user.return_value = {"name": "Alice", "age": 30}

result = mock_api.get_user(user_id=1)
assert result == {"name": "Alice", "age": 30}
mock_api.get_user.assert_called_once_with(user_id=1)

# patch — replace real objects temporarily
"""
CRITICAL: Patch where the object is USED, not where it's defined!
"""

# Module under test (my_module.py):
# import requests
# def fetch_data(url):
#     response = requests.get(url)
#     return response.json()

# Test:
# @patch("my_module.requests.get")  # Patch where it's USED
# def test_fetch_data(mock_get):
#     mock_get.return_value.json.return_value = {"data": "test"}
#     result = fetch_data("http://api.example.com")
#     assert result == {"data": "test"}
#     mock_get.assert_called_once_with("http://api.example.com")

# patch as context manager
def test_with_patch():
    with patch("builtins.open", mock_open(read_data="file content")):
        # All open() calls return mock with "file content"
        pass

# MagicMock — Mock that supports magic methods
mock_list = MagicMock()
mock_list.__len__.return_value = 5
assert len(mock_list) == 5

# Spec — restrict mock to only have real object's attributes
# mock = Mock(spec=RealClass)
# mock.nonexistent_method()  # AttributeError! Not in spec.

from unittest.mock import mock_open

# Mock file operations
def test_read_config():
    m = mock_open(read_data='{"key": "value"}')
    with patch("builtins.open", m):
        import json
        with open("config.json") as f:
            data = json.load(f)
        assert data == {"key": "value"}


# =============================================================================
# 16.5 TEST ORGANIZATION
# =============================================================================
"""
WHAT: How to structure tests in a Python project.

RECOMMENDED STRUCTURE:
    project/
    ├── src/
    │   └── mypackage/
    │       ├── __init__.py
    │       ├── core.py
    │       └── utils.py
    ├── tests/
    │   ├── conftest.py      ← Shared fixtures
    │   ├── test_core.py     ← Tests for core module
    │   ├── test_utils.py    ← Tests for utils module
    │   └── integration/
    │       └── test_api.py  ← Integration tests
    └── pyproject.toml

NAMING CONVENTIONS:
- Test files: test_*.py or *_test.py
- Test functions: test_*
- Test classes: Test* (no __init__)
- Fixtures: descriptive names (not test_*)

CONFTEST.PY:
- Shared fixtures available to all tests in directory and subdirectories
- No need to import — pytest discovers automatically
- Can have multiple conftest.py at different levels
"""

# conftest.py example:
"""
import pytest

@pytest.fixture(scope="session")
def database_url():
    return "sqlite:///test.db"

@pytest.fixture
def auth_headers():
    return {"Authorization": "Bearer test-token"}
"""

# Test class grouping (when tests share setup/context)
class TestUserValidation:
    """Group related tests."""

    def test_valid_email(self):
        pass

    def test_invalid_email(self):
        pass

    def test_valid_age(self):
        pass


# =============================================================================
# 16.6 TESTING BEST PRACTICES
# =============================================================================
"""
KEY PRINCIPLES:

1. AAA Pattern: Arrange, Act, Assert
   - Arrange: set up test data and dependencies
   - Act: call the code under test
   - Assert: verify the result

2. One assertion per test (ideally)
   - Multiple asserts → multiple failure points
   - One assert → clear what failed and why

3. Test names should describe the scenario
   - test_add_returns_sum_of_two_positive_numbers
   - test_login_fails_with_invalid_password

4. Tests should be independent (no order dependency)

5. Tests should be fast (mock external dependencies)

6. Test edge cases:
   - Empty inputs
   - None/null values
   - Boundary values
   - Error conditions
"""

# AAA Pattern example
def test_discount_calculation():
    # Arrange
    price = 100.0
    discount_percent = 20

    # Act
    discounted_price = price * (1 - discount_percent / 100)

    # Assert
    assert discounted_price == 80.0

# Testing edge cases
class TestStringProcessor:
    def test_empty_string(self):
        assert "".strip() == ""

    def test_whitespace_only(self):
        assert "   ".strip() == ""

    def test_no_whitespace(self):
        assert "hello".strip() == "hello"

    def test_unicode(self):
        assert "  café  ".strip() == "café"

# Markers for test organization
# @pytest.mark.slow
# def test_heavy_computation():
#     pass

# Run: pytest -m "not slow"  → skip slow tests

# @pytest.mark.skip(reason="Feature not implemented yet")
# def test_future_feature():
#     pass

# @pytest.mark.xfail(reason="Known bug #123")
# def test_known_bug():
#     pass


# =============================================================================
# SUMMARY: TESTING BEST PRACTICES
# =============================================================================
"""
1. Use pytest (not unittest) for new projects
2. Follow AAA pattern: Arrange → Act → Assert
3. One test = one behavior = one assert (ideally)
4. Use fixtures for shared setup (not setUp/tearDown)
5. Use parametrize for testing multiple inputs
6. Mock external dependencies, not internal logic
7. Patch where the object is USED, not where it's defined
8. Test edge cases: empty, None, boundary, error conditions
9. Keep tests fast — mock I/O, avoid real network/DB in unit tests
10. Use markers to categorize tests (slow, integration, etc.)
11. Write conftest.py for shared fixtures
12. Aim for high coverage but don't optimize for 100% — test behavior, not lines
"""
