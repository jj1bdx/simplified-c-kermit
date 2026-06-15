char *cknetv = "Network support, 10.0.304, 18 Sep 2023";

/*  C K C N E T  --  Network support  */

/*
  Copyright (C) 1985, 2023,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  REMINDER: Any changes made to this file that other modules depend must
  also be made to cklnet.c (for VOS) until such time as cklnet.c and this
  module are merged back together.  (Update 2022-12-06: VOS C-Kermit has
  not been released since 7.0.197 08 Feb 2000, so if the VOS version is
  ever to be resurrected, it should start with the current ckcnet.c and
  then merge back any VOS dependencies.)

  NOTE TO CONTRIBUTORS: This file, and all the other shared (ckc and cku)
  C-Kermit source files, must be compatible with C preprocessors that support
  only #ifdef, #else, #endif, #define, and #undef.  Please do not use #if,
  logical operators, or other preprocessor features in this module.  Also,
  don't use any ANSI C constructs except within #ifdef CK_ANSIC..#endif.

  Authors:

  Frank da Cruz (fdc@columbia.edu),
    Columbia University Academic Information Systems, New York City.
  Jeffrey E Altman (jaltman@secure-endpoints.com) -- Primary
    maintainer/developer since about 1996.
  netopen() routine for TCP/IP originally by Ken Yap, Rochester University
    (ken@cs.rochester.edu) (no longer at that address).
  Missing pieces for Excelan sockets library from William Bader.
  Telnet protocol by Frank da Cruz and Jeffrey Altman.
  Rlogin protocol by Jeffrey E Altman.
  SSL support adapted by Jeffrey E Altman from work done by
    Tim Hudson <tjh@cryptosoft.com> +61 7 32781581
  TLS support by Jeffrey E Altman.
  HTTP support by Jeffrey E Altman.
  TGV MultiNet code by Frank da Cruz.
  MultiNet code adapted to WIN/TCP by Ray Hunter of TWG.
  MultiNet code adapted to DEC TCP/IP by Lee Tibbert of DEC and Frank da Cruz.
  TCP/IP support adapted to IBM TCP/IP 1.2.1,2.0 for OS/2 by Kai Uwe Rommel.
  CMU-OpenVMS/IP modifications by Mike O'Malley, Digital (DEC),
    with subsequent improvements by Steven M Schweda (SMS).
  X.25 support by Marcello Frutig, Catholic University,
    Rio de Janeiro, Brazil (frutig@rnp.impa.br) with fixes from
    Stefaan Eeckels, Eurokom, Luxembourg.
    David Lane added support for Stratus VOS X.25 1996 (in cklnet.c).
    Stephen Riehm added support for IBM AIX X.25 in April 1998.
  Other contributions as indicated in the code.
*/
#ifdef NORLOGIN
#ifdef RLOGCODE
#undef RLOGCODE
#endif /* RLOGCODE */
#endif /* NORLOGIN */

#define CKCNET_C
/* clang-format off */
#include "ckcdeb.h"
/* clang-format on */
#include "ckcker.h"
#include "ckcasc.h"
#include "ckcnet.h" /* which includes ckctel.h */
#include "ckuusr.h"

#ifdef CK_DNS_SRV
#include <arpa/inet.h>
#ifdef USE_NAMESER_COMPAT
#include <arpa/nameser_compat.h>
#endif /* USE_NAMESER_COMPAT */

#include <arpa/nameser.h>
#include <resolv.h>

#ifndef BSD4
#include <netdb.h>
#endif /* BSD4 */
#ifndef T_SRV
#define T_SRV 33
#endif /* T_SRV */
#ifndef T_TXT
#define T_TXT 16
#endif /* T_TXT */

/* for old Unixes and friends ... */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif /* MAXHOSTNAMELEN */

#define MAX_DNS_NAMELEN (15 * (MAXHOSTNAMELEN + 1) + 1)
#endif /* CK_DNS_SRV */

#ifdef NONET
#ifdef TCPIPLIB
#undef TCPIPLIB
#endif /* TCPIPLIB */
#endif /* NONET */

#ifndef NOMHHOST
#endif /* NOMHHOST */

#ifdef INADDRX
struct in_addr inaddrx;
#endif /* INADDRX */

int ttnet = NET_NONE;      /* Network type */
int ttnproto = NP_DEFAULT; /* Network virtual terminal protocol */

/* 0 = don't lowercase username for Rlogin/Telnet protocol */
/* nonzero = do lowercase it.  Add a SET command if necessary... */
int ck_lcname = 0;

extern int /* External variables */
    duplex,
    debses, seslog, sessft, wasclosed, quiet, msgflg, what, nettype, ttmdm;
extern CK_TTYFD_T ttyfd;
#ifdef IKSD
extern int inserver;
#endif /* IKSD */

char myipaddr[20] = {'\0'};   /* Global copy of my IP address */
char hostipaddr[64] = {'\0'}; /* Global copy of remote IP address */

#ifdef NETCONN
/* Don't need any of this if there is no network support. */

/* Current fd-swapping hack is not thread-safe */
#define HTTP_BUFFERING

#ifdef HTTP_BUFFERING
#define HTTP_INBUFLEN 8192
static char http_inbuf[HTTP_INBUFLEN];
static int http_bufp = 0, http_count;
#endif /* HTTP_BUFFERING */

/*
  NETLEBUF is (must be) defined for those platforms that call this
  module to do network i/o (e.g. netinc(), nettchk(), etc) rather
  than doing it themselves (ttinc(), ttchk(), etc).  In this case
  the Telnet local-echo buffers and routines are defined and referenced
  here, rather than in the ck?tio.c module.
*/
#ifdef NETLEBUF
#define LEBUFSIZ 4096
int ttpush = -1, le_data = 0; /* These are seen from outside */
static CHAR le_buf[LEBUFSIZ]; /* These are used internally */
static int le_start = 0, le_end = 0;
int tt_push_inited = 0;
#endif /* NETLEBUF */

#ifdef CK_SOCKS  /* SOCKS Internet relay package */
#ifdef CK_SOCKS5 /* SOCKS 5 */
#define accept SOCKSaccept
#define bind SOCKSbind
#define connect SOCKSconnect
#define getsockname SOCKSgetsockname
#define listen SOCKSlisten
#else /* Not SOCKS 5 */
#define accept Raccept
#define bind Rbind
#define connect Rconnect
#define getsockname Rgetsockname
#define listen Rlisten
#endif /* CK_SOCKS5 */
#endif /* CK_SOCKS */

/* Also see ckcnet.h -- hmmm, why don't we always include inet.h? */

#ifndef NODCLTIMEVAL
#ifdef DCLTIMEVAL /* UnixWare 7 */
struct timeval {  /* And define these ourselves. */
  long tv_sec;    /* (see comments in ckutio.c) */
  long tv_usec;
};
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};
#endif /* DCLTIMEVAL */
#endif /* NODCLTIMEVAL */

#include <errno.h> /* Error number symbols */

#include <signal.h> /* Everybody needs this */

#include <setjmp.h>

#ifndef socket_errno
#define socket_errno errno
#endif /* socket_errno */

#ifdef TNCODE
extern int tn_deb;
#endif /* TNCODE */

int tcp_rdns = /* Reverse DNS lookup */
#ifdef DEC_TCPIP_OLD
    SET_OFF /* Doesn't seem to work in UCX */
#else
    SET_AUTO
#endif /* DEC_TCPIP */
    ;
#ifdef CK_DNS_SRV
int tcp_dns_srv = SET_OFF;
#endif /* CK_DNS_SRV */

char *cmcvtdate(char *, int);

#ifdef RLOGCODE
int rlog_ctrl(CHAR *, int);
static int rlog_oob(CHAR *, int);
#ifndef TCPIPLIB
static void rlogoobh(int);
#endif /* TCPIPLIB */
static int rlog_ini(CHAR *, int, struct sockaddr_in *, struct sockaddr_in *);
int rlog_mode = RL_COOKED;
int rlog_stopped = 0;
int rlog_inband = 0;
#endif /* RLOGCODE */

#ifndef NOICP
extern int doconx; /* CONNECT-class command active */
#endif             /* NOICP */

#ifdef IBMX25
/* This variable should probably be generalised for true client/server
 * support - ie: the fd of the listening server, accepted calls should
 * be forked or at least handled via a second fd (for IBM's X.25 -
 * ttyfd always holds the active fd - ie the server becomes inactive
 * as long as a client is connected, and becomes active again when the
 * connection is closed)
 */
int x25serverfd = 0; /* extern in ckcnet.h */
int x25seqno = 0;    /* Connection sequence number */
int x25lastmsg = -1; /* A cheapskate's state table */

#define X25_CLOSED 0        /* Default state: no connection, no STREAM */
#define X25_SETUP 1         /* X.25 has been set up (no connection) */
#define X25_CONNECTED 2     /* X.25 connection has been established */
int x25_state = X25_CLOSED; /* Default state */
#endif                      /* IBMX25 */

#ifndef DEBUG
#define deblog 0
#endif /* DEBUG */

#ifdef CK_NAWS /* Negotiate About Window Size */
#ifdef RLOGCODE
int rlog_naws(void);
#endif /* RLOGCODE */
#endif /* CK_NAWS */

#include "ckcsig.h"

static ckjmpbuf njbuf;

#define NAMECPYL 1024     /* Local copy of hostname */
char namecopy[NAMECPYL];  /* Referenced by ckctel.c */
char namecopy2[NAMECPYL]; /* Referenced by ckctel.c */
#ifndef NOHTTP
char http_host_port[NAMECPYL]; /* orig host/port necessary for http */
char http_ip[20] = {'\0'};     /* ip address of host */
char http_port = 0;
int http_ssl = 0;
char *http_agent = 0;
int httpfd = -1; /* socket for http connections */
int http_code = 0;
#define HTTPBUFLEN 1024
char http_reply_str[HTTPBUFLEN] = "";
#endif /* NOHTTP */

char ipaddr[20] = {'\0'};     /* Global copy of IP address */
unsigned long myxipaddr = 0L; /* Ditto as a number */
#endif                        /* NETCONN */

char *tcp_address = NULL; /* Preferred IP Address */
extern char uidbuf[];     /* User ID buffer */
extern char pwbuf[];      /* Password buffer */

#ifndef NOHTTP
char *tcp_http_proxy = NULL; /* Name[:port] of http proxy server */
int tcp_http_proxy_errno = 0;
char *tcp_http_proxy_user = NULL;
char *tcp_http_proxy_pwd = NULL;
char *tcp_http_proxy_agent = NULL;
#define HTTPCPYL 1024
static char proxycopy[HTTPCPYL];
#endif /* NOHTTP */

extern int tt_rows, tt_cols; /* Everybody has this */

extern int cmd_cols, cmd_rows;

#ifdef STREAMING /* Use blocking writes for streaming */
extern int streaming;
#endif /* STREAMING */

#ifndef NOTCPOPTS

/* Skip all this if NOTCPOPTS specified. */

#ifdef SOL_SOCKET

#ifdef TCP_NODELAY
int tcp_nodelay = 0; /* Nagle algorithm TCP_NODELAY */
#endif               /* TCP_NODELAY */

#ifdef SO_DONTROUTE
int tcp_dontroute = 0;
#endif /* SO_DONTROUTE */

#ifdef SO_LINGER
int tcp_linger = 0;     /* SO_LINGER */
int tcp_linger_tmo = 0; /* SO_LINGER timeout */
#endif                  /* SO_LINGER */

#ifndef SO_OOBINLINE /* Hopefully only HP-UX 7.0 */
#define SO_OOBINLINE 0x0100
#endif /* SO_OOBINLINE */

#ifndef TCPSNDBUFSIZ
#endif /* TCPSNDBUFSIZ */

#ifndef TCPSNDBUFSIZ
#define TCPSNDBUFSIZ -1
#endif /* TCPSNDBUFSIZ */

#ifdef SO_SNDBUF
int tcp_sendbuf = TCPSNDBUFSIZ;
#endif /* SO_SNDBUF */

#ifdef SO_RCVBUF
int tcp_recvbuf = -1;
#endif /* SO_RCVBUF */

#ifdef SO_KEEPALIVE
int tcp_keepalive = 1;
#endif /* SO_KEEPALIVE */

#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

#ifndef NETCONN
/*
  Network support not defined.
  Dummy functions here in case #ifdef's forgotten elsewhere.
*/
int /* Open network connection */
netopen(name, lcl, nett)
char *name;
int *lcl, nett;
{
  return (-1);
}
int /* Close network connection */
netclos() {
  return (-1);
}
int /* Check network input buffer */
nettchk() {
  return (-1);
}
int /* Flush network input buffer */
netflui() {
  return (-1);
}
int /* Send network BREAK */
netbreak() {
  return (-1);
}
int /* Input character from network */
netinc(timo)
int timo;
{
  return (-1);
}
int /* Output character to network */
nettoc(CHAR c)
/* nettoc */ {
  return (-1);
}
int nettol(s, n)
CHAR *s;
int n;
{
  return (-1);
}

#else /* NETCONN is defined (much of this module...) */

#ifdef NETLEBUF
void le_init() { /* LocalEchoInit() */
  int i;
  for (i = 0; i < LEBUFSIZ; i++)
    le_buf[i] = '\0';
  le_start = 0;
  le_end = 0;
  le_data = 0;
  tt_push_inited = 1;
}

void le_clean() { /* LocalEchoCleanup() */
  le_init();
  return;
}

int le_inbuf() {
  int rc = 0;
  if (le_start != le_end) {
    rc = (le_end - le_start + LEBUFSIZ) % LEBUFSIZ;
  }
  return (rc);
}

int le_putchar(CHAR ch)
/* le_putchar */ {
  if ((le_start - le_end + LEBUFSIZ) % LEBUFSIZ == 1) {
    debug(F110, "le_putchar", "buffer is full", 0);
    return (-1);
  }
  le_buf[le_end++] = ch;
  if (le_end == LEBUFSIZ)
    le_end = 0;
  le_data = 1;
  return (0);
}

int le_puts(CHAR *s, int n)
/* le_puts */ {
  int rc = 0;
  int i = 0;
  CHAR *p = (CHAR *)"le_puts";
  ckhexdump(p, s, n);
  for (i = 0; i < n; i++)
    rc = le_putchar((char)s[i]);
  debug(F101, "le_puts", "", rc);
  return (rc);
}

int le_putstr(CHAR *s)
/* le_puts */ {
  CHAR *p;
  int rc = 0;
  p = (CHAR *)"le_putstr";
  ckhexdump(p, s, (int)strlen((char *)s));
  for (p = s; *p && !rc; p++)
    rc = le_putchar(*p);
  return (rc);
}

int le_getchar(CHAR *pch)
/* le_gatchar */ {
  int rc = 0;
  if (le_start != le_end) {
    *pch = le_buf[le_start];
    le_buf[le_start] = 0;
    le_start++;

    if (le_start == LEBUFSIZ)
      le_start = 0;

    if (le_start == le_end) {
      le_data = 0;
    }
    rc++;
  } else {
    *pch = 0;
  }
  return (rc);
}
#endif /* NETLEBUF */

int tcpsrfd = -1;

#ifdef TCPSOCKET
#ifndef NOLISTEN /* For incoming connections */

#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif /* INADDR_ANY */

int ttbufr(void);
int tcpsrv_open(char *, int *, int, int);

static unsigned short tcpsrv_port = 0;

#endif /* NOLISTEN */

static char svcbuf[80]; /* TCP service string */
static int svcnum = 0;  /* TCP port number */

#endif /* TCPSOCKET */

/*
  TCPIPLIB means use separate socket calls for i/o, while on UNIX the
  normal file system calls are used for TCP/IP sockets too.
  Means "DEC_TCPIP or MULTINET or WINTCP or OS2 or BEBOX" (see ckcnet.h),
*/

#ifdef TCPIPLIB

/* For buffered network reads... */
/*
  If the buffering code is written right, it shouldn't matter
  how long this buffer is.
*/
#define TTIBUFL 8191 /* Let's use 8K. */

CHAR ttibuf[TTIBUFL + 1];

/*
  select() is used in preference to alarm()/signal(), but different systems
  use different forms of select()...
*/
#ifndef NOSELECT     /* Option to override BSDSELECT */
#endif               /* NOSELECT */
/*
  Others (TGV, TCPware, ...) use alarm()/signal().  The BSDSELECT case does not
  compile at all; the IBMSELECT case compiles and links but crashes at runtime.
  NOTE: If any of these can be converted to select(), they should be for two
  reasons: (1) It's a lot faster; (2) certain sockets libraries do not like
  their socket_read() calls to be interrupted; subsequent socket_read()'s tend
  to fail with EBUSY.  This happened in the UCX case before it was converted
  to use select().
*/
static /* These are used in CKVTIO.C */
    int ttibp = 0,
        ttibn = 0;
/*
  Read bytes from network into internal buffer ttibuf[].
  To be called when input buffer is empty, i.e. when ttibn == 0.

  Other network reading routines, like ttinc, ttinl, ttxin, should check the
  internal buffer first, and call this routine for a refill if necessary.

  Returns -1 on error, 0 if nothing happens.  When data is read successfully,
  returns number of bytes read, and sets global ttibn to that number and
  ttibp (the buffer pointer) to zero.
*/
int ttbufr(void);
int ttbufr() { /* TT Buffer Read */
  int count;

  if (ttnet != NET_TCPB) /* First make sure current net is */
    return (-1);         /* TCP/IP; if not, do nothing. */

  if (ttibn > 0) {  /* Our internal buffer is not empty, */
    return (ttibn); /* so keep using it. */
  }

  if (ttyfd == -1) { /* No connection, error */
    return (-2);
  }

  ttibp = 0; /* Else reset pointer to beginning */

  count = ttchk(); /* Check network input buffer, */
  if (ttibn > 0) { /* which can put a char there! */
    debug(F111, "ttbufr", "ttchk() returns", count);
    return (ttibn);
  }
  if (count < 0) { /* Read error - connection closed */
    return (-2);
  } else if (count > TTIBUFL) /* Too many to read */
    count = TTIBUFL;
  else if (count == 0) /* None, so force blocking read */
    count = 1;
  debug(F101, "ttbufr count 1", "", count);

  /* This is for blocking reads */

#ifdef SO_OOBINLINE
  {
    int outofband = 0;
#ifdef BELLSELECT
    if (select(128, NULL, NULL, efds, 0) > 0 && FD_ISSET(ttyfd, efds))
      outofband = 1;
#else
#ifdef BSDSELECT
    fd_set efds;
    struct timeval tv;
    FD_ZERO(&efds);
    FD_SET(ttyfd, &efds);
    tv.tv_sec = tv.tv_usec = 0L;
    debug(F100, "Out-of-Band BSDSELECT", "", 0);
    if (select(FD_SETSIZE, NULL, NULL, &efds, &tv) > 0 &&
        FD_ISSET(ttyfd, &efds))
      outofband = 1;
#else /* !BSDSELECT */
#ifdef IBMSELECT
    /* Is used by OS/2 ... */
    /* ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set
     */
    /* and timeval stuff since this is the only place where it is used. */
    CK_TTYFD_T socket = ttyfd;
    debug(F100, "Out-of-Band IBMSELECT", "", 0);
    if ((select(&socket, 0, 0, 1, 0L) == 1) && (socket == ttyfd))
      outofband = 1;
#else /* !IBMSELECT */
    /*
      If we can't use select(), then we use the regular alarm()/signal()
      timeout mechanism.
    */
    debug(F101, "Out-of-Band data not supported", "", 0);
    outofband = 0;

#endif /* IBMSELECT */
#endif /* BSDSELECT */
#endif /* BELLSELECT */
    if (outofband) {
      /* Get the Urgent Data */
      /* if OOBINLINE is disabled this should be only a single byte      */
      /* MS Winsock has a bug in Windows 95.  Extra bytes are delivered  */
      /* That were never sent.                                           */
      count = socket_recv(ttyfd, &ttibuf[ttibp + ttibn], count, MSG_OOB);
      if (count <= 0) {
        int s_errno = socket_errno;
        debug(F101, "ttbufr socket_recv MSG_OOB", "", count);
        debug(F101, "ttbufr socket_errno", "", s_errno);
        netclos(); /* *** *** */
        return (-2);
      } else { /* we got out-of-band data */
        ckhexdump("ttbufr out-of-band chars", &ttibuf[ttibp + ttibn], count);
#ifdef BETADEBUG
        bleep(BP_NOTE);
#endif                /* BETADEBUG */
#ifdef RLOGCODE       /* blah */
        if (ttnproto == NP_RLOGIN || ttnproto == NP_K4LOGIN ||
            ttnproto == NP_EK4LOGIN ||
            ((ttnproto == NP_K5LOGIN || ttnproto == NP_EK5LOGIN) &&
             !rlog_inband)) {
          /*
            When urgent data is read with MSG_OOB and not OOBINLINE
            then urgent data and normal data are not mixed.  So
            treat the entire buffer as urgent data.
          */
          rlog_oob(&ttibuf[ttibp + ttibn], count);
          return ttbufr();
        } else
#endif /* RLOGCODE */ /* blah */
        {
          /* For any protocols we don't have a special out-of-band  */
          /* handler for, just put the bytes in the normal buffer   */
          /* and return                                             */

          ttibp += 0; /* Reset buffer pointer. */
          ttibn += count;
#ifdef DEBUG
          /* Got some bytes. */
          debug(F101, "ttbufr count 2", "", count);
          if (count > 0)
            ttibuf[ttibp + ttibn] = '\0';
          debug(F111, "ttbufr ttibuf", ttibuf, ttibp);
#endif /* DEBUG */
          return (ttibn); /* Return buffer count. */
        }
      }
    }
  }
#endif /* SO_OOBINLINE */

  count = socket_read(ttyfd, &ttibuf[ttibp + ttibn], count);
  if (count <= 0) {
    int s_errno = socket_errno;
    debug(F101, "ttbufr socket_read", "", count);
    debug(F101, "ttbufr socket_errno", "", s_errno);
    netclos(); /* *** *** */
    return (-2);
  } else {
    ttibp = 0; /* Reset buffer pointer. */
    ttibn += count;
#ifdef DEBUG
    debug(F101, "ttbufr count 2", "", count); /* Got some bytes. */
    if (count > 0)
      ttibuf[ttibp + ttibn] = '\0';
    debug(F111, "ttbufr ttibuf", &ttibuf[ttibp], ttibn);
#endif /* DEBUG */

    return (ttibn); /* Return buffer count. */
  }
}
#endif /* TCPIPLIB */

#ifndef IBMSELECT
#ifndef BELLSELECT
#ifndef BSDSELECT /* Non-TCPIPLIB case */
#ifdef SELECT
#define BSDSELECT
#endif /* SELECT */
#endif /* BSDSELECT */
#endif /* BELLSELECT */
#endif /* IBMSELECT */

#define TELNET_PORT 23 /* Should do lookup, but it won't change */
#define RLOGIN_PORT 513
#define KERMIT_PORT 1649
#define KLOGIN_PORT 543
#define EKLOGIN_PORT 2105

#ifndef NONET
/*
  C-Kermit network open/close functions for BSD-sockets.
  Much of this code shared by SunLink X.25, which also uses the socket library.
*/
#ifdef TCPSOCKET
struct hostent *ck_copyhostent(struct hostent *h) {
  /*
   *  The hostent structure is dynamic in nature.
   *  struct  hostent {
   *  char    * h_name;
   *  char    * * h_aliases;
   *  short   h_addrtype;
   *  short   h_length;
   *  char    * * h_addr_list;
   *  #define h_addr  h_addr_list[0]
   */
#define HOSTENTCNT 5
  static struct hostent hosts[HOSTENTCNT] = {{NULL, NULL, 0, 0, NULL},
                                             {NULL, NULL, 0, 0, NULL},
                                             {NULL, NULL, 0, 0, NULL},
                                             {NULL, NULL, 0, 0, NULL},
                                             {NULL, NULL, 0, 0, NULL}};
  static int next = 0;
  int i, cnt;
  char **pp;

  if (h == NULL)
    return (NULL);

  if (next == HOSTENTCNT)
    next = 0;

  if (hosts[next].h_name) {
    free(hosts[next].h_name);
    hosts[next].h_name = NULL;
  }
  if (hosts[next].h_aliases) {
    pp = hosts[next].h_aliases;
    while (*pp) {
      free(*pp);
      pp++;
    }
    free(hosts[next].h_aliases);
  }
#ifdef HADDRLIST
  if (hosts[next].h_addr_list) {
    pp = hosts[next].h_addr_list;
    while (*pp) {
      free(*pp);
      pp++;
    }
    free(hosts[next].h_addr_list);
  }
#endif /* HADDRLIST */

  makestr(&hosts[next].h_name, h->h_name);
  if (h->h_aliases) {
    for (cnt = 0, pp = h->h_aliases; pp && *pp; pp++, cnt++)
      ;
    /* The following can give warnings in non-ANSI builds */
    hosts[next].h_aliases = (char **)malloc(sizeof(char *) * (cnt + 1));
    for (i = 0; i < cnt; i++) {
      hosts[next].h_aliases[i] = NULL;
      makestr(&hosts[next].h_aliases[i], h->h_aliases[i]);
    }
    hosts[next].h_aliases[i] = NULL;
  } else
    hosts[next].h_aliases = NULL;

  hosts[next].h_addrtype = h->h_addrtype;
  hosts[next].h_length = h->h_length;

#ifdef HADDRLIST
#ifdef h_addr
  if (h->h_addr_list) {
    for (cnt = 0, pp = h->h_addr_list; pp && *pp; pp++, cnt++)
      ;
    /* The following can give warnings non-ANSI builds */
    hosts[next].h_addr_list = (char **)malloc(sizeof(char *) * (cnt + 1));
    for (i = 0; i < cnt; i++) {
      hosts[next].h_addr_list[i] = malloc(h->h_length);
      bcopy(h->h_addr_list[i], hosts[next].h_addr_list[i], h->h_length);
    }
    hosts[next].h_addr_list[i] = NULL;
  } else
    hosts[next].h_addr_list = NULL;
#else
  bcopy(h->h_addr, &hosts[next].h_addr, h->h_length);
#endif /* h_addr */
#else  /* HADDRLIST */
  bcopy(h->h_addr, &hosts[next].h_addr, h->h_length);
#endif /* HADDRLIST */

  return (&hosts[next++]);
}

int gettcpport() { return (svcnum); }

#endif /* TCPSOCKET */

#ifndef NOTCPOPTS
int ck_linger(int sock, int onoff, int timo) {
/*
  The following, from William Bader, turns off the socket linger parameter,
  which makes a close() block until all data is sent.  "I don't think that
  disabling linger can ever cause kermit to lose data, but you telnet to a
  flaky server (or to our modem server when the modem is in use), disabling
  linger prevents kermit from hanging on the close if you try to exit."

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_LINGER
  struct linger set_linger_opt;
  struct linger get_linger_opt;
  SOCKOPT_T x;

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_linger = onoff;
      tcp_linger_tmo = timo;
      return (1);
    }
  x = sizeof(get_linger_opt);
  if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&get_linger_opt, &x)) {
    debug(F111, "TCP ck_linger can't get SO_LINGER", ck_errstr(), errno);
  } else if (x != sizeof(get_linger_opt)) {
    debug(F111, "TCP ck_linger error: SO_LINGER", "len", x);
    debug(F111, "TCP ck_linger SO_LINGER", "expected len",
          sizeof(get_linger_opt));
    debug(F111, "TCP ck_linger SO_LINGER", "linger_opt.l_onoff",
          get_linger_opt.l_onoff);
    debug(F111, "TCP linger SO_LINGER", "linger_opt.l_linger",
          get_linger_opt.l_linger);
  } else if (get_linger_opt.l_onoff != onoff ||
             get_linger_opt.l_linger != timo) {
    set_linger_opt.l_onoff = onoff;
    set_linger_opt.l_linger = timo;
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&set_linger_opt,
                   sizeof(set_linger_opt))) {
      debug(F111, "TCP ck_linger can't set SO_LINGER", ck_errstr(), errno);
      tcp_linger = get_linger_opt.l_onoff;
      tcp_linger_tmo = get_linger_opt.l_linger;
    } else {
      debug(F101, "TCP ck_linger new SO_LINGER", "", set_linger_opt.l_onoff);
      tcp_linger = set_linger_opt.l_onoff;
      tcp_linger_tmo = set_linger_opt.l_linger;
      return 1;
    }
  } else {
    debug(F101, "TCP ck_linger SO_LINGER unchanged", "",
          get_linger_opt.l_onoff);
    tcp_linger = get_linger_opt.l_onoff;
    tcp_linger_tmo = get_linger_opt.l_linger;
    return 1;
  }
#else
  debug(F100, "TCP ck_linger SO_LINGER not defined", "", 0);
#endif /* SO_LINGER */
#else
  debug(F100, "TCP ck_linger SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return (0);
}

int sendbuf(int sock, int size) {
/*
  The following, from William Bader, allows changing of socket buffer sizes,
  in case that might affect performance.

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_SNDBUF
  int i, j;
  SOCKOPT_T x;

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_sendbuf = size;
      return 1;
    }
  x = sizeof(i);
  if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&i, &x)) {
    debug(F111, "TCP sendbuf can't get SO_SNDBUF", ck_errstr(), errno);
  } else if (x != sizeof(i)) {
    debug(F111, "TCP sendbuf error: SO_SNDBUF", "len", x);
    debug(F111, "TCP sendbuf SO_SNDBUF", "expected len", sizeof(i));
    debug(F111, "TCP sendbuf SO_SNDBUF", "i", i);
  } else if (size <= 0) {
    tcp_sendbuf = i;
    debug(F101, "TCP sendbuf SO_SNDBUF retrieved", "", i);
    return 1;
  } else if (i != size) {
    j = size;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&j, sizeof(j))) {
      debug(F111, "TCP sendbuf can't set SO_SNDBUF", ck_errstr(), errno);
      tcp_sendbuf = i;
    } else {
      debug(F101, "TCP sendbuf old SO_SNDBUF", "", i);
      debug(F101, "TCP sendbuf new SO_SNDBUF", "", j);
      tcp_sendbuf = size;
      return 1;
    }
  } else {
    debug(F101, "TCP sendbuf SO_SNDBUF unchanged", "", i);
    tcp_sendbuf = size;
    return 1;
  }
#else
  debug(F100, "TCP sendbuf SO_SNDBUF not defined", "", 0);
#endif /* SO_SNDBUF */
#else
  debug(F100, "TCP sendbuf SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return (0);
}

int recvbuf(int sock, int size) {
/*
  The following, from William Bader, allows changing of socket buffer sizes,
  in case that might affect performance.

  Modified by Jeff Altman to be generally useful.
*/
#ifdef SOL_SOCKET
#ifdef SO_RCVBUF
  int i, j;
  SOCKOPT_T x;

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_recvbuf = size;
      return (1);
    }
  x = sizeof(i);
  if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&i, &x)) {
    debug(F111, "TCP recvbuf can't get SO_RCVBUF", ck_errstr(), errno);
  } else if (x != sizeof(i)) {
    debug(F111, "TCP recvbuf error: SO_RCVBUF", "len", x);
    debug(F111, "TCP recvbuf SO_RCVBUF", "expected len", sizeof(i));
    debug(F111, "TCP recvbuf SO_RCVBUF", "i", i);
  } else if (size <= 0) {
    tcp_recvbuf = i;
    debug(F101, "TCP recvbuf SO_RCVBUF retrieved", "", i);
    return 1;
  } else if (i != size) {
    j = size;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&j, sizeof(j))) {
      debug(F111, "TCP recvbuf can't set SO_RCVBUF", ck_errstr(), errno);
      tcp_recvbuf = i;
    } else {
      debug(F101, "TCP recvbuf old SO_RCVBUF", "", i);
      debug(F101, "TCP recvbuf new SO_RCVBUF", "", j);
      tcp_recvbuf = size;
      return 1;
    }
  } else {
    debug(F101, "TCP recvbuf SO_RCVBUF unchanged", "", i);
    tcp_recvbuf = size;
    return 1;
  }
#else
  debug(F100, "TCP recvbuf SO_RCVBUF not defined", "", 0);
#endif /* SO_RCVBUF */
#else
  debug(F100, "TCP recvbuf SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return 0;
}

int keepalive(int sock, int onoff) {
#ifdef SOL_SOCKET
#ifdef SO_KEEPALIVE
  int get_keepalive_opt;
  int set_keepalive_opt;
  SOCKOPT_T x;

  debug(F111, "TCP keepalive", "sock", sock);
  debug(F111, "TCP keepalive", "nettype", nettype);
  debug(F111, "TCP keepalive", "ttmdm", ttmdm);

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_keepalive = onoff;
      return 1;
    }
  x = sizeof(get_keepalive_opt);
  if (getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&get_keepalive_opt,
                 &x)) {
    debug(F111, "TCP keepalive can't get SO_KEEPALIVE", ck_errstr(), errno);
  } else if (x != sizeof(get_keepalive_opt)) {
    debug(F111, "TCP keepalive error: SO_KEEPALIVE", "len", x);
    debug(F111, "TCP keepalive SO_KEEPALIVE", "expected len",
          sizeof(get_keepalive_opt));
    debug(F111, "TCP keepalive SO_KEEPALIVE", "keepalive_opt",
          get_keepalive_opt);
  } else if (get_keepalive_opt != onoff) {
    set_keepalive_opt = onoff;
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&set_keepalive_opt,
                   sizeof(set_keepalive_opt))) {
      debug(F111, "TCP keepalive can't clear SO_KEEPALIVE", ck_errstr(), errno);
      tcp_keepalive = get_keepalive_opt;
    } else {
      debug(F101, "TCP keepalive new SO_KEEPALIVE", "", set_keepalive_opt);
      tcp_keepalive = onoff;
      return 1;
    }
  } else {
    debug(F101, "TCP keepalive SO_KEEPALIVE unchanged", "", get_keepalive_opt);
    tcp_keepalive = onoff;
    return 1;
  }
#else
  debug(F100, "TCP keepalive SO_KEEPALIVE not defined", "", 0);
#endif /* SO_KEEPALIVE */
#else
  debug(F100, "TCP keepalive SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return (0);
}

int dontroute(int sock, int onoff) {
#ifdef SOL_SOCKET
#ifdef SO_DONTROUTE
  int get_dontroute_opt;
  int set_dontroute_opt;
  SOCKOPT_T x;

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_dontroute = onoff;
      return 1;
    }
  x = sizeof(get_dontroute_opt);
  if (getsockopt(sock, SOL_SOCKET, SO_DONTROUTE, (char *)&get_dontroute_opt,
                 &x)) {
    debug(F111, "TCP dontroute can't get SO_DONTROUTE", ck_errstr(), errno);
  } else if (x != sizeof(get_dontroute_opt)) {
    debug(F111, "TCP dontroute error: SO_DONTROUTE", "len", x);
    debug(F111, "TCP dontroute SO_DONTROUTE", "expected len",
          sizeof(get_dontroute_opt));
    debug(F111, "TCP dontroute SO_DONTROUTE", "dontroute_opt",
          get_dontroute_opt);
  } else if (get_dontroute_opt != onoff) {
    set_dontroute_opt = onoff;
    if (setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, (char *)&set_dontroute_opt,
                   sizeof(set_dontroute_opt))) {
      debug(F111, "TCP dontroute can't clear SO_DONTROUTE", ck_errstr(), errno);
      tcp_dontroute = get_dontroute_opt;
    } else {
      debug(F101, "TCP dontroute new SO_DONTROUTE", "", set_dontroute_opt);
      tcp_dontroute = onoff;
      return 1;
    }
  } else {
    debug(F101, "TCP dontroute SO_DONTROUTE unchanged", "", get_dontroute_opt);
    tcp_dontroute = onoff;
    return 1;
  }
