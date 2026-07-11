# C-Kermit 10.0 Beta.12 (simplified fork) — `main` vs `wart-cpp-comment`: C++-style comment conversion verified functionally equivalent

**Subject:** the `wart-cpp-comment` branch converts every `/* … */` comment in the tree to
C++-style `//` comments (44 files, ~66 000 insertions / ~68 000 deletions) and teaches the
`wart` preprocessor to parse them. This report verifies, by fixed-epoch build comparison and by
an independent source-level token-stream proof, that the branch introduces **no functional
change** to the `wermit` binary.

- **Branches compared:** `main` (`008c216`) vs `origin/wart-cpp-comment` (`3ae6ed7`).
  `main` is fully merged into the branch, so `git diff main...origin/wart-cpp-comment` is
  exactly the branch's own changes.
- **Date:** 2026-07-11
- **Method:** the repo's build-and-verify methodology (level 1/2: `SOURCE_DATE_EPOCH`
  byte-identical clean rebuild + per-object comparison) performed in the main session, plus a
  c-expert agent analysis (level 3: token-stream proof, hazard scan, `ckwart.c` review).

---

## TL;DR

1. **Verdict: functionally equivalent.** All executable code in `wermit` is **byte-identical**
   between the two branches. 26 of 28 object files are byte-identical outright.
2. The only differences in the linked `wermit` are (a) the 20-byte GNU build-id (a hash, so it
   necessarily differs) and (b) one `.rodata` string: the wart version banner
   `"Wart Version 2.17, 04 February 2024 "` → `"Wart Version 2.18, 11 July 2026 "`, which wart
   embeds in the generated `ckcpro.c`. Both are expected and intentional.
3. `ckwart.o` (the wart build tool itself, **not** linked into `wermit`) carries the branch's
   one deliberate functional change: a new `rdcmns()` function so wart can skip `//` comments
   in `ckcpro.w`. Reviewed and found correct (§4).
4. All 43 other changed files (42 `.c`/`.h` + `ckcpro.w`) are **token-identical** after
   comment stripping, with zero string/character-literal drift (§3).
5. **One non-blocking follow-up recommended before merge:** the branch regresses the repo's
   clang-format stability gate on 11 of the 44 files (`main` is 0/44). Ten of them converge
   after a single `clang-format -i` pass — it looks like the final formatting pass simply
   wasn't re-run after the mechanical substitution (§6.1).

---

## 1. What the branch changes

Commits unique to the branch:

```
3ae6ed7 Make all *.[ch] source files using C++-style comments
6e8dfcb Merge branch 'main' into wart-cpp-comment
2d08b8a ckwart.c: Use C++-style comments
947ce2a ckcpro.w: convert comments into C++ style
2c63c07 ckwart.c: add C++ comment handling as rdcmns()
```

- Every changed `.c`/`.h` file and `ckcpro.w`: pure `/* … */` → `//` comment resyntax.
- `ckwart.c`: the same comment resyntax **plus** exactly four non-comment changes — the version
  banner bump (2.17 → 2.18), a `void rdcmns(FILE *);` prototype, a new
  `else if (c == '/') { rdcmns(fp); continue; }` dispatch inside `gettoken()`'s existing
  `case '/':`, and the `rdcmns()` function body. A word-level diff of the stripped token
  streams confirms nothing else in the 433-line diff is functional.

## 2. Build verification (byte-identical rebuild gate)

Two detached worktrees (one per branch) were built from scratch with
`SOURCE_DATE_EPOCH=1750000000 make linux`. Both builds completed **warning-clean**.

| Artifact | Result |
|---|---|
| 26 of 28 `.o` files | **byte-identical** (`cmp`) |
| `ckcpro.o` | `objdump -d` disassembly **identical**; only `.rodata` difference is the wart banner string |
| `ckwart.o` | differs — the intentional `rdcmns()` change; wart is a build tool, never linked into `wermit` |
| `wermit` | `objdump -d` disassembly **identical**; `cmp -l` shows exactly two differing regions: bytes 865–884 (GNU build-id note) and bytes 1 702 873–1 702 892 (the banner string in `.rodata`) |

The generated `ckcpro.c` (new wart run on the `//`-commented `ckcpro.w` vs old wart on the old
`ckcpro.w`) was compared after comment stripping and whitespace normalization: token-identical
except the banner string and two intra-statement newline placements (a trailing `/* comment */`
that had sat mid-statement had to move to its own line as `//`), which are insignificant to the
compiler.

