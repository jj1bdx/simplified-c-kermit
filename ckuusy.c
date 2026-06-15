#define XFATAL fatal

/*  C K U U S Y --  "User Interface" for C-Kermit Kermit, part Y  */

/*  Command-Line Argument Parser */

/*
  Authors:
    Frank da Cruz <fdc@columbia.edu>,
      The Kermit Project, New York City
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2023,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Most recent update:
    Fri Apr 14 14:37:46 2023 (function prototypes and declations)
*/
/* clang-format off */
#include "ckcdeb.h"
/* clang-format on */

char *bannerfile = NULL;
char *helpfile = NULL;
extern int xferlog, filepeek, nolinks;
extern char *xferfile;
extern int debtim;

#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckcnet.h"
#include "ckuusr.h"
#include "ckcxla.h"
#include <signal.h>

/* prototype for static funtion - fdc 30 November 2022 */
static int xx_ftp(char *, char *);

/*
  ckcfnp.c: new to C-Kermit 1.0.  Prototypes for functions used in
  multiple modules.  This header file should be included only after
  all others that define symbols or typedefs needed for the prototypes.
*/
#include "ckcfnp.h"

/* Prototypes for static functions used only in this module */
#ifdef USE_CL_INT
/*
  Clang 16 wrongly says cl_int() has no prototype.
  But it does have a prototype and it has an ANSI function declaration.
  In Clang 17 this will be a fatal error and C-Kermit can't be built any more.
  cl_int() is for trapping Ctrl-C before C-Kermit enters its command parser.
  Let's see if we can live without it - fdc, Fri Apr 14 14:35:16 2023
*/
static void cl_int(int);
#endif /* USE_CL_INT */
static int pmsg(char *);
static int fmsg(char *);
#ifdef TNCODE
static int dotnarg(char);
#endif /* TNCODE */
#ifdef RLOGCODE
static int dorlgarg(char);
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
static int dossharg(char);
#endif /* SSHBUILTIN */

extern int inserver, fncnv, f_save, xfermode;
#ifdef PATTERNS
extern int patterns;
#endif /* PATTERNS */

#ifndef NOICP
extern int cmdint;
#endif /* NOICP */
extern int xsuspend;

#ifdef NETCONN
#ifdef ANYX25
extern int revcall, closgr, cudata;
extern char udata[];
extern int x25fd;
#endif /* ANYX25 */
extern

    int telnetfd;
extern struct keytab netcmd[];
extern int tn_exit;
#ifndef NOICP
#ifndef NODIAL
extern int nnets, nnetdir; /* Network services directory */
extern char *netdir[];
extern char *nh_p[];  /* Network directory entry pointers */
extern char *nh_p2[]; /* Network directory entry nettype */
extern char *nh_px[4][MAXDNUMS + 1];
#endif /* NODIAL */
extern int nhcount;
extern char *n_name; /* Network name pointer */
#endif               /* NOICP */
#endif               /* NETCONN */

#ifndef NOSPL
extern int nmac;
extern struct mtab *mactab;
#endif /* NOSPL */
extern char uidbuf[];

#ifdef CK_LOGIN
extern int logintimo;
#endif /* CK_LOGIN */

extern char *myname, *dftty;
extern int howcalled;

extern char *ckxsys, *ckzsys, **xargv, *xarg0, **cmlist, *clcmds;

extern int action, cflg, xargc, cnflg, local, quiet, escape, network, mdmtyp,
    bgset, backgrd, xargs, binary, parity, turn, turnch, duplex, flow, clfils,
    noinit, stayflg, nettype, cfilef, noherald, cmask, cmdmsk, exitonclose,
    haveline, justone, cxtype, xfinish, ttnproto;

extern long speed;
extern char ttname[];
extern char *pipedata, cmdfil[];

#ifndef NOXFER
extern char *cmarg, *cmarg2;

extern int nfils, stdouf, stdinf, displa, maxrps, rpsiz, ckwarn, urpsiz, wslotr,
    swcapr, ckdelay, recursive, reliable, xreliable, fnspath, fncact, clearrq,
    setreliable;

#ifdef PIPESEND
extern int usepipes, pipesend;
#endif /* PIPESEND */
extern int protocol;
#endif /* NOXFER */

#ifndef NOPUSH
extern int nopush;
#endif /* NOPUSH */

#ifdef CK_NETBIOS
extern unsigned char NetBiosAdapter;
#endif /* CK_NETBIOS */

#ifdef XFATAL
#undef XFATAL
#endif /* XFATAL */

int haveftpuid = 0; /* Have FTP user ID */

static char *failmsg = NULL; /* Failure message */

#ifdef NEWFTP
extern char *ftp_host;
#endif /* NEWFTP */

extern int what;

#ifndef NOICP
#ifndef NODIAL
extern int nmdm, telephony;
extern struct keytab mdmtab[];
extern int usermdm, dialudt;
#endif /* NODIAL */

static int pmsg(char *s) {
  printf("%s\n", s);
  return (0);
}

static int fmsg(char *s) {
  fatal(s);
  return (0);
}

#define XFATAL(s) return (what == W_COMMAND ? pmsg(s) : fmsg(s))
#else
#define XFATAL fatal
#endif /* NOICP */

#ifndef NOHTTP
#define HTTP_GET 1
#define HTTP_PUT 2
#define HTTP_HED 3
#endif /* NOHTTP */

#ifdef CK_URL
/* URLs we recognize */

#define URL_FTP 1
#define URL_HTTP 2
#define URL_HTTPS 3
#define URL_IKSD 4
#define URL_TELNET 5
#define URL_LOGIN 6

struct keytab urltab[] = {
#ifdef NEWFTP
    "ftp",    URL_FTP,    0,
#endif /* NEWFTP */
#ifndef NOHTTP
    "http",   URL_HTTP,   0, "https",  URL_HTTPS, 0,
#endif /* NOHTTP */
    "iksd",   URL_IKSD,   0, "kermit", URL_IKSD,  0,
    "telnet", URL_TELNET, 0, "",       0,         0};
int nurltab = sizeof(urltab) / sizeof(struct keytab) - 1;

struct urldata g_url = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/* u r l p a r s e  --  Parse a possible URL */

/*
  Returns 0 if the candidate does not seem to be a URL.
  Returns 1 if it might be a URL, with the above pointers set to its pieces:
    service : [ // ] [ user [ : password ] @ ] host [ : service ] [ / path ] -
              [ ? name [ = value ]]

  Example: ftp://ds.internic.net:21/rfc/rfc1234.txt
    url.svc = [ftp]
    url.usr = [(NULL)]
    url.psw = [(NULL)]
    url.hos = [ds.internic.net]
    url.por = [21]
    url.pth = [rfc/rfc1234.txt]

  It might be a URL if it contains a possible service name followed by a
  a colon (:).  Thus "telnet:xyzcorp.com" is a minimal URL, whereas a
  full-blown example would be:

    ftp://olga:secret@ftp.xyzcorp.com/public/oofa.txt

  The caller must verify the results, i.e. that the service string is a real
  TCP service, etc.  This routine just parses the fields.

  struct urldata defined in ckcker.h
*/

int urlparse(char *s, struct urldata *url) {
  char *p = NULL, *urlbuf = NULL;

  if (!s || !url)
    return (0);

  if (!*s)
    return (0);

  makestr(&urlbuf, s);

  if (url->sav) { /* In case we were called before... */
    free(url->sav);
    url->sav = NULL;
  }
  if (url->svc) {
    free(url->svc);
    url->svc = NULL;
  }
  if (url->hos) {
    free(url->hos);
    url->hos = NULL;
  }
  if (url->por) {
    free(url->por);
    url->por = NULL;
  }
  if (url->usr) {
    free(url->usr);
    url->usr = NULL;
  }
  if (url->psw) {
    free(url->psw);
    url->psw = NULL;
  }
  if (url->pth) {
    free(url->pth);
    url->pth = NULL;
  }
  if (url->nopts) {
    int i;
    for (i = 0; i < url->nopts && i < MAX_URL_OPTS; i++) {
      if (url->opt[i].nam) {
        free(url->opt[i].nam);
        url->opt[i].nam = NULL;
      }
      if (url->opt[i].val) {
        free(url->opt[i].val);
        url->opt[i].val = NULL;
      }
    }
    url->nopts = 0;
  }
  p = urlbuf;             /* Was a service requested? */
  while (*p && *p != ':') /* Look for colon */
    p++;
  if (*p == ':') { /* Have a colon */
    *p++ = NUL;    /* Get service name or number */
    if (*p == ':') /* a second colon */
      *p++ = NUL;  /* get rid of that one too */
    while (*p == '/')
      *p++ = NUL;    /* and slashes */
    if (urlbuf[0]) { /* Anything left? */
      char *q = p, *r = p, *w = p;
      makestr(&url->svc, urlbuf);

      while (*p != NUL && *p != '@') /* look for @ */
        p++;
      if (*p == '@') { /* Signifies user ID, maybe password */
        *p++ = NUL;
        url->hos = p;
        while (*w != NUL && *w != ':')
          w++;
        if (*w == ':')
          *w++ = NUL;
        url->usr = r; /* Username */
        if (*w)
          url->psw = w; /* Password */
        q = p;
      } else { /* No username or password */
        p = q;
        url->hos = p;
      }

      while (*p != NUL && *p != ':' && *p != '/') /* Port? */
        p++;
      if (*p == ':') { /* TCP port */
        *p++ = NUL;
        r = p;
        url->por = r;
        while (*p != NUL && *p != '/')
          p++;
        /* '/' is part of path, leave it until we can copy */
        if (*p == '/') {
          makestr(&url->pth, p); /* Path */
          *p = NUL;
        }
      } else { /* No port */
        /* '/' is part of path, leave it */
        if (*p == '/') {
          makestr(&url->pth, p); /* Path */
          *p = NUL;
        }
      }
    }
    /* Copy non-NULL result strings */
    if (url->svc)
      if (*url->svc) {
        p = url->svc;
        url->svc = NULL;
        makestr(&url->svc, p);
      }
    if (url->hos)
      if (*url->hos) {
        p = url->hos;
        url->hos = NULL;
        makestr(&url->hos, p);
      }
    if (url->por)
      if (*url->por) {
        p = url->por;
        url->por = NULL;
        makestr(&url->por, p);
      }
    /*
      WARNING (Wed Oct  9 16:09:03 2002): We now allow the username and
      password to be empty strings.  These are treated differently from null
      pointers: an empty string means the URL included username and/or password
      fields that were empty, e.g. ftp://:@ftp.xyzcorp.com/somepath/somefile,
      which causes the client to prompt for the username and/or password.
    */
    if (url->usr) /* if (*url->usr) */ {
      p = url->usr;
      url->usr = NULL;
      makestr(&url->usr, p);
    }
    if (url->psw) /* if (*url->psw) */ {
      p = url->psw;
      url->psw = NULL;
      makestr(&url->psw, p);
    }
    /* Save a copy of the full url if one was found. */
    if (url->svc)
      makestr(&url->sav, s);
    free(urlbuf);
    return (url->svc ? 1 : 0);
  }
  return (0);
}
#endif /* CK_URL */

#ifndef NOCMDL

char *hlp1[] = {
#ifndef NOICP
    " [filename] [-x arg [-x arg]...[-yyy]..] [ = text ] ]\n",
#else
    "[-x arg [-x arg]...[-yyy]..]\n",
#endif /* NOICP */
    "\n",
    "  -x is an option requiring an argument, -y an option with no argument.\n",
    "  If the first command-line argument is the name of a file, "
    "interactive-\n",
    "  mode commands are executed from the file.  The '=' argument tells "
    "Kermit\n",
    "  not to parse the remainder of the command line, but to make the words\n",
    "  following '=' available as \\%1, \\%2, ... \\%9.  The command file \
(if any)\n",
    "  is executed before the command-line options.\n",
    "\n",
#ifndef NOICP
    "If no action command is included, or -S is, then after the command line "
    "is\n",
    "executed, Kermit issues its prompt and waits for you to type commands.\n",
#else
    "Operation by command-line options only.\n",
#endif /* NOICP */
    ""};

static char *hlp2[] = {
    "  [option-list] host[:port] [port]\n",
    "  The option-list consists of zero, one, or more of:\n",
    "  -8                Negotiate Telnet Binary in both directions\n",
    "  -a                Require use of Telnet authentication\n",
    "  -d                Turn on debug mode\n",
    "  -E                No escape character\n",
    "  -K                Refuse use of authentication; do not send username\n",
    "  -l user           Set username and request Telnet authentication\n",
    "  -L                Negotiate Telnet Binary Output only\n",
    "  -x                Require Encryption\n",
    "  -D                Disable forward-X\n",
#ifndef NOICP
    "  -T cert=file      Use certificate in file\n",
    "  -T key=file       Use private key in file\n",
    "  -T crlfile=file   Use CRL in file\n",
    "  -T crldir=dir     Use CRLs in directory\n",
    "  -T cipher=string  Use only ciphers in string\n",
#endif /* NOICP */
    "  -f                Forward credentials to host\n",
    "  -k realm          Set default Kerberos realm\n",
    ""};

static char *hlp3[] = {/* rlogin */
                       "  [option-list] host[:port] [port]\n",
                       "  The option-list consists of zero, one, or more of:\n",
                       "  -d                Turn on debug mode\n",
                       "  -l user           Set username\n", ""};

static char *hlp4[] = {/* ssh */
                       "  [option-list] host[:port] [port]\n",
                       "  The option-list consists of zero, one, or more of:\n",
                       "  -d                Turn on debug mode\n",
                       "  -Y                Disable init file processing\n",
                       "  -l user           Set username\n",
                       ""};

/* Command-line option help lines.  Update this when adding new options! */

char *opthlp[128]; /* Option help */
char *arghlp[128]; /* Argument for option */
int optact[128];   /* Action-option flag */

void fatal2(char *msg1, char *msg2) {
  char buf[256];
  if (!msg1)
    msg1 = "";
  if (!msg2)
    msg2 = "";
  ckmakmsg(buf, 256, "\"", msg1, "\" - ", msg2);
#ifndef NOICP
  if (what == W_COMMAND)
    printf("%s\n", buf);
  else
#endif /* NOICP */
    fatal((char *)buf);
}

#ifdef USE_CL_INT
/*
  cl_int() ("command-line interrupt") is for trapping Ctrl-C as
  C-Kermit executes any command-line options before entering its
  command parser.  But cl_int() generates an inexplicable
  "non-prototype" warning by Clang 15 that will turn into a fatal
  error in Clang C2x.  Look, here's the prototype right here.
  fdc - Fri Apr 14 14:51:22 2023
*/
static void
#ifdef CK_ANSI
cl_int(int dummy) /* Command-line interrupt handler */
#else             /* CK_ANSI */
    cl_int(dummy) int dummy;
#endif            /* CK_ANSI */
{
  doexit(BAD_EXIT, 1);
  return;
}
#endif /* USE_CL_INT */

#ifdef NEWFTP
extern int ftp_action, ftp_cmdlin;

