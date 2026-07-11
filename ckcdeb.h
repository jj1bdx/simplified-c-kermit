//  C K C D E B . H

// For recent additions search below for "2021" and "2022" and "2023".
// Most recent updates: Sat Jul  1 10:27:16 2023 (David Goodwin, fdc)
// More recent: Sun Feb  4 20:17:33 2024 (removed prototypes for malloc())
//
// NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
// compatible with C preprocessors that support only #ifdef, #else, #endif,
// #define, and #undef.  Please do not use #if, logical operators, or other
// later-model preprocessor features in any of the portable C-Kermit modules.
// You can, of course, use these constructions in platform-specific modules
// when you know they are supported.

// This file is included by all C-Kermit modules, including the modules
// that aren't specific to Kermit (like the command parser and the ck?tio and
// ck?fio modules).  It should be included BEFORE any other C-Kermit header
// files.  It specifies format codes for debug(), tlog(), and similar
// functions, and includes any necessary definitions to be used by all C-Kermit
// modules, and also includes some feature selection compile-time switches, and
// also system- or compiler-dependent definitions, plus #includes and prototypes
// required by all C-Kermit modules.

// Author: Frank da Cruz <fdc@columbia.edu>,
//  Columbia University Academic Information Systems, NYC (1974-2011)
//  The Kermit Project, Bronx NY (2011-present)
//  Changes from David Goodwin for Windows and OS/2 (2022)
//
// Copyright (C) 1985, 2023,
//  Trustees of Columbia University in the City of New York.
//  All rights reserved.  See the C-Kermit COPYING.TXT file or the
//  copyright text in the ckcmai.c module for disclaimer and permissions.

// Etymology: The name of this file means "C-Kermit Common-C-Language Debugging
// Header", because originally it contained only the formats (F000-F111) for
// the debug() and tlog() functions.  Since then it has grown to include all
// material required by all or most of the other C-Kermit modules, including
// the non-Kermit specific ones.  In other words, this is the one header file
// that is guaranteed to be included by all C-Kermit source modules.
#ifndef CKCDEB_H // Don't include me more than once.
#define CKCDEB_H

// Some ancient MIPS compilers for Windows NT define "MIPS" which causes
// problems here and elsewhere. None of the windows headers depend on MIPS being
// defined (they all check for _MIPS_), so it's safe to just undefine it.

// Moved here from ckcmai.c October 2022... REMOVE THIS AFTER BETA TEST!
#ifndef BETATEST
#define BETATEST
#endif // BETATEST

// Now that WTMP and Syslog are "deprecated" don't include them by default

#ifndef DOWTMP // Unless explicitly requested
#ifndef NOWTMP // No more WTMP logging
#define NOWTMP
#endif           // NOWTMP
#endif           // DOWTMP
#ifndef DOSYSLOG // Unless explicitly requested
#ifndef NOSYSLOG // No more syslog
#define NOSYSLOG
#endif // NOSYSLOG
#endif // DOSYSLOG

// FTP client removed 2026-07-09: the built-in FTP client (ckcftp.c) has been
// deleted from this tree, so NOFTP is now hard-wired on.  There is no DOFTP
// escape hatch -- re-enabling FTP would mean restoring ckcftp.c and its command
// tables.  The old NOFTP/NEWFTP/SYSFTP arbitration and the scattered
// feature-implied "#define NOFTP" blocks have been removed accordingly.  See
// doc/NOFTP-20260709.md.
#ifndef NOFTP
#define NOFTP
#endif // NOFTP

// 14 Sep 2022 - TYPE command's new /INTERPRET switch enabled by default
// except in Windows where it doesn't work because of character-set issues.

#ifdef NOICP // 2 Nov 2022
#ifndef NOIKSD
#define NOIKSD
#endif // NOIKSD
#endif // NOICP

#ifdef NOSPL   // 30 Oct 2022
#ifndef NOIKSD // 30 Oct 2022
#define NOIKSD
#endif          // NOIKSD
#ifndef NOLEARN // 30 Oct 2022
#define NOLEARN
#endif // NOLEARN
#else  // 12 December 2022

#ifndef NOTYPEINTERPRET // 23 August 2022 - TYPE /INTERPRET
#ifndef TYPEINTERPRET
#define TYPEINTERPRET
#endif // TYPEINTERPRET
#endif // NOTYPEINTERPRET

#ifndef NOCOPYINTERPRET // 20 Sep 2022 - COPY /INTERPRET
#ifndef COPYINTERPRET
#define COPYINTERPRET
#endif // COPYINTERPRET
#endif // NOCOPYINTERPRET

#endif // NOSPL
// RLOGIN client removed 2026-07-10: the old NODEPRECATED flag (which defined
// NOTELNET and NORLOGIN -- features "deprecated" in 2022, -fdc 12 May 2022)
// is hard-wired on, so NORLOGIN is permanently defined and (via the former
// RLOGCODE arbitration in ckcnet.h) RLOGCODE is permanently undefined: the
// built-in Rlogin client is gone and the RLOGIN command always reports "not
// configured".  There is no re-enable escape hatch.  NOTELNET is defined
// too, for parity with NODEPRECATED, but it never disabled the TELNET
// command or the Telnet protocol engine (ckctel.c), which remain fully
// functional; the old block's "#undef TNCODE" was a no-op because the
// TCPSOCKET-implies-TNCODE backstops further down re-define it.
// See doc/NODEPRECATED-20260709.md.
#ifndef NOTELNET
#define NOTELNET
#endif // NOTELNET
#ifndef NORLOGIN
#define NORLOGIN
#endif // NORLOGIN
// As of 26 September 2022, the Arrow-key feature is included only if
// explicitly requested because the API is disappearing not only in glibc
// but also other libcs like musl and whatever Android uses.
#ifndef DOARROWKEYS
#ifndef NOARROWKEYS // Arrow keys use a deprecated API
#define NOARROWKEYS // (at least in glibc)
#endif              // NOARROWKEYS
#endif              // DOARROWKEYS

// Unsigned numbers
// Defined unconditionally - it's 2026 already

#define USHORT unsigned short
#define UINT unsigned int
#define ULONG unsigned long

#ifdef MACOSX10 // Mac OS X 1.0
#ifndef MACOSX  // implies Mac OS X
#define MACOSX
#endif // MACOSX
#endif // MACOSX10

#ifdef MACOSX // Mac OS X
#ifndef BSD44 // implies 4.4 BSD
#define BSD44
#endif // BSD44
#endif // MACOSX

// The UNIX Seventh Edition (1979) compiler doesn't allow a lot of -D's on
// the make command line and big modules can result in nonsensical fatal
// compilation errors so I have to remove a lot of features to make it
// compile at all without putting more -D's on the make-command line.
//
// KFLAGS=-DV7MIN can also be used on other platforms, e.g. Linux, to build
// the smallest possible C-Kermit program, about 400kb: command-line only, no
// script programming, no making network connections, no character-set
// support, dialing of only the most common modem types, no use of external
// processes, and no logging or debugging.  On Ubuntu this results in an
// executable of about 402KB, compared to the normal one of 2.8MB.
//
// -fdc, 3 November 2022
#ifdef V7MIN  // UNIX V7 MINIMUM-SIZE BUILD
#ifndef NOICP // No interactive commands
#define NOICP
#endif        // NOICP
#ifndef NONET // No networking of any kind
#define NONET
#endif          // NONET
#ifndef NOCSETS // No character-set conversion
#define NOCSETS
#endif           // NOCSETS
#ifndef MINIDIAL // Minimum modem support
#define MINIDIAL
#endif        // MINIDIAL
#ifndef NOPTY // No spawning
#define NOPTY
#endif          // NOPTY
#ifndef NODEBUG // No debugging
#define NODEBUG
#endif // NODEBUG
#endif // V7MIN

#ifdef NOICP  // If no command parser
#ifndef NOSPL // Then no script language either
#define NOSPL
#endif          // NOSPL
#ifndef NOCSETS // Or characer sets
#define NOCSETS
#endif // NOCSETS
#endif // NOICP

// Built-in makefile entries

// Features that can be eliminated from a no-file-transfer version

#ifdef NOXFER
#ifndef NOCURSES // Fullscreen file-transfer display
#define NOCURSES
#endif            // NOCURSES
#ifndef NOCKSPEED // Ctrl-char unprefixing
#define NOCKSPEED
#endif           // NOCKSPEED
#ifndef NOSERVER // Server mode
#define NOSERVER
#endif             // NOSERVER
#ifndef NOCKTIMERS // Dynamic packet timers
#define NOCKTIMERS
#endif             // NOCKTIMERS
#ifndef NOPATTERNS // File-type patterns
#define NOPATTERNS
#endif              // NOPATTERNS
#ifndef NOSTREAMING // Streaming
#define NOSTREAMING
#endif         // NOSTREAMING
#ifndef NOIKSD // Internet Kermit Service
#define NOIKSD
#endif             // NOIKSD
#ifndef NOPIPESEND // Sending from pipes
#define NOPIPESEND
#endif           // NOPIPESEND
#ifndef NOAUTODL // Autodownload
#define NOAUTODL
#endif          // NOAUTODL
#ifndef NOMSEND // MSEND
#define NOMSEND
#endif         // NOMSEND
#ifndef NOTLOG // Transaction logging
#define NOTLOG
#endif             // NOTLOG
#ifndef NOCKXXCHAR // Packet character doubling
#define NOCKXXCHAR
#endif // NOCKXXCHAR
#endif // NOXFER

#ifdef NOICP   // No Interactive Command Parser
#ifndef NODIAL // Implies No DIAL command
#define NODIAL
#endif // NODIAL
#endif // NOICP

#ifndef NOIKSD
#ifdef IKSDONLY
#ifndef IKSD
#define IKSD
#endif // IKSD
#ifndef NOLOCAL
#define NOLOCAL
#endif // NOLOCAL
#ifndef NOPUSH
#define NOPUSH
#endif // NOPUSH
#ifndef TNCODE
#define TNCODE
#endif // TNCODE
#ifndef TCPSOCKET
#define TCPSOCKET
#endif // TCPSOCKET
#ifndef NETCONN
#define NETCONN
#endif // NETCONN
#ifdef IBMX25
#undef IBMX25
#endif // IBMX25
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif // CK_NETBIOS
#ifdef SUPERLAT
#undef SUPERLAT
#endif // SUPERLAT
#ifdef NPIPE
#undef NPIPE
#endif // NPIPE
#ifdef NETFILE
#undef NETFILE
#endif // NETFILE
#ifdef NETCMD
#undef NETCMD
#endif // NETCMD
#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#ifdef NETDLL
#undef NETDLL
#endif // NETDLL
#ifndef NOSSH
#undef NOSSH
#endif // NOSSH
#ifndef NOFORWARDX
#define NOFORWARDX
#endif // NOFORWARDX
#ifndef NOBROWSER
#define NOBROWSER
#endif // NOBROWSER
#ifndef NOHTTP
#define NOHTTP
#endif // NOHTTP
#ifndef NO_COMPORT
#define NO_COMPORT
#endif // NO_COMPORT
#endif // IKSDONLY
#endif // NOIKSD

// Features that can be eliminated from a remote-only version

#ifdef NOLOCAL
#ifndef NOHTTP
#define NOHTTP
#endif // NOHTTP
#ifndef NOSSH
#define NOSSH
#endif // NOSSH
#ifndef NOTERM
#define NOTERM
#endif           // NOTERM
#ifndef NOCURSES // Fullscreen file-transfer display
#define NOCURSES
#endif // NOCURSES
#ifndef NODIAL
#define NODIAL
#endif // NODIAL
#ifndef NOSCRIPT
#define NOSCRIPT
#endif // NOSCRIPT
#ifndef NOSETKEY
#define NOSETKEY
#endif // NOSETKEY
#ifndef NOKVERBS
#define NOKVERBS
#endif // NOKVERBS
#ifndef NOXMIT
#define NOXMIT
#endif // NOXMIT
#ifdef CK_CURSES
#undef CK_CURSES
#endif // CK_CURSES
#ifndef IKSDONLY
#ifndef NOAPC
#define NOAPC
#endif // NOAPC
#ifndef NONET
#define NONET
#endif // NONET
#endif // IKSDONLY
#endif // NOLOCAL

#ifdef NONET
#ifndef NOTCPIP
#define NOTCPIP
#endif // NOTCPIP
#ifndef NONETDIR
#define NONETDIR
#endif // NONETDIR
#ifndef NOIKSD
#define NOIKSD
#endif // NOIKSD
#ifdef TNCODE
#undef TNCODE
#endif // TNCODE
#ifdef NETCONN
#undef NETCONN
#endif // NETCONN
#ifdef TCPSOCKET
#undef TCPSOCKET
#endif // TCPSOCKET
#ifndef NOTCPOPTS
#define NOTCPOPTS
#endif // NOTCPOPTS
#ifdef IBMX25
#undef IBMX25
#endif // IBMX25
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif // CK_NETBIOS
#ifdef SUPERLAT
#undef SUPERLAT
#endif // SUPERLAT
#ifdef NPIPE
#undef NPIPE
#endif // NPIPE
#ifdef NETFILE
#undef NETFILE
#endif // NETFILE
#ifdef NETCMD
#undef NETCMD
#endif // NETCMD
// Commented out fdc May 2020 to allow external SSH command
// #ifdef NETPTY
// #undef NETPTY
// #endif NETPTY
#ifdef NETDLL
#undef NETDLL
#endif         // NETDLL
#ifndef NO_SSL // added May 2020 fdc
#define NO_SSL
#endif // NO_SSL
// Commented out fdc May 2020
// so we can use external ssh client in NONET builds
// #ifndef NOSSH
// #define NOSSH
// #endif
// NOSSH
#ifndef NOHTTP
#define NOHTTP
#endif // NOHTTP
#ifndef NOBROWSER
#define NOBROWSER
#endif // NOBROWSER
#ifndef NOFORWARDX
#define NOFORWARDX
#endif        // NOFORWARDX
#ifndef NOURL // 1 July 2023 for -DV7MIN, -DNOTCP, -DNONET, etc
#define NOURL
#endif // NOURL
#endif // NONET

