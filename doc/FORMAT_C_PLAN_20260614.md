# Plan: Apply clang-format to all `*.c` files (one-by-one, binary-verified)

## Outcome (2026-06-14) — COMPLETE

All 32 tracked `*.c` files reformatted; **zero `*.h` files touched**. A full
clean rebuild (`make clean && make wart && make ckcpro.c && make linux`) produces
a `wermit` and a generated `ckcpro.c` that are **byte-identical** to the
pre-formatting baseline; `make check` reports SUCCESS and the binary runs/exits
cleanly.

**Key decision — `InsertBraces` disabled.** During execution, `ckcnet.c` failed
the byte-identical gate even though its token stream was provably equivalent. The
sole cause was `InsertBraces: true`: adding `{}` around single-statement bodies is
the one *non-cosmetic* (AST-altering) transform in the config, and at `-O` it made
GCC emit equivalent-but-not-identical code (different register allocation / branch
layout) in `locate_srv_dns`. Per user decision, **`.clang-format` was changed to
`InsertBraces: false`** and all 32 files reformatted uniformly under that config —
making the formatting purely cosmetic and every file byte-identical. This is the
only config change; it is a deviation from the original committed `InsertBraces:
true` and is intentional.

Verification specials: `ckwart.c` verified via byte-identical regenerated
`ckcpro.c`; `ckucon.c` (not linked by `make linux`, which uses `ckucns.c`) verified
by a standalone byte-identical `.o` compiled under the same filename.

The body below is the original plan as approved.

---

## Context

The repo recently adopted a committed `.clang-format` (commit `df03945`, with
`SortIncludes` disabled to avoid breaking include-order-sensitive code). The
goal now is to actually **apply** that formatting to every tracked C *source*
file (`*.c` only — **never** the `*.h` headers, per the request), while proving
that the reformatting is **behavior-preserving**.

C-Kermit is conservative, macro-heavy, portable C. The risk of a blind bulk
`clang-format -i` is that `InsertBraces`, comment reflow, or string-literal
breaking silently changes meaning or breaks the build. To contain that risk we
format **one file at a time** and, after each file, rebuild and require the
compiled binary to be **byte-identical** to a pre-formatting baseline. A
byte-identical binary is a provable equivalence gate for this codebase (see
"Why the gate is sound" below).

This matches the project's established "binary identical" verification idiom
used throughout the prior simplifications (see `CLAUDE.md`).

## Scope

- **In scope:** the 32 git-tracked `*.c` files:
  `ckcfn2.c ckcfn3.c ckcfns.c ckcftp.c ckclib.c ckcmai.c ckcmdb.c ckcnet.c
  ckctel.c ckcuni.c cku2tm.c ckucmd.c ckucns.c ckucon.c ckudia.c ckufio.c
  ckupty.c ckuscr.c ckusig.c ckustr.c ckutio.c ckuus2.c ckuus3.c ckuus4.c
  ckuus5.c ckuus6.c ckuus7.c ckuusr.c ckuusx.c ckuusy.c ckuxla.c ckwart.c`
- **Excluded:** all `*.h` files (explicit request); `ckcpro.c` (generated from
  `ckcpro.w` by `wart`, gitignored, not a source of truth); `ckcpro.w` (not a
  `.c` file). The grammar/protocol path is still *verified* indirectly via
  `ckwart.c` handling below.
- **Config:** the repo-committed `.clang-format`, auto-discovered
  (`clang-format -i <file>`). Notable settings: `SortIncludes: false`,
  `InsertBraces: true`, `ReflowComments: Always`, `ColumnLimit: 80`,
  `BreakStringLiterals: true`. The untracked experimental `clang-format-config`
  (SortIncludes on / InsertBraces off) is **ignored**. clang-format is 21.1.8.

## Why the binary-identical gate is sound (verified during planning)

- **No `__LINE__` / `__FILE__`** anywhere in the tree → line-number shifts from
  reformatting cannot change codegen.
- **`__DATE__` / `__TIME__`** appear only in `ckcmai.c` and `ckuus5.c`. They are
  neutralized by exporting a **fixed `SOURCE_DATE_EPOCH`** (confirmed: gcc then
  emits byte-identical `.o`; without it, two builds differ by a timestamp byte).
- **c-expert analysis** (read-only, spot-checked ckclib/ckcfns/ckutio/ckufio/
  ckcftp/ckuus*) found **no** construct that fails to compile, changes meaning,
  or changes the binary:
  - `if`/`while` conditions split across `#ifdef/#else/#endif` always have the
    controlled brace/statement *after* the last `#endif`; `InsertBraces` wraps
    the whole post-`#endif` body, never injects a brace inside one branch.
  - Bare-`if` function-like macros (e.g. `ckcftp.c` `CHECKCONN()`) are only used
    as standalone `CHECKCONN();` — tokens unchanged after reflow.
  - `BreakStringLiterals` only splits long literals into **adjacent** literals;
    concatenated content is md5-identical and the compiler re-joins them before
    codegen → identical `.o`.
  - Empty bodies (`while(...) ;`) keep the `;` (not turned into `{}`).
- Therefore: a formatted-but-equivalent `.c` **cannot** produce a different
  `.o`; the only thing the gate ignores is pure comment/text change, which has
  no behavioral effect. The gate is sufficient.

## Execution procedure

### Step 0 — write the plan into the repo
Create `FORMAT_C_PLAN_20260614.md` in the repo root containing this plan (the
request asked for the plan to live there). It is a normal tracked doc.