static int xx_ftp(char *host, char *port) {
#ifdef CK_URL
  extern int haveurl;
#endif /* CK_URL */
  extern char *ftp_logname;
  int use_tls = 0;
  char *p;

  if (port)
    if (!*port)
      port = NULL;

  if (!host)
    return (0);
  if (!*host)
    return (0);
  debug(F111, "ftp xx_ftp host", ftp_host, haveftpuid);
  debug(F111, "ftp xx_ftp uidbuf 1", uidbuf, haveftpuid);
  ftp_cmdlin = 1; /* 1 = FTP started from command line */
  if (nfils > 0)
    ftp_cmdlin++; /* 2 = same plus file transfer */

#ifndef NOURL
  /* debug(F111,"ftp xx_ftp g_url.usr",g_url.usr,g_url.usr); */
  if (haveurl && g_url.usr) { /* Wed Oct  9 15:15:22 2002 */
    if (!*(g_url.usr)) {      /* Force username prompt if */
      haveftpuid = 0;         /* "ftp://:@host" given. */
      uidbuf[0] = NUL;
      makestr(&ftp_logname, NULL);
    }
    debug(F111, "ftp xx_ftp uidbuf 2", uidbuf, haveftpuid);
  }
#endif /* NOURL */
  debug(F111, "ftp xx_ftp uidbuf 3", uidbuf, haveftpuid);
  if (haveftpuid) {
    makestr(&ftp_logname, uidbuf);
    debug(F111, "ftp_logname", ftp_logname, haveftpuid);
  }
  if (!port) {
    if ((p = ckstrchr(ftp_host, ':')))
      *p++ = NUL;
    port = p;
  }
  if (!port) {
#ifdef CK_URL
    if (haveurl) {
      if (g_url.por)
        port = g_url.por;
      else if (g_url.svc)
        port = g_url.svc;
      else
        port = "ftp";
    } else
#endif /* CK_URL */
      port = "ftp";
  }

  if (ftpopen(ftp_host, port, use_tls) < 1)
    return (-1);
  debug(F111, "ftp xx_ftp action", ckctoa((char)ftp_action), nfils);
  if (nfils > 0) {
    switch (ftp_action) {
    case 'g':
      return (cmdlinget(stayflg));
    case 'p':
    case 's':
      return (cmdlinput(stayflg));
    }
  }
  return (1);
}
#endif /* NEWFTP */

#ifndef NOHTTP
static char *http_hlp[] = {
    " -h             This message.\n",
    " -d             Debug to debug.log.\n",
    " -S             Stay (issue command prompt when done).\n",
    " -Y             Do not execute Kermit initialization file.\n",
    " -q             Quiet (suppress most messages).\n",
    " -u name        Username.\n",
    " -P password    Password.\n",
    " -g pathname    Get remote pathname.\n",
    " -p pathname    Put remote pathname.\n",
    " -H pathname    Head remote pathname.\n",
    " -l pathname    Local path for -g, -p, and -H.\n",
    ""};

#endif /* NOHTTP */

/*  U S A G E */

void usage() {
  conol("Usage: ");
  conol(xarg0);
  if (howcalled == I_AM_KERMIT || howcalled == I_AM_IKSD ||
      howcalled == I_AM_SSHSUB)
    conola(hlp1);
  else if (howcalled == I_AM_TELNET)
    conola(hlp2);
  else if (howcalled == I_AM_RLOGIN)
    conola(hlp3);
  else if (howcalled == I_AM_SSH)
    conola(hlp4);
  if (howcalled == I_AM_KERMIT || howcalled == I_AM_IKSD ||
      howcalled == I_AM_SSHSUB) {
    int c;
    conoll("");
    conoll("Complete listing of command-line options:");
    conoll("");
    for (c = 31; c < 128; c++) {
      if (!opthlp[c])
        continue;
      if (arghlp[c]) {
        printf(" -%c <arg>%s\n", (char)c,
               (optact[c] ? " (action option)" : ""));
        printf("     %s\n", opthlp[c]);
        printf("     Argument: %s\n\n", arghlp[c]);
      } else { /* Option without arg */
        printf(" -%c  %s%s\n", (char)c, opthlp[c],
               (optact[c] ? " (action option)" : ""));
        printf("     Argument: (none)\n\n");
      }
    }
    printf("To prevent this message from scrolling, use '%s -h | more'.\n",
           xarg0);
    printf("For a list of extended options use '%s --help'.\n", xarg0);
  }
}

/*  C M D L I N  --  Get arguments from command line  */

