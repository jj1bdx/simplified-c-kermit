/* ckcnet.h -- Symbol and macro definitions for C-Kermit network support */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2023,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKCNET_H
#define CKCNET_H

/* Network types */

#define NET_NONE 0  /* None */
#define NET_TCPB 1  /* TCP/IP Berkeley (socket) */
#define NET_TCPA 2  /* TCP/IP AT&T (streams) */
#define NET_SX25 3  /* SUNOS SunLink X.25 */
#define NET_DEC 4   /* DECnet */
#define NET_VPSI 5  /* VAX PSI */
#define NET_PIPE 6  /* LAN Manager Named Pipe */
#define NET_VX25 7  /* Stratus VOS X.25 */
#define NET_BIOS 8  /* IBM NetBios */
#define NET_SLAT 9  /* Meridian Technologies' SuperLAT */
#define NET_FILE 10 /* Read from a file */
#define NET_CMD 11  /* Read from a sub-process */
#define NET_DLL 12  /* Load a DLL for use as comm channel*/
#define NET_IX25 13 /* IBM AIX 4.1 X.25 */
#define NET_HX25 14 /* HP-UX 10 X.25 */
#define NET_PTY 15  /* Pseudoterminal */
#define NET_SSH 16  /* SSH */

#ifdef _M_PPC
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#endif /* _M_PPC */

#ifdef NPIPE        /* For items in common to */
#define NPIPEORBIOS /* Named Pipes and NETBIOS */
#endif              /* NPIPE */
#ifdef CK_NETBIOS
#ifndef NPIPEORBIOS
#define NPIPEORBIOS
#endif /* NPIPEORBIOS */
#endif /* CK_NETBIOS */

/* Network virtual terminal protocols (for SET HOST connections) */
/* FTP, HTTP and SSH have their own stacks                       */

#define NP_DEFAULT 255
#define NP_NONE 0        /* None (async) */
#define NP_TELNET 1      /* TCP/IP telnet */
#define NP_VTP 2         /* ISO Virtual Terminal Protocol */
#define NP_X3 3          /* CCITT X.3 */
#define NP_X28 4         /* CCITT X.28 */
#define NP_X29 5         /* CCITT X.29 */
#define NP_RLOGIN 6      /* TCP/IP Remote login */
#define NP_KERMIT 7      /* TCP/IP Kermit */
#define NP_TCPRAW 8      /* TCP/IP Raw socket */
#define NP_TCPUNK 9      /* TCP/IP Unknown */
#define NP_SSL 10        /* TCP/IP SSLv23 */
#define NP_TLS 11        /* TCP/IP TLSv1 */
#define NP_SSL_TELNET 12 /* TCP/IP Telnet over SSLv23 */
#define NP_TLS_TELNET 13 /* TCP/IP Telnet over TLSv1 */
#define NP_K4LOGIN 14    /* TCP/IP Kerberized remote login */
#define NP_EK4LOGIN 15   /* TCP/IP Encrypted Kerberized ... */
#define NP_K5LOGIN 16    /* TCP/IP Kerberized remote login */
#define NP_EK5LOGIN 17   /* TCP/IP Encrypted Kerberized ... */
#define NP_K5U2U 18      /* TCP/IP Kerberos 5 User to User */
#define NP_CTERM 19      /* DEC CTERM */
#define NP_LAT 20        /* DEC LAT */
#define NP_SSL_RAW 21    /* SSL with no Telnet permitted */
#define NP_TLS_RAW 22    /* TLS with no Telnet permitted */

/* others here... */

#define IS_TELNET()                                                            \
  (nettype == NET_TCPB && (ttnproto == NP_TELNET || ttnproto == NP_KERMIT))

#define IS_RLOGIN() (nettype == NET_TCPB && (ttnproto == NP_RLOGIN))

#define IS_SSH() (nettype == NET_SSH)

/* RLOGIN Modes */
#define RL_RAW 0    /*  Do Not Process XON/XOFF */
#define RL_COOKED 1 /*  Do Process XON/XOFF */

/* Encryption types */

#define CX_NONE 999

