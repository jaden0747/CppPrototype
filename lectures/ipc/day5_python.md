# Day 5 — Python for Automation, Testing, and Data

> **Time budget:** 60 min concept · 120 min lab · 45 min drills · 15 min recap
> **Prerequisites:** You've completed Days 1–4. You can write Python, but haven't used it to drive multi-process C++ systems.
> **By the end you can:**
> - Launch and control C++ daemons from Python using `subprocess` without common deadlock traps
> - Write a `pytest` test suite with fixtures, `tmp_path`, `monkeypatch`, and parametrize
> - Process log files efficiently using generators and `pathlib`
> - Explain the GIL and pick between `threading`, `multiprocessing`, and `asyncio` for a given problem
> - Write a Python supervisor that reads from your Day 3 IPC pipeline and validates output

---

## 1. Conceptual overview

Python's role on an embedded team is almost never "runs on the device." It's the glue: test harnesses, log scrapers, CI scripts, protocol fuzzers, and data pipelines that drive the C++ binaries you're actually shipping.

Interviewers at this level don't test whether you know Python—they test whether you know the **sharp edges**: subprocess deadlocks, the GIL, mock boundaries, and the difference between a fixture that's fast and one that's accidentally stateful.

Three areas get the most coverage:

1. **`subprocess`** — launching binaries, capturing output, handling timeouts, not shooting yourself with pipe buffer deadlocks
2. **`pytest`** — not just "write tests" but fixtures, monkeypatch, parametrize, and the `yield`-based teardown pattern
3. **Concurrency model** — the GIL, when `multiprocessing` beats `threading`, and when `asyncio` is the right tool

---

## 2. Deep dive: subprocess

### The right way to launch a process

```python
# run.py
import subprocess
import shlex

# ✅ Basic invocation — prefer list, not string
result = subprocess.run(
    ["./sensor_proc", "--device", "/dev/ttyS0", "--rate", "100"],
    capture_output=True,
    text=True,
    timeout=5.0,     # raises TimeoutExpired; always set this
    check=True,      # raises CalledProcessError on non-zero exit
)
print(result.stdout)
```

```bash
python3 run.py
```

**Why list, not string:** `shell=True` with user-controlled input is a command injection vector. It also makes `timeout` and signal handling less predictable. Use `shlex.split()` if you must parse a string.

### Capturing both streams without deadlock

The most common mistake: reading stdout and stderr sequentially when the process is blocking on a full pipe buffer.

```python
import subprocess, threading

def drain(stream: "IO[bytes]", buf: list[bytes]) -> None:
    for line in iter(stream.readline, b""):
        buf.append(line)

proc = subprocess.Popen(
    ["./aggregator"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
)

stdout_lines: list[bytes] = []
stderr_lines: list[bytes] = []

t_out = threading.Thread(target=drain, args=(proc.stdout, stdout_lines))
t_err = threading.Thread(target=drain, args=(proc.stderr, stderr_lines))
t_out.start(); t_err.start()

try:
    proc.wait(timeout=10.0)
except subprocess.TimeoutExpired:
    proc.kill()
    proc.wait()
finally:
    t_out.join(); t_err.join()

stdout = b"".join(stdout_lines).decode()
stderr = b"".join(stderr_lines).decode()
```

**The deadlock:** `communicate()` handles this internally with threads or `select`. For simple cases, use `communicate(timeout=N)`. For streaming (reading line-by-line while the process runs), use the threading pattern above.

### Sending signals and clean shutdown

```python
import signal, os

proc = subprocess.Popen(["./sensor_proc"])

# Ask nicely first
proc.send_signal(signal.SIGTERM)
try:
    proc.wait(timeout=2.0)
except subprocess.TimeoutExpired:
    proc.kill()   # SIGKILL — no cleanup possible
    proc.wait()
```

---

## 3. Deep dive: pytest fixtures and test patterns

### Fixture fundamentals

```python
# tests/conftest.py
import pytest
from pathlib import Path

@pytest.fixture
def log_dir(tmp_path: Path) -> Path:
    """Creates a fake log directory with known content."""
    (tmp_path / "app.log").write_text(
        "INFO starting\nERROR disk full\nINFO stopping\n"
    )
    (tmp_path / "net.log").write_text(
        "ERROR timeout\nWARNING retry\n"
    )
    return tmp_path
```

