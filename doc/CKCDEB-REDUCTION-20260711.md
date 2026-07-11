# C-Kermit 10.0 Beta.12 — `ckcdeb.h` obsolete-entry removal verified functionally equivalent

**Subject:** commit `3d183bd` ("ckcdeb.h: remove obsolete entries and comments") on branch
`reduce-ckcdeb`, compared against `wart-cpp-comment`, hard-wires several previously-guarded
`#define`s, deletes dead `#ifdef` blocks, restructures one cascade to use `#else`, and fixes a
few `#endif` trailing-comment labels — all confined to the single master header `ckcdeb.h`. This
report verifies, by fixed-epoch build comparison, macro-definition-dump equivalence across every
live platform family, and cross-reference analysis of every touched symbol, that the change
introduces **no functional change** to `wermit`.

- **Branches compared:** `wart-cpp-comment` (`5d63508`) vs `reduce-ckcdeb` (`757c7d9`, merges
  `wart-cpp-comment` and adds one commit, `3d183bd`, on top). `git diff
  wart-cpp-comment..reduce-ckcdeb` touches exactly one file: `ckcdeb.h`
  (45 insertions, 129 deletions).
- **Date:** 2026-07-11
- **Method:** the repo's build-and-verify methodology, all applicable levels — level 1
  (`SOURCE_DATE_EPOCH` byte-identical clean rebuild), level 4 (macOS/BSD `#ifdef`-path
  `gcc -E -dM` equivalence dumps for every live platform family, including the Linux target as a
  cross-check), a full cross-reference grep of every touched symbol against the whole tree and
  the makefile, and a runtime smoke test.

---

## TL;DR

1. **Verdict: functionally equivalent.** The `wermit` binary is **byte-identical** between the
   two branches (`cmp` clean, matching SHA-256, identical size 2,655,672 bytes). Both clean
   builds are warning-free.
2. `gcc -E -dM` macro-definition dumps of `ckcdeb.h` under five platform-flag families — the
   actual Linux `make linux` flags, the actual macOS (`MACOSX10`/`BSD44`) flags, and generic/
   FreeBSD/OpenBSD/NetBSD (`BSD44`) flags, all lifted verbatim from the makefile's real targets —
   show **exactly the same two differences in every one of the five families**: the intentional
   `CK_SLEEPINT 250` → `(250)` reparenthesization (cosmetic; the macro has **zero consumers**
   anywhere in the current tree) and the removal of the now-orphaned `NO_FTP_AUTH` macro (zero
   consumers, confirmed by tree-wide grep). No other macro — including `NOWTMP`, `NOSYSLOG`,
   `NOFTP`, `NOTELNET`, `NORLOGIN`, `NOCKXYZ`, `CK_ANSILIBS`, `CKMAXNAM`, `CKREALPATH`,
   `CKSYMLINK`, `ZCHKPID`, `UNIX` — differs on any of the five platform families.
3. **One deliberate, pre-existing-dead behavior delta, clearly flagged as intentional:** the
   `DOWTMP`/`DOSYSLOG` "unless explicitly requested" escape hatches are removed. Empirically
   confirmed (`gcc -E -dM -DDOWTMP -DDOSYSLOG`) that these hatches *did* work in the old branch
   for a user who knew to pass `-DDOWTMP`/`-DDOSYSLOG` by hand, but **no makefile target ever
   did so** (`grep` finds zero occurrences in `makefile`), so removing them changes nothing for
   any shipped build. A related, independent finding: the macOS makefile target's own comment
   claiming `KFLAGS=-UNOWTMP` re-enables Wtmp logging was **already non-functional before this
   diff** (verified: `-UNOWTMP` alone never suppressed the header's `#define NOWTMP` even on the
   old branch, since nothing predefines `NOWTMP` via `-D` for `-U` to cancel) — a pre-existing
   makefile documentation bug, unrelated to and not worsened by this change.