int cmdlin() {
  char x; /* Local general-purpose char */
  extern int haveurl;

#ifdef NEWFTP
  char *port = NULL;
#endif /* NEWFTP */

#ifndef NOXFER
  cmarg = ""; /* Initialize globals */
  cmarg2 = "";
#endif /* NOXFER */
  action = 0;
  cflg = 0;

#ifdef USE_CL_INT
  signal(SIGINT, cl_int);
#endif /* USE_CL_INT */

  /* Here we handle different "Command Line Personalities" */

#ifdef TCPSOCKET
#ifndef NOHTTP
  if (howcalled == I_AM_HTTP) { /* If I was called as HTTP... */
    char rdns[128];
    char *agent = "C-Kermit";

    debug(F100, "http personality", "", 0);
#ifndef NOICP
#ifdef CK_URL
    if (haveurl) {
      int type;
      char *lfile;

      type = lookup(urltab, g_url.svc, nurltab, NULL);
      if (!(type == URL_HTTP || type == URL_HTTPS)) {
        printf("?Internal Error: HTTP command line processing\n");
        debug(F100, "Error: HTTP command line processing", "", 0);
        doexit(BAD_EXIT, 1);
      }
      rdns[0] = '\0';
      lfile = "";
      x = (http_open(g_url.hos, g_url.por ? g_url.por : g_url.svc,
                     type == URL_HTTPS, rdns, 128, NULL) == 0);
      if (x) {
        if (!quiet) {
          if (rdns[0])
            printf("Connected to %s [%s]\r\n", g_url.hos, rdns);
          else
            printf("Connected to %s\r\n", g_url.hos);
        }
        if (g_url.pth)
          zstrip(g_url.pth, &lfile);
        else
          g_url.pth = "/";

        if (!*lfile)
          lfile = "index.html";

        x = http_get(agent, NULL,                                 /* hdrlist */
                     g_url.usr, g_url.psw, 0, lfile, g_url.pth, 0 /* stdio */
        );
        x = (http_close() == 0);
      } else {
        if (!quiet)
          printf("?HTTP Connection failed.\r\n");
      }
      doexit(x ? GOOD_EXIT : BAD_EXIT, -1);
    } else
#endif /* CK_URL */
#endif /* NOICP */
    {
      int http_action = 0;
      char *host = NULL, *svc = NULL, *lpath = NULL;
      char *user = NULL, *pswd = NULL, *path = NULL;
      char *xp;

      while (--xargc > 0) { /* Go through command line words */
        xargv++;
        debug(F111, "cmdlin http xargv", *xargv, xargc);
        xp = *xargv + 1;
        if (**xargv == '-') { /* Got an option */
          x = *(*xargv + 1);  /* Get the option letter */
          switch (x) {
          case 'd': /* Debug */
#ifdef DEBUG
            if (deblog) {
              debtim = 1;
            } else {
              deblog = debopn("debug.log", 0);
            }
#endif /* DEBUG */
            break;
          case 'S': /* Stay */
          case 'Y': /* No initialization file */
            break;  /* (already done in prescan) */
          case 'q': /* Quiet */
            quiet = 1;
            break;
          case 'u': /* Options that require arguments */
          case 'P':
          case 'g':
          case 'p':
          case 'H':
          case 'l':
            if (*(xp + 1)) {
              XFATAL("Invalid argument bundling");
            }
            xargv++, xargc--;
            if ((xargc < 1) || (**xargv == '-')) {
              XFATAL("Missing argument");
            }
            switch (x) {
            case 'u':
              user = *xargv;
              break;
            case 'P':
              pswd = *xargv;
              break;
            case 'l':
              if (http_action != HTTP_PUT)
                lpath = *xargv;
              break;
            case 'g':
              http_action = HTTP_GET;
              path = *xargv;
              debug(F111, "cmdlin http GET", path, http_action);
              break;
            case 'p':
              http_action = HTTP_PUT;
              path = *xargv;
              break;
            case 'H':
              http_action = HTTP_HED;
              path = *xargv;
            }
            break;

          case 'h': /* Help */
          default:
            printf("Usage: %s host [ options... ]\n", xarg0);
            conola(http_hlp);
            doexit(GOOD_EXIT, -1);
          }
        } else { /* No dash - must be hostname */
          host = *xargv;
          if (xargc > 1) {
            svc = *(xargv + 1);
            if (svc)
              if (*svc == '-' || !*svc)
                svc = NULL;
            if (svc) {
              xargv++;
              xargc--;
            }
          }
        }
      }
      if (!svc)
        svc = "";
      if (!*svc)
        svc = "http";
      if (!host)
        XFATAL("No http host given");

      /* Check action args before opening the connection */
      if (http_action) {
        if (http_action == HTTP_PUT) {
          if (!lpath)
            XFATAL("No local path for http PUT");
        }
        if (!path)
          XFATAL("No remote path for http action");
      }
      /* Now it's OK to open the connection */
      rdns[0] = NUL;
      x = (http_open(host, svc, !ckstrcmp("https", svc, -1, 0), rdns, 128,
                     NULL) == 0);
      if (!x) {
        if (!quiet)
          printf("?HTTP Connection failed.\r\n");
        doexit(BAD_EXIT, -1);
      }
      if (!quiet) {
        if (rdns[0])
          printf("Connected to %s [%s]\r\n", host, rdns);
        else
          printf("Connected to %s\r\n", host);
      }
      if (http_action) {
        int pcpy = 0;
        if (http_action != HTTP_PUT) { /* Supply default */
          if (!lpath) {                /* local path... */
            zstrip(path, &lpath);
            if (!lpath)
              lpath = "";
            if (!*lpath)
              lpath = "index.html";
          }
        }
        if (*path != '/') {
          char *p = (char *)malloc(strlen(path) + 2);
          if (!p)
            fatal("?Memory allocation error\n");
          *p = '/';
          strcpy(&p[1], path); /* safe */
          path = p;
          pcpy = 1;
        }
        switch (http_action) {
        case HTTP_GET:
          x = http_get(agent, NULL, user, pswd, 0, lpath, path, 0);
          break;

        case HTTP_PUT:
          x = http_put(agent, NULL, "text/HTML", user, pswd, 0, lpath, path,
                       NULL, 0);
          break;

        case HTTP_HED:
          x = http_head(agent, NULL, user, pswd, 0, lpath, path, 0);
          break;
        }
        debug(F101, "cmdline http result", "", x);
        x = (http_close() == 0);
        if (pcpy)
          free(path);
        doexit(x ? GOOD_EXIT : BAD_EXIT, -1);
      }
      return (0);
    }
  } else
#endif /* NOHTTP */
#ifdef NEWFTP
      if (howcalled == I_AM_FTP) { /* If I was called as FTP... */
    debug(F100, "ftp personality", "", 0);
#ifdef CK_URL
    if (haveurl)
      doftparg('U');
    else
#endif /* CK_URL */
    {
      while (--xargc > 0) { /* Go through command line words */
        xargv++;
        debug(F111, "cmdlin ftp xargv", *xargv, xargc);
        if (**xargv == '-') { /* Got an option */
          int xx;
          x = *(*xargv + 1); /* Get the option letter */
          xx = doftparg(x);
          if (xx < 0) {
            if (what == W_COMMAND)
              return (0);
            else
              doexit(BAD_EXIT, 1);
          }
        } else { /* No dash - must be hostname */
          makestr(&ftp_host, *xargv);
          if (xargc > 1) {
            port = *(xargv + 1);
            if (port)
              if (*port == '-' || !*port)
                port = NULL;
            if (port) {
              xargv++;
              xargc--;
            }
          }
          debug(F110, "cmdlin ftp host", ftp_host, 0);
          debug(F110, "cmdlin ftp port", port, 0);
        }
      } /* while */
    } /* if (haveurl) */

    if (ftp_host) {
      int xx;
#ifdef NODIAL
      xx = xx_ftp(ftp_host, port);
      if (xx < 0 && (haveurl || ftp_cmdlin > 1))
        doexit(BAD_EXIT, -1);
#else
#ifdef NOICP
      xx = xx_ftp(ftp_host, port);
      if (xx < 0 && (haveurl || ftp_cmdlin > 1))
        doexit(BAD_EXIT, -1);
#else
      if (*ftp_host == '=') { /* Skip directory lookup */
        xx = xx_ftp(&ftp_host[1], port);
        if (xx < 0 && (haveurl || ftp_cmdlin > 1))
          doexit(BAD_EXIT, -1);
      } else { /* Want lookup */
        int i;
        nhcount = 0; /* Check network directory */
        debug(F101, "cmdlin nnetdir", "", nnetdir);
        if (nnetdir > 0)   /* If there is a directory... */
          lunet(ftp_host); /* Look up the name */
        else               /* If no directory */
          nhcount = 0;     /* we didn't find anything there */
#ifdef DEBUG
        if (deblog) {
          debug(F101, "cmdlin lunet nhcount", "", nhcount);
          if (nhcount > 0) {
            debug(F110, "cmdlin lunet nh_p[0]", nh_p[0], 0);
            debug(F110, "cmdlin lunet nh_p2[0]", nh_p2[0], 0);
            debug(F110, "cmdlin lunet nh_px[0][0]", nh_px[0][0], 0);
          }
        }
#endif /* DEBUG */
        if (nhcount == 0) {
          xx = xx_ftp(ftp_host, port);
          if (xx < 0 && (haveurl || ftp_cmdlin > 1))
            doexit(BAD_EXIT, -1);
        } else {
          for (i = 0; i < nhcount; i++) {
            if (ckstrcmp(nh_p2[i], "tcp/ip", 6, 0))
              continue;
            makestr(&ftp_host, nh_p[i]);
            debug(F110, "cmdlin calling xx_ftp", ftp_host, 0);
            if (!quiet)
              printf("Trying %s...\n", ftp_host);
            if (xx_ftp(ftp_host, port) > -1)
              break;
          }
        }
      }
#endif /* NODIAL */
#endif /* NOICP */
      if (!ftpisconnected())
        doexit(BAD_EXIT, -1);
    }
    return (0);
  }
#endif /* NEWFTP */

#ifdef TNCODE
  if (howcalled == I_AM_TELNET) { /* If I was called as Telnet... */

    while (--xargc > 0) { /* Go through command line words */
      xargv++;
      debug(F111, "cmdlin telnet xargv", *xargv, xargc);
      if (**xargv == '=')
        return (0);
      if (!strcmp(*xargv, "--")) /* getopt() conformance */
        return (0);
      else if (**xargv == '-') { /* Got an option (begins with dash) */
        int xx;
        x = *(*xargv + 1); /* Get the option letter */
        debug(F111, "cmdlin telnet args 1", *xargv, xargc);
        xx = dotnarg(x);
        debug(F101, "cmdlin telnet doarg", "", xx);
        debug(F111, "cmdlin telnet args 2", *xargv, xargc);
        if (xx < 0) {
#ifndef NOICP
          if (what == W_COMMAND)
            return (0);
          else
#endif /* NOICP */
          {
            doexit(BAD_EXIT, 1); /* Go handle option */
          }
        }
      } else { /* No dash must be hostname */
        ckstrncpy(ttname, *xargv, TTNAMLEN + 1);
        debug(F110, "cmdlin telnet host", ttname, 0);

#ifndef NOICP
#ifndef NODIAL
        nhcount = 0; /* Check network directory */
        debug(F101, "cmdlin telnet nnetdir", "", nnetdir);
        if (nnetdir > 0) /* If there is a directory... */
          lunet(*xargv); /* Look up the name */
        else             /* If no directory */
          nhcount = 0;   /* we didn't find anything there */
#ifdef DEBUG
        if (deblog) {
          debug(F101, "cmdlin telnet lunet nhcount", "", nhcount);
          if (nhcount > 0) {
            debug(F110, "cmdlin telnet lunet nh_p[0]", nh_p[0], 0);
            debug(F110, "cmdlin telnet lunet nh_p2[0]", nh_p2[0], 0);
            debug(F110, "cmdlin telnet lunet nh_px[0][0]", nh_px[0][0], 0);
          }
        }
#endif                               /* DEBUG */
        if (nhcount > 0 && nh_p2[0]) /* If network type specified */
          if (ckstrcmp(nh_p2[0], "tcp/ip", 6, 0)) /* it must be TCP/IP */
            nhcount = 0;
        if (nhcount == 1) { /* Still OK, so make substitution */
          ckstrncpy(ttname, nh_p[0], TTNAMLEN + 1);
          debug(F110, "cmdlin telnet lunet substitution", ttname, 0);
        }
#endif /* NODIAL */
#endif /* NOICP */

        if (--xargc > 0 && !haveurl) { /* Service from command line? */
          xargv++;
          ckstrncat(ttname, ":", TTNAMLEN + 1);
          ckstrncat(ttname, *xargv, TTNAMLEN + 1);
          debug(F110, "cmdlin telnet host2", ttname, 0);
        }
#ifndef NOICP
#ifndef NODIAL
        else if (nhcount) { /* No - how about in net directory? */
          if (nh_px[0][0]) {
            ckstrncat(ttname, ":", TTNAMLEN + 1);
            ckstrncat(ttname, nh_px[0][0], TTNAMLEN + 1);
          }
        }
#endif             /* NODIAL */
#endif             /* NOICP */
        local = 1; /* Try to open the connection */
        nettype = NET_TCPB;
        mdmtyp = -nettype;
        if (ttopen(ttname, &local, mdmtyp, 0) < 0) {
          XFATAL("can't open host connection");
        }
        network = 1; /* It's open */
#ifdef CKLOGDIAL
        dolognet();
#endif /* CKLOGDIAL */
#ifndef NOXFER
        reliable = 1;  /* It's reliable */
        xreliable = 1; /* ... */
        setreliable = 1;
#endif               /* NOXFER */
        cflg = 1;    /* Connect */
        stayflg = 1; /* Stay */
        tn_exit = 1; /* Telnet-like exit condition */
        quiet = 1;
        exitonclose = 1; /* Exit when connection closes */
#ifndef NOSPL
        if (local) {
          if (nmac) {                           /* Any macros defined? */
            int k;                              /* Yes */
            k = mlook(mactab, "on_open", nmac); /* Look this up */
            if (k >= 0) {                       /* If found, */
              if (dodo(k, ttname, 0) > -1)      /* set it up, */
                parser(1);                      /* and execute it */
            }
          }
        }
#endif /* NOSPL */
        break;
      }
    }
    return (0);
  }
#endif /* TNCODE */
#ifdef RLOGCODE
  else if (howcalled == I_AM_RLOGIN) { /* If I was called as Rlogin... */
    while (--xargc > 0) {              /* Go through command line words */
      xargv++;
      debug(F111, "cmdlin rlogin xargv", *xargv, xargc);
      if (**xargv == '=')
        return (0);
      if (!strcmp(*xargv, "--")) /* getopt() conformance */
        return (0);
      else if (**xargv == '-') { /* Got an option (begins with dash) */
        int xx;
        x = *(*xargv + 1); /* Get the option letter */
        debug(F111, "cmdlin rlogin args 1", *xargv, xargc);
        xx = dorlgarg(x);
        debug(F101, "cmdlin rlogin doarg", "", xx);
        debug(F111, "cmdlin rlogin args 2", *xargv, xargc);
        if (xx < 0) {
#ifndef NOICP
          if (what == W_COMMAND)
            return (0);
          else
#endif /* NOICP */
          {
            doexit(BAD_EXIT, 1); /* Go handle option */
          }
        }
      } else { /* No dash must be hostname */
        ckstrncpy(ttname, *xargv, TTNAMLEN + 1);
        debug(F110, "cmdlin rlogin host", ttname, 0);

#ifndef NOICP
#ifndef NODIAL
        nhcount = 0; /* Check network directory */
        debug(F101, "cmdlin rlogin nnetdir", "", nnetdir);
        if (nnetdir > 0) /* If there is a directory... */
          lunet(*xargv); /* Look up the name */
        else             /* If no directory */
          nhcount = 0;   /* we didn't find anything there */
#ifdef DEBUG
        if (deblog) {
          debug(F101, "cmdlin rlogin lunet nhcount", "", nhcount);
          if (nhcount > 0) {
            debug(F110, "cmdlin rlogin lunet nh_p[0]", nh_p[0], 0);
            debug(F110, "cmdlin rlogin lunet nh_p2[0]", nh_p2[0], 0);
            debug(F110, "cmdlin rlogin lunet nh_px[0][0]", nh_px[0][0], 0);
          }
        }
#endif                               /* DEBUG */
        if (nhcount > 0 && nh_p2[0]) /* If network type specified */
          if (ckstrcmp(nh_p2[0], "tcp/ip", 6, 0)) /* it must be TCP/IP */
            nhcount = 0;
        if (nhcount == 1) { /* Still OK, so make substitution */
          ckstrncpy(ttname, nh_p[0], TTNAMLEN + 1);
          debug(F110, "cmdlin rlogin lunet substitution", ttname, 0);
        }
#endif /* NODIAL */
#endif /* NOICP */

        if (!haveurl) { /* Service from command line? */
          ckstrncat(ttname, ":login", TTNAMLEN + 1);
          debug(F110, "cmdlin rlogin host2", ttname, 0);
        }
        local = 1; /* Try to open the connection */
        nettype = NET_TCPB;
        mdmtyp = -nettype;
        if (ttopen(ttname, &local, mdmtyp, 0) < 0) {
          XFATAL("can't open host connection");
        }
        network = 1; /* It's open */
#ifdef CKLOGDIAL
        dolognet();
#endif /* CKLOGDIAL */
#ifndef NOXFER
        reliable = 1;  /* It's reliable */
        xreliable = 1; /* ... */
        setreliable = 1;
#endif               /* NOXFER */
        cflg = 1;    /* Connect */
        stayflg = 1; /* Stay */
        tn_exit = 1; /* Telnet-like exit condition */
        quiet = 1;
        exitonclose = 1; /* Exit when connection closes */
#ifndef NOSPL
        if (local) {
          if (nmac) {                           /* Any macros defined? */
            int k;                              /* Yes */
            k = mlook(mactab, "on_open", nmac); /* Look this up */
            if (k >= 0) {                       /* If found, */
              if (dodo(k, ttname, 0) > -1)      /* set it up, */
                parser(1);                      /* and execute it */
            }
          }
        }
#endif /* NOSPL */
        break;
      }
    }
    return (0);
  }
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifdef SSHBUILTIN
  if (howcalled == I_AM_SSH) { /* If I was called as SSH... */
    debug(F100, "ssh personality", "", 0);
#ifdef CK_URL
    if (haveurl) {
      ssh_set_sparam(SSH_SPARAM_HST, g_url.hos);
      ssh_set_sparam(SSH_SPARAM_PRT, g_url.svc);
      ckstrncpy(ttname, ssh_get_sparam(SSH_SPARAM_HST), TTNAMLEN + 1);
      ckstrncat(ttname, ":", TTNAMLEN + 1);
      ckstrncat(ttname, ssh_get_sparam(SSH_SPARAM_PRT), TTNAMLEN + 1);
    } else
#endif /* CK_URL */
    {
      while (--xargc > 0) { /* Go through command line words */
        xargv++;
        debug(F111, "cmdlin ssh xargv", *xargv, xargc);
        if (**xargv == '=')
          return (0);
        if (!strcmp(*xargv, "--")) /* getopt() conformance */
          return (0);
        /* Got an option (begins with dash) */
        else if (**xargv == '-') {
          int xx;
          x = *(*xargv + 1); /* Get the option letter */
          debug(F111, "cmdlin args 1", *xargv, xargc);
          xx = dossharg(x);
          debug(F101, "cmdlin doarg", "", xx);
          debug(F111, "cmdlin args 2", *xargv, xargc);
          if (xx < 0) {
#ifndef NOICP
            if (what == W_COMMAND)
              return (0);
            else
#endif /* NOICP */
            {
              doexit(BAD_EXIT, 1); /* Go handle option */
            }
          }
        } else { /* No dash must be hostname */
          ckstrncpy(ttname, *xargv, TTNAMLEN + 1);
          ssh_set_sparam(SSH_SPARAM_HST, ttname);
          debug(F110, "cmdlin ssh host", ttname, 0);
#ifndef NOICP
#ifndef NODIAL
          nhcount = 0; /* Check network directory */
          debug(F101, "cmdlin nnetdir", "", nnetdir);
          if (nnetdir > 0) /* If there is a directory... */
            lunet(*xargv); /* Look up the name */
          else             /* If no directory */
            nhcount = 0;   /* we didn't find anything there */
#ifdef DEBUG
          if (deblog) {
            debug(F101, "cmdlin lunet nhcount", "", nhcount);
            if (nhcount > 0) {
              debug(F110, "cmdlin lunet nh_p[0]", nh_p[0], 0);
              debug(F110, "cmdlin lunet nh_p2[0]", nh_p2[0], 0);
              debug(F110, "cmdlin lunet nh_px[0][0]", nh_px[0][0], 0);
            }
          }
#endif /* DEBUG */
          /* If network type specified */
          /* it must be TCP/IP */
          if (nhcount > 0 && nh_p2[0])
            if (ckstrcmp(nh_p2[0], "tcp/ip", 6, 0))
              nhcount = 0;
          if (nhcount == 1) { /* Still OK, so make substitution */
            ckstrncpy(ttname, nh_p[0], TTNAMLEN + 1);
            ssh_set_sparam(SSH_SPARAM_HST, ttname);
            debug(F110, "cmdlin lunet substitution", ttname, 0);
          }
#endif /* NODIAL */
#endif /* NOICP */
          /* Service from command line? */
          if (--xargc > 0 && !haveurl) {
            xargv++;
            ckstrncat(ttname, ":", TTNAMLEN + 1);
            ckstrncat(ttname, *xargv, TTNAMLEN + 1);
            ssh_set_sparam(SSH_SPARAM_PRT, *xargv);
            debug(F110, "cmdlin telnet host2", ttname, 0);
          }
          break;
        }
      }
    }
    local = 1; /* Try to open the connection */
    nettype = NET_SSH;
    mdmtyp = -nettype;
    if (ttopen(ttname, &local, mdmtyp, 0) < 0) {
      XFATAL("can't open host connection");
    }
    network = 1; /* It's open */
#ifdef CKLOGDIAL
    dolognet();
#endif /* CKLOGDIAL */
#ifndef NOXFER
    reliable = 1;  /* It's reliable */
    xreliable = 1; /* ... */
    setreliable = 1;
#endif           /* NOXFER */
    cflg = 1;    /* Connect */
    stayflg = 1; /* Stay */
    tn_exit = 1; /* Telnet-like exit condition */
    quiet = 1;
    exitonclose = 1; /* Exit when connection closes */
#ifndef NOSPL
    if (local) {
      if (nmac) {                           /* Any macros defined? */
        int k;                              /* Yes */
        k = mlook(mactab, "on_open", nmac); /* Look this up */
        if (k >= 0) {                       /* If found, */
          if (dodo(k, ttname, 0) > -1)      /* set it up, */
            parser(1);                      /* and execute it */
        }
      }
    }
#endif /* NOSPL */
    return (0);
  }
#endif /* SSHBUILTIN */

  if (howcalled == I_AM_SSHSUB)
    return (0);

  /*
    From here down: We were called as "kermit" or "iksd".

    If we were started directly from a Kermit script file,
    the filename of the script is in argv[1], so skip past it.
  */
  if (xargc > 1) {
    int n = 1;
    if (*xargv[1] != '-') {

#ifdef KERBANG
      /* If we were started with a Kerbang script, the script */
      /* arguments were already picked up in prescan / cmdini() */
      /* and there is nothing here for us anyway. */
      if (!strcmp(xargv[1], "+"))
        return (0);
#endif /* KERBANG */

      if (cfilef) { /* Command file found in prescan() */
        xargc -= n; /* Skip past it */
        xargv += n;
        cfilef = 0;
        debug(F101, "cmdlin cfilef set to 0", "", cfilef);
      }
    }
  }
  /*
    Regular Unix-style command line parser, mostly conforming with 'A Proposed
    Command Syntax Standard for Unix Systems', Hemenway & Armitage, Unix/World,
    Vol.1, No.3, 1984.
  */
  while (--xargc > 0) { /* Go through command line words */
    xargv++;
    debug(F111, "cmdlin xargv", *xargv, xargc);
    if (**xargv == '=')
      return (0);
    if (!strcmp(*xargv, "--")) /* getopt() conformance */
      return (0);
    else if (**xargv == '-') { /* Got an option (begins with dash) */
      int xx;
      x = *(*xargv + 1); /* Get the option letter */
      debug(F111, "cmdlin args 1", *xargv, xargc);
      xx = doarg(x);
      debug(F101, "cmdlin doarg", "", xx);
      debug(F111, "cmdlin args 2", *xargv, xargc);
      if (xx < 0) {
#ifndef NOICP
        if (what == W_COMMAND)
          return (0);
        else
#endif /* NOICP */
        {
          doexit(BAD_EXIT, 1); /* Go handle option */
        }
      }
    } else if (!haveurl) { /* No dash where expected */
      char xbuf[40];       /* enlarged a bit due to warning 20211209 */
      char buf[128];
      int k;
      k = ckstrncpy(xbuf, *xargv, 40);
      if (k > 30) {
        xbuf[30] = '.';
        xbuf[29] = '.';
        xbuf[28] = '.';
      }
      xbuf[31] = NUL;
      ckmakmsg(buf, 128, "invalid command-line option, type \"", myname,
               " -h\" for help", NULL);
      fatal2(xbuf, buf);
    }
  }
#ifdef DEBUG
  if (deblog) {
#ifndef NOICP
    debug(F101, "cmdlin what", "", what);
#endif /* NOICP */
    debug(F101, "cmdlin action", "", action);
#ifndef NOXFER
    debug(F101, "cmdlin stdouf", "", stdouf);
#endif /* NOXFER */
  }
#endif /* DEBUG */

#ifdef NOICP
  if (!action && !cflg && !cnflg) {
    debug(F100, "cmdlin NOICP fatal no action", "", 0);
    XFATAL("?No actions specified on command line");
  }
#else
  if (inserver && what == 0) { /* Internet Kermit server checks */
    if (local || (action != 0 && action != 'x')) {
      if (local)
        printf("local\r\n");
      if (action)
        printf("action=%c\r\n", action);
      debug(F100, "cmdlin fatal 1", "", 0);
      XFATAL("No actions or connections allowed with -A");
    }
  }
#endif /* NOICP */

#ifndef NOLOCAL
  if (!local) {
    if ((action == 'c') || (cflg != 0)) {
      debug(F100, "cmdlin fatal 2", "", 0);
      XFATAL("-l or -j or -X required");
    }
  }
#endif /* NOLOCAL */
#ifndef NOXFER
  if (*cmarg2 != 0) {
    if ((action != 's') && (action != 'r') && (action != 'v')) {
      debug(F100, "cmdlin fatal 3", "", 0);
      XFATAL("-a without -s, -r, or -g");
    }
    if (action == 'r' || action == 'v') {
#ifdef CK_TMPDIR
      if (isdir(cmarg2)) {     /* -a is a directory */
        if (!zchdir(cmarg2)) { /* try to change to it */
          debug(F100, "cmdlin fatal 4", "", 0);
          XFATAL("can't change to '-a' directory");
        } else
          cmarg2 = "";
      } else
#endif /* CK_TMPDIR */
        if (zchko(cmarg2) < 0) {
          debug(F100, "cmdlin fatal 5", "", 0);
          XFATAL("write access to -a file denied");
        }
    }
  }
  if ((action == 'v') && (stdouf) && (!local)) {
    if (is_a_tty(1)) {
      debug(F100, "cmdlin fatal 6", "", 0);
      XFATAL("unredirected -k can only be used in local mode");
    }
  }
  if ((action == 's') || (action == 'v') || (action == 'r') ||
      (action == 'x')) {
    if (local)
      displa = 1;
    if (stdouf) {
      displa = 0;
      quiet = 1;
    }
  }
  if (quiet)
    displa = 0; /* No display if quiet requested */
#endif          /* NOXFER */
#ifdef DEBUG
  if (action)
    debug(F000, "cmdlin returns action", "", action);
  else
    debug(F101, "cmdlin returns action", "", action);
#endif /* DEBUG */

  return (action); /* Then do any requested protocol */
}

