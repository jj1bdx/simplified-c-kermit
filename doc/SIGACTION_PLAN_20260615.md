# Replace save/restore `signal()` with `sigaction()` via `ck_signal()` — 2026-06-15

## Summary

C-Kermit saved and restored the previous signal disposition with the classic
`old = signal(SIG, handler); ...; signal(SIG, old);` idiom at ~70 call sites
across 8 files. Classic `signal()` has historically unreliable cross-platform
semantics (System V one-shot reset vs. BSD persistent handler; differing
syscall-restart behavior). The original author already noted (ckcftp.c:574-583)
that `sigaction()` is the correct POSIX replacement, avoided only for pre-POSIX
portability — a constraint this Linux/macOS/*BSD-only tree has dropped.

This change introduces a single drop-in wrapper, **`ck_signal()`**, implemented
with `sigaction()`, and converts only the **save/restore** call sites to use it.
Non-saving installs (`signal(SIG, SIG_IGN/SIG_DFL)` and installs of Kermit's own
handlers passed as arguments) are left as plain `signal()` — they have no previous
handler to capture, so they are outside the stated goal.

## Why a wrapper (and why unconditional)

- **Minimal, auditable churn.** Every saved value stays a `void (*)(int)`, so all
  existing inspections keep working **unchanged**: `== SIG_IGN`, `== SIG_DFL`,
  `if (savhandler)`, `oldintr != cmdcancel`, `(*oldintr)(SIGINT)`. The only genuinely
  new logic (the sigaction flags/mask) lives in one reviewable function. The
  alternative — converting each saved variable to `struct sigaction` and rewriting
  every comparison and the `ckcftp.c` struct fields — would be large, error-prone,
  rippling hand-edits.
- **Unconditional, not gated on `CK_POSIX_SIG`.** Critical finding: `CK_POSIX_SIG`
  is defined **only on the Linux build** (`makefile` `linuxa` target); the `macos`
  target and the modern-BSD targets do **not** define it, and nothing in `ckcdeb.h`
  implies it. So the existing `#ifdef CK_POSIX_SIG` `sigaction` idiom in `ckupty.c`
  compiles on Linux only. Gating the new code on `CK_POSIX_SIG` would have left
  macOS on classic `signal()`. `sigaction()` is POSIX.1-1988 and present on every
  surviving target (Linux, macOS, FreeBSD, NetBSD, OpenBSD, MirBSD), so the wrapper
  is unconditional and covers both Linux and macOS. (`ckupty.c` already calls
  `sigaction()` unconditionally in `ptyint_vhangup`, proving it links everywhere.)

## The wrapper

Added to `ckusig.c` (the signal module, linked into every target, already
`#include <signal.h>`):

```c
void (*ck_signal(int sig, void (*func)(int)))(int) {
  struct sigaction sa, old;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = func;
  if (sigaction(sig, &sa, &old) < 0)
    return SIG_ERR;
  return old.sa_handler;
}
```

`SA_RESTART` + empty mask reproduces the BSD/glibc/macOS `signal()` semantics the
code already relied on (persistent handler, interrupted syscalls auto-restarted),
making this a **behavior-preserving** drop-in. The prototype
`void (*ck_signal(int, void (*)(int)))(int);` was added to `ckcdeb.h` (the master
header included everywhere) so the many caller files need no new include; the bare
function-pointer form needs no `<signal.h>` types, so it is safe there.

### SIGALRM timeout handlers and SA_RESTART

The SIGALRM timeout handlers (`ckutio.c` `timerh`/`xtimerh`/`catch`, `ckusig.c`
`alrm_execute`/`cc_alrm_execute`, `ckudia.c` `dialtime`) abort their blocking call
by `longjmp`/`siglongjmp`-ing out of the handler, not by relying on `EINTR`, so
`SA_RESTART` is irrelevant to them — and they already ran under `SA_RESTART` today
(glibc `signal()`), so behavior is unchanged. Verified at runtime: `PAUSE 2` takes
exactly 2.00s and Ctrl-C aborts a `PAUSE` (see Verification).

## Converted sites

`signal(` → `ck_signal(` at save and matching restore calls only:

| File       | Saved handlers converted |
|------------|--------------------------|
| `ckusig.c` | `savhandler` (×2, alrm_execute/cc_alrm_execute) |
| `ckufio.c` | `istat`, `qstat` (zshcmd shell-out) |
| `ckuus6.c` | `oldsig` (dotype) |
| `ckuus4.c` | `oldsig` (×2) |
| `ckuscr.c` | `savealm` (1 save, 2 restores) |
| `ckudia.c` | `savalrm`, `savint` |
| `ckutio.c` | `savquit`, `savpipe`, `savdanger`(`#ifdef SIGDANGER`), `savusr1`, `savusr2`, `jchdlr`, `occt` (6 restores), `saval` (8 saves), `osigint` (probe), `save_sigchld`, `istat`/`qstat` |
| `ckcftp.c` | `oldintr`/`oldintp` and the `ftpsnd.*`/`ftprecv.*` struct fields (48 sites) |

**Left as plain `signal()` (non-saving / not a saved old handler):** the
`signal(SIG, SIG_IGN/SIG_DFL)` installs in `ckutio.c` (e.g. 1353-1391, 6722-7034,
10156), `ckcftp.c` (10032, 11080), `ckucns.c`, `ckupty.c`, `cku2tm.c`,
`ckuusy.c`, `ckcnet.c`, `ckuusx.c`; the SIGALRM re-arm calls in `ckutio.c`
(`signal(SIGALRM, xtimerh)` at 2640/2655); and `conint()`'s installs of its
handler-function arguments `f`/`s` and the `esctrp` reinstall (`ckutio.c` 6950-7392).
Guards (`#ifdef SIGDANGER`/`SIGPIPE`/`SIGUSR1`/`SIGUSR2`/`SIGTSTP`) were preserved
exactly. `CK_POSIX_SIG` was neither added nor removed. The historical
`signal()`/`sigaction()` essay in `ckcftp.c:538-584` and the `#ifdef CKNTSIG`
`ckntsignal` block in `ckcdeb.h` were left untouched.

## Verification

Byte-identical `cmp` is **not** applicable — `sigaction()` emits different code than
`signal()`. Verification performed:

1. **Clean Linux build:** `make clean && rm -f wermit && make linux` — succeeds,
   no warnings on the converted files (`ckusig.c` also `-Wall`-clean for the
   wrapper). `make check` reports **SUCCESS**.
2. **macOS `#ifdef` path:** `cc -fsyntax-only -DMACOSX10 -DMACOSX103 ...`
   (no `-DLINUX`, no `CK_POSIX_SIG`) on `ckusig.c` — no `ck_signal`/`sigaction`/
   `SA_RESTART` errors on the macOS branch (remaining diagnostics are only the
   absence of macOS-only system headers on this Linux host, unrelated to this
   change). `sigaction`, `sigemptyset`, `SA_RESTART`, `SIG_ERR` are all POSIX and
   present on macOS.
3. **Runtime smoke tests** (built `wermit`):
   - Startup + script + exit clean (`echo`/`exit`, status 0) — `sysinit()`'s
     `ck_signal`-based handler installation works.
   - `PAUSE 2` → 2.00s wall clock — SIGALRM timer fires correctly under
     `SA_RESTART` (longjmp timeout path unaffected).
   - SIGINT sent ~1s into `PAUSE 10` → `^C...` then control returns to the command
     loop and runs the next command — the SIGINT save/restore path
     (`occt`/`cctrap`) works.

No automated test suite exists (per `CLAUDE.md`); verification is build + `make
check` + targeted manual runs. A macOS build (`make macos && make check`) should
be run on a Mac to confirm the now-unconditional path that previously took the
classic-`signal()` branch.