#ifdef ENCTYPE_ANY
#define CX_AUTO ENCTYPE_ANY
#else
#define CX_AUTO 0
#endif /* ENCTYPE_ANY */

#ifdef ENCTYPE_DES_CFB64
#define CX_DESC64 ENCTYPE_DES_CFB64
#else
#define CX_DESC64 1
#endif /* ENCTYPE_DES_CFB64 */

#ifdef ENCTYPE_DES_OFB64
#define CX_DESO64 ENCTYPE_DES_OFB64
#else
#define CX_DESO64 2
#endif /* ENCTYPE_DES_OFB64 */

#ifdef ENCTYPE_DES3_CFB64
#define CX_DES3C64 ENCTYPE_DES3_CFB64
#else
#define CX_DES3C64 3
#endif /* ENCTYPE_DES_CFB64 */

#ifdef ENCTYPE_DES3_OFB64
#define CX_DESO64 ENCTYPE_DES3_OFB64
#else
#define CX_DES3O64 4
#endif /* ENCTYPE_DES_OFB64 */

#ifdef ENCTYPE_CAST5_40_CFB64
#define CX_C540C64 ENCTYPE_CAST5_40_CFB64
#else
#define CX_C540C64 8
#endif /* ENCTYPE_CAST5_40_CFB64 */

#ifdef ENCTYPE_CAST5_40_OFB64
#define CX_C540O64 ENCTYPE_CAST5_40_OFB64
#else
#define CX_C540O64 9
#endif /* ENCTYPE_CAST5_40_OFB64 */

#ifdef ENCTYPE_CAST128_CFB64
#define CX_C128C64 ENCTYPE_CAST128_CFB64
#else
#define CX_C128C64 10
#endif /* ENCTYPE_CAST128_CFB64 */

#ifdef ENCTYPE_CAST128_OFB64
#define CX_C128O64 ENCTYPE_CAST128_OFB64
#else
#define CX_C128O64 11
#endif /* ENCTYPE_CAST128_OFB64 */

/* Basic network function prototypes, common to all. */

int netopen(char *, int *, int);
int netclos(void);
int netflui(void);
int nettchk(void);
int netxchk(int);
int netbreak(void);
int netinc(int);
int netxin(int, CHAR *);
int nettol(CHAR *, int);
int nettoc(CHAR);
int net_read(int fd, register char *buf, register int len);
int net_write(int fd, register const char *buf, int len);
int rlog_ctrl(unsigned char *cp, int n);
int locate_txt_rr(char *prefix, char *name, char **retstr);

#ifdef TCPSOCKET
int gettcpport(void);
int gettcpport(void);
#endif /* TCPSOCKET */

/*
  SunLink X.25 support by Marcello Frutig, Catholic University,
  Rio de Janeiro, Brazil, 1990.

  Maybe this can be adapted to VAX PSI and other X.25 products too.
*/

#ifdef IBMX25 /* AIX 4.1 X.25 */
#undef IBMX25
#endif /* IBMX25 */

#ifdef HPX25 /* HP-UX 10.* X.25 */
#undef HPX25
#endif /* HPX25 */

#ifdef ANYX25
#ifndef NETCONN /* ANYX25 implies NETCONN */
#define NETCONN
#endif /* NETCONN */

#define MAXPADPARMS 22 /* Number of PAD parameters */
#define MAXCUDATA 12   /* Max length of X.25 call user data */
#define X29PID 1       /* X.29 protocol ID */
#define X29PIDLEN 4    /* X.29 protocol ID length */

#define X29_SET_PARMS 2
#define X29_READ_PARMS 4
#define X29_SET_AND_READ_PARMS 6
#define X29_INVITATION_TO_CLEAR 1
#define X29_PARAMETER_INDICATION 0
#define X29_INDICATION_OF_BREAK 3
#define X29_ERROR 5

#define INVALID_PAD_PARM 1

#define PAD_BREAK_CHARACTER 0