#else
  debug(F100, "TCP dontroute SO_DONTROUTE not defined", "", 0);
#endif /* SO_DONTROUTE */
#else
  debug(F100, "TCP dontroute SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return (0);
}

int no_delay(int sock, int onoff) {
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
  int get_nodelay_opt;
  int set_nodelay_opt;
  SOCKOPT_T x;

#ifdef IKSD
  if (!inserver)
#endif /* IKSD */
    if (sock == -1 ||
        nettype != NET_TCPA && nettype != NET_TCPB && nettype != NET_SSH ||
        ttmdm >= 0) {
      tcp_nodelay = onoff;
      return (1);
    }
  x = sizeof(get_nodelay_opt);
  if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&get_nodelay_opt,
                 &x)) {
    debug(F111, "TCP no_delay can't get TCP_NODELAY", ck_errstr(), errno);
  } else if (x != sizeof(get_nodelay_opt)) {
    debug(F111, "TCP no_delay error: TCP_NODELAY", "len", x);
    debug(F111, "TCP no_delay TCP_NODELAY", "expected len",
          sizeof(get_nodelay_opt));
    debug(F111, "TCP no_delay TCP_NODELAY", "nodelay_opt", get_nodelay_opt);
  } else if (get_nodelay_opt != onoff) {
    set_nodelay_opt = onoff;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&set_nodelay_opt,
                   sizeof(set_nodelay_opt))) {
      debug(F111, "TCP no_delay can't clear TCP_NODELAY", ck_errstr(), errno);
      tcp_nodelay = get_nodelay_opt;
    } else {
      debug(F101, "TCP no_delay new TCP_NODELAY", "", set_nodelay_opt);
      tcp_nodelay = onoff;
      return 1;
    }
  } else {
    debug(F101, "TCP no_delay TCP_NODELAY unchanged", "", get_nodelay_opt);
    tcp_nodelay = onoff;
    return (1);
  }
#else
  debug(F100, "TCP no_delay TCP_NODELAY not defined", "", 0);
#endif /* TCP_NODELAY */
#else
  debug(F100, "TCP no_delay SO_SOCKET not defined", "", 0);
#endif /* SOL_SOCKET */
  return 0;
}
#endif /* NOTCPOPTS */

#ifdef TCPSOCKET
#ifndef NOLISTEN

#ifdef BSDSELECT

/****** THIS SECTION ADDED BY STEVE RANCE - OS9 NETWORK SERVER
 *       ------------------------------------------------------
 *
 *       Due to OS9's Lack of a select() call, the following seems to be
 *       enough to fool the rest of the code into compiling. The only
 *       effect that I can see is using control L to refresh the status
 *       display gets qued up until some network packets arrive.
 *
 *       This solution is by no means elegant but works enough to be
 *       a (the) solution.
 *
 *       Also with the defines I had specified in my makefile I had to
 *       have an #endif right at the end of the file when compiling.
 *       I did not bother speding time to find out why.
 *
 *       COPTS   = -to=osk -d=OSK -d=TCPSOCKET -d=SELECT -d=VOID=void -d=SIG_V \
 *          -d=DYNAMIC -d=PARSENSE -d=KANJI -d=MYCURSES -d=ZFCDAT \
 *          -d=CK_APC -d=CK_REDIR -d=RENAME -d=CK_TTYFD -d=NOOLDMODEMS \
 *          -d=CK_ANSIC -d=CK_XYZ -tp=68040d -l=netdb.l -l=socklib.l \
 *          -l=termlib.l -l=math.l -l=sys_clib.l
 *
 *       stever@ozemail.com.au
 */

#include <sys/time.h>
#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */
#endif /* BSDSELECT */

#ifdef SELECT
#endif /* SELECT */

#ifdef NOTUSED
/* T C P S O C K E T _ O P E N -- Open a preexisting socket number */

int tcpsocket_open(name, lcl, nett, timo)
char *name;
int *lcl;
int nett;
int timo {
  int on = 1;
  static struct servent *service, servrec;
  static struct hostent *host;
  static struct sockaddr_in saddr;
  static int saddrlen;
#ifdef BSDSELECT
  fd_set rfds;
  struct timeval tv;
#else
#ifdef BELLSELECT
  fd_set rfds;
#else
  fd_set rfds;
  fd_set rfds;
  struct timeval {
    long tv_sec;
    long tv_usec;
  } tv;
#endif /* BELLSELECT */
#endif /* BSDSELECT */

  debug(F101, "tcpsocket_open nett", "", nett);
  *ipaddr = '\0';

  if (nett != NET_TCPB)
    return (-1); /* BSD socket support */

  netclos();                           /* Close any previous connection. */
  ckstrncpy(namecopy, name, NAMECPYL); /* Copy the hostname. */
  /* fdc's version from 4 Dec 2005 works ok */
  if (ttnproto != NP_TCPRAW)
    ttnproto = NP_NONE; /* No protocol selected yet. */
  debug(F110, "tcpsocket_open namecopy", namecopy, 0);

  /* Assign the socket number to ttyfd and then fill in tcp structures */
  ttyfd = atoi(&name[1]);
  debug(F111, "tcpsocket_open", "ttyfd", ttyfd);

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
  setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);