/* Extended argument parsing: --keyword[:value] (or =value) */

/*
  XA_xxxx symbols are defined in ckuusr.h.
  If you add a new one, also remember to update doshow(),
  SHXOPT section, in ckuus5.c.
*/
struct keytab xargtab[] = {
#ifdef CK_LOGIN
    {"anonymous", XA_ANON, CM_ARG | CM_PRE},
#endif /* CK_LOGIN */
    {"bannerfile", XA_BAFI, CM_ARG},
    {"cdfile", XA_CDFI, CM_ARG},
    {"cdmessage", XA_CDMS, CM_ARG},
    {"cdmsg", XA_CDMS, CM_ARG | CM_INV},
#ifndef NOCSETS
    {"charset", XA_CSET, CM_ARG | CM_PRE},
#endif /* NOCSETS */
#ifdef IKSDB
    {"database", XA_DBAS, CM_ARG | CM_PRE},
    {"dbfile", XA_DBFI, CM_ARG | CM_PRE},
#endif /* IKSDB */
#ifndef NOLOCAL
#endif /* NOLOCAL */
    {"help", XA_HELP, 0},
#ifndef NOHELP
    {"helpfile", XA_HEFI, CM_ARG},
#endif /* NOHELP */
#ifdef CK_LOGIN
    {"initfile", XA_ANFI, CM_ARG | CM_PRE},
#endif /* CK_LOGIN */
    {"nointerrupts", XA_NOIN, CM_PRE},
    {"nolocale", XA_NOLOCALE, CM_PRE},
    {"noperms", XA_NPRM, 0},
#ifndef NOPUSH
    {"nopush", XA_NOPUSH, CM_PRE},
#endif /* NOPUSH */
#ifdef CK_LOGIN
#ifndef NOXFER
#ifdef CK_PERM
    {"permissions", XA_PERM, CM_ARG | CM_PRE},
    {"perms", XA_PERM, CM_ARG | CM_PRE | CM_INV},
#endif /* CK_PERM */
#endif /* NOXFER */
#ifdef UNIX
    {"privid", XA_PRIV, CM_ARG | CM_PRE},
#endif /* UNIX */
#ifndef NOLOCAL
#ifndef NOCSETS
    {"rcharset", XA_CSET, CM_ARG | CM_PRE | CM_INV},
#endif /* NOCSETS */
#endif /* NOLOCAL */
#ifdef UNIX
    {"root", XA_ROOT, CM_ARG | CM_PRE},
#else /* UNIX */
#ifdef CKROOT
    {"root", XA_ROOT, CM_ARG | CM_PRE},
#endif /* CKROOT */
#endif /* UNIX */
#ifdef CKSYSLOG
    {"syslog", XA_SYSL, CM_ARG | CM_PRE},
#endif /* CKSYSLOG */
#ifndef NOLOCAL
    {"termtype", XA_TERM, CM_ARG | CM_PRE},
#endif /* NOLOCAL */
    {"timeout", XA_TIMO, CM_ARG | CM_PRE},
#ifndef NOLOCAL
#ifdef UNIX
    {"unbuffered", XA_UNBUF, 0},
#endif /* UNIX */
#ifndef NOSPL
    {"user", XA_USER, CM_ARG},
#endif /* NOSPL */
#endif /* NOLOCAL */
    {"userfile", XA_USFI, CM_ARG | CM_PRE},
    {"version", XA_VERS, 0},
#ifndef NOLOCAL
#endif /* NOLOCAL */
#ifdef CKWTMP
    {"wtmpfile", XA_WTFI, CM_ARG | CM_PRE},
    {"wtmplog", XA_WTMP, CM_ARG | CM_PRE},
#endif /* CKWTMP */
#endif /* CK_LOGIN */
    {"xferfile", XA_IKFI, CM_ARG | CM_PRE},
    {"xferlog", XA_IKLG, CM_ARG | CM_PRE},
#ifndef NOLOCAL
#endif /* NOLOCAL */
    {"", 0, 0}};
int nxargs = sizeof(xargtab) / sizeof(struct keytab) - 1;

static struct keytab oktab[] = {
    {"0", 0, 0},     {"1", 1, 0},  {"2", 2, 0},   {"3", 3, 0},  {"4", 4, 0},
    {"5", 5, 0},     {"6", 6, 0},  {"7", 7, 0},   {"8", 8, 0},  {"9", 9, 0},
    {"false", 0, 0}, {"no", 0, 0}, {"off", 0, 0}, {"ok", 1, 0}, {"on", 1, 0},
    {"true", 1, 0},  {"yes", 1, 0}};
static int noktab = sizeof(oktab) / sizeof(struct keytab);

#define XARGBUFL 32

char *xopthlp[XA_MAX + 1]; /* Extended option help */
char *xarghlp[XA_MAX + 1]; /* Extended argument for option */

static void inixopthlp() {
  int i, j;
  for (i = 0; i <= XA_MAX; i++) { /* Initialize all to null */
    xopthlp[i] = NULL;
    xarghlp[i] = NULL;
  }
  for (i = 0; i < nxargs; i++) { /* Then for each defined keyword */
    j = xargtab[i].kwval;        /* index by associated value */
    if (j < 0 || j > XA_MAX)
      continue;
    switch (j) {
#ifdef CK_LOGIN
    case XA_ANON: /* "--anonymous" */
      xopthlp[j] = "--anonymous:{on,off} [IKSD only]";
      xarghlp[j] = "Whether to allow anonymous IKSD logins";
      break;
#ifdef UNIX
    case XA_PRIV:
      xopthlp[j] = "--privid:{on,off} [IKSD only]";
      xarghlp[j] = "Whether to allow privileged IDs to login to IKSD";
      break;
#endif            /* UNIX */
#endif            /* CK_LOGIN */
    case XA_BAFI: /* "--bannerfile" */
      xopthlp[j] = "--bannerfile:<filename>";
      xarghlp[j] = "File to display upon startup or IKSD login";
      break;
    case XA_CDFI: /* "--cdfile" */
      xopthlp[j] = "--cdfile:<filename>";
      xarghlp[j] = "File to display when server changes directory";
      break;
    case XA_CDMS: /* "--cdmessage" */
      xopthlp[j] = "--cdmessage:{on,off}";
      xarghlp[j] = "Whether to display CD message file";
      break;
    case XA_HELP: /* "--help" */
      xopthlp[j] = "--help";
      xarghlp[j] = "Print this help text about extended options";
      break;
    case XA_HEFI: /* "--help" */
      xopthlp[j] = "--helpfile:<filename>";
      xarghlp[j] = "File containing custom info for HELP command";
      break;
    case XA_IKFI: /* "--xferfile" */
      xopthlp[j] = "--xferfile:<filename> [IKSD only]";
      xarghlp[j] = "Name of ftpd-like logfile.";
      break;
    case XA_IKLG: /* "--xferlog" */
      xopthlp[j] = "--xferlog:{on,off} [IKSD only]";
      xarghlp[j] = "Whether to keep an ftpd-like logfile.";
      break;
#ifdef CK_LOGIN
    case XA_ANFI: /* "--initfile" */
      xopthlp[j] = "--initfile:<filename> [IKSD only]";
      xarghlp[j] = "Initialization file for anonymous users.";
      break;
#ifdef CK_PERM
    case XA_PERM: /* "--permissions" */
      xopthlp[j] = "--permissions:<octalnum> [IKSD only]";
      xarghlp[j] = "Permissions for files uploaded by anonymous users.";
      break;
#endif /* CK_PERM */
#ifdef UNIX
    case XA_ROOT: /* "--root" */
      xopthlp[j] = "--root:<directory> [IKSD only]";
      xarghlp[j] = "File-system root for anonymous users.";
      break;
#else /* UNIX */
#ifdef CKROOT
    case XA_ROOT: /* "--root" */
      xopthlp[j] = "--root:<directory> [IKSD only]";
      xarghlp[j] = "File-system root for anonymous users.";
      break;
#endif /* CKROOT */
#endif /* UNIX */
#endif /* CK_LOGIN */
#ifdef CKSYSLOG
    case XA_SYSL: /* "--syslog" */
      xopthlp[j] = "--syslog:<digit> [IKSD only]";
      xarghlp[j] = "Syslog recording level, 0-6.";
      break;
#endif            /* CKSYSLOG */
    case XA_USFI: /* "--userfile" */
      xopthlp[j] = "--userfile:<filename> [IKSD only]";
      xarghlp[j] = "Forbidden user file.";
      break;
#ifdef CKWTMP
    case XA_WTFI: /* "--wtmpfile" */
      xopthlp[j] = "--wtmpfile:<filename> [IKSD only]";
      xarghlp[j] = "Name of wtmp logfile.";
      break;
    case XA_WTMP: /* "--wtmplog" */
      xopthlp[j] = "--wtmplog:{on,off} [IKSD only]";
      xarghlp[j] = "Whether to keep a wtmp logfile.";
      break;
#endif /* CKWTMP */
#ifdef CK_LOGIN
    case XA_TIMO: /* "--timeout" */
      xopthlp[j] = "--timeout:<seconds> [IKSD only]";
      xarghlp[j] = "How long to wait for login before closing the connection.";
      break;
#endif /* CK_LOGIN */
    case XA_NOIN:
      xopthlp[j] = "--nointerrupts";
      xarghlp[j] = "Disable keyboard interrupts.";
      break;
#ifdef UNIX
    case XA_UNBUF:
      xopthlp[j] = "--unbuffered";
      xarghlp[j] = "Force unbuffered console i/o.";
      break;
#endif /* UNIX */
#ifdef IKSDB
    case XA_DBAS:
      xopthlp[j] = "--database:{on,off}";
      xarghlp[j] = "Enable/Disable IKSD database (IKSD only)";
      break;
    case XA_DBFI:
      xopthlp[j] = "--dbfile:<filename>";
      xarghlp[j] = "Specify IKSD database file (IKSD only)";
      break;
#endif /* IKSDB */
#ifdef CK_PERMS
    case XA_NPRM:
      xopthlp[j] = "--noperms";
      xarghlp[j] = "Disable file-transfer Permissions attribute.";
      break;
#endif /* CK_PERMS */
#ifndef NOPUSH
    case XA_NOPUSH:
      xopthlp[j] = "--nopush";
      xarghlp[j] = "Disable external command execution.";
      break;
#endif /* NOPUSH */
    case XA_CSET:
      xopthlp[j] = "--rcharset:name";
      xarghlp[j] = "Name of remote terminal character set.";
      break;
    case XA_TERM:
      xopthlp[j] = "--termtype:name";
      xarghlp[j] = "Choose terminal type.";
      break;
    case XA_USER:
      xopthlp[j] = "--user:name";
#ifndef NETCONN
      xarghlp[j] = "Username (for network login)";
#else
      xarghlp[j] = "Username.";
#endif /* NETCONN */
      break;
#ifdef HAVE_LOCALE
    case XA_NOLOCALE:
      xopthlp[j] = "--nolocale";
      xarghlp[j] = "Disable use of locale for messages and strings.";
      break;
#endif /* HAVE_LOCALE */
    }
  }
}