#define PAD_ESCAPE 1
#define PAD_ECHO 2
#define PAD_DATA_FORWARD_CHAR 3
#define PAD_DATA_FORWARD_TIMEOUT 4
#define PAD_FLOW_CONTROL_BY_PAD 5
#define PAD_SUPPRESSION_OF_SIGNALS 6
#define PAD_BREAK_ACTION 7
#define PAD_SUPPRESSION_OF_DATA 8
#define PAD_PADDING_AFTER_CR 9
#define PAD_LINE_FOLDING 10
#define PAD_LINE_SPEED 11
#define PAD_FLOW_CONTROL_BY_USER 12
#define PAD_LF_AFTER_CR 13
#define PAD_PADDING_AFTER_LF 14
#define PAD_EDITING 15
#define PAD_CHAR_DELETE_CHAR 16
#define PAD_BUFFER_DELETE_CHAR 17
#define PAD_BUFFER_DISPLAY_CHAR 18

#define MAXIX25 MAX_USER_DATA * 7
#define MAXOX25 MAX_USER_DATA
#endif /* ANYX25 */

#ifdef ANYX25

#ifdef IBMX25 /* X.25 includes, AIX only */
#include <fcntl.h>
#include <sys/twtypes.h>
#include <sys/twlib.h>

#include <sys/stream.h>
#include <stropts.h>

#define NPI_20 /* required to include the whole NPI */
#include <sys/npi_20.h>
#include <sys/npiapi.h>
#include <sys/pktintf.h>

#include <odmi.h>       /* required for access to the ODM   */
#include <sys/cfgodm.h> /* database, needed to find out the */
                        /* local NUA. see x25local_nua()    */

/* IBM X25 NPI generic primitive type */
typedef union N_npi_ctl_t {
  ulong PRIM_type;          /* generic primitive type */
  char buffer[NPI_MAX_CTL]; /* maximum primitive size */
  N_bind_ack_t bind_ack;
  N_bind_req_t bind_req;
  N_conn_con_t conn_con;
  N_conn_ind_t conn_ind;
  N_conn_req_t conn_req;
  N_conn_res_t conn_res;
  N_data_req_t data_req;
  N_data_ind_t data_ind;
  N_discon_ind_t discon_ind;
  N_discon_req_t discon_req;
  N_error_ack_t error_ack;
  N_exdata_ind_t exdata_ind;
  N_info_ack_t info_ack;
  N_ok_ack_t ok_ack;
  N_reset_con_t reset_con;
  N_reset_req_t reset_req;
  N_reset_ind_t reset_ind;
} N_npi_ctl_t;

/* some extra definitions to help out */
typedef char x25addr_t[45]; /* max 40 defined by CCITT */
typedef char N_npi_data_t[NPI_MAX_DATA];

/* fd or server waiting for connections, used by netclos and netopen */
extern int x25serverfd;

#endif /* IBMX25 */

#ifdef HPX25 /* X.25 includes, HP-UX only */
#include <x25/ccittproto.h>
#include <x25/x25.h>
#include <x25/x25addrstr.h>
#include <x25/x25codes.h>
#include <x25/x25hd_ioctl.h>
#include <x25/x25ioctls.h>
#include <x25/x25str.h>
#include <sys/ioctl.h>
#endif /* HPX25 */

/* C-Kermit X.3 / X.25 / X.29 / X.121 support functions */

/* (riehm: this list of functions isn't quite right for AIX) */

int shopad(int);
int shox25(int);
void initpad(void);
void setpad(CHAR *, int);
void readpad(CHAR *, int, CHAR *);
int qbitpkt(CHAR *, int);
void setqbit(void);
void resetqbit(void);
void breakact(void);
int pkx121(char *, CHAR *);
void x25oobh(int);
int x25diag(void);
int x25intr(char);
int x25reset(char, char);
int x25clear(void);
int x25stat(void);
int x25in(int, CHAR *);
int setpadp(void);
int setx25(void);
int x25xin(int, CHAR *);
int x25inl(CHAR *, int, int, CHAR);

#ifdef IBMX25
/* setup x25 */
ulong x25bind(int, char *, char *, int, int, int, ulong);
int x25call(int, char *, char *); /* connect to remote */
int x25unbind(int);               /* disconnect */
char *x25prim(int);               /* display primitives */
int x25local_nua(char *);         /* find local NUA */
#endif                            /* IBMX25 */

