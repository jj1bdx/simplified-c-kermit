---
name: format-and-editor-tooling
description: clang-format rules (InsertBraces=true is the one non-cosmetic transform, SortIncludes=true with ckcdeb.h pinned first via clang-format off/on islands, format-stability check before commit) and clangd/.clang-tidy editor config (keep in sync with the linux target's CFLAGS; spurious C++-mode diagnostics are not build errors). Use when reformatting, editing .clang-format/.clangd/.clang-tidy, or interpreting clangd diagnostics.
---

# clang-format, clangd, and clang-tidy in this tree

## clang-format

`.clang-format` is git-tracked and the whole `*.c`/`*.h` tree is formatted with it. Two settings
are load-bearing:

- **`InsertBraces: true`** (enabled 2026-06-15, see `doc/INSERT_BRACES_20260615.md`): wraps
  single-statement `if/else/for/while/do` bodies in `{}`. This is the **one non-cosmetic**
  clang-format transform — the added empty block scope perturbs register allocation, so a
  reformat that touches braces is **not** byte-identical. Verify such changes by the
  per-object `.o` + token-proof method from that report (see the build-and-verify skill), not
  whole-binary `cmp`. clang-format 21 handles the two real hazards correctly: dangling-else
  stays bound to the inner `if`, and it refuses to brace a body spanning `#ifdef/#else/#endif`.
- **`SortIncludes: true`** (enabled 2026-06-15, see `doc/ENABLE_SORTINCLUDES_20260615.md` +
  `doc/MOVE_CKCDEB_H_20260615.md`): the master header `ckcdeb.h` must precede every other `ck*.h`,
  so it is wrapped `/* clang-format off */ … /* clang-format on */` as the first `#include` in
  every `*.c`/`*.h` and in both branches of `ckcpro.w` (regenerate `ckcpro.c` after touching
  it). `ckcsig.h` similarly pins its own `<setjmp.h>` (it typedefs `ckjmpbuf` from
  `sigjmp_buf`; the include-category sort would otherwise place `"ckcsig.h"` before
  `<setjmp.h>` and break 5 modules). **Preserve those islands and keep
  `IncludeBlocks: Preserve`.**

After hand-editing a file, confirm it is format-stable before committing:

```sh
clang-format FILE | diff FILE -    # empty output = clean
```

Known non-idempotent files: `ckufio.c`/`ckcfn2.c` (and generated `ckcpro.c`) oscillate under
`ReflowComments` on pathological embedded-space comments — comment-only, codegen-irrelevant,
pre-existing; don't chase it.

Historical note: `ckuver.h`'s herald selector was flattened from an `#else`-nested ladder into
independent first-match-wins `#ifndef HERALD` guard blocks specifically to make it
clang-format-able (see `doc/SIMPLIFY_SUMMARY_20260613.md` §3). Priority is encoded by physical
order + the per-block `#ifndef HERALD` guard: macOS > OpenBSD > NetBSD > FreeBSD > " 4.4BSD".
That block only compiles under `BSD44`, so verify edits to it via `gcc -E -DBSD44 …`, not
`make linux`.

## clangd / clang-tidy

`.clangd` and `.clang-tidy` (both git-tracked) configure clangd for editors such as Helix.
There is no `compile_commands.json`/`bear`: every `.c`/`.h` lives in one flat directory and
compiles with identical flags, so `.clangd`'s `CompileFlags.Add` covers the whole tree.
**Keep it in sync with the `linux` target's `CFLAGS`** — it mirrors that target's
`-funsigned-char` plus the platform `-D` defines (`-DLINUX -DFNFLOAT -DCK_POSIX_SIG
-DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX -DUSE_STRERROR`) so clangd evaluates
the live `#ifdef` branches.

It carries **no `-std=` flag**: the `linux` target dropped its pinned `-std=gnu17`, so the
build (and clangd) use the compiler's default C standard — currently C23, which is what
required the empty-paren signal-handler pointers to gain explicit `(int)` prototypes (see
`doc/SIGNAL_TYPE_20260615.md`). It also deliberately omits `-Wall`/`-Wextra` (the build uses
neither; they only produce unused-variable/sign-compare lint noise on this old code) and the
codegen-only `-O`/`-pipe`. `.clang-tidy` pins clang-tidy to `-*,clang-diagnostic-*` so it adds
no lint beyond the compiler's own diagnostics.

These files are editor-only — the makefile references neither, so they cannot affect the
build. Caveats when reading clangd output:

- `clangd --check`'s "N errors" count is its internal refactoring self-tests, not code
  diagnostics.
- clangd analyzes each header in isolation in **C++ mode**, so it emits spurious diagnostics
  that are NOT build errors — e.g. `ISO C++17 does not allow 'register'` (the C source uses
  `register`) and `unused-includes` warnings. The tree is C and `make linux` is warning-clean;
  do not "fix" these.
