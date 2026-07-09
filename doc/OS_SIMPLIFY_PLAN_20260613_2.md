# Plan — Removal of Obsolete / Not-Buildable Makefile Platform Targets

**Date:** 2026-06-13
**Tree:** C-Kermit 10.0 Beta.12 (`/home/kenji/src/simplified-c-kermit`)
**Status:** PLAN ONLY — no `makefile` edits or commits are made by this document. Execution
is a separate follow-up pass.
**Predecessors:** `SIMPLIFY_20260613_4.md` (dead flag-defining targets),
`SIMPLIFY_20260613_5.md` (Xenix / ancient-BSD targets).

## 0. Goal

This is now a **Linux / macOS / modern-BSD only** tree. The hand-maintained `makefile`
(6,302 lines) still defines ~400 build targets for long-dead vendor Unixes, plus obsolete
in-family variants and now-orphaned generic base recipes. This plan defines exactly which
platform targets to **remove** and which to **keep**, the safe method to remove them, and
how to verify the result.

## 1. Verified dependency facts

Confirmed by reading the makefile (not assumed):

- The modern keep-set — `linux`/`linuxa`, `macos`/`macosx`, `freebsd*`, `netbsd*`,
  `openbsd`, `mirbsd`, `oldmacosx*` — all dispatch **directly to `xermit`** with inline
  `CFLAGS`. **None route through** the generic `bsd`/`bsd42`/`bsd44`/`bsd44c`/`posix`
  recipes.
- `bsd` → `bsd42` → `xermit`; `bsd44`/`bsd44c` → `xermit`. Their only historical callers
  were SGI IRIX / MIPS / DNIX, which are themselves being removed here. The generic bases
  therefore become **orphaned** and are safe to delete.
  - **This reverses the `bsd`/`bsd42` "keep" caveat in `SIMPLIFY_20260613_5.md`**, which
    held *only while* SGI/MIPS/DNIX were still present. Once those go, the caveat lapses.
- Makefile structure: top-of-file **platform menu** ≈ lines 141–684; **platform target
  definitions** ≈ lines 1180–6302.
- The C-source `SVR4`/`SVR3`/`ATTSV` guardrail (macOS/BSD `#ifdef`s in `ckcdeb.h`) is
  **unaffected** — this pass touches only build targets, never C source.

## 2. Decisions (confirmed with user)

1. **Prune old in-family variants** — keep only canonical / auto-detecting targets plus
   useful modern sub-variants; drop version-specific obsolete ones.
2. **Remove the orphaned generic bases** `bsd`, `bsd42`, `bsd44`, `bsd44c`, `posix`.
3. **Deliverable = this planning document only.** No `makefile` edits this pass.

## 3. KEEP set (explicit)

### Platform targets
- **Linux:** `linux`, `gnu-linux`, `linux-clang`, `linuxclang`, `linux-nonet`,
  `linux-notcp`, `linux-nodeprecated`, `linux-pedantic`.
  - Internal base `linuxa`: **keep iff** the closure check (§5.2) shows a kept Linux
    sub-variant still dispatches to it; otherwise remove. (`linux` itself builds `xermit`
    directly, so `linuxa` is expected to be removable.)
- **macOS:** `macos`.
- **FreeBSD:** the modern auto-detecting recipe line
  `freebsd freebsd41 freebsd72 freebsd5 freebsd6 freebsd7 freebsd8 freebsd9` (kept intact).
- **NetBSD:** `netbsd` (+ `netbsd2` on the shared recipe line), `netbsd-clang`,
  `netbsdclang`, `netbsd-pedantic`, `netbsd-nonet`, `netbsd-notcp`, `netbsd-nodeprecated`,
  `netbsdnc`, `netbsdn`.
- **OpenBSD / MirBSD:** `openbsd`, `mirbsd`.

### Infrastructure (always keep)
`wermit`, `xermit`, `mermit`, `wart`, `all`, `clean`, `check`, `install`, `uninstall`,
`show`, `count`, `list`, `makewhat`, `buildlog`, the `.c.o` / suffix rules, and all
variable definitions.

## 4. REMOVE set

Everything that is a column-0 platform target and **not** in the KEEP set, namely:

### 4.1 Foreign-OS families
| Family | Target prefixes / names |
|---|---|
| SGI IRIX | `iris`, `irix*` |
| MIPS RISC/OS | `mips`, `mipstcpc` |
| DNIX | `dnix*` |
| Apollo Aegis | `aegis`, `apollo*` |
| Encore Multimax | `encore*`, `umax*` |
| AT&T System V | `att*`, `sys3*`, `sys5*`, `s5r4m*`, `unisys5r2`, `xos23*`, `valid`, `white`, `pixel` |
| HP-UX | `hpux*` |
| IBM AIX / RS6000 | `aix*`, `rs6000*`, `rs6aix*`, `ps2aix*`, `aux*`, `aixesa`, `oldaix*` |
| SCO UNIX / OpenServer / Unixware | `sco*`, `uw*`, `unixware*` |
| Solaris / SunOS 5 | `solaris*` |
| DEC Ultrix / OSF / Tru64 | `ultrix*`, `dec-osf*`, `osf`, `osf1`, `old-dec-osf`, `du32`, `du40*`, `tru64-*` |
| Sequent | `dynix*` |
| NeXTSTEP / OPENSTEP | `next*`, `openstep42` |
| QNX | `qnx*` |
| Interactive UNIX | `is3`, `is5r3*`, `isi` |
| Motorola / DG / misc V | `sv68*`, `sv88*`, `tower*`, `dgux*`, `ft18`, `ft21`, `ftx`, `ftxtcp`, `rtu`, `rtus5*`, `utek*`, `clix*`, `ctix`, `cx_ux`, `ridge32`, `pyramid*`, `convex*`, `cray*`, `craycsos`, `bulldpx2`, `ccop1`, `mpras*`, `mpsysv`, `esixr4`, `bellv10`, `riscix*`, `s5r4mi`, `sr10-s5r3`, `t31tos40x`, `svr4amiganet` |
| Coherent | `coherent*` |
| MINIX | `minix*` |
| BeOS | `bebox*`, `beos*` |
| V7 / System III base | `v7`, `v7min`, `altos3` |

