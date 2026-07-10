# New makefile targets `macos-notcp` and `macos-nonet` — 2026-07-09

C-Kermit 10.0 Beta.12 test repository, branch `macos-nonet-notcp` (based on `main` at
`269f2a5`). Adds macOS equivalents of the existing `linux-notcp` and `linux-nonet`
targets, both thin wrappers that re-invoke the `macos` entry with extra `KFLAGS`.

## Changes

### `makefile`

Two new entries in the Mac OS X section, directly after `macos`:

```make
macos-notcp:
	@echo Making C-Kermit $(CKVER) for macOS NO TCP/IP...
	$(MAKE) macos KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNOTCPIP -UCKHTTP -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"

macos-nonet:
	@echo Making C-Kermit $(CKVER) for macOS NO NETWORKING...
	$(MAKE) macos KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNONET -DNONETDIR -UCKHTTP -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"
```

They mirror `linux-notcp` / `linux-nonet` (same `-DNOTCPIP` / `-DNONET -DNONETDIR`
selectors, same `-DSSHCMD -DANYSSH` for the external ssh client), with one addition,
`-UCKHTTP`, explained below.

### `ckcnet.c`

The `#ifndef NETCONN` dummy-function block (`ckcnet.c:326`) still contained three
K&R-style function definitions (`netopen`, `netinc`, `nettol`) that the 2026-06
tree-wide K&R removal missed, because that block is compiled only in `NONET` builds —
never by `make linux`, and not even by `*-notcp` builds (where `NETCONN` stays defined
via the external-ssh/pty path). Under clang on macOS they produced three
`-Wdeprecated-non-prototype` warnings in the `macos-nonet` build. They were converted
to ANSI definitions matching their prototypes in `ckcnet.h:151/157/159`. No other
platform compiles this block, so `make linux` / `make macos` object code is unaffected.

## Analysis: why a simple KFLAGS wrapper is safe

The `macos` entry hard-codes `-DTCPSOCKET -DCKHTTP` in its CFLAGS (the `linux` entry
does not — on Linux both are derived in `ckcdeb.h`), and `$(KFLAGS)` is appended
*after* them. Findings:

1. **`TCPSOCKET` needs no `-U`.** Header logic neutralizes it: `NONET` undefines
   `TCPSOCKET` at `ckcdeb.h:382-384` and implies `NOTCPIP`/`NONETDIR`
   (`ckcdeb.h:366-372`); `NOTCPIP` undefines `TCPSOCKET` at `ckcdeb.h:3622-3625`.
   Command-line order is irrelevant since this happens in the preprocessor.
2. **`CKHTTP` does need `-UCKHTTP`.** `ckcdeb.h` only ever *defines* `CKHTTP`
   (`ckcdeb.h:1350-1374`, and only when `TCPSOCKET` is alive — which is why the Linux
   no-net builds never see it); nothing undefines a command-line `-DCKHTTP`. Leaving it
   defined still compiles (all real HTTP code is behind `#ifndef NOHTTP`, which both
   selectors define), but `SHOW FEATURES` would falsely list `CKHTTP` alongside
   `NOHTTP`. Since `$(KFLAGS)` follows `-DCKHTTP` on the compiler command line,
   `-UCKHTTP` wins and restores exact parity with the Linux targets.
3. **`-DSSHCMD -DANYSSH`** behave as on Linux (external `/usr/bin/ssh`); `NONET`
   deliberately does not imply `NOSSH` (`ckcdeb.h:417-420`).
4. **`-DUSE_NAMESER_COMPAT` and `-lresolv`** are harmless: the resolver-using code is
   compiled out and the unused library links without complaint.

## Verification (macOS 26.5.2, arm64, Apple LLVM 21.0.0 / clang-2100.1.1.101)

Each target built with `make clean; rm -f wermit; make <target>`, full logs checked for
`warning:` / `error:` — **zero diagnostics** for all three of `macos-notcp`,
`macos-nonet`, and (regression check) plain `macos`.

| Target | `SHOW FEATURES` | `SHOW NET` | `PAUSE 2` |
|---|---|---|---|
| `macos-notcp` | `Target: macos-notcp`; options list `NOHTTP`, no `TCPSOCKET`/`CKHTTP`; `NETCONN` retained (ssh/pty) | `SSH COMMAND: ssh -e none`, no TCP network types | 2.016 s |
| `macos-nonet` | `Target: macos-nonet`; `NETCONN` gone | "No network support in this version of C-Kermit." | 2.014 s |
| `macos` | `Target: macos` (unchanged baseline) | — | — |

`ckcnet.c` is clang-format-stable (`clang-format ckcnet.c | diff ckcnet.c -` empty).
