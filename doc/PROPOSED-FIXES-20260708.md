# C-Kermit 10.0 Beta.12 (simplified fork) — Proposed Fixes

**Companion to:** `VULNERABILITIES-20260707.md`
**Commit:** `21e58e2b059ff75b872f27cbf069999a09451955`
**Date:** 2026-07-08
**Analysis by:** four parallel `c-expert` reviews (one per severity band), each re-reading the actual source at every cited `file:line` to verify the finding and draft a concrete patch.

---

## How to read this document

For every vulnerability in `VULNERABILITIES-20260707.md` (numbered **V-1** … **V-43**) this file gives:

- **In plain terms** — what the bug actually is, in language a working developer (not a security specialist) can follow.
- **Why it matters / How it's triggered** — the real-world consequence and the exact path an attacker or user takes to reach it.
- **Proposed fix** — a concrete before/after patch written in this codebase's conservative, portable C style (`/* */` comments, mandatory braces, `#ifdef` feature gates, no modern-toolchain assumptions, `ckcdeb.h` stays the first include).
- **Fix effort & risk** — size of the change plus any caveat about the macOS/BSD build, `clang-format` stability, or a deliberate behavior change.

**Nothing here has been applied to the source tree.** These are proposals. Every line number was verified against the reviewed commit; the Critical reviewer found no line-number drift.

> **Build reminders that apply throughout** (see the `build-and-verify` skill):
> - `ckcpro.w` is wart-preprocessed. After editing it, run `make wart && make ckcpro.c` **before** `make linux`.
> - After editing any `.c`/`.h`, confirm format-stability with `clang-format FILE | diff FILE -` (empty output = clean) and keep `make linux` warning-clean.
> - None of the proposed fixes touch the load-bearing `SVR4`/`SVR3`/`ATTSV`/`BSD44`/`MACOSX10` macro chain in `ckcdeb.h`; all use plain POSIX facilities available on Linux, macOS, and modern BSD alike. The file-I/O (V-6, V-21, V-22) and network (V-7, V-16) fixes warrant an actual runtime smoke test, not just a clean compile.

---

## Severity summary

| Severity | Findings | Remotely triggerable (default/near-default) | Recommended action |
|---|---|---|---|
| Critical | V-1 … V-8 | 6 of 8 | Fix first — all involve remote memory corruption or arbitrary file write; V-1 and V-5 are pre-authentication. |
| High | V-9 … V-15 | 2 of 7 | Fix next — straightforward, well-understood memory-safety fixes; two (V-9, V-10) are ASan-confirmed. |
| Medium | V-16 … V-28 | 2 of 13 | Fix opportunistically; several are trivial one-liners. |
| Low / Informational | V-29 … V-43 | 0 | Batch-harden or defer; three are outright dead code in this fork. |

---

## Recurring root causes (fix the pattern, not just the instance)

Most of the 43 findings are instances of a small number of repeated coding patterns. Understanding these makes the individual fixes obvious and helps prevent regressions:

1. **"Range-check, then copy anyway."** A bounds check is present but doesn't actually stop the unsafe write — either it's missing an early `return` (V-1), it runs *after* the copy loop instead of inside it (V-13, V-14), it isn't applied per-append (V-4), or it checks the wrong thing (V-28, V-31, V-43). *Fix pattern: move the guard so it gates the write, and abort/truncate on failure.*

2. **`xunchar()` wraps instead of clamping.** `xunchar(ch)` = `((ch)-SP)&0xFF` (`ckcker.h:646`) is meant to decode the protocol's printable 0–94 range, but a control byte (0x00–0x1F) wraps to 225–255 instead of being rejected. This hands attackers oversized peer-controlled lengths that feed V-1, V-2, and V-3. *Fix pattern: clamp the macro centrally (V-3), but still add per-site bounds because a legitimate 0–94 length can also overflow a small buffer.*

3. **Unbounded copy loop into a fixed buffer.** `while ((*d++ = *s++))` / `*p++ = c` with no end check: V-5, V-7, V-8, V-9, V-15, V-17. *Fix pattern: bound the write cursor against the buffer end; truncate rather than overrun. Where a bounded helper already exists in-file (`addbuf()` in `ckucmd.c`), reuse it.*

4. **Attacker/user-controlled data used as a `printf` format string.** V-20 (`SET MODEM DIAL-COMMAND`), V-27 (`SET EDITOR`/`BROWSER /OPTIONS`), and the dead V-34. *Fix pattern: never pass such data as a format; hand-roll a `%s`-only substitution.*

5. **Trusting remote-supplied file/host names.** Path traversal via `..` (V-6, V-12) and SSRF-style redirect via a server's PASV reply (V-16). *Fix pattern: reject `..` path components and absolute paths lexically; ignore the address a server tells you to connect to and reuse the control-connection host.*

6. **TOCTOU / missing `O_EXCL`/`O_NOFOLLOW` (especially under `priv_on()`).** V-19, V-22, plus the related fd leak V-21. *Fix pattern: create with `O_CREAT|O_EXCL`, open with `O_NOFOLLOW`, close probe descriptors.*

7. **Table index missing a bound.** A `CHAR`-indexed lookup table read without a `& 0x80`/lower-bound guard: V-23, V-26, V-38. *Fix pattern: add the same guard every sibling function in the file already uses.*

---

# Critical (V-1 … V-8)

### V-1. Pre-authentication stack overflow in REMOTE LOGIN username
**In plain terms:** When a Kermit client sends a "REMOTE LOGIN" command with a username, the server reads a length byte, checks "is this too long?", prints an error if so — and then copies the username into a 33-byte stack buffer anyway, using the original (too-long) length. The safety check exists but does nothing.
**Why it matters:** A stack buffer overflow lets an attacker overwrite whatever is sitting after that buffer on the stack, up to and including the function's return address. That is a classic path to crashing the process or hijacking control of it, and it happens before any password is checked.
**How it's triggered:** Connect to any C-Kermit server/IKSD that requires login (`x_login` set, a normal IKSD configuration) and send a REMOTE LOGIN command whose username field is longer than 32 bytes (up to 255, since the length byte is only range-checked loosely — see the `xunchar()` note under V-3). No valid credentials are needed; the oversized username *is* the attack.
**Proposed fix:** `srv_login()` (`ckcpro.w:2564`) already has the length check; it just never stops execution. Add the same abort idiom (`RESUME; return(-1);`) used two cases earlier in this same file for the analogous "not available" error (`ckcpro.w:2557-2559`):

```c
/* ckcpro.w:2611-2617, before: */
		if (len > LOGINLEN) {
		    errpkt((CHAR *)"Username too long");
		}
		p = srvcmd + 2;		/* Point to it */
		for (i = 0; i < len; i++) /* Copy it */
		  f1[i] = p[i];
		f1[len] = NUL;		/* Terminate it */

/* after: */
		if (len > LOGINLEN) {
		    errpkt((CHAR *)"Username too long");
		    RESUME;		/* Wait for next server command */
		    return(-1);
		}
		p = srvcmd + 2;		/* Point to it */
		for (i = 0; i < len; i++) /* Copy it */
		  f1[i] = p[i];
		f1[len] = NUL;		/* Terminate it */
```
**Fix effort & risk:** Small — one file, mirrors an existing local idiom exactly. `ckcpro.w` is wart-preprocessed: run `make wart && make ckcpro.c` before `make linux`. **This fix is required even after V-3's `xunchar()` clamp** — a clamped-to-94 length still exceeds the 32-byte `LOGINLEN`, so the missing `return` is an independent bug.

### V-2. Unbounded heap write in packet-decode sink `putsrv()`
**In plain terms:** Incoming Kermit packets are "encoded" (compressed with a repeat-count scheme) and then "decoded" back into a buffer called `srvcmd`. The function that writes each decoded byte, `putsrv()`, just writes and advances a pointer — it never checks whether it has reached the end of the buffer. Because the repeat-count encoding lets 3 bytes on the wire expand into up to 94 (or, before V-3 is fixed, up to 255) output bytes, a fairly small packet can decode into far more data than the buffer was sized for.
**Why it matters:** This is a heap buffer overflow with content and length chosen by the remote peer — one of the more dangerous classes of bug, and a strong candidate for full remote code execution, not just a crash.
**How it's triggered:** Ordinary, everyday operations — receiving a file from a malicious/compromised sender, or a server/IKSD session processing an incoming REMOTE command. No special configuration needed.
**Proposed fix:** `putsrv()` (`ckcfns.c:413-419`) already has both `srvcmd` and `srvcmdlen` visible as externs in this file (`ckcfns.c:158-159`), so it can bound itself with no signature or call-site changes:

```c
/* ckcfns.c:413-419, before: */
int /*  Put character in server command buffer  */
putsrv(char c)
/* putsrv */ {
  *srvptr++ = c;
  *srvptr = '\0'; /* Make sure buffer is null-terminated */
  return (0);
}

/* after: */
int /*  Put character in server command buffer  */
putsrv(char c)
/* putsrv */ {
  /* srvcmd holds srvcmdlen usable bytes (see inibufs()); stop one byte  */
  /* short so the terminating NUL below always lands inside the buffer,  */
  /* silently truncating oversized/hostile decoded output instead of     */
  /* writing past the allocation.                                        */
  if (srvptr < srvcmd + srvcmdlen) {
    *srvptr++ = c;
  }
  *srvptr = '\0'; /* Make sure buffer is null-terminated */
  return (0);
}
```
**Fix effort & risk:** Small — single function, no header/signature changes, no macOS/BSD risk. **Clamping `xunchar()` (V-3) does not fix this by itself:** even legitimate 0–94 repeat counts applied repeatedly across one packet can overflow `srvcmd`, so `putsrv()` needs its own bound regardless.

