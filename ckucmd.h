/*  C K U C M D . H  --  Header file for interactive command parser  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University Kermit Project, New York City.

  Copyright (C) 1985, 2023,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Note: the name of these files really should be ckccmd.h and ckccmd.c
    because they are for all platforms, not just Unix.  But "don't fix
    what ain't broke".

  Previous update: Sat Sep 24 14:05:17 2022 -fdc
    Removed redundant arrow-key #ifdefs (they only need to be in ckcdeb.h)
  Last update: Sat Sep 23 20:19:19 2023 -fdc
    Raised CMDBL and TMPBUFSIZ from 4092 to 16384 for SunOS 4
*/

#ifndef CKUCMD_H
#define CKUCMD_H

/* Command recall */

#ifdef DYNAMIC /* Dynamic command buffers */
/*
  Use malloc() to allocate the many command-related buffers in ckucmd.c.
*/
#ifndef DCMDBUF
#ifndef NORECALL
#define NORECALL
#endif /* NORECALL */
#endif /* DCMDBUF */

#ifndef NORECALL
#define CK_RECALL
#else
#ifdef CK_RECALL
#undef CK_RECALL
#endif /* CK_RECALL */
#endif /* NORECALL */
#else
#ifndef NORECALL
#define NORECALL
#endif /*  NORECALL */
#endif /* DYNAMIC */

#ifdef NORECALL
#ifdef CK_RECALL
#undef CK_RECALL
#endif /* CK_RECALL */
#endif /* NORECALL */

/* Special getchars */

/* Sizes of things */

#ifndef CMDDEP
#ifdef BIGBUFOK
#define CMDDEP 64 /* Maximum command recursion depth */
#else
#define CMDDEP 20
#endif            /* BIGBUFOK */
#endif            /* CMDDEP */
#define HLPLW 78  /* Width of ?-help line */
#define HLPCW 19  /* Width of ?-help column */
#define HLPBL 100 /* Help string buffer length */
#ifdef BIGBUFOK
#define ATMBL 10238 /* Command atom buffer length */
#else
#ifdef NOSPL
#define ATMBL 256
#else
#define ATMBL 1024
#endif /* NOSPL */
#endif /* BIGBUFOK */

#ifndef CMDBL
#ifdef NOSPL
/* No script programming language, save some space */
#define CMDBL 608 /* Command buffer length */
#else
#ifdef BIGBUFOK
#define CMDBL 32763
#else
#define CMDBL 4092
#endif /* BIGBUFOK */
#endif /* NOSPL */
#endif /* CMDBL */

/* Special characters */

#define RDIS 0022 /* Redisplay   (^R) */
#define LDEL 0025 /* Delete line (^U) */
#define WDEL 0027 /* Delete word (^W) */
#ifdef CK_RECALL
#define C_UP 0020  /* Go Up in recall buffer (^P) */
#define C_UP2 0002 /* Alternate Go Up (^B) for VMS */
#define C_DN 0016  /* Go Down in recall buffer (^N) */
#endif             /* CK_RECALL */

/* Keyword flags (bits, powers of 2) */

#define CM_INV 1   /* Invisible keyword */
#define CM_ABR 2   /* Abbreviation for another keyword */
#define CM_HLP 4   /* Help-only keyword */
#define CM_ARG 8   /* An argument is required */
#define CM_NOR 16  /* No recall for this command */
#define CM_PRE 32  /* Long-form cmdline arg for prescan */
#define CM_PSH 64  /* Command disabled if nopush */
#define CM_LOC 128 /* Command disabled if nolocal */

/*
  A long-form command line option is a keyword using the regular struct keytab
  and lookup mechanisms.  Flags that make sense in this context are CM_ARG,
  indicating this option requires an argument (operand), and CM_PRE, which
  means this option must be processed before the initialization file.  The
  absence of CM_PRE means the option is to be processed after the
  initialization file in the normal manner.
*/

/* Token flags (numbers) */

#define CMT_COM 0 /* Comment (; or #) */
#define CMT_SHE 1 /* Shell escape (!) */
#define CMT_LBL 2 /* Label (:) */
#define CMT_FIL 3 /* Indirect filespec (@) (not used) */

/* Path separator for path searches */

#ifdef UNIX
#define PATHSEP ':'
#else
#define PATHSEP ','
#endif /* UNIX */

#ifndef CK_KEYTAB
#define CK_KEYTAB

/* Keyword Table Template perhaps already defined in ckcdeb.h */

struct keytab { /* Keyword table */
  char *kwd;    /* Pointer to keyword string */
  int kwval;    /* Associated value */
  int flgs;     /* Flags (as defined above) */
};
#endif /* CK_KEYTAB */

