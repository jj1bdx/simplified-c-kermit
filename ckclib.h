/* ckclib.h -- C-Kermit library routine prototypes */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 2002, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKCLIB_H
#define CKCLIB_H

struct stringarray {
  char **a_head;
  int a_size;
};

int ckstrncpy(char *, const char *, int);
int ckstrncat(char *, const char *, int);

int ckmakmsg(char *, int, char *, char *, char *, char *);
int ckmakxmsg(char *, int, char *, char *, char *, char *, char *, char *,
              char *, char *, char *, char *, char *, char *);

char *ckstrpbrk(char *, char *);
char *ckstrstr(char *, char *);
char *chartostr(int);
int cklower(char *);
int ckupper(char *);
int ckindex(char *, char *, int, int, int);
char *ckctoa(char);
char *ckctox(CHAR, int);
char *ckitoa(int);
char *ckuitoa(unsigned int);
char *ckltoa(long);
char *ckultoa(unsigned long);
char *ckfstoa(CK_OFF_T);
CK_OFF_T ckatofs(char *);
char *ckitox(int);
char *ckltox(long);
int ispattern(char *);
int ckmatch(char *, char *, int, int);
void ckmemcpy(char *, char *, int);
char *ckstrchr(char *, char);
char *ckstrrchr(char *, char);
int ckrchar(char *);
int ckstrcmp(char *, char *, int, int);
#define xxstrcmp(a, b, c) ckstrcmp(a, b, c, 0)
int ckstrpre(char *, char *);
void sh_sort(char **, char **, int, int, int, int);
char *brstrip(char *);
char *fnstrip(char *);
int dquote(char *, int, int);
int untabify(char *, char *, int);
void makelist(char *, char *[], int);
void makestr(char **, const char *);
void xmakestr(char **, const char *);
int chknum(char *);
int rdigits(char *);
char *ckradix(char *, int, int);

/* Base-64 conversion needed for script programming and HTTP */

#ifndef NOB64
int b8tob64(char *, int, char *, int);
int b64tob8(char *, int, char *, int);
#endif /* NOB64 */

#ifdef CKFLOAT
int isfloat(char *, int);
#ifndef CKCLIB_C
#ifndef CKWART_C
extern CKFLOAT floatval;
#endif /* CKWART_C */
#endif /* CKCLIB_C */
#endif /* CKFLOAT */

char *parnam(char);
char *hhmmss(long);

void lset(char *, char *, int, int);
void rset(char *, char *, int, int);
char *ulongtohex(unsigned long, int);
long hextoulong(char *, int);
struct stringarray *cksplit(int, int, char *, char *, char *, int, int, int,
                            int);

int ckhexbytetoint(char *);
#endif /* CKCLIB_H */