4. Runtime smoke test on the built `reduce-ckcdeb` `wermit`: clean startup/exit, version banner
   intact (`C-Kermit 10.0.416 Beta.12, 2025/03/22`), `SHOW FEATURES` lists `NOFTP`, `NOSYSLOG`,
   `NOWTMP`, `CK_ANSILIBS` as expected.

---

## 1. What the commit changes

```
3d183bd ckcdeb.h: remove obsolete entries and comments
```
> The following compilation flags are now permanently defined within the header file: NOWTMP,
> NOSYSLOG, NOFTP, NOTELNET, NORLOGIN, CK_ANSILIBS, ZCHKPID, NOCKXYZ
>
> The following unused compilation flags are removed: CKNTSIG, NO_FTP_AUTH, VMS64

Full diff reviewed (`git diff wart-cpp-comment..reduce-ckcdeb`); every hunk falls into one of
eight categories:

| # | Change | Kind |
|---|---|---|
| 1 | Header banner reworded; "OBSOLETED" note added about the old #ifdef-only preprocessor restriction | comment-only |
| 2 | `NOWTMP`, `NOSYSLOG` now `#define`d unconditionally; `#ifndef DOWTMP`/`#ifndef DOSYSLOG` escape hatches removed | guard removal |
| 3 | `NOFTP`, `NOTELNET`, `NORLOGIN`, `NOCKXYZ` now `#define`d unconditionally, dropping their (already dead, no-escape-hatch) `#ifndef` wrappers | guard removal |
| 4 | `#define CK_ANSILIBS` added unconditionally; the `#ifdef CK_ANSILIBS … #else … #endif` wrapper around the string-library/prototype section removed (its `#else` branch was empty comments) | guard removal |
| 5 | Dead blocks deleted: `CKNTSIG` (Windows NT signal hack), `VMS64`/`FLT_NOT_DBL` else-branch, `NO_FTP_AUTH`, the `#ifndef ZCHKPID → #define zchkpid(x) 1` fallback, two empty `#ifdef UNIX … #endif` pairs, empty `#ifndef NOREALPATH` scaffolding, the System V/68 comment | dead-code deletion |
| 6 | `CK_SLEEPINT` changed from `250` (with a line-continuation comment) to `(250)` | reparenthesization |
| 7 | `CKMAXNAM` cascade restructured to use `#else` (previously forbidden by the project's old preprocessor-portability rule, now lifted) | structural, same semantics |
| 8 | Several `#endif` trailing-comment labels corrected (e.g. the `CKREALPATH`/`CKSYMLINK` blocks where the labels were swapped) | comment-only |

## 2. Build verification (byte-identical rebuild gate)

A worktree of `wart-cpp-comment` was created in the scratch area
(`git worktree add .../scratchpad/base wart-cpp-comment`). Both trees were built from scratch:

```sh
make clean && SOURCE_DATE_EPOCH=1750000000 make linux
```

| Check | Result |
|---|---|
| Build exit code | 0 in both trees |
| Compiler warnings (`grep -i warning` on both build logs, excluding benign `USE_STRERROR` substring matches) | **none** in either tree |
| `wermit` size | 2,655,672 bytes, identical in both trees |
| `cmp wermit.base wermit.reduce` | **identical** (exit 0) |
| SHA-256 | `af29116326706a9ba81444806fe0c360f68735824bc9565acfc43f09718bab65` — matches for both |
| Per-object `.o` comparison (28 objects each) | **all byte-identical** (`cmp -s`, looped over every `*.o`) |

This is the strongest evidence available: **whole-binary, byte-for-byte identity**, exactly as
expected for a diff confined to preprocessor directives and comments. (One irrelevant finding
during this step: a stale, untracked `ckcftp.o` — left over from a build predating the FTP
client's removal on 2026-07-09, and no longer part of any link target — was sitting in the
working tree and inflated the object count to 29/28 before being deleted; it played no part in
either build or link.)

## 3. macOS/BSD preprocessor-path equivalence

`make linux` alone does not exercise the `BSD44`/`MACOSX` `#ifdef` arms. Per the project's
verification methodology and `CLAUDE.md`'s explicit instruction not to assume "always undefined"
without checking the macOS/BSD builds, `gcc -E -dM` macro dumps of `ckcdeb.h` were taken from
both branches under five flag sets, each lifted verbatim from the corresponding real makefile
target (`linux`, `macos`, the generic/FreeBSD `CFLAGS`, `openbsd`, `netbsd`):

| Target family | Flags used (from `makefile`) |
|---|---|
| Linux (actual `make linux` invocation, captured from the build log) | `-DLINUX -DCK_POSIX_SIG -DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX -DUSE_STRERROR -DCK_NCURSES -DHAVE_PTMX -DHAVE_CRYPT_H -DHAVE_OPENPTY -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64` |
| macOS (`macos` target) | `-DMACOSX10 -DMACOSX103 -DCK_NCURSES -DTCPSOCKET -DCKHTTP -DUSE_STRERROR -DUSE_NAMESER_COMPAT -DNOCHECKOVERFLOW -DFNFLOAT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DNODCLINITGROUPS -DNOUUCP` |
| Generic/FreeBSD (`freebsd*` targets) | `-DBSD44 -DCK_NCURSES -DCK_NEWTERM -DTCPSOCKET -DNOCOTFMC -DFREEBSD4 -DUSE_UU_LOCK -DFNFLOAT -DTPUTSARGTYPE=int -DUSE_STRERROR` |
| OpenBSD (`openbsd` target) | `-DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD -DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR` |
| NetBSD (`netbsd`/`netbsd2` target) | `-DTIMEH -DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR -DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DFNFLOAT` |

For every one of the five families, `MACOSX10`/`BSD44` were confirmed to correctly cascade to
`MACOSX`→`BSD44`→`SVR4`→`SVR3`/`ATTSV`→`UNIX` (per `CLAUDE.md`'s chain — `#define UNIX` present
exactly once in every dump), so the comparison genuinely exercises the live SVR4/SVR3/ATTSV code
paths rather than accidentally skipping them.

**Result: identical outcome in all five families.** The diff between the old-branch dump and the
new-branch dump is, in every single case, exactly these two lines:

```diff
< #define CK_SLEEPINT 250
---
> #define CK_SLEEPINT (250)
< #define NO_FTP_AUTH
```

No other macro differs — confirmed specifically (not just by absence from the generic diff) for
`NOWTMP`, `NOSYSLOG`, `NOFTP`, `NOTELNET`, `NORLOGIN`, `NOCKXYZ`, `ZCHKPID`, `CK_ANSILIBS`,
`CKREALPATH`, `CKSYMLINK`, and `CKMAXNAM` (resolves to `FILENAME_MAX` identically in all five
families, both branches). Zero preprocessor errors/warnings on any of the ten runs (5 families ×
2 branches).

**Methodology note (worth recording for future sessions):** this repo's interactive shell is
`zsh`. An initial pass storing multi-flag strings like `LINUX_FLAGS="-DA -DB -DC"` and expanding
them unquoted (`gcc -E -dM $LINUX_FLAGS file.h`) silently failed — zsh does not word-split
unquoted parameter expansions by default (unlike `sh`/`bash`), so the entire flag string
collapsed into a single bogus argument and none of the intended `-D`s took effect (verified: this
dropped the dump from ~2,595 lines to ~2,128 and `UNIX` went undefined). All five dumps above
were regenerated inside `bash -c '...'` and cross-checked (`grep -c "^#define UNIX "` = 1 in
every case, and total line counts back to the expected ~2,560–2,600) before being relied upon.

## 4. Cross-reference analysis of every removed/changed symbol

- **`DOWTMP` / `DOSYSLOG`:** zero references anywhere in `makefile` or any `*.c`/`*.h`/`*.w` file
  (`grep -rn "DOWTMP\|DOSYSLOG"` across the whole tree returns nothing). No shipped target ever
  set them. Empirically, in the old branch `-DDOWTMP -DDOSYSLOG` did correctly suppress
  `NOWTMP`/`NOSYSLOG` (confirmed with `gcc -E -dM`), so the escape hatch was real but **entirely
  unreached by any build in this tree** — removing it is a no-op for every default and every
  documented build. Flagged as an intentional, effect-free change (see TL;DR item 3 for the
  related pre-existing `-UNOWTMP` makefile-comment finding, which is independent of this diff).
- **`-DNOFTP`/`-DNOTELNET`/`-DNORLOGIN`/`-DNOCKXYZ`/`-DNOWTMP`/`-DNOSYSLOG` in the makefile:**
  `grep -n` over `makefile` finds these symbols only inside two comment lines in the `macos`
  target block (discussed above); no target's `CFLAGS`/`KFLAGS` passes any of them as a `-D`.
  Since the linux build log is confirmed warning-free, no macro-redefinition warning risk
  materializes.
- **`CK_ANSILIBS`:** consumers are `ckuus5.c:9577` (`SHOW FEATURES` listing) and
  `ckuusx.c:739`/`ckutio.c:9812` (comment only). In the **old** branch, `CK_ANSILIBS` was already
  effectively unconditional on every live platform via two independent chains already present in
  `ckcdeb.h` *before* this diff: `#ifdef POSIX → #define _POSIX_SOURCE` (old line 631) together
  with `#ifdef _POSIX_SOURCE → #define CK_ANSILIBS` (old line 2886) covers Linux (`-DPOSIX`), and
  `#ifdef SVR4 → #define CK_ANSILIBS` (old line 2891) covers every macOS/BSD family via the
  `BSD44→SVR4` chain. The macro dumps in §3 confirm both branches define `CK_ANSILIBS` on all
  five families — the new unconditional `#define CK_ANSILIBS` near the top of the file simply
  makes explicit what was already true everywhere; the guarded `#ifdef CK_ANSILIBS` section it
  now always enters was, in the old branch, *also* already always entered on every supported
  target (its `#else` branch — for pre-ANSI compilers — was dead code on this Unix-only tree).
- **`NO_FTP_AUTH`, `CKNTSIG`, `VMS64`:** zero remaining references anywhere in the tree
  (`grep -rn` across all `*.c`/`*.h`/`*.w`). Confirmed dead before removal.
- **`zchkpid`/`ZCHKPID`:** the real function `int zchkpid(unsigned long xpid) { return
  ((kill((PID_T)xpid, 0) < 0) ? 0 : 1); }` is defined unconditionally in `ckufio.c:2771` (not
  gated by `ZCHKPID` at all), its prototype is unconditional in `ckcdeb.h`, and it is called from
  `ckuusx.c:6788` and `ckuusx.c:6881`. The removed fallback (`#ifndef ZCHKPID #define
  zchkpid(x) 1 #endif`) sat immediately after an *unconditional* `#define ZCHKPID` two lines
  above it in the **old** file too — meaning that `#ifndef ZCHKPID` could never have been true
  even before this diff. The deleted block was already unreachable dead code; `ZCHKPID` remains
  defined exactly as before (confirmed in all five macro dumps).
- **`CK_SLEEPINT`:** tree-wide grep (`grep -rln "CK_SLEEPINT"`) finds it only in `ckcdeb.h` itself
  and two `doc/*.md` reports — **zero live use sites** anywhere in `*.c`/`*.h`. (Its comment
  references `msleep()`/`in_chk()`, but neither currently reads this macro.) With no consumer,
  there is nothing for the added parentheses to interact with via stringification or token
  pasting; the change is inert by construction, and the byte-identical binary in §2 confirms it
  independently.
- **`CKMAXNAM`:** the restructured `#else` cascade was checked with `gcc -E -dM` and resolves to
  `FILENAME_MAX` identically in both branches across all five platform families (§3). The
  restructuring only changes which nested `#ifndef`/`#endif` pair encloses the fallback path; the
  effective winning definition (`MAXNAMLEN` if defined, else `FILENAME_MAX`, else the
  `_POSIX_NAME_MAX`/`DIRSIZ`/`sun` fallback chain) is unchanged, and none of the live targets
  define `MAXNAMLEN` so all five land on the `FILENAME_MAX` branch in both branches.
- **`CKREALPATH`/`CKSYMLINK`:** the swapped `#endif` trailing-comment labels are cosmetic; both
  macros are confirmed `#define`d under exactly the same conditions in both branches across all
  five families (§3) — `CKREALPATH` unconditionally (via `#ifndef NOREALPATH`, nothing defines
  `NOREALPATH`), `CKSYMLINK` under `#ifdef UNIX` (true on all five families) and `#ifndef
  NOSYMLINK` (nothing defines `NOSYMLINK`).

## 5. Runtime smoke test

```
$ ./wermit -Y -q -C "show version,exit"
 C-Kermit 10.0.416 Beta.12, 2025/03/22
 ... (full version banner, all module versions, unchanged)

$ ./wermit -Y -q -C "show features,exit" | grep -E "NOFTP|NOTELNET|NORLOGIN|NOCKXYZ|NOWTMP|NOSYSLOG|CK_ANSILIBS"
 BSD44ORPOSIX CK_64BIT CK_ANSIC CK_ANSILIBS CK_APC CK_AUTODL CK_CURSES
 NETPTY NOARROWKEYS NOCOTFMC NOFILEH NOFTP NOKVERBS NOSETBUF NOSYSLOG NOWTMP

$ echo exit | ./wermit
C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)
 Copyright (C) 1985, 2025, ...
$ echo $?
0
```

Clean startup and exit in both invocation styles; version banner and `SHOW FEATURES` output as
expected. Given the byte-identical binary from §2, this is confirmatory rather than load-bearing.

## 6. Conclusion

The `reduce-ckcdeb` branch's `ckcdeb.h` reduction (commit `3d183bd`) is a behavior-preserving
cleanup, proven at the strongest applicable level of the repo's verification methodology:
**byte-identical `wermit` binary and all 28 object files**, corroborated by `gcc -E -dM`
macro-dump equivalence across all five live platform-flag families (Linux, macOS, generic/
FreeBSD, OpenBSD, NetBSD) showing only the two intentional, consumer-free differences
(`CK_SLEEPINT` parenthesization, `NO_FTP_AUTH` removal), a full cross-reference audit of every
touched symbol against the whole tree and the makefile, and a clean runtime smoke test. The one
genuine behavior delta — the `DOWTMP`/`DOSYSLOG` escape hatches becoming permanently inert — is
deliberate and provably has zero effect on any build this tree can produce, since no makefile
target, documented or otherwise, ever set them.

## Appendix: verification artifacts

Performed in a session-temporary worktree (`.../scratchpad/base`, since removed):
`git worktree add`; `make clean && SOURCE_DATE_EPOCH=1750000000 make linux` clean builds of both
branches; `cmp`/`sha256sum` on `wermit`; a `cmp -s` loop over all 28 `*.o` pairs; `gcc -E -dM`
macro dumps of `ckcdeb.h` (via `bash -c` for correct word-splitting under the `zsh` interactive
shell) for the Linux/macOS/FreeBSD/OpenBSD/NetBSD flag sets on both branches, diffed and
spot-checked for `UNIX`/`CK_ANSILIBS`/`CKREALPATH`/`CKSYMLINK`/`CKMAXNAM`/`NOWTMP`/`NOSYSLOG`/
`NOFTP`/`NOTELNET`/`NORLOGIN`/`NOCKXYZ`/`ZCHKPID`; targeted `-DDOWTMP`/`-DDOSYSLOG`/`-UNOWTMP`
probes against the old branch; tree-wide `grep -rn` for `DOWTMP`, `DOSYSLOG`, `NO_FTP_AUTH`,
`CKNTSIG`, `VMS64`, `zchkpid`/`ZCHKPID`, `CK_SLEEPINT`, `CK_ANSILIBS` across `*.c`/`*.h`/`*.w`/
`makefile`; runtime `show version`/`show features`/plain-exit smoke test of the built
`reduce-ckcdeb` `wermit`.