(The execution pass derives the authoritative list mechanically — §5.1 — rather than from
this prose; the table is a human-readable summary.)

### 4.2 Orphaned generic bases
`bsd`, `bsd42`, `bsd44`, `bsd44c`, `posix`.

### 4.3 Obsolete in-family variants
- macOS: `oldmacosx10`, `oldmacosx10c`, `oldmacosx10nc`, `oldmacosx10ncx`,
  `oldmacosx102nc`, `oldmacosx103`, the legacy `macosx macosx10 macosx10.3.9 …` recipe
  line, and `machten`.
- FreeBSD: `freebsd1`, `freebsd2`, `freebsd2c`, `freebsd3`, `freebsd3c`, `freebsd40`.
- NetBSD: trim the obsolete aliases `netbsd15`, `netbsd16`, `old-netbsd` from the kept
  `netbsd …` recipe header (keep `netbsd`, `netbsd2`).
- OpenBSD: `openbsdold`.
- Linux: `linuxa`*, `linuxp`, `linuxgcc`, `linuxmin`, `linux-2015`, `linux-clang-wimplicit`,
  `linux-clang-c89`, `linux-clang-gnu89`, `linux+shadow+pam`, `linuxns`, `linuxnc`,
  `linuxso`, `linuxppc`, `linuxegcs`, `linuxnotcp-lcc`, `linux10`, `linuxold`.

(*`linuxa` per the conditional in §3.)

### 4.4 Platform-specific lint / utility targets
Remove lint targets that reference only removed platforms (e.g. `lintsun`, `lints5`).
Keep any generic lint target. Resolve mechanically during execution.

## 5. Method (mirrors `SIMPLIFY_20260613_4/5`)

### 5.1 Enumerate and partition
Parse every column-0 `name:` target header (exclude `=` variable assignments and tab
recipe lines). `REMOVE = all_platform_targets − KEEP − infrastructure`.

### 5.2 Dispatch-closure + dangling-reference check
Generalize `/tmp/closure*.py`. For every KEEP target, confirm its recipe contains **no
`$(MAKE) <removed>`** invocation. Expected result: **zero** dangling references (already
verified by hand for the generic bases). If any KEEP target dispatches to a REMOVE target,
promote that base back into KEEP and flag it for review before deleting.

### 5.3 Stanza-accurate deletion
Generalize `/tmp/strip*.py`. For each removed target delete its attached **preceding
comment block + header line + tab recipe + one trailing blank line**, with exact
boundaries (do not swallow an unrelated following stanza).

### 5.4 Entry-aware menu pruning
Generalize `/tmp/prune_menu*.py` over the top-of-file platform menu (≈ lines 141–684):
- delete whole menu entries (including continuation lines) that reference only removed
  targets;
- **hand-rewrite mixed entries** that also name a kept target, keeping the valid
  alternative;
- delete entries that name only nonexistent targets.

### 5.5 Safety
Keep a pre-edit backup at `/tmp/makefile.bak3` for the duration of the execution pass.

## 6. Verification plan (execution pass)

```sh
make -n linux                 # parses, exit 0
rm -f wermit && make linux    # exit 0
make check                    # SUCCESS
./wermit -V                   # banner unchanged: C-Kermit 10.0.416 Beta.12, ... for Linux (64-bit)

# Post-edit greps (all must be empty):
grep -nE '^[A-Za-z0-9].*:' makefile     # no headers for any removed target
# no "$(MAKE) <removed>" dispatches; no "make <removed>" references in the menu
```

Spot-build a representative kept non-default target with `make -n` (e.g. `make -n macos`,
`make -n freebsd`, `make -n openbsd`, `make -n netbsd`) to confirm those recipes still
parse.

## 7. Expected impact

- Targets removed: **≈400**; kept: **≈25** platform targets + infrastructure.
- `makefile` lines: **6,302 → ≈1,500–2,000** (final count reported after execution).
- Top-of-file platform menu collapses to the Linux / macOS / FreeBSD / NetBSD / OpenBSD /
  MirBSD entries.

## 8. Out of scope / follow-ups

- **No `makefile` edits and no commits in this pass** — this document is the only artifact.
- On execution: add `SIMPLIFY_20260613_6.md` (the removal report) and a CLAUDE.md history
  bullet, and record that the `bsd`/`bsd42` keep-caveat from report 5 is now retired.
- C source is untouched; the macOS/BSD `SVR4`/`SVR3`/`ATTSV` guardrail still applies to any
  future C-source cleanup.
