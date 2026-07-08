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
/* clang-format off */
#include "ckcdeb.h" /* Debug & other symbols */
/* clang-format on */
#include "ckcasc.h" /* ASCII character symbols */
#include "ckcker.h" /* Kermit symbols */
#include "ckcnet.h" /* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

#include "ckcfnp.h" /* Prototypes (must be last) */
#include "ckcsig.h"
#include <setjmp.h>
#include <signal.h>

#ifdef NOCCTRAP
extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */

/*
  ck_signal() -- a signal()-compatible wrapper implemented with sigaction().

  Installs func as the handler for sig and returns the previous handler (or
  SIG_ERR on failure), just like signal().  Using sigaction() gives
  deterministic, identical semantics on every platform this tree supports
  (Linux, macOS, *BSD), whereas signal()'s behavior historically varied
  (System V one-shot reset vs. BSD persistent).  SA_RESTART plus an empty
  mask reproduces the BSD/glibc/macOS signal() semantics the code already
  relies on, so this is a behavior-preserving drop-in.  The returned value is
  a plain void(*)(int), so existing tests against SIG_IGN/SIG_DFL and the
  saved-pointer comparisons throughout the tree keep working unchanged.
*/
ck_sig_t ck_signal(int sig, ck_sig_t func) {
  struct sigaction sa, old;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = func;
  if (sigaction(sig, &sa, &old) < 0) {
    return SIG_ERR;
  }
  return old.sa_handler;
}

#ifndef NOCCTRAP
int cc_execute(ckjptr(sj_buf), ck_sigfunc dofunc, ck_sigfunc failfunc)
/* cc_execute */ {
  int rc = 0;
  if (cksetjmp(ckjdref(sj_buf))) {
    (*failfunc)(NULL);
    rc = -1;
  } else {
    (*dofunc)(NULL);
  }
  return rc;
}
#endif /* NOCCTRAP */

int alrm_execute(ckjptr(sj_buf), int timo, ck_sighand handler,
                 ck_sigfunc dofunc, ck_sigfunc failfunc)

/* alrm_execute */ {

  int rc = 0;
  int savalrm = 0;
  ck_sig_t savhandler;

  /* [V-42] Install the new SIGALRM handler before arming the timer, so an
     already-pending or very-short alarm cannot fire the previous handler. */
  savhandler = ck_signal(SIGALRM, handler);
  savalrm = alarm(timo);

  if (cksetjmp(ckjdref(sj_buf))) {
    (*failfunc)(NULL);
    rc = -1;
  } else {
    (*dofunc)(NULL);
  }
  alarm(savalrm);
  if (savhandler) {
    ck_signal(SIGALRM, savhandler);
  }
  return rc;
}

int cc_alrm_execute(ckjptr(sj_buf), int timo, ck_sighand handler,
                    ck_sigfunc dofunc, ck_sigfunc failfunc)

/* cc_alrm_execute */ {

  int rc = 0;
  int savalrm = 0;
  ck_sig_t savhandler;
  /* [V-42] Install the new SIGALRM handler before arming the timer, so an
     already-pending or very-short alarm cannot fire the previous handler. */
  savhandler = ck_signal(SIGALRM, handler);
  savalrm = alarm(timo);

  if (cksetjmp(ckjdref(sj_buf))) {
    (*failfunc)(NULL);
    rc = -1;
  } else {
    (*dofunc)(NULL);
  }
  alarm(savalrm);
  if (savhandler) {
    ck_signal(SIGALRM, savhandler);
  }
  return (rc);
}