`tmp_path` is a built-in pytest fixture — a unique `Path` per test, cleaned up automatically. Never use `tempfile.mkdtemp()` directly in tests; you'll forget cleanup.

### The yield pattern for teardown

```python
@pytest.fixture
def live_process(tmp_path: Path):
    """Starts a real subprocess; kills it after the test."""
    fifo = tmp_path / "ctrl.fifo"
    fifo_path = str(fifo)
    import os
    os.mkfifo(fifo_path)

    proc = subprocess.Popen(["./sensor_proc", "--fifo", fifo_path])
    yield proc, fifo_path   # test body runs here

    # Teardown — runs even if test fails
    proc.terminate()
    try:
        proc.wait(timeout=2.0)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
```

### monkeypatch: patching without changing source

```python
# tests/test_logscan.py
def test_logscan_uses_env_dir(monkeypatch, tmp_path, log_dir):
    monkeypatch.setenv("LOGSCAN_ROOT", str(log_dir))
    from logscan import scan
    report = scan()
    assert len(report["errors"]) == 2  # ERROR disk full, ERROR timeout
```

`monkeypatch` reverts all changes at test end — no `os.environ` pollution between tests.

### parametrize

```python
@pytest.mark.parametrize("log_line,expected_level", [
    ("ERROR disk full", "ERROR"),
    ("WARNING low memory", "WARNING"),
    ("INFO started", "INFO"),
    ("GARBAGE", None),
])
def test_parse_level(log_line: str, expected_level: str | None) -> None:
    from logscan import parse_level
    assert parse_level(log_line) == expected_level
```

### Mocking subprocess calls

```python
from unittest.mock import patch, MagicMock

def test_sensor_timeout(monkeypatch):
    mock_run = MagicMock(side_effect=subprocess.TimeoutExpired(cmd=["sensor"], timeout=5))
    with patch("logscan.subprocess.run", mock_run):
        with pytest.raises(RuntimeError, match="sensor timed out"):
            from logscan import collect_sensor_data
            collect_sensor_data()
```

**The key insight:** patch at the import point, not the definition point. If `logscan.py` does `import subprocess` and calls `subprocess.run`, patch `logscan.subprocess.run`, not `subprocess.run`.

---

## 4. Deep dive: concurrency model and the GIL

The GIL (Global Interpreter Lock) prevents two Python threads from executing Python bytecodes simultaneously. This matters in exactly one scenario: **CPU-bound work in threads.**

| Use case | Right tool | Why |
|---|---|---|
| Launching subprocesses, waiting on I/O | `threading` | Threads release GIL during I/O/`wait` — fine |
| CPU-bound: parsing 10GB log file | `multiprocessing` | Each process has its own GIL |
| Many concurrent I/O ops (HTTP, sockets) | `asyncio` | Single-threaded; zero GIL contention |
| Calling a C extension (numpy, pandas ops) | `threading` | C extensions release GIL |

200-word interview answer: "The GIL means Python threads don't parallelize CPU-bound work — only one thread runs Python bytecodes at a time. For I/O-bound work (subprocess, network, disk), threads work fine because the GIL is released during system calls. For CPU-bound work, use `multiprocessing` which spawns real processes, or `concurrent.futures.ProcessPoolExecutor` for a higher-level API. If you need high-concurrency I/O without threads, `asyncio` is better — it's single-threaded cooperative multitasking with no GIL concerns at all. Most embedded test harness work is I/O-bound (waiting on subprocesses, reading FIFOs), so threading is usually fine there."

---

## 5. Lab

### Setup

```bash
# Working directory for the day
mkdir -p day5 && cd day5

# Verify Python version
python3 --version   # 3.10+

# Install dependencies
pip3 install pytest pytest-timeout pydantic --break-system-packages 2>/dev/null || \
  pip install pytest pytest-timeout pydantic
```

### Tasks

**Task 1: `logscan` CLI tool**

Done when: `python3 logscan.py --dir ./logs --output report.json` produces a valid JSON report listing all ERROR lines grouped by file, with a timestamp.

