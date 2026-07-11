# IFDEF-UNROLL-20260710 — applying the `#ifdef`-nesting fixes

Date: 2026-07-10

## Purpose

Running summary of applying the remediations proposed in `doc/IFDEF-NESTING-20260710.md`
(the nesting-depth audit), phase by phase, in the report's recommended order of attack. Each
phase is applied, verified against the levels in `.claude/skills/build-and-verify`, and appended
here before the user reviews and the coordinator commits it. No phase in this document has been
committed or pushed — that happens separately, per phase, after user approval.

Planned phases (per the audit's "Conclusion — recommended order of attack"):

1. `ckcdeb.h` `BPS_*` speed-selection ladder → `#elif` chain.
2. `ckuusx.c`/`ckcmai.c` network-transport ladder and `ckucmd.c` libc-internals ladder → `#elif`
   chains. **By user decision, `ckcdeb.h`'s `USE_LSTAT` cascade (flagged as Phase 1's
   discovered-but-deferred bottleneck) was folded into this phase too**, and — per its own
   re-scan-and-fix-if-trivial instruction — two further `ckcdeb.h` cascades (`openpty()`
   detection ×2, `CK_64BIT` detection) that surfaced as tied/successive bottlenecks while working
   through it. **A severe `clang-format`-time regression discovered in the `ckcdeb.h` edits was
   then investigated; two of the four `ckcdeb.h` cascades were reverted as a result — see the
   "Investigation" subsection below.**
3. Delete the three `#ifdef NOTUSED` dead-code blocks (`ckcnet.c`, `ckuxla.c`, `ckufio.c`).
4. AND-nest collapsing in `ckutio.c`/`ckcnet.c`/`ckctel.c`. **Given Phase 2's finding, any AND-nest
   collapse whose arms contain `#include` directives should get the same timing scrutiny before
   being kept.**

## Phase 1 — `ckcdeb.h` `BPS_*` ladder

### What changed

`ckcdeb.h`, the `MAX_SPD` (max serial port speed) selector. The 11-step cascade of `#ifdef X
#else #ifdef Y #else ...` (originally at lines 2109–2154 — see note below on the report's line
numbers) was rewritten as a single flat `#if defined(BPS_1500K) / #elif defined(BPS_921K) / ...
/ #else / #endif` chain. The `#ifdef TTSPDLIST` wrapper and the `CKCDEB_H` include guard were left
untouched; all 11 `#define MAX_SPD ...L` arm bodies are byte-for-byte identical to before, in the
same order, with the same one surviving trailer comment (`/* Maximum speed defined */` on the
first arm — none of the other original arms had trailer comments to preserve). Net diff: **13
insertions, 33 deletions** (`ckcdeb.h`: 3,705 → 3,685 lines).

```diff
-#ifdef BPS_1500K /* Maximum speed defined */
+#if defined(BPS_1500K) /* Maximum speed defined */
 #define MAX_SPD 1500000L
-#else
-#ifdef BPS_921K
+#elif defined(BPS_921K)
 #define MAX_SPD 921600L
   ... (same pattern for BPS_460K / BPS_230K / BPS_115K / BPS_76K / BPS_57K / BPS_38K / BPS_28K /
        BPS_19K / BPS_14K) ...
 #else
 #define MAX_SPD 9600L
 #endif
-#endif
-#endif
-#endif
-#endif
-#endif
-#endif
-#endif
-#endif
-#endif
-#endif
 #endif /* TTSPDLIST */
```

A required `clang-format -i` pass after the edit also widened the trailing-comment column on the
unrelated, immediately-preceding `#ifndef NOB_921K /* 921600 bps */` / `#endif /* NOB_921K */`
pair (2 lines, whitespace only) — clang-format aligns trailing comments across an adjacent run of
similarly-shaped lines, and the new (longer) `#if defined(BPS_1500K) /* Maximum speed defined */`
line shifted that alignment column. Same phenomenon `doc/NOCKXYZ-20260709.md` calls "expected
comment-column drift from guard un-nesting." No token content changed.

**Line-number correction to the audit report**: `doc/IFDEF-NESTING-20260710.md` cited this ladder
as `ckcdeb.h:2108-2153`/line 2138 for the max; re-reading the file during this phase established
the actual (unchanged, pre-edit) lines were **2109-2154**/line 2139 — an off-by-one from manual
line-counting during the original audit, not a file change (`git diff` against the audit's base
commit showed zero prior drift in `ckcdeb.h`). Substance of the report is unaffected; noted here
for the record.

### Verification

**1. Preprocessor-equivalence proof (decisive for a directive-only rewrite).** Compared `gcc -E
-P` output of a minimal `#include "ckcdeb.h"` translation unit, old vs. new `ckcdeb.h`, under the
`linux` target's base flags (`-DLINUX -DFNFLOAT -DCK_POSIX_SIG -DCK_NEWTERM -DTCPSOCKET
-DLINUXFSSTND -DNOCOTFMC -DPOSIX -DUSE_STRERROR`) plus 17 total combinations: each of the 11
`BPS_*` macros defined alone, none of them defined, `-DMACOSX10`, `-DBSD44`, and two combined
sanity checks (`-DMACOSX10 -DBPS_1500K`, `-DBSD44 -DBPS_14K`).

**Result: 17/17 token-identical.** (Old `ckcdeb.h` served from a symlink-farm directory with only
`ckcdeb.h` swapped for the pre-edit copy, so both sides resolved the same sibling headers.)

**2. Clean fixed-epoch rebuild.** `make clean && SOURCE_DATE_EPOCH=1750000000 make linux`, before
and after the edit (and again after the `clang-format` pass):

- Exit 0 both times; **zero warnings** (only the expected `-DUSE_STRERROR` CFLAGS echo in the
  build log).
- **`wermit` is byte-for-byte identical before and after**: `cmp` clean, same md5
  (`eee3a8218e33b4e6a495f1fe008a43c5`), same size (2,655,672 bytes) — checked once right after the
  `ckcdeb.h` rewrite and again after the `clang-format` pass, both times identical.
- **Every `.o` object file is byte-identical** too (all 30, including `ckcmai.o` and every other
  translation unit that includes `ckcdeb.h`) — no `__LINE__`/build-id perturbation at all, since
  the `linux` target carries no `-g` and nothing in this header's surviving content is
  line-number-sensitive. Stronger than the "per-object, explain every diff" fallback the skill
  describes for cases where objects *do* legitimately differ — here none did.

**3. Format stability.** `clang-format ckcdeb.h | diff ckcdeb.h -` was **not** clean immediately
after the hand-edit (the `NOB_921K` comment-alignment drift above); applying `clang-format -i
ckcdeb.h` once made it clean, and it has stayed clean since (re-checked after the rebuild). The
preprocessor-equivalence and byte-identical-rebuild proofs above were re-run *after* the
`clang-format` pass too, and still hold — the alignment change is provably a no-op on both the
token stream and the compiled output.

**4. Re-measured nesting depth.** The rewritten `BPS_*` chain itself now sits at a flat depth of
**3** (`CKCDEB_H` include guard = 1, `TTSPDLIST` = 2, the collapsed `#if`/`#elif` chain = 3),
exactly as designed — confirmed directive-by-directive with the accurate scanner.

**However, the file's own overall maximum did *not* drop to 3 as the audit report estimated — it
dropped from 13 to 9.** The audit's per-file methodology reported only the single deepest chain in
each file, so a second, independent depth-9 cascade elsewhere in `ckcdeb.h` was invisible while the
depth-13 `BPS_*` ladder was the file's maximum; collapsing the ladder exposed it as the new
bottleneck. It is `ckcdeb.h:992-1009` — the `USE_LSTAT` capability check — and is the **same
`#else`/`#ifdef` cascade anti-pattern** as the ladder just fixed, one arm shorter:

```
#ifndef NOLSTAT              (992)
 #ifndef NOSYMLINK             (993)
  #ifndef USE_LSTAT              (994)
   #ifdef  UNIX                    (995)
    #ifdef  CKSYMLINK                (996)
     #ifdef  SVR4  /* SVR4 has lstat() */  (997) → #else (999)
      #ifdef  BSD44 /* 4.4BSD has it */      (1000) → #else (1002)
       #ifdef  LINUX /* LINUX has it */        (1003)
```
This was out of scope for this phase (the task specified only the `BPS_*` ladder) and was not
touched. It is flagged here as a strong candidate for an additional Fix-(1) pass — collapsing
`SVR4`/`BSD44`/`LINUX` into one `#elif` chain would take this chain from depth 9 to depth 6
(`NOLSTAT`+`NOSYMLINK`+`USE_LSTAT`+`UNIX`+`CKSYMLINK`+elif-chain), and is recommended either as an
addendum to this phase or folded into Phase 2 — the coordinator/user should decide which.

