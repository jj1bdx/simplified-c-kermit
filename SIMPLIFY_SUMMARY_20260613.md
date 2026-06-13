# Change Summary — `c0eb7ec` → `HEAD` (`f5456b8`)

**Date:** 2026-06-13
**Tree:** C-Kermit 10.0 Beta.12 (`/home/kenji/src/simplified-c-kermit`)
**Range:** `c0eb7ecec245f44f6bc6ad6ae773ece00d074a8e..HEAD`

## Overview

Two independent simplification passes landed in this range, plus the merge that
joined them. C source and the makefile were each cleaned of dead material; the
compiled `wermit` binary is unchanged on every supported target.

| Commit | Title | Effect |
|---|---|---|
| `3201982` | Remove compilation macro `COMMENT` from C code | 37 `.c`/`.h`/`.w` files, ~6,300 dead lines removed |
| `1eb9e0b` | Remove obsolete/not-buildable makefile targets (Linux/macOS/BSD only) | `makefile` 6,302 → 1,202 lines (−5,100), 514 target names removed |
| `f5456b8` | Merge `origin/remove-comment` | merge of `3201982` into `main` (no new content) |

`git diff --stat` for the range: **41 files changed, 363 insertions(+), 11,596
deletions(-)** (the makefile work also added the two report/plan markdown files
and updated `CLAUDE.md`).

> **Scope note.** At merge time `origin/remove-comment` pointed at `3201982`. The
> four later `ckuver.h` herald-string commits on that remote branch
> (`54bbdfe`, `5560705`, `9b418f8`, `0e8be87`) are **not** part of this range / not
> in `HEAD`. The only `ckuver.h` change here is the 5-line `#ifdef COMMENT` block
> removed by `3201982`.

---

## 1. `COMMENT` dead-code removal (`3201982`)

### What `COMMENT` was

`COMMENT` is **not a compilation feature flag** — it is a long-standing C-Kermit
idiom for commenting out a region of code that cannot be wrapped in `/* … */`
because the region itself already contains `/* */` comments or `#ifdef`s (C
comments do not nest). Wrapping such a region in `#ifdef COMMENT … #endif` and
**never defining `COMMENT`** makes the preprocessor discard it while keeping it
visible in the source as a pseudo-comment.

Verified tree-wide:

- `grep -rn "define COMMENT" .` → **no matches** (never `#define`d in any
  `.c`/`.h`/`.w`).
- `grep -n "COMMENT" makefile` → **no matches** (never passed via `-D` / `KFLAGS`).

So every `#ifdef COMMENT` block was **unconditionally dead** on all targets, and
the inverse `#ifndef COMMENT` was always true.

### What was removed

All "frozen for the record" material:

- **Old/superseded implementations parked for reference** — the dominant category
  (largest blocks in `ckcftp.c` (305) and `ckufio.c` (452)).
- **Disabled debug / experimental code** — e.g. the `EVENMAX` UCS-2
  alternating-byte detector and a `CK_SPEED` control-prefix restore loop in
  `ckuusx.c`.
- **Ineffective / alternate code paths** — e.g. `ckutio.c` `ttptycmd`'s
  `signal(SIGTTOU/SIGTSTP, SIG_IGN)` calls self-annotated `/* THIS HAS NO EFFECT */`.
- **Dead conditions / historical value tables** — e.g. the `ckuver.h` default
  `#ifndef CKCPU / #define CKCPU "unknown"` (itself nested inside `#ifdef COMMENT`).

Nested forms are common (that is *why* the idiom exists): many `#ifdef COMMENT`
blocks bracket inner `#ifdef`s, e.g. `ckuusx.c`:
`#ifdef COMMENT → #ifdef CK_SPEED … #endif → #endif /* COMMENT */`.

### Scale

- **37 files** (`.c`/`.h`/`.w` only).
- **731** COMMENT-form opening directives removed (**704** `#ifdef COMMENT` +
  **27** `#ifndef COMMENT`).
- **~6,300 lines** removed; the 2 reported insertions are blank-line whitespace
  normalization, not code.
