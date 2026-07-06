---
name: build-and-verify
description: Build C-Kermit (make linux → wermit), regenerate ckcpro.c after editing ckcpro.w, and verify changes are behavior-preserving — SOURCE_DATE_EPOCH byte-identical rebuild gate, per-object .o comparison, macOS/BSD #ifdef-path checks, runtime smoke tests. Use whenever building the tree, editing ckcpro.w, or proving a source change does not alter behavior.
---

# Building and verifying C-Kermit

## Build

There is no autoconf/configure; the hand-maintained `makefile` (~1200 lines) contains
per-platform targets (Linux, macOS, modern BSD only) that invoke `make` recursively, passing
options through `KFLAGS`/`LIBS` environment variables. Do not use `make -e`.

The standard build on this machine (Linux):

```sh
make linux
```

Other useful invocations:

```sh
make clean          # removes objects, the wart binary, and generated ckcpro.c
make check          # reports SUCCESS if an executable wermit exists (rm wermit before building so this is meaningful)
```

The resulting binary is always named `wermit`, regardless of target. There is no test suite;
verification is building and running the binary. `ckubuildlog` is a Kermit script used to produce
build-log table entries after a successful build, not part of the build itself.

A build leaves `wermit`, `wart`, and the generated `ckcpro.c` in the tree; all three are
gitignored (along with `*.o`) — do not commit them. `make clean` removes `wart` and `ckcpro.c`
but not `wermit`. (`resume-claude.sh` in the tree is a local Claude Code session helper, also not
part of the build.)

Most platform targets (including `linux`) build via the `xermit` link target, which uses the
select()-based CONNECT module `ckucns.c`. The older `wermit` link target uses the fork()-based
`ckucon.c` instead.

## Protocol changes: ckcpro.w and wart

`ckcpro.c` (the protocol state machine) is **generated**, not hand-edited. It is produced from
`ckcpro.w` (a lex-like grammar) by the `wart` preprocessor (built from `ckwart.c`). This
conversion is NOT run automatically by the platform targets — after editing `ckcpro.w` you must
run, before the normal build:

```sh
make wart
make ckcpro.c
```

## Verification methodology

Every "behavior-preserving" change in this tree is verified at the strongest applicable level.
Pick the level that matches the change:

1. **Byte-identical whole-binary rebuild** — for pure source-spelling changes (macro expansion,
   typedef renames, cosmetic reformatting). Build original and modified trees with a fixed
   timestamp and compare:

   ```sh
   make clean && SOURCE_DATE_EPOCH=1750000000 make linux   # in each tree
   cmp wermit.orig wermit
   ```

   **The build must be fully clean (`make clean` first).** A non-clean rebuild relinks stale
   `.o` whose `__DATE__` perturbs the GNU build-id, giving a spurious early-byte diff; stale
   `.o`s also make a whole-binary diff misleading in the other direction.

2. **Per-object `.o` comparison** — when one file legitimately differs (e.g. an intentional
   user-visible change, or clang-format `InsertBraces` perturbing register allocation). Do the
   fixed-epoch clean build of both trees, compare all `*.o` pairwise, and explain every
   non-identical object (e.g. disassembly at `-O0` showing only label renumbering).

3. **Token-stream proof** — for mechanical text substitutions, show
   `replacer(git HEAD) == new file` exactly, or that a comment/string-aware token stream is
   identical before/after. This covers `#ifdef` branches that never compile on Linux.

4. **macOS/BSD path checks** — `make linux` alone does NOT cover the macOS/BSD `#ifdef`
   branches. Check them with:

   ```sh
   gcc -E -DBSD44 [-DMACOSX10 | -D__OpenBSD__ | -D__NetBSD__ | -DFREEBSD] file.h   # preprocessor-level claims
   cc -fsyntax-only -DBSD44 -DMACOSX ... file.c                                     # syntax-level
   ```

   Remember: `MACOSX10` → `MACOSX` → `BSD44` → `SVR4` → `SVR3`/`ATTSV` are all live there, and
   `POSIX`/`CK_POSIX_SIG` are NOT (they are Linux-target defines).

5. **Runtime smoke tests** — after any signal- or timing-related change, run the battery from
   the signal-handling skill: clean startup/exit, `PAUSE 2` completes in ~2.00s (SIGALRM
   longjmp timer), SIGINT mid-`PAUSE` aborts back to the command loop.

The `make linux` build is warning-clean; keep it that way — a new warning is a regression.