#ifdef TCP_NODELAY
  no_delay(ttyfd, tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
  keepalive(ttyfd, tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
  ck_linger(ttyfd, tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
  sendbuf(ttyfd, tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
  recvbuf(ttyfd, tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

  /* Get the name of the host we are connected to */

  saddrlen = sizeof(saddr);
  getpeername(ttyfd, (struct sockaddr *)&saddr, &saddrlen);

  ckstrncpy(ipaddr, (char *)inet_ntoa(saddr.sin_addr), 20);

  if (tcp_rdns == SET_ON
#ifndef NOHTTP
      && (tcp_http_proxy == NULL)
#endif /* NOHTTP */
  ) { /* Reverse DNS */
    if (!quiet) {
      printf(" Reverse DNS Lookup... ");
      fflush(stdout);
    }
    host = gethostbyaddr((char *)&saddr.sin_addr, 4, PF_INET);
    debug(F110, "tcpsocket_open gethostbyaddr", host ? "OK" : "FAILED", 0);
    if (host) {
      host = ck_copyhostent(host);
      debug(F100, "tcpsocket_open gethostbyaddr != NULL", "", 0);
      if (!quiet) {
        printf("(OK)\n");
        fflush(stdout);
      }
      ckstrncpy(name, host->h_name, 80);
      ckstrncat(name, ":", 80);
      ckstrncat(name, ckuitoa(ntohs(saddr.sin_port)), 80);
      if (!quiet
#ifndef NOICP
          && !doconx
#endif /* NOICP */
      )
        printf("%s connected on port %d\n", host->h_name,
               ntohs(saddr.sin_port));
    } else if (!quiet)
      printf("Failed\n");
  } else if (!quiet)
    printf("(OK)\n");

  if (tcp_rdns != SET_ON || !host) {
    ckstrncpy(name, ipaddr, 80);
    ckstrncat(name, ":", 80);
    ckstrncat(name, ckuitoa(ntohs(saddr.sin_port)), 80);
    if (!quiet
#ifdef NOICP
        && !doconx
#endif /* NOICP */
    )
      printf("%s connected on port %d\n", ipaddr, ntohs(saddr.sin_port));
  }
  if (!quiet)
    fflush(stdout);
  ttnet = nett; /* TCP/IP (sockets) network */

#ifdef RLOGCODE
  if (ntohs(saddr.sin_port) == 513)
    ttnproto = NP_LOGIN;
  else
#endif /* RLOGCODE */
    /* Assume the service is TELNET. */
    /* fdc's code from 2005/12/04 */
    if (ttnproto != NP_TCPRAW)
      ttnproto = NP_TELNET; /* Yes, set global flag. */
  if (tn_ini() < 0)         /* Start/Reset TELNET negotiations */
    if (ttchk() < 0)        /* Did it fail due to connect loss? */
      return (-1);

  if (*lcl < 0)
    *lcl = 1; /* Local mode. */

  return (0); /* Done. */
}
#endif /* NOTUSED */

/*  T C P S R V _ O P E N  --  Open a TCP/IP Server connection  */
/*
  Calling conventions same as ttopen(), except third argument is network
  type rather than modem type.
*/
int tcpsrv_open(char *name, int *lcl, int nett, int timo) {
  char *p;
  int i, x;
  SOCKOPT_T on = 1;
  int ready_to_accept = 0;
  static struct servent *service, *service2, servrec;
  static struct hostent *host;
  static struct sockaddr_in saddr;
  struct sockaddr_in l_addr;
  GSOCKNAME_T l_slen;
  static SOCKOPT_T saddrlen;

#ifdef BSDSELECT
  fd_set rfds;
  struct timeval tv;
#else
#ifdef BELLSELCT
  fd_set rfds;
#else
  fd_set rfds;
  struct timeval {
    long tv_sec;
    long tv_usec;
  } tv;
#endif /* BELLSELECT */
#endif /* BSDSELECT */

  debug(F101, "tcpsrv_open nett", "", nett);
  *ipaddr = '\0';

  if (nett != NET_TCPB)
    return (-1); /* BSD socket support */

  netclos();                           /* Close any previous connection. */
  ckstrncpy(namecopy, name, NAMECPYL); /* Copy the hostname. */
  /* Don't do this. */
  debug(F110, "tcpsrv_open namecopy", namecopy, 0);

  p = namecopy; /* Was a service requested? */
  while (*p != '\0' && *p != ':')
    p++;           /* Look for colon */
  if (*p == ':') { /* Have a colon */
    *p++ = '\0';   /* Get service name or number */
  } else {         /* Otherwise use kermit */
    p = "kermit";
  }
  debug(F110, "tcpsrv_open service requested", p, 0);
  if (isdigit(*p)) { /* Use socket number without lookup */
    service = &servrec;
    service->s_port = htons((unsigned short)atoi(p));
  } else { /* Otherwise lookup the service name */
    service = getservbyname(p, "tcp");
  }
  if (!service && !strcmp("kermit", p)) { /* Use Kermit service port */
    service = &servrec;
    service->s_port = htons(1649);
  }
#ifdef RLOGCODE
  if (service && !strcmp("login", p) && service->s_port != htons(513)) {
    fprintf(stderr, "  Warning: login service on port %d instead of port 513\n",
            ntohs(service->s_port));
    fprintf(stderr, "  Edit SERVICES file if RLOGIN fails to connect.\n");
    debug(F101, "tcpsrv_open login on port", "", ntohs(service->s_port));
  }
#endif /* RLOGCODE */
  if (!service) {
    fprintf(stderr, "Cannot find port for service: %s\n", p);
    debug(F111, "tcpsrv_open can't get service", p, errno);
    errno = 0; /* rather than mislead */
    return (-1);
  }

  /* If we currently have a listen active but port has changed then close */

  debug(F101, "tcpsrv_open checking previous connection", "", tcpsrfd);
  debug(F101, "tcpsrv_open previous tcpsrv_port", "", tcpsrv_port);
  if (tcpsrfd != -1 && tcpsrv_port != ntohs((unsigned short)service->s_port)) {
    debug(F100, "tcpsrv_open closing previous connection", "", 0);
#ifdef TCPIPLIB
    socket_close(tcpsrfd);
#else
    close(tcpsrfd);
#endif /* TCPIPLIB */
    tcpsrfd = -1;
  }
  debug(F100, "tcpsrv_open tcpsrfd", "", tcpsrfd);
  if (tcpsrfd == -1) {

    /* Set up socket structure and get host address */

    bzero((char *)&saddr, sizeof(saddr));
    debug(F100, "tcpsrv_open bzero ok", "", 0);
    saddr.sin_family = AF_INET;
    if (tcp_address) {
#ifdef INADDRX
      inaddrx = inet_addr(tcp_address);
      saddr.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
      saddr.sin_addr.s_addr = inet_addr(tcp_address);
#endif /* INADDRX */
    } else
      saddr.sin_addr.s_addr = INADDR_ANY;

    /* Get a file descriptor for the connection. */

    saddr.sin_port = service->s_port;
    ipaddr[0] = '\0';

    debug(F100, "tcpsrv_open calling socket", "", 0);
    if ((tcpsrfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("TCP socket error");
      debug(F101, "tcpsrv_open socket error", "", errno);
      return (-1);
    }
    errno = 0;

    /* Specify the Port may be reused */

    debug(F100, "tcpsrv_open calling setsockopt", "", 0);
    x = setsockopt(tcpsrfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
    debug(F101, "tcpsrv_open setsockopt", "", x);

    /* Now bind to the socket */
    printf("\nBinding socket to port %d ...\n",
           ntohs((unsigned short)service->s_port));
    if (bind(tcpsrfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
      i = errno; /* Save error code */
#ifdef TCPIPLIB
      socket_close(tcpsrfd);
#else  /* TCPIPLIB */
      close(tcpsrfd);
#endif /* TCPIPLIB */
      tcpsrfd = -1;
      tcpsrv_port = 0;
      ttyfd = -1;
      wasclosed = 1;
      errno = i; /* and report this error */
      debug(F101, "tcpsrv_open bind errno", "", errno);
      printf("?Unable to bind to socket (errno = %d)\n", errno);
      return (-1);
    }
    debug(F100, "tcpsrv_open bind OK", "", 0);
    printf("Listening ...\n");
    if (listen(tcpsrfd, 15) < 0) {
      i = errno; /* Save error code */
#ifdef TCPIPLIB
      socket_close(tcpsrfd);
#else  /* TCPIPLIB */
      close(tcpsrfd);
#endif /* TCPIPLIB */
      tcpsrfd = -1;
      tcpsrv_port = 0;
      ttyfd = -1;
      wasclosed = 1;
      errno = i; /* And report this error */
      debug(F101, "tcpsrv_open listen errno", "", errno);
      return (-1);
    }
    debug(F100, "tcpsrv_open listen OK", "", 0);
    tcpsrv_port = ntohs((unsigned short)service->s_port);
  }

  printf("\nWaiting to Accept a TCP/IP connection on port %d ...\n",
         ntohs((unsigned short)service->s_port));
  saddrlen = sizeof(saddr);

#ifdef BSDSELECT
  tv.tv_sec = tv.tv_usec = 0L;
  if (timo < 0)
    tv.tv_usec = (long)-timo * 10000L;
  else
    tv.tv_sec = timo;
  debug(F101, "tcpsrv_open BSDSELECT", "", timo);
#else
  debug(F101, "tcpsrv_open not BSDSELECT", "", timo);
#endif /* BSDSELECT */

  if (timo) {
    while (!ready_to_accept) {
#ifdef BSDSELECT
      FD_ZERO(&rfds);
      FD_SET(tcpsrfd, &rfds);
      ready_to_accept = ((select(FD_SETSIZE,
#ifdef __DECC
#ifdef INTSELECT
                                 (int *)
#else  /* def INTSELECT */
                                 (fd_set *)
#endif /* def INTSELECT [else] */
#endif /* __DECC */
                                 &rfds,
                                 NULL, NULL, &tv) > 0) &&
                         FD_ISSET(tcpsrfd, &rfds));
#else  /* BSDSELECT */
#ifdef IBMSELECT
#define ck_sleepint 250
      ready_to_accept =
          (select(&tcpsrfd, 1, 0, 0,
                  timo < 0 ? -timo : (timo > 0 ? timo * 1000L : ck_sleepint)) ==
           1);
#else
#ifdef BELLSELECT
      FD_ZERO(rfds);
      FD_SET(tcpsrfd, rfds);
            ready_to_accept =
              ((select(128, rfds, NULL, NULL, timo < 0 ? -timo :
                      (timo > 0 ? timo * 1000L)) > 0) &&
               FD_ISSET(tcpsrfd, rfds));
#else
      /* Try this - what's the worst that can happen... */

      FD_ZERO(&rfds);
      FD_SET(tcpsrfd, &rfds);
      ready_to_accept =
          ((select(FD_SETSIZE, (fd_set *)&rfds, NULL, NULL, &tv) > 0) &&
           FD_ISSET(tcpsrfd, &rfds));

#endif /* BELLSELECT */
#endif /* IBMSELECT */
#endif /* BSDSELECT */
    }
  }
  if (ready_to_accept || timo == 0) {
    if ((ttyfd = accept(tcpsrfd, (struct sockaddr *)&saddr, &saddrlen)) < 0) {
      i = errno; /* save error code */
#ifdef TCPIPLIB
      socket_close(tcpsrfd);
#else  /* TCPIPLIB */
      close(tcpsrfd);
#endif /* TCPIPLIB */
      ttyfd = -1;
      wasclosed = 1;
      tcpsrfd = -1;
      tcpsrv_port = 0;
      errno = i; /* and report this error */
      debug(F101, "tcpsrv_open accept errno", "", errno);
      return (-1);
    }
    setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
    no_delay(ttyfd, tcp_nodelay);
    debug(F101, "tcpsrv_open no_delay", "", tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
    keepalive(ttyfd, tcp_keepalive);
    debug(F101, "tcpsrv_open keepalive", "", tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
    ck_linger(ttyfd, tcp_linger, tcp_linger_tmo);
    debug(F101, "tcpsrv_open linger", "", tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
    sendbuf(ttyfd, tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
    recvbuf(ttyfd, tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

    ttnet = nett;     /* TCP/IP (sockets) network */
    tcp_incoming = 1; /* This is an incoming connection */
    sstelnet = 1;     /* Do server-side Telnet protocol */

    /* See if the service is TELNET. */
    x = (unsigned short)service->s_port;
    service2 = getservbyname("telnet", "tcp");
    if (service2 && x == service2->s_port) {
      /* fdc 2005/12/04 */
      if (ttnproto != NP_TCPRAW) /* Yes and if raw port not requested */
        ttnproto = NP_TELNET;    /* set protocol to TELNET. */
    }
    ckstrncpy(ipaddr, (char *)inet_ntoa(saddr.sin_addr), 20);
    if (tcp_rdns) {
      if (!quiet) {
        printf(" Reverse DNS Lookup... ");
        fflush(stdout);
      }
      if ((host = gethostbyaddr((char *)&saddr.sin_addr, 4, PF_INET))) {
        host = ck_copyhostent(host);
        debug(F100, "tcpsrv_open gethostbyaddr != NULL", "", 0);
        if (!quiet) {
          printf("(OK)\n");
          fflush(stdout);
        }
        name[0] = '*';
        ckstrncpy(&name[1], host->h_name, 78);
        ckstrncat(name, ":", 80 - strlen(name));
        ckstrncat(name, p, 80 - strlen(name));
        if (!quiet
#ifndef NOICP
            && !doconx
#endif /* NOICP */
        )
          printf("%s connected on port %s\n", host->h_name, p);
      } else {
        if (!quiet)
          printf("Failed.\n");
      }
    } else if (!quiet)
      printf("(OK)\n");

    if (!tcp_rdns || !host) {
      ckstrncpy(name, ipaddr, 80);
      ckstrncat(name, ":", 80);
      ckstrncat(name, ckuitoa(ntohs(saddr.sin_port)), 80);
      if (!quiet
#ifndef NOICP
          && !doconx
#endif /* NOICP */
      )
        printf("%s connected on port %d\n", ipaddr, ntohs(saddr.sin_port));
    }
    if (!quiet)
      fflush(stdout);

    /* Find out our own IP address. */
    l_slen = sizeof(l_addr);
    bzero((char *)&l_addr, l_slen);
    if (!getsockname(ttyfd, (struct sockaddr *)&l_addr, &l_slen)) {
      char *s = (char *)inet_ntoa(l_addr.sin_addr);
      ckstrncpy(myipaddr, s, 20);
      debug(F110, "getsockname", myipaddr, 0);
    }

    if (tn_ini() < 0)    /* Start TELNET negotiations. */
      if (ttchk() < 0) { /* Disconnected? */
        i = errno;       /* save error code */
#ifdef TCPIPLIB
        socket_close(tcpsrfd);
#else  /* TCPIPLIB */
        close(tcpsrfd);
#endif /* TCPIPLIB */
        ttyfd = -1;
        wasclosed = 1;
        tcpsrfd = -1;
        tcpsrv_port = 0;
        errno = i; /* and report this error */
        debug(F101, "tcpsrv_open accept errno", "", errno);
        return (-1);
      }
    debug(F101, "tcpsrv_open service", "", x);
    if (*lcl < 0) /* Set local mode. */
      *lcl = 1;

    return (0); /* Done. */
  } else {
    i = errno; /* save error code */
#ifdef TCPIPLIB
    socket_close(tcpsrfd);
#else  /* TCPIPLIB */
    close(tcpsrfd);
#endif /* TCPIPLIB */
    ttyfd = -1;
    wasclosed = 1;
    tcpsrfd = -1;
    tcpsrv_port = 0;
    errno = i; /* and report this error */
    debug(F101, "tcpsrv_open accept errno", "", errno);
    return (-1);
  }
}
#endif /* NOLISTEN */
#endif /* TCPSOCKET */
#endif /* NONET */

#ifdef TCPSOCKET
char *ckname2addr(char *name) {
  struct hostent *host;

  if (name == NULL || *name == '\0')
    return ("");

  host = gethostbyname(name);
  if (host) {
    host = ck_copyhostent(host);
    return (inet_ntoa(*((struct in_addr *)host->h_addr)));
  }
  return ("");
}

char *ckaddr2name(char *addr) {
  struct hostent *host;
  struct in_addr sin_addr;

  if (addr == NULL || *addr == '\0')
    return ("");

  sin_addr.s_addr = inet_addr(addr);
  host = gethostbyaddr((char *)&sin_addr, 4, AF_INET);
  if (host) {
    host = ck_copyhostent(host);
    return ((char *)host->h_name);
  }
  return ("");
}
#endif /* TCPSOCKET */

unsigned long peerxipaddr = 0L;

char *ckgetpeer() {
#ifdef TCPSOCKET
  static char namebuf[256];
  static struct hostent *host;
  static struct sockaddr_in saddr;
#ifdef GPEERNAME_T
  static GPEERNAME_T saddrlen;
#else
#ifdef MACOSX10
  static unsigned int saddrlen;
#else
#ifdef CK_64BIT
  static socklen_t saddrlen;
#else
  static int saddrlen;
#endif /* CK_64BIT */
#endif /* MACOSX10 */
#endif /* GPEERNAME_T */
  saddrlen = sizeof(saddr);
  if (getpeername(ttyfd, (struct sockaddr *)&saddr, &saddrlen) < 0) {
    debug(F111, "ckgetpeer failure", ckitoa(ttyfd), errno);
    return (NULL);
  }
  host = gethostbyaddr((char *)&saddr.sin_addr, 4, AF_INET);
  if (host) {
    host = ck_copyhostent(host);
    ckstrncpy(namebuf, (char *)host->h_name, 80);
  } else {
    ckstrncpy(namebuf, (char *)inet_ntoa(saddr.sin_addr), 80);
  }
  peerxipaddr = ntohl(saddr.sin_addr.s_addr);
  debug(F111, "ckgetpeer", namebuf, peerxipaddr);
  return (namebuf);
#else
  return (NULL);
#endif /* TCPSOCKET */
}

#ifndef NOTCPIP
/* Get fully qualified IP hostname */

char *ckgetfqhostname(char *name) {
#ifdef NOCKGETFQHOST

  return (name);

#else /* If the following code dumps core, define NOCKGETFQHOST and rebuild.   \
       */

  static char namebuf[256];
  struct hostent *host = NULL;
  struct sockaddr_in r_addr;
  int i;

  debug(F110, "ckgetfqhn()", name, 0);

  ckstrncpy(namebuf, name, 256);
  namebuf[255] = '\0';
  i = ckindex(":", namebuf, 0, 0, 0);
  if (i)
    namebuf[i - 1] = '\0';

  bzero((char *)&r_addr, sizeof(r_addr));

  host = gethostbyname(namebuf);
  if (host) {
    host = ck_copyhostent(host);
    debug(F100, "ckgetfqhn() gethostbyname != NULL", "", 0);
    r_addr.sin_family = host->h_addrtype;
#ifdef HADDRLIST
#ifdef h_addr
    /* This is for trying multiple IP addresses - see <netdb.h> */
    if (!(host->h_addr_list))
      goto exit_func;
    bcopy(host->h_addr_list[0], (caddr_t)&r_addr.sin_addr, host->h_length);
#else
    bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* h_addr */
#else  /* HADDRLIST */
    bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* HADDRLIST */
    debug(F111, "BCOPY", "host->h_length", host->h_length);

    host = gethostbyaddr((char *)&r_addr.sin_addr, 4, PF_INET);
    if (host) {
      host = ck_copyhostent(host);
      debug(F100, "ckgetfqhn() gethostbyaddr != NULL", "", 0);
      ckstrncpy(namebuf, host->h_name, 256);
    }
  }

#ifdef HADDRLIST
#ifdef h_addr
exit_func:
#endif /* h_addr */
#endif /* HADDRLIST */

  if (i > 0)
    ckstrncat(namebuf, &name[i - 1],
              256 - strlen(namebuf) - strlen(&name[i - 1]));
  debug(F110, "ckgetfqhn()", namebuf, 0);
  return (namebuf);
#endif /* NOCKGETFQHOST */
}

void setnproto(char *p) {
  if (!isdigit(*p)) {
    if (!strcmp("kermit", p))
      ttnproto = NP_KERMIT;
    else if (!strcmp("telnet", p))
      ttnproto = NP_TELNET;
    else if (!strcmp("http", p))
      ttnproto = NP_TCPRAW;
#ifdef RLOGCODE
    else if (!strcmp("login", p))
      ttnproto = NP_RLOGIN;
#endif /* RLOGCODE */
    else
      ttnproto = NP_NONE;
  } else {
    switch (atoi(p)) {
    case 23: /* Telnet */
      ttnproto = NP_TELNET;
      break;
    case 513:
      ttnproto = NP_RLOGIN;
      break;
    case 1649:
      ttnproto = NP_KERMIT;
      break;
    case 80: /* HTTP */
      ttnproto = NP_TCPRAW;
      break;
    default:
      ttnproto = NP_NONE;
      break;
    }
  }
}

/* ckgetservice() is used to determine the port number for a given */
/* service taking into account the use of DNS SRV records.         */

static struct servent servrec;

/* prototype for static functions - fdc 01 December 2022 */
static struct servent *ckgetservice(char *, char *, char *, int);

static struct servent *ckgetservice(char *hostname, char *servicename, char *ip,
                                    int iplen) {
  struct servent *service = NULL;
#ifdef CK_DNS_SRV
  struct sockaddr *dns_addrs = NULL;
  int dns_naddrs = 0;
#endif /* CK_DNS_SRV */

  if (isdigit(*servicename)) { /* Use socket number without lookup */
    service = &servrec;
    service->s_port = htons((unsigned short)atoi(servicename));
  } else { /* Otherwise lookup the service name */
#ifdef CK_DNS_SRV
    if (tcp_dns_srv && !quiet) {
      printf(" DNS SRV Lookup... ");
      fflush(stdout);
    }
    if (tcp_dns_srv &&
        locate_srv_dns(hostname, servicename, "tcp", &dns_addrs, &dns_naddrs)) {
      /* Use the first one.  Eventually we should cycle through all */
      /* the returned IP addresses and port numbers. */
      struct sockaddr_in *sin = NULL;
#ifdef BETADEBUG
      int i;
      printf("\r\n");
      for (i = 0; i < dns_naddrs; i++) {
        sin = (struct sockaddr_in *)&dns_addrs[i];
        printf("dns_addrs[%d] = %s %d\r\n", i, (char *)inet_ntoa(sin->sin_addr),
               ntohs(sin->sin_port));
      }
#endif /* BETADEBUG */
      sin = (struct sockaddr_in *)&dns_addrs[0];
      if (ip && iplen > 0)
        ckstrncpy(ip, (char *)inet_ntoa(sin->sin_addr), iplen);
      service = &servrec;
      service->s_port = sin->sin_port;

      free(dns_addrs);
      dns_addrs = NULL;
      dns_naddrs = 0;
    } else
#endif /* CK_DNS_SRV */
      service = getservbyname(servicename, "tcp");
  }
  if (!service) {
    if (!ckstrcmp("kermit", servicename, -1, 0)) { /* Kermit service port */
      service = &servrec;
      service->s_port = htons(1649);
    } else if (!ckstrcmp("telnet", servicename, -1, 0)) { /* Telnet port */
      service = &servrec;
      service->s_port = htons(23);
    } else if (!ckstrcmp("http", servicename, -1, 0)) {
      service = &servrec;
      service->s_port = htons(80);
    }
#ifdef RLOGCODE
    else if (!ckstrcmp("login", servicename, -1, 0)) {
      service = &servrec;
      service->s_port = htons(513);
    }
#endif /* RLOGCODE */
  }
  return (service);
}
#endif /* NOTCPIP */

/*  N E T O P E N  --  Open a network connection  */
/*
  Calling conventions same as ttopen(), except third argument is network
  type rather than modem type.  Designed to be called from within ttopen.
  Call with:
    name - name of host (or host:service),
    lcl  - local-mode flag to be set if this function succeeds,
    nett - network type (value defined in ckcnet.h)
*/

int netopen(char *name, int *lcl, int nett) {
  char *p;
  int i, x, dns = 0;
#ifdef TCPSOCKET
  int isconnect = 0;
#ifdef SO_OOBINLINE
  int on = 1;
#endif /* SO_OOBINLINE */
  struct servent *service = NULL;
  struct hostent *host = NULL;
  struct sockaddr_in r_addr;
  struct sockaddr_in sin;
  struct sockaddr_in l_addr;
  GSOCKNAME_T l_slen;

#ifdef INADDRX
  /* inet_addr() is of type struct in_addr */
  struct in_addr iax;
#else
#ifdef INADDR_NONE
  struct in_addr iax;
#else  /* INADDR_NONE */
  long iax;
#endif /* INADDR_NONE */
#endif /* INADDRX */
#endif /* TCPSOCKET */

#ifdef ANYX25
  extern int revcall, closgr, cudata;
  extern char udata[];
#endif /* ANYX25 */

#ifdef IBMX25 /* Variables for IBM X25 */
  extern int x25port;          /* Logical port to use */
  extern x25addr_t local_nua;  /* Local X.25 address */
  extern x25addr_t remote_nua; /* Remote X.25 address */
  extern char x25name[];       /* X25 device name (sx25a0) */
  extern char x25dev[];        /* X25 device file /dev/x25pkt */
  ulong bind_flags = 0;        /* Flags for binding the X25 stream */
  ulong token = 0;             /* Temporary return code */
#endif        /* IBMX25 */

  debug(F101, "netopen nett", "", nett);
  *ipaddr = '\0'; /* Initialize IP address string */

#ifdef IBMX25
  /* riehm */
  if (nett == NET_IX25) { /* IBM AIX X.25 */
    netclos();            /* Close any previous net connection */
    ttnproto = NP_NONE;   /* No protocol selected yet */

    /* find out who we are - this is not so easy on AIX */
    /* riehm: need to write the code that finds this out
     * automatically, or at least allow it to be configured
     * somehow
     */
    if (!local_nua[0] && !x25local_nua(local_nua)) {
      return (-1);
    }

    /* Initialise the X25 API (once per process? once per connection?) */

    debug(F110, "Opening ", x25dev, 0);
    /* set O_NDELAY to allow polling? */
    if ((ttyfd = open(x25dev, O_RDWR)) < 0) {
      perror("X.25 device open error");
      debug(F101, "netopen: device open error", "", errno);
      return (-1);
    }

    /* push the NPI onto the STREAM */
    if (ioctl(ttyfd, I_PUSH, "npi") < 0) {
      close(ttyfd);
      ttyfd = -1;
      wasclosed = 1;
      perror("kermit: netopen(): couldn't push npi on the X25 stream");
      debug(F101, "netopen: can't push npi on the X25 stream", "", errno);
      return (-1);
    }

    /* set up server mode - bind the x25 port and wait for
     * incoming connections
     */
    if (name[0] == '*') { /* Server */
      /* set up a server - see the warning in x25bind() */
      bind_flags |= TOKEN_REQUEST;

      /* bind kermit to the local X25 address */
      token = x25bind(ttyfd, local_nua, udata, (int)strlen(udata), 1, x25port,
                      bind_flags);
      if (token < 0) {
        debug(F100, "netopen: couldn't bind to local X25 address", "", 0);
        netclos();
        return (-1);
      }
      /* Currently not connected to a remote host */

      remote_nua[0] = '\0';

      /* store the fd so that incoming calls can have their own fd
       * This is almost support for a true server (ie: a'la ftpd)
       * but we're not quite there yet.
       * used in netclos()
       */
      x25serverfd = ttyfd;
      /*
       * wait for an incoming call
       * this should happen in the "server" command and not in
       * the "set host *" command.
       */
      if ((ttyfd = x25getcall(ttyfd)) < 0) {
        netclos();
        return (-1);
      }
    } else { /* Client */
      /* Bind kermit to the local X25 address */
      token =
          x25bind(ttyfd, local_nua, (char *)NULL, 0, 0, x25port, bind_flags);
      if (token < 0) {
        debug(F100, "netopen: couldn't bind to local X25 address", "", 0);
        netclos();
        return (-1);
      }
      /* riehm: this should be done via the CONNECT command, not HOST! */
      {
        x25serverfd = 0;
        /* call the remote host */
        /* name == address of remote host as char* */
        if (x25call(ttyfd, name, udata) < 0) {
          debug(F100, "netopen: couldn't connect to remote X25 address", "", 0);
          netclos();
          return (-1);
        }
        strcpy(remote_nua, name);
      }
    }
    ttnet = nett; /* AIX X.25 network */
    if (lcl)
      if (*lcl < 0)
        *lcl = 1; /* Local mode */
    return (0);

  } else /* Note that IBMX25 support can coexist with TCP/IP support. */
#endif /* IBMX25 */

    /*   Add support for other networks here. */

    if (nett != NET_TCPB)
      return (-1); /* BSD socket support */

#ifdef TCPSOCKET
  netclos();                           /* Close any previous connection. */
  ckstrncpy(namecopy, name, NAMECPYL); /* Copy the hostname. */
  debug(F110, "netopen namecopy", namecopy, 0);

#ifndef NOLISTEN
  if (name[0] == '*')
    return (tcpsrv_open(name, lcl, nett, 0));
#endif /* NOLISTEN */

  p = namecopy; /* Was a service requested? */
  while (*p != '\0' && *p != ':')
    p++;           /* Look for colon */
  if (*p == ':') { /* Have a colon */
    debug(F110, "netopen name has colon", namecopy, 0);
    *p++ = '\0'; /* Get service name or number */
#ifdef CK_URL
    /*
       Here we have to check for various popular syntaxes:
       host:port (our original syntax)
       URL such as telnet:host or telnet://host/
       Or even telnet://user:password@host:port/path/
       Or a malformed URL such as generated by Netscape 4.0 like:
       telnet:telnet or telnet::host.
    */

    /*
     * REPLACE THIS CODE WITH urlparse() but not on the day of the
     * C-Kermit 8.0 RELEASE.
     */

    if (*p == ':') /* a second colon */
      *p++ = '\0'; /* get rid of that one too */
    while (*p == '/')
      *p++ = '\0';       /* and slashes */
    x = strlen(p);       /* Length of remainder */
    if (p[x - 1] == '/') /* If there is a trailing slash */
      p[x - 1] = '\0';   /* remove it. */
    debug(F110, "netopen namecopy after stripping", namecopy, 0);
    debug(F110, "netopen p after stripping", p, 0);
    service = getservbyname(namecopy, "tcp");
    if (service ||
#ifdef RLOGCODE
        !ckstrcmp("rlogin", namecopy, NAMECPYL, 0) ||
#endif /* RLOGCODE */
        !ckstrcmp("iksd", namecopy, NAMECPYL, 0)) {
      char temphost[256], tempservice[80], temppath[256];
      char *q = p, *r = p, *w = p;
      int uidfound = 0;
      extern char pwbuf[];
      extern int pwflg, pwcrypt;

      if (ttnproto == NP_DEFAULT)
        setnproto(namecopy);

      /* Check for userid and possibly password */
      while (*p != '\0' && *p != '@')
        p++; /* look for @ */
      if (*p == '@') {
        /* found username and perhaps password */
        debug(F110, "netopen namecopy found @", "", 0);
        *p = '\0';
        p++;
        while (*w != '\0' && *w != ':')
          w++;
        if (*w == ':')
          *w++ = '\0';
        /* r now points to username, save it and the password */
        debug(F110, "netopen namecopy username", r, 0);
        debug(F110, "netopen namecopy password", w, 0);
        uidfound = 1;
        if (strcmp(uidbuf, r) || *w)
          ckstrncpy(pwbuf, w, PWBUFL + 1);
        ckstrncpy(uidbuf, r, UIDBUFLEN);
        pwflg = 1;
        pwcrypt = 0;
        q = p; /* Host after user and pwd */
      } else {
        p = q; /* No username or password */
      }
      /* Now we must look for the optional port. */
      debug(F110, "netopen x p", p, 0);
      debug(F110, "netopen x q", q, 0);

      /* Look for the port/service or a file/directory path */
      while (*p != '\0' && *p != ':' && *p != '/')
        p++;
      if (*p == ':') {
        debug(F110, "netopen found port", q, 0);
        *p++ = '\0'; /* Found a port name or number */
        r = p;

        /* Look for the end of port/service or a file/directory path */
        while (*p != '\0' && *p != '/')
          p++;
        if (*p == '/')
          *p++ = '\0';

        debug(F110, "netopen port", r, 0);
        ckstrncpy(tempservice, r, 80);
        ckstrncpy(temphost, q, 256);
        ckstrncpy(temppath, p, 256);
        ckstrncpy(namecopy, temphost, NAMECPYL);
        debug(F110, "netopen tempservice", tempservice, 0);
        debug(F110, "netopen temphost", temphost, 0);
        debug(F110, "netopen temppath", temppath, 0);

        /* move port/service to a buffer that won't go away */
        x = strlen(namecopy);
        p = namecopy + x + 1;
        ckstrncpy(p, tempservice, NAMECPYL - x);
      } else {
        /* Handle a path if we found one */
        if (*p == '/')
          *p++ = '\0';
        ckstrncpy(temppath, p, 256);

        /* We didn't find another port, but if q is a service */
        /* then assume that namecopy is actually a host.      */
        if (getservbyname(q, "tcp")) {
          p = q;
        } else {
#ifdef RLOGCODE
          /* rlogin is not a valid service */
          if (!ckstrcmp("rlogin", namecopy, 6, 0)) {
            ckstrncpy(namecopy, "login", NAMECPYL);
          }
#endif /* RLOGCODE */
          /* iksd is not a valid service */
          if (!ckstrcmp("iksd", namecopy, 6, 0)) {
            ckstrncpy(namecopy, "kermit", NAMECPYL);
          }
          /* Reconstruct namecopy */
          ckstrncpy(tempservice, namecopy, 80);
          ckstrncpy(temphost, q, 256);
          ckstrncpy(namecopy, temphost, NAMECPYL);
          debug(F110, "netopen tempservice", tempservice, 0);
          debug(F110, "netopen temphost", temphost, 0);
          debug(F110, "netopen temppath", temppath, 0);

          /* move port/service to a buffer that won't go away */
          x = strlen(namecopy);
          p = namecopy + x + 1;
          ckstrncpy(p, tempservice, NAMECPYL - x - 1);
        }
      }
      debug(F110, "netopen URL result: host", namecopy, 0);
      debug(F110, "netopen URL result: service", p, 0);
      debug(F110, "netopen URL result: path", temppath, 0);

#ifdef IKS_GET
      /* If we have set a path specified, we need to try to GET it */
      /* But we have another problem, we have to login first.  How */
      /* do we specify that a login must be done before the GET?   */
      /* The user's name if specified is in 'userid' and the       */
      /* password if any is in 'pwbuf'.                            */
      if (temppath[0]) {
        extern int action;
        extern char *cmarg;

        if (!uidfound) {
          /* If no userid was specified as part of the URL but
           * a path was specified, then we
           * set the user name to anonymous and the password
           * to the current userid.
           */
          ckstrncpy(pwbuf, uidbuf, PWBUFL);
          ckstrncat(pwbuf, "@", PWBUFL);
          pwflg = 1;
          pwcrypt = 0;
          ckstrncpy(uidbuf, "anonymous", UIDBUFLEN);
        }

        /*
         * If a file path was specified we perform the GET
         * operation and then terminate the connection.
         *
         * If a directory was given instead of a file, then
         * we should REMOTE CD to the directory and list its
         * contents.  But how do we tell the difference?
         */
        makestr(&cmarg, temppath);
        action = 'r';
      }
#endif /* IKS_GET */
    }
#endif /* CK_URL */
  } else { /* Otherwise use telnet */
    p = "telnet";
  }
  /*
    By the time we get here, namecopy[] should hold the null-terminated
    hostname or address, and p should point to the service name or number.
  */
  debug(F110, "netopen host", namecopy, 0);
  debug(F110, "netopen service requested", p, 0);

  /* Use the service port to set the default protocol type if necessary */
  if (ttnproto == NP_DEFAULT)
    setnproto(p);

  ckstrncpy(namecopy2, namecopy, NAMECPYL);
  service = ckgetservice(namecopy, p, namecopy, NAMECPYL);
  if (!service) {
    fprintf(stderr, "Can't find port for service %s\n", p);
    debug(F101, "netopen can't get service", "", errno);
    errno = 0; /* (rather than mislead) */
    return (-1);
  } else {
    if (!ckstrcmp(namecopy, namecopy2, -1, 0))
      namecopy2[0] = '\0';
    ckstrncpy(svcbuf, ckuitoa(ntohs(service->s_port)), sizeof(svcbuf));
    debug(F110, "netopen service ok", svcbuf, 0);
  }

#ifdef RLOGCODE
  if (service && !strcmp("login", p) && service->s_port != htons(513)) {
    fprintf(stderr, "  Warning: login service on port %d instead of port 513\n",
            ntohs(service->s_port));
    fprintf(stderr, "  Edit SERVICES file if RLOGIN fails to connect.\n");
    debug(F101, "tcpsrv_open login on port", "", ntohs(service->s_port));
  }
#endif /* RLOGCODE */

#ifndef NOHTTP
  /* For HTTP connections we must preserve the original hostname and */
  /* service requested so we can include them in the Host header.    */
  ckmakmsg(http_host_port, sizeof(http_host_port), namecopy, ":",
           ckitoa(ntohs(service->s_port)), NULL);

  /* 'namecopy' contains the name of the host to which we want to connect */
  /* 'svcbuf'   contains the service name                                 */
  /* 'service->s_port' contains the port number in network byte order     */

  /* If we are using an http proxy, we need to create a buffer containing */
  /*   hostname:port-number                                               */
  /* to pass to the http_connect() function.  Then we need to replace     */
  /* 'namecopy' with the name of the proxy server and the service->s_port */
  /* with the port number of the proxy (default port 80).                 */

  if (tcp_http_proxy) {
    ckmakmsg(proxycopy, sizeof(proxycopy), namecopy, ":",
             ckuitoa(ntohs(service->s_port)), NULL);
    ckstrncpy(namecopy, tcp_http_proxy, NAMECPYL);

    p = namecopy; /* Was a service requested? */
    while (*p != '\0' && *p != ':')
      p++;           /* Look for colon */
    if (*p == ':') { /* Have a colon */
      debug(F110, "netopen name has colon", namecopy, 0);
      *p++ = '\0'; /* Get service name or number */
    } else {
      strcpy(++p, "http");
    }

    service = ckgetservice(namecopy, p, namecopy, NAMECPYL);
    if (!service) {
      fprintf(stderr, "Can't find port for service %s\n", p);
      debug(F101, "netopen can't get service for proxy", "", errno);
      errno = 0; /* (rather than mislead) */
      return (-1);
    }
    ckstrncpy(p, ckuitoa(ntohs(service->s_port)), NAMECPYL - (p - namecopy));
  }
#endif /* NOHTTP */

  /* Set up socket structure and get host address */

  bzero((char *)&r_addr, sizeof(r_addr));
  debug(F100, "netopen bzero ok", "", 0);
/*
   NOTE: Originally the inet_addr() check was #ifdef NT, but is enabled for
   all as of 20 Sep 97, to allow people to "set host" to a specific numeric IP
   address without going through the multihomed host sequence and winding up
   at a different place than the one requested.
*/
#ifdef INADDR_NONE
  debug(F101, "netopen INADDR_NONE defined", "", INADDR_NONE);
#else  /* INADDR_NONE */
  debug(F100, "netopen INADDR_NONE not defined", "", 0);
#endif /* INADDR_NONE */
#ifdef INADDRX
  debug(F100, "netopen INADDRX defined", "", 0);
#else  /* INADDRX */
  debug(F100, "netopen INADDRX not defined", "", 0);
#endif /* INADDRX */

#ifndef NOMHHOST
#ifdef INADDRX
  iax = inet_addr(namecopy);
  debug(F111, "netopen inet_addr", namecopy, iax.s_addr);
#else /* INADDRX */
#ifdef INADDR_NONE
  iax.s_addr = inet_addr(namecopy);
  debug(F111, "netopen inet_addr", namecopy, iax.s_addr);
#else  /* INADDR_NONE */
  iax = (unsigned int)inet_addr(namecopy);
  debug(F111, "netopen inet_addr", namecopy, iax);
#endif /* INADDR_NONE */
#endif /* INADDRX */

  dns = 0;
  if (
/* This might give warnings on 64-bit platforms but they should be harmless */
/* because INADDR_NONE should be all 1's anyway, thus the OR part is */
/* probably superfluous -- not sure why it's even there, maybe it should be */
/* removed. */
#ifdef INADDR_NONE
      iax.s_addr == INADDR_NONE /* || iax.s_addr == (unsigned long) -1L */
#else  /* INADDR_NONE */
      iax < 0
#endif /* INADDR_NONE */
  ) {
    if (!quiet) {
      printf(" DNS Lookup... ");
      fflush(stdout);
    }
    if ((host = gethostbyname(namecopy)) != NULL) {
      debug(F110, "netopen gethostbyname != NULL", namecopy, 0);
      host = ck_copyhostent(host);
      dns = 1; /* Remember we performed dns lookup */
      r_addr.sin_family = host->h_addrtype;
      if (tcp_rdns && host->h_name && host->h_name[0]
#ifndef NOHTTP
          && (tcp_http_proxy == NULL)
#endif /* NOHTTP */
      ) {
        ckstrncpy(name, host->h_name, 80); /* Bad Bad Bad */
        if ((80 - strlen(name)) > (strlen(svcbuf) + 1)) {
          ckstrncat(name, ":", 80 - strlen(name));
          ckstrncat(name, svcbuf, 80 - strlen(name));
        }
      }
      debug(F110, "netopen name after lookup", name, 0);

#ifdef HADDRLIST
#ifdef h_addr
      /* This is for trying multiple IP addresses - see <netdb.h> */
      if (!(host->h_addr_list))
        return (-1);
      bcopy(host->h_addr_list[0], (caddr_t)&r_addr.sin_addr, host->h_length);
#else
      bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* h_addr */
#else  /* HADDRLIST */
      bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* HADDRLIST */

      debug(F111, "BCOPY", "host->h_length", host->h_length);
    }
  }
#endif /* NOMHHOST */

  debug(F101, "netopen dns", "", dns);

  if (!dns) {
#ifdef INADDRX
    /* inet_addr() is of type struct in_addr */
    struct in_addr ina;
    unsigned long uu;
    debug(F100, "netopen gethostbyname == NULL: INADDRX", "", 0);
    ina = inet_addr(namecopy);
    uu = *(unsigned int *)&ina;
#else  /* Not INADDRX */
    /* inet_addr() is unsigned long */
    unsigned long uu;
    debug(F100, "netopen gethostbyname == NULL: Not INADDRX", "", 0);
    uu = inet_addr(namecopy);
#endif /* INADDRX */
    debug(F101, "netopen uu", "", uu);
    if (
#ifdef INADDR_NONE
        !(uu == INADDR_NONE || uu == (unsigned int)-1L)
#else  /* INADDR_NONE */
        uu != ((unsigned long)-1)
#endif /* INADDR_NONE */
    ) {
      r_addr.sin_addr.s_addr = uu;
      r_addr.sin_family = AF_INET;
    } else {
      fprintf(stderr, "Can't get address for %s\n", namecopy);
      debug(F101, "netopen can't get address", "", errno);
      errno = 0; /* Rather than mislead */
      return (-1);
    }
  }

  /* Get a file descriptor for the connection. */

  r_addr.sin_port = service->s_port;
  ckstrncpy(ipaddr, (char *)inet_ntoa(r_addr.sin_addr), 20);
  debug(F110, "netopen trying", ipaddr, 0);
  if (!quiet && *ipaddr) {
    printf(" Trying %s... ", ipaddr);
    fflush(stdout);
  }

  /* Loop to try additional IP addresses, if any. */

  do {

    if ((ttyfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("TCP socket error");
      debug(F101, "netopen socket error", "", errno);
      return (-1);
    }
    errno = 0;

#ifdef RLOGCODE
    /* Not part of the RLOGIN RFC, but the BSD implementation     */
    /* requires that the client port be a priviliged port (<1024) */
    /* on a Unix system this would require SuperUser permissions  */
    /* thereby saying that the root of the Unix system has given  */
    /* permission for this connection to be created               */
    if (service->s_port == htons((unsigned short)RLOGIN_PORT)) {
      static unsigned short lport = 1024; /* max reserved port */

      lport--; /* Make sure we do not reuse a port */
      if (lport == 512)
        lport = 1023;

      sin.sin_family = AF_INET;
      if (tcp_address) {
#ifdef INADDRX
        inaddrx = inet_addr(tcp_address);
        sin.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
        sin.sin_addr.s_addr = inet_addr(tcp_address);
#endif /* INADDRX */
      } else
        sin.sin_addr.s_addr = INADDR_ANY;
      while (1) {
        sin.sin_port = htons(lport);
        if (bind(ttyfd, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
          break;
        if (errno != EADDRINUSE) {
          debug(F101, "rlogin bind errno", "", errno);
          perror("rlogin bind");
          debug(F101, "rlogin local port", "", lport);
          netclos();
          return -1;
        }
        lport--;
        if (lport == 512 /* lowest reserved port to use */) {
          printf("\nNo reserved ports available.\n");
          netclos();
          return -1;
        }
      }
      debug(F101, "rlogin lport", "", lport);
      ttnproto = NP_RLOGIN;
    } else
#endif /* RLOGCODE  */

      /* If a specific TCP address on the local host is desired we */
      /* must bind it to the socket.                               */
      if (tcp_address) {
        int s_errno;

        debug(F110, "netopen binding socket to", tcp_address, 0);
        bzero((char *)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
#ifdef INADDRX
        inaddrx = inet_addr(tcp_address);
        sin.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
      sin.sin_addr.s_addr = inet_addr(tcp_address);
#endif /* INADDRX */
        sin.sin_port = 0;
        if (bind(ttyfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
          s_errno = socket_errno; /* Save error code */
#ifdef TCPIPLIB
          socket_close(ttyfd);
#else  /* TCPIPLIB */
        close(ttyfd);
#endif /* TCPIPLIB */
          ttyfd = -1;
          wasclosed = 1;
          errno = s_errno; /* and report this error */
          debug(F101, "netopen bind errno", "", errno);
          return (-1);
        }
      }

    /* Now connect to the socket on the other end. */

    if (connect(ttyfd, (struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) {
      i = errno; /* Save error code */
#ifdef RLOGCODE
      if (errno == EADDRINUSE && ttnproto == NP_RLOGIN) {
#ifdef TCPIPLIB
        socket_close(ttyfd); /* Close it. */
#else
        close(ttyfd);
#endif /* TCPIPLIB */
        continue; /* Try a different lport */
      }
#endif /* RLOGCODE */
#ifdef HADDRLIST
#ifdef h_addr
      if (host && host->h_addr_list && host->h_addr_list[1]) {
        perror("");
        host->h_addr_list++;
        bcopy(host->h_addr_list[0], (caddr_t)&r_addr.sin_addr, host->h_length);

        ckstrncpy(ipaddr, (char *)inet_ntoa(r_addr.sin_addr), 20);
        debug(F110, "netopen h_addr_list", ipaddr, 0);
        if (!quiet && *ipaddr) {
          printf(" Trying %s... ", ipaddr);
          fflush(stdout);
        }
#ifdef TCPIPLIB
        socket_close(ttyfd); /* Close it. */
#else
        close(ttyfd);
#endif /* TCPIPLIB */
        continue;
      }
#endif /* h_addr */
#endif /* HADDRLIST */
      netclos();
      ttyfd = -1;
      wasclosed = 1;
      ttnproto = NP_NONE;
      errno = i; /* And report this error */
      debug(F101, "netopen connect errno", "", errno);
      if (!quiet)
        perror("Failed");
      return (-1);
    }
    isconnect = 1;
  } while (!isconnect);

#ifdef NON_BLOCK_IO
  on = 1;
  x = socket_ioctl(ttyfd, FIONBIO, &on);
  debug(F101, "netopen FIONBIO", "", x);
#endif /* NON_BLOCK_IO */

  ttnet = nett; /* TCP/IP (sockets) network */

#ifndef NOHTTP
  /* We have succeeded in connecting to the HTTP PROXY.  So now we   */
  /* need to attempt to connect through the proxy to the actual host */
  /* If that is successful, we have to pretend that we made a direct */
  /* connection to the actual host.                                  */

  if (tcp_http_proxy) {
    char *agent = "C-Kermit";

    if (http_connect(ttyfd, tcp_http_proxy_agent ? tcp_http_proxy_agent : agent,
                     NULL, tcp_http_proxy_user, tcp_http_proxy_pwd, 0,
                     proxycopy) < 0) {
      netclos();
      return (-1);
    }

    ckstrncpy(namecopy, proxycopy, NAMECPYL);
    p = namecopy; /* Was a service requested? */
    while (*p != '\0' && *p != ':')
      p++; /* Look for colon */
    *p = '\0';
  }
#endif /* NOHTTP */

  /* Jeff - Does this next block of code that set's the protocol */
  /* need to be here anymore?  5/10/2000                         */

  /* There are certain magic port numbers that when used require */
  /* the use of specific protocols.  Check this now before we    */
  /* set the SO_OOBINLINE state or we might get it wrong.        */
  x = ntohs((unsigned short)service->s_port);
  svcnum = x;
  /* See if the service is TELNET. */
  if (x == TELNET_PORT) {
    /* Yes, so if raw port not requested */
    /* fdc 2005/12/04 */
    if (ttnproto != NP_TCPRAW && ttnproto != NP_NONE)
      ttnproto = NP_TELNET; /* Select TELNET protocol. */
  }
#ifdef RLOGCODE
  else if (x == RLOGIN_PORT) {
    ttnproto = NP_RLOGIN;
  }
#endif /* RLOGCODE */
#ifdef IKS_OPTION
  else if (x == KERMIT_PORT) { /* IKS uses Telnet protocol */
    if (ttnproto == NP_NONE)
      ttnproto = NP_KERMIT;
  }
#endif /* IKS_OPTION */

#ifdef SO_OOBINLINE
/*
  The symbol SO_OOBINLINE is not known to Ultrix 2.0.
  It means "leave out of band data inline".  The normal value is 0x0100,
  but don't try this on systems where the symbol is undefined.
*/
/*
  Note from Jeff Altman: 12/13/95
  In implementing rlogin protocol I have come to the conclusion that it is
  a really bad idea to read out-of-band data inline.
  At least Windows and OS/2 does not handle this well.
  And if you need to know that data is out-of-band, then it becomes
  absolutely pointless.

  Therefore, at least on OS2 and Windows (NT) I have changed the value of
  on to 0, so that out-of-band data stays out-of-band.

  12/18/95
  Actually, OOB data should be read inline when possible.  Especially with
  protocols that don't care about the Urgent flag.  This is true with Telnet.
  With Rlogin, you need to be able to catch OOB data.  However, the best
  way to do this is to set a signal handler on SIGURG.  This isn't possible
  on OS/2 and Windows.  But it is in UNIX.  We will also need OOB data for
  FTP so better create a general mechanism.

  The reason for making OOB data be inline is that the standard ttinc/ttoc
  calls can be used for reading that data on UNIX systems.  If we didn't
  have the OOBINLINE option set then we would have to use recv(,MSG_OOB)
  to read it.
*/
#ifdef RLOGCODE
#ifdef TCPIPLIB
  if (ttnproto == NP_RLOGIN)
    on = 0;
#else  /* TCPIPLIB */
  if (ttnproto == NP_RLOGIN) {
    debug(F100, "Installing rlogoobh on SIGURG", "", 0);
    signal(SIGURG, rlogoobh);
    on = 0;
  } else {
    debug(F100, "Ignoring SIGURG", "", 0);
    signal(SIGURG, SIG_DFL);
  }
#endif /* TCPIPLIB */
#endif /* RLOGCODE */

#ifdef POSIX
  setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);
#else
#ifdef CLIX
  setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);
#else
  setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif /* CLIX */
#endif /* POSIX */
#endif /* SO_OOBINLINE */

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
  no_delay(ttyfd, tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
  keepalive(ttyfd, tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
  ck_linger(ttyfd, tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
  sendbuf(ttyfd, tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
  recvbuf(ttyfd, tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

  /* Find out our own IP address. */
  /* We need the l_addr structure for [E]KLOGIN. */
  l_slen = sizeof(l_addr);
  bzero((char *)&l_addr, l_slen);
  if (!getsockname(ttyfd, (struct sockaddr *)&l_addr, &l_slen)) {
    char *s = (char *)inet_ntoa(l_addr.sin_addr);
    ckstrncpy(myipaddr, s, 20);
    debug(F110, "getsockname", myipaddr, 0);
  }

  /*
    This is really only needed for Kerberos IV but is useful information in any
    case.  If we connect to a name that is really a pool, we need to get the
    name of the machine we are actually connecting to for K4 to authenticate
    properly.  This way we also update the names properly.

    However, it is a security hole when used with insecure DNS.

    Note: This does not work on Windows 95 or Windows NT 3.5x.  This is because
    of the Microsoft implementation of gethostbyaddr() in both Winsock 1.1
    and Winsock 2.0 on those platforms.  Their algorithm is:

    1. Check the HOSTENT cache.
    2. Check the HOSTS file at %SystemRoot%\System32\DRIVERS\ETC.
    3. Do a DNS query if the DNS server is configured for name resolution.
    4. Do an additional NetBIOS remote adapter status to an IP address for its
       NetBIOS name table. This step is specific only to the Windows NT version
       3.51 implementation.

    The problem is the use of the HOSTENT cache.  It means that gethostbyaddr()
    can not be used to resolve the real name of machine if it was originally
    accessed by an alias used to represent a cluster.
  */
  if ((tcp_rdns && dns || tcp_rdns == SET_ON)
#ifndef NOHTTP
      && (tcp_http_proxy == NULL)
#endif /* NOHTTP */
  ) {
    if (!quiet) {
      printf(" Reverse DNS Lookup... ");
      fflush(stdout);
    }
    if ((host = gethostbyaddr((char *)&r_addr.sin_addr, 4, PF_INET))) {
      char *s;
      host = ck_copyhostent(host);
      debug(F100, "netopen gethostbyname != NULL", "", 0);
      if (!quiet) {
        printf("(OK)\n");
        fflush(stdout);
      }
      s = host->h_name;
      if (!s) { /* This can happen... */
        debug(F100, "netopen host->h_name is NULL", "", 0);
        s = "";
      }
      /* Something is wrong with inet_ntoa() on HPUX 10.xx */
      /* The compiler says "Integral value implicitly converted to */
      /* pointer in assignment."  The prototype is right there */
      /* in <arpa/inet.h> so what's the problem? */
      /* Ditto in HP-UX 5.x, but not 8.x or 9.x... */
      if (!*s) { /* No name so substitute the address */
        debug(F100, "netopen host->h_name is empty", "", 0);
        s = inet_ntoa(r_addr.sin_addr); /* Convert address to string */
        if (!s)                         /* Trust No 1 */
          s = "";
        if (*s) { /* If it worked, use this string */
          ckstrncpy(ipaddr, s, 20);
        }
        s = ipaddr;     /* Otherwise stick with the IP */
        if (!*s)        /* or failing that */
          s = namecopy; /* the name we were called with. */
      }
      if (*s) {                 /* Copying into our argument? */
        ckstrncpy(name, s, 80); /* Bad Bad Bad */
        if ((80 - strlen(name)) > (strlen(svcbuf) + 1)) {
          ckstrncat(name, ":", 80 - strlen(name));
          ckstrncat(name, svcbuf, 80 - strlen(name));
        }
      }
      if (!quiet && *s
#ifndef NOICP
          && !doconx
#endif /* NOICP */
      ) {
        printf(" %s connected on port %s\n", s, p);
#ifdef BETADEBUG
        /* This is simply for testing the DNS entries */
        if (host->h_aliases) {
          char **a = host->h_aliases;
          while (*a) {
            printf(" alias => %s\n", *a);
            a++;
          }
        }
#endif /* BETADEBUG */
      }
    } else {
      if (!quiet)
        printf("Failed.\n");
    }
  } else if (!quiet)
    printf("(OK)\n");
  if (!quiet)
    fflush(stdout);

  /* This should already have been done but just in case */
  ckstrncpy(ipaddr, (char *)inet_ntoa(r_addr.sin_addr), 20);

#ifdef RLOGCODE
  if (ttnproto == NP_RLOGIN) { /* Similar deal for rlogin */
    if (rlog_ini(((tcp_rdns && host && host->h_name && host->h_name[0])
                      ? (CHAR *)host->h_name
                      : (CHAR *)ipaddr),
                 service->s_port, &l_addr, &r_addr) < 0) {
      debug(F100, "rlogin initialization failed", "", 0);
      netclos();
      return (-1);
    }
  } else
#endif /* RLOGCODE */
    if (tn_ini() < 0) { /* Start Telnet negotiations. */
      netclos();
      return (-1); /* Gone, so open failed.  */
    }
  if (ttchk() < 0) {
    netclos();
    return (-1);
  }

  debug(F101, "netopen service", "", svcnum);
  debug(F110, "netopen name", name, 0);
  debug(F110, "netopen ipaddr", ipaddr, 0);
  ckstrncpy(hostipaddr, ipaddr, 63);

  if (lcl)
    if (*lcl < 0) /* Local mode. */
      *lcl = 1;
#endif /* TCPSOCKET */
  return (0); /* Done. */
}

/*  N E T C L O S  --  Close current network connection.  */

#ifndef NOLOCAL
#endif /* NOLOCAL */

int netclos() {
  static int close_in_progress = 0;
  int x = 0;
  debug(F101, "netclos", "", ttyfd);

#ifdef NETLEBUF
  if (!tt_push_inited)
    le_init();
#endif /* NETLEBUF */

  if (ttyfd == -1) /* Was open? */
    return (0);    /* Wasn't. */

  if (close_in_progress)
    return (0);
  close_in_progress = 1; /* Remember */

#ifndef NOLOCAL
  /* This function call should not be here since this is a direct call */
  /* from an I/O routine to a user interface level function.  However, */
  /* the reality is that we do not have pure interfaces.  If we ever   */
  /* decide to clean this up the UI level should assign this function  */
  /* via a pointer assignment.  - Jeff 9/10/1999                       */
  slrestor();
#endif /* NOLOCAL */
  if (ttyfd > -1) /* Was. */
  {
#ifdef TNCODE
    if (ttnproto == NP_TELNET) {
      if (!TELOPT_ME(TELOPT_LOGOUT)) {
        /* Send LOGOUT option before close */
        if (tn_sopt(DO, TELOPT_LOGOUT) >= 0) {
          TELOPT_UNANSWERED_DO(TELOPT_LOGOUT) = 1;
          /* It would be nice to call tn_wait but we can't */
        }
      }
      tn_push(); /* Place any waiting data into input*/
    }
#endif /* TNCODE */
#ifdef TCPIPLIB
    x = socket_close(ttyfd); /* Close it. */
#else
#ifdef IBMX25
    if (ttnet == NET_IX25) {
      /* riehm: should send a disc_req - but only if link is still OK */
      x = x25clear();
      close(ttyfd);
      if (x25serverfd) {
        /* we were the passive client of a server, now we
         * go back to being the normal client.
         * I hope that kermit can cope with the logic that
         * there can still be a connection after netclos
         * has been called.
         */
        ttyfd = x25serverfd;
        x25serverfd = 0;
        /*
         * need to close the server connection too - because
         * all file descriptors connected to the NPI have the
         * same status.
         *
         * The problem is that any waiting connections get
         * lost, the client doesn't realise, and hangs.
         */
        netclos();
      }
      x25_state = X25_CLOSED; /* riehm: dead code? */
    } else
#endif /* IBMX25 */
      x = close(ttyfd);
#endif /* TCPIPLIB */
  }
  ttyfd = -1; /* Mark it as closed. */
  wasclosed = 1;
#ifdef TNCODE
  tn_reset(); /* The Reset Telnet Option table.  */
  debug(F100, "netclose setting tn_init = 0", "", 0);
  tn_init = 0;  /* Remember about telnet protocol... */
  sstelnet = 0; /* Client-side Telnet */
#endif /* TNCODE */
  *ipaddr = '\0';   /* Zero the IP address string */
  tcp_incoming = 0; /* No longer incoming */
  /* Don't reset ttnproto so that we can remember which protocol is in use */

#ifdef TCPIPLIB
  /*
    Empty the internal buffers so they won't be used as invalid input on
    the next connect attempt (rlogin).
  */
  ttibp = 0;
  ttibn = 0;
#endif /* TCPIPLIB */
  close_in_progress = 0; /* Remember we are done. */
  return (x);
}
#ifndef NOTCPIP

/*  N E T T C H K  --  Check if network up, and how many bytes can be read */
/*
  Returns number of bytes waiting, or -1 if connection has been dropped.
*/
int         /* Check how many bytes are ready */
nettchk() { /* for reading from network */
#ifdef TCPIPLIB
  long count = 0;
  int x = 0, z;
  unsigned long uy;
  char c;
  int rc;
  long y;

  debug(F101, "nettchk entry ttibn", "", ttibn);
  debug(F101, "nettchk entry ttibp", "", ttibp);

#ifdef NETLEBUF
  {
    int n = 0;
    if (ttpush >= 0)
      n++;
    n += le_inbuf();
    if (n > 0)
      return (n);
  }
#endif /* NETLEBUF */

  socket_errno = 0; /* This is a function call in NT, and BeOS */

  if (ttyfd == -1) {
    debug(F100, "nettchk socket is closed", "", 0);
    return (-1);
  }
  /*
    Note: this socket_ioctl() call does NOT return an error if the
    connection has been broken.  (At least not in MultiNet.)
  */

  if (socket_ioctl(ttyfd, FIONREAD, &count) < 0) {
    debug(F101, "nettchk socket_ioctl error", "", socket_errno);
    /* If the connection is gone, the connection is gone. */
    netclos();

    count += ttibn;
    return (count > 0 ? count : -1);
  } else {
  }
  debug(F101, "nettchk count", "", count);

  /* For the sake of efficiency, if there is still data in the ttibuf */
  /* do not go to the bother of checking to see of the connection is  */
  /* still valid.  The handle is still good, so just return the count */
  /* of the bytes that we already have left to process.               */
  if (count > 0 || ttibn > 0) {
    debug(F101, "nettchk returns", "", count + ttibn);
    return (count + ttibn);
  }
  ttibn = ttibp = 0;

/*
  The following code works well in most settings, but messes things up in
  others, including CMU/Tek TCP/IP and UCX 2.0, where it somehow manages to
  make it impossible to ever make a new connection to the same host again with
  CONNECT, once it has been logged out from the first time.  Not even if you
  HANGUP first, or SET HOST<CR>, or SET LINE<CR>.  Reportedly, however, it
  does work OK in later releases of UCX.  But there is no way we can
  accommodate both old and new -- we might have static linking or dynamic
  linking, etc etc.  If we have static, I only have access to 2.0, where this
  doesn't work, etc etc blah blah.

  In the following lines, we define a symbol NOCOUNT for builds where we want
  to omit this code.  By default, it is omitted for CMU/Tek.  You can force
  omission of it for other combinations by defining NOCOUNT in CFLAGS.  You
  can force inclusion of this code, even for CMU/Tek, by including NONOCOUNT
  in CFLAGS.
*/
#ifdef NONOCOUNT
#ifdef NOCOUNT
#undef NOCOUNT
#endif /* NOCOUNT */
#else
#ifndef NOCOUNT
#endif /* NOCOUNT */
#endif /* NONOCOUNT */

  /* From this point forward we have a possible race condition in K95
   * due to its use of multiple threads.  Therefore, we must ensure
   * that only one thread attempt to read/write from the socket at a
   * time.  Otherwise, it is possible for a buffer to be overwritten.
   */
  /* we know now that count >= 0 and that ttibn == 0 */

  if (count == 0
#ifdef RLOGCODE
#endif /* RLOGCODE */
  ) {
    int s_errno = 0;
#ifndef NOCOUNT
/*
  Here we need to tell the difference between a 0 count on an active
  connection, and a 0 count because the remote end of the socket broke the
  connection.  There is no mechanism in TGV MultiNet (or WIN/TCP?) to query
  the status of the connection, so we have to do a read.  -1 means there was
  no data available (socket_errno == EWOULDBLOCK), 0 means the connection is
  down.  But if, by chance, we actually get a character, we have to put it
  where it won't be lost.
*/
#ifndef NON_BLOCK_IO
    y = 1; /* Turn on nonblocking reads */
    z = socket_ioctl(ttyfd, FIONBIO, &y);
    debug(F111, "nettchk FIONBIO", "on", z);
#endif /* NON_BLOCK_IO */
    {
      x = socket_read(ttyfd, &c, 1); /* Returns -1 if no data */
    }
    s_errno = socket_errno; /* socket_errno may be a function */
    debug(F101, "nettchk socket_read", "", x);

#ifndef NON_BLOCK_IO
    y = 0; /* Turn off nonblocking reads */
    z = socket_ioctl(ttyfd, FIONBIO, &y);
    debug(F111, "nettchk FIONBIO", "off", z);
#endif /* NON_BLOCK_IO */

    if (x == -1) {
      debug(F101, "nettchk socket_read errno", "", s_errno);
    } else if (x == 0) {
      debug(F100, "nettchk connection closed", "", 0);
      netclos(); /* *** *** */
      rc = -1;
      goto nettchk_return;
    }
    if (x >= 1) { /* Oops, actually got a byte? */
      {
        debug(F101, "nettchk socket_read char", "", c);
        debug(F101, "nettchk ttibp", "", ttibp);
        debug(F101, "nettchk ttibn", "", ttibn);
        /*
          In the case of Overlapped I/O the character would have come from
          the beginning of the buffer, so put it back.
        */
        if (ttibp > 0) {
          ttibp--;
          ttibuf[ttibp] = c;
          ttibn++;
        } else {
          ttibuf[ttibp + ttibn] = c;
          ttibn++;
        }
      }
    }
#else  /* NOCOUNT */
    if (ttnet == NET_TCPB) {
      char dummy;
      x = read(ttyfd, &dummy, 0); /* Try to read nothing */
      if (x < 0) {                /* "Connection reset by peer" */
        perror("TCP/IP");         /* or somesuch... */
        ttclos(0);                /* Close our end too. */
        rc = -1;
        goto nettchk_return;
      }
    }
#endif /* NOCOUNT */
  }

  debug(F101, "nettchk returns", "", count + ttibn);
  rc = count + ttibn;

nettchk_return:
  return (rc);

#else  /* Not TCPIPLIB */
  /*
    UNIX just uses ttchk(), in which the ioctl() calls on the file descriptor
    seem to work OK.
  */
  return (ttchk());
#endif /* TCPIPLIB */
  /*
    But what about X.25?
  */
}

void nettout(int i) /* Catch the alarm interrupts */
{
  debug(F100, "nettout caught timeout", "", 0);
  ttimoff();
  cklongjmp(njbuf, -1);
}

#ifdef TCPIPLIB

void donetinc(void *threadinfo)
/* donetinc */ {
#ifdef CK_LOGIN
#endif /* CK_LOGIN */
  while (1) {
    if (ttbufr() < 0) /* Keep trying to refill it. */
      break;          /* Till we get an error. */
    if (ttibn > 0)    /* Or we get a character. */
      break;
  }
}
#endif /* TCPIPLIB */

void failnetinc(void *threadinfo)
/* failnetinc */ {
  ; /* Nothing to do on an error */
}

/* N E T X I N -- Input block of characters from network */

int netxin(int n, CHAR *buf) {
  int len;
#ifdef TCPIPLIB
  int rc;
#else
  int i, j;
#endif /* TCPIPLIB */

  if (ttyfd == -1) {
    debug(F100, "netxin socket is closed", "", 0);
    return (-2);
  }

#ifdef TCPIPLIB
  if (ttibn == 0)
    if ((rc = ttbufr()) <= 0) {
      return (rc);
    }

  if (ttibn <= n) {
    len = ttibn;
    memcpy(buf, &ttibuf[ttibp], len); /* safe */
    ttibp += len;
    ttibn = 0;
  } else {
    memcpy(buf, &ttibuf[ttibp], n); /* safe */
    ttibp += n;
    ttibn -= n;
    len = n;
  }
#else  /* TCPIPLIB */
  for (i = 0; i < n; i++) {
    if ((j = netinc(0)) < 0) {
      if (j < -1)
        return (j);
      else
        break;
    }
    buf[i] = j;
  }
  len = i;
#endif /* TCPIPLIB */

  return (len);
}

/*  N E T I N C --  Input character from network */

#ifdef NETLEBUF
#define LEBUF
#endif /* NETLEBUF */
#ifdef TTLEBUF
#define LEBUF
#endif /* TTLEBUF */
#ifndef LEBUF
#endif /* LEBUF */

int netinc(int timo) {
#ifdef TCPIPLIB
  int x;
  unsigned char c; /* The locals. */

#ifdef NETLEBUF
  if (ttpush >= 0) {
    debug(F111, "netinc", "ttpush", ttpush);
    c = ttpush;
    ttpush = -1;
    return (c);
  }
  if (le_data) {
    if (le_getchar((CHAR *)&c) > 0) {
      debug(F111, "netinc le_getchar", "c", c);
      return (c);
    }
  }
#endif /* NETLEBUF */

  if (ttyfd == -1) {
    debug(F100, "netinc socket is closed", "", 0);
    return (-2);
  }

  if (ttibn > 0) { /* Something in internal buffer? */
    x = 0;         /* Success. */
  } else {         /* Else must read from network. */
    x = -1;        /* Assume failure. */
#ifdef DEBUG
    debug(F101, "netinc goes to net, timo", "", timo);
#endif /* DEBUG */
#ifndef LEBUF
    if (timo == 0) {      /* Untimed case. */
      while (1) {         /* Wait forever if necessary. */
        if (ttbufr() < 0) /* Refill buffer. */
          break;          /* Error, fail. */
        if (ttibn > 0) {  /* Success. */
          x = 0;
          break;
        }
      }
    } else /* Timed case... */
#endif /* LEBUF */
    {
#ifdef BSDSELECT
      fd_set rfds;
      struct timeval tv;
      int timeout = timo < 0 ? -timo : 1000 * timo;
      debug(F101, "netinc BSDSELECT", "", timo);

      for (; timeout >= 0; timeout -= (timo ? 100 : 0)) {
        int rc;
        debug(F111, "netinc", "timeout", timeout);
        /* Don't move select() initialization out of the loop. */
        FD_ZERO(&rfds);
        FD_SET(ttyfd, &rfds);
        tv.tv_sec = tv.tv_usec = 0L;
        if (timo)
          tv.tv_usec = (long)100000L;
        else
          tv.tv_sec = 30;
        rc = select(FD_SETSIZE,
#ifdef __DECC
#ifdef INTSELECT
                    (int *)
#else  /* def INTSELECT */
                    (fd_set *)
#endif /* def INTSELECT [else] */
#else  /* def __DECC */
                    (fd_set *)
#endif /* def __DECC [else] */
                    &rfds,
                    NULL, NULL, &tv);
        if (rc < 0) {
          int s_errno = socket_errno;
          debug(F111, "netinc", "select", rc);
          debug(F111, "netinc", "socket_errno", s_errno);
          if (s_errno) {
            return (-1);
          }
        }
        debug(F111, "netinc", "select", rc);

        /*
         * For some yet to be determined reason, on NT 3.1 the select()
         * call is claiming the socket isn't ready for reading -
         *   !FD_ISSET(ttyfd, &rfds) == true && rc == 0
         * this occurs even when ttchk() returns some number of bytes
         * ready for reading which should mean the socket is ready for
         * reading. And indeed if we go ahead and try to read when
         * ttchk() > 0 telnet suddenly works!
         *
         * So maybe there is some bug in the NT 3.1 Winsock
         * implementation? Or is K95 doing something that NT 3.1 doesn't
         * like?
         *
         * problem is, the API ttchk() relies on (FIONREAD) is slow and
         * potentially unreliable so its not really an acceptable
         * replacement for select() unless we're ok with really bad
         * performance
         *
         * Note: ttchk eventually arrives at  nettchk further up in this file.
         * Line 5473 is about where the FIONREAD is.
         *
         */
        if (!FD_ISSET(ttyfd, &rfds)) {
#ifdef LEBUF
          if (le_inbuf() > 0) {
            timeout = -1;
            break;
          }
#endif /* LEBUF */
          /* If waiting forever we have no way of knowing if the */
          /* socket closed so try writing a 0-length TCP packet  */
          /* which should force an error if the socket is closed */
          if (!timo) {
            if ((rc = socket_write(ttyfd, "", 0)) < 0) {
              int s_errno = socket_errno;
              debug(F101, "netinc socket_write error", "", s_errno);
              return (-1); /* Call it an i/o error */
            }
          }
          continue;
        }
        while (1) {
          if (ttbufr() < 0) { /* Keep trying to refill it. */
            timeout = -1;
            break; /* Till we get an error. */
          }
          if (ttibn > 0) { /* Or we get a character. */
            x = 0;
            timeout = -1;
            break;
          }
        }
      }
#else  /* !BSDSELECT */
#ifdef IBMSELECT
      /*
        Was used by OS/2, currently not used, but might come in handy some
        day...
        ... and it came in handy!  For our TCP/IP layer, it avoids all the
        fd_set and timeval stuff since this is the only place where it is used.
      */
      int socket = ttyfd;
      int timeout = timo < 0 ? -timo : 1000 * timo;

      debug(F101, "netinc IBMSELECT", "", timo);
      for (; timeout >= 0; timeout -= (timo ? 100 : 0)) {
        if (select(&socket, 1, 0, 0, 100L) == 1) {
          while (1) {
            if (ttbufr() < 0) { /* Keep trying to refill it. */
              timeout = -1;
              break; /* Till we get an error. */
            }
            if (ttibn > 0) { /* Or we get a character. */
              x = 0;
              timeout = -1;
              break;
            }
          }
        }
#ifdef LEBUF
        else if (le_inbuf() > 0) {
          timeout = -1;
          break;
        }
#endif /* LEBUF */
      }
#else  /* !IBMSELECT */
#ifdef WINSOCK
      /* Actually, under WinSock we have a better mechanism than select() */
      /* for setting timeouts (SO_RCVTIMEO, SO_SNDTIMEO) */
      SOCKET socket = ttyfd;
      debug(F101, "netinc NTSELECT", "", timo);
      if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timo,
                     sizeof(timo)) == NO_ERROR)
        while (1) {
          if (ttbufr() < 0) /* Keep trying to refill it. */
            break;          /* Till we get an error. */
          if (ttibn > 0) {  /* Or we get a character. */
            x = 0;
            break;
          }
        }
#else  /* WINSOCK */
      /*
        If we can't use select(), then we use the regular alarm()/signal()
        timeout mechanism.
      */
      debug(F101, "netinc alarm", "", timo);
      x = alrm_execute(ckjaddr(njbuf), timo, nettout, donetinc, failnetinc);
      ttimoff(); /* Timer off. */
#endif /* WINSOCK */
#endif /* IBMSELECT */
#endif /* BSDSELECT */
    }
  }

#ifdef LEBUF
  if (le_inbuf() > 0) {               /* If data was inserted into the */
    if (le_getchar((CHAR *)&c) > 0) { /* Local Echo buffer while the   */
      return (c);
    }
  }
#endif /* LEBUF */
  if (x < 0) { /* Return -1 if we failed. */
    debug(F100, "netinc timed out", "", 0);
    return (-1);
  } else {             /* Otherwise */
    c = ttibuf[ttibp]; /* Return the first char in ttibuf[] */
    if (deblog) {
      debug(F101, "netinc returning", "", c);
      if (c == 0) {
        debug(F101, "netinc 0 ttibn", "", ttibn);
        debug(F101, "netinc 0 ttibp", "", ttibp);
#ifdef BETADEBUG
        {
          ckhexdump("netinc &ttbuf[ttibp]", &ttibuf[ttibp], ttibn);
        }
#endif /* BETADEBUG */
      }
    }
    ttibp++;
    ttibn--;
    return (c);
  }
#else  /* Not using TCPIPLIB */
  return (-1);
#endif /* TCPIPLIB */
}

/*  N E T T O L  --  Output a string of bytes to the network  */
/*
  Call with s = pointer to string, n = length.
  Returns number of bytes actually written on success, or
  -1 on i/o error, -2 if called improperly.
*/

int nettol(CHAR *s, int n) {
#ifdef TCPIPLIB
  int count = 0;
  int len = n;
  int try = 0;

  if (ttyfd == -1) {
    debug(F100, "nettol socket is closed", "", 0);
    return -1;
  }
  debug(F101, "nettol TCPIPLIB ttnet", "", ttnet);

nettol_retry:
  try++; /* Increase the try counter */

  if (ttnet == NET_TCPB) {
#ifdef BSDSELECT
    fd_set wfds;
    struct timeval tv;

    debug(F101, "nettol BSDSELECT", "", 0);
    tv.tv_usec = 0L;
    tv.tv_sec = 30;
#ifdef STREAMING
  do_select:
#endif /* STREAMING */
    FD_ZERO(&wfds);
    FD_SET(ttyfd, &wfds);
    if (select(FD_SETSIZE, NULL,
#ifdef __DECC
#ifndef __DECC_VER
               (int *)
#endif /* __DECC_VER */
#endif /* __DECC */
               &wfds,
               NULL, &tv) < 0) {
      int s_errno = socket_errno;
      debug(F101, "nettol select failed", "", s_errno);
#ifdef BETADEBUG
      printf("nettol select failed: %d\n", s_errno);
#endif /* BETADEBUG */
      return (-1);
    }
    if (!FD_ISSET(ttyfd, &wfds)) {
#ifdef STREAMING
      if (streaming)
        goto do_select;
#endif /* STREAMING */
      debug(F111, "nettol", "!FD_ISSET", ttyfd);
      return (-1);
    }
#else  /* BSDSELECT */
#ifdef IBMSELECT
    {
      int tries = 0;
      debug(F101, "nettol IBMSELECT", "", 0);
      while (select(&ttyfd, 0, 1, 0, 1000) != 1) {
        int count;
        if (tries++ >= 60) {
          /* if after 60 seconds we can't get permission to write */
          debug(F101, "nettol select failed", "", socket_errno);
          return (-1);
        }
        if ((count = nettchk()) < 0) {
          debug(F111, "nettol", "nettchk()", count);
          return (count);
        }
      }
    }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
    if ((count = socket_write(ttyfd, s, n)) < 0) {
      int s_errno = socket_errno; /* maybe a function */
      debug(F101, "nettol socket_write error", "", s_errno);
      return (-1); /* Call it an i/o error */
    }
    if (count < n) {
      debug(F111, "nettol socket_write", s, count);
      if (try > 25) {
        /* don't try more than 25 times */
        debug(F100, "nettol tried more than 25 times", "", 0);
        return (-1);
      }
      if (count > 0) {
        s += count;
        n -= count;
      }
      debug(F111, "nettol retry", s, n);
      goto nettol_retry;
    } else {
      debug(F111, "nettol socket_write", s, count);
      return (len); /* success - return total length */
    }
  } else
    return (-2);
#else
  debug(F100, "nettol TCPIPLIB not defined", "", 0);
  return (-2);
#endif /* TCPIPLIB */
}

/*  N E T T O C  --   Output character to network */
/*
  Call with character to be transmitted.
  Returns 0 if transmission was successful, or
  -1 upon i/o error, or -2 if called improperly.
*/
int nettoc(CHAR c)
/* nettoc */ {
#ifdef UNIX
  return (ttoc(c));
#else
#ifdef TCPIPLIB
  unsigned char cc;
  if (ttyfd == -1) {
    debug(F100, "nettoc socket is closed", "", 0);
    return -1;
  }
  cc = c;
  debug(F101, "nettoc cc", "", cc);

  if (ttnet == NET_TCPB) {
#ifdef BSDSELECT
    fd_set wfds;
    struct timeval tv;

    debug(F101, "nettoc BSDSELECT", "", 0);
    tv.tv_usec = 0L;
    tv.tv_sec = 30;

#ifdef STREAMING
  do_select:
#endif /* STREAMING */

    FD_ZERO(&wfds);
    FD_SET(ttyfd, &wfds);
    if (select(FD_SETSIZE, NULL,
#ifdef __DECC
#ifndef __DECC_VER
               (int *)
#endif /* __DECC_VER */
#endif /* __DECC */
               &wfds,
               NULL, &tv) < 0) {
      int s_errno = socket_errno;
      debug(F101, "nettoc select failed", "", s_errno);
#ifdef BETADEBUG
      printf("nettoc select failed: %d\n", s_errno);
#endif /* BETADEBUG */
      return (-1);
    }
    if (!FD_ISSET(ttyfd, &wfds)) {
#ifdef STREAMING
      if (streaming)
        goto do_select;
#endif /* STREAMING */
      debug(F111, "nettoc", "!FD_ISSET", ttyfd);
      return (-1);
    }
#else  /* BSDSELECT */
#ifdef IBMSELECT
    {
      int tries = 0;
      while (select(&ttyfd, 0, 1, 0, 1000) != 1) {
        int count;
        if (tries++ >= 60) {
          /* if after 60 seconds we can't get permission to write */
          debug(F101, "nettoc select failed", "", socket_errno);
          return (-1);
        }
        if ((count = nettchk()) < 0) {
          debug(F111, "nettoc", "nettchk()", count);
          return (count);
        }
      }
    }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
    if (socket_write(ttyfd, &cc, 1) < 1) {
      int s_errno = socket_errno; /* maybe a function */
      debug(F101, "nettoc socket_write error", "", s_errno);
      return (-1);
    }
    debug(F101, "nettoc socket_write", "", cc);
    return (0);
  } else
    return (-2);
#else
  return (-2);
#endif /* TCPIPLIB */
#endif /* UNIX */
}

/*  N E T F L U I  --  Flush network input buffer  */

#ifdef TNCODE
static int netgetct(int timo) /* Input function to point to... */
{                             /* ...in the tn_doop() call */
#ifdef TCPIPLIB
  return netinc(timo);
#else  /* TCPIPLIB */
  return ttinc(timo);
#endif /* TCPIPLIB */
}
#endif /* TNCODE */

int netflui() {
  int n;
  int ch;
#ifdef NETLEBUF
  ttpush = -1; /* Clear the peek-ahead char */
  while (le_data && (le_inbuf() > 0)) {
    CHAR ch = '\0';
    if (le_getchar(&ch) > 0) {
      debug(F101, "ttflui le_inbuf ch", "", ch);
    }
  }
#endif /* NETLEBUF */

#ifdef TCPIPLIB
#ifdef TNCODE
  if (ttnproto == NP_TELNET) {
    /* Netflui must process Telnet negotiations or get out of sync */
    if ((n = nettchk()) <= 0)
      goto exit_flui;
    while (n-- > 0) {
      ch = netinc(1);
      if (ch == IAC) {
        extern int duplex; /* this really shouldn't be here but ... */
        int tx = tn_doop((CHAR)(ch & 0xff), duplex, netgetct);
        if (tx == 1)
          duplex = 1;
        else if (tx == 2)
          duplex = 0;
        n = nettchk();
      }
    }
  } else
#endif /* TNCODE */
  {
    ttibuf[ttibp + ttibn] = '\0';
    debug(F111, "netflui 1", ttibuf, ttibn);
    ttibn = ttibp = 0; /* Flush internal buffer *FIRST* */
    if (ttyfd < 1)
      goto exit_flui;
    if ((n = nettchk()) > 0) { /* Now see what's waiting on the net */
      if (n > TTIBUFL)
        n = TTIBUFL;                     /* and sponge it up */
      debug(F101, "netflui 2", "", n);   /* ... */
      n = socket_read(ttyfd, ttibuf, n); /* into our buffer */
      if (n >= 0)
        ttibuf[n] = '\0';
      debug(F111, "netflui 3", ttibuf, n);
      ttibuf[0] = '\0';
    }
  }
#else  /* !TCPIPLIB */
  if (ttyfd < 1)
    goto exit_flui;
#ifdef TNCODE
  if (ttnproto == NP_TELNET) {
    if ((n = ttchk()) <= 0)
      goto exit_flui;
    while (n-- >= 0) {
      /* Netflui must process Telnet negotiations or get out of sync */
      ch = ttinc(1);
      if (ch == IAC) {
        extern int duplex; /* this really shouldn't be here but ... */
        int tx = tn_doop((CHAR)(ch & 0xff), duplex, netgetct);
        if (tx == 1)
          duplex = 1;
        else if (tx == 2)
          duplex = 0;
        n = ttchk();
      }
    };
  } else
#endif /* TNCODE */
    if ((n = ttchk()) > 0) {
      debug(F101, "netflui non-TCPIPLIB", "", n);
      while ((n--) && ttinc(1) > -1) /* Don't worry, ttinc() is buffered */
        ;                            /* and it handles the decryption... */
    }
#endif /* TCPIPLIB */
exit_flui:
  return (0);
}

/* getlocalipaddr() attempts to resolve an IP Address for the local machine.
 *   If the host is multi-homed it returns only one address.
 *
 * Two techniques are used.
 * (1) get the local host name and perform a DNS lookup, then take
 *     the first entry;
 * (2) open a UDP socket, use it to connect to a fictitious host (it's OK,
 *    no data is sent), then retrieve the local address from the socket.
 * Note: the second technique won't work on Microsoft systems.  See
 * Article ID: Q129065 PRB: Getsockname() Returns IP Address 0.0.0.0 for UDP
 */

/* Technique number one cannot work reliably if the machine is a laptop
 * and the hostname is associated with a physical adapter which is not
 * installed and a PPP connection is being used instead.  This is because
 * the hostname DNS lookup will succeed for the physical adapter even though
 * it would be impossible to use it.  In NT4 SP4, the gethostbyname()
 * when given the result of gethostname() returns not the real DNS entries
 * for that name+domain.  Instead it returns all of the static and dynamic
 * IP addresses assigned to any physical or virtual adapter defined in the
 * system regardless of whether or not it is installed.  The order of the
 * addresses is fixed according to the binding order in the NT registry.
 */

/*
 * It appears that calling gethostbyname(NULL) is more reliable than
 * calling gethostbyname(gethostname()) on Windows.  So on Windows we will
 * only call gethostbyname(NULL).
 */

int getlocalipaddr(void) {
  struct sockaddr_in l_sa;
  struct sockaddr_in r_sa;
  GSOCKNAME_T slen = sizeof(struct sockaddr_in);
  int sock;
  int rc;

  /* if still not resolved, then try second strategy */
  /* This second strategy does not work on Windows */

  debug(F100, "getlocalipaddr", "", 0);
  memset(&l_sa, 0, slen);
  memset(&r_sa, 0, slen);

  /* get a UDP socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock != -1) {
    /* connect to arbirary port and address (NOT loopback) */
    r_sa.sin_family = AF_INET;
    r_sa.sin_port = htons(IPPORT_ECHO);

    /* The following is an "illegal conversion" in AOS/VS */
    /* (and who knows where else) */

#ifdef INADDRX
    inaddrx = inet_addr("128.127.50.1");
    r_sa.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
    r_sa.sin_addr.s_addr = inet_addr("128.127.50.1");
#endif /* INADDRX */
    rc = connect(sock, (struct sockaddr *)&r_sa, sizeof(struct sockaddr));
    if (!rc) { /* get local address */
      getsockname(sock, (struct sockaddr *)&l_sa, &slen);
#ifdef TCPIPLIB
      socket_close(sock); /* We're done with the socket */
#else
      close(sock);
#endif /* TCPIPLIB */
      if (l_sa.sin_addr.s_addr != INADDR_ANY) {
        myxipaddr = ntohl(l_sa.sin_addr.s_addr);
        ckstrncpy(myipaddr, (char *)inet_ntoa(l_sa.sin_addr), 20);
        debug(F110, "getlocalipaddr setting buf to", myipaddr, 0);
        return (0);
      }
    }
  }
  return getlocalipaddrs(myipaddr, sizeof(myipaddr), 0);
}

int getlocalipaddrs(char *buf, int bufsz, int index) {
  char localhost[256];
  struct hostent *host = NULL;
  struct sockaddr_in l_sa;
  struct sockaddr_in r_sa;
  GSOCKNAME_T slen = sizeof(struct sockaddr_in);
  int rc;

  debug(F100, "getlocalipaddrs", "", 0);
  memset(&l_sa, 0, slen);
  memset(&r_sa, 0, slen);

  /* init local address (to zero) */
  l_sa.sin_addr.s_addr = INADDR_ANY;

#ifdef CKGHNLHOST
  rc = gethostname(localhost, 256);
  debug(F110, "getlocalipaddrs localhost", localhost, 0);
#else
  /* This doesn't work on some platforms, e.g. Solaris */
  rc = 0;
  localhost[0] = '\0';
#endif /* CKGHNLHOST */
  if (!rc) {
    /* resolve host name for local address */
    debug(F110, "getlocalipaddrs", "calling gethostbyname()", 0);
    host = gethostbyname(localhost);
    /* debug(F111,"getlocalipaddrs","gethostbyname() returned",host); */
    if (host) {
#ifdef HADDRLIST
      host = ck_copyhostent(host);
      if (index < 0 || index > 63 || !host->h_addr_list[index]) {
        buf[0] = '\0';
        return (-1);
      }
      l_sa.sin_addr.s_addr = *((unsigned long *)(host->h_addr_list[index]));
      ckstrncpy(buf, (char *)inet_ntoa(l_sa.sin_addr), 20);
      debug(F110, "getlocalipaddrs setting buf to", buf, 0);

#else  /* HADDRLIST */
      if (index != 0) {
        buf[0] = '\0';
        return (-1);
      }
      l_sa.sin_addr.s_addr = *((unsigned long *)(host->h_addr));
      ckstrncpy(buf, (char *)inet_ntoa(l_sa.sin_addr), bufsz);
      debug(F110, "getlocalipaddrs setting buf to", buf, 0);
#endif /* HADDRLIST */
      return (0);
    } else
      debug(F110, "getlocalipaddrs: gethostbyname() failed", localhost, 0);
  }
  return (-1);
}

#ifdef RLOGCODE /* TCP/IP RLOGIN protocol support code */
#ifdef CK_NAWS
int rlog_naws(void) {
  struct rlog_naws {
    unsigned char id[4];
    unsigned short rows, cols, ypix, xpix;
  } nawsbuf;

  if (ttnet != NET_TCPB)
    return 0;
  if (ttnproto != NP_RLOGIN)
    return 0;
  if (!TELOPT_ME(TELOPT_NAWS))
    return 0;

  debug(F100, "rlogin Window Size sent", "", 0);

  nawsbuf.id[0] = nawsbuf.id[1] = 0377;
  nawsbuf.id[2] = nawsbuf.id[3] = 's';
  nawsbuf.rows = htons((unsigned short)tt_rows);
  nawsbuf.cols = htons((unsigned short)tt_cols);
  nawsbuf.ypix = htons(0); /* y pixels */

  nawsbuf.xpix = htons(0); /* x pixels */
  if (ttol((CHAR *)(&nawsbuf), sizeof(nawsbuf)) < 0)
    return (-1);
  return (0);
}
#endif /* CK_NAWS */
#endif /* NOTCPIP */

#ifndef NORLOGIN
#define RLOGOUTBUF
static int rlog_ini(CHAR *hostname, int port, struct sockaddr_in *l_addr,
                    struct sockaddr_in *r_addr)
/* rlog_ini */ {

#ifdef RLOGOUTBUF
  char outbuf[512];
  int outbytes = 0;
#endif /* RLOGOUTBUF */
  int flag = 0;
#define TERMLEN 16
#define CONSPDLEN 16
  CHAR localuser[UIDBUFLEN + 1];
  CHAR remoteuser[UIDBUFLEN + 1];
  int userlen = 0;
  CHAR term_speed[TERMLEN + CONSPDLEN + 1];
#ifdef CONGSPD
  long conspd = -1L;
#endif /* CONGSPD */
  int i, n;

  int rc = 0;
  tn_reset(); /* This call will reset all of the Telnet */
              /* options and then quit.  We need to do  */
              /* this since we use the Telnet options   */
              /* to hold various state information      */
  duplex = 0; /* Rlogin is always remote echo */
  rlog_inband = 0;

#ifdef CK_TTGWSIZ
  /*
    But compute the values anyway before the first read since the out-
    of-band NAWS request would arrive before the first data byte (NULL).
  */
  debug(F101, "rlog_ini tt_rows 1", "", tt_rows);
  debug(F101, "rlog_ini tt_cols 1", "", tt_cols);
  if (tt_rows < 0 || tt_cols < 0) { /* Not known yet */
    ttgwsiz();                      /* Try to find out */
  }
  debug(F101, "rlog_ini tt_rows 2", "", tt_rows);
  debug(F101, "rlog_ini tt_cols 2", "", tt_cols);
#endif /* CK_TTGWSIZ */

  ttflui(); /* Start by flushing the buffers */

  rlog_mode = RL_COOKED;

  /* Determine the user's local username ... */

  localuser[0] = '\0';
  {
    char *user = getenv("USER");
    if (!user)
      user = "";
    userlen = strlen(user);
    debug(F111, "rlogin getenv(USER)", user, userlen);
    ckstrncpy((char *)localuser, user, UIDBUFLEN);
    debug(F110, "rlog_ini localuser 1", localuser, 0);
  }
  if (!localuser[0])
    strcpy((char *)localuser, "unknown");
  else if (ck_lcname) {
    cklower((char *)localuser);
    debug(F110, "rlog_ini localuser 2", localuser, 0);
  }

  /* And the username to login with */
  if (uidbuf[0]) {
    ckstrncpy((char *)remoteuser, uidbuf, UIDBUFLEN);
    debug(F110, "rlog_ini remoteuser 1", remoteuser, 0);
  } else if (localuser[0]) {
    ckstrncpy((char *)remoteuser, (char *)localuser, UIDBUFLEN);
    debug(F110, "rlog_ini remoteuser 2", remoteuser, 0);
  } else {
    remoteuser[0] = '\0';
    debug(F110, "rlog_ini remoteuser 3", remoteuser, 0);
  }
  if (ck_lcname)
    cklower((char *)remoteuser);
  debug(F110, "rlog_ini remoteuser 4", remoteuser, 0);

  /* The command to issue is the terminal type and speed */
  term_speed[0] = '\0';
  if (tn_term) {    /* SET TELNET TERMINAL-TYPE value */
    if (*tn_term) { /* (if any) takes precedence. */
      ckstrncpy((char *)term_speed, tn_term, TERMLEN);
      flag = 1;
    }
  } else { /* Otherwise the local terminal type */
    /* In the others, we just look at the TERM environment variable */
    {
      char *p = getenv("TERM");
      if (p)
        ckstrncpy((char *)term_speed, p, TERMLEN);
      else
        term_speed[0] = '\0';
    }
  }
  n = strlen((char *)term_speed);
  if (n > 0) {                /* We have a terminal type */
    if (!flag) {              /* If not user-specified */
      for (i = 0; i < n; i++) /* then lowercase it.    */
        if (isupper(term_speed[i]))
          term_speed[i] = tolower(term_speed[i]);
    }
    debug(F110, "rlog_ini term_speed 1", term_speed, 0);

#ifdef CONGSPD
    /* conspd() is not yet defined in all ck*tio.c modules */
    conspd = congspd();
    if (conspd > 0L) {
      ckstrncat((char *)term_speed, "/", sizeof(term_speed));
      ckstrncat((char *)term_speed, ckltoa(conspd), sizeof(term_speed));
    } else
#endif /* CONGSPD */
      ckstrncat((char *)term_speed, "/19200", sizeof(term_speed));
    debug(F110, "rlog_ini term_speed 2", term_speed, 0);
  } else {
    term_speed[0] = '\0';
    debug(F110, "rlog_ini term_speed 3", term_speed, 0);
  }

  if (ttnproto == NP_RLOGIN) {
#ifdef RLOGOUTBUF
    /*
     *  The rcmds start the connection with a series of init data:
     *
     *    a port number upon which client is listening for stderr data
     *    the user's name on the client machine
     *    the user's name on the server machine
     *    the terminal_type/speed or command to execute
     */
    outbuf[outbytes++] = 0;
    strcpy((char *)outbuf + outbytes, (char *)localuser);
    outbytes += strlen((char *)localuser) + 1;
    strcpy((char *)outbuf + outbytes, (char *)remoteuser);
    outbytes += strlen((char *)remoteuser) + 1;
    strcpy((char *)outbuf + outbytes, (char *)term_speed);
    outbytes += strlen((char *)term_speed) + 1;
    rc = ttol((CHAR *)outbuf, outbytes);
#else  /* RLOGOUTBUF */
    ttoc(0); /* Send an initial NUL as wake-up */
    /* Send each variable with the trailing NUL */
    rc = ttol(localuser, strlen((char *)localuser) + 1);
    if (rc > 0)
      rc = ttol(remoteuser, strlen((char *)remoteuser) + 1);
    if (rc > 0)
      rc = ttol(term_speed, strlen((char *)term_speed) + 1);
#endif /* RLOGOUTBUF */

    /* Now we are supposed to get back a single NUL as confirmation */
    errno = 0;
    rc = ttinc(60);
    debug(F101, "rlogin first ttinc", "", rc);
    if (rc > 0) {
      debug(F101, "rlogin ttinc 1", "", rc);
      printf("Rlogin protocol error - 0x%x received instead of 0x00\n", rc);
      return (-1);
    } else if (rc < 0) {
      debug(F101, "rlogin ttinc errno", "", errno);
      /* printf("Network error: %d\n", errno); */
      return (-1);
    }
  }
  return (0);
}
/* two control messages are defined:

   a double flag byte of 'o' indicates a one-byte message which is
   identical to what was once carried out of band.

   a double flag byte of 'q' indicates a zero-byte message.  This
   message is interpreted as two \377 data bytes.  This is just a
   quote rule so that binary data from the server does not confuse the
   client.  */

int rlog_ctrl(unsigned char *cp, int n) {
  if ((n >= 5) && (cp[2] == 'o') && (cp[3] == 'o')) {
    if (rlog_oob(&cp[4], 1))
      return (-5);
    return (5);
  } else if ((n >= 4) && (cp[2] == 'q') && (cp[3] == 'q')) {
    /* this is somewhat of a hack */
    cp[2] = '\377';
    cp[3] = '\377';
    return (2);
  }
  return (0);
}

static int rlog_oob(CHAR *oobdata, int count) {
  int i;
  int flush = 0;

  debug(F111, "rlogin out_of_band", "count", count);

  for (i = 0; i < count; i++) {
    debug(F101, "rlogin out_of_band", "", oobdata[i]);
    if (oobdata[i] & 0x01)
      continue;

    if (oobdata[i] & 0x02) { /* Flush Buffered Data not yet displayed */
      debug(F101, "rlogin Flush Buffered Data command", "", oobdata[i]);

      /* Only flush the data if in fact we are in a mode that won't */
      /* get out of sync.  Ie, not when we are in protocol mode.    */
      switch (what) {
      case W_NOTHING:
      case W_CONNECT:
      case W_COMMAND:
        if (rlog_inband)
          flush = 1;
        else
          ttflui();
        break;
      }
    }
    if (oobdata[i] & 0x10) { /* Switch to RAW mode */
      debug(F101, "rlogin Raw Mode command", "", oobdata[i]);
      rlog_mode = RL_RAW;
    }

    if (oobdata[i] & 0x20) { /* Switch to COOKED mode */
      debug(F101, "rlogin Cooked Mode command", "", oobdata[i]);
      rlog_mode = RL_COOKED;
    }
    if (oobdata[i] & 0x80) { /* Send Window Size Info */
      debug(F101, "rlogin Window Size command", "", oobdata[i]);
      /* Remember to send WS Info when Window Size changes */
      if (!TELOPT_ME(TELOPT_NAWS)) {
        TELOPT_ME(TELOPT_NAWS) = 1;
        rlog_naws();
      }
    }
  }
  return (flush);
}
#ifndef TCPIPLIB
static void rlogoobh(int sig) {
  CHAR oobdata;

  /* int  count = 0; */ /* (not used) */

  while (recv(ttyfd, &oobdata, 1, MSG_OOB) < 0) {
    /*
     * We need to do some special processing here.
     * Just in case the socket is blocked for input
     *
     */
    switch (errno) {
    case EWOULDBLOCK:
      break;
    default:
      return;
    }
  }
  debug(F101, "rlogin out_of_band", "", oobdata);
  if (oobdata == 0x02) { /* Flush Buffered Data not yet displayed */
    debug(F101, "rlogin Flush Buffered Data command", "", oobdata);
    netflui();
  }
  if (oobdata & 0x10) { /* Switch to raw mode */
    debug(F101, "rlogin Raw Mode command", "", oobdata);
    rlog_mode = RL_RAW;
  }
  if (oobdata & 0x20) { /* Switch to cooked mode */
    debug(F101, "rlogin Cooked Mode command", "", oobdata);
    rlog_mode = RL_COOKED;
  }
  if (oobdata & 0x80) { /* Send Window Size Info */
    debug(F101, "rlogin Window Size command", "", oobdata);
    /* Remember to send WS Info when Window Size changes */
    if (!TELOPT_ME(TELOPT_NAWS)) {
      TELOPT_ME(TELOPT_NAWS) = 1;
      rlog_naws();
    }
  }
}
#endif /* TCPIPLIB */
#endif /* RLOGCODE */
#endif /* NORLOGIN */

/* Send network BREAK */
/*
  Returns -1 on error, 0 if nothing happens, 1 if BREAK sent successfully.
*/
int netbreak() {
  CHAR buf[3];
  if (ttnet == NET_TCPB) {
    if (ttnproto == NP_TELNET) {
#ifdef TNCODE
      buf[0] = (CHAR)IAC;
      buf[1] = (CHAR)BREAK;
      buf[2] = (CHAR)0;
      if (ttol(buf, 2) < 2)
        return (-1);
      if (tn_deb || debses || deblog) {
        extern char tn_msg[];
        ckmakmsg(tn_msg, TN_MSG_LEN, "TELNET SENT ", TELCMD(BREAK), NULL, NULL);
        debug(F101, tn_msg, "", BREAK);
        if (debses || tn_deb)
          tn_debug(tn_msg);
      }
      return (1);
#else
      debug(F100, "netbreak no TNCODE", "", 0);
      return (0);
#endif /* TNCODE */
    }
    /* Insert other TCP/IP protocols here */
  }
  /* Insert other networks here */
  return (0);
}
#endif /* NETCONN */

#ifdef NETCONN

#ifdef IBMX25
/*
 * IBM X25 support - using the NPI streams interface
 * written by Stephen Riehm, pc-plus, Munich Germany
 */

/* riehm: missing functions / TODO list */

/*
  x25intr() - Send an interrupt packet
*/

/* return an error message depending on packet type */
char *x25err(n)
int n;
{
  static char buf[30];
  switch (n) {
  case NBADADDR:
    return "invalid address";
  case NBADOPT:
    return "invalid options";
  case NACCESS:
    return "no permission";
  case NNOADDR:
    return "unable to allocate address";
  case NOUTSTATE:
    return "invalid state";
  case NBADSEQ:
    return "invalid sequence number";
  case NSYSERR:
    return "system error";
  case NBADDATA:
    return "invalid data size";
  case NBADFLAG:
    return "invalid flag";
  case NNOTSUPPORT:
    return "unsupported primitive";
  case NBOUND:
    return "address in use";
  case NBADQOSPARAM:
    return "bad QOS parameters";
  case NBADQOSTYPE:
    return "bad QOS type";
  case NBADTOKEN:
    return "bad token value";
  case NNOPROTOID:
    return "protocol id could not be allocated";
  case NODDCUD:
    return "odd length call user data";
  default:
    ckmakmsg(buf, sizeof(buf), "Unknown NPI error ", ckitoa(n), NULL, NULL);
    return buf;
  }
}

/* turn a meaningless primitive number into a meaningful primitive name */
char *x25prim(n)
int n;
{
  static char buf[30];
  switch (n) {
  case N_BIND_ACK:
    return "N_BIND_ACK";
  case N_BIND_REQ:
    return "N_BIND_REQ";
  case N_CONN_CON:
    return "N_CONN_CON";
  case N_CONN_IND:
    return "N_CONN_IND";
  case N_CONN_REQ:
    return "N_CONN_REQ";
  case N_CONN_RES:
    return "N_CONN_RES";
  case N_DATACK_IND:
    return "N_DATAACK_IND";
  case N_DATACK_REQ:
    return "N_DATAACK_REQ";
  case N_DATA_IND:
    return "N_DATA_IND";
  case N_DATA_REQ:
    return "N_DATA_REQ";
  case N_DISCON_IND:
    return "N_DISCON_IND";
  case N_DISCON_REQ:
    return "N_DISCON_REQ";
  case N_ERROR_ACK:
    return "N_ERROR_ACK";
  case N_EXDATA_IND:
    return "N_EXDATA_IND";
  case N_EXDATA_REQ:
    return "N_EXDATA_REQ";
  case N_INFO_ACK:
    return "N_INFO_ACK";
  case N_INFO_REQ:
    return "N_INFO_REQ";
  case N_OK_ACK:
    return "N_OK_ACK";
  case N_OPTMGMT_REQ:
    return "N_OPTMGMT_REQ";
  case N_RESET_CON:
    return "N_RESET_CON";
  case N_RESET_IND:
    return "N_RESET_IND";
  case N_RESET_REQ:
    return "N_RESET_REQ";
  case N_RESET_RES:
    return "N_RESET_RES";
  case N_UDERROR_IND:
    return "N_UDERROR_IND";
  case N_UNBIND_REQ:
    return "N_UNBIND_REQ";
  case N_UNITDATA_REQ:
    return "N_UNITDATA_REQ";
  case N_UNITDATA_IND:
    return "N_UNITDATA_IND";
  default:
    ckmakmsg(buf, sizeof(buf), "UNKNOWN (", ckitoa(n), ")", NULL);
    return buf;
  }
}

/*****************************************************************************
 * Function: x25getmsg()
 * Description: get a STREAMS message, and check it for errors
 *
 * Parameters:
 * fd           - file descriptor to x25 device (opened)
 * control      - control buffer (pre-allocated)
 * ctl_size     - size of control buffer
 * data         - data buffer (pre-allocated)
 * data_size    - size of data buffer
 * flags        - flags for getmsg()
 * expected     - expected Primitive type
 *
 * Return Value:
 *      >= 0    OK (size of data returned)
 *      -1      error
 *
 */
int x25getmsg(fd, control, ctl_size, data, data_size, get_flags, expected)
int fd;               /* X25 device (opened) */
N_npi_ctl_t *control; /* control buffer (pre-allocated) */
int ctl_size;         /* size of control buffer */
N_npi_data_t *data;   /* data buffer (pre-allocated) */
int data_size;        /* size of data buffer */
int *get_flags;       /* getmsg() flags */
int expected;         /* expected primitive type */
/* x25getmsg */ {
  int rc = 0;                     /* return code */
  struct strbuf *get_ctl = NULL;  /* getmsg control */
  struct strbuf *get_data = NULL; /* getmsg data */
  int more = 0;                   /* flag for more data etc */
  int file_status = -1;           /* file async status */
  N_npi_ctl_t *result;            /* pointer to simplify switch() */
  int packet_type = -1;           /* unknown packet thus far */

#ifdef TRACE
  printf("TRACE: entering x25getmsg\n");
#endif /* TRACE */

  debug(F110, "x25getmsg waiting for packet ", x25prim(expected), 0);
  /* prepare the control structures for getmsg */
  if (control) {
    if ((get_ctl = (struct strbuf *)malloc(sizeof(struct strbuf))) == NULL) {
      perror("kermit x25getmsg(): get_ctl malloc failed\n");
      debug(F100, "x25getmsg malloc failed for get_ctl\n", "", 0);
      return (-1);
    }
    /* allow getmsg to return an unexpected packet type (which may be
     * larger than the expected one)
     */
    get_ctl->maxlen = NPI_MAX_CTL;
    get_ctl->len = 0;
    get_ctl->buf = (char *)control;
  } else {
    printf("kermit x25getmsg(): internal error. control buffer MUST be "
           "pre-allocated!\n");
    debug(F100, "x25getmsg internal error. no buffer pre-allocated", "", 0);
    return (-1);
  }
  if (data) {
    if ((get_data = (struct strbuf *)malloc(sizeof(struct strbuf))) == NULL) {
      perror("kermit x25getmsg(): get_data malloc failed\n");
      debug(F100, "x25getmsg malloc failed for get_data\n", "", 0);
      return (-1);
    }
    get_data->maxlen = (NPI_MAX_DATA < data_size) ? NPI_MAX_DATA : data_size;
    get_data->len = 0;
    get_data->buf = (char *)data;
  }

  /* get an X.25 packet -
   * it may be any kind of packet, so check for special cases
   * it may be split into multiple parts - so loop if necessary
   */
  do {
#ifdef DEBUG
    printf("kermit: x25getmsg(): getting a message\n");
#endif /* DEBUG */
    errno = 0;
    if ((more = getmsg(fd, get_ctl, get_data, get_flags)) < 0) {
#ifdef DEBUG
      printf("kermit: x25getmsg(): getmsg returned an error\n");
      perror("getmsg error was");
#endif /* DEBUG */
      debug(F101, "x25getmsg getmsg returned an error\n", "", errno);
      if ((errno == EAGAIN) && (get_data && (get_data->len > 0))) {
        /* was in non-blocking mode, nothing to get, but we're
         * already waiting for the rest of the packet -
         * switch to blocking mode for the next read.
         * file_status used to reset file status before returning
         */
        if ((file_status = fcntl(fd, F_GETFL, 0)) < 0 ||
            fcntl(fd, F_SETFL, file_status & ~O_NDELAY) < 0) {
          perror("x25getmsg(): couldn't change x25 blocking mode");
          debug(F101, "x25getmsg fcntl returned an error\n", "", errno);
          /* netclos(); */
          rc = -1;
          break;
        } else {
          /* loop again into a blocking getmsg() */
          continue;
        }
      } else {
        /* no data to get in non-blocking mode - return empty handed */
        perror("x25getmsg(): getmsg failed");
        debug(F101, "x25getmsg getmsg returned an error\n", "", errno);
        rc = -1;
        break;
      }
    } else if (more & MORECTL) {
      /* panic - the control information was larger than the
       * maximum control buffer size!
       */
      /* riehm: close connection? */
#ifdef DEBUG
      printf("x25getmsg(): received partial control packet - panic\n");
#endif /* DEBUG */
      debug(F101, "x25getmsg getmsg bad control block\n", "", errno);
      rc = -1;
      break;
    }

    if (result = (N_npi_ctl_t *)control) {
      packet_type = result->bind_ack.PRIM_type;
      if (packet_type != N_OK_ACK) {
        x25lastmsg = packet_type;
      }
    }
#ifdef DEBUG
    /* printf( "kermit: x25getmsg(): getting " ); */
    if (get_ctl->len > 0) {
      x25dump_prim(result);
    }
    debug(F110, "x25getmsg got packet ", x25prim(result->bind_ack.PRIM_type),
          0);
#endif /* DEBUG */

    if (get_ctl->len >= (int)sizeof(result->bind_ack.PRIM_type)) {
      /* not as pretty as a switch(), but switch can't handle
       * runtime variable values :-(
       */
      if (packet_type == expected) {
        /* got what we wanted, special case for DATA_IND
         * packets though */
        /* riehm: check Q-bit ? */
#ifdef DEBUG
        printf("x25getmsg(): got expected packet\nrc is %d\n", rc);
#endif /* DEBUG */
        if (packet_type == N_DATA_IND) {
          /* data received. May be incomplete, even though
           * getmsg returned OK
           */
          if (result->data_ind.DATA_xfer_flags & N_MORE_DATA_FLAG)
            more |= MOREDATA;
          if (result->data_ind.DATA_xfer_flags & N_RC_FLAG)
            printf("x25getmsg(): data packet wants ack\n");
        }
      } else if (packet_type == N_DISCON_IND) {
        printf("X25 diconnected\n");
        /* riehm: need to acknowledge a disconnection? */
        x25clear();
        /* x25unbind( ttyfd ); */
        rc = -1;
      } else if (packet_type == N_ERROR_ACK) {
        errno = result->error_ack.UNIX_error;
        perror("X25 error received");
        rc = -1;
      } else {
        printf("x25getmsg(): failed %s\n", x25err(packet_type));
        rc = -1;
      }
    }

    if (get_data && (get_data->len >= 0)) {
      get_data->buf += get_data->len;
      get_data->maxlen -= get_data->len;
    }
  } while ((rc == 0) && (get_data && (get_data->maxlen > 0)) &&
           (more & MOREDATA));

  /* return the file status to its original value, unless its still
   * set to -1, or one of the fcntl's failed */
  if ((file_status >= 0) && fcntl(fd, F_SETFL, file_status) < 0)
    rc = -1;

  /*
   * Verify that we received an expected primitive
   * there is apparantly an error case where the primitive is set
   * correctly, but there is not enough data in the control structure
   */
  if ((packet_type != expected) && (get_ctl->len >= ctl_size)) {
    fprintf(stderr, "x25getmsg(): %s NOT received. Primitive received was %s\n",
            x25prim(expected), x25prim(packet_type));
    debug(F110, "x25getmsg got an unexpected packet ", x25prim(packet_type), 0);
    rc = -1;
  }

  if (rc == 0) {
    if (get_data && (get_data->len >= 0)) {
      rc = get_data->len;
    }
  }

  if (get_ctl) {
    free(get_ctl);
    get_ctl = NULL;
  }
  if (get_data) {
    free(get_data);
    get_data = NULL;
  }

  debug(F110, "x25getmsg returning packet ", x25prim(packet_type), 0);

#ifdef TRACE
  printf("TRACE: leaving x25getmsg\n");
#endif /* TRACE */
  return (rc);
}

/*****************************************************************************
 * Function: x25putmsg()
 *
 * Description:
 *      send a message to a X25 STREAM
 *
 * Parameters:
 *      fd              - file descriptor to x25 device (opened)
 *      control         - control buffer (pre-allocated)
 *      data            - data buffer (pre-allocated)
 *      data_len        - length of data to be transmitted
 *      put_flags       - flags for putmsg()
 *
 * Return Value:
 *      >= 0    number of bytes transmitted
 *      -1      error
 */
int x25putmsg(fd, control, data, data_len, put_flags)
int fd;               /* X25 device (opened) */
N_npi_ctl_t *control; /* control buffer (pre-allocated) */
N_npi_data_t *data;   /* data buffer (pre-allocated) */
int data_len;         /* length of data (not the size of
                         the buffer) */
int *put_flags;       /* putmsg() flags */
/* x25putmsg */ {
  int rc = 0;                     /* return code */
  ulong type;                     /* primitive type */
  struct strbuf *put_ctl = NULL;  /* putmsg control */
  struct strbuf *put_data = NULL; /* putmsg data */

#ifdef TRACE
  printf("TRACE: entering x25putmsg\n");
#endif /* TRACE */

#ifdef DEBUG
  printf("kermit: x25putmsg(): putting ");
  x25dump_prim(control);
  printf("\tdata:\t\t");
  x25dump_data(data, 0, data_len);
  debug(F110, "x25putmsg: putting packet ", x25prim(control->PRIM_type), 0);
#endif /* DEBUG */

  if (control) {
    put_ctl = (struct strbuf *)malloc(sizeof(struct strbuf));
    if (put_ctl == NULL) {
      perror("kermit x25putmsg(): put_ctl malloc failed\n");
      return (-1);
    }
    put_ctl->maxlen = 0; /* unused by putmsg */
    put_ctl->len = NPI_MAX_CTL;
    put_ctl->buf = (char *)control;
  }
  if (data && (data_len > 0)) {
    put_data = (struct strbuf *)malloc(sizeof(struct strbuf));
    if (put_data == NULL) {
      perror("kermit x25putmsg(): put_data malloc failed\n");
      return (-1);
    }
    put_data->maxlen = 0; /* unused by putmsg */
    put_data->len = data_len;
    put_data->buf = (char *)data;
  }

  errno = 0;
  rc = putmsg(fd, put_ctl, put_data, 0);
  if (rc < 0) {
    printf("x25putmsg(): couldn't put %s\n", x25prim(control->PRIM_type));
    perror("kermit: x25putmsg(): putmsg failed");
    return (-1);
  }

  /* riehm: this should perhaps be discounted! */
  x25lastmsg = control->PRIM_type;

  debug(F101, "x25putmsg block size put ", "", data_len);

#ifdef TRACE
  printf("TRACE: leaving x25putmsg\n");
#endif /* TRACE */

  return (data_len);
}

/*****************************************************************************
 * Function: x25bind
 * Description:  The bind submitted to NPI provides the information required
 *               by the packet layer for it to listen for suitable incoming
 *               calls.
 *
 * WARNING:
 *
 * This routine needs to be called in a completely different manner for
 * the client and server side. When starting a client, the
 * num_waiting_calls and CUD information should all be set to 0! The
 * client's CUD must be inserted in the CONN_REQ data block.
 * When starting a server, the CUD must be set to a CUD pattern, and
 * the number of waiting calls should be set to a number other than 0.
 * (num waiting calls is the number of incomming calls which are to be
 * put on hold while the server is servicing another client.)
 *
 * Who invented this crap?
 *
 * Parameters:
 *       fd              - X25 device (opened)
 *       addr            - local address
 *       cud             - User Data (null terminated)
 *       cud_len         - User Data length
 *       num_waiting_calls - number of outstanding calls allowed on this stream
 *       line            - logical port number (1)
 *       flags           - 0, DEFAULT_LISTENER or TOKEN_REQUEST
 *
 * Return Value:
 *       if binding is successful, 0 is returned for a client, and a token is
 *       returned for a server
 *
 * Return code: 0 if successful
 *              -1 if unsuccessful
 *****************************************************************************/

ulong x25bind(fd, addr, cud, cud_len, num_waiting_calls, line, bind_flags)
int fd;                /* X25 device (opened) */
char *addr;            /* local address */
char *cud;             /* Call User Data (null terminated) */
int cud_len;           /* User Data length */
int num_waiting_calls; /* Outstanding calls allowed */
int line;              /* logical port number */
ulong bind_flags;      /* 0, DEFAULT_LISTENER or TOKEN_REQUEST */
/* x25bind */ {
  ulong rc;               /* return code */
  int get_flags;          /* priority flag passed to getmsg */
  int put_flags = 0;      /* output flags for putmsg, always 0 */
  ulong type;             /* primitive type */
  N_bind_req_t *bind_req; /* pointer to N_BIND_REQ primitive */
  N_bind_ack_t *bind_ack; /* pointer to N_BIND_ACK primitive */
  char *addtl_info;       /* pointer to info in addition to
                           * the N_BIND_REQ primitive that is
                           * passed in the control structure
                           * to putmsg */
  int addr_len = 0;       /* length of address string */
  ulong bind_req_t_size;  /* for debugging only */

#ifdef TRACE
  printf("TRACE: entering x25bind\n");
#endif /* TRACE */

#ifdef DEBUG
  printf("TRACE: x25bind( %d, %s, %s, %d, %d )\n", fd, addr, cud, line,
         bind_flags);
#endif /* DEBUG */

  /*
   * Allocate  and zero out space to hold the control portion of the
   * message passed to putmsg. This will contain the N_BIND_REQ
   * primitive and any additional info required for that.
   *
   * Note: allocated space is the size of the union typedef
   * N_npi_ctl_t to allow the use fo the generic x25putmsg routine.
   */
  bind_req = (N_bind_req_t *)malloc(sizeof(N_npi_ctl_t));
  if (bind_req == NULL) {
    perror("kermit: x25bind(): bind_req malloc failed");
    debug(F100, "x25bind bind_req malloc failed", "", 0);
    return (-1);
  }
  bzero((char *)bind_req, sizeof(N_npi_ctl_t));

  /* Build the Bind Request Primitive */
  bind_req->PRIM_type = (ulong)N_BIND_REQ;

  /* Note that the address length is n+2 and NOT n. Two bytes MUST preceed
   * the actual address in an N_BIND_REQ. The first byte contains the
   * line number being used with this address, and the second byte is the
   * X.121 address prefix, which must be zero.
   */
  addr_len = strlen(addr);
  bind_req->ADDR_length = (ulong)(addr_len + 2);
  bind_req->ADDR_offset = (ulong)(sizeof(N_bind_req_t));
  bind_req->CONIND_number = (ulong)num_waiting_calls; /* server only */
  bind_req->BIND_flags = (ulong)bind_flags;           /* 0 in client */
  bind_req->PROTOID_length = (ulong)cud_len;          /* 0 in client */
  if (cud_len == 0) {
    bind_req->PROTOID_offset = (ulong)0;
  } else {
    /* need to remember the trailing NULL in the address - not
     * counted in the address length
     */
    bind_req->PROTOID_offset =
        (ulong)(sizeof(N_bind_req_t) + bind_req->ADDR_length);
  }

  /*
   * Now fill in the additional information required with this primitive
   * (address and protocol information (Call User Data))
   */
  addtl_info = (char *)((void *)bind_req + bind_req->ADDR_offset);
  /*
   * The bitwise "&" ensures that the line number is only one byte long
   */
  *addtl_info++ = (char)line & 0xff;
  *addtl_info++ = (char)0;           /* X.121 format */
  bcopy(addr, addtl_info, addr_len); /* include trailing null */
  addtl_info += addr_len;
  if (cud_len > 0)
    bcopy(cud, addtl_info, cud_len);
  /*
   * Call putmsg() to put the bind request message on the stream
   */
  if (x25putmsg(fd, (N_npi_ctl_t *)bind_req, (N_npi_data_t *)NULL, 0,
                &put_flags) < 0) {
    printf("kermit: x25bind(): x25putmsg failed\n");
    return (-1);
  }

  /*
   * Allocate and zero out space for the N_BIND_ACK primitive
   */
  bind_ack = (N_bind_ack_t *)malloc(sizeof(N_npi_ctl_t));
  if (bind_ack == NULL) {
    perror("kermit: x25bind(): bind_ack malloc failed");
    return (-1);
  }
  bzero(bind_ack, sizeof(N_npi_ctl_t));
  /*
   * Initialize the control structure and flag variable sent to getmsg
   */
  get_flags = 0;

  /* get the ACK for the bind */
#ifdef DEBUG
  printf("kermit: x25bind() trying to get a BIND_ACK\n");
#endif /* DEBUG */
  rc = (ulong)x25getmsg(fd, (N_npi_ctl_t *)bind_ack, (int)sizeof(N_bind_ack_t),
                        (N_npi_data_t *)NULL, 0, &get_flags, N_BIND_ACK);

  /* turn quantitive return code into a qualitative one */
  if (rc > 0)
    rc = 0;

  /* if all went well, get the token from the acknowledgement packet */
  if ((bind_flags & TOKEN_REQUEST) && (rc >= 0)) {
    rc = bind_ack->TOKEN_value;
  }

  /* free up the memory we allocated earlier */
  free(bind_req);
  free(bind_ack);

#ifdef TRACE
  printf("TRACE: leaving x25bind\n");
#endif /* TRACE */

  return (rc);
}

/*****************************************************************************
 * Function: x25call
 * Description:  This routine builds and sends an N_CONN_REQ primitive, then
 *               checks for an N_CONN_CON primitive in return.
 *
 * Parameters:
 * fd    - file descriptor of stream
 * caddr - called address (remote address)
 *
 * Functions Referenced:
 * malloc()
 * bzero()
 * getmsg()
 * putmsg()
 *
 * Return code:
 * 0 - if successful
 * -1 if not successful
 *****************************************************************************/
int x25call(fd, remote_nua, cud)
int fd;           /* X25 device (opened) */
char *remote_nua; /* remote address to call */
char *cud;        /* call user data */
/* x25call */ {
  int rc;                     /* return code */
  int flags;                  /* Connection flags */
  int get_flags;              /* priority flags for getmsg */
  ulong type;                 /* primitive type */
  N_conn_req_t *connreq_ctl;  /* pointer to N_CONN_REQ primitive */
  N_npi_data_t *connreq_data; /* pointer to N_CONN_REQ data (CUD) */
  int connreq_data_len;       /* length of filled data buffer */
  N_conn_con_t *conncon_ctl;  /* pointer to N_CONN_CON primitive */
  N_npi_data_t *conncon_data; /* pointer to any data associated with
                               * the N_CONN_CON primitive */
  char *addtl_info;           /* pointer to additional info needed
                               * for N_CONN_REQ primitive */
  int addr_len;               /* length of address */

#ifdef TRACE
  printf("TRACE: entering x25call\n");
#endif /* TRACE */

#ifdef DEBUG
  printf("x25call( %d, %s )\n", fd, remote_nua);
  printf("connecting to %s on fd %d\n", remote_nua, fd);
#endif /* DEBUG */

  /*
   * Allocate and zero out space for the N_CONN_REQ primitive
   * use the size of the generic NPI primitive control buffer
   */
  connreq_ctl = (N_conn_req_t *)malloc(sizeof(N_npi_ctl_t));
  if (connreq_ctl == NULL) {
    perror("kermit: x25call(): connreq_ctl malloc failed");
    return (-1);
  }
  bzero(connreq_ctl, sizeof(N_npi_ctl_t));
  /*
   * Build the Connection Request Primitive
   */
  flags = 0;
  connreq_ctl->PRIM_type = (ulong)N_CONN_REQ;

  /* Note that the address length is nchai+1 and not n+2. The line number
   * is only passed with the address for the bind. The first byte of
   * the address for the N_CONN primitives contains the X.121
   * address prefix, which must be zero. The remaining bytes are the
   * address itself.
   */
  addr_len = strlen(remote_nua);
  connreq_ctl->DEST_length = (ulong)(addr_len + 1);
  connreq_ctl->DEST_offset = (ulong)sizeof(N_conn_req_t);
  /* connreq_ctl->CONN_flags = (ulong)EX_DATA_OPT | REC_CONF_OPT; */
  connreq_ctl->CONN_flags = (ulong)0;
  connreq_ctl->QOS_length = (ulong)0; /* unsupported in AIX 4.1 */
  connreq_ctl->QOS_offset = (ulong)0; /* unsupported in AIX 4.1 */

  addtl_info = (char *)((void *)connreq_ctl + connreq_ctl->DEST_offset);
  *addtl_info++ = (char)0; /* X.121 format */
  bcopy(remote_nua, addtl_info, addr_len);

  /*
   * setup the data buffer for the connection request
   */
  connreq_data = (N_npi_data_t *)malloc(sizeof(N_npi_data_t));
  if (connreq_data == NULL) {
    perror("kermit: x25call(): connreq_data malloc failed");
    return (-1);
  }
  bzero(connreq_data, sizeof(N_npi_data_t));

  /* facility selection needs to be put in the front of connreq_data */
  connreq_data_len = 0;
  connreq_data_len += x25facilities((char *)connreq_data);
  if (cud && *cud) {
    bcopy(cud, (char *)((char *)connreq_data + connreq_data_len), strlen(cud));
    connreq_data_len += strlen(cud);
  }

  /*
   * Call putmsg() to put the connection request message on the stream
   */
  rc = x25putmsg(fd, (N_npi_ctl_t *)connreq_ctl, connreq_data, connreq_data_len,
                 &flags);
  if (rc < 0) {
    return (-1);
  }

  /*
   * Allocate and zero out space for the N_CONN_CON primitive
   */
  if ((conncon_ctl = (N_conn_con_t *)malloc(sizeof(N_npi_ctl_t))) == NULL) {
    perror("kermit: x25call(): conncon_ctl malloc failed");
    return (-1);
  }
  bzero(conncon_ctl, sizeof(N_npi_ctl_t));

  /*
   * Allocate and zero out space for any data associated with N_CONN_CON
   */
  if ((conncon_data = (N_npi_data_t *)malloc(NPI_MAX_DATA)) == NULL) {
    perror("kermit: x25call(): conncon_data malloc failed");
    return (-1);
  }
  bzero(conncon_data, NPI_MAX_DATA);

  /* Initialize and build the structures for getmsg */
  get_flags = 0;

  rc = x25getmsg(fd, (N_npi_ctl_t *)conncon_ctl, (int)sizeof(N_conn_con_t),
                 conncon_data, NPI_MAX_DATA, &get_flags, N_CONN_CON);

  /* turn quantitive return code into a qualitative one */
  if (rc > 0)
    rc = 0;

  /* Free the space that we no longer need */
  if (connreq_ctl) {
    free(connreq_ctl);
    connreq_ctl = NULL;
  }
  if (conncon_ctl) {
    free(conncon_ctl);
    conncon_ctl = NULL;
  }
  if (conncon_data) {
    free(conncon_data);
    conncon_data = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25call\n");
#endif /* TRACE */

  return (rc);
}

/*****************************************************************************
 * Function: x25getcall
 *
 * Description: This routine checks for an incomming call, verified
 * that it is a CONNIND (connection indication) message, and then
 * accepts the call and returns the file descriptor of the new stream
 *
 * Parameters:
 * fd   - file descriptor of listening stream
 *
 * Return Codes:
 * callfd       - file descriptor of connected incomming call.
 *              - set to -1 if an error occured
 *
 *****************************************************************************/
int x25getcall(fd)
int fd;
{
  int x25callfd;              /* fd of incomming call */
  N_conn_ind_t *connind_ctl;  /* connind controll buffer */
  N_npi_data_t *connind_data; /* connind data buffer */
  int get_flags;              /* flags for getmsg */
  ulong flags;                /* connection flags */
  int rc;                     /* return code */

  extern x25addr_t remote_nua; /* remote X.25 addr global var */

#ifdef TRACE
  printf("TRACE: entering x25getcall\n");
#endif /* TRACE */

  /* allocate space for connection indication buffers */
  if ((connind_ctl = (N_conn_ind_t *)malloc(sizeof(N_npi_ctl_t))) == NULL) {
    perror("kermit: x25getcall(): connind_ctl malloc failed");
    return (-1);
  }
  bzero(connind_ctl, sizeof(N_npi_ctl_t));

  if ((connind_data = (N_npi_data_t *)malloc(NPI_MAX_DATA)) == NULL) {
    perror("kermit: x25getcall(): connind_data malloc failed");
    return (-1);
  }
  bzero(connind_data, NPI_MAX_DATA);

  /* initialise control structures */
  get_flags = 0;

  /* call getmsg to check for a connection indication */
  if (x25getmsg(fd, (N_npi_ctl_t *)connind_ctl, (int)sizeof(N_conn_ind_t),
                connind_data, NPI_MAX_DATA, &get_flags, N_CONN_IND) < 0) {
#ifdef DEBUG
    printf("x25getcall(): errno is: %d\n", errno);
#endif /* DEBUG */
    perror("x25getcall(): getmsg failed");
    return (-1);
  }

  /* a connection indication was received
   * - pull it to bits and answer the call
   */
  x25seqno = connind_ctl->SEQ_number;
  flags = connind_ctl->CONN_flags;
#ifdef DEBUG
  printf("setting remote_nua to a new value due to incomming call\n");
#endif /* DEBUG */
  /*
   * no guarantee that the address is null terminated, ensure that
   * after copying that it is (assumption: remote_nua is longer than
   * the address + 1)
   */
  bzero(remote_nua, sizeof(remote_nua));
  /* note: connind_ctl contains a x121 address, which has a null as
   * the FIRST character - strip it off!
   */
  ckstrncpy(remote_nua,
            (char *)((char *)connind_ctl + connind_ctl->SRC_offset + 1),
            connind_ctl->SRC_length - 1);
#ifdef DEBUG
  printf("remote_nua set to new value of %s\n", remote_nua);
#endif /* DEBUG */

  /* errors handled by callee */
  x25callfd = x25accept(x25seqno, flags);

  /* free the malloc'd buffers */
  if (connind_ctl) {
    free(connind_ctl);
    connind_ctl = NULL;
  }
  if (connind_data) {
    free(connind_data);
    connind_data = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25getcall\n");
#endif /* TRACE */

  /* return the file descriptor (or error if < 0) */
  return (x25callfd);
}

/*****************************************************************************
 * Function: x25accept
 *
 * Description: accept an incomming call
 *              This essentially means opening a new STREAM and sending
 *              an acknowledge back to the caller.
 *
 * Parameters:
 *      seqno   - sequence number for acknowledgement
 *      flags   - flags passed to us by the caller
 *
 * Return Codes:
 *      fd      - file descriptor of new STREAM
 *                set to -1 if an error occured
 *
 *****************************************************************************/
int x25accept(seqno, flags)
ulong seqno; /* connection sequence number */
ulong flags; /* connection flags */
/* x25accept */ {
  int x25callfd;          /* fd for incomming call */
  int get_flags;          /* priority flags for getmsg */
  int put_flags = 0;      /* flags for putmsg, always 0 */
  int addr_len;           /* length of local address */
  ulong token;            /* connection token */
  N_conn_res_t *conn_res; /* N_CONN_RES primitive */
  N_ok_ack_t *ok_ack;     /* N_OK_ACK primitive */
  char *addtl_info;       /* temp pointer */
  int rc;                 /* temporary return code */

  /* global variables from ckcmai.c */
  extern int revcall, closgr, cudata;
  extern char udata[];
  extern x25addr_t local_nua; /* local X.25 address */
  extern char x25name[];      /* x25 device name (sx25a0) */
  extern char x25dev[];       /* x25 device file /dev/x25pkt */
  extern int x25port;         /* logical port to use */
  ulong bind_flags = 0;       /* flags for binding the X25 stream */

#ifdef TRACE
  printf("TRACE: entering x25accept\n");
#endif /* TRACE */

  /* open a new packet level stream */
  if ((x25callfd = open(x25dev, O_RDWR)) < 0) {
    perror("kermit: x25accept(): X.25 device open error");
    debug(F101, "x25accept() device open error", "", errno);
    return (-1);
  }

  /* push the NPI onto the STREAM */
  if (ioctl(x25callfd, I_PUSH, "npi") < 0) {
    perror("kermit: x25accept(): couldn't push npi on the X25 stream");
    debug(F101, "x25accept can't push npi on the X25 stream", "", errno);
    return (-1);
  }

  /* bind kermit server to the local X25 address */
  /* taken from /usr/samples/sx25/npi/npiserver.c (AIX 4) */
  bind_flags |= TOKEN_REQUEST;
  token =
      x25bind(x25callfd, local_nua, (char *)NULL, 0, 0, x25port, bind_flags);
  if (token < 0) {
    printf("kermit: x25accept(): couldn't bind to local X25 address\n");
    netclos();
    return (-1);
  }

  /* allocate connection response primitive */
  if ((conn_res = (N_conn_res_t *)malloc(NPI_MAX_CTL)) == NULL) {
    perror("kermit: x25accept(): conn_res malloc failed");
    return (-1);
  }
  bzero((char *)conn_res, NPI_MAX_CTL);

  /* setup connection response primitive */
  addr_len = strlen(local_nua);
  conn_res->PRIM_type = (ulong)N_CONN_RES;
  conn_res->TOKEN_value = token;
  /* note address length is n+1 to accomodate the X.121 address prefix */
  conn_res->RES_length = (ulong)(addr_len + 1);
  conn_res->RES_offset = (ulong)sizeof(N_conn_res_t);
  conn_res->SEQ_number = seqno;
  conn_res->CONN_flags = 0;
  conn_res->QOS_length = 0; /* unsupported - must be 0 (!?) */
  conn_res->QOS_offset = 0;

  addtl_info = (char *)((char *)conn_res + conn_res->RES_offset);
  *addtl_info++ = (char)0; /* X.121 address prefix */
  bcopy(local_nua, addtl_info, addr_len);

  /*
   * send off the connect response
   */
  if (x25putmsg(x25callfd, (N_npi_ctl_t *)conn_res, (N_npi_data_t *)NULL, 0,
                &put_flags) < 0) {
    perror("kermit: x25accept(): putmsg connect response failed");
    return (-1);
  }

  /*
   * Allocate and zero out space for the OK_ACK primitive
   */
  if ((ok_ack = (N_ok_ack_t *)malloc(sizeof(N_npi_ctl_t))) == NULL) {
    perror("kermit: x25call(): ok_ack malloc failed");
    return (-1);
  }
  bzero(ok_ack, sizeof(N_npi_ctl_t));

  /* Initialize and build the structures for getmsg */
  get_flags = 0;

  rc = (int)x25getmsg(x25callfd, (N_npi_ctl_t *)ok_ack, (int)sizeof(N_ok_ack_t),
                      (N_npi_data_t *)NULL, 0, &get_flags, N_OK_ACK);
  if (rc == 0) {
    /* sequence number is only for disconnecting when not connected !? */
    x25seqno = 0;
  }

  /* free up malloc'ed buffer space */
  if (conn_res) {
    free(conn_res);
    conn_res = NULL;
  }
  if (ok_ack) {
    free(ok_ack);
    ok_ack = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25accept\n");
#endif /* TRACE */

  return ((rc >= 0) ? x25callfd : -1);
}

/*****************************************************************************
 * Function: x25unbind
 *
 * Description:  This subroutine builds and sends an unbind request and gets
 * the acknowledgement for it.
 *
 * Parameters:
 * fd - File descriptor of the stream
 *
 * Functions Referenced:
 * getmsg()
 * putmsg()
 * malloc()
 * bzero()
 *
 * Return code:
 * 0 - if successful
 * -1 - if not successful
 *****************************************************************************/
int x25unbind(fd)
int fd;
{                             /* X25 device (opened) */
  int rc;                     /* return code */
  int flags;                  /* bind flags */
  int get_flags;              /* priority flag for getmsg */
  ulong type;                 /* primitive type */
  N_unbind_req_t *unbind_req; /* pointer to N_UNBIND_REQ */
  N_ok_ack_t *ok_ack;         /* pointer to N_OK_ACK */

#ifdef TRACE
  printf("TRACE: entering x25unbind\n");
#endif /* TRACE */

#ifdef DEBUG
  /* printf( "x25unbind( %d )\n", fd ); */
#endif /* DEBUG */
  debug(F101, "x25unbind closing x25 connection #", "", fd);

  /* Allocate and zero out space to hold the N_UNBIND_REQ primitive */
  unbind_req = (N_unbind_req_t *)malloc(sizeof(N_npi_ctl_t));
  if (unbind_req == NULL) {
    perror("kermit: x25unbind(): unbind_req malloc failed");
    return (-1);
  }
  bzero(unbind_req, sizeof(N_npi_ctl_t));

  /*
   * Build the Unbind Request Primitive
   */
  flags = 0;
  unbind_req->PRIM_type = (ulong)N_UNBIND_REQ;

  /*
   * Call putmsg() to put the bind request message on the stream
   */
  if (x25putmsg(fd, (N_npi_ctl_t *)unbind_req, (N_npi_data_t *)NULL, 0,
                &flags) < 0) {
    perror("kermit: x25unbind(): putmsg failed");
    return (-1);
  }

  /* Allocate and Zero out space for the N_OK_ACK primitive */
  ok_ack = (N_ok_ack_t *)malloc(sizeof(N_npi_ctl_t));
  if (ok_ack == NULL) {
    perror("kermit x25unbind(): ok_ack malloc failed\n");
    return (-1);
  }
  bzero(ok_ack, sizeof(N_npi_ctl_t));

  /* Initialize and build the control structure for getmsg */
  get_flags = 0;

  /* Call getmsg() to check for an acknowledgement */
  rc = x25getmsg(fd, (N_npi_ctl_t *)ok_ack, (int)sizeof(N_ok_ack_t),
                 (N_npi_data_t *)NULL, 0, &get_flags, N_OK_ACK);
  if (rc < 0) {
    perror("kermit: x25unbind: getmsg failed");
    return (-1);
  }

  /* Free up the space that we no longer need */
  if (unbind_req) {
    free(unbind_req);
    unbind_req = NULL;
  }
  if (ok_ack) {
    free(ok_ack);
    ok_ack = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25unbind\n");
#endif /* TRACE */

  return (0);
}

/*****************************************************************************
 * Function: x25xin
 *
 * Description:
 *      Read n characters from X.25 circuit into buf (AIX only)
 *
 * Parameters:
 *      data_buf_len    maximum size of data buffer
 *      data_buf        pointer to pre-allocated buffer space
 *
 * Return Value:
 *      the number of characters actually read
 */
int x25xin(data_buf_len, data_buf)
int data_buf_len;
CHAR *data_buf;
{
  struct strbuf getmsg_ctl;  /* streams control structure */
  struct strbuf getmsg_data; /* streams data structure */
  int rc = 0;                /* return code */
  int getmsg_flags;          /* packet priority flags */
  char *ctl_buf;             /* npi control buffer */
  N_npi_ctl_t *result;       /* pointer to simplify switch() */

#ifdef TRACE
  printf("TRACE: entering x25xin\n");
#endif /* TRACE */

  /* ensure that no maximum's are overridden */
  data_buf_len = (NPI_MAX_DATA < data_buf_len) ? NPI_MAX_DATA : data_buf_len;

  /* allocate space for packet control info */
  if ((ctl_buf = (char *)malloc(NPI_MAX_CTL)) == NULL) {
    perror("kermit: x25xin(): ctl_buf malloc");
    return (-1);
  }

  getmsg_flags = 0; /* get the first packet available */

  rc = x25getmsg(ttyfd, ctl_buf, NPI_MAX_CTL, data_buf, data_buf_len,
                 &getmsg_flags, N_DATA_IND);

  /* free buffers */
  if (ctl_buf) {
    free(ctl_buf);
    ctl_buf = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25xi\n");
#endif /* TRACE */

  return (rc);
}

/*****************************************************************************
 * Function: x25write
 *
 * Description:
 *      write a block of characters to the X25 STREAM (AIX)
 *
 * Parameters:
 *      fd              file descriptor to write to
 *      databuf         buffer containing data to write
 *      databufsize             size of the buffer to write
 *
 * Return Value:
 *      size            the number of bytes actually transmitted
 */
int x25write(fd, databuf, databufsize)
int fd;          /* X25 STREAMS file descriptor (ttyfd) */
char *databuf;   /* buffer to write */
int databufsize; /* buffer size */
/* x25write */ {
  N_data_req_t *data_req_ctl;
  int rc;              /* return code (size transmitted) */
  int write_flags = 0; /* always 0 !? */

#ifdef TRACE
  printf("TRACE: entering x25write\n");
#endif /* TRACE */

  if ((data_req_ctl = (N_data_req_t *)malloc(NPI_MAX_CTL)) == NULL) {
    perror("kermit: x25write(): data_req_ctl malloc");
    return (-1);
  }
  data_req_ctl->PRIM_type = N_DATA_REQ;
  data_req_ctl->DATA_xfer_flags = 0;

  /* riehm: possible extension
   * possibly need to think about splitting up the data buffer
   * into multiple parts if databufsize > NPI_MAX_DATA
   */

  rc = x25putmsg(fd, (N_npi_ctl_t *)data_req_ctl, (N_npi_data_t *)databuf,
                 databufsize, &write_flags);
  if (data_req) {
    free(data_req_ctl);
    data_req = NULL;
  }

#ifdef TRACE
  printf("TRACE: leaving x25write\n");
#endif /* TRACE */

  return (rc);
}

/*****************************************************************************
 * Function: x25local_nua
 *
 * Description:
 *      This routine is only interesting for IBM computers. In order
 *      to set up a connection (see x25bind()) you need to know the
 *      local NUA (x25 address). Unfortunately, you need all this code
 *      to find that out, I just hope this works for everyone else!
 *
 * Parameters:
 *      a pre-allocated character buffer, long enough to hold an X.25 address
 *      and the tailing null.
 *
 * Return Value:
 *      the length of the address string.
 *      0 = error
 */
int x25local_nua(char *buf) {
  struct CuAt *response; /* structure to fill with info from ODM */
  CLASS_SYMBOL retClass; /* ODM class */
  char query[64];        /* odm database query */
  int rc = 0;            /* return value (length of local NUA) */
  extern char x25name[]; /* x25 device name (sx25a0) */

#ifdef TRACE
  printf("TRACE: entering x25local_nua\n");
#endif /* TRACE */

  /* set up query string */
  if (x25name[0] == '\0') {
#ifdef DEBUG
    printf("kermit: x25local_nua(): No x25 device set, trying sx25a0\n");
#endif /* DEBUG */
    strcpy(x25name, "sx25a0");
  }
  ckmakmsg(query, sizeof(query), "name like ", x25name,
           " and attribute like local_nua");

  /* initialise ODM database */
  odmerrno = 0;
  if (odm_initialize() == -1) {
    printf("x25local_nua(): can't initialize ODM database");
    switch (odmerrno) {
    case ODMI_INVALID_PATH:
      printf("invalid path\n");
      break;
    case ODMI_MALLOC_ERR:
      printf("malloc failed\n");
      break;
    default:
      printf("unknown error %d\nPlease call IBM\n", odmerrno);
    }
    return (rc);
  }

  /* open the CuAt class */
  retClass = odm_open_class(CuAt_CLASS);
  if (((int)retClass) == -1) {
    printf("kermit: x25local_nua(): can't open CuAt class in odm. ");
    switch (odmerrno) {
    case ODMI_CLASS_DNE:
      printf("CuAt class doesn't exist\n");
      break;
    case ODMI_CLASS_PERMS:
      printf("permission to CuAt class file denied\n");
      break;
    case ODMI_MAGICNO_ERR:
      printf("CuAt is an invalid ODM object class\n");
      break;
    case ODMI_OPEN_ERR:
      printf("cannot open CuAt class - and don't know why!\n");
      break;
    case ODMI_INVALID_PATH:
      printf("invalid path\n");
      break;
    case ODMI_TOOMANYCLASSES:
      printf("too many object classes have been opened\n");
      break;
    default:
      printf("unknown error %d\nPlease call IBM\n", odmerrno);
    }
    return (rc);
  }

#ifdef DEBUG
  printf("retClass= %d\n", retClass);
#endif /* DEBUG */

  response = (struct CuAt *)odm_get_first(retClass, query, NULL);
  if (((int)response) == -1) {
    printf("kermit: x25local_nua(): odm query failed ");
    switch (odmerrno) {
    case ODMI_BAD_CRIT: /* Programming error */
      printf("bad search criteria\n");
      break;
    case ODMI_CLASS_DNE:
      printf("CuAt class doesn't exist\n");
      break;
    case ODMI_CLASS_PERMS:
      printf("permission to CuAt class file denied\n");
      break;
    case ODMI_INTERNAL_ERR:
      printf("odm internal error\nPlease contact your administrator\n");
      break;
    case ODMI_INVALID_CLXN:
      printf("CuAt is invalid or inconsistent odm class collection\n");
      break;
    case ODMI_INVALID_PATH:
      printf("invalid path\n");
      break;
    case ODMI_MAGICNO_ERR:
      printf("CuAt is an invalid ODM object class\n");
      break;
    case ODMI_MALLOC_ERR:
      printf("malloc failed\n");
      break;
    case ODMI_OPEN_ERR:
      printf("cannot open CuAt class - and don't know why!\n");
      break;
    case ODMI_TOOMANYCLASSES:
      printf("too many object classes have been opened\n");
      break;
    default:
      printf("unknown error %d\nPlease call IBM\n", odmerrno);
    }
    return (rc);
  }

  /* check for a meaningfull response */
  if (response != NULL) {
    if (response->value != NULL) {
      strcpy(buf, response->value);
      rc = strlen(buf);
#ifdef DEBUG
/*
            printf( "attribute name is: %s\n", (char *)response->attribute );
            printf( "I think my address is %s\n", (char*)response->value );
*/
#endif /* DEBUG */
    } else {
      printf("kermit: x25local_nua(): couldn't find the local NUA\n");
    }
  } else {
    switch (odmerrno) {
    case ODMI_BAD_CRIT:
      printf("Error: ODMI_BAD_CRIT - bad criteria\n");
      break;
    case ODMI_CLASS_DNE:
      printf("Error: ODMI_CLASS_DNE - class doesn't exist\n");
      break;
    case ODMI_CLASS_PERMS:
      printf("Error: ODMI_CLASS_PERMS - class permissions\n");
      break;
    case ODMI_INTERNAL_ERR:
      printf("Error: ODMI_INTERNAL_ERR - panic\n");
      break;
    case ODMI_INVALID_CLXN:
      printf("Error: ODMI_INVALID_CLXN - invalid collection\n");
      break;
    case ODMI_INVALID_PATH:
      printf("Error: ODMI_INVALID_PATH - invalid path - what path?\n");
      break;
    case ODMI_MAGICNO_ERR:
      printf("Error: ODMI_MAGICNO_ERR - invalid object magic\n");
      break;
    case ODMI_MALLOC_ERR:
      printf("Error: ODMI_MALLOC_ERR - malloc failed\n");
      break;
    case ODMI_OPEN_ERR:
      printf("Error: ODMI_OPEN_ERR - cannot open class\n");
      break;
    case ODMI_TOOMANYCLASSES:
      printf("Error: ODMI_TOOMANYCLASSES - too many classes\n");
      break;
    default:
      printf("Unknown error!\n");
    }
    return (rc);
  }

  /* close the database again */
  odm_close_class(retClass);

  /* forget about ODM all together */
  odm_terminate();

#ifdef TRACE
  printf("TRACE: leaving x25local_nua\n");
#endif /* TRACE */

  debug(F110, "x25local_nua local address is ", buf, 0);
  return (rc);
}

/*****************************************************************************
 * Function: x25facilities
 *
 * Description:
 *      build up the facilities data packet for a connection request
 *
 * Parameters:
 *      a pre-allocated char buffer, normally NPI_MAX_DATA big.
 *
 * Return Value:
 *      the number of characters inserted into the buffer
 */
int x25facilities(buffer)
char *buffer;
{
  extern int revcall;
  extern int closgr;
  char *p;     /* temp pointer */
  char *start; /* temp pointer */

#ifdef TRACE
  printf("TRACE: entering x25facilities\n");
#endif /* TRACE */

  p = buffer + 1;
  start = p;

#ifdef DEBUG
  printf("kermit: x25facilities(): getting X25 facilities\n");
#endif /* DEBUG */

  if (revcall != 0) {
#ifdef DEBUG
    printf("reverse charge: %d\n", revcall);
#endif /* DEBUG */
    *++p = 0x01;
    *++p = revcall;
  }
  if (closgr > 0) {
#ifdef DEBUG
    printf("closed user group: %d\n", closgr);
#endif /* DEBUG */
    *++p = 0x03;
    *++p = closgr;
  }

#ifdef DEBUG
  if (p == start) {
    printf("no facilities\n");
  }
#endif /* DEBUG */

  /* set the size of the facilities buffer */
  *buffer = (char)(p - start) & 0xff;

#ifdef DEBUG
  printf("kermit: x25facilities(): returning %d\n", (int)(p - buffer));
#endif /* DEBUG */

#ifdef TRACE
  printf("TRACE: leaving x25facilities\n");
#endif /* TRACE */

  /* return the size of the facilities with size byte */
  /* 1 == no facilities, 0 byte returned as facilities size */
  return ((int)(p - buffer));
}

/*
 * reset the connection
 */
int x25reset(cause, diagn)
char cause;
char diagn;
{
  /* not implemented */

#ifdef TRACE
  printf("TRACE: entering x25reset\n");
#endif /* TRACE */

#ifdef TRACE
  printf("TRACE: leaving x25reset\n");
#endif /* TRACE */

  return (0);
}

/*
 * clear the x25 connection - ie: hang up
 */
int x25clear() {
  int get_flags = 0;          /* priority flag for getmsg */
  int put_flags = 0;          /* send flags, always 0 */
  ulong type;                 /* primitive type */
  N_discon_req_t *discon_req; /* pointer to N_DISCON_REQ */
  N_discon_ind_t *discon_ind; /* pointer to N_DISCON_IND */
  N_npi_data_t *discon_data;  /* pointer to N_DISCON_IND data */
  int rc = 0;                 /* return code */

#ifdef TRACE
  printf("TRACE: entering x25clear\n");
#endif /* TRACE */

#ifdef DEBUG
  /* printf( "x25clear(): checking last msg: %s\n", x25prim(x25lastmsg)); */
#endif /* DEBUG */

  /*
   * The following checks are used to ensure that we don't disconnect
   * or unbind twice - this seems to throw the NPI interface right out of
   * kilter.
   */
  switch (x25lastmsg) {
  case N_BIND_ACK:
  case N_CONN_CON:
  case N_CONN_REQ:
  case N_DATA_REQ:
  case N_DATA_IND: {
#ifdef DEBUG
    /* printf("x25clear(): actively disconnecting\n"); */
#endif /* DEBUG */

    discon_req = (N_discon_req_t *)malloc(NPI_MAX_CTL);
    if (discon_req == NULL) {
      perror("kermit x25clear(): discon_req malloc failed\n");
      /* fallthrough, try to unbind the NPI anyway */
    } else {
      discon_req->PRIM_type = N_DISCON_REQ;
      discon_req->DISCON_reason = 0; /* not used by AIX */
      discon_req->RES_length = 0;
      discon_req->RES_offset = (ulong)(sizeof(N_discon_req_t));
      discon_req->SEQ_number = x25seqno; /* global */

      if (x25putmsg(ttyfd, (N_npi_ctl_t *)discon_req, (N_npi_data_t *)NULL, 0,
                    &put_flags) < 0) {
        perror("x25putmsg failed in x25clear()");
      }
      discon_ind = (N_discon_ind_t *)malloc(NPI_MAX_CTL);
      discon_data = (N_npi_data_t *)malloc(NPI_MAX_DATA);
      if ((discon_ind == NULL) || (discon_data == NULL)) {
        perror("x25clear(): discon_ind malloc failed\n");
        /* fallthrough, try to unbind the NPI anyway */
      } else {
        if (x25getmsg(ttyfd, (N_npi_ctl_t *)discon_ind, NPI_MAX_CTL,
                      (N_npi_data_t *)discon_data, NPI_MAX_DATA, &get_flags,
                      N_OK_ACK) < 0) {
          perror("x25getmsg failed in x25clear()");
          /* fallthrough, try to unbind the NPI anyway */
        }
      }
    }
    break;
  }
  }

  if (x25lastmsg != N_UNBIND_REQ) {
    rc = x25unbind(ttyfd);
  }

#ifdef TRACE
  printf("TRACE: leaving x25clear\n");
#endif /* TRACE */

  return (rc);
}

#ifdef DEBUG
/*
 * only for debugging
 *
 * turn the internal representation of a datablock into something
 * half-way readable. Because the length is known, we can print
 * the string including null's etc (important, because the first(!)
 * byte of an X121 address is a null! (X121 addr == 0 + X25 addr)
 */
x25dump_data(char *addr, ulong offset, ulong length) {
  char *ptr = addr + offset;
  ulong i = length;
  /* allocate enough memory for all unprintable chars */
  char *buf = (char *)malloc(length * 4);
  char *bptr = buf; /* pointer to current place in the print buffer */

  while (i > 0) {
    if (isprint(*ptr)) {
      *bptr++ = *ptr;
    } else {
      *bptr++ = '[';
      strcpy(bptr, ckctox(*ptr, 1));
      bptr += 2;
      *bptr++ = ']';
    }
    ptr++;
    i--;
  }
  if (length > 0) {
    *bptr = '\0';
    printf("%s", buf);
  }
  printf(" (%d+%d)\n", offset, length);

  if (buf) {
    free(buf);
    buf = NULL;
  }
  return;
}

/*
 * only for debugging
 * print as much useful information about a packet as possible
 */
x25dump_prim(primitive) N_npi_ctl_t *primitive;
{
  printf("Primitive");
  switch (primitive->PRIM_type) {
  case N_BIND_ACK:
    printf("\tN_BIND_ACK\n\taddress:\t");
    x25dump_data((char *)primitive, primitive->bind_ack.ADDR_offset,
                 primitive->bind_ack.ADDR_length);
    printf("\tproto id:\t");
    x25dump_data((char *)primitive, primitive->bind_ack.PROTOID_offset,
                 primitive->bind_ack.PROTOID_length);
    printf("\tconnind:\t%d\n\ttoken:\t\t%d\n",
           primitive->bind_ack.CONIND_number, primitive->bind_ack.TOKEN_value);
    break;

  case N_BIND_REQ:
    printf("\tN_BIND_REQ\n\taddress:\t");
    x25dump_data((char *)primitive, primitive->bind_req.ADDR_offset,
                 primitive->bind_req.ADDR_length);
    printf("\tproto id:\t");
    x25dump_data((char *)primitive, primitive->bind_req.PROTOID_offset,
                 primitive->bind_req.PROTOID_length);
    printf("\tconnind:\t%d\n\tflags:\t\t%d\n",
           primitive->bind_req.CONIND_number, primitive->bind_req.BIND_flags);
    break;

  case N_CONN_CON:
    printf("\tN_CONN_CON\n");
    printf("\tRES\t\t");
    x25dump_data((char *)primitive, primitive->conn_con.RES_offset,
                 primitive->conn_con.RES_length);
    printf("\tflags:\t%d\n", primitive->conn_con.CONN_flags);
    break;

  case N_CONN_IND:
    printf("\tN_CONN_IND\n");
    printf("\tsource:\t\t");
    x25dump_data((char *)primitive, primitive->conn_ind.SRC_offset,
                 primitive->conn_ind.SRC_length);
    printf("\tdestination:\t");
    x25dump_data((char *)primitive, primitive->conn_ind.DEST_offset,
                 primitive->conn_ind.DEST_length);
    printf("\tSEQ_number:\t%d\n", primitive->conn_ind.SEQ_number);
    printf("\tflags:\t%d\n", primitive->conn_ind.CONN_flags);
    break;

  case N_CONN_REQ:
    printf("\tN_CONN_REQ\n\tdestination:\t");
    x25dump_data((char *)primitive, primitive->conn_req.DEST_offset,
                 primitive->conn_req.DEST_length);
    printf("\tflags:\t%d\n", primitive->conn_req.CONN_flags);
    break;

  case N_CONN_RES:
    printf("\tN_CONN_RES\n");
    printf("\tTOKEN_value\t%d\n", primitive->conn_res.TOKEN_value);
    printf("\tSEQ_number\t%d\n", primitive->conn_res.SEQ_number);
    printf("\tCONN_flags\t%d\n", primitive->conn_res.CONN_flags);
    printf("\tRES\t\t");
    x25dump_data((char *)primitive, primitive->conn_res.RES_offset,
                 primitive->conn_res.RES_length);
    break;

  case N_DATACK_IND:
    printf("\tN_DATACK_IND\n");
    break;

  case N_DATACK_REQ:
    printf("\tN_DATACK_REQ\n");
    printf("\tflags:\t%d\n", primitive->data_req.DATA_xfer_flags);
    break;

  case N_DATA_IND:
    printf("\tN_DATA_IND\n");
    printf("\tflags:\t%d\n", primitive->data_ind.DATA_xfer_flags);
    break;

  case N_DATA_REQ:
    printf("\tN_DATA_REQ\n");
    break;

  case N_DISCON_IND:
    printf("\tN_DISCON_IND\n");
    printf("\torigin:\t%d\n", primitive->discon_ind.DISCON_orig);
    printf("\treason:\t\t%d\n", primitive->discon_ind.DISCON_reason);
    printf("\tseq no:\t\t%d\n", primitive->discon_ind.SEQ_number);
    printf("\tRES:\t");
    x25dump_data((char *)primitive, primitive->discon_ind.RES_offset,
                 primitive->discon_ind.RES_length);
    break;

  case N_DISCON_REQ:
    printf("\tN_DISCON_REQ\n");
    printf("\tDISCON_reason:\t%d\n", primitive->discon_req.DISCON_reason);
    printf("\tRES:\t");
    x25dump_data((char *)primitive, primitive->discon_req.RES_offset,
                 primitive->discon_req.RES_length);
    printf("\tSEQ_number:\t%d\n", primitive->discon_req.SEQ_number);
    break;

  case N_ERROR_ACK:
    printf("\tN_ERROR_ACK\n");
    printf("\tCaused by:\t%s\n", x25prim(primitive->error_ack.ERROR_prim));
    printf("\tNPI error:\t%s\n", x25err(primitive->error_ack.NPI_error));
    errno = primitive->error_ack.UNIX_error;
    perror("\t");
    break;

  case N_EXDATA_IND:
    printf("\tN_EXDATA_ACK\n");
    break;

  case N_EXDATA_REQ:
    printf("\tN_EXDATA_REQ\n");
    break;

  case N_INFO_ACK:
    printf("\tN_INFO_ACK\n");
    printf("\tNSDU size:\t%d\n", primitive->info_ack.NSDU_size);
    printf("\tENSDU size:\t%d\n", primitive->info_ack.ENSDU_size);
    printf("\tCDATA size:\t%d\n", primitive->info_ack.CDATA_size);
    printf("\tDDATA size:\t%d\n", primitive->info_ack.DDATA_size);
    printf("\tADDR size:\t%d\n", primitive->info_ack.ADDR_size);
    printf("\tNIDU size:\t%d\n", primitive->info_ack.NIDU_size);
    break;

  case N_INFO_REQ:
    printf("\tN_INFO_REQ\n");
    break;

  case N_OK_ACK:
    printf("\tN_OK_ACK\n");
    break;

  case N_OPTMGMT_REQ:
    printf("\tN_OPTMGMT_REQ\n");
    break;

  case N_RESET_CON:
    printf("\tN_RESET_CON\n");
    break;

  case N_RESET_IND:
    printf("\tN_RESET_IND\n");
    printf("\treason:\t\t%d\n", primitive->reset_ind.RESET_reason);
    printf("\torigin:\t\t%d\n", primitive->reset_ind.RESET_orig);
    break;

  case N_RESET_REQ:
    printf("\tN_RESET_REQ\n");
    printf("\treason:\t\t%d\n", primitive->reset_req.RESET_reason);
    break;

  case N_RESET_RES:
    printf("\tN_RESET_RES\n");
    break;

  case N_UDERROR_IND:
    printf("\tN_UDERROR_IND\n");
    break;

  case N_UNBIND_REQ:
    printf("\tN_UNBIND_REQ\n");
    break;

  case N_UNITDATA_REQ:
    printf("\tN_UNITDATA_REQ\n");
    break;

  case N_UNITDATA_IND:
    printf("\tN_UNITDATA_IND\n");
    break;

  default:
    (void)printf("Unknown NPI error %d", primitive->PRIM_type);
    return 0;
  }
}
#endif /* DEBUG */

/* it looks like signal handling is not needed with streams! */
/* x25oobh()    - handle SIGURG signals - take from isode ? */

#endif /* IBMX25 */

#ifndef NOHTTP
/*
  Which time.h files to include... See ckcdeb.h for defaults.
  Note that 0, 1, 2, or all 3 of these can be included according to
  the symbol definitions.
*/
#ifndef NOTIMEH
#ifdef TIMEH
#include <time.h>
#endif /* TIMEH */
#endif /* NOTIMEH */

#ifndef NOSYSTIMEH
#ifdef SYSTIMEH
#include <sys/time.h>
#endif /* SYSTIMEH */
#endif /* NOSYSTIMEH */

#ifndef NOSYSTIMEBH
#ifdef SYSTIMEBH
#include <sys/timeb.h>
#endif /* SYSTIMEBH */
#endif /* NOSYSTIMEBH */

#ifndef TIMEH
#ifndef SYSTIMEH
#ifndef SYSTIMEBH
#ifdef SYSTIMEH
#include <sys/time.h>
#else
#ifdef POSIX
#include <posix/time.h>
#else
#ifdef CLIX
#include <sys/time.h>
#else
#include <time.h>
/* #include <utime.h> */
#endif /* CLIX */
#endif /* POSIX */
#endif /* SYSTIMEH */
#endif
#endif
#endif

#ifdef SYSUTIMEH       /* <sys/utime.h> if requested,  */
#include <sys/utime.h> /* for extra fields required by */
#else                  /* 88Open spec. */
#ifdef UTIMEH          /* or <utime.h> if requested */
#include <utime.h>     /* (SVR4, POSIX) */
#define SYSUTIMEH      /* Use this for both cases. */
#endif                 /* UTIMEH */
#endif                 /* SYSUTIMEH */

#ifndef HTTP_VERSION
#define HTTP_VERSION "HTTP/1.1"
#endif /* HTTP_VERSION */

#ifdef CMDATE2TM
time_t http_date(char *date)
/* http_date */ {
  /* HTTP dates are of the form:  "Sun, 06 Oct 1997 20:11:47 GMT" */
  /* There are two older formats which we are required to parse
   * that we currently do not:
   *
   * RFC 850:   "Sunday, 06-Oct-97 20:11:47 GMT"
   * asctime(): "Sun Nov  6 20:11:47 1997"
   *
   * However, it is required that all dates be sent in the form we
   * do accept.  The other two formats are for compatibility with
   * really old servers.
   */
  extern char cmdatebuf[18];
  struct tm t_tm;
  time_t t;
  char ldate[32];
  int j;

  j = ckindex(",", date, 0, 0, 0);
  ckstrncpy(ldate, &date[j + 1], 25);

  { /*
       cmcvtate() date changed to return a string pointer.
       fdc, 12 Aug 2001.
    */
    char *dp;
    dp = (char *)cmcvtdate(ldate, 0); /* Convert to normal form */
    if (!dp)
      return (0);
    t_tm = *cmdate2tm(dp, 1);
  }
/*
  From Lucas Hart, 5 Dec 2001:
  "On the systems to which I have access (SunOS 4.1.1, Solaris 8, and Tru64),
  setting tm_isdst to -1 maintains the correct timezone offsets, i.e., writes
  the specified (GMT) time if the buffer size is 21, or the contemporaneous
  localtime if the buffer size is 25.  Perhaps tm_isdst should be set in
  cmdate2tm(), rather than only in http_date."
*/
#ifndef NOTM_ISDST    /* For platforms where */
  t_tm.tm_isdst = -1; /* tm_isdst doesn't exist. */
#endif                /* NOTM_ISDST */

  t = mktime(&t_tm); /* NOT PORTABLE */

#ifdef XX_TIMEZONE
  t -= _timezone;
#endif /* XX_TIMEZONE */

  return (t);
}
#endif /* CMDATE2TM */

char *http_now() {
  static char nowstr[32];
#ifdef CMDATE2TM
  struct tm *gmt;
  time_t ltime; /* NOT PORTABLE */

  time(&ltime);

  gmt = gmtime(&ltime); /* PROBABLY NOT PORTABLE */
  strftime(nowstr, 32, "%a, %d %b %Y %H:%M:%S GMT", gmt); /* NOT PORTABLE */
  /* not only is it not portable but it's locale-dependent */
#else
  /*
    This is hopeless.  First of all, it seems that HTTP wants Day and Month
    NAMES?  In English?  Whose idea was that?  Even worse, the date/time must be
    expressed in Zulu (UTC (GMT)), and converting from local time to GMT is a
    nightmare.  Every platform does it differently, if at all -- even if we
    restrict ourselves to UNIX.  For example (quoting from recent C-Kermit edit
    history), "Fixed a longstanding bug in the BSDI version, in which incoming
    file dates were set in GMT rather than local time.  It seems in 4.4BSD,
    localtime() does not return the local time, but rather Zero Meridian (Zulu)
    time (GMT), and must be adjusted by the tm_gmtoff value."  Swell.  For
    greater appreciation of the scope of the problem, just take a look at the
    time-related #ifdefs in ckutio.c.  The only right way to do this is to add
    our own portable API for converting between local time and GMT/UTC/Zulu
    that shields us not only from UNIXisms like time_t and struct tm, but also
    the unbelievable amount of differences in time-related APIs -- e.g. is
    "timezone" an external variable or a function; which header file(s) do we
    include, etc etc etc.  It's a major project.
  */
  int x;
  x = cmcvtdate("", 1);

  Evidently this code is not used-- if it is,
      it must be fixed to use new (aug 2001) cmcvtdate() calling conventions.

      if (x < 0) return ("");
  /*  yyyymmdd hh:mm:ss */
  /*  01234567890123456 */
  nowstr[0] = 'X'; /* 1st letter of day */
  nowstr[1] = 'x'; /* 2nd letter of day */
  nowstr[2] = 'x'; /* 3rd letter of day */
  nowstr[3] = ',';
  nowstr[4] = ' ';
  nowstr[5] = cmdate[6];
  nowstr[6] = cmdate[7];
  nowstr[7] = ' ';
  nowstr[8] = ' ';  /* first letter of month */
  nowstr[9] = ' ';  /* second letter of month */
  nowstr[10] = ' '; /* third letter of month */
  nowstr[11] = ' ';
  nowstr[12] = cmdate[0];
  nowstr[13] = cmdate[1];
  nowstr[14] = cmdate[2];
  nowstr[15] = cmdate[3];
  nowstr[16] = ' ';
  nowstr[17] = cmdate[9];
  nowstr[18] = cmdate[10];
  nowstr[19] = cmdate[11];
  nowstr[20] = cmdate[12];
  nowstr[21] = cmdate[13];
  nowstr[22] = cmdate[14];
  nowstr[23] = cmdate[15];
  nowstr[24] = cmdate[16];
  nowstr[25] = ' ';
  nowstr[26] = 'G';
  nowstr[27] = 'M';
  nowstr[28] = 'T';
  nowstr[29] = '\0';
#endif /* CMDATE2TM */
  return (nowstr);
}

/* from ckuusr.h, which this module normally doesn't include */
int dclarray(char, int);
/*
  Assign http response pairs to given array.
  For best results, response pairs should contain no spaces.

  Call with:
    resp  =  pointer to response list.
    n     =  size of response list.
    array =  array letter.
  Returns:
    0 on failure.
    >= 1, size of array, on success.
*/
static int http_mkarray(char **resp, int n, char array) {
#ifndef NOSPL
  int i, x;
  char **ap;
  extern char **a_ptr[];
  extern int a_dim[];

  if (!array || n <= 0)
    return (0);
  if ((x = dclarray(array, n)) < 0) {
    printf("?Array declaration failure\n");
    return (-9);
  }
  /* Note: argument array is 0-based but Kermit array is 1-based */
  ap = a_ptr[x];
  ap[0] = NULL; /* 0th element is empty */
  for (i = 1; i <= n; i++) {
    ap[i] = resp[i - 1]; /* If resp elements were malloc'd */
    resp[i - 1] = NULL;
  }
  a_dim[x] = n;
  return (n);
#else
  return (0);
#endif /* NOSPL */
}

#define HTTPHEADCNT 64
int http_get_chunk_len() {
  int len = 0;
  int i = 0, j = -1;
  char buf[24];
  int ch;

  while ((ch = http_inc(0)) >= 0 && i < 24) {
    buf[i] = ch;
    if (buf[i] == ';') /* Find chunk-extension (if any) */
      j = i;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      buf[i] = '\0';
      break;
    }
    i++;
  }
  if (i < 24) { /* buf now contains len in Hex */
    len = hextoulong(buf, j == -1 ? i : j - 1);
  }

  return (len);
}

int http_isconnected() { return (httpfd != -1); }

char *http_host() { return (httpfd != -1 ? http_host_port : ""); }

char *http_security() {
  if (httpfd == -1)
    return ("NULL");
  return ("NULL");
}

int http_reopen() {
  int rc = 0;
  char *s = NULL; /* strdup is not portable */
  if (tcp_http_proxy) {
    char *p;
    makestr(&s, (char *)http_host_port);
    p = s;
    while (*p != '\0' && *p != ':')
      p++;           /* Look for colon */
    if (*p == ':') { /* Have a colon */
      *p++ = '\0';   /* Get service name or number */
    } else {
      p = "http";
    }
    rc = http_open(s, p, http_ssl, NULL, 0, http_agent);
  } else {
    makestr(&s, (char *)http_ip);
    rc = http_open(s, ckuitoa(http_port), http_ssl, NULL, 0, http_agent);
  }
  free(s);
  return (rc);
}

int http_open(char *hostname, char *svcname, int use_ssl, char *rdns_name,
              int rdns_len, char *agent) {
  char namecopy[NAMECPYL];
  char *p;
  int i, dns = 0;
#ifdef NON_BLOCK_IO
  int x;
#endif /* NON_BLOCK_IO */
#ifdef TCPSOCKET
  int isconnect = 0;
#ifdef SO_OOBINLINE
  int on = 1;
#endif /* SO_OOBINLINE */
  struct servent *service = NULL;
  struct hostent *host = NULL;
  struct sockaddr_in r_addr;
  struct sockaddr_in sin;
  struct sockaddr_in l_addr;
  GSOCKNAME_T l_slen;

#ifdef INADDRX
  /* inet_addr() is of type struct in_addr */
  struct in_addr iax;
#else
#ifdef INADDR_NONE
  struct in_addr iax;
#else  /* INADDR_NONE */
  long iax;
#endif /* INADDR_NONE */
#endif /* INADDRX */

  if (rdns_name == NULL || rdns_len < 0)
    rdns_len = 0;

  *http_ip = '\0'; /* Initialize IP address string */
  namecopy[0] = '\0';

#ifdef DEBUG
  if (deblog) {
    debug(F110, "http_open hostname", hostname, 0);
    debug(F110, "http_open svcname", svcname, 0);
  }
#endif /* DEBUG */
  if (!hostname)
    hostname = "";
  if (!svcname)
    svcname = "";
  if (!*hostname || !*svcname)
    return (-1);

  service = ckgetservice(hostname, svcname, http_ip, 20);

  if (service == NULL) {
    if (!quiet)
      printf("?Invalid service: %s\r\n", svcname);
    return (-1);
  }

  /* For HTTP connections we must preserve the original hostname and */
  /* service requested so we can include them in the Host header.    */
  ckmakmsg(http_host_port, sizeof(http_host_port), hostname, ":",
           ckuitoa(ntohs(service->s_port)), NULL);
  http_port = ntohs(service->s_port);
  http_ssl = use_ssl;
  debug(F111, "http_open", http_host_port, http_port);

  /* 'http_ip' contains the IP address to which we want to connect        */
  /* 'svcnam'   contains the service name                                 */
  /* 'service->s_port' contains the port number in network byte order     */

  /* If we are using an http proxy, we need to create a buffer containing */
  /*   hostname:port-number                                               */
  /* to pass to the http_connect() function.  Then we need to replace     */
  /* 'namecopy' with the name of the proxy server and the service->s_port */
  /* with the port number of the proxy (default port 80).                 */

  if (tcp_http_proxy) {

    ckmakmsg(proxycopy, sizeof(proxycopy), hostname, ":",
             ckuitoa(ntohs(service->s_port)), NULL);
    ckstrncpy(namecopy, tcp_http_proxy, NAMECPYL);

    p = namecopy; /* Was a service requested? */
    while (*p != '\0' && *p != ':')
      p++;           /* Look for colon */
    if (*p == ':') { /* Have a colon */
      debug(F110, "http_open name has colon", namecopy, 0);
      *p++ = '\0'; /* Get service name or number */
    } else {
      strcpy(++p, "http");
    }

    service = ckgetservice(namecopy, p, http_ip, 20);
    if (!service) {
      fprintf(stderr, "Can't find port for service %s\n", p);
      debug(F101, "http_open can't get service for proxy", "", errno);
      errno = 0; /* (rather than mislead) */
      return (-1);
    }

    /* copy the proxyname and remove the service if any so we can use
     * it as the hostname
     */
    ckstrncpy(namecopy, tcp_http_proxy, NAMECPYL);
    p = namecopy; /* Was a service requested? */
    while (*p != '\0' && *p != ':')
      p++;           /* Look for colon */
    if (*p == ':') { /* Have a colon */
      *p = '\0';     /* terminate string */
    }
    hostname = namecopy; /* use proxy as hostname */
  }

  /* Set up socket structure and get host address */
  bzero((char *)&r_addr, sizeof(r_addr));
  debug(F100, "http_open bzero ok", "", 0);

#ifdef INADDR_NONE
  debug(F101, "http_open INADDR_NONE defined", "", INADDR_NONE);
#else  /* INADDR_NONE */
  debug(F100, "http_open INADDR_NONE not defined", "", 0);
#endif /* INADDR_NONE */
#ifdef INADDRX
  debug(F100, "http_open INADDRX defined", "", 0);
#else  /* INADDRX */
  debug(F100, "http_open INADDRX not defined", "", 0);
#endif /* INADDRX */

#ifndef NOMHHOST
#ifdef INADDRX
  iax = inet_addr(http_ip[0] ? http_ip : hostname);
  debug(F111, "http_open inet_addr", http_ip[0] ? http_ip : hostname,
        iax.s_addr);
#else /* INADDRX */
#ifdef INADDR_NONE
  iax.s_addr = inet_addr(http_ip[0] ? http_ip : hostname);
  debug(F111, "http_open inet_addr", http_ip[0] ? http_ip : hostname,
        iax.s_addr);
#else  /* INADDR_NONE */
  iax = (unsigned int)inet_addr(http_ip[0] ? http_ip : hostname);
  debug(F111, "http_open inet_addr", http_ip[0] ? http_ip : hostname, iax);
#endif /* INADDR_NONE */
#endif /* INADDRX */

  dns = 0;
  if (
#ifdef INADDR_NONE
      /* This might give warnings on 64-bit platforms but they should be
         harmless */
      /* because INADDR_NONE should be all 1's anyway, thus the OR part is */
      /* probably superfluous -- not sure why it's even there, maybe it should
         be */
      /* removed. */
      iax.s_addr == INADDR_NONE /* || iax.s_addr == (unsigned long) -1L */
#else                           /* INADDR_NONE */
      iax == -1
#endif                          /* INADDR_NONE */
  ) {
    if (!quiet) {
      printf(" DNS Lookup... ");
      fflush(stdout);
    }
    if ((host = gethostbyname(http_ip[0] ? http_ip : hostname)) != NULL) {
      debug(F100, "http_open gethostbyname != NULL", "", 0);
      host = ck_copyhostent(host);
      dns = 1; /* Remember we performed dns lookup */
      r_addr.sin_family = host->h_addrtype;
      if (tcp_rdns && host->h_name && host->h_name[0] && (rdns_len > 0) &&
          (tcp_http_proxy == NULL))
        ckmakmsg(rdns_name, rdns_len, host->h_name, ":", svcname, NULL);

#ifdef HADDRLIST
#ifdef h_addr
      /* This is for trying multiple IP addresses - see <netdb.h> */
      if (!(host->h_addr_list))
        return (-1);
      bcopy(host->h_addr_list[0], (caddr_t)&r_addr.sin_addr, host->h_length);
#else
      bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* h_addr */
#else  /* HADDRLIST */
      bcopy(host->h_addr, (caddr_t)&r_addr.sin_addr, host->h_length);
#endif /* HADDRLIST */
      debug(F111, "BCOPY", "host->h_length", host->h_length);
    }
  }
#endif /* NOMHHOST */

  if (!dns) {
#ifdef INADDRX
    /* inet_addr() is of type struct in_addr */
    struct in_addr ina;
    unsigned long uu;
    debug(F100, "http_open gethostbyname == NULL: INADDRX", "", 0);
    ina = inet_addr(http_ip[0] ? http_ip : hostname);
    uu = *(unsigned int *)&ina;
#else  /* Not INADDRX */
    /* inet_addr() is unsigned long */
    unsigned long uu;
    debug(F100, "http_open gethostbyname == NULL: Not INADDRX", "", 0);
    uu = inet_addr(http_ip[0] ? http_ip : hostname);
#endif /* INADDRX */
    debug(F101, "http_open uu", "", uu);
    if (
#ifdef INADDR_NONE
        !(uu == INADDR_NONE || uu == (unsigned int)-1L)
#else  /* INADDR_NONE */
        uu != ((unsigned long)-1)
#endif /* INADDR_NONE */
    ) {
      r_addr.sin_addr.s_addr = uu;
      r_addr.sin_family = AF_INET;
    } else {
      fprintf(stderr, "Can't get address for %s\n",
              http_ip[0] ? http_ip : hostname);
      debug(F101, "http_open can't get address", "", errno);
      errno = 0; /* Rather than mislead */
      return (-1);
    }
  }

  /* Get a file descriptor for the connection. */

  r_addr.sin_port = service->s_port;
  ckstrncpy(http_ip, (char *)inet_ntoa(r_addr.sin_addr), 20);
  debug(F110, "http_open trying", http_ip, 0);
  if (!quiet && *http_ip) {
    printf(" Trying %s... ", http_ip);
    fflush(stdout);
  }

  /* Loop to try additional IP addresses, if any. */

  do {
    if ((httpfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("TCP socket error");
      debug(F101, "http_open socket error", "", errno);
      return (-1);
    }
    errno = 0;

    /* If a specific TCP address on the local host is desired we */
    /* must bind it to the socket.                               */
    if (tcp_address) {
      int s_errno;

      debug(F110, "http_open binding socket to", tcp_address, 0);
      bzero((char *)&sin, sizeof(sin));
      sin.sin_family = AF_INET;
#ifdef INADDRX
      inaddrx = inet_addr(tcp_address);
      sin.sin_addr.s_addr = *(unsigned long *)&inaddrx;
#else
      sin.sin_addr.s_addr = inet_addr(tcp_address);
#endif /* INADDRX */
      sin.sin_port = 0;
      if (bind(httpfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        s_errno = socket_errno; /* Save error code */
#ifdef TCPIPLIB
        socket_close(httpfd);
#else  /* TCPIPLIB */
        close(httpfd);
#endif /* TCPIPLIB */
        httpfd = -1;
        errno = s_errno; /* and report this error */
        debug(F101, "http_open bind errno", "", errno);
        return (-1);
      }
    }

    /* Now connect to the socket on the other end. */

    if (connect(httpfd, (struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) {
      i = errno; /* Save error code */
#ifdef HADDRLIST
#ifdef h_addr
      if (host && host->h_addr_list && host->h_addr_list[1]) {
        perror("");
        host->h_addr_list++;
        bcopy(host->h_addr_list[0], (caddr_t)&r_addr.sin_addr, host->h_length);

        ckstrncpy(http_ip, (char *)inet_ntoa(r_addr.sin_addr), 20);
        debug(F110, "http_open h_addr_list", http_ip, 0);
        if (!quiet && *http_ip) {
          printf(" Trying %s... ", http_ip);
          fflush(stdout);
        }
#ifdef TCPIPLIB
        socket_close(httpfd); /* Close it. */
#else
        close(httpfd);
#endif /* TCPIPLIB */
        continue;
      }
#endif /* h_addr */
#endif /* HADDRLIST */
      http_close();
      httpfd = -1;
      errno = i; /* And report this error */
      debug(F101, "http_open connect errno", "", errno);
      if (!quiet)
        perror("Failed");
      return (-1);
    }
    isconnect = 1;
  } while (!isconnect);

#ifdef NON_BLOCK_IO
  on = 1;
  x = socket_ioctl(httpfd, FIONBIO, &on);
  debug(F101, "http_open FIONBIO", "", x);
#endif /* NON_BLOCK_IO */

  /* We have succeeded in connecting to the HTTP PROXY.  So now we   */
  /* need to attempt to connect through the proxy to the actual host */
  /* If that is successful, we have to pretend that we made a direct */
  /* connection to the actual host.                                  */

  if (tcp_http_proxy) {
    if (!agent)
      agent = "C-Kermit";

    if (http_connect(
            httpfd, tcp_http_proxy_agent ? tcp_http_proxy_agent : agent, NULL,
            tcp_http_proxy_user, tcp_http_proxy_pwd, 0, proxycopy) < 0) {
      http_close();
      return (-1);
    }
  }

#ifdef SO_OOBINLINE
  /* See note on SO_OOBINLINE in netopen() */
#ifdef POSIX
  setsockopt(httpfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);
#else
#ifdef CLIX
  setsockopt(httpfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);
#else
  setsockopt(httpfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif /* CLIX */
#endif /* POSIX */
#endif /* SO_OOBINLINE */

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
  no_delay(ttyfd, tcp_nodelay);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
  keepalive(ttyfd, tcp_keepalive);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
  ck_linger(ttyfd, tcp_linger, tcp_linger_tmo);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
  sendbuf(ttyfd, tcp_sendbuf);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
  recvbuf(ttyfd, tcp_recvbuf);
#endif /* SO_RCVBUF */
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

  /* Find out our own IP address. */
  /* We need the l_addr structure for [E]KLOGIN. */
  l_slen = sizeof(l_addr);
  bzero((char *)&l_addr, l_slen);
  if (!getsockname(httpfd, (struct sockaddr *)&l_addr, &l_slen)) {
    char *s = (char *)inet_ntoa(l_addr.sin_addr);
    ckstrncpy(myipaddr, s, 20);
    debug(F110, "getsockname", myipaddr, 0);
  }

  /* See note in netopen() on Reverse DNS lookups */
  if (tcp_rdns == SET_ON) {
    if (!quiet) {
      printf(" Reverse DNS Lookup... ");
      fflush(stdout);
    }
    if ((host = gethostbyaddr((char *)&r_addr.sin_addr, 4, PF_INET))) {
      char *s;
      host = ck_copyhostent(host);
      debug(F100, "http_open gethostbyname != NULL", "", 0);
      if (!quiet) {
        printf("(OK)\n");
        fflush(stdout);
      }
      s = host->h_name;
      if (!s) { /* This can happen... */
        debug(F100, "http_open host->h_name is NULL", "", 0);
        s = "";
      }
      /* Something is wrong with inet_ntoa() on HPUX 10.xx */
      /* The compiler says "Integral value implicitly converted to */
      /* pointer in assignment."  The prototype is right there */
      /* in <arpa/inet.h> so what's the problem? */
      /* Ditto in HP-UX 5.x, but not 8.x or 9.x... */
      if (!*s) { /* No name so substitute the address */
        debug(F100, "http_open host->h_name is empty", "", 0);
        s = inet_ntoa(r_addr.sin_addr); /* Convert address to string */
        if (!s)                         /* Trust No 1 */
          s = "";
        if (*s) { /* If it worked, use this string */
          ckstrncpy(http_ip, s, 20);
        }
        s = http_ip;          /* Otherwise stick with the IP */
        if (!*s)              /* or failing that */
          s = http_host_port; /* the name we were called with. */
      }
      if (*s) /* return the rdns name */
        ckmakmsg(rdns_name, rdns_len, s, ":", svcname, NULL);

      if (!quiet && *s
#ifndef NOICP
          && !doconx
#endif /* NOICP */
      ) {
        printf(" %s connected on port %s\n", s,
               ckuitoa(ntohs(service->s_port)));
#ifdef BETADEBUG
        /* This is simply for testing the DNS entries */
        if (host->h_aliases) {
          char **a = host->h_aliases;
          while (*a) {
            printf(" alias => %s\n", *a);
            a++;
          }
        }
#endif /* BETADEBUG */
      }
    } else {
      if (!quiet)
        printf("Failed.\n");
    }
  } else if (!quiet)
    printf("(OK)\n");
  if (!quiet)
    fflush(stdout);

  if (tcp_http_proxy) {
    /* Erase the IP address since we cannot reuse it */
    http_ip[0] = '\0';
  } else {
    /* This should already have been done but just in case */
    ckstrncpy(http_ip, (char *)inet_ntoa(r_addr.sin_addr), 20);
  }
  makestr(&http_agent, agent);

#endif /* TCPSOCKET */

  return (0); /* Done. */
}

int http_close(void) {
  int x = 0;
  debug(F101, "http_close", "", httpfd);

#ifdef HTTP_BUFFERING
  http_count = 0;
  http_bufp = 0;
#endif /* HTTP_BUFFERING */

  if (httpfd == -1) /* Was open? */
    return (0);     /* Wasn't. */

  if (httpfd > -1) /* Was. */
  {
#ifdef TCPIPLIB
    x = socket_close(httpfd); /* Close it. */
#else
    x = close(httpfd);
#endif /* TCPIPLIB */
  }
  httpfd = -1; /* Mark it as closed. */
  /* do not erase http_host_port, http_ip, http_port so they */
  /* can be used by http_reopen() */
  return (x);
}

/* http_tol()
 * Call with s = pointer to string, n = length.
 * Returns number of bytes actually written on success, or
 * -1 on i/o error, -2 if called improperly.
 */

int http_tol(CHAR *s, int n) {
  int count = 0;
  int len = n;
  int try = 0;

  if (httpfd == -1) {
    debug(F100, "http_tol socket is closed", "", 0);
    return -1;
  }
  debug(F101, "http_tol TCPIPLIB ttnet", "", ttnet);

http_tol_retry:
  try++; /* Increase the try counter */

  {
#ifdef BSDSELECT
    fd_set wfds;
    struct timeval tv;

    debug(F101, "http_tol BSDSELECT", "", 0);
    tv.tv_usec = 0L;
    tv.tv_sec = 30;
#ifdef STREAMING
  do_select:
#endif /* STREAMING */
    FD_ZERO(&wfds);
    FD_SET(httpfd, &wfds);
    if (select(FD_SETSIZE, NULL,
#ifdef __DECC
#ifndef __DECC_VER
               (int *)
#endif /* __DECC_VER */
#endif /* __DECC */
               &wfds,
               NULL, &tv) < 0) {
      int s_errno = socket_errno;
      debug(F101, "http_tol select failed", "", s_errno);
#ifdef BETADEBUG
      printf("http_tol select failed: %d\n", s_errno);
#endif /* BETADEBUG */
      return (-1);
    }
    if (!FD_ISSET(httpfd, &wfds)) {
#ifdef STREAMING
      if (streaming)
        goto do_select;
#endif /* STREAMING */
      debug(F111, "http_tol", "!FD_ISSET", ttyfd);
      return (-1);
    }
#else /* BSDSELECT */
#ifdef IBMSELECT
    {
      int tries = 0;
      debug(F101, "http_tol IBMSELECT", "", 0);
      while (select(&httpfd, 0, 1, 0, 1000) != 1) {
        int count;
        if (tries++ >= 60) {
          /* if after 60 seconds we can't get permission to write */
          debug(F101, "http_tol select failed", "", socket_errno);
          return (-1);
        }
      }
    }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
#ifdef TCPIPLIB
    if ((count = socket_write(httpfd, s, n)) < 0) {
      int s_errno = socket_errno; /* maybe a function */
      debug(F101, "http_tol socket_write error", "", s_errno);
      return (-1); /* Call it an i/o error */
    }
#else  /* TCPIPLIB */
    if ((count = write(httpfd, s, n)) < 0) {
      debug(F101, "http_tol socket_write error", "", errno);
      return (-1); /* Call it an i/o error */
    }
#endif /* TCPIPLIB */
    if (count < n) {
      debug(F111, "http_tol socket_write", s, count);
      if (try > 25) {
        /* don't try more than 25 times */
        debug(F100, "http_tol tried more than 25 times", "", 0);
        return (-1);
      }
      if (count > 0) {
        s += count;
        n -= count;
      }
      debug(F111, "http_tol retry", s, n);
      goto http_tol_retry;
    } else {
      debug(F111, "http_tol socket_write", s, count);
      return (len); /* success - return total length */
    }
  }
}

int http_inc(int timo) {
  int x = -1;
  unsigned char c; /* The locals. */

  if (httpfd == -1) {
#ifdef HTTP_BUFFERING
    http_count = 0;
    http_bufp = 0;
#endif /* HTTP_BUFFERING */
    debug(F100, "http_inc socket is closed", "", 0);
    return (-2);
  }

#ifdef HTTP_BUFFERING
  /* Skip all the select() stuff if we have bytes buffered locally */
  if (http_count > 0)
    goto getfrombuffer;
#endif /* HTTP_BUFFERING */

  {
#ifdef BSDSELECT
    fd_set rfds;
    struct timeval tv;
    int timeout = timo < 0 ? -timo : 1000 * timo;
    debug(F101, "http_inc BSDSELECT", "", timo);

    for (; timeout >= 0; timeout -= (timo ? 100 : 0)) {
      int rc;
      debug(F111, "http_inc", "timeout", timeout);
      /* Don't move select() initialization out of the loop. */
      FD_ZERO(&rfds);
      FD_SET(httpfd, &rfds);
      tv.tv_sec = tv.tv_usec = 0L;
      if (timo)
        tv.tv_usec = (long)100000L;
      else
        tv.tv_sec = 30;
      rc = select(FD_SETSIZE,
#ifndef __DECC
                  (fd_set *)
#endif /* __DECC */
                  &rfds,
                  NULL, NULL, &tv);
      if (rc < 0) {
        int s_errno = socket_errno;
        debug(F111, "http_inc", "select", rc);
        debug(F111, "http_inc", "socket_errno", s_errno);
#ifdef HTTP_BUFFERING
        http_count = 0;
        http_bufp = 0;
#endif /* HTTP_BUFFERING */
        if (s_errno)
          return (-1);
      }
      debug(F111, "http_inc", "select", rc);
      if (FD_ISSET(httpfd, &rfds)) {
        x = 0;
        break;
      } else {
        /* If waiting forever we have no way of knowing if the */
        /* socket closed so try writing a 0-length TCP packet  */
        /* which should force an error if the socket is closed */
        if (!timo) {
#ifdef TCPIPLIB
          if ((rc = socket_write(httpfd, "", 0)) < 0) {
#ifdef HTTP_BUFFERING
            http_count = 0;
            http_bufp = 0;
#endif /* HTTP_BUFFERING */
            int s_errno = socket_errno;
            debug(F101, "http_inc socket_write error", "", s_errno);
            return (-1); /* Call it an i/o error */
          }
#else /* TCPIPLIB */
          if ((rc = write(httpfd, "", 0)) < 0) {
#ifdef HTTP_BUFFERING
            http_count = 0;
            http_bufp = 0;
#endif /* HTTP_BUFFERING */
            debug(F101, "http_inc socket_write error", "", errno);
            return (-1); /* Call it an i/o error */
          }
#endif /* TCPIPLIB */
        }
        continue;
      }
    }
#else /* !BSDSELECT */
#ifdef IBMSELECT
    /*
     Was used by OS/2, currently not used, but might come in handy some day...
     ... and it came in handy!  For our TCP/IP layer, it avoids all the fd_set
     and timeval stuff since this is the only place where it is used.
    */
    int socket = httpfd;
    int timeout = timo < 0 ? -timo : 1000 * timo;

    debug(F101, "http_inc IBMSELECT", "", timo);
    for (; timeout >= 0; timeout -= (timo ? 100 : 0)) {
      if (select(&socket, 1, 0, 0, 100L) == 1) {
        x = 0;
        break;
      }
    }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
  }

  if (timo && x < 0) { /* select() timed out */
#ifdef HTTP_BUFFERING
    http_count = 0;
    http_bufp = 0;
#endif /* HTTP_BUFFERING */
    debug(F100, "http_inc select() timed out", "", 0);
    return (-1); /* Call it an i/o error */
  }

#ifdef HTTP_BUFFERING
  /*
    Buffering code added by fdc 15 Dec 2005 for non-SSL case only because HTTP
    GETs were orders of magnitude too slow due to the single-byte read()s.  The
    file-descriptor swapping is pretty gross, but the more elegant solution
    (calling a nettchk() like routine with the fd as a parameter) doesn't work,
    because nettchk() relies on too many other routines that, like itself, are
    hardwired for ttyfd.
  */
getfrombuffer:
  if (--http_count >= 0) {
    c = http_inbuf[http_bufp++];
    x = 1;
  } else {
    int savefd;
    savefd = ttyfd;
    ttyfd = httpfd;
    x = nettchk();
    ttyfd = savefd;
    debug(F101, "http_inc nettchk", "", x);
    if (x > HTTP_INBUFLEN)
      x = HTTP_INBUFLEN;
#ifdef TCPIPLIB
    x = socket_read(httpfd, http_inbuf, x);
#else  /* Not TCPIPLIB */
    x = read(httpfd, http_inbuf, x);
#endif /* TCPIPLIB */
    http_count = 0;
    http_bufp = 0;
    if (x > 0) {
      c = http_inbuf[http_bufp++];
      http_count = x - 1;
    }
  }
#else /* Not HTTP_BUFFERING */
#ifdef TCPIPLIB
  x = socket_read(httpfd, &c, 1);
#else  /* Not TCPIPLIB */
  x = read(httpfd, &c, 1);
#endif /* TCPIPLIB */
#endif /* HTTP_BUFFERING */

  if (x <= 0) {
    int s_errno = socket_errno;
    debug(F101, "ttbufr socket_read", "", x);
    debug(F101, "ttbufr socket_errno", "", s_errno);
    http_close(); /* *** *** */
    return (-2);
  }
  return (c);
}

void http_set_code_reply(char *msg) {
  char *p = msg;
  char buf[16];
  int i = 0;

  while (*p != SP && *p != NUL) {
    buf[i] = *p;
    p++;
    i++;
  }

  http_code = atoi(buf);

  while (*p == SP)
    p++;

  ckstrncpy(http_reply_str, p, HTTPBUFLEN);
}

int http_get(char *agent, char **hdrlist, char *user, char *pwd, char array,
             char *local, char *remote, int stdio) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
#ifdef SYSUTIMEH
  struct utimbuf u_t;
#else
  struct utimbuf {
    time_t atime;
    time_t mtime;
  } u_t;
#endif /* SYSUTIMH */
  time_t mod_t = 0;
  time_t srv_t = 0;
  time_t local_t = 0;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int chunked = 0;
  int zfile = 0;
  int first = 1;

#ifdef DEBUG
  if (deblog) {
    debug(F101, "http_get httpfd", "", httpfd);
    debug(F110, "http_agent", agent, 0);
    debug(F110, "http_user", user, 0);
    debug(F110, "http_local", local, 0);
    debug(F110, "http_remote", remote, 0);
  }
#endif /* DEBUG */
  if (!remote)
    remote = "";

  if (httpfd == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }
  len = 8; /* GET */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += (int)strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd)); /* NOT PORTABLE */
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "GET %s %s\r\n", remote, HTTP_VERSION); /* safe */
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  ckstrncat(request, "\r\n", len);

getreq:
  if (http_tol((CHAR *)request, strlen(request)) < 0) {
    http_close();
    if (first) {
      first--;
      http_reopen();
      goto getreq;
    }
    rc = -1;
    goto getexit;
  }

  /* Process the headers */
  local_t = time(NULL);
  nullline = 0;
  i = 0;
  len = -1;
  while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
    buf[i] = ch;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      if (i < 1)
        nullline = 1;
      buf[i] = '\0';
      if (array && !nullline && hdcnt < HTTPHEADCNT)
        makestr(&headers[hdcnt++], buf);
      if (!ckstrcmp(buf, "HTTP", 4, 0)) {
        http_fnd = 1;
        j = ckindex(" ", buf, 0, 0, 0);
        p = &buf[j];
        while (isspace(*p))
          p++;
        switch (p[0]) {
        case '1': /* Informational message */
          break;
        case '2': /* Success */
          break;
        case '3': /* Redirection */
        case '4': /* Client failure */
        case '5': /* Server failure */
        default:  /* Unknown */
          if (!quiet)
            printf("Failure: Server reports %s\n", p);
          rc = -1;
          local = NULL;
        }
        http_set_code_reply(p);
#ifdef CMDATE2TM
      } else if (!ckstrcmp(buf, "Last-Modified", 13, 0)) {
        mod_t = http_date(&buf[15]);
      } else if (!ckstrcmp(buf, "Date", 4, 0)) {
        srv_t = http_date(&buf[4]);
#endif /* CMDATE2TM */
      } else if (!ckstrcmp(buf, "Connection:", 11, 0)) {
        if (ckindex("close", buf, 11, 0, 0) != 0)
          closecon = 1;
      } else if (!ckstrcmp(buf, "Content-Length:", 15, 0)) {
        len = atoi(&buf[16]);
      } else if (!ckstrcmp(buf, "Transfer-Encoding:", 18, 0)) {
        if (ckindex("chunked", buf, 18, 0, 0) != 0)
          chunked = 1;
        debug(F101, "http_get chunked", "", chunked);
      }
      i = 0;
    } else {
      i++;
    }
  }
  if (ch < 0 && first) {
    first--;
    http_close();
    http_reopen();
    goto getreq;
  }
  if (http_fnd == 0) {
    rc = -1;
    closecon = 1;
    goto getexit;
  }

  /* Now we have the contents of the file */
  if (local && local[0]) {
    if (zopeno(ZOFILE, local, NULL, NULL))
      zfile = 1;
    else
      rc = -1;
  }

  if (chunked) {
    while ((len = http_get_chunk_len()) > 0) {
      while (len && (ch = http_inc(0)) >= 0) {
        len--;
        if (zfile)
          zchout(ZOFILE, (CHAR)ch);
        if (stdio)
          conoc((CHAR)ch);
      }
      if ((ch = http_inc(0)) != CK_CR)
        break;
      if ((ch = http_inc(0)) != LF)
        break;
    }
  } else {
    while (len && (ch = http_inc(0)) >= 0) {
      len--;
      if (zfile)
        zchout(ZOFILE, (CHAR)ch);
      if (stdio)
        conoc((CHAR)ch);
    }
  }

  if (zfile)
    zclose(ZOFILE);

  if (chunked) { /* Parse Trailing Headers */
    nullline = 0;
    while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
      buf[i] = ch;
      if (buf[i] == 10) { /* found end of line */
        if (i > 0 && buf[i - 1] == 13)
          i--;
        if (i < 1)
          nullline = 1;
        buf[i] = '\0';
        if (array && !nullline && hdcnt < HTTPHEADCNT)
          makestr(&headers[hdcnt++], buf);
#ifdef CMDATE2TM
        if (!ckstrcmp(buf, "Last-Modified", 13, 0)) {
          mod_t = http_date(&buf[15]);
        } else if (!ckstrcmp(buf, "Date", 4, 0)) {
          srv_t = http_date(&buf[4]);
        }
#endif /* CMDATE2TM */
        else if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        }
        i = 0;
      } else {
        i++;
      }
    }
  }

  if (zfile) { /* Set timestamp */
#ifndef NOSETTIME
#ifdef SYSUTIMEH
    u_t.actime = srv_t ? srv_t : local_t;
    u_t.modtime = mod_t ? mod_t : local_t;
#else
#ifdef BSD44
    u_t[0].tv_sec = srv_t ? srv_t : local_t;
    u_t[1].tv_sec = mod_t ? mod_t : local_t;
#else
    u_t.mtime = srv_t ? srv_t : local_t;
    u_t.atime = mod_t ? mod_t : local_t;
#endif /* BSD44 */
#endif /* SYSUTIMEH */
    utime(local, &u_t);
#endif /* NOSETTIME */
  }

getexit:
  if (array)
    http_mkarray(headers, hdcnt, array);

  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_head(char *agent, char **hdrlist, char *user, char *pwd, char array,
              char *local, char *remote, int stdio) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int first = 1;

  if (httpfd == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }
  len = 9; /* HEAD */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd)); /* NOT PORTABLE */
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = (char *)malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "HEAD %s %s\r\n", remote, HTTP_VERSION);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  ckstrncat(request, "\r\n", len);

  if (local && local[0]) {
    if (!zopeno(ZOFILE, local, NULL, NULL)) {
      free(request);
      return (-1);
    }
  }

headreq:
  if (http_tol((CHAR *)request, strlen(request)) < 0) {
    http_close();
    if (first) {
      first--;
      http_reopen();
      goto headreq;
    }
    rc = -1;
    goto headexit;
  }

  /* Process the headers */

  local_t = time(NULL);
  nullline = 0;
  i = 0;
  while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
    buf[i] = ch;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      if (i < 1)
        nullline = 1;
      buf[i] = '\0';
      if (array && !nullline && hdcnt < HTTPHEADCNT)
        makestr(&headers[hdcnt++], buf);
      if (!ckstrcmp(buf, "HTTP", 4, 0)) {
        http_fnd = 1;
        j = ckindex(" ", buf, 0, 0, 0);
        p = &buf[j];
        while (isspace(*p))
          p++;
        switch (p[0]) {
        case '1': /* Informational message */
          break;
        case '2': /* Success */
          break;
        case '3': /* Redirection */
        case '4': /* Client failure */
        case '5': /* Server failure */
        default:  /* Unknown */
          if (!quiet)
            printf("Failure: Server reports %s\n", p);
          rc = -1;
        }
        http_set_code_reply(p);
      } else {
        if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        }
        if (local && local[0]) {
          zsout(ZOFILE, buf);
          zsout(ZOFILE, "\r\n");
        }
        if (stdio)
          printf("%s\r\n", buf);
      }
      i = 0;
    } else {
      i++;
    }
  }
  if (ch < 0 && first) {
    first--;
    http_close();
    http_reopen();
    goto headreq;
  }
  if (http_fnd == 0)
    rc = -1;

  if (array)
    http_mkarray(headers, hdcnt, array);

headexit:
  if (local && local[0])
    zclose(ZOFILE);
  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_index(char *agent, char **hdrlist, char *user, char *pwd, char array,
               char *local, char *remote, int stdio) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int chunked = 0;
  int zfile = 0;
  int first = 1;

  if (httpfd == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }
  len = 10; /* INDEX */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd));
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "INDEX %s\r\n", HTTP_VERSION);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  ckstrncat(request, "\r\n", len);
indexreq:
  if (http_tol((CHAR *)request, strlen(request)) < 0) {
    http_close();
    if (first) {
      first--;
      http_reopen();
      goto indexreq;
    }
    rc = -1;
    goto indexexit;
  }

  /* Process the headers */
  local_t = time(NULL);
  nullline = 0;
  i = 0;
  len = -1;
  while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
    buf[i] = ch;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      if (i < 1)
        nullline = 1;
      buf[i] = '\0';
      if (array && !nullline && hdcnt < HTTPHEADCNT)
        makestr(&headers[hdcnt++], buf);
      if (!ckstrcmp(buf, "HTTP", 4, 0)) {
        http_fnd = 1;
        j = ckindex(" ", buf, 0, 0, 0);
        p = &buf[j];
        while (isspace(*p))
          p++;
        switch (p[0]) {
        case '1': /* Informational message */
          break;
        case '2': /* Success */
          break;
        case '3': /* Redirection */
        case '4': /* Client failure */
        case '5': /* Server failure */
        default:  /* Unknown */
          if (!quiet)
            printf("Failure: Server reports %s\n", p);
          rc = -1;
        }
        http_set_code_reply(p);
      } else if (!nullline) {
        if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        } else if (!ckstrcmp(buf, "Content-Length:", 15, 0)) {
          len = atoi(&buf[16]);
        } else if (!ckstrcmp(buf, "Transfer-Encoding:", 18, 0)) {
          if (ckindex("chunked", buf, 18, 0, 0) != 0)
            chunked = 1;
          debug(F101, "http_index chunked", "", chunked);
        }
        printf("%s\n", buf);
      }
      i = 0;
    } else {
      i++;
    }
  }

  if (ch < 0 && first) {
    first--;
    http_close();
    http_reopen();
    goto indexreq;
  }
  if (http_fnd == 0) {
    rc = -1;
    closecon = 1;
    goto indexexit;
  }

  /* Now we have the contents of the file */
  if (local && local[0]) {
    if (zopeno(ZOFILE, local, NULL, NULL))
      zfile = 1;
    else
      rc = -1;
  }

  if (chunked) {
    while ((len = http_get_chunk_len()) > 0) {
      while (len && (ch = http_inc(0)) >= 0) {
        len--;
        if (zfile)
          zchout(ZOFILE, (CHAR)ch);
        if (stdio)
          conoc((CHAR)ch);
      }
      if ((ch = http_inc(0)) != CK_CR)
        break;
      if ((ch = http_inc(0)) != LF)
        break;
    }
  } else {
    while (len && (ch = http_inc(0)) >= 0) {
      len--;
      if (zfile)
        zchout(ZOFILE, (CHAR)ch);
      if (stdio)
        conoc((CHAR)ch);
    }
  }

  if (zfile)
    zclose(ZOFILE);

  if (chunked) { /* Parse Trailing Headers */
    nullline = 0;
    while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
      buf[i] = ch;
      if (buf[i] == 10) { /* found end of line */
        if (i > 0 && buf[i - 1] == 13)
          i--;
        if (i < 1)
          nullline = 1;
        buf[i] = '\0';
        if (array && !nullline && hdcnt < HTTPHEADCNT)
          makestr(&headers[hdcnt++], buf);
        if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        }
        i = 0;
      } else {
        i++;
      }
    }
  }
  rc = 0;

indexexit:
  if (array)
    http_mkarray(headers, hdcnt, array);

  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_put(char *agent, char **hdrlist, char *mime, char *user, char *pwd,
             char array, char *local, char *remote, char *dest, int stdio) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  int filelen;
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int chunked = 0;
  int first = 1;
  int zfile = 0;

  if (httpfd == -1)
    return (-1);
  if (!mime)
    mime = "";
  if (!remote)
    remote = "";
  if (!local)
    local = "";
  if (!*local)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }
  filelen = zchki(local);
  if (filelen < 0)
    return (-1);

  /* Compute length of request header */
  len = 8; /* PUT */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd));
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
  len += 16 + strlen(mime); /* Content-type: */
  len += 32;                /* Content-length: */
  len += 32;                /* Date: */
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "PUT %s %s\r\n", remote, HTTP_VERSION);
  ckstrncat(request, "Date: ", len);
#ifdef CMDATE2TM
  ckstrncat(request, http_now(), len);
#else
  ckstrncat(request, ..., len);
#endif /* CMDATE2TM */
  ckstrncat(request, "\r\n", len);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
  ckstrncat(request, "Content-type: ", len);
  ckstrncat(request, mime, len);
  ckstrncat(request, "\r\n", len);
  sprintf(buf, "Content-length: %d\r\n", filelen); /* safe */
  ckstrncat(request, buf, len);
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  ckstrncat(request, "\r\n", len);

  /* Now we have the contents of the file */
  if (zopeni(ZIFILE, local)) {

  putreq: /* Send request */
    if (http_tol((CHAR *)request, strlen(request)) <= 0) {
      http_close();
      if (first) {
        first--;
        http_reopen();
        goto putreq;
      }
      zclose(ZIFILE);
      rc = -1;
      goto putexit;
    }
    /* Request headers have been sent */

    i = 0;
    while (zchin(ZIFILE, &ch) == 0) {
      buf[i++] = ch;
      if (i == HTTPBUFLEN) {
        if (http_tol((CHAR *)buf, HTTPBUFLEN) <= 0) {
          http_close();
          if (first) {
            first--;
            http_reopen();
            goto putreq;
          }
        }
        i = 0;
      }
    }
    if (i > 0) {
      if (http_tol((CHAR *)buf, i) < 0) {
        http_close();
        if (first) {
          first--;
          http_reopen();
          goto putreq;
        }
      }
    }
    zclose(ZIFILE);

    /* Process the response headers */
    local_t = time(NULL);
    nullline = 0;
    i = 0;
    len = -1;
    while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
      buf[i] = ch;
      if (buf[i] == 10) { /* found end of line */
        if (i > 0 && buf[i - 1] == 13)
          i--;
        if (i < 1)
          nullline = 1;
        buf[i] = '\0';
        if (array && !nullline && hdcnt < HTTPHEADCNT)
          makestr(&headers[hdcnt++], buf);
        if (!ckstrcmp(buf, "HTTP", 4, 0)) {
          http_fnd = 1;
          j = ckindex(" ", buf, 0, 0, 0);
          p = &buf[j];
          while (isspace(*p))
            p++;
          switch (p[0]) {
          case '1': /* Informational message */
            break;
          case '2': /* Success */
            break;
          case '3': /* Redirection */
          case '4': /* Client failure */
          case '5': /* Server failure */
          default:  /* Unknown */
            if (!quiet)
              printf("Failure: Server reports %s\n", p);
            rc = -1;
          }
          http_set_code_reply(p);
        } else {
          if (!ckstrcmp(buf, "Connection:", 11, 0)) {
            if (ckindex("close", buf, 11, 0, 0) != 0)
              closecon = 1;
          } else if (!ckstrcmp(buf, "Content-Length:", 15, 0)) {
            len = atoi(&buf[16]);
          } else if (!ckstrcmp(buf, "Transfer-Encoding:", 18, 0)) {
            if (ckindex("chunked", buf, 18, 0, 0) != 0)
              chunked = 1;
            debug(F101, "http_put chunked", "", chunked);
          }
          if (stdio)
            printf("%s\n", buf);
        }
        i = 0;
      } else {
        i++;
      }
    }
    if (ch < 0 && first) {
      first--;
      http_close();
      http_reopen();
      goto putreq;
    }
    if (http_fnd == 0) {
      closecon = 1;
      rc = -1;
      goto putexit;
    }

    /* Any response data? */
    if (dest && dest[0]) {
      if (zopeno(ZOFILE, dest, NULL, NULL))
        zfile = 1;
      else
        rc = -1;
    }

    if (chunked) {
      while ((len = http_get_chunk_len()) > 0) {
        while (len && (ch = http_inc(0)) >= 0) {
          len--;
          if (zfile)
            zchout(ZOFILE, (CHAR)ch);
          if (stdio)
            conoc((CHAR)ch);
        }
        if ((ch = http_inc(0)) != CK_CR)
          break;
        if ((ch = http_inc(0)) != LF)
          break;
      }
    } else {
      while (len && (ch = http_inc(0)) >= 0) {
        len--;
        if (zfile)
          zchout(ZOFILE, (CHAR)ch);
        if (stdio)
          conoc((CHAR)ch);
      }
    }

    if (zfile)
      zclose(ZOFILE);

    if (chunked) { /* Parse Trailing Headers */
      nullline = 0;
      while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
        buf[i] = ch;
        if (buf[i] == 10) { /* found end of line */
          if (i > 0 && buf[i - 1] == 13)
            i--;
          if (i < 1)
            nullline = 1;
          buf[i] = '\0';
          if (array && !nullline && hdcnt < HTTPHEADCNT)
            makestr(&headers[hdcnt++], buf);
          if (!ckstrcmp(buf, "Connection:", 11, 0)) {
            if (ckindex("close", buf, 11, 0, 0) != 0)
              closecon = 1;
          }
          i = 0;
        } else {
          i++;
        }
      }
    }
  } else {
    rc = -1;
  }

putexit:
  if (array)
    http_mkarray(headers, hdcnt, array);

  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_delete(char *agent, char **hdrlist, char *user, char *pwd, char array,
                char *remote) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int chunked = 0;
  int first = 1;

  if (httpfd == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }

  /* Compute length of request header */
  len = 11; /* DELETE */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd));
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
  len += 32; /* Date: */
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "DELETE %s %s\r\n", remote, HTTP_VERSION);
  ckstrncat(request, "Date: ", len);
