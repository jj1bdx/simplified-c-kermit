# Codebase Simplification Report — `NODEPRECATED` hard-wired (RLOGIN client removed)

**Date:** 2026-07-10
**Tree:** C-Kermit 10.0 Beta.12 (`/home/kenji/src/simplified-c-kermit`), branch `remove-deprecated`
**Scope:** Treat the compile flag **`NODEPRECATED` as permanently defined**, with its
*actual* (strict) semantics: `NOTELNET` and `NORLOGIN` become hard-wired on, and
`RLOGCODE` becomes permanently undefined, which removes the **built-in RLOGIN client**.
The TELNET command, the Telnet protocol engine (`TNCODE`/`ckctel.c`), `TN_COMPORT`,
and IKSD are **deliberately untouched** — see "Scope decision" below.

## Summary

| Metric | Before | After | Delta |
|---|---|---|---|
| Source lines (`*.c *.h *.w`, excl. generated `ckcpro.c`) | 220,948 | 219,996 | **−952** |
| Source files edited via `unifdef -m` | — | 14 | incl. `ckcpro.w` (then `wart`-regenerated) |
| Files hand-edited | — | 7 | `ckcdeb.h`, `ckcmai.c`, `ckcnet.c`, `ckcnet.h`, `ckcfnp.h`, `ckuus5.c`, `makefile` |
| `makefile` lines | 1,084 | 1,063 | −21 (two `*-nodeprecated` stanzas + 2 doc mentions) |
| **Whole diff** | | | **119 insertions, 1,092 deletions (18 files)** |
| `wermit` binary (linux) | 2,669,352 | 2,664,904 | **−4,448 bytes** |

Build verification on Linux (this host): **success** — `make linux` exits 0, **zero
compiler warnings**. `SHOW FEATURES` now prints ` No built-in RLOGIN client` and no
longer lists `RLOGCODE`; `TNCODE` and `TN_COMPORT` remain listed (Telnet intact).

## Scope decision — what `NODEPRECATED` actually does

The `ckcdeb.h` `NODEPRECATED` block (old lines 123–133) defined `NOTELNET`, `#undef`'d
`TNCODE`, and defined `NORLOGIN`, with intent "No more Telnet client / No more RLOGIN
client" (-fdc, 12 May 2022). Empirically, upstream **never finished the Telnet half**:

- The `#undef TNCODE` was a **no-op** on every real build, because the
  `TCPSOCKET`-implies-`TNCODE` backstops (old `ckcdeb.h:486–490`, `ckcnet.h:354–357`)
  and the `CK_LOGIN`-implies-`TNCODE` backstop (IKSD login runs over the Telnet
  protocol, old `ckcdeb.h:3506–3510`) all execute *after* the block and re-define it.
- The TELNET command keytab entry (`ckuusr.c`) is gated by `TCPSOCKET`, not
  `NOTELNET`; `NOTELNET` gated exactly one thing in the whole tree — a
  " No built-in TELNET client" line in `SHOW FEATURES`.
- Only `NORLOGIN` had teeth: it prevents `ckcnet.h` from defining `RLOGCODE`
  (the rlogin client implementation flag) on `__linux__`/`BSD44`.

Proved with `gcc -E -dM` dumps (see Verification). Truly removing the Telnet engine
would also force IKSD out (its login *requires* Telnet negotiation, RFC 2839/2840) —
a much larger, different change. The user was asked and chose **strict flag
semantics**: hard-wire exactly what `-DNODEPRECATED` does, i.e. remove the RLOGIN
client only; TELNET and IKSD stay.

## Method

1. **Analysis** by a `c-expert` subagent: complete guard-site inventory for
   `RLOGCODE`/`NORLOGIN`/`NOTELNET`, dangling-reference audit (the bug class
   `unifdef` leaves behind), `ckcdeb.h`/`ckcnet.h` arbitration plan, makefile stanza
   boundaries, and the verification checklist.
2. **Mechanical pass:** `/usr/bin/unifdef -DNOTELNET -DNORLOGIN -URLOGCODE -m` over
   14 files (all exited 1 = modified, none 2 = parse error): `ckcnet.h ckctel.c
   ckcfns.c ckuusy.c ckuus7.c ckuusx.c ckcnet.c ckuus5.c ckuus4.c ckuus2.c ckutio.c
   ckcpro.w ckuusr.c ckuus3.c`. `ckuus6.c` has zero references (untouched).
   `ckcpro.c` was regenerated from the edited `ckcpro.w` via `make wart && make
   ckcpro.c` (never hand-edited; gitignored).
3. **Hand edits** to `ckcdeb.h` (excluded from unifdef — `-DNOTELNET` would have
   deleted the very `#ifndef NOTELNET #define NOTELNET #endif` trio being installed)
   and to the dangling-reference hazards below.
