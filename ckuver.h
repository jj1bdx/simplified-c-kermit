/* ckuver.h -- C-Kermit UNIX Version heralds */
/*
  Author: Frank da Cruz <fdc@kermitproject.edu>.

  Copyright (C) 1985, 2022,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#ifndef CKUVER_H
#define CKUVER_H

/* Arranged more or less alphabetically by compiler symbol */
/* Must be included AFTER ckcdeb.h. */

/* Removed obsolete herald strings as of June 2026 */
/*
  Nested #ifdef - #define - #else - #ifdef -... clauses
  renders clang-format simply unusable due to parsing workload
*/

#ifndef HERALD

#ifdef BSD44
#ifdef MACOSX
#define HERALD " macOS" /* Use a newer name */
#else
#ifdef __OpenBSD__
#define HERALD " OpenBSD"
#else
#ifdef __NetBSD__
#ifndef HERALD
#define HERALD " NetBSD"
#endif /* HERALD */
#else  /* __NetBSD__ */
#ifdef __FreeBSD__
#define HERALD " FreeBSD"
#else
#define HERALD " 4.4BSD"
#endif /* __FreeBSD__ */
#endif /* __NetBSD__ */
#endif /* __OpenBSD__ */
#endif /* MACOSX */
#endif /* BSD44 */

#ifdef POSIX
#ifdef HERALD
#undef HERALD
#endif /* HERALD */
#ifdef __linux__
#ifdef ZSL5500
#define HERALD " Sharp Zaurus SL-5500"
#else
#ifdef RH90
#define HERALD " Red Hat Linux 9.0"
#else
#ifdef RH80
#define HERALD " Red Hat Linux 8.0"
#else
#ifdef RH73
#define HERALD " Red Hat Linux 7.3"
#else
#ifdef RH72
#define HERALD " Red Hat Linux 7.2"
#else
#ifdef RH71
#define HERALD " Red Hat Linux 7.1"
#else
#define HERALD " Linux"
#endif /* RH71 */
#endif /* RH72 */
#endif /* RH73 */
#endif /* RH80 */
#endif /* RH90 */
#endif /* ZSL5500 */
#else  /* __linux__ */
#endif /* __linux__ */
#endif /* POSIX */

/* Catch-alls for anything not defined explicitly above */

#ifndef HERALD
#ifdef SVR4
#define HERALD " AT&T System V R4"
#else
#ifdef SVR3
#define HERALD " AT&T System V R3"
#else
#ifdef ATTSV
#define HERALD " AT&T System III / System V"
#else
#ifdef BSD4
#ifdef vax
#define HERALD " 4.2 BSD VAX"
#else
#define HERALD " 4.2 BSD"
#endif /* vax */
#else
#ifdef V7
#define HERALD " UNIX Version 7"
#endif /* V7 */
#endif /* BSD4 */
#endif /* ATTSV */
#endif /* SVR3 */
#endif /* SVR4 */
#endif /* HERALD */

#endif /* HERALD */

#ifndef HERALD
#define HERALD " Unknown Platform"
#endif /* HERALD */

/* Hardware type */

#ifdef vax				/* DEC VAX */
#ifndef CKCPU
#define CKCPU "vax"
#endif /* CKCPU */
#endif /*  vax */

#ifdef __ALPHA				/* DEC Alpha */
#ifndef CKCPU
#define CKCPU "Alpha"
#endif /* CKCPU */
#endif /* __ALPHA */

#ifdef __alpha				/* OSF/1 uses lowercase... */
#ifndef CKCPU
#define CKCPU "Alpha"
#endif /* CKCPU */
#endif /* __alpha */