- Largest single-file removals: `ckuus4.c` (537), `ckuus5.c` (488),
  `ckutio.c` (482), `ckufio.c` (452), `ckuus6.c` (446), `ckuusr.c` (404),
  `ckuus7.c` (393), `ckuusx.c` (386), `ckucmd.c` (338), `ckcftp.c` (305).

### Safety

Behavior-preserving by construction: because `COMMENT` is never defined, the
guarded regions were excluded from compilation on every target, so deleting them
cannot change the emitted object code on Linux, macOS, or BSD. Confirmed by a
clean `rm -f wermit && make linux` (exit 0, **zero** warnings/errors,
`make check` → SUCCESS) — a mismatched `#endif`/`#else` from an over- or
under-greedy deletion would have broken the preprocessor. Per-file
`#ifdef`/`#endif` count asymmetries are expected (inner nested `#endif`s carry
their own comments) and the clean build is the authoritative balance proof.

Unrelated surviving uses of the word "COMMENT" were correctly **left intact**:

- the user-facing Kermit `COMMENT` script command (`ckuus2.c`, `ckuusr.c`);
- SSH key-comment fields;
- `ckuusx.c:2008-2012`, where the directive is escaped *inside* `/* */`
  (`/* #ifdef COMMENT */ … /* WHY WAS THIS COMMENTED OUT? */`), i.e. not a real
  preprocessor directive;
- prose mentions in `ckufio.c`, `ckuus5.c`.

`ckcpro.w` (the wart grammar source) was edited directly; `ckcpro.c` is
regenerated from it via `make wart && make ckcpro.c` and is gitignored.

---

## 2. Obsolete makefile-target removal (`1eb9e0b`)

Full detail in `SIMPLIFY_20260613_6.md` (report) and
`OS_SIMPLIFY_PLAN_20260613_2.md` (plan). Summary:

- `makefile`: **6,302 → 1,202 lines** (−5,100, −80.9%); **487 stanzas / 514
  target names** removed; diff **+80 / −5,181**.
- The makefile now keeps **only** Linux, macOS, and modern-BSD
  (FreeBSD/NetBSD/OpenBSD/MirBSD) targets, plus infrastructure (49 kept targets).
- Removed: all foreign-Unix families (SGI IRIX, MIPS, DNIX, Apollo, Encore, AT&T
  SysV, HP-UX, AIX, SCO/Unixware, Solaris/SunOS5, DEC Ultrix/OSF/Tru64, Sequent,
  NeXT, QNX, Interactive, DG/UX, Coherent, Cray, Convex, MINIX, BeOS, V7/V10, …),
  the now-orphaned generic bases `bsd`/`bsd42`/`bsd44`/`bsd44c`/`posix` (retiring
  the `bsd`/`bsd42` keep-caveat from report 5 — their only callers, SGI/MIPS/DNIX,
  were removed here), and obsolete in-family variants (`oldmacosx*`, legacy
  `macosx*`, `machten`, `freebsd1`–`40`, `openbsdold`, old `linux*` variants).
  `linuxa` was kept — `linux`/`gnu-linux` dispatch to it.
- The ~540-line top-of-file platform menu was rewritten to a concise
  Linux/macOS/BSD menu.
- **Method caveat:** GNU make treats blank/comment lines *between* tab-prefixed
  recipe lines as part of one recipe (e.g. HP-UX targets chain several `$(MAKE)`
  calls separated by blanks); stanza boundary detection must consume the whole
  recipe to its last tab-line, or recipe fragments get orphaned onto kept targets.

C source — including the macOS/BSD `SVR4`/`SVR3`/`ATTSV` guardrail — was untouched
by this pass.

---

## Net result

Two orthogonal dead-weight removals: ~6,300 lines of never-compiled C
(`#ifdef COMMENT`) and ~5,100 lines of unbuildable makefile targets. The tree
still builds cleanly with `make linux` and produces a working `wermit`
(`C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)`); the compiled
output is unchanged.