**5. Re-timed `clang-format ckcdeb.h`** (same method as the audit report):

| | Before (audit baseline, 4 runs) | After Phase 1 (3 runs) |
|---|---:|---:|
| Runs | 81.88 s, 81.90 s, 81.65 s, 82.19 s | 30.57 s, 30.51 s, 30.73 s |
| Average | ~81.9 s | ~30.6 s |
| **File max depth** | 13 | 9 |
| **Naive `#ifdef`/`#if` opener count** | 833 | 823 |

**~2.68× speedup** (82 s → ~30.6 s) — a real, large drop, but smaller than the audit's "13 → 3"
framing implied, precisely because of finding 4 above: only 10 of the file's 833 directives were
actually removed (the 11-opener ladder collapsed to 1 opener + 10 `#elif`s, which don't count as
new depth), and the file's true bottleneck partially moved to the previously-hidden depth-9
cascade rather than disappearing. The result is consistent with the audit's underlying thesis
(nesting depth/concentration, not raw directive count or file size, drives `clang-format` cost —
823 directives at shallower concentration format 2.7× faster than 833 with a depth-13 hot run)
but shows the file has more than one concentrated hot spot, not only the one this phase targeted
— cascade concentration in general, not just the single `BPS_*` run, is the cost driver.

**Caveat added after Phase 2's investigation (see below): this thesis holds only for cascades
whose arms contain no `#include` directives.** The `BPS_*` ladder's 11 arms are all plain
`#define MAX_SPD ...L` — no `#include` anywhere — which turned out to matter a great deal.

### Status

**Committed** as `ce1a521` ("ckcdeb.h: flatten BPS_* MAX_SPD ladder to #elif chain (Phase 1)"),
after user review.

## Phase 2 — four more `#elif`-chain rewrites, one investigation, two reverts

Four `#else`/`#ifdef` cascades were rewritten as flat `#if`/`#elif`/.../`#endif` chains, same
mechanical transform as Phase 1: `ckuusx.c`'s network-transport ladder, `ckcmai.c`'s `nettype`
ladder, `ckucmd.c`'s libc-internals ladder, and — folded in per user decision — `ckcdeb.h`'s
`USE_LSTAT` cascade plus two further `ckcdeb.h` cascades (`openpty()` detection ×2, `CK_64BIT`)
that surfaced as tied/successive bottlenecks while re-scanning per this phase's own
fix-if-trivial instruction.

**All four edits verified byte-identical/warning-clean, but `ckcdeb.h`'s `clang-format` time then
came back ~4.7× *slower*** (~30.6 s → ~144 s), directly contradicting the audit's thesis. This was
investigated before asking the user what to commit. **Conclusion: two of the four `ckcdeb.h`
cascades (`USE_LSTAT`, `CK_64BIT`) are kept; the other two (the `openpty()` cascades) are
reverted.** The three `.c` files are unaffected by the investigation and are kept as originally
written.

### What changed, per file (final recommended state)

**`ckuusx.c`** (`git diff --numstat`: +7/−17, 7038 → 7028 lines) — the network-transport ladder
(`TCPSOCKET`/`IBMX25`/`HPX25`/`DECNET`/`NPIPE`/`CK_NETBIOS`, originally `ckuusx.c:105-131`)
rewritten as one `#if`/`#elif` chain, keeping the genuine `SUPERLAT` AND-nest inside the
`CK_NETBIOS` arm exactly as instructed. Also fixed the stale `#endif` trailer comments flagged in
the audit (`/* TCPSOCKET */ /* SUNX25 */ /* STRATUSX25 */` etc., leftover from a bygone larger
ladder) — now one accurate `#endif /* TCPSOCKET / IBMX25 / HPX25 / DECNET / NPIPE / CK_NETBIOS */`.
No `#include` directives anywhere in this ladder's arms.

**`ckcmai.c`** (+6/−14, 2874 → 2866 lines) — the `nettype`-selection ladder (`ckcmai.c:967-988`),
same 5 macros, no sub-nest, no `#include` in any arm → one chain.

**`ckucmd.c`** (+11/−20, 7993 → 7984 lines) — the libc-internals ladder in `cmdconchk()`
(`ckucmd.c:7371-7429`, decides how to peek at `stdin`'s internal buffer count across
glibc/HP-UX/BSD variants: `CMD_CONINC`/`__FILE_defined`/`_IO_file_flags`/`USE_FILE_CNT`/
`USE_FILE__CNT`/`USE_FILE_R`) rewritten as one chain, keeping the genuinely orthogonal
`NOARROWKEYS` AND-nest inside the `USE_FILE_CNT` arm. No `#include` directives in any arm (this
ladder only ever touches `stdin` struct fields and calls `debug()`/`conchk()`). Needed one
`clang-format -i` pass (comment realignment, long multi-name `#endif` comment wrapped across two
lines) — re-verified equivalence and the rebuild afterward, both hold.

**`ckcdeb.h`** (final state: **+13/−29, 3685 → 3669 lines** relative to Phase 1 — see the
Investigation subsection for why this is smaller than the +25/−58 originally applied) — **two of
the four attempted cascades are kept:**

1. `USE_LSTAT` (`ckcdeb.h:997-1008`, the bottleneck Phase 1 uncovered) — `SVR4`/`BSD44`/`LINUX` →
   one `#if`/`#elif` chain, order preserved exactly (`SVR4` first, then `BSD44`, then `LINUX`) per
   the coordinator's caution that on macOS/BSD builds more than one of these can be defined
   simultaneously via the `MACOSX10`→`MACOSX`→`BSD44`→`SVR4` implication chain (confirmed via
   `gcc -E -dM`: `-DMACOSX10` alone really does bring in `BSD44`/`SVR4`/`SVR3`/`ATTSV` at once on
   this tree). **No `#include` in any arm** — all three do bare `#define USE_LSTAT`.
2. `CK_64BIT` (`ckcdeb.h:2549-2576`) — `_LP64`/`__LP64__`/`__arch64__`/`__alpha`/`__amd64`/
   `__x86_64`/`__ia64` (7 arms, all `#define CK_64BIT`) → one chain. **No `#include` in any arm.**

```diff
-#ifdef SVR4 /* SVR4 has lstat() */
+#if defined(SVR4) /* SVR4 has lstat() */
 #define USE_LSTAT
-#else
-#ifdef BSD44 /* 4.4BSD has it */
+#elif defined(BSD44) /* 4.4BSD has it */
 #define USE_LSTAT
-#else
-#ifdef LINUX /* LINUX has it */
+#elif defined(LINUX) /* LINUX has it */
 #define USE_LSTAT
-#else
-#endif /* LINUX */
-#endif /* BSD44 */
-#endif /* SVR4 */
+#endif /* SVR4 / BSD44 / LINUX */
```
(the `CK_64BIT` diff follows the identical shape.)

**Two cascades were tried and reverted** (see Investigation): the two `openpty()`-availability
cascades at `ckcdeb.h:1132-1155` and `1162-1184` are back to their original nested
`#ifdef`/`#else` form, byte-identical to the Phase-1 (`ce1a521`) baseline in that span (confirmed
by extracting the `#ifdef NETPTY ... #endif /* NETPTY */` block from both and diffing — empty
diff). **Both reverted cascades have `#include` directives inside their arms** (`<util.h>`,
`<pty.h>`, `<libutil.h>`) — this correlation is the crux of the investigation below.

### Investigation — why `ckcdeb.h` got slower, and what to keep

