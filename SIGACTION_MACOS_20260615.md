# Verification: `ck_signal()` / `sigaction()` conversion — macOS review — 2026-06-15

Reviews the code change documented in `SIGACTION_PLAN_20260615.md`, committed as
`a4713fd` ("Replace save/restore signal() with sigaction() via ck_signal()").
Goal: confirm the change causes **no unexpected actions**, with particular
attention to the macOS / BSD `#ifdef` paths, which the Linux build on the
original machine could not exercise.

Analysis performed by the `c-expert` agent. **Note:** the verification machine
turned out to be **macOS 26.5.1 (arm64)**, so this is backed by a real
`make macos` build plus runtime smoke tests against the live macOS signal
subsystem — substantially stronger than the `-fsyntax-only` check the plan
anticipated.

## Verdict

**The change is correct and behavior-preserving on macOS. No regressions
found.** Every macOS claim in the plan is confirmed by direct evidence. One
*pre-existing* latent bug (not introduced by this commit) is noted for
information only.

## What was checked

### 1. The `ck_signal()` wrapper is correct on macOS (`ckusig.c:42-49`)

- **All `struct sigaction` fields are initialized.** On macOS (`sys/signal.h`)
  the struct has exactly three members: `__sigaction_u` (the union of
  `sa_handler`/`sa_sigaction`), `sa_mask`, `sa_flags`. The wrapper sets all
  three (`sigemptyset(&sa.sa_mask)`, `sa.sa_flags = SA_RESTART`,
  `sa.sa_handler = func`). The Linux/glibc-only `sa_restorer` field does **not**
  exist on macOS, so its absence is correct. `sigaction()` reads no
  uninitialized field on the macOS path.
- **`SA_RESTART` is genuinely behavior-preserving.** The macOS `signal(3)` man
  page states verbatim that "Any handler installed with signal(3) will have the
  SA_RESTART flag set." macOS `signal()` is therefore BSD semantics (persistent
  handler + restartable syscalls), which `ck_signal()` reproduces exactly. The
  plan's "drop-in / behavior-preserving" claim holds on macOS.
- **`return old.sa_handler` is correct.** `sa_handler` and `sa_sigaction` are
  `#define`s onto the same union `__sigaction_u`; since no call site uses
  `SA_SIGINFO`, reading `old.sa_handler` reads exactly the storage the previous
  installer wrote. Round-trip is exact.
- **Prototype matches definition.** `ckcdeb.h:783` declares
  `void (*ck_signal(int type, void (*)(int)))(int);`, identical to the
  definition. The bare function-pointer form pulls in no `<signal.h>` types, so
  callers need no new include — confirmed by the clean build of all 8 caller
  files.

### 2. The actual diff matches the plan — only save/restore sites converted

- Every converted file was scanned for **half-converted save/restore pairs** (a
  save via `ck_signal` whose matching restore stayed plain `signal()`, or vice
  versa). **None exist.** All pairs are symmetric: `ckuus4.c`/`ckuus6.c`
  `oldsig`, `ckuscr.c` `savealm`, `ckudia.c` `savalrm`/`savint`, `ckutio.c`
  `occt`/`saval`/`savquit`/etc., and all `ckcftp.c` `oldintr`/`oldintp` pairs.
- All remaining plain `signal()` calls are genuine **non-saving installs**
  (`SIG_IGN`/`SIG_DFL`/`SIG_ACK`, or handler-function arguments
  `f`/`s`/`esctrp`/`winchh`/`sighup`/`xtimerh` re-arms) — correctly left alone.
- `#ifdef SIGDANGER/SIGPIPE/SIGUSR1/SIGUSR2/SIGTSTP` guards are preserved
  exactly. `CK_POSIX_SIG` was neither added nor removed.
- The one converted site under `#ifdef V7` (`ckutio.c:7089`) is dead code (`V7`
  is never defined on any surviving platform) — harmless.

### 3. SIGALRM longjmp handlers under `SA_RESTART` — no one-shot reliance