void iniopthlp() {
  int i;
  for (i = 0; i < 128; i++) {
    optact[i] = 0;
    switch (i) {
    case '0': /* In the middle */
      opthlp[i] =
          "100% transparent CONNECT mode for \"in-the-middle\" operation";
      arghlp[i] = NULL;
      break;

    case '8':
      opthlp[i] = "Connection is 8-bit clean";
      arghlp[i] = NULL;
      break;

#ifdef NEWFTP
    case '9':
      opthlp[i] = "Make a connection to an FTP server";
      arghlp[i] = "IP-address-or-hostname[:optional-TCP-port]";
      break;
#endif /* NEWFTP */

#ifdef IKSD
    case 'A':
      opthlp[i] = "Kermit is to be started as an Internet service";
      arghlp[i] = NULL;
      break;
#endif /* IKSD */
    case 'B':
      opthlp[i] =
          "Kermit is running in Batch or Background (no controlling terminal)";
      break;
#ifndef NOSPL
    case 'C':
      opthlp[i] = "Interactive-mode Commands to be executed";
      arghlp[i] = "Commands separated by commas, list in doublequotes";
      break;
#endif /* NOSPL */
    case 'D':
      opthlp[i] = "Delay before starting to send";
      arghlp[i] = "Number of seconds";
      break;
    case 'E':
      opthlp[i] = "Exit automatically when connection closes";
      arghlp[i] = NULL;
      break;
#ifdef TCPSOCKET
    case 'F':
      opthlp[i] = "Use an existing TCP connection";
      arghlp[i] = "Numeric file descriptor of open TCP connection";
      break;
#endif /* TCPSOCKET */
    case 'G':
      opthlp[i] = "GET from server, send to standard output";
      arghlp[i] = "Remote file specification";
      optact[i] = 1;
      break;
    case 'H':
      opthlp[i] = "Suppress program startup Herald and greeting";
      arghlp[i] = NULL;
      break;
    case 'I':
      opthlp[i] = "Connection is reliable, streaming is allowed";
      arghlp[i] = NULL;
      break;
#ifdef TCPSOCKET
    case 'J':
      opthlp[i] = "'Be like Telnet'";
      arghlp[i] = "IP hostname/address optionally followed by service";
      break;
#endif /* TCPSOCKET */
    case 'L':
      opthlp[i] = "Recursive directory descent for files in -s option";
      arghlp[i] = NULL;
      break;
    case 'M':
      opthlp[i] = "My user name (for use with Telnet, Rlogin, etc)";
      arghlp[i] = "Username string";
      break;
#ifdef NETBIOS
    case 'N':
      opthlp[i] = "NETBIOS adapter number";
      arghlp[i] = "Number";
      break;
#endif        /* NETBIOS */
    case 'O': /* Be a server for One command only */
      opthlp[i] = "Be a server for One command only";
      arghlp[i] = NULL;
      optact[i] = 1;
      break;
    case 'P':
      opthlp[i] = "Don't convert file (Path) names";
      arghlp[i] = NULL;
      break;
    case 'Q':
      opthlp[i] = "Quick (FAST) Kermit protocol settings";
      arghlp[i] = NULL;
      break;
    case 'R': /* Remote-Only */
      opthlp[i] = "Remote-only (makes IF REMOTE true)";
      arghlp[i] = NULL;
      break;
    case 'S': /* "Stay" - enter interactive */
      opthlp[i] = "Stay (enter command parser after action options)";
      arghlp[i] = NULL;
      break;
    case 'T': /* Text file transfer mode */
      opthlp[i] = "Transfer files in Text mode";
      arghlp[i] = NULL;
      break;
#ifdef ANYX25
    case 'U': /* X.25 call user data */
      opthlp[i] = "X.25 call User data";
      arghlp[i] = "Call-user-data string";
      break;
#endif        /* ANYX25 */
    case 'V': /* No automatic filetype switching */
      opthlp[i] = "Disable automatic per-file text/binary switching";
      arghlp[i] = NULL;
      break;
#ifdef ANYX25
    case 'X': /* SET HOST to X.25 address */
      opthlp[i] = "Make an X.25 connection";
      arghlp[i] = "X.25 or X.121 address";
      break;
#endif        /* ANYX25 */
    case 'Y': /* No initialization file */
      opthlp[i] = "Skip initialization file";
      arghlp[i] = NULL;
      break;
#ifdef ANYX25
    case 'Z': /* SET HOST to X.25 file descriptor */
      opthlp[i] = "Make an X.25 connection";
      arghlp[i] = "Numeric file descriptor of open X.25 connection";
      break;
#endif        /* ANYX25 */
    case 'a': /* as-name */
      opthlp[i] = "As-name for file(s) in -s, -r, or -g";
      arghlp[i] = "As-name string (alternative filename)";
      break;
    case 'b': /* Set bits-per-second for serial */
      opthlp[i] = "Speed for serial device";
      arghlp[i] = "Numeric Bits per second";
      break;
    case 'c': /* Connect before */
      optact[i] = 1;
      opthlp[i] = "CONNECT before transferring files";
      arghlp[i] = NULL;
      break;
    case 'd': /* DEBUG */
      opthlp[i] = "Create debug.log file (a second -d adds timestamps)";
      arghlp[i] = NULL;
      break;
    case 'e': /* Extended packet length */
      opthlp[i] = "Maximum length for incoming file-transfer packets";
      arghlp[i] = "Length in bytes";
      break;
    case 'f': /* finish */
      optact[i] = 1;
      opthlp[i] = "Send Finish command to a Kermit server";
      arghlp[i] = NULL;
      break;
    case 'g': /* get */
      optact[i] = 1;
      opthlp[i] = "GET file(s) from a Kermit server";
      arghlp[i] = "Remote file specification";
      break;
    case 'h': /* help */
      optact[i] = 1;
      opthlp[i] = "Print this message (pipe thru 'more' to prevent scrolling)";
      arghlp[i] = NULL;
      break;
    case 'i': /* Treat files as binary */
      opthlp[i] = "Transfer files in binary mode";
      arghlp[i] = NULL;
      break;
#ifdef TCPSOCKET
    case 'j': /* SET HOST (TCP/IP socket) */
      opthlp[i] = "Make a TCP connection";
      arghlp[i] = "TCP host name/address and optional service name or number";
      break;
#endif        /* TCPSOCKET */
    case 'k': /* receive to stdout */
      optact[i] = 1;
      opthlp[i] = "RECEIVE file(s) to standard output";
      arghlp[i] = NULL;
      break;
    case 'l': /* SET LINE */
      opthlp[i] = "Make connection on serial communications device";
      arghlp[i] = "Serial device name";
      break;
    case 'm': /* Modem type */
      opthlp[i] = "Modem type for use with -l device";
      arghlp[i] = "Modem name as in SET MODEM TYPE command";
      break;
    case 'n': /* connect after */
      optact[i] = 1;
      opthlp[i] = "CONNECT after transferring files";
      arghlp[i] = NULL;
      break;
#ifdef ANYX25
    case 'o': /* X.25 closed user group */
      opthlp[i] = "X.25 closed user group";
      arghlp[i] = "User group string";
      break;
#endif        /* ANYX25 */
    case 'p': /* SET PARITY */
      opthlp[i] = "Parity";
      arghlp[i] = "One of the following: even, odd, mark, none, space";
      break;
    case 'q': /* Quiet */
      opthlp[i] = "Quiet (suppress most messages)";
      arghlp[i] = NULL;
      break;
    case 'r': /* receive */
      optact[i] = 1;
      opthlp[i] = "RECEIVE file(s)";
      arghlp[i] = NULL;
      break;
    case 's': /* send */
      optact[i] = 1;
      opthlp[i] = "SEND file(s)";
      arghlp[i] = "One or more file specifications";
      break;
    case 't': /* Line turnaround handshake */
      opthlp[i] = "XON Turnaround character for half-duplex connections";
      arghlp[i] = NULL;
      break;
#ifdef ANYX25
    case 'u': /* X.25 reverse charge call */
      opthlp[i] = "X.25 reverse charge call";
      arghlp[i] = NULL;
      break;
#endif        /* ANYX25 */
    case 'v': /* Vindow size */
      opthlp[i] = "Window size";
      arghlp[i] = "Number, 1 to 32";
      break;
    case 'w': /* Writeover */
      opthlp[i] = "Incoming files Write over existing files";
      arghlp[i] = NULL;
      break;
    case 'x': /* Server */
      optact[i] = 1;
      opthlp[i] = "Be a Kermit SERVER";
      arghlp[i] = NULL;
      break;
    case 'y': /* Alternate init-file name */
      opthlp[i] = "Alternative initialization file";
      arghlp[i] = "File specification";
      break;
    case 'z': /* Not background */
      opthlp[i] = "Force foreground behavior";
      arghlp[i] = NULL;
      break;
    default:
      opthlp[i] = NULL;
      arghlp[i] = NULL;
    }
  }
  inixopthlp();
}

#ifndef NOICP
int doxarg(char **s, int pre) {
#ifdef IKSD
#ifdef CK_LOGIN
  extern int ckxsyslog, ckxwtmp, ckxanon;
#ifdef UNIX
  extern int ckxpriv;
#endif /* UNIX */
#ifdef CK_PERMS
  extern int ckxperms;
#endif /* CK_PERMS */
  extern char *anonfile, *userfile, *anonroot;
#endif /* CK_LOGIN */
#ifdef CKWTMP
  extern char *wtmpfile;
#endif /* CKWTMP */
#endif /* IKSD */
  extern int srvcdmsg;
  extern char *cdmsgfile[], *cdmsgstr;
  char tmpbuf[CKMAXPATH + 1];

  int i, x, y, z, havearg = 0;
  char buf[XARGBUFL], c, *p;

  if (nxargs < 1)
    return (-1);

  c = *(*s + 1); /* Hyphen or Plus sign */

  p = *s + 2;
  for (i = 0; *p && i < XARGBUFL; i++) {
    buf[i] = *p++;
    if (buf[i] == '=' || buf[i] == ':') {
      havearg = 1;
      buf[i] = NUL;
      break;
    } else if (buf[i] < ' ') {
      buf[i] = NUL;
      break;
    }
  }
  if (i > XARGBUFL - 1)
    return (-1);
  buf[i] = NUL;

  x = lookup(xargtab, buf, nxargs, &z); /* Lookup the option keyword */

  if (x < 0)     /* On any kind of error */
    return (-1); /* fail. */

  /* Handle prescan versus post-initialization file */

  if (((xargtab[z].flgs & CM_PRE) || (c == '+')) && !pre)
    return (0);
  else if (pre && !(xargtab[z].flgs & CM_PRE) && (c != '+'))
    return (0);

  /* Ensure that argument is given if and only if required */

  p = havearg ? *s + i + 3 : NULL;

  if ((xargtab[z].flgs & CM_ARG) && !havearg)
    return (-1);
  else if ((!(xargtab[z].flgs & CM_ARG)) && havearg)
    return (-1);

  switch (x) { /* OK to process this option... */
#ifdef CKSYSLOG
  case XA_SYSL: /* IKS: Syslog level */
    y = 0;
    if (isdigit(*p)) {
      while (*p) {
        if (*p < '0' || *p > '9')
          return (-1);
        y = y * 10 + (*p++ - '0');
      }
    } else {
      y = lookup(oktab, p, noktab, &z);
      if (y > 0)
        y = SYSLG_DF; /* Yes = default logging level */
    }
#ifndef SYSLOGLEVEL
    /* If specified on cc command line, user can't change it. */
    if (!inserver)      /* Don't allow voluminous syslogging */
      if (y > SYSLG_FA) /* by ordinary users. */
        y = SYSLG_FA;
#endif /* SYSLOGLEVEL */
    if (y < 0)
      return (-1);
#ifdef DEBUG
    if (y >= SYSLG_DB)
      if (!deblog)
        deblog = debopn("debug.log", 0);
#endif /* DEBUG */
#ifdef SYSLOGLEVEL
    /* If specified on cc command line, user can't change it. */
    y = SYSLOGLEVEL;
#endif /* SYSLOGLEVEL */
    ckxsyslog = y;
    /* printf("ckxsyslog=%d\n",ckxsyslog); */
    break;
#endif /* CKSYSLOG */

#ifdef CK_LOGIN
#ifdef CKWTMP
  case XA_WTMP: /* IKS: wtmp log */
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    ckxwtmp = y;
    /* printf("ckxwtmp=%d\n",ckxwtmp); */
    break;

  case XA_WTFI: /* IKS: wtmp logfile */
    if (zfnqfp(p, CKMAXPATH, tmpbuf))
      p = tmpbuf;
    makestr(&wtmpfile, p);
    /* printf("wtmpfile=%s\n",wtmpfile); */
    break;
#endif /* CKWTMP */

#ifndef NOIKSD
  case XA_ANON: /* IKS: Anonymous login allowed */
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    ckxanon = y;
    /* printf("ckxanon=%d\n",ckxanon); */
    break;

#ifdef UNIX
  case XA_PRIV: /* IKS: Priv'd login allowed */
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    ckxpriv = y;
    /* printf("ckxpriv=%d\n",ckxpriv); */
    break;
#endif /* UNIX */
#endif /* NOIKSD */

#ifndef NOIKSD
#ifdef CK_PERMS
  case XA_PERM: /* IKS: Anonymous Upload Permissions */
    y = 0;
    while (*p) {
      if (*p < '0' || *p > '7')
        return (-1);
      y = y * 8 + (*p++ - '0');
    }
    ckxperms = y;
    /* printf("ckxperms=%04o\n",ckxperms); */
    break;
#endif /* CK_PERMS */

  case XA_ANFI: /* Anonymous init file */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&anonfile, p);
    /* printf("anonfile=%s\n",anonfile); */
    break;

  case XA_USFI: /* IKS: Forbidden user file */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&userfile, p);
    /* printf("userfile=%s\n",userfile); */
    break;

  case XA_ROOT: /* IKS: Anonymous root */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&anonroot, p);
    /* printf("anonroot=%s\n",anonroot); */
    break;
#endif /* NOIKSD */
#endif /* CK_LOGIN */

  case XA_CDFI: /* CD filename */
    makelist(p, cdmsgfile, 16);
    makestr(&cdmsgstr, p);
    /* printf("cdmsgstr=%s\n",cdmsgstr); */
    break;

  case XA_CDMS: /* CD messages */
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    srvcdmsg = y;
    /* printf("srvcdmsg=%d\n",srvcdmsg); */
    break;

#ifndef NOXFER
  case XA_IKLG: /* Transfer log on/off */
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    xferlog = y;
    /* printf("xferlog=%d\n",xferlog); */
    break;

  case XA_IKFI: /* Transfer log file */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&xferfile, p);
    xferlog = 1;
    /* printf("xferfile=%s\n",xferfile); */
    break;

  case XA_BAFI: /* IKS: banner (greeting) file */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&bannerfile, p);
    /* printf("bannerfile=%s\n",bannerfile); */
    break;
#endif /* NOXFER */

#ifndef NOHELP
  case XA_HELP: /* Help */
    /* printf("help\n"); */
    for (i = 0; i <= XA_MAX; i++)
      if (xopthlp[i])
        printf("%s\n   %s\n\n", xopthlp[i], xarghlp[i]);
    if (stayflg || what == W_COMMAND)
      break;
    else
      doexit(GOOD_EXIT, -1);
#endif /* NOHELP */

#ifndef NOHELP
  case XA_HEFI: /* IKS: custom help file */
    if (!isabsolute(p))
      if (zfnqfp(p, CKMAXPATH, tmpbuf))
        p = tmpbuf;
    makestr(&helpfile, p);
    /* printf("helpfile=%s\n",helpfile); */
    break;
#endif /* NOHELP */

#ifdef CK_LOGIN
  case XA_TIMO:
    if (!rdigits(p))
      return (-1);
    logintimo = atoi(p);
    /* printf("logintimo=%d\n",p); */
    break;
#endif /* CK_LOGIN */

  case XA_NOIN: /* No interrupts */
#ifndef NOICP
    cmdint = 0;
#endif /* NOICP */
    xsuspend = 0;
    break;

