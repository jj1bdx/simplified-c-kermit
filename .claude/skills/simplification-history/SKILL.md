---
name: simplification-history
description: Detailed changelog of the 2026-06 simplification campaign - platform/feature removals (Windows/OS2/VMS, Android, SSL/Kerberos, K&R C, obsolete platform macros, dead makefile targets, ifdef COMMENT blocks) and the macro-expansion modernizations, with per-change caveats and pointers to the SIMPLIFY_*/report files. Use when investigating why code, macros, files, or makefile targets are absent; before removing anything as "obsolete"; or when planning/writing a new simplification.
---

# Simplification history (2026-06-12 → 2026-06-16)

`doc/BASE_20260616.md` is the consolidated overview of the whole campaign
(`initial-without-windows-or-vms` = `5a8fc5a` → `base-20260616` = `2eab866`, 60 commits). This
skill keeps the per-change caveats that matter for future work. The premise throughout: this is
a **Linux/macOS/Unix-only** tree.

## Platform & feature removals

- **Windows/OS2/VMS** removed 2026-06-12, *before* the initial commit (`doc/SIMPLIFY_20260612.md`):
  all `ckv*` files deleted, `#ifdef` blocks stripped with unifdef.
- **Android** removed the same day, commit `ad4139f` (`doc/SIMPLIFY_20260612_2.md`).
- **SSL/TLS + Kerberos** removed 2026-06-13 (`doc/SIMPLIFY_20260613.md`): the whole Telnet
  authentication/encryption stack (KRB4, SRP, DES/CAST). The `ck_*` security files plus
  `ckuath.*`/`ckuat2.h` deleted; all `+ssl`/`+krb*`/`+srp` makefile targets removed.
- **Non-ANSI / K&R C** removed, commit `20bf398` (`doc/SIMPLIFY_20260613_2.md`): `CK_ANSIC` is
  assumed always defined; all pre-ANSI fallback branches stripped.
- **26 obsolete platform macros** removed from the C source, commit `c1c710b`
  (`doc/SIMPLIFY_20260613_3.md`, `doc/OS_FLAGS_20260613.md`): `AS400 __386BSD__ C70 CIE SINIX SNI544
  SNI543 SNI541 POWERMAX ICL_SVR4 XF68R3V6 XF88R32 IX370 PCIX sxaE50 RTU TRS16 _386BSD OU8 UW7
  NEUTRINO LYNXOS UTS24 VXVE ZILOG AMIX` treated as permanently undefined; dead branches
  deleted with `unifdef`.
- **Dead makefile targets** removed in waves (`doc/SIMPLIFY_20260613_4.md`, `_5.md`, `_6.md`,
  `doc/OS_SIMPLIFY_20260613.md`, `doc/OS_SIMPLIFY_PLAN_20260613*.md`): first the 38 targets defining
  those macros (commit `aca665a`), then 64 Xenix/ancient-BSD/SunOS-3-4 targets (commit
  `86e1cb0`), then **all** remaining foreign-Unix families and the orphaned generic bases
  `bsd`/`bsd42`/`bsd44`/`bsd44c`/`posix`. The makefile keeps **only** Linux, macOS, and
  modern-BSD (FreeBSD/NetBSD/OpenBSD/MirBSD) targets; **`linuxa` is kept** — `linux`/
  `gnu-linux` dispatch to it. `makefile`: 9,520 → ~1,200 lines. Some stale foreign-Unix
  *comment prose* remains — harmless, recipe-less text.
  **Method caveat for future stanza removal:** GNU make treats blank/comment lines *between*
  tab-prefixed recipe lines as part of one recipe; boundary detection must consume the whole
  recipe to its last tab-line or it orphans recipe fragments onto kept targets.