#ifdef CMDATE2TM
  ckstrncat(request, http_now(), len);
#else
  ckstrncat(request, ..., len);
#endif /* CMDATE2TM */
  ckstrncat(request, "\r\n", len);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  ckstrncat(request, "\r\n", len);
delreq:
  if (http_tol((CHAR *)request, strlen(request)) < 0) {
    http_close();
    if (first) {
      first--;
      http_reopen();
      goto delreq;
    }
    rc = -1;
    goto delexit;
  }

  /* Process the response headers */
  local_t = time(NULL);
  nullline = 0;
  i = 0;
  len = -1;
  while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
    buf[i] = ch;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      if (i < 1)
        nullline = 1;
      buf[i] = '\0';
      if (array && !nullline && hdcnt < HTTPHEADCNT)
        makestr(&headers[hdcnt++], buf);
      if (!ckstrcmp(buf, "HTTP", 4, 0)) {
        http_fnd = 1;
        j = ckindex(" ", buf, 0, 0, 0);
        p = &buf[j];
        while (isspace(*p))
          p++;
        switch (p[0]) {
        case '1': /* Informational message */
          break;
        case '2': /* Success */
          break;
        case '3': /* Redirection */
        case '4': /* Client failure */
        case '5': /* Server failure */
        default:  /* Unknown */
          if (!quiet)
            printf("Failure: Server reports %s\n", p);
          rc = -1;
        }
        http_set_code_reply(p);
      } else {
        if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        } else if (!ckstrcmp(buf, "Content-Length:", 15, 0)) {
          len = atoi(&buf[16]);
        } else if (!ckstrcmp(buf, "Transfer-Encoding:", 18, 0)) {
          if (ckindex("chunked", buf, 18, 0, 0) != 0)
            chunked = 1;
          debug(F101, "http_delete chunked", "", chunked);
        }
        printf("%s\n", buf);
      }
      i = 0;
    } else {
      i++;
    }
  }
  if (ch < 0 && first) {
    first--;
    http_close();
    http_reopen();
    goto delreq;
  }
  if (http_fnd == 0) {
    rc = -1;
    closecon = 1;
    goto delexit;
  }

  /* Any response data? */
  if (chunked) {
    while ((len = http_get_chunk_len()) > 0) {
      while (len && (ch = http_inc(0)) >= 0) {
        len--;
        conoc((CHAR)ch);
      }
      if ((ch = http_inc(0)) != CK_CR)
        break;
      if ((ch = http_inc(0)) != LF)
        break;
    }
  } else {
    while (len && (ch = http_inc(0)) >= 0) {
      len--;
      conoc((CHAR)ch);
    }
  }

  if (chunked) { /* Parse Trailing Headers */
    nullline = 0;
    while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
      buf[i] = ch;
      if (buf[i] == 10) { /* found end of line */
        if (i > 0 && buf[i - 1] == 13)
          i--;
        if (i < 1)
          nullline = 1;
        buf[i] = '\0';
        if (array && !nullline && hdcnt < HTTPHEADCNT)
          makestr(&headers[hdcnt++], buf);
        if (!ckstrcmp(buf, "Connection:", 11, 0)) {
          if (ckindex("close", buf, 11, 0, 0) != 0)
            closecon = 1;
        }
        i = 0;
      } else {
        i++;
      }
    }
  }

