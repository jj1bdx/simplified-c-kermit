# C-Kermit 10.0 Beta.12 (simplified fork) — closing the `lpcapu == 0` short-packet LEN overflow (residual gap of BUGFIX_20260709_2)

**Subject:** review and fix of the residual gap recorded in `BUGFIX_20260709_2.md` §4.1: when
long packets are *not* negotiated (`lpcapu == 0`), `spack()` has no extended-length format to
escape to, and several producers can still hand it more data than the short-packet LEN field can
legally encode — producing the same silent-retransmission hang that document fixed for the
`lpcapu > 0` case.

- **Branch:** `main` (HEAD = `c77999d` + this fix, uncommitted)
- **Date:** 2026-07-12
- **Analysis:** c-expert subagent (two rounds; the second corrected three factual errors in the
  first — see §6)
- **Companion docs:** `BUGFIX_20260709_2.md` (the spack() threshold fix and the §4.1 note this
  closes), `BUGFIX_20260709.md`, `BUGFIX_20260708.md`.

---

## TL;DR

1. The §4.1 note was **right that a gap exists but understated it three ways**: (a) the flat
   `spsiz = 90` clamp in `sdata()` is itself off by one for block check 3 — `len == 90` is still
   reachable and yields LEN = 95 = DEL, the exact failure the original repro hit; (b) the gap is
   not confined to the file-data path — every `encstr()`-based packet (F/X filenames, GET
   requests, REMOTE commands, E-packet reasons) is bounded by **our own** `rpsiz` (~4094 under
   `NEWDEFAULTS`), not by anything the peer negotiated, so a single ~90-byte pathname or command
   is enough; and (c) no "old peer negotiating CRC-16" is needed — block check 3 is this fork's
   *default* negotiation (`DFBCT = 3`, switched in by `ckcpro.w`'s `bctu = bctr`), so any non-LP
   peer plus default settings is exposed.
2. Both classes were **reproduced live** against stock binaries using a stock-Kermit way to
   simulate a limited peer: `SET SEND NEGOTIATION-STRING-MAX-LENGTH 8` on the receiver truncates
   its ACK after the block-check/repeat fields, so the sender sees no CAPAS byte (`lpcapu = 0`)
   while still negotiating block check 3. A 90-byte file transfer and a 120-char filename each
   hang the baseline until the retry cap; a 110-char `REMOTE HOST` command likewise.
3. **Fix applied** (7 hunks, 3 files): a block-check-aware clamp in `sdata()`, the same bound in
   `encstr()` when `lpcapu == 0`, a clean-refusal check of `encstr()`'s result in `sfile()`, a
   last-resort guard in `spack()` that refuses (returns -1) instead of emitting an unanswerable
   frame, a defensive `spsiz` cap in `spar()` against nonconforming MAXL bytes, and two
   `ckcpro.w` call-site fixes (`if (x)` → `if (x > 0)`) without which the spack() guard's -1
   would be misread as success. Not committed, per instruction.
4. **Verified:** clean warning-free `make linux` (with `wart`/`ckcpro.c` regeneration),
   clang-format-stable, and a runtime battery (§5): every baseline hang now either completes
   (packet splitting at the legal boundary) or fails fast and visibly; a 200 KB transfer over
   normal long-packet negotiation is unaffected.

## 1. The constraint, restated

A short Kermit packet's LEN field is one printable character, `tochar(len + bctl + 2)`, legal
only for values 0..94 (`tochar(94)` = `~` = 126, top of the printable range). The receiving
framer `ttinl()` (`ckutio.c:8028-8046`) rejects LEN bytes that decode above 94 **without
replying**: 95 (DEL) and 96 (0x80) are cleanly discarded, and because the arithmetic wraps mod
256 and the framer masks with 0x7f, values further past 94 can decode to a *wrong but in-range*
length — the receiver then consumes the wrong number of bytes, a misparse rather than a clean
discard. Either way the sender retransmits identical bytes until "Too many retries" (~4½ minutes
by default); to the user the session is hung.

`BUGFIX_20260709_2.md` fixed the `lpcapu > 0` case by escaping to extended-length format at
`len + bctl + 2 > 94` (`ckcfn2.c` `spack()`). With `lpcapu == 0` there is no escape: **every
producer must keep `len ≤ 92 - bctl`**, and before this fix none of them reliably did.

## 2. Reachability (c-expert analysis, corrected round)

### 2.1 Who can be talking to us with `lpcapu == 0`

Any peer that does not advertise the long-packet CAPAS bit (old or minimal Kermit
implementations), or a session where `SET RECEIVE PACKET-LENGTH` demotes it
(`ckcfns.c` `spar()`, the `rpsizf` path zeroes `lpcapu` with `spsiz = urpsiz - 1`, which can
itself be 95). Between two instances of this fork long packets are always negotiated, which is
why the gap never bites in fork-to-fork testing.

### 2.2 Block-check length in the data phase

`bctr` defaults to `DFBCT = 3` (`ckcmai.c:659`, `ckcker.h`), and the negotiated switch is in the
protocol grammar, not ckcfns.c: `ckcpro.w` `<ssinit>Y` (sender) and the receive-side equivalent
run `bctu = bctr; bctl = (bctu == 4) ? 2 : bctu;` right after `spar()`/`rinit()`. So data-phase
`bctl` ∈ {1, 2, 3} through perfectly normal negotiation — **block check 3 is the default**, and
the §4.1 framing ("reachable only against an old/limited peer negotiating CRC-16") was too
narrow: our own `bctl` is what matters, and even block check 1 has its own window at `len > 91`.
(Command packets sent before the S-exchange — GET, REMOTE, I — always use type 1, so the bound
`92 - bctl` is evaluated with whatever `bctl` `spack()` itself will use; there is no staleness
hazard.)

### 2.3 The producers that could exceed `92 - bctl` (ranked by likelihood)

1. **F/X-packet filenames** — `sfile()` → `encstr()` (`ckcfns.c`). `encstr()` bounds its
   `getpkt()` call by `rpsiz` — **our own receive size**, ~4094 under `NEWDEFAULTS` (which is
   active: `LINUX` → `BIGBUFOK` → `NEWDEFAULTS`, so `DRPSIZ = 4095`, and `rpar()` sets
   `rpsiz = urpsiz - 1` ≈ 4094). Nothing peer-related bounds it. Any transmitted pathname whose
   encoded form exceeds `92 - bctl` (89 with the default block check 3) makes the very first
   packet of the transfer unanswerable. `sfile()` also ignored `encstr()`'s
   string-didn't-fit return code entirely.
2. **GET-request filenames** (`srinit()`) and **REMOTE command strings** (`scmd()`) — same
   `encstr()` exposure. Both, unlike `sfile()`, already refuse cleanly ("String too long") when
   `encstr()` returns -1 — the fix only had to lower the bound `encstr()` enforces.
3. **E-packet reason strings** (`errpkt()`) — same exposure via a long reason.
4. **File data** — `sdata()`'s flat clamp `if (spsiz <= 94 && spsiz > 90) spsiz = 90;`
   (fdc, 14 Sep 2022) admits `len == 90`, and with `bctl == 3` that is LEN = 95 = DEL. This is
   the §4.1 case — but note the *existing clamp itself* was off by one; the danger did not start
   at `spsiz` 91 as the note implied.
5. **`sattr()` attributes** — bounded by explicit first-fit checks against `max = rpsiz`, so
   only exposed with ~90 bytes of attribute data; and **memory-string sources generally**:
   `getpkt()`'s memstr branch returns an overfull packet *without* the leftover-truncation that
   file sources get (`ckcfns.c`, the early `if (memstr) return (size);` inside the overflow
   handler), so `encstr()` output and server-response text (`sndstring()` feeding `sdata()`,
   e.g. REMOTE SPACE/DIRECTORY output) can exceed the requested bound by the width of one
   prefixed/repeat-doubled sequence. This is why a purely input-side clamp cannot be a complete
   fix and a last-resort guard in `spack()` is also needed.

Additionally, `spar()`'s non-LP path takes `spsiz = xunchar(s[1])` from the peer's MAXL byte
with no upper clamp — a corrupt or nonconforming byte can push `spsiz` past 94 (up to 255),
sailing past `sdata()`'s `spsiz <= 94` guard entirely.

## 3. The fix (7 hunks, 3 files, uncommitted)

**`ckcfns.c` — `sdata()`: block-check-aware clamp** (replaces the flat 90):

```c
if (spsiz <= 94 && spsiz > 92 - bctl) {
  spsiz = 92 - bctl;
}
```

`LEN = len + bctl + 2 ≤ 94 ⇔ len ≤ 92 - bctl`. Kept unconditional (not gated on `lpcapu`),
matching the old clamp's scope: a peer can negotiate the LP capability bit while declaring only
a short MAXL and no MAXLX fields, in which case short-range `spsiz` with `lpcapu == 1` is
legitimate and this keeps such packets inside the peer's declared buffer instead of relying on
the extended-format escape (fdc's 13 Sep 2022 border-case comment in `spack()`). For
`bctl == 1` the new clamp is one byte *less* conservative than the old one (91 vs 90), verified
on the wire (§5).

**`ckcfns.c` — `encstr()`: same bound when long packets are unavailable:**

```c
bufmax = rpsiz;
if (!lpcapu && bufmax > 92 - bctl) {
  bufmax = 92 - bctl;
}
rc = getpkt(bufmax, 0);
```

This makes `encstr()`'s *existing* didn't-fit detection (rc = -1) fire at the boundary that
actually matters, and `scmd()`/`srinit()` already turn that into a clean "String too long"
refusal instead of sending anything.

**`ckcfns.c` — `sfile()`: honor `encstr()`'s return code:**

```c
if (encstr((CHAR *)s) < 0) {             // Encode the name.
  debug(F110, "sfile name too long for packet", s, 0);
  return (0);
}
```

`sfile()` was the one `encstr()` caller that ignored the -1; without this it would silently
send a *truncated* filename (the receiver stores the file under the wrong name). Returning 0
uses `sfile()`'s existing failure contract — the caller sends an E packet and the SEND fails
visibly. This also completes the intent of the upstream 18 Oct 2021 change that made
`encstr()` report truncation in the first place.

**`ckcfn2.c` — `spack()`: last-resort guard**, placed after the long-packet decision, before any
packet bytes or `s_pkt[]` bookkeeping are written:

```c
if (!longpkt && (len + bctl + 2) > 94) {
  debug(F101, "SPACK refusing unencodable short packet len", "", len);
  return (-1);
}
```

This is the only choke point that covers *every* producer, including the `getpkt()` memstr
overrun (§2.3 item 5) and anything added in the future. Truncating here instead was rejected:
the data is already encoded and counted; a partial write silently corrupts the file or name.

**`ckcfns.c` — `spar()`: defensive cap against nonconforming MAXL**, placed after the
long-packet negotiation block so it also catches the `rpsizf` demotion path (`spsiz` up to 95
with `lpcapu` freshly zeroed):

```c
if (!lpcapu && spsiz > 94) {
  spsiz = 94;
}
```

**`ckcpro.w` — two call-site truthiness fixes** (regenerated `ckcpro.c` via
`make wart && make ckcpro.c`):

- `<ssinit>Y`: `if (x)` → `if (x > 0)` after `x = sfile(xflg);`
- `<sseof>Y` batch loop: `if (sfile(xflg))` → `if (sfile(xflg) > 0)`

Without these, the spack() guard's -1 would be read as "packet sent OK" (negative is truthy),
the state machine would advance and wait for an ACK that never comes — and because the guard
fires before `s_pkt[]` is filled in, `resend()` hits its "No packet to resend" branch and
silently no-ops each retry: the guard would have *reproduced* the multi-minute hang it exists
to prevent. c-expert traced every other consumer of a possibly-negative send wrapper
(`scmd`, `srinit`, `sopkt`, `sattr`, `sdata`, `seof` sites in `ckcpro.w`); all already test
`< 0` explicitly or discard the value harmlessly. These two were the only unsafe sites.

### Considered and rejected

- **Resurrecting `maxdata()`** (`ckcfns.c`, zero callers — the §4.1 note pointed at it): its
  formula treats `spsiz` as the full wire length, but `getpkt()`'s `bufmax` parameter de facto
  bounds the *data field*. Wiring it into `sdata()` would shrink every packet of every transfer
  by ~5-8 bytes — an across-the-board throughput regression to fix a boundary case. The
  `92 - bctl` clamp encodes the same block-check awareness under the semantics the code
  actually has.
- **Truncation in `spack()`** — silent data corruption; see above.

## 4. Reproduction harness

`ckcfns.c` `rpar()` emits its negotiation string field-guarded (`if (max < K)` before
`dada[K]`), so **`SET SEND NEGOTIATION-STRING-MAX-LENGTH 8`** on the receiver produces an ACK
with MAXL..REPT (9 fields) but **no CAPAS byte**: the sender gets `lpcapu = 0` while block
check 3 still negotiates normally. (9 does *not* work — the guard before the CAPAS byte at
`dada[9]` is `max < 9`, so 9 still includes it; verified in the packet log.) This simulates an
old/limited peer with two stock binaries — no special build needed.

Harness: socat PTY pair, server side `set send negotiation-string-max-length 8` + `server`,
client pushes `head -c N` of an incompressible alphanumeric cycle (repeated characters would be
RLE-compressed and miss the target size). Scripts preserved in the session scratchpad
(`lpgap/run_one.sh`, `run_longname.sh`, `run_remote.sh`).

## 5. Verification

Build: `make wart && make ckcpro.c && make clean && make linux` — exit 0, zero warnings.
`clang-format` stability: `ckcfns.c` and `ckcfn2.c` clean.

Runtime, baseline (`c77999d`) vs fixed, all over the §4 non-LP + block-check-3 negotiation
unless noted:

| Scenario | Baseline | Fixed |
|---|---|---|
| 50-byte file (control) | pass | pass |
| 90-byte file (D packet, len 90 → LEN 95) | **hang** — LEN byte 0x7F on the wire, receiver silent, identical retransmissions every 15 s, killed at 45 s | **pass** — split 89 + 1, LEN `~` (94) |
| File-size sweep N = 85..95, 180 | (90 hangs) | all pass, contents byte-identical |
| 89-char filename (F packet at the exact bound) | — | pass |
| 120-char filename (F packet, len 125 → LEN 0x9D, masks to "garbage") | **hang** — receiver NAKs, sender can only resend the same illegal frame, killed at 45 s | **clean failure in 3.5 s** — "SEND-class command failed", exit 2 |
| `REMOTE HOST echo <110 chars>` (C packet) | **hang** — 48 s wall, killed by timeout | **clean refusal ~3 s** — client sends `E "String too long"`, exit 1; subsequent `finish` works |
| `set block-check 1`, N = 91 / 92 | — | pass; 91 goes as a *single* packet with LEN `~` (94), proving the `92 - bctl` arithmetic for bctl = 1 |
| Normal LP negotiation, 200 KB file (regression) | — | pass |

No previously-working case regresses: every input the new clamps/guard reject produced an
illegal, unanswerable frame on the baseline (hang), and every legal boundary case (len exactly
`92 - bctl`) still goes out as a single maximal short packet.

## 6. Analysis-process notes

The first c-expert round contained three errors worth recording because both were
grep-methodology traps: (a) a `--include="*.c" --include="*.h"` grep missed the `bctu = bctr`
switch in `ckcpro.w`, producing a false "block check 3 is never actually used" conclusion —
**`.w` files must be included when tracing protocol-phase state**; (b) a literal grep for
`NEWDEFAULTS` missed that it is derived transitively (`LINUX` → `BIGBUFOK` → `NEWDEFAULTS` in
`ckcdeb.h`/`ckcker.h`), inverting the `rpsiz` default (90 vs 4095); (c) the first-round Option C
patch, applied as-is, would have silently degraded into the same hang via the
`sfile()`-truthiness/`resend()` no-op chain — caught only by tracing every consumer of the new
-1. All three were corrected in the second round before anything was applied.

## 7. Out of scope / follow-ups

1. **Hostile CHKT `'5'` in an ACK:** `spar()` accepts block-check field values 1..5, and the
   `bctf` forcing convention is only recognized on incoming I/S packets (`rpack()`), not Y — so
   a peer ACKing CHKT `'5'` yields `bctu = bctl = 5` via `ckcpro.w`, and `spack()`'s checksum
   switch has no case 5 or default: it would append **zero** check bytes while LEN claims 5.
   Distinct failure signature, hostile/buggy-peer-only; deserves its own small fix
   (e.g. clamping `bctu` to 1..4 at the `ckcpro.w` switch).
2. **`getpkt()` memstr overrun:** the memstr early-return in the overflow handler can exceed
   `bufmax` by one prefixed/repeat-doubled sequence. The `spack()` guard makes this safe
   (refusal, not hang), but a proper fix needs a way to un-encode a variable-length sequence
   for memory-string sources — non-trivial, low value while the guard stands.
3. **Batch `SEND *` skips over-long-named files silently:** the `<sseof>Y` loop treats
   `sfile() ≤ 0` as "keep trying the next file", so against a non-LP peer a batch send now
   skips such files without a per-file warning (previously: hung on the first one). A
   user-visible warning in that loop would be a UX improvement.
4. **Receive-side silence** (§4.2 of `BUGFIX_20260709_2.md`) remains as documented there:
   `ttinl()`'s garbage-LEN early returns still skip `ttimoff()`, and a corrupt-LEN packet still
   draws no NAK when input is pending. Unchanged; the sender-side fixes above remove every
   known way for *this* fork to be the offender.
