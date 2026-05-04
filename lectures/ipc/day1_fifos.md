# Day 1 — POSIX IPC Foundations + Named Pipes (FIFOs)

> **Time budget:** 75 min concept · 120 min lab · 45 min drills · 15 min recap
> **Prerequisites:** Comfort with C++ and Python, basic understanding of processes and file descriptors
> **By the end you can:**
> - Explain the file-descriptor model and how it underlies all POSIX IPC
> - Implement a producer/consumer pair using a FIFO
> - Handle SIGPIPE correctly and reason about blocking semantics
> - Multiplex a FIFO with `poll()`
> - Answer the four most common FIFO interview questions cold

---

## 1. Conceptual overview

**The file-descriptor mental model — memorize this, it applies for the next six days.**

The kernel mediates all inter-process communication. Every IPC primitive you will use — FIFOs, sockets, shared memory segments, eventfds, timerfds — is represented to user space as an integer *file descriptor*. A process doesn't "connect to" another process; it reads from and writes to file descriptors, and the kernel arranges the plumbing.

```
Process A                   Kernel                  Process B
fd=3 (write end) ──write──▶ [FIFO buffer ≤65536B] ──read──▶ fd=4 (read end)
```

That buffer lives in kernel memory — not in either process's address space. This is the key difference from shared memory (Day 2): every byte through a pipe or FIFO incurs a copy into the kernel and a copy out. That's slow at scale but safe by default.

**The four POSIX IPC families — when to use each:**

| Family         | API entry point             | Lifetime              | Copies            | Best when                          |
| -------------- | --------------------------- | --------------------- | ----------------- | ---------------------------------- |
| Pipes / FIFOs  | `pipe()` / `mkfifo()`       | Process / filesystem  | 2× kernel         | Simple streaming, control channels |
| Message queues | `mq_open()`                 | Kernel (until unlink) | 2× kernel         | Discrete messages with priorities  |
| Shared memory  | `shm_open()` + `mmap()`     | Kernel (until unlink) | Zero on data path | High-throughput, latency-sensitive |
| Semaphores     | `sem_open()` / `sem_init()` | Kernel / process      | N/A               | Synchronization primitives         |

**Anonymous pipes vs named pipes (FIFOs):**

`pipe(2)` creates an anonymous pipe — two file descriptors, only usable by processes with a common ancestor (parent/child, post-fork). No filesystem entry. Dies when both ends close.

`mkfifo(3)` creates a named pipe — a filesystem entry (type `p` in `ls -l`). Any process that can open the path gets an end. Persists until `unlink(2)` is called. This is what you use for unrelated processes.

**Blocking semantics — the part that trips people up:**

1. `open()` on a FIFO *blocks* until the other end is also opened, unless `O_NONBLOCK` is set.
   - A process that opens the read end will sleep in `open()` until a writer arrives, and vice versa.
   - This is not a bug. It's the handshake.

2. `read()` on an empty FIFO blocks (or returns `EAGAIN` with `O_NONBLOCK`).

3. `write()` to a FIFO with *no readers* delivers `SIGPIPE` to the writer. The default action is termination. If `SIGPIPE` is ignored or handled, the write returns `-1` with `errno == EPIPE`.

**PIPE_BUF and atomicity:**

Writes of `<= PIPE_BUF` bytes (4096 on Linux, 512 POSIX minimum) are atomic — they will not be interleaved with writes from other processes. Larger writes may be split. This matters when multiple writers share one FIFO.

---

## 2. Deep dive: file descriptors and fork/exec inheritance

