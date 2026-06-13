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
#define HERALD " Linux"
#endif /* __linux__ */
#endif /* POSIX */

/* Catch-alls for anything not defined explicitly above */

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

#ifdef sparc				/* SUN SPARC */
#ifndef CKCPU
#define CKCPU "sparc"
#endif /* CKCPU */
#endif /* sparc */

#endif /* CKUVER_H */
