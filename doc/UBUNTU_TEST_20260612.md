# Ubuntu / GCC 15 Build Test: `make linux+ssl`

**Date:** 2026-06-12
**Purpose:** Test the standard Linux SSL build target (`make linux+ssl`) of C-Kermit 10.0 Beta.12 on Ubuntu with GCC 15, and analyze the resulting failure.

**Status:** Initial build **FAILED**; root cause identified; the recommended fix (Option 1, `-std=gnu17`) was **applied and verified** — `make linux+ssl` now builds cleanly to a working `wermit` binary. See [Resolution](#resolution-applied).

## Environment

| Component | Version / Detail |
|-----------|------------------|
| OS / kernel | Ubuntu, Linux 7.0.0-22-generic, x86_64 |
| Compiler | gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0, invoked as `cc` |
| Default C standard | C23 (`__STDC_VERSION__` = `202311L`); makefile passes **no** `-std` flag |
| OpenSSL | 3.5.5 (27 Jan 2026), headers in `/usr/include/openssl` |
| Dev libraries present | PAM, zlib, shadow, ncurses, crypt — all resolved |

Build log: `/tmp/kermit_build.log`. Make chain: `linux+ssl` (makefile:7000) → `linux` (makefile:6524) → `linuxa` (makefile:6484) → `ckuus4.o` (makefile:1028).

## Result: BUILD FAILED

The build progressed almost to completion. `wart` was built, `ckcpro.c` was generated, and every `ckc*` / `cku*` module compiled successfully **except one**. The build stopped at the very first compile error, in `ckuus4.o`, with exactly **4 errors**, all sharing a single root cause. No SSL, PAM, zlib, shadow, or other dependency failed — those were entirely resolved (see note below).

### Failing diagnostics (from `/tmp/kermit_build.log`)

```
ckuus4.c: In function 'transmit':
ckuus4.c:2312:12: error: assignment to 'void (*)(void)' from incompatible pointer type
  '__sighandler_t' {aka 'void (*)(int)'} [-Wincompatible-pointer-types]
   2312 |     oldsig = signal(SIGINT, trtrap);

ckuus4.c:2755:19: error: passing argument 2 of 'signal' from incompatible pointer type
  [-Wincompatible-pointer-types]
   2755 |     signal(SIGINT,oldsig);
        expected '__sighandler_t' {aka 'void (*)(int)'} but argument is of type 'void (*)(void)'

ckuus4.c: In function 'xlate':
ckuus4.c:2837:12: error: assignment to 'void (*)(void)' from incompatible pointer type ... (same)
ckuus4.c:2995:19: error: passing argument 2 of 'signal' from incompatible pointer type ... (same)
```

## Root Cause

Two independent, recent compiler/language changes combine to turn previously-accepted code into a hard error:

### 1. C23 redefines the empty parameter list `()`

In `ckuus4.c`, `oldsig` is declared twice as a K&R-style function pointer with an **empty parameter list**:

- `ckuus4.c:2137` — `SIGTYP (* oldsig)();` (in `transmit`)
- `ckuus4.c:2791` — `SIGTYP (* oldsig)();` (in `xlate`)

`SIGTYP` expands to `void` via this path in `ckcdeb.h`:

- The Linux build defines `-DPOSIX` (see the `linuxa` CFLAGS).
- `ckcdeb.h:1561` — `#ifdef POSIX` → `#define SIG_V`.
- `ckcdeb.h:1619-1621` — `#ifdef SIG_V` → `#define SIGTYP void`.

So each declaration is effectively `void (* oldsig)();`.

Under the **pre-C23** rules the project was written for, `()` means "unspecified arguments" (the classic K&R form), and the pointer is compatible with any function type. Under **C23** (GCC 15's default), `()` is now equivalent to `(void)`, so `oldsig` has the concrete type `void (*)(void)`.

The signal handler `trtrap` is genuinely a one-argument function:

- `ckuus4.c:1194` — `trtrap(int foo)` (the `CK_ANSIC` branch).

And `signal()` on this glibc is `__sighandler_t signal(int, __sighandler_t)` where `__sighandler_t` is `void (*)(int)` (`/usr/include/signal.h:72`). Therefore:

- `oldsig = signal(SIGINT, trtrap);` assigns a `void (*)(int)` into a `void (*)(void)` — incompatible.
- `signal(SIGINT, oldsig);` passes a `void (*)(void)` where `void (*)(int)` is expected — incompatible.

### 2. GCC 14+ promotes `-Wincompatible-pointer-types` to an error

Historically this mismatch was a warning. GCC 14 and later (so GCC 15 here) treat `-Wincompatible-pointer-types` as an **error by default**. The combination of the C23 `()` semantics and the warning-as-error promotion is what stops the build; under any earlier GCC default this would have compiled (possibly with warnings).

## Other At-Risk Sites

`ckuus4.c` failed first only because it is the first translation unit (in compile order) that both declares an empty-paren signal-handler pointer **and** assigns/passes it against `signal()`. The same pattern is widespread in the tree, and these sites will fail next once `ckuus4.c` is fixed in isolation. Confirmed sites that declare `SIGTYP (*...)()` (i.e. `void (*)(void)` under C23) **and** are used with `signal()`:

| File | Declaration | Used with `signal()` |
|------|-------------|----------------------|
| `ckuus4.c` | 2137, 2791 `oldsig` | 2312, 2755, 2837, 2995 (the failing site) |
| `ckuus6.c` | 3788 `oldsig` | 3882 (`= signal`), 4146 |
| `ckudia.c` | 4025 `savalrm`, 4026 `savint` | 4266, 4267, 5323, 5331, 6398, 6401 |
| `ckuscr.c` | 472 `savealm` | 524 (`= signal`), 553, 559 |
| `ckufio.c` | 7382 `istat`, `qstat`; 15851 `istat`, `qstat` | 7386, 7387, 7396, 7397, 15854-15872 |
| `ckutio.c` | 1292-1308 `saval`/`savquit`/`savpipe`/`savdanger`/`jchdlr`; 1720 `occt`; 9764 `osigint`; 14962 `save_sigchld` | 2118-2211, 2753-3758, 9766, 10006, 10880, 11122, 11580, 13285, 14471, 14974-15872 (many) |

Related empty-paren declarations that are part of the same class (typedefs / prototypes, fix them if doing a source-level pass):

- `ckcdeb.h:4354` — `SIGTYP (*signal())();` (Kermit's own `signal` prototype, also empty-paren).
- `ckcsig.h:19-20` — `typedef SIGTYP (*ck_sigfunc)();`, `(*ck_sighand)();`.
- `ckusig.h:19-20` — `typedef VOID (*ck_sigfunc)();`, `(*ck_sighand)();`.
- `ckcftp.c:917` — `typedef sigtype (*sig_t)();`.
- `ckutio.c:9820/9825` — `conint(f,s) SIGTYP (*f)(), (*s)();` (K&R definition).

The breadth of this list is the key finding: a piecemeal source fix would have to touch many files across the vendored upstream tree, and would have to be repeated as each newly-fixed file exposes the next one.

## Remediation Options

### Option 1 — Add `-std=gnu17` to the Linux target CFLAGS (recommended)

Compile with a pre-C23 dialect so `()` keeps its "unspecified arguments" meaning and the type mismatch disappears at the language level. `gnu17` (or `gnu11`) preserves the GNU extensions the makefile already relies on (`-funsigned-char` behavior, etc.) while restoring the C semantics the codebase was written for.

- **Correctness:** Restores the exact semantics the source assumes; no behavioral change to the program.
- **Invasiveness:** One-line makefile change, zero source edits.
- **Fidelity to upstream:** Highest — the source stays byte-identical to upstream; only the local build recipe diverges. Matches the project's stated "maximally portable, pre-ANSI-friendly C" philosophy.
- **Scope note:** Adding it to the `linuxa` CFLAGS fixes the whole Linux build (all sub-targets, including `linux+ssl`, route through `linuxa`). It does not affect other platform targets. A project-wide application would be even broader but is unnecessary to unblock this build and would be a larger divergence; scoping to the Linux target is the minimal change.

### Option 2 — Add `-Wno-error=incompatible-pointer-types`

Downgrade the diagnostic from error back to warning.

- **Correctness:** Weaker. The types are still genuinely incompatible under C23; you are silencing a real mismatch rather than making the code well-typed. Other C23-vs-K&R differences would remain latent.
- **Invasiveness:** One-line makefile change.
- **Fidelity:** Leaves the build compiling under C23 semantics it was not written for. Not recommended as the primary fix.

### Option 3 — Fix the source declarations (e.g. `SIGTYP (* oldsig)(int);`)

Give every signal-handler pointer an explicit `(int)` prototype.

- **Correctness:** Most correct in the absolute sense — the code becomes well-typed under any standard.
- **Invasiveness:** High. As the at-risk table shows, there are many sites across `ckuus4.c`, `ckuus6.c`, `ckudia.c`, `ckuscr.c`, `ckufio.c`, `ckutio.c`, plus the typedefs/prototypes in the `ck*sig.h` headers and `ckcdeb.h:4354`. This is a broad edit of a vendored upstream tree and risks diverging from / conflicting with future upstream updates.
- **Fidelity:** Lowest — substantial local modification of upstream source.

## Recommendation

**Adopt Option 1**: add `-std=gnu17` to the `linuxa` target's CFLAGS. It is the lowest-risk change, requires no edits to vendored source, preserves upstream fidelity, and is consistent with the project's pre-ANSI-portable philosophy. Investigation found nothing that `-std=gnu17` would break: the makefile relies on `-funsigned-char` and GNU extensions, all of which `gnu17` retains. Because every Linux sub-target funnels through `linuxa`, this single change fixes `make linux+ssl` and the other Linux targets at once.

### Exact makefile change

In `makefile`, the `linuxa` target (line 6480) builds the CFLAGS string starting at the `$(MAKE) xermit` invocation on line 6484. Append `-std=gnu17` to that CFLAGS block. Current (lines 6485-6488):

```make
	"CFLAGS = -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
	-DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX \
	-DUSE_STRERROR $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(LIBS) -lm"
```

Proposed (add `-std=gnu17` on the first CFLAGS line):

```make
	"CFLAGS = -std=gnu17 -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
	-DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX \
	-DUSE_STRERROR $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(LIBS) -lm"
```

## Resolution (Applied)

Option 1 was applied to the `linuxa` target in `makefile` (line 6485): `-std=gnu17` was prepended to the CFLAGS string.

```diff
-	"CFLAGS = -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
+	"CFLAGS = -std=gnu17 -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
```

A clean rebuild (`rm -f wermit; make clean; make linux+ssl`) then succeeded:

| Check | Result |
|-------|--------|
| `make linux+ssl` exit code | `0` |
| Compile errors | `0` (previously 4, all in `ckuus4.o`) |
| Link step | Completed — produced `wermit` |
| `make check` | `SUCCESS:` |
| Binary | `wermit`, ELF 64-bit x86-64, dynamically linked, ~3.0 MB |
| `./wermit -V` | `C-Kermit 10.0.416 Beta.12, 2025/03/22, for Linux+SSL (64-bit)` |

The only remaining compiler output is benign OpenSSL 3.0 *deprecation notes* (e.g. `RSA_free`), not errors — expected for this codebase against OpenSSL 3.x and unrelated to the failure or the fix. All previously failing sites in `ckuus4.c` and the other at-risk files now compile, confirming that the `-std=gnu17` dialect switch resolves the empty-paren `()` semantics for the whole Linux build (all sub-targets route through `linuxa`) without any source edits.

## Note on Dependencies (not the problem)

SSL/TLS, PAM, zlib, and shadow all resolved correctly and were **not** implicated in the failure:

- The compile line included `-DCK_SSL -DCK_PAM -DZLIB -DCK_SHADOW -DOPENSSL_300` and `-I/usr/local/ssl/include`, and the link library list `-lssl -lcrypto -lpam -ldl -lz -lutil -lncurses -lresolv -lcrypt -lm` — no header-not-found or symbol errors appeared.
- All SSL-, PAM-, and crypto-dependent modules (e.g. `ckcmai.c`, `ckclib.c`, and the other `ck*` files compiled before `ckuus4.c`) built cleanly.
- The build never reached the link stage, but every compilation that ran succeeded apart from `ckuus4.o`.

The failure is purely a C23 language-semantics / GCC-15 warning-as-error interaction in `ckuus4.c`, not a missing or misconfigured dependency.