```python
# logscan.py
import argparse
import json
import logging
import sys
from concurrent.futures import ProcessPoolExecutor
from datetime import datetime, timezone
from pathlib import Path

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    stream=sys.stderr,
)
log = logging.getLogger(__name__)


def scan_file(path: Path) -> dict:
    errors = []
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            for lineno, line in enumerate(f, start=1):
                if "ERROR" in line:
                    errors.append({"line": lineno, "text": line.rstrip()})
    except OSError as exc:
        log.warning("Cannot read %s: %s", path, exc)
    return {"file": str(path), "errors": errors}


def scan(root: Path, parallel: bool = False) -> dict:
    log_files = list(root.rglob("*.log"))
    log.info("Found %d log files under %s", len(log_files), root)

    if parallel:
        with ProcessPoolExecutor() as ex:
            results = list(ex.map(scan_file, log_files))
    else:
        results = [scan_file(p) for p in log_files]

    total_errors = sum(len(r["errors"]) for r in results)
    return {
        "scanned_at": datetime.now(timezone.utc).isoformat(),
        "root": str(root),
        "total_errors": total_errors,
        "files": results,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Scan log files for ERROR lines")
    parser.add_argument("--dir", type=Path, required=True, help="Root directory")
    parser.add_argument("--output", type=Path, default=None, help="JSON output file")
    parser.add_argument("--parallel", action="store_true")
    args = parser.parse_args()

    if not args.dir.is_dir():
        log.error("%s is not a directory", args.dir)
        sys.exit(1)

    report = scan(args.dir, parallel=args.parallel)

    out = json.dumps(report, indent=2)
    if args.output:
        args.output.write_text(out)
        log.info("Report written to %s", args.output)
    else:
        print(out)


if __name__ == "__main__":
    main()
```

**Task 2: pytest suite for `logscan`**

Done when: `pytest tests/ -v` passes with at least 6 tests, including one parametrized case and one using `tmp_path`.

```python
# tests/test_logscan.py
import json
import subprocess
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from logscan import scan, scan_file


@pytest.fixture
def log_dir(tmp_path: Path) -> Path:
    (tmp_path / "app.log").write_text(
        "INFO start\nERROR disk full\nINFO stop\n"
    )
    (tmp_path / "net.log").write_text(
        "ERROR timeout\nWARNING retry\nERROR dropped\n"
    )
    (tmp_path / "empty.log").write_text("")
    return tmp_path


def test_scan_counts_errors(log_dir: Path) -> None:
    report = scan(log_dir)
    assert report["total_errors"] == 3


def test_scan_empty_file(log_dir: Path) -> None:
    report = scan(log_dir)
    empty = next(f for f in report["files"] if "empty.log" in f["file"])
    assert empty["errors"] == []


def test_scan_parallel_matches_serial(log_dir: Path) -> None:
    serial = scan(log_dir, parallel=False)
    parallel = scan(log_dir, parallel=True)
    # Order may differ; compare sorted
    serial_errors = sorted(
        e["text"] for f in serial["files"] for e in f["errors"]
    )
    parallel_errors = sorted(
        e["text"] for f in parallel["files"] for e in f["errors"]
    )
    assert serial_errors == parallel_errors


@pytest.mark.parametrize("content,expected_count", [
    ("INFO only\n", 0),
    ("ERROR one\n", 1),
    ("ERROR one\nERROR two\n", 2),
    ("", 0),
])
def test_scan_file_error_count(tmp_path: Path, content: str, expected_count: int) -> None:
    f = tmp_path / "test.log"
    f.write_text(content)
    result = scan_file(f)
    assert len(result["errors"]) == expected_count


def test_cli_produces_valid_json(log_dir: Path, tmp_path: Path) -> None:
    output = tmp_path / "report.json"
    result = subprocess.run(
        [sys.executable, "logscan.py", "--dir", str(log_dir), "--output", str(output)],
        capture_output=True, text=True, timeout=10.0, check=True,
    )
    assert output.exists()
    report = json.loads(output.read_text())
    assert "total_errors" in report
    assert report["total_errors"] == 3


def test_cli_invalid_dir(tmp_path: Path) -> None:
    result = subprocess.run(
        [sys.executable, "logscan.py", "--dir", str(tmp_path / "nonexistent")],
        capture_output=True, text=True, timeout=5.0,
    )
    assert result.returncode != 0
```

