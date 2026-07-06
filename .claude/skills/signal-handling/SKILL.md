---
name: signal-handling
description: Signal-handling conventions - the ck_signal() sigaction wrapper (SA_RESTART, unconditional, NOT gated on CK_POSIX_SIG), the ck_sig_t handler-pointer typedef, which calls deliberately remain plain signal(), the SIGALRM longjmp timers, and the runtime signal test battery (PAUSE 2 = 2.00s, SIGINT mid-PAUSE). Use when touching signal handlers, ck_signal/ck_sig_t, timers, or anything under <signal.h>.
---

# Signal handling in this tree

The signal code was modernized in three steps on 2026-06-15/16. The conventions below are what
new code must follow.

## ck_signal() — the sigaction wrapper

The classic `old = signal(SIG, h); …; signal(SIG, old);` save/restore idiom (~70 sites, 8
files) was replaced by `ck_signal()` in `ckusig.c` (prototype in `ckcdeb.h`, so callers need no
new include). It calls `sigaction()` with `sigemptyset(&sa.sa_mask)` + `sa.sa_flags =
SA_RESTART` and returns `old.sa_handler` — faithfully reproducing glibc/BSD/macOS `signal()`
(persistent handler, restarted syscalls), so it is a drop-in. See `SIGACTION_PLAN_20260615.md`;
macOS verified on real hardware (`SIGACTION_MACOS_20260615.md`).

- **The wrapper is unconditional — NOT gated on `CK_POSIX_SIG`**, which is defined only on the
  `linux` makefile target (not `macos`, not the BSD targets, not implied in `ckcdeb.h`).
  Gating on it would leave macOS on classic `signal()`. `sigaction()` is POSIX.1-1988 and
  present on every surviving target.
- Use `ck_signal()` for any **save/restore** of a handler. Saved values are `ck_sig_t`, so all
  the existing inspections (`== SIG_IGN`, `== SIG_DFL`, `if (savhandler)`,
  `(*oldintr)(SIGINT)`) work unchanged.
- **Deliberately left as plain `signal()`** (no prior handler captured — do not "fix"): the
  `signal(SIG, SIG_IGN/SIG_DFL)` installs, the SIGALRM re-arm calls in `ckutio.c`, and
  `conint()`'s installs of its handler-function arguments `f`/`s` plus the `esctrp` reinstall.
- The SIGALRM timeout handlers (`timerh`/`xtimerh`/`catch`, `alrm_execute`, `dialtime`) abort
  via `longjmp` out of the handler, not `EINTR`, so `SA_RESTART` does not affect them.
- Preserve the `#ifdef SIGDANGER/SIGPIPE/SIGUSR1/SIGUSR2/SIGTSTP` guards around signal sites.

## ck_sig_t — the handler-pointer typedef

`typedef void (*ck_sig_t)(int);` lives in `ckcdeb.h` (just above the `ck_signal` block). Every
verbatim `void (*NAME)(int)` signal-handler spelling was converted to it (~25 sites; also
`ckcsig.h`'s `typedef ck_sig_t ck_sighand;`). It is structurally identical to
`void (*)(int)` — pure source spelling, codegen-neutral (all 29 objects byte-identical). Use
`ck_sig_t` for new handler variables/params/returns. See `CK_SIG_T_20260616.md`.

**Left unconverted, correctly:** `ck_sigfunc` (`void (*)(void *)` — different signature),
`ckcftp.c`'s own `sig_t` typedef (deliberately mirrors the platform `sig_t`; that module is
untouched) and its historical `signal()`/`sigaction()` essay at `ckcftp.c:538-584`, and the
inactive `#ifdef CKNTSIG` `ckntsignal` block in `ckcdeb.h`.

## Removed history (context for reading old diffs)

- `SIGTYP`/`SIGRETURN` (and `ckcftp.c`'s lowercase `sigtype`) were expanded to
  `void`/`return` and removed from `ckcdeb.h`, on the proven premise that the old selector
  always picked `SIG_V` on Linux/macOS/*BSD (`SIGTYPE_PLAN_20260615.md`). One intentional
  behavior change: `SHOW FEATURES` no longer lists `SIG_V`.
- C23 (the default `-std` now that `-std=gnu17` was dropped) required the empty-paren
  signal-handler pointer declarations to gain explicit `(int)` prototypes
  (`SIGNAL_TYPE_20260615.md`).

## Known latent bug (pre-existing, low severity)

`ckcftp.c:10030` (`failftprecv2`) passes `oldintr` (saved SIGINT handler) where `oldintp`
(saved SIGPIPE) is expected — a typo that predates the mechanical rename (`git show
a4713fd~1`). Error/cancel path only; fix separately if at all.

## Runtime test battery

After any signal- or timing-related change, run against the fresh `wermit`:

1. Clean startup and `exit` (no hang, no stray output).
2. `PAUSE 2` completes in ~2.00s — proves the SIGALRM longjmp timer fires under `SA_RESTART`.
3. SIGINT mid-`PAUSE 10` aborts promptly and returns to the command loop.

macOS reference results (real hardware, arm64): `PAUSE 2` = 2.01s; SIGINT mid-`PAUSE 10`
aborts at 1.01s.
