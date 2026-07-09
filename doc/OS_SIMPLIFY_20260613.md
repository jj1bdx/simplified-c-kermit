# Codebase Simplification Report — Removal of Dead OS-Selector Macros

**Date:** 2026-06-13
**Tree:** C-Kermit 10.0 Beta.12 (`/home/kenji/src/cpp-kermit`, Ubuntu/Linux)
**Scope:** Remove the preprocessor branches for all operating systems other than
**Linux**, **macOS (10+)**, and the **live BSD targets** (FreeBSD/NetBSD/OpenBSD).
CPU-architecture identifier macros were **kept** (see [CPU macros](#cpu-macros-kept)).
This continues the earlier Windows/OS2/VMS, Android, and SSL/Kerberos removals.

Planning document: `OS_SIMPLIFY_PLAN_20260613.md`. Pre-removal flag catalog:
`OS_FLAGS_20260613.md`.

## Summary

| Metric | Before | After | Delta |
|---|---|---|---|
| Source lines (`*.c *.h *.w`, excl. generated `ckcpro.c`) | 240,218 | 228,207 | **−12,011 (−5.0%)** |
| Source files edited in place | — | 41 | via `unifdef -m` |
| Whole `git diff` | | | **12,074 deletions, 63 insertions** |
| OS-selector macros removed (`-U`) | | | **206 distinct symbols** |
| `ckcpro.w` (protocol grammar) | 3,703 | 3,685 | −18 (regenerated `ckcpro.c`) |

Build verification on **Linux**: **success** — `make linux` exits 0 with **zero
errors and zero warnings**, `make check` reports **SUCCESS**. The banner reads
`C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)`; `wermit` is
2,864,024 bytes. A full backup of the original tree was made before any change:
`/tmp/cpp-kermit-backup-os-20260613.tar.gz`.

## Method

Mechanically identical to the SSL/Kerberos removal (`SIMPLIFY_20260613.md`):
whole-tree `unifdef -m -U<symbol>...` in place.

1. **Analysis** by a `c-expert` agent over `OS_FLAGS_20260613.md` and `ckcdeb.h`
   established the removal set, the derivation chains, and the risk list. The key
   finding: the tree uses **only** `#ifdef`/`#ifndef`/`#else` — **zero**
   `#if`/`#elif`/`defined()` expressions anywhere. So unifdef block-deletion (and
   `#else`-promotion) is provably correct; there is **no compound-expression
   "-D trap"** (unlike the Windows/VMS removal) and no kept umbrella macro can lose
   its derivation through partial evaluation.
2. **Four unifdef passes** (`unifdef -m`, per-file exit-status checked; 0/1 = OK,
   2 = parse error — **none occurred**):
   - Pass 1 (105 symbols): the originally-requested list, families expanded.
   - Pass 2 (71 symbols): variant macros of the same families discovered by a
     broad scan (e.g. `QNX16`/`QNX6`, plain `AIX`/`_AIX`, `SOLARIS23`, `OSF1`/
     `__osf__`, the full `SCO_*`/`HPUX*`/`IRIX*`/`DGUX543*` sets, `NeXT`), plus
     approved unnamed-dead OSes (`TOWER1`, `FT18`/`FT21`, `A986`, `MOTSV88R4`,
     `BSD42`/`BSD2_10`).
   - Pass 3 (30 symbols): HP-UX/IRIX/Sequent hardware stragglers (`__hp9000s*`,
     `hp9000s500`, `sgi`, `sequent`) and other dead foreign OSes
     (`CRAY`/`CONVEX`/`ENCORE`/`PYRAMID`/`UTEK`/`sony_news`/`u370`/`u3b*`/`is68k`/
     `DELL_SVR4`/`NCRMPRAS`/`M_SYSV`/`sysV`/`sysvimp`/`SYSV_TERMIO`).
   - Pass 4 (1 symbol): `_M_UNIX` (SCO 3.2v4 marker).
3. **Residual scan**: confirmed no surviving `#ifdef`/`#ifndef` of a removed
   symbol (one false positive: `HPUX10` inside a `/* */` comment in `ckuus3.c`,
   correctly left untouched).
4. **Regeneration of `ckcpro.c`** from the cleaned `ckcpro.w`
   (`make clean && make wart && make ckcpro.c`), then rebuild and smoke test.

## What was removed (206 symbols, by family)

- **Classic Mac OS:** `MAC`
- **Solaris/SunOS:** `SOLARIS`, `SOLARIS23/24/25/26/7/8/9/10/11`, `SUNOS`, `SUNOS4`,
  `SUNOS41`, `SUN4S5`, `SUNOS4S5`, `SUNS4S5`, `SUNX25`
- **HP-UX:** `HPUX`, `HPUX5/5WINTCP/6/7/8/9/9PLUS/10/100/1000/1010/1020/1030/1100`,
  `HPUX10_TRUSTED`, `HPUXJOBCTL`, `HPUXPRE65`, `__hpux`, `hpux`,
  `__hp9000s200/300/400/500/700/800`, `hp9000s500`
- **AIX:** `AIXRS`, `AIX41–53`, `AIX370`, `AIXPS2`, `AIXESA`, `RTAIX`, `PS2AIX10`,
  `_AIX`, `_AIXFS`
- **OSF/1 / Tru64:** `OSF`, `OSF1/10/13/2/20/30/32/40/50`, `OSFPC`,
  `__OSF`/`__OSF1`/`__OSF1__`/`__OSF__`/`__osf__`, `CK_OSF_BSD`
- **IRIX / SGI:** `IRIX`, `IRIX40/51/52/53/60/62/63/64/65`, `sgi`
- **SCO / Xenix / UnixWare:** `XENIX`, `M_XENIX`, `SCO_XENIX`, `SCO_OSR504/505/507`,
  `SCO234`, `SCO32`, `SCO3R2`, `SCO_32V4`, `CK_SCO32V4`, `CK_SCOV5`, `ANYSCO`,
  `SVR3JC`, `CK_SCOUNIX`, `__SCO__`, `_SCO_DS`, `UNIXWARE`, `UNIXWARE7`,
  `UNIXWAREPOSIX`, `OLD_UNIXWARE`, `M_SYSV`, `M_UNIX`, `_M_UNIX`, `sysV`, `sysvimp`,
  `SYSV_TERMIO`
- **Data General:** `DGUX`, `DGUX430/540/543/54410/54411`, `datageneral`
- **NeXT:** `NEXT`, `NEXT33`, `NeXT`
- **Sequent / Interactive / misc SysV:** `PTX`, `I386IX`, `OXOS`, `sequent`,
  `u3b`, `u3b2`, `u3b5`, `u370`, `UTSV`, `ISIII`, `AUX`
- **Motorola SysV:** `SV68R3V6`, `SV88R32`, `SV68`, `MOTSV88R4`, `ATT6300`, `ATT7300`
- **DEC Ultrix:** `ultrix`, `ULTRIX3`
- **Old BSD tiers / BSDI:** `__bsdi__`, `BSDI`, `BSDI2`, `bsdi`, `BSD43`, `BSD42`,
  `BSD42HACK`, `BSD41`, `BSD29`, `BSD2_10`
- **MINIX / Coherent / QNX:** `MINIX`, `MINIX2/3/315/340`, `COHERENT`, `QNX`,
  `QNX16`, `QNX6`, `CK_QNX16`, `CK_QNX32`
- **Non-Unix & niche:** `Plan9`, `OSK`, `OSKXXC`, `GEMDOS`, `AMIGA`, `STRATUS`,
  `STRATUSX25`, `aegis`, `apollo`, `APOLLOSR10`, `BELLV10`, `PROVX1`, `EXCELAN`,
  `BEBOX`, `BEBOX_DR7`, `BEOS`, `BEOSORBEBOX`, `pdp11`
- **Other dead vendors/hardware:** `CRAY`, `_CRAY`, `_CRAYCOM`, `CONVEX9`,
  `CONVEX10`, `ENCORE`, `PYRAMID`, `UTEK`, `sony_news`, `is68k`, `DELL_SVR4`,
  `NCRMPRAS`, `TOWER1`, `FT18`, `FT21`, `A986`
- **CPU predefines requested for removal:** `sun`, `mips`, `MIPS`

(Derived-dead umbrellas `ANYSCO`, `BSDI`, `BEOSORBEBOX`, `SVR3JC` were removed
because their only definition sources were removed flags.)

## What was kept

### Live umbrellas / capabilities (verified to still derive for Linux/macOS/BSD)
`UNIX`, `POSIX`, `ATTSV`, `ANYBSD`, `BSD4`, `BSD44`, `SVR3`, `SVR4`, `V7`,
`SVORPOSIX`, `SVR4ORPOSIX`, `BSD44ORPOSIX`, `VMSORUNIX`, `SELECT`, `DIRENT`,
`NOFILEH`, `NOSETBUF`, `CK_POSIX_SIG`, `CK_NEWTERM`, `STERMIOX`, `HDBUUCP`,
`SIG_V`/`SIG_I`, `SYSTIMEH`/`SYSTIMEBH`, etc. macOS still resolves
`MACOSX10 ⇒ MACOSX ⇒ BSD44 ⇒ SVR4 ⇒ ATTSV+SVR3`; Linux via `POSIX`. No live
target sets `BSD4`/`ANYBSD`/`MYREAD`, so those (kept) symbols are simply
unreachable now — unchanged from prior behavior.

### Live platform selectors
`LINUX`/`__linux__`/`LINUXFSSTND`, `MACOSX`/`MACOSX10`/`MACOSX103`/`__APPLE__`,
`__FreeBSD__`/`__NetBSD__`/`__OpenBSD__`/`FREEBSD2/3/4/41–49/8/9`/`OPENBSD`.

### CPU macros (kept by design) <a name="cpu-macros-kept"></a>
CPU/architecture identifiers are not operating systems and were kept — notably
`__alpha__` guards **live "Linux on DEC Alpha"** code (`ckutio.c`):
`alpha`, `__alpha`, `__alpha__`, `ALPHA`, `__ALPHA`, `__ALPHA__`, `vax`, `VAX`,
`sun3`, `sun4`, `sun386`, `_SUN`, `__SUN`, and the Intel `M_I86/I286/I386/I486/
I586/I686` (+`_M_I*`) `SHOW FEATURES` identifiers.

### Feature flags (not OS selectors)
`CLSONDISC`, `NOSKIPMATCH`, `NOSYSCONF`, and all non-OS `#ifdef`s were untouched.

## Largest per-file reductions (lines)

| File | Before | After | Removed |
|---|---|---|---|
| ckutio.c | 15,978 | 12,447 | 3,531 |
| ckcdeb.h | 6,039 | 4,168 | 1,871 |
| ckcnet.c | 10,941 | 9,938 | 1,003 |
| ckuver.h | 1,266 | 518 | 748 |
| ckufio.c | 8,963 | 8,346 | 617 |
| ckuus5.c | 11,345 | 10,751 | 594 |
| ckuusx.c | 7,847 | 7,452 | 395 |
| ckcnet.h | 1,107 | 758 | 349 |
| ckustr.c | 409 | 144 | 265 |
| ckuus4.c | 14,494 | 14,236 | 258 |
| ckucmd.c | 8,230 | 7,975 | 255 |
| ckucns.c | 2,688 | 2,473 | 215 |
| ckucon.c | 2,680 | 2,494 | 186 |
| ckcmai.c | 3,215 | 3,037 | 178 |
| ckupty.c | 2,053 | 1,879 | 174 |

(21 more files had smaller reductions. `ckcpro.c` was regenerated from the cleaned
`ckcpro.w`, 3,902 → 3,884.) No file became empty — every file retains its live
Linux/macOS/BSD code.

## Makefile

**Not touched** (the task was scoped to `.c/.h/.w` source). No live target
(`linux*`, `macos`, `freebsd`, `netbsd`, `openbsd`) passes any removed `-D` flag —
verified against `build_linux.log` (`-DLINUX -DPOSIX -DCK_POSIX_SIG -DCK_NEWTERM
-DLINUXFSSTND -DFNFLOAT …`) — so the live builds compile from the code changes
alone. The obsolete Solaris/AIX/HP-UX/SCO/… makefile targets that still pass
removed flags do not build today anyway; pruning them is a separate follow-up.

## Verification

```sh
make clean
make wart && make ckcpro.c     # regenerate protocol module from cleaned ckcpro.w
rm -f wermit
make linux                     # exit status 0
make check                     # SUCCESS
```

| Check | Result |
|---|---|
| `make linux` exit status | 0 |
| `make check` | SUCCESS |
| Compiler errors | 0 |
| Compiler warnings | 0 |
| `wermit` size | 2,864,024 bytes |

Smoke test:

```
$ ./wermit -C "show version, exit"
C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)
```

## Notes / follow-ups

- `ckutio.c`/`ckufio.c` retain a now-inert `#define BSD42` inside their
  `#ifdef BSD4` blocks (`BSD4` is kept but never set on a live target, and `BSD42`
  is no longer tested anywhere). Harmless dead code; removable with `BSD4`/`ANYBSD`
  in a future "dead-umbrella" pass.
- A handful of dead-OS names survive in **comments** and **user-visible help/status
  strings** (e.g. the `HPUX10` mention in a `ckuus3.c` comment, OS labels in
  `SHOW FEATURES`); these are inert text, not code paths.
- CPU identifier macros (`alpha`/`vax`/`sun*`/`M_I*`) were deliberately kept; a
  future cleanup could drop the cosmetic ones, but `__alpha__`/`__alpha` must stay
  (Linux-on-Alpha).
- The original tree (pre-removal) is preserved at
  `/tmp/cpp-kermit-backup-os-20260613.tar.gz`; nothing was committed to git.
```