Runtime smoke test: both binaries start cleanly, print identical `show version` output
(C-Kermit 10.0.416 Beta.12 and all module versions match), and exit 0.

## 3. Source-level token-stream proof (c-expert analysis)

**Methodology note.** The obvious tool, `gcc -fpreprocessed -dD -E -P`, was tried and
**rejected**: `-fpreprocessed` disables translation-phase-2 line splicing, and this codebase
contains backslash-continued *string literals* (e.g. the server-mode banner in `ckcmai.c`),
which made it emit `missing terminating "` warnings and would have corrupted the comparison.
Instead a small Python stripper was written that replicates the real C translation-phase order:
phase 2 (splice `\`+newline everywhere) first, then phase 3 (tokenize with string/char-literal
tracking, so `/*`, `*/`, `//` inside literals are never misread; real comments become one
space). It also emits each file's ordered list of string/char-literal tokens verbatim for an
independent literal check.

**Results for all 43 comment-only files (42 `.c`/`.h` + `ckcpro.w`):**

- **Token-identical: 43/43.** No file diverges in its comment-stripped, whitespace-normalized
  token stream.
- **Literal drift: zero.** The ordered literal-token lists match exactly in every file
  (including all 620 literals in `ckcpro.w`); a separate grep of the git diff for changed lines
  containing quotes confirmed no string content was touched.
- **Line counts:** every file shrank (−1 to −311 lines; largest: `ckutio.c` −311,
  `ckcnet.c` −191, `ckufio.c` −160 — standalone `/*` and `*/` delimiter lines disappearing),
  except `ckcsig.h` (+2, dense single-line multi-clause comments that unpack into one `//` per
  clause). Line-count shifts are harmless here: **no source file uses `__LINE__` or `__FILE__`
  at all**; `__DATE__`/`__TIME__` appear in exactly two places (`ckcmai.c`, `ckuus5.c`), both
  line-count-insensitive (and pinned by `SOURCE_DATE_EPOCH` in the build gate anyway).

Seven files show *placement-only* differences (a token on a different physical line/column),
all one mechanism: a same-line trailing `/* comment */` became `//`, which cannot be followed
by code on the same line, so clang-format's line-fit computation broke the line elsewhere.
Token sequences are character-for-character identical in each case:

| Location (branch) | What moved |
|---|---|
| `ckucns.c:211`, `ckucon.c:218`, `ckuus4.c:423` | `extern CHAR (*xlr[…])(CHAR);` line-break position |
| `ckutio.c:1988`, `ckutio.c:2063` | `p = malloc(x);` split across two lines |
| `ckuus3.c:275` | `struct keytab ctltab[] = {` brace/first-entry packing |
| `ckuusr.c:6456` | one `cmfdbi(&sw, …)` call's wrap point (the other ~12 sites converted cleanly) |
| `ckuusx.c:704` | `char *` split from `ttgtpn() {` |

## 4. `ckwart.c` / `rdcmns()` review

```c
void rdcmns(FILE *fp) {
  int c;
  while ((c = getc(fp)) != '\n') {
    if (c == EOF) {
      fatal("Unterminated single-line comment");
    }
  }
  lines++;
}
```

Assessment: **correct, no bugs found.** Key points:

- `gettoken()` (and hence `rdcmnt()`/`rdcmns()`) only scans the inter-rule text between the two
  `%%` markers of `ckcpro.w`. The declarations section, each `{ action }` body, and the
  epilogue are copied to `ckcpro.c` verbatim with no comment-awareness (unchanged behavior);
  the real C compiler interprets comments there. So the classic edge cases — `//` inside a
  string literal, `/` as division — are structurally unreachable by `rdcmns()`.
- A bare `/` in the region `gettoken()` does scan was already
  `fatal("Invalid character in input")` before this change and still is.
- `rdcmnt()`'s runaway-`%%`-swallow guard is correctly *absent* from `rdcmns()`: a `//` comment
  is bounded by construction at the next newline (or `fatal()` at EOF), so the failure mode the
  guard defends against cannot occur.
- `rdcmns()` does not implement backslash-newline splicing. Acceptable: wart discards this
  text (it is never copied into `ckcpro.c`), and the converted `ckcpro.w` contains no
  backslash-continued comment lines in that region anyway.
- `lines++` once per call matches `rdcmnt()`'s per-newline convention (a line comment consumes
  exactly one newline).
- The authoritative dynamic check: branch-wart run on branch-`ckcpro.w` produces a `ckcpro.c`
  identical (modulo the banner) to old-wart on old-`ckcpro.w`, and `ckcpro.o`'s machine code is
  byte-identical (§2).

