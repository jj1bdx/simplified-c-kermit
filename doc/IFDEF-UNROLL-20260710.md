# IFDEF-UNROLL-20260710 — applying the `#ifdef`-nesting fixes

Date: 2026-07-10

## Purpose

Running summary of applying the remediations proposed in `doc/IFDEF-NESTING-20260710.md`
(the nesting-depth audit), phase by phase, in the report's recommended order of attack. Each
phase is applied, verified against the levels in `.claude/skills/build-and-verify`, and appended
here before the user reviews and the coordinator commits it. No phase in this document has been
committed or pushed — that happens separately, per phase, after user approval.

Planned phases (per the audit's "Conclusion — recommended order of attack"):

1. `ckcdeb.h` `BPS_*` speed-selection ladder → `#elif` chain (this phase).
2. `ckuusx.c`/`ckcmai.c` network-transport ladder and `ckucmd.c` libc-internals ladder → `#elif` chains.
3. Delete the three `#ifdef NOTUSED` dead-code blocks (`ckcnet.c`, `ckuxla.c`, `ckufio.c`).
4. AND-nest collapsing in `ckutio.c`/`ckcnet.c`/`ckctel.c`.

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

### Status

**Applied to the working tree, not committed.** Awaiting user review before the coordinator
commits this phase.
