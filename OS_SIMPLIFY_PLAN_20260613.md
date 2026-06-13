# Plan: Remove dead OS-selector macros (keep Linux, macOS 10+, live BSD)

## Context

C-Kermit's source still carries preprocessor branches for ~50 dead operating
systems (classic Mac OS, Solaris, AIX, HP-UX, IRIX, SCO/Xenix, NeXT, QNX, MINIX,
Amiga, Plan 9, old BSD tiers, etc.). After the earlier Windows/OS2/VMS, Android,
and SSL/Kerberos removals, these OS branches are unreachable from any buildable
target on this machine. This change strips them so the tree carries only the
code paths that actually build today: **Linux**, **macOS 10+**, and the **live
BSD targets** (FreeBSD/NetBSD/OpenBSD), which are explicitly NOT in the removal
list and stay intact.

This is the same mechanical, low-risk procedure used for the SSL/Kerberos
removal (`SIMPLIFY_20260613.md`): whole-tree `unifdef -m -U<symbol>...`. A
c-expert analysis confirmed the tree uses **only** `#ifdef`/`#ifndef`/`#else`
(zero `#if`/`#elif`/`defined()` expressions), so unifdef block-deletion is
provably correct — there is no compound-expression "-D trap" and no kept
umbrella macro loses its derivation for Linux/macOS/BSD.

## Prerequisites

1. `sudo apt install unifdef` (candidate 2.12-1; not currently installed).
2. Make a backup before any edit: `tar czf /tmp/cpp-kermit-backup-os-20260613.tar.gz -C /home/kenji/src cpp-kermit` (nothing is committed to git).
3. Copy this plan into the repo as `OS_SIMPLIFY_PLAN_20260613.md` (user-requested deliverable).

## What to remove — the `-U` symbol set

Feed all of these to `unifdef -U` (de-duplicated; families expanded; derived-dead
umbrellas `ANYSCO`/`BSDI`/`BEOSORBEBOX`/`SVR3JC` included because their only
sources are removed flags):

```
MAC COHERENT
MINIX MINIX2 MINIX3 MINIX315 MINIX340
QNX CK_QNX16 CK_QNX32
Plan9 OSK GEMDOS AMIGA STRATUS aegis apollo
BELLV10 PROVX1 EXCELAN
BEBOX BEOS BEOSORBEBOX
pdp11 ultrix sun mips MIPS
SOLARIS SOLARIS24 SOLARIS25 SOLARIS26 SOLARIS7 SOLARIS8 SOLARIS9 SOLARIS10 SOLARIS11
SUNOS4 SUNOS41
HPUX HPUX9 HPUX10 HPUX9PLUS HPUX1010 HPUX1020 HPUX1030 HPUX1100
AIXRS AIX41 AIX42 AIX43 AIX44 AIX45 AIX50 AIX51 AIX52 AIX53 AIX370 AIXPS2 AIXESA RTAIX
OSF OSF10 OSF20 OSF30 OSF32 OSF40 OSF50
IRIX IRIX40 IRIX51 IRIX52 IRIX60 IRIX62 IRIX64 IRIX65
XENIX SCO_OSR504 SCO_OSR505 CK_SCO32V4 ANYSCO SVR3JC CK_SCOV5 UNIXWARE
PTX I386IX OXOS DGUX DGUX430 DGUX540 datageneral
NEXT NEXT33
SV68R3V6 SV88R32 ATT6300 ATT7300 AUX ISIII UTSV
__bsdi__ BSDI BSD43 BSD41 BSD29
```

## What to KEEP (must NOT be in the `-U` list)

Live umbrellas/capabilities and live BSD — these still derive correctly for
Linux/macOS/BSD after removal (verified against `ckcdeb.h`):

- Family umbrellas: `UNIX`, `POSIX`, `ATTSV`, `ANYBSD`, `BSD4`, `BSD44`, `SVR3`, `SVR4`, `V7`
- Rollups: `SVORPOSIX`, `SVR4ORPOSIX`, `BSD44ORPOSIX`, `VMSORUNIX`
- Capabilities: `SELECT`, `BSDSELECT`, `RDCHK`, `MYREAD`, `FIONREAD`, `CK_POSIX_SIG`, `SIG_V`, `SIG_I`, `DIRENT`, `NOFILEH`, `CK_NEWTERM`, `STERMIOX`, `TERMIOX`, `NOSETBUF`, `HDBUUCP`, `CK_UTSNAME`, `FNFLOAT`, `SYSTIMEH`, `SYSTIMEBH`, `POSIX_CRTSCTS`
- Linux: `LINUX`, `__linux__`, `LINUXFSSTND`
- macOS: `MACOSX`, `MACOSX10`, `MACOSX103`, `__APPLE__`
- BSD: `__FreeBSD__`, `__NetBSD__`, `__OpenBSD__`, `FREEBSD4`, `FREEBSD8`, `FREEBSD9`, `OPENBSD`