#endif /* ANYX25 */

/* CMU-OpenVMS/IP */

/* DEC TCP/IP for (Open)VMS, previously known as UCX */

/* SRI/TGV/Cisco/Process MultiNet, TCP/IP for VAX/VMS */

/* Wollongong TCP/IP for VAX/VMS */

/* Wollongong TCP/IP for AT&T Sys V */

#ifdef WOLLONGONG      /* WOLLONGONG implies TCPSOCKET */
#ifndef TCPSOCKET      /* Don't confuse WOLLONGONG */
#define TCPSOCKET      /* (which is for UNIX) with */
#endif /* TCPSOCKET */ /* WINTCP, which is for VMS! */
#endif                 /* WOLLONGONG */

#ifdef INTERLAN /* INTERLAN implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* INTERLAN */

/* Telnet protocol */

#ifdef TCPSOCKET /* TCPSOCKET implies TNCODE */
#ifndef TNCODE   /* Which means... */
#define TNCODE   /* Compile in telnet code */
#endif           /* TNCODE */

/*
   Platforms where we must call gethostname(buf,len) and then
   gethostbyname(buf) to get local IP address, rather than calling
   gethostbyname("").
*/
#ifndef CKGHNLHOST
#endif /* CKGHNLHOST */

#ifndef RLOGCODE /* What about Rlogin? */
#ifndef NORLOGIN
/*
  Rlogin can be enabled only for UNIX versions that have both SIGURG
  (SCO doesn't) and CK_TTGWSIZ (OSF/1 doesn't), so we don't assume that
  any others have these without verifying first.  Not that it really makes
  much difference since you can only use Rlogin if you are root...
*/
#ifdef __linux__
#define RLOGCODE
#else
#ifdef BSD44
#define RLOGCODE
#endif /* BSD44 */
#endif /* __linux__ */
#endif /* NORLOGIN */
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifdef TNCODE
/*
  Telnet local-echo buffer, used for saving up user data that can't be
  properly displayed and/or evaluated until pending Telnet negotiations are
  complete.  TTLEBUF is defined for platforms (like UNIX) where net i/o is
  done by the same routines that do serial i/o (in which case the relevant
  code goes into the ck?tio.c module, in the ttinc(), ttchk(), etc, routines);
  NETLETBUF is defined for platforms (like VMS) that use different APIs for
  network and serial i/o, and enables the copies of the same routines that
  are in ckcnet.c.
*/
#ifndef TTLEBUF
#ifdef UNIX
#define TTLEBUF
#else
#endif /* UNIX */
#endif /* TTLEBUF */

#ifndef NETLEBUF
#endif /* NETLEBUF */
#endif /* TNCODE */

#ifndef TCPSOCKET
#ifndef NO_DNS_SRV
#define NO_DNS_SRV
#endif /* NO_DNS_SRV */
#endif /* TCPSOCKET */

/* This is another TCPSOCKET section... */

#ifdef TCPSOCKET
#ifndef NETCONN /* TCPSOCKET implies NETCONN */
#define NETCONN
#endif /* NETCONN */

#ifndef NO_DNS_SRV
#ifdef NOLOCAL
#define NO_DNS_SRV
#endif /* NOLOCAL */
#endif /* NO_DNS_SRV */

#ifndef CK_DNS_SRV /* Use DNS SRV records to determine */
#ifndef NO_DNS_SRV /* host and ports */
#define CK_DNS_SRV
#endif /* NO_DNS_SRV */
#endif /* CK_DNS_SRV */

#ifndef NOLISTEN /* select() is required to support */
#ifndef SELECT   /* incoming connections. */
#define NOLISTEN
#endif /* SELECT */
#endif /* NOLISTEN */

/* BSD sockets library header files */

#ifdef UNIX /* UNIX section */

