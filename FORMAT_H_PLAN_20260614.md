# Plan: Apply clang-format to all `*.h` files (one-by-one, dual-verified)

## Context

We just reformatted all 32 `*.c` files with clang-format, proved each rebuild
byte-identical, and committed (branch `clang-format-c-files`). As part of that we
set `.clang-format` `InsertBraces: false` — it was the only non-cosmetic
(AST-altering) transform, and at `-O` it made GCC emit equivalent-but-not-identical
code (see `FORMAT_C_PLAN_20260614.md`).

Now do the **headers**: apply clang-format to all 18 tracked `*.h` files, **never
touching the already-formatted `*.c` files**, and verify after each header that
the code still works exactly as before. With `InsertBraces` already false,
formatting headers is purely cosmetic (whitespace + comment reflow + string-literal
splitting + multi-line macro re-wrapping).

## Scope

- **In scope:** the 18 git-tracked `*.h` files:
  `ckcasc.h ckcdeb.h ckcfnp.h ckcftp.h ckcker.h ckclib.h ckcnet.h ckcsig.h
  ckcsym.h ckctel.h ckcuni.h ckcxla.h ckucmd.h ckupty.h ckusig.h ckuusr.h
  ckuver.h ckuxla.h`
- **Excluded:** all `*.c` files (already formatted — do not re-touch); the
  generated `ckcpro.c`; build artifacts.
- **Config:** the in-repo `.clang-format`, auto-discovered, **already with
  `InsertBraces: false`** (no config change this round). clang-format is 21.1.8.

## Why the verification is sound (proven during planning)

Two independent gates per header; together they cover every platform and TU.

1. **Token-equivalence (PRIMARY, platform-independent).** Tokenize the header
   *skipping comments*, after *splicing `\`-newline line-continuations* (as the
   preprocessor does) and *merging adjacent string literals*; require the token
   stream to be **identical** before vs after formatting. Dry-run result:
   **all 18 headers are already token-identical.** This proves semantic identity
   on *every* platform, including the macOS/BSD-only `#ifdef MACOSX` / `#ifdef
   BSD44` regions that the Linux build never compiles. c-expert confirmed this is
   sufficient: there are **zero `#` stringize / `##` paste operators** in any
   header (the only mechanism by which macro-body whitespace could become
   significant — C 6.10.3.2 — simply does not occur here), and no
   `WhitespaceSensitiveMacros` are used.
   - The 4 macro-heavy headers (`ckcdeb.h`, `ckcker.h`, `ckcnet.h`, `ckctel.h`)
     only differ pre-splice by **repositioned `\` continuations** from re-wrapped
     multi-line macro bodies — preprocessor-transparent (verified: `CK_SLEEPINT`,
     `bleep`, `tlog` etc. expand unchanged; `gcc -E` of `ckctel.h` is
     byte-identical).
   - `ckcdeb.h` master-header implication chain (`MACOSX10→MACOSX→BSD44→SVR4→
     SVR3/ATTSV`) and all `#`-directives are byte-identical in sequence; no `#`
     moved off column 1 (`IndentPPDirectives: None`). `ckuver.h` herald
     first-match-wins guard order is preserved.

2. **Byte-identical `wermit` (SECONDARY, Linux build integrity).** The makefile
   **tracks header→object dependencies explicitly** (e.g. `ckcmai.$(EXT):
   ckcmai.c ckcker.h ckcdeb.h …`), so editing a header recompiles its dependent
   TUs on `make linux`. After each header, rebuild and `cmp wermit` against a
   fixed baseline (with `SOURCE_DATE_EPOCH` exported, as in the .c plan). This
   confirms no compile breakage and that all Linux-compiled code is byte-identical.
   - **16 of 18 headers** are compiled into the linux build. **2 are not**
     (`ckcftp.h`, `ckusig.h`) — for those this gate is vacuous, so gate #1
     (token-equivalence, already passed) is the verification, optionally plus a
     standalone `gcc -fsyntax-only` parse.

## Execution procedure

### Step 0 — write the plan into the repo
Create `FORMAT_H_PLAN_20260614.md` (repo root) containing this plan.

