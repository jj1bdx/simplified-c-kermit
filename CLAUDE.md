# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

C-Kermit 10.0 Beta.12 test repository (serial/network communication software, file transfer via the Kermit protocol). Original author: Frank da Cruz, kermitproject.org. Changelog: https://www.kermitproject.org/ckdaily.html#changelog

Windows, OS/2, and VMS support was removed from this tree on 2026-06-12 (see `SIMPLIFY_20260612.md`): all `ckv*` files were deleted and the corresponding `#ifdef` blocks stripped with unifdef. That state is the initial git commit (`5a8fc5a`). Android support was removed the same day in commit `ad4139f` (see `SIMPLIFY_20260612_2.md`). SSL/TLS and Kerberos support (and with them the whole Telnet authentication/encryption stack: KRB4, SRP, DES/CAST) was removed on 2026-06-13 (see `SIMPLIFY_20260613.md`): the `ck_*` security files plus `ckuath.*`/`ckuat2.h` were deleted and all `+ssl`/`+krb*`/`+srp` makefile targets removed.

Four further simplifications followed on 2026-06-13, on the premise that this is now a Linux/macOS/Unix-only tree:

- **Non-ANSI / K&R C removed** (commit `20bf398`, see `SIMPLIFY_20260613_2.md`): all pre-ANSI fallback branches were stripped on the assumption that `CK_ANSIC` is always defined. The portability aliases (`VOID`, `CKVOID`, `CONST`, …) are now unconditional ANSI definitions but were intentionally left in use at call sites. (`_PROTOTYP` was left in use here too, then fully expanded and removed on 2026-06-15 — see the `_PROTOTYP` bullet below.)
- **26 obsolete platform macros removed from the C source** (commit `c1c710b`, see `SIMPLIFY_20260613_3.md`): `AS400 __386BSD__ C70 CIE SINIX SNI544 SNI543 SNI541 POWERMAX ICL_SVR4 XF68R3V6 XF88R32 IX370 PCIX sxaE50 RTU TRS16 _386BSD OU8 UW7 NEUTRINO LYNXOS UTS24 VXVE ZILOG AMIX` were treated as permanently undefined and their dead `#ifdef` branches deleted with `unifdef`.
- **38 dead makefile targets removed** (commit `aca665a`): the per-platform targets that defined those 26 flags (SINIX/SNI, UnixWare 7/OpenUNIX, ICL SVR4, LynxOS, Concurrent RTU-BSD, 386BSD, PowerMAX, CIE, TRS-16, QNX Neutrino 2, PC/IX, IX/370, UTS24, C/70, ZEUS, VX/VE, SX/A), plus their alias/dispatcher targets and the matching top-of-file platform-menu entries.
- **Stale `-DRTU` comments removed** (commit `dd1cccd`).
- **64 dead Xenix and ancient-BSD makefile targets removed** (commit `86e1cb0`, see `SIMPLIFY_20260613_5.md`): SCO Xenix (`sco86`/`sco286*`/`sco386*`/`sco3r2*`/`sco234*`, `xenix`), Altos Xenix (`altos*`), pre-4.4 BSD (`bsd29`/`bsd41`/`bsd43*`/`bsdhdb`/`bsdlck`/`bsdix`/`rtacis`/`apollobsd`/`sr10-bsd`), BSDI (`bsdi*`), and SunOS 3/4 (`sunos3gcc`/`sunos4*`/`sunos41*`), plus their platform-menu entries. **Kept** the SCO UNIX/OpenServer targets (`sco3r22*`, `sco32v*`, `sco_osr600`) and `altos3` — these are System V, not Xenix — and `bsd44`/`bsd44c`. **Also kept the generic `bsd` and `bsd42` targets**: they are the shared dispatch base for the still-present (out-of-scope) SGI IRIX (`iris`/`irix40`), MIPS (`mipstcpc`), and DNIX (`dnix*`) targets, so removing them would cascade beyond scope.
- **All remaining obsolete/not-buildable makefile targets removed** (see `OS_SIMPLIFY_PLAN_20260613_2.md` for the plan and `SIMPLIFY_20260613_6.md` for the report): the makefile now keeps **only** Linux, macOS, and modern-BSD (FreeBSD/NetBSD/OpenBSD/MirBSD) targets — 487 stanzas / 514 target names removed, **`makefile` 6,302 → 1,202 lines**. Removed: all foreign-Unix families (SGI IRIX, MIPS, DNIX, Apollo, Encore, AT&T SysV `att*`/`sys3*`/`sys5*`, HP-UX, AIX, SCO/Unixware, Solaris/SunOS5, DEC Ultrix/OSF/Tru64, Sequent, NeXT, QNX, Interactive, DG/UX, Coherent, Cray, Convex, MINIX, BeOS, V7/V10, …), the now-orphaned generic bases **`bsd`/`bsd42`/`bsd44`/`bsd44c`/`posix`** (this **retires the `bsd`/`bsd42` keep-caveat above** — their only callers, SGI/MIPS/DNIX, are gone), and obsolete in-family variants (`oldmacosx*`, legacy `macosx*`, `machten`, `freebsd1`–`40`, `openbsdold`, old `linux*` variants). **Kept `linuxa`** — `linux`/`gnu-linux` dispatch to it. The top-of-file platform menu was rewritten. **Method caveat:** GNU make treats blank/comment lines *between* tab-prefixed recipe lines as part of one recipe (e.g. the HP-UX targets chain several `$(MAKE)` calls separated by blanks); stanza-removal boundary detection must consume the whole recipe to its last tab-line, or it orphans recipe fragments onto kept targets.
- **`#ifdef COMMENT` dead-code blocks removed from the C source** (commit `3201982`, merged via `f5456b8`; see `SIMPLIFY_SUMMARY_20260613.md`): `COMMENT` was never `#define`d anywhere (not in any source file, not in the makefile), so `#ifdef COMMENT … #endif` was C-Kermit's idiom for "comment out" a region that itself contains `/* */` comments or `#ifdef`s (which cannot nest as C comments). All 731 such guards (704 `#ifdef COMMENT` + 27 `#ifndef COMMENT`) across 37 `.c`/`.h`/`.w` files were unreachable dead code — old/superseded implementations parked for reference, disabled debug/experimental code (e.g. the `EVENMAX` UCS-2 detector in `ckuusx.c`), and ineffective alternate code paths. ~6,300 lines removed; the build is provably unchanged (binary identical on Linux/macOS/BSD since the blocks never compiled). **Note:** unrelated surviving uses of the word "COMMENT" were correctly left alone — the user-facing Kermit `COMMENT` script command (`ckuus2.c`/`ckuusr.c`), SSH key-comment fields, the directive escaped *inside* `/* */` in `ckuusx.c`, and prose mentions. `ckcpro.w` was edited directly (it is the wart grammar source); `ckcpro.c` is regenerated from it.
- **`ckuver.h` herald detection flattened** (4 commits `54bbdfe`→`0e8be87`, merged via `848693b`; see `SIMPLIFY_SUMMARY_20260613.md` §3): obsolete herald strings were trimmed, then the structural commit `0e8be87` rewrote the `#ifdef BSD44` `HERALD` selector from a deeply nested `#ifndef HERALD … #ifdef MACOSX/#else #ifdef __OpenBSD__/…/#else " 4.4BSD"` if-else-if ladder into a **flat sequence of independent `#ifndef HERALD / #ifdef <PLATFORM> / #define HERALD / #endif / #endif` guard blocks**. This is a behavior-preserving cosmetic refactor: priority is now encoded by physical order + the per-block `#ifndef HERALD` guard (the load-bearing element) instead of `#else`-nesting depth, but first-match-wins order is identical — **macOS > OpenBSD > NetBSD > FreeBSD > " 4.4BSD"**, with Linux's `POSIX`/`__linux__` → `" Linux"` path and the `" Unknown Platform"` catch-all unchanged. **Motivation is tooling, not behavior:** the nested ladder made `clang-format` unusable (see the file's own comment and the untracked `clang-format-config`); the flat form is `clang-format`-friendly. The rewritten block is **never compiled on Linux** (`BSD44` undefined), so `make linux` is byte-identical; verify equivalence on a macOS/BSD build (`gcc -E -DBSD44 [-DMACOSX|-D__OpenBSD__|…] ckuver.h`), not just `make linux`.
- **All `_PROTOTYP()` macros expanded and the macro removed** (2026-06-15, see `SIMPLIFY_20260615.md`): `_PROTOTYP(func, parms)` was the pre-ANSI prototype-forwarding alias, defined in `ckcdeb.h` as the one-liner `func parms`. All **1,175** invocations across `*.c`, `*.h`, and `ckcpro.w` were mechanically expanded to plain ANSI prototypes (`_PROTOTYP(int ttclos, (int));` → `int ttclos(int);`), and the now-unused `#define _PROTOTYP` plus its comment and the dead empty `#ifdef __STDC__` were deleted from `ckcdeb.h`. Expansion used a comment/string-aware, paren-balanced parser (splits on the first depth-1 comma; preserves trailing text such as the `= NULL` initializers on `_PROTOTYP(CHAR(*sxo), (CHAR)) = NULL;` function-pointer **variable** declarations; skips the `#define`, prose, and commented-out occurrences). Headers were `clang-format`-ed afterward (they were already clang-clean, so only the expanded lines moved); **`*.c` and `ckcpro.w` were left mechanical** (the `.c` files have never been clang-formatted — formatting them would add large unrelated churn). `ckcpro.c` was regenerated via `make wart && make ckcpro.c`. Because the expansion is exactly the preprocessor's own substitution (whitespace is insignificant; `arg2` always starts with `(`, so no token pasting), the build is **byte-identical** on every `#ifdef` path — verified by a `SOURCE_DATE_EPOCH`-fixed `make linux` producing a `cmp`-identical `wermit`. Only `VOID`/`CKVOID`/`CONST` remain as portability aliases now.