#ifdef UNIX
  case XA_UNBUF: /* Unbuffered console i/o*/
    break;       /* This one is handled in ckcmai.c */
#endif           /* UNIX */

#ifdef IKSDB
  case XA_DBFI: {
    extern char *dbdir, *dbfile;
    extern int dbenabled;
    struct zfnfp *zz;
    if ((zz = zfnqfp(p, CKMAXPATH, tmpbuf))) {
      char *s, *s2 = NULL;
      makestr(&dbdir, zz->fpath);
      makestr(&dbfile, zz->fpath);
      for (s = dbdir; *s; s++) {
        if (ISDIRSEP(*s))
          s2 = s + 1;
      }
      if (s2)
        *s2 = NUL;
      debug(F110, "XA_DBFI dbdir", dbdir, 0);
      debug(F110, "XA_DBFI dbfile", dbfile, 0);
      dbenabled = 1;
    }
    break;
  }
  case XA_DBAS: {
    extern int dbenabled;
    y = lookup(oktab, p, noktab, &z);
    if (y < 0)
      return (-1);
    dbenabled = y;
    break;
  }
#endif /* IKSDB */

  case XA_VERS: {
    extern char *ck_s_ver;
    printf("%s", ck_s_ver);
    printf("\n");
    if (stayflg || what == W_COMMAND)
      break;
    else
      doexit(GOOD_EXIT, -1);
  }
#ifndef NOXFER
#ifdef CK_PERMS
  case XA_NPRM: {
    extern int atlpri, atlpro, atgpri, atgpro;
    atlpri = 0;
    atlpro = 0;
    atgpri = 0;
    atgpro = 0;
    break;
  }
#endif /* CK_PERMS */
#endif /* NOXFER */

#ifndef NOPUSH
  case XA_NOPUSH:
    nopush = 1;
    break;
#endif /* NOPUSH */

#ifndef NOLOCAL
  case XA_TERM: { /* Terminal type */
    extern struct keytab ttyptab[];
    extern int nttyp;
#ifdef TNCODE
    extern char *tn_term;
#endif /* TNCODE */
#ifdef TNCODE
    makestr(&tn_term, p);
#endif /* TNCODE */
    break;
  }
  case XA_CSET: { /* Remote Character Set */
#ifndef NOCSETS
#ifdef CKOUNI
    extern struct keytab txrtab[];
    extern int ntxrtab;
    x = lookup(txrtab, p, ntxrtab, &z);
#else  /* CKOUNI */
    extern struct keytab ttcstab[];
    extern int ntermc;
    x = lookup(ttcstab, p, ntermc, &z);
#endif /* CKOUNI */
    if (x < 0)
      return (-1);
    setremcharset(z, 4 /* TT_GR_ALL (in ckuus7.c) */);
#else  /* NOCSETS */
    return (-1);
#endif /* NOCSETS */
    break;
  }
  case XA_ROWS: { /* Screen rows (height) */
    extern int tt_rows;
    if (!rdigits(p))
      return (-1);
    tt_rows = atoi(p);
    break;
  }
  case XA_COLS: { /* Screen columns (width) */
    extern int tt_cols;
    if (!rdigits(p))
      return (-1);
    tt_cols = atoi(p);
    break;
  }

#ifndef NOSPL
  case XA_USER: /* Username for login */
#ifdef IKSD
    if (!inserver)
#endif /* IKSD */
    {
      ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
      haveftpuid = 1;
    }
    break;
#endif /* NOSPL */
#endif /* NOLOCAL */

  case XA_NOLOCALE: { /* Don't do locale */
    extern int nolocale;
    nolocale = 1;
    break;
  }
  default:
    return (-1);
  }
  return (0);
}
#endif /* NOICP */

#ifdef IKSD
#ifdef IKSDCONF
#define IKS_ANON 0
#define IKS_BAFI 1
#define IKS_CDFI 2
#define IKS_CDMS 3
#define IKS_HEFI 4
#define IKS_ANFI 5
#define IKS_USFI 6
#define IKS_IKLG 7
#define IKS_IKFI 8
#define IKS_DBAS 9
#define IKS_DBFI 10
#define IKS_PERM 11
#define IKS_PRIV 12
#define IKS_ROOT 13
#define IKS_TIMO 14
#define IKS_WTFI 15
#define IKS_WTMP 16
#define IKS_SRVR 17
#define IKS_NOIN 18
#define IKS_INIT 19
#define IKS_ANLG 20
#define IKS_ACCT 21
#define IKS_NTDOM 22
#define IKS_SYSL 23

#ifdef CK_LOGIN
static struct keytab iksantab[] = {{"initfile", IKS_ANFI, 0},
                                   {"login", IKS_ANLG, 0},
#ifdef UNIX
                                   {"root", IKS_ROOT, 0},
#else
#ifdef CKROOT
                                   {"root", IKS_ROOT, 0},
#endif /* CKROOT */
#endif /* UNIX */
                                   {"", 0, 0}};
static int niksantab = sizeof(iksantab) / sizeof(struct keytab) - 1;
#endif /* CK_LOGIN */

static struct keytab ikstab[] = {
#ifdef CK_LOGIN
    {"anonymous", IKS_ANON, 0},
#endif /* CK_LOGIN */
    {"bannerfile", IKS_BAFI, 0},  {"cdfile", IKS_CDFI, 0},
    {"cdmessage", IKS_CDMS, 0},   {"cdmsg", IKS_CDMS, CM_INV},
#ifdef IKSDB
    {"database", IKS_DBAS, 0},    {"dbfile", IKS_DBFI, 0},
#endif /* IKSDB */
#ifdef CK_LOGIN
#endif /* CK_LOGIN */
#ifndef NOHELP
    {"helpfile", IKS_HEFI, 0},
#endif /* NOHELP */
    {"initfile", IKS_INIT, 0},    {"no-initfile", IKS_NOIN, 0},
#ifdef CK_LOGIN
#ifdef CK_PERM
    {"permissions", IKS_PERM, 0}, {"perms", IKS_PERM, CM_INV},
#endif /* CK_PERM */
#ifdef UNIX
    {"privid", IKS_PRIV, 0},
#endif /* UNIX */
    {"server-only", IKS_SRVR, 0},
#ifdef CKSYSLOG
    {"syslog", IKS_SYSL, 0},
#endif /* CKSYSLOG */
    {"timeout", IKS_TIMO, 0},     {"userfile", IKS_USFI, 0},
#ifdef CKWTMP
    {"wtmpfile", IKS_WTFI, 0},    {"wtmplog", IKS_WTMP, 0},
#endif /* CKWTMP */
#endif /* CK_LOGIN */
    {"xferfile", IKS_IKFI, 0},    {"xferlog", IKS_IKLG, 0}};
static int nikstab = sizeof(ikstab) / sizeof(struct keytab);
#endif /* IKSDCONF */

#ifndef NOICP
int setiks() { /* SET IKS */
#ifdef IKSDCONF
#ifdef CK_LOGIN
  extern int ckxsyslog, ckxwtmp, ckxanon;
#ifdef UNIX
  extern int ckxpriv;
#endif /* UNIX */
#ifdef CK_PERMS
  extern int ckxperms;
#endif /* CK_PERMS */
  extern char *anonfile, *userfile, *anonroot;
#endif /* CK_LOGIN */
#ifdef CKWTMP
  extern char *wtmpfile;
#endif /* CKWTMP */
  extern int srvcdmsg, success, iksdcf, rcflag, noinit, arg_x;
  extern char *cdmsgfile[], *cdmsgstr, *kermrc;
  extern xx_strp xxstring;
  int x, y, z;
  char *s;
  char tmpbuf[CKMAXPATH + 1];

  if ((y = cmkey(ikstab, nikstab, "", "", xxstring)) < 0)
    return (y);

#ifdef CK_LOGIN
  if (y == IKS_ANON) {
    if ((y = cmkey(iksantab, niksantab, "", "", xxstring)) < 0)
      return (y);
  }
#endif /* CK_LOGIN */

  switch (y) {
#ifdef CKSYSLOG
  case IKS_SYSL: /* IKS: Syslog level */
    if ((z = cmkey(oktab, noktab, "", "", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
#ifndef SYSLOGLEVEL
    /* If specified on cc command line, user can't change it. */
    if (!inserver)      /* Don't allow voluminous syslogging */
      if (y > SYSLG_FA) /* by ordinary users. */
        y = SYSLG_FA;
#endif /* SYSLOGLEVEL */
    if (y < 0)
      return (-1);
#ifdef DEBUG
    if (y >= SYSLG_DB)
      if (!deblog)
        deblog = debopn("debug.log", 0);
#endif /* DEBUG */
#ifdef SYSLOGLEVEL
    /* If specified on cc command line, user can't change it. */
    y = SYSLOGLEVEL;
#endif /* SYSLOGLEVEL */
    ckxsyslog = y;
    /* printf("ckxsyslog=%d\n",ckxsyslog); */
    break;
#endif /* CKSYSLOG */

#ifdef CK_LOGIN
  case IKS_ANLG:
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    ckxanon = z;
    break;
#endif /* CK_LOGIN */
  case IKS_BAFI:
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    debug(F110, "bannerfile before zfnqfp()", s, 0);
    if (zfnqfp(s, CKMAXPATH, tmpbuf)) {
      debug(F110, "bannerfile after zfnqfp()", tmpbuf, 0);
      s = tmpbuf;
    }
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (*s)
      makestr(&bannerfile, s);
    break;
  case IKS_CDFI:
    if ((z = cmtxt("list of cd message file names", "READ.ME", &s, xxstring)) <
        0)
      return (z);
    if (iksdcf)
      return (success = 0);
    if (*s) {
      makelist(s, cdmsgfile, 16);
      makestr(&cdmsgstr, s);
    }
    break;
  case IKS_CDMS:
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    srvcdmsg = z;
    break;
  case IKS_HEFI:
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    if (zfnqfp(s, CKMAXPATH, tmpbuf))
      s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (*s)
      makestr(&helpfile, s);
    break;
  case IKS_ANFI:
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    if (zfnqfp(s, CKMAXPATH, tmpbuf))
      s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
#ifdef CK_LOGIN
    if (*s)
      makestr(&anonfile, s);
#endif /* CK_LOGIN */
    break;
  case IKS_USFI:
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    if (zfnqfp(s, CKMAXPATH, tmpbuf))
      s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
#ifdef CK_LOGIN
    if (*s)
      makestr(&userfile, s);
#endif /* CK_LOGIN */
    break;
  case IKS_IKFI:
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    if (zfnqfp(s, CKMAXPATH, tmpbuf))
      s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (*s) {
      makestr(&xferfile, s);
      xferlog = 1;
    }
    break;
  case IKS_IKLG:
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    xferlog = z;
    break;

#ifdef CK_LOGIN
#ifdef CK_PERM
  case IKS_PERM:
    if ((z = cmtxt("Octal file permssion code", "000", &s, xxstring)) < 0)
      return (z);
    if (z < 0)
      return (z);
    if (iksdcf)
      return (success = 0);
    y = 0;
    while (*s) {
      if (*s < '0' || *s > '7')
        return (-9);
      y = y * 8 + (*s++ - '0');
    }
    ckxperms = y;
    break;
#endif /* CK_PERM */
#ifdef UNIX
  case IKS_PRIV: /* IKS: Priv'd login allowed */
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    ckxpriv = z;
    break;
#endif /* UNIX */

  case IKS_ROOT: /* IKS: Anonymous root */
    if ((z = cmdir("Name of disk and/or directory", "", &s, xxstring)) < 0) {
      if (z != -3)
        return (z);
    }
    if (*s) {
      if (zfnqfp(s, CKMAXPATH, tmpbuf))
        s = tmpbuf;
    } else
      s = "";
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (*s)
      makestr(&anonroot, s);
    /* printf("anonroot=%s\n",anonroot); */
    break;

  case IKS_TIMO:
    z = cmnum("login timeout, seconds", "0", 10, &x, xxstring);
    if (z < 0)
      return (z);
    if (x < 0 || x > 7200) {
      printf("?Value must be between 0 and 7200\r\n");
      return (-9);
    }
    if ((z = cmcfm()) < 0)
      return (z);
    if (iksdcf)
      return (success = 0);
    logintimo = x;
    break;

#ifdef CKWTMP
  case IKS_WTMP: /* IKS: wtmp log */
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    ckxwtmp = z;
    break;

  case IKS_WTFI: /* IKS: wtmp logfile */
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    if (zfnqfp(s, CKMAXPATH, tmpbuf))
      s = tmpbuf;
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (*s)
      makestr(&wtmpfile, s);
    break;
#endif /* CKWTMP */
#endif /* CK_LOGIN */
#ifdef IKSDB
  case IKS_DBFI: {
    extern char *dbdir, *dbfile;
    extern int dbenabled;
    struct zfnfp *zz;
    if ((z = cmifi("Filename", "", &s, &x, xxstring)) < 0)
      return (z);
    if (x) {
      printf("?Wildcards not allowed\n");
      return (-9);
    }
    zz = zfnqfp(s, CKMAXPATH, tmpbuf);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    if (zz) {
      makestr(&dbdir, zz->fpath);
      makestr(&dbfile, (char *)tmpbuf);
      dbenabled = 1;
    } else
      return (success = 0);
    break;
  }
  case IKS_DBAS: {
    extern int dbenabled;
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    dbenabled = z;
    break;
  }
#endif /* IKSDB */

  case IKS_INIT:
    if ((z = cmtxt("Alternate init file specification", "", &s, xxstring)) < 0)
      return (z);
    if (z < 0)
      return (z);
    if (iksdcf)
      return (success = 0);
    ckstrncpy(kermrc, s, KERMRCL);
    rcflag = 1; /* Flag that this has been done */
    break;

  case IKS_NOIN:
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    noinit = z;
    break;

  case IKS_SRVR:
    if ((z = cmkey(oktab, noktab, "", "no", xxstring)) < 0)
      return (z);
    if ((x = cmcfm()) < 0)
      return (x);
    if (iksdcf)
      return (success = 0);
    arg_x = z;
    break;

  default:
    return (-9);
  }
  return (success = (inserver ? 1 : 0));
#else  /* IKSDCONF */
  if ((x = cmcfm()) < 0)
    return (x);
  return (success = 0);
#endif /* IKSDCONF */
}
#endif /* NOICP */
#endif /* IKSD */

/*  D O A R G  --  Do a command-line argument.  */

int doarg(char x)
/* doarg */ {
  int i, n, y, z, xx;
  long zz;
  char *xp;

#ifdef NETCONN
  extern char *line, *tmpbuf; /* Character buffers for anything */
#endif                        /* NETCONN */

#ifdef IKSD
  /* Internet Kermit Server set some way besides -A... */
  if (inserver)
    dofast();
#endif /* IKSD */

  xp = *xargv + 1; /* Pointer for bundled args */
  debug(F111, "doarg entry", xp, xargc);
  while (x) {
    debug(F000, "doarg arg", "", x);
    switch (x) { /* Big switch on arg */

#ifndef NOICP
    case '-': /* Extended commands... */
      if (doxarg(xargv, 0) < 0) {
        XFATAL("Extended option error");
      } /* Full thru... */
    case '+': /* Extended command for prescan() */
      return (0);
#else  /* NOICP */
    case '-':
    case '+':
      XFATAL("Extended options not configured");
#endif /* NOICP */

#ifndef NOSPL
    case 'C': { /* Commands for parser */
      char *s;
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("No commands given for -C");
      }
      s = *xargv; /* Get the argument (must be quoted) */
      if (!*s)    /* If empty quotes */
        s = NULL; /* ignore this option */
      if (s) {
        makestr(&clcmds, s); /* Make pokeable copy */
        s = clcmds;          /* Change tabs to spaces */
        while (*s) {
          if (*s == '\t')
            *s = ' ';
          s++;
        }
      }
      break;
    }
#endif /* NOSPL */

#ifndef NOXFER
    case 'D': /* Delay */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing delay value");
      }
      z = atoi(*xargv); /* Convert to number */
      if (z > -1)       /* If in range */
        ckdelay = z;    /* set it */
      else {
        XFATAL("bad delay value");
      }
      break;
#endif /* NOXFER */

    case 'E': /* Exit on close */
#ifdef NETCONN
      tn_exit = 1;
#endif /* NETCONN */
      exitonclose = 1;
      break;

#ifndef NOICP
    case 'S':      /* "Stay" - enter interactive */
      stayflg = 1; /* command parser after executing */
      xfinish = 0; /* command-line actions. */
      break;
#endif /* NOICP */

    case 'T': /* File transfer mode = text */
      binary = XYFT_T;
      xfermode = XMODE_M; /* Transfer mode manual */
      filepeek = 0;
#ifdef PATTERNS
      patterns = 0;
#endif /* PATTERNS */
      break;

    case '7':
      break;

#ifdef IKSD
    case 'A': {     /* Internet server */
                    /* Already done in prescan() */
                    /* but implies 'x' &&  'Q'   */
#ifdef NOICP        /* If no Interactive Command Parser */
      action = 'x'; /* -A implies -x. */
#endif              /* NOICP */
#ifndef NOXFER
      dofast();
#endif /* NOXFER */
      break;
    }
#endif /* IKSD */

#ifndef NOXFER
    case 'Q': /* Quick (i.e. FAST) */
      dofast();
      break;
#endif /* NOXFER */

    case 'R': /* Remote-Only */
      break;  /* This is handled in prescan(). */

#ifndef NOSERVER
    case 'x': /* server */
    case 'O': /* (for One command only) */
      if (action) {
        XFATAL("conflicting actions");
      }
      if (x == 'O')
        justone = 1;
      xfinish = 1;
      action = 'x';
      break;
#endif /* NOSERVER */

#ifndef NOXFER
    case 'f': /* finish */
      if (action) {
        XFATAL("conflicting actions");
      }
      action = setgen('F', "", "", "");
      break;
#endif /* NOXFER */

    case 'r': { /* receive */
      if (action) {
        XFATAL("conflicting actions");
      }
      action = 'v';
      break;
    }

#ifndef NOXFER
    case 'k': /* receive to stdout */
      if (action) {
        XFATAL("conflicting actions");
      }
      stdouf = 1;
      action = 'v';
      break;

    case 's': { /* send */
      int fil2snd, rc;
      if (!recursive)
        nolinks = 0; /* Follow links by default */

      if (action) {
        XFATAL("conflicting actions");
      }
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -s");
      }
      nfils = 0;                     /* Initialize file counter */
      fil2snd = 0;                   /* Assume nothing to send  */
      z = 0;                         /* Flag for stdin */
      cmlist = xargv + 1;            /* Remember this pointer */
      while (++xargv, --xargc > 0) { /* Traverse the list */
#ifdef PIPESEND
        if (usepipes && protocol == PROTO_K && **xargv == '!') {
          cmarg = *xargv;
          cmarg++;
          debug(F110, "doarg pipesend", cmarg, 0);
          nfils = -1;
          z = 1;
          pipesend = 1;
        } else
#endif                                    /* PIPESEND */
          if (**xargv == '-') {           /* Check for sending stdin */
            if (strcmp(*xargv, "-") != 0) /* next option? */
              break;
            z++; /* "-" alone means send from stdin. */
#ifdef RECURSIVE
          } else if (!strcmp(*xargv, ".")) {
            fil2snd = 1;
            nfils++;
            recursive = 1;
            nolinks = 2;
#endif           /* RECURSIVE */
          } else /* Check if file exists */
            if ((rc = zchki(*xargv)) > -1 || (rc == -2)) {
              if (rc != -2)
                fil2snd = 1;
              nfils++; /* Bump file counter */
            } else if (iswild(*xargv) && nzxpand(*xargv, 0) > 0) {
              /* or contains wildcard characters matching real files */
              fil2snd = 1;
              nfils++;
            } else {
              if (!failmsg)
                failmsg = (char *)malloc(2000);
              if (failmsg) {
                ckmakmsg(failmsg, 2000, "kermit -s ", *xargv, ": ",
                         ck_errstr());
              }
            }
      }
      xargc++, xargv--; /* Adjust argv/argc */
      if (!fil2snd && z == 0) {
        if (!failmsg) {
          failmsg = "No files for -s";
        }
        XFATAL(failmsg);
      }
      if (z > 1) {
        XFATAL("kermit -s: too many -'s");
      }
      if (z == 1 && fil2snd) {
        XFATAL("invalid mixture of filenames and '-' in -s");
      }
      debug(F101, "doarg s nfils", "", nfils);
      debug(F101, "doarg s z", "", z);
      if (nfils == 0) {    /* no file parameters were specified */
        if (is_a_tty(0)) { /* (used to be is_a_tty(1) - why?) */
          XFATAL("sending from terminal not allowed");
        } else
          stdinf = 1;
      }
      debug(F101, "doarg s stdinf", "", stdinf);
      debug(F111, "doarg", *xargv, nfils);
      action = 's';
      break;
    }

    case 'g': /* get */
    case 'G': /* get to stdout */
      if (action) {
        XFATAL("conflicting actions");
      }
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -g");
      }
      xargv++, xargc--;
      if ((xargc == 0) || (**xargv == '-')) {
        XFATAL("missing filename for -g");
      }
      if (x == 'G')
        stdouf = 1;
      cmarg = *xargv;
      action = 'r';
      break;
#endif /* NOXFER */

#ifndef NOLOCAL
    case 'c': /* connect before */
      cflg = 1;
      break;

    case 'n': /* connect after */
      cnflg = 1;
      break;
#endif /* NOLOCAL */

    case 'h': /* help */
      usage();
#ifndef NOICP
      if (stayflg || what == W_COMMAND)
        break;
      else
#endif /* NOICP */
        doexit(GOOD_EXIT, -1);

#ifndef NOXFER
    case 'a': /* "as" */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -a");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing name in -a");
      }
      cmarg2 = *xargv;
      debug(F111, "doarg a", cmarg2, xargc);
      break;