#ifdef SVR4
/*
  These suggested by Rob Healey, rhealey@kas.helios.mn.org, to avoid
  bugs in Berkeley compatibility library on Sys V R4 systems, but untested
  by me (fdc).  Remove this bit if it gives you trouble.
  (Later corrected by Marc Boucher <mboucher@iro.umontreal.ca> because
  bzero/bcopy are not argument-compatible with memset/memcpy|memmove.)
*/
#ifndef bzero
#define bzero(s, n) memset(s, 0, n)
#endif
#ifndef bcopy
#define bcopy(h, a, l) memmove(a, h, l)
#endif
#else  /* !SVR4 */
#endif /* SVR4 */

#ifdef INTERLAN /* Racal-Interlan TCP/IP */
#include <interlan/socket.h>
#include <interlan/il_types.h>
#include <interlan/telnet.h>
#include <interlan/il_errno.h>
#include <interlan/in.h>
#include <interlan/telnet.h> /* Why twice ? ? ? */
#else                        /* Not Interlan */
#include <sys/socket.h>
#ifdef WOLLONGONG
#include <sys/in.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h> /* Added June 2001 */
#endif                   /* WOLLONGONG */
#endif                   /* INTERLAN */

#include <netdb.h>
#ifndef INTERLAN
#ifdef WOLLONGONG
#define minor /* Do not include <sys/macros.h> */
#include <sys/inet.h>
#else
#include <arpa/inet.h>
#endif /* WOLLONGONG */
#endif /* INTERLAN */

/*
  Data type of the inet_addr() function...
  We define INADDRX if it is of type struct inaddr.
  If it is undefined, unsigned long is assumed.
  Look at <arpa/inet.h> to find out.  The following known cases are
  handled here.  Other systems that need it can be added here, or else
  -DINADDRX can be included in the CFLAGS on the cc command line.
*/
#ifndef NOINADDRX
#ifdef DU2 /* DEC Ultrix 2.0 */
#define INADDRX
#endif /* DU2 */
#endif /* NOINADDRX */

#else /* Not UNIX */

#endif /* UNIX */
#endif /* TCPSOCKET */

#ifndef NOINADDRX /* 301 - Needed for Solaris 10 and 11 */
#endif            /* NOINADDRX */

#ifdef NOINADDRX
#ifdef INADDRX
#undef INADDRX
#endif /* INADDRX */
#endif /* NOINADDRX */

#ifdef TCPSOCKET
#ifndef NOHADDRLIST
#ifndef HADDRLIST
#ifdef LINUX
#define HADDRLIST
#endif /* LINUX */
#endif /* HADDRLIST */
/* A system that defines h_addr as h_addr_list[0] should be HADDRLIST */
#ifndef HADDRLIST
#ifdef h_addr
#define HADDRLIST
#endif /* h_addr */
#endif /* HADDRLIST */
#endif /* NOHADDRLIST */
#endif /* TCPSOCKET */

#ifdef TNCODE /* If we're compiling telnet code... */
#ifndef IKS_OPTION
#ifndef NOIKSD
#define IKS_OPTION
#endif /* NOIKSD */
#endif /* IKS_OPTION */
#include "ckctel.h"
#else
extern int sstelnet;
#ifdef IKSD
#undef IKSD
#endif /* IKSD */
#ifndef NOIKSD
#define NOIKSD
#endif /* NOIKSD */
#ifdef IKS_OPTION
#undef IKS_OPTION
#endif /* IKS_OPTION */
#endif /* TNCODE */

#ifndef NOTCPOPTS
/*
  Automatically define NOTCPOPTS for configurations where they can't be
  used at runtime or cause too much trouble at compile time.
*/
#endif /* NOTCPOPTS */

#ifdef NOTCPOPTS
#ifdef TCP_NODELAY
#undef TCP_NODELAY
#endif /* TCP_NODELAY */
#ifdef SO_LINGER
#undef SO_LINGER
#endif /* SO_LINGER */
#ifdef SO_KEEPALIVE
#undef SO_KEEPALIVE
#endif /* SO_KEEPALIVE */
#ifdef SO_SNDBUF
#undef SO_SNDBUF
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
#undef SO_RCVBUF
#endif /* SO_RCVBUF */
#endif /* NOTCPOPTS */

/* This function is declared even when TCPSOCKET is not available */
char *ckgetpeer(void);

