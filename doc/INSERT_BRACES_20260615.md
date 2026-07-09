# Enable clang-format `InsertBraces` and reformat the tree

Date: 2026-06-15

## What changed

`.clang-format` `InsertBraces` was flipped `false → true`, and `clang-format -i` was run
across all `*.c`/`*.h`. `InsertBraces` wraps single-statement `if/else/for/while/do` bodies
in `{}`. Result: **29 `.c` files reformatted (+22,527 / −12,615 lines)**; **all `.h` files
unchanged** (prototype-only, no statement bodies to brace). `ckcpro.c` is generated/gitignored
(regenerated from `ckcpro.w`, not committed). The existing `/* clang-format off */` islands
around `ckcdeb.h` and `ckcsig.h`'s `<setjmp.h>` are honored (no include reordering).

`InsertBraces` is the one **non-cosmetic** clang-format transform in this tree (CLAUDE.md had
kept it `false` since 2026-06-14 because it defeats the byte-identical build gate). It is also
the one clang-format flag the tool's own docs warn "can produce incorrect formatting" due to
incomplete semantic info. So equivalence was proven rather than assumed.

## Hazard analysis

The two real `InsertBraces` hazards in this `#ifdef`-heavy C code are **dangling-else
mis-binding** and **braces spanning a preprocessor conditional**. clang-format 21.1.8 was
tested on both (stdin, before applying):
- Dangling else (`if (a) if (b) x(); else y();`): `else` stays bound to the inner `if (b)`.
  Misleading-indentation variants also bind per C grammar, not per whitespace.
- `if`-body spanning `#ifdef/#else/#endif`: clang-format **refuses to brace it** (leaves the
  directive-spanning body unbraced), braces only the safe sibling.

`InsertBraces` operates on the parsed AST, so a brace-position regression that rebinds `else`
or regroups statements is structurally impossible from the tool.

## Verification — layered (byte-identical does NOT hold, so equivalence proven another way)

**L0 — Build + functional gate.** Clean fixed-`SOURCE_DATE_EPOCH` `make clean && make wart &&
make ckcpro.c && make linux`: SUCCESS, **warning-clean**; `make check`: SUCCESS.

**L1 — Linux codegen (per-object).** Pre-format objects (`/tmp/before_obj/*.o`) vs post-format:
**28 of 29 byte-identical.** The single outlier `ckcnet.o` was localized to a benign
`if (addr == NULL) { return 0; }` in `locate_srv_dns`, and proven semantically identical by
compiling `ckcnet.c` before/after at **`-O0`**: the only assembly delta is internal label
renumbering (`.L1405→.L1384`) plus one coalesced redundant label — **zero instruction or
control-flow change**. The `-O` object difference is pure register-allocation churn from the
added block scope. ⇒ For all Linux-compiled code, brace *position* is proven semantically
irrelevant (else-binding preserved).

**L2 — Brace-only token proof (all 45 files, config-independent).** A comment/string-aware
tokenizer (`/tmp/brace_only.py`) shows the token stream with `{`/`}` removed is **identical
before vs after for every file** (45/45, 0 mismatch). ⇒ clang-format changed **only** brace
tokens (plus whitespace/comment reflow); no statement text was added, removed, or reordered in
**any** `#ifdef` branch — including the macOS/BSD regions never compiled on Linux.

**L3/L4 — c-expert + non-Linux-region review (the gap L1 can't reach).** L1's codegen proof
covers only the Linux path; L2 ignores brace *position*. The residual risk — a mis-brace inside
a macOS/BSD-only `#ifdef` region — was closed by c-expert:
- AST root-cause argument (above), verified on the misleading-indentation trap.
- An **exhaustive 688-site check**: of every `else` immediately followed by a preprocessor
  directive in the pre-format source (the only config where brace position can matter), **none**
  had a `{` inserted between the `else` and the directive, and **no `else` keyword moved across
  any directive** in any file.
- Inspection of every directive-straddling `if/else` in the platform-heavy files (`ckcnet.c`
  RLOGCODE/INADDRX, `ckctel.c` CK_SNDLOC else-if ladder, `ckudia.c` TN_COMPORT, `ckcfns.c`):
  clang-format correctly braces only the non-directive-spanning body and never splits `} else`
  across a directive.
- `cc -fsyntax-only -DBSD44 -DMACOSX -DMACOSX10` (± RLOGCODE/INADDRX/CK_SNDLOC/TN_COMPORT) on
  `ckcnet.c`/`ckctel.c`: **no** `expected '}'` / `else without a previous if` / unbalanced-brace
  diagnostics (only expected implicit-declaration errors from absent macOS headers).

**Verdict: semantically equivalent, no findings, safe to commit.**

## Idempotency note

Re-running clang-format is a no-op for code in every file. `ckufio.c` and `ckcfn2.c` (and
generated `ckcpro.c`) are **not** idempotent, but **only** in block-comment reflow of pathological
historical comments containing long embedded-space runs (the `ReflowComments` setting oscillating).
This is **comment-only, codegen-irrelevant, and pre-existing** (independent of `InsertBraces`;
already noted in the 2026-06-15 SortIncludes work). It does not affect the equivalence conclusion.

## Files

- `.clang-format` — `InsertBraces: true`.
- 29 `.c` files reformatted (e.g. `ckcftp.c`, `ckuus4.c`, `ckutio.c`, `ckucmd.c`, `ckcnet.c`).
- `.h` files: unchanged by the tool (covered by the L2 token proof regardless).
- Excluded: `ckcpro.c` (generated/gitignored), `ckcpro.w` (wart grammar).

## Follow-up

The CLAUDE.md guidance "keep `InsertBraces: false`" is now superseded — `InsertBraces: true` is
verified safe. Update it separately.
