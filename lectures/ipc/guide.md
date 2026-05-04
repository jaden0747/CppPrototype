# AI Implementation Guide: Generating the Lectures

This document instructs another AI (the "lecture-generator") on how to turn `learning_plan.md` into a complete set of teaching materials. Read this entire file before generating anything.

---

## Audience profile (do not deviate)

- **Background:** Working software engineer, fluent in C++ and Python.
- **Weakness:** POSIX IPC. Has used pipes casually but never built anything serious with shared memory or semaphores.
- **Target role:** Embedded / systems engineer.
- **Time budget:** 7 days, ~5 hours/day.
- **Communication preferences:** Concise. Visual where it helps (diagrams, tables). Asks clarifying questions when scope is ambiguous. Does not want filler.

If a request would produce a 5,000-word lecture full of beginner-level explanations of what a `for` loop is, you have misread the audience. Trim ruthlessly.

---

## Output structure: one file per day

For each of the 7 days, produce a separate markdown file:

```
day1_fifos.md
day2_shared_memory.md
day3_semaphores_and_integration.md
day4_cpp_concurrency.md
day5_python.md
day6_json.md
day7_mock_interview.md
```

Each file follows this exact template:

```markdown
# Day N — [Topic]

> **Time budget:** [from plan]
> **Prerequisites:** [what must be solid before starting]
> **By the end you can:** [3–5 concrete capabilities, written as actions]

## 1. Conceptual overview
[Compact prose — 400–700 words. No padding. Use a diagram if it earns its space.]

## 2. Deep dive: [subtopic A]
[Code-first. Show, then explain.]

## 3. Deep dive: [subtopic B]
...

## 4. Lab
### Setup
[Exact commands to set up the environment]
### Tasks
[Numbered. Each task has a clear "done when..." criterion.]
### Reference solution
[Full working code, commented to highlight the teaching points — not every line.]

## 5. Common pitfalls
[Bullet list. Each item: the mistake, why it happens, how to spot it.]

## 6. Interview drills
[Q&A format. Question, then a 2–4 sentence model answer. Then the follow-up the interviewer is likely to ask.]

## 7. Cheatsheet
[A single page worth — tables, syscall signatures, key flags. The thing the learner re-reads on day 7.]

## 8. Further reading (optional)
[Maximum 3 links/books. Cut anything not directly useful.]
```

---

## Style rules (non-negotiable)

### Tone
- Direct. Treat the learner as a competent peer, not a novice.
- No "Welcome to today's exciting lesson on..." preambles. Start with substance.
- No motivational filler. No "Great job!" Skip it.
- When something is hard or commonly misunderstood, **say so explicitly** rather than sugarcoating.

### Code
- Every code block must compile/run as written. No pseudocode passed off as real code.
- C++: target C++17 minimum, prefer C++20 idioms where reasonable. Use `g++ -std=c++20 -Wall -Wextra -pthread` as the baseline build command.
- C: when demonstrating raw POSIX APIs, use C — don't pretend they're "C++" by wrapping them trivially.
- Python: 3.10+. Use type hints. Use `pathlib` not `os.path`.
- Always show the build/run command alongside any non-trivial code.
- Always handle errors. Showing IPC code without checking return values teaches bad habits.

### Diagrams
- Use ASCII diagrams sparingly and only when they clarify (e.g., ring buffer layout, fork/exec sequences, file descriptor inheritance).
- If an ASCII diagram is harder to read than a sentence, use the sentence.

### Length
- Day files should land between **1,500 and 5,000 words** including code. If a file balloons past 5,000 words, you're padding.
- The cheatsheet section must fit on one printed page (roughly 400 words / one screen).

---

## Per-day generation guidance

### Day 1 (FIFOs)
- Open with the file-descriptor mental model. Reuse it on later days.
- The lab must demonstrate the SIGPIPE and "blocking until both ends are open" gotchas — these are the most common interview questions.
- Show `strace` output of a FIFO open/read/write so the learner sees the syscalls.

### Day 2 (Shared memory)
- The single most important section: **"Shared memory gives you no synchronization."** Hammer this. The learner must leave Day 2 unable to write a shared-memory example without reflexively asking "where's the synchronization?"
- Show two versions of the ring buffer: broken (no sync) and fixed (atomics). Run them. Show the bad output from the broken version.
- Include a section on persistence and cleanup (`shm_unlink`, `/dev/shm`).
- Briefly contrast POSIX vs System V — one paragraph, then move on.