#ifdef TCPSOCKET
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
int no_delay(int, int);
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
int keepalive(int, int);
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
int ck_linger(int, int, int);
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
int sendbuf(int, int);
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
int recvbuf(int, int);
#endif /* SO_RCVBUF */
#ifdef SO_DONTROUTE
int dontroute(int, int);
#endif /* SO_DONTROUTE */
#endif /* SOL_SOCKET */
int getlocalipaddr(void);
int getlocalipaddrs(char *, int, int);
char *ckgetfqhostname(char *);
struct hostent *ck_copyhostent(struct hostent *);
char *ckname2addr(char *);
char *ckaddr2name(char *);

/* AIX */

#endif /* TCPSOCKET */

#ifdef RLOGCODE
#ifndef CK_TTGWSIZ
SORRY_RLOGIN_REQUIRES_TTGWSIZ_see_ckcplm
    .doc
#endif /* CK_TTGWSIZ */
#endif /* RLOGCODE */

#ifdef CK_NAWS
#ifndef CK_TTGWSIZ
        SORRY_CK_NAWS_REQUIRES_TTGWSIZ_see_ckcplm.doc
#endif /* CK_TTGWSIZ */
#endif /* CK_NAWS */

#ifndef PF_INET
#ifdef AF_INET
#define PF_INET AF_INET
#endif /* AF_INET */
#endif /* PF_INET */

#ifndef IPPORT_ECHO
#define IPPORT_ECHO 7
#endif /* IPPORT_ECHO */

#ifdef CK_DNS_SRV
    int locate_srv_dns(char *host, char *service, char *protocol,
                       struct sockaddr **addr_pp, int *naddrs);
#endif /* CK_DNS_SRV */

#ifndef NOHTTP
int http_open(char *, char *, int, char *, int, char *);
int http_reopen(void);
int http_close(void);
int http_get(char *, char **, char *, char *, char, char *, char *, int);
int http_head(char *, char **, char *, char *, char, char *, char *, int);
int http_put(char *, char **, char *, char *, char *, char, char *, char *,
             char *, int);
int http_delete(char *, char **, char *, char *, char, char *);
int http_connect(int, char *, char **, char *, char *, char, char *);
int http_post(char *, char **, char *, char *, char *, char, char *, char *,
              char *, int);
int http_index(char *, char **, char *, char *, char, char *, char *, int);
int http_inc(int);
int http_isconnected(void);

extern char *tcp_http_proxy;      /* Name[:port] of http proxy server */
extern int tcp_http_proxy_errno;  /* Return value from server */
extern char *tcp_http_proxy_user; /* Name of user for authentication */
extern char *tcp_http_proxy_pwd;  /* Password of user */
#endif                            /* NOHTTP */

#ifdef TCPSOCKET

/* Type needed as 5th argument (length) to get/setsockopt() */

#ifdef TRU64
/* They say it themselves - this does not conform to standards */
#define socklen_t int
#else
#endif /* TRU64 */

#ifndef SOCKOPT_T
#ifdef CK_64BIT
#define SOCKOPT_T socklen_t
#endif /* CK_64BIT */
#endif /* SOCKOPT_T */

#ifndef SOCKOPT_T
#define SOCKOPT_T int
#ifdef MACOSX10
#undef SOCKOPT_T
#define SOCKOPT_T unsigned int
#else
#endif /* MACOSX10 */
#endif /* SOCKOPT_T */

/* Ditto for getsockname() */

#ifndef GSOCKNAME_T
#ifdef CK_64BIT
#define GSOCKNAME_T socklen_t
#endif /* CK_64BIT */
#endif /* GSOCKNAME_T */

#ifndef GSOCKNAME_T
#define GSOCKNAME_T int
#ifdef MACOSX10
#undef GSOCKNAME_T
#define GSOCKNAME_T unsigned int
#else
#endif /* MACOSX10 */
#endif /* GSOCKNAME_T */

#endif /* TCPSOCKET */

#ifdef MACOSX10
#ifdef bcopy
#undef bcopy
#endif
#ifdef bzero
#undef bzero
#endif
#endif /* MACOSX10 */

#endif /* CKCNET_H */
