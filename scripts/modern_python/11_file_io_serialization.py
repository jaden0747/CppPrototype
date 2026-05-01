"""
=============================================================================
CHAPTER 11: FILE I/O & SERIALIZATION
=============================================================================
File operations and serialization are fundamental for any real application.
Python makes I/O intuitive with context managers and the pathlib module.

WHY learn file I/O deeply:
- Every application reads/writes data
- Wrong patterns cause resource leaks, data corruption, encoding issues
- pathlib is the modern way (replaces os.path — much more readable)

GOLDEN RULE: Always use `with` statement for file operations.

=============================================================================
"""

# =============================================================================
# 11.1 READING AND WRITING TEXT FILES
# =============================================================================
"""
WHAT: Text files are opened in text mode (default). Python handles
      encoding/decoding (bytes ↔ str) automatically.

KEY PARAMETERS:
- mode: 'r' (read), 'w' (write/truncate), 'a' (append), 'x' (create new)
- encoding: always specify! Default is platform-dependent.
- newline: controls line ending translation

BEST PRACTICE: ALWAYS specify encoding='utf-8' explicitly.
"""

# Writing text
with open("example.txt", "w", encoding="utf-8") as f:
    f.write("Hello, World!\n")
    f.write("Python is great.\n")
    # File is automatically closed when exiting `with` block

# Writing multiple lines
lines = ["Line 1\n", "Line 2\n", "Line 3\n"]
with open("example.txt", "w", encoding="utf-8") as f:
    f.writelines(lines)  # NOTE: doesn't add newlines — include them yourself

# Reading entire file
with open("example.txt", "r", encoding="utf-8") as f:
    content = f.read()  # Entire file as one string
    print(content)

# Reading line by line (memory efficient for large files)
with open("example.txt", "r", encoding="utf-8") as f:
    for line in f:  # File object is an iterator over lines
        print(line.strip())  # strip() removes trailing newline

# Reading all lines into a list
with open("example.txt", "r", encoding="utf-8") as f:
    lines = f.readlines()  # List of strings (each includes \n)

