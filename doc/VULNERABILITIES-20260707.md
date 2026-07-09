# C-Kermit 10.0 Beta.12 (simplified fork) — Vulnerability Review

**Commit reviewed:** `21e58e2b059ff75b872f27cbf069999a09451955`
**Date:** 2026-07-07
**Scope:** all 46 tracked `.c`/`.h`/`.w` files (`git ls-files '*.c' '*.h' '*.w'`), i.e. the full simplified Linux/macOS/BSD source tree. The generated `ckcpro.c`, `wart`, and `wermit` build artifacts are gitignored and were not reviewed directly, but `ckcpro.w` (the source they're generated from) was.

## Methodology

The tree was split into 10 subsystem-scoped reviews, each run as an independent `c-expert` agent doing a grep-driven sweep for known-risky patterns (`strcpy`/`strcat`/`sprintf`/`gets`, unchecked `memcpy`, unbounded array indexing, `system`/`popen`/`exec*`, non-literal format strings, signed-char table indexing, TOCTOU/race patterns) followed by manual tracing of call sites back to their actual input source, to separate real, attacker-reachable bugs from superficially-risky-looking but actually-bounded code. Two of the most significant findings (the CONNECT-mode `ecbuf` overflow and the `cmdini()` startup heap overflow) were independently reproduced under AddressSanitizer against a live build. Severity reflects reachability (remote/pre-auth vs. local vs. build-config-gated) and impact (memory corruption / arbitrary file write vs. crash vs. information leak vs. correctness-only).

Findings are numbered **V-1** through **V-43**, grouped by severity. File:line references are accurate as of the reviewed commit.

---

## Summary

| Severity | Count | Remotely triggerable (no special local config) |
|---|---|---|
| Critical | 8 | 6 |
| High | 7 | 2 |
| Medium | 13 | 2 |
| Low / Informational | 15 | 0 |

---

## Critical

### V-1. Pre-authentication stack buffer overflow in REMOTE LOGIN username handling
**Location:** `ckcpro.w:2567` (buffer decl), `2577` (length from wire), `2611–2617` (missing `return`)
**Description:** `srv_login()` declares `char f1[LOGINLEN+1]` (33 bytes) and reads a peer-supplied length `len = xunchar(srvcmd[1])`, which can be 0–255 (see V-3 root cause note on `xunchar()`). When `len > LOGINLEN`, the code sends an error packet but has **no `return`/early exit** — execution falls through into an unconditional `for (i = 0; i < len; i++) f1[i] = p[i];` copy loop anyway.
**Trigger:** Any Kermit server/IKSD session configured with login required (`x_login`, a common IKSD deployment). Reached via `srv_login()` pre-authentication — no valid credentials needed; the oversized username *is* the payload.
**Impact:** Remote stack corruption with up to ~222 attacker-controlled bytes past a 33-byte stack buffer, smashing adjacent locals and plausibly the return address. Crash/DoS at minimum, plausible RCE (no explicit `-fstack-protector` flag in the `linux` makefile target, so this depends on distro `gcc` defaults).
**Confidence:** High — directly read, unambiguous missing-return bug.
**Fix direction:** Add `return` (or equivalent abort of the copy) in the `len > LOGINLEN` branch.

### V-2. Remotely-triggerable heap overflow in packet-decode sink `putsrv()`
**Location:** `ckcfns.c:414–419` (`putsrv()`, zero bounds checking); feeders at `ckcfns.c:3330` (filename packets), `ckcpro.w:746` (pre-auth generic REMOTE command), `ckcpro.w:787` (REMOTE HOST), `ckcpro.w:1828`, `ckcpro.w:3440`; allocation at `ckcfn3.c:327` (`malloc(r + 100)`, sized for *encoded* length only).
**Description:** `putsrv()` writes through `srvptr` with no bound. `decode()`'s repeat-count expansion (`ckcfns.c:1261–1266`) can expand a 3-byte encoded sequence into up to 255 output bytes (repeat processing defaults on, `ckcmai.c:703`), so a modest wire packet can decode to hundreds of KB, all funneled through `putsrv()` into a buffer sized only for the encoded length.
**Trigger:** Ordinary file receive from a malicious sender, or server/IKSD mode receiving a REMOTE command — default, everyday operations.
**Impact:** Heap buffer overflow of attacker-chosen size (tens–hundreds of KB) with attacker-controlled content. Strong RCE candidate.
**Confidence:** High.

### V-3. Global buffer overflow + adjacent-memory info leak in `spar()` outbound padding
**Location:** `ckcfns.c:4849–4864`; buffer `CHAR padbuf[96]` at `ckcmai.c:838` (declared immediately before live pointers `*recpkt, *rdatap, *data, *srvptr`).
**Description:** `npad = xunchar(s[3])` (0–255, attacker-controlled) is used unclamped as a loop bound writing into `padbuf[96]`. Root cause: `xunchar(ch)` is `((ch - SP) & 0xFF)` (`ckcker.h:646`) — for `ch` in 0–31 this wraps mod 256 rather than clamping to the intended 0–94 protocol range, so it can yield any byte 0–255. `spar()` runs on every `S`/`I` packet exchange (`ckcfns.c:3071`, `ckcpro.w:501`, `ckcpro.w:591`) — i.e. at every connection handshake.
**Trigger:** A single crafted `S`/`I` packet (or ACK) with field 3 in 0x00–0x1F.
**Impact:** Out-of-bounds write of up to ~159 bytes past a global buffer adjacent to critical pointers; the same overflowed `npad` is later used to `ttol(padbuf, npad)` (`ckcfn2.c:1325`), **transmitting adjacent process memory back to the attacker**.
**Confidence:** High.

### V-4. Unbounded reply-buffer overflow in Attributes (`A`) packet parsing
**Location:** `ckcfn3.c:1340` (`static char rpbuf[RPBUFL+1]`, 21 bytes), rejection-path appends at lines `1392/1420/1458/1528/1556/1598/1610/1621/1651/1659` (none check against `RPBUFL`); reached via `ckcpro.w:1576` on every incoming Attributes packet.
**Description:** Every attribute-rejection branch does `*rp++ = c` with no bound check; the outer loop iterates once per attribute entry, bounded only by wire packet length, not entry count. The `'/'` (record-format) case validates/rejects unconditionally, so a single `A` packet packed with many minimal 2-byte invalid entries (even a default 90-byte short packet yields ~40+ entries, already ~2x capacity) overflows `rpbuf`.
**Trigger:** A single crafted `A` packet — Attributes exchange happens by default before every file in a transfer.
**Impact:** Overflow of a function-static 21-byte buffer.
**Confidence:** High.

### V-5. Pre-authentication stack buffer overflow in Telnet NEW-ENVIRON reply builder
**Location:** `ckctel.c:4081–4082` (unguarded write), buffer `char varname[16]` at `ckctel.c:3714`; the sibling first pass at line `3862` correctly checks `if (j < 16)` — the second pass does not.
**Description:** `tn_snenv()` replies to a Telnet server's `IAC SB NEW-ENVIRON SEND` request in two passes: a length-computing pass (correctly bounded) and a write pass (`ckctel.c:3889–4084`) that omits the bound check present in pass 1. The subnegotiation buffer is up to ~1021 bytes (`TSBUFSIZ=1024`), so a malicious server sending a "variable name" of hundreds of bytes overflows the 16-byte stack buffer.
**Trigger:** Any Telnet server (or MITM, since Telnet is unencrypted) the user connects to — fires automatically during protocol negotiation, no login or user interaction beyond connecting.
**Impact:** Remote pre-auth stack corruption with ~1000 attacker-controlled bytes. Strong RCE candidate. `CK_ENVIRONMENT` (which gates this code) is unconditionally defined on UNIX (`ckcdeb.h:1547–1552`), so it's in every standard build.
**Confidence:** High — full data flow traced from `tn_sb()` through to the missing check.
**Fix direction:** Add the same `if (j < 16)` guard used in pass 1 (line 3862) and in the sibling receive-side function `tn_rnenv()` (line 3687).

### V-6. Path traversal via unstripped `..` in remote-supplied filenames (default configuration)
**Location:** `ckufio.c:2420–2547` (`nzrtol()`), branch selection `2455–2465`; `zmkdir()` `ckufio.c:6617–6720` (no `..` filtering, calls `mkdir()` per path segment); write-open `zopeno()` `ckufio.c:1133`.
**Description:** `zstrip()` (which safely discards path components) is only invoked when `fnrpath == PATH_OFF`. The compiled-in default is `PATH_AUTO` (`ckcmai.c:1153`), under which `nzrtol()` copies the peer-supplied filename through with only control-character stripping — `/`, `.`, `..` pass untouched. `PATH_AUTO` is only rewritten to the safer `PATH_REL` for *recursive* transfers; an ordinary single-file `RECEIVE`/`GET` stays `PATH_AUTO` for the whole session. `zmkdir()` then creates directories along the traversal path with no `..` filtering, and `zopeno()` has no traversal check either. The one confinement mechanism, `CKROOT`/`zinroot()`, is off by default and requires an explicit local `CHROOT` command.
**Trigger:** A malicious/compromised remote Kermit peer sends a file named e.g. `../../../../home/victim/.ssh/authorized_keys` during any ordinary `RECEIVE`/`GET`. Default settings are sufficient.
**Impact:** Arbitrary file write/overwrite outside the intended download directory, bounded only by OS permissions — SSH key injection, cron injection, potential full account/host compromise.
**Confidence:** High — full call path traced, no intervening sanitizer found for the default configuration.
**Fix direction:** Canonicalize/reject embedded `..` components in `nzrtol()` for the `PATH_REL`/`PATH_AUTO`/`PATH_ABS` branches, reusing the lexical `..`-collapsing logic `zinroot()`'s fallback already implements.

### V-7. Buffer overflow in FTP client's PASV reply parser
**Location:** `ckcftp.c:818` (`pasv[64]` static buffer), `ckcftp.c:9709` (unbounded `*pt++ = c;` copy loop in `getreply()`).
**Description:** The parenthesized address/port text from a server's `227 ...` PASV reply is copied into a fixed 64-byte buffer with no length check — unlike the adjacent `ftp_reply_str` copy two lines later, which *is* checked.
**Trigger:** Any FTP server sending an oversized `227` reply line during ordinary PASV-mode use.
**Impact:** Stack/static buffer overflow, attacker-controlled (malicious/compromised FTP server) content and length.
**Confidence:** High.

### V-8. Unbounded global array write in server-mode GET filename parsing, remotely reachable
**Location:** `ckuusx.c:2295–2357`, overflow write at `2337`; buffer `char *msfiles[MSENDMAX]` at `ckuusx.c:314` (`MSENDMAX`=1024 on the standard `BIGBUFOK` Linux build); called from `sgetinit()` at `ckcpro.w:3539`.
**Description:** `fnparse()` tokenizes its input on whitespace and does `msfiles[r] = p; r++;` with no bound check on `r`. It is called from the server-side handler for an incoming Kermit **GET** packet, where the filespec is taken directly from a remote peer's packet when unquoted.
**Trigger:** A remote peer being served by a local `SERVER`/`kermit -x` session sends a GET request whose filespec packs >1024 space-separated tokens (~2KB string, well within normal packet sizes) — no local interactive input required.
**Impact:** Overflow of a fixed global pointer array, corrupting adjacent global data; remote, unauthenticated (within a server session) trigger.
**Confidence:** High (confirmed by reading; call chain to `ckcpro.w:3539` traced).

---

## High

### V-9. CONNECT-mode escape-sequence buffer overflow (ASan-confirmed)
**Location:** `ckucns.c:181` / `ckucon.c:160` (`char ecbuf[10]`, 10-byte static buffer); overflow loop `ckucns.c:2458–2463` / `ckucon.c:2461–2466` (`doesc()`).
**Description:** After the CONNECT-mode escape character + `\`, every subsequent keystroke is copied into `ecbuf` with **no bounds check** until CR/LF. Since keyboard input is read in bulk (`CONGKS()`/`kbget()` reads up to 257 bytes from stdin at once), this is trivially triggerable via piped/scripted stdin, not just live typing.
**Verification:** Built with `-fsanitize=address`, drove a real CONNECT session over a pty, sent escape-char + `\` + 200 `A`s with no CR/LF. ASan reported an immediate global-buffer-overflow write at `ckucns.c:2461`.
**Impact:** Local DoS at minimum, memory corruption at worst; materially worse in setuid/setgid deployments (the codebase explicitly supports `priv_on()`/`priv_off()`/`initsuid()`).
**Confidence:** Confirmed (ASan reproduction).

### V-10. Unconditional heap-buffer overflow on every `wermit` startup
**Location:** `ckuus5.c:1034` (root cause: `int n = sizeof spdtab + 2;` where `spdtab` is `struct keytab *` — a pointer, giving `n=10`/`n=6` regardless of actual string lengths), overflowing write at `ckuus5.c:1070` (`ckstrncpy(spdtab[i].kwd, tmp[i].kwd, n)`); related latent defect at line `1052` (`speeds[i]` allocated 12 bytes, claimed 20).
**Description:** In `cmdini()`'s `#ifndef NOSORTSPEEDS` speed-table-sort block, `n` is computed from `sizeof` of a pointer instead of a string length. `spdtab[i].kwd` buffers were allocated as small as 2 bytes; the copy-back uses `n=10` as the claimed destination size, overflowing most/all entries. `TTSPDLIST` auto-defines for Linux builds, `NOSORTSPEEDS` is never defined anywhere in the tree, and `cmdini()` runs unconditionally from `main()` before any user input.
**Verification:** Confirmed under AddressSanitizer.
**Impact:** Heap corruption on every startup. Content isn't directly attacker-steerable (it's the local termios speed table), so this is a reliability/memory-safety bug rather than a directly-steerable RCE primitive, but it is real, unconditional heap corruption.
**Confidence:** Confirmed (ASan reproduction; root cause pinned by reading `spdtab`'s type declaration).

### V-11. Remote filename triggers local shell command execution via pipes
**Location:** `ckcfns.c:3455–3467` (pipe-filename detection: leading `!` in a received filename), `3489–3502`; `ckufio.c:2868–2990` (`zxcmd()`, `popen(comand,"w")` at `2909` / `execl(shpath, ...)` at `2981`).
**Description:** When the local user has `SET TRANSFER PIPES ON` (`usepipes`, off by default), a received file whose Kermit-protocol filename starts with `!` has the `!` stripped and the **entire remainder treated as a shell command**, executed via `popen()`/`execl()` with the incoming file data piped to its stdin. The flag is documented for *outbound* piping (SEND side) but the same flag also authorizes the *remote peer* to execute commands on receive — an easy-to-miss asymmetric consequence.
**Trigger:** Local user enables `SET TRANSFER PIPES ON` (legitimate documented feature); any subsequent receive from an untrusted peer with filename `!<shell command>` executes that command locally.
**Impact:** Remote code execution as the local Kermit user, conditional on a non-default opt-in.
**Confidence:** High.
**Fix direction:** Require a separate, distinctly-named opt-in for *inbound* pipe execution, and/or warn loudly when a peer requests it.

### V-12. Recursive GET/MGET path traversal via FTP server-supplied filenames
**Location:** `ckcftp.c:6788`, `6813`, and the MLSD directory branch `12946–12967`.
**Description:** Server-supplied NLST/MLSD names are used verbatim as local file/directory paths when `/RECURSIVE` is set — no `..`/absolute-path filtering anywhere in the file, and `zmkdir()`/`zfnqfp()` (`ckufio.c`) don't sandbox this path either.
**Trigger:** A hostile/compromised FTP server returning crafted directory-listing names during a recursive GET/MGET.
**Impact:** Files planted/overwritten outside the intended download directory.
**Confidence:** High.

### V-13. `SET KEY` `\K`-doubling loop overflow (confirmed)
**Location:** `ckuus3.c:6789–6803` (`dosetkey()`); length check at line `6805` happens **after** the unbounded copy loop that precedes it.
**Description:** For every `\K`/`\k` pair in a `SET KEY` definition, the loop can write up to ~1.5 output bytes per input byte into the global `line[LINBUFSIZ+1]` buffer before any length check runs. A definition near the max command-buffer length (achievable interactively, via macro, or via a long `.kermrc`/take-file line) overflows `line` with attacker-chosen content.
**Verification:** Loop logic extracted and reproduced under ASan (14-byte input overflowed a 20-byte test buffer).
**Impact:** Memory corruption of a global buffer with attacker-controlled content and length.
**Confidence:** Confirmed.

### V-14. `OUTPUT`/`LINEOUT` quoting-loop overflow (same pattern as V-13, confirmed)
**Location:** `ckuusr.c:8396–8407`. Command quoting (`cmdgquo()`) defaults ON, so this path is live by default.
**Description:** Identical root cause to V-13: unbounded copy into `line[]`, bound check applied only after the fact. An `OUTPUT`/`LINEOUT` argument packed with repeated `\n`/`\b`/`\l`/`\\` sequences overflows `line[]`.
**Impact/Confidence:** Same as V-13.
**Fix direction (V-13/V-14 shared):** Compute required output length before writing, or bounds-check the write cursor inside the loop rather than only after it completes.

### V-15. Unbounded copy into global `cmdbuf` during interactive filename/directory completion
**Location:** `ckucmd.c:2210–2211`, `2393–2395`, `2427–2429` (inside `cmifi2()`, behind ESC/TAB completion); buffer `char cmdbuf[CMDBL+4]` at line `262`. Contrast with the correctly-bounded sibling path at line `2477` which uses the bounds-checked helper `addbuf()`.
**Description:** Three completion-insertion sites use a raw `while ((*bp++ = *sp++));` copy with no check against `cmdbuf`'s end, for tilde expansion, partial-completion repaint, and unique-directory completion — all compiled in by default.
**Trigger:** A user (or an automation harness driving the controlling tty — `expect`, `tmux send-keys`, etc.; only requires `is_a_tty(0)`, not a human) types/pastes a long command line, then presses ESC/TAB to complete a filename near the buffer's end. Under the default `BIGBUFOK` build (`CMDBL`≈32763) this needs a large amount of prior input; under `-DNOBIGBUF` (`CMDBL`=4092/608) it's dramatically easier.
**Impact:** Overwrite of global memory following `cmdbuf`; crash/DoS at minimum, potential corruption of adjacent globals.
**Confidence:** Medium-High.

---

## Medium

### V-16. FTP client does not validate PASV-reply address/port before connecting (SSRF-style redirect)
**Location:** `ckcftp.c:11364–11368` (`initconn()`).
**Description:** The data-connection `connect()` target is taken straight from the server's PASV reply with no check against the control-connection's own address.
**Impact:** A malicious FTP server can redirect the client's outbound data connection to an arbitrary host:port.
**Confidence:** Medium-High.

### V-17. `.netrc` parser buffer overflow
**Location:** `ckcftp.c:13392` (`tokval[100]`), unbounded copy loop `13401–13434` — same missing-bounds-check pattern as V-7.
**Trigger:** Local `.netrc` content only (not remote-server-controlled), so outside the "malicious server" threat model, but still a real local overflow if `.netrc` is ever attacker-influenced (e.g. shared/synced dotfiles).
**Confidence:** High.

### V-18. Off-by-one write past `varname[16]` in Telnet NEW-ENVIRON handling (independent of V-5)
**Location:** `ckctel.c:3763, 3802` (pass 1) and `3902, 3990` (pass 2); buffer declared `ckctel.c:3714`.
**Description:** Even with the `if (j<16)` guard from V-5's fix in place, `j` can reach exactly 16, after which `varname[j] = '\0'` writes `varname[16]` — one byte past the 16-byte buffer, in both passes. The sibling receive-side function `tn_rnenv()` correctly sized its equivalent buffer as `char varname[17]` for the same contract.
**Trigger:** Same NEW-ENVIRON `SEND` path as V-5, needs a variable name ≥16 bytes (lower bar than V-5).
**Confidence:** High.
**Fix direction:** Declare `char varname[17]` in `tn_snenv()` to match `tn_rnenv()`.

### V-19. Predictable serial-lock-file name with no `O_EXCL`, under privilege elevation
**Location:** `ckutio.c:3902` (`ttlock()`).
**Description:** Lock file named `LTMP.<pid>` created via `creat()` without `O_EXCL`, executed under `priv_on()` elevation, typically in world-writable/sticky `/var/lock`.
**Trigger:** Requires a setuid/setgid deployment, which the codebase explicitly supports (`priv_on()`/`priv_off()`/`initsuid()`).
**Impact:** Classic symlink-race/TOCTOU pattern.
**Confidence:** Medium-High.

### V-20. `SET MODEM DIAL-COMMAND` used as an unvalidated `printf` format string
**Location:** `ckudia.c:5422` (`sprintf(lbuf, dcmd, xnum)`).
**Description:** The user-configurable dial-command string is used as a format string; validation elsewhere only requires *a* `%s` to be present, not that it's the *only* conversion. A malicious `DIAL-COMMAND` value (e.g. via a hostile take-script) containing `%n` triggers undefined behavior.
**Confidence:** Medium.

### V-21. File descriptor leak on every transfer that overwrites an existing file
**Location:** `ckufio.c:1192–1226` (`zopeno()`, specifically `1210–1218`).
**Description:** A probe `open()` (no `O_CREAT`) used only to check `isatty()` succeeds whenever the destination already exists but is never closed on the non-tty path; a separate `fopen()` immediately after does the real I/O.
**Trigger:** Any transfer overwriting an existing destination filename — trivially reproducible.
**Impact:** FD exhaustion on long-running sessions/servers (e.g. IKSD handling repeated uploads) → `EMFILE`, DoS for further opens/connections.
**Confidence:** High.
**Fix direction:** `else close(fd);` in the non-tty branch.

### V-22. No `O_EXCL`/`O_NOFOLLOW` on output-file creation; TOCTOU between precheck and open
**Location:** `zchko()` precheck (`ckufio.c:2183–2374`) vs. actual write-open in `zopeno()` (`ckufio.c:1224`, plain `fopen(name,"w"/"a")`).
**Description:** No symlink check or exclusive-creation flag between the writability precheck and the actual open. Requires local write access to the destination directory by another principal (classic shared-directory symlink race); no protocol-exposed primitive was found that lets a *remote* peer place the symlink directly, so this is defense-in-depth rather than a confirmed remote exploit chain.
**Confidence:** Medium.

### V-23. Out-of-bounds read `table[-1]` in keyword lookup, dereferenced as a string pointer, under standard debug logging
**Location:** `ckucmd.c:7849` (`nlookup()`).
**Description:** `lastmatch` is initialized to `-1` and only set on a match; on the first non-matching table entry (the common case), `table[lastmatch].kwd` evaluates `table[-1].kwd`, reading garbage bytes before the array and dereferencing them as a `char *` in a `debug()` call.
**Trigger:** `DEBUG` is compiled in by default; any session run with `-d` or `SET DEBUG ON` + `LOG DEBUG` (an entirely ordinary troubleshooting step) hits this on essentially every `nlookup()` call against a numeric keyword table whose input doesn't match the table's first entries.
**Impact:** Crash if the garbage pointer is invalid, or leakage of adjacent memory content into the debug log.
**Confidence:** High.

### V-24. `filbuf` sized for `ATMBL` but unconditionally NUL-terminated at `CKMAXPATH` (build-config gated)
**Location:** `ckucmd.c:2259, 2263`; buffer `static char filbuf[ATMBL+4]` at line `268`.
**Description:** Safe under the default `BIGBUFOK` build (`ATMBL`=10238 ≫ `CKMAXPATH`≈4096), but under the makefile-documented, supported `-DNOBIGBUF` build, `ATMBL` drops to 1024 (`filbuf`=1028 bytes) while the unconditional `filbuf[CKMAXPATH] = NUL;` still writes ~3068 bytes past the buffer on every successful file-completion.
**Trigger:** Unconditional (any successful completion) under `-DNOBIGBUF` builds only.
**Confidence:** High.

### V-25. Off-by-one heap overflow in `\s(name[n.])` compact-substring notation (confirmed)
**Location:** `ckuus4.c:15352–15411`, write at `15410–15411` (`q[x1+1] = NUL;`).
**Description:** The bounds guard rejects `x1 > k` but not `x1 == k`; `q` is `malloc(k+1)` (valid indices 0..k), so `q[x1+1]` writes `q[k+1]`, one byte past the allocation, whenever the user supplies `N = strlen(value)+1` in `\s(var[N.])`. The sibling `':'` and `'_'` branches correctly clamp; only the `.` case is affected.
**Verification:** Standalone reproduction of the exact index arithmetic confirmed a heap-buffer-overflow under ASan.
**Impact:** 1-byte NUL write past a heap allocation — likely crash/DoS, allocator-dependent corruption at worst. Triggerable through ordinary `\s()`/`\v()` substring syntax in any macro/command line.
**Confidence:** Confirmed.

### V-26. Unguarded 128-element table index with remote information disclosure
**Location:** `ckuxla.c:5396` (`xeglg()`, `yeglg[]`).
**Description:** A 128-element array indexed by a full-range `CHAR` with no `& 0x80` guard, unlike every sibling translation function in the file (compare `xh7lh`, which has the guard). Reachable via `SET TRANSFER-CHARACTER-SET LATIN/GREEK` + `SET FILE-CHARACTER-SET ELOT927-GREEK` during file send/translate.
**Trigger:** A local file byte ≥0x80 under that specific charset configuration.
**Impact:** Out-of-bounds read whose garbage value is transmitted to the remote peer — information disclosure of adjacent static memory, potential crash.
**Confidence:** High — full call chain traced (`setxlatype` → `xls[TC_GREEK][FC_ELOT]` → `ckcfns.c:xgnbyte` → `(*sx)(rt)`), confirmed the vulnerable path is actually reachable via `cseqtab[]`.

### V-27. `SET EDITOR`/`SET BROWSER /OPTIONS` value used as an unvalidated `printf` format string
**Location:** `ckuusr.c:6389–6398` (`doedit()`), `6433–6441` (`dobrowse()`).
**Description:** `editopts`/`browsopts` are used as `sprintf` format strings whenever they contain `"%s"`; the check confirms presence of `"%s"` but not absence of other conversions (`%n`, extra `%s`, etc.).
**Trigger:** Requires a `.kermrc`/init file setting `SET EDITOR /OPTIONS:"...%n..."` — i.e. first requires compromising a locally-trusted config value (e.g. a shared/synced dotfile), then running `EDIT`/`BROWSE`.
**Confidence:** Medium (design flaw clear; didn't fully trace all guardrails).

### V-28. `setword()` partial-initialization NULL dereference under allocation failure
**Location:** `ckclib.c:2879–2894` (used by `cksplit()`).
**Description:** If the first of two `malloc()` calls succeeds but the second fails, the function returns early leaving `wordarray` non-NULL and `wordsize` NULL. The guard on subsequent calls (`if (!wordarray)`) is now false, so a later call falls through to dereference the still-NULL `wordsize`.
**Trigger:** Requires a transient allocation failure (memory pressure/OOM) between the two mallocs; reachable via any script/command path using word-splitting.
**Impact:** Crash only (no corruption).
**Confidence:** High on the code path being real; low likelihood of natural triggering.

---

## Low / Informational

| # | Finding | Location | Note |
|---|---|---|---|
| V-29 | `decode()`/`bdecode()` truncated-escape over-read | `ckcfns.c:1258–1289, 545–558` | Reads 1–2 bytes past a truncated escape sequence without checking remaining length first, but stays inside the already-allocated receive buffer (block-check/terminator bytes) — data-integrity quirk, not confirmed memory-unsafe. |
| V-30 | Active-mode FTP data-connection hijack race | `ckcftp.c:11490` (`dataconn()` `accept()`s without peer verification) | Inherited weakness of the classic BSD ftp lineage; requires network-race positioning. |
| V-31 | Dead overflow-detection code in MLSD/NLST line reading | `ckcftp.c:12898` | Checks the wrong index so the warning can never fire; not itself unsafe since `fgets()` bounds the underlying write. |
| V-32 | Asymmetric `#ifdef CK_SNDLOC` guards between `tn_snenv()`'s two passes | `ckctel.c:3816–3820` vs `4018–4024` | Would cause a heap overflow of `reply` if `CK_SNDLOC` were ever undefined while `ckctel.c` still compiled — currently impossible (`ckctel.c` is excluded entirely under the only build config that undefines `CK_SNDLOC`). Fix for robustness, not currently exploitable. |
| V-33 | `TELOPT_NAWS` server-side handler reads stale buffer on short subnegotiation | `ckctel.c:3457–3481` | Logic/stale-data bug (stays within the 1024-byte static buffer); requires `sstelnet`/IKSD mode (off by default) and a malicious *client*, not server. |
| V-34 | `ckxfprintf()`/`ckxprintf()` check overflow only after `vsprintf()` into a fixed 4096-byte buffer | `ckutio.c:11784, 11861` | IKSD-mode printf wrappers; fires under IKSD only. |
| V-35 | `DIAL TIMEOUT` case falls through into `DIAL ESCAPE-CHARACTER` handling | `ckuus3.c:5501–5516` | Missing `break`; correctness bug only, no security impact identified. |
| V-36 | Possible 1-byte OOB read (`p[-1]`) when `TMPDIR`/`TMP`/`TEMP`/`CK_TMP` is set but empty | `ckuus4.c:13766–13786` | UB per the standard; reads adjacent environment-block bytes, not attacker-controlled beyond the environment itself. |
| V-37 | `NULL` passed to `%s` in `printf` when `LANG` is unset | `ckuus5.c:6784` | UB per the C standard (glibc tolerates it); every other locale value in the same block is NULL-guarded, this one was missed. Local `SHOW` command only. |
| V-38 | `dgi_u()`/`hproman8_u()` missing lower-bound self-check (latent) | `ckcuni.c:10520–10534` | ~50 sibling `*_u()` functions all self-guard against negative indices; these two rely on a cross-file invariant (`fcsinfo[...].size==256`) that currently holds at every call site — not independently triggerable today, but fragile. |
| V-39 | `ckfstoa()` OVERFLOW-path length argument computed from the wrong base | `ckclib.c:590` | `ckstrncpy(&buf[23], "OVERFLOW", 32)` — `32` doesn't account for the `23`-byte offset; harmless only because the source is a fixed 9-byte literal that happens to fit the real 9-byte remainder. Maintenance hazard if ever refactored. |
| V-40 | `b64tob8()` streaming output-size bound ignores leftover bits from a prior call | `ckclib.c:2475–2524` | The function's own bounds check doesn't account for cross-call state it explicitly documents supporting; the one real streaming call site (`ckuus6.c:8801`) happens to stay within its actual (larger) buffer, so not currently reachable. |
| V-41 | `ckmatch()` unbounded recursion on nested `{...}` pattern groups | `ckclib.c` ~1356–1950 | Stack exhaustion possible with deeply-nested brace patterns; patterns are normally locally supplied (filespecs, `SET`/`DEFINE` values), not remote-controlled — local robustness issue only. |
| V-42 | `alarm()` armed before the new SIGALRM handler is installed | `ckusig.c:80–81, 104–105` | Narrow race window where an already-pending/very-short alarm could fire the *previous* handler instead of the new one. Correctness/robustness nit, not a memory-safety or privilege issue. |
| V-43 | `-N` NetBIOS adapter range-check has a `&&`/`||` mixup making it tautologically false | `ckuusy.c:3896` | Dead code — `CK_NETBIOS` is unconditionally undefined for this Linux/macOS/BSD-only fork (`ckcdeb.h:283–285, 399–401, 453–455`); unreachable in any buildable configuration. |

---

## Areas reviewed with no significant findings

- **`rpack()`** length/checksum arithmetic (`ckcfn2.c:2497–2900`) — `rln` is properly bounds-checked against the actual receive-buffer allocation before use.
- **Receive-buffer pool management** `getrbuf()`/`freerbuf()`/`freesbuf()` (`ckcfn3.c:564–664`) — index-based pool, no UAF/double-free pattern.
- **`srv_copy()`/`srv_rename()`, `cwd()`/`remset()`** (`ckcpro.w`, `ckcfns.c`) — unclamped `xunchar()`-derived offsets, but destination buffers happen to be large enough that no overflow occurs as written (fragile but not exploitable today).
- **`tn_rnenv()`** (Telnet NEW-ENVIRON receive side, `ckctel.c:3613–3700`) and **`tn_sb()`** (core subnegotiation reader, `ckctel.c:2158–2224`) — correctly bounded throughout.
- **`TELOPT()`/`TELCMD()`** debug-formatting macros — range-checked before array access.
- **`ck_copyhostent()`** (`ckcnet.c:721–814`) — `gethostbyname()` results copied into dynamically-sized allocations, not the classic fixed-buffer aliasing bug.
- **`CK_DNS_SRV`** hand-rolled DNS wire parser (`ckcnet.c:9150–9563`) — gated by a macro never defined by any target in this fork; dead code, not compiled into `wermit`.
- **RLOGIN username/term-speed buffer construction** (`ckcnet.c:4100–4237`) — sources are local config/environment (not remote-attacker-controlled), pre-truncated via `ckstrncpy`.
- **`system()`/`popen()`/`exec*()` in `ckupty.c`/`ckutio.c`** (`ttruncmd`, `ttptycmd`, `exec_cmd`) — all go through `cksplit()`+`execvp()`, never a shell; commands run are locally-configured, not modem/remote-derived.
- **Modem response parsing** (`ckudia.c` `dook()`/`getok()`/`_dodial()`) — consistently bounds-checked.
- **pty allocation** — goes through `openpty()`; legacy manual `chmod`/`chown` pty code is dead (gated on a macro never defined for this build).
- **`ckuscr.c`** (SCRIPT command) — no `system`/`popen`/`exec*`/`fork` calls at all; only reads/writes the already-open descriptor. Escape-sequence buffer (`seq_buf[SBUFL+2]`) is explicitly bounded.
- **`CKROOT`/`zinroot()`** design itself (`ckufio.c:7076–7155`) — sound `realpath()`-based canonicalization when active (see V-6 for why it's not active by default).
- **`zoutdump()`** (`ckufio.c:1790–1845`) — correctly loops on partial `write()` returns.
- **Kerberos `mktemp()` race** (`ckufio.c:7517–7533`) — gated by a macro never defined in this fork (consistent with the documented Kerberos removal); dead code.
- **`gtword()`'s main character-deposit loop, `setatm()`, `addbuf()`** (`ckucmd.c`) — correctly bounds-checked; these are the "good examples" that V-13/V-14/V-15 deviate from.
- **TAKE-file/macro recursion depth** (`cmpush()`/`cmpop()`, `ckucmd.c:972–1222`) — `CMDDEP` enforced with a correct bounds check; all call sites check the return value before recursing.
- **`lookup()`/`xlookup()`/`rlookup()`** — correctly bounded array indexing (contrast V-23's `nlookup()`).
- **`ckuus2.c`** — almost entirely static HELP/SHOW text tables; no unsafe string functions, no format-string or exec risk.
- **`ckstrncpy`/`ckstrncat`/`ckmakmsg`/`ckmakxmsg`** (`ckclib.c:131–310`) — hand-verified bounds arithmetic, correctly constructed; the codebase's ~180+ other `strcpy`/`sprintf` call sites were sampled and found to consistently precede the copy with a matching `malloc(strlen(s)+1)` or explicit capacity check.
- **`ckcmai.c`** — no unsafe string functions; every `malloc`/`realloc` return checked; no `setuid`/`setgid`/`system`/`exec` calls at all.
- **`ckwart.c`** (build-time-only grammar preprocessor) — `rdword()`'s token buffer is precisely bounded; no unbounded recursion on malformed grammar input.
- **`ckcdeb.h`, `ckclib.h`** — no unsafe macro wrapping found.
- **Signal handling** (`ckusig.c`, beyond V-42) — no actual signal-handler function is defined in this file (`ck_signal()` itself, the sigaction wrapper, was out of scope per existing project convention); no non-async-signal-safe calls or unguarded shared-state mutation found in the setjmp/longjmp trampolines that are in scope.
- **UTF-8 state machine, byte↔UTF-8 conversion, Kanji/Shift-JIS/JIS-7/EUC tables** (`ckcuni.c`, `ckuxla.c`) — element counts verified against range checks throughout; ~60 other direct-index translation functions correctly bounded (contrast V-26/V-38).

---

## Priority recommendation

Fix in this order:
1. **V-1, V-2, V-3, V-4, V-5, V-6, V-7, V-8** (Critical) — all are remotely triggerable with default or near-default configuration and involve memory corruption or arbitrary file write. V-1 and V-5 are pre-authentication.
2. **V-9, V-10, V-13, V-14, V-15** (High, confirmed/near-confirmed memory corruption) — straightforward, well-understood fixes (add missing bound checks / fix the `sizeof` mistake).
3. **V-11, V-12** (High, requires non-default opt-in or malicious server) — fix opportunistically, document the risk in the meantime.
4. Remaining Medium findings, roughly in the order listed.
