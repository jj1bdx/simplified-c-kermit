# Ubuntu Build Test Report — `make linux` — 2026-06-13

C-Kermit 10.0 Beta.12 test repository, branch `main` at `61de8c3` (working tree clean).
Procedure: `make clean; rm -f wermit; make linux` (full log captured in `build_linux.log`),
followed by `make check` and an expert analysis pass (c-expert agent).

## Result: SUCCESS

- Build completed with exit status 0 and **zero compiler diagnostics** in the default build.
- `make check` reports SUCCESS; `wermit` produced (2,864,024 bytes, not stripped, PIE).
- Smoke test: `timeout 15 ./wermit -C "show version, exit" </dev/null` exits 0 and prints
  **C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux (64-bit)**; CONNECT module is
  `UNIX:select()` 10.0.143, confirming the select()-based `ckucns.c` was linked via the
  `xermit` target as expected.
- Note: a naive `grep -ci error build_linux.log` reports 30 hits — all are the
  `-DUSE_STRERROR` macro in compile lines, not errors.

## 1. Build Environment

| Component | Value |
|---|---|
| OS | Ubuntu 26.04 LTS (Resolute Raccoon) |
| Kernel | Linux 7.0.0-22-generic, x86_64, PREEMPT_DYNAMIC |
| Compiler | gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0 |
| C library | glibc 2.43 |
| ncurses | 6.6+20251231 (libncurses6 / libtinfo6, libncurses-dev) |
| libcrypt | libxcrypt 1:4.5.1 (`libcrypt.so.1.1.0`, `crypt.h` present) |

A very current toolchain. libcrypt is the standalone libxcrypt implementation (the modern
replacement after glibc dropped built-in crypt), so `-DHAVE_CRYPT_H` / `-lcrypt` resolve
correctly.

## 2. Build Configuration

The `linux` target (subtarget `linuxa`) compiled everything with:

```
cc -std=gnu17 -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG -DCK_NEWTERM
   -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX -DUSE_STRERROR -DCK_NCURSES
   -I/usr/include/ncurses -DHAVE_PTMX -DHAVE_CRYPT_H -DHAVE_OPENPTY
   -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DKTARGET=\"linux\"
```

Link line: `-lutil -lncurses -lresolv -lcrypt -lm`

Assessment for modern Ubuntu:

- **`-std=gnu17` is correct and load-bearing.** GCC 15 defaults to C23 (`-std=gnu23`),
  which turns several pre-ANSI idioms into hard errors (implicit `int` removal, `()`
  prototype semantics, `bool`/`true`/`false`/`nullptr` as keywords). The gnu17 pin
  (commit `24f0d2b`) is what keeps this tree building.
- **`-funsigned-char`** — appropriate; Kermit's byte handling assumes unsigned char.
- **`-D_FILE_OFFSET_BITS=64` / `-D_LARGEFILE_SOURCE`** — correct; effectively the
  default ABI on 64-bit glibc 2.43, harmless belt-and-suspenders.
- **`-DHAVE_CRYPT_H` + `-lcrypt`** — correct given libxcrypt-dev is installed; this is
  the right handling of the post-glibc crypt split.
- **`-DCK_NCURSES -lncurses`** — resolves against ncurses 6.6 without issue.
- **`-O`** (not `-O2`) — conservative, intentional for portability; fine.
- **No `-Wall` is passed by the makefile**, so the zero-warning default build is
  *silence, not verified cleanliness* (addressed by the spot-check below).
- **Minor:** `readelf -d wermit` shows `-lresolv` and `-lutil` are dropped by the
  linker — `res_*` and `openpty`/`forkpty` are satisfied directly by glibc 2.43. They
  are dead flags on modern glibc but harmless, and still needed on older libcs.

## 3. Warnings Spot-Check (`-Wall -Wextra`)

Eight representative modules (`ckcmai.c ckclib.c ckutio.c ckufio.c ckcfns.c ckcnet.c
ckuusr.c ckucns.c`) were recompiled with the exact build CFLAGS plus `-Wall -Wextra`
using `-fsyntax-only` (no object files or the shipped binary were touched). All eight
compile; every diagnostic is non-fatal. By category:

**Benign legacy noise (vast majority):** `-Wunused-parameter` in signal handlers and
fixed-prototype stubs; `-Wunused-variable`/`-Wunused-but-set-variable`/`-Wunused-label`
leftovers under `#ifdef` feature selection; `-Wmissing-field-initializers` on partial
struct initializers (e.g. `http_url` in `ckuusr.c`). Intentional/cosmetic.

**Worth a glance, not defects:**

- `-Wsign-compare` — classic K&R-era signed/unsigned comparisons (`pw_uid == ruid` in
  `ckufio.c`, repeat-count checks in `ckcfns.c`/`ckclib.c`, `?:` signedness in
  `ckutio.c` `ttinc`). Values are small/non-negative in practice; no real bug, but the
  category most likely to hide an off-by-sign issue if ranges ever change.
- `-Wparentheses` — many `&&` within `||` in `ckcnet.c`/`ckcfns.c` and the `TELCMD_OK`
  macro in `ckctel.h`. Precedence is correct as written; readability nit only.
- `-Wmisleading-indentation` — two sites, `ckcnet.c:3258` (`netopen` bind/errno) and
  `ckucns.c:799` (`ckcgetc`). Both were inspected and the control flow is correct; the
  indentation is just misleading. These are the only findings worth a human glance.

**Dangerous warnings: none.** No `-Wimplicit-function-declaration`, `-Wint-conversion`,
`-Wincompatible-pointer-types`, or implicit-`int` anywhere in the sample — the four
categories GCC 15 promotes to hard errors under C23. Their absence confirms `-std=gnu17`
is providing the older language semantics the code needs, **not** papering over latent
type-safety errors.

## 4. Binary Sanity

- `file`: ELF 64-bit LSB PIE executable, x86-64, dynamically linked, for GNU/Linux
  3.2.0, not stripped.
- `ldd`: clean — `libncurses.so.6, libtinfo.so.6, libcrypt.so.1, libm.so.6, libc.so.6`
  only; no missing or surprising dependencies.
- Functional: version smoke test passes (see Result above); FTP, Telnet, and network
  support confirmed present in the version block.

## 5. Conclusions and Recommendations

**Overall: healthy, fully functional build** on Ubuntu 26.04 / GCC 15 / glibc 2.43.

1. **Keep the `-std=gnu17` pin** — it is the single thing standing between this tree
   and hard C23 errors under GCC 15's defaults. Already documented; do not drop it.
2. The default build's zero warnings reflect the absence of `-Wall`, not audited
   cleanliness. A `-Wall -Wextra` build surfaces only benign legacy-style noise in the
   sampled modules; not actionable for a portability-first codebase.
3. If anything is worth a one-line upstream cleanup, it is the two
   `-Wmisleading-indentation` sites (`ckcnet.c:3258`, `ckucns.c:799`) — verified
   correct, but the pattern merits a reindent or comment.
4. `-lresolv -lutil` are inert on glibc 2.43; retain them for portability to older
   libcs.

---
*Build log: `build_linux.log` (untracked). No source, makefile, object files, or the
`wermit` binary were modified by the analysis.*