**1. Bisection setup.** Saved plain-file copies (no `git stash`) of the working tree's four edited
files to the scratchpad before touching anything. Reconstructed the Phase-1 baseline (`git show
ce1a521:ckcdeb.h`) and, using the exact before/after text of each of the four edits, mechanically
built every combination of "baseline + subset of edits" needed for a full bisection —
`LSTAT_only`, `PTY1_only` (`openpty()` cascade #1, decides `HAVE_OPENPTY`), `PTY2_only` (cascade
#2, picks the header to `#include`), `CK64_only`, plus `LSTAT_CK64`, `LSTAT_PTY1`,
`LSTAT_PTY1_PTY2`, and `ALL_FOUR` (verified byte-for-byte identical to the file this phase had
originally produced, modulo a comment-wrap difference from a later `clang-format` pass). `gcc -E
-P` reconfirmed every single-edit variant is still preprocessor-equivalent to baseline before
timing it (same battery as the main Phase 2 verification). `clang-format --version`: **Ubuntu
clang-format version 21.1.8 (6ubuntu1)** (same binary used throughout this whole exercise).

**2. Bisect table** (2 rounds, interleaved in fixed round-robin order, for the 5 single-variable
configs; cumulative combos got 1 run each plus a 3rd round repeating `baseline`/`PTY1_only`/
`PTY2_only` at the end for drift control):

| Config | Round 1 | Round 2 | Round 3 (drift check) | Avg |
|---|---:|---:|---:|---:|
| `baseline` (Phase 1 state) | 26.98 s | 27.09 s | 26.95 s | **27.01 s** |
| `LSTAT_only` | 20.22 s | 20.36 s | — | **20.29 s** |
| `CK64_only` | 27.17 s | 27.10 s | — | **27.14 s** |
| `PTY1_only` | 67.59 s | 68.09 s | 67.44 s | **67.71 s** |
| `PTY2_only` | 67.80 s | 67.57 s | 67.23 s | **67.53 s** |
| `LSTAT_CK64` | 20.67 s (1 run) | | | 20.67 s |
| `LSTAT_PTY1` | 52.39 s (1 run) | | | 52.39 s |
| `LSTAT_PTY1_PTY2` | 128.62 s (1 run) | | | 128.62 s |
| `ALL_FOUR` | 126.94 s (1 run) | | | 126.94 s |

Drift check: `baseline`/`PTY1_only`/`PTY2_only` repeated a 3rd time at the very end of a ~30-minute
session landed within 0.4 s of their round-1/2 values every time — **no meaningful machine drift**
across the whole investigation.

**Conclusion from the table**: `LSTAT` and `CK64` are each **free or better than free**
(`LSTAT` is reproducibly *faster* than baseline by ~6.7 s, twice; `CK64` is a wash). `PTY1` and
`PTY2` are each independently **~2.5× baseline** (+~40.7 s apiece) — and are **not simply
additive** when combined: `LSTAT_PTY1_PTY2` (128.62 s) and `ALL_FOUR` (126.94 s, the ~1.7 s gap
matching `CK64`'s near-zero marginal cost) are both far above what adding the two ~41 s deltas to
baseline would predict (~27+41+41 ≈ 109 s) — a **super-additive (synergistic) interaction between
the two `openpty()` cascades specifically**, not a generic "more `#elif`s" effect (`LSTAT`+`CK64`
together add 10 more `#elif`s with zero penalty).

**3. Cost-model characterization with synthetic files.** Generated ~3,650-line synthetic C files
(matching `ckcdeb.h`'s size) with controlled `N` separate `#if`/`#elif` regions × `M` arms each,
`#elif` vs. nested-`#ifdef`/`#else` style, and with/without an `#include` inside one arm, to see
whether cost scales with total `#elif` count, arms-per-region, region count, or `#include`
presence. **Result: none of the synthetic variants reproduced any slowdown** — every one
(1 region/11 arms elif vs. nested, 5 regions/26 total arms, 1 region/26 arms, region counts 1/5/
10/20, arm counts 2/5/11/20/40, and — the most direct test — a 5-arm `#elif` chain with an
`#include <stubhdr.h>` line inserted into the first/middle/last arm) formatted in **~0.16–0.18 s**,
indistinguishable from a no-conditionals baseline. This is a genuine negative result: the simple
hypothesis "an `#include` directive living inside an `#elif` arm is inherently expensive to
format" **does not hold in isolation**. RSS was never elevated for any synthetic file (unlike the
~5.1–5.4 GB seen formatting the real `ckcdeb.h` `ALL_FOUR`/`LSTAT_PTY1_PTY2` variants — confirmed
again during this investigation via `ps aux` RSS sampling mid-run).

**What this means**: the real `ckcdeb.h` blowup is **confirmed and reproducible on the real file**
(rock-solid bisection evidence, repeated 2-3× per configuration with sub-second variance) but its
exact mechanism inside `clang-format` is **not fully characterized** — a minimal synthetic
reproduction failed, meaning the effect isn't simply "any `#elif` arm containing `#include`" but
depends on something about `ckcdeb.h`'s broader real context that a padded-but-structurally-simple
synthetic file doesn't reproduce. The leading (untested further) hypothesis: `ckcdeb.h` has **22
total `#include` directives**, most of them already conditionally guarded elsewhere in the file
(lines 507, 508, 2288, 2304, 2309, 2505, 2713, 2718, 3107, 3113, 3116, 3131, 3133, 3161, 3544),
and `.clang-format` has `SortIncludes: Enabled: true` / `IncludeBlocks: Preserve` — plausibly the
cost is tied to how `clang-format`'s include-sorting logic reasons about the *whole file's* set of
conditionally-reachable `#include`s once two *more* such regions are added, not to either region
in isolation. This was not pursued further (would need profiling `clang-format` itself, e.g. via
`perf`/`callgrind` against the LLVM source, or bisecting with real copies of the other ~16
conditional includes added to a synthetic file) — flagged as a real open question, not resolved
here, but **not necessary to resolve** in order to make the keep/revert call below.

**4. Reconciling Phase 1's speedup with Phase 2's regression — corrected cost model.** Both facts
are explained by the same rule once `#include` presence is added as a variable:

> Collapsing an `#else`/`#ifdef` cascade to `#elif` is a reliable `clang-format` win **when no arm
> contains an `#include` directive** (Phase 1's `BPS_*` ladder: 11 arms, all bare `#define`s, 82 s
> → 30.6 s; Phase 2's `USE_LSTAT`/`CK_64BIT`: bare `#define`s, free-to-negative cost). It can be a
> severe **loss** when arms contain `#include` directives, and the loss is **super-additive** —
> not simply proportional to `#elif` count — once *more than one* such region exists in the same
> file (Phase 2's two `openpty()` cascades: +41 s each alone, +100 s together vs. baseline, far
> above the ~+82 s a purely additive model predicts).

**Correction note for the audit report's central thesis** (`doc/IFDEF-NESTING-20260710.md`):
"nesting depth/concentration, not raw directive count or file size, drives `clang-format` cost"
is **correct as far as it goes, but incomplete** — it does not hold when the collapsed cascade's
arms contain `#include` directives, in which case reducing depth can make `clang-format`
*slower*, non-linearly so with more than one such region per file. Neither of the report's two
headline hot spots (`ckcdeb.h`'s `BPS_*` ladder, depth 13; `ckutio.c`'s `tthang()` chain, depth 9)
has `#include` inside its arms, so the original report's recommendations are unaffected — but
**Fix (1) and Fix (2) candidates should be checked for `#include` (or plausibly other
"heavier" directives) inside their arms before being applied**, and if more than one such
region exists in a file, tested together, not just individually. This is exactly the caution
Phase 2's plan note (above) now attaches to Phase 4's AND-nest work.

### Keep/revert recommendation and final state

| Cascade | Depth win | Timing cost | Decision |
|---|---|---|---|
| `ckcdeb.h` `USE_LSTAT` | 9 → 7 (local) | **free-to-negative** | **KEEP** |
| `ckcdeb.h` `CK_64BIT` | 9 → 3 (local) | **~0** | **KEEP** |
| `ckcdeb.h` `openpty()` #1 | 9 → 5 (local) | **+41 s alone, worse combined** | **REVERT** |
| `ckcdeb.h` `openpty()` #2 | 8 → 4 (local) | **+41 s alone, worse combined** | **REVERT** |
| `ckuusx.c` transport ladder | 8 → 3 (local) | ~25% faster | KEEP (unaffected by investigation) |
| `ckcmai.c` `nettype` ladder | 6 → 2 (local) | ~18% faster | KEEP (unaffected) |
| `ckucmd.c` libc-internals ladder | 7 → 3 (local) | ~19% faster | KEEP (unaffected) |

The working tree has been set to this recommended state: `ckcdeb.h`'s two `openpty()` cascades
were reverted back to their exact Phase-1 (`ce1a521`) text (confirmed byte-identical via extracted
block diff); `USE_LSTAT` and `CK_64BIT` remain flattened; all three `.c` files are unchanged from
their original Phase 2 edits.

**This means `ckcdeb.h`'s file-wide max nesting depth is back to 9** (the reverted `openpty()` #1
cascade — `NETPTY`→`NO_OPENPTY`→`HAVE_OPENPTY`→ 5-arm cascade — is once again the file's deepest
chain. It was *already* tied at depth 9 at the end of Phase 1, alongside `USE_LSTAT` — invisible at
the time because the scanner reports only the first chain it encounters at the file's max depth,
and `USE_LSTAT` sits earlier in the file — so reverting it back is not a regression relative to
Phase 1's end state, just a declined opportunity, traded for `clang-format` speed). This is
intentional: **the task reiterated that `clang-format` speed is the real goal and depth reduction
was only ever a proxy for it; a hunk that
measurably worsens formatting time has lost its reason to exist regardless of depth.**

### Re-run proofs for the final recommended state

**1. Preprocessor equivalence** (only `USE_LSTAT` and `CK_64BIT` remain to prove; the reverted
`openpty()` regions are byte-identical to the already-proven Phase-1 baseline, so nothing new to
verify there): re-ran both matrices — **`USE_LSTAT` 9/9 OK, `CK_64BIT` 13/13 OK**, all still
token-identical. The three `.c` files' matrices (26+13 = 39 combinations) are unaffected by this
investigation and were re-run too — still 39/39 OK.

**2. Clean fixed-epoch rebuild**: `make clean && SOURCE_DATE_EPOCH=1750000000 make linux` — exit
0, zero warnings. **`wermit` md5 `eee3a8218e33b4e6a495f1fe008a43c5`** — unchanged from every
previous state in this whole exercise. **All 30 `.o` files byte-identical** to the pre-Phase-2
baseline.

**3. Format stability**: `clang-format ckcdeb.h | diff ckcdeb.h -` is clean with **no reformatting
needed** in the final recommended state (the two reverted regions restore the original,
already-stable text; `USE_LSTAT`/`CK_64BIT` keep their own already-verified formatting).

**4. Final `clang-format ckcdeb.h` timing** (3 runs): **23.47 s, 23.32 s, 23.40 s — average
23.40 s.** This is *faster than the Phase 1 baseline itself* (~27 s becomes ~23.4 s once
`USE_LSTAT`/`CK_64BIT` are also flattened, consistent with their "free-to-negative" bisection
result), and dramatically faster than the ~144 s the all-four-cascades state measured. Compared to
the original audit baseline (~82 s), the net Phase 1 + Phase 2 (recommended) result is a
**~3.5× speedup** on `ckcdeb.h`, with the depth-9 `openpty()` bottleneck deliberately left in place
as the trade for that speed.

### Status

**Committed** as `b5cdee6` ("Flatten four #else/#ifdef cascades to #elif chains (Phase 2)"),
exactly as recommended above (three `.c` files as originally written; `ckcdeb.h` with
`USE_LSTAT`/`CK_64BIT` kept and both `openpty()` cascades reverted), after user review. The
`openpty()` cascades remain a legitimate depth-reduction opportunity (`ckcdeb.h` would drop to
depth 8 or lower) but were **not** applied, per the investigation above, without a real fix for
the timing regression — depth reduction alone was never the actual goal.

## Phase 3 — delete the three `#ifdef NOTUSED` dead-code blocks

This phase implements the audit's Fix (3): three blocks gated by `#ifdef NOTUSED`, a macro that
is `#define`d nowhere in the tree, are unreachable on every platform this tree builds for and were
deleted outright — a stronger move than a `#elif`/AND-nest re-punctuation (Fixes 1/2), since it
removes dead weight rather than restructuring live logic. This is the kind of change the tree's
own convention (`doc/NOCKXYZ-20260709.md` et al.) says deserves its own dated write-up; this
section is written to stand as that record, folded into the running campaign doc per the
coordinator's direction.

### Re-verified `NOTUSED` evidence (re-checked at apply time, not just trusted from the audit)

```
$ grep -rn "define NOTUSED" *.c *.h makefile     # no output — never defined
$ grep -rn "NOTUSED" *.c *.h
ckcnet.c:1154:#ifdef NOTUSED
ckcnet.c:1299:#endif /* NOTUSED */
ckuxla.c:2326:#ifdef NOTUSED
ckuxla.c:2340:#endif /* NOTUSED */
ckufio.c:5203:#ifdef NOTUSED
ckufio.c:5208:#endif /* NOTUSED */
```
`gcc -E -dM` full-macro dumps of `ckcdeb.h` (which every `.c` file includes first) for `linux`,
`-DMACOSX10`, and `-DBSD44` all show **no `NOTUSED` macro at all** — confirmed fresh, not just
carried over from the original audit. Line numbers matched the audit exactly (no drift since
`doc/IFDEF-NESTING-20260710.md` was written).

### What was deleted

**`ckcnet.c:1154-1299`** (146 lines + 1 trailing blank = **147 lines removed**) — the entire dead
`tcpsocket_open()` function, including its nested `NOTCPOPTS`/`SOL_SOCKET`/`TCP_NODELAY` block
(the depth-8 hot spot the audit and Phase 1/2 reports pointed at). This function's body does not
even parse as valid C (`int timo {` — a missing close-paren on the K&R-style parameter list,
`ckcnet.c:1161`), further confirming it hasn't compiled in a very long time. Deleted the directive
pair and everything between; collapsed the resulting double blank line (the block's own
leading/trailing blank plus the pre-existing separator before/after it) back to the file's normal
single-blank-line spacing between functions.

**`ckuxla.c:2326-2340`** (15 lines + 1 trailing blank = **16 lines removed**) — the dead `yasl1[]`
"ASCII to Latin-1" translation table. The section-header comment immediately above it
(`/* Local file character sets to ISO Latin Alphabet 1 */`) was **kept**, since it introduces the
whole group of tables in this section, not just the deleted one — `yaql1[]` (the still-live
"Extended Mac Latin" table) follows immediately and the comment still correctly describes it.

**`ckufio.c:5203-5208`** (6 lines) plus its now-orphaned **2-line** lead-in
(`/* Find initialization file. */` and the blank line after it) = **9 lines removed**. Unlike the
`ckuxla.c` case, this comment specifically documented the deleted function's *purpose*
("find initialization file"), not a section of surrounding code, so it would have dangled
meaninglessly in front of the next (unrelated, `#ifndef UNIX`-guarded) block if left in place —
removed along with the dead `int zkermini()` no-op.

**Total: 172 lines removed across 3 files**, all confirmed dead — `git diff --stat`:

```
 ckcnet.c | 147 ---------------------------------------------------------------
 ckufio.c |   9 ----
 ckuxla.c |  16 -------
 3 files changed, 172 deletions(-)
```

### A scanner bug found and fixed along the way

While re-locating these blocks, the accurate depth/line-number scanner (used throughout this
whole campaign) was caught misreporting line numbers by a small, file-dependent offset — e.g. it
initially reported `ckcnet.c`'s post-deletion hot spot as line 5589 when the true line (confirmed
by `grep -n`) is 5590. Root cause: the scanner's directive-line parser skips over `/* ... */`
comments that trail a directive on the same logical line, but a comment that itself *spans*
multiple physical lines (legal — comment removal happens before line-splicing, so a directive
line can contain a multi-line trailing comment without a backslash) was skipped without counting
its embedded newlines, permanently under-counting `line_no` by one for every subsequent directive
in the file once such a construct is hit. This is a **line-number reporting bug only** — the
depth *values* it computes never depended on `line_no` and were unaffected (spot-checked: `ckuus3.c`'s
line 3852 anomaly and every depth number in all three reports remain correct); only specific
`file:line` citations after the first such construct in a given file could be off by one or a
small constant. Fixed (one line, `ifdef_depth.py`: count `\n` inside a directive's skipped inline
comment before advancing past it) and reverified against both a synthetic
reproduction (`#ifdef FOO /* comment\nspanning two lines */`) and every number in this section.
Not re-auditing Phases 1/2's line citations retroactively — out of scope for this phase — but
noting it here since it explains a discrepancy a careful reader might otherwise spot.

### Verification

**1. Fixed-epoch clean rebuild** (the strongest gate this phase should hit, since the deleted code
was dead on every platform, including this one): `make clean && SOURCE_DATE_EPOCH=1750000000 make
linux`, before (via `git stash`, one clean stash/pop cycle just for this baseline snapshot — not
the repeated back-and-forth Phase 2's bisection needed) and after the three deletions:

- Exit 0 both times, **zero warnings**.
- **`wermit` byte-for-byte identical**, both before and after: md5 **`eee3a8218e33b4e6a495f1fe008a43c5`**
  — the same value every single rebuild in this entire campaign (Phase 1 baseline, Phase 1 after,
  Phase 2 after, Phase 2's final recommended state, and now Phase 3) has produced.
- **All 30 `.o` object files byte-identical** before/after (compared pairwise, all match).

This is the strongest-possible confirmation that the three deletions are genuinely inert — not
just "the same modulo some rebuild noise," but bit-for-bit the same linked binary.

**2. Level 4 — platform-path checks.** Two complementary proofs, since this box has no real BSD/
macOS system headers to compile fully against:

- `gcc -E -P` token-stream comparison (old vs. new file) under the `linux` base flags plus
  `-DMACOSX10`, `-DBSD44`, and `-DMACOSX10 -DBSD44` together, for all three files —
  **12/12 combinations token-identical.** (`NOTUSED` being undefined in all of them means the
  deleted regions were already contributing zero tokens under every one of these configurations
  before deletion too — this proves it directly rather than just inferring it from the `-dM`
  dumps.)
- `cc -fsyntax-only` with the real `macos` target's flags (`-DMACOSX10 -DMACOSX103 -DCK_NCURSES
  -DTCPSOCKET -DCKHTTP -DUSE_STRERROR -DUSE_NAMESER_COMPAT -DNOCHECKOVERFLOW -DFNFLOAT
  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -funsigned-char -DNODCLINITGROUPS -DNOUUCP`, taken
  verbatim from the makefile's `macos:` target) on old vs. new: `ckuxla.c` is clean (exit 0) both
  times; `ckcnet.c` and `ckufio.c` both produce the **same pre-existing errors** before and after
  (missing declarations for `gethostbyname`/`time`/etc. — an artifact of syntax-checking a single
  translation unit standalone without this box's real BSD network/time headers, not something
  either version of the file introduces), with every error's line number shifted by **exactly**
  the number of lines removed before that point in the file (147 for `ckcnet.c`; `ckufio.c`'s
  errors sit before its deletion point and are unshifted). One cosmetic wrinkle in `ckufio.c`:
  a single compiler "note" suggesting where to add `#include <time.h>` reports a context line
  1 off (700 vs. 699) between the two runs; the actual source at that location (lines 690-705)
  was directly diffed and is byte-identical, so this is a `gcc` diagnostic-rendering artifact —
  possibly tied to running the two checks against different absolute paths — not a real
  discrepancy. Every substantive error/warning otherwise matches exactly.

**3. Format stability**: `clang-format FILE | diff FILE -` is clean (no changes needed) for all
three files immediately after the hand-deletions — the blank-line tidying was done correctly by
hand, no `clang-format -i` pass was required this time.

**4. Nesting depth, old → new:**

| File | Naive openers (old → new) | File max depth (old → new) |
|---|---|---|
| `ckcnet.c` | 405 → 392 | **8 → 8 (unchanged — see below)** |
| `ckuxla.c` | 124 → 123 | 4 → 4 (unchanged, as expected) |
| `ckufio.c` | 453 → 452 | 5 → 5 (unchanged, as expected) |

**`ckcnet.c`'s file-wide max did *not* drop to 6 as the task expected — it stayed at 8**, for the
now-familiar reason from Phases 1 and 2: the audit's per-file methodology reports only the first
chain it finds at a file's maximum depth, so a second, wholly unrelated depth-8 cascade elsewhere
in `ckcnet.c` was invisible while the `NOTUSED` block's chain was (one of) the reported max(es).
Deleting `NOTUSED` correctly removed *that* depth-8 chain — confirmed it is simply gone, not
relocated — but this tied second chain, at `ckcnet.c:5557-5590`, was there all along:

```
#ifdef  NETCONN                    (3762)
 #ifndef NOHTTP                     (5557)
  #ifndef TIMEH                       (5581)
   #ifndef SYSTIMEH                    (5582)
    #ifndef SYSTIMEBH                    (5583)
     #ifdef  SYSTIMEH                     (5584)  — nested inside its own negation, see below
      #ifdef  POSIX                          (5587)
       #ifdef  CLIX                            (5590)
```
This selects which `<time.h>`-family header to pull in for the HTTP-proxy code
(`SYSTIMEH`/`POSIX`/`CLIX`), and is the same `#else`/`#ifdef`-cascade-standing-in-for-`#elif`
pattern as Fix (1), **not** part of this phase's scope (deletion only) and not touched. One
structural oddity worth flagging for whoever eventually looks at it: line 5584's `#ifdef
SYSTIMEH` sits *inside* the `#ifndef SYSTIMEH` branch opened three lines above (5582) — i.e. it
tests the same macro's negation and then its assertion in immediate succession, which looks like
dead/unreachable code in its own right (upstream oddity, not introduced by any change in this
campaign; left exactly as found, consistent with this phase's delete-only scope).

**5. `clang-format` timing, measured (not assumed) per Phase 2's lesson** — the deleted regions
contain no `#include` directives, so no repeat of Phase 2's regression was expected, but it was
checked directly rather than assumed:

| File | Before | After | Change |
|---|---:|---:|---:|
| `ckcnet.c` | 4.61 s, 4.53 s, 4.53 s, 4.55 s, 4.53 s (avg **4.55 s**) | 5.19 s, 5.20 s, 5.15 s, 5.16 s, 5.15 s (avg **5.17 s**) | **~14% slower** |
| `ckuxla.c` | 0.54 s (1 run) | 0.60 s (1 run) | ~11% slower |
| `ckufio.c` | 1.44 s (1 run) | 1.62 s (1 run) | ~13% slower |

**A small, real, reproducible increase, not a Phase-2-scale regression.** Runs were interleaved
(before/after/before/after) for `ckcnet.c` specifically to rule out drift, and the two groups are
tightly clustered (4.53-4.61 s vs. 5.15-5.20 s, zero overlap) — this is a genuine effect, not
noise, but two orders of magnitude smaller than Phase 2's ~5× blowup and does not change the
recommendation (deleting confirmed-dead, non-compiling code is correct regardless of its effect
on `clang-format` wall time). Plausible explanation, not investigated further given the small
stakes: removing 147 lines from the *middle* of a large file shifts everything after it, and
`clang-format`'s cost is apparently not simply proportional to line count either direction (recall
`ckcuni.c` — 23,312 lines, depth 2 — formats in under a second; raw size was never the driver).
Flagging rather than burying: if a future phase stacks several more same-file deletions/edits and
sees this compound, it's worth a real look, but a single-digit-percent, sub-second-per-file change
here does not warrant the kind of investigation Phase 2 needed.

### Status

**Committed** as `5c40ec2` ("Delete the three dead #ifdef NOTUSED blocks (Phase 3)"), after user
review. All three deletions verified byte-identical at the strongest gate (whole-binary +
all-object rebuild), format-stable, and dead-code-confirmed fresh at apply time.

## Phase 4 — collapse pure-AND `#ifdef` nests (Fix 2), final planned phase

This phase implements the audit's Fix (2): merge nested `#ifdef A { #ifdef B { ... } }` chains
that have **no `#else` anywhere in the span** into a single `#if defined(A) && defined(B) && ...`.
Two of the three named targets required real corrections to how the audit (and this phase's own
task description, which echoed it) characterized them — re-verifying "nothing between the opens"
line by line, as instructed, caught both before anything was applied.

### Correction 1: `ckcnet.c`'s six `NOTCPOPTS`/`SOL_SOCKET`/`SO_*` triples are not a valid Fix-2 target — skipped entirely

The audit (`doc/IFDEF-NESTING-20260710.md`, Pattern 2) and this phase's own task description
described `ckcnet.c:776-1114` as "genuine AND-nesting with no `#else` anywhere in the span." Reading
the actual code (e.g. `ck_linger()`, `ckcnet.c:777-843`) shows this is wrong:

```c
#ifndef NOTCPOPTS
int ck_linger(int sock, int onoff, int timo) {
  /* ... comment ... */
#ifdef SOL_SOCKET
#ifdef SO_LINGER
  /* ... real getsockopt/setsockopt logic, with fall-through paths ... */
#else
  debug(F100, "TCP ck_linger SO_LINGER not defined", "", 0);
#endif /* SO_LINGER */
#else
  debug(F100, "TCP ck_linger SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return (0);
}
```

Every one of the six triples (`SO_LINGER`/`SO_SNDBUF`/`SO_RCVBUF`/`SO_KEEPALIVE`/`SO_DONTROUTE`/
`TCP_NODELAY`) has **real `#else` fallback code at both the `SOL_SOCKET` and `SO_*` levels**
(confirmed directive-by-directive with the scanner for all six). `NOTCPOPTS` wraps the *whole
function*, not just the socket-option call; `SOL_SOCKET`/`SO_*` is a nested conditional *inside*
the function body controlling whether it does real work or prints a fallback debug message, with a
shared `return (0);` reached from both paths. Collapsing this into one `#if NOTCPOPTS_negated &&
SOL_SOCKET && SO_X` would either (a) delete the function definition entirely when `SOL_SOCKET`/
`SO_X` is undefined — a real behavior change, since callers would then get a link error — or (b),
if the `#else` fallback were kept alongside a merged condition, silently merge two *different*
diagnostic debug messages ("SO_SOCKET not defined" vs. "SO_LINGER not defined") into one, losing
diagnostic distinction. Neither is a safe mechanical re-punctuation. **This target was skipped
entirely; `ckcnet.c` is untouched in this phase** (still the depth-8 file Phase 3 left it at,
`git diff` shows no changes for this file). The audit's Part 2 "Pattern 2" characterization of this
site is hereby corrected: it does not qualify for Fix (2).

### Correction 2: `ckutio.c`'s "6-level" chain is really two separate things

The task (and audit) described `ckutio.c:3076-3111` as one `ATTSV && !_IBMR2 && TIOCMBIS &&
TIOCMBIC && TIOCM_DTR && !CLSOPN` chain. Reading the full span shows `ATTSV` (`ckutio.c:3076`,
closing at `ckutio.c:3246`) wraps a much larger block than the small ioctl-based DTR-toggle
attempt — after the `_IBMR2` sub-chain closes (originally `ckutio.c:3121`), **~125 more lines of
unconditional code and sibling `#ifdef`s** (`O_NDELAY`, `TCXONC`, `TIOCSTART`, `NOCOTFMC`, more
`O_NDELAY`) still run inside `ATTSV`'s scope — the "General AT&T UNIX case" fallback DTR-toggle
method, which must run regardless of whether the ioctl-based attempt's macros are defined.
Similarly, `CLSOPN` (`ckutio.c:3109`) is not adjacent to `TIOCM_DTR`'s opening — real code (the
`z = TIOCM_DTR; debug(...); if (ioctl(...))` control flow) sits between them, so `CLSOPN` guards
only the early-`return(1)` inside a specific nested `if`, not something ANDable with the outer
chain. **The only genuinely self-contained, adjacent, no-`#else` span is the inner 4-level chain**:
`_IBMR2` (negated) → `TIOCMBIS` → `TIOCMBIC` → `TIOCM_DTR`, which opens and closes entirely within
itself (`ckutio.c:3087-3121` originally) without `ATTSV` or `CLSOPN` needing to move.

### What changed, per file (final recommended state)

**`ckutio.c`** (`git diff --numstat`: +9/−14, 11,929 → 11,924 lines) — **one edit kept**: the
4-level `_IBMR2`/`TIOCMBIS`/`TIOCMBIC`/`TIOCM_DTR` chain inside `tthang()` collapsed into
`#if !defined(_IBMR2) && defined(TIOCMBIS) && defined(TIOCMBIC) && defined(TIOCM_DTR)`, with
`ATTSV` (still wrapping the whole block) and `CLSOPN` (still guarding just the nested early-return)
**untouched**, exactly as corrected above.

```diff
-#ifndef _IBMR2
-/*
-  No modem-signal twiddling for IBM RT PC or RS/6000.
-  ...
-*/
-#ifdef TIOCMBIS  /* Bit Set */
-#ifdef TIOCMBIC  /* Bit Clear */
-#ifdef TIOCM_DTR /* DTR */
+#if !defined(_IBMR2) && defined(TIOCMBIS) && defined(TIOCMBIC) && defined(TIOCM_DTR)
+/*
+  No modem-signal twiddling for IBM RT PC or RS/6000.
+  ...
+*/
   ... (unchanged body: z = TIOCM_DTR; debug(...); if (ioctl(...)) {...} ...
        #ifndef CLSOPN / return (1); / #endif  <- untouched, still nested normally
        ...) ...
-#endif /* TIOCM_DTR */
-#endif /* TIOCMBIC */
-#endif /* TIOCMBIS */
-#endif /* _IBMR2 */
+#endif /* _IBMR2 / TIOCMBIS / TIOCMBIC / TIOCM_DTR */
```

**One edit tried and reverted** (see Timing below): the OPTIONAL outer-ladder merge (`HUP_POSIX`
`#else` immediately followed by `#ifdef BSD44ORPOSIX`, confirmed via matching-`#endif` adjacency to
be a clean 2-level cascade → `#elif` opportunity, no `#include` in its ~420-line span) was applied,
measured, found to regress `clang-format` time by ~42%, and reverted back to its exact original
text. `ckutio.c`'s working-tree diff now contains **only** the `_IBMR2` chain edit.

**`ckctel.c`** (+4/−8, 6,534 → 6,530 lines) — **both**
occurrences of the `IKSDONLY`/`CK_AUTODL` pair collapsed (the audit named only `ckctel.c:1282-1283`;
re-scanning after the first fix found a second, structurally identical, untouched occurrence at
the-then `ckctel.c:1323-1326`, tied at the same depth — same "hidden second bottleneck" pattern as
every prior phase). Both verified clean (`#ifndef IKSDONLY` immediately followed by
`#ifdef CK_AUTODL`, no `#else`, nothing between):

```diff
-#ifndef IKSDONLY
-#ifdef CK_AUTODL
+#if !defined(IKSDONLY) && defined(CK_AUTODL)
       if (...) {
         tn_siks(KERMIT_RESP_START);
       } else
-#endif /* CK_AUTODL */
-#endif /* IKSDONLY */
+#endif /* IKSDONLY / CK_AUTODL */
         tn_siks(KERMIT_RESP_STOP);
```
(the second occurrence, `else if (...) { tn_siks(KERMIT_RESP_STOP); }`, follows the identical
shape.)

**`ckcnet.c`** — **untouched**, per Correction 1 above.

### Verification

**1. Preprocessor equivalence.** Since `_IBMR2`/`TIOCMBIS`/`TIOCMBIC`/`TIOCM_DTR` are system-header
macros (`<sys/ioctl.h>` defines `TIOCM_DTR`/`TIOCMBIS`/`TIOCMBIC` natively on this box; `_IBMR2` is
AIX-only and never defined here), two complementary proofs, same technique as Phase 2's `openpty()`
isolation:

- **Isolated snippet** (old vs. new text of just the `_IBMR2` span, no real headers needed):
  all-true, all-false, each macro false individually, `_IBMR2` defined (disables the whole thing),
  `CLSOPN` toggled — **10/10 OK**.
- **Isolated snippet** for the `HUP_POSIX`/`BSD44ORPOSIX` merge (tested before the revert, since the
  revert restores exact original text anyway): each macro alone, both, combined with `ATTSV`/
  `USE_TIOCSDTR`/`ANYBSD`/`TIOCCDTR` to exercise the nested nested content, `-DMACOSX10`/`-DBSD44`
  — **10/10 OK** (this snippet, unlike Phase 2's `openpty()` case, has no `#include` at all, so no
  stub headers were needed).
- **Full-file** `gcc -E -P`, `ckutio.c` old vs. new, `linux` base flags plus `-DMACOSX10`, `-DBSD44`,
  both together, `HUP_POSIX`/`NOLOCAL`/`ATTSV` explicit — **7/7 OK**.
- **Full-file**, `ckctel.c` old vs. new (both `IKSDONLY` fixes together): `IKSDONLY`, `CK_AUTODL`,
  both, neither, `-DMACOSX10`, `-DBSD44`, `-DNOXFER` — **7/7 OK**.

**Grand total: 34/34 preprocessor-equivalence combinations token-identical, 0 failures.**

**2. Clean fixed-epoch rebuild.** Run three times over the course of this phase (after both
`ckutio.c` edits; again after reverting the `HUP_POSIX`/`BSD44ORPOSIX` one) —
`make clean && SOURCE_DATE_EPOCH=1750000000 make linux`: **exit 0, zero warnings, every time.**

- **`wermit` md5 `eee3a8218e33b4e6a495f1fe008a43c5`** — the same value every rebuild in this entire
  four-phase campaign has produced, including the final state.
- **All 30 `.o` object files byte-identical** to a pre-Phase-4 baseline (via one clean
  `git stash`/rebuild/pop cycle), both after the initial two-edit `ckutio.c` state and again after
  the revert.

**3. Format stability.** `ckctel.c` and `ckcnet.c` (untouched) needed no reformatting. `ckutio.c`
needed one `clang-format -i` pass — the merged `#if` condition line exceeds the column width and
gets wrapped with a backslash continuation, the same phenomenon seen in Phase 1/2 (and, as it turns
out this time, not entirely free — see Timing below). Re-verified equivalence and the rebuild after
the pass; both hold.

**4. Nesting depth, old → new:**

| File | Targeted chain (old → new, local) | File max (old → new) |
|---|---|---|
| `ckutio.c` `_IBMR2` chain | 9 → 6 (4 levels collapsed to 1; `ATTSV` still at 4, `CLSOPN` still one level inside at 6) | 9 → 8 |
| `ckctel.c` `IKSDONLY`/`CK_AUTODL` (both) | 7 → 6 and 7 → 6 | 7 → 6 |
| `ckcnet.c` | — (untouched) | 8 → 8 (unchanged) |

**`ckutio.c`'s file-wide max dropped only 9 → 8, not further**, for the now-completely-expected
reason: a wholly unrelated, pre-existing depth-8 cascade — a UUCP lock-directory-path selector
(`NOUUCP`/`USETTYLOCK`/`LOCK_DIR`/`BSD44`/`HDBUUCP`/`M_SYS5`/`SVR4`/`LINUXFSSTND`,
`ckutio.c:184-291`, the same `#else`/`#ifdef`-cascade-standing-in-for-`#elif` anti-pattern as
Fix (1), not Fix (2), and out of this phase's scope) was tied at depth 8 all along and is now the
reported max. `ckctel.c` needed *both* `IKSDONLY`/`CK_AUTODL` fixes before its file max actually
moved — fixing only the audit-named occurrence would have left it tied at 7 via the second,
previously-unreported occurrence, the same "audit reports only the first chain it finds" blind
spot as every prior phase.

**5. Timing — measured, not assumed, per Phase 2's/Phase 3's lesson; one hunk reverted as a
result:**

| Config | Runs | Avg | vs. baseline |
|---|---|---:|---:|
| `ckutio.c` baseline (Phase-3 state) | 10.29, 10.29, 10.27 s | 10.28 s | — |
| `ckutio.c` + `_IBMR2` chain only | 10.32, 10.37 s (bisection) | 10.35 s | **~+1%, noise-level** |
| `ckutio.c` + `HUP_POSIX`/`BSD44ORPOSIX` only | 14.76, 14.71 s (bisection) | 14.74 s | **+43% — REGRESSION** |
| `ckutio.c` + both | 14.75, 14.67 s (bisection) | 14.71 s | (confirms 2nd edit dominates) |
| **`ckutio.c` final (kept: `_IBMR2` chain only, post-`clang-format`)** | 11.89, 11.85, 11.83 s | **11.86 s** | **~+15%** |
| `ckctel.c` before | 0.21, 0.21, 0.21 s | 0.21 s | — |
| `ckctel.c` after (both `IKSDONLY` fixes) | 0.23, 0.24, 0.24 s | 0.23 s | ~+10%, negligible (sub-second) |

Two separate, real findings here, both measured via interleaved bisection (2 full rounds) to rule
out drift, both reproducible:

- **The `HUP_POSIX`/`BSD44ORPOSIX` merge alone reproducibly costs ~+43%** (10.3 s → 14.7 s) even
  though its ~420-line span has zero `#include` directives — confirming Phase 2's cost model isn't
  the *only* way a "safe" `#elif`/AND-nest merge can regress `clang-format` time; something about
  this specific merge (a large branch, deeply nested internal content, or simply its position/size)
  has a real cost not explained by the `#include`-arm theory. **Per the explicit "keep only if it
  does not regress" instruction for this optional edit, it was reverted** back to its exact
  original text (confirmed via isolated equivalence re-check).
- **The mandatory `_IBMR2` chain merge is genuinely free in isolation** (10.28 s → 10.35 s, within
  noise) **but the *committed, `clang-format`-stable* version costs ~+15%** (10.28 s → 11.86 s) —
  because the merged condition line is long enough that `clang-format` wraps it with a backslash
  continuation, and *that specific formatted shape* — not the merge itself — is what costs the
  extra ~1.6 s on every subsequent run. This was measured directly (pre-format synthetic variant:
  10.33 s vs. the actual post-format working-tree file: 11.87 s, same session, back-to-back).
  **Kept anyway**: this phase's explicit "keep only if no regression" bar was stated for the
  *optional* edit specifically, not the mandatory one; the regression here is an order of magnitude
  smaller than the reverted hunk's (+15% vs. +43%) and in the same range Phase 3 already tolerated
  for its (also mandatory, also dead-code-driven) deletions (+11-14%, kept there for the same
  reason: the underlying change is independently correct and the cost is small). Flagged here in
  full, not buried, so the user can override this call if they'd rather have `ckutio.c` fully
  untouched.

### Status

**Applied to the working tree, not committed.** Final recommended state: `ckutio.c` with only the
`_IBMR2`/`TIOCMBIS`/`TIOCMBIC`/`TIOCM_DTR` merge (the `HUP_POSIX`/`BSD44ORPOSIX` merge tried and
reverted); `ckctel.c` with both `IKSDONLY`/`CK_AUTODL` merges; `ckcnet.c` untouched. Awaiting user
review before the coordinator commits it — including the disclosed ~15% `ckutio.c` timing cost,
which the user may choose to decline by reverting the one remaining `ckutio.c` hunk too.

## Campaign summary

Four phases, `doc/IFDEF-NESTING-20260710.md`'s audit → this document, all individually reviewed
and committed except this final phase (awaiting review as this is written).

### Full-tree depth re-scan: before-campaign vs. after (all 44 in-scope files, fixed scanner)

| File | Before | After | File |  Before | After |
|---|---:|---:|---|---:|---:|
| `ckcdeb.h` | 13 | **9** | `ckcfn2.c` | 5 | 5 |
| `ckutio.c` | 9 | **8** | `ckcmai.c` | 6 | **5** |
| `ckcnet.c` | 8 | 8 | `ckcnet.h` | 5 | 5 |
| `ckuus4.c` | 8 | 8 | `ckucmd.c` | 7 | **5** |
| `ckuusx.c` | 8 | **6** | `ckucns.c` | 5 | 5 |
| `ckctel.c` | 7 | **6** | `ckufio.c` | 5 | 5 |
| `ckuusr.h` | 7 | 7 | `ckcfns.c` | 4 | 4 |
| | | | `ckclib.h` | 4 | 4 |
| `ckcker.h` | 6 | 6 | `ckcpro.w` | 4 | 4 |
| `ckudia.c` | 6 | 6 | `ckctel.h` | 4 | 4 |
| `ckupty.c` | 6 | 6 | `ckcxla.h` | 4 | 4 |
| `ckuus2.c` | 6 | 6 | `ckucmd.h` | 4 | 4 |
| `ckuus3.c` | 6 | 6 | `ckucon.c` | 4 | 4 |
| `ckuus5.c` | 6 | 6 | `ckuscr.c` | 4 | 4 |
| `ckuus6.c` | 6 | 6 | `ckuver.h` | 4 | 4 |
| `ckuus7.c` | 6 | 6 | `ckuxla.c` | 4 | 4 |
| `ckuusr.c` | 6 | 6 | `ckcfn3.c` | 3 | 3 |
| `ckuusy.c` | 6 | 6 | `ckcfnp.h` | 3 | 3 |
| | | | `ckupty.h` | 3 | 3 |
| | | | `ckclib.c`/`ckcsig.h`/`ckcuni.c`/`ckcuni.h`/`ckuxla.h`/`ckwart.c` | 2 | 2 |
| | | | `ckcasc.h`/`ckusig.c` | 1 | 1 |

**6 files improved** (`ckcdeb.h` 13→9, `ckutio.c` 9→8, `ckuusx.c` 8→6, `ckctel.c` 7→6, `ckcmai.c`
6→5, `ckucmd.c` 7→5); **38 files unchanged** (either never touched, or — `ckcnet.c` — touched by
Phase 3's dead-code deletion without moving the file's own max, since the deleted block wasn't the
sole occupant of that depth). Tree-wide max nesting depth (the audit's original headline number)
drops from **13 to 9**.

### `clang-format` timing, previously-hot files, original-audit baseline vs. final state

| File | Original audit baseline | Final (this session) | Change |
|---|---:|---:|---:|
| `ckcdeb.h` | ~82 s | ~23.2 s | **~3.5× faster** |
| `ckutio.c` | ~11.96 s (single measurement, separate session) | ~11.86 s | ~1%, effectively a wash |
| `ckuusx.c` | ~3.16 s | ~2.35 s | ~26% faster |
| `ckucmd.c` | ~1.92 s | ~1.52 s | ~21% faster |
| `ckcmai.c` | ~0.72 s | ~0.58 s | ~19% faster |
| `ckcnet.c` | *(not in the original audit's timing table)* | ~5.06 s | Phase 3 measured its own before/after: ~4.55 s → ~5.17 s (~14% slower, from the `NOTUSED` deletion; unrelated to this phase) |

`ckutio.c` is the one file in this table where the campaign nets out close to neutral on timing
despite a real depth improvement (9→8) — its only edit landed in this final phase and carries the
~15% cost disclosed above. Every other previously-hot file is faster than when the campaign
started.

### The corrected cost model, in one paragraph

Collapsing an `#else`/`#ifdef` cascade or a genuine AND-nest into `#elif`/compound-`#if` form is a
reliable, often large, `clang-format` speed win when the arms contain only simple content (bare
`#define`s, plain statements) — this describes most of the campaign's wins (`ckcdeb.h`'s `BPS_*`,
`USE_LSTAT`, `CK_64BIT`; `ckuusx.c`/`ckcmai.c`'s transport ladders; `ckucmd.c`'s libc-internals
ladder; `ckctel.c`'s `IKSDONLY` pairs) — but it is **not a universal law**: it can regress
`clang-format` time, sometimes severely, for reasons that are real, measured, and reproducible but
only partially explained. Two distinct regression mechanisms were found and neither reduces to
simple "nesting depth" or "line count": (1) **`#include` directives living inside cascade arms**,
where a *second* such region in the same file triggers super-additive cost (`ckcdeb.h`'s two
`openpty()` cascades, Phase 2: +41 s each alone, +100 s together vs. an additive ~+82 s prediction;
reverted); and (2) **the resulting merged condition line's own formatted shape** — a
backslash-continued long `#if` line costs measurably more on every subsequent format than the
equivalent nested form did (`ckutio.c`'s `_IBMR2` chain, Phase 4: neutral pre-format, +15% once
`clang-format`'s own line-wrap is applied — the regression is in the *formatting choice*, not the
merge). A third, smaller-magnitude, mechanism-agnostic effect showed up in Phase 3 (deleting 147
dead lines from `ckcnet.c`'s middle made the file ~14% *slower* to format, with no plausible
`#include`- or line-wrap-based explanation at all, and no further investigation attempted given the
small stakes). **The practical rule this campaign leaves behind: never assume, always measure
before/after on the real file, and revert per-hunk (not per-file) when a specific change regresses
— depth reduction is a proxy for the real goal, not the goal itself, and the proxy fails often
enough that skipping the measurement step is not safe.**

### What was deliberately left alone, and why

- **`ckcdeb.h`'s two `openpty()` cascades** (Phase 2) — mechanically valid Fix-1 targets, but
  reverted after measurement showed a severe, super-additive `clang-format` regression tied to
  `#include` directives in their arms. Depth benefit (would take `ckcdeb.h` from 9 toward 5) was
  judged not worth the ~4.7× file-wide formatting cost.
- **The `V7MIN`-gated feature-subset wrappers** (`NOICP`/`NONET`/`NOLOCAL`/`NOXFER` etc., audit
  Pattern 3) — a real, currently-invocable (if not `make`-target-wired) minimum-size build mode;
  hard-wiring or removing these wrappers would silently break it. Never a candidate for any of the
  four phases' mechanical transforms in the first place (they're neither cascades nor pure AND-nests
  in the collapsible sense — they gate whole build configurations).
- **`ckcnet.c`'s `NOHTTP`/`TIMEH`/`SYSTIMEH`/`SYSTIMEBH`/`POSIX`/`CLIX` `<time.h>`-selector cascade**
  (`ckcnet.c:5557-5590`, exposed as the file's bottleneck once Phase 3 deleted the `NOTUSED` block
  that used to tie with it) — a legitimate Fix-1 (`#elif`) candidate, structurally clean, but out of
  every phase's named scope so far (Phases 1-2 covered named `#elif` targets, this phase covered
  named AND-nest targets); left as a documented opportunity for a future pass. Also contains a
  self-negating oddity (`#ifdef SYSTIMEH` nested inside `#ifndef SYSTIMEH`) worth a human look
  before anyone touches it.
- **`ckutio.c`'s `NOUUCP`/`USETTYLOCK`/`LOCK_DIR`/`BSD44`/`HDBUUCP`/`M_SYS5`/`SVR4`/`LINUXFSSTND`
  UUCP-lock-directory cascade** (`ckutio.c:184-291`, exposed as the file's new bottleneck by this
  phase's own edit) — same situation: a clean Fix-1 candidate, out of this phase's AND-nest-only
  scope, left for a future pass.
- **`ckcdeb.h`'s `TNCODE`/`NOSSH`/`NOLOCAL`/`UNIX`/`SSHCMD`/`NETPTY`/`NOPUSH` AND-nest**
  (`ckcdeb.h:1465-1474` currently, flagged in Phase 2 as Fix-2 material) — **re-verified now, under
  this phase's own rules: it qualifies.** Confirmed directive-by-directive: `#ifndef NOLOCAL` →
  `#ifdef UNIX` → `#ifndef SSHCMD` → `#ifdef NETPTY` → `#ifndef NOPUSH` → `#define SSHCMD`, five
  levels, zero `#else` anywhere in the span — a clean, safe, mechanically-collapsible AND-nest by
  every criterion this phase applied to `ckutio.c`'s and `ckctel.c`'s targets. It was **not**
  applied here because it was never named in this phase's task scope (three named files plus one
  named optional `ckutio.c` edit; `ckcdeb.h`'s `SSHCMD` nest is none of those) — left as a
  ready-to-apply, pre-verified opportunity for a future pass rather than an unrequested scope
  expansion.

### Lines removed/changed, whole campaign

`git diff --stat` against the pre-campaign commit (`388476b`), source files only:

```
 ckcdeb.h |  88 +++++++++++---------------------------
 ckcmai.c |  20 +++------
 ckcnet.c | 147 ---------------------------------------------------------------
 ckctel.c |  12 ++----
 ckucmd.c |  31 +++++---------
 ckufio.c |   9 ----
 ckutio.c |  23 ++++------
 ckuusx.c |  24 +++--------
 ckuxla.c |  16 -------
 9 files changed, 63 insertions(+), 307 deletions(-)
```

**Net -244 lines across 9 files** (147 of which are the one confirmed-dead, non-compiling
`ckcnet.c` function). Every change in every phase verified byte-identical at the `wermit`/`.o`
level — the campaign changed zero bytes of compiled behavior on this platform, and was verified
(to the extent this sandbox allows) not to change it on macOS/BSD either.