#ifdef IKSDONLY
#ifdef IBMX25
#undef IBMX25
#endif // IBMX25
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif // CK_NETBIOS
#ifdef SUPERLAT
#undef SUPERLAT
#endif // SUPERLAT
#ifdef NPIPE
#undef NPIPE
#endif // NPIPE
#ifdef NETFILE
#undef NETFILE
#endif // NETFILE
#ifdef NETCMD
#undef NETCMD
#endif // NETCMD
#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#ifdef NETDLL
#undef NETDLL
#endif // NETDLL
#ifndef NOSSH
#define NOSSH
#endif // NOSSH
#ifndef NOHTTP
#define NOHTTP
#endif // NOHTTP
#ifndef NOBROWSER
#define NOBROWSER
#endif // NOBROWSER
#endif // IKSDONLY
// Note that none of the above precludes TNCODE, which can be defined in
// the absence of TCPSOCKET, etc, to enable server-side Telnet negotation.
#ifndef TNCODE   // This is for the benefit of
#ifdef TCPSOCKET // modules that might need TNCODE
#define TNCODE   // not all of ckcnet.h...
#endif           // TCPSOCKET
#endif           // TNCODE

#ifndef NETCONN
#ifdef TCPSOCKET
#define NETCONN
#endif // TCPSOCKET
#endif // NETCONN

#ifndef DEFPAR   // Default parity
#define DEFPAR 0 // Must be here because it is used
#endif           // DEFPAR
                 // by all classes of modules

// Kermit 95 can now be 64-bit so OS2ORWIN32 is a misnomer

// Moved here from ckcfnp.h 3 May 2023
// NEW PROTOTYPE FOR MAIN() ADDED 02 MAY 2023

#ifndef MAINNAME
#define MAINNAME main
#endif // MAINNAME

#ifdef MAINISVOID
// This is a leftover from original Macintosh
typedef void MAINTYPE;
#else
typedef int MAINTYPE;
// if any other types are needed add them here
#endif // MAINISVOID

#include <ctype.h> // and this.
#include <stdio.h> // Begin by including this.

// System-type compilation switches

// 4.4BSD is a mixture of System V R4, POSIX, and 4.3BSD.
#ifdef BSD44 // 4.4 BSD
#ifndef SVR4 // BSD44 implies SVR4
#define SVR4
#endif           // SVR4
#ifndef NOSETBUF // NOSETBUF is safe
#define NOSETBUF
#endif         // NOSETBUF
#ifndef DIRENT // Uses <dirent.h>
#define DIRENT
#endif // DIRENT
#endif // BSD44

#ifdef OPENBSD      // OpenBSD might or might not
#ifndef __OpenBSD__ // have this defined...
#define __OpenBSD__
#endif // __OpenBSD__
#endif // OPENBSD

#ifdef SVR3 // SVR3 implies ATTSV
#ifndef ATTSV
#define ATTSV
#endif // ATTSV
#endif // SVR3

#ifdef SVR4 // SVR4 implies ATTSV
#ifndef ATTSV
#define ATTSV
#endif       // ATTSV
#ifndef SVR3 // ...as well as SVR3
#define SVR3
#endif // SVR3
#endif // SVR4

#ifdef BSD4 // BSD4 implies ANYBSD
#ifndef ANYBSD
#define ANYBSD
#endif // ANYBSD
#endif // BSD4

#ifdef ATTSV // ATTSV implies UNIX
#ifndef UNIX
#define UNIX
#endif // UNIX
#endif // ATTSV

#ifdef ANYBSD // ANYBSD implies UNIX
#ifndef UNIX
#define UNIX
#endif // UNIX
#endif // ANYBSD

#ifdef POSIX // POSIX implies UNIX
#ifndef UNIX
#define UNIX
#endif         // UNIX
#ifndef DIRENT // and DIRENT, i.e. <dirent.h>
#ifndef SDIRENT
#define DIRENT
#endif          // SDIRENT
#endif          // DIRENT
#ifndef NOFILEH // POSIX doesn't use <sys/file.h>
#define NOFILEH
#endif // NOFILEH
#endif // POSIX

#ifdef V7
#ifndef UNIX
#define UNIX
extern int errno; // fdc 1 November 2022
#endif            // UNIX
#endif            // V7

// The symbol SVORPOSIX is defined for both AT&T and POSIX compilations
// to make it easier to select items that System V and POSIX have in common,
// but which BSD, V7, etc, do not have.
#ifdef ATTSV
#ifndef SVORPOSIX
#define SVORPOSIX
#endif // SVORPOSIX
#endif // ATTSV

#ifdef POSIX
#ifndef SVORPOSIX
#define SVORPOSIX
#endif // SVORPOSIX
#endif // POSIX

// The symbol SVR4ORPOSIX is defined for both AT&T System V R4 and POSIX
// compilations to make it easier to select items that System V R4 and POSIX
// have in common, but which BSD, V7, and System V R3 and earlier, etc, do
// not have.
#ifdef POSIX
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif // SVR4ORPOSIX
#endif // POSIX
#ifdef SVR4
#ifndef SVR4ORPOSIX
#define SVR4ORPOSIX
#endif // SVR4ORPOSIX
#endif // SVR4

// The symbol BSD44ORPOSIX is defined for both 4.4BSD and POSIX compilations
// to make it easier to select items that 4.4BSD and POSIX have in common,
// but which System V, BSD, V7, etc, do not have.
#ifdef BSD44
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif // BSD44ORPOSIX
#endif // BSD44

#ifdef POSIX
#ifndef BSD44ORPOSIX
#define BSD44ORPOSIX
#endif // BSD44ORPOSIX
#endif // POSIX

#ifdef UNIX // For items common to OS/2 and UNIX
#endif      // UNIX

#ifdef UNIX // For items common to Win32 and UNIX
#endif      // UNIX

#ifdef UNIX // For items common to VMS and UNIX
#define VMSORUNIX
#else
#endif // UNIX

#ifdef __DECC // For DEC Alpha VMS or OSF/1
#ifndef CK_ANSILIBS
#define CK_ANSILIBS // (Martin Zinser, Feb 1995)
#endif              // CK_ANSILIBS
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif // _POSIX_C_SOURCE
#endif // __DECC

#ifdef POSIX          // -DPOSIX on cc command line
#ifndef _POSIX_SOURCE // Implies _POSIX_SOURCE
#define _POSIX_SOURCE
#endif // _POSIX_SOURCE
#endif // POSIX

#ifdef NOLOGIN // NOLOGIN implies NOIKSD
#ifndef NOIKSD
#define NOIKSD
#endif // NOIKSD
#endif // NOLOGIN

#ifdef NOIKSD // Internet Kermit Service Daemon
#ifndef NOPRINTFSUBST
#define NOPRINTFSUBST
#endif // NOPRINTFSUBST
#ifndef NOLOGIN
#define NOLOGIN
#endif // NOLOGIN
#ifndef NOSYSLOG
#define NOSYSLOG
#endif         // NOSYSLOG
#ifndef NOWTMP // Redundant but does no harm
#define NOWTMP
#endif // NOWTMP
#else
#ifndef IKSD
#define IKSD
#endif // IKSD
#endif // NOIKSD

#ifdef IKSD      // IKSD options...
#ifndef IKSDCONF // IKSD configuration file
#ifdef UNIX
#define IKSDCONF "/etc/iksd.conf"
#else
#endif // UNIX
#endif // IKSDCONF
#ifndef NOIKSDB
#ifndef IKSDB // IKSD database
#ifdef UNIX
#define IKSDB
#define IK_LCKTRIES 16          // How many times to try to get lock
#define IK_LCKSLEEP 1           // How long to sleep between tries
#define IK_LOCKFILE "iksd.lck"  // Database lockfilename
#define IK_DBASEDIR "/var/log/" // Database directory
#define IK_DBASEFIL "iksd.db"   // Database filename
#else                           // UNIX
#endif                          // UNIX
#endif                          // IKSDB
#endif                          // NOIKSDB
#endif                          // IKSD
// Substitutes for printf() and friends used in IKS to compensate for
// lack of a terminal driver, mainly to supply CR after LF.
#ifndef NOPRINTFSUBST

#ifndef CKWART_C
#ifdef UNIX
#ifndef CKXPRINTF
#define CKXPRINTF
#endif // CKXPRINTF
#endif // UNIX
#endif // CKWART_C
#endif // NOPRINTFSUBST

#ifdef CKXPRINTF
#define printf ckxprintf
#define fprintf ckxfprintf
int ckxprintf(const char *, ...);
int ckxperror(const char *);
int ckxfprintf(FILE *, const char *, ...);
#ifdef putchar
#undef putchar
#endif // putchar
#define putchar(x) ckxprintf("%c", x)
#ifdef putc
#undef putc
#endif // putc
#define putc(a, b) ckxfprintf(b, "%c", a)
#define perror(x) ckxperror(x)
#endif // CKXPRINTF

// Altos-specific items: 486, 586, 986 models...

// Signal handling

#ifdef CKNTSIG
// This does not work, so don't use it.
#define signal ckntsignal
void (*ckntsignal(int type, void (*)(int)))(int);
#endif // CKNTSIG

// Signal-handler pointer type: void function of one int, returning void.
typedef void (*ck_sig_t)(int);

// signal()-compatible wrapper implemented with sigaction() (in ckusig.c), used
// at the save/restore call sites in place of signal() for deterministic
// cross-platform (Linux, macOS, BSD) semantics.  Returns the previous handler.
ck_sig_t ck_signal(int type, ck_sig_t);

// We want all characters to be unsigned if the compiler supports it

#ifdef V7
typedef char CHAR;
#else
#ifdef CHAR
#undef CHAR
#endif // CHAR
typedef unsigned char CHAR;
#endif // V7

union ck_short { // Mainly for Unicode
  USHORT x_short;
  CHAR x_char[2];
};

// Systems whose mainline modules have access to the communication-line
// file descriptor, ttyfd.
#ifndef CK_TTYFD
#ifdef UNIX
#define CK_TTYFD
#else
#endif // UNIX
#endif // CK_TTYFD

// Systems where we can get our own process ID

#ifndef CK_PID
#ifdef UNIX
#define CK_PID
#endif // UNIX
#endif // CK_PID

// Systems that support the Microsoft Telephony API (TAPI)

#ifndef NODIAL
#ifndef CK_TAPI
#endif // CK_TAPI
#endif // NODIAL

#ifndef NONZXPAND
#ifndef NZXPAND
#define NZXPAND
#endif // NZXPAND
#else
#ifdef NZXPAND
#undef NZXPAND
#endif // NZXPAND
#endif // NONZXPAND

// nzxpand() option flags

#define ZX_FILONLY 1   // Match only regular files
#define ZX_DIRONLY 2   // Match only directories
#define ZX_RECURSE 4   // Descend through directory tree
#define ZX_MATCHDOT 8  // Match "dot files"
#define ZX_NOBACKUP 16 // Don't match "backup files"
#define ZX_NOLINKS 32  // Don't follow symlinks

#ifndef NZXPAND
#define nzxpand(a, b) zxpand(a)
#endif // NZXPAND

#ifndef NOZXREWIND
#ifndef ZXREWIND // Platforms that have zxrewind()
#define ZXREWIND
#endif // ZXREWIND
#else
#ifdef ZXREWIND
#undef ZXREWIND
#endif // ZXREWIND
#endif // NOZXREWIND

// Temporary-directory-for-RECEIVE feature ...
// This says whether we have the isdir() function defined.

#ifdef UNIX // UNIX has it
#ifndef CK_TMPDIR
#define CK_TMPDIR
#define TMPDIRLEN 256
#endif // CK_TMPDIR
#endif // UNIX

#ifdef CK_TMPDIR // Needs command parser
#ifdef NOICP
#undef CK_TMPDIR
#endif // NOICP
#endif // CK_TMPDIR

// Whether to include <time.h> or <sys/time.h>

#ifndef NOTIMEH // <time.h>
#ifndef TIMEH
#define TIMEH
#endif // TIMEH
#endif // NOTIMEH

#ifndef NOSYSTIMEH // <sys/time.h>
#ifndef SYSTIMEH
#ifdef UNIX      // UNIX
#ifdef SVORPOSIX // System V or POSIX...
#ifdef BSD44
#define SYSTIMEH
#else
#ifdef __linux__
#define SYSTIMEH
#else
#endif // __linux__
#endif // BSD44

#else // Not SVORPOSIX

#ifndef V7
#define SYSTIMEH
#endif // V7
#endif // SVORPOSIX
#endif // UNIX
#endif // SYSTIMEH
#endif // NOSYSTIMEH

#ifndef NOSYSTIMEBH // <sys/timeb.h>
#ifndef SYSTIMEBH
#endif // SYSTIMEBH
#endif // NOSYSTIMEBH