- `timerh`/`xtimerh` (`ckutio.c`) abort via `longjmp`/`siglongjmp`, not
  `EINTR`, so `SA_RESTART` is irrelevant to their control flow. On macOS
  `CK_POSIX_SIG` is undefined, so they use plain `setjmp`/`longjmp`; macOS
  `longjmp` restores the signal mask, correctly unblocking the SIGALRM that
  `sigaction` auto-blocks during the handler — identical to the pre-change path,
  since macOS `signal()` already used the same masking.
- **No code relied on SysV one-shot reset.** macOS `signal()` is never one-shot
  (BSD-persistent), so no macOS path ever depended on auto-reset; handlers
  re-install explicitly (e.g. `timerh` → `ttimoff()` → `ck_signal(SIGALRM,
  saval)`). No regression.
- **Runtime-confirmed on macOS:** `PAUSE 2` measured **2.01s** wall clock
  (SIGALRM fires via longjmp under `SA_RESTART`); `SIGINT` ~1s into `PAUSE 10`
  aborted at **1.01s**, printed `^C...`, and returned to the command loop (the
  `occt`/`cctrap` SIGINT save/restore path works).

### 4. macOS implication chain is live; `CK_POSIX_SIG` correctly not a gate

Preprocessing `ckcdeb.h` with `-DMACOSX10 -DMACOSX103` confirms on the real
macOS path: **MACOSX, BSD44, SVR4, SVR3, ATTSV, SVORPOSIX are all defined;
POSIX and `CK_POSIX_SIG` are NOT.** This validates the plan's central decision:
gating the wrapper on `CK_POSIX_SIG` would have left macOS on classic
`signal()`. The unconditional wrapper is correct. The `SVORPOSIX`-guarded
signal code in `conint()` (`ckutio.c:6995-7013`, live on macOS / dead on Linux)
only installs `esctrp`/`SIG_IGN` (non-saving) and was correctly left as plain
`signal()` — no interaction with converted calls.

### 5. Type round-trip is exact (`ckcftp.c`)

Saved handlers there are typed `sig_t`, which on macOS is
`typedef void (*sig_t)(int)` — identical to `ck_signal`'s return and
second-parameter types. Assigning `ck_signal(...)` to `sig_t` and passing it
back round-trips with no conversion; `(*oldintr)(SIGINT)` (`ckcftp.c:9306,
11003`) invokes the genuine prior handler. The zero-warning macOS build
confirms no type mismatch.

### 6. Real macOS build + runtime (strongest evidence)

- `make clean && rm -f wermit && make macos` → **exit 0, zero warnings** across
  the whole build (incl. `ckusig.c`, `ckcftp.c`, `ckutio.c`); no
  uninitialized-`struct sigaction` warning. `make check` → **SUCCESS** (2.3 MB
  `wermit`).
- `./wermit -V` → `C-Kermit 10.0.416 Beta.12 ... for macOS 26.5.1 (64-bit)`;
  clean script exit, status 0.

## Pre-existing bug noted (NOT introduced by this commit — informational)

`ckcftp.c:10030` (`failftprecv2`): `ck_signal(SIGPIPE, ftprecv.oldintr)` passes
`oldintr` (the saved SIGINT handler) where it should pass `oldintp` (the saved
SIGPIPE handler). `git show a4713fd~1` confirms this typo **predates** the
commit — the mechanical `signal(` → `ck_signal(` rename preserved it verbatim,
which is correct conduct for a rename-only change. **Severity: low** — an
error/cancel path that restores SIGPIPE to the wrong saved disposition. If
fixed, do it in a separate, clearly-scoped commit. (`ckcftp.c:10032`'s
`signal(SIGINT, SIG_IGN)` install is intentional and correctly left as plain
`signal()`.)

## Bottom line

The wrapper is correct on macOS; the conversion is complete and symmetric with
no half-converted pairs; the `SA_RESTART`/persistent-handler semantics exactly
match what macOS `signal()` already did; the type round-trip is exact; and a
real macOS build + runtime smoke tests pass cleanly. **No unexpected actions
introduced.** The only flagged item is a low-severity, pre-existing
`oldintr`/`oldintp` typo unrelated to this change.
