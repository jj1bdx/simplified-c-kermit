# IFDEF-NESTING-20260710 — preprocessor-conditional nesting depth audit

Date: 2026-07-10

## Motivation

`clang-format` is visibly slow on parts of this tree. A quick timing spot-check makes the point
better than any theory:

| File | Lines | Max `#if`/`#ifdef`/`#ifndef` nesting | `clang-format` wall time |
|---|---:|---:|---:|
| `ckcuni.c` | 23,312 | 2 | 0.99 s |
| `ckuus5.c` | 11,098 | 6 | 1.17 s |
| `ckufio.c` | 7,931 | 5 | 1.60 s |
| `ckuusr.h` | 2,569 | 7 | 1.37 s |
| `ckucmd.c` | 7,993 | 7 | 2.15 s |
| `ckuusx.c` | 7,038 | 8 | 3.62 s |
| `ckutio.c` | 11,929 | 9 | 11.96 s |
| **`ckcdeb.h`** | **3,705** | **13** | **~82 s (4 runs: 81.88/81.90/81.65/82.19 s)** |

`ckcdeb.h` is the **smallest** file measured (3,705 lines — about one sixth the size of
`ckcuni.c`, which is mostly flat character-set data tables and has far fewer `#ifdef` directives
*and* far shallower nesting) and yet takes **~70× longer to format than `ckuus5.c`** (11,098
lines, depth 6) and **~83× longer than `ckcuni.c`** (23,312 lines — 6× more lines — depth 2).
Even against the next-deepest file, `ckutio.c` (depth 9), `ckcdeb.h` is still ~7× slower despite
being a third the size. File size does not predict `clang-format` cost here; conditional-nesting
depth (and, more precisely, sustained runs of many directives *at* high depth — see `ckcdeb.h`'s
`BPS_*` ladder below) does.
This report measures nesting depth precisely across the tree and proposes concrete,
behavior-preserving ways to bring it down, prioritized by where the worst offenders are.

## Methodology

**Scope**: all `*.c`, `*.h`, `*.w` files in the repo root — 44 files, 219,202 lines total.
**`ckcpro.c` is excluded**: it is a generated, gitignored build artifact of `ckcpro.w` (produced
by the `wart` preprocessor; see `.claude/skills/build-and-verify`), not a source file — analyzing
it would double-count `ckcpro.w`'s conditionals through a mechanically-expanded lens.  `ckcpro.w`
itself **is** in scope and was scanned like any other file.

**Counting rule**: `#if`/`#ifdef`/`#ifndef` push the nesting depth by 1; `#endif` pops it;
`#elif`/`#else` do not change it (they're a branch of the enclosing conditional, not a new one).
"Max depth" is the highest value the stack reaches anywhere in the file.

**A naive line-anchored regex is not sufficient.** A prompt-supplied baseline scan (`^\s*#\s*(if|
ifdef|ifndef|elif|else|endif)`, no awareness of comments or string literals) was used as a
starting point and cross-checked with a real lexer. A purpose-built Python scanner walks each
file character-by-character with a small state machine (`NORMAL` / `BLOCK_COMMENT` /
`LINE_COMMENT` / `STRING` / `CHAR`), so:

- a `#`-looking line that is actually inside a `/* ... */` block comment — including block
  comments that *span into* the line, where the line itself has no `/*` of its own — is not
  counted;
- directive lines are only recognized when the `#` is the first non-whitespace, non-comment token
  since the last real newline (matching real C phase-4 lexing);
- backslash-newline continuation inside a directive is honored when reconstructing its condition
  text for the chain reports below.

Every file's scan ends with the stack back at depth 0 and lexer state `NORMAL` (no unterminated
comments/strings, no unmatched `#endif`) — i.e. the accurate scanner agrees the tree is
well-formed everywhere, which is itself a useful cross-check.

**Result: the naive baseline was correct for 43 of 44 files.** Only `ckuus3.c` differed.

### The `ckuus3.c` anomaly, resolved