## 5. Hazard scan

The dangerous failure class for a `/*`→`//` conversion is a converted `//` line ending in a
backslash: phase-2 splicing folds the *next* physical line into the comment. The branch tree
contains **9 such lines across 6 files — all verified inert** (in every case the spliced-in
next line is itself comment text or blank; `cat -A` confirmed the `\` is the true last
character in each):

| Location (branch) | Why it is harmless |
|---|---|
| `ckucon.c:299`, `ckucns.c:297` | next line is already a `//` comment line |
| `ckucmd.c:7314–7315` | next line is comment; real code two lines below is unaffected |
| `ckcnet.c:1109–1112` | 4-line `COPTS` build-flags block, every line `//`-prefixed |
| `ckcnet.c:1582` | splices a blank line; following code line unaffected (matches the original 2-line `/* */` comment on main) |
| `ckcdeb.h:1701–1702` | `CK_SLEEPINT` value stays `250`; both continuation lines are pure comment |
| `ckcdeb.h:2502–2503` | next real directive is on an unspliced line |

**Maintenance tripwire:** these 9 lines are latent hazards — a future edit that puts real code
on the line directly after one of them, or removes a leading `//` from a continuation line,
would silently comment code out (or expose comment text as code). They are inert today only
because every spliced line happens to also be a comment.

No instance was found anywhere of real code swallowed into a comment or comment text exposed
as code. `//` comments inside multi-line `#define` continuations and literals containing
comment markers were also scanned; nothing live was found.

## 6. Secondary checks

### 6.1 clang-format stability regression (follow-up recommended)

The repo's commit gate requires `clang-format FILE | diff FILE -` to be empty. Checked with
clang-format 21.1.8 on all 44 changed files:

- `main`: **0/44 unstable**.
- branch: **11/44 unstable** — `ckcfn2.c`, `ckcfns.c`, `ckucon.c`, `ckudia.c`, `ckufio.c`,
  `ckutio.c`, `ckuus3.c`, `ckuus5.c`, `ckuus6.c`, `ckuus7.c`, `ckuusy.c`.

All diffs are cosmetic (trailing-`//` column alignment, single line-break placement) and are
already covered by the token-identical proof — zero functional risk. Ten of the eleven converge
after a single pass; only `ckufio.c` truly oscillates, which is the pre-existing documented
`ReflowComments` quirk (see the format-and-editor-tooling skill). **Recommendation:** run
`clang-format -i` once over the ten convergent files on the branch before merging, purely for
hygiene against the format-stability gate. Not a correctness blocker.

### 6.2 macOS/BSD (`#ifdef`) coverage

`make linux` does not compile the `BSD44`/`MACOSX` regions, but the whole-file token proof in
§3 covers them textually (dead or live, every token was compared). As a corroborating
spot-check, `gcc -fsyntax-only` with the `macos` target's defines on `ckutio.c`, `ckcnet.c`,
`ckuxla.c` produced identical outcomes on both trees (identical error kind/count/messages from
the missing macOS SDK headers, shifted only by the known line-count deltas; `ckuxla.c` clean on
both).

## 7. Conclusion

The `wart-cpp-comment` branch is a behavior-preserving comment-style conversion, proven at the
strongest applicable level of the repo's verification methodology: byte-identical executable
code across the whole of `wermit`, with the only differences being the intentional wart version
banner (2.17 → 2.18) and the intentional `rdcmns()` addition to the wart build tool, both of
which were reviewed and found correct. Before merging, a single `clang-format -i` pass over the
ten convergent files in §6.1 is recommended to restore the format-stability gate, and the nine
backslash-terminated `//` lines in §5 are worth knowing about as future-edit tripwires.

## Appendix: verification artifacts

Performed in session-temporary worktrees (since removed):
`SOURCE_DATE_EPOCH=1750000000 make linux` clean builds of both branches; `cmp`/`cmp -l` on
`wermit` and all `.o`; `objdump -d`/`-s` diffs of `ckcpro.o` and `wermit`; comment-stripping
token comparison of the generated `ckcpro.c`; a phase-2/phase-3-faithful Python comment
stripper over all 43 source files with per-file literal-token lists; `grep -rnE '//.*\\$'`
hazard scan; per-file line-count deltas; clang-format 21.1.8 stability checks on both trees;
`gcc -fsyntax-only` macOS-define spot-checks; runtime `show version` smoke test of both
binaries.