### Step 1 — establish a reproducible baseline
```sh
export SOURCE_DATE_EPOCH=1749859200      # fixed (2026-06-14); used for ALL builds below
make clean
make wart && make ckcpro.c               # generate the protocol module
make linux                               # produce wermit
cp wermit  /tmp/wermit.baseline
cp wart    /tmp/wart.baseline
cp ckcpro.c /tmp/ckcpro.c.baseline
```
Then prove reproducibility BEFORE touching any file:
```sh
make clean && make wart && make ckcpro.c && make linux
cmp wermit /tmp/wermit.baseline          # MUST be identical
```
If this fails, stop — the build is not reproducible and the gate is invalid;
fall back to per-`.o` comparison (the 30 non-timestamp files compare `.o`
directly; `ckcmai.o`/`ckuus5.o` get a manual diff review + clean compile). Do
not proceed until the baseline is trustworthy.

> Keep `SOURCE_DATE_EPOCH` exported for the entire session. Do **not** use
> `make -e` (forbidden by CLAUDE.md); an exported env var is read by the
> compiler directly and is fine.

### Step 2 — per-file loop (for each `.c` in the order below)
```sh
clang-format -i <file>.c                 # uses repo .clang-format
make linux                               # incremental: recompiles <file>.o + relinks
cmp wermit /tmp/wermit.baseline          # invariant: identical to ORIGINAL baseline
```
- **Identical → pass.** Move to the next file. (Each formatted file must keep
  `wermit` identical to the *original* baseline, so the invariant is the same
  comparison every time — earlier formatted files stay formatted.)
- **Differs or fails to compile → STOP.** Do not continue. Hand the file's
  `clang-format` diff to a **c-expert** subagent for analysis (the request
  mandates c-expert for analysis). Likely culprits and remedies:
  - a real semantic change → revert that file (`git checkout -- <file>.c`) and
    record it as un-formattable / needing a localized `// clang-format off`
    guard, or skip it;
  - a benign cause the gate over-flagged → document and decide explicitly.
  Resolve before resuming.

### Step 3 — special handling

**`ckwart.c`** (plain `make linux` does NOT re-run wart, so format it explicitly
exercised):
```sh
clang-format -i ckwart.c
make clean && make wart && make ckcpro.c
cmp ckcpro.c /tmp/ckcpro.c.baseline      # KEY: proves wart's emitted output unchanged
make linux && cmp wermit /tmp/wermit.baseline
```
(The `wart` binary itself may differ harmlessly; only its *output* `ckcpro.c`
must match.)

**`ckcmai.c` and `ckuus5.c`** (the `__DATE__`/`__TIME__` files): do these **last**,
only after the loop has already confirmed the gate holds on an
already-formatted file, so a stray date/time mismatch can't be misread as a
formatting-induced diff. `SOURCE_DATE_EPOCH` must be set for their builds.

### Processing order (low-risk → high-risk; recommended by c-expert)
1. Tiny/trivial: `ckusig.c ckustr.c cku2tm.c ckcmdb.c ckuscr.c`
2. `ckwart.c` (verify via `ckcpro.c` cmp, Step 3)
3. Medium: `ckclib.c ckcfn2.c ckcfn3.c ckupty.c ckucns.c ckucon.c`
4. Large: `ckcfns.c ckctel.c ckcnet.c ckcuni.c ckucmd.c ckudia.c ckufio.c
   ckutio.c ckuxla.c ckuusy.c ckuusx.c ckuusr.c ckuus2.c ckuus3.c ckuus4.c
   ckuus6.c ckuus7.c ckcftp.c`
5. Last (timestamp files): `ckcmai.c ckuus5.c`

### Manual eyeball (even when the binary matches — belt-and-suspenders)
`ckcftp.c` (CHECKCONN macro + heaviest literal splitting + continued `#define`s),
`ckutio.c`/`ckufio.c` (cond split across `#ifdef`), `ckcfn2.c` (checksum `if`
across `#ifdef CKTUNING`), `ckcmai.c`/`ckuus5.c` (herald/version strings),
`ckuus2.c` (765 extra literal pieces — skim for readability).

## Final verification

1. Full clean rebuild from the fully-formatted tree:
   `make clean && make wart && make ckcpro.c && make linux`
2. `cmp wermit /tmp/wermit.baseline` → **byte-identical** (the whole-tree proof).
3. Sanity-run the binary, e.g. `./wermit -V` / `./wermit --version` (or
   `echo exit | ./wermit`) → starts and exits cleanly. (Redundant given the
   byte-identical proof, but a cheap smoke test.)
4. `git diff --stat` to review the scale of whitespace-only changes; spot-check
   the manual-eyeball files above.

## Deliverables

- `FORMAT_C_PLAN_20260614.md` (this plan, in repo).
- 32 reformatted `*.c` files, each proven binary-neutral.
- No `*.h` changes, no `ckcpro.c`/`wermit`/`wart` committed (they stay
  gitignored). `SOURCE_DATE_EPOCH` is a session-only env var, not committed.
- Commit only on explicit request; if so, branch first.

## Notes / risks

- If any single file can't pass the gate, it is reverted and reported rather
  than force-formatted — the request prioritizes "works as before" over 100%
  coverage.
- The displayed compile date will read as the fixed `SOURCE_DATE_EPOCH` date in
  verification builds only; normal builds (no env var) show the real date. The
  committed sources' behavior is unchanged.