The naive scan reports 552 openers vs. 551 `#endif`s for `ckuus3.c` (never returns to depth 0)
and a max depth of 7. The comment/string-aware scanner reports depth 6, balanced. The extra
"opener" is **line 3852**, inside a `/* ... */` block comment that runs from line 3846 to 3853:

```c
3845: #ifdef FNFLOAT
3846:           /*
3847:             This gets a "Types incompatible in conditional expression"
3848:             warning in HP-UX 10 with the pre-ANSI HP C 76.3 compiler
3849:             if there is no prototype for fpformat... even though
3850:             pre-ANSI C does not have prototypes!  This prototype has
3851:             been added above just after '#include "ckcfnp.h"' within
3852:             #ifdef HPUX10..#endif.
3853:           */
```

Line 3852 is *prose* — a comment describing where a prototype was added, referencing the literal
text `#ifdef HPUX10..#endif.` as an English description of another part of the file. Because it's
indented and begins with `#ifdef`, the naive regex (which only checks the start of the line, not
the lexer state) miscounts it as a real opener with no matching `#endif` on the same line (the
trailing `..#endif.` doesn't match `^\s*#\s*endif` either, since it isn't at the start of the
line) — hence the 552-vs-551 imbalance and the inflated max of 7. This is a **near-miss**, not
the culprit: task guidance flagged the *other* `ckuus3.c` HP-UX-comment landmine at lines
11389/11398 (`/* #ifdef CK_XYZ */` … `/* #endif */`, documented in
`doc/NOCKXYZ-20260709.md`'s residuals) as *not* the cause, because both of those lines have `/*`
**before** the `#`, so the naive regex's `^\s*#` anchor never matches them at all — correctly
ruled out in the task brief. Line 3852 is the real (and only) false positive in the tree; it has
no `/*` before the `#ifdef` text because the comment opened three lines earlier.

No source change is proposed for this — it's a comment, not a defect — but it is the reason this
report's depth numbers use the accurate scanner throughout rather than the naive baseline.

## Part 1 — Per-file max nesting depth (accurate, sorted deepest first)

| Depth | Files |
|---:|---|
| 13 | `ckcdeb.h` |
| 9 | `ckutio.c` |
| 8 | `ckcnet.c`, `ckuus4.c`, `ckuusx.c` |
| 7 | `ckctel.c`, `ckucmd.c`, `ckuusr.h` |
| 6 | `ckcker.h`, `ckcmai.c`, `ckudia.c`, `ckupty.c`, `ckuus2.c`, **`ckuus3.c`** (corrected from naive 7), `ckuus5.c`, `ckuus6.c`, `ckuus7.c`, `ckuusr.c`, `ckuusy.c` |
| 5 | `ckcfn2.c`, `ckcnet.h`, `ckucns.c`, `ckufio.c` |
| 4 | `ckcfns.c`, `ckclib.h`, `ckcpro.w`, `ckctel.h`, `ckcxla.h`, `ckucmd.h`, `ckucon.c`, `ckuscr.c`, `ckuver.h`, `ckuxla.c` |
| 3 | `ckcfn3.c`, `ckcfnp.h`, `ckupty.h` |
| 2 | `ckclib.c`, `ckcsig.h`, `ckcuni.c`, `ckcuni.h`, `ckuxla.h`, `ckwart.c` |
| 1 | `ckcasc.h`, `ckusig.c` |

44 files, 0 scanner errors, 0 unclosed conditionals. Every entry above matches the naive
line-regex baseline except `ckuus3.c` (naive 7 → accurate 6, explained above).

## Hot-spot breakdown (depth ≥ 7): where the max occurs and the macro chain

For each file, the chain is the live directive stack (outermost → innermost) at the point the
file's maximum depth is first reached — i.e. the actual set of nested conditions active there.
Where a link is reached through that condition's `#else` branch (the *negation* of what's
written), it's noted.

### `ckcdeb.h` — depth 13, line 2138