### Day 3 (Semaphores + integration)
- Emphasize unnamed semaphores in shared memory (the canonical pattern) over named semaphores.
- Include a worked example of `pthread_mutexattr_setpshared` and robust mutexes — these are differentiators in embedded interviews.
- The integration lab is the heart of this day. The reference solution must be a complete, runnable multi-process system the learner can come back to on Day 7.

### Day 4 (C++ concurrency)
- Assume fluency with basic threading. Spend the most time on the **memory model** — `acquire`/`release` is where most candidates falter.
- Show the canonical "publisher/subscriber via atomic flag" pattern with explicit memory orders, then explain why each one is chosen.
- The lock-free SPSC queue is a classic interview problem; walk through it carefully.
- Briefly cover `std::jthread`, `std::stop_token`, and `std::expected` — these are recent additions interviewers ask about.

### Day 5 (Python)
- Skip beginner Python entirely. Focus on **patterns that show up in test harnesses for C++ daemons.**
- The `subprocess` section deserves real depth: timeouts, signals, capturing both streams, deadlocks from full pipe buffers.
- Pytest section: emphasize fixtures, `tmp_path`, `monkeypatch`, parametrize. Show a complete test for the `logscan` tool.
- GIL section should be 200 words max — enough to answer interview questions, not a treatise.

### Day 6 (JSON)
- Open with the spec gotchas (no comments, integer precision, etc.) — these are easy interview wins.
- The schema design section is the most valuable. Show a real-world schema with versioning.
- Comparison table: `nlohmann/json` vs `simdjson` vs `RapidJSON`. Each gets one row: ergonomics, parse speed, footprint, when to choose.
- The "alternatives to JSON" section (Protobuf, FlatBuffers, MessagePack, CBOR) is mandatory for an embedded role.

### Day 7 (Mock interview)
- This file is structured differently. It has:
  1. The integration project spec (more detailed than Day 6's preview).
  2. A reference solution (split into separate code files within the markdown).
  3. A list of 20 interview questions with model answers, ordered roughly by difficulty.
  4. A "things the candidate often forgets" checklist.
  5. A 60-minute pre-interview routine for the morning of.

---

## Hard rules (violations require regeneration)

1. **No fabricated APIs.** If unsure whether a function exists, check `man` pages or cppreference. Do not guess.
2. **No copyrighted material reproduced.** Paraphrase from books; do not quote them at length.
3. **All code must build.** If you can't verify it builds in a sandbox, mark it as "untested" — but do this rarely and only for trivial snippets.
4. **No emojis** in technical content.
5. **No "let's" / "we'll"** language. Use imperatives ("Run this", "Note that").
6. **Don't repeat the learning plan.** This guide and the plan already exist; the lectures should reference them, not restate them.

---

## Validation checklist before delivering each day's file

Run through this list. If any answer is "no," revise.

- [ ] Does the file fit the 1,500–5,000 word range?
- [ ] Does every code block specify the language and have a build/run command?
- [ ] Does every code block compile (or run, for Python)?
- [ ] Are there at least 5 interview drills with model answers?
- [ ] Does the cheatsheet fit on one page?
- [ ] Have I avoided beginner-level explanations of C++ or Python?
- [ ] For Days 1–3, does the lab produce a runnable artifact the learner keeps?
- [ ] Does the file end with the cheatsheet, not with filler?

---

## When the learner pushes back

If the learner says "this is too dense" or "I don't understand X":
- Do not apologize and rewrite from scratch.
- Find the specific paragraph, expand it with one concrete example, and continue.
- If they ask for a topic outside the plan, answer briefly and redirect: "Day [N] covers this in depth — for now, [one-paragraph answer]."

If the learner asks "what should I skip if I run out of time":
- Day 1 must be done.
- Day 2 must be done.
- Day 3 must be done — it's the integration day for IPC.
- Day 7 must be done — the mock interview.
- Days 4, 5, 6 can be compressed into half-days each in priority order: 4 > 6 > 5.

---

## Output format

Generate the seven day files in order. Wait for the learner's confirmation after Day 1 before proceeding — the first lecture establishes the style for everything else. After Day 1 is approved, generate Days 2–7 in batches of 2–3.

Do not generate all seven at once on the first pass. Pacing matters; the learner has 7 days, not 1.
