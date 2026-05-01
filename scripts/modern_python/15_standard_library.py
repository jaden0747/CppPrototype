"""
=============================================================================
CHAPTER 15: STANDARD LIBRARY HIGHLIGHTS
=============================================================================
Python's "batteries included" philosophy means the stdlib covers most needs.
This chapter highlights modules you should know well.

WHY master the stdlib:
- No external dependencies needed
- Well-tested and maintained
- Portable across platforms
- Often faster than reinventing (implemented in C)

=============================================================================
"""

# =============================================================================
# 15.1 DATETIME, ZONEINFO, CALENDAR
# =============================================================================
"""
WHAT: Working with dates, times, and time zones.

KEY RULES:
- Always store/transmit times in UTC
- Convert to local time only for display
- Use zoneinfo (3.9+) for time zones (not pytz)
- Use ISO 8601 format for serialization
"""

from datetime import datetime, date, time, timedelta
from zoneinfo import ZoneInfo  # Python 3.9+

# Current time
now = datetime.now()                      # Naive (no timezone) — avoid!
now_utc = datetime.now(ZoneInfo("UTC"))   # Aware (with timezone) — GOOD
print(f"UTC now: {now_utc.isoformat()}")

# Creating dates/times
birthday = date(1990, 5, 15)
meeting = datetime(2024, 3, 15, 14, 30, tzinfo=ZoneInfo("America/New_York"))
print(f"Meeting: {meeting}")

# Time zones
tokyo_time = now_utc.astimezone(ZoneInfo("Asia/Tokyo"))
london_time = now_utc.astimezone(ZoneInfo("Europe/London"))
print(f"Tokyo: {tokyo_time.strftime('%Y-%m-%d %H:%M %Z')}")
print(f"London: {london_time.strftime('%Y-%m-%d %H:%M %Z')}")

# Arithmetic with timedelta
tomorrow = date.today() + timedelta(days=1)
in_2_hours = datetime.now() + timedelta(hours=2)
duration = timedelta(days=7, hours=3, minutes=30)

# Parsing strings
parsed = datetime.fromisoformat("2024-03-15T14:30:00+00:00")
parsed2 = datetime.strptime("15/03/2024", "%d/%m/%Y")

# Formatting
formatted = now.strftime("%B %d, %Y at %I:%M %p")
print(f"Formatted: {formatted}")  # "March 15, 2024 at 02:30 PM"


# =============================================================================
# 15.2 REGULAR EXPRESSIONS (re)
# =============================================================================
"""
WHAT: Pattern matching and text manipulation using regex.

WHEN to use regex:
- Parsing structured text (logs, emails, URLs)
- Validation (email, phone, patterns)
- Search and replace with patterns
- Extracting data from strings

WHEN NOT to use:
- Simple string operations (use str methods instead)
- Parsing HTML/XML (use proper parsers)
- Complex grammars (use a parser library)

BEST PRACTICE: Always use raw strings r"..." for patterns.
"""

import re

# Basic patterns
text = "Contact us at support@example.com or sales@company.org"

# Find all emails
emails = re.findall(r'[\w.]+@[\w.]+\.\w+', text)
print(f"Emails: {emails}")

# Compile pattern for reuse (faster when used multiple times)
EMAIL_PATTERN = re.compile(r'''
    [\w.]+       # Username (word chars and dots)
    @            # @ symbol
    [\w.]+       # Domain
    \.\w+        # TLD
''', re.VERBOSE)  # VERBOSE allows comments and whitespace in pattern

# Search — find first match
match = EMAIL_PATTERN.search(text)
if match:
    print(f"First email: {match.group()}")

# Groups — extract parts of the match
PHONE_PATTERN = re.compile(r'(\d{3})-(\d{3})-(\d{4})')
phone_match = PHONE_PATTERN.search("Call 555-123-4567 today")
if phone_match:
    area, prefix, number = phone_match.groups()
    print(f"Area code: {area}")

# Named groups — more readable
DATE_PATTERN = re.compile(
    r'(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2})'
)
date_match = DATE_PATTERN.search("Today is 2024-03-15")
if date_match:
    print(f"Year: {date_match.group('year')}")
    print(f"Month: {date_match.group('month')}")

# Substitution
cleaned = re.sub(r'\s+', ' ', "Too    many    spaces")
print(f"Cleaned: {cleaned}")

# PYTHONIC: Use str methods when possible (simpler, faster)
# Use regex only when patterns are needed
text = "hello world"
# GOOD (simple case — use str method):
text.startswith("hello")
text.replace("world", "python")
"hello" in text
# USE REGEX (when pattern needed):
re.match(r'hello \w+', text)