```
#ifndef CKCDEB_H            (47, include guard)
 #ifdef  TTSPDLIST           (1965)
  #ifdef  BPS_1500K           (2108) ─┐
   #ifdef  BPS_921K            (2111)  │
    #ifdef  BPS_460K            (2114)  │ 11-deep #else/#ifdef
     #ifdef  BPS_230K            (2117)  │ "cascade" — see Part 2
      #ifdef  BPS_115K            (2120)  │
       #ifdef  BPS_76K             (2123)  │
        #ifdef  BPS_57K             (2126)  │
         #ifdef  BPS_38K             (2129)  │
          #ifdef  BPS_28K             (2132)  │
           #ifdef  BPS_19K             (2135)  │
            #ifdef  BPS_14K             (2138) ─┘
```
This is the `MAX_SPD` (max serial port speed) selector — 11 mutually-exclusive `#ifdef X #else
#ifdef Y #else ...` steps, each testing a *different* macro (not testing the same macro twice),
terminated by a flat `#define MAX_SPD 9600L` (line 2141) and 11 closing `#endif`s in a row
(`ckcdeb.h:2142-2152`, followed by a 12th `#endif /* TTSPDLIST */` at 2153 that closes the outer
wrapper). Single biggest depth contributor in the tree.

### `ckutio.c` — depth 9, line 3109 (inside `tthang()`, DTR-drop-on-hangup logic)

```
#ifdef  NOLOCAL   (2778) → #else (2780, i.e. NOT NOLOCAL)
 #ifdef  HUP_POSIX (2833) → #else (2890, i.e. NOT HUP_POSIX)
  #ifdef  BSD44ORPOSIX (2891) → #else (3047, i.e. NOT BSD44ORPOSIX)
   #ifdef  ATTSV       (3076)
    #ifndef _IBMR2       (3087)
     #ifdef  TIOCMBIS      (3094)
      #ifdef  TIOCMBIC      (3095)
       #ifdef  TIOCM_DTR     (3096)
        #ifndef CLSOPN         (3109)
```
Levels 1–3 are a genuine priority-ordered "try POSIX, else try BSD, else fall through" ladder for
raising/lowering DTR (each has real, different code in its `#else`). Levels 4–9
(`ATTSV`→`CLSOPN`) are **pure AND-nesting with zero `#else` anywhere in that span** — confirmed by
walking every directive from `ckutio.c:3076` to its matching `#endif` at `ckutio.c:3249`: no
`#else` appears between `ATTSV`'s opening and `CLSOPN`'s `#endif` at `ckutio.c:3111`.

### `ckcnet.c` — depth 8, line 1206

```
#ifndef NETCONN   (326)
 #ifndef NONET     (671)
  #ifdef  TCPSOCKET  (1116)
   #ifndef NOLISTEN   (1117)
    #ifdef  NOTUSED     (1154)   ← dead code, see Part 2
     #ifndef NOTCPOPTS    (1202)
      #ifdef  SOL_SOCKET    (1203)
       #ifdef  TCP_NODELAY    (1206)
```
The innermost 4 levels (`NOTUSED`→`TCP_NODELAY`) sit **inside a block that can never compile**:
`NOTUSED` is a leftover "commented out via `#ifdef`" idiom and is never `#define`d anywhere in
this tree (verified below). Deleting that dead block drops this file's real max from 8 to 6.

### `ckuus4.c` — depth 8, line 4762

```
#ifndef NOICP → #ifndef NOSHOW → #ifndef NOLOCAL → #ifndef NONET (4584)
 #ifndef NETCONN (4587) → #else (4590, i.e. NETCONN defined)
  #ifdef  NOLOCAL (4591) → #else (4594, i.e. local support present)
   #ifdef  TCPSOCKET (4721)
    #ifdef  TNCODE (4762)
```
`TCPSOCKET`→`TNCODE` at the bottom is a clean 2-level AND with no `#else`. The four outer `NOICP`/
`NOSHOW`/`NOLOCAL`/`NONET` wrappers are the "feature-subset build" guards discussed in Part 2
(live, not removable — see the `V7MIN` caveat).

### `ckuusx.c` — depth 8, line 122