// Debug and transaction logging is included automatically unless you define
// NODEBUG or NOTLOG.  Do this if you want to save the space and overhead.
// (Note, in version 4F these definitions changed from "{}" to the null string
// to avoid problems with semicolons after braces, as in: "if (x) tlog(this);
// else tlog(that);"
#ifndef NODEBUG
#ifndef DEBUG
#define DEBUG
#endif // DEBUG
#else
#ifdef DEBUG
#undef DEBUG
#endif // DEBUG
#endif // NODEBUG

#ifdef NOTLOG
#ifdef TLOG
#undef TLOG
#endif // TLOG
#else  // NOTLOG
#ifndef TLOG
#define TLOG
#endif // TLOG
#endif // NOTLOG

// debug() macro style selection.

#ifndef CKCMAI
extern int deblog;
extern int debok;
extern int debxlen;
extern int matchdot;
extern int tt_bell;
#endif // CKCMAI

#define bleep(x)                                                               \
  if (tt_bell)                                                                 \
  putchar('\07')

#ifdef NOICP
#ifdef TLOG
#undef TLOG
#endif // TLOG
#endif // NOICP

// Formats for debug() and tlog()

#define F000 0
#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

#ifdef __linux__
#ifndef LINUX
#define LINUX
#endif // LINUX
#endif // __linux__

// Platforms where small size is needed

// Can we use realpath()?

#ifndef NOREALPATH
#endif // NOREALPATH

#ifndef NOREALPATH
#ifdef UNIX
#endif // NOREALPATH

#ifndef NOREALPATH
#ifndef CKREALPATH
#define CKREALPATH
#endif // NOREALPATH
#endif // CKREALPATH
#endif // UNIX

#ifdef CKREALPATH
#ifndef CKROOT
#define CKROOT
#endif // CKROOT
#endif // CKREALPATH

// CKSYMLINK should be set only if we can use readlink()

#ifdef UNIX
#ifndef NOSYMLINK
#ifndef CKSYMLINK
#define CKSYMLINK
#endif // NOSYMLINK
#endif // CKSYMLINK
#endif // UNIX

// Platforms where we can use lstat() instead of stat() (for symlinks)
// This should be set only if both lstat() and readlink() are available

#ifndef NOLSTAT
#ifndef NOSYMLINK
#ifndef USE_LSTAT
#ifdef UNIX
#ifdef CKSYMLINK
#if defined(SVR4) // SVR4 has lstat()
#define USE_LSTAT
#elif defined(BSD44) // 4.4BSD has it
#define USE_LSTAT
#elif defined(LINUX) // LINUX has it
#define USE_LSTAT
#endif // SVR4 / BSD44 / LINUX
#endif // CKSYMLINK
#endif // UNIX
#endif // USE_LSTAT
#endif // NOSYMLINK
#endif // NOLSTAT

#ifdef NOLSTAT
#ifdef USE_LSTAT
#undef USE_LSTAT
#endif // USE_LSTAT
#endif // NOLSTAT

#ifndef NOTTYLOCK // UNIX systems that have ttylock()
#ifndef USETTYLOCK
#ifdef USE_UU_LOCK // FreeBSD or other with uu_lock()
#define USETTYLOCK
#else
// Prior to 8.0.299 Alpha.08 this was HAVE_BAUDBOY which was added for
// Red Hat 7.2 in May 2003 but which is no longer supported in Debian and
// OpenSuse (at least).
#ifdef HAVE_LOCKDEV
#define USETTYLOCK
#endif // HAVE_LOCKDEV
#endif // USE_UU_LOCK
#endif // USETTYLOCK
#endif // NOTTYLOCK
// This could be more inclusive...  But better not to use snprintf() at all,
// it's hard to find a way to test for its availability without using
// nonportable preprocessor constructions.  Use ckclib.c: ckmakmsg() or
// ckmakxmsg() instead of both sprintf() and snprintf() to squelch compiler
// warnings and ensure no memory leaks.
#ifndef HAVE_SNPRINTF // Safe to use snprintf()
#ifdef HAVE_OPENPTY
#define HAVE_SNPRINTF
#endif // HAVE_OPENPTY
#endif // HAVE_SNPRINTF

// Kermit feature selection

#ifndef NOSPL
#ifndef NOCHANNELIO // Channel-based file i/o package
#ifndef CKCHANNELIO
#ifdef UNIX
#define CKCHANNELIO
#else
#endif // UNIX
#endif // CKCHANNELIO
#endif // NOCHANNELIO
#endif // NOSPL

#ifndef NOCKEXEC // EXEC command
#ifndef NOPUSH
#ifndef CKEXEC
#ifdef UNIX // UNIX can do it
#define CKEXEC
#endif // UNIX
#endif // CKEXEC
#endif // NOPUSH
#endif // NOCKEXEC

#ifndef NOFAST // Fast Kermit protocol by default
#ifndef CK_FAST
#ifdef UNIX
#define CK_FAST
#else
#endif // UNIX
#endif // CK_FAST
#endif // NOFAST

#ifdef UNIX // Transparent print
#ifndef NOXPRINT
#ifndef XPRINT
#define XPRINT
#endif // XPRINT
#endif // NOXPRINT
#endif // UNIX

#ifndef NOHWPARITY // Hardware parity
#ifndef HWPARITY
#ifdef SVORPOSIX // System V or POSIX can have it
#define HWPARITY
#else
#endif // SVORPOSIX
#endif // HWPARITY
#endif // NOHWPARITY

#ifndef NOSTOPBITS // Stop-bit selection
#ifndef STOPBITS
// In Unix really this should only be if CSTOPB is defined.
// But we don't know that yet.
#define STOPBITS
#endif // STOPBITS
#endif // NOSTOPBITS

#ifdef UNIX
#ifndef NETCMD // Can SET NETWORK TYPE COMMAND
#define NETCMD
#endif // NETCMD
#endif // UNIX

// Pty support, nonportable, available on a case-by-case basis

#ifndef NOPTY
#ifdef BSD44 // BSD44, {Net,Free,Open}BSD
#define NETPTY
#else
#ifdef LINUX // Linux
#define NETPTY
#else
#endif // LINUX
#endif // BSD44

#else // NOPTY

#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#endif // NOPTY

#ifdef NETPTY // NETCMD required for NETPTY
#ifndef NETCMD
#define NETCMD
#endif // NETCMD

#ifndef NO_OPENPTY // Can use openpty()
#ifndef HAVE_OPENPTY
#ifdef __linux__
#define HAVE_OPENPTY
#else
#ifdef __FreeBSD__
#define HAVE_OPENPTY
#else
#ifdef __OpenBSD__
#define HAVE_OPENPTY
#else
#ifdef __NetBSD__
#define HAVE_OPENPTY
#include <util.h>
#else
#ifdef MACOSX10
#define HAVE_OPENPTY
#endif // MACOSX10
#endif // __NetBSD__
#endif // __OpenBSD__
#endif // __FreeBSD__
#endif // __linux__
#endif // HAVE_OPENPTY
#endif // NO_OPENPTY
// This needs to be expanded and checked.
// The makefile assumes the library (at least for all linuxes)
// is always libutil but I've only verified it for a few.
// If a build fails because
#ifdef HAVE_OPENPTY
#ifdef __linux__
#include <pty.h>
#else
#ifdef __NetBSD__
#include <util.h>
#else
#ifdef __OpenBSD__
#include <util.h>
#else
#ifdef __FreeBSD__
#include <libutil.h>
#else
#ifdef MACOSX
#include <util.h>
#else
#endif // MACOSX
#endif // __FreeBSD__
#endif // __OpenBSD__
#endif // __NetBSD__
#endif // __linux__
#endif // HAVE_OPENPTY
#endif // NETPTY

#ifndef CK_UTSNAME // Can we call uname()?
#ifdef POSIX       // It's in POSIX.1
#define CK_UTSNAME
#else
#ifdef SVR4 // It's in SVR4 (but not SVR3)
#define CK_UTSNAME
#else
#endif // SVR4
#endif // POSIX
#endif // CK_UTSNAME

// This section for anything that might use floating-point

// If the following causes trouble use -DFLOAT=float on the command line

#ifdef NOSPL
#ifdef FNFLOAT
#undef FNFLOAT
#endif // FNFLOAT
#ifdef CKFLOAT
#undef CKFLOAT
#endif // CKFLOAT
#endif // NOSPL

#ifndef NOFLOAT

#ifdef __alpha      // Why only __alpha?  Other 64-bit systems?
#define FLT_NOT_DBL // (See also ckclib.c:ckround()).
#else               // def __alpha
#ifdef VMS64
#define FLT_NOT_DBL // Was testing only __alpha below.
#endif              // def VMS64
#endif              // def __alpha [else]

#ifndef CKFLOAT
#ifdef FLT_NOT_DBL // 2024-05-16 SMS.  Use instead of __alpha.
// Don't use double on 64-bit platforms -- bad things happen
// "double" on 64-bit platforms typically means 128-bit?  Do we care?
#define CKFLOAT float
#define CKFLOAT_S "float"
#else // def FLT_NOT_DBL
#define CKFLOAT double
#define CKFLOAT_S "double"
#endif // def FLT_NOT_DBL [else]
#endif // CKFLOAT

#ifndef NOGFTIMER // Floating-point timers
#ifndef GFTIMER
#ifdef UNIX // For UNIX
#define GFTIMER
#endif // UNIX
#endif // GFTIMER
#endif // NOGFTIMER

#ifndef NOSPL
#ifndef FNFLOAT // Floating-point math functions
#endif          // FNFLOAT
#endif          // NOSPL

#else // NOFLOAT is defined

#ifdef CKFLOAT
#undef CKFLOAT
#endif // CKFLOAT

#ifdef GFTIMER
#undef GFTIMER
#endif // GFTIMER

#ifdef FNFLOAT
#undef FNFLOAT
#endif // FNFLOAT

#endif // NOFLOAT

#ifdef GFTIMER    // Fraction of second to use when
#ifndef GFMINTIME // elapsed time is <= 0
#define GFMINTIME 0.005
#endif // GFMINTIME
#endif // GFTIMER

#ifndef CKCMAI
extern long ztmsec, ztusec; // Fraction of sec of current time
#endif                      // CKCMAI

#ifndef NOUNPREFIXZERO // Allow unprefixing of NUL (0)
#ifndef UNPREFIXZERO   // in file-transfer packets
#define UNPREFIXZERO
#endif // UNPREFIXZERO
#endif // NOUNPREFIXZERO

#ifdef CK_SMALL
#define NOCAL // Calibrate
#endif        // CK_SMALL

#ifndef NOPATTERNS // Filetype matching patterns
#ifndef PATTERNS
#ifndef CK_SMALL
#define PATTERNS
#endif // CK_SMALL
#endif // PATTERNS
#endif // NOPATTERNS

#ifndef NOCAL
#ifndef CALIBRATE
#define CALIBRATE
#endif // CALIBRATE
#else
#ifdef CALIBRATE
#undef CALIBRATE
#endif // CALIBRATE
#endif // NOCAL

#ifndef NORECURSE // Recursive directory traversal
#ifndef RECURSIVE
#ifndef CK_SMALL
#define RECURSIVE
#endif // CK_SMALL
#endif // RECURSIVE
#endif // NORECURSE

#ifndef CK_SMALL // Enable file-transfer tuning code
#ifndef CKTUNING // in which more code is added
#ifndef NOTUNING // to avoid function calls, etc
#define CKTUNING
#endif // NOTUNING
#endif // CKTUNING
#endif // CK_SMALL

#ifndef NOURL // Parse URLs in SET HOST, etc
#define CK_URL
#define NO_FTP_AUTH // No auth "ftp" / "anonymous"
#endif              // NOURL

#ifndef NOTRIGGER
#ifndef CK_TRIGGER // Trigger string to exit CONNECT
#define CK_TRIGGER
#endif // CK_TRIGGER
#endif // NOTRIGGER

#ifdef CK_TRIGGER
#define TRIGGERS 8 // How many triggers allowed
#endif             // CK_TRIGGER

#ifndef XLIMITS // CONNECT limits
#endif          // XLIMITS

#ifdef NOFRILLS
#ifndef NOBROWSER
#define NOBROWSER
#endif // NOBROWSER
#endif // NOFRILLS

#ifndef NOHTTP // HTTP features need...
#ifdef NOICP   // an interactive command parser
#define NOHTTP
#endif // NOICP
#endif // NOHTTP

#ifndef NONET
#ifdef TCPSOCKET

// The HTTP code is not very portable, so it must be asked for with -DCKHTTP

#ifndef NOHTTP
#ifndef CKHTTP
#ifdef LINUX // And Linux
#define CKHTTP
#endif            // LINUX
#ifdef __NetBSD__ // NetBSD
#define CKHTTP
#endif // __NetBSD__
#ifdef __FreeBSD__
#define CKHTTP
#endif // __FreeBSD__
#ifdef __OpenBSD__
#define CKHTTP
#endif // __OpenBSD__
// Add more here...
#endif         // CKHTTP
#ifndef CKHTTP // If CKHTTP not defined yet
#define NOHTTP // then define NOHTTP
#endif         // CKHTTP
#endif         // NOHTTP

#ifdef NETCONN // Special "network" types...
#ifndef NOLOCAL
#endif // NOLOCAL
#endif // NETCONN

#ifndef NOBROWSER
#ifdef UNIX
#ifndef BROWSER
#ifndef NOPUSH
#define BROWSER
#endif // NOPUSH
#endif // BROWSER
#endif // UNIX
#else
#ifdef BROWSER
#undef BROWSER
#endif // BROWSER
#endif // NOBROWSER