4. **Makefile:** deleted the `linux-nodeprecated` and `netbsd-nodeprecated` stanzas
   (now no-ops — the flag's effect is unconditional) and their two mentions in the
   commonly-used-targets comment.
5. **`clang-format`** re-normalization of the 8 touched files that drifted (guard
   removal un-nests the `#ifdef RLOGCODE if (...) { ... } else { #endif` bracket
   idiom); format-stability gate then confirmed clean on every touched file.

## Macro logic — why `NORLOGIN` alone kills `RLOGCODE`

`RLOGCODE` was `#define`d in **exactly one place** — the arbitration at old
`ckcnet.h:367–383` — gated `#ifndef NORLOGIN` (then `#ifdef __linux__` /
`#ifdef BSD44`). No makefile target passes `-DRLOGCODE`, and nothing else defines it.
`ckcdeb.h` (always the first `#include`) now defines `NORLOGIN` unconditionally, so
`RLOGCODE` can never become defined, platform-independently.

**Equivalence proof:** `gcc -E -dM` full-macro dumps for the three live target
families (`-DLINUX -DCK_POSIX_SIG -DTCPSOCKET -DPOSIX …`, `-DMACOSX10 -DMACOSX103
-DTCPSOCKET -DCKHTTP`, `-DBSD44 -DTCPSOCKET -DOPENBSD`), each probed both with
`ckcdeb.h` alone and with `ckcdeb.h`+`ckcnet.h` (six configurations): the dump of the
**pre-edit tree with `-DNODEPRECATED`** and the **post-edit tree with no flag** differ
in exactly one line — `#define NODEPRECATED 1` itself. `NOTELNET`/`NORLOGIN` defined,
`RLOGCODE` undefined, and `TNCODE`, `TN_COMPORT`, `IKS_OPTION`, `IKSD`, `CK_LOGIN`,
`CKSENDUID`, `CK_ENVIRONMENT`, `CK_SNDLOC`, `CK_NAWS`, `TTLEBUF` bit-for-bit
unchanged on all six.

## What was removed

- **`ckcdeb.h`:** the `#ifdef NODEPRECATED` block replaced by an explanatory comment
  plus unconditional `NOTELNET`/`NORLOGIN` definitions (the inert `#undef TNCODE`
  dropped — see comment in file); three now-moot `#ifdef RLOGCODE #undef RLOGCODE`
  trios (in the `IKSDONLY`×2 and `NONET` cascades); the now-redundant `NORLOGIN`
  trio in the `NOTCPIP` cascade.
- **`ckcnet.c`:** the rlogin implementation — `rlog_ini()`, `rlog_naws()`,
  `rlog_ctrl()`, `rlog_oob()`, `rlogoobh()`, the `rlog_mode`/`rlog_inband`/
  `rlog_stopped` globals, "login"/"rlogin" service- and URL-recognition, the
  login-port-mismatch warning (−624 lines, the bulk of the change).
- **`ckuusy.c`:** the `I_AM_RLOGIN` command-line personality branch and `dorlgarg()`
  (rlogin-style `-l`/`-h`/`-d`/`-q` argument parser) (−190 lines).
- **`ckuus7.c`:** `SET HOST` rlogin machinery — `shrlgtab[]`, the `/rlogin` entry in
  `tcprawtab[]`, rlogin username/service prompting in `setlin()` (−161 lines).
- **UI/help:** `HELP RLOGIN` topic, HELP bullets, `SET NETWORK` help variant,
  CONNECT-status and `SHOW NETWORK` "rlogin protocol" lines, `RLOGCODE` in the
  `SHOW FEATURES` options list (`ckuus2.c ckuus3.c ckuus4.c ckuus5.c ckuusr.c
  ckuusx.c ckcfns.c ckctel.c ckutio.c ckcpro.w`).
- **`makefile`:** `linux-nodeprecated`, `netbsd-nodeprecated` targets.

## Dangling-reference hazards found and fixed (beyond unifdef)

1. **`ckcnet.c` `setnproto()` — unguarded `case 513:`** set `ttnproto = NP_RLOGIN`
   for a numeric service 513 (`set host <host> 513`). The string case `"login"` was
   properly `RLOGCODE`-guarded, the numeric one was not. With the handshake code
   gone this would have produced a silently mis-tagged raw connection. Fixed:
   case deleted; port 513 now falls to `default: NP_NONE` (generic TCP).
2. **`ckcmai.c` — unguarded argv[0] personality detection** set `howcalled =
   I_AM_RLOGIN` when the binary is invoked/symlinked as `rlogin`, whose handler in
   `ckuusy.c` is gone; `usage()` would still have shown rlogin-flavored help for a
   parser that no longer implements those semantics (a pre-existing latent
   inconsistency in every historical `-DNODEPRECATED` build; the adjacent
   `I_AM_HTTP` clause is correctly guarded). Fixed: clause deleted — a binary named
   `rlogin` now gets the generic Kermit personality and generic usage text.
3. **Dangling prototypes:** `rlog_ctrl()` was declared unguarded in `ckcnet.h` and
   `ckcfnp.h` (it already had zero callers in the whole tree — pre-existing dead
   code); both declarations removed with the definition.
4. **Mislabeled `#endif`:** `ckcnet.c` had a pre-existing swapped-comment pair — the
   `#endif` closing `#ifndef NOTCPIP` was commented `/* NORLOGIN */` (and the one
   closing an rlogin guard said `/* NOTCPIP */`). unifdef matches by nesting, not
   comments, so correctness was unaffected; the surviving wrong label was corrected
   to `/* NOTCPIP */`.

Verified non-hazards: the `case XXRLOG:` help entry is label+body inside one guard
(no FTP-style orphaned-label fallthrough); `rlog_naws()`'s four forward declarations
and all call sites are uniformly guarded; the `rlog_*` globals have no external
references; no unused-variable fallout (zero new warnings).

## Behavior changes

- **`rlogin` command:** still parses (its keytab entry is `TCPSOCKET`-gated by
  upstream design) and prints `?Sorry, RLOGIN is not configured in this copy of
  C-Kermit.` — byte-identical to what a `-DNODEPRECATED` build already did.
- **`help rlogin`** → `Sorry, help not available for "rlogin"`.
- **`set host <host> /rlogin`** → `/rlogin` no longer a switch; falls through to
  service-name lookup: `Can't find port for service rlogin` (clean failure).
- **`set host <host> 513`** → generic TCP connection attempt (`NP_NONE`), no rlogin
  tagging (hazard-1 fix).
- **Binary invoked as `rlogin`** → generic Kermit personality and usage text
  (hazard-2 fix).
- **`SHOW FEATURES`** → gains unconditional ` No built-in RLOGIN client`; `RLOGCODE`
  gone from the options list.
- **One deliberate deviation from strict letter-of-the-flag semantics:** a
  `-DNODEPRECATED` build also printed ` No built-in TELNET client`, which is
  **false** on this tree (the TELNET command and engine are fully functional). That
  single `printf` was dropped rather than hard-wired; `NOTELNET` itself is still
  defined for macro-state parity (see the equivalence proof — the *macro* state is
  exactly `-DNODEPRECATED`'s).

## Verification

- **Build:** `make clean && make linux` exit 0, zero warnings; `ckcpro.c`
  regenerated from `ckcpro.w` by `wart` before the build.
- **Macro equivalence:** the six-configuration `gcc -E -dM` diff above (only
  `NODEPRECATED` itself differs). Covers the macOS (`MACOSX10`) and BSD (`BSD44`)
  implication chains per the SVR4/ATTSV caveat in CLAUDE.md.
- **Format:** every touched file passes `clang-format FILE | diff FILE -` (empty).
- **Runtime smoke tests (linux):** banner unchanged (`C-Kermit 10.0.416 Beta.12,
  2025/03/22`); the five behavior changes above observed exactly as described;
  `help telnet` and `set host localhost /telnet` exercise the intact Telnet path
  (DNS lookup → connect attempt → clean refusal with nothing listening);
  `PAUSE 2` = 2.00 s (signal-path sanity).
- Not applicable: byte-identical rebuild gate (this is a feature removal, not a
  refactor).

## Residuals (inert, kept deliberately)

- `NOTELNET` now gates **nothing** — it is defined in `ckcdeb.h` purely for macro
  parity with `-DNODEPRECATED` and as documentation; a future pass may remove it.
- `NP_RLOGIN`/`NP_K4LOGIN`/`NP_EK4LOGIN`/`NP_K5LOGIN`/`NP_EK5LOGIN` constants and
  the `IS_RLOGIN()` macro remain defined in `ckcnet.h` (unguarded upstream; nothing
  can set `ttnproto` to these values any more). Likewise the `I_AM_RLOGIN` tag, the
  rlogin `hlp3[]` usage text in `ckuusy.c`, the RLOGIN failure hint in `ckuus7.c`,
  and two "rlogin protocol" status printfs in `ckuus4.c` are unreachable but
  harmless — same disposition as `I_AM_FTP` in the NOFTP removal.
- The `RLOGIN Modes` `RL_RAW`/`RL_COOKED` defines in `ckcnet.h` are now unreferenced.
- `TELNET` (the ancient VMS-era macro, distinct from `NOTELNET`) still appears in
  the `NOTCPIP` cascade; always undefined on live targets, out of scope here.

## Cross-references

- `doc/NOFTP-20260709.md` — the FTP-client removal this change mirrors in method.
- `doc/BASE_20260616.md`, `doc/BASE_20260709.md` — consolidated simplification
  overviews.
- `ckcdeb.h` (the hard-wire comment block) — points back to this file.