/* String preprocessing function */

typedef int (*xx_strp)(char *, char **, int *);

/* FLDDB struct */

typedef struct FDB {
  int fcode;             /* Function code */
  char *hlpmsg;          /* Help message */
  char *dflt;            /* Default */
  char *sdata;           /* Additional string data */
  int ndata1;            /* Additional numeric data 1 */
  int ndata2;            /* Additional numeric data 2 */
  xx_strp spf;           /* String processing function */
  struct keytab *kwdtbl; /* Keyword table */
  struct FDB *nxtfdb;    /* Pointer to next alternative */
} fdb;

typedef struct OFDB {
  struct FDB *fdbaddr; /* Address of succeeding FDB struct */
  int fcode;           /* Function code */
  char *sresult;       /* String result */
  int nresult;         /* Integer result */
  int kflags;          /* Keyword flags if any */
  CK_OFF_T wresult;    /* Long integer ("wide") result */
} ofdb;

#ifndef CKUCMD_C
extern struct OFDB cmresult;
#endif /* CKUCMD_C */

/* Codes for primary parsing function  */

#define _CMNUM 0 /* Number */
#define _CMOFI 1 /* Output file */
#define _CMIFI 2 /* Input file */
#define _CMFLD 3 /* Arbitrary field */
#define _CMTXT 4 /* Text string */
#define _CMKEY 5 /* Keyword */
#define _CMCFM 6 /* Confirmation */
#define _CMDAT 7 /* Date/time */
#define _CMNUW 8 /* Wide version of cmnum */

/* Function prototypes */

int xxesc(char **);
int cmrini(int);
VOID cmsetp(char *);
VOID cmsavp(char[], int);
char *cmgetp(void);
VOID prompt(xx_strp);
VOID pushcmd(char *);
VOID cmres(void);
VOID cmini(int);
int cmgbrk(void);
int cmgkwflgs(void);
int cmpush(void);
int cmpop(void);
VOID untab(char *);
int cmnum(char *, char *, int, int *, xx_strp);
int cmnumw(char *, char *, int, CK_OFF_T *, xx_strp);
int cmofi(char *, char *, char **, xx_strp);
int cmifi(char *, char *, char **, int *, xx_strp);
int cmiofi(char *, char *, char **, int *, xx_strp);
int cmifip(char *, char *, char **, int *, int, char *, xx_strp);
int cmifi2(char *, char *, char **, int *, int, char *, xx_strp, int);
int cmdir(char *, char *, char **, xx_strp);
int cmdirp(char *, char *, char **, char *, xx_strp);
int cmfld(char *, char *, char **, xx_strp);
int cmtxt(char *, char *, char **, xx_strp);
int cmkey(struct keytab[], int, char *, char *, xx_strp);
int cmkeyx(struct keytab[], int, char *, char *, xx_strp);
int cmkey2(struct keytab[], int, char *, char *, char *, xx_strp, int);
int cmswi(struct keytab[], int, char *, char *, xx_strp);
int cmdate(char *, char *, char **, int, xx_strp);
char *cmpeek(void);
int cmfdb(struct FDB *);
VOID cmfdbi(struct FDB *, int, char *, char *, char *, int, int, xx_strp,
            struct keytab *, struct FDB *);
int chktok(char *);
int cmcfm(void);
int lookup(struct keytab[], char *, int, int *);
VOID kwdhelp(struct keytab[], int, char *, char *, char *, int, int);
int ungword(void);
VOID unungw(void);
int cmdsquo(int);
int cmdgquo(void);
char *ckcvtdate(char *, int);
int cmdgetc(int);
#ifndef NOARROWKEYS
int cmdconchk(void);
#endif /* NOARROWKEYS */

#ifdef CK_RECALL
char *cmgetcmd(char *);
VOID addcmd(char *);
VOID cmaddnext(void);
#endif /* CK_RECALL */
char *cmcvtdate(char *, int);
char *cmdiffdate(char *, char *);
char *cmdelta(int, int, int, int, int, int, int, int, int, int, int, int, int);
char *shuffledate(char *, int);
int filhelp(int, char *, char *, int, int);
int xfilhelp(int, char *, char *, int, int, int, char *, char *, char *, char *,
             CK_OFF_T, CK_OFF_T, int, int, char **);
int delta2sec(char *, long *);

#ifdef DCMDBUF
int cmsetup(void);
#endif /* DCMDBUF */

#endif /* CKUCMD_H */

/* End of ckucmd.h */