#else          // TCPSOCKET
#ifndef NOHTTP // HTTP requires TCPSOCKET
#define NOHTTP
#endif // NOHTTP
#endif // TCPSOCKET
#endif // NONET

#ifdef TCPSOCKET
#ifndef NOCKGETFQHOST
#ifdef __ia64__
#define NOCKGETFQHOST
#else  // __ia64__
#endif // __ia64
#endif // NOCKGETFQHOST
// Regarding System V/68 (SV68) (from Gerry Belanger, Oct 2002):
//
//  1) The gethostbyname() appears to return the actual host IP
//     address in the hostent struct, instead of the expected pointer
//     to the address. Hence the bogus address in the bcopy/memcopy.
//     This is despite the header agreeing with our expectations.
//
//  2) the expected argument swap between bcopy and memcopy
//     did not happen.  What grief this might cause, I know not.
#endif // TCPSOCKET

#ifdef TCPSOCKET
#ifdef NOSOCKS
#ifdef CK_SOCKS
#undef CK_SOCKS
#endif // CK_SOCKS
#ifdef CK_SOCKS5
#undef CK_SOCKS5
#endif           // CK_SOCKS5
#else            // NOSOCKS
#ifdef CK_SOCKS5 // CK_SOCKS5 implies CK_SOCKS
#ifndef CK_SOCKS
#define CK_SOCKS
#endif // CK_SOCKS
#endif // CK_SOCKS5
#endif // NOSOCKS
#endif // TCPSOCKET

#ifdef TNCODE

#ifdef NO_AUTHENTICATION // Allow authentication to be
#endif                   // NO_AUTHENTICATION

#ifdef NO_ENCRYPTION // Allow encryption to be
#endif               // NO_ENCRYPTION

// SSH section.  NOSSH disables any form of SSH support.
// If NOSSH is not defined (or implied by NONET, NOLOCAL, etc)
// then SSHBUILTIN is defined for K95 and SSHCMD is defined for UNIX.
// Then, if either SSHBUILTIN or SSHCMD is defined, ANYSSH is also defined.

#ifdef NOSSH      // NOSSH
#ifdef SSHBUILTIN // undefines any SSH selctors
#undef SSHBUILTIN
#endif // SSHBUILTIN
#ifdef SFTP_BUILTIN
#undef SFTP_BUILTIN
#endif // SFTP_BUILTIN
#ifdef SSHCMD
#undef SSHCMD
#endif // SSHCMD
#ifdef ANYSSH
#undef ANYSSH
#endif // ANYSSH

#else // Not NOSSH

#ifndef NOLOCAL
#ifdef UNIX
#ifndef SSHCMD
#ifdef NETPTY
#ifndef NOPUSH
#define SSHCMD
#endif // NOPUSH
#endif // NETPTY
#endif // SSHCMD
#endif // UNIX

#ifndef ANYSSH
#ifdef SSHBUILTIN
#define ANYSSH
#ifdef SSHCMD
#undef SSHCMD
#endif // SSHCMD
#else  // SSHBUILTIN
#ifdef SSHCMD
#define ANYSSH
#endif // SSHCMD
#endif // SSHBUILTIN
#endif // ANYSSH
#endif // NOLOCAL
#endif // NOSSH

// This is in case #ifdef SSH is used anywhere in the K95 modules

// Environment stuff

#ifndef CK_ENVIRONMENT
#ifdef UNIX
#define CK_ENVIRONMENT
#else
#endif           // UNIX
#endif           // CK_ENVIRONMENT
#ifndef NOSNDLOC // RFC 779 SEND LOCATION
#ifndef CK_SNDLOC
#define CK_SNDLOC
#endif             // CK_SNDLOC
#endif             // NOSNDLOC
#ifndef NOXDISPLOC // RFC 1096 XDISPLOC
#ifndef CK_XDISPLOC
#define CK_XDISPLOC
#endif // CK_XDISPLOC
#endif // NOXDISPLOC
#ifndef NOFORWARDX
#ifndef NOPUTENV
#ifndef NOSELECT
#endif // NOSELECT
#endif // NOPUTENV
#endif // NOFORWARDX
#ifndef NO_COMPORT
#ifdef TCPSOCKET
#ifndef TN_COMPORT
#define TN_COMPORT
#endif // TN_COMPORT
#endif // TCPSOCKET
#endif // NO_COMPORT
#endif // TNCODE

#ifndef NOXFER
#ifndef NOCTRLZ // Allow SET FILE EOF CTRL-Z
#ifndef CK_CTRLZ
#define CK_CTRLZ
#endif // CK_CTRLZ
#endif // NOCTRLZ
#endif // NOXFER

#ifndef NOPERMS // File permissions in A packets
#ifndef CK_PERMS
#ifdef UNIX
#define CK_PERMS
#else
#endif // UNIX
#endif // CK_PERMS
#endif // NOPERMS
#ifdef CK_PERMS
#define CK_PERMLEN 24 // Max length of sys-dependent perms
#endif                // CK_PERMS

#ifdef UNIX // NOSETBUF for everybody
#ifndef NOSETBUF
#ifndef USE_SETBUF // This is the escape clause
#define NOSETBUF
#endif // USE_SETBUF
#endif // NOSETBUF
#endif // UNIX

#ifndef USE_STRERROR // Whether to use strerror()
#endif               // USE_STRERROR

#ifndef NOCKTIMERS // Dynamic timeouts
#ifndef CK_TIMERS
#define CK_TIMERS
#endif // CK_TIMERS
#endif // NOCKTIMERS

#define CK_SPEED // Control-prefix removal
#ifdef NOCKSPEED
#undef CK_SPEED
#endif // NOCKSPEED

#ifndef NOCKXXCHAR
#ifndef CKXXCHAR
#ifdef UNIX
#define CKXXCHAR
#else
#endif // UNIX
#endif // CKXXCHAR
#endif // NOCKXXCHAR

// Systems where we can call zmkdir() to create directories.

#ifndef CK_MKDIR
#ifndef NOMKDIR

#ifdef UNIX
#define CK_MKDIR
#endif // UNIX

#endif // CK_MKDIR
#endif // NOMKDIR

#ifdef NOMKDIR // Allow for command-line override
#ifdef CK_MKDIR
#undef CK_MKDIR
#endif // CK_MKDIR
#endif // NOMKDIR

// Systems for which we can enable the REDIRECT command automatically
//   As of 6.0.193, it should work for all UNIX...

#ifndef NOREDIRECT
#ifndef CK_REDIR
#ifdef UNIX
#define CK_REDIR
#endif // UNIX
#endif // CK_REDIR
#endif // NOREDIRECT

#ifdef NOPUSH   // But... REDIRECT command is not
#ifdef CK_REDIR //  allowed if NOPUSH is defined.
#undef CK_REDIR
#endif        // CK_REDIR
#ifdef NETCMD // Nor is SET NET COMMAND
#undef NETCMD
#endif // NETCMD
#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#endif // NOPUSH

#ifndef PEXITSTAT // \v(pexitstat) variable defined
#define PEXITSTAT
#endif // PEXITSTAT

// The following allows automatic enabling of REDIRECT to be overridden...

#ifdef NOREDIRECT
#ifdef NETCMD
#undef NETCMD
#endif // NETCMD
#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#ifdef CK_REDIR
#undef CK_REDIR
#endif // CK_REDIR
#endif // NOREDIRECT

#ifdef NONETCMD
#ifdef NETCMD
#undef NETCMD
#endif // NETCMD
#ifdef NETPTY
#undef NETPTY
#endif // NETPTY
#endif // NONETCMD

#ifdef CK_REDIR
int ttruncmd(char *);
#endif // CK_REDIR

// Use built-in DIRECTORY command

#ifndef NOMYDIR
#ifndef DOMYDIR
#define DOMYDIR
#endif // DOMYDIR
#endif // NOMYDIR

// Sending from and receiving to commands/pipes

#ifndef PIPESEND
#ifdef UNIX
#define PIPESEND
#endif // UNIX
#endif // PIPESEND

#ifdef PIPESEND
#ifdef NOPIPESEND
#undef PIPESEND
#endif // NOPIPESEND
#ifdef NOPUSH
#undef PIPESEND
#endif // NOPUSH
#endif // PIPESEND

#ifdef NOPUSH
#ifdef BROWSER
#undef BROWSER
#endif // BROWSER
#endif // NOPUSH

// Versions where we support the RESEND command

#ifndef NOXFER
#ifndef NORESEND
#ifndef CK_RESEND
#ifdef UNIX
#define CK_RESEND
#endif // UNIX

#endif // CK_RESEND
#endif // NORESEND
#endif // NOXFER

// Systems implementing "Doomsday Kermit" protocol ...

#ifndef DOOMSDAY
#ifdef UNIX
#define DOOMSDAY
#else
#endif // UNIX
#endif // DOOMSDAY

// Systems where we want the Thermometer to be used for fullscreen

// Systems where we have a REXX command

// Platforms that have a ZCHKPID function

#define ZCHKPID

#ifndef ZCHKPID
// If we can't check pids then we have treat all pids as active & valid.
#define zchkpid(x) 1
#endif // ZCHKPID

// Systems that have a ZRENAME function

#define ZRENAME // They all do

// Systems that have a ZCOPY function

#ifndef ZCOPY
#ifdef UNIX
#define ZCOPY
#else
#endif // UNIX
#endif // ZCOPY

// Systems that have ttgwsiz() (they all should but they don't)

#ifndef NOTTGWSIZ
#ifndef CK_TTGWSIZ
#ifdef UNIX
#define CK_TTGWSIZ
#else
#endif // UNIX
#endif // CK_TTGWSIZ
#endif // NOTTGWSIZ

#ifdef NOTTGWSIZ
#ifdef CK_TTGWSIZ
#undef CK_TTGWSIZ
#endif // CK_TTGWSIZ
#endif // NOTTGWSIZ

// Systems that have select().
// This is used for both msleep() and for read-buffer checking in in_chk().
#define CK_SLEEPINT                                                            \
  250 // milliseconds - set this to something that                             \
      // divides evenly into 1000
#ifndef SELECT
#ifndef NOSELECT
#ifdef __linux__
#define SELECT
#else
#ifdef BSD44
#define SELECT
#else
#ifdef BSD4
#define SELECT
#else
#endif // BSD4
#endif // BSD44
#endif // SUNOS4
#endif // NOSELECT
#endif // SELECT

// The following section moved here from ckcnet.h in 6.1 because select()
// is now used for non-networking purposes.

// On HP-9000/500 HP-UX 5.21 this stuff is not defined in any header file