delexit:
  if (array)
    http_mkarray(headers, hdcnt, array);

  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_post(char *agent, char **hdrlist, char *mime, char *user, char *pwd,
              char array, char *local, char *remote, char *dest, int stdio) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int ch;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  int filelen;
  char *headers[HTTPHEADCNT];
  int closecon = 0;
  int chunked = 0;
  int zfile = 0;
  int first = 1;

  if (httpfd == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }
  filelen = zchki(local);
  if (filelen < 0)
    return (-1);

  /* Compute length of request header */
  len = 9; /* POST */
  len += strlen(HTTP_VERSION);
  len += strlen(remote);
  len += 16;

  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  len += strlen(http_host_port) + 8;

  if (agent)
    len += 13 + strlen(agent);
  if (user) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd));
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 24;
  }
  len += 16 + strlen(mime); /* Content-type: */
  len += 32;                /* Content-length: */
  len += 32;                /* Date: */
#ifdef HTTP_CLOSE
  len += 19; /* Connection: close */
#endif
  len += 3; /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "POST %s %s\r\n", remote, HTTP_VERSION);
  ckstrncat(request, "Date: ", len);
  ckstrncat(request, http_now(), len);
  ckstrncat(request, "\r\n", len);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user) {
    ckstrncat(request, "Authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
  }
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
  ckstrncat(request, "Content-type: ", len);
  ckstrncat(request, mime, len);
  ckstrncat(request, "\r\n", len);
#ifdef HTTP_CLOSE
  ckstrncat(request, "Connection: close\r\n", len);
#endif
  sprintf(buf, "Content-length: %d\r\n", filelen); /* safe */
  ckstrncat(request, buf, len);
  ckstrncat(request, "\r\n", len);

  /* Now we have the contents of the file */
postopen:
  if (zopeni(ZIFILE, local)) {
  postreq:
    if (http_tol((CHAR *)request, strlen(request)) < 0) {
      http_close();
      if (first) {
        first--;
        http_reopen();
        goto postreq;
      }
      rc = -1;
      zclose(ZIFILE);
      goto postexit;
    }

    i = 0;
    while (zchin(ZIFILE, &ch) == 0) {
      buf[i++] = ch;
      if (i == HTTPBUFLEN) {
        http_tol((CHAR *)buf, HTTPBUFLEN);
        i = 0;
      }
    }
    if (i > 0)
      http_tol((CHAR *)buf, HTTPBUFLEN);
    zclose(ZIFILE);

    /* Process the response headers */
    local_t = time(NULL);
    nullline = 0;
    i = 0;
    len = -1;
    while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
      buf[i] = ch;
      if (buf[i] == 10) { /* found end of line */
        if (i > 0 && buf[i - 1] == 13)
          i--;
        if (i < 1)
          nullline = 1;
        buf[i] = '\0';
        if (array && !nullline && hdcnt < HTTPHEADCNT)
          makestr(&headers[hdcnt++], buf);
        if (!ckstrcmp(buf, "HTTP", 4, 0)) {
          http_fnd = 1;
          j = ckindex(" ", buf, 0, 0, 0);
          p = &buf[j];
          while (isspace(*p))
            p++;
          switch (p[0]) {
          case '1': /* Informational message */
            break;
          case '2': /* Success */
            break;
          case '3': /* Redirection */
          case '4': /* Client failure */
          case '5': /* Server failure */
          default:  /* Unknown */
            if (!quiet)
              printf("Failure: Server reports %s\n", p);
            rc = -1;
          }
          http_set_code_reply(p);
        } else {
          if (!ckstrcmp(buf, "Connection:", 11, 0)) {
            if (ckindex("close", buf, 11, 0, 0) != 0)
              closecon = 1;
          } else if (!ckstrcmp(buf, "Content-Length:", 15, 0)) {
            len = atoi(&buf[16]);
          } else if (!ckstrcmp(buf, "Transfer-Encoding:", 18, 0)) {
            if (ckindex("chunked", buf, 18, 0, 0) != 0)
              chunked = 1;
            debug(F101, "http_post chunked", "", chunked);
          }
          if (stdio)
            printf("%s\n", buf);
        }
        i = 0;
      } else {
        i++;
      }
    }
    if (ch < 0 && first) {
      first--;
      http_close();
      http_reopen();
      goto postopen;
    }
    if (http_fnd == 0) {
      rc = -1;
      closecon = 1;
      goto postexit;
    }

    /* Any response data? */
    if (dest && dest[0]) {
      if (zopeno(ZOFILE, dest, NULL, NULL))
        zfile = 1;
      else
        rc = -1;
    }

    if (chunked) {
      while ((len = http_get_chunk_len()) > 0) {
        while (len && (ch = http_inc(0)) >= 0) {
          len--;
          if (zfile)
            zchout(ZOFILE, (CHAR)ch);
          if (stdio)
            conoc((CHAR)ch);
        }
        if ((ch = http_inc(0)) != CK_CR)
          break;
        if ((ch = http_inc(0)) != LF)
          break;
      }
    } else {
      while (len && (ch = http_inc(0)) >= 0) {
        len--;
        if (zfile)
          zchout(ZOFILE, (CHAR)ch);
        if (stdio)
          conoc((CHAR)ch);
      }
    }

    if (zfile)
      zclose(ZOFILE);

    if (chunked) { /* Parse Trailing Headers */
      nullline = 0;
      while (!nullline && (ch = http_inc(0)) >= 0 && i < HTTPBUFLEN) {
        buf[i] = ch;
        if (buf[i] == 10) { /* found end of line */
          if (i > 0 && buf[i - 1] == 13)
            i--;
          if (i < 1)
            nullline = 1;
          buf[i] = '\0';
          if (array && !nullline && hdcnt < HTTPHEADCNT)
            makestr(&headers[hdcnt++], buf);
          if (!ckstrcmp(buf, "Connection:", 11, 0)) {
            if (ckindex("close", buf, 11, 0, 0) != 0)
              closecon = 1;
          }
          i = 0;
        } else {
          i++;
        }
      }
    }
  } else {
    rc = -1;
  }