#ifdef m88000				/* Motorola 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* m88000 */
#ifdef __using_M88KBCS			/* DG symbol for Motorola 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* __using_M88KBCS */
#ifdef m88k				/* Motorola symbol for 88000 */
#ifndef CKCPU
#define CKCPU "mc88000"
#endif /* CKCPU */
#endif /* m88k */
#ifdef mc68040				/* Motorola 68040 */
#ifndef CKCPU
#define CKCPU "mc68040"
#endif /* CKCPU */
#endif /* mc68040 */
#ifdef mc68030				/* Motorola 68030 */
#ifndef CKCPU
#define CKCPU "mc68030"
#endif /* CKCPU */
#endif /* mc68030 */
#ifdef mc68020				/* Motorola 68020 */
#ifndef CKCPU
#define CKCPU "mc68020"
#endif /* CKCPU */
#endif /* mc68020 */
#ifdef mc68010				/* Motorola 68010 */
#ifndef CKCPU
#define CKCPU "mc68010"
#endif /* CKCPU */
#endif /* mc68010 */
#ifdef mc68000				/* Motorola 68000 */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* mc68000 */
#ifdef mc68k				/* Ditto (used by DIAB DS90) */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* mc68k */
#ifdef m68				/* Ditto */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* m68 */
#ifdef m68k				/* Ditto */
#ifndef CKCPU
#define CKCPU "mc68000"
#endif /* CKCPU */
#endif /* m68k */

#ifdef ia64				/* IA64 / Itanium */
#ifndef CKCPU
#define CKCPU "ia64"
#endif /* CKCPU */
#endif /* i686 */

#ifdef i686				/* Intel 80686 */
#ifndef CKCPU
#define CKCPU "i686"
#endif /* CKCPU */
#endif /* i686 */

#ifdef i586				/* Intel 80586 */
#ifndef CKCPU
#define CKCPU "i586"
#endif /* CKCPU */
#endif /* i586 */

#ifdef i486				/* Intel 80486 */
#ifndef CKCPU
#define CKCPU "i486"
#endif /* CKCPU */
#endif /* i80486 */
#ifdef i386				/* Intel 80386 */
#ifndef CKCPU
#define CKCPU "i386"
#endif /* CKCPU */
#endif /* i80386 */
#ifdef i286				/* Intel 80286 */
#ifndef CKCPU
#define CKCPU "i286"
#endif /* CKCPU */
#endif /* i286 */
#ifdef i186				/* Intel 80186 */
#ifndef CKCPU
#define CKCPU "i186"
#endif /* CKCPU */
#endif /* i186 */
#ifdef M_I586				/* Intel 80586 */
#ifndef CKCPU
#define CKCPU "i586"
#endif /* CKCPU */
#endif /* M_I586 */
#ifdef M_I486				/* Intel 80486 */
#ifndef CKCPU
#define CKCPU "i486"
#endif /* CKCPU */
#endif /* M_I486 */
#ifdef _M_I386				/* Intel 80386 */
#ifndef CKCPU
#define CKCPU "i386"
#endif /* CKCPU */
#endif /* _M_I386 */
#ifdef M_I286				/* Intel 80286 */
#ifndef CKCPU
#define CKCPU "i286"
#endif /* CKCPU */
#endif /* M_I286 */
#ifdef M_I86				/* Intel 80x86 */
#ifndef CKCPU
#define CKCPU "ix86"
#endif /* CKCPU */
#endif /* M_I86 */
#ifdef sparc				/* SUN SPARC */
#ifndef CKCPU
#define CKCPU "sparc"
#endif /* CKCPU */
#endif /* sparc */
#ifdef _IBMR2				/* IBM RS/6000 */
#ifndef CKCPU				/* (what do they call the chip?) */
#define CKCPU "rs6000"
#endif /* CKCPU */
#endif /* rs6000 */
#ifdef n3b
#ifndef CKCPU
#define CKCPU "n3b"
#endif /* CKCPU */
#endif /* n3b */
#ifdef n16				/* Encore Multimax */
#ifndef CKCPU
#define CKCPU "n16"
#endif /* CKCPU */
#endif /* n16 */

#endif /* CKUVER_H */