```bash
# Run it
pytest tests/ -v
```

**Task 3: Python supervisor for the Day 3 IPC pipeline**

Done when: the script launches both C++ processes, reads the FIFO output, and asserts at least one valid telemetry line arrives within 5 seconds.

```python
# supervisor.py
"""
Launches the Day 3 sensor + aggregator pipeline, reads from the control FIFO,
validates output, and shuts everything down cleanly.

Assumes binaries: ./sensor_proc --fifo <path>
                  ./aggregator  --fifo <path>
built from Day 3.
"""
import json
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path


def run_pipeline(binary_dir: Path = Path("."), duration: float = 3.0) -> list[dict]:
    with tempfile.TemporaryDirectory() as tmpdir:
        fifo_path = os.path.join(tmpdir, "data.fifo")
        os.mkfifo(fifo_path)

        sensor = subprocess.Popen(
            [str(binary_dir / "sensor_proc"), "--fifo", fifo_path],
            stderr=subprocess.PIPE,
        )
        aggregator = subprocess.Popen(
            [str(binary_dir / "aggregator"), "--fifo", fifo_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        messages = []
        deadline = time.monotonic() + duration

        with open(fifo_path, "r") as fifo:
            while time.monotonic() < deadline:
                line = fifo.readline()
                if not line:
                    break
                try:
                    msg = json.loads(line)
                    messages.append(msg)
                except json.JSONDecodeError:
                    print(f"WARN: unparseable line: {line!r}", file=sys.stderr)

        sensor.terminate()
        aggregator.terminate()
        for proc in (sensor, aggregator):
            try:
                proc.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()

        return messages


if __name__ == "__main__":
    print("Starting pipeline...", file=sys.stderr)
    msgs = run_pipeline()
    print(f"Received {len(msgs)} messages")
    assert len(msgs) > 0, "No messages received — is the pipeline running?"
    print("Pipeline OK")
```

**Task 4: Add `--parallel` to `logscan` using `ProcessPoolExecutor`**

Already included in Task 1 above via the `--parallel` flag and `ProcessPoolExecutor`. Verify the test `test_scan_parallel_matches_serial` passes.

---

## 6. Common pitfalls

- **`shell=True` with variables.** `subprocess.run(f"ls {user_input}", shell=True)` is a shell injection. Always pass a list. If you must parse a string, use `shlex.split`.

- **Reading stdout and stderr sequentially.** If stderr fills its pipe buffer while you're reading stdout, the process blocks and you deadlock. Use `communicate()` or drain both streams with threads.

- **Forgetting `proc.wait()` after `proc.kill()`.** The process becomes a zombie until waited on. In test teardown this leaks file descriptors.

- **Using `print` for logging in tools.** `print` goes to stdout, which may be captured by the caller. Use the `logging` module with a stderr handler.

- **Fixture scope footguns.** A `session`-scoped fixture that mutates state will bleed between tests. Keep fixtures `function`-scoped by default; only promote scope when you've measured the cost.

- **Patching at the wrong import path.** `patch("subprocess.run")` doesn't affect code that already imported `subprocess`. Patch `yourmodule.subprocess.run`.

- **`tmp_path` vs `tmpdir`.** `tmp_path` is the modern (3.9+) fixture returning a `Path`. `tmpdir` returns a legacy `py.path.local`. Use `tmp_path`.

---

## 7. Interview drills

**Q: When would you choose `multiprocessing` over `threading` in Python?**

When the work is CPU-bound. The GIL prevents true parallelism for Python bytecodes in threads — only one thread runs at a time. `multiprocessing` spawns real OS processes with separate GILs. `ProcessPoolExecutor` is the ergonomic wrapper. Tradeoff: processes have higher startup cost and IPC overhead (pickling across process boundaries). For I/O-bound work — waiting on subprocesses, network, disk — threads are fine.

*Follow-up: "Can you use threads with numpy?" Yes — numpy releases the GIL during array operations, so threads do parallelize CPU-bound numpy code.*

---

**Q: How do you test code that calls `subprocess`?**

