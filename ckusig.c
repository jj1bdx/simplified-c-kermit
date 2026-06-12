char *ckusigv = "Signal support, 10.0.100, 23 Sep 2022";

/* C K U S I G  --  Kermit signal handling for Unix and OS/2 systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2022,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#include "ckcsym.h"
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckcdeb.h"			/* Debug & other symbols */
#include "ckcker.h"			/* Kermit symbols */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

#include <signal.h>
#include <setjmp.h>
#include "ckcsig.h"
#include "ckcfnp.h"                     /* Prototypes (must be last) */

#ifdef NOCCTRAP
extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */

#ifdef MAC
#define signal msignal
#define SIGTYP long
#define alarm malarm
#define SIG_IGN 0
#define SIGALRM 1
#define SIGINT  2
SIGTYP (*msignal(int type, SIGTYP (*func)(int)))(int);
#endif /* MAC */

#ifdef STRATUS
/* We know these are set here.  MUST unset them before the definitions. */
#define signal vsignal
#define alarm valarm
SIGTYP (*vsignal(int type, SIGTYP (*func)(int)))(int);
int valarm(int interval);
#endif /* STRATUS */

#ifdef AMIGA
#define signal asignal
#define alarm aalarm
#define SIGALRM (_NUMSIG+1)
#define SIGTYP void
SIGTYP (*asignal(int type, SIGTYP (*func)(int)))(int);
unsigned aalarm(unsigned);
#endif /* AMIGA */



#ifndef NOCCTRAP
int
#ifdef CK_ANSIC
cc_execute( ckjptr(sj_buf), ck_sigfunc dofunc, ck_sigfunc failfunc )
#else
cc_execute( sj_buf, dofunc, failfunc)
    ckjptr(sj_buf);
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */
/* cc_execute */ {
    int rc = 0 ;
    if (
		 cksetjmp(ckjdref(sj_buf))
		 ) {
            (*failfunc)(NULL) ;
             rc = -1 ;
         } else {
            (*dofunc)(NULL);
         }
   return rc ;
}
#endif /* NOCCTRAP */

int
#ifdef CK_ANSIC				/* ANSIC C declaration... */
alrm_execute(ckjptr(sj_buf),
	     int timo,
	     ck_sighand handler,
	     ck_sigfunc dofunc,
	     ck_sigfunc failfunc
	     )

#else /* Not ANSIC C ... */

alrm_execute(sj_buf,
	     timo,
	     handler,
	     dofunc,
	     failfunc
	     )
    ckjptr(sj_buf);
    int timo;
    ck_sighand handler;
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */

/* alrm_execute */ {

    int rc = 0;
    int savalrm = 0;
_PROTOTYP(SIGTYP (*savhandler), (int));

    savalrm = alarm(timo);
    savhandler = signal(SIGALRM, handler);

    if (
		 cksetjmp(ckjdref(sj_buf))
		) {
	(*failfunc)(NULL) ;
	rc = -1 ;
    } else {
       (*dofunc)(NULL) ;
    }
    alarm(savalrm) ;
    if ( savhandler )
        signal( SIGALRM, savhandler ) ;
    return rc ;
}

int
#ifdef CK_ANSIC				/* ANSIC C declaration... */
cc_alrm_execute(ckjptr(sj_buf),
		int timo,
		ck_sighand handler,
		ck_sigfunc dofunc,
		ck_sigfunc failfunc
		)

#else /* Not ANSIC C ... */

cc_alrm_execute(sj_buf,
	     timo,
	     handler,
	     dofunc,
	     failfunc
	     )
    ckjptr(sj_buf);
    int timo;
    ck_sighand handler;
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */

/* cc_alrm_execute */ {

    int rc = 0;
    int savalrm = 0;
_PROTOTYP(SIGTYP (*savhandler), (int));
    savalrm = alarm(timo);
    savhandler = signal( SIGALRM, handler );

    if (
		 cksetjmp(ckjdref(sj_buf))
		) {
	(*failfunc)(NULL) ;
	rc = -1 ;
    } else {
       (*dofunc)(NULL) ;
    }
    alarm(savalrm);
    if (savhandler)
      signal(SIGALRM,savhandler);
    return(rc);
}