### V-3. Global buffer overflow + memory-leak-over-the-wire in `spar()` padding
**In plain terms:** During the connection handshake, the peer sends a "padding count" byte. The code turns that byte into a number and loops that many times, writing into a fixed 96-byte buffer (`padbuf`) — but the number isn't range-checked, so it can come out far larger than 96. The same corrupted value is later used to read `npad` bytes back out of `padbuf` and transmit them to the peer, so the bug also leaks whatever adjacent memory got overwritten.
**Why it matters:** Out-of-bounds write next to live pointers used throughout the protocol engine (`recpkt`, `rdatap`, `data`, `srvptr` are declared immediately after `padbuf`), plus an information leak that hands the attacker a copy of nearby process memory. This runs on every connection handshake.
**How it's triggered:** A single crafted `S`/`I` packet (or ACK) with field 3 set to a control character (0x00–0x1F).
**Root cause:** `xunchar(ch)` (`ckcker.h:645`) is `((ch)-SP)&0xFF`. The protocol only ever encodes 0–94 as printable `!`..`~`; a control byte in 0x00–0x1F wraps to 225–255 instead of failing safe. Fix the macro centrally:

```c
/* ckcker.h:645-646, before: */
#define tochar(ch) (((ch) + SP) & 0xFF)  /* Number to character */
#define xunchar(ch) (((ch) - SP) & 0xFF) /* Character to number */

/* after: */
#define tochar(ch) (((ch) + SP) & 0xFF)  /* Number to character */
/*
  Character to number.  Valid encoded quantities are the printable chars
  SP+1..SP+95 ('!'..'~'), representing 0-94.  A conforming peer never sends
  anything else here; a corrupted or hostile one might.  Clamp instead of
  wrapping mod 256, so a bad byte maps to 0 (i.e. "field absent/empty")
  instead of an attacker-chosen value up to 255.
*/
#define xunchar(ch)                                                          \
  (((ch) < SP || (ch) > (SP + 94)) ? 0 : (((ch)-SP) & 0xFF))
```
**Centralized vs. per-call-site:** Fix the macro, not the 84 call sites individually. Every site's intended range is the same 0–94, none legitimately needs values above 94, and a single fix closes the whole class (including sites this review didn't enumerate). One thing to check before landing: `ckutio.c:979-981` has its own fallback `#ifndef xunchar` definition — confirm `ckcker.h` is included ahead of it (so the fallback is inert); if not, that copy needs the identical clamp.
**Caveat:** Clamping bounds `npad` to 0–94, which *is* small enough for `padbuf[96]` — so this fully closes V-3. But V-1 (33-byte buffer) and V-2 (no bound at all) still need their own fixes.
**Fix effort & risk:** Small change to a widely-included macro. Grep all 84 call sites first and spot-check the few that combine two `xunchar()` results arithmetically (e.g. `ckcfn2.c:2474-2475` CRC assembly, `ckcfns.c:5025` extended-length) to confirm none relies on the wraparound (this review found none). Needs a `clang-format` pass (multi-line macro) and the byte-identical-rebuild verification given how many translation units include it.

### V-4. Unbounded reply-buffer overflow in Attributes (`A`) packet parsing
**In plain terms:** `gattr()` builds a short reply string (`rpbuf`, 21 bytes) listing which attributes it rejected, one character per rejected attribute. Every rejection branch appends with `*rp++ = c` and never checks whether `rpbuf` is full. The outer loop runs once per attribute *entry* in the incoming packet, bounded only by how many the attacker packs in.
**Why it matters:** Overflow of a function-`static` buffer — corrupts adjacent `static`/global data, on every ordinary file transfer.
**How it's triggered:** A single crafted `A` packet with many minimal (2-byte) invalid attribute entries. Attributes exchange happens automatically before every file; even a default ~90-byte packet carries ~40+ entries, already ~2× `rpbuf`'s capacity.
**Proposed fix:** Add one bound check, applied at all 10 append sites (`ckcfn3.c:1392, 1420, 1458, 1528, 1556, 1598, 1610, 1621, 1651, 1659`). A small macro keeps this DRY and prevents future branches from reintroducing the bug:

```c
/* ckcfn3.c, near the rpbuf declaration (~line 1340), add: */
#define RPBUFL 20  /* Attribute reply */
  static char rpbuf[RPBUFL + 1];
  /* Append one rejection code to rpbuf, silently dropping it once the   */
  /* buffer is full instead of writing past the end -- a hostile packet  */
  /* can pack far more rejected entries than RPBUFL into one A packet.    */
#define RPBUFPUT(ch)                                                         \
  do {                                                                       \
    if (rp < rpbuf + RPBUFL)                                                 \
      *rp++ = (ch);                                                          \
  } while (0)

/* then at each of the 10 sites, e.g. ckcfn3.c:1392, before: */
        *rp++ = c;
/* after: */
        RPBUFPUT(c);
```
**Fix effort & risk:** Small-medium — mechanical but touches 10 sites; diff carefully so none is skipped. Behavioral consequence: once >~20 attributes are rejected in one packet, the reply string is silently truncated (cosmetic only; the accept/reject decision `retcode` is unaffected). Unrelated to `xunchar()` — driven by entry *count*, not a length field.

### V-5. Pre-authentication stack overflow in Telnet NEW-ENVIRON reply builder
**In plain terms:** When connecting to a Telnet server, C-Kermit may be asked to send back environment variable names/values (NEW-ENVIRON). `tn_snenv()` builds this reply in two passes: pass 1 measures needed space and correctly stops at 16 bytes (`if (j < 16)`); pass 2 actually copies bytes into the same 16-byte stack buffer, `varname`, but is missing that check.
**Why it matters:** A malicious Telnet server (or a MITM, since Telnet is unencrypted) can send a "variable name" of ~1000 bytes and overflow a 16-byte stack buffer with attacker-controlled content — a strong RCE candidate, firing automatically during negotiation before the user does anything beyond connecting.
**How it's triggered:** Connect to any Telnet server speaking NEW-ENVIRON. `CK_ENVIRONMENT` is unconditionally compiled in on every Unix build (`ckcdeb.h:1547-1552`).
**Proposed fix:** Add the guard already present in pass 1 (`ckctel.c:3862`) and in the sibling `tn_rnenv()` (`ckctel.c:3687`):

```c
/* ckctel.c:4081-4082, before: */
    default:
      varname[j++] = sb[i];
    }

/* after: */
    default:
      if (j < 16) {
        varname[j++] = sb[i];
      }
    }
```
**Fix effort & risk:** Small — one line, mirroring an existing pattern twice in the same file. No protocol-compatibility concern: RFC 1572 variable names (`USER`, `JOB`, `ACCT`, `PRINTER`, `SYSTEMTYPE`, `DISPLAY`, `LOCATION`) are only a few characters; truncating an oversized/malformed name at 16 bytes affects only already-invalid input. **Apply together with V-18** (widen `varname` to `[17]`) so the terminator write is also safe.

### V-6. Path traversal via unstripped `..` in remote-supplied filenames
**In plain terms:** When receiving a file, the peer sends the filename to write locally. `nzrtol()` converts it to a local path. It has a "strip to base filename" safe mode (`zstrip()`), but that only runs when path handling is `PATH_OFF` — and the compiled-in default is `PATH_AUTO`, which copies the remote name through (control characters aside), `..` and all. `zmkdir()` then creates directories along whatever path it's given with no `..` check.
**Why it matters:** A malicious peer can write anywhere the receiving user's permissions allow — e.g. `../../../../home/victim/.ssh/authorized_keys` — turning a file receive into arbitrary file write (SSH key injection, cron injection). The only real guardrail (`CKROOT`/`zinroot()`) is off unless the user explicitly runs `CHROOT`.
**How it's triggered:** Any ordinary `RECEIVE`/`GET` from a malicious peer, default settings.
**Proposed fix:** Reject any `..` path *component* (not a mere substring — `..foo` is a legitimate filename) lexically, for every branch that doesn't already go through `zstrip()`:

