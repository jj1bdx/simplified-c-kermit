# Enable clang-format `SortIncludes` and pin `<setjmp.h>` in ckcsig.h

Date: 2026-06-15

## What changed

Building on the previous commit (which pinned `#include "ckcdeb.h"` first in every
module with `/* clang-format off */ … /* clang-format on */`), this change enables
`clang-format`'s include sorting tree-wide:

1. **`.clang-format`**: `SortIncludes.Enabled` flipped `false → true` (and the
   "# DISABLE SortIncludes" header comment removed). Nothing else changed —
   `IncludeBlocks: Preserve`, `IncludeCategories`, and `InsertBraces: false` are
   unchanged.
2. **`clang-format` re-run across `*.c`/`*.h`**: contiguous (blank-line-delimited)
   `#include` blocks are now alphabetically sorted, plus incidental general
   reformatting (trailing-comment realignment, comment reflow) that clang-format
   applies in the touched regions.
3. **`ckcsig.h`**: added `#include <setjmp.h>` (it typedefs `ckjmpbuf` from
   `sigjmp_buf`/`jmp_buf`), wrapped in `/* clang-format off */ … /* clang-format on */`
   so it stays pinned.

## Why the `ckcsig.h` fix is required

`ckcsig.h` previously got `<setjmp.h>` only *transitively* — every includer happened
to pull `<setjmp.h>` in before `"ckcsig.h"`. With sorting enabled, in a contiguous
include block the quoted `"ckcsig.h"` (IncludeCategory Priority 1) sorts **before**
the angled `<setjmp.h>` (Priority 3). That broke the ordering in 5 modules —
`ckcmai.c`, `ckuscr.c`, `ckucon.c`, `ckcftp.c`, `ckuusx.c`. In `ckcmai.c`, `ckjmpbuf`
is used on the line right after the include, so without the fix it is a hard compile
error. Making `ckcsig.h` self-contained (it includes its own `<setjmp.h>` definer)
removes the dependency on caller include order entirely; the off/on wrap keeps the
include from being re-sorted out of place.

This was the **only** header that relied on transitive include order in a way sorting
exposed (verified — see Analysis). The `ckcfnp.h` "must be last" convention is now
cosmetically violated in a few TUs but is harmless: `ckcfnp.h` self-includes all its
dependencies (`ckcdeb.h`, `ckcker.h`, `ckucmd.h`, `ckuusr.h`), all multiple-inclusion
guarded.

## Verification

- **Whole-binary byte-identical (Linux path):** a fully-clean fixed-`SOURCE_DATE_EPOCH`
  `make clean && make wart && make ckcpro.c && make linux` of this working tree is
  `cmp`-identical to a clean build of the previous commit (HEAD). Comment reformatting
  is codegen-neutral (no `__LINE__`/`__FILE__` dependence; `InsertBraces` stays off),
  include reordering is order-independent on this path, and the explicit `<setjmp.h>`
  is a no-op where it was already transitively present. `make check`: SUCCESS.
- **macOS/BSD paths (c-expert analysis):** the only *live* (non-Linux-compiled)
  include reorders are `ckuusx.c` (`<curses.h>` now before `<term.h>` — strictly
  *safer*, since term.h needs curses types) and `ckufio.c` (`<sys/time.h>`/
  `<sys/timeb.h>` — independent). Reorders inside dead-platform guards
  (TRU64/IBMX25/HPX25/INTERLAN/STREAMSPTY/HAVE_SYS_LABEL_H) are not compiled on any
  surviving target.
- **Idempotency:** re-running clang-format does not move any include; the off/on
  islands around `ckcdeb.h` and `ckcsig.h`'s `<setjmp.h>` hold (verified on
  `ckcsig.h`). Note: `ckufio.c` and `ckcftp.c` have a pre-existing,
  SortIncludes-independent comment-reflow non-idempotency in their historical
  embedded-space comments — no include or codegen impact.

## Net effect

`SortIncludes` is now safe to keep enabled: `ckcdeb.h` is pinned first everywhere,
`ckcsig.h`'s `<setjmp.h>` is pinned and self-contained, and no other module depends on
transitive include order. `ckcpro.c`/`wart`/`wermit` remain gitignored build artifacts.