postexit:
  if (array)
    http_mkarray(headers, hdcnt, array);
  if (closecon)
    http_close();
  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}

int http_connect(int socket, char *agent, char **hdrlist, char *user, char *pwd,
                 char array, char *host_port) {
  char *request = NULL;
  int i, j, len = 0, hdcnt = 0, rc = 0;
  int http_fnd = 0;
  char buf[HTTPBUFLEN], *p, ch;
  int nullline;
  time_t local_t;
  char passwd[64];
  char b64in[128];
  char b64out[256];
  char *headers[HTTPHEADCNT];
  int connected = 0;

  tcp_http_proxy_errno = 0;

  if (socket == -1)
    return (-1);

  if (array) {
    for (i = 0; i < HTTPHEADCNT; i++)
      headers[i] = NULL;
  }

  /* Compute length of request header */
  len = 12; /* CONNECT */
  len += strlen(HTTP_VERSION);
  len += strlen(host_port);
  len += (int)strlen(http_host_port) + 8;
  len += 16;
  len += strlen("Proxy-Connection: Keep-Alive\r\n");
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++)
      len += strlen(hdrlist[i]) + 2;
  }
  if (agent && agent[0])
    len += 13 + strlen(agent);
  if (user && user[0]) {
    if (!pwd) {
      readpass("Password: ", passwd, 64);
      pwd = passwd;
    }
    ckmakmsg(b64in, sizeof(b64in), user, ":", pwd, NULL);
    j = b8tob64(b64in, strlen(b64in), b64out, 256);
    memset(pwd, 0, strlen(pwd));
    if (j < 0)
      return (-1);
    b64out[j] = '\0';
    len += j + 72;
  }
  len += 32; /* Date: */
  len += 3;  /* blank line + null */

  request = malloc(len);
  if (!request)
    return (-1);

  sprintf(request, "CONNECT %s %s\r\n", host_port, HTTP_VERSION);
  ckstrncat(request, "Date: ", len);