# PYTHONIC: Read lines stripped
with open("example.txt", "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f]

# Append mode (doesn't truncate)
with open("example.txt", "a", encoding="utf-8") as f:
    f.write("Appended line\n")

# Exclusive creation (fails if file exists)
import os
try:
    with open("new_file.txt", "x", encoding="utf-8") as f:
        f.write("Created new file")
except FileExistsError:
    print("File already exists")


# =============================================================================
# 11.2 BINARY FILES
# =============================================================================
"""
WHAT: Binary mode ('rb', 'wb') works with raw bytes — no encoding/decoding.

WHEN to use binary mode:
- Images, videos, audio
- Database files, archives
- Network protocols
- Any non-text data
"""

# Write binary data
data = bytes(range(256))  # All byte values 0-255
with open("binary_data.bin", "wb") as f:
    f.write(data)

# Read binary data
with open("binary_data.bin", "rb") as f:
    content = f.read()
    print(f"Read {len(content)} bytes")
    print(f"First 10: {content[:10]}")

# Read in chunks (for large files)
def read_chunks(filepath, chunk_size=4096):
    """Generator that reads file in chunks."""
    with open(filepath, "rb") as f:
        while chunk := f.read(chunk_size):
            yield chunk

# Copy file efficiently
def copy_file(src, dst, chunk_size=4096):
    with open(src, "rb") as fin, open(dst, "wb") as fout:
        while chunk := fin.read(chunk_size):
            fout.write(chunk)


# =============================================================================
# 11.3 PATHLIB — MODERN FILE PATHS
# =============================================================================
"""
WHAT: pathlib.Path provides object-oriented file system operations.
      It's the MODERN replacement for os.path string manipulation.

WHY pathlib over os.path:
- Object-oriented: p / "subdir" / "file.txt" (operator / for joining!)
- Cross-platform by default (handles Windows/Unix differences)
- Rich API: exists(), is_file(), suffix, stem, parent, etc.
- Works with all stdlib functions that accept paths
- More readable and less error-prone

WHEN to use pathlib:
- ALL new code should use pathlib for path manipulation
- os.path is only needed for very low-level operations
"""

from pathlib import Path

# Creating paths
home = Path.home()                    # User's home directory
cwd = Path.cwd()                     # Current working directory
config = Path("/etc") / "app" / "config.json"  # Join with / operator!
relative = Path("src") / "main.py"

# Path components
p = Path("/Users/alice/projects/app/main.py")
print(f"Name: {p.name}")       # main.py
print(f"Stem: {p.stem}")       # main (without extension)
print(f"Suffix: {p.suffix}")   # .py
print(f"Parent: {p.parent}")   # /Users/alice/projects/app
print(f"Parts: {p.parts}")     # ('/', 'Users', 'alice', 'projects', 'app', 'main.py')

# Change extension
readme = p.with_suffix(".md")   # /Users/alice/projects/app/main.md
renamed = p.with_name("test.py")  # /Users/alice/projects/app/test.py

# File operations
path = Path("test_pathlib.txt")

# Write (creates file, no context manager needed for simple operations)
path.write_text("Hello from pathlib!", encoding="utf-8")
path.write_bytes(b"Binary data")

# Read
content = path.read_text(encoding="utf-8")
data = path.read_bytes()

# Check existence and type
print(f"Exists: {path.exists()}")
print(f"Is file: {path.is_file()}")
print(f"Is dir: {path.is_dir()}")

# Directory operations
project = Path("my_project")
project.mkdir(parents=True, exist_ok=True)  # Like mkdir -p
(project / "src").mkdir(exist_ok=True)

# Listing directory contents
for item in Path(".").iterdir():
    if item.is_file():
        print(f"File: {item.name}")

# Glob — find files by pattern
# All Python files recursively
for py_file in Path(".").rglob("*.py"):
    print(f"Python file: {py_file}")

# All files in current dir
txt_files = list(Path(".").glob("*.txt"))

# Delete file
path.unlink(missing_ok=True)  # missing_ok prevents FileNotFoundError

# PYTHONIC: Use pathlib with open()
config_path = Path("config") / "settings.json"
# with open(config_path, "r", encoding="utf-8") as f:
#     data = json.load(f)
# Or even simpler:
# data = json.loads(config_path.read_text())


# =============================================================================
# 11.4 JSON SERIALIZATION
# =============================================================================
"""
WHAT: Convert Python objects to/from JSON format.
      JSON supports: str, int/float, bool, None, list, dict.

WHY JSON is ubiquitous:
- Universal data interchange format
- Human-readable
- Supported by every language
- Standard for REST APIs and config files

GOTCHAS:
- JSON keys must be strings
- No support for: sets, tuples (become lists), bytes, datetime, custom objects
- JSON has no comments (use TOML or YAML for config with comments)
"""

import json

# Basic serialization
data = {
    "name": "Alice",
    "age": 30,
    "languages": ["Python", "Rust", "Go"],
    "active": True,
    "address": None,
}

# Python → JSON string
json_str = json.dumps(data, indent=2)
print(json_str)

# JSON string → Python
parsed = json.loads(json_str)
print(f"Name: {parsed['name']}")

# File I/O
with open("data.json", "w", encoding="utf-8") as f:
    json.dump(data, f, indent=2, ensure_ascii=False)  # ensure_ascii=False for Unicode

with open("data.json", "r", encoding="utf-8") as f:
    loaded = json.load(f)

# Custom serialization for unsupported types
from datetime import datetime, date
from pathlib import Path

class CustomEncoder(json.JSONEncoder):
    """Handle types JSON doesn't support natively."""

    def default(self, obj):
        if isinstance(obj, (datetime, date)):
            return obj.isoformat()
        if isinstance(obj, Path):
            return str(obj)
        if isinstance(obj, set):
            return list(obj)
        return super().default(obj)

complex_data = {
    "timestamp": datetime.now(),
    "path": Path("/usr/local"),
    "tags": {"python", "coding"},
}
print(json.dumps(complex_data, cls=CustomEncoder, indent=2))

# PYTHONIC: Use pathlib for reading JSON
# config = json.loads(Path("config.json").read_text())


# =============================================================================
# 11.5 CSV
# =============================================================================
"""
WHAT: Read/write comma-separated value files.
WHEN: Tabular data interchange (spreadsheets, databases, data science).
"""

import csv

# Writing CSV
rows = [
    ["name", "age", "city"],
    ["Alice", 30, "NYC"],
    ["Bob", 25, "LA"],
]
with open("people.csv", "w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerows(rows)

# Reading CSV
with open("people.csv", "r", newline="", encoding="utf-8") as f:
    reader = csv.reader(f)
    header = next(reader)  # First row is header
    for row in reader:
        print(f"{row[0]} is {row[1]} from {row[2]}")

# DictReader/DictWriter — access by column name (PYTHONIC)
with open("people.csv", "r", newline="", encoding="utf-8") as f:
    reader = csv.DictReader(f)
    for row in reader:
        print(f"{row['name']} is {row['age']} from {row['city']}")

# Writing with DictWriter
fieldnames = ["name", "age", "city"]
data = [
    {"name": "Charlie", "age": 35, "city": "Chicago"},
    {"name": "Diana", "age": 28, "city": "Houston"},
]
with open("people2.csv", "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(data)


# =============================================================================
# 11.6 TOML (Python 3.11+)
# =============================================================================
"""
WHAT: Tom's Obvious Minimal Language — config file format.
      Supports comments, is more readable than JSON for config.

WHY TOML for config:
- Has comments (JSON doesn't!)
- More human-friendly than YAML (less error-prone)
- Standard for Python packaging (pyproject.toml)
- Built into Python 3.11+ as tomllib (read-only)
"""

import tomllib  # Python 3.11+ (read-only)

# Reading TOML
toml_content = """
[database]
host = "localhost"
port = 5432
name = "myapp"

[database.credentials]
user = "admin"
password = "secret"

[server]
host = "0.0.0.0"
port = 8080
debug = true
workers = 4
"""

# Parse TOML string
config = tomllib.loads(toml_content)
print(f"DB host: {config['database']['host']}")
print(f"Server port: {config['server']['port']}")

# Read from file
# with open("pyproject.toml", "rb") as f:  # NOTE: binary mode required!
#     config = tomllib.load(f)

# For writing TOML, use third-party `tomli-w` package:
# import tomli_w
# with open("config.toml", "wb") as f:
#     tomli_w.dump(config, f)


# =============================================================================
# 11.7 TEMPORARY FILES
# =============================================================================
"""
WHAT: Create temporary files and directories that are automatically cleaned up.
WHEN: Tests, intermediate processing, downloads before verification.
"""

import tempfile

# Temporary file (auto-deleted when closed)
with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
    f.write("Temporary data")
    temp_path = f.name
    print(f"Temp file: {temp_path}")

# Temporary directory (auto-deleted when context exits)
with tempfile.TemporaryDirectory() as tmpdir:
    temp_path = Path(tmpdir) / "data.txt"
    temp_path.write_text("hello")
    print(f"Temp dir: {tmpdir}")
    # Directory and all contents deleted after `with` block


# =============================================================================
# SUMMARY: FILE I/O BEST PRACTICES
# =============================================================================
"""
1. ALWAYS use `with` statement for file operations (guarantees cleanup)
2. ALWAYS specify encoding='utf-8' explicitly (don't rely on platform default)
3. Use pathlib.Path for ALL path manipulation (not os.path)
4. Use binary mode ('rb', 'wb') for non-text files
5. Process large files in chunks or line-by-line (don't read entire file into memory)
6. Use json for data interchange, TOML for config files
7. Use csv.DictReader/DictWriter for CSV (access by column name)
8. Use tempfile for temporary data (auto-cleanup)
9. Use Path.read_text() / Path.write_text() for simple one-shot operations
10. Handle encoding errors: errors='replace' or errors='ignore' for robustness
"""