#endif /* NOXFER */

#ifndef NOICP
    case 'Y': /* No initialization file */
      noinit = 1;
      break;

    case 'y': /* Alternate init-file name */
      noinit = 0;
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -y");
      }
      xargv++, xargc--;
      if (xargc < 1) {
        XFATAL("missing filename in -y");
      }
      /* strcpy(kermrc,*xargv); ... already done in prescan()... */
      break;
#endif /* NOICP */

#ifndef NOXFER
    case 'I':       /* Assume we have an "Internet" */
      reliable = 1; /* or other reliable connection */
      xreliable = 1;
      setreliable = 1;

      /* I'm not so sure about this -- what about VMS? (next comment) */
      clearrq = 1; /* therefore the channel is clear */

      /*
        Since this can trigger full control-character unprefixing, we need to
        ensure that our terminal or pty driver is not doing Xon/Xoff; otherwise
        we can become deadlocked the first time we receive a file that contains
        Xoff.
      */
      flow = FLO_NONE;
      break;
#endif /* NOXFER */

#ifndef NOLOCAL
    case 'l': /* SET LINE */
#ifdef NETCONN
#ifdef ANYX25
    case 'X': /* SET HOST to X.25 address */
#endif        /* ANYX25 */
#ifdef TCPSOCKET
    case 'J':
    case 'j': /* SET HOST (TCP/IP socket) */
#endif        /* TCPSOCKET */
#endif        /* NETCONN */
#ifndef NOXFER
      if (x == 'j' || x == 'J' || x == 'X' || x == 'Z') {
        reliable = 1; /* or other reliable connection */
        xreliable = 1;
        setreliable = 1;
      }
#endif /* NOXFER */
      network = 0;
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -l or -j");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("communication line device name missing");
      }

#ifdef NETCONN
      if (x == 'J') {
        cflg = 1;    /* Connect */
        stayflg = 1; /* Stay */
        tn_exit = 1; /* Telnet-like exit condition */
        exitonclose = 1;
      }
#endif /* NETCONN */
      ckstrncpy(ttname, *xargv, TTNAMLEN + 1);
      local = (strcmp(ttname, CTTNAM) != 0);
      if (local && strcmp(ttname, "0") == 0)
        local = 0;
      /*
        NOTE: We really do not need to call ttopen here, since it should be
        called again later, automatically, when we first try to condition the
        device via ttpkt or ttvt.  Calling ttopen here has the bad side effect
        of making the order of the -b and -l options significant when the order
        of command-line options should not matter.  However, the network cases
        immediately below complicate matters a bit, so we'll settle this in a
        future edit.
      */
      if (x == 'l') {
        if (ttopen(ttname, &local, mdmtyp, 0) < 0) {
          XFATAL("can't open device");
        }
#ifdef CKLOGDIAL
        dologline();
#endif /* CKLOGDIAL */
        debug(F101, "doarg speed", "", speed);
        cxtype = (mdmtyp > 0) ? CXT_MODEM : CXT_DIRECT;
        speed = ttgspd(); /* Get the speed. */
        setflow();        /* Do something about flow control. */
#ifndef NOSPL
        if (local) {
          if (nmac) {                           /* Any macros defined? */
            int k;                              /* Yes */
            k = mlook(mactab, "on_open", nmac); /* Look this up */
            if (k >= 0) {                       /* If found, */
              if (dodo(k, ttname, 0) > -1)      /* set it up, */
                parser(1);                      /* and execute it */
            }
          }
        }
#endif /* NOSPL */

#ifdef NETCONN
      } else {
        if (x == 'j' || x == 'J') { /* IP network host name */
          char *s = line;
          char *service = tmpbuf;
          if (xargc > 0) { /* Check if it's followed by */
            /* A service name or number */
            if (*(xargv + 1) && *(*(xargv + 1)) != '-') {
              xargv++, xargc--;
              ckstrncat(ttname, ":", TTNAMLEN + 1);
              ckstrncat(ttname, *xargv, TTNAMLEN + 1);
            }
          }
          nettype = NET_TCPB;
          mdmtyp = -nettype; /* Perhaps already set in init file */
          telnetfd = 1;      /* Or maybe an open file descriptor */
          ckstrncpy(line, ttname, LINBUFSIZ); /* Working copy */
          for (s = line; *s != NUL && *s != ':'; s++)
            ;
          if (*s) {
            *s++ = NUL;
            ckstrncpy(service, s, TMPBUFSIZ);
          } else
            *service = NUL;
          s = line;
#ifndef NODIAL
#ifndef NOICP
          /* Look up in network directory */
          x = 0;
          if (*s == '=') {   /* If number starts with = sign */
            s++;             /* strip it */
            while (*s == SP) /* and also any leading spaces */
              s++;
            ckstrncpy(line, s, LINBUFSIZ); /* Do this again. */
            nhcount = 0;
          } else if (!isdigit(line[0])) {
            /*
              nnetdir will be greater than 0 if the init file has been processed
              and it contained a SET NETWORK DIRECTORY command.
            */
            xx = 0;             /* Initialize this */
            if (nnetdir > 0)    /* If there is a directory... */
              xx = lunet(line); /* Look up the name */
            else                /* If no directory */
              nhcount = 0;      /* we didn't find anything there */
            if (xx < 0) {       /* Lookup error: */
              ckmakmsg(tmpbuf, TMPBUFSIZ,
                       "?Fatal network directory lookup error - ", line, "\n",
                       NULL);
              XFATAL(tmpbuf);
            }
          }
#endif /* NOICP */
#endif /* NODIAL */
          /* Add service to line specification for ttopen() */
          if (*service) { /* There is a service specified */
            ckstrncat(line, ":", LINBUFSIZ);
            ckstrncat(line, service, LINBUFSIZ);
            ttnproto = NP_DEFAULT;
          } else {
            ckstrncat(line, ":telnet", LINBUFSIZ);
            ttnproto = NP_TELNET;
          }

#ifndef NOICP
#ifndef NODIAL
          if ((nhcount > 1) && !quiet && !backgrd) {
            printf("%d entr%s found for \"%s\"%s\n", nhcount,
                   (nhcount == 1) ? "y" : "ies", s, (nhcount > 0) ? ":" : ".");
            for (i = 0; i < nhcount; i++)
              printf("%3d. %s %-12s => %s\n", i + 1, n_name, nh_p2[i], nh_p[i]);
          }
          if (nhcount == 0)
            n = 1;
          else
            n = nhcount;
#else
          n = 1;
          nhcount = 0;
#endif /* NODIAL */
          for (i = 0; i < n; i++) {
#ifndef NODIAL
            if (nhcount >= 1) {
              /* Copy the current entry to line */
              ckstrncpy(line, nh_p[i], LINBUFSIZ);
              /* Check to see if the network entry contains a service */
              for (s = line; (*s != NUL) && (*s != ':'); s++)
                ;
              /* If directory does not have a service ... */
              /* and the user specified one */
              if (!*s && *service) {
                ckstrncat(line, ":", LINBUFSIZ);
                ckstrncat(line, service, LINBUFSIZ);
              }
              if (lookup(netcmd, nh_p2[i], nnets, &z) > -1) {
                mdmtyp = 0 - netcmd[z].kwval;
              } else {
                printf("Error - network type \"%s\" not supported\n", nh_p2[i]);
                continue;
              }
            }
#endif /* NODIAL */
          }
#endif /* NOICP */
          ckstrncpy(ttname, line, TTNAMLEN + 1);
          cxtype = CXT_TCPIP; /* Set connection type */
          setflow();          /* Set appropriate flow control. */
#ifdef IBMX25
        } else if (x == 'X') { /* X.25 address */
          nettype = NET_IX25;
          mdmtyp = -nettype;
#endif /* IBMX25 */
#ifdef HPX25
        } else if (x == 'X') { /* X.25 address */
          nettype = NET_HX25;
          mdmtyp = -nettype;
#endif /* HPX25 */
        }
        if (ttopen(ttname, &local, mdmtyp, 0) < 0) {
          XFATAL("can't open host connection");
        }
        network = 1;
#ifdef CKLOGDIAL
        dolognet();
#endif                    /* CKLOGDIAL */
        cxtype = CXT_X25; /* Set connection type */
        setflow();        /* Set appropriate flow control. */
#ifndef NOSPL
        if (local) {
          if (nmac) {                           /* Any macros defined? */
            int k;                              /* Yes */
            k = mlook(mactab, "on_open", nmac); /* Look this up */
            if (k >= 0) {                       /* If found, */
              if (dodo(k, ttname, 0) > -1)      /* set it up, */
                parser(1);                      /* and execute it */
            }
          }
        }
#endif /* NOSPL */
#endif /* NETCONN */
      }
      /* add more here -- decnet, etc... */
      haveline = 1;
      break;