```c
/* ckufio.c, add near zstrip() (~line 2553), before nzrtol(): */
/*
  Z H A S D O T D O T  --  Detect a ".." directory-traversal component.

  Returns 1 if 'name' contains a path component that is exactly ".." (as
  opposed to merely starting with ".." -- e.g. "..foo" is a legitimate
  filename and must NOT be flagged).  Purely lexical; does no filesystem
  access, so it works equally well for paths whose directories don't exist
  yet (the normal case for a fresh RECEIVE into a new subdirectory).
*/
static int
zhasdotdot(char *name) {
    char *p;
    if (!name) {
        return (0);
    }
    for (p = name; *p; p++) {
        if (p[0] == '.' && p[1] == '.' &&
            (p == name || ISDIRSEP(p[-1])) &&
            (p[2] == '\0' || ISDIRSEP(p[2]))) {
            return (1);
        }
    }
    return (0);
}

/* ckufio.c:2465-2466, before: */
  fullname[CKMAXPATH] = NUL;
  debug(F110, "nzrtol fullname", fullname, 0);

/* after: */
  fullname[CKMAXPATH] = NUL;
  if (zhasdotdot(fullname)) {   /* Refuse directory traversal */
      debug(F110, "nzrtol rejecting traversal", fullname, 0);
      zstrip(name, &p);         /* Fall back to bare filename only */
      strncpy(fullname, p, CKMAXPATH);
      fullname[CKMAXPATH] = NUL;
  }
  debug(F110, "nzrtol fullname", fullname, 0);
```
**Fix effort & risk:** Medium — small code, but a security-behavior change to the default receive path; deserves real transfer testing (nested-but-safe paths like `sub/dir/file.txt` must keep working; only literal `..` segments should be affected). Flag it in the changelog: transfers relying on `..` in a received filename now get flattened to a bare filename. Worth a comment in `zmkdir()` pointing back to this check for defense-in-depth.

### V-7. Buffer overflow in FTP client's PASV reply parser
**In plain terms:** When the FTP client requests passive mode, the server replies with a `227 ...` line containing an address/port in parentheses. The code copies the parenthesized text into a fixed 64-byte buffer (`pasv`) character-by-character with no length check — even though the very next copy in the same function, into `ftp_reply_str`, *does* check its bound.
**Why it matters:** Stack/static buffer overflow with content and length controlled by whatever FTP server (or MITM/compromised server) the user connects to.
**How it's triggered:** Ordinary PASV-mode FTP against a server sending an oversized `227` reply — no unusual config.
**Proposed fix:** Add the same bound check used one statement later for `ftp_reply_str` (`ckcftp.c:9721`):

```c
/* ckcftp.c:9707-9709, before: */
      if (pflag == 2) {
        if (c != '\r' && c != ')') {
          *pt++ = c;
        } else {

/* after: */
      if (pflag == 2) {
        if (c != '\r' && c != ')') {
          if (pt < &pasv[sizeof(pasv) - 1]) {
            *pt++ = c;
          }
        } else {
```
**Fix effort & risk:** Small — mirrors the sibling check at `ckcftp.c:9721`. A malformed/oversized `227` reply now truncates to 63 bytes; the subsequent parse fails cleanly rather than crashing.

### V-8. Unbounded global array write in server-mode GET filename parsing
**In plain terms:** When a local `SERVER`/`kermit -x` session receives a GET request, the requested filenames are split on whitespace by `fnparse()` into a fixed array of pointers, `msfiles[MSENDMAX]` (1024 entries). `fnparse()` does `msfiles[r] = p; r++;` for every token with no check that `r` stays under 1024.
**Why it matters:** Overflow of a fixed global array corrupts adjacent global data — remote, needs no authentication beyond already being served by a local SERVER session.
**How it's triggered:** A remote peer sends a GET whose (unquoted) filespec has >1024 space-separated tokens (~2KB string, well within normal packet sizes).
**Proposed fix:** Add a bound check where each pointer is recorded (`ckuusx.c:2334-2339`):

```c
/* ckuusx.c:2334-2339, before: */
    } else if (*s == SP || *s == NUL) { /* Unquoted space or NUL? */
      *q++ = NUL;                       /* End of output filename. */
      msfiles[r] = p;                   /* Add this filename to the list */
      debug(F111, "fnparse", msfiles[r], r);
      r++; /* Count it */
      if (*s == NUL) {

/* after: */
    } else if (*s == SP || *s == NUL) { /* Unquoted space or NUL? */
      *q++ = NUL;                       /* End of output filename. */
      if (r < MSENDMAX - 1) {           /* Leave room for the NULL entry */
        msfiles[r] = p;                 /* Add this filename to the list */
        debug(F111, "fnparse", msfiles[r], r);
        r++; /* Count it */
      }
      if (*s == NUL) {
```
(The `MSENDMAX - 1` headroom matters: the function terminates the list with `msfiles[r] = "";` after the loop at `ckuusx.c:2354`, so `r` must never reach `MSENDMAX`.)
**Fix effort & risk:** Small — one bound check. A GET with >1023 filespecs silently drops the excess; no legitimate use sends anywhere near that many.

---

# High (V-9 … V-15)