Note (harmless side effect, no action): `BSD4`, `ANYBSD`, `MYREAD`, and the
SOLARIS-derived `CK_NEWTERM`/`STERMIOX`/`HDBUUCP` derivation sites become
unreachable, but no live target relies on those derivations (macOS/BSD select via
`BSD44`, Linux via `POSIX` and explicit makefile `-D`). The symbols stay in the
source. We do NOT expand the list to adjacent dead flags (`vax`, `alpha`,
`A986`, `SUN4S5`, `sun386`, `TOWER1`, `FT21`, …) — out of scope and harmless.

## Execution steps

1. **Backup + tooling**: steps in Prerequisites above.
2. **unifdef sweep** over every `*.c` and `*.h` (excluding generated `ckcpro.c`)
   plus `ckcpro.w`, in place (`unifdef -m`), passing the full `-U` set. Check
   each file's exit status: unifdef returns 0 (unchanged) or 1 (changed) on
   success, **2 = parse error** (must investigate). Script it per-file so a 2 is
   caught.
3. **Post-pass residual scan**: grep the tree for any surviving `#ifdef`/`#ifndef`
   of a removed symbol (e.g. a symbol that was only `#define`d inside a now-deleted
   block); confirm zero remain.
4. **Regenerate the protocol module** from the cleaned `ckcpro.w`:
   `make clean && make wart && make ckcpro.c` (CLAUDE.md procedure — ckcpro.c is
   generated, never hand-edited; it is gitignored).
5. **Build + verify** (this is a Linux box): `rm -f wermit && make linux`
   then `make check` → expect SUCCESS. (Verified live flags: `-DLINUX -DPOSIX
   -DCK_POSIX_SIG -DCK_NEWTERM -DLINUXFSSTND -DFNFLOAT …`, per `build_linux.log`.)
6. **Smoke test**: `./wermit -C "show version, exit"` and `./wermit -C "show
   features, exit"` — confirm banner + clean run, no missing-platform breakage.

## Files affected (blast radius)

Heaviest: `ckcdeb.h` (derivation hub) and `ckutio.c` (tty driver). Substantial
edits also in `ckufio.c`, `ckuus5.c` (SHOW FEATURES OS/CPU dump),
`ckuusx.c`, `ckcnet.c`/`ckcnet.h`, `ckuver.h`, `ckucmd.c`, `ckcmai.c`,
`ckupty.c`, and ~25 more files. No file becomes empty — every file retains its
live Linux/macOS/BSD code. `ckcpro.w` loses 3 dead-flag guards (`pdp11`,
`datageneral`, `COHERENT`) and is regenerated into `ckcpro.c`.

## Out of scope

- **makefile**: not touched. The user scoped this to `.c/.h/.w` source. No live
  target (`linux*`, `macos`, `freebsd`, `netbsd`, `openbsd`) passes any removed
  `-D` flag, so the live builds compile from the code changes alone. The obsolete
  Solaris/AIX/HP-UX/SCO/… targets that pass removed flags don't build today
  anyway; pruning them is a separate optional follow-up.
- Adjacent dead OS macros not in the user's list (see Note above).

## Deliverables

- Stripped `.c`/`.h`/`.w` source + regenerated `ckcpro.c`, building cleanly via
  `make linux`.
- `OS_SIMPLIFY_PLAN_20260613.md` — this plan, committed to the repo.
- `OS_SIMPLIFY_20260613.md` — final report (mirroring `SIMPLIFY_20260613.md`'s
  format: metrics table, method, exact `-U` list, per-file reductions, build
  verification output, notes/follow-ups).

## Verification summary

| Check | Expected |
|---|---|
| `unifdef` exit status per file | 0 or 1 (never 2) |
| residual grep for removed `#ifdef`s | none |
| `make wart && make ckcpro.c` | regenerates cleanly |
| `make linux` | exit 0, no warnings about removed code |
| `make check` | SUCCESS |
| `./wermit -C "show version, exit"` | prints banner, exits 0 |
