/*
 * ckustr.c - string extraction/restoration routines
*/

#include <stdio.h>
#include <sysexits.h>
#include <varargs.h>
#include <paths.h>
#include "ckcfnp.h"                     /* Prototypes (must be last) */
/*
  STR_FILE must be defined as a quoted string on the cc command line,
  for example:

        -DSTR_FILE=\\\"/usr/local/lib/cku196.sr\\\"

  This is the file where the strings go, and where C-Kermit looks for them
  at runtime.
*/

#ifdef STR_FILE
char    *StringFile = STR_FILE;
#else
char    *StringFile = "/usr/local/lib/cku196.sr";
#endif /* STR_FILE */

/*
 * If _PATH_CTIMED is defined (in <paths.h>) then use that definition.  2.11BSD
 * has this defined but 2.10BSD and other systems do not.
*/

#ifndef _PATH_CTIMED
#define _PATH_CTIMED STR_CTIMED
#endif

extern int errno;
static int strfile = -1, ourpid = 0;

#define BUFLEN 256

errprep(offset, buf)
unsigned short offset;
char *buf;
{
register int pid = getpid();

        if (pid != ourpid) {
                ourpid = pid;
                if (strfile >= 0) {
                        close(strfile);
                        strfile = -1;
                }
        }
        if (strfile < 0) {
                char *p, *getenv();
                if (p = getenv("KSTR"))
                  if (strlen(p))
                    StringFile = p;
                strfile = open(StringFile, 0);
                if (strfile < 0) {
oops:
                        fprintf(stderr, "Cannot find %s\r\n", StringFile);
                        exit(EX_OSFILE);
                }
        }
        if (lseek(strfile, (long) offset, 0) < 0
                        || read(strfile, buf, BUFLEN) <= 0)
                goto oops;
}

/* extracted string front end for printf() */
/*VARARGS1*/
strprerror(fmt, va_alist)
        int fmt;
        va_dcl
{
        va_list ap;
        char buf[BUFLEN];

        errprep(fmt, buf);
        va_start(ap);
        vprintf(buf, ap);
        va_end(ap);
}

/* extracted string front end for sprintf() */
/*VARARGS1*/
strsrerror(fmt, obuf, va_alist)
        int fmt;
        char *obuf;
        va_dcl
{
        char buf[BUFLEN];
        va_list ap;

        errprep(fmt, buf);
        va_start(ap);
        vsprintf(obuf, buf, ap);
        va_end(ap);
}

/* extracted string front end for fprintf() */
/*VARARGS1*/
strfrerror(fmt, fd, va_alist)
        int fmt;
        FILE *fd;
        va_dcl
{
        va_list ap;
        char buf[BUFLEN];

        errprep(fmt, buf);
        va_start(ap);
        vfprintf(fd, buf, ap);
        va_end(ap);
}

/* extracted string front end for perror() */
strperror(fmt)
        int fmt;
{
        char buf[BUFLEN];
        register int saverr = errno;

        errprep(fmt, buf);
        errno = saverr;
        perror(buf);
}

perror(str)
        char    *str;
        {

        printf("%s: errno %d\n", str, errno);
        }

/*
 * The following is needed _only_ on systems which do not have the C library
 * stubs for the ctime() and getpw*() functions.  In 2.11BSD these are
 * present in the libstubs.a library and accessed via "-lstubs" at link time.
 *
 * 2.10BSD's cpp has the BSD2_10 symbol builtin.  Other systems without
 * libstubs.a will need to define (via a -D option in CFLAGS) 'BSD2_10'.
*/