**IMPORTANT — do not remove `SVR4`, `SVR3`, or `ATTSV`.** They look like obsolete System V flags but are still defined and load-bearing on macOS and BSD via the `MACOSX10` → `MACOSX` → `BSD44` → `SVR4` → `SVR3`/`ATTSV` implication chain in `ckcdeb.h`, where they gate live serial/file/network code. They are undefined only on the `linux` build. Verify any future "always undefined" assumption against the macOS/BSD builds (`gcc -E -DMACOSX10 …` / `-DBSD44`), not just `make linux`.

Linux/macOS/Unix platform code is otherwise unaffected by all of the above.

## Build

There is no autoconf/configure; the hand-maintained `makefile` (~1200 lines, down from ~7300 after the simplifications above) contains per-platform targets (now only Linux/macOS/modern-BSD) that invoke `make` recursively, passing options through `KFLAGS`/`LIBS` environment variables. Do not use `make -e`.

The standard build on this machine (Linux) is:

```sh
make linux
```

Other useful invocations:

```sh
make clean          # removes objects, the wart binary, and generated ckcpro.c
make check          # reports SUCCESS if an executable wermit exists (rm wermit before building so this is meaningful)
```

The resulting binary is always named `wermit`, regardless of target. There is no test suite; verification is building and running the binary. `ckubuildlog` is a Kermit script used to produce build-log table entries after a successful build, not part of the build itself.