```
#ifndef NETCONN  (101)
 #ifdef  TCPSOCKET (105) → #else (107)
  #ifdef  IBMX25     (108) → #else (110)
   #ifdef  HPX25       (111) → #else (113)
    #ifdef  DECNET       (114) → #else (116)
     #ifdef  NPIPE          (117) → #else (119)
      #ifdef  CK_NETBIOS       (120)
       #ifdef  SUPERLAT           (122)
```
This is the *network-transport-type selection ladder*: `TCPSOCKET` / `IBMX25` / `HPX25` /
`DECNET` / `NPIPE` / `CK_NETBIOS` are mutually exclusive `#else`/`#ifdef` cascade steps (same
anti-pattern as the `ckcdeb.h` speed ladder), with `SUPERLAT` as one further genuine AND-nested
sub-condition inside the `CK_NETBIOS` arm only. Confirmed **all of `IBMX25`, `HPX25`, `DECNET`,
`NPIPE`, `CK_NETBIOS`, `SUPERLAT` are never `#define`d anywhere in this tree** (`grep -rn
"define IBMX25\|define HPX25\|..."` across `*.c *.h makefile`: zero hits for any of the six, on
any of the three build flag sets — see Part 2). Also notable: the `#endif` trailer comments on
`ckuusx.c:126-128` read `/* TCPSOCKET */ /* SUNX25 */ /* STRATUSX25 */` — `SUNX25`/`STRATUSX25`
are never opened anywhere in this block; the comments are stale copy-paste debris from an older,
larger version of this ladder, a small illustration of how these cascades rot once nested this
deep (nobody re-derives the comments by hand at 8 levels of indent).

### `ckctel.c` — depth 7, line 1283

```
#ifndef NONET → #ifndef NOTCPIP → #ifdef TNCODE → #ifdef IKS_OPTION (1136)
 #ifndef NOXFER (1272)
  #ifndef IKSDONLY (1282)
   #ifdef  CK_AUTODL (1283)
```
`IKSDONLY`→`CK_AUTODL` is a clean AND pair, no `#else`.

### `ckucmd.c` — depth 7, line 7412

```
#ifndef NOICP (76)
 #ifdef  CMD_CONINC     (7371) → #else (7376)
  #ifdef  __FILE_defined  (7380) → #else (7383)
   #ifdef  _IO_file_flags   (7384) → #else (7387)
    #ifdef  USE_FILE_CNT      (7388) → #else (7401)
     #ifdef  USE_FILE__CNT      (7402) → #else (7411)
      #ifdef  USE_FILE_R           (7412)
```
A third cascade instance: this one is a **glibc/libc-internals detection ladder** (probing which
internal `FILE` struct field holds the input buffer count, for raw-mode arrow-key input editing)
— `CMD_CONINC` → `__FILE_defined` → `_IO_file_flags` → `USE_FILE_CNT` → `USE_FILE__CNT` →
`USE_FILE_R`, again written as nested `#ifdef`/`#else` instead of `#elif`. (A genuinely orthogonal
`NOARROWKEYS` AND-condition is nested one level inside the `USE_FILE_CNT` arm only —
`ckucmd.c:7389` — so this one doesn't flatten to a single `#if`/`#elif` chain quite as cleanly as
the other two; see Part 2.)

### `ckuusr.h` — depth 7, line 171

```
#ifndef CKUUSR_H (13, include guard) → #ifndef NOICP (146) → #ifndef CK_SYSINI (160)
 #ifdef  CK_DSYSINI (161)
  #ifdef  UNIX (165)
   #ifdef  CU_ACIS (168) → #else (170)
    #ifdef  __linux__ (171)
```
Small instance of the same `#else`/`#ifdef` cascade pattern (`CU_ACIS` vs. `__linux__`).

## Part 2 — Patterns behind the depth

Four recurring shapes account for nearly all of the deep nesting; they call for different fixes.

**1. Cascading `#else` / `#ifdef` chains standing in for `#elif`.** By far the largest
contributor. A tree-wide scan for "`#else` immediately followed by `#ifdef`/`#ifndef` on the very
next directive line" (the signature of this anti-pattern — the `#else` branch's *entire* body is
just another nested conditional) found:

