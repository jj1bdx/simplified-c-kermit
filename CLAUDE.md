# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

C-Kermit 10.0 Beta.12 test repository (serial/network communication software, file transfer via the Kermit protocol). Original author: Frank da Cruz, kermitproject.org. Changelog: https://www.kermitproject.org/ckdaily.html#changelog

Windows, OS/2, and VMS support was removed from this tree on 2026-06-12 (see `SIMPLIFY_20260612.md`): all `ckv*` files were deleted and the corresponding `#ifdef` blocks stripped with unifdef. That state is the initial git commit (`5a8fc5a`). Android support was removed the same day in commit `ad4139f` (see `SIMPLIFY_20260612_2.md`). SSL/TLS and Kerberos support (and with them the whole Telnet authentication/encryption stack: KRB4, SRP, DES/CAST) was removed on 2026-06-13 (see `SIMPLIFY_20260613.md`): the `ck_*` security files plus `ckuath.*`/`ckuat2.h` were deleted and all `+ssl`/`+krb*`/`+srp` makefile targets removed. Linux/macOS/Unix platform code is unaffected.

## Build

There is no autoconf/configure; the hand-maintained `makefile` (~7300 lines) contains per-platform targets that invoke `make` recursively, passing options through `KFLAGS`/`LIBS` environment variables. Do not use `make -e`.

The standard build on this machine is in `makecmd.sh`:

```sh
make macos
```

Other useful invocations:

```sh
make clean          # removes objects, the wart binary, and generated ckcpro.c
make check          # reports SUCCESS if an executable wermit exists (rm wermit before building so this is meaningful)
```

The resulting binary is always named `wermit`, regardless of target. There is no test suite; verification is building and running the binary. `ckubuildlog` is a Kermit script used to produce build-log table entries after a successful build, not part of the build itself.

A build leaves `wermit`, `wart`, and the generated `ckcpro.c` in the tree; all three are gitignored (along with `*.o`) — do not commit them. `make clean` removes `wart` and `ckcpro.c` but not `wermit`. (`resume-claude.sh` in the tree is a local Claude Code session helper, also not part of the build.)

Most platform targets (including `macos`) build via the `xermit` link target, which uses the select()-based CONNECT module `ckucns.c`. The older `wermit` link target uses the fork()-based `ckucon.c` instead.

### Protocol changes: ckcpro.w and wart

`ckcpro.c` (the protocol state machine) is **generated**, not hand-edited. It is produced from `ckcpro.w` (a lex-like grammar) by the `wart` preprocessor (built from `ckwart.c`). This conversion is NOT run automatically by the platform targets — after editing `ckcpro.w` you must run, before the normal build:

```sh
make wart
make ckcpro.c
```

## Code conventions

The code is intentionally written in maximally portable, pre-ANSI-friendly C (the project predates and deliberately avoids autoconf and modern toolchain assumptions; it still targets very old Unixes, VMS, etc.). Match the existing style: feature selection via `#ifdef` blocks keyed off platform/feature macros, `/* */` comments, conservative C constructs. `ckcdeb.h` is the master header included everywhere (platform definitions, debug macros); `ckcker.h` holds Kermit protocol symbols.

## File naming scheme

The first letters of each filename encode its layer — this is the primary structure of the codebase:

- `ckc*` — system-independent "common" modules: `ckcmai.c` (main program, version/copyright), `ckcfns.c`/`ckcfn2.c`/`ckcfn3.c` (protocol support functions), `ckcnet.c` (network), `ckctel.c` (Telnet), `ckcftp.c` (FTP client), `ckclib.c` (library routines), `ckcuni.c` (Unicode/character sets), `ckcpro.w` (protocol grammar, see above)
- `cku*` — Unix-specific: `ckutio.c` (serial/tty I/O), `ckufio.c` (file I/O), `ckucns.c`/`ckucon.c` (CONNECT modules), `ckucmd.c` (command parser engine), `ckuusr.c` + `ckuus2.c`–`ckuus7.c` (the interactive user interface / command tables), `ckuusx.c`/`ckuusy.c` (UI support, command-line args), `ckudia.c` (modem dialing), `ckuscr.c` (script command), `ckupty.c` (pty)
- `ckwart.c` — the wart preprocessor itself