File descriptors are process-wide. After `fork()`, the child inherits copies of all open file descriptors. Each copy is an independent reference to the same underlying open file description (the kernel's `struct file`). This has one subtle consequence: if parent and child both hold the write end of a pipe, the read end will not see EOF until *both* close their write end.

```
          fork()
Parent ─────────────────────────── Parent (holds write_fd)
          └── Child (inherits write_fd)

Read end stays open until BOTH write_fds are closed.
```

After `exec()`, file descriptors survive unless `FD_CLOEXEC` is set (or `O_CLOEXEC` at open time). Set `O_CLOEXEC` on everything you don't explicitly intend to pass across exec.

**Observing this with strace:**

```bash
strace -e trace=open,openat,read,write,close ./producer /tmp/myfifo
```

A FIFO open looks like:
```
openat(AT_FDCWD, "/tmp/myfifo", O_WRONLY) = 3   # blocks until reader opens
write(3, "hello\n", 6)                  = 6
```

Run `strace` on your lab producer. Seeing the syscalls directly is more instructive than any diagram.

---

## 3. Deep dive: O_NONBLOCK and poll()

With `O_NONBLOCK`, `open()` on a FIFO returns immediately. If no reader is present, the write-end open returns `-1 / ENXIO`. If no writer is present, the read-end open succeeds (you have a read-only FIFO with no data — `read()` will return 0 or `EAGAIN`).

**Multiplexing with `poll()`:**

```c
// poll_consumer.c — wait for data on a FIFO without blocking forever
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

// Build: gcc -Wall -Wextra -o poll_consumer poll_consumer.c
// Run:   ./poll_consumer /tmp/myfifo

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "usage: %s <fifo>\n", argv[0]); return 1; }

    // Open non-blocking so we don't hang if no writer yet
    int fd = open(argv[1], O_RDONLY | O_NONBLOCK);
    if (fd < 0) { perror("open"); return 1; }

    char buf[4096];
    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    for (;;) {
        int ret = poll(&pfd, 1, 5000);  // 5-second timeout
        if (ret < 0) { perror("poll"); break; }
        if (ret == 0) { puts("timeout — no data in 5s"); continue; }

        if (pfd.revents & POLLHUP) {
            // Writer closed. Drain remaining data then exit.
            puts("writer disconnected");
        }
        if (pfd.revents & POLLIN) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            fputs(buf, stdout);
        }
        if (pfd.revents & POLLHUP && !(pfd.revents & POLLIN)) break;
    }
    close(fd);
    return 0;
}
```

`POLLHUP` is set when all write ends are closed. Drain the FIFO after `POLLHUP` — data written before the close is still there.

---

## 4. Lab

### Setup

```bash
# Create the FIFO once; both processes share it
mkfifo /tmp/ipc_lab_fifo

# Build commands (used throughout)
gcc -Wall -Wextra -o producer producer.c
gcc -Wall -Wextra -o consumer consumer.c
gcc -Wall -Wextra -o poll_consumer poll_consumer.c
```

### Tasks

**Task 1 — Basic producer/consumer**

Write `producer.c`: opens the FIFO for writing, sends a timestamped line every 500 ms, runs until killed.

```c
// producer.c
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

// Build: gcc -Wall -Wextra -o producer producer.c
// Run:   ./producer /tmp/ipc_lab_fifo

static volatile sig_atomic_t got_sigpipe = 0;

static void sigpipe_handler(int sig) {
    (void)sig;
    got_sigpipe = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "usage: %s <fifo>\n", argv[0]); return 1; }

    // Install SIGPIPE handler BEFORE opening the FIFO.
    // Without this, a write to a FIFO with no readers kills the process.
    struct sigaction sa = { .sa_handler = sigpipe_handler };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) < 0) { perror("sigaction"); return 1; }

    printf("[producer] opening %s — will block until consumer connects\n", argv[1]);
    int fd = open(argv[1], O_WRONLY);  // blocks here until reader opens
    if (fd < 0) { perror("open"); return 1; }
    printf("[producer] connected\n");

    char buf[128];
    long seq = 0;

    while (!got_sigpipe) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        int n = snprintf(buf, sizeof(buf), "%ld.%03ld seq=%ld\n",
                         (long)ts.tv_sec, ts.tv_nsec / 1000000L, seq++);

        ssize_t written = write(fd, buf, (size_t)n);
        if (written < 0) {
            if (errno == EPIPE) {
                // SIGPIPE was ignored/handled; write returns EPIPE instead
                fprintf(stderr, "[producer] EPIPE — consumer gone, exiting\n");
                break;
            }
            perror("write");
            break;
        }
        // 500 ms sleep
        nanosleep(&(struct timespec){0, 500000000L}, NULL);
    }

    if (got_sigpipe)
        fprintf(stderr, "[producer] caught SIGPIPE — consumer gone\n");

    close(fd);
    return 0;
}
```

Write `consumer.c`: opens the FIFO for reading, prints each line, exits on EOF.

```c
// consumer.c
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

// Build: gcc -Wall -Wextra -o consumer consumer.c
// Run:   ./consumer /tmp/ipc_lab_fifo

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "usage: %s <fifo>\n", argv[0]); return 1; }

    printf("[consumer] opening %s\n", argv[1]);
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    printf("[consumer] connected\n");

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        fputs(buf, stdout);
        fflush(stdout);
    }
    if (n < 0) perror("read");
    puts("[consumer] EOF — producer closed write end");
    close(fd);
    return 0;
}
```

**Done when:** Both processes run, you see timestamped lines appearing in the consumer's terminal.

**Task 2 — SIGPIPE observation**

Start `./producer /tmp/ipc_lab_fifo` in one terminal, `./consumer /tmp/ipc_lab_fifo` in another. Kill the consumer (`Ctrl-C`). Observe the producer print the SIGPIPE message and exit cleanly.

**Done when:** Producer exits with the `[producer] caught SIGPIPE` message rather than crashing silently.

**Task 3 — Two consumers, data splitting**

Run the producer, then start two consumers simultaneously:
```bash
./consumer /tmp/ipc_lab_fifo &
./consumer /tmp/ipc_lab_fifo &
```

Observe: data is split between the two consumers, not duplicated. Each `read()` dequeues data from the kernel buffer; the first reader wins each byte.

**Done when:** You've confirmed that a single message appears in exactly one consumer's output, not both.

**Task 4 — Non-blocking consumer with poll()**

Replace the blocking consumer with `poll_consumer.c` (code in section 3).

```bash
./poll_consumer /tmp/ipc_lab_fifo
```

Kill the producer. Observe `POLLHUP` handling — the consumer prints "writer disconnected" and exits rather than hanging.

**Done when:** Consumer exits cleanly on producer death without requiring a `Ctrl-C`.

### Reference solution

The four source files above are the reference solution. Key teaching points:

- `sigaction` not `signal` — `signal()` has unspecified behavior on Linux for some signals.
- Install the `SIGPIPE` handler **before** opening the FIFO. A write can fail immediately on open if the reader disappears.
- After `POLLHUP`, drain remaining `POLLIN` data before breaking — there may be bytes buffered in the kernel.
- Two blocking `open()` calls (one reader, one writer) is the handshake — neither proceeds until both arrive.

---

## 5. Common pitfalls

- **Forgetting `SIGPIPE` entirely.** Default disposition is `TERM`. Production code always installs a handler or `SIG_IGN` on write-facing file descriptors. The symptom is a mysteriously dead process with no log output.

- **Opening both ends in the same process.** `open(path, O_RDWR)` on a FIFO works on Linux but is not POSIX-portable and defeats the purpose. Don't do it except in specific `O_NONBLOCK` workaround patterns.

- **Assuming write atomicity above PIPE_BUF.** A 64 KB write from one process can be interleaved with writes from another. If you have multiple writers, keep each write `<= PIPE_BUF` or add your own framing.

- **Not handling `POLLHUP` correctly.** If you break immediately on `POLLHUP` without draining `POLLIN`, you lose data that arrived before the writer closed. Always check `POLLIN` first, or in the same branch.

- **Leaking the FIFO node.** `mkfifo` creates a filesystem entry. `close()` closes your file descriptor; it does not remove the FIFO. Call `unlink("/tmp/ipc_lab_fifo")` in your cleanup path.

- **Blocking `open()` deadlock.** If a process opens a FIFO for `O_RDWR` or opens both ends sequentially in the same thread, it can deadlock waiting for itself. Use two threads or `O_NONBLOCK` with retry logic.

---

## 6. Interview drills

**Q: Why would you choose a FIFO over a socket?**

FIFOs are simpler when both processes are on the same machine, you want file-system-based access control, and you don't need bidirectional communication or multiplexing. Sockets add addressing, bidirectionality, and network capability at the cost of setup complexity. For a simple command channel between two daemons on the same host, a FIFO is often the right call.

*Follow-up: "What if you need bidirectional communication?"* — Use two FIFOs (one each direction), or switch to a Unix domain socket, which is full-duplex and lower overhead than an inet socket.

---

**Q: What happens if a writer writes 5 KB and PIPE_BUF is 4096?**

The write is no longer atomic. The kernel may deliver it in multiple chunks, interleaved with writes from other processes on the same FIFO. The data itself is not corrupted — every byte arrives in order from *this* writer — but bytes from concurrent writers may be interleaved. For reliable message framing with multiple writers, prefix each write with a length header and keep the header write `<= PIPE_BUF`.

*Follow-up: "How do you verify PIPE_BUF on a given system?"* — `fpathconf(fd, _PC_PIPE_BUF)` or `ulimit -p` (reported in 512-byte blocks).

---

**Q: How do you build a pub/sub system with FIFOs?**

You don't — not reliably. A FIFO has a single read end; data delivered to one reader is gone. For true pub/sub (one writer, multiple independent readers each seeing all messages), use Unix domain sockets with one socket per subscriber, or a shared memory ring buffer with per-subscriber read pointers. A message queue (`mq_open`) also doesn't solve it directly since each message is consumed once.

*Follow-up: "So FIFOs are useless for multiple consumers?"* — No. If you want work distribution (task queue semantics, where each item goes to exactly one worker), multiple readers on a single FIFO work perfectly.

---

**Q: Describe the FIFO blocking handshake.**

`open(path, O_RDONLY)` blocks until at least one writer has opened the other end. `open(path, O_WRONLY)` blocks until at least one reader has opened. Neither process proceeds until both are ready. This is a kernel-enforced synchronization point — you get a connected pair with no additional handshake code.

*Follow-up: "What if I don't want to block on open?"* — Pass `O_NONBLOCK`. The read-end open returns immediately (succeeds even without a writer). The write-end open with `O_NONBLOCK` and no reader returns `-1 / ENXIO`.

---

**Q: How does a process recover from SIGPIPE?**

Install a handler with `sigaction` before the first write. In the handler, set a `volatile sig_atomic_t` flag. Check the flag in the write loop and exit cleanly. Alternatively, block `SIGPIPE` with `sigprocmask` and inspect `errno == EPIPE` after each `write()`. Never use `signal(SIGPIPE, SIG_IGN)` in a library — you're changing global state that the caller doesn't expect.

---

## 7. Cheatsheet

### Key syscalls

| Syscall  | Signature                                       | Notes                                              |
| -------- | ----------------------------------------------- | -------------------------------------------------- |
| `mkfifo` | `mkfifo(const char *path, mode_t mode)`         | Creates FIFO node; `mode` is `0666` typically      |
| `open`   | `open(path, O_RDONLY\|O_WRONLY [, O_NONBLOCK])` | Blocks until both ends open (without `O_NONBLOCK`) |
| `read`   | `read(fd, buf, count) → ssize_t`                | Returns 0 on EOF (all writers closed)              |
| `write`  | `write(fd, buf, count) → ssize_t`               | SIGPIPE/EPIPE if no readers                        |
| `poll`   | `poll(fds, nfds, timeout_ms)`                   | Use `POLLIN`, watch for `POLLHUP`                  |
| `unlink` | `unlink(path)`                                  | Removes the FIFO node from filesystem              |

### Key flags

| Flag         | Meaning                               |
| ------------ | ------------------------------------- |
| `O_NONBLOCK` | Non-blocking open and I/O             |
| `O_CLOEXEC`  | Close on exec (always set this)       |
| `POLLIN`     | Data available to read                |
| `POLLHUP`    | Write end closed; drain then exit     |
| `EPIPE`      | Write to FIFO with no readers (errno) |

### PIPE_BUF
- Linux: **65536 bytes** (pipe capacity); atomic write unit: **4096 bytes**
- Check: `fpathconf(fd, _PC_PIPE_BUF)`
- Writes `<= PIPE_BUF` are atomic; larger writes may interleave

### Lifecycle
```
mkfifo("/tmp/x", 0666)
  └─ Process A: open(O_WRONLY)  ←→  Process B: open(O_RDONLY)
       write() → kernel buffer → read()
  └─ cleanup: close(fd) + unlink("/tmp/x")
```

### Signal handling (production pattern)
```c
static volatile sig_atomic_t got_sigpipe = 0;
static void on_sigpipe(int s) { (void)s; got_sigpipe = 1; }
// In main(), before any write:
sigaction(SIGPIPE, &(struct sigaction){.sa_handler = on_sigpipe}, NULL);
```

---

## 8. Further reading

- `man 7 fifo` — the canonical reference; read the whole thing (short)
- Kerrisk, *The Linux Programming Interface*, Chapter 44 (Pipes and FIFOs) — the definitive treatment
- Beej's Guide to Unix IPC, "FIFOs" section — fast overview with examples if the above is too dense