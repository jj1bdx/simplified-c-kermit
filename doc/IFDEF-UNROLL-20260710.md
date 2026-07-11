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

**Applied to the working tree, not committed.** This reflects the *recommended* final state
(three `.c` files as originally written; `ckcdeb.h` with `USE_LSTAT`/`CK_64BIT` kept and both
`openpty()` cascades reverted) — awaiting user review before the coordinator commits it. The
`openpty()` cascades remain a legitimate depth-reduction opportunity (`ckcdeb.h` would drop to
depth 8 or lower) but are **not recommended** without a real fix for the timing regression, since
depth reduction alone was never the actual goal.