#ifdef NEEDSELECTDEFS
typedef long fd_mask;
#ifndef NBBY
#define NBBY 8
#endif // NBBY
#ifndef FD_SETSIZE
#define FD_SETSIZE 32
#endif // FD_SETSIZE
#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask) * NBBY)
#endif // NFDBITS
#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif // howmany
typedef struct fd_set {
  fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
#ifndef FD_SET
#define FD_SET(n, p) ((p)->fds_bits[(n) / NFDBITS] |= (1 << ((n) % NFDBITS)))
#endif // FD_SET
#ifndef FD_CLR
#define FD_CLR(n, p) ((p)->fds_bits[(n) / NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#endif // FD_CLR
#ifndef FD_ISSET
#define FD_ISSET(n, p) ((p)->fds_bits[(n) / NFDBITS] & (1 << ((n) % NFDBITS)))
#endif // FD_ISSET
#ifndef FD_COPY
#define FD_COPY(f, t) (bcopy(f,t,sizeof(*(f)))
#endif // FD_COPY
#ifndef FD_ZERO
#define FD_ZERO(p) bzero((char *)(p), sizeof(*(p)))
#endif // FD_ZERO
#endif // NEEDSELECTDEFS

// CK_NEED_SIG is defined if the system cannot check the console to
// to see if characters are waiting.  This is used during local-mode file
// transfer to interrupt the transfer, refresh the screen display, etc.
// If CK_NEED_SIG is defined, then file-transfer interruption characters
// have to be preceded a special character, e.g. the SIGQUIT character.
// CK_NEED_SIG should be defined if the conchk() function is not operational.
#ifdef NOPOLL // For overriding CK_POLL definition
#ifdef CK_POLL
#undef CK_POLL
#endif // CK_POLL
#endif // NOPOLL

#ifndef CK_POLL // If we don't have poll()
#ifndef RDCHK   // And we don't have rdchk()
#ifndef SELECT  // And we don't have select()
#ifdef ATTSV
#define CK_NEED_SIG
#endif // ATTSV
#ifdef POSIX
#ifndef CK_NEED_SIG
#define CK_NEED_SIG
#endif // CK_NEED_SIG
#endif // POSIX
#endif // SELECT
#endif // RDCHK
#endif // CK_POLL

#ifdef BSD44 // 4.4BSD has FIONREAD
#ifdef CK_NEED_SIG
#undef CK_NEED_SIG
#endif // CK_NEED_SIG
#endif // BSD44

#ifdef UNIX
#ifndef HAVE_TZ // Can we use struct timezone?
#ifndef NOTIMEZONE
#ifndef SELECT
#endif            // SELECT
#endif            // NOTIMEZONE
#endif            // HAVE_TZ
#ifndef NOTIMEVAL // Can we use struct timeval?
#ifndef HAVE_TV
#define HAVE_TV
#endif // HAVE_TV
#endif // NOTIMEVAL
#ifndef NOTIMEZONE
#ifndef HAVE_TZ
#define HAVE_TZ
#endif // HAVE_TZ
#endif // NOTIMEZONE
#endif // UNIX

// Automatic parity detection.
// This actually implies a lot more now: length-driven packet reading,
// "Doomsday Kermit" IBM Mainframe file transfer through 3270 data streams, etc.
#ifdef UNIX // For Unix
#ifndef NOPARSEN
#define PARSENSE
#endif // NOPARSEN
#endif // UNIX

#ifndef NODYNAMIC // DYNAMIC is default for UNIX
#ifndef DYNAMIC   // as of C-Kermit 7.0
#ifdef UNIX
#define DYNAMIC
#endif // UNIX
#endif // DYNAMIC
#endif // NODYNAMIC

#ifdef DYNAMIC  // If DYNAMIC is defined
#define DCMDBUF // then also define this.
#endif          // DYNAMIC

#ifndef CK_LBRK // Can send Long BREAK

#ifdef UNIX // (everybody but OS-9)
#define CK_LBRK
#endif // UNIX

#endif // CK_LBRK

// Carrier treatment
// These are defined here because they are shared by the system dependent
// and the system independent modules.

#define CAR_OFF 0 // Off: ignore carrier always.
#define CAR_ON 1  // On: heed carrier always, except during DIAL.
#define CAR_AUT                                                                \
  2 // Auto: heed carrier, but only if line is declared
    // to be a modem line, and only during CONNECT.

// And more generically (for use with any ON/OFF/AUTO feature)
#define CK_OFF 0
#define CK_ON 1
#define CK_AUTO 2

#ifndef NOLOCAL
// Serial interface speeds available.
//
// As of C-Kermit 6.1 there is a new method to get the supported
// speeds, which obviates the need for all the craziness below.  At runtime,
// just call the new ttspdlist() routine to get a list of supported speeds.
// Then the user interface module can build a keyword table or menu from it.
#ifndef TTSPDLIST
#ifdef UNIX           // For now, only for UNIX
#ifndef OLINUXHISPEED // But not systems with hacks for
#define TTSPDLIST
#endif // OLINUXHISPEED
#else
#endif // UNIX
#endif // TTSPDLIST

#ifndef NODIAL // Hangup by modem command
#ifndef NOMDMHUP
#ifndef MDMHUP
#define MDMHUP
#endif // MDMHUP
#endif // NOMDMHUP
#endif // NODIAL

#ifdef NOSPL
#ifndef NOLOGDIAL // Connection log needs mjd(), etc.
#define NOLOGDIAL
#endif // NOLOGDIAL
#endif // NOSPL

#ifndef NOLOGDIAL // Connection log
#ifndef CXLOGFILE
#define CXLOGFILE "CX.LOG" // Default connection log file name
#endif                     // CXLOGFILE
#ifndef CKLOGDIAL
#ifndef CK_SMALL
#define CKLOGDIAL
#define CXLOGBUFL 1024 // Connection log record buffer size
#endif                 // CK_SMALL
#endif                 // NOLOGDIAL
#endif                 // CKLOGDIAL

#endif // NOLOCAL

#ifdef NOTTSPDLIST // Except if NOTTSPDLIST is defined
#ifdef TTSPDLIST
#undef TTSPDLIST
#endif // TTSPDLIST
#endif // NOTTSPDLIST

#ifdef TTSPDLIST

long *ttspdlist(void);

#else // TTSPDLIST not defined
// We must use a long and convoluted series of #ifdefs that have to be kept in
// sync with the code in the ck?tio.c module.
//
// We assume that everybody supports: 0, 110, 300, 600, 1200, 2400, 4800, and
// 9600 bps.  Symbols for other speeds are defined here.  You can also add
// definitions on the CC command lines.  These definitions affect the SET SPEED
// keyword table, and are not necessarily usable in the system-dependent
// speed-setting code in the ck?tio.c modules, which depends on system-specific
// symbols like (in UNIX) B19200.  In other words, just defining it doesn't
// mean it'll work -- you also have to supply the supporting code in ttsspd()
// and ttgspd() in ck?tio.c.
//
// The symbols have the form BPS_xxxx, where xxxx is the speed in bits per
// second, or (for bps values larger than 9999) thousands of bps followed by K.
// The total symbol length should be 8 characters or less.  Some values are
// enabled automatically below.  You can disable a particular value by defining
// NOB_xxxx on the CC command line.
//

#ifndef NOB_50
#define BPS_50 // 50 bps
#endif

#ifndef NOB_75
#define BPS_75 // 75 bps
#endif

#ifndef NOB7512
#ifdef ANYBSD
#define BPS_7512 // 75/1200 Split Speed
#endif           // ANYBSD
#endif           // NOB7512

#ifndef NOB134
#undef BPS_134 // 134.5 bps (IBM 2741)
#endif         // NOB134

#ifndef NOB_150
#define BPS_150 // 150 bps
#endif

#ifndef NOB_200
#define BPS_200 // 200 bps
#endif

#ifndef NOB_1800
#endif

#ifndef NOB_3600
#define BPS_3600 // 3600 bps
#endif

#ifndef NOB_7200
#define BPS_7200 // 7200 bps
#endif

#ifndef NOB_14K
#ifdef BSD44
#define BPS_14K // 14400 bps
#else
#endif // BSD44
#endif // NOB_14K

#ifndef NOB_19K
#define BPS_19K // 19200 bps
#endif

#ifndef NOB_28K
#ifdef BSD44
#define BPS_28K
#else
#endif // BSD44
#endif // NOB_28K

#ifndef NOB_38K
#define BPS_38K // 38400 bps
#endif

#ifndef NOB_57K
#ifdef __linux__
#define BPS_57K
#else
#ifdef __FreeBSD__
#define BPS_57K
#else
#ifdef __NetBSD__
#define BPS_57K
#else
#endif // __NetBSD__
#endif // __FreeBSD__
#endif // __linux__
#endif // NOB_57K

#ifndef NOB_76K
#endif // NOB_76K

#ifndef NOB_115K
#ifdef __linux__
#define BPS_115K
#define BPS_1500K
#else
#ifdef __FreeBSD__
#define BPS_115K
#else
#ifdef __NetBSD__
#define BPS_115K
#else
#endif // __NetBSD__
#endif // __FreeBSD__
#endif // __linux__
#endif // NOB_115K

#ifndef NOB_230K // 230400 bps
#ifdef __linux__
#define BPS_230K
#else
#undef BPS_230K
#endif // __linux__
#endif // NOB_230K

#ifndef NOB_460K // 460800 bps
#ifdef __linux__
#define BPS_460K
#else
#undef BPS_460K
#endif // SCO_OSR504
#endif // NOB_460K

#ifndef NOB_921K       // 921600 bps
#endif                 // NOB_921K

// 13 October 2021
// From Elad Lahav:
// Added support for 1.5MHz (1500000bps) serial speed for Linux and QNX.
#if defined(BPS_1500K) // Maximum speed defined
#define MAX_SPD 1500000L
#elif defined(BPS_921K)
#define MAX_SPD 921600L
#elif defined(BPS_460K)
#define MAX_SPD 460800L
#elif defined(BPS_230K)
#define MAX_SPD 230400L
#elif defined(BPS_115K)
#define MAX_SPD 115200L
#elif defined(BPS_76K)
#define MAX_SPD 76800L
#elif defined(BPS_57K)
#define MAX_SPD 57600L
#elif defined(BPS_38K)
#define MAX_SPD 38400L
#elif defined(BPS_28K)
#define MAX_SPD 28800L
#elif defined(BPS_19K)
#define MAX_SPD 19200L
#elif defined(BPS_14K)
#define MAX_SPD 14400L
#else
#define MAX_SPD 9600L
#endif
#endif // TTSPDLIST

#ifndef CONGSPD // Systems that can call congspd()
#ifdef UNIX
#define CONGSPD
#endif // UNIX
#endif // CONGSPD

// Types of flow control available

#define CK_XONXOFF // Everybody can do this, right?

#ifdef BSD44 // And in 4.4 BSD, including BSDI
#define CK_RTSCTS
#endif // BSD44

#ifdef TERMIOX // Sys V R4 <termiox.h>
#ifndef CK_RTSCTS
#define CK_RTSCTS
#endif // CK_RTSCTS
#ifndef CK_DTRCD
#define CK_DTRCD
#endif // CK_DTRCD
#else
#ifdef STERMIOX // Sys V R4 <sys/termiox.h>
#ifndef CK_RTSCTS
#define CK_RTSCTS
#endif // CK_RTSCTS
#ifndef CK_DTRCD
#define CK_DTRCD
#endif // CK_DTRCD
#endif // STERMIOX
#endif // TERMIOX

#ifdef __linux__ // Linux
#define CK_RTSCTS
#endif // __linux__
// Hardware flow control is not defined in POSIX.1.  Nevertheless, a certain
// style API for hardware flow control, using tcsetattr() and the CRTSCTS
// bit(s), seems to be gaining currency on POSIX-based UNIX systems.  The
// following code defines the symbol POSIX_CRTSCTS for such systems.
#ifdef CK_RTSCTS
#ifdef __linux__ // Linux
#define POSIX_CRTSCTS
#endif            // __linux__
#ifdef __NetBSD__ // NetBSD
#define POSIX_CRTSCTS
#endif // __NetBSD__
#ifdef __OpenBSD__
#define POSIX_CRTSCTS
#endif // __OpenBSD__
#endif // CK_RTSCTS

// Implementations that have implemented the ttsetflow() function.

#ifndef CK_TTSETFLOW
#ifdef UNIX
#define CK_TTSETFLOW
#endif // UNIX
#endif // CK_TTSETFLOW

#ifdef CK_TTSETFLOW
int ttsetflow(int);
#endif // CK_TTSETFLOW
// Systems where we can expand tilde at the beginning of file or directory names
#ifdef POSIX
#ifndef DTILDE
#define DTILDE
#endif // DTILDE
#endif // POSIX
#ifdef BSD4
#ifndef DTILDE
#define DTILDE
#endif // DTILDE
#endif // BSD4
#ifdef ATTSV
#ifndef DTILDE
#define DTILDE
#endif // DTILDE
#endif // ATTSV

// This is mainly for the benefit of ckufio.c (UNIX and OS/2 file support).
// Systems that have an atomic rename() function, so we don't have to use
// link() and unlink().
#ifdef POSIX
#ifndef RENAME
#define RENAME
#endif // RENAME
#endif // POSIX

#ifdef SVR4
#ifndef RENAME
#define RENAME
#endif // RENAME
#endif // SVR4

#ifdef BSD44
#ifndef RENAME
#define RENAME
#endif // RENAME
#endif // BSD44

#ifdef NORENAME // Allow for compile-time override
#ifdef RENAME
#undef RENAME
#endif // RENAME
#endif // NORENAME

// Line delimiter for text files

// If the system uses a single character for text file line delimitation,
// define NLCHAR to the value of that character.  For text files, that
// character will be converted to CRLF upon output, and CRLF will be converted
// to that character on input during text-mode (default) packet operations.
#define NLCHAR 012

// At this point, if there's a system that uses ordinary CRLF line
// delimitation AND the C compiler actually returns both the CR and
// the LF when doing input from a file, then #undef NLCHAR.

// VMS file formats are so complicated we need to do all the conversion
// work in the CKVFIO module, so we tell the rest of C-Kermit not to fiddle
// with the bytes.

#ifdef vms
#undef NLCHAR
#endif // vms

// The device name of a job's controlling terminal
// Special for VMS, same for all Unixes (?), not used by Macintosh

#ifdef vms
#define CTTNAM "SYS$INPUT:" // (4 Jan 2002) Was TT:
#else
#ifdef UNIX
#define CTTNAM "/dev/tty"
#else
#define CTTNAM "stdout" // This is a kludge used by Mac
#endif                  // UNIX
#endif                  // vms

#ifndef HAVECTTNAM
#ifdef UNIX
#define HAVECTTNAM
#else
#endif // UNIX
#endif // HAVECTTNAM

#ifndef ZFCDAT // zfcdat() function available?
#ifdef UNIX
#define ZFCDAT
#else
#endif // UNIX
#endif // ZFCDAT

// Error number

#ifdef __GLIBC__
// "glibc uses threads, kermit uses glibc; errno access is in Thread Local
// Storage (TLS) from glibc-3.2.2.  ...a thread specific errno is being run in
// thread local storage relative to the %gs segment register, so some means to
// revector gets/puts needs to be done." - Jeff Johnson, Red Hat, Feb 2003.
#include <errno.h>
#else
// It is assumed that if the foregoing code doesn't explicitly include errno.h,
// that it gets included anyway by some other header file that *is* included.
// If there is still some platform where the build fails because errno is not
// defined, add -DDCL_ERRNO to the Cflags for that makefile target.  Also
// see the new first stanza of the "linux" makefile target for code that
// that checks for this at 'make' time and adds DCL_ERRNO only if necessary.
// WARNING: this might break if errno.h does not exist or is not in the
// the default directory for header files.
// - fdc, 7-8 October 2020
#ifdef DCL_ERRNO
extern int errno;
#else
#include <errno.h>
#endif // DCL_ERRNO
#endif // __GLIBC__

#ifndef ESRCH      // access to error mnemonics
#include <errno.h> // in all modules - 2007/08/25
#endif             // ESRCH
                   // 2024-06-07 SMS.  Added VMSOR.

#ifndef NOBIGBUF
#ifndef BIGBUFOK // Platforms with lots of memory

#ifdef BSD44
#define BIGBUFOK
#endif // BSD44

#ifdef sparc // SPARC processors
#define BIGBUFOK
#else
#endif // sparc

#ifdef LINUX // Linux from 1998 on should be OK
#ifndef BIGBUFOK
#define BIGBUFOK
#endif // BIGBUFOK
#endif // LINUX

#ifdef __alpha   // DEC 64-bit Alpha, e.g. OSF/1
#ifndef BIGBUFOK // Might already be defined for VMS
#define BIGBUFOK
#endif // BIGBUFOK
#endif // __alpha

#endif // BIGBUFOK
#endif // NOBIGBUF

#ifdef CK_SMALL
#ifdef BIGBUFOK
#undef BIGBUFOK
#endif // BIGBUFOK
#endif // CK_SMALL

// If "memory is no problem" then this improves performance

#ifdef DEBUG
#ifdef BIGBUFOK
#ifndef IFDEBUG
#define IFDEBUG
#endif // IFDEBUG
#endif // BIGBUFOK
#endif // DEBUG

// File System Defaults

#ifndef UIDBUFLEN // Length of User ID
#ifdef BIGBUFOK
#define UIDBUFLEN 256
#else
#define UIDBUFLEN 64
#endif // BIGBUFOK
#endif // UIDBUFLEN

#ifdef UNIX
#ifdef BIGBUFOK
#define MAXWLD 102400
#else
#define MAXWLD 1024
#endif // BIGBUFOK
#else
#endif // UNIX

#define DBLKSIZ 0
#define DLRECL 0

// Communication device / network host name length

#ifdef BIGBUFOK
#define TTNAMLEN 512
#else
#ifndef CK_SMALL
#define TTNAMLEN 128
#else
#define TTNAMLEN 80
#endif // CK_SMALL
#endif // BIGBUFOK

// Program return codes for DECUS C and UNIX (VMS uses UNIX codes)

#ifdef decus
#define GOOD_EXIT IO_NORMAL
#define BAD_EXIT IO_ERROR
#else
#define GOOD_EXIT 0
#define BAD_EXIT 1
#endif // decus

// Special hack for Fortune, which doesn't have <sys/file.h>...

// Special hack for OS-9/68k

// Escape/quote character used by the command parser

#define CMDQ '\\'

// Symbols for RS-232 modem signals

#define KM_FG 1   // Frame ground
#define KM_TXD 2  // Transmit
#define KM_RXD 3  // Receive
#define KM_RTS 4  // Request to Send
#define KM_CTS 5  // Clear to Send
#define KM_DSR 6  // Data Set Ready
#define KM_SG 7   // Signal ground
#define KM_DCD 8  // Carrier Detect
#define KM_DTR 20 // Data Terminal Ready
#define KM_RI 22  // Ring Indication

// Bit mask values for modem signals

#define BM_CTS 0001 // Clear to send       (From DCE)
#define BM_DSR 0002 // Dataset ready       (From DCE)
#define BM_DCD 0004 // Carrier             (From DCE)
#define BM_RNG 0010 // Ring Indicator      (From DCE)
#define BM_DTR 0020 // Data Terminal Ready (From DTE)
#define BM_RTS 0040 // Request to Send     (From DTE)

// Codes for full duplex flow control

#define FLO_NONE 0  // None
#define FLO_XONX 1  // Xon/Xoff (soft)
#define FLO_RTSC 2  // RTS/CTS (hard)
#define FLO_DTRC 3  // DTR/CD (hard)
#define FLO_ETXA 4  // ETX/ACK (soft)
#define FLO_STRG 5  // String-based (soft)
#define FLO_DIAL 6  // DIALing kludge
#define FLO_DIAX 7  // Cancel dialing kludge
#define FLO_DTRT 8  // DTR/CTS (hard)
#define FLO_KEEP 9  // Keep, i.e. don't touch or change
#define FLO_AUTO 10 // Figure out automatically

// Types of connections

#define CXT_REMOTE 0  // Remote mode - no connection
#define CXT_DIRECT 1  // Direct serial connection
#define CXT_MODEM 2   // Modem dialout
#define CXT_TCPIP 3   // TCP/IP - Telnet, Rlogin, etc
#define CXT_X25 4     // X.25 peer-to-peer
#define CXT_DECNET 5  // DECnet (CTERM, etc)
#define CXT_LAT 6     // LAT
#define CXT_NETBIOS 7 // NETBIOS
#define CXT_NPIPE 8   // Named Pipe
#define CXT_PIPE 9    // Pipe, Command, PTY, DLL, etc
#define CXT_SSH 10    // SSH
#define CXT_MAX 10    // Highest connection type

// Autodownload Detection Options

#define ADL_PACK 0 // Auto-Download detect packet
#define ADL_STR 1  // Auto-Download detect string

// And finally...

// zstr zattr filinfo were here (moved to top for DECC 5 Jun 2000)

#ifndef ZFNQFP // Versions that have zfnqfp()
#ifdef UNIX
#define ZFNQFP
#else
#endif // UNIX
struct zfnfp {
  int len;     // Length of full pathname
  char *fpath; // Pointer to full pathname
  char *fname; // Pointer to name part
};
#endif // ZFNQFP

// Systems that support FILE TYPE LABELED

// Systems that support builtin variable "exedir", use getexedir() function

#ifdef UNIX
#define GETEXEDIR
#define HAVE_VN_EXEDIR
#else  // def UNIX
#endif // def UNIX [else]

// LABELED FILE options bitmask

// Data types.  First the header file for data types so we can pick up the
// types used for pids, uids, and gids.  Override this section by putting
// -DCKTYP_H=xxx on the command line to specify the header file where your
// system defines these types.
#ifdef __ALPHA
#endif // __ALPHA

#ifndef CKTYP_H
#define CKTYP_H <sys/types.h>
#endif // CKTYP_H

#ifdef CKTYP_H // Include it.
#include CKTYP_H
#endif // CKTYP_H

// File lengths and offsets.  This section is expected to grow as we
// support long files on 32-bit platforms.  We want this data type to be
// signed because so many functions return either a file size or a negative
// value to indicate an error.
#ifndef CK_OFF_T
#endif // CK_OFF_T

// FreeBSD and OpenBSD set off_t to the appropriate size unconditionally

#ifndef CK_OFF_T
#ifdef __FreeBSD__
#define CK_OFF_T off_t
#else
#ifdef __OpenBSD__
#define CK_OFF_T off_t
#endif // __OpenBSD__
#endif // __FreeBSD__
#endif // CK_OFF_T

// 32-bit platforms that support long files thru "transitional interface"
// These include Linux, Solaris, NetBSD...

#ifdef _LARGEFILE_SOURCE
#ifndef CK_OFF_T
#define CK_OFF_T off_t
#endif // CK_OFF_T
#define CKFSEEK(a, b, c) fseeko(a, b, c)
#define CKFTELL(a) ftello(a)
#else // Not  _LARGEFILE_SOURCE
#define CKFSEEK(a, b, c) fseek(a, b, c)
#define CKFTELL(a) ftell(a)
// See below the next section for the catch-all case
#endif // _LARGEFILE_SOURCE

// 32-bit or 64-bit platforms

// CK_64BIT is a compile-time symbol indicating a true 64-bit build
// meaning that longs and pointers are 64 bits

#ifndef CK_64BIT
#if defined(_LP64) // Solaris
#define CK_64BIT
#elif defined(__LP64__) // MacOS X 10.4 (or _LP64,__ppc64__)
#define CK_64BIT
#elif defined(__arch64__) // gcc alpha, sparc
#define CK_64BIT
#elif defined(__alpha) // Alpha decc (or __ALPHA)
#define CK_64BIT
#elif defined(__amd64) // AMD x86_64
#define CK_64BIT
#elif defined(__x86_64) // AMD/Intel x86_64
#define CK_64BIT
#elif defined(__ia64) // Intel IA64
#define CK_64BIT
#endif // _LP64 / __LP64__ / __arch64__ / __alpha / __amd64 / __x86_64 /       \
       // __ia64
#endif // CK_64BIT

#ifndef CK_OFF_T
#ifdef CK_64BIT
#define CK_OFF_T off_t // This has to be signed
#else                  // CK_64BIT
#define CK_OFF_T long  // Signed
#endif                 // CK_64BIT
#endif                 // CK_OFF_T

#ifndef TLOG
#define tlog(a, b, c, d)
#else
#ifndef CKCMAI
// Debugging included.  Declare debug log flag in main program only.
extern int tralog, tlogfmt;
#endif // CKCMAI
void dotlog(int, char *, char *, CK_OFF_T);
#define tlog(a, b, c, d)                                                       \
  if (tralog && tlogfmt)                                                       \
  dotlog(a, b, c, (CK_OFF_T)d)
void doxlog(int, char *, CK_OFF_T, int, int, char *);
#endif // TLOG

#ifndef DEBUG
// Compile all the debug() statements away.  Saves a lot of space and time.
#define debug(a, b, c, d)
#define ckhexdump(a, b, c)
// Now define the debug() macro.
#else // DEBUG
int dodebug(int, char *, char *, CK_OFF_T);
int dohexdump(CHAR *, CHAR *, int);
#ifdef IFDEBUG
// Use this form to avoid function calls:
#define debug(a, b, c, d)                                                      \
  ((void)(deblog ? dodebug(a, b, (char *)(c), (CK_OFF_T)(d)) : 0))
#define ckhexdump(a, b, c)                                                     \
  ((void)(deblog ? dohexdump((CHAR *)(a), (CHAR *)(b), c) : 0))
#else // IFDEBUG
// Use this form to save space:
#define debug(a, b, c, d) dodebug(a, b, (char *)(c), (CK_OFF_T)(d))
#define ckhexdump(a, b, c) dohexdump((CHAR *)(a), (CHAR *)(b), c)
#endif // IFDEBUG
#endif // DEBUG

// Structure definitions for Kermit file attributes
// All strings come as pointer and length combinations
// Empty string (or for numeric variables, -1) = unused attribute.

struct zstr { // string format
  int len;    // length
  char *val;  // value
};

struct zattr {          // Kermit File Attribute structure
  CK_OFF_T lengthk;     // (!) file length in K
  struct zstr type;     // (") file type (text or binary)
  struct zstr date;     // (#) file creation date yyyymmdd[ hh:mm[:ss]]
  struct zstr creator;  // ($) file creator id
  struct zstr account;  // (%) file account
  struct zstr area;     // (&) area (e.g. directory) for file
  struct zstr password; // (') password for area
  long blksize;         // (() file blocksize
  struct zstr xaccess;  // ()) file access: new, supersede, append, warn
  struct zstr encoding; // (*) encoding (transfer syntax)
  struct zstr disp;     // (+) disposition (mail, message, print, etc)
  struct zstr lprotect; // (,) protection (local syntax)
  struct zstr gprotect; // (-) protection (generic syntax)
  struct zstr systemid; // (.) ID for system of origin
  struct zstr recfm;    // (/) record format
  struct zstr sysparam; // (0) system-dependent parameter string
  CK_OFF_T length;      // (1) exact length on system of origin
  struct zstr charset;  // (2) transfer syntax character set
  struct zstr reply;    // This goes last, used for attribute reply
};

// Kermit file information structure

struct filinfo {
  int bs;            // Blocksize
  int cs;            // Character set
  long rl;           // Record length
  int org;           // Organization
  int fmt;           // Record format
  int cc;            // Carriage control
  int typ;           // Type (text/binary)
  int dsp;           // Disposition
  char *os_specific; // OS-specific attributes
  int lblopts;
};

// Data type for pids.  If your system uses a different type, put something
// like -DPID_T=pid_t on command line, or override here.
#ifndef PID_T
#define PID_T int
#endif // PID_T
// Data types for uids and gids.  Same deal as for pids.
// Wouldn't be nice if there was a preprocessor test to find out if a
// typedef existed?

#ifdef POSIX
// Or would it be better (or worse?) to use _POSIX_SOURCE here?
#ifndef UID_T
#define UID_T uid_t
#endif // UID_T
#ifndef GID_T
#define GID_T gid_t
#endif // GID_T
#else  // Not POSIX
#ifdef SVR4
// SVR4 and later have uid_t and gid_t.
// SVR3 and earlier use int, or unsigned short, or....
#ifndef UID_T
#define UID_T uid_t
#endif // UID_T
#ifndef GID_T
#define GID_T gid_t
#endif // GID_T
#else  // Not SVR4
// Default these to int for older UNIX versions
#ifndef UID_T
#define UID_T int
#endif // UID_T
#ifndef GID_T
#define GID_T int
#endif // GID_T
#endif // SVR4
#endif // POSIX

// getpwuid() arg type, which is not necessarily the same as UID_T,
// e.g. in SCO UNIX SVR3, it's int.
#ifndef PWID_T
#define PWID_T UID_T
#endif // PWID_T

#ifdef CK_REDIR
#ifdef MACH
#define MACHWAIT
#endif // MACH

#ifdef MACHWAIT // WAIT_T argument for wait()
#include <sys/wait.h>
#define CK_WAIT_H
typedef union wait WAIT_T;
#else
#ifdef POSIX
#include <sys/wait.h>
#define CK_WAIT_H
#ifndef WAIT_T
typedef int WAIT_T;
#endif // WAIT_T
#else  // !POSIX
typedef int WAIT_T;
#endif // POSIX
#endif // MACHWAIT
#else
typedef int WAIT_T;
#endif // CK_REDIR

// Assorted other blah_t's handled here...

#ifndef SIZE_T
#define SIZE_T size_t
#endif // SIZE_T

// Forward declarations of system-dependent functions callable from all
// C-Kermit modules.

// File-related functions from system-dependent file i/o module

#ifndef CKVFIO_C
// For some reason, this does not agree with DEC C
int zkself(void);
#endif // CKVFIO_C
int zopeni(int, char *);
int zopeno(int, char *, struct zattr *, struct filinfo *);
int zclose(int);
int zchin(int, int *);
int zxin(int, char *, int);
int zsinl(int, char *, int);
int zinfill(void);
int zsout(int, char *);
int zsoutl(int, char *);
int zsoutx(int, char *, int);
int zchout(int, char);
int zoutdump(void);
int zsyscmd(char *);
int zshcmd(char *);
#ifdef UNIX
int zsetfil(int, int);
#endif // UNIX
int zchkpid(unsigned long);
#ifdef CKEXEC
void z_exec(char *, char **, int);
#endif // CKEXEC
int chkfn(int);
CK_OFF_T zchki(char *);
CK_OFF_T zgetfs(char *);
int iswild(char *);
int isdir(char *);
int zchko(char *);
int zdelet(char *);
void zrtol(char *, char *);
void zltor(char *, char *);
void zstrip(char *, char **);
int zchdir(char *);
char *zhome(void);
char *zgtdir(void);
int zxcmd(int, char *);
int zclosf(int);
#ifdef NZXPAND
int nzxpand(char *, int);
#else  // NZXPAND
int zxpand(char *);
#endif // NZXPAND
int znext(char *);
#ifdef ZXREWIND
int zxrewind(void);
#endif // ZXREWIND
int zchkspa(char *, CK_OFF_T);
void znewn(char *, char **);
int zrename(char *, char *);
int zcopy(char *, char *);
int zsattr(struct zattr *);
int zfree(char *);
char *zfcdat(char *);
int zstime(char *, struct zattr *, int);
#ifdef CK_PERMS
char *zgperm(char *);
char *ziperm(char *);
#else  // CK_PERMS
#endif // CK_PERMS
int zmail(char *, char *);
int zprint(char *, char *);
char *tilde_expand(char *);
int zmkdir(char *);
int zfseek(CK_OFF_T);
#ifdef ZFNQFP
struct zfnfp *zfnqfp(char *, int, char *);
#else
#define zfnqfp(a, b, c) ckstrncpy(c, a, b)
#endif // ZFNQFP
int zvuser(char *);
int zvpass(char *);
void zvlogout(void);

// Functions from system-dependent terminal i/o module

int ttopen(char *, int *, int, int); // tty functions
int ttclos(int);
int tthang(void);
int ttres(void);
int ttpkt(long, int, int);
int ttvt(long, int);
int ttsspd(int);
long ttgspd(void);
int ttflui(void);
int ttfluo(void);
int ttpushback(CHAR *, int);
int ttpeek(void);
int ttgwsiz(void);
int ttchk(void);
int ttxin(int, CHAR *);
int ttxout(CHAR *, int);
int ttol(CHAR *, int);
int ttoc(char);
int ttinc(int);
int ttscarr(int);
int ttgmdm(void);
int ttsndb(void);
int ttsndlb(void);
#ifdef UNIX
char *ttglckdir(void);
#endif // UNIX
#ifdef PARSENSE
#ifdef UNIX
int ttinl(CHAR *, int, int, CHAR, CHAR, int);
#else
int ttinl(CHAR *, int, int, CHAR, CHAR);
#endif // UNIX
#else  // ! PARSENSE
int ttinl(CHAR *, int, int, CHAR);
#endif // PARSENSE

// XYZMODEM support

// XMODEM/YMODEM/ZMODEM support removed 2026-07-10: NOCKXYZ is now always
// defined, so CK_XYZ (which enabled the external-protocol commands and
// data structures) is never defined, and the XYZ_INTERNAL / XYZ_DLL
// variants (built-in / loadable protocol engines, never enabled by any
// makefile target in this tree) are gone with it.
// See doc/NOCKXYZ-20260709.md.

#ifndef NOCKXYZ
#define NOCKXYZ
#endif // NOCKXYZ

// Console functions

int congm(void);
void conint(ck_sig_t, ck_sig_t);
void connoi(void);
int concb(char);
#ifdef CONGSPD
long congspd(void);
#endif // CONGSPD
int conbin(char);
int conres(void);
int conoc(char);
int conxo(int, char *);
int conol(char *);
int conola(char *[]);
int conoll(char *);
int conchk(void);
int coninc(int);
char *conkbg(void);
int psuspend(int);
int priv_ini(void);
int priv_on(void);
int priv_off(void);
int priv_can(void);
int priv_chk(void);
int priv_opn(char *, int);

int sysinit(void); // Misc Kermit functions
int syscleanup(void);
int msleep(int);
void rtimer(void);
int gtimer(void);
#ifdef GFTIMER
void rftimer(void);
CKFLOAT gftimer(void);
#endif // GFTIMER
void ttimoff(void);
void ztime(char **);
int parchk(CHAR *, CHAR, int);
void doexit(int, int);
int askmore(void);
void fatal(char *);
void fatal2(char *, char *);

// Key mapping support

#ifdef NOICP
#ifndef NOSETKEY
#define NOSETKEY
#endif // NOSETKEY
#endif // NOICP

int congks(int);
#ifndef NOSETKEY
// Catch-all for systems where we don't know how to read keyboard scan
// codes > 255.
#define KMSIZE 256
// Note: CHAR (i.e. unsigned char) is very important here.
typedef CHAR KEY;
typedef CHAR *MACRO;
#define congks coninc
#endif // NOSETKEY

#ifndef NOKVERBS // No \Kverbs unless...
#define NOKVERBS
#endif // NOKVERBS

#ifndef NOKVERBS
#endif // NOKVERBS

#define F_ESC 0x8000 // Bit indicating ESC char combination
#define IS_ESC(x) (x & F_ESC)
#define F_CSI 0x10000 // Bit indicating CSI char combination
#define IS_CSI(x) (x & F_CSI)

#ifdef NOSPL     // This might be overkill..
#ifndef NOKVERBS // Not all \Kverbs require
#define NOKVERBS // the script programming language.
#endif           // NOKVERBS
#ifndef NOTAKEARGS
#define NOTAKEARGS
#endif // NOTAKEARGS
#endif // NOSPL

// Function prototypes for system and library functions.
#ifdef _POSIX_SOURCE
#define CK_ANSILIBS
#endif // _POSIX_SOURCE

#ifdef SVR4
#define CK_ANSILIBS
#endif // SVR4

// Fullscreen file transfer display items...

#ifndef NOCURSES
#ifdef CK_NCURSES // CK_NCURSES implies CK_CURSES
#ifndef CK_CURSES
#define CK_CURSES
#endif // CK_CURSES
#endif // CK_NCURSES

#ifdef MYCURSES // MYCURSES implies CK_CURSES
#ifndef CK_CURSES
#define CK_CURSES
#endif // CK_CURSES
#endif // MYCURSES
#endif // NOCURSES

#ifdef NOCURSES
#ifdef CK_CURSES
#undef CK_CURSES
#endif // CK_CURSES
#ifndef NODISPLAY
#define NODISPLAY
#endif // NODISPLAY
#endif // NOCURSES

#ifdef CK_CURSES
// The CK_WREFRESH symbol is defined if the curses library provides
// clearok() and wrefresh() functions, which are used in repainting
// the screen.
#ifdef NOWREFRESH // Override CK_WREFRESH

#ifdef CK_WREFRESH // If this is defined,
#undef CK_WREFRESH // undefine it.
#endif             // CK_WREFRESH

#else // !NOWREFRESH
// No override...

#ifndef CK_WREFRESH // If CK_WREFRESH not defined
// Automatically define it for systems known to have it ...
#ifdef SVR3         // System V has it
#define CK_WREFRESH
#else
#ifdef BSD44 // 4.4 BSD has it
#define CK_WREFRESH
#else
#endif // BSD44
#endif // SVR3

#else // CK_WREFRESH is defined

// This is within an ifdef CK_CURSES block.  The following is not needed

#ifndef CK_CURSES // CK_WREFRESH implies CK_CURSES
#define CK_CURSES
#endif // CK_CURSES

#endif // CK_WREFRESH
#endif // NOWREFRESH

#ifndef TRMBUFL
#ifdef BIGBUFOK
#define TRMBUFL 16384
#else
#ifdef DYNAMIC
#define TRMBUFL 8192
#else
#define TRMBUFL 1024
#endif // BIGBUFOK
#endif // DYNAMIC
#endif // TRMBUFL
#endif // CK_CURSES

// Whether to use ckmatch() in all its glory for C-Shell-like patterns.
// If CKREGEX is NOT defined, all but * and ? matching are removed from
// ckmatch().  NOTE: Defining CKREGEX does not necessarily mean that ckmatch()
// regexes are used for filename matching.  That depends on whether zxpand()
// in ck?fio.c calls ckmatch().  NOTE 2: REGEX is a misnomer -- these are not
// regular expressions in the computer-science sense (in which, e.g. "a*b"
// matches 0 or more 'a' characters followed by 'b') but patterns (in which
// "a*b" matches 'a' followed by 0 or more non-b characters, followed by b).
#ifndef NOCKREGEX
#ifndef CKREGEX
#define CKREGEX
#endif // CKREGEX
#endif // NOCKREGEX

// Learned-script feature

#ifndef NOLEARN
#ifdef NOSPL
#define NOLEARN
#else
#ifdef NOLOCAL
#define NOLEARN
#endif // NOLOCAL
#endif // NOSPL
#endif // NOLEARN

#ifdef NOLEARN
#ifdef CKLEARN
#undef CKLEARN
#endif // CKLEARN
#else  // !NOLEARN
#ifndef CKLEARN
// In UNIX this can work only with ckucns.c builds
#define CKLEARN
#endif // CKLEARN
#endif // NOLEARN

#ifdef CKLEARN
#ifndef LEARNBUFSIZ
#define LEARNBUFSIZ 128
#endif // LEARNBUFSIZ
#endif // CKLEARN

#ifndef IKSDONLY
#ifndef CKTIDLE // Pseudo-keepalive in CONNECT
#ifdef UNIX     // In UNIX but only ckucns versions
#ifndef NOLEARN
#ifndef NOSELECT
#define CKTIDLE
#endif // NOSELECT
#endif // NOLEARN
#endif // UNIX
#endif // CKTIDLE
#endif // IKSDONLY

#ifdef CK_ANSILIBS
// String library functions.
// For ANSI C, get prototypes from <string.h>.
// Otherwise, skip the prototypes.
#include <string.h>

// Prototypes for other commonly used library functions, such as
// malloc, free, getenv, atol, atoi, and exit.  Otherwise, no prototypes.
#include <stdlib.h>
#ifdef DIAB // DIAB DS90
// #include <commonC.h>
#include <sys/wait.h>
#define CK_WAIT_H
extern int chmod(char *path, int mode);
extern int ioctl(int fildes, int request, ...);
extern int rdchk(int ttyfd);
extern int nap(int m);
extern int _filbuf(FILE *stream);
extern int _flsbuf(char c, FILE *stream);
#endif // DIAB

// Prototypes for UNIX functions like access, alarm, chdir, sleep, fork,
// and pause.  Otherwise, no prototypes.

#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif // HAVE_CRYPT_H

#else  // Not ANSI libs...

// It is essential that these are declared correctly!
// Which is not always easy.  Take malloc() for instance ...
// NOTE: there were a bunch of protypes here for malloc() here
// before but why???  The specs come from the header files.
#endif // CK_ANSILIBS
// <sys/param.h> generally picks up NULL, MAXPATHLEN, and MAXNAMLEN
// and seems to present on all Unixes going back at least to SCO Xenix
// with the exception(s) noted.
#ifndef NO_PARAM_H // 2001-11-03
#ifndef UNIX       // Non-Unixes don't have it
#define NO_PARAM_H
#else
#endif // UNIX
#endif // NO_PARAM_H

#ifndef NO_PARAM_H
#ifndef INCL_PARAM_H
#define INCL_PARAM_H
#endif // INCL_PARAM_H
#include <sys/param.h>
#endif // NO_PARAM_H

#ifndef NULL // In case NULL is still not defined
#define NULL 0L
// or #define NULL 0
// or #define NULL ((char *) 0)
// or #define NULL ((void *) 0)
#endif // NULL

// Macro to differentiate "" from NULL (to avoid comparisons with literals)

#ifndef isemptystring
#define isemptystring(s) ((s ? (*s ? 0 : 1) : 0))
#endif // isemptystring

// Maximum length for a fully qualified filename, not counting \0 at end.
// This is a rough cut, and errs on the side of being too big.  We don't
// want to pull in hundreds of header files looking for many and varied
// symbols, for fear of introducing unnecessary conflicts.
#ifndef CKMAXPATH
#ifdef MAXPATHLEN // (it probably isn't)
#define CKMAXPATH MAXPATHLEN
#else
#ifdef PATH_MAX // POSIX
#define CKMAXPATH PATH_MAX
#else       // def PATH_MAX
#ifdef UNIX // Even though some are way less...
#define CKMAXPATH 1024
#else // def UNIX
#define CKMAXPATH 255
#endif // def UNIX [else]
#endif // def PATH_MAX [else]
#endif // def MAXPATHLEN [else]
#endif // ndef CKMAXPATH

// 2021-10-30 SMS (Steven M Schweda)
// MAXPATHLEN might have come up undefined because all the consumers
// should be using CKMAXPATH instead of MAXPATHLEN, or PATH_MAX, or
// whatever.  The idea was to put all the system dependencies into the
// definition of CKMAXPATH.  Previously, MAXPATHLEN was not used in
// ckuus4.c or ckuus6.c,
//
// Apparently, I failed to make the required change(s) in UNIX
// modules/sections ckufio.c, ckuus3.c, ckuus5.c.  (But _I_ didn't spell
// "#define" as "def".)
//
// On VMS, PATH_MAX is defined as 256 in <limits.h>, but that is an
// obsolete value, which is why NAMX_C_MAXRSS is used instead.

// Maximum length for a simple filename, not counting \0 at end.
// Define maximum length for a file name if not already defined.
// NOTE: This applies to a path segment (directory or file name),
// not the entire path string, which can be CKMAXPATH bytes long.

// On VMS, this is ill-defined, and depends on the file system:
// ODS2: 39.39 + version (;32767), so 84.
// ODS5: 238 + version (;32767), so 233.
#ifndef CKMAXNAM
// Non-VMS definitions moved here from ckufio.c. with MAXNAMLEN -> CKMAXNAM.

#ifndef CKMAXNAM // If MAXNAMLEN is defined, then use that.
#ifdef MAXNAMLEN
#define CKMAXNAM MAXNAMLEN
#endif // def MAXNAMLEN
#endif // ndef CKMAXNAM

#ifndef CKMAXNAM
#ifdef FILENAME_MAX
#define CKMAXNAM FILENAME_MAX
#else
#ifdef NAME_MAX
#define CKMAXNAM NAME_MAX
#else
#ifdef _POSIX_NAME_MAX
#define CKMAXNAM _POSIX_NAME_MAX
#else
#ifdef _D_NAME_MAX
#define CKMAXNAM _D_NAME_MAX
#else
#ifdef DIRSIZ
#define CKMAXNAM DIRSIZ
#else
#define CKMAXNAM 14
#endif // DIRSIZ
#endif // _D_NAME_MAX
#endif // _POSIX_NAME_MAX
#endif // _POSIX_NAME_MAX
#endif // NAME_MAX
#endif // sun

#endif // def VMS [else]

// Maximum length for the name of a tty device
#ifndef DEVNAMLEN
#define DEVNAMLEN CKMAXPATH
#endif // DEVNAMLEN

// Directory (path segment) separator
// Not fully general - Tricky for VMS, Amiga, ...

#ifndef DIRSEP
#ifdef UNIX
#define DIRSEP '/'
#define STRDIRSEP "/"
#define ISDIRSEP(c) ((c) == '/')
#else
#define DIRSEP '/'
#define STRDIRSEP "/"
#define ISDIRSEP(c) ((c) == '/')
#endif // UNIX
#endif // DIRSEP

// FILE package parameters

#ifndef CKMAXOPEN
#ifdef OPEN_MAX
#define CKMAXOPEN OPEN_MAX
#else
#ifdef FOPEN_MAX
#define CKMAXOPEN FOPEN_MAX
#else
#define CKMAXOPEN 64
#endif // FOPEN_MAX
#endif // OPEN_MAX
#endif // CKMAXOPEN

// Maximum channels for FOPEN = CKMAXOPEN minus logs, stdio, etc

#ifndef Z_MINCHAN
#define Z_MINCHAN 16
#endif // Z_MINCHAN

#ifndef Z_MAXCHAN
#define Z_MAXCHAN (CKMAXOPEN - ZNFILS - 5)
#endif // Z_MAXCHAN

// New-format nzltor() and nzrtol() functions that handle pathnames

#ifndef NZLTOR
#ifdef UNIX
#define NZLTOR
#else
#endif // UNIX
#endif // NZLTOR

#ifdef NZLTOR
void nzltor(char *, char *, int, int, int);
void nzrtol(char *, char *, int, int, int);
#endif // NZLTOR

// Implementations with a zrmdir() function

#ifndef ZRMDIR
#ifdef UNIX
#define ZRMDIR
#else
#endif // UNIX
#endif // ZRMDIR

#ifdef ZRMDIR
int zrmdir(char *);
#endif // ZRMDIR

#ifndef FILECASE
#define FILECASE 1
#ifndef CKCMAI
extern int filecase;
#endif // CKCMAI
#endif // FILECASE

// Funny names for library functions department...

#define isWin95() (0)

#ifndef BPRINT
#endif // BPRINT

#ifndef SESLIMIT
#endif // SESLIMIT

#ifndef NOTERM
#ifndef PCTERM
#endif // PCTERM
#endif // NOTERM

#ifndef PTYORPIPE // NETCMD and/or NETPTY defined
#ifdef NETCMD
#define PTYORPIPE
#else
#ifdef NETPTY
#define PTYORPIPE
#endif // NETPTY
#endif // NETCMD
#endif // PTYORPIPE

// mktemp() and mkstemp()

#ifndef NOMKTEMP
#ifndef MKTEMP
#define MKTEMP
#endif // MKTEMP

#ifdef MKTEMP
#ifndef NOMKSTEMP
#ifndef MKSTEMP
#ifdef BSD44
#define MKSTEMP
#else
#ifdef __linux__
#define MKSTEMP
#endif // __linux__
#endif // BSD44
#endif // MKSTEMP
#endif // NOMKSTEMP
#endif // MKTEMP
#endif // NOMKTEMP

// Platforms that have memcpy() -- only after all headers included

#ifndef USE_MEMCPY
#ifdef __linux__
#define USE_MEMCPY
#else
#ifdef POSIX
#define USE_MEMCPY
#else
#ifdef SVR4
#define USE_MEMCPY
#else
#endif // SVR4
#endif // POSIX
#endif // __linux__
#endif // USE_MEMCPY

#ifndef USE_MEMCPY
#define memcpy(a, b, c) ckmemcpy((a), (b), (c))
#else
#endif // USE_MEMCPY

// User authentication for IKS -- So far K95 and UNIX only

#ifdef NOICP
#ifndef NOLOGIN
#define NOLOGIN
#endif // NOLOGIN
#endif // NOICP

#ifndef NOLOGIN
#ifndef CK_LOGIN
#define CK_LOGIN
#ifndef NOSHADOW
#endif // NOSHADOW
#endif // CK_LOGIN
#else  // NOLOGIN
#ifdef CK_LOGIN
#undef CK_LOGIN
#endif // CK_LOGIN
#endif // NOLOGIN

#ifdef CK_LOGIN // Telnet protocol required
#ifndef TNCODE  // for login to IKSD.
#define TNCODE
#endif // TNCODE
#endif // CK_LOGIN

#ifdef TNCODE // Should TELNET send user ID?
#ifndef NOSENDUID
#ifndef CKSENDUID
#define CKSENDUID
#endif // CKSENDUID
#endif // NOSENDUID
#endif // TNCODE

// UNIX platforms that don't have getusershell()

#ifdef UNIX
#ifndef NOGETUSERSHELL
#endif // NOGETUSERSHELL
#endif // UNIX

#ifdef CK_LOGIN
#ifdef UNIX
#ifndef NOSYSLOG
#ifndef CKSYSLOG
#define CKSYSLOG
#endif // CKSYSLOG
#endif // NOSYSLOG
#ifndef NOWTMP
#ifndef CKWTMP
#define CKWTMP
#endif // CKWTMP
#endif // NOWTMP
#ifndef NOGETUSERSHELL
#ifndef GETUSERSHELL
#define GETUSERSHELL
#endif // GETUSERSHELL
#endif // NOGETUSERSHELL
#endif // UNIX
int ckxlogin(CHAR *, CHAR *, CHAR *, int);
int ckxlogout(void);
#endif // CK_LOGIN

#ifndef NOZLOCALTIME // zlocaltime() available.
#define ZLOCALTIME
char *zlocaltime(char *);
#endif // NOZLOCALTIME

#ifdef CKSYSLOG           // Syslogging levels
#define SYSLG_NO 0        // No logging
#define SYSLG_LI 1        // Login/out
#define SYSLG_DI 2        // Dialing out
#define SYSLG_AC 3        // Making any kind of connection
#define SYSLG_PR 4        // Protocol Operations
#define SYSLG_FC 5        // File creation
#define SYSLG_FA 6        // File reading
#define SYSLG_CM 7        // Top-level commands
#define SYSLG_CX 8        // All commands
#define SYSLG_DB 9        // Debug
#define SYSLGMAX 9        // Highest level
#define SYSLG_DF SYSLG_FA // Default level
// Logging function
void cksyslog(int, int, char *, char *, char *);
#endif // CKSYSLOG
#ifndef CKCMAI
extern int ckxlogging, ckxsyslog, ikdbopen;
#endif // CKCMAI

#ifndef CK_KEYTAB
#define CK_KEYTAB
// Keyword Table Template

// Note: formerly defined in ckucmd.h but now more widely used

struct keytab { // Keyword table
  char *kwd;    // Pointer to keyword string
  int kwval;    // Associated value
  int flgs;     // Flags (as defined above)
};
#endif // CK_KEYTAB

#ifdef UNIX
int isalink(char *);
#endif // UNIX

#ifdef NETPTY
int do_pty(int *, char *, int);
void end_pty(void);
#endif // NETPTY

#ifdef CKROOT
int zsetroot(char *);
char *zgetroot(void);
int zinroot(char *);
#endif // CKROOT

// Local Echo Buffer prototypes
void le_init(void);
void le_clean(void);
int le_inbuf(void);
int le_putstr(CHAR *);
int le_puts(CHAR *, int);
int le_putchar(CHAR);
int le_getchar(CHAR *);

// #ifndef NOHTTP
#ifndef NOCMDATE2TM
#ifndef CMDATE2TM
#define CMDATE2TM
#endif // CMDATE2TM
#endif // NOCMDATE2TM

#ifndef NOLOCALE
#ifdef BSD44ORPOSIX
#ifndef NO_NL_LANGINFO
#ifndef HAVE_LOCALE
#define HAVE_LOCALE
#include <locale.h>
#endif // HAVE_LOCALE
#endif // NO_NL_LANGINFO
#endif // BSD44ORPOSIX
#endif // NOLOCALE

#ifdef CMDATE2TM
struct tm *cmdate2tm(char *, int);
#endif // CMDATE2TM
// #endif
// NOHTTP

#ifndef NOSETTIME  // This would be set in CFLAGS
#ifdef SVR4ORPOSIX // Defined in IEEE 1003.1-1996
#ifndef UTIMEH     // and in SVID for SVR4
#define UTIMEH
#endif // UTIMEH
#else  // SVR4ORPOSIX
#endif // SVR4ORPOSIX
#endif // NOSETTIME

// -DNOTCPIP = Build with no TCP/IP support,
// which unexpectedly turned out not to be a major task.
// - fdc May 2022
#ifdef NOTCPIP
#ifdef TCPSOCKET
#undef TCPSOCKET
#endif // TCPSOCKET
#ifdef SFTP
#undef SFTP
#endif // SFTP
#ifdef SFTP_BUILTIN
#undef SFTP_BUILTIN
#endif        // SFTP_BUILTIN
#ifdef TELNET // No Telnet
#undef TELNET
#endif // TELNET
#ifdef TNCODE
#undef TNCODE
#endif            // TNCODE
#ifdef TN_COMPORT // No Telnet terminal server
#undef TN_COMPORT
#endif         // TN_COMPORT
#ifndef NOHTTP // no HTTP...
#define NOHTTP
#endif            // NOHTTP
#ifndef NOBROWSER // no Web browser...
#define NOBROWSER
#endif      // NOBROWSER
#ifdef IKSD // No Internet Kermit Server
#undef IKSD
#endif // IKSD
#ifdef IKSDB
#undef IKSDB
#endif // IKSDB
#ifdef IKSDCONF
#undef IKSDCONF
#endif         // IKSDCONF
#ifndef NOIKSD // Internet Kermit Service
#define NOIKSD
#endif // NOIKSD
#ifdef IKS_OPTION
#undef IKS_OPTION
#endif // IKS_OPTION
#ifdef CK_LOGIN
#undef CK_LOGIN
#endif // CK_LOGIN
#ifdef CKSYSLOG
#undef CKSYSLOG
#endif // CKSYSLOG
#ifndef NOWTMP
#define NOWTMP
#endif // NOWTMP
#ifdef CKWTMP
#undef CKWTMP
#endif // CKWTMP
#ifndef NOSSL
#define NOSSL
#endif // NOSSL
#ifdef CK_SNDLOC
#undef CK_SNDLOC
#endif            // CK_SNDLOC
#ifdef SSHBUILTIN // No built-in SSH client
#undef SSHBUILTIN
#endif // SSHBUILTIN
#ifdef SSHTEST
#undef SSHTEST
#endif // SSHTEST
#else
#ifndef NETCONN
#define NETCONN
#endif // NETCONN
#endif // NOTCPIP

int readpass(char *, char *, int);
int readtext(char *, char *, int);

// On Windows, ttyfd is frequently used to hold HANDLEs which are a kind of
// pointer. So on ttyfd must be of a sufficient size to hold a pointer.
// Not on Windows or OS/2? its just an int
#define CK_TTYFD_T int

#include "ckclib.h"

// End of ckcdeb.h
#endif // CKCDEB_H