# =============================================================================
# 15.3 CONTEXTLIB
# =============================================================================
"""
WHAT: Utilities for working with context managers (with statement).

KEY TOOLS:
- @contextmanager: create context managers from generator functions
- suppress: ignore specific exceptions
- redirect_stdout/redirect_stderr: capture output
- ExitStack: manage dynamic number of context managers
- nullcontext: no-op context manager (useful for conditional use)
"""

from contextlib import contextmanager, suppress, redirect_stdout, ExitStack, nullcontext
import io

# @contextmanager — simplest way to create context managers
@contextmanager
def temporary_change(obj, attr, value):
    """Temporarily change an attribute, restore on exit."""
    old_value = getattr(obj, attr)
    setattr(obj, attr, value)
    try:
        yield old_value
    finally:
        setattr(obj, attr, old_value)

# suppress — cleanly ignore exceptions
with suppress(FileNotFoundError, PermissionError):
    import os
    os.remove("might_not_exist.txt")

# redirect_stdout — capture print output
f = io.StringIO()
with redirect_stdout(f):
    print("This goes to f, not console")
captured = f.getvalue()
print(f"Captured: {captured!r}")

# ExitStack — manage variable number of resources
def process_files(filenames):
    """Open multiple files safely."""
    with ExitStack() as stack:
        files = [
            stack.enter_context(open(fn, "r"))
            for fn in filenames
            if os.path.exists(fn)
        ]
        # All files auto-closed when exiting

# nullcontext — conditional context manager
def get_lock(use_threading: bool):
    """Return a lock or no-op depending on whether threading is used."""
    import threading
    if use_threading:
        return threading.Lock()
    return nullcontext()

# Usage:
# with get_lock(multi_threaded):
#     do_work()


# =============================================================================
# 15.4 COPY (SHALLOW AND DEEP)
# =============================================================================
"""
WHAT: Creating copies of objects.

KEY DISTINCTION:
- Assignment (=): creates a new REFERENCE (no copy at all)
- Shallow copy: new container, but elements are same objects
- Deep copy: new container AND new copies of all nested objects

WHEN to use each:
- Reference: when you WANT both names to point to same object
- Shallow copy: flat structures (list of primitives)
- Deep copy: nested/complex structures you want fully independent
"""

import copy

# Assignment — NO copy (same object)
original = [[1, 2], [3, 4], [5, 6]]
alias = original
alias[0][0] = 999
print(f"Original affected: {original[0][0]}")  # 999!

# Shallow copy — new outer list, same inner lists
original = [[1, 2], [3, 4], [5, 6]]
shallow = copy.copy(original)  # or original.copy() or list(original) or original[:]
shallow.append([7, 8])       # Doesn't affect original (new outer list)
shallow[0][0] = 999          # AFFECTS original (inner lists shared!)
print(f"Original[0]: {original[0]}")  # [999, 2]

# Deep copy — fully independent copy
original = [[1, 2], [3, 4], [5, 6]]
deep = copy.deepcopy(original)
deep[0][0] = 999
print(f"Original[0]: {original[0]}")  # [1, 2] — unchanged!


# =============================================================================
# 15.5 HASHLIB AND SECRETS
# =============================================================================
"""
WHAT: Cryptographic hashing and secure random values.

hashlib: one-way hashes (SHA-256, SHA-512, etc.)
secrets: cryptographically secure random values

WHEN to use:
- hashlib: checksums, file integrity, password storage (with salt)
- secrets: tokens, passwords, API keys, session IDs

CRITICAL: Use `secrets` for security, `random` for non-security!
"""

import hashlib
import secrets

# Hashing
data = "Hello, World!"
hash_sha256 = hashlib.sha256(data.encode()).hexdigest()
print(f"SHA-256: {hash_sha256}")

# File checksum
def file_hash(filepath, algorithm="sha256"):
    """Compute hash of a file (memory efficient)."""
    h = hashlib.new(algorithm)
    with open(filepath, "rb") as f:
        while chunk := f.read(8192):
            h.update(chunk)
    return h.hexdigest()

# Secure random values (for security purposes)
token = secrets.token_hex(32)          # 64-char hex string
url_token = secrets.token_urlsafe(32)  # URL-safe base64 string
random_int = secrets.randbelow(100)    # Secure random int [0, 100)

print(f"Token: {token}")
print(f"URL token: {url_token}")

# Secure password generation
import string
alphabet = string.ascii_letters + string.digits + string.punctuation
password = ''.join(secrets.choice(alphabet) for _ in range(16))
print(f"Secure password: {password}")