Patch at the module's import point. If `mymodule.py` calls `subprocess.run`, use `unittest.mock.patch("mymodule.subprocess.run", ...)`. For integration tests that need the real process, use a `pytest` fixture with `yield` to start the process and a teardown block to kill it. Use `tmp_path` to create FIFOs or temp files the process needs. Assert on output with a timeout to avoid hanging tests.

*Follow-up: "What about flaky tests caused by timing?" Use `--timeout` plugin to kill slow tests. In the process fixture, do a brief `time.sleep(0.1)` or poll a status file after launch rather than a fixed sleep.*

---

**Q: Walk me through a `pytest` fixture with `yield`.**

The fixture runs up to `yield`, the value at `yield` is passed to the test, the test runs, then execution continues after `yield` for teardown — even if the test raises. This is equivalent to `try/finally`. Example: start a subprocess before yield, kill it after. The scope (`function`, `module`, `session`) controls how often the fixture is invoked. A `session`-scoped fixture runs once per test session; `function`-scoped runs once per test.

*Follow-up: "What if the setup fails before yield?" The teardown block doesn't run — pytest only runs teardown for fixtures that successfully completed setup.*

---

**Q: How would you automate testing of a C++ daemon?**

Write a Python harness with `pytest`. Launch the daemon as a subprocess in a `yield` fixture. Communicate via its actual IPC mechanism (FIFO, shm, socket) rather than mocking it. Parametrize tests over different input scenarios. Use `tmp_path` for any files the daemon reads or writes. Assert on output with a timeout and clear error messages. Run it in CI with `pytest --timeout=30`. For shm and semaphores, check `/dev/shm` and use `/proc/<pid>/status` to verify the daemon exits cleanly.

*Follow-up: "How do you test for resource leaks?" Run under `valgrind --leak-check=full` from a subprocess call in a separate slow test, or check that `/dev/shm` has no residual segments after teardown.*

---

**Q: When would you use `asyncio`?**

When you need high-concurrency I/O — hundreds of concurrent connections, websockets, or multiplexing many subprocess outputs without threads. `asyncio` is single-threaded cooperative multitasking; tasks yield at `await` points. No GIL concerns, low overhead. Not the right tool for: CPU-bound work (still one thread), mixing with synchronous blocking code (blocks the event loop), or a simple test harness where threading is simpler. On embedded teams, `asyncio` shows up in protocol simulators and device fleet management scripts.

---

## 8. Cheatsheet

### subprocess quick reference

| Need | Code |
|---|---|
| Run and capture | `subprocess.run([...], capture_output=True, text=True, timeout=5, check=True)` |
| Stream output live | `Popen([...], stdout=PIPE)` + read line-by-line |
| Both streams safe | Use `communicate(timeout=N)` or drain threads |
| Send signal | `proc.send_signal(signal.SIGTERM)` |
| Kill + wait | `proc.kill(); proc.wait()` |

### pytest patterns

```python
@pytest.fixture              # function scope (default)
def thing(tmp_path):
    setup()
    yield resource           # test runs here
    teardown()               # always runs

@pytest.mark.parametrize("x,y", [(1,2),(3,4)])
def test_f(x, y): ...

monkeypatch.setenv("VAR", "val")
monkeypatch.setattr(mod, "fn", mock_fn)

with patch("mod.subprocess.run") as m:
    m.return_value.stdout = "ok"
```

### GIL decision table

| Work type | Tool |
|---|---|
| I/O-bound | `threading` or `asyncio` |
| CPU-bound Python | `multiprocessing` / `ProcessPoolExecutor` |
| CPU-bound C extension (numpy) | `threading` (GIL released) |
| Many concurrent I/O ops | `asyncio` |

### pathlib essentials

```python
p = Path("/var/log")
p.rglob("*.log")          # recursive glob
p / "sub" / "file.txt"    # join
p.read_text()             # read
p.write_text("data")      # write
p.is_dir(), p.exists()    # checks
p.stem, p.suffix, p.name  # parts
```

### logging setup (not print)

```python
import logging, sys
logging.basicConfig(level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    stream=sys.stderr)
log = logging.getLogger(__name__)
log.info("started"); log.error("failed: %s", err)
```