# Fix `signal()` handler type for C23 — 2026-06-15

Commit `8c8e07f` ("Fix signal() type as in C23, apply clang-format"), 22 files,
220 insertions / 234 deletions.

## Problem

C-Kermit declared its saved signal-handler function pointers with an **empty
parameter list**, e.g.

```c
static void (*saval)() = NULL;   /* For saving alarm() handler */
void (*occt)();                  /* For saving old SIGINT handler */
void (*savhandler)(int);         /* OK */
```

Under K&R / C17 (`-std=gnu17`) an empty `()` means *"unspecified arguments"*, so
assigning the result of `signal(SIGALRM, handler)` — whose type is
`void (*)(int)` — to a `void (*)()` was accepted.

**C23 changes the meaning of `()` to `(void)`** (a function taking *no*
arguments). A `void (*)(void)` is **not** compatible with `signal()`'s
`void (*)(int)`, so building under a C23-default compiler now produces
incompatible-pointer errors/warnings at every `savptr = signal(...)` assignment
and every `signal(sig, savptr)` restore.

## Fix

Two coordinated changes:

### 1. Build under the compiler's default standard (now C23)

`makefile`, `linuxa` target — drop the pinned `-std=gnu17` so the build uses the
toolchain's default C standard (C23 on a current gcc):

```make
-"CFLAGS = -std=gnu17 -O -DLINUX -pipe -funsigned-char -DFNFLOAT ...
+"CFLAGS = -O -DLINUX -pipe -funsigned-char -DFNFLOAT ...
```

### 2. Give every signal-handler pointer an explicit `(int)` prototype

Each empty-paren handler pointer was changed from `()` to `(int)` so it matches
the C23 type of `signal()` and the handlers themselves (which already take an
`int` arg). 18 declarations across 6 files:

| File       | Pointers fixed `()` → `(int)`                                            |
|------------|--------------------------------------------------------------------------|
| `ckutio.c` | `saval`, `savquit`, `savpipe`, `savdanger`, `jchdlr`, `occt`, `osigint` (9) |
| `ckusig.c` | `savhandler` (×2)                                                        |
| `ckuscr.c` | `savealm`                                                                |
| `ckufio.c` | `save_sigchld`                                                           |
| `ckuus4.c` | `istat`, `qstat`, `oldsig` (×2)                                          |
| `ckuus6.c` | `oldsig`                                                                 |

Example:

```c
-static void (*saval)() = NULL;    /* empty parens: C23 = (void), wrong */
+static void (*saval)(int) = NULL; /* matches void(*)(int) from signal() */

-    void (*istat)(), (*qstat)();
+    void (*istat)(int), (*qstat)(int);
```

This is the substance of the fix and is what makes the tree compile under C23.

## Also in this commit: clang-format

The same commit ran `clang-format` over the touched `.c` files, which accounts
for most of the line churn (it touches 22 files but only the 6 above carry
signal-type changes). Purely cosmetic deltas include:

- spacing normalization: `void(*savhandler)(int)` → `void (*savhandler)(int)`,
  `int(*xx_ok)(int,int)` → `int (*xx_ok)(int, int)`, `CHAR(*sxo)(CHAR)` →
  `CHAR (*sxo)(CHAR)`;
- cast spacing: `(void) signal(...)` → `(void)signal(...)`,
  `(void) gettimeofday(...)` → `(void)gettimeofday(...)`;
- trailing-comment re-alignment and reflowing of long block comments;
- collapsing split K&R definition headers onto one line, e.g.
  `void genbrk(fn, msec)\nint fn, msec;` → `void genbrk(fn, msec) int fn, msec;`.

These reformatting changes are behavior-neutral; only the `()` → `(int)` edits
and the `-std=gnu17` removal affect compilation.

## Follow-up note

`.clangd` still pins `-std=gnu17` (line 6) to mirror the old `CFLAGS`. With the
makefile now building under the compiler default, `.clangd` should be updated to
match (drop `-std=gnu17`, or set it to the standard the build actually uses) so
the editor's clangd evaluates the same language rules as the real build.