# Compare strings safely (constant-time comparison — prevents timing attacks)
# secrets.compare_digest(user_token, stored_token)


# =============================================================================
# 15.6 ARGPARSE (CLI ARGUMENT PARSING)
# =============================================================================
"""
WHAT: Parse command-line arguments into structured data.
WHEN: Building CLI tools and scripts.
"""

import argparse

def build_parser():
    """Build an argument parser for a CLI tool."""
    parser = argparse.ArgumentParser(
        description="Process data files",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    # Positional argument (required)
    parser.add_argument("input", help="Input file path")

    # Optional arguments
    parser.add_argument("-o", "--output", default="output.txt",
                        help="Output file path (default: output.txt)")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Enable verbose output")
    parser.add_argument("-n", "--count", type=int, default=10,
                        help="Number of items to process")
    parser.add_argument("--format", choices=["json", "csv", "text"],
                        default="json", help="Output format")

    # Mutually exclusive options
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--compress", action="store_true")
    group.add_argument("--no-compress", action="store_true")

    return parser

# Usage:
# args = build_parser().parse_args()
# print(f"Input: {args.input}, Output: {args.output}, Verbose: {args.verbose}")


# =============================================================================
# 15.7 INSPECT MODULE
# =============================================================================
"""
WHAT: Inspect live objects — get source code, signatures, call stacks.
WHEN: Debugging, metaprogramming, building frameworks, documentation tools.
"""

import inspect

def example_function(x: int, y: str = "hello") -> bool:
    """An example function."""
    return True

# Get function signature
sig = inspect.signature(example_function)
print(f"Signature: {sig}")  # (x: int, y: str = 'hello') -> bool

# Get parameters
for name, param in sig.parameters.items():
    print(f"  {name}: default={param.default}, annotation={param.annotation}")

# Get source code
source = inspect.getsource(example_function)
print(f"Source:\n{source}")

# Check what something is
print(f"Is function: {inspect.isfunction(example_function)}")
print(f"Is class: {inspect.isclass(str)}")

# Get caller information
def who_called_me():
    frame = inspect.currentframe().f_back
    return f"Called from {frame.f_code.co_filename}:{frame.f_lineno}"


# =============================================================================
# 15.8 WEAKREF
# =============================================================================
"""
WHAT: References to objects that don't prevent garbage collection.
      Normal references keep objects alive; weak references don't.

WHEN to use:
- Caches (cache without preventing GC of unused objects)
- Observer pattern (observers shouldn't keep subjects alive)
- Circular reference breaking
"""

import weakref

class ExpensiveObject:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return f"ExpensiveObject({self.name!r})"

# Normal reference keeps object alive
obj = ExpensiveObject("important")
weak = weakref.ref(obj)

print(f"Object alive: {weak()}")  # ExpensiveObject('important')
del obj  # Object can now be garbage collected
print(f"After del: {weak()}")     # None (object was collected)

# WeakValueDictionary — cache that doesn't prevent GC
cache = weakref.WeakValueDictionary()

def get_expensive_object(key):
    """Cache that allows GC of unused objects."""
    if key in cache:
        return cache[key]
    obj = ExpensiveObject(key)
    cache[key] = obj
    return obj


# =============================================================================
# SUMMARY: STANDARD LIBRARY ESSENTIALS
# =============================================================================
"""
MODULES EVERY PYTHON DEVELOPER SHOULD KNOW:
┌─────────────────────┬──────────────────────────────────────────────────────┐
│ Category            │ Modules                                              │
├─────────────────────┼──────────────────────────────────────────────────────┤
│ Data Structures     │ collections, dataclasses, heapq, bisect              │
│ File System         │ pathlib, os, shutil, tempfile, glob                  │
│ Serialization       │ json, csv, tomllib, pickle, struct                   │
│ Text Processing     │ re, string, textwrap, difflib                        │
│ Date/Time           │ datetime, zoneinfo, calendar, time                   │
│ Math/Numbers        │ math, statistics, random, decimal, fractions         │
│ Crypto/Security     │ hashlib, secrets, hmac                               │
│ Concurrency         │ asyncio, threading, multiprocessing, concurrent.futures│
│ Network/Web         │ urllib, http.server, socket, email                   │
│ Testing             │ unittest, doctest, unittest.mock                     │
│ Debugging           │ pdb, traceback, logging, warnings, inspect           │
│ Functional          │ functools, itertools, operator                       │
│ Context Management  │ contextlib                                           │
│ CLI                 │ argparse, sys                                        │
│ Type System         │ typing, abc                                          │
│ Metaprogramming     │ inspect, importlib, types, copy                      │
└─────────────────────┴──────────────────────────────────────────────────────┘
"""
