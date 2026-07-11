# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

C-Kermit 10.0 test repository (serial/network communication software, file transfer via the Kermit protocol), versioned `10.0.416-jj1bdx-simplified Dev.1` (based on upstream Beta.12). Original author: Frank da Cruz, kermitproject.org. Changelog: https://www.kermitproject.org/ckdaily.html#changelog

This tree has been heavily **simplified** (2026-06-12 → 2026-07-11) on the premise that it is Linux/macOS/Unix-only: Windows/OS2/VMS, Android, SSL/TLS+Kerberos, K&R C, obsolete platform macros, dead makefile targets, and `#ifdef COMMENT` blocks were removed, and the pre-ANSI macro layer (`_PROTOTYP`, `VOID`/`CKVOID`/`CONST`, `SIGTYP`/`SIGRETURN`) was expanded away. Later waves removed the built-in FTP, RLOGIN, and XMODEM/YMODEM/ZMODEM clients, converted all comments to `//` style, and lifted the old #ifdef-only preprocessor restriction (`#if`/`#elif` are now allowed). `doc/BASE_20260616.md`, `doc/BASE_20260709.md`, and `doc/BASE_20260711.md` are the consolidated overviews; per-change reports live in the dated `*_2026*.md` files under `doc/`. Linux/macOS/Unix platform code is otherwise unaffected.

**IMPORTANT — do not remove `SVR4`, `SVR3`, or `ATTSV`.** They look like obsolete System V flags but are still defined and load-bearing on macOS and BSD via the `MACOSX10` → `MACOSX` → `BSD44` → `SVR4` → `SVR3`/`ATTSV` implication chain in `ckcdeb.h`, where they gate live serial/file/network code. They are undefined only on the `linux` build. Verify any future "always undefined" assumption against the macOS/BSD builds (`gcc -E -DMACOSX10 …` / `-DBSD44`), not just `make linux`.

## Project skills

Detailed guidance lives in `.claude/skills/` — consult the matching skill before working in these areas:

- **build-and-verify** — building (`make linux` → `wermit`), `ckcpro.w`/wart regeneration, and the verification methodology (byte-identical rebuilds, per-object proofs, macOS/BSD path checks, runtime smoke tests).
- **format-and-editor-tooling** — clang-format rules (`InsertBraces`, `SortIncludes`, the pinned `ckcdeb.h` islands, format-stability check) and clangd/clang-tidy caveats.
- **signal-handling** — the `ck_signal()` sigaction wrapper, `ck_sig_t` typedef, what stays plain `signal()`, and the runtime signal test battery.
- **simplification-history** — the full changelog with caveats; check it before removing anything as "obsolete".

## Build

No autoconf/configure; the hand-maintained `makefile` (~1200 lines) has per-platform targets (Linux/macOS/modern-BSD only) that invoke `make` recursively via `KFLAGS`/`LIBS`. Do not use `make -e`. The standard build on this machine is:

```sh
make linux
```

The binary is always named `wermit`. There is no test suite; verification is building and running the binary. `wermit`, `wart`, and the generated `ckcpro.c` are gitignored build artifacts — do not commit them.

**`ckcpro.c` is generated** from `ckcpro.w` by the `wart` preprocessor, and the platform targets do NOT regenerate it — after editing `ckcpro.w`, run `make wart && make ckcpro.c` before the normal build.

After hand-editing a `.c`/`.h` file, confirm it is format-stable with `clang-format FILE | diff FILE -` (empty = clean) before committing. The `make linux` build is warning-clean; keep it that way.

## Code conventions

The code is intentionally written in a conservative, highly portable C style (the project predates and deliberately avoids autoconf and modern toolchain assumptions). It now assumes an ANSI/ISO C compiler — `CK_ANSIC` is treated as always defined and the K&R fallback paths have been removed — but otherwise keeps the original portable idiom. Match the existing style: feature selection via `#ifdef` blocks keyed off platform/feature macros, `//` comments (the whole tree was converted from `/* */` in 2026-07), conservative C constructs. `ckcdeb.h` is the master header included everywhere (platform definitions, debug macros) and must stay the first `#include`; `ckcker.h` holds Kermit protocol symbols.

## File naming scheme

The first letters of each filename encode its layer — this is the primary structure of the codebase:

- `ckc*` — system-independent "common" modules: `ckcmai.c` (main program, version/copyright), `ckcfns.c`/`ckcfn2.c`/`ckcfn3.c` (protocol support functions), `ckcnet.c` (network), `ckctel.c` (Telnet), `ckclib.c` (library routines), `ckcuni.c` (Unicode/character sets), `ckcpro.w` (protocol grammar, see above)
- `cku*` — Unix-specific: `ckutio.c` (serial/tty I/O), `ckufio.c` (file I/O), `ckucns.c`/`ckucon.c` (CONNECT modules), `ckucmd.c` (command parser engine), `ckuusr.c` + `ckuus2.c`–`ckuus7.c` (the interactive user interface / command tables), `ckuusx.c`/`ckuusy.c` (UI support, command-line args), `ckudia.c` (modem dialing), `ckuscr.c` (script command), `ckupty.c` (pty)
- `ckwart.c` — the wart preprocessor itself