### Step 1 — baseline
```sh
export SOURCE_DATE_EPOCH=1749859200            # same fixed epoch as the .c run
make clean && make wart && make ckcpro.c && make linux
cmp wermit /tmp/wermit.baseline 2>/dev/null && echo "matches prior baseline" || cp wermit /tmp/wermit.baseline
cp wermit /tmp/wermit.hbase                     # baseline for this run
```
The current committed (`.c`-formatted) tree builds byte-identical to the prior
baseline, so `/tmp/wermit.baseline` is reused; `/tmp/wermit.hbase` is the gate
target. Confirm reproducibility with a second clean build + `cmp` before editing
any header. Keep `SOURCE_DATE_EPOCH` exported all session. Do not use `make -e`.

### Step 2 — per-header loop (order below)
```sh
clang-format -i <hdr>.h
# GATE 1 (primary): token-equivalence must be identical
python3 verify_tokens.py <hdr>.h           # original (git show HEAD:) vs working tree
# GATE 2 (secondary): linux build byte-identical
make linux
cmp wermit /tmp/wermit.hbase
```
- **Both gates pass → next header.**
- **Token-equivalence differs, or build fails, or wermit differs → STOP.** Hand
  the diff to a **c-expert** subagent (request mandates c-expert for analysis);
  `git checkout -- <hdr>.h` to revert, decide whether to skip or guard with
  `// clang-format off`. (None expected — all 18 passed the dry-run.)

The token check tokenizes with: strip `/* */` and `//` comments, splice
`\`+newline, merge adjacent string literals, then compare the token sequence of
`git show HEAD:<hdr>.h` against the working-tree file. (This is the exact check
already validated on all 18 during planning.)

### Order (low → high risk; per c-expert)
1. `ckcsym.h` (clang-format no-op), `ckcasc.h`, `ckcftp.h`, `ckusig.h`,
   `ckclib.h`, `ckcsig.h`
2. `ckuxla.h`, `ckuver.h`, `ckupty.h`, `ckcuni.h`, `ckucmd.h`, `ckcxla.h`
3. `ckcfnp.h`, `ckcnet.h`, `ckctel.h`, `ckcker.h`
4. `ckuusr.h` (2607 lines), `ckcdeb.h` (4038 lines, master) — last; eyeball after.

### Special handling
- **`ckcftp.h`, `ckusig.h`** (not in linux build): rely on Gate 1; add a
  `gcc -fsyntax-only -x c <hdr>.h` parse as a cheap extra check.
- **`ckcdeb.h`, `ckuver.h`** (manual eyeball even though tokens match): after
  `-i`, confirm `git diff` shows only whitespace / comment-alignment / `\`-position
  changes and that **no `#`-directive line content** changed.

## Final verification
1. `make clean && make wart && make ckcpro.c && make linux`
2. `cmp wermit /tmp/wermit.hbase` → **byte-identical** (whole-tree proof).
3. `cmp ckcpro.c /tmp/ckcpro.c.baseline` → identical (headers feed `ckcpro.c`).
4. Smoke-run `./wermit -V` → version banner, clean exit.
5. `git diff --stat` to review scale; spot-check `ckcdeb.h` / `ckuver.h`.

## Deliverables
- `FORMAT_H_PLAN_20260614.md` (this plan, in repo).
- 18 reformatted `*.h` files, each dual-verified (token-identical + byte-identical
  build). No `*.c` changes, no `.clang-format` change, no artifacts committed.
- Commit only on explicit request; branch first.

## Notes / risks
- Token-equivalence is the load-bearing gate (covers macOS/BSD-only regions the
  Linux build can't exercise). All 18 already pass it in the dry-run, so no
  surprises are expected; any single failing header is reverted and reported
  rather than force-formatted.
- No `__LINE__`/`__FILE__`/`__DATE__`/`__TIME__` in any header, so header
  reformatting cannot perturb codegen via those; `SOURCE_DATE_EPOCH` is kept set
  only because the two .c timestamp files are recompiled when shared headers
  change.