#ifdef CMDATE2TM
  ckstrncat(request, http_now(), len);
#else
  strcat(request, ...);
#endif /* CMDATE2TM */
  ckstrncat(request, "\r\n", len);
  ckstrncat(request, "Host: ", len);
  ckstrncat(request, http_host_port, len);
  ckstrncat(request, "\r\n", len);
  if (agent && agent[0]) {
    ckstrncat(request, "User-agent: ", len);
    ckstrncat(request, agent, len);
    ckstrncat(request, "\r\n", len);
  }
  if (user && user[0]) {
    ckstrncat(request, "Proxy-authorization: Basic ", len);
    ckstrncat(request, b64out, len);
    ckstrncat(request, "\r\n", len);
    ckstrncat(request, "Extension: Security/Remote-Passphrase\r\n", len);
  }
  ckstrncat(request, "Proxy-Connection: Keep-Alive\r\n", len);
  if (hdrlist) {
    for (i = 0; hdrlist[i]; i++) {
      ckstrncat(request, hdrlist[i], len);
      ckstrncat(request, "\r\n", len);
    }
  }
  ckstrncat(request, "\r\n", len);
  len = strlen(request);

#ifdef TCPIPLIB
  /* Send request */
  if (socket_write(socket, (CHAR *)request, strlen(request)) < 0) {
    rc = -1;
    goto connexit;
  }
