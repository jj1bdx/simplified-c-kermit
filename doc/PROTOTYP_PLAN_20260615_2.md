# VOID / CKVOID / CONST alias expansion and removal — 2026-06-15

## Summary

Expanded and removed the last three portability alias macros in the tree. Every
exact-token use of `VOID`, `CKVOID`, and `CONST` was mechanically rewritten to the
plain ANSI keyword, and the three `#define` blocks were deleted from `ckcdeb.h`:

| Macro    | Was (`ckcdeb.h`)     | Now      |
|----------|----------------------|----------|
| `VOID`   | `#define VOID void`  | `void`   |
| `CKVOID` | `#define CKVOID void` | `void`  |
| `CONST`  | `#define CONST const` | `const` |

This mirrors the `_PROTOTYP()` expansion done earlier the same day
(`SIMPLIFY_20260615.md`). After this change **no portability alias macros remain** in
the tree — only the C keywords `void`/`const` are used.

These were pre-ANSI fallbacks (so the code could degrade to empty/`int`/nothing on
compilers lacking `void`/`const`). `CKVOID` additionally existed only to dodge a clash
between the `VOID` *macro* and `curses.h` inside `ckuusx.c`. Since the tree now assumes
ANSI/ISO C (`CK_ANSIC` always defined, K&R paths already removed), all three are
unconditionally the ANSI keyword and the macros are pure noise.

## Scope of change

- **41 tracked source files** changed, **685 insertions / 716 deletions**
  (688 token substitutions plus the structural deletions below).
- `ckcpro.c` (generated, gitignored) was regenerated from the edited `ckcpro.w` via
  `make wart && make ckcpro.c`; never hand-edited.
- Per-macro: 617 `VOID`, 4 `CKVOID`, 86 `CONST` original occurrences (the deleted
  `#define`/comment lines account for the difference vs. the 688 substitutions).

## Method

A comment/string-aware, exact-identifier replacer (`/tmp/expand_aliases.py`) walked the
C source recognizing `//` and `/* */` comments, string literals, and char literals, and
replaced **only** the bare identifier tokens `VOID`→`void`, `CKVOID`→`void`,
`CONST`→`const` in actual code. Word-boundary exact matching:

- **Protected** sibling macros that share substrings and must stay untouched:
  `MAINISVOID`, `DMAINISVOID`, `TPUTSISVOID`, `TPUTSARG1CONST`, `XXVOID` (the user
  command code `232`). (`PAM_CONST` is protected because `_` is a word character.)
- **Skipped** all false positives, which were all in comments or one string literal:
  `ckupty.c:1006`, `ckcdeb.h:675`, `ckcnet.c:1163` (OS-9 build note), `ckuusr.c:1604`
  & `ckuusr.h:684` (`/* VOID */` beside `XXVOID`), and crucially the user-facing
  Kermit `VOID` script-command help text at `ckuus2.c:6009` (`"Syntax: VOID text…"`).

## Special-case sites (manual, beyond the mechanical replace)

1. **`ckcdeb.h`** — deleted the three `#ifndef/#define/#endif` definition blocks and
   their now-obsolete explanatory comments ("void type, normally available only in ANSI
   compilers…", "Exactly the same as VOID but…", "Const type"). Genuine `VOID` uses in
   the rest of the file (e.g. `typedef VOID MAINTYPE;`) were expanded normally.
2. **`ckuusx.c`** — deleted the `#ifndef MYCURSES / #undef VOID / #endif` block (a no-op
   once `VOID` is no longer a macro; system `curses.h`/`ncurses.h` do not define `VOID`,
   and the lowercase `void` keyword cannot collide) and the stale
   `/* Can't use VOID because of curses.h */` comment. The lone `CKVOID` (`fxdinit`)
   became `void`.
3. **`ckufio.c:412`** — `#define PAM_CONST CONST` → `#define PAM_CONST const`. The
   `PAM_CONST` macro itself was kept (separate PAM-specific alias, out of scope); only
   its body expanded.
4. **`ckcpro.w`** — 8 `VOID` sites expanded; `ckcpro.c` regenerated.

No `#ifdef VOID` / `#ifdef CONST` feature tests exist anywhere; the only directives that
referenced these tokens were the 3 definition guards and the 1 `#undef`, all deleted, so
there is zero conditional-compilation impact.

## Verification

1. **Byte-identical Linux build (primary).** `SOURCE_DATE_EPOCH=1700000000 make linux`
   before and after produced **`cmp`-identical `wermit`** (md5 `dd08c4c0…`). The `linux`
   target auto-detects ncurses and links `-lncurses`, so this build already compiles the
   curses section affected by the `#undef VOID` removal.
2. **All `#ifdef` paths (incl. macOS/BSD) — textual equivalence.** For each of the 39
   purely-mechanical files, running the replacer on the original (`git show HEAD:`)
   version reproduced the new file **exactly** (`cmp -s`). Since the textual substitution
   is identical to the preprocessor's own object-like-macro expansion, post-preprocessing
   token streams are identical on *every* platform path (MACOSX/BSD44/CK_CURSES/
   CK_NCURSES/MYCURSES/CK_PAM), not just Linux — a stronger guarantee than a single
   `gcc -E` spot check. The two structurally-edited files (`ckcdeb.h`, `ckuusx.c`) were
   reviewed by hand.
3. **Sanity grep.** No exact-token `VOID`/`CKVOID`/`CONST` remains in code anywhere
   (only the 6 comment/string sites above); all sibling macros still present.
4. **`make check` → `SUCCESS`**; `./wermit` starts and prints the version banner.

A **c-expert** sub-agent analysis validated the approach (GO) before execution.