| File | Cascade instances |
|---|---:|
| `ckutio.c` | 87 |
| `ckcdeb.h` | 71 |
| `ckufio.c` | 49 |
| `ckcnet.c` | 28 |
| `ckuusx.c` | 19 |
| `ckuus5.c` | 14 |
| (17 more files) | 1–8 each |

Tree-wide total: **335 instances across 23 files** — this shape alone outnumbers every other
pattern in this report combined.

The `BPS_*` speed ladder (`ckcdeb.h:2108-2144`) and the network-transport ladder
(`TCPSOCKET`/`IBMX25`/`HPX25`/`DECNET`/`NPIPE`/`CK_NETBIOS`, appearing near-verbatim in
`ckuusx.c:105-131`, `ckcmai.c:967-980`, and elsewhere) are the two biggest instances and are both
quoted above. `ckucmd.c`'s libc-internals ladder (`ckucmd.c:7371-7412`) is a third pattern
variant: same anti-pattern, but selecting among *implementation strategies* rather than
platforms.

**2. Genuine AND-nesting with no `#else` anywhere in the span.** `ckutio.c`'s
`ATTSV`→`_IBMR2`→`TIOCMBIS`→`TIOCMBIC`→`TIOCM_DTR`→`CLSOPN` (6 levels, `ckutio.c:3076-3249`,
confirmed no intervening `#else`) is one example. `ckcnet.c:776-1114` has six near-identical
triples of the same shape — `NOTCPOPTS`→`SOL_SOCKET`→one of `{SO_LINGER, SO_SNDBUF, SO_RCVBUF,
SO_KEEPALIVE, SO_DONTROUTE, TCP_NODELAY}`, each further nested one level under `IKSD` — all live
code reaching depth 6. (A *second*, near-identical but dead `NOTCPOPTS`→`SOL_SOCKET`→`TCP_NODELAY`
triple, at `ckcnet.c:1202-1206` inside the `NOTUSED` block, is the file's actual depth-8 hot spot
— see pattern 4, not this one.) In both files, each level is a capability macro that must ALL be
true simultaneously for the guarded code to make sense, written as sequential nesting instead of a
single compound condition.

**3. Whole-file / whole-section feature-subset wrappers.** `#ifndef NOICP` wraps most of the body
of `ckuus2.c`, `ckuus3.c`, `ckuus4.c`, `ckuus5.c`, `ckuus6.c`, `ckuus7.c`, `ckucmd.c`, `ckuusr.c`,
`ckuusr.h` — one "free" level of nesting at the top of nine interactive-UI source files, plus
the parallel `#ifndef NONET`/`#ifndef NOLOCAL`/`#ifndef NOXFER` wrappers nested inside it in
several of them (visible in the `ckuus4.c` chain above). These are the reduced-feature-set build
guards for `KFLAGS=-DV7MIN` (a real, currently-usable "smallest possible C-Kermit" build mode
documented in `ckcdeb.h:168-184` — not wired into any named `make` target, but invocable by hand
per that comment, and not dead: **do not hard-wire these away**, matching the spirit of the
`SVR4`/`SVR3`/`ATTSV` warning in `CLAUDE.md`).

**4. Dead code hiding behind `#ifdef NOTUSED`.** `NOTUSED` is a self-referential guard name — the
kind an author writes to comment out a whole function without deleting it. It is `#define`d
**nowhere** in this tree (`grep -rn "define NOTUSED" *.c *.h makefile` → zero hits; confirmed
also via `gcc -E -dM` full-macro dumps for `linux`/`-DMACOSX10`/`-DBSD44`, none define it). It
gates three blocks:

- `ckcnet.c:1154-1299` (145 lines, includes the `NOTCPOPTS`→`SOL_SOCKET`→`TCP_NODELAY` depth-8
  hot spot above) — `int tcpsocket_open(...)` with a body that doesn't even parse (`int timo {`
  — a missing close-paren typo, further proof it hasn't been compiled in a very long time).
- `ckuxla.c:2326-2340` — a dead `yasl1[]` translation table.
- `ckufio.c:5203-5208` — a dead no-op `zkermini()`.