### V-9. CONNECT-mode escape buffer overflow (`ecbuf[10]`) — ASan-confirmed
**In plain terms:** While connected to a remote system in terminal mode, typing the escape key then `\` starts a mini text field that Kermit collects into a 10-byte buffer (`ecbuf`) until Enter. The collection loop never checks whether the buffer is full — it just keeps stuffing in whatever you typed.
**Why it matters:** Anything typed (or piped in) after the `\` without a prompt CR/LF overwrites memory next to that 10-byte buffer. Worst case is memory corruption; materially worse in setuid/setgid deployments (`priv_on()`/`priv_off()`).
**How it's triggered:** In CONNECT mode: escape char, `\`, then keep sending without CR/LF. Keyboard input is read in bulk (up to 257 bytes), so scripted/piped stdin triggers it just as easily as live typing. Confirmed live under AddressSanitizer.
**Proposed fix:** Bound the write inside the existing collection loop (leave room for the terminating NUL). Apply identically to **both** near-identical files:

```c
/* ckucns.c:2456-2464 and ckucon.c:2459-2467 (identical fix in both files) */
/* Before */
      if (c == CMDQ) { /* Backslash escape */
        int x;
        ecbp = ecbuf;
        *ecbp++ = c;
        while (((c = (CONGKS() & cmdmsk)) != '\r') && (c != '\n')) {
          *ecbp++ = c;
        }
        *ecbp = NUL;

/* After */
      if (c == CMDQ) { /* Backslash escape */
        int x;
        ecbp = ecbuf;
        *ecbp++ = c;
        while (((c = (CONGKS() & cmdmsk)) != '\r') && (c != '\n')) {
          if (ecbp < ecbuf + sizeof(ecbuf) - 1) { /* Room left in ecbuf? */
            *ecbp++ = c;
          }
        }
        *ecbp = NUL;
```
**Fix effort & risk:** Small; one added `if` in each of two files. An escape sequence longer than 9 characters is now silently truncated (and rejected by `xxesc()` as invalid, beeping) instead of corrupting memory — no legitimate escape code is anywhere near that long.

### V-10. Heap overflow on every startup from `sizeof spdtab` pointer-size bug — ASan-confirmed
**In plain terms:** At startup Kermit re-sorts its serial-speed list into numeric order, copying each speed's text (like `"9600"`) into new buffers and back. The copy-back computes "how many bytes may I write" using `sizeof spdtab` — but `spdtab` is a *pointer*, not the array, so `sizeof` returns the pointer size (8 bytes on 64-bit), giving `n = 10`. That's then used as if it were the true capacity of every destination buffer, even though those were allocated just barely large enough for their specific string (as little as 2 bytes for `"0"`).
**Why it matters:** Runs unconditionally, every `wermit` startup, no attacker input needed — real heap corruption of the process's own data every run.
**How it's triggered:** Automatically, in `cmdini()` from `main()`, whenever `TTSPDLIST` is defined (true for any `UNIX` build per `ckcdeb.h:1970-1977`, not just Linux) and `NOSORTSPEEDS` is not (it never is in this tree).
**Proposed fix:** The destination length must equal the allocation size, not `sizeof(pointer)`. Make every buffer a uniform `maxspeedlen + 2`, so one correctly-computed `n` is valid everywhere. This also fixes the related `speeds[i]` defect (allocated 12 bytes, copied into with a claimed 20). **Both edits are required together** — fixing `n` alone leaves the overflow, because `spdtab[i].kwd` is populated *before* the sort block runs.

```c
/* ckuus5.c:997 -- original per-entry allocation, sized to the individual
   string instead of a shared worst-case bound: */
/* Before */
        if ((n > 0) && (p = (char *)malloc(n + 1))) {
/* After: reserve enough room for any speed string */
        if ((n > 0) && (p = (char *)malloc(20 + 2))) { /* match maxspeedlen+2 */
```
```c
/* ckuus5.c:1030-1053 -- the NOSORTSPEEDS block */
/* Before */
    int n = sizeof spdtab + 2;
    int maxspeedlen = 20;
    ...
    for (i = 0; i < nspd; i++) { /* Allocate string storage */
      speeds[i] = malloc(n + 2);
      tmp[i].kwd = malloc(maxspeedlen + 2);
    }
    for (i = 0; i < nspd; i++) { /* Copy speeds into a sortable array */
      ckstrncpy(speeds[i], spdtab[i].kwd, maxspeedlen);
    }
/* After */
    int maxspeedlen = 20;
    int n = maxspeedlen + 2; /* Bug fix: was sizeof spdtab (a pointer!) + 2 */
    ...
    for (i = 0; i < nspd; i++) { /* Allocate string storage */
      speeds[i] = malloc(n);     /* Was malloc(n+2)=12 bytes, too small */
      tmp[i].kwd = malloc(n);
    }
    for (i = 0; i < nspd; i++) { /* Copy speeds into a sortable array */
      ckstrncpy(speeds[i], spdtab[i].kwd, n);
    }
```
The later copies (`ckstrncpy(tmp[i].kwd, spdtab[k].kwd, n)` and `ckstrncpy(spdtab[i].kwd, tmp[i].kwd, n)`) need no change once `n` and every participating buffer are consistently `maxspeedlen + 2`.
**Fix effort & risk:** Small-medium; two spots in one file, done together. Trivial memory cost (~20 bytes × ~20 entries). `TTSPDLIST` is UNIX-wide, so the same fix applies unmodified on macOS/BSD.

### V-11. Remote `!filename` triggers local shell execution via `SET TRANSFER PIPES ON`
**In plain terms:** Kermit has a documented feature: with "Transfer Pipes" on, a file whose name starts with `!` has the rest of the name treated as a shell command to pipe through. The same flag, once on, also applies on *receive* — a file arriving from the peer named `!<cmd>` has its `!` stripped and the remainder run as a shell command locally, with the incoming file content piped to its stdin.
**Why it matters:** The feature is designed for outbound piping under your control, but the same on/off switch silently also lets the *remote peer* pick and run a command on your machine — a much riskier consequence than the setting's name suggests.
**How it's triggered:** Local user runs `SET TRANSFER PIPES ON` (off by default) for legitimate reasons; any subsequent receive from an untrusted peer named `!<shell command>` executes it locally.
**Proposed fix:** Split the single `usepipes` flag into send- and receive-direction flags, so enabling outbound piping does not silently also enable inbound. Gate the receive path (`ckcfns.c:3457`) on a new receive-specific flag (default off) and warn when a peer requests it:

```c
/* ckcfns.c:3455-3467, after (add usepipes_recv gate + warning): */
#ifdef PIPESEND
  /* If it starts with "bang", it's a pipe, not a file. Note: this is a
     REMOTE PEER requesting local command execution on receipt, distinct
     from the local user's own SEND-side piping (usepipes). Require a
     separate opt-in so enabling outbound piping doesn't silently also
     authorize inbound command execution. */
  if (usepipes && usepipes_recv && protocol == PROTO_K && *srvcmd == '!' &&
      !rcvfilter) {
    CHAR *s;
    printf("Warning: peer requested pipe execution: %s\n", srvcmd + 1);
    s = srvcmd + 1;  /* srvcmd[] is not a pointer. */
    while (*s) {     /* So we have to slide the contents */
      *(s - 1) = *s; /* over 1 space to the left. */
      s++;
    }
    *(s - 1) = NUL;
    pipesend = 1;
  }
#endif /* PIPESEND */
```
Requires `int usepipes_recv = 0;` next to `usepipes` in `ckcmai.c:777`, a matching `extern` in `ckcfns.c:130`, and a new keyword (e.g. `SET TRANSFER PIPES RECEIVE ON`) wired into the SET parser (`ckuus6.c:10702-10703`) and SHOW output (`ckuus4.c`).
**Fix effort & risk:** Medium — genuinely new command-table entry and help text touch `ckuus3.c`/`ckuus4.c`/`ckuus6.c`, and it's a behavior change users must opt into again even if they had `TRANSFER PIPES ON`. The `popen()`/`execl()` mechanics in `zxcmd()` are untouched; only the gating condition changes.

### V-12. Recursive FTP GET/MGET path traversal via server-supplied names
**In plain terms:** During a recursive `GET`/`MGET`, Kermit asks the FTP server for a directory listing and creates local files/directories matching whatever names came back — including `../../..` names that walk out of your download directory.
**Why it matters:** A hostile/compromised FTP server can plant or overwrite files anywhere your account can write.
**How it's triggered:** Any recursive `GET`/`MGET` (`/RECURSIVE`) against an untrusted server. The NLST name (`ckcftp.c:6788`/`6813`) or MLSD entry name (`ckcftp.c:12946-12967`) is used to build the local path with no filtering.
**Proposed fix:** Reject any server-supplied name with a `..` component or a leading separator before it builds a local path. A small helper checked at both use sites:

```c
/* New static helper, near remote_files() in ckcftp.c: */
static int has_dotdot(char *s) { /* Reject ../ traversal + absolute paths */
  char *p = s;
  if (ISDIRSEP(*p)) {
    return (1);
  }
  while (*p) {
    if (p[0] == '.' && p[1] == '.' &&
        (p[2] == '\0' || ISDIRSEP(p[2])) &&
        (p == s || ISDIRSEP(p[-1]))) {
      return (1);
    }
    p++;
  }
  return (0);
}
```
```c
/* ckcftp.c:12946 (MLSD recursion), before the CWD/recurse: */
      if (has_dotdot(p)) {
        printf("?Refused: unsafe path from server: %s\n", p);
        goto again;
      }
      if (mlsdepth < MLSDEPTH) {
        ...
```
Symmetrically guard the `local = *s2 ? s2 : s;` assembly at `ckcftp.c:6813` before it opens/creates the local file.
**Fix effort & risk:** Medium — two sites in an intricate function; only `..` *components* and leading separators should be blocked, not all separators (recursive transfers legitimately create subdirectories). Consider reusing `zinroot()`'s lexical logic if it's factored out as part of V-6.

### V-13 & V-14. `\K`-doubling (`SET KEY`) and `OUTPUT`/`LINEOUT` quoting overflow the global `line[]` buffer — confirmed
**In plain terms:** Both `SET KEY` (defining a key mapping) and `OUTPUT`/`LINEOUT` (sending literal text) run your text through a loop that doubles up certain backslash sequences so they survive a later expansion step. That loop writes its (grown) result into a single global scratch buffer, `line[]`, but only checks whether the result *fits* after the loop has already finished writing.
**Why it matters:** The loop can add up to ~1.5 characters per input character, so a long adversarial input overflows `line[]` well before the length check runs, corrupting adjacent global data.
**How it's triggered:** V-13 — a `SET KEY` definition packed with `\K`/`\k` pairs, near the command-line length limit (via a long `.kermrc`/take-file line, macro, or direct typing). V-14 — an `OUTPUT`/`LINEOUT` argument packed with `\n`/`\b`/`\l`/`\\` (live by default; command quoting is on by default). Confirmed by extracting the loop and overflowing a 20-byte buffer under ASan with 14 bytes.
**Proposed fix (shared):** Bound the write cursor *inside* the loop against `line[]`'s real end:

```c
/* ckuus3.c:6789-6805 (dosetkey(), \K-doubling loop) */
/* Before */
  for (x = 0, y = 0; s[x]; x++, y++) {             /* Convert \K to \\K */
    if ((x > 0) && (s[x] == 'K' || s[x] == 'k')) { /* Have K */
      if ((x == 1 && s[x - 1] == CMDQ) ||
          (x > 1 && s[x - 1] == CMDQ && s[x - 2] != CMDQ)) {
        line[y++] = CMDQ; /* Make it \\K */
      }
      if (x > 1 && s[x - 1] == '{' && s[x - 2] == CMDQ) {
        line[y - 1] = CMDQ; /* Have \{K */
        line[y++] = '{';    /* Make it \\{K */
      }
    }
    line[y] = s[x];
  }
  line[y++] = NUL;                       /* Terminate */

/* After */
  for (x = 0, y = 0; s[x] && y < LINBUFSIZ - 1; x++, y++) { /* Convert \K to \\K */
    if ((x > 0) && (s[x] == 'K' || s[x] == 'k')) { /* Have K */
      if ((x == 1 && s[x - 1] == CMDQ) ||
          (x > 1 && s[x - 1] == CMDQ && s[x - 2] != CMDQ)) {
        if (y < LINBUFSIZ - 1) {
          line[y++] = CMDQ; /* Make it \\K */
        }
      }
      if (x > 1 && s[x - 1] == '{' && s[x - 2] == CMDQ) {
        line[y - 1] = CMDQ; /* Have \{K */
        if (y < LINBUFSIZ - 1) {
          line[y++] = '{'; /* Make it \\{K */
        }
      }
    }
    if (y < LINBUFSIZ - 1) {
      line[y] = s[x];
    }
  }
  if (y >= LINBUFSIZ - 1) { /* Definition too long even after truncation */
    printf("?Key definition too long\n");
    if (flag) {
      cmsetp(psave);
    }
    return (-9);
  }
  line[y++] = NUL; /* Terminate */
```
The identical pattern applies to `ckuusr.c:8397-8407` (`OUTPUT`/`LINEOUT` quoting loop): add `&& y < LINBUFSIZ - 1` to the `for`, guard each of the two writes, then check `y >= LINBUFSIZ - 1` before the terminator and report `?Output text too long` (matching the existing `return (success = 0);` failure path two lines later).
**Fix effort & risk:** Small-medium; keep the `\{K`/`\K` two-character-lookback logic correct at the new truncation boundary (don't split a two-character insertion). An over-long definition/argument now errors out instead of corrupting memory — matching the existing "too long" UX in the same functions.

### V-15. Unbounded completion-insertion copies into global `cmdbuf`
**In plain terms:** When you Tab/Esc-complete a filename, three completion paths (tilde `~` expansion, partial-match repaint, unique-directory completion) each copy the completed text into the in-progress command line with a raw `while ((*bp++ = *sp++));` loop — no check on room left in `cmdbuf`.
**Why it matters:** If the buffer is nearly full, memory after `cmdbuf` gets overwritten. The file already has a proven-safe insertion helper (`addbuf()`, `ckucmd.c:7006`, stops at `cmdbuf + CMDBL`), used at a fourth sibling site (`ckucmd.c:2477`) — these three just don't use it.
**How it's triggered:** A user or tty-driving harness (`expect`/`tmux send-keys`; only needs `is_a_tty(0)`) types/pastes a long line, then Tab/Esc-completes near the end. Under default `BIGBUFOK` (`CMDBL`≈32763) this needs a lot of prior input; under `-DNOBIGBUF` (`CMDBL`=4092/608) it's much easier.
**Proposed fix:** `addbuf()` isn't a drop-in (it appends a trailing space and updates `np`/`cmbptr`, changing mid-line-insertion behavior). Add a second small helper sharing `addbuf()`'s exact bound without the trailing-space side effect, and use it at all three sites:

```c
/* ckucmd.c -- add next to addbuf(), ~line 7017 */
/*
  B U F C P Y  --  Bounded copy of a NUL-terminated string into the command
  buffer at the current position bp, without addbuf()'s trailing space.
  Used for in-place completion/insertion (tilde expansion, partial-match
  repaint, unique-directory completion) where the cursor must land right
  after the inserted text.  Shares addbuf()'s bound so cmdbuf can never
  be overrun; silently truncates if the text doesn't fit.
*/
static void bufcpy(char *sp) {
  while (*sp && bp < (cmdbuf + CMDBL)) {
    *bp++ = *sp++;
  }
  *bp = NUL;
}
```
```c
/* ckucmd.c:2210-2212, :2393-2395, :2427-2429 -- same replacement at all three */
/* Before */
          while ((*bp++ = *sp++))
            ;   /* Copy to command buffer */
          bp--; /* Back up over NUL */
/* After */
          bufcpy(sp); /* Bounded copy; leaves bp at the terminating NUL */
```
**Fix effort & risk:** Small; one ~5-line helper plus three mechanical substitutions, reusing the already-audited `addbuf()` bound. Works with both the static `cmdbuf[CMDBL+4]` and the `DCMDBUF` dynamic variant (same `cmdbuf + CMDBL` bound). A completion that would have overflowed now silently truncates, consistent with `addbuf()`'s existing behavior.

---

# Medium (V-16 … V-28)

### V-16. FTP client trusts the PASV reply's IP address (SSRF-style data-connection redirect)
**In plain terms:** When your FTP client asks for passive mode, the server writes back the IP and port to connect to for the transfer. The client connects there blindly — it never checks that the address matches the server it's already talking to.
**Why it matters:** A malicious/compromised server can redirect your client's data connection to some other machine (an internal service, a portscan/flood target with your host as the source) — the FTP analogue of SSRF. Default passive-mode behavior.
**How it's triggered:** Any ordinary passive-mode session against a hostile/hijacked server.
**Proposed fix:** The standard client mitigation (curl's `--ftp-skip-pasv-ip`): ignore the reply's address, always reconnect to the control-connection host; take only the port from the server.

```c
/* ckcftp.c:11362-11372, inside initconn() */
     {
       data_addr.sin_family = AF_INET;
       /* [V-16] Never trust the address in a PASV reply -- a malicious
          server can redirect the data connection to an arbitrary
          host:port (SSRF-style). Always reconnect to the same host as
          the already-established control connection; take only the
          port from the reply. */
       data_addr.sin_addr = hisctladdr.sin_addr;
       data_addr.sin_port = htons((p1 << 8) | p2);

       if (connect(data, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
```
**Fix effort & risk:** Small. Breaks the rare legitimate case of a multi-homed server whose data channel lives on a different IP than its control channel — an accepted, industry-standard tradeoff.

### V-17. `.netrc` parser buffer overflow
**In plain terms:** When Kermit reads `~/.netrc` (saved FTP credentials), it copies each token into a fixed 100-byte buffer character-by-character with no limit.
**Why it matters:** A `.netrc` token ≥100 bytes overflows a static buffer. Local-file-controlled, not remote — practical risk is untrusted `.netrc` (shared/synced dotfiles, a compromised account writing to your home dir).
**How it's triggered:** FTP `.netrc` auto-login against a file with a token ≥100 bytes.
**Proposed fix:** Bound both copy loops against the buffer size (`ckcftp.c:13392-13434`):

```c
 static char tokval[100];
#define TOKVALMAX (tokval + sizeof(tokval) - 1) /* leave room for NUL */
 ...
   cp = tokval;
   if (c == '"') {
     while ((c = getc(cfile)) != EOF && c != '"') {
       if (c == '\\') { c = getc(cfile); }
       if (cp < TOKVALMAX) { *cp++ = c; }
     }
   } else {
     if (cp < TOKVALMAX) { *cp++ = c; }
     while ((c = getc(cfile)) != EOF && c != '\n' && c != '\t' && c != ' ' &&
            c != ',') {
       if (c == '\\') { c = getc(cfile); }
       if (cp < TOKVALMAX) { *cp++ = c; }
     }
   }
   *cp = 0;
```
**Fix effort & risk:** Small. Silent truncation of over-length tokens matches the codebase's conservative fallback. Inherited BSD `ruserpass.c` code, unrelated to the SVR4 chain.

### V-18. Off-by-one write past `varname[16]` in Telnet NEW-ENVIRON reply builder
**In plain terms:** `tn_snenv()` keeps the current "variable name" in a 16-byte buffer. Four places terminate it with `varname[j] = '\0'`, and `j` can legitimately reach exactly 16 — writing one byte past the array.
**Why it matters:** One-byte stack overflow reachable from any Telnet server. The sibling `tn_rnenv()` already declares `varname[17]` for exactly this reason.
**How it's triggered:** A NEW-ENVIRON `SEND` subnegotiation with a variable name ≥16 bytes.
**Proposed fix:** Match `tn_rnenv()`'s sizing (`ckctel.c:3712-3714`):

```c
 int tn_snenv(CHAR *sb, int len)
 /* tn_snenv */ { /* Send new environment */
-  char varname[16];
+  char varname[17]; /* [V-18] match tn_rnenv() -- j can reach 16 in both
+                        passes, and varname[j]='\0' then writes byte 16. */
```
**Fix effort & risk:** Trivial, but **only closes half the gap** — pass 2's write at `ckctel.c:4082` still has no upper bound on `j` (that's V-5). Apply this **together with V-5**'s `if (j < 16)` guard for the buffer to actually be safe.

### V-19. Predictable serial-lock-file name created without `O_EXCL`, under privilege elevation
**In plain terms:** When Kermit locks a serial device it first creates a temp file `LTMP.<pid>` in a shared lock dir (typically world-writable `/var/lock`) using `creat()`, which doesn't check whether that name already exists — including as a symlink.
**Why it matters:** In setuid/setgid builds (which this codebase supports), the PID is guessable and the dir is shared, so another local user can pre-create `LTMP.<pid>` as a symlink to any file; the victim's elevated `creat()` follows it and truncates the target — classic symlink-race privilege escalation. Not relevant to the plain single-user Linux build.
**How it's triggered:** Only in setuid/setgid deployments.
**Proposed fix:** Use `open()` with `O_CREAT|O_EXCL` (`ckutio.c:3902-3908`):

```c
   priv_on();                    /* Turn on privileges if possible. */
-  lockfd = creat(tmpnam, 0444); /* Try to create temp lock file. */
+  /* [V-19] O_EXCL refuses to follow/replace a pre-existing name --
+     including a symlink another local user planted in the shared,
+     world-writable lock directory -- closing a TOCTOU/symlink race
+     that plain creat() leaves open while running under priv_on(). */
+  lockfd = open(tmpnam, O_WRONLY | O_CREAT | O_EXCL, 0444);
   if (lockfd < 0) {             /* Create failed. */
```
**Fix effort & risk:** Small. `fcntl.h` is already included; standard POSIX flags, no macOS/BSD gating. A pre-existing name now correctly falls into the existing error path.

### V-20. `SET MODEM DIAL-COMMAND` used as an unvalidated `printf` format string
**In plain terms:** The user-configured dial template (e.g. `"ATDT%s\r"`) is handed straight to `sprintf()` as the *format*, with the phone number as its argument. Setup only checks that `%s` appears — not that nothing else does.
**Why it matters:** If `DIAL-COMMAND` contains `%n` (or a second `%s`, etc.) — e.g. from a booby-trapped `.kermrc`/dial-directory script — `sprintf()` reads/writes through stack garbage; `%n` is a write-to-arbitrary-address primitive.
**How it's triggered:** `DIAL` after a malicious `SET MODEM DIAL-COMMAND` value has been loaded.
**Proposed fix:** Stop treating the string as a format; substitute the single `%s` by hand.

```c
/* ckudia.c, add a static helper used at line 5422: */
/*
  d i a l e x p a n d  --  [V-20]
  Substitute the phone number for the *first* literal "%s" in a
  user-configured DIAL-COMMAND string, without ever passing that string
  to printf()/sprintf() as a format.  Any other '%' sequence (a second
  "%s", "%n", "%%", ...) is copied through as ordinary text.
*/
static void dialexpand(char *dst, const char *fmt, const char *num,
                        int dstsize) {
  const char *f = fmt;
  char *d = dst, *dend = dst + dstsize - 1; /* leave room for NUL */
  int replaced = 0;

  while (*f && d < dend) {
    if (!replaced && f[0] == '%' && f[1] == 's') {
      const char *n = num;
      while (*n && d < dend) { *d++ = *n++; }
      f += 2;
      replaced = 1;
    } else {
      *d++ = *f++;
    }
  }
  *d = '\0';
}
```
```c
/* ckudia.c:5418-5423 */
   REDIAL:
     if ((int)strlen(dcmd) + (int)strlen(xnum) > LBUFL) {
       ckstrncpy(lbuf, "NUMBER TOO LONG!", LBUFL);
     } else {
-      sprintf(lbuf, dcmd, xnum); /* safe (prechecked) */
+      dialexpand(lbuf, dcmd, xnum, (int)sizeof(lbuf)); /* [V-20]: no printf */
     }
```
**Fix effort & risk:** Medium (new helper, one call-site swap). Behavior for well-formed values (one `%s`) is byte-for-byte identical.

### V-21. File descriptor leak on every transfer that overwrites an existing file
**In plain terms:** Before opening a destination for real, `zopeno()` does a throwaway `open()` just to test `isatty()`. On the normal (non-tty) path it never closes that probe descriptor.
**Why it matters:** Every overwrite leaks one fd. A long-running server (IKSD handling repeated uploads) eventually hits `EMFILE` — a slow self-inflicted DoS.
**How it's triggered:** Trivially — any transfer whose destination already exists.
**Proposed fix:** Close the probe descriptor on the non-tty path (`ckufio.c:1209-1218`):

```c
     fd = open(name, O_WRONLY | flags, 0600);
     if (fd > -1) {
       if (isatty(fd)) {
         filefd = fd;
         istty++;
+      } else {
+        close(fd); /* [V-21] probe-only open for isatty(); the real
+                       open happens below via fopen() -- this was never
+                       closed on the non-tty path, leaking one fd per
+                       overwrite. */
       }
     }
```
**Fix effort & risk:** Trivial, one line. No change on the tty path (`filefd` still handed to `fdopen()`).

### V-22. No `O_NOFOLLOW`/atomicity between the writability precheck and the real open
**In plain terms:** Kermit checks "can I write here?" (`zchko()`) and then, separately, opens the file for writing (`zopeno()`). Nothing stops the path from being swapped for a symlink in the gap.
**Why it matters:** Classic TOCTOU/symlink race — but requires another local user with write access to the same directory to win a timing race; no remote peer can place the symlink directly, so this is defense-in-depth for shared directories, not a confirmed remote hole.
**How it's triggered:** Local, race-dependent; needs a shared-write destination directory.
**Proposed fix:** Add `O_NOFOLLOW` to the real write-open. The tty-output feature (`/dev/...`, `istty`) uses a different path (`fdopen(filefd, p)`) and is unaffected.

```c
/* ckufio.c:1219-1226 */
   if (istty) {
     fp[n] = fdopen(filefd, p);
   } else {
-    fp[n] = fopen(name, p); /* Try to open the file */
+    /* [V-22] O_NOFOLLOW: refuse to write through a symlink planted at
+       the destination name between zchko()'s precheck and this open. */
+    int wflags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC) |
+                 O_NOFOLLOW;
+    int wfd = open(name, wflags, 0600);
+    fp[n] = (wfd > -1) ? fdopen(wfd, p) : NULL;
+    if (wfd > -1 && !fp[n]) {
+      close(wfd);
+    }
   }
```
For symmetry, add `O_NOFOLLOW` to the `zchko()` probe open (`ckufio.c:2275`).
**Fix effort & risk:** Medium — restructures the non-tty open from `fopen()` to `open()`+`fdopen()`. `O_NOFOLLOW` is POSIX.1-2008, present on Linux/macOS/BSD. Smoke-test an overwriting transfer afterward.

### V-23. Out-of-bounds read `table[-1]` in keyword lookup under DEBUG logging
**In plain terms:** `nlookup()` tracks the last matching table entry in `lastmatch`, initialized to `-1`. On the first non-matching entry — the common case — a debug line reads `table[lastmatch].kwd` = `table[-1].kwd`: memory before the array, treated as a string pointer.
**Why it matters:** `DEBUG` is compiled in by default; this fires on nearly every `nlookup()` (e.g. `SET SPEED`) as soon as debug logging is on (`-d` or `SET DEBUG ON` + `LOG DEBUG` — ordinary troubleshooting). Reading garbage as a pointer can crash, or leak adjacent memory into the debug log.
**How it's triggered:** Ordinary debug logging during a numeric-keyword lookup with no matching prefix yet.
**Proposed fix:** Use `this` (the current entry's `kwd`, always valid) instead of the stale `lastmatch` index (`ckucmd.c:7847-7850`):

```c
     tmp = strncmp(this, word, wordlen);
     if (tmp) {
-      debug(F111, "nlookup no match", table[lastmatch].kwd, tmp);
+      debug(F111, "nlookup no match", this, tmp); /* [V-23]: 'this' is the
+         current (non-matching) entry, always valid; lastmatch is -1 until
+         some entry matches, so the old code read table[-1]. */
     } else {
```
**Fix effort & risk:** Trivial, one line; also a correctness improvement (logs the entry actually compared). No impact outside DEBUG runs.

### V-24. `filbuf` sized for `ATMBL` but NUL-terminated at `CKMAXPATH` (`-DNOBIGBUF` only)
**In plain terms:** The filename-completion buffer `filbuf` is sized from `ATMBL`, but the code always writes a terminating NUL at index `CKMAXPATH` regardless of `filbuf`'s real size.
**Why it matters:** Harmless under default `BIGBUFOK` (`ATMBL`=10238 ≫ `CKMAXPATH`≈4096). But `-DNOBIGBUF` is a supported build option where `ATMBL` drops to 1024 (`filbuf`=1028 bytes) — then `filbuf[CKMAXPATH]` writes ~3068 bytes past the buffer on every successful completion.
**How it's triggered:** Any successful completion, but only in `-DNOBIGBUF` builds.
**Proposed fix:** Bound to `filbuf`'s real size (`ckucmd.c:2258-2263`):

```c
       if (y > 0) {
-        ckstrncpy(filbuf, mtchs[0], CKMAXPATH);
+        ckstrncpy(filbuf, mtchs[0], (int)sizeof(filbuf));
       } else {
         *filbuf = '\0';
       }
-      filbuf[CKMAXPATH] = NUL;
+      filbuf[sizeof(filbuf) - 1] = NUL; /* [V-24]: bound to filbuf's real
+         size (ATMBL+4), not CKMAXPATH -- under -DNOBIGBUF ATMBL (1024) is
+         far smaller than CKMAXPATH (~4096 on Linux). */
```
**Fix effort & risk:** Trivial. No change under default `BIGBUFOK` (same effective bound); only affects — and fixes — the `-DNOBIGBUF` build.

### V-25. Off-by-one heap overflow in `\s(name[n.])` compact-substring notation — confirmed
**In plain terms:** `\s(var[N.])` takes the first `N` characters of a value. The bounds check rejects `N` past the end but not exactly *at* the end, so asking for one more character than exists writes one byte past a heap allocation.
**Why it matters:** `q` is `malloc(k+1)` (valid 0..k); when the guard lets `x1 == k` through, `q[x1+1]` = `q[k+1]` — one past. Reachable through ordinary macro/command substring syntax; ASan-confirmed. Crash/heap-corruption, not info disclosure.
**How it's triggered:** `\s(var[N.])` where `N == strlen(value)+1`.
**Proposed fix:** Bound the write, matching the sibling `':'` branch (`ckuus4.c:15408-15413`):

```c
                 if ((q = malloc(k + 1))) {
                   strcpy(q, vp); /* safe */
                   if (c == '.') {
-                    q[x1 + 1] = NUL;
+                    if (x1 + 1 <= k) { /* [V-25]: x1==k means "all of the
+                         value already fits"; only write the truncation
+                         NUL at an in-bounds index (q is malloc(k+1)). */
+                      q[x1 + 1] = NUL;
+                    }
                     debug(F000, "XXX. q", q, c);
                   }
```
**Fix effort & risk:** Trivial. `N == strlen(value)+1` now returns the full string (sensible "asked for more than exists" fallback) instead of corrupting the heap — consistent with the `:`/`_` branches.

### V-26. Unguarded 128-element table index in ELOT-927→Latin/Greek translation, remote info leak
**In plain terms:** `xeglg()` translates one character via a 128-entry table `yeglg[]`, indexed directly by the input byte with no range check. Every sibling function in the file (e.g. `xh7lh`) has that check — this one is missing it.
**Why it matters:** An input byte ≥128 reads past the table into adjacent static memory, and the result is a translated character *sent to the remote peer* — adjacent memory leaks over the wire (and the garbage value could crash later processing).
**How it's triggered:** `SET TRANSFER-CHARACTER-SET LATIN/GREEK` + `SET FILE-CHARACTER-SET ELOT927-GREEK`, then sending a file with a byte ≥0x80. A normal (non-default) supported charset config.
**Proposed fix:** Add the guard every sibling uses (`ckuxla.c:5396-5398`):

```c
 CHAR xeglg(CHAR c) { /* xeglg */ /* Latin/Greek to ELOT 927 */
-  return (yeglg[c]);
+  if (c & 0x80) { /* [V-26]: yeglg[] has only 128 entries; match the
+                     guard every sibling translator uses (e.g. xh7lh). */
+    return (UNK);
+  }
+  return (yeglg[c]);
 }
```
**Fix effort & risk:** Trivial, matching an existing in-file pattern. `UNK` is already used for this purpose in the file.

### V-27. `SET EDITOR`/`SET BROWSER /OPTIONS` used as an unvalidated `printf` format string
**In plain terms:** Same shape as V-20: the user-configurable `EDITOR`/`BROWSER` options string is passed to `sprintf()` as a *format*, with the filename/URL as its argument, whenever it contains `%s`. The check only confirms `%s` is present.
**Why it matters:** Requires a `.kermrc` value like `SET EDITOR /OPTIONS:"...%n..."` to already be in place (compromised/shared dotfile or take-script), then `EDIT`/`BROWSE`. Same UB-via-`%n` as V-20.
**How it's triggered:** `EDIT`/`BROWSE` after a hostile options string is loaded.
**Proposed fix:** Reuse the V-20 approach — a `%s`-only expander — at both sites (`ckuusr.c:6389-6398`, `6433-6441`):

```c
/* ckuusr.c, add before doedit() (~line 6337); shared by dobrowse(): */
/*
  p c t s e x p a n d  --  [V-27]  Substitute 'arg' for the *first*
  literal "%s" in a user-configured EDITOR/BROWSER options string,
  without handing that string to sprintf() as a format.  Any other '%'
  is copied through as ordinary text.
*/
static void pctsexpand(char *dst, const char *opts, const char *arg,
                        int dstsize) {
  const char *f = opts;
  char *d = dst, *dend = dst + dstsize - 1;
  int replaced = 0;

  while (*f && d < dend) {
    if (!replaced && f[0] == '%' && f[1] == 's') {
      const char *a = arg;
      while (*a && d < dend) { *d++ = *a++; }
      f += 2;
      replaced = 1;
    } else {
      *d++ = *f++;
    }
  }
  *d = '\0';
}
```
```c
/* ckuusr.c:6388-6398 (doedit) and 6432-6441 (dobrowse) -- same edit twice */
   if (((int)strlen(editopts) + (int)strlen(editfile) + 1) < TMPBUFSIZ) {
     if (x) {
-      sprintf(tmpbuf, editopts, editfile);
+      pctsexpand(tmpbuf, editopts, editfile, TMPBUFSIZ); /* [V-27]: no printf */
     } else {
       sprintf(tmpbuf, "%s %s", editopts, editfile); /* both literals, safe */
     }
   }
```
**Fix effort & risk:** Medium (shared helper, two swaps). Identical behavior for well-formed options with one `%s`.

### V-28. `setword()` partial-initialization NULL dereference under allocation failure
**In plain terms:** `setword()` (used by word-splitting) allocates two parallel arrays on first call. If the first `malloc()` succeeds but the second fails, it returns leaving `wordarray` non-NULL and `wordsize` NULL. The re-allocation guard checks `wordarray`, so it's now permanently satisfied — the *next* call skips allocation and dereferences the still-NULL `wordsize`.
**Why it matters:** A guaranteed crash on the next word-split after a transient OOM lands between the two mallocs. Low likelihood, but real and cleanly fixable.
**How it's triggered:** OOM at exactly the second `malloc()` in the one-time init block.
**Proposed fix:** Undo the first allocation and reset `wordarray` to NULL so the guard stays honest (`ckclib.c:2879-2890`):

```c
   if (!wordarray) { /* Allocate result array (only once) */
     if (!(wordarray = (char **)malloc((MAXWORDS + 1) * sizeof(char *)))) {
       return;
     }
     if (!(wordsize = (int *)malloc((MAXWORDS + 1) * sizeof(int)))) {
-      return;
+      /* [V-28]: undo the first allocation and reset wordarray to NULL so
+         the "if (!wordarray)" guard still holds next call.  Previously
+         wordarray stayed non-NULL while wordsize stayed NULL, so the next
+         setword() bypassed the guard and dereferenced NULL below. */
+      free(wordarray);
+      wordarray = NULL;
+      return;
     }
```
**Fix effort & risk:** Trivial, three lines. No success-path change; converts a guaranteed-crash-on-next-call into a clean retry.

---

# Low / Informational (V-29 … V-43)

> Reachability verdicts: **real** (reachable in a normal `make linux` build), **latent** (currently safe by an invariant that isn't locally enforced), **dead-code** (gated on a macro/mode never active in this fork). Three findings — **V-32, V-34, V-43** — are outright dead code today and can be deferred or batch-hardened.

### V-29. `decode()`/`bdecode()` truncated-escape over-read — real, low-impact
**In plain terms:** Unpacking a truncated packet mid-escape can read 1–2 bytes past the field length `rln`, but those bytes are still inside the allocated receive buffer (trailing checksum/terminator) — a data-integrity quirk, not a confirmed memory-safety bug.
**Recommendation:** Add a remaining-length check before consuming trailing escape bytes — in `decode()` before the repeat/`ctlq` operand reads (`if (len < 2) break;` / `if (len < 1) break;`), mirrored in `bdecode()`. Batchable with V-2.
**Fix effort & risk:** Small, but a hot path (every packet) — smoke-test a file transfer afterward.

### V-30. Active-mode FTP data-connection hijack race — real, hard-to-trigger
**In plain terms:** `dataconn()` (`ckcftp.c:11490`) `accept()`s the active-mode data connection without checking the peer matches the control connection's peer (`hisctladdr`) — an inherited BSD-ftp weakness needing network-race positioning.
**Recommendation:** Compare the accepted peer against `hisctladdr.sin_addr`, close+reject on mismatch:
```c
s = accept(data, (struct sockaddr *)&hisdataaddr, &fromlen);
if (s >= 0 &&
    hisdataaddr.sin_addr.s_addr != hisctladdr.sin_addr.s_addr) {
    close(s);
    s = -1;   /* reject: data conn from unexpected peer */
}
```
**Fix effort & risk:** Small, but a behavior change that could break legitimate NAT/multi-homed servers — test against a real server first, or accept/document the limitation given Low severity.

### V-31. Dead overflow-detection code in MLSD/NLST line reading — cosmetic
**In plain terms:** `if (buf[FTPNAMBUFLEN - 1])` (`ckcftp.c:12898`) can never fire — `fgets()` always NUL-terminates at that index. Not unsafe; just a broken diagnostic.
**Recommendation:** Replace with a real truncation check (e.g. test for a missing trailing `'\n'` before EOF). Low priority.
**Fix effort & risk:** Trivial; only affects whether the (rare) overlong-line warning ever prints.

### V-32. Asymmetric `CK_SNDLOC` guards in `tn_snenv()` — **dead code**
**In plain terms:** Pass 1's LOCATION accounting is `#ifdef CK_SNDLOC` (`ckctel.c:3816`), pass 2's equivalent write (`4018`) is unconditional. Dead today: `CK_SNDLOC` is only undefined under `-DNOTCPIP`, which also excludes essentially all of `ckctel.c`.
**Recommendation:** No action needed for correctness. For robustness against future refactors, wrap the pass-2 block in `#ifdef CK_SNDLOC` to match its sibling. Batch with V-43.
**Fix effort & risk:** Trivial, zero runtime effect in any buildable config.

### V-33. `TELOPT_NAWS` server-side handler reads stale buffer — real only in dead server mode
**In plain terms:** A short/malformed NAWS subnegotiation from a *client* makes `ckctel.c:3457-3481` read width/height without checking 4 payload bytes are present, possibly reusing leftover bytes — stays within the buffer. Only reachable in `sstelnet` server mode (off by default; essentially no live entry point in this fork) against a malicious client.
**Recommendation:** Bound each `sb[i++]` read by the actual subnegotiation length, defaulting to 80×24 on a short payload. Low priority.
**Fix effort & risk:** Small; no macOS/BSD-chain interaction.

### V-34. `ckxfprintf()`/`ckxprintf()` IKSD-only `vsprintf` branch — **dead code**
**In plain terms:** The risky `vsprintf()`-into-fixed-4096-buffer-then-check branch (`ckutio.c:11784`, `11861`) only runs when global `inserver` is true, and `inserver` is only set inside `#ifdef IKSD` blocks; `IKSD` is never defined by any target in this fork, so every `printf`/`fprintf` always takes the safe `vprintf`/`vfprintf` path.
**Recommendation:** No action for this fork's builds. If IKSD is ever reintroduced, switch these to `vsnprintf` with a proper post-check first.
**Fix effort & risk:** Defer; dead code, consistent with the documented IKSD removal.

### V-35. `DIAL TIMEOUT` falls through into `DIAL ESCAPE-CHARACTER` — real, correctness-only
**In plain terms:** `case XYDTMO:` (`ckuus3.c:5501-5514`) has no `break`/`return` before `case XYDESC:`, so `DIAL TIMEOUT` also silently re-prompts for the escape character. No memory-safety/security impact.
**Recommendation:**
```c
    dialtmo = x;
    mdmwaitd = z;
+   return (success = 1);
  case XYDESC: /* DIAL ESCAPE-CHARACTER */
```
matching the `return`-per-case style of sibling cases.
**Fix effort & risk:** Trivial; removes an unintended extra prompt.

### V-36. `p[-1]` OOB read when a `TMPDIR`-family env var is set but empty — real, benign
**In plain terms:** `ckuus4.c:13766-13786` does `if (p[len-1] != '/')`; if `TMPDIR`/`TMP`/`TEMP`/`CK_TMP` is exported but empty, `len==0` and it reads `p[-1]` (UB, practically an adjacent environment byte).
**Recommendation:**
```c
      if (p) {
        int len = strlen(p);
-       if (p[len - 1] != '/') {
+       if (len > 0 && p[len - 1] != '/') {
          ckstrncpy(vvbuf, p, VVBUFL);
          ...
+       } else if (len == 0) {
+         p = NULL; /* empty value ~ not set */
        }
      }
```
**Fix effort & risk:** Trivial; no change for the common non-empty case.

### V-37. `NULL` passed to `%s` for unset `LANG` — real, benign
**In plain terms:** `ckuus5.c:6784` prints `getenv("LANG")` with no NULL guard, unlike every other `LC_*` value in the same `SHOW` block (UB per the standard; glibc prints `"(null)"`).
**Recommendation:**
```c
-    printf("  LANG=\"%s\"\n", getenv("LANG"));
+    { char *lg = getenv("LANG"); printf("  LANG=\"%s\"\n", lg ? lg : ""); }
```
matching the `s = ...; if (!s) s = "";` idiom used just above.
**Fix effort & risk:** Trivial.

### V-38. `dgi_u()`/`hproman8_u()` missing lower-bound self-check — latent
**In plain terms:** Both index `map[(c&0x7f) - offset]` (offset=32) without first checking `(c&0x7f) < offset`, unlike ~50 sibling `*_u()` functions; masked values 0–31 compute a negative index. Latent: callers only reach these when the byte is already ≥32 by a cross-file invariant that isn't locally enforced.
**Recommendation:** Add the sibling self-guard:
```c
+ if ((c & 0x7f) < u_dgi.offset) {
+   return (c);
+ }
  return (u_dgi.map[(c & 0x7f) - u_dgi.offset]);
```
(mirror for `hproman8_u`).
**Fix effort & risk:** Small; purely defensive, no in-range behavior change.

### V-39. `ckfstoa()` OVERFLOW-path length argument from wrong base — latent, cosmetic
**In plain terms:** `ckstrncpy(&buf[23], "OVERFLOW", 32)` (`ckclib.c:590`) claims 32 bytes starting at offset 23 in a 32-byte `buf` (only 9 remain); harmless only because `"OVERFLOW"`+NUL is exactly 9 bytes. Maintenance trap.
**Recommendation:**
```c
-      ckstrncpy(&buf[23], "OVERFLOW", 32);
+      ckstrncpy(&buf[23], "OVERFLOW", sizeof(buf) - 23);
```
**Fix effort & risk:** Trivial; self-documenting, zero behavior change.

### V-40. `b64tob8()` streaming bound ignores carried-over bits — latent, unreachable today
**In plain terms:** The output-size check (`ckclib.c:2475-2524`) is computed from the current call's input only, ignoring the static `bits`/`r` state from a prior streaming call. The one real streaming call site (`ckuus6.c:8801`) has 120 bytes of slack, so it's not reachable.
**Recommendation:** Add an in-loop belt-and-suspenders bound:
```c
    if (bits >= 8) {
      bits -= 8;
      c = (unsigned)((r >> bits) & 0xff);
+     if (k >= len) return (-1); /* destination exhausted mid-stream */
      out[k++] = c;
    }
```
**Fix effort & risk:** Small; only changes the (currently unreached) overflow case into the documented `-1` error return.

### V-41. `ckmatch()` unbounded recursion on nested `{...}` groups — real, local-only
**In plain terms:** `ckmatch()` (`ckclib.c` ~1356-1950) recurses once per nested brace level with only a depth *counter*, never a cap — a deeply nested pattern can exhaust the stack. Patterns are locally supplied (filespecs, `SET`/`DEFINE`), not remote — a local self-DoS.
**Recommendation:** Cap depth using the existing counter, mirroring the `CMDDEP` guard:
```c
  matchdepth++; /* Now increment call depth */
+ if (matchdepth > 200) {
+   matchdepth--;
+   return (0); /* Fail rather than exhaust the stack */
+ }
```
**Fix effort & risk:** Small; confirm no realistic pattern nests near 200 levels before adopting.

### V-42. `alarm()` armed before the new `SIGALRM` handler is installed — real, robustness nit
**In plain terms:** In `alrm_execute()` (`ckusig.c:80-81`) and `cc_alrm_execute()` (`104-105`), `alarm(timo)` runs one line *before* `ck_signal(SIGALRM, handler)` installs the handler — a narrow race where a very-short/already-pending alarm fires the previous handler.
**Recommendation:** Swap the order in both:
```c
- savalrm = alarm(timo);
- savhandler = ck_signal(SIGALRM, handler);
+ savhandler = ck_signal(SIGALRM, handler);
+ savalrm = alarm(timo);
```
**Fix effort & risk:** Trivial, behavior-preserving in the intended case. Touches signal code — re-run the `signal-handling` skill's PAUSE 2 / SIGINT-mid-PAUSE battery afterward.

### V-43. `-N` NetBIOS adapter range-check tautology — **dead code**
**In plain terms:** `ckuusy.c:3896`'s `(atoi(*xargv) < 0) && (atoi(*xargv) > 9)` can never be true, so the range check never fires — but the whole `case 'N':` is `#ifdef CK_NETBIOS`, which is unconditionally undefined in this Linux/macOS/BSD-only fork. Confirmed unreachable.
**Recommendation:** No action needed. If ever resurrected, the fix is one operator: `&&` → `||`.
**Fix effort & risk:** Trivial, currently inert. Batch with V-32.

---

## Recommended remediation roadmap

**Phase 1 — Critical, remote/pre-auth memory corruption & file write (do first):**
1. **V-3** `xunchar()` clamp in `ckcker.h` — a single central change that hardens the whole packet-decode path (and is the root cause behind V-1/V-2's worst amplification). Verify with a byte-identical rebuild given its broad inclusion.
2. **V-1** (missing `return` in `srv_login()`) and **V-5 + V-18 together** (`tn_snenv()` write-pass guard *and* `varname[17]` sizing) — the two pre-auth stack overflows.
3. **V-2** (`putsrv()` bound), **V-4** (`RPBUFPUT` macro), **V-7** (PASV parser bound), **V-8** (`fnparse()` bound) — the remaining remote overflows; all small, self-contained bound checks.
4. **V-6** (path-traversal `zhasdotdot()` in `nzrtol()`) — needs real transfer testing since it changes default receive behavior.

**Phase 2 — High, confirmed/near-confirmed memory corruption:**
- **V-9** (`ecbuf`, both `ckucns.c`/`ckucon.c`), **V-10** (`spdtab` `sizeof` + `speeds[]` allocation, together), **V-13/V-14** (shared `line[]` loop-bound fix), **V-15** (`bufcpy()` helper at three sites). All are straightforward, well-understood, and two are ASan-confirmed.

**Phase 3 — High, conditional; document the risk meanwhile:**
- **V-11** (split `usepipes` into send/receive opt-ins) and **V-12** (`has_dotdot()` guard on FTP recursive names).

**Phase 4 — Medium, opportunistic (many are one-liners):**
- Trivial: **V-21** (fd close), **V-23** (`this` vs `table[-1]`), **V-24** (`sizeof(filbuf)`), **V-25** (`x1+1 <= k`), **V-26** (`& 0x80` guard), **V-28** (reset `wordarray`).
- Small/medium: **V-16** (PASV SSRF), **V-17** (`.netrc` bound), **V-18** (already folded into V-5), **V-19** (`O_EXCL`), **V-20**/**V-27** (`%s`-only expanders), **V-22** (`O_NOFOLLOW`).

**Phase 5 — Low/Informational:**
- Fix the trivial real ones opportunistically (**V-35, V-36, V-37, V-39, V-42**).
- Defensively harden the latent ones when touching that code (**V-29, V-38, V-40, V-41**).
- Defer or batch-comment the dead code (**V-32, V-34, V-43**).

**Verification for every change** (per the `build-and-verify` skill): `make linux` warning-clean; `clang-format FILE | diff FILE -` empty; for `ckcpro.w` edits run `make wart && make ckcpro.c` first; runtime smoke tests for the file-I/O (V-6, V-21, V-22) and network (V-7, V-16) fixes and the signal battery for V-42; a SOURCE_DATE_EPOCH byte-identical rebuild is *not* expected here since these are intentional behavior changes, so rely on per-object review and targeted runtime tests instead.