- **`#ifdef COMMENT` dead-code blocks** removed, commit `3201982` merged via `f5456b8`
  (`doc/SIMPLIFY_SUMMARY_20260613.md`): `COMMENT` was never `#define`d anywhere, so
  `#ifdef COMMENT … #endif` was C-Kermit's "comment out a region" idiom. All 731 guards
  (704 `#ifdef` + 27 `#ifndef`) across 37 files removed (~6,300 lines); build provably
  unchanged. **Surviving uses of the word "COMMENT" are correct and must stay**: the
  user-facing Kermit `COMMENT` script command (`ckuus2.c`/`ckuusr.c`), SSH key-comment fields,
  the directive escaped inside `/* */` in `ckuusx.c`, and prose mentions.

## Source modernization (behavior-preserving)

- **`_PROTOTYP()` expanded and removed** (`doc/SIMPLIFY_20260615.md`): all 1,175 pre-ANSI
  prototype-forwarding macro invocations expanded to plain ANSI prototypes; the `#define`
  deleted from `ckcdeb.h`. Byte-identical build.
- **`VOID`/`CKVOID`/`CONST` aliases expanded and removed** (`doc/PROTOTYP_PLAN_20260615_2.md`):
  688 exact-token substitutions to `void`/`const`; **no portability alias macros remain in the
  tree**. Sibling macros sharing substrings (`MAINISVOID`, `TPUTSISVOID`, `TPUTSARG1CONST`,
  `PAM_CONST`, …) and the Kermit `VOID` script-command help string were correctly left alone.
  The `ckuusx.c` `#undef VOID` curses guard became a no-op and was removed. Byte-identical.
- **Signal modernization** — `SIGTYP`/`SIGRETURN` expansion, the `ck_signal()` sigaction
  wrapper, and the `ck_sig_t` typedef: see the **signal-handling** skill
  (`doc/SIGTYPE_PLAN_20260615.md`, `doc/SIGACTION_PLAN_20260615.md`, `doc/SIGACTION_MACOS_20260615.md`,
  `doc/CK_SIG_T_20260616.md`, `doc/SIGNAL_TYPE_20260615.md`).
- **`ckuver.h` herald selector flattened** and **clang-format
  `SortIncludes`/`InsertBraces` enabled with `ckcdeb.h` pinned first**: see the
  **format-and-editor-tooling** skill (`doc/SIMPLIFY_SUMMARY_20260613.md` §3,
  `doc/MOVE_CKCDEB_H_20260615.md`, `doc/ENABLE_SORTINCLUDES_20260615.md`, `doc/INSERT_BRACES_20260615.md`).
- **Dead/unused file deletions**: `ckcsym.h` (`doc/CKCSYM_H_DELETION_20260615.md`), `ckusig.h`
  (superseded by `ckcsig.h` — **`ckusig.c` is kept and still built**), `ckcmdb.c`, `cku2tm.c`,
  `ckustr.c`/`ckustr.sed`, plus build cruft (`.cmake-format.py ckpker.mak ckubs2.mak
  makecmd.sh`). See `doc/BASE_20260616.md` §3.

## Rules distilled from the campaign

- **Do not remove `SVR4`, `SVR3`, or `ATTSV`** (also in CLAUDE.md): live on macOS/BSD via the
  `MACOSX10` → `MACOSX` → `BSD44` → `SVR4` → `SVR3`/`ATTSV` implication chain in `ckcdeb.h`;
  undefined only on the `linux` build.
- Before declaring any macro "always undefined", check the macOS/BSD builds
  (`gcc -E -DMACOSX10 …` / `-DBSD44`), not just `make linux`.
- `ckcpro.w` is the source of truth for the protocol state machine; after editing it,
  regenerate `ckcpro.c` (`make wart && make ckcpro.c`) — see the build-and-verify skill.
- Every simplification gets a dated `*_20260NNN.md` report under `doc/` and a verification
  at the strongest applicable level (byte-identical / per-object / token proof / runtime —
  see the build-and-verify skill).
- Build test reports: `doc/UBUNTU_TEST_20260612.md`, `doc/UBUNTU_TEST_20260613.md`.
