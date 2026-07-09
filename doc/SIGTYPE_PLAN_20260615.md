# SIGTYP / SIGRETURN signal-type macro elimination — 2026-06-15

## Summary

Assume `SIG_V` is always set and `SIG_I` is always unset (true on every platform
this tree still supports: Linux, macOS, *BSD). Under that assumption the
signal-handler type machinery in `ckcdeb.h` is dead configurability:

- `SIGTYP` always expands to `void`
- `SIGRETURN` always expands to `return`

This change mechanically expanded both macros to the concrete tokens they always
resolve to and deleted the macros — the same "expand-and-remove" treatment already
applied to the `VOID`/`CKVOID`/`CONST` aliases and `_PROTOTYP()`.

## Why `SIG_V` is always selected

`ckcdeb.h` chose the signal type with:

```c
#ifndef SIG_V
#ifndef SIG_I
#ifdef POSIX
#define SIG_V
#else
#ifdef SVR3            /* System V R3 and later */
#define SIG_V
#else
#define SIG_I
#endif
#endif
#endif
#endif
```

Every supported platform defines `POSIX` (Linux build passes `-DPOSIX`; macOS/BSD
reach it via the `MACOSX10 → MACOSX → BSD44 → SVR4 → SVR3/ATTSV` implication chain
and also define `POSIX`). So `SIG_V` is always defined and `SIG_I` never is.
Verified by preprocessing the original `ckcdeb.h`:

- Linux flags → `SIGTYP=void`, `SIGRETURN=return`, `SIG_V` defined, `SIG_I` not.
- `-DBSD44` with each of `-DMACOSX10/-D__OpenBSD__/-D__NetBSD__/-DFREEBSD`
  → `SIGTYP=void`, `SIGRETURN=return`, `SIG_V` defined, `SIG_I` not.

## Changes

1. **Mechanical token expansion** (comment/string-aware, exact identifier):
   - `SIGTYP` → `void` (handler return type) and `SIGRETURN` → `return`
     across `ckufio.c ckusig.c ckcnet.c ckuscr.c ckucon.c ckuusy.c ckutio.c
     ckuus6.c ckuus4.c ckuusx.c ckudia.c ckcker.h ckcfnp.h ckcsig.h ckcnet.h`,
     plus the two prototypes in `ckcdeb.h` (`ckntsignal`, `conint`).
   - e.g. `SIGTYP timerh(int);` → `void timerh(int);`, `SIGRETURN;` → `return;`,
     `typedef SIGTYP (*ck_sigfunc)(void *);` → `typedef void (*ck_sigfunc)(void *);`.

2. **`ckcftp.c` / `ckcftp.h` lowercase `sigtype` alias** removed: deleted
   `#define sigtype SIGTYP` and expanded every lowercase `sigtype` token to `void`
   (declarations, definitions, the `typedef void (*sig_t)(int);`, and the K&R
   `pscancel` under `#ifdef FTP_PROXY`).

3. **`ckcdeb.h`**: deleted the whole "Signal type" block (the `SIG_V`/`SIG_I`
   selection, the `#ifdef SIG_I`/`#ifdef SIG_V` `SIGTYP`/`SIGRETURN` definitions,
   and the `#ifndef SIGTYP`/`#ifndef SIGRETURN` fallbacks), and the now-pointless
   `#ifndef SIG_V / #define SIG_V` lines inside the dead `#ifdef __DECC` block.

4. **`ckuus5.c`**: removed the `#ifdef SIG_I … "SIG_I"` and `#ifdef SIG_V … "SIG_V"`
   `SHOW FEATURES` entries (they reference macros that no longer exist).

5. **Stale prose comments** in `ckuusy.c` ("`SIGTYP` is typedef'd in ckcdeb.h")
   trimmed. The historical discussions in `ckcftp.c` (the `sig_t`/`SIGTYP` essay)
   and `ckcnet.c` (an obsolete OSK build-option example mentioning `-d=SIG_V`) were
   left as historical prose.

No `wart`/`ckcpro.c` regeneration was needed — `ckcpro.w` has no `SIGTYP`/`SIGRETURN`.

## The only behavior change

`SHOW FEATURES` no longer lists `SIG_V`. Previously `SIG_V` was defined, so the
feature string was emitted; with the symbol removed it is gone. This was the
explicitly accepted user-visible change.

## Verification

The build is deterministic on this machine with a fixed `SOURCE_DATE_EPOCH` **after
a `make clean`** (a non-clean rebuild keeps stale `.o` files and is not comparable).
Verified both the original and the modified trees rebuild bit-for-bit reproducibly.

Equivalence was proven at the **object-file** level, which isolates the intended
behavior change from the mechanical expansion:

- Clean-built the original and the modified tree and compared every `*.o`.
- **Every object file is byte-identical except `ckuus5.o`.** All the
  `SIGTYP`/`SIGRETURN`/`sigtype` expansions therefore produce exactly the
  preprocessor's own output — byte-identical object code on Linux (and, by the
  preprocessor checks above, equivalent on the macOS/BSD `#ifdef` paths).
- `ckuus5.o` differs **solely** because of the intentional `SHOW FEATURES` removal
  (`ckuus5.c` contains no `SIGTYP`/`SIGRETURN`). Its small size change shifts
  downstream addresses/relocations, which is why the *whole* binary is not
  byte-identical — by design, not a regression.

`make linux` builds cleanly and `make check` reports SUCCESS. A final
`grep -nw 'SIGTYP\|SIGRETURN\|sigtype\|SIG_V\|SIG_I'` over `*.c *.h *.w` returns
only the two intentionally-kept historical prose comments. All edited headers
remain `clang-format`-clean (the one pre-existing `ckcnet.h` Kerberos
misalignment was left untouched as unrelated).
