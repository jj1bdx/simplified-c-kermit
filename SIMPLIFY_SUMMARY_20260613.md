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
| `54bbdfe`→`0e8be87` | `ckuver.h` herald simplification (4 commits) | obsolete herald strings trimmed; nested ladder → flat `clang-format`-friendly guards (§3) |
| `848693b` | Merge `remove-comment` (herald series) | brings the four `ckuver.h` commits into `HEAD` (no new content) |

`git diff --stat` for the range: **41 files changed, 363 insertions(+), 11,596
deletions(-)** (the makefile work also added the two report/plan markdown files
and updated `CLAUDE.md`).

> **Scope note (updated).** At the time `f5456b8` was written, `origin/remove-comment`
> pointed at `3201982` and the four later `ckuver.h` herald-string commits on that
> branch (`54bbdfe`, `5560705`, `9b418f8`, `0e8be87`) were **not** yet in `HEAD`. They
> have since been merged via `848693b` ("Merge branch 'remove-comment'") and are now
> in `HEAD`; `0e8be87` is analyzed in §3 below. The `#ifdef COMMENT` removal of §1
> (`3201982`) touched only a 5-line block in `ckuver.h`; the herald rewrite is separate.

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

## 3. `ckuver.h` herald-detection simplification (`0e8be87`)

`HERALD` is the platform-name string appended to the C-Kermit version banner.
`0e8be87` ("ckuver.h: simplify herald detection", +22/−11, `ckuver.h` only) is the
last of a four-commit series merged via `848693b`
(`54bbdfe` newline trim → `5560705` / `9b418f8` obsolete-string removal → `0e8be87`
the structural rewrite).

### Structural change

The `#ifdef BSD44` herald selector was converted from a **deeply nested
if-else-if ladder** —

```c
#ifndef HERALD
#ifdef MACOSX
#define HERALD " macOS"
#else
#ifdef __OpenBSD__  ... #else #ifdef __NetBSD__ ... #else #ifdef __FreeBSD__ ...
#else #define HERALD " 4.4BSD"  ... (nested #endif chain)
#endif /* HERALD */
```

— into a **flat sequence of five independent guard blocks**, each of the form
`#ifndef HERALD / #ifdef <PLATFORM> / #define HERALD … / #endif / #endif`, with the
fifth block the unconditional `#ifndef HERALD → #define HERALD " 4.4BSD"` fallback.
Priority is no longer encoded by `#else` nesting depth but by **physical order plus
the per-block `#ifndef HERALD` outer guard**: once any block defines `HERALD`, every
later block's `#ifndef HERALD` is false and is skipped. The `POSIX`/`__linux__` block
and the `" Unknown Platform"` catch-all are unchanged apart from cosmetic blank lines.

### Semantic equivalence (verified per configuration)

First-match-wins priority — **macOS > OpenBSD > NetBSD > FreeBSD > " 4.4BSD"** — is
preserved exactly; the load-bearing element is the per-block `#ifndef HERALD` guard,
which reproduces the old `#else`-chain semantics.

| Build config | `BSD44`? | HERALD before | HERALD after |
|---|---|---|---|
| macOS (`MACOSX`, → `BSD44`) | yes | `" macOS"` | `" macOS"` |
| OpenBSD (`__OpenBSD__`) | yes | `" OpenBSD"` | `" OpenBSD"` |
| NetBSD (`__NetBSD__`) | yes | `" NetBSD"` | `" NetBSD"` |
| FreeBSD (`__FreeBSD__`) | yes | `" FreeBSD"` | `" FreeBSD"` |
| generic `BSD44`, none above | yes | `" 4.4BSD"` | `" 4.4BSD"` |
| Linux (`POSIX`+`__linux__`) | no | `" Linux"` | `" Linux"` |
| none matched | no | `" Unknown Platform"` | `" Unknown Platform"` |

Co-definition is real for `MACOSX`+`BSD44` (the `MACOSX → BSD44` implication chain in
`ckcdeb.h`), and the order correctly resolves to `" macOS"` with `" 4.4BSD"` never
reached. `MACOSX` and `__FreeBSD__` never co-occur in practice (macOS defines
`__APPLE__`, not `__FreeBSD__`; `ckcdeb.h` already treats them as exclusive branches),
so the macOS-over-FreeBSD ordering is defensive, not relied upon. Each native BSD
defines exactly one `__*BSD__` macro.

### Rationale

The file comment (lines 17–21) states the motivation is **tooling, not behavior**: the
deeply nested `#ifdef/#else` ladder rendered `clang-format` "simply unusable due to
parsing workload"; the flat independent-guard form is `clang-format`-friendly
(consistent with the untracked `clang-format-config` in the tree).

### Safety

A pure cosmetic/`clang-format` refactor, semantically equivalent for every build
configuration. The `make linux` build is **byte-identical** across the commit: on
Linux `BSD44` is undefined, so the entire rewritten block is preprocessed away and the
unchanged `POSIX`/`__linux__` → `" Linux"` path is what compiles. The change is only
exercised by a macOS or BSD build (`-DBSD44 [-DMACOSX|-D__OpenBSD__|…]`), verifiable
with `gcc -E -DBSD44 … ckuver.h` against the pre-commit file.

---

## Net result

Three orthogonal cleanups: ~6,300 lines of never-compiled C (`#ifdef COMMENT`),
~5,100 lines of unbuildable makefile targets, and a behavior-preserving
`clang-format`-friendly rewrite of the `ckuver.h` herald selector. The tree still
builds cleanly with `make linux` and produces a working `wermit`
(`C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)`); the compiled
output is unchanged.