#ifdef ANYX25
    case 'U': /* X.25 call user data */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing call user data string");
      }
      ckstrncpy(udata, *xargv, MAXCUDATA);
      if ((int)strlen(udata) <= MAXCUDATA) {
        cudata = 1;
      } else {
        XFATAL("Invalid call user data");
      }
      break;

    case 'o': /* X.25 closed user group */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing closed user group index");
      }
      z = atoi(*xargv); /* Convert to number */
      if (z >= 0 && z <= 99) {
        closgr = z;
      } else {
        XFATAL("Invalid closed user group index");
      }
      break;

    case 'u': /* X.25 reverse charge call */
      revcall = 1;
      break;
#endif /* ANYX25 */
#endif /* NOLOCAL */

    case 'b': /* Bits-per-second for serial device */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing bps");
      }
      zz = atol(*xargv); /* Convert to long int */
      i = zz / 10L;
#ifndef NOLOCAL
      if (ttsspd(i) > -1) /* Check and set it */
#endif                    /* NOLOCAL */
        speed = ttgspd(); /* and read it back. */
#ifndef NOLOCAL
      else {
        XFATAL("unsupported transmission rate");
      }
#endif /* NOLOCAL */
      break;

#ifndef NODIAL
#ifndef NOICP
    case 'm': /* Modem type */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -m");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("modem type missing");
      }
      y = lookup(mdmtab, *xargv, nmdm, &z);
      if (y < 0) {
        XFATAL("unknown modem type");
      }
      usermdm = 0;
      usermdm = (y == dialudt) ? x : 0;
      initmdm(y);
      break;
#endif /* NOICP */
#endif /* NODIAL */

#ifndef NOXFER
    case 'e': /* Extended packet length */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -e");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing length");
      }
      z = atoi(*xargv); /* Convert to number */
      if (z > 10 && z <= maxrps) {
        rpsiz = urpsiz = z;
        if (z > 94)
          rpsiz = 94; /* Fallback if other Kermit can't */
      } else {
        XFATAL("Unsupported packet length");
      }
      break;

    case 'v': /* Vindow size */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing or bad window size");
      }
      z = atoi(*xargv); /* Convert to number */
      if (z < 32) {     /* If in range */
        wslotr = z;     /* set it */
        if (z > 1)
          swcapr = 1; /* Set capas bit if windowing */
      } else {
        XFATAL("Unsupported packet length");
      }
      break;
#endif /* NOXFER */

    case 'i': /* Treat files as binary */
      binary = XYFT_B;
      xfermode = XMODE_M; /* Transfer mode manual */
      filepeek = 0;
#ifdef PATTERNS
      patterns = 0;
#endif /* PATTERNS */
      break;

#ifndef NOXFER
    case 'w': /* Writeover */
      ckwarn = 0;
      fncact = XYFX_X;
      break;
#endif /* NOXFER */

    case 'q': /* Quiet */
      quiet = 1;
      break;

#ifdef DEBUG
    case 'd': /* DEBUG */
      break;  /* Handled in prescan() */
#endif        /* DEBUG */

    case '0': { /* In the middle */
      extern int tt_escape, lscapr;
      tt_escape = 0; /* No escape character */
      flow = 0;      /* No Xon/Xoff (what about hwfc?) */
#ifndef NOXFER
      lscapr = 0; /* No locking shifts */
#endif            /* NOXFER */
#ifdef CK_APC
      {
        extern int apcstatus; /* No APCs */
        apcstatus = APC_OFF;
      }
#endif /* CK_APC */
#ifndef NOLOCAL
#ifdef CK_AUTODL
      setautodl(0, 0); /* No autodownload */
#endif                 /* CK_AUTODL */
#endif                 /* NOLOCAL */
#ifndef NOCSETS
      {
        extern int tcsr, tcsl; /* No character-set translation */
        tcsr = 0;
        tcsl = tcsr; /* Make these equal */
      }
#endif /* NOCSETS */
#ifdef TNCODE
      TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = TN_NG_RF;
      TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = TN_NG_RF;
      TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = TN_NG_RF;
      TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = TN_NG_RF;
#endif /* TNCODE */
    }
      /* Fall thru... */

    case '8': /* 8-bit clean */
      parity = 0;
      cmdmsk = 0xff;
      cmask = 0xff;
      break;

    case 'V': {
      extern int xfermode;
#ifdef PATTERNS
      extern int patterns;
      patterns = 0;       /* No patterns */
#endif                    /* PATTERNS */
      xfermode = XMODE_M; /* Manual transfer mode */
      filepeek = 0;
      break;
    }

    case 'p': /* SET PARITY */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing parity");
      }
      switch (x = **xargv) {
      case 'e':
      case 'o':
      case 'm':
      case 's':
        parity = x;
        break;
      case 'n':
        parity = 0;
        break;
      default: {
        XFATAL("invalid parity");
      }
      }
      break;

    case 't': /* Line turnaround handshake */
      turn = 1;
      turnch = XON; /* XON is turnaround character */
      duplex = 1;   /* Half duplex */
      flow = 0;     /* No flow control */
      break;

    case 'B':
      bgset = 1; /* Force background (batch) */
      backgrd = 1;
      break;

    case 'z': /* Force foreground */
      bgset = 0;
      backgrd = 0;
      break;

#ifndef NOXFER
#ifdef RECURSIVE
    case 'L':
      recursive = 2;
      nolinks = 2;
      fnspath = PATH_REL;
      break;
#endif /* RECURSIVE */
#endif /* NOXFER */

#ifndef NOSPL
    case 'M': /* My User Name */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
      }
      if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
      }
#ifdef IKSD
      if (!inserver)
#endif /* IKSD */
      {
        ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
        haveftpuid = 1;
      }
      break;
#endif /* NOSPL */

#ifdef CK_NETBIOS
    case 'N': /* NetBios Adapter Number follows */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -N");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing NetBios Adapter number");
      }
      if ((strlen(*xargv) != 1) ||
          (*xargv)[0] != 'X' && (atoi(*xargv) < 0) && (atoi(*xargv) > 9)) {
        XFATAL("Invalid NetBios Adapter - Adapters 0 to 9 are valid");
      }
      break;
#endif /* CK_NETBIOS */

#ifdef NETCONN
    case 'F':
      network = 1;
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -F");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("network file descriptor missing");
      }
      ckstrncpy(ttname, *xargv, TTNAMLEN + 1);
      nettype = NET_TCPB;
      mdmtyp = -nettype;
      telnetfd = 1;
      local = 1;
      break;
#endif /* NETCONN */

    case 'P': /* Filenames literal */
      fncnv = XYFN_L;
      f_save = XYFN_L;
      break;

#ifndef NOICP
    case 'H':
      noherald = 1;
      break;
#endif /* NOICP */

#ifdef NEWFTP
    case '9': /* FTP */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling after -9");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("FTP server address missing");
      }
      makestr(&ftp_host, *xargv);
      break;
#endif /* NEWFTP */

    default:
      fatal2(*xargv,
             "invalid command-line option, type \"kermit -h\" for help");
    }
    if (!xp)
      break;
    x = *++xp; /* See if options are bundled */
  }
  return (0);
}

#ifdef TNCODE
/*  D O T N A R G  --  Do a telnet command-line argument.  */

static int dotnarg(char x)
/* dotnarg */ {
  char *xp;

  xp = *xargv + 1; /* Pointer for bundled args */
  debug(F111, "dotnarg entry", xp, xargc);
  while (x) {
    debug(F000, "dotnarg arg", "", x);
    switch (x) { /* Big switch on arg */

#ifndef NOICP
    case '-': /* Extended commands... */
      if (doxarg(xargv, 0) < 0) {
        XFATAL("Extended option error");
      } /* Full thru... */
    case '+': /* Extended command for prescan() */
      return (0);
#else  /* NOICP */
    case '-':
    case '+':
      XFATAL("Extended options not configured");
#endif /* NOICP */

      /*
       * -#                Kermit 95 Startup Flags
       * -8                Negotiate Telnet Binary in both directions
       * -a                Require use of Telnet authentication
       * -c                Do not read the .telnetrc file
       * -d                Turn on debug mode
       * -E                No escape character
       * -f                Forward credentials to host
       * -K                Refuse use of authentication; do not send username
       * -k realm          Set default realm
       * -l user           Set username and request Telnet authentication
       * -L                Negotiate Telnet Binary Output only
       * -q                Quiet mode (suppress messages)
       * -S tos            Use the IP type-of-service tos
       * -x                Require Encryption
       * -D                Disable forward-X
       * -T cert=file      Use certificate in file
       * -T key=file       Use private key in file
       * -T crlfile=file   Use CRL in file
       * -T crldir=dir     Use CRLs in directory
       * -T cipher=string  Use only ciphers in string
       * -X atype          Disable use of atype authentication
       * -Y                Disable init file processing
       *
       */
    case 'h': /* help */
      usage();
      doexit(GOOD_EXIT, -1);
      break;

    case '8': /* Telnet Binary in both directions */
      TELOPT_DEF_C_U_MODE(TELOPT_BINARY) = TN_NG_MU;
      TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = TN_NG_MU;
      parity = 0;
      cmdmsk = 0xff;
      cmask = 0xff;
      break;

    case 'a': /* Require Telnet Auth */
      TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
      break;

    case 'Y':
      xargv++, xargc--; /* Skip past argument */
      break;            /* Action done in prescan */

    case 'q': /* Quiet */
      quiet = 1;
      break;

    case 'd':
#ifdef DEBUG
      if (deblog) {
        debtim = 1;
      } else {
        deblog = debopn("debug.log", 0);
      }
#endif /* DEBUG */
      break;

    case 'E': { /* No Escape character */
      extern int tt_escape;
      tt_escape = 0;
    } break;

    case 'K':
      TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
      uidbuf[0] = NUL;
      break;

    case 'l': /* Set username and request telnet authentication */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
      }
      if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
      }
      ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
      TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_MU;
      break;

    case 'L': /* Require BINARY mode outbound only */
      TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = TN_NG_MU;
      break;

    case 'x': /* Require Encryption */
      TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
      TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
      break;

    case 'D': /* Disable use of Forward X */
      TELOPT_DEF_C_U_MODE(TELOPT_FORWARD_X) = TN_NG_RF;
      break;

    case 'f': /* Forward credentials to host */
    {
      break;
    }

    case 'k': {
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing realm");
      }
      break;
    }

    case 'T': {
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing cert=, key=, crlfile=, crldir=, or cipher=");
      }
      break;
    }

    default:
      fatal2(*xargv,
             "invalid command-line option, type \"telnet -h\" for help");
    }

    if (!xp)
      break;
    x = *++xp; /* See if options are bundled */
  }
  return (0);
}
#endif /* TNCODE */

#ifdef RLOGCODE

/*  D O R L G A R G  --  Do a rlogin command-line argument.  */

static int dorlgarg(char x)
/* dorlgarg */ {
  char *xp;

  xp = *xargv + 1; /* Pointer for bundled args */
  debug(F111, "dorlgarg entry", xp, xargc);
  while (x) {
    debug(F000, "dorlgarg arg", "", x);
    switch (x) { /* Big switch on arg */

#ifndef NOICP
    case '-': /* Extended commands... */
      if (doxarg(xargv, 0) < 0) {
        XFATAL("Extended option error");
      } /* Full thru... */
    case '+': /* Extended command for prescan() */
      return (0);
#else  /* NOICP */
    case '-':
    case '+':
      XFATAL("Extended options not configured");
#endif /* NOICP */

      /*
       * -d                Debug
       * -l user           Set username
       *
       */
    case 'h': /* help */
      usage();
      doexit(GOOD_EXIT, -1);
      break;

    case 'Y':
      xargv++, xargc--; /* Skip past argument */
      break;            /* Action done in prescan */
    case 'q':           /* Quiet */
      quiet = 1;
      break;

    case 'd':
#ifdef DEBUG
      if (deblog) {
        debtim = 1;
      } else {
        deblog = debopn("debug.log", 0);
      }
#endif /* DEBUG */
      break;

    case 'l': /* Set username and request telnet authentication */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
      }
      if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
      }
      ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
      break;

    default:
      fatal2(*xargv,
             "invalid command-line option, type \"rlogin -h\" for help");
    }

    if (!xp)
      break;
    x = *++xp; /* See if options are bundled */
  }
  return (0);
}
#endif /* RLOGCODE */

#ifdef SSHBUILTIN

/*  D O S S H A R G  --  Do a ssh command-line argument.  */

static int dossharg(char x)
/* dossharg */ {
  char *xp;

  xp = *xargv + 1; /* Pointer for bundled args */
  debug(F111, "dossharg entry", xp, xargc);
  while (x) {
    debug(F000, "dossharg arg", "", x);
    switch (x) { /* Big switch on arg */

#ifndef NOCICP
    case '-': /* Extended commands... */
      if (doxarg(xargv, 0) < 0) {
        XFATAL("Extended option error");
      } /* Full thru... */
    case '+': /* Extended command for prescan() */
      return (0);
#else  /* NOICP */
    case '-':
    case '+':
      XFATAL("Extended options not configured");
#endif /* NOICP */

      /*
       * -d                Debug
       * -# args           Init
       * -Y                no init file
       * -l user           Set username
       *
       */
    case 'h': /* help */
      usage();
      doexit(GOOD_EXIT, -1);
      break;

    case 'Y':
      xargv++, xargc--; /* Skip past argument */
      break;            /* Action done in prescan */
    case 'q':           /* Quiet */
      quiet = 1;
      break;

    case 'd':
#ifdef DEBUG
      if (deblog) {
        debtim = 1;
      } else {
        deblog = debopn("debug.log", 0);
      }
#endif /* DEBUG */
      break;

    case 'l': /* Set username and request telnet authentication */
      if (*(xp + 1)) {
        XFATAL("invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        XFATAL("missing username");
      }
      if ((int)strlen(*xargv) > 63) {
        XFATAL("username too long");
      }
      ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
      break;

    default:
      fatal2(*xargv, "invalid command-line option, type \"ssh -h\" for help");
    }

    if (!xp)
      break;
    x = *++xp; /* See if options are bundled */
  }
  return (0);
}
#endif /* SSHBUILTIN */

#else  /* No command-line interface... */

int cmdlin() {
  extern int xargc;
  if (xargc > 1) {
    XFATAL("Sorry, command-line options disabled.");
  }
}
#endif /* NOCMDL */