A build leaves `wermit`, `wart`, and the generated `ckcpro.c` in the tree; all three are gitignored (along with `*.o`) — do not commit them. `make clean` removes `wart` and `ckcpro.c` but not `wermit`. (`resume-claude.sh` in the tree is a local Claude Code session helper, also not part of the build.)

Most platform targets (including `linux`) build via the `xermit` link target, which uses the select()-based CONNECT module `ckucns.c`. The older `wermit` link target uses the fork()-based `ckucon.c` instead.

### Protocol changes: ckcpro.w and wart

`ckcpro.c` (the protocol state machine) is **generated**, not hand-edited. It is produced from `ckcpro.w` (a lex-like grammar) by the `wart` preprocessor (built from `ckwart.c`). This conversion is NOT run automatically by the platform targets — after editing `ckcpro.w` you must run, before the normal build:

```sh
make wart
make ckcpro.c
```

## Code conventions

The code is intentionally written in a conservative, highly portable C style (the project predates and deliberately avoids autoconf and modern toolchain assumptions). It now assumes an ANSI/ISO C compiler — `CK_ANSIC` is treated as always defined and the K&R fallback paths have been removed (see above) — but otherwise keeps the original portable idiom. Match the existing style: feature selection via `#ifdef` blocks keyed off platform/feature macros, `/* */` comments, conservative C constructs. `ckcdeb.h` is the master header included everywhere (platform definitions, debug macros); `ckcker.h` holds Kermit protocol symbols.

## File naming scheme

The first letters of each filename encode its layer — this is the primary structure of the codebase:

- `ckc*` — system-independent "common" modules: `ckcmai.c` (main program, version/copyright), `ckcfns.c`/`ckcfn2.c`/`ckcfn3.c` (protocol support functions), `ckcnet.c` (network), `ckctel.c` (Telnet), `ckcftp.c` (FTP client), `ckclib.c` (library routines), `ckcuni.c` (Unicode/character sets), `ckcpro.w` (protocol grammar, see above)
- `cku*` — Unix-specific: `ckutio.c` (serial/tty I/O), `ckufio.c` (file I/O), `ckucns.c`/`ckucon.c` (CONNECT modules), `ckucmd.c` (command parser engine), `ckuusr.c` + `ckuus2.c`–`ckuus7.c` (the interactive user interface / command tables), `ckuusx.c`/`ckuusy.c` (UI support, command-line args), `ckudia.c` (modem dialing), `ckuscr.c` (script command), `ckupty.c` (pty)
- `ckwart.c` — the wart preprocessor itself