All three are unreachable on every platform this tree builds for.

## Part 3 — Proposed fixes, ordered by benefit/risk

None of these have been applied — analysis only, per the task. Each names its verification level
using the numbering from `.claude/skills/build-and-verify` ("Level 1" = byte-identical rebuild …
"Level 4" = macOS/BSD path checks).

### Fix (1): Rewrite `#else`/`#ifdef` cascades as `#if`/`#elif`/`#else` chains — highest priority

**Mechanical, zero-risk transform.** The C standard defines `#else #if X ... #endif #endif` as
behaviorally identical to `#elif X ... #endif` in every case where nothing but whitespace/comments
sits between the `#else` and the nested `#if` — true for every instance tallied in Part 2's table
(that's exactly what the "immediately followed by" heuristic searched for). This is a pure
re-punctuation of existing `#ifdef`/`#else` keywords; no macro is added, removed, or reordered,
and the set of `#define`s each arm produces is untouched.

- **`ckcdeb.h` `BPS_*` ladder**: 11 nested steps → 1 flat `#if defined(BPS_1500K) #elif
  defined(BPS_921K) #elif ... #else ... #endif`. File max drops from **13 → 3**
  (`CKCDEB_H` include guard + `TTSPDLIST` + the flat elif chain). This single edit removes 10
  levels from the tree's worst offender and, per the Motivation section's timing data, is the one
  most likely to actually move the needle on `clang-format` wall time (it's not just the deepest
  point but the *longest run of directives sitting at high depth* in the whole tree — 33
  directives, 11 `#ifdef`/11 `#else`/11 `#endif`, packed into `ckcdeb.h:2108-2153`, with 6 of the
  11 steps at depth ≥ 8).
- **`ckuusx.c` transport ladder**: 6 nested steps + 1 genuine AND (`SUPERLAT` inside `CK_NETBIOS`)
  → `#if defined(TCPSOCKET) #elif defined(IBMX25) #elif ... #elif defined(CK_NETBIOS) #ifdef
  SUPERLAT ... #endif #endif`. File max drops from **8 → 3**. A near-identical cascade (missing
  `NPIPE`/`CK_NETBIOS`, no `SUPERLAT` sub-nest) sets `nettype` in `ckcmai.c:967-985`; collapsing it
  the same way takes that file's max from **6 → 2** (just the `NETCONN` wrapper plus the flat elif
  chain — there's no leftover AND-nest in this instance).
- **`ckucmd.c` libc-internals ladder**: all 6 arms (`CMD_CONINC`, `__FILE_defined`,
  `_IO_file_flags`, `USE_FILE_CNT`, `USE_FILE__CNT`, `USE_FILE_R`) collapse into one `#if`/`#elif`
  chain; `NOARROWKEYS` (genuinely nested one level inside the `USE_FILE_CNT` arm only) stays
  nested inside that one `#elif` arm. File max drops from **7 → 3** (`NOICP` wrapper + the flat
  elif chain + the one surviving `NOARROWKEYS` nest).

**Risk**: none, if the "no code between `#else` and the nested `#if`" precondition holds — this
report verified it holds for all three quoted examples by reading the full span, not just
inferring it from the adjacency heuristic; the heuristic should be re-checked per-instance before
mechanically applying it elsewhere (a `#else` line followed by a *comment* then an `#ifdef` a few
lines later, for instance, is not a hazard, but a `#else` block with a comment plus a couple of
`#define`s **before** the nested `#ifdef` would not collapse cleanly — the tree-wide 335-instance
count in Part 2 has not been individually vetted at that level, only the ones this report walks in
detail).
**Verification**: Level 3 (token-stream proof) — a comment/string-aware diff should show the
`#else`/`#ifdef` pairs replaced by `#elif` with **zero** change to which `#define`s are reachable
under which macro combinations; ideally backed by Level 1/2 (`SOURCE_DATE_EPOCH` clean rebuild,
`cmp` the binary, or per-object `.o` `cmp` if `clang-format`'s own comment rewrap perturbs
whitespace in the touched hunks) on `make linux`, plus Level 4 (`gcc -E -DMACOSX10`/`-DBSD44`
dumps of `ckcdeb.h` and `ckcnet.h`, since the `BPS_*` and transport ladders are platform-selection
logic) to confirm the macOS/BSD preprocessor output is unchanged token-for-token.

### Fix (2): Collapse pure-AND `#ifdef` nests into `#if defined(A) && defined(B) && ...`

Applies where consecutive nested `#ifdef`s have **no `#else` anywhere** in the enclosed span (verified,
not just adjacency-sniffed):

- `ckutio.c`: `ATTSV && !_IBMR2 && TIOCMBIS && TIOCMBIC && TIOCM_DTR && !CLSOPN`
  (`ckutio.c:3076-3111`) — 6 levels → 1, dropping the file's own max hot spot from depth 9 down to
  around 4 (the remaining `NOLOCAL`/`HUP_POSIX`/`BSD44ORPOSIX` cascade — itself a Fix-(1)
  candidate — plus this one collapsed level).
- `ckcnet.c`: each of the six `NOTCPOPTS && SOL_SOCKET && SO_LINGER` (/`SO_SNDBUF`/`SO_RCVBUF`/
  `SO_KEEPALIVE`/`SO_DONTROUTE`/`TCP_NODELAY`) triples (`ckcnet.c:776-1114`) — 3 levels → 1, with a
  genuinely orthogonal `IKSD` check remaining nested one level inside each (net: depth 6 → 4 at
  each site, relative to the file's own top).
- `ckctel.c`: `IKSDONLY && CK_AUTODL` (`ckctel.c:1282-1283`) — small but free.

This is the mechanical win named directly in the task brief; in this tree it turns out to be
**smaller** in aggregate than Fix (1), because most of the very deep chains here are cascades, not
ANDs — but it composes with Fix (1) (apply both to `ckutio.c` and the depth there drops from 9 to
roughly 3–4).

**Risk**: none — `#ifdef A #ifdef B ... #endif #endif` with nothing between the two `#ifdef`s and
nothing but `#endif #endif` at the close is definitionally identical to `#if defined(A) &&
defined(B) ...`. **Verification**: Level 3, same as Fix (1); Level 1/2 rebuild since these are all
Linux-reachable branches (`ATTSV` also gates live BSD/macOS code per `CLAUDE.md` — confirm with
Level 4 too before touching `ckutio.c`).

### Fix (3): Delete the three `#ifdef NOTUSED` dead-code blocks

Not a nesting-reduction technique per se, but the highest-leverage single deletion: removing
`ckcnet.c:1154-1299` alone drops that file's max from 8 to 6 (eliminates the `NOTUSED`→
`NOTCPOPTS`→`SOL_SOCKET`→`TCP_NODELAY` tail entirely, not just re-punctuates it), and removes 145
dead, non-compiling lines. The other two instances (`ckuxla.c:2326-2340`, `ckufio.c:5203-5208`)
are smaller but equally dead.

**Risk**: low but not zero — deletion (unlike Fixes 1–2) actually removes reachable-if-`NOTUSED`
code, so it depends on `NOTUSED` truly never being defined, forever, not just today. This report's
evidence (zero `#define NOTUSED` anywhere in `*.c`/`*.h`/`makefile`; zero `gcc -E -dM` hits for
`linux`/`-DMACOSX10`/`-DBSD44`) is the same standard of proof this tree's other "hard-wire"
write-ups use (e.g. `doc/NOCKXYZ-20260709.md`), but as with those, it should get its own dated
`doc/*.md` if actually applied, per that established pattern, rather than being folded quietly
into a formatting pass. **Verification**: Level 1 (byte-identical rebuild — the three functions
are already dead weight in every current object file, so the `.o`s and the final `wermit` should
be **byte-for-byte identical**, not just equivalent, which is a stronger and easier check than
Fixes 1–2 get); Level 4 for macOS/BSD.

### Fix (4): Do NOT hard-wire `TCPSOCKET`/`NETCONN`/`TNCODE` — flagged and rejected

These looked, at first pass, like the biggest hard-wiring opportunity in the hot spots (`TNCODE`
is defined whenever `TCPSOCKET` is, `ckcdeb.h:473-478`; `TCPSOCKET` is unconditionally defined for
`UNIX` builds, `ckcdeb.h:276`; both are always live in every `make linux`/`macos`/BSD build).
**Verified and rejected**: `ckcdeb.h:168-184` shows all three (indirectly, via `NONET`) are
disabled by the documented, currently-invocable `KFLAGS=-DV7MIN` minimum-size build — a real,
non-dead alternate build mode, not wired to a named `make` target but usable by hand per its own
comment. Hard-wiring `TCPSOCKET`/`NETCONN`/`TNCODE` "on" would silently break that build mode.
Recorded here mainly as a cautionary example — the same shape of mistake the `CLAUDE.md`
`SVR4`/`SVR3`/`ATTSV` warning exists to prevent, just for a different macro family. If `V7MIN` is
ever confirmed truly dead (e.g. by grepping every fork/downstream user, not just this repo), this
would become a legitimate high-value hard-wire target — `TCPSOCKET`/`NETCONN`/`TNCODE` nesting
appears in several of the depth-6/7/8 hot spots above — but that determination is out of scope
here and should not be made from this tree alone.

### Fix (5): `clang-format` configuration — already at its floor, no further win available there

`.clang-format` already sets `IndentPPDirectives: None` — `clang-format` is not re-indenting each
directive relative to its nesting depth (the `BeforeHash`/`AfterHash` settings, which would add
per-directive column computation proportional to depth, are off). There is no other
`.clang-format` knob that changes how many preprocessor branches the formatter has to walk or how
it accounts for `InsertBraces: true` (the project's one non-cosmetic transform, per
`.claude/skills/format-and-editor-tooling`) across conditional regions. The Motivation section's
timing data (`ckcdeb.h` at depth 13 taking ~80× longer than a 6×-larger, depth-2 file) is
consistent with `clang-format`'s known cost blowing up with *nesting depth × directive density* in
a region, not configurable away — Fixes (1)–(3) are the only way to move this number.

## Conclusion — recommended order of attack

1. **Fix (1) first, `ckcdeb.h` first.** One file, one mechanical rewrite (11-step `#else`/`#ifdef`
   → `#elif`), takes the tree's single worst offender from depth 13 to depth 3 and directly
   targets the 33-directive high-depth run (`ckcdeb.h:2108-2153`) that the timing data implicates
   as the actual `clang-format` cost driver. Highest benefit-to-risk ratio in this report by a wide
   margin.
2. **Fix (1) on `ckuusx.c`/`ckcmai.c` (transport ladder) and `ckucmd.c` (libc-internals ladder)**
   next — same technique, smaller files: depth 8→3, 6→2, and 7→3 respectively.
3. **Fix (3) — delete the three `NOTUSED` dead blocks** — independently low-effort, removes real
   dead weight (including one non-compiling function body) and trims `ckcnet.c`'s hot spot at the
   same time; do this whenever convenient relative to (1)/(2), not necessarily blocked on them.
4. **Fix (2) — AND-nest collapsing in `ckutio.c`/`ckcnet.c`/`ckctel.c`** — smaller aggregate win
   here than in most codebases (this tree's depth is dominated by cascades, not ANDs), but free
   once you're already re-verifying those files for (1)/(3), and composes with them.
5. **Leave Fix (4)'s candidates and the `NOICP`/`NONET`/`NOXFER` feature-subset wrappers alone** —
   both gate the real, if currently unused-by-any-`make`-target, `V7MIN` minimum-size build.

Every fix above is either a **re-punctuation of live logic** (Fixes 1–2, which change no `#define`
reachability at all) or a **deletion of already-dead branches** (Fix 3) — never a semantic change
to a live code path. None should alter `wermit`'s behavior on any of the three supported
platforms, and (1) and (2) in particular should be provable at Level 1 (byte-identical rebuild) or
Level 2 (per-object `.o` comparison) directly, the strongest gates this project has.