#else
  if (write(socket, (CHAR *)request, strlen(request)) < 0) { /* Send request */
    rc = -1;
    goto connexit;
  }
#endif /* TCPIPLIB */

  /* Process the response headers */
  local_t = time(NULL);
  nullline = 0;
  i = 0;
  while (!nullline &&
#ifdef TCPIPLIB
         (socket_read(socket, &ch, 1) == 1) &&
#else
         (read(socket, &ch, 1) == 1) &&
#endif /* TCPIPLIB */
         i < HTTPBUFLEN) {
    buf[i] = ch;
    if (buf[i] == 10) { /* found end of line */
      if (i > 0 && buf[i - 1] == 13)
        i--;
      if (i < 1)
        nullline = 1;
      buf[i] = '\0';

      if (array && !nullline && hdcnt < HTTPHEADCNT)
        makestr(&headers[hdcnt++], buf);
      if (!ckstrcmp(buf, "HTTP", 4, 0)) {
        http_fnd = 1;
        j = ckindex(" ", buf, 0, 0, 0);
        p = &buf[j];
        while (isspace(*p))
          p++;
        tcp_http_proxy_errno = atoi(p);
        switch (p[0]) {
        case '1': /* Informational message */
          break;
        case '2': /* Success */
          connected = 1;
          break;
        case '3': /* Redirection */
        case '4': /* Client failure */
        case '5': /* Server failure */
        default:  /* Unknown */
          if (!quiet)
            printf("Failure: Server reports %s\n", p);
          rc = -1;
        }
        http_set_code_reply(p);
      } else {
        printf("%s\n", buf);
      }
      i = 0;
    } else {
      i++;
    }
  }
  if (http_fnd == 0)
    rc = -1;

  if (array)
    http_mkarray(headers, hdcnt, array);

connexit:
  if (!connected) {
    if (socket == ttyfd) {
      ttclos(0);
    } else if (socket == httpfd) {
      http_close();
    }
  }

  free(request);
  for (i = 0; i < hdcnt; i++) {
    if (headers[i])
      free(headers[i]);
  }
  return (rc);
}
#endif /* NOHTTP */

#ifdef CK_DNS_SRV

#define INCR_CHECK(x, y)                                                       \
  x += y;                                                                      \
  if (x > size + answer.bytes)                                                 \
  goto dnsout
#define CHECK(x, y)                                                            \
  if (x + y > size + answer.bytes)                                             \
  goto dnsout
#define NTOHSP(x, y)                                                           \
  x[0] << 8 | x[1];                                                            \
  x += y

#ifndef CKQUERYTYPE
#endif /* CKQUERYTYPE */

#ifndef CKQUERYTYPE
#define CKQUERYTYPE char
#endif /* CKQUERYTYPE */

/* 1 is success, 0 is failure */
int locate_srv_dns(char *host, char *service, char *protocol,
                   struct sockaddr **addr_pp, int *naddrs) {
  int nout, j, count;
  union {
    unsigned char bytes[2048];
    HEADER hdr;
  } answer;
  unsigned char *p = NULL;
  CKQUERYTYPE query[MAX_DNS_NAMELEN];
  const char *h;
  struct sockaddr *addr = NULL;
  struct sockaddr_in *sin = NULL;
  struct hostent *hp = NULL;
  int type, class;
  int priority, weight, size, len, numanswers, numqueries, rdlen;
  unsigned short port;
  const int hdrsize = sizeof(HEADER);
  struct srv_dns_entry {
    struct srv_dns_entry *next;
    int priority;
    int weight;
    unsigned short port;
    char *host;
  };
  struct srv_dns_entry *head = NULL;
  struct srv_dns_entry *srv = NULL, *entry = NULL;
  char *s = NULL;

  nout = 0;
  addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
  if (addr == NULL)
    return 0;

  count = 1;

  /*
   * First build a query of the form:
   *
   *   service.protocol.host
   *
   * which will most likely be something like:
   *
   *   _telnet._tcp.host
   *
   */
  if (((int)strlen(service) + strlen(protocol) + strlen(host) + 5) >
      MAX_DNS_NAMELEN)
    goto dnsout;

  /* Realm names don't (normally) end with ".", but if the query
     doesn't end with "." and doesn't get an answer as is, the
     resolv code will try appending the local domain.  Since the
     realm names are absolutes, let's stop that.

     But only if a name has been specified.  If we are performing
     a search on the prefix alone then the intention is to allow
     the local domain or domain search lists to be expanded.
  */
  h = host + strlen(host);
  ckmakxmsg(query, sizeof(query), "_", service, "._", protocol, ".", host,
            ((h > host) && (h[-1] != '.') ? "." : NULL), NULL, NULL, NULL, NULL,
            NULL);

  size = res_search(query, C_IN, T_SRV, answer.bytes, sizeof(answer.bytes));

  if (size < hdrsize)
    goto dnsout;

  /* We got a reply - See how many answers it contains. */

  p = answer.bytes;

  numqueries = ntohs(answer.hdr.qdcount);
  numanswers = ntohs(answer.hdr.ancount);

  p += sizeof(HEADER);

  /*
   * We need to skip over all of the questions, so we have to iterate
   * over every query record.  dn_expand() is able to tell us the size
   * of compressed DNS names, so we use it.
   */
  while (numqueries--) {
    len = dn_expand(answer.bytes, answer.bytes + size, p, query, sizeof(query));
    if (len < 0)
      goto dnsout;
    INCR_CHECK(p, len + 4);
  }

  /*
   * We're now pointing at the answer records.  Only process them if
   * they're actually T_SRV records (they might be CNAME records,
   * for instance).
   *
   * But in a DNS reply, if you get a CNAME you always get the associated
   * "real" RR for that CNAME.  RFC 1034, 3.6.2:
   *
   * CNAME RRs cause special action in DNS software.  When a name server
   * fails to find a desired RR in the resource set associated with the
   * domain name, it checks to see if the resource set consists of a CNAME
   * record with a matching class.  If so, the name server includes the CNAME
   * record in the response and restarts the query at the domain name
   * specified in the data field of the CNAME record.  The one exception to
   * this rule is that queries which match the CNAME type are not restarted.
   *
   * In other words, CNAMEs do not need to be expanded by the client.
   */
  while (numanswers--) {

    /* First is the name; use dn_expand() to get the compressed size. */
    len = dn_expand(answer.bytes, answer.bytes + size, p, query, sizeof(query));
    if (len < 0)
      goto dnsout;
    INCR_CHECK(p, len);

    CHECK(p, 2); /* Query type */
    type = NTOHSP(p, 2);

    CHECK(p, 6);          /* Query class */
    class = NTOHSP(p, 6); /* Also skip over 4-byte TTL */

    CHECK(p, 2); /* Record data length */
    rdlen = NTOHSP(p, 2);
    /*
     * If this is an SRV record, process it.  Record format is:
     *
     * Priority
     * Weight
     * Port
     * Server name
     */
    if (class == C_IN && type == T_SRV) {
      CHECK(p, 2);
      priority = NTOHSP(p, 2);
      CHECK(p, 2);
      weight = NTOHSP(p, 2);
      CHECK(p, 2);
      port = NTOHSP(p, 2);
      len =
          dn_expand(answer.bytes, answer.bytes + size, p, query, sizeof(query));
      if (len < 0)
        goto dnsout;
      INCR_CHECK(p, len);
      /*
       * We got everything.  Insert it into our list, but make sure
       * it's in the right order.  Right now we don't do anything
       * with the weight field
       */
      srv = (struct srv_dns_entry *)malloc(sizeof(struct srv_dns_entry));
      if (srv == NULL)
        goto dnsout;

      srv->priority = priority;
      srv->weight = weight;
      srv->port = port;
      makestr(&s, (char *)query); /* strdup() is not portable */
      srv->host = s;

      if (head == NULL || head->priority > srv->priority) {
        srv->next = head;
        head = srv;
      } else
        /*
         * Confusing.  Insert an entry into this spot only if:
         *  . The next person has a higher priority (lower
         *    priorities are preferred), or:
         *  . There is no next entry (we're at the end)
         */
        for (entry = head; entry != NULL; entry = entry->next)
          if ((entry->next && entry->next->priority > srv->priority) ||
              entry->next == NULL) {
            srv->next = entry->next;
            entry->next = srv;
            break;
          }
    } else
      INCR_CHECK(p, rdlen);
  }

  /*
   * Now we've got a linked list of entries sorted by priority.
   * Start looking up A records and returning addresses.
   */
  if (head == NULL)
    goto dnsout;

  for (entry = head; entry != NULL; entry = entry->next) {
    hp = gethostbyname(entry->host);
    if (hp != 0) {

      /* Watch out - memset() and memcpy() are not portable... */

      switch (hp->h_addrtype) {
      case AF_INET:
        for (j = 0; hp->h_addr_list[j]; j++) {
          sin = (struct sockaddr_in *)&addr[nout++];
          memset((char *)sin, 0, sizeof(struct sockaddr));
          sin->sin_family = hp->h_addrtype;
          sin->sin_port = htons(entry->port);
          memcpy((char *)&sin->sin_addr, (char *)hp->h_addr_list[j],
                 sizeof(struct in_addr)); /* safe */
          if (nout + 1 >= count) {
            count += 5;
            addr = (struct sockaddr *)realloc((char *)addr,
                                              sizeof(struct sockaddr) * count);
            if (!addr)
              goto dnsout;
          }
        }
        break;
      default:
        break;
      }
    }
  }
  for (entry = head; entry != NULL;) {
    free(entry->host);
    entry->host = NULL;
    srv = entry;
    entry = entry->next;
    free(srv);
    srv = NULL;
  }

dnsout:
  if (srv)
    free(srv);

  if (nout == 0) { /* No good servers */
    if (addr)
      free(addr);
    return 0;
  }
  *addr_pp = addr;
  *naddrs = nout;
  return 1;
}
#undef INCR_CHECK
#undef CHECK
#undef NTOHSP

#define INCR_CHECK(x, y)                                                       \
  x += y;                                                                      \
  if (x > size + answer.bytes)                                                 \
  return 0
#define CHECK(x, y)                                                            \
  if (x + y > size + answer.bytes)                                             \
  return 0
#define NTOHSP(x, y)                                                           \
  x[0] << 8 | x[1];                                                            \
  x += y

int locate_txt_rr(char *prefix, char *name, char **retstr) {
  union {
    unsigned char bytes[2048];
    HEADER hdr;
  } answer;
  unsigned char *p;
  char host[MAX_DNS_NAMELEN], *h;
  int size;
  int type, class, numanswers, numqueries, rdlen, len;

  /*
   * Form our query, and send it via DNS
   */

  if (name == NULL || name[0] == '\0') {
    strcpy(host, prefix);
  } else {
    if (strlen(prefix) + strlen(name) + 3 > MAX_DNS_NAMELEN)
      return 0;

    /* Realm names don't (normally) end with ".", but if the query
       doesn't end with "." and doesn't get an answer as is, the
       resolv code will try appending the local domain.  Since the
       realm names are absolutes, let's stop that.

       But only if a name has been specified.  If we are performing
       a search on the prefix alone then the intention is to allow
       the local domain or domain search lists to be expanded.
    */
    h = host + strlen(host);
    ckmakmsg(host, sizeof(host), prefix, ".", name,
             ((h > host) && (h[-1] != '.')) ? "." : NULL);
  }
  size = res_search(host, C_IN, T_TXT, answer.bytes, sizeof(answer.bytes));

  if (size < 0)
    return 0;

  p = answer.bytes;

  numqueries = ntohs(answer.hdr.qdcount);
  numanswers = ntohs(answer.hdr.ancount);

  p += sizeof(HEADER);

  /*
   * We need to skip over the questions before we can get to the answers,
   * which means we have to iterate over every query record.  We use
   * dn_expand to tell us how long each compressed name is.
   */

  while (numqueries--) {
    len = dn_expand(answer.bytes, answer.bytes + size, p, host, sizeof(host));
    if (len < 0)
      return 0;
    INCR_CHECK(p, len + 4); /* Name plus type plus class */
  }

  /*
   * We're now pointing at the answer records.  Process the first
   * TXT record we find.
   */

  while (numanswers--) {

    /* First the name; use dn_expand to get the compressed size */
    len = dn_expand(answer.bytes, answer.bytes + size, p, host, sizeof(host));
    if (len < 0)
      return 0;
    INCR_CHECK(p, len);

    /* Next is the query type */
    CHECK(p, 2);
    type = NTOHSP(p, 2);

    /* Next is the query class; also skip over 4 byte TTL */
    CHECK(p, 6);
    class = NTOHSP(p, 6);

    /* Record data length - make sure we aren't truncated */

    CHECK(p, 2);
    rdlen = NTOHSP(p, 2);

    if (p + rdlen > answer.bytes + size)
      return 0;

    /*
     * If this is a TXT record, return the string.  Note that the
     * string has a 1-byte length in the front
     */
    /* XXX What about flagging multiple TXT records as an error?  */

    if (class == C_IN && type == T_TXT) {
      len = *p++;
      if (p + len > answer.bytes + size)
        return 0;
      *retstr = malloc(len + 1);
      if (*retstr == NULL)
        return ENOMEM;
      strncpy(*retstr, (char *)p, len);
      (*retstr)[len] = '\0';
      /* Avoid a common error. */
      if ((*retstr)[len - 1] == '.')
        (*retstr)[len - 1] = '\0';
      return 1;
    }
  }

  return 0;
}
#undef INCR_CHECK
#undef CHECK
#undef NTOHSP
#endif /* CK_DNS_SRV */

#ifdef TNCODE
#endif /* TNCODE */
#endif /* NETCONN */
