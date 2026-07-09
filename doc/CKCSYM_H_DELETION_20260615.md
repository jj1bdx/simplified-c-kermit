# Removal of `ckcsym.h` and all its inclusions — 2026-06-15

## Summary

`ckcsym.h` and every `#include "ckcsym.h"` were removed from the tree, and the
header's prerequisite token was dropped from the makefile dependency lists. The
change is **byte-identical** on the `linux` build (verified by `cmp`) and is
behavior-preserving on every `#ifdef` path.

## Why

`ckcsym.h` existed for compilers that *"don't have the capability to `#define`
symbols on the C compiler command line"* — symbols would be hard-coded in this
header, `#include`d before all other `ck*.h` files. On this Linux/macOS/Unix-
only tree (gcc/clang, which take `-D`), that mechanism is obsolete, and the file
had become **content-free**: its entire body was a 5-line `/* */` comment with
**zero `#define`s**. Every `#include "ckcsym.h"` therefore expanded to nothing.

This continues the simplification effort (cf. the earlier `Remove ckusig.h` and
`Remove cku2tm.c` commits).

## Changes

### 1. Removed `#include "ckcsym.h"` from 30 `.c` files

The full include line (including any trailing comment that referred to
`ckcsym`, e.g. `/* Compilation options */`, `/* This must go first */`,
`/* Needed for Stratus VOS */`, `/* Symbol definitions */`, `/* Includes... */`,
`/* Standard includes */`) was deleted from each file. No `.h` file included it.

Files (29 `ck*.c` + `ckwart.c`):
`ckcfn2.c ckcfn3.c ckcfns.c ckcftp.c ckclib.c ckcmai.c ckcmdb.c ckcnet.c
ckctel.c ckcuni.c ckucmd.c ckucns.c ckucon.c ckudia.c ckufio.c ckupty.c
ckuscr.c ckusig.c ckutio.c ckuus2.c ckuus3.c ckuus4.c ckuus5.c ckuus6.c
ckuus7.c ckuusr.c ckuusx.c ckuusy.c ckuxla.c ckwart.c`

`ckcpro.w` did **not** include `ckcsym.h`, so the generated `ckcpro.c` is
unaffected — no `wart`/`ckcpro.c` regeneration was needed.

### 2. Removed the stale documentation comment in `ckcmai.c`

A 5-line comment block (formerly above the include) that described what
`ckcsym.h` was for was removed, since it now documents nothing:

```c
/*
  ckcsym.h is used for defining symbols that normally would be defined
  using -D or -d on the cc command line, for use with compilers that don't
  support this feature.  Must come before any tests for preprocessor symbols.
*/
```

### 3. Removed `ckcsym.h` from makefile dependency lists

The `ckcsym.h` prerequisite token (26 occurrences across ~22 dependency lines,
including the stale one on the `ckcpro.c` target) was deleted from the
"Dependencies for each module" block. The `ckctel.$(EXT):` line — where
`ckcsym.h` was the *first* prerequisite right after the colon — now begins with
`ckcdeb.h`. These are dependency declarations only; removing them does not
affect compilation output.

### 4. Deleted the orphaned header

`ckcsym.h` was removed via `git rm`.

## Verification

- **Byte-identical `linux` build.** With `SOURCE_DATE_EPOCH` pinned to the HEAD
  commit time, a clean `make linux` of the modified tree and of a pristine
  `HEAD` worktree both succeeded (`make` exit 0, `make check` → SUCCESS,
  warning-clean) and produced **`cmp`-identical** `wermit` binaries
  (2,859,968 bytes each). This is expected: removing includes of a comment-only
  header emits no preprocessor tokens.

  ```sh
  SDE=$(git log -1 --format=%ct)
  git worktree add --detach /tmp/ckcsym-head-wt HEAD
  ( cd /tmp/ckcsym-head-wt && make clean && SOURCE_DATE_EPOCH=$SDE make linux )
  make clean && SOURCE_DATE_EPOCH=$SDE make linux
  cmp wermit /tmp/ckcsym-head-wt/wermit   # IDENTICAL
  ```

- **macOS/BSD paths.** Equally unaffected — the header was comment-only on every
  `#ifdef` path, so its removal emits no tokens regardless of platform macros.

- **No stragglers.** `grep -rn ckcsym . --exclude-dir=.git` now matches only
  prose in `FORMAT_H_PLAN_20260614.md` (a prior plan document, intentionally
  left untouched).
