#define FTP_TIMEOUT

/*  C K C F T P  --  FTP Client for C-Kermit  */

char *ckftpv = "FTP Client, 10.0.281, 18 Sep 2023";

/*
  Authors:
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City
    Frank da Cruz <fdc@columbia.edu>,
      The Kermit Project, Columbia University.
    David Goodwin, New Zealand

  Copyright (C) 2000, 2023
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Portions of conditionally included code Copyright Regents of the
    University of California and The Stanford SRP Authentication Project;
    see notices below.
*/

/*
  Pending...

  . Implement recursive NLST downloads by trying to CD to each filename.
    If it works, it's a directory; if not, it's a file -- GET it.  But
    that won't work with servers like wu-ftpd that don't send directory
    names.  Recursion with MLSD is done.

  . Make syslog entries for session?  Files?

  . Messages are printed to stdout and stderr in random fashion.  We should
    either print everything to stdout, or else be systematic about when
    to use stderr.

  . Implement mail (MAIL, MLFL, MSOM, etc) if any servers support it.

  . Adapt to VMS.  Big job because of its record-oriented file system.
    RMS programmer required.  There are probably also some VMS TCP/IP
    product-specific wrinkles, e.g. attribute preservation in VMS-to-VMS
    transfers using special options for Multinet or other FTP servers
    (find out about STRU VMS).
*/

/*
  Quick FTP command reference:

  RFC765 (1980) and earlier:
    MODE  S(tream), B(lock), C(ompressed)
    STRU  F(ILE), R(ECORD), P(AGE)
    TYPE  A(SCII) <format>,  E(BCDIC) <format>, I(MAGE), L(OCAL) <bytesize>
    PORT  - Port
    PASV  - Passive mode
    USER  - User
    PASS  - Password
    ACCT  - Account
    CWD   - Change Working Directory
    REIN  - Logout but not disconnect
    QUIT  - Bye
    RETR  - Retreive
    STOR  - Store
    APPE  - Append
    ALLO  - Allocate
    REST  - Restart
    RNFR  - Rename from
    RNTO  - Rename to
    ABOR  - Cancel
    DELE  - Delete
    LIST  - Directory
    NLST  - Name List
    SITE  - Site parameters or commands
    STAT  - Status
    HELP  - Help
    NOOP  - Noop

  RFC959 (1985):
    CDUP  - Change to Parent Directory
    SMNT  - Structure Mount
    STOU  - Store Unique
    RMD   - Remove Directory
    MKD   - Make Directory
    PWD   - Print Directory
    SYST  - System

  RFC2389 (1998):
    FEAT  - List Features (done)
    OPTS  - Send options (done)

  RFC2640 (1999):
    LANG  - Specify language for messages (not done)

  Pending (Internet Drafts):
    SIZE  - File size (done)
    MDTM  - File modification date-time (done)
    MLST  - File name and attribute list (single file) (not done)
    MLSD  - File list with attributes (multiple files) (done)
    MAIL, MLFL, MSOM - mail delivery (not done)

  Alphabetical syntax list:
    ABOR <CRLF>
    ACCT <SP> <account-information> <CRLF>
    ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>
    APPE <SP> <pathname> <CRLF>
    CDUP <CRLF>
    CWD  <SP> <pathname> <CRLF>
    DELE <SP> <pathname> <CRLF>
    FEAT <CRLF>
    HELP [<SP> <string>] <CRLF>
    LANG [<SP> <language-tag> ] <CRLF>
    LIST [<SP> <pathname>] <CRLF>
    MKD  <SP> <pathname> <CRLF>
    MLSD [<SP> <pathname>] <CRLF>
    MLST [<SP> <pathname>] <CRLF>
    MODE <SP> <mode-code> <CRLF>
    NLST [<SP> <pathname-or-wildcard>] <CRLF>
    NOOP <CRLF>
    OPTS <SP> <commandname> [ <SP> <command-options> ] <CRLF>
    PASS <SP> <password> <CRLF>
    PASV <CRLF>
    PORT <SP> <host-port> <CRLF>
    PWD  <CRLF>
    QUIT <CRLF>
    REIN <CRLF>
    REST <SP> <marker> <CRLF>
    RETR <SP> <pathname> <CRLF>
    RMD  <SP> <pathname> <CRLF>
    RNFR <SP> <pathname> <CRLF>
    RNTO <SP> <pathname> <CRLF>
    SITE <SP> <string> <CRLF>
    SIZE <SP> <pathname> <CRLF>
    SMNT <SP> <pathname> <CRLF>
    STAT [<SP> <pathname>] <CRLF>
    STOR <SP> <pathname> <CRLF>
    STOU <CRLF>
    STRU <SP> <structure-code> <CRLF>
    SYST <CRLF>
    TYPE <SP> <type-code> <CRLF>
    USER <SP> <username> <CRLF>
*/
/* clang-format off */
#include "ckcdeb.h"
/* clang-format on */

#ifndef NOFTP    /* NOFTP  = no FTP */
#ifndef SYSFTP   /* SYSFTP = use external ftp client */
#ifdef TCPSOCKET /* Build only if TCP/IP included */
#define CKCFTP_C

/* Note: much of the following duplicates what was done in ckcdeb.h */
/* but let's not mess with it unless it causes trouble. */

#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include "ckcsig.h"
#include <sys/stat.h>
#include <ctype.h>

#include <errno.h> /* Error number symbols */

#ifndef NOTIMEH
#include <time.h>
#endif /* NOTIMEH */
#ifndef EPIPE
#define EPIPE 32 /* Broken pipe error */
#endif           /* EPIPE */

/* Kermit includes */

#include "ckcasc.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"
#include "ckcnet.h" /* Includes ckctel.h */
#include "ckctel.h" /* (then why include it again?) */
#include "ckcxla.h"

/*
  How to get the struct timeval definition so we can call select().  The
  xxTIMEH symbols are defined in ckcdeb.h, overridden in various makefile
  targets.  The problem is: maybe we have already included some header file
  that defined struct timeval, and maybe we didn't.  If we did, we don't want
  to include another header file that defines it again or the compilation will
  fail.  If we didn't, we have to include the header file where it's defined.
  But in some cases even that won't work because of strict POSIX constraints
  or somesuch, or because this introduces other conflicts (e.g. struct tm
  multiply defined), in which case we have to define it ourselves, but this
  can work only if we didn't already encounter a definition.
*/
#ifndef DCLTIMEVAL
#endif /* DCLTIMEVAL */

#ifdef DCLTIMEVAL
/* Also maybe in some places the elements must be unsigned... */
struct timeval {
  long tv_sec;
  long tv_usec;
};
#else /* !DCLTIMEVAL */
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
#endif /* DCLTIMEVAL */

/* 2010-03-09 SMS.  VAX C needs help to find "sys".  It's easier not to try. */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#ifndef NOSETTIME

#ifdef SYSUTIMEH
#include <sys/utime.h>
#else
#ifdef UTIMEH
#include <utime.h>
#define SYSUTIMEH
#endif /* UTIMEH */
#endif /* SYSUTIMEH */
#endif /* NOSETTIME */

#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */

#ifndef INADDR_NONE /* 2010-03-29 */
#define INADDR_NONE -1
#endif /* INADDR_NONE */

/* select() dialects... */

#ifdef UNIX
#define BSDSELECT /* BSD select() syntax/semantics */
#ifndef FD_SETSIZE
#define FD_SETSIZE 128
#endif /* FD_SETSIZE */

#else
#endif /* UNIX */

/* Other select() peculiarities */

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

#ifndef NOHTTP
extern char *tcp_http_proxy; /* Name[:port] of http proxy server */
extern int tcp_http_proxy_errno;
extern char *tcp_http_proxy_user;
extern char *tcp_http_proxy_pwd;
extern char *tcp_http_proxy_agent;
#define HTTPCPYL 1024
static char proxyhost[HTTPCPYL];
#endif                 /* NOHTTP */
int ssl_ftp_proxy = 0; /* FTP over SSL/TLS Proxy Server */

/* Feature selection */

#ifndef USE_SHUTDOWN
/*
  We don't use shutdown() because (a) we always call it just before close()
  so it's redundant and unnecessary, and (b) it introduces a long pause on
  some platforms like SV/68 R3.
*/
/* #define USE_SHUTDOWN */
#endif /* USE_SHUTDOWN */

#ifndef NORESEND
#ifndef NORESTART /* Restart / recover */
#ifndef FTP_RESTART
#define FTP_RESTART
#endif /* FTP_RESTART */
#endif /* NORESTART */
#endif /* NORESEND */

#ifndef NOUPDATE /* Update mode */
#ifndef DOUPDATE
#define DOUPDATE
#endif /* DOUPDATE */
#endif /* NOUPDATE */

#ifndef UNICODE /* Unicode required */
#ifndef NOCSETS /* for charset translation */
#define NOCSETS
#endif /* NOCSETS */
#endif /* UNICODE */

#ifndef HAVE_MSECS /* Millisecond timer */
#ifdef UNIX
#ifdef GFTIMER
#define HAVE_MSECS
#endif /* GFTIMER */
#endif /* UNIX */
#endif /* HAVE_MSECS */

#ifdef PIPESEND /* PUT from pipe */
#ifndef PUTPIPE
#define PUTPIPE
#endif /* PUTPIPE */
#endif /* PIPESEND */

#ifndef NOSPL /* PUT from array */
#ifndef PUTARRAY
#define PUTARRAY
#endif /* PUTARRAY */
#endif /* NOSPL */

/* Security... */

/* FTP_SECURITY is defined if any of the above is selected */

/* getreply() function codes */

#define GRF_AUTH 1 /* Reply to AUTH command */
#define GRF_FEAT 2 /* Reply to FEAT command */

/* Operational definitions */

#define DEF_VBM 0    /* Default verbose mode */
/* #define SETVBM */ /* (see getreply) */

#define URL_ONEFILE /* GET, not MGET, for FTP URL */

#define FTP_BUFSIZ 10240 /* Max size for FTP cmds & replies */
#define SRVNAMLEN 32     /* Max length for server type name */
#define PWDSIZ 256
#define PASSBUFSIZ 256
#define PROMPTSIZ 256

#ifndef MGETMAX /* Max operands for MGET command */
#define MGETMAX 1000
#endif /* MGETMAX */

/*
  Amount of growth from cleartext to ciphertext.  krb_mk_priv adds this
  number bytes.  Must be defined for each auth type.
  GSSAPI appears to add 52 bytes, but I'm not sure it is a constant--hartmans
  3DES requires 56 bytes.  Lets use 96 just to be sure.
*/

#ifndef FUDGE_FACTOR /* In case no auth types define it */
#define FUDGE_FACTOR 0
#endif /* FUDGE_FACTOR */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif /* MAXHOSTNAMELEN */
#define MAX_DNS_NAMELEN (15 * (MAXHOSTNAMELEN + 1) + 1)

/* Fascist compiler toadying */

#ifndef SENDARG2TYPE
#define SENDARG2TYPE char *
#endif /* SENDARG2TYPE */

/* Common text messages */

static char *nocx = "?No FTP control connection\n";

static char *fncnam[] = {"rename", "overwrite",    "backup",
                         "append", "discard",      "ask",
                         "update", "dates-differ", ""};

/* Macro definitions */

/* Used to speed up text-mode PUTs */
#define zzout(fd, c)                                                           \
  ((fd < 0) ? (-1)                                                             \
            : ((nout >= ucbufsiz) ? (zzsend(fd, c)) : (ucbuf[nout++] = c)))

#define CHECKCONN()                                                            \
  if (!connected) {                                                            \
    printf("%s\n", nocx);                                                      \
    return (-9);                                                               \
  }

/* Externals */

#ifdef CK_URL
extern struct urldata g_url;
#endif /* CK_URL */

#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer; /* Regular Kermit file i/o */
#else
extern char zinbuffer[], zoutbuffer[];
#endif /* DYNAMIC */
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt, zobufsize, fncact;

#ifdef CK_TMPDIR
extern int f_tmpdir;  /* Directory changed temporarily */
extern char savdir[]; /* For saving current directory */
extern char *dldir;
#endif /* CK_TMPDIR */

extern char *rfspec, *sfspec, *srfspec, *rrfspec; /* For WHERE command */

extern xx_strp xxstring;
extern struct keytab onoff[], txtbin[], rpathtab[];
extern int nrpathtab, xfiletype, patterns, gnferror, moving, what, pktnum;
extern int success, nfils, sndsrc, quiet, nopush, recursive, inserver, binary;
extern int filepeek, nscanfile, fsecs, xferstat, xfermode, lastxfer, tsecs;
extern int backgrd, spackets, rpackets, spktl, rpktl, xaskmore, cmd_rows;
extern int nolinks, msgflg, keep;
extern CK_OFF_T fsize, ffc, tfc, sendstart, sndsmaller, sndlarger, rs_len;
extern long filcnt, xfsecs, tfcps, cps, oldcps;

#ifdef FTP_TIMEOUT
int ftp_timed_out = 0;
long ftp_timeout = 0;
#endif /* FTP_TIMEOUT */

#ifdef GFTIMER
extern CKFLOAT fptsecs, fpfsecs, fpxfsecs;
#else
extern long xfsecs;
#endif /* GFTIMER */

extern char filnam[], *filefile, myhost[];
extern char *snd_move, *rcv_move, *snd_rename, *rcv_rename;
extern int g_skipbup, skipbup, sendmode;
extern int g_displa, fdispla, displa;

#ifdef LOCUS
extern int locus, autolocus;
#endif /* LOCUS */

#ifndef NOCSETS
extern int nfilc, dcset7, dcset8, fileorder;
extern struct csinfo fcsinfo[];
extern struct keytab fcstab[];
extern int fcharset;
#endif /* NOCSETS */

extern char sndbefore[], sndafter[], *sndexcept[]; /* Selection criteria */
extern char sndnbefore[], sndnafter[], *rcvexcept[];
extern CHAR feol;

extern char *remdest;
extern int remfile, remappd, rempipe;

#ifndef NOSPL
extern int cmd_quoting;
#ifdef PUTARRAY
extern int sndxlo, sndxhi, sndxin;
extern char sndxnam[];
extern char **a_ptr[]; /* Array pointers */
extern int a_dim[];    /* Array dimensions */
#endif                 /* PUTARRAY */
#endif                 /* NOSPL */

#ifndef NOMSEND /* MPUT and ADD SEND-LIST lists */
extern char *msfiles[];
extern int filesinlist;
extern struct filelist *filehead;
extern struct filelist *filetail;
extern struct filelist *filenext;
extern int addlist;
extern char fspec[]; /* Most recent filespec */
extern int fspeclen; /* Length of fspec[] buffer */
#endif               /* NOMSEND */

extern int pipesend;
#ifdef PIPESEND
extern char *sndfilter, *rcvfilter;
#endif /* PIPESEND */

#ifdef CKROOT
extern int ckrooterr;
#endif /* CKROOT */

#ifdef DCMDBUF
extern char *atmbuf; /* Atom buffer (malloc'd) */
extern char *cmdbuf; /* Command buffer (malloc'd) */
extern char *line;   /* Big string buffer #1 */
extern char *tmpbuf; /* Big string buffer #2 */
#else
extern char atmbuf[]; /* The same, but static */
extern char cmdbuf[];
extern char line[];
extern char tmpbuf[];
#endif /* DCMDBUF */

extern char *cmarg, *cmarg2, **cmlist; /* For setting up file lists */

/* Public variables declared here */

#ifdef NOXFER
int ftpget = 1; /* GET/PUT/REMOTE orientation FTP */
#else
int ftpget = 2; /* GET/PUT/REMOTE orientation AUTO */
#endif              /* NOXFER */
int ftpcode = -1;   /* Last FTP response code */
int ftp_cmdlin = 0; /* FTP invoked from command line */
int ftp_fai = 0;    /* FTP failure count */
int ftp_deb = 0;    /* FTP debugging */
int ftp_dis = -1;   /* FTP display style */
int ftp_log = 1;    /* FTP Auto-login */
int sav_log = -1;
int ftp_action = 0;         /* FTP action from command line */
int ftp_dates = 1;          /* Set file dates from server */
int ftp_xfermode = XMODE_A; /* FTP-specific transfer mode */

char ftp_reply_str[FTP_BUFSIZ] = "";      /* Last line of previous reply */
char ftp_srvtyp[SRVNAMLEN] = {NUL, NUL};  /* Server's system type */
char ftp_user_host[MAX_DNS_NAMELEN] = ""; /* FTP hostname specified by user */
char *ftp_host = NULL;                    /* FTP hostname */
char *ftp_logname = NULL;                 /* FTP username */
char *ftp_rdir = NULL;                    /* Remote directory from cmdline */
char *ftp_apw = NULL;                     /* Anonymous password */

/* Definitions and typedefs needed for prototypes */

/*
  #define sig_t my_sig_t

  I don't understand the statement above, which has been in this code going
  back to at least C-Kermit 8.0, because my_sig_t is not defined anywhere.
  And yet sig_t is used below with no complaint, no matter whether the the
  above #define is commented out or not.  However, if I #define sig_t SIGTYP
  (which is what it should be according to ckcdeb.h), all hell breaks loose.
  Same if I replace all references to sig_t by SIGTYPE.  On the other hand, if
  I remove the sig_t definition, there is no complaint, so where is the sig_t
  definition coming from?  I find this in Ubuntu signal.h:

    <comment> 4.4 BSD uses the name 'sig_t' for this. </comment>
    typedef __sighandler_t sig_t;

  In NetBSD I find this in sys/signal.h:

    typedef    void (*sig_t)(int);

  Can we really count on sig_t being defined in some nook or cranny
  on every single Unix, VMS, and Windows system?

  Jeff Altman says to use sighandler_t, which is ok on Ubuntu but
  not on (say) NetBSD.  So I can't do this:

#ifndef sig_t
#define sig_t sighandler_t
#endif

  I think the only alternative is to leave my_sig_t undefined and then
  see who squawks.  Since the previous 'my_sig_t' definition apparently
  had no effect, I'm hoping there will be no change in behavior after
  commenting it out.    -fdc Fri Jun 23 06:53:23 2023

  P.S. All this is aside from the fact that signal() has long since been
  "deprecated" in favor of sigaction(), defined in POSIX.1-1988; see:

    https://man7.org/linux/man-pages/man2/signal.2.html
    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html

  but C-Kermit can't be switched over because it has to build and run on
  pre-POSIX operating systems that don't have sigaction(0) (despite the fact
  that C-Kermit's ckupty.c function ptyint_vhangup() calls it...  how did
  *that* happen?).
*/

typedef void (*sig_t)(int);

/* Made this global static -fdc 21 June 2023 */
/* It's used in many ckcftp.c routines but wasn't declared in all of them */
static sig_t oldintr;

/* Prototypes for static functions defined in ckcftp.c */

static void cancel_remote(int);
static void changetype(int, int);
static void dbtime(char *, struct tm *);
static void ftscreen(int, char, CK_OFF_T, char *);
static char *ftp_hookup(char *, int, int);
static char *radix_error(int);
static char *strval(char *, char *);
static int check_data_connection(int, int);
static int chkmodtime(char *, char *, int);
static int dataconn(char *);
static int doftpcwd(char *, int);
static int doftpdir(int);
static int doftpxmkd(char *, int);
static int ftp_login(char *);
static int ftp_rename(char *, char *);
static int ftp_umask(char *);
static int ftp_user(char *, char *, char *);
static int ftpcmd(char *, char *, int, int, int);
static int getfile(char *, char *, int, int, char *, int, int, int);
static int getreply(int, int, int, int, int);
static int ispathsep(int);
static int looping_read(int, register char *, register int);
static int looping_write(int, register const char *, int);
static int openftp(char *, int);
static int putfile(int, char *, char *, int, int, char *, char *, char *, int,
                   int, int, int, int, int, int);
static int recvrequest(char *, char *, char *, char *, int, int, char *, int,
                       int, int);
static int secure_flush(int);
static int secure_getbyte(int, int);
static int secure_read(int fd, char *, int);
static int sendrequest(char *, char *, char *, int, int, int, int);
static int syncdir(char *, int);
static int tmcompare(struct tm *, struct tm *);
static int xlatec(int, int, int, int);
static void cancelrecv(int);
static void cancelsend(int);
static void cmdcancel(int);

/* Static global variables */

static char ftpsndbuf[FTP_BUFSIZ + 64];

static char *fts_sto = NULL;

static int ftpsndret = 0;
static struct _ftpsnd {
  sig_t oldintr, oldintp;
  int reply;
  int incs, outcs;
  char *cmd, *local, *remote;
  int bytes;
  int restart;
  int xlate;
  char *lmode;
} ftpsnd;

/*
  This is just a first stab -- these strings should match how the
  corresponding FTP servers identify themselves.
*/
#ifdef UNIX
static char *myostype = "UNIX";
#else
static char *myostype = "UNSUPPORTED";
#endif /* UNIX */

static int noinit = 0;      /* Don't send REST, STRU, MODE */
static int alike = 0;       /* Client/server like platforms */
static int local = 1;       /* Shadows Kermit global 'local' */
static int dout = -1;       /* Data connection file descriptor */
static int dpyactive = 0;   /* Data transfer is active */
static int globaldin = -1;  /* Data connection f.d. */
static int out2screen = 0;  /* GET output is to screen */
static int forcetype = 0;   /* Force text or binary mode */
static int cancelfile = 0;  /* File canceled */
static int cancelgroup = 0; /* Group canceled */
static int anonymous = 0;   /* Logging in as anonymous */
static int loggedin = 0;    /* Logged in (or not) */
static int puterror = 0;    /* What to do on PUT error */
static int geterror = 0;    /* What to do on GET error */
static int rfrc = 0;        /* remote_files() return code */
static int okrestart = 0;   /* Server understands REST */
static int printlines = 0;  /* getreply()should print data lines */
static int haveurl = 0;     /* Invoked by command-line FTP URL */
static int mdtmok = 1;      /* Server supports MDTM */
static int sizeok = 1;
static int featok = 1;
static int mlstok = 1;
static int stouarg = 1;
static int typesent = 0;
static int havesigint = 0;
static long havetype = 0;
static CK_OFF_T havesize = (CK_OFF_T)-1;
static char *havemdtm = NULL;
static int mgetmethod = 0; /* NLST or MLSD */
static int mgetforced = 0;

static int i, /* j, k, */ x, y, z; /* Volatile temporaries */
static int c0, c1;                 /* Temp variables for characters */

static char putpath[CKMAXPATH + 1] = {NUL, NUL};
static char asnambuf[CKMAXPATH + 1] = {NUL, NUL};

#define RFNBUFSIZ 4096 /* Remote filename buffer size */

static unsigned int maxbuf = 0, actualbuf = 0;
static CHAR *ucbuf = NULL;
static int ucbufsiz = 0;
static unsigned int nout = 0; /* Number of chars in ucbuf */

static jmp_buf recvcancel;
static jmp_buf sendcancel;

#ifdef NOT_USED
static jmp_buf jcancel;
#endif /* NOT_USED */

#ifdef FTP_PROXY
static jmp_buf ptcancel;

static int ptabflg = 0;
#endif /* FTP_PROXY */

/* Protection level symbols */

#define FPL_CLR 1 /* Clear */
#define FPL_SAF 2 /* Safe */
#define FPL_PRV 3 /* Private */
#define FPL_CON 4 /* Confidential */

/* Symbols for file types returned by MLST/MLSD */

#define FTYP_FILE 1 /* Regular file */
#define FTYP_DIR 2  /* Directory */
#define FTYP_CDIR 3 /* Current directory */
#define FTYP_PDIR 4 /* Parent directory */

/* File type symbols keyed to the file-type symbols from ckcker.h */

#define FTT_ASC XYFT_T /* ASCII (text) */
#define FTT_BIN XYFT_B /* Binary (image) */
#define FTT_TEN XYFT_X /* TENEX (TOPS-20) */

/* Server feature table - sfttab[0] > 0 means server supports FEAT and OPTS */

static int sfttab[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define SFT_AUTH 1 /* FTP server feature codes */
#define SFT_LANG 2
#define SFT_MDTM 3
#define SFT_MLST 4
#define SFT_PBSZ 5
#define SFT_PROT 6
#define SFT_REST 7
#define SFT_SIZE 8
#define SFT_TVFS 9
#define SFT_UTF8 10

#define CNV_AUTO 2 /* FTP filename conversion */
#define CNV_CNV 1
#define CNV_LIT 0

/* SET FTP values */

static int             /* SET FTP values... */
    ftp_aut = 1,       /* Auto-authentication */
    ftp_cpl = FPL_CLR, /* Command protection level */
    ftp_dpl = FPL_CLR, /* Data protection level */
#ifdef FTP_PROXY
    ftp_prx = 0,        /* Use proxy */
#endif                  /* FTP_PROXY */
    sav_psv = -1,       /* For saving passive mode */
    ftp_psv = 1,        /* Passive mode */
    ftp_spc = 1,        /* Send port commands */
    ftp_typ = FTT_ASC,  /* Type */
    get_auto = 1,       /* Automatic type switching for GET */
    tenex = 0,          /* Type is Tenex */
    ftp_usn = 0,        /* Unique server names */
    ftp_prm = 0,        /* Permissions */
    ftp_cnv = CNV_AUTO, /* Filename conversion (2 = auto) */
    ftp_vbm = DEF_VBM,  /* Verbose mode */
    ftp_vbx = DEF_VBM,  /* Sticky version of same */
    ftp_err = 0,        /* Error action */
    ftp_fnc = -1;       /* Filename collision action */

static int
#ifdef NOCSETS
    ftp_csr = -1, /* Remote (server) character set */
#else
    ftp_csr = FC_UTF8,
#endif            /* NOCSETS */
    ftp_xla = 0;  /* Character-set translation on/off */
int ftp_csx = -1, /* Remote charset currently in use */
    ftp_csl = -1; /* Local charset currently in use */

static int g_ftp_typ = FTT_ASC; /* For saving and restoring ftp_typ */

char *ftp_nml = NULL;          /* /NAMELIST */
char *ftp_tmp = NULL;          /* Temporary string */
static char *ftp_acc = NULL;   /* Account string */
static char *auth_type = NULL; /* Authentication type */
static char *srv_renam = NULL; /* Server-rename string */
FILE *fp_nml = NULL;           /* Namelist file pointer */

static int csocket = -1;                  /* Control socket */
static int connected = 0;                 /* Connected to FTP server */
/* static unsigned short ftp_port = 0; */ /* FTP port */
/* static int ftp_port = 0; */            /* SMS 2007/02/15 */
static int ftp_port = 0;                  /* fdc 2007/08/30 */
#ifdef FTPHOST
static int hostcmd = 0; /* Has HOST command been sent */
#endif                  /* FTPHOST */
static int form, mode, stru, bytesize, curtype = FTT_ASC;
static char bytename[8];

/* For parsing replies to FTP server command */
static char *reply_parse, reply_buf[FTP_BUFSIZ], *reply_ptr;

#ifdef FTP_PROXY
static int proxy, unix_proxy
#endif /* FTP_PROXY */

    static char pasv[64]; /* Passive-mode port */
static int passivemode = 0;
static int sendport = 0;
static int servertype = 0; /* FTP server's OS type */

static int testing = 0;
static char ftpcmdbuf[FTP_BUFSIZ];

/* Macro definitions */

#define UC(b) ckitoa(((int)b) & 0xff)
#define nz(x) ((x) == 0 ? 1 : (x))

/* Command tables and definitions */

#define FTP_ACC 1 /* FTP command keyword codes */
#define FTP_APP 2
#define FTP_CWD 3
#define FTP_CHM 4
#define FTP_CLS 5
#define FTP_DEL 6
#define FTP_DIR 7
#define FTP_GET 8
#define FTP_IDL 9
#define FTP_MDE 10
#define FTP_MDI 11
#define FTP_MGE 12
#define FTP_MKD 13
#define FTP_MOD 14
#define FTP_MPU 15
#define FTP_OPN 16
#define FTP_PUT 17
#define FTP_PWD 18
#define FTP_RGE 19
#define FTP_REN 20
#define FTP_RES 21
#define FTP_HLP 22
#define FTP_RMD 23
#define FTP_STA 24
#define FTP_SIT 25
#define FTP_SIZ 26
#define FTP_SYS 27
#define FTP_UMA 28
#define FTP_GUP 29
#define FTP_USR 30
#define FTP_QUO 31
#define FTP_TYP 32
#define FTP_FEA 33
#define FTP_OPT 34
#define FTP_CHK 35
#define FTP_VDI 36
#define FTP_ENA 37
#define FTP_DIS 38
#define FTP_REP 39

struct keytab gprtab[] = {/* GET-PUT-REMOTE keywords */
                          {"auto", 2, 0},
                          {"ftp", 1, 0},
                          {"kermit", 0, 0}};

static struct keytab qorp[] = {
    /* QUIT or PROCEED keywords */
    {"proceed", 0, 0}, /* 0 = proceed */
    {"quit", 1, 0}     /* 1 = quit */
};

static struct keytab ftpcmdtab[] = {/* FTP command table */
                                    {"account", FTP_ACC, 0},
                                    {"append", FTP_APP, 0},
                                    {"bye", FTP_CLS, 0},
                                    {"cd", FTP_CWD, 0},
                                    {"cdup", FTP_GUP, 0},
                                    {"check", FTP_CHK, 0},
                                    {"chmod", FTP_CHM, 0},
                                    {"close", FTP_CLS, 0},
                                    {"cwd", FTP_CWD, CM_INV},
                                    {"delete", FTP_MDE, 0},
                                    {"directory", FTP_DIR, 0},
                                    {"disable", FTP_DIS, 0},
                                    {"enable", FTP_ENA, 0},
                                    {"features", FTP_FEA, 0},
                                    {"get", FTP_GET, 0},
                                    {"help", FTP_HLP, 0},
                                    {"idle", FTP_IDL, 0},
                                    {"login", FTP_USR, CM_INV},
                                    {"mdelete", FTP_MDE, CM_INV},
                                    {"mget", FTP_MGE, 0},
                                    {"mkdir", FTP_MKD, 0},
                                    {"modtime", FTP_MOD, 0},
                                    {"mput", FTP_MPU, 0},
                                    {"open", FTP_OPN, 0},
                                    {"opt", FTP_OPT, CM_INV | CM_ABR},
                                    {"opts", FTP_OPT, CM_INV},
                                    {"options", FTP_OPT, 0},
                                    {"put", FTP_PUT, 0},
                                    {"pwd", FTP_PWD, 0},
                                    {"quit", FTP_CLS, CM_INV},
                                    {"quote", FTP_QUO, 0},
                                    {"reget", FTP_RGE, 0},
                                    {"rename", FTP_REN, 0},
                                    {"reput", FTP_REP, 0},
                                    {"resend", FTP_REP, CM_INV},
                                    {"reset", FTP_RES, 0},
                                    {"rmdir", FTP_RMD, 0},
                                    {"send", FTP_PUT, CM_INV},
                                    {"site", FTP_SIT, 0},
                                    {"size", FTP_SIZ, 0},
                                    {"status", FTP_STA, 0},
                                    {"system", FTP_SYS, 0},
                                    {"type", FTP_TYP, 0},
                                    {"umask", FTP_UMA, 0},
                                    {"up", FTP_GUP, CM_INV},
                                    {"user", FTP_USR, 0},
                                    {"vdirectory", FTP_VDI, 0},
                                    {"", 0, 0}};
static int nftpcmd = (sizeof(ftpcmdtab) / sizeof(struct keytab)) - 1;

#define OPN_ANO 1 /* FTP OPEN switch codes */
#define OPN_PSW 2
#define OPN_USR 3
#define OPN_ACC 4
#define OPN_ACT 5
#define OPN_PSV 6
#define OPN_TLS 7
#define OPN_NIN 8
#define OPN_NOL 9

static struct keytab ftpswitab[] = {/* FTP command switches */
                                    {"/account", OPN_ACC, CM_ARG},
                                    {"/active", OPN_ACT, 0},
                                    {"/anonymous", OPN_ANO, 0},
                                    {"/noinit", OPN_NIN, 0},
                                    {"/nologin", OPN_NOL, 0},
                                    {"/passive", OPN_PSV, 0},
                                    {"/password", OPN_PSW, CM_ARG},
                                    {"/user", OPN_USR, CM_ARG},
                                    {"", 0, 0}};
static int nftpswi = (sizeof(ftpswitab) / sizeof(struct keytab)) - 1;

/* FTP { ENABLE, DISABLE } items */

#define ENA_FEAT 1
#define ENA_MDTM 2
#define ENA_MLST 3
#define ENA_SIZE 4
#define ENA_AUTH 5

static struct keytab ftpenatab[] = {{"AUTH", ENA_AUTH, 0},
                                    {"FEAT", ENA_FEAT, 0},
                                    {"MDTM", ENA_MDTM, 0},
                                    {"ML", ENA_MLST, CM_INV | CM_ABR},
                                    {"MLS", ENA_MLST, CM_INV | CM_ABR},
                                    {"MLSD", ENA_MLST, CM_INV},
                                    {"MLST", ENA_MLST, 0},
                                    {"SIZE", ENA_SIZE, 0},
                                    {"", 0, 0}};
static int nftpena = (sizeof(ftpenatab) / sizeof(struct keytab)) - 1;

/* SET FTP command keyword indices */

#define FTS_AUT 1  /* Autoauthentication */
#define FTS_CRY 2  /* Encryption */
#define FTS_LOG 3  /* Autologin */
#define FTS_CPL 4  /* Command protection level */
#define FTS_CFW 5  /* Credentials forwarding */
#define FTS_DPL 6  /* Data protection level */
#define FTS_DBG 7  /* Debugging */
#define FTS_PSV 8  /* Passive mode */
#define FTS_SPC 9  /* Send port commands */
#define FTS_TYP 10 /* (file) Type */
#define FTS_USN 11 /* Unique server names (for files) */
#define FTS_VBM 12 /* Verbose mode */
#define FTS_ATP 13 /* Authentication type */
#define FTS_CNV 14 /* Filename conversion */
#define FTS_TST 15 /* Test (progress) messages */
#define FTS_PRM 16 /* (file) Permissions */
#define FTS_XLA 17 /* Charset translation */
#define FTS_CSR 18 /* Server charset */
#define FTS_ERR 19 /* Error action */
#define FTS_FNC 20 /* Collision */
#define FTS_SRP 21 /* SRP options */
#define FTS_GFT 22 /* GET automatic file-type switching */
#define FTS_DAT 23 /* Set file dates */
#define FTS_STO 24 /* Server time offset */
#define FTS_APW 25 /* Anonymous password */
#define FTS_DIS 26 /* File-transfer display style */
#define FTS_BUG 27 /* Bug(s) */
#define FTS_TMO 28 /* Timeout */

/* FTP BUGS */

#define FTB_SV2 1 /* use SSLv2 */
#define FTB_SV3 2 /* use SSLv3 */

static struct keytab ftpbugtab[] = {{"use-ssl-v2", FTB_SV2, 0},
                                    {"use-ssl-v3", FTB_SV3, 0}

};
static int nftpbug = (sizeof(ftpbugtab) / sizeof(struct keytab));

/* FTP PUT options (mutually exclusive, not a bitmask) */

#define PUT_UPD 1 /* Update */
#define PUT_RES 2 /* Restart */
#define PUT_SIM 4 /* Simulation */
#define PUT_DIF 8 /* Dates Differ */

static struct keytab ftpcolxtab[] =
    {                                  /* SET FTP COLLISION options */
     {"append", XYFX_A, 0},            /* append to old file */
     {"backup", XYFX_B, 0},            /* rename old file */
     {"dates-differ", XYFX_M, 0},      /* accept if dates differ */
     {"discard", XYFX_D, 0},           /* don't accept new file */
     {"no-supersede", XYFX_D, CM_INV}, /* ditto (MSK compatibility) */
     {"overwrite", XYFX_X, 0},         /* overwrite the old file */
     {"rename", XYFX_R, 0},            /* rename the incoming file */
     {"update", XYFX_U, 0},            /* replace if newer */
     {"", 0, 0}};
static int nftpcolx = (sizeof(ftpcolxtab) / sizeof(struct keytab)) - 1;

static struct keytab ftpset[] = {/* SET FTP commmand table */
                                 {"anonymous-password", FTS_APW, 0},
                                 {"autologin", FTS_LOG, 0},
                                 {"bug", FTS_BUG, 0},
#ifndef NOCSETS
                                 {"character-set-translation", FTS_XLA, 0},
#endif /* NOCSETS */
                                 {"collision", FTS_FNC, 0},
                                 {"dates", FTS_DAT, 0},
                                 {"debug", FTS_DBG, 0},
                                 {"display", FTS_DIS, 0},
                                 {"error-action", FTS_ERR, 0},
                                 {"filenames", FTS_CNV, 0},
                                 {"get-filetype-switching", FTS_GFT, 0},
                                 {"passive-mode", FTS_PSV, 0},
                                 {"pasv", FTS_PSV, CM_INV},
                                 {"permissions", FTS_PRM, 0},
                                 {"progress-messages", FTS_TST, 0},
                                 {"send-port-commands", FTS_SPC, 0},
#ifndef NOCSETS
                                 {"server-character-set", FTS_CSR, 0},
#endif /* NOCSETS */
                                 {"server-time-offset", FTS_STO, 0},
                                 {"srp", FTS_SRP, CM_INV},
#ifdef FTP_TIMEOUT
                                 {"timeout", FTS_TMO, 0},
#endif /* FTP_TIMEOUT */
                                 {"type", FTS_TYP, 0},
                                 {"unique-server-names", FTS_USN, 0},
                                 {"verbose-mode", FTS_VBM, 0},
                                 {"", 0, 0}};
static int nftpset = (sizeof(ftpset) / sizeof(struct keytab)) - 1;

/*
  GET and PUT switches are approximately the same as Kermit GET and SEND,
  and use the same SND_xxx definitions, but hijack a couple for FTP use.
  Don't just make up new ones, since the number of SND_xxx options must be
  known in advance for the switch-parsing arrays.
*/
#define SND_USN SND_PRO /* /UNIQUE instead of /PROTOCOL */
#define SND_PRM SND_PIP /* /PERMISSIONS instead of /PIPES */
#define SND_TEN SND_CAL /* /TENEX instead of /CALIBRATE */

static struct keytab putswi[] = {/* FTP PUT switch table */
                                 {"/after", SND_AFT, CM_ARG},
#ifdef PUTARRAY
                                 {"/array", SND_ARR, CM_ARG},
#endif /* PUTARRAY */
                                 {"/as", SND_ASN, CM_ARG | CM_INV | CM_ABR},
                                 {"/as-name", SND_ASN, CM_ARG},
                                 {"/ascii", SND_TXT, CM_INV},
                                 {"/b", SND_BIN, CM_INV | CM_ABR},
                                 {"/before", SND_BEF, CM_ARG},
                                 {"/binary", SND_BIN, 0},
#ifdef PUTPIPE
                                 {"/command", SND_CMD, CM_PSH},
#endif /* PUTPIPE */
                                 {"/delete", SND_DEL, 0},
                                 {"/dotfiles", SND_DOT, 0},
                                 {"/error-action", SND_ERR, CM_ARG},
                                 {"/except", SND_EXC, CM_ARG},
                                 {"/filenames", SND_NAM, CM_ARG},
#ifdef PIPESEND
#ifndef NOSPL
                                 {"/filter", SND_FLT, CM_ARG | CM_PSH},
#endif /* NOSPL */
#endif /* PIPESEND */
#ifdef CKSYMLINK
                                 {"/followlinks", SND_LNK, 0},
#endif /* CKSYMLINK */
                                 {"/image", SND_BIN, CM_INV},
                                 {"/larger-than", SND_LAR, CM_ARG},
                                 {"/listfile", SND_FIL, CM_ARG},
#ifndef NOCSETS
                                 {"/local-character-set", SND_CSL, CM_ARG},
#endif /* NOCSETS */
#ifdef CK_TMPDIR
                                 {"/move-to", SND_MOV, CM_ARG},
#endif /* CK_TMPDIR */
                                 {"/nobackupfiles", SND_NOB, 0},
                                 {"/nodotfiles", SND_NOD, 0},
#ifdef CKSYMLINK
                                 {"/nofollowlinks", SND_NLK, 0},
#endif /* CKSYMLINK */

                                 {"/not-after", SND_NAF, CM_ARG},
                                 {"/not-before", SND_NBE, CM_ARG},
#ifdef UNIX
                                 {"/permissions", SND_PRM, CM_ARG},
#else
                                 {"/permissions", SND_PRM, CM_ARG | CM_INV},
#endif /* UNIX */
                                 {"/quiet", SND_SHH, 0},
#ifdef FTP_RESTART
                                 {"/recover", SND_RES, 0},
#endif /* FTP_RESTART */
#ifdef RECURSIVE
                                 {"/recursive", SND_REC, 0},
#endif /* RECURSIVE */
                                 {"/rename-to", SND_REN, CM_ARG},
#ifdef FTP_RESTART
                                 {"/restart", SND_RES, CM_INV},
#endif /* FTP_RESTART */
#ifndef NOCSETS
                                 {"/server-character-set", SND_CSR, CM_ARG},
#endif /* NOCSETS */
                                 {"/server-rename-to", SND_SRN, CM_ARG},
                                 {"/simulate", SND_SIM, 0},
                                 {"/since", SND_AFT, CM_INV | CM_ARG},
                                 {"/smaller-than", SND_SMA, CM_ARG},
#ifdef RECURSIVE
                                 {"/subdirectories", SND_REC, CM_INV},
#endif /* RECURSIVE */
                                 {"/tenex", SND_TEN, 0},
                                 {"/text", SND_TXT, 0},
#ifndef NOCSETS
                                 {"/transparent", SND_XPA, 0},
#endif /* NOCSETS */
                                 {"/type", SND_TYP, CM_ARG},
#ifdef DOUPDATE
                                 {"/update", SND_UPD, 0},
#endif /* DOUPDATE */
                                 {"/unique-server-names", SND_USN, 0},
                                 {"", 0, 0}};
static int nputswi = (sizeof(putswi) / sizeof(struct keytab)) - 1;

static struct keytab getswi[] = {/* FTP [M]GET switch table */
                                 {"/after", SND_AFT, CM_INV},
                                 {"/as", SND_ASN, CM_ARG | CM_INV | CM_ABR},
                                 {"/as-name", SND_ASN, CM_ARG},
                                 {"/ascii", SND_TXT, CM_INV},
                                 {"/before", SND_BEF, CM_INV},
                                 {"/binary", SND_BIN, 0},
                                 {"/collision", SND_COL, CM_ARG},
#ifdef PUTPIPE
                                 {"/command", SND_CMD, CM_PSH},
#endif /* PUTPIPE */
                                 {"/delete", SND_DEL, 0},
                                 {"/error-action", SND_ERR, CM_ARG},
                                 {"/except", SND_EXC, CM_ARG},
                                 {"/filenames", SND_NAM, CM_ARG},
#ifdef PIPESEND
#ifndef NOSPL
                                 {"/filter", SND_FLT, CM_ARG | CM_PSH},
#endif /* NOSPL */
#endif /* PIPESEND */
                                 {"/image", SND_BIN, CM_INV},
                                 {"/larger-than", SND_LAR, CM_ARG},
                                 {"/listfile", SND_FIL, CM_ARG},
#ifndef NOCSETS
                                 {"/local-character-set", SND_CSL, CM_ARG},
#endif /* NOCSETS */
                                 {"/match", SND_PAT, CM_ARG},
                                 {"/ml", SND_MLS, CM_INV | CM_ABR},
                                 {"/mls", SND_MLS, CM_INV | CM_ABR},
                                 {"/mlsd", SND_MLS, 0},
                                 {"/mlst", SND_MLS, CM_INV},
#ifdef CK_TMPDIR
                                 {"/move-to", SND_MOV, CM_ARG},
#endif /* CK_TMPDIR */
                                 {"/namelist", SND_NML, CM_ARG},
                                 {"/nlst", SND_NLS, 0},
                                 {"/nobackupfiles", SND_NOB, 0},
                                 {"/nodotfiles", SND_NOD, 0},
#ifdef DOUPDATE
                                 {"/dates-differ", SND_DIF, CM_INV},
#endif /* DOUPDATE */
                                 {"/not-after", SND_NAF, CM_INV},
                                 {"/not-before", SND_NBE, CM_INV},
                                 {"/permissions", SND_PRM, CM_INV},
                                 {"/quiet", SND_SHH, 0},
#ifdef FTP_RESTART
                                 {"/recover", SND_RES, 0},
#endif /* FTP_RESTART */
#ifdef RECURSIVE
                                 {"/recursive", SND_REC, 0},
#endif /* RECURSIVE */
                                 {"/rename-to", SND_REN, CM_ARG},
#ifdef FTP_RESTART
                                 {"/restart", SND_RES, CM_INV},
#endif /* FTP_RESTART */
#ifndef NOCSETS
                                 {"/server-character-set", SND_CSR, CM_ARG},
#endif /* NOCSETS */
                                 {"/server-rename-to", SND_SRN, CM_ARG},
                                 {"/smaller-than", SND_SMA, CM_ARG},
#ifdef RECURSIVE
                                 {"/subdirectories", SND_REC, CM_INV},
#endif /* RECURSIVE */
                                 {"/text", SND_TXT, 0},
                                 {"/tenex", SND_TEN, 0},
#ifndef NOCSETS
                                 {"/transparent", SND_XPA, 0},
#endif /* NOCSETS */
                                 {"/to-screen", SND_MAI, 0},
#ifdef DOUPDATE
                                 {"/update", SND_UPD, CM_INV},
#endif /* DOUPDATE */
                                 {"", 0, 0}};
static int ngetswi = (sizeof(getswi) / sizeof(struct keytab)) - 1;

static struct keytab delswi[] = {/* FTP [M]DELETE switch table */
                                 {"/error-action", SND_ERR, CM_ARG},
                                 {"/except", SND_EXC, CM_ARG},
                                 {"/filenames", SND_NAM, CM_ARG},
                                 {"/larger-than", SND_LAR, CM_ARG},
                                 {"/nobackupfiles", SND_NOB, 0},
                                 {"/nodotfiles", SND_NOD, 0},
                                 {"/quiet", SND_SHH, 0},
#ifdef RECURSIVE
                                 {"/recursive", SND_REC, 0},
#endif /* RECURSIVE */
                                 {"/smaller-than", SND_SMA, CM_ARG},
#ifdef RECURSIVE
                                 {"/subdirectories", SND_REC, CM_INV},
#endif /* RECURSIVE */
                                 {"", 0, 0}};
static int ndelswi = (sizeof(delswi) / sizeof(struct keytab)) - 1;

static struct keytab fntab[] = {/* Filename conversion keyword table */
                                {"automatic", 2, CNV_AUTO},
                                {"converted", 1, CNV_CNV},
                                {"literal", 0, CNV_LIT}};
static int nfntab = (sizeof(fntab) / sizeof(struct keytab));

static struct keytab ftptyp[] = {/* SET FTP TYPE table */
                                 {"ascii", FTT_ASC, 0},
                                 {"binary", FTT_BIN, 0},
                                 {"tenex", FTT_TEN, 0},
                                 {"text", FTT_ASC, CM_INV},
                                 {"", 0, 0}};
static int nftptyp = (sizeof(ftptyp) / sizeof(struct keytab)) - 1;

/* Definitions for FTP from RFC765. */

/* Reply codes */

#define REPLY_PRELIM 1    /* Positive preliminary */
#define REPLY_COMPLETE 2  /* Positive completion */
#define REPLY_CONTINUE 3  /* Positive intermediate */
#define REPLY_TRANSIENT 4 /* Transient negative completion */
#define REPLY_ERROR 5     /* Permanent negative completion */
#define REPLY_SECURE 6    /* Security encoded message */

/* Form codes and names */

#define FORM_N 1 /* Non-print */
#define FORM_T 2 /* Telnet format effectors */
#define FORM_C 3 /* Carriage control (ASA) */

/* Structure codes and names */

#define STRU_F 1 /* File (no record structure) */
#define STRU_R 2 /* Record structure */
#define STRU_P 3 /* Page structure */

/* Mode types and names */

#define MODE_S 1 /* Stream */
#define MODE_B 2 /* Block */
#define MODE_C 3 /* Compressed */

/* Protection levels and names */

#define PROT_C 1 /* Clear */
#define PROT_S 2 /* Safe */
#define PROT_P 3 /* Private */
#define PROT_E 4 /* Confidential */

#define RADIX_ENCODE 0 /* radix_encode() function codes */
#define RADIX_DECODE 1

/*
  The default setpbsz() value in the Unix FTP client is 1<<20 (1MB).  This
  results in a serious performance degradation due to the increased number
  of page faults and the inability to overlap encrypt/decrypt, file i/o, and
  network i/o.  So instead we set the value to 1<<13 (8K), about half the size
  of the typical TCP window.  Maybe we should add a command to allow the value
  to be changed.
*/
#define DEFAULT_PBSZ 1 << 13

/* Prototypes */

int remtxt(char **);
char *gskreason(int);
static int ftpclose(void);
static int zzsend(int, CHAR);
static int getreply(int, int, int, int, int);
static int radix_encode(CHAR[], CHAR[], int, int *, int);
static int setpbsz(unsigned int);
static int recvrequest(char *, char *, char *, char *, int, int, char *, int,
                       int, int);
static int ftpcmd(char *, char *, int, int, int);
static int ftp_user(char *, char *, char *);
static int ftp_login(char *);
static int ftp_reset(void);
static int ftp_rename(char *, char *);
static int ftp_umask(char *);
static int secure_flush(int);
static int secure_write(int, CHAR *, unsigned int);
static int scommand(char *);
static int secure_putbuf(int, CHAR *, unsigned int);
static int secure_getc(int, int);
static int secure_getbyte(int, int);
static int secure_read(int, char *, int);
static int initconn(void);
static int dataconn(char *);
static int setprotbuf(unsigned int);
static int sendrequest(char *, char *, char *, int, int, int, int);

static char *radix_error(int);
static char *ftp_hookup(char *, int, int);
static CHAR *remote_files(int, CHAR *, CHAR *, int);

static void mlsreset(void);
static void secure_error(char *fmt, ...);
static void lostpeer(void);
static void cancel_remote(int);
static void changetype(int, int);

static void cmdcancel(int);

/*  D O F T P A R G  --  Do an FTP command-line argument.  */

static char *strval(char *s1, char *s2) {
  if (!s1)
    s1 = "";
  if (!s2)
    s2 = "";
  return (*s1 ? s1 : (*s2 ? s2 : "(none)"));
}

#ifndef NOCSETS
static char *rfnptr = NULL;
static int rfnlen = 0;
static char rfnbuf[RFNBUFSIZ]; /* Remote filename translate buffer */
static char *xgnbp = NULL;

static int /* Helper function for xgnbyte() */
strgetc(void) {
  int c;
  if (!xgnbp)
    return (-1);
  if (!*xgnbp)
    return (-1);
  c = (unsigned)*xgnbp++;
  return (((unsigned)c) & 0xff);
}

static int /* Helper function for xpnbyte() */
strputc(char c) {
  rfnlen = rfnptr - rfnbuf;
  if (rfnlen >= (RFNBUFSIZ - 1))
    return (-1);
  *rfnptr++ = c;
  *rfnptr = NUL;
  return (0);
}

#endif /* NOCSETS */

#ifdef CKLOGDIAL
char ftplogbuf[CXLOGBUFL] = {NUL, NUL}; /* Connection Log */
int ftplogactive = 0;
long ftplogprev = 0L;

void ftplogend() {
  extern int dialog;
  extern char diafil[];
  long d1, d2, t1, t2;
  char buf[32], *p;

  debug(F111, "ftp cx log active", ckitoa(dialog), ftplogactive);
  debug(F110, "ftp cx log buf", ftplogbuf, 0);

  if (!ftplogactive || !ftplogbuf[0]) /* No active record */
    return;

  ftplogactive = 0; /* Record is not active */

  d1 = mjd((char *)ftplogbuf);  /* Get start date of this session */
  ckstrncpy(buf, ckdate(), 31); /* Get current date */
  d2 = mjd(buf);                /* Convert them to mjds */
  p = ftplogbuf;                /* Get start time */
  p[11] = NUL;
  p[14] = NUL; /* Convert to seconds */
  t1 = atol(p + 9) * 3600L + atol(p + 12) * 60L + atol(p + 15);
  p[11] = ':';
  p[14] = ':';
  p = buf; /* Get end time */
  p[11] = NUL;
  p[14] = NUL;
  t2 = atol(p + 9) * 3600L + atol(p + 12) * 60L + atol(p + 15);
  t2 = ((d2 - d1) * 86400L) + (t2 - t1); /* Compute elapsed time */
  if (t2 > -1L) {
    ftplogprev = t2;
    p = hhmmss(t2);
    ckstrncat(ftplogbuf, "E=", CXLOGBUFL); /* Append to log record */
    ckstrncat(ftplogbuf, p, CXLOGBUFL);
  } else
    ftplogprev = 0L;
  debug(F101, "ftp cx log dialog", "", dialog);
  if (dialog) { /* If logging */
    int x;
    x = diaopn(diafil, 1, 1); /* Open log in append mode */
    if (x > 0) {
      debug(F101, "ftp cx log open", "", x);
      x = zsoutl(ZDIFIL, ftplogbuf); /* Write the record */
      debug(F101, "ftp cx log write", "", x);
      x = zclose(ZDIFIL); /* Close the log */
      debug(F101, "ftp cx log close", "", x);
    }
  }
}

void dologftp() {
  ftplogend(); /* Previous session not closed out? */
  ftplogprev = 0L;
  ftplogactive = 1; /* Record is active */

  ckmakxmsg(ftplogbuf, CXLOGBUFL, ckdate(), " ", strval(ftp_logname, NULL), " ",
            ckgetpid(), " T=FTP N=", strval(ftp_host, NULL), " H=", myhost,
            " P=", ckitoa(ftp_port), " "); /* SMS 2007/02/15 */
  debug(F110, "ftp cx log begin", ftplogbuf, 0);
}
#endif /* CKLOGDIAL */

static char *dummy[2] = {NULL, NULL};

static struct keytab modetab[] = {{"active", 0, 0}, {"passive", 1, 0}};

#ifndef NOCMDL
int /* Called from ckuusy.c */
doftparg(char c)
/* doftparg */ {
  char *xp;
  extern char **xargv, *xarg0;
  extern int xargc, stayflg, haveftpuid;
  extern char uidbuf[];

  xp = *xargv + 1; /* Pointer for bundled args */
  while (c) {
    if (ckstrchr("MuDPkcHzm", c)) { /* Options that take arguments */
      if (*(xp + 1)) {
        fatal("?Invalid argument bundling");
      }
      xargv++, xargc--;
      if ((xargc < 1) || (**xargv == '-')) {
        fatal("?Required argument missing");
      }
    }
    switch (c) { /* Big switch on arg */
    case 'h':    /* help */
      printf("C-Kermit's FTP client command-line personality.  Usage:\n");
      printf("  %s [ options ] host [ port ] [-pg files ]\n\n", xarg0);
      printf("Options:\n");
      printf("  -h           = help (this message)\n");
      printf("  -m mode      = \"passive\" (default) or \"active\"\n");
      printf("  -u name      = username for autologin (or -M)\n");
      printf("  -P password  = password for autologin (RISKY)\n");
      printf("  -A           = autologin anonymously\n");
      printf("  -D directory = cd after autologin\n");
      printf("  -b           = force binary mode\n");
      printf("  -a           = force text (\"ascii\") mode (or -T)\n");
      printf("  -d           = debug (double to add timestamps)\n");
      printf("  -n           = no autologin\n");
      printf("  -v           = verbose (default)\n");
      printf("  -q           = quiet\n");
      printf("  -S           = Stay (issue command prompt when done)\n");
      printf("  -Y           = do not execute Kermit init file\n");
      printf("  -p files     = files to put after autologin (or -s)\n");
      printf("  -g files     = files to get after autologin\n");
      printf("  -R           = recursive (for use with -p)\n");

      printf("\n-p or -g, if given, should be last.  Example:\n");
      printf("  ftp -A kermit.columbia.edu -D kermit -ag TESTFILE\n");

      doexit(GOOD_EXIT, -1);
      break;

    case 'R': /* Recursive */
      recursive = 1;
      break;

    case 'd': /* Debug */
#ifdef DEBUG
      if (deblog) {
        extern int debtim;
        debtim = 1;
      } else {
        deblog = debopn("debug.log", 0);
        debok = 1;
      }
#endif /* DEBUG */
      /* fall thru on purpose */

    case 't': /* Trace */
      ftp_deb++;
      break;

    case 'n': /* No autologin */
      ftp_log = 0;
      break;

    case 'i': /* No prompt */
    case 'v': /* Verbose */
      break;  /* (ignored) */

    case 'q': /* Quiet */
      quiet = 1;
      break;

    case 'S': /* Stay */
      stayflg = 1;
      break;

    case 'M':
    case 'u': /* My User Name */
      if ((int)strlen(*xargv) > 63) {
        fatal("username too long");
      }
      ckstrncpy(uidbuf, *xargv, UIDBUFLEN);
      haveftpuid = 1;
      break;

    case 'A':
      ckstrncpy(uidbuf, "anonymous", UIDBUFLEN);
      haveftpuid = 1;
      break;

    case 'T': /* Text */
    case 'a': /* "ascii" */
    case 'b': /* Binary */
      binary = (c == 'b') ? FTT_BIN : FTT_ASC;
      ftp_xfermode = XMODE_M;
      filepeek = 0;
      patterns = 0;
      break;

    case 'g':   /* Get */
    case 'p':   /* Put */
    case 's': { /* Send (= Put) */
      int havefiles, rc;
      if (ftp_action) {
        fatal("Only one FTP action at a time please");
      }
      if (*(xp + 1)) {
        fatal("invalid argument bundling after -s");
      }
      nfils = 0;          /* Initialize file counter */
      havefiles = 0;      /* Assume nothing to send  */
      cmlist = xargv + 1; /* Remember this pointer */

      while (++xargv, --xargc > 0) { /* Traverse the list */
        if (c == 'g') {
          havefiles++;
          nfils++;
          continue;
        }
#ifdef RECURSIVE
        if (!strcmp(*xargv, ".")) {
          havefiles = 1;
          nfils++;
          recursive = 1;
        } else
#endif /* RECURSIVE */
          if ((rc = zchki(*xargv)) > -1 || (rc == -2)) {
            if (rc != -2)
              havefiles = 1;
            nfils++;
          } else if (iswild(*xargv) && nzxpand(*xargv, 0) > 0) {
            havefiles = 1;
            nfils++;
          }
      }
      xargc++, xargv--; /* Adjust argv/argc */
      if (!havefiles) {
        if (c == 'g') {
          fatal("No files to put");
        } else {
          fatal("No files to get");
        }
      }
      ftp_action = c;
      break;
    }
    case 'D': /* Directory */
      makestr(&ftp_rdir, *xargv);
      break;

    case 'm': /* Mode (Active/Passive */
      ftp_psv = lookup(modetab, *xargv, 2, NULL);
      if (ftp_psv < 0)
        fatal("Invalid mode");
      break;

    case 'P':
      makestr(&ftp_tmp, *xargv); /* You-Know-What */
      break;

    case 'Y': /* No initialization file */
      break;  /* (already done in prescan) */

#ifdef CK_URL
    case 'U': { /* URL */
      /* These are set by urlparse() - any not set are NULL */
      if (g_url.hos) {
        /*
          Kermit has accepted host:port notation since many years before URLs
          were invented.  Unfortunately, URLs conflict with this notation.  Thus
          "ftp host:449" looks like a URL and results in service = host and host
          = 449. Here we try to catch this situation transparently to the user.
        */
        if (ckstrcmp(g_url.svc, "ftp", -1, 0)) {
          if (!g_url.usr && !g_url.psw && !g_url.por && !g_url.pth) {
            g_url.por = g_url.hos;
            g_url.hos = g_url.svc;
            g_url.svc = "ftp";
          } else {
            ckmakmsg(tmpbuf, TMPBUFSIZ, "Non-FTP URL: service=", g_url.svc,
                     " host=", g_url.hos);
            fatal(tmpbuf);
          }
        }
        makestr(&ftp_host, g_url.hos);
        if (g_url.usr) {
          haveftpuid = 1;
          ckstrncpy(uidbuf, g_url.usr, UIDBUFLEN);
          makestr(&ftp_logname, uidbuf);
        }
        if (g_url.psw) {
          makestr(&ftp_tmp, g_url.psw);
        }
        if (g_url.pth) {
          if (!g_url.usr) {
            haveftpuid = 1;
            ckstrncpy(uidbuf, "anonymous", UIDBUFLEN);
            makestr(&ftp_logname, uidbuf);
          }
          if (ftp_action) {
            fatal("Only one FTP action at a time please");
          }
          if (!stayflg)
            quiet = 1;
          nfils = 1;
          dummy[0] = g_url.pth;
          cmlist = dummy;
          ftp_action = 'g';
        }
        xp = NULL;
        haveurl = 1;
      }
      break;
    }
#endif /* CK_URL */

    default:
      fatal2(*xargv, "unknown command-line option, type \"ftp -h\" for help");
    }
    if (!xp)
      break;
    c = *++xp; /* See if options are bundled */
  }
  return (0);
}
#endif /* NOCMDL */

int ftpisconnected() { return (connected); }

int ftpisloggedin() { return (connected ? loggedin : 0); }

int ftpissecure() { return ((ftp_dpl == FPL_CLR && !ssl_ftp_proxy) ? 0 : 1); }

static void ftscreen(int n, char c, CK_OFF_T z, char *s) {
  if (displa && fdispla && !backgrd && !quiet && !out2screen) {
    if (!dpyactive) {
      ckscreen(SCR_PT, 'S', (CK_OFF_T)0, "");
      dpyactive = 1;
    }
    ckscreen(n, c, z, s);
  }
}

/*  g m s t i m e r  --  Millisecond timer */

long gmstimer() {
#ifdef HAVE_MSECS
  /* For those versions of ztime() that also set global ztmsec. */
  char *p = NULL;
  long z;
  ztime(&p);
  if (!p)
    return (0L);
  if (!*p)
    return (0L);
  z = atol(p + 11) * 3600L + atol(p + 14) * 60L + atol(p + 17);
  return (z * 1000 + ztmsec);
#else
  return ((long)time(NULL) * 1000L);
#endif /* HAVE_MSECS */
}

/*  d o s e t f t p  --  The SET FTP command  */

int dosetftp() {
  int cx;
  if ((cx = cmkey(ftpset, nftpset, "", "", xxstring)) < 0) /* Set what? */
    return (cx);
  switch (cx) {

  case FTS_FNC: /* Filename collision action */
    if ((x = cmkey(ftpcolxtab, nftpcolx, "", "", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_fnc = x;
    return (1);

  case FTS_CNV: /* Filename conversion */
    if ((x = cmkey(fntab, nfntab, "", "automatic", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_cnv = x;
    return (1);

  case FTS_DBG: /* Debug messages */
    return (seton(&ftp_deb));

  case FTS_LOG: /* Auto-login */
    return (seton(&ftp_log));

  case FTS_PSV: /* Passive mode */
    return (dosetftppsv());

  case FTS_SPC: /* Send port commands */
    x = seton(&ftp_spc);
    if (x > 0)
      sendport = ftp_spc;
    return (x);

  case FTS_TYP: /* Type */
    if ((x = cmkey(ftptyp, nftptyp, "", "", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_typ = x;
    g_ftp_typ = x;
    tenex = (ftp_typ == FTT_TEN);
    return (1);

  case FTS_USN: /* Unique server names */
    return (seton(&ftp_usn));

  case FTS_VBM:                    /* Verbose mode */
    if ((x = seton(&ftp_vbm)) < 0) /* Per-command copy */
      return (x);
    ftp_vbx = ftp_vbm; /* Global sticky copy */
    return (x);

  case FTS_TST: /* "if (testing)" messages */
    return (seton(&testing));

  case FTS_PRM: /* Send permissions */
    return (setonaut(&ftp_prm));

  case FTS_AUT: /* Auto-authentication */
    return (seton(&ftp_aut));

  case FTS_ERR: /* Error action */
    if ((x = cmkey(qorp, 2, "", "", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_err = x;
    return (success = 1);

#ifndef NOCSETS
  case FTS_XLA: /* Translation */
    return (seton(&ftp_xla));

  case FTS_CSR: /* Server charset */
    if ((x = cmkey(fcstab, nfilc, "character-set", "utf8", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_csr = x;
    ftp_xla = 1; /* Also enable translation */
    return (success = 1);
#endif /* NOCSETS */

  case FTS_GFT:
    return (seton(&get_auto)); /* GET-filetype-switching */

  case FTS_DAT:
    return (seton(&ftp_dates)); /* Set file dates */

#ifdef FTP_TIMEOUT
  case FTS_TMO: /* Timeout */
    if ((x = cmnum("Number of seconds", "0", 10, &z, xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    ftp_timeout = z;
    return (success = 1);
#endif /* FTP_TIMEOUT */

  case FTS_STO: { /* Server time offset */
    char *s, *p = NULL;
    long k;
    if ((x = cmfld("[+-]hh[:mm[:ss]]", "+0", &s, xxstring)) < 0)
      return (x);
    if (!strcmp(s, "+0")) {
      s = NULL;
    } else if ((x = delta2sec(s, &k)) < 0) { /* Check format */
      printf("?Invalid time offset\n");
      return (-9);
    }
    makestr(&p, s);          /* Make a safe copy the string */
    if ((x = cmcfm()) < 0) { /* Get confirmation */
      if (p)
        makestr(&p, NULL);
      return (x);
    }
    fts_sto = p; /* Confirmed - set the string. */
    return (success = 1);
  }
  case FTS_APW: {
    char *s;
    if ((x = cmtxt("Text", "", &s, xxstring)) < 0)
      return (x);
    makestr(&ftp_apw, *s ? s : NULL);
    return (success = 1);
  }

  case FTS_BUG: {
    if ((x = cmkey(ftpbugtab, nftpbug, "", "", xxstring)) < 0)
      return (x);
    switch (x) {
    default:
      return (-2);
    }
  }

  case FTS_DIS:
    doxdis(2); /* 2 == ftp */
    return (success = 1);

  default:
    return (-2);
  }
}

int ftpbye() {
  int x;
  if (!connected)
    return (1);
  if (testing)
    printf(" ftp closing %s...\n", ftp_host);
  x = ftpclose();
  return ((x > -1) ? 1 : 0);
}

/*  o p e n f t p  --  Parse FTP hostname & port and open */

static int openftp(char *s, int opn_tls) {
  char c, *p, *hostname = NULL, *hostsave = NULL, *service = NULL;
  int i, n, havehost = 0, getval = 0, rc = -9, opn_psv = -1, nologin = 0;
  int haveuser = 0;
  struct FDB sw, fl, cm;
  extern int nnetdir;   /* Network services directory */
  extern int nhcount;   /* Lookup result */
  extern char *nh_p[];  /* Network directory entry pointers */
  extern char *nh_p2[]; /* Network directory entry nettype */

  if (!s)
    return (-2);
  if (!*s)
    return (-2);

  makestr(&hostname, s);
  hostsave = hostname;
  makestr(&ftp_logname, NULL);
  anonymous = 0;
  noinit = 0;

  debug(F110, "ftp open", hostname, 0);

  if (sav_psv > -1) {  /* Restore prevailing active/passive */
    ftp_psv = sav_psv; /* selection in case it was */
    sav_psv = -1;      /* temporarily overriden by a switch */
  }
  if (sav_log > -1) { /* Ditto for autologin */
    ftp_log = sav_log;
    sav_log = -1;
  }
  cmfdbi(&sw,                                             /* Switches */
         _CMKEY, "Service name or port;\n or switch", "", /* default */
         "",        /* addtl string data */
         nftpswi,   /* addtl numeric data 1: tbl size */
         4,         /* addtl numeric data 2: none */
         xxstring,  /* Processing function */
         ftpswitab, /* Keyword table */
         &fl        /* Pointer to next FDB */
  );
  cmfdbi(&fl,      /* A host name or address */
         _CMFLD,   /* fcode */
         "",       /* help */
         "xYzBoo", /* default */
         "",       /* addtl string data */
         0,        /* addtl numeric data 1 */
         0,        /* addtl numeric data 2 */
         xxstring, NULL, &cm);
  cmfdbi(&cm, /* Command confirmation */
         _CMCFM, "", "", "", 0, 0, NULL, NULL, NULL);

  for (n = 0;; n++) {
    rc = cmfdb(&sw); /* Parse a service name or a switch */
    if (rc < 0)
      goto xopenftp;

    if (cmresult.fcode == _CMCFM) { /* Done? */
      break;
    } else if (cmresult.fcode == _CMFLD) { /* Port */
      if (ckstrcmp("xYzBoo", cmresult.sresult, -1, 1))
        makestr(&service, cmresult.sresult);
      else
        makestr(&service, opn_tls ? "ftps" : "ftp");
    } else if (cmresult.fcode == _CMKEY) { /* Have a switch */
      c = cmgbrk();                        /* get break character */
      getval = (c == ':' || c == '=');
      rc = -9;
      if (getval && !(cmresult.kflags & CM_ARG)) {
        printf("?This switch does not take arguments\n");
        goto xopenftp;
      }
      if (!getval && (cmresult.kflags & CM_ARG)) {
        printf("?This switch requires an argument\n");
        goto xopenftp;
      }
      switch (cmresult.nresult) { /* Switch */
      case OPN_ANO:               /* /ANONYMOUS */
        anonymous++;
        nologin = 0;
        break;
      case OPN_NIN: /* /NOINIT */
        noinit++;
        break;
      case OPN_NOL: /* /NOLOGIN */
        nologin++;
        anonymous = 0;
        makestr(&ftp_logname, NULL);
        break;
      case OPN_PSW:     /* /PASSWORD */
        if (!anonymous) /* Don't log real passwords */
          debok = 0;
        rc = cmfld("Password for FTP server", "", &p, xxstring);
        if (rc == -3) {
          makestr(&ftp_tmp, NULL);
        } else if (rc < 0) {
          goto xopenftp;
        } else {
          makestr(&ftp_tmp, brstrip(p));
          nologin = 0;
        }
        break;
      case OPN_USR: /* /USER */
        rc = cmfld("Username for FTP server", "", &p, xxstring);
        if (rc == -3) {
          makestr(&ftp_logname, NULL);
        } else if (rc < 0) {
          goto xopenftp;
        } else {
          nologin = 0;
          anonymous = 0;
          haveuser = 1;
          makestr(&ftp_logname, brstrip(p));
        }
        break;
      case OPN_ACC:
        rc = cmfld("Account for FTP server", "", &p, xxstring);
        if (rc == -3) {
          makestr(&ftp_acc, NULL);
        } else if (rc < 0) {
          goto xopenftp;
        } else {
          makestr(&ftp_acc, brstrip(p));
        }
        break;
      case OPN_ACT:
        opn_psv = 0;
        break;
      case OPN_PSV:
        opn_psv = 1;
        break;
      case OPN_TLS:
        opn_tls = 1;
        break;
      default:
        break;
      }
    }
    if (n == 0) { /* After first time through */
      cmfdbi(&sw, /* accept only switches */
             _CMKEY, "\nCarriage return to confirm to command, or switch", "",
             "", nftpswi, 4, xxstring, ftpswitab, &cm);
    }
  }

  if (opn_psv > -1) { /* /PASSIVE or /ACTIVE switch given */
    sav_psv = ftp_psv;
    ftp_psv = opn_psv;
  }
  if (nologin || haveuser) { /* /NOLOGIN or /USER switch given */
    sav_log = ftp_log;
    ftp_log = haveuser ? 1 : 0;
  }
  if (*hostname == '=') { /* Bypass directory lookup */
    hostname++;           /* if hostname starts with '=' */
    havehost++;
  } else if (isdigit(*hostname)) { /* or if it starts with a digit */
    havehost++;
  }
  if (!service)
    makestr(&service, opn_tls ? "ftps" : "ftp");

#ifndef NODIAL
  if (!havehost && nnetdir > 0) { /* If there is a networks directory */
    lunet(hostname);              /* Look up the name */
    debug(F111, "ftp openftp lunet", hostname, nhcount);
    if (nhcount == 0) {
      if (testing)
        printf(" ftp open trying \"%s %s\"...\n", hostname, service);
      success = ftpopen(hostname, service, opn_tls);
      debug(F101, "ftp openftp A ftpopen success", "", success);
      rc = success;
    } else {
      int found = 0;
      for (i = 0; i < nhcount; i++) {
        if (nh_p2[i]) /* If network type specified */
          if (ckstrcmp(nh_p2[i], "tcp/ip", strlen(nh_p2[i]), 0))
            continue;
        found++;
        makestr(&hostname, nh_p[i]);
        debug(F111, "ftpopen lunet substitution", hostname, i);
        if (testing)
          printf(" ftp open trying \"%s %s\"...\n", hostname, service);
        success = ftpopen(hostname, service, opn_tls);
        debug(F101, "ftp openftp B ftpopen success", "", success);
        rc = success;
        if (success)
          break;
      }
      if (!found) { /* E.g. if no network types match */
        if (testing)
          printf(" ftp open trying \"%s %s\"...\n", hostname, service);
        success = ftpopen(hostname, service, opn_tls);
        debug(F101, "ftp openftp C ftpopen success", "", success);
        rc = success;
      }
    }
  } else {
#endif /* NODIAL */
    if (testing)
      printf(" ftp open trying \"%s %s\"...\n", hostname, service);
    success = ftpopen(hostname, service, opn_tls);
    debug(F111, "ftp openftp D ftpopen success", hostname, success);
    debug(F111, "ftp openftp D ftpopen connected", hostname, connected);
    rc = success;
#ifndef NODIAL
  }
#endif /* NODIAL */

xopenftp:
  debug(F101, "ftp openftp xopenftp rc", "", rc);
  if (hostsave)
    free(hostsave);
  if (service)
    free(service);
  if (rc < 0 && ftp_logname) {
    free(ftp_logname);
    ftp_logname = NULL;
  }
  if (ftp_tmp) {
    free(ftp_tmp);
    ftp_tmp = NULL;
  }
  return (rc);
}

void /* 12 Aug 2007 */
doftpglobaltype(int x) {
  ftp_xfermode = XMODE_M; /* Set manual FTP transfer mode */
  ftp_typ = x;            /* Used by top-level BINARY and */
  g_ftp_typ = x;          /* ASCII commands. */
  get_auto = 0;
  forcetype = 1;
}

int doftpacct() {
  int x;
  char *s;
  if ((x = cmtxt("Remote account", "", &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  makestr(&ftp_acc, brstrip(s));
  if (testing)
    printf(" ftp account: \"%s\"\n", ftp_acc);
  success = (ftpcmd("ACCT", ftp_acc, -1, -1, ftp_vbm) == REPLY_COMPLETE);
  return (success);
}

int doftpusr() { /* Log in as USER */
  extern char uidbuf[];
  extern char pwbuf[];
  extern int pwflg, pwcrypt;
  int x;
  char *s, *acct = "";

  debok = 0; /* Don't log */

  if ((x = cmfld("Remote username or ID", uidbuf, &s, xxstring)) < 0)
    return (x);
  ckstrncpy(line, brstrip(s), LINBUFSIZ); /* brstrip: 15 Jan 2003 */
  if ((x = cmfld("Remote password", "", &s, xxstring)) < 0) {
    if (x == -3) { /* no input */
      if (pwbuf[0] && pwflg) {
        ckstrncpy(tmpbuf, (char *)pwbuf, TMPBUFSIZ);
      }
    } else {
      return (x);
    }
  } else {
    ckstrncpy(tmpbuf, brstrip(s), TMPBUFSIZ);
  }
  if ((x = cmtxt("Remote account\n or Enter or CR to confirm the command", "",
                 &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  if (*s) {
    x = strlen(tmpbuf);
    if (x > 0) {
      acct = &tmpbuf[x + 2];
      ckstrncpy(acct, brstrip(s), TMPBUFSIZ - x - 2);
    }
  }
  if (testing)
    printf(" ftp user \"%s\" password \"%s\"...\n", line, tmpbuf);
  success = ftp_user(line, tmpbuf, acct);
#ifdef CKLOGDIAL
  dologftp();
#endif /* CKLOGDIAL */
  return (success);
}

/* DO (various FTP commands)... */

int doftptyp(int type) /* TYPE */
{
  CHECKCONN();
  ftp_typ = type;
  changetype(ftp_typ, ftp_vbm);
  debug(F101, "doftptyp changed type", "", type);
  return (1);
}

static int doftpxmkd(char *s, int vbm) /* MKDIR action */
{
  int lcs = -1, rcs = -1;
#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */
  debug(F110, "ftp doftpmkd", s, 0);
  if (ftpcmd("MKD", s, lcs, rcs, vbm) == REPLY_COMPLETE)
    return (success = 1);
  if (ftpcode == 500 || ftpcode == 502) {
    if (!quiet)
      printf("MKD command not recognized, trying XMKD\n");
    if (ftpcmd("XMKD", s, lcs, rcs, vbm) == REPLY_COMPLETE)
      return (success = 1);
  }
  return (success = 0);
}

static int doftpmkd(void) /* MKDIR parse */
{
  int x;
  char *s;
  if ((x = cmtxt("Remote directory name", "", &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  ckstrncpy(line, s, LINBUFSIZ);
  if (testing)
    printf(" ftp mkdir \"%s\"...\n", line);
  return (success = doftpxmkd(line, -1));
}

static int doftprmd() { /* RMDIR */
  int x, lcs = -1, rcs = -1;
  char *s;
  if ((x = cmtxt("Remote directory", "", &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  ckstrncpy(line, s, LINBUFSIZ);
  if (testing)
    printf(" ftp rmdir \"%s\"...\n", line);
#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */
  if (ftpcmd("RMD", line, lcs, rcs, ftp_vbm) == REPLY_COMPLETE)
    return (success = 1);
  if (ftpcode == 500 || ftpcode == 502) {
    if (!quiet)
      printf("RMD command not recognized, trying XMKD\n");
    success = (ftpcmd("XRMD", line, lcs, rcs, ftp_vbm) == REPLY_COMPLETE);
  } else
    success = 0;
  return (success);
}

static int doftpren() { /* RENAME */
  int x;
  char *s;
  if ((x = cmfld("Remote filename", "", &s, xxstring)) < 0)
    return (x);
  ckstrncpy(line, s, LINBUFSIZ);
  if ((x = cmfld("New name for remote file", "", &s, xxstring)) < 0)
    return (x);
  ckstrncpy(tmpbuf, s, TMPBUFSIZ);
  if ((x = cmcfm()) < 0)
    return (x);
  CHECKCONN();
  if (testing)
    printf(" ftp rename \"%s\" (to) \"%s\"...\n", line, tmpbuf);
  success = ftp_rename(line, tmpbuf);
  return (success);
}

int doftpres() { /* RESET (log out without close) */
  int x;
  if ((x = cmcfm()) < 0)
    return (x);
  CHECKCONN();
  if (testing)
    printf(" ftp reset...\n");
  return (success = ftp_reset());
}

static int doftpxhlp() { /* HELP */
  int x;
  char *s;
  if ((x = cmtxt("Command name", "", &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  ckstrncpy(line, s, LINBUFSIZ);
  if (testing)
    printf(" ftp help \"%s\"...\n", line);
  /* No need to translate -- all FTP commands are ASCII */
  return (success = (ftpcmd("HELP", line, 0, 0, 1) == REPLY_COMPLETE));
}

static int doftpdir(int cx) /* [V]DIRECTORY */
{
  int x, lcs = 0, rcs = 0, xlate = 0;
  char *p, *s, *m = "";
  if (cx == FTP_VDI) {
    switch (servertype) {
    case SYS_VMS:
    case SYS_DOS:
    case SYS_TOPS10:
    case SYS_TOPS20:
      m = "*.*";
      break;
    default:
      m = "*";
    }
  }
  if ((x = cmtxt("Remote filespec", m, &s, xxstring)) < 0)
    return (x);
  if ((x = remtxt(&s)) < 0)
    return (x);
#ifdef NOCSETS
  xlate = 0;
#else
  xlate = ftp_xla;
#endif /* NOCSETS */
  line[0] = NUL;
  ckstrncpy(line, s, LINBUFSIZ);
  s = line;
  CHECKCONN();

#ifndef NOCSETS
  if (xlate) {     /* SET FTP CHARACTER-SET-TRANSLATION */
    lcs = ftp_csl; /* Local charset */
    if (lcs < 0)
      lcs = fcharset;
    if (lcs < 0)
      xlate = 0;
  }
  if (xlate) {     /* Still ON? */
    rcs = ftp_csx; /* Remote (Server) charset */
    if (rcs < 0)
      rcs = ftp_csr;
    if (rcs < 0)
      xlate = 0;
  }
#endif /* NOCSETS */

  if (testing) {
    p = s;
    if (!p)
      p = "";
    if (*p)
      printf("Directory of files %s at %s:\n", line, ftp_host);
    else
      printf("Directory of files at %s:\n", ftp_host);
  }
  debug(F111, "doftpdir", s, cx);

  if (cx == FTP_DIR) {
    /* Translation of line[] is done inside recvrequest() */
    /* when it calls ftpcmd(). */
    return (success = (recvrequest("LIST", "-", s, "wb", 0, 0, NULL, xlate, lcs,
                                   rcs) == 0));
  }
  success = 1; /* VDIR - one file at a time... */
  p = (char *)remote_files(1, (CHAR *)s, NULL, 0); /* Get the file list */
  cancelgroup = 0;
  if (!ftp_vbm && !quiet)
    printlines = 1;
  while (p && !cancelfile && !cancelgroup) { /* STAT one file */
    if (ftpcmd("STAT", p, lcs, rcs, ftp_vbm) < 0) {
      success = 0;
      break;
    }
    p = (char *)remote_files(0, NULL, NULL, 0); /* Get next file */
    debug(F110, "ftp vdir file", s, 0);
  }
  return (success);
}

static int doftppwd() { /* PWD */
  int x, lcs = -1, rcs = -1;
#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */
  if ((x = cmcfm()) < 0)
    return (x);
  CHECKCONN();
  if (ftpcmd("PWD", NULL, lcs, rcs, 1) == REPLY_COMPLETE) {
    success = 1;
  } else if (ftpcode == 500 || ftpcode == 502) {
    if (ftp_deb)
      printf("PWD command not recognized, trying XPWD\n");
    success = (ftpcmd("XPWD", NULL, lcs, rcs, 1) == REPLY_COMPLETE);
  }
  return (success);
}

static int doftpcwd(char *s, int vbm) /* CD (CWD) */
{
  int lcs = -1, rcs = -1;
#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */

  debug(F110, "ftp doftpcwd", s, 0);
  if (ftpcmd("CWD", s, lcs, rcs, vbm) == REPLY_COMPLETE)
    return (success = 1);
  if (ftpcode == 500 || ftpcode == 502) {
    if (!quiet)
      printf("CWD command not recognized, trying XCWD\n");
    if (ftpcmd("XCWD", s, lcs, rcs, vbm) == REPLY_COMPLETE)
      return (success = 1);
  }
  return (success = 0);
}

static int doftpcdup() { /* CDUP */
  debug(F100, "ftp doftpcdup", "", 0);
  if (ftpcmd("CDUP", NULL, 0, 0, 1) == REPLY_COMPLETE)
    return (success = 1);
  if (ftpcode == 500 || ftpcode == 502) {
    if (!quiet)
      printf("CDUP command not recognized, trying XCUP\n");
    if (ftpcmd("XCUP", NULL, 0, 0, 1) == REPLY_COMPLETE)
      return (success = 1);
  }
  return (success = 0);
}

/* s y n c d i r  --  Synchronizes client & server directories */

/*
  Call with:
    local = pointer to pathname of local file to be sent.
    sim   = 1 for simulation, 0 for real uploading.
  Returns 0 on failure, 1 on success.

  The 'local' argument is relative to the initial directory of the MPUT,
  i.e. the root of the tree being uploaded.  If the directory of the
  argument file is different from the directory of the previous file
  (which is stored in global putpath[]), this routine does the appropriate
  CWDs, CDUPs, and/or MKDIRs to position the FTP server in the same place.
*/
static int cdlevel = 0, cdsimlvl = 0; /* Tree-level trackers */

static int syncdir(char *local, int sim) {
  char buf[CKMAXPATH + 1];
  char tmp[CKMAXPATH + 1];
  char msgbuf[CKMAXPATH + 64];
  char c, *p = local, *s = buf, *q = buf, *psep, *ssep;
  int i, k = 0, done = 0, itsadir = 0, saveq;

  debug(F110, "ftp syncdir local (new)", local, 0);
  debug(F110, "ftp syncdir putpath (old)", putpath, 0);

  itsadir = isdir(local); /* Is the local file a directory? */
  saveq = quiet;

  while ((*s = *p)) {     /* Copy the argument filename */
    if (++k == CKMAXPATH) /* so we can poke it. */
      return (-1);
    if (*s == '/') /* Pointer to rightmost dirsep */
      q = s;
    s++;
    p++;
  }
  if (!itsadir) /* If it's a regular file */
    *q = NUL;   /* keep just the path part */

  debug(F110, "ftp syncdir buf", buf, 0);
  if (!strcmp(buf, putpath)) {  /* Same path as previous file? */
    if (itsadir) {              /* This file is a directory? */
      if (doftpcwd(local, 0)) { /* Try to CD to it */
        doftpcdup();            /* Worked - CD back up */
      } else if (sim) {         /* Simulating... */
        if (fdispla == XYFD_B) {
          printf("WOULD CREATE DIRECTORY %s\n", local);
        } else if (fdispla) {
          ckmakmsg(msgbuf, CKMAXPATH, "WOULD CREATE DIRECTORY", local, NULL,
                   NULL);
          ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, msgbuf);
        }
        /* See note above */
        return (0);
      } else if (!doftpxmkd(local, 0)) { /* Can't CD - try to create */
        return (0);
      } else { /* Remote directory created OK */
        if (fdispla == XYFD_B) {
          printf("CREATED DIRECTORY %s\n", local);
        } else if (fdispla) {
          ckmakmsg(msgbuf, CKMAXPATH + 64, "CREATED DIRECTORY ", local, NULL,
                   NULL);
          ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, msgbuf);
        }
      }
    }
    debug(F110, "ftp syncdir no change", buf, 0);
    return (1); /* Yes, done. */
  }
  ckstrncpy(tmp, buf, CKMAXPATH + 1); /* Make a safe (pre-poked) copy */
  debug(F110, "ftp syncdir new path", buf, 0); /* for later (see end) */

  p = buf;     /* New */
  s = putpath; /* Old */

  debug(F110, "ftp syncdir A (old) s", s, 0); /* Previous */
  debug(F110, "ftp syncdir A (new) p", p, 0); /* New */

  psep = buf;
  ssep = putpath;
  while (*p != NUL && *s != NUL && *p == *s) {
    if (*p == '/') {
      psep = p + 1;
      ssep = s + 1;
    }
    p++, s++;
  }
  /*
    psep and ssep point to the first path segment that differs.
    We have to do as many CDUPs as there are path segments in ssep.
    then we have to do as many MKDs and CWDs as there are segments in psep.
  */
  s = ssep;
  p = psep;

  debug(F110, "ftp syncdir B (old) s", s, 0); /* Previous */
  debug(F110, "ftp syncdir B (new) p", p, 0); /* New */

  /* p and s now point to the leftmost spot where the paths differ */

  if (*s) {              /* We have to back up */
    k = 1;               /* How many levels counting this one */
    while ((c = *s++)) { /* Count dirseps remaining in prev */
      if (c == '/' && *s)
        k++;
    }
    debug(F101, "ftp syncdir levels up", "", k);

    for (i = 1; i <= k; i++) { /* Do that many CDUPs */
      debug(F111, "ftp syncdir CDUP A", p, i);
      if (fdispla == XYFD_B)
        printf(" CDUP\n");
      if (sim && cdsimlvl) {
        cdsimlvl--;
      } else {
        if (!doftpcdup()) {
          quiet = saveq;
          return (0);
        }
      }
      cdlevel--;
    }
    if (!*p)     /* If we don't have to go down */
      goto xcwd; /* we're done. */
  }

  debug(F110, "ftp syncdir NEW PATH", p, 0);

  s = p;                    /* Point to start of new down path. */
  while (1) {               /* Loop through characters. */
    if (*s == '/' || !*s) { /* Have a segment. */
      if (!*s)              /* If end of string, */
        done++;             /* after this segment we're done. */
      else
        *s = NUL; /* NUL out the separator. */
      if (*p) {   /* If segment is not empty */
        debug(F110, "ftp syncdir down segment", p, 0);
        if (!doftpcwd(p, 0)) { /* Try to CD to it */
          if (sim) {
            if (fdispla == XYFD_B) {
              printf(" WOULD CREATE DIRECTORY %s\n", local);
            } else if (fdispla) {
              ckmakmsg(msgbuf, CKMAXPATH, "WOULD CREATE DIRECTORY", local, NULL,
                       NULL);
              ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, msgbuf);
            }
            cdsimlvl++;
          } else {
            if (!doftpxmkd(p, 0)) { /* Can't CD - try to create */
              debug(F110, "ftp syncdir mkdir failed", p, 0);
              /*
                Suppose we are executing SEND /RECURSIVE.  Locally we have a
                directory FOO but the remote has a regular file with the same
                name.  We can't CD to it, can't MKDIR it either.  There's no way
                out but to fail and let the user handle the problem.
              */
              quiet = saveq;
              return (0);
            }
            debug(F110, "ftp syncdir mkdir OK", p, 0);
            if (fdispla == XYFD_B) {
              printf(" CREATED DIRECTORY %s\n", p);
            } else if (fdispla) {
              ckmakmsg(msgbuf, CKMAXPATH, "CREATED DIRECTORY ", p, NULL, NULL);
              ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, msgbuf);
            }
            if (!doftpcwd(p, 0)) { /* Try again to CD */
              debug(F110, "ftp syncdir CD failed", p, 0);
              quiet = saveq;
              return (0);
            }
            if (fdispla == XYFD_B)
              printf(" CWD %s\n", p);
            debug(F110, "ftp syncdir CD OK", p, 0);
          }
        }
        cdlevel++;
      }
      if (done) /* Quit if no next segment */
        break;
      p = s + 1; /* Point to next segment */
    }
    s++; /* Point to next source char */
  }

xcwd:
  ckstrncpy(putpath, tmp, CKMAXPATH + 1); /* All OK - make this the new path */
  quiet = saveq;
  return (1);
}

#ifdef DOUPDATE
#ifdef DEBUG
static void dbtime(char *s, struct tm *xx) /* Write struct tm to debug log */
{
  if (deblog) {
    debug(F111, "ftp year ", s, xx->tm_year);
    debug(F111, "ftp month", s, xx->tm_mon);
    debug(F111, "ftp day  ", s, xx->tm_mday);
    debug(F111, "ftp hour ", s, xx->tm_hour);
    debug(F111, "ftp min  ", s, xx->tm_min);
    debug(F111, "ftp sec  ", s, xx->tm_sec);
  }
}
#endif /* DEBUG */

/*  t m c o m p a r e  --  Compare two struct tm's */

/*  Like strcmp() but for struct tm's  */
/*  Returns -1 if xx < yy, 0 if they are equal, 1 if xx > yy */

static int tmcompare(struct tm *xx, struct tm *yy) {
  if (xx->tm_year < yy->tm_year) /* First year less than second */
    return (-1);
  if (xx->tm_year > yy->tm_year) /* First year greater than second */
    return (1);

  /* Years are equal so compare months */

  if (xx->tm_mon < yy->tm_mon) /* And so on... */
    return (-1);
  if (xx->tm_mon > yy->tm_mon)
    return (1);

  if (xx->tm_mday < yy->tm_mday)
    return (-1);
  if (xx->tm_mday > yy->tm_mday)
    return (1);

  if (xx->tm_hour < yy->tm_hour)
    return (-1);
  if (xx->tm_hour > yy->tm_hour)
    return (1);

  if (xx->tm_min < yy->tm_min)
    return (-1);
  if (xx->tm_min > yy->tm_min)
    return (1);

  if (xx->tm_sec < yy->tm_sec)
    return (-1);
  if (xx->tm_sec > yy->tm_sec)
    return (1);

  return (0);
}
#endif /* DOUPDATE */

#ifndef HAVE_TIMEGM             /* For platforms that do not have timegm() */
static const int MONTHDAYS[] = {/* Number of days in each month. */
                                31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* Macro for whether a given year is a leap year. */
#define ISLEAP(year)                                                           \
  (((year) % 4) == 0 && (((year) % 100) != 0 || ((year) % 400) == 0))
#endif /* HAVE_TIMEGM */

/*  m k u t i m e  --  Like mktime() but argument is already UTC */

static time_t mkutime(struct tm *tm)
/* mkutime */ {
#ifdef HAVE_TIMEGM
  return (timegm(tm)); /* Have system service, use it. */
#else
  /*
    Contributed by Russ Allbery (rra@stanford.edu), used by permission.
    Given a struct tm representing a calendar time in UTC, convert it to
    seconds since epoch.  Returns (time_t) -1 if the time is not
    convertable.  Note that this function does not canonicalize the provided
    struct tm, nor does it allow out-of-range values or years before 1970.
    Result should be identical with timegm().
  */
  time_t result = 0;
  int i;
  /*
    We do allow some ill-formed dates, but we don't do anything special
    with them and our callers really shouldn't pass them to us.  Do
    explicitly disallow the ones that would cause invalid array accesses
    or other algorithm problems.
  */
#ifdef DEBUG
  if (deblog) {
    debug(F101, "mkutime tm_mon", "", tm->tm_mon);
    debug(F101, "mkutime tm_year", "", tm->tm_year);
  }
#endif /* DEBUG */
  if (tm->tm_mon < 0 || tm->tm_mon > 11 || tm->tm_year < 70)
    return ((time_t)-1);

  /* Convert to time_t. */
  for (i = 1970; i < tm->tm_year + 1900; i++)
    result += 365 + ISLEAP(i);
  for (i = 0; i < tm->tm_mon; i++)
    result += MONTHDAYS[i];
  if (tm->tm_mon > 1 && ISLEAP(tm->tm_year + 1900))
    result++;
  result = 24 * (result + tm->tm_mday - 1) + tm->tm_hour;
  result = 60 * result + tm->tm_min;
  result = 60 * result + tm->tm_sec;
  debug(F101, "mkutime result", "", result);
  return (result);
#endif /* HAVE_TIMEGM */
}

/*
  s e t m o d t i m e  --  Set file modification time.

  f = char * filename;
  t = time_t date/time to set (Secs since 19700101 0:00:00 UTC, NOT local)

  UNIX-specific; isolates mainline code from hideous #ifdefs.
  Returns:
    0 on success,
   -1 on error.

*/
static int setmodtime(char *f, time_t t)
/* setmodtime */ {
  struct stat sb;
  int x, rc = 0;
#ifdef BSD44
  struct timeval tp[2];
#else /* def BSD44 */
#ifdef V7
  struct utimbuf {
    time_t timep[2];
  } tp;
#else /* def V7 */
#ifdef SYSUTIMEH
  struct utimbuf tp;
#else  /* def SYSUTIMEH */
  struct utimbuf {
    time_t atime;
    time_t mtime;
  } tp;
#endif /* def SYSUTIMEH [else] */
#endif /* def V7 [else] */
#endif /* def BSD44 [else] */

  if (stat(f, &sb) < 0) {
    debug(F111, "setmodtime stat failure", f, errno);
    return (-1);
  }
#ifdef BSD44
  tp[0].tv_sec = sb.st_atime; /* Access time first */
  tp[1].tv_sec = t;           /* Update time second */
  debug(F111, "setmodtime BSD44", f, t);
#else
#ifdef V7
  tp.timep[0] = t;           /* Set modif. time to creation date */
  tp.timep[1] = sb.st_atime; /* Don't change the access time */
  debug(F111, "setmodtime V7", f, t);
#else
#ifdef SYSUTIMEH
  tp.modtime = t;          /* Set modif. time to creation date */
  tp.actime = sb.st_atime; /* Don't change the access time */
  debug(F111, "setmodtime SYSUTIMEH", f, t);
#else
  tp.mtime = t;           /* Set modif. time to creation date */
  tp.atime = sb.st_atime; /* Don't change the access time */
  debug(F111, "setmodtime (other)", f, t);
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* BSD44 */

  /* Try to set the file date */

#ifdef BSD44
  x = utimes(f, tp);
  debug(F111, "setmodtime utimes()", "BSD44", x);
#else
  x = utime(f, &tp);
  debug(F111, "setmodtime utime()", "other", x);
#endif /* BSD44 */
  if (x)
    rc = -1;

  debug(F101, "setmodtime result", "", rc);
  return (rc);
}

/*
  c h k m o d t i m e  --  Check/Set file modification time.

  fc = function code:
    0 = Check; returns:
      -1 on error,
       0 if local older than remote,
       1 if modtimes are equal,
       2 if local newer than remote.
    1 = Set (local file's modtime from remote's); returns:
      -1 on error,
       0 on success.
*/
static int chkmodtime(char *local, char *remote, int fc) {
  struct stat statbuf;
  struct tm *tmlocal = NULL;
  struct tm tmremote;
  int rc = 0, havedate = 0, lcs = -1, rcs = -1, flag = 0;
  char *s, timebuf[64];

  debug(F111, "chkmodtime", local, mdtmok);
  if (!mdtmok)   /* Server supports MDTM? */
    return (-1); /* No don't bother. */

#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */

  if (fc == 0) {
    rc = stat(local, &statbuf);
    if (rc == 0) { /* Get local file's mod time */
                   /* Convert to struct tm */
      tmlocal = gmtime((time_t *)&statbuf.st_mtime);
#ifdef DEBUG
      if (tmlocal) {
        dbtime(local, tmlocal);
      }
#endif /* DEBUG */
    }
  }
  /* Get remote file's mod time as yyyymmddhhmmss */

  if (havemdtm) { /* Already got it from MLSD? */
    s = havemdtm;
    flag++;
  } else if (ftpcmd("MDTM", remote, lcs, rcs, 0) == REPLY_COMPLETE) {
    char c;
    bzero((char *)&tmremote, sizeof(struct tm));
    s = ftp_reply_str;
    while ((c = *s++)) { /* Skip past response code */
      if (c == SP) {
        flag++;
        break;
      }
    }
  }
  if (flag) {
    debug(F111, "ftp chkmodtime string", s, flag);
    if (fts_sto) { /* User gave server time offset? */
      char *p;
      debug(F110, "ftp chkmodtime offset", fts_sto, 0);
      ckmakmsg(timebuf, 64, s, " ", fts_sto, NULL); /* Build delta time */
      if ((p = cmcvtdate(timebuf, 1))) {            /* Apply delta time */
        ckstrncpy(timebuf, p, 64);                  /* Convert to MDTM format */
        timebuf[8] = timebuf[9];                    /* h */
        timebuf[9] = timebuf[10];                   /* h */
        timebuf[10] = timebuf[12];                  /* m */
        timebuf[11] = timebuf[13];                  /* m */
        timebuf[12] = timebuf[12];                  /* s */
        timebuf[13] = timebuf[13];                  /* s */
        timebuf[14] = NUL;
        s = timebuf;
        debug(F110, "ftp chkmodtime adjust", s, 0);
      }
    }
    if (flag) { /* Convert to struct tm */
      char *pat;
      int y2kbug = 0; /* Seen in Kerberos 4 FTP servers */
      if (!ckstrcmp(s, "191", 3, 0)) {
        pat = "%05d%02d%02d%02d%02d%02d";
        y2kbug++;
        debug(F110, "ftp chkmodtime Y2K BUG detected", s, 0);
      } else {
        pat = "%04d%02d%02d%02d%02d%02d";
      }
      if (sscanf(s, /* Parse into struct tm */
                 pat, &(tmremote.tm_year), &(tmremote.tm_mon),
                 &(tmremote.tm_mday), &(tmremote.tm_hour), &(tmremote.tm_min),
                 &(tmremote.tm_sec)) == 6) {
        tmremote.tm_year -= (y2kbug ? 19000 : 1900);
        debug(F101, "ftp chkmodtime year", "", tmremote.tm_year);
        tmremote.tm_mon--;

#ifdef DEBUG
        debug(F100, "SERVER TIME FOLLOWS:", "", 0);
        dbtime(remote, &tmremote);
#endif /* DEBUG */

        if (havedate > -1)
          havedate = 1;
      }
    }
  } else { /* Failed */
    debug(F101, "ftp chkmodtime ftpcode", "", ftpcode);
    if (ftpcode == 500 || /* Command unrecognized */
        ftpcode == 502 || /* Command not implemented */
        ftpcode == 202)   /* Command superfluous */
      mdtmok = 0;         /* Don't ask this server again */
    return (-1);
  }
  if (fc == 0) {         /* Compare */
    if (havedate == 1) { /* Only if we have both file dates */
                         /*
                           Compare with local file's time.  We don't use
                           clock time (time_t) here in case of signed/unsigned
                           confusion, etc.
                         */
      int xx;
      xx = tmcompare(tmlocal, &tmremote);
      debug(F101, "chkmodtime tmcompare", "", xx);
      return (xx + 1);
    }
  } else if (ftp_dates) { /* Set */
    /*
      Here we must convert struct tm to time_t
      without applying timezone conversion, for which
      there is no portable API.  The method is hidden
      in mkutime(), defined above.
    */
    time_t utc;
    utc = mkutime(&tmremote);
    debug(F111, "ftp chkmodtime mkutime", remote, utc);
    if (utc != (time_t)-1)
      return (setmodtime(local, utc));
  }
  return (-1);
}

/* getfile() returns: -1 on error, 0 if file received, 1 if file skipped */

static int getfile(char *remote, char *local, int recover, int append,
                   char *pipename, int xlate, int fcs, int rcs) {
  int rc = -1;
  ULONG t0, t1;

#ifdef GFTIMER
  CKFLOAT sec;
#else
  int sec = 0;
#endif /* GFTIMER */
  char fullname[CKMAXPATH + 1];

  debug(F110, "ftp getfile remote A", remote, 0);
  debug(F110, "ftp getfile local A", local, 0);
  debug(F110, "ftp getfile pipename", pipename, 0);
  if (!remote)
    remote = "";

#ifdef PATTERNS
  /* Automatic type switching? */
  if (ftp_xfermode == XMODE_A && patterns && get_auto && !forcetype) {
    int x;
    x = matchname(remote, 0, servertype);
    debug(F111, "ftp getfile matchname", remote, x);
    switch (x) {
    case 0:
      ftp_typ = FTT_ASC;
      break;
    case 1:
      ftp_typ = tenex ? FTT_TEN : FTT_BIN;
      break;
    default:
      if (g_ftp_typ > -1)
        ftp_typ = g_ftp_typ;
    }
    changetype(ftp_typ, ftp_vbm);
    binary = ftp_typ; /* For file-transfer display */
  }
#endif /* PATTERNS */

#ifndef NOCSETS
  ftp_csx = -1; /* For file-transfer display */
  ftp_csl = -1; /* ... */

  if (rcs > -1)               /* -1 means no translation */
    if (ftp_typ == FTT_ASC)   /* File type is "ascii"? */
      if (fcs < 0)            /* File charset not forced? */
        fcs = fcharset;       /* use prevailing FILE CHARACTER-SET */
  if (fcs > -1 && rcs > -1) { /* Set up translation functions */
    debug(F110, "ftp getfile", "initxlate", 0);
    initxlate(rcs, fcs); /* NB: opposite order of PUT */
    ftp_csx = rcs;
    ftp_csl = fcs;
  } else
    xlate = 0;
#endif /* NOCSETS */

  if (!local)
    local = "";
  if (!pipename && !*local)
    local = remote;

  out2screen = !strcmp(local, "-");

  fullname[0] = NUL;
  if (pipename) {
    ckstrncpy(fullname, pipename, CKMAXPATH + 1);
  } else {
    zfnqfp(local, CKMAXPATH, fullname);
    if (!fullname[0])
      ckstrncpy(fullname, local, CKMAXPATH + 1);
  }
  if (!out2screen && displa && fdispla) { /* Screen */
    ftscreen(SCR_FN, 'F', (CK_OFF_T)pktnum, remote);
    ftscreen(SCR_AN, 0, (CK_OFF_T)0, fullname);
    ftscreen(SCR_FS, 0, fsize, "");
  }
  tlog(F110, ftp_typ ? "ftp get BINARY:" : "ftp get TEXT:", remote, 0);
  tlog(F110, " as", fullname, 0);
  debug(F111, "ftp getfile size", remote, fsize);
  debug(F111, "ftp getfile local", local, out2screen);

  ckstrncpy(filnam, pipename ? remote : local, CKMAXPATH);

  t0 = gmstimer();                           /* Start time */
  debug(F111, "ftp getfile t0", remote, t0); /* ^^^ */
  rc = recvrequest("RETR", local, remote, append ? "ab" : "wb", 0, recover,
                   pipename, xlate, fcs, rcs);
  t1 = gmstimer(); /* End time */
  debug(F111, "ftp getfile t1", remote, t1);
  debug(F111, "ftp getfile sec", remote, (t1 - t0) / 1000);
#ifdef GFTIMER
  sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
  fpxfsecs = sec;                               /* (for doxlog()) */
#else
  sec = (t1 - t0) / 1000;
  xfsecs = (int)sec;
#endif /* GFTIMER */

#ifdef FTP_TIMEOUT
  if (ftp_timed_out)
    rc = -4;
#endif /* FTP_TIMEOUT */

  debug(F111, "ftp recvrequest rc", remote, rc);
  if (cancelfile || cancelgroup) {
    debug(F111, "ftp get canceled", ckitoa(cancelfile), cancelgroup);
    ftscreen(SCR_ST, ST_INT, (CK_OFF_T)0, "");
  } else if (rc > 0) {
    debug(F111, "ftp get skipped", ckitoa(cancelfile), cancelgroup);
    ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)0, cmarg);
  } else if (rc < 0) {
    switch (ftpcode) {
    case -4: /* Network error */
    case -2: /* File error */
      ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, ck_errstr());
      break;
    case -3:
      ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, "Failure to make data connection");
      break;
    case -1: /* (should be covered above) */
      ftscreen(SCR_ST, ST_INT, (CK_OFF_T)0, "");
      break;
    default:
      ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, &ftp_reply_str[4]);
    }
  } else { /* Tudo bem */
    ftscreen(SCR_PT, 'Z', (CK_OFF_T)0, "");
    if (rc == 0) {
      ftscreen(SCR_ST, ST_OK, (CK_OFF_T)0, ""); /* For screen */
      makestr(&rrfspec, remote);                /* For WHERE command */
      makestr(&rfspec, fullname);
    }
  }
  if (rc > -1) {
    if (ftp_dates)                          /* If FTP DATES ON... */
      if (!pipename && !out2screen)         /* and it's a real file */
        if (rc < 1 && rc != -3)             /* and it wasn't skipped */
          if (connected)                    /* and we still have a connection */
            if (zchki(local) > -1) {        /* and the file wasn't discarded */
              chkmodtime(local, remote, 1); /* set local file date */
              debug(F110, "ftp get set date", local, 0);
            }
    filcnt++; /* Used by \v(filenum) */
  }
#ifdef TLOG
  if (tralog) {
    if (rc > 0) {
      tlog(F100, " recovery skipped", "", 0);
    } else if (rc == 0) {
      tlog(F101, " complete, size", "", fsize);
    } else if (cancelfile) {
      tlog(F100, " canceled by user", "", 0);
#ifdef FTP_TIMEOUT
    } else if (ftp_timed_out) {
      tlog(F100, " timed out", "", 0);
#endif /* FTP_TIMEOUT */
    } else {
      tlog(F110, " failed:", ftp_reply_str, 0);
    }
    if (!tlogfmt)
      doxlog(what, local, fsize, ftp_typ, rc, "");
  }
#endif /* TLOG */
  return (rc);
}

/* putfile() returns: -1 on error, >0 if file not selected, 0 on success. */
/* Positive return value is Skip Reason, SKP_xxx, from ckcker.h. */

static int putfile(int cx, char *local, char *remote, int force, int moving,
                   char *mvto, char *rnto, char *srvrn, int x_cnv, int x_usn,
                   int xft, int prm, int fcs, int rcs, int flg) {
  char asname[CKMAXPATH + 1];
  char fullname[CKMAXPATH + 1];
  int k = -1, x = 0, y = 0, o = -1, rc = 0, nc = 0;
  int xlate = 0, restart = 0, mt = -1;
  char *s = NULL, *cmd = NULL;
  ULONG t0 = 0, t1 = 0; /* Times for stats */
  int ofcs = 0, orcs = 0;

#ifdef GFTIMER
  CKFLOAT sec = 0.0;
#else
  int sec = 0;
#endif /* GFTIMER */
  debug(F111, "ftp putfile flg", local, flg);
  debug(F110, "ftp putfile srv_renam", srvrn, 0);
  debug(F101, "ftp putfile fcs", "", fcs);
  debug(F101, "ftp putfile rcs", "", rcs);

  ofcs = fcs; /* Save charset args */
  orcs = rcs;

  sendstart = (CK_OFF_T)0;
  restart = flg & PUT_RES;
  if (!remote)
    remote = "";

  /* FTP protocol command to send to server */
  cmd = (cx == FTP_APP) ? "APPE" : (x_usn ? "STOU" : "STOR");

  if (x_cnv == SET_AUTO) { /* Name conversion is auto */
    if (alike) {           /* If server & client are alike */
      nc = 0;              /* no conversion */
    } else {               /* If they are different */
      if (servertype == SYS_UNIX || servertype == SYS_WIN32)
        nc = -1; /* only minimal conversions needed */
      else       /* otherwise */
        nc = 1;  /* full conversion */
    }
  } else /* Not auto - do what user said */
    nc = x_cnv;

  /* If Transfer Mode is Automatic, determine file type */
  if (ftp_xfermode == XMODE_A && filepeek && !pipesend) {
    if (isdir(local)) { /* If it's a directory */
      k = FT_BIN;       /* skip the file scan */
    } else {
      debug(F110, "FTP PUT calling scanfile", local, 0);
      k = scanfile(local, &o, nscanfile); /* Scan the file */
    }
    debug(F111, "FTP PUT scanfile", local, k);
    if (k > -1 && !forcetype) {
      ftp_typ = (k == FT_BIN) ? 1 : 0;
      if (xft > -1 && ftp_typ != xft) {
        if (flg & PUT_SIM)
          tlog(F110, "ftp put SKIP (Type):", local, 0);
        return (SKP_TYP);
      }
      if (ftp_typ == 1 && tenex) /* User said TENEX? */
        ftp_typ = FTT_TEN;
    }
  }
#ifndef NOCSETS
  ftp_csx = -1; /* For file-transfer display */
  ftp_csl = -1; /* ... */

  if (rcs > -1) {         /* -1 means no translation */
    if (ftp_typ == 0) {   /* File type is "ascii"? */
      if (fcs < 0) {      /* File charset not forced? */
        if (k < 0) {      /* If we didn't scan */
          fcs = fcharset; /* use prevailing FILE CHARACTER-SET */
        } else {          /* If we did scan, use scan result */
          switch (k) {
          case FT_TEXT: /* Unknown text */
            fcs = fcharset;
            break;
          case FT_7BIT: /* 7-bit text */
            fcs = dcset7;
            break;
          case FT_8BIT: /* 8-bit text */
            fcs = dcset8;
            break;
          case FT_UTF8: /* UTF-8 */
            fcs = FC_UTF8;
            break;
          case FT_UCS2: /* UCS-2 */
            fcs = FC_UCS2;
            if (o > -1) /* Input file byte order */
              fileorder = o;
            break;
          default:
            rcs = -1;
          }
        }
      }
    }
  }
  if (fcs > -1 && rcs > -1) { /* Set up translation functions */
    debug(F110, "ftp putfile", "initxlate", 0);
    initxlate(fcs, rcs);
    debug(F111, "ftp putfile rcs", fcsinfo[rcs].keyword, rcs);
    xlate = 1;
    ftp_csx = rcs;
    ftp_csl = fcs;
  }
#endif /* NOCSETS */

  binary = ftp_typ; /* For file-transfer display */
  asname[0] = NUL;

  if (recursive) {                      /* If sending recursively, */
    if (!syncdir(local, flg & PUT_SIM)) /* synchronize directories. */
      return (-1);                      /* Don't PUT if it fails. */
    else if (isdir(local))              /* It's a directory */
      return (0);                       /* Don't send it! */
  }
  if (*remote) { /* If an as-name template was given */
#ifndef NOSPL
    if (cmd_quoting) { /* and COMMAND QUOTING is ON */
      y = CKMAXPATH;   /* evaluate it for this file */
      s = asname;
      zzstring(remote, &s, &y);
    } else
#endif                                       /* NOSPL */
      ckstrncpy(asname, remote, CKMAXPATH);  /* (or take it literally) */
  } else {                                   /* No as-name */
    nzltor(local, asname, nc, 0, CKMAXPATH); /* use local name strip path */
    debug(F110, "FTP PUT nzltor", asname, 0);
  }
  /* Preliminary messages and log entries */

  fullname[0] = NUL;
  zfnqfp(local, CKMAXPATH, fullname);
  if (!fullname[0])
    ckstrncpy(fullname, local, CKMAXPATH + 1);
  fullname[CKMAXPATH] = NUL;

  if (displa && fdispla) { /* Screen */
    ftscreen(SCR_FN, 'F', (CK_OFF_T)pktnum, local);
    ftscreen(SCR_AN, 0, (CK_OFF_T)0, asname);
    ftscreen(SCR_FS, 0, fsize, "");
  }
#ifdef DOUPDATE
  if (flg & (PUT_UPD | PUT_DIF)) { /* Date-checking modes... */
    mt = chkmodtime(fullname, asname, 0);
    debug(F111, "ftp putfile chkmodtime", asname, mt);
    if (mt == 0 && ((flg & PUT_DIF) == 0)) { /* Local is older */
      tlog(F110, "ftp put /update SKIP (Older modtime): ", fullname, 0);
      /* Skip this one */
      ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)SKP_DAT, fullname);
      filcnt++;
      return (SKP_DAT);
    } else if (mt == 1) { /* Times are equal */
      tlog(F110, "ftp put /update SKIP (Equal modtime): ", fullname, 0);
      ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)SKP_EQU, fullname); /* Skip it */
      filcnt++;
      return (SKP_DAT);
    }
    /* Local file is newer */
    tlog(F110, ftp_typ ? "ftp put /update BINARY:" : "ftp put /update TEXT:",
         fullname, 0);
  } else if (flg & PUT_RES) {
    tlog(F110, ftp_typ ? "ftp put /recover BINARY:" : "ftp put /recover TEXT:",
         fullname, 0);
  } else {
    tlog(F110, ftp_typ ? "ftp put BINARY:" : "ftp put TEXT:", fullname, 0);
  }
#else
  tlog(F110, ftp_typ ? "ftp put BINARY:" : "ftp put TEXT:", fullname, 0);
#endif /* DOUPDATE */
  tlog(F110, " as", asname, 0);

#ifndef NOCSETS
  if (xlate) {
    debug(F111, "ftp putfile fcs", fcsinfo[fcs].keyword, fcs);
    tlog(F110, " file character set:", fcsinfo[fcs].keyword, 0);
    tlog(F110, " server character set:", fcsinfo[rcs].keyword, 0);
  } else if (!ftp_typ) {
    tlog(F110, " character sets:", "no conversion", 0);
    fcs = ofcs; /* Binary file but we still must */
    rcs = orcs; /* translate its name */
  }
#endif /* NOCSETS */

  /* PUT THE FILE */

  t0 = gmstimer();                   /* Start time */
  if (flg & PUT_SIM) {               /* rc > 0 is a skip reason code */
    if (flg & (PUT_UPD | PUT_DIF)) { /* (see SKP_xxx in ckcker.h) */
      rc = (mt < 0) ?                /* Update mode... */
               SKP_XNX
                    :   /* Remote file doesn't exist */
               SKP_XUP; /* Remote file is older */
    } else {
      rc = SKP_SIM; /* "Would be sent", period. */
    }
  } else {
    rc = sendrequest(cmd, local, asname, xlate, fcs, rcs, restart);
  }
  t1 = gmstimer(); /* End time */
  filcnt++;        /* File number */

#ifdef GFTIMER
  sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
  fpxfsecs = sec;                               /* (for doxlog()) */
#else
  sec = (t1 - t0) / 1000;
  xfsecs = (int)sec;
#endif /* GFTIMER */

  debug(F111, "ftp sendrequest rc", local, rc);

  if (cancelfile || cancelgroup) {
    debug(F111, "ftp put canceled", ckitoa(cancelfile), cancelgroup);
    ftscreen(SCR_ST, ST_INT, (CK_OFF_T)0, "");
  } else if (rc > 0) {
    debug(F101, "ftp put skipped", local, rc);
    ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)rc, fullname);
  } else if (rc < 0) {
    debug(F111, "ftp put error", local, ftpcode);
    ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, &ftp_reply_str[4]);
  } else {
    debug(F111, "ftp put not canceled", ckitoa(displa), fdispla);
    ftscreen(SCR_PT, 'Z', (CK_OFF_T)0, "");
    debug(F111, "ftp put ST_OK", local, rc);
    ftscreen(SCR_ST, ST_OK, (CK_OFF_T)0, "");
    debug(F110, "ftp put old sfspec", sfspec, 0);
    makestr(&sfspec, fullname); /* For WHERE command */
    debug(F110, "ftp put new sfspec", sfspec, 0);
    debug(F110, "ftp put old srfspec", srfspec, 0);
    makestr(&srfspec, asname);
    debug(F110, "ftp put new srfspec", srfspec, 0);
  }

  /* Final log entries */

#ifdef TLOG
  if (tralog) {
    if (rc > 0) {
      switch (rc) {
      case SKP_XNX:
        tlog(F100, " /simulate: WOULD BE SENT:", "no remote file", 0);
        break;
      case SKP_XUP:
        tlog(F100, " /simulate: WOULD BE SENT:", "remote file older", 0);
        break;
      case SKP_SIM:
        tlog(F100, " /simulate: WOULD BE SENT", "", 0);
        break;
      default:
        tlog(F110, " skipped:", gskreason(rc), 0);
      }
    } else if (rc == 0) {
      tlog(F101, " complete, size", "", fsize);
    } else if (cancelfile) {
      tlog(F100, " canceled by user", "", 0);
    } else {
      tlog(F110, " failed:", ftp_reply_str, 0);
    }
    if (!tlogfmt)
      doxlog(what, local, fsize, ftp_typ, rc, "");
  }
#endif /* TLOG */

  if (rc < 0)    /* PUT did not succeed */
    return (-1); /* so done. */

  if (flg & PUT_SIM) /* Simulating, skip the rest. */
    return (SKP_SIM);

#ifdef UNIX
  /* Set permissions too? */

  if (prm) {           /* Change permissions? */
    s = zgperm(local); /* Get perms of local file */
    if (!s)
      s = "";
    x = strlen(s);
    if (x > 3)
      s += (x - 3);
    if (rdigits(s)) {
      ckmakmsg(ftpcmdbuf, FTP_BUFSIZ, s, " ", asname, NULL);
      x = ftpcmd("SITE CHMOD", ftpcmdbuf, fcs, rcs, ftp_vbm) == REPLY_COMPLETE;
      tlog(F110, x ? " chmod" : " chmod failed", s, 0);
      if (!x)
        return (-1);
    }
  }
#endif /* UNIX */

  /* Disposition of source file */

  if (moving) {
    x = zdelet(local);
    tlog(F110, (x > -1) ? " deleted" : " failed to delete", local, 0);
    if (x < 0)
      return (-1);
  } else if (mvto) {
    x = zrename(local, mvto);
    tlog(F110, (x > -1) ? " moved source to" : " failed to move source to",
         mvto, 0);
    if (x < 0)
      return (-1);
    /* ftscreen(SCR_ST,ST_MSG,(CK_OFF_T)0,mvto); */

  } else if (rnto) {
    char *s = rnto;
#ifndef NOSPL
    int y;                  /* Pass it thru the evaluator */
    extern int cmd_quoting; /* for \v(filename) */
    if (cmd_quoting) {      /* But only if cmd_quoting is on */
      y = CKMAXPATH;
      s = (char *)asname;
      zzstring(rnto, &s, &y);
      s = (char *)asname;
    }
#endif /* NOSPL */
    if (s)
      if (*s) {
        int x;
        x = zrename(local, s);
        tlog(F110,
             (x > -1) ? " renamed source file to"
                      : " failed to rename source file to",
             s, 0);
        if (x < 0)
          return (-1);
        /* ftscreen(SCR_ST,ST_MSG,(CK_OFF_T)0,s); */
      }
  }

  /* Disposition of destination file */

  if (srvrn) { /* /SERVER-RENAME: */
    char *s = srvrn;
#ifndef NOSPL
    int y;                  /* Pass it thru the evaluator */
    extern int cmd_quoting; /* for \v(filename) */
    debug(F111, "ftp putfile srvrn", s, 1);

    if (cmd_quoting) { /* But only if cmd_quoting is on */
      y = CKMAXPATH;
      s = (char *)fullname; /* We can recycle this buffer now */
      zzstring(srvrn, &s, &y);
      s = (char *)fullname;
    }
#endif /* NOSPL */
    debug(F111, "ftp putfile srvrn", s, 2);
    if (s)
      if (*s) {
        int x;
        x = ftp_rename(asname, s);
        debug(F111, "ftp putfile ftp_rename", asname, x);
        tlog(F110,
             (x > 0) ? " renamed destination file to"
                     : " failed to rename destination file to",
             s, 0);
        if (x < 1)
          return (-1);
      }
  }
  return (0);
}

/* xxout must only be used for ASCII transfers */
static int xxout(char c) {
  /* For Unix, DG, Stratus, Amiga, Gemdos, other */
  if (c == '\012') {
    if (zzout(dout, (CHAR)'\015') < 0)
      return (-1);
    ftpsnd.bytes++;
  }
  if (zzout(dout, (CHAR)c) < 0)
    return (-1);
  ftpsnd.bytes++;
  return (0);
}

static int scrnout(char c) { return (putchar(c)); }

static int pipeout(char c) { return (zmchout(c)); }

static int ispathsep(int c) {
  switch (servertype) {
  case SYS_VMS:
  case SYS_TOPS10:
  case SYS_TOPS20:
    return (((c == ']') || (c == '>') || (c == ':')) ? 1 : 0);
  case SYS_OS2:
  case SYS_WIN32:
  case SYS_DOS:
    return (((c == '\\') || (c == '/') || (c == ':')) ? 1 : 0);
  case SYS_VOS:
    return ((c == '>') ? 1 : 0);
  default:
    return ((c == '/') ? 1 : 0);
  }
}

static int iscanceled() {
#ifdef CK_CURSES
  extern int ck_repaint();
#endif /* CK_CURSES */
  int x, rc = 0;
  char c = 0;
  if (cancelfile)
    return (1);
  x = conchk();    /* Any chars waiting at console? */
  if (x-- > 0) {   /* Yes...  */
    c = coninc(5); /* Get one */
    switch (c) {
    case 032: /* Ctrl-X or X */
    case 'z':
    case 'Z':
      cancelgroup++; /* fall thru on purpose */
    case 030:        /* Ctrl-Z or Z */
    case 'x':
    case 'X':
      cancelfile++;
      rc++;
      break;
#ifdef CK_CURSES
    case 'L':
    case 'l':
    case 014: /* Ctrl-L or L or Ctrl-W */
    case 027:
      ck_repaint(); /* Refresh screen */
#endif              /* CK_CURSES */
    }
  }
  while (x-- > 0) /* Soak up any rest */
    c = coninc(1);
  return (rc);
}

#ifdef FTP_TIMEOUT
/* fc = 0 for read; 1 for write */
static int check_data_connection(int fd, int fc) {
  int x;
#ifdef BSDSELECT
  struct timeval tv;
  fd_set in, out, err;

  if (ftp_timeout < 1L)
    return (0);

  FD_ZERO(&in);
  FD_ZERO(&out);
  FD_ZERO(&err);
  FD_SET(fd, fc ? &out : &in);
  tv.tv_sec = ftp_timeout; /* Time limit */
  tv.tv_usec = 0L;

#ifdef INTSELECT
  x = select(FD_SETSIZE, (int *)&in, (int *)&out, (int *)&err, &tv);
#else
  x = select(FD_SETSIZE, &in, &out, &err, &tv);
#endif /* INTSELECT */
#else  /* BSDSELECT */
#ifdef IBMSELECT
  if (ftp_timeout < 1L)
    return (0);

  if (fc) { /* write */
    x = select(&fd, 0, 1, 0, ftp_timeout * 1000L);
  } else { /* read */
    x = select(&fd, 1, 0, 0, ftp_timeout * 1000L);
  }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
  if (x == 0) {
#ifdef EWOULDBLOCK
    errno = EWOULDBLOCK;
#else
#ifdef EAGAIN
    errno = EAGAIN;
#else
    errno = 11;
#endif /* EAGAIN */
#endif /* EWOULDBLOCK */
    debug(F100, "ftp check_data_connection TIMOUT", "", 0);
    return (-3);
  }
  return (0);
}
#endif /* FTP_TIMEOUT */

/* zzsend - used by buffered output macros. */

static int zzsend(int fd, CHAR c) {
  int rc;

  debug(F101, "zzsend ucbufsiz", "", ucbufsiz);
  debug(F101, "zzsend nout", "", nout);
  debug(F111, "zzsend", "secure?", ftpissecure());

  if (iscanceled()) /* Check for cancellation */
    return (-9);

#ifdef FTP_TIMEOUT
  ftp_timed_out = 0;
  if (check_data_connection(fd, 1) < 0) {
    ftp_timed_out = 1;
    return (-3);
  }
#endif /* FTP_TIMEOUT */

  rc = (!ftpissecure()) ? send(fd, (SENDARG2TYPE)ucbuf, nout, 0)
                        : secure_putbuf(fd, ucbuf, nout);
  ucbuf[nout] = NUL;
  nout = 0;
  ucbuf[nout++] = c;
  spackets++;
  pktnum++;
  if (rc > -1 && fdispla != XYFD_B) {
    spktl = nout;
    ftscreen(SCR_PT, 'D', (CK_OFF_T)spackets, NULL);
  }
  return (rc);
}

/* c m d l i n p u t  --  Command-line PUT */

int cmdlinput(int stay) {
  int x, rc = 0, done = 0, good = 0, status = 0;
  ULONG t0, t1; /* Times for stats */
#ifdef GFTIMER
  CKFLOAT sec;
#else
  int sec = 0;
#endif /* GFTIMER */

  if (quiet) { /* -q really means quiet */
    displa = 0;
    fdispla = 0;
  } else {
    displa = 1;
    fdispla = XYFD_B;
  }
  testing = 0;
  out2screen = 0;
  dpyactive = 0;
  what = W_FTP | W_SEND;

#ifndef NOSPL
  cmd_quoting = 0;
#endif /* NOSPL */
  sndsrc = nfils;

  t0 = gmstimer(); /* Record starting time */

  while (!done && !cancelgroup) { /* Loop for all files */

    cancelfile = 0;
    x = gnfile(); /* Get next file from list(s) */
    if (x == 0)   /* (see gnfile() comments...) */
      x = gnferror;

    switch (x) {
    case 1:                   /* File to send */
      rc = putfile(FTP_PUT,   /* Function (PUT, APPEND) */
                   filnam,    /* Local file to send */
                   filnam,    /* Remote name for file */
                   forcetype, /* Text/binary mode forced */
                   0,         /* Not moving */
                   NULL,      /* No move-to */
                   NULL,      /* No rename-to */
                   NULL,      /* No server-rename */
                   ftp_cnv,   /* Filename conversion */
                   0,         /* Unique-server-names */
                   -1,        /* All file types */
                   0,         /* No permissions */
                   -1,        /* No character sets */
                   -1,        /* No character sets */
                   0          /* No update or restart */
      );
      if (rc > -1) {
        good++;
        status = 1;
      }
      if (cancelfile) {
        continue; /* Or break? */
      }
      if (rc < 0) {
        ftp_fai++;
      }
      continue; /* Or break? */

    case 0: /* No more files, done */
      done++;
      continue;

    case -2:
    case -1:
      printf("?%s: file not found - \"%s\"\n", puterror ? "Fatal" : "Warning",
             filnam);
      continue; /* or break? */
    case -3:
      printf("?Warning access denied - \"%s\"\n", filnam);
      continue; /* or break? */
    case -5:
      printf("?Too many files match\n");
      done++;
      break;
    case -6:
      if (good < 1)
        printf("?No files selected\n");
      done++;
      break;
    default:
      printf("?getnextfile() - unknown failure\n");
      done++;
    }
  }
  if (status > 0) {
    if (cancelgroup)
      status = 0;
    else if (cancelfile && good < 1)
      status = 0;
  }
  success = status;
  x = success;
  if (x > -1) {
    lastxfer = W_FTP | W_SEND;
    xferstat = success;
  }
  t1 = gmstimer(); /* End time */
#ifdef GFTIMER
  sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
  if (!sec)
    sec = 0.001;
  fptsecs = sec;
#else
  sec = (t1 - t0) / 1000;
  if (!sec)
    sec = 1;
#endif /* GFTIMER */
  tfcps = (long)(tfc / sec);
  tsecs = (int)sec;
  lastxfer = W_FTP | W_SEND;
  xferstat = success;
  if (dpyactive)
    ftscreen(status > 0 ? SCR_TC : SCR_CW, 0, (CK_OFF_T)0, "");
  if (!stay)
    doexit(success ? GOOD_EXIT : BAD_EXIT, -1);
  return (success);
}

/*  d o f t p p u t  --  Parse and execute PUT, MPUT, and APPEND  */

int doftpput(int cx, int who) /* who == 1 for ftp, 0 for kermit */
{
  struct FDB sf, fl, sw, cm;
  int n, rc, confirmed = 0, wild = 0, getval = 0, mput = 0, done = 0;
  int x_cnv = 0, x_usn = 0, x_prm = 0, putflags = 0, status = 0, good = 0;
  char *s, *s2;

  int x_csl, x_csr = -1; /* Local and remote charsets */
  int x_xla = 0;
  int x_recurse = 0;
  char c, *p; /* Workers */
#ifdef PUTARRAY
  int range[2];           /* Array range */
  char **ap = NULL;       /* Array pointer */
  int arrayx = -1;        /* Array index */
#endif                    /* PUTARRAY */
  ULONG t0 = 0L, t1 = 0L; /* Times for stats */
#ifdef GFTIMER
  CKFLOAT sec;
#else
  int sec = 0;
#endif /* GFTIMER */

  struct stringint pv[SND_MAX + 1]; /* Temporary array for switch values */
  success = 0;                      /* Assume failure */
  forcetype = 0;                    /* No /TEXT or /BINARY given yet */
  out2screen = 0;                   /* Not outputting file to screen */
  putflags = 0;                     /* PUT options */
  x_cnv = ftp_cnv;                  /* Filename conversion */
  x_usn = ftp_usn;                  /* Unique server names */
  x_prm = ftp_prm;                  /* Permissions */
  if (x_prm == SET_AUTO)            /* Permissions AUTO */
    x_prm = alike;

#ifndef NOCSETS
  x_csr = ftp_csr; /* Inherit global server charset */
  x_csl = ftp_csl;
  if (x_csl < 0)
    x_csl = fcharset;
  x_xla = ftp_xla;
#endif /* NOCSETS */

  makestr(&filefile, NULL);   /* No filename list file yet. */
  makestr(&srv_renam, NULL);  /* Clear /SERVER-RENAME: */
  makestr(&snd_rename, NULL); /*  PUT /RENAME */
  makestr(&snd_move, NULL);   /*  PUT /MOVE */
  putpath[0] = NUL;           /* Initialize for syncdir(). */
  puterror = ftp_err;         /* Inherit global error action. */
  what = W_SEND | W_FTP;      /* What we're doing (sending w/FTP) */
  asnambuf[0] = NUL;          /* Clear as-name buffer */

  if (g_ftp_typ > -1) { /* Restore TYPE if saved */
    ftp_typ = g_ftp_typ;
    /* g_ftp_typ = -1; */
  }
  for (i = 0; i <= SND_MAX; i++) { /* Initialize switch values */
    pv[i].sval = NULL;             /* to null pointers */
    pv[i].ival = -1;               /* and -1 int values */
    pv[i].wval = (CK_OFF_T)-1;     /* and -1 wide values */
  }
  if (who == 0) { /* Called with unprefixed command */
    switch (cx) {
    case XXRSEN:
      pv[SND_RES].ival = 1;
      break;
    case XXCSEN:
      pv[SND_CMD].ival = 1;
      break;
    case XXMOVE:
      pv[SND_DEL].ival = 1;
      break;
    case XXMMOVE:
      pv[SND_DEL].ival = 1; /* fall thru */
    case XXMSE:
      mput++;
      break;
    }
  } else {
    if (cx == FTP_REP)
      pv[SND_RES].ival = 1;
    if (cx == FTP_MPU)
      mput++;
  }
  cmfdbi(&sw,                   /* First FDB - command switches */
         _CMKEY,                /* fcode */
         "Filename, or switch", /* hlpmsg */
         "",                    /* default */
         "",                    /* addtl string data */
         nputswi,               /* addtl numeric data 1: tbl size */
         4,                     /* addtl numeric data 2: 4 = cmswi */
         xxstring,              /* Processing function */
         putswi,                /* Keyword table */
         &sf                    /* Pointer to next FDB */
  );
  cmfdbi(&fl,    /* 3rd FDB - local filespec */
         _CMFLD, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         xxstring, NULL, &cm);
  cmfdbi(&cm,    /* 4th FDB - Confirmation */
         _CMCFM, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         NULL, NULL, NULL);

again:
  cmfdbi(&sf,    /* 2nd FDB - file to send */
         _CMIFI, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         /* 0 = parse files, 1 = parse files or dirs, 2 = skip symlinks */
         nolinks | x_recurse, /* addtl numeric data 1 */
         0,                   /* dirflg 0 means "not dirs only" */
         xxstring, NULL, &fl);

  while (1) {       /* Parse zero or more switches */
    x = cmfdb(&sw); /* Parse something */
    debug(F101, "ftp put cmfdb A", "", x);
    debug(F101, "ftp put fcode A", "", cmresult.fcode);
    if (x < 0)                    /* Error */
      goto xputx;                 /* or reparse needed */
    if (cmresult.fcode != _CMKEY) /* Break out of loop if not a switch */
      break;
    c = cmgbrk();                    /* Get break character */
    getval = (c == ':' || c == '='); /* to see how they ended the switch */
    if (getval && !(cmresult.kflags & CM_ARG)) {
      printf("?This switch does not take arguments\n");
      x = -9;
      goto xputx;
    }
    if (!getval && (cmgkwflgs() & CM_ARG)) {
      printf("?This switch requires an argument\n");
      x = -9;
      goto xputx;
    }
    n = cmresult.nresult; /* Numeric result = switch value */
    debug(F101, "ftp put switch", "", n);

    switch (n) {  /* Process the switch */
    case SND_AFT: /* Send /AFTER:date-time */
    case SND_BEF: /* Send /BEFORE:date-time */
    case SND_NAF: /* Send /NOT-AFTER:date-time */
    case SND_NBE: /* Send /NOT-BEFORE:date-time */
      if (!getval)
        break;
      if ((x = cmdate("File date-time", "", &s, 0, xxstring)) < 0) {
        if (x == -3) {
          printf("?Date-time required\n");
          x = -9;
        }
        goto xputx;
      }
      pv[n].ival = 1;
      makestr(&(pv[n].sval), s);
      break;

    case SND_ASN: /* /AS-NAME: */
      debug(F101, "ftp put /as-name getval", "", getval);
      if (!getval)
        break;
      if ((x = cmfld("Name to send under", "", &s, NULL)) < 0) {
        if (x == -3) {
          printf("?name required\n");
          x = -9;
        }
        goto xputx;
      }
      makestr(&(pv[n].sval), brstrip(s));
      debug(F110, "ftp put /as-name 1", pv[n].sval, 0);
      if (pv[n].sval)
        pv[n].ival = 1;
      break;

#ifdef PUTARRAY
    case SND_ARR: /* /ARRAY */
      if (!getval)
        break;
      ap = NULL;
      if ((x = cmfld("Array name (a single letter will do)", "", &s, NULL)) <
          0) {
        if (x == -3)
          break;
        else
          return (x);
      }
      if ((x = arraybounds(s, &(range[0]), &(range[1]))) < 0) {
        printf("?Bad array: %s\n", s);
        return (-9);
      }
      if (!(ap = a_ptr[x])) {
        printf("?No such array: %s\n", s);
        return (-9);
      }
      pv[n].ival = 1;
      pv[SND_CMD].ival = 0; /* Undo any conflicting ones... */
      pv[SND_RES].ival = 0;
      pv[SND_FIL].ival = 0;
      arrayx = x;
      break;
#endif /* PUTARRAY */

    case SND_BIN: /* /BINARY */
    case SND_TXT: /* /TEXT or /ASCII */
    case SND_TEN: /* /TENEX */
      pv[SND_BIN].ival = 0;
      pv[SND_TXT].ival = 0;
      pv[SND_TEN].ival = 0;
      pv[n].ival = 1;
      break;

#ifdef PUTPIPE
    case SND_CMD: /* These take no args */
      if (nopush) {
        printf("?Sorry, system command access is disabled\n");
        x = -9;
        goto xputx;
      }
#ifdef PIPESEND
      else if (sndfilter) {
        printf("?Sorry, no PUT /COMMAND when SEND FILTER selected\n");
        x = -9;
        goto xputx;
      }
#endif                                  /* PIPESEND */
      sw.hlpmsg = "Command, or switch"; /* Change help message */
      pv[n].ival = 1;                   /* Just set the flag */
      pv[SND_ARR].ival = 0;
      break;
#endif /* PUTPIPE */

#ifdef CKSYMLINK
    case SND_LNK:
      nolinks = 0;
      goto again; /* Because CMIFI params changed... */
    case SND_NLK:
      nolinks = 2;
      goto again;
#endif /* CKSYMLINK */

#ifdef FTP_RESTART
    case SND_RES:           /* /RECOVER (resend) */
      pv[SND_ARR].ival = 0; /* fall thru on purpose... */
#endif                      /* FTP_RESTART */

    case SND_NOB:
    case SND_DEL:     /* /DELETE */
    case SND_SHH:     /* /QUIET */
    case SND_UPD:     /* /UPDATE */
    case SND_SIM:     /* /UPDATE */
    case SND_USN:     /* /UNIQUE */
      pv[n].ival = 1; /* Just set the flag */
      break;

    case SND_REC:    /* /RECURSIVE */
      recursive = 2; /* Must be set before cmifi() */
      x_recurse = 1;
      goto again; /* Because CMIFI params changed... */
      break;

    case SND_DOT: /* /DOTFILES */
      matchdot = 1;
      break;
    case SND_NOD: /* /NODOTFILES */
      matchdot = 0;
      break;

    case SND_ERR: /* /ERROR-ACTION */
      if ((x = cmkey(qorp, 2, "", "", xxstring)) < 0)
        goto xputx;
      pv[n].ival = x;
      break;

    case SND_EXC: /* Excludes */
      if (!getval)
        break;
      if ((x = cmfld("Pattern", "", &s, xxstring)) < 0) {
        if (x == -3) {
          printf("?Pattern required\n");
          x = -9;
        }
        goto xputx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s);
      if (pv[n].sval)
        pv[n].ival = 1;
      break;

    case SND_PRM: /* /PERMISSIONS */
      if (!getval)
        x = 1;
      else if ((x = cmkey(onoff, 2, "", "on", xxstring)) < 0)
        goto xputx;
      pv[SND_PRM].ival = x;
      break;

#ifdef PIPESEND
    case SND_FLT: /* /FILTER */
      debug(F101, "ftp put /filter getval", "", getval);
      if (!getval)
        break;
      if ((x = cmfld("Filter program to send through", "", &s, NULL)) < 0) {
        if (x == -3)
          s = "";
        else
          goto xputx;
      }
      if (*s)
        s = brstrip(s);
      y = strlen(s);
      for (x = 0; x < y; x++) { /* Make sure they included "\v(...)" */
        if (s[x] != '\\')
          continue;
        if (s[x + 1] == 'v')
          break;
      }
      if (x == y) {
        printf("?Filter must contain a replacement variable for filename.\n");
        x = -9;
        goto xputx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s);
      if (pv[n].sval)
        pv[n].ival = 1;
      break;
#endif /* PIPESEND */

    case SND_NAM: /* /FILENAMES */
      if (!getval)
        break;
      if ((x = cmkey(fntab, nfntab, "", "automatic", xxstring)) < 0)
        goto xputx;
      debug(F101, "ftp put /filenames", "", x);
      pv[n].ival = x;
      break;

    case SND_SMA: /* Smaller / larger than */
    case SND_LAR: {
      CK_OFF_T y;
      if (!getval)
        break;
      if ((x = cmnumw("Size in bytes", "0", 10, &y, xxstring)) < 0)
        goto xputx;
      pv[n].wval = y;
      break;
    }
    case SND_FIL: /* Name of file containing filenames */
      if (!getval)
        break;
      if ((x = cmifi("Name of file containing list of filenames", "", &s, &y,
                     xxstring)) < 0) {
        if (x == -3) {
          printf("?Filename required\n");
          x = -9;
        }
        goto xputx;
      } else if (y && iswild(s)) {
        printf("?Wildcards not allowed\n");
        x = -9;
        goto xputx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s);
      if (pv[n].sval) {
        pv[n].ival = 1;
        pv[SND_ARR].ival = 0;
      } else {
        pv[n].ival = 0;
      }
      mput = 0;
      break;

    case SND_MOV:   /* MOVE after */
    case SND_REN:   /* RENAME after */
    case SND_SRN: { /* SERVER-RENAME after */
      char *m = "";
      switch (n) {
      case SND_MOV:
        m = "device and/or directory for source file after sending";
        break;
      case SND_REN:
        m = "new name for source file after sending";
        break;
      case SND_SRN:
        m = "new name for destination file after sending";
        break;
      }
      if (!getval)
        break;
      if ((x = cmfld(m, "", &s, n == SND_MOV ? xxstring : NULL)) < 0) {
        if (x == -3) {
          printf("%s\n",
                 n == SND_MOV ? "?Destination required" : "?New name required");
          x = -9;
        }
        goto xputx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s ? brstrip(s) : NULL);
      pv[n].ival = (pv[n].sval) ? 1 : 0;
      break;
    }
    case SND_STA: /* Starting position (= PSEND) */
      if (!getval)
        break;
      if ((x = cmnum("0-based position", "0", 10, &y, xxstring)) < 0)
        goto xputx;
      pv[n].ival = y;
      break;

    case SND_TYP: /* /TYPE */
      if (!getval)
        break;
      if ((x = cmkey(txtbin, 3, "", "all", xxstring)) < 0)
        goto xputx;
      pv[n].ival = (x == 2) ? -1 : x;
      break;

#ifndef NOCSETS
    case SND_CSL: /* Local character set */
    case SND_CSR: /* Remote (server) charset */
      if ((x = cmkey(fcstab, nfilc, "", "", xxstring)) < 0) {
        return ((x == -3) ? -2 : x);
      }
      if (n == SND_CSL)
        x_csl = x;
      else
        x_csr = x;
      x_xla = 1; /* Overrides global OFF setting */
      break;

    case SND_XPA: /* Transparent */
      x_xla = 0;
      x_csr = -1;
      x_csl = -1;
      break;
#endif /* NOCSETS */
    }
  }
#ifdef PIPESEND
  if (pv[SND_RES].ival > 0) { /* /RECOVER */
    if (sndfilter || pv[SND_FLT].ival > 0) {
      printf("?Sorry, no /RECOVER or /START if SEND FILTER selected\n");
      x = -9;
      goto xputx;
    }
    if (sfttab[0] > 0 && sfttab[SFT_REST] == 0)
      printf("WARNING: Server says it doesn't support REST.\n");
  }
#endif /* PIPESEND */

  cmarg = "";
  cmarg2 = asnambuf;
  line[0] = NUL;
  s = line;
  wild = 0;

  switch (cmresult.fcode) { /* How did we get out of switch loop */
  case _CMIFI:              /* Input filename */
    if (pv[SND_FIL].ival > 0) {
      printf("?You may not give a PUT filespec and a /LISTFILE\n");
      x = -9;
      goto xputx;
    }
    ckstrncpy(line, cmresult.sresult, LINBUFSIZ); /* Name */
    if (pv[SND_ARR].ival > 0)
      ckstrncpy(asnambuf, line, CKMAXPATH);
    else
      wild = cmresult.nresult; /* Wild flag */
    debug(F111, "ftp put wild", line, wild);
    if (!wild && !recursive && !mput)
      nolinks = 0;
    break;
  case _CMFLD: /* Field */
    /* Only allowed with /COMMAND and /ARRAY */
    if (pv[SND_FIL].ival > 0) {
      printf("?You may not give a PUT filespec and a /LISTFILE\n");
      x = -9;
      goto xputx;
    }
    /* For MPUT it's OK to have filespecs that don't match any files */
    if (mput)
      break;
    if (pv[SND_CMD].ival < 1 && pv[SND_ARR].ival < 1) {
#ifdef CKROOT
      if (ckrooterr)
        printf("?Off limits: %s\n", cmresult.sresult);
      else
#endif /* CKROOT */
        printf("?%s - \"%s\"\n",
               iswild(cmresult.sresult) ? "No files match" : "File not found",
               cmresult.sresult);
      x = -9;
      goto xputx;
    }
    ckstrncpy(line, cmresult.sresult, LINBUFSIZ);
    if (pv[SND_ARR].ival > 0)
      ckstrncpy(asnambuf, line, CKMAXPATH);
    break;
  case _CMCFM: /* Confirmation */
    confirmed = 1;
    break;
  default:
    printf("?Unexpected function code: %d\n", cmresult.fcode);
    x = -9;
    goto xputx;
  }
  debug(F110, "ftp put string", s, 0);
  debug(F101, "ftp put confirmed", "", confirmed);

  /* Save and change protocol and transfer mode */
  /* Global values are restored in main parse loop */

  g_displa = fdispla;
  if (ftp_dis > -1)
    fdispla = ftp_dis;
  g_skipbup = skipbup;

  if (pv[SND_NOB].ival > -1) { /* /NOBACKUP (skip backup file) */
    g_skipbup = skipbup;
    skipbup = 1;
  }
  if (pv[SND_TYP].ival > -1) { /* /TYPE */
    xfiletype = pv[SND_TYP].ival;
    if (xfiletype == 2)
      xfiletype = -1;
  }
  if (pv[SND_BIN].ival > 0) {        /* /BINARY really means binary... */
    forcetype = 1;                   /* So skip file scan */
    ftp_typ = FTT_BIN;               /* Set binary */
  } else if (pv[SND_TXT].ival > 0) { /* Similarly for /TEXT... */
    forcetype = 1;
    ftp_typ = FTT_ASC;
  } else if (pv[SND_TEN].ival > 0) { /* and /TENEX*/
    forcetype = 1;
    ftp_typ = FTT_TEN;
  } else if (ftp_cmdlin && ftp_xfermode == XMODE_M) {
    forcetype = 1;
    ftp_typ = binary;
    g_ftp_typ = binary;
  }

#ifdef PIPESEND
  if (pv[SND_CMD].ival > 0) { /* /COMMAND - strip any braces */
    debug(F110, "PUT /COMMAND before stripping", s, 0);
    s = brstrip(s);
    debug(F110, "PUT /COMMAND after stripping", s, 0);
    if (!*s) {
      printf("?Sorry, a command to send from is required\n");
      x = -9;
      goto xputx;
    }
    cmarg = s;
  }
#endif /* PIPESEND */

  /* Set up /MOVE and /RENAME */

  if (pv[SND_DEL].ival > 0 && (pv[SND_MOV].ival > 0 || pv[SND_REN].ival > 0)) {
    printf("?Sorry, /DELETE conflicts with /MOVE or /RENAME\n");
    x = -9;
    goto xputx;
  }
#ifdef CK_TMPDIR
  if (pv[SND_MOV].ival > 0) {
    int len;
    char *p = pv[SND_MOV].sval;
    len = strlen(p);
    if (!isdir(p)) { /* Check directory */
#ifdef CK_MKDIR
      char *s = NULL;
      s = (char *)malloc(len + 4);
      if (s) {
        strcpy(s, p); /* safe */
        if (s[len - 1] != '/') {
          s[len++] = '/';
          s[len] = NUL;
        }
        s[len++] = 'X';
        s[len] = NUL;
#ifdef NOMKDIR
        x = -1;
#else
        x = zmkdir(s);
#endif /* NOMKDIR */
        free(s);
        if (x < 0) {
          printf("?Can't create \"%s\"\n", p);
          x = -9;
          goto xputx;
        }
      }
#else
      printf("?Directory \"%s\" not found\n", p);
      x = -9;
      goto xputx;
#endif /* CK_MKDIR */
    }
    makestr(&snd_move, p);
  }
#endif /* CK_TMPDIR */

  if (pv[SND_REN].ival > 0) { /* /RENAME */
    char *p = pv[SND_REN].sval;
    if (!p)
      p = "";
    if (!*p) {
      printf("?New name required for /RENAME\n");
      x = -9;
      goto xputx;
    }
    p = brstrip(p);
#ifndef NOSPL
    /* If name given is wild, rename string must contain variables */
    if (wild) {
      char *s = tmpbuf;
      x = TMPBUFSIZ;
      zzstring(p, &s, &x);
      if (!strcmp(tmpbuf, p)) {
        printf("?/RENAME for file group must contain variables such as "
               "\\v(filename)\n");
        x = -9;
        goto xputx;
      }
    }
#endif /* NOSPL */
    makestr(&snd_rename, p);
    debug(F110, "FTP snd_rename", snd_rename, 0);
  }
  if (pv[SND_SRN].ival > 0) { /* /SERVER-RENAME */
    char *p = pv[SND_SRN].sval;
    if (!p)
      p = "";
    if (!*p) {
      printf("?New name required for /SERVER-RENAME\n");
      x = -9;
      goto xputx;
    }
    p = brstrip(p);
#ifndef NOSPL
    if (wild) {
      char *s = tmpbuf;
      x = TMPBUFSIZ;
      zzstring(p, &s, &x);
      if (!strcmp(tmpbuf, p)) {
        printf("?/SERVER-RENAME for file group must contain variables such as "
               "\\v(filename)\n");
        x = -9;
        goto xputx;
      }
    }
#endif /* NOSPL */
    makestr(&srv_renam, p);
    debug(F110, "ftp put srv_renam", srv_renam, 0);
  }
  if (!confirmed) { /* CR not typed yet, get more fields */
    char *lp;
    if (mput) {  /* MPUT or MMOVE */
      nfils = 0; /* We already have the first one */
#ifndef NOMSEND
      if (cmresult.fcode == _CMIFI) {
        /* First filespec is valid */
        msfiles[nfils++] = line;           /* Store pointer */
        lp = line + (int)strlen(line) + 1; /* Point past it */
        debug(F111, "ftp put mput", msfiles[nfils - 1], nfils - 1);
      } else {
        /* First filespec matches no files */
        debug(F110, "ftp put mput skipping first filespec", cmresult.sresult,
              0);
        lp = line;
      }
      /* Parse a filespec, a "field", or confirmation */

      cmfdbi(&sf,                 /* 1st FDB - file to send */
             _CMIFI,              /* fcode */
             "",                  /* hlpmsg */
             "",                  /* default */
             "",                  /* addtl string data */
             nolinks | x_recurse, /* addtl numeric data 1 */
             0,                   /* dirflg 0 means "not dirs only" */
             xxstring, NULL, &fl);
      cmfdbi(&fl,    /* 2nd FDB - local filespec */
             _CMFLD, /* fcode */
             "",     /* hlpmsg */
             "",     /* default */
             "",     /* addtl string data */
             0,      /* addtl numeric data 1 */
             0,      /* addtl numeric data 2 */
             xxstring, NULL, &cm);
      cmfdbi(&cm,    /* 3rd FDB - Confirmation */
             _CMCFM, /* fcode */
             "", "", "", 0, 0, NULL, NULL, NULL);

      while (!confirmed) { /* Get more filenames */
        x = cmfdb(&sf);    /* Parse something */
        debug(F101, "ftp put cmfdb B", "", x);
        debug(F101, "ftp put fcode B", "", cmresult.fcode);
        if (x < 0)    /* Error */
          goto xputx; /* or reparse needed */
        switch (cmresult.fcode) {
        case _CMCFM: /* End of command */
          confirmed++;
          if (nfils < 1) {
            debug(F100, "ftp put mput no files match", "", 0);
            printf("?No files match MPUT list\n");
            x = -9;
            goto xputx;
          }
          break;
        case _CMFLD: /* No match */
          debug(F110, "ftp put mput skipping", cmresult.sresult, 0);
          continue;
        case _CMIFI: /* Good match */
          s = cmresult.sresult;
          msfiles[nfils++] = lp;           /* Got one, count, point to it, */
          p = lp;                          /* remember pointer, */
          while ((*lp++ = *s++))           /* and copy it into buffer */
            if (lp > (line + LINBUFSIZ)) { /* Avoid memory leak */
              printf("?MPUT list too long\n");
              line[0] = NUL;
              x = -9;
              goto xputx;
            }
          debug(F111, "ftp put mput adding", msfiles[nfils - 1], nfils - 1);
          if (nfils == 1) /* Take care of \v(filespec) */
            fspec[0] = NUL;
#ifdef ZFNQFP
          zfnqfp(p, TMPBUFSIZ, tmpbuf);
          p = tmpbuf;
#endif /* ZFNQFP */
          if (((int)strlen(fspec) + (int)strlen(p) + 1) < fspeclen) {
            strcat(fspec, p);   /* safe */
            strcat(fspec, " "); /* safe */
          } else {
            debug(F101, "doxput filespec buffer overflow", "", 0);
          }
        }
      }
#endif       /* NOMSEND */
    } else { /* Regular PUT */
      nfils = -1;
      if ((x = cmtxt(
               wild
                   ? "\nOptional as-name template containing replacement variables \
like \\v(filename)"
                   : "Optional name to send it with",
               "", &p, NULL)) < 0)
        goto xputx;

      if (p)
        if (!*p)
          p = NULL;
      p = brstrip(p);

      if (p && *p) {
        makestr(&(pv[SND_ASN].sval), p);
        if (pv[SND_ASN].sval)
          pv[SND_ASN].ival = 1;
        debug(F110, "ftp put /as-name 2", pv[SND_ASN].sval, 0);
      }
    }
  }
  /* Set cmarg2 from as-name, however we got it. */

  CHECKCONN();
  if (pv[SND_ASN].ival > 0 && pv[SND_ASN].sval && !asnambuf[0]) {
    char *p;
    p = brstrip(pv[SND_ASN].sval);
    ckstrncpy(asnambuf, p, CKMAXPATH + 1);
  }
  debug(F110, "ftp put asnambuf", asnambuf, 0);

  if (pv[SND_FIL].ival > 0) {
    if (confirmed) {
      if (zopeni(ZMFILE, pv[SND_FIL].sval) < 1) {
        debug(F110, "ftp put can't open", pv[SND_FIL].sval, 0);
        printf("?Failure to open %s\n", pv[SND_FIL].sval);
        x = -9;
        goto xputx;
      }
      makestr(&filefile, pv[SND_FIL].sval); /* Open, remember name */
      debug(F110, "ftp PUT /LISTFILE opened", filefile, 0);
      wild = 1;
    }
  }
  if (confirmed && !line[0] && !filefile) {
#ifndef NOMSEND
    if (filehead) { /* OK if we have a SEND-LIST */
      nfils = filesinlist;
      sndsrc = nfils; /* Like MSEND */
      addlist = 1;    /* But using a different list... */
      filenext = filehead;
      goto doput;
    }
#endif /* NOMSEND */
    printf("?Filename required but not given\n");
    x = -9;
    goto xputx;
  }
#ifndef NOMSEND
  addlist = 0; /* Don't use SEND-LIST. */
#endif         /* NOMSEND */

  if (mput) { /* MPUT (rather than PUT) */
#ifndef NOMSEND
    cmlist = msfiles; /* List of filespecs */
    sndsrc = nfils;   /* rather filespec and as-name */
#endif                /* NOMSEND */
    pipesend = 0;
  } else if (filefile) { /* File contains list of filenames */
    s = "";
    cmarg = "";
    line[0] = NUL;
    nfils = 1;
    sndsrc = 1;

  } else if (pv[SND_ARR].ival < 1 && pv[SND_CMD].ival < 1) {

    /* Not MSEND, MMOVE, /LIST, or /ARRAY */
    nfils = sndsrc = -1;
    if (!wild) {
      if (zchki(s) < 0) {
        printf("?Read access denied - \"%s\"\n", s);
        x = -9;
        goto xputx;
      }
    }
    if (s != line)                   /* We might already have done this. */
      ckstrncpy(line, s, LINBUFSIZ); /* Copy of string just parsed. */
#ifdef DEBUG
    else
      debug(F110, "doxput line=s", line, 0);
#endif            /* DEBUG */
    cmarg = line; /* File to send */
  }
#ifndef NOMSEND
  zfnqfp(cmarg, fspeclen, fspec); /* Get full name */
#endif                            /* NOMSEND */

  if (!mput) { /* For all but MPUT... */
#ifdef PIPESEND
    if (pv[SND_CMD].ival > 0) /* /COMMAND sets pipesend flag */
      pipesend = 1;
    debug(F101, "ftp put /COMMAND pipesend", "", pipesend);
    if (pipesend && filefile) {
      printf("?Invalid switch combination\n");
      x = -9;
      goto xputx;
    }
#endif /* PIPESEND */

#ifndef NOSPL
    /* If as-name given and filespec is wild, as-name must contain variables */
    if ((wild || mput) && asnambuf[0]) {
      char *s = tmpbuf;
      x = TMPBUFSIZ;
      zzstring(asnambuf, &s, &x);
      if (!strcmp(tmpbuf, asnambuf)) {
        printf("?As-name for file group must contain variables such as "
               "\\v(filename)\n");
        x = -9;
        goto xputx;
      }
    }
#endif /* NOSPL */
  }

doput:

  if (pv[SND_SHH].ival > 0) { /* SEND /QUIET... */
    fdispla = 0;
    debug(F101, "ftp put display", "", fdispla);
  } else {
    displa = 1;
    if (ftp_deb)
      fdispla = XYFD_B;
  }

#ifdef PUTARRAY /* SEND /ARRAY... */
  if (pv[SND_ARR].ival > 0) {
    if (!ap) {
      x = -2;
      goto xputx;
    } /* (shouldn't happen) */
    if (range[0] == -1)         /* If low end of range not specified */
      range[0] = 1;             /* default to 1 */
    if (range[1] == -1)         /* If high not specified */
      range[1] = a_dim[arrayx]; /* default to size of array */
    if ((range[0] < 0) ||       /* Check range */
        (range[0] > a_dim[arrayx]) || (range[1] < range[0]) ||
        (range[1] > a_dim[arrayx])) {
      printf("?Bad array range - [%d:%d]\n", range[0], range[1]);
      x = -9;
      goto xputx;
    }
    sndarray = ap;     /* Array pointer */
    sndxin = arrayx;   /* Array index */
    sndxlo = range[0]; /* Array range */
    sndxhi = range[1];
    sndxnam[7] = (char)((sndxin == 1) ? 64 : sndxin + ARRAYBASE);
    if (!asnambuf[0])
      ckstrncpy(asnambuf, sndxnam, CKMAXPATH);
    cmarg = "";
  }
#endif /* PUTARRAY */

  moving = 0;

  if (pv[SND_ARR].ival < 1) { /* File selection & disposition... */
    if (pv[SND_DEL].ival > 0) /* /DELETE was specified */
      moving = 1;
    if (pv[SND_AFT].ival > 0) /* Copy SEND criteria */
      ckstrncpy(sndafter, pv[SND_AFT].sval, 19);
    if (pv[SND_BEF].ival > 0)
      ckstrncpy(sndbefore, pv[SND_BEF].sval, 19);
    if (pv[SND_NAF].ival > 0)
      ckstrncpy(sndnafter, pv[SND_NAF].sval, 19);
    if (pv[SND_NBE].ival > 0)
      ckstrncpy(sndnbefore, pv[SND_NBE].sval, 19);
    if (pv[SND_EXC].ival > 0)
      makelist(pv[SND_EXC].sval, sndexcept, NSNDEXCEPT);
    if (pv[SND_SMA].ival > -1)
      sndsmaller = pv[SND_SMA].wval;
    if (pv[SND_LAR].ival > -1)
      sndlarger = pv[SND_LAR].wval;
    if (pv[SND_NAM].ival > -1)
      x_cnv = pv[SND_NAM].ival;
    if (pv[SND_USN].ival > -1)
      x_usn = pv[SND_USN].ival;
    if (pv[SND_ERR].ival > -1)
      puterror = pv[SND_ERR].ival;

#ifdef DOUPDATE
    if (pv[SND_UPD].ival > 0) {
      if (x_usn) {
        printf("?Conflicting switches: /UPDATE /UNIQUE\n");
        x = -9;
        goto xputx;
      }
      putflags |= PUT_UPD;
      ftp_dates |= 2;
    }
#endif /* DOUPDATE */

    if (pv[SND_SIM].ival > 0)
      putflags |= PUT_SIM;

    if (pv[SND_PRM].ival > -1) {
#ifdef UNIX
      if (x_usn) {
        printf("?Conflicting switches: /PERMISSIONS /UNIQUE\n");
        x = -9;
        goto xputx;
      }
      x_prm = pv[SND_PRM].ival;
#else  /* UNIX */
      printf("?/PERMISSIONS switch is not supported\n");
#endif /* UNIX */
    }
#ifdef FTP_RESTART
    if (pv[SND_RES].ival > 0) {
      if (!sizeok) {
        printf("?PUT /RESTART can't be used because SIZE disabled.\n");
        x = -9;
        goto xputx;
      }
      if (x_usn || putflags) {
        printf("?Conflicting switches: /RECOVER %s\n",
               x_usn && putflags ? "/UNIQUE /UPDATE"
                                 : (x_usn ? "/UNIQUE" : "/UPDATE"));
        x = -9;
        goto xputx;
      }
#ifndef NOCSETS
      if (x_xla && (x_csl == FC_UCS2 || x_csl == FC_UTF8 || x_csr == FC_UCS2 ||
                    x_csr == FC_UTF8)) {
        printf("?/RECOVER can not be used with Unicode translation\n");
        x = -9;
        goto xputx;
      }
#endif /* NOCSETS */
      putflags = PUT_RES;
    }
#endif /* FTP_RESTART */
  }
  debug(F101, "ftp PUT restart", "", putflags & PUT_RES);
  debug(F101, "ftp PUT update", "", putflags & PUT_UPD);

#ifdef PIPESEND
  if (pv[SND_FLT].ival > 0) { /* Have SEND FILTER? */
    if (!pv[SND_FLT].sval) {
      sndfilter = NULL;
    } else {
      sndfilter = (char *)malloc((int)strlen(pv[SND_FLT].sval) + 1);
      if (sndfilter)
        strcpy(sndfilter, pv[SND_FLT].sval); /* safe */
    }
    debug(F110, "ftp put /FILTER", sndfilter, 0);
  }
  if (sndfilter || pipesend) /* No /UPDATE or /RESTART */
    if (putflags)            /* with pipes or filters */
      putflags = 0;
#endif /* PIPESEND */

  tfc = (CK_OFF_T)0; /* Initialize stats and counters */
  filcnt = 0;
  pktnum = 0;
  spackets = 0L;

  if (wild) /* (is this necessary?) */
    cx = FTP_MPU;

  t0 = gmstimer(); /* Record starting time */

  done = 0; /* Loop control */
  cancelgroup = 0;

  cdlevel = 0;
  cdsimlvl = 0;
  while (!done && !cancelgroup) { /* Loop for all files */
                                  /* or until canceled. */
#ifdef FTP_PROXY
    /*
       If we are using a proxy, we don't use the local file list;
       instead we use the list on the remote machine which we want
       sent to someone else, and we use remglob() to get the names.
       But in that case we shouldn't even be executing this routine;
       see ftp_mput().
    */
#endif /* FTP_PROXY */

    cancelfile = 0;
    x = gnfile(); /* Get next file from list(s) */

    if (x == 0) /* (see gnfile() comments...) */
      x = gnferror;
    debug(F111, "FTP PUT gnfile", filnam, x);
    debug(F111, "FTP PUT binary", filnam, binary);

    switch (x) {
    case 1: /* File to send */
      s2 = asnambuf;
#ifndef NOSPL
      if (asnambuf[0]) { /* As-name */
        int n;
        char *p; /* to be evaluated... */
        n = TMPBUFSIZ;
        p = tmpbuf;
        zzstring(asnambuf, &p, &n);
        s2 = tmpbuf;
        debug(F110, "ftp put asname", s2, 0);
      }
#endif                                /* NOSPL */
      rc = putfile(cx,                /* Function (PUT, APPEND) */
                   filnam, s2,        /* Name to send, as-name */
                   forcetype, moving, /* Parameters from switches... */
                   snd_move, snd_rename, srv_renam, x_cnv, x_usn, xfiletype,
                   x_prm,
#ifndef NOCSETS
                   x_csl, (!x_xla ? -1 : x_csr),
#else
                   -1, -1,
#endif /* NOCSETS */
                   putflags);
      debug(F111, "ftp put putfile rc", filnam, rc);
      debug(F111, "ftp put putfile cancelfile", filnam, cancelfile);
      debug(F111, "ftp put putfile cancelgroup", filnam, cancelgroup);
      if (rc > -1) {
        good++;
        status = 1;
      }
      if (cancelfile)
        continue;
      if (rc < 0) {
        ftp_fai++;
        if (puterror) {
          status = 0;
          printf("?Fatal upload error: %s\n", filnam);
          done++;
        }
      }
      continue;
    case 0: /* No more files, done */
      done++;
      continue;
    case -1:
      printf("?%s: file not found - \"%s\"\n", puterror ? "Fatal" : "Warning",
             filnam);
      if (puterror) {
        status = 0;
        done++;
        break;
      }
      continue;
    case -2:
      if (puterror) {
        printf("?Fatal: file not found - \"%s\"\n", filnam);
        status = 0;
        done++;
        break;
      }
      continue; /* Not readable, keep going */
    case -3:
      if (puterror) {
        printf("?Fatal: Read access denied - \"%s\"\n", filnam);
        status = 0;
        done++;
        break;
      }
      printf("?Warning access denied - \"%s\"\n", filnam);
      continue;
    case -5:
      printf("?Too many files match\n");
      done++;
      break;
    case -6:
      if (good < 1)
        printf("?No files selected\n");
      done++;
      break;
    default:
      printf("?getnextfile() - unknown failure\n");
      done++;
    }
  }
  if (cdlevel > 0) {
    while (cdlevel--) {
      if (cdsimlvl) {
        cdsimlvl--;
      } else if (!doftpcdup())
        break;
    }
  }
  if (status > 0) {
    if (cancelgroup)
      status = 0;
    else if (cancelfile && good < 1)
      status = 0;
  }
  success = status;
  x = success;

xputx:
  if (x > -1) {
#ifdef GFTIMER
    t1 = gmstimer();                              /* End time */
    sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
    if (!sec)
      sec = 0.001;
    fptsecs = sec;
#else
    sec = (t1 - t0) / 1000;
    if (!sec)
      sec = 1;
#endif /* GFTIMER */
    tfcps = (long)(tfc / sec);
    tsecs = (int)sec;
    lastxfer = W_FTP | W_SEND;
    xferstat = success;
    if (dpyactive)
      ftscreen(status > 0 ? SCR_TC : SCR_CW, 0, (CK_OFF_T)0, "");
  }
  for (i = 0; i <= SND_MAX; i++) { /* Free malloc'd memory */
    if (pv[i].sval)
      free(pv[i].sval);
  }
  ftreset(); /* Undo switch effects */
  dpyactive = 0;
  return (x);
}

static char **mgetlist = NULL; /* For MGET */
static int mgetn = 0, mgetx = 0;
static char xtmpbuf[4096];

/*
  c m d l i n g e t

  Get files specified by -g command-line option.
  File list is set up in cmlist[] by ckuusy.c; nfils is length of list.
*/
int cmdlinget(int stay) {
  int x, rc = 0, done = 0, good = 0, status = 0, append = 0;
  int lcs = -1, rcs = -1, xlate = 0;
  int first = 1;
  int mget = 1;
  int nc;
  char *s, *s2, *s3;
  ULONG t0, t1; /* Times for stats */
#ifdef GFTIMER
  CKFLOAT sec;
#else
  int sec = 0;
#endif /* GFTIMER */

  if (quiet) { /* -q really means quiet */
    displa = 0;
    fdispla = 0;
  } else {
    displa = 1;
    fdispla = XYFD_B;
  }
  testing = 0;
  dpyactive = 0;
  out2screen = 0;
  what = W_FTP | W_RECV;
  mgetmethod = 0;
  mgetforced = 0;

  havetype = 0;
  havesize = (CK_OFF_T)-1;
  makestr(&havemdtm, NULL);

  if (ftp_fnc < 0)
    ftp_fnc = fncact;

#ifndef NOSPL
  cmd_quoting = 0;
#endif /* NOSPL */
  debug(F101, "ftp cmdlinget nfils", "", nfils);

  if (ftp_cnv == CNV_AUTO) { /* Name conversion is auto */
    if (alike) {             /* If server & client are alike */
      nc = 0;                /* no conversion */
    } else {                 /* If they are different */
      if (servertype == SYS_UNIX || servertype == SYS_WIN32)
        nc = -1; /* only minimal conversions needed */
      else       /* otherwise */
        nc = 1;  /* full conversion */
    }
  } else /* Not auto - do what user said */
    nc = ftp_cnv;

  if (nfils < 1)
    doexit(BAD_EXIT, -1);

  t0 = gmstimer(); /* Starting time for this batch */

#ifndef NOCSETS
  if (xlate) {     /* SET FTP CHARACTER-SET-TRANSLATION */
    lcs = ftp_csl; /* Local charset */
    if (lcs < 0)
      lcs = fcharset;
    if (lcs < 0)
      xlate = 0;
  }
  if (xlate) {     /* Still ON? */
    rcs = ftp_csx; /* Remote (Server) charset */
    if (rcs < 0)
      rcs = ftp_csr;
    if (rcs < 0)
      xlate = 0;
  }
#endif /* NOCSETS */
  /*
    If we have only one file and it is a directory, then we ask for a
    listing of its contents, rather than retrieving the directory file
    itself.  This is what (e.g.) Netscape does.
  */
  if (nfils == 1) {
    if (doftpcwd((char *)cmlist[mgetx], -1)) {
      /* If we can CD to it, it must be a directory */
      if (recursive) {
        cmlist[mgetx] = "*";
      } else {
        status = (recvrequest("LIST", "-", "", "wb", 0, 0, NULL, xlate, lcs,
                              rcs) == 0);
        done = 1;
      }
    }
  }
  /*
    The following is to work around UNIX servers which, when given a command
    like "NLST path/blah" (not wild) returns the basename without the path.
  */
  if (!done && servertype == SYS_UNIX && nfils == 1) {
    mget = iswild(cmlist[mgetx]);
  }
  if (!mget && !done) { /* Invoked by command-line FTP URL */
    if (ftp_deb)
      printf("DOING GET...\n");
    done++;
    cancelfile = 0; /* This file not canceled yet */
    s = cmlist[mgetx];
    rc = 0; /* Initial return code */
    fsize = (CK_OFF_T)-1;
    if (sizeok) {
      x = ftpcmd("SIZE", s, lcs, rcs, ftp_vbm); /* Get remote file's size */
      if (x == REPLY_COMPLETE)
        fsize = ckatofs(&ftp_reply_str[4]);
    }
    ckstrncpy(filnam, s, CKMAXPATH); /* For \v(filename) */
    debug(F111, "ftp cmdlinget filnam", filnam, fsize);

    nzrtol(s, tmpbuf, nc, 0, CKMAXPATH); /* Strip path and maybe convert */
    s2 = tmpbuf;

    /* If local file already exists, take collision action */

    if (zchki(s2) > -1) {
      switch (ftp_fnc) {
      case XYFX_A: /* Append */
        append = 1;
        break;
      case XYFX_R:   /* Rename */
      case XYFX_B: { /* Backup */
        char *p = NULL;
        int x = -1;
        znewn(s2, &p); /* Make unique name */
        debug(F110, "ftp cmdlinget znewn", p, 0);
        if (ftp_fnc == XYFX_B) { /* Backup existing file */
          x = zrename(s2, p);
          debug(F111, "ftp cmdlinget backup zrename", p, x);
        } else { /* Rename incoming file */
          x = ckstrncpy(tmpbuf, p, CKMAXPATH + 1);
          s2 = tmpbuf;
          debug(F111, "ftp cmdlinget rename incoming", p, x);
        }
        if (x < 0) {
          printf("?Backup/Rename failed\n");
          return (success = 0);
        }
        break;
      }
      case XYFX_D: /* Discard */
        ftscreen(SCR_FN, 'F', (CK_OFF_T)0, s);
        ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)SKP_NAM, s);
        tlog(F100, " refused: name", "", 0);
        debug(F110, "ftp cmdlinget skip name", s2, 0);
        goto xclget;

      case XYFX_X: /* Overwrite */
      case XYFX_U: /* Update (already handled above) */
      case XYFX_M: /* ditto */
        break;
      }
    }
    rc = getfile(s,      /* Remote name */
                 s2,     /* Local name */
                 0,      /* Recover/Restart */
                 append, /* Append */
                 NULL,   /* Pipename */
                 0,      /* Translate charsets */
                 -1,     /* File charset (none) */
                 -1      /* Server charset (none) */
    );
    debug(F111, "ftp cmdlinget rc", s, rc);
    debug(F111, "ftp cmdlinget cancelfile", s, cancelfile);
    debug(F111, "ftp cmdlinget cancelgroup", s, cancelgroup);

    if (rc < 0 && haveurl && s[0] == '/') /* URL failed - try again */
      rc = getfile(&s[1],                 /* Remote name without leading '/' */
                   s2,                    /* Local name */
                   0,                     /* Recover/Restart */
                   append,                /* Append */
                   NULL,                  /* Pipename */
                   0,                     /* Translate charsets */
                   -1,                    /* File charset (none) */
                   -1                     /* Server charset (none) */
      );
    if (rc > -1) {
      good++;
      status = 1;
    }
    if (cancelfile)
      goto xclget;
    if (rc < 0) {
      ftp_fai++;
#ifdef FTP_TIMEOUT
      if (ftp_timed_out)
        status = 0;
#endif /* FTP_TIMEOUT */
      if (geterror) {
        status = 0;
        done++;
      }
    }
  }
  if (ftp_deb && !done)
    printf("DOING MGET...\n");
  while (!done && !cancelgroup) {
    cancelfile = 0; /* This file not canceled yet */
    s = (char *)remote_files(first, (CHAR *)cmlist[mgetx], NULL, 0);
    if (!s)
      s = "";
    if (!*s) {
      first = 1;
      mgetx++;
      if (mgetx < nfils)
        s = (char *)remote_files(first, (CHAR *)cmlist[mgetx], NULL, 0);
      else
        s = NULL;
      debug(F111, "ftp cmdlinget remote_files B", s, 0);
      if (!s) {
        done = 1;
        break;
      }
    }
    /*
      The semantics of NLST are ill-defined.  Suppose we have just sent
      NLST /path/[a-z]*.  Most servers send back names like /path/foo,
      /path/bar, etc.  But some send back only foo and bar, and subsequent
      RETR commands based on the pathless names are not going to work.
    */
    if (servertype == SYS_UNIX && !ckstrchr(s, '/')) {
      if ((s3 = ckstrrchr(cmlist[mgetx], '/'))) {
        int len, left = 4096;
        char *tmp = xtmpbuf;
        len = s3 - cmlist[mgetx] + 1;
        ckstrncpy(tmp, cmlist[mgetx], left);
        tmp += len;
        left -= len;
        ckstrncpy(tmp, s, left);
        s = xtmpbuf;
        debug(F111, "ftp cmdlinget remote_files X", s, 0);
      }
    }
    first = 0; /* Not first any more */

    debug(F111, "ftp cmdlinget havetype", s, havetype);
    if (havetype > 0 && havetype != FTYP_FILE) { /* Server says not file */
      debug(F110, "ftp cmdlinget not-a-file", s, 0);
      continue;
    }
    rc = 0;                        /* Initial return code */
    if (havesize > (CK_OFF_T)-1) { /* Already have file size? */
      fsize = havesize;
    } else { /* No - must ask server */
      /*
        Prior to sending the NLST command we necessarily put the
        server into ASCII mode.  We must now put it back into the
        the requested mode so the upcoming SIZE command returns
        right kind of size; this is especially important for
        GET /RECOVER; otherwise the server returns the "ASCII" size
        of the file, rather than its true size.
      */
      changetype(ftp_typ, 0); /* Change to requested type */
      fsize = (CK_OFF_T)-1;
      if (sizeok) {
        x = ftpcmd("SIZE", s, lcs, rcs, ftp_vbm);
        if (x == REPLY_COMPLETE)
          fsize = ckatofs(&ftp_reply_str[4]);
      }
    }
    ckstrncpy(filnam, s, CKMAXPATH); /* For \v(filename) */
    debug(F111, "ftp cmdlinget filnam", filnam, fsize);

    nzrtol(s, tmpbuf, nc, 0, CKMAXPATH); /* Strip path and maybe convert */
    s2 = tmpbuf;

    /* If local file already exists, take collision action */

    if (zchki(s2) > -1) {
      switch (ftp_fnc) {
      case XYFX_A: /* Append */
        append = 1;
        break;
      case XYFX_R:   /* Rename */
      case XYFX_B: { /* Backup */
        char *p = NULL;
        int x = -1;
        znewn(s2, &p); /* Make unique name */
        debug(F110, "ftp cmdlinget znewn", p, 0);
        if (ftp_fnc == XYFX_B) { /* Backup existing file */
          x = zrename(s2, p);
          debug(F111, "ftp cmdlinget backup zrename", p, x);
        } else { /* Rename incoming file */
          x = ckstrncpy(tmpbuf, p, CKMAXPATH + 1);
          s2 = tmpbuf;
          debug(F111, "ftp cmdlinget rename incoming", p, x);
        }
        if (x < 0) {
          printf("?Backup/Rename failed\n");
          return (success = 0);
        }
        break;
      }
      case XYFX_D: /* Discard */
        ftscreen(SCR_FN, 'F', (CK_OFF_T)0, s);
        ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)SKP_NAM, s);
        tlog(F100, " refused: name", "", 0);
        debug(F110, "ftp cmdlinget skip name", s2, 0);
        continue;
      case XYFX_X: /* Overwrite */
      case XYFX_U: /* Update (already handled above) */
      case XYFX_M: /* ditto */
        break;
      }
    }
    /* ^^^ ADD CHARSET STUFF HERE ^^^ */
    rc = getfile(s,      /* Remote name */
                 s2,     /* Local name */
                 0,      /* Recover/Restart */
                 append, /* Append */
                 NULL,   /* Pipename */
                 0,      /* Translate charsets */
                 -1,     /* File charset (none) */
                 -1      /* Server charset (none) */
    );
    debug(F111, "ftp cmdlinget rc", s, rc);
    debug(F111, "ftp cmdlinget cancelfile", s, cancelfile);
    debug(F111, "ftp cmdlinget cancelgroup", s, cancelgroup);

    if (rc > -1) {
      good++;
      status = 1;
    }
    if (cancelfile)
      continue;
    if (rc < 0) {
      ftp_fai++;
#ifdef FTP_TIMEOUT
      if (ftp_timed_out)
        status = 0;
#endif /* FTP_TIMEOUT */
      if (geterror) {
        status = 0;
        done++;
      }
    }
  }

xclget:
  if (cancelgroup)
    mlsreset();
  if (status > 0) {
    if (cancelgroup)
      status = 0;
    else if (cancelfile && good < 1)
      status = 0;
  }
  success = status;

#ifdef GFTIMER
  t1 = gmstimer();                              /* End time */
  sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
  if (!sec)
    sec = 0.001;
  fptsecs = sec;
#else
  sec = (t1 - t0) / 1000;
  if (!sec)
    sec = 1;
#endif /* GFTIMER */

  tfcps = (long)(tfc / sec);
  tsecs = (int)sec;
  lastxfer = W_FTP | W_RECV;
  xferstat = success;
  if (dpyactive)
    ftscreen(status > 0 ? SCR_TC : SCR_CW, 0, (CK_OFF_T)0, "");
  if (!stay)
    doexit(success ? GOOD_EXIT : BAD_EXIT, -1);
  return (success);
}

/*  d o f t p g e t  --  Parse and execute GET, MGET, MDELETE, ...  */

/*
  Note: if we wanted to implement /AFTER:, /BEFORE:, etc, we could use
  zstrdat() to convert to UTC-based time_t.  But it doesn't make sense from
  the user-interface perspective, since the server's directory listings show
  its own local times and since we don't know what timezone it's in, there's
  no way to reconcile our local times with the server's.
*/
int doftpget(int cx, int who) /* who == 1 for ftp, 0 for kermit */
{
  struct FDB fl, sw, cm;
  int i, n, rc, getval = 0, mget = 0, done = 0, pipesave = 0;
  int x_cnv = 0, x_prm = 0, restart = 0, status = 0, good = 0;
  int x_fnc = 0, first = 0, skipthis = 0, append = 0, selected = 0;
  int renaming = 0, mdel = 0, listfile = 0, updating = 0, getone = 0;
  int moving = 0, deleting = 0, toscreen = 0, haspath = 0;
  int gotsize = 0;
  CK_OFF_T getlarger = (CK_OFF_T)-1;
  CK_OFF_T getsmaller = (CK_OFF_T)-1;
  char *msg, *s, *s2, *nam, *pipename = NULL, *pn = NULL;
  char *src = "", *local = "";

  int x_csl = -1, x_csr = -1; /* Local and remote charsets */
  int x_xla = 0;
  char c;            /* Worker char */
  ULONG t0 = 0L, t1; /* Times for stats */
#ifdef GFTIMER
  CKFLOAT sec;
#else
  int sec = 0;
#endif /* GFTIMER */

  struct stringint pv[SND_MAX + 1]; /* Temporary array for switch values */

  success = 0;    /* Assume failure */
  forcetype = 0;  /* No /TEXT or /BINARY given yet */
  restart = 0;    /* No restart yet */
  out2screen = 0; /* No TO-SCREEN switch given yet */
  mgetmethod = 0; /* No NLST or MLSD switch yet */
  mgetforced = 0;

  g_displa = fdispla;
  if (ftp_dis > -1)
    fdispla = ftp_dis;

  x_cnv = ftp_cnv;         /* Filename conversion */
  if (x_cnv == CNV_AUTO) { /* Name conversion is auto */
    if (alike) {           /* If server & client are alike */
      x_cnv = 0;           /* no conversion */
    } else {               /* If they are different */
      if (servertype == SYS_UNIX || servertype == SYS_WIN32)
        x_cnv = -1; /* only minimal conversions needed */
      else          /* otherwise */
        x_cnv = 1;  /* full conversion */
    }
  } else /* Not auto - do what user said */
    x_cnv = ftp_cnv;

  x_prm = ftp_prm;       /* Permissions */
  if (x_prm == SET_AUTO) /* Permissions AUTO */
    x_prm = alike;

#ifndef NOCSETS
  x_csr = ftp_csr;    /* Inherit global server charset */
  x_csl = ftp_csl;    /* Inherit global local charset */
  if (x_csl < 0)      /* If none, use current */
    x_csl = fcharset; /* file character-set. */
  x_xla = ftp_xla;    /* Translation On/Off */
#endif                /* NOCSETS */

  geterror = ftp_err; /* Inherit global error action. */
  asnambuf[0] = NUL;  /* No as-name yet. */
  pipesave = pipesend;
  pipesend = 0;

  havetype = 0;
  havesize = (CK_OFF_T)-1;
  makestr(&havemdtm, NULL);

  if (g_ftp_typ > -1) { /* Restore TYPE if saved */
    ftp_typ = g_ftp_typ;
    /* g_ftp_typ = -1; */
  }
  for (i = 0; i <= SND_MAX; i++) { /* Initialize switch values */
    pv[i].sval = NULL;             /* to null pointers */
    pv[i].ival = -1;               /* and -1 int values */
    pv[i].wval = (CK_OFF_T)-1;     /* and -1 wide values */
  }
  zclose(ZMFILE); /* In case it was left open */

  x_fnc = ftp_fnc > -1 ? ftp_fnc : fncact; /* Filename collision action */

  if (fp_nml) { /* Reset /NAMELIST */
    if (fp_nml != stdout)
      fclose(fp_nml);
    fp_nml = NULL;
  }
  makestr(&ftp_nml, NULL);

  /* Initialize list of remote filespecs */

  if (!mgetlist) {
    mgetlist = (char **)malloc(MGETMAX * sizeof(char *));
    if (!mgetlist) {
      printf("?Memory allocation failure - MGET list\n");
      return (-9);
    }
    for (i = 0; i < MGETMAX; i++)
      mgetlist[i] = NULL;
  }
  mgetn = 0; /* Number of mget arguments */
  mgetx = 0; /* Current arg */

  if (who == 0) { /* Called with unprefixed command */
    if (cx == XXGET || cx == XXREGET || cx == XXRETR)
      getone++;
    switch (cx) {
    case XXREGET:
      pv[SND_RES].ival = 1;
      break;
    case XXRETR:
      pv[SND_DEL].ival = 1;
      break;
    case XXGET:
    case XXMGET:
      mget++;
      break;
    }
  } else { /* FTP command */
    if (cx == FTP_GET || cx == FTP_RGE)
      getone++;
    switch (cx) {
    case FTP_DEL: /* (fall thru on purpose) */
    case FTP_MDE:
      mdel++;     /* (ditto) */
    case FTP_GET: /* (ditto) */
    case FTP_MGE:
      mget++;
      break;
    case FTP_RGE:
      pv[SND_RES].ival = 1;
      break;
    }
  }
  cmfdbi(&sw,                            /* First FDB - command switches */
         _CMKEY,                         /* fcode */
         "Remote filename;\n or switch", /* hlpmsg */
         "",                             /* default */
         "",                             /* addtl string data */
         mdel ? ndelswi : ngetswi,       /* addtl numeric data 1: tbl size */
         4,                              /* addtl numeric data 2: 4 = cmswi */
         xxstring,                       /* Processing function */
         mdel ? delswi : getswi,         /* Keyword table */
         &fl                             /* Pointer to next FDB */
  );
  cmfdbi(&fl,    /* 2nd FDB - remote filename */
         _CMFLD, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         xxstring, NULL, &cm);
  cmfdbi(&cm,    /* 3rd FDB - Confirmation */
         _CMCFM, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         NULL, NULL, NULL);

  while (1) {       /* Parse 0 or more switches */
    x = cmfdb(&sw); /* Parse something */
    debug(F101, "ftp get cmfdb", "", x);
    if (x < 0)                    /* Error */
      goto xgetx;                 /* or reparse needed */
    if (cmresult.fcode != _CMKEY) /* Break out of loop if not a switch */
      break;
    c = cmgbrk();                    /* Get break character */
    getval = (c == ':' || c == '='); /* to see how they ended the switch */
    if (getval && !(cmresult.kflags & CM_ARG)) {
      printf("?This switch does not take arguments\n");
      x = -9;
      goto xgetx;
    }
    n = cmresult.nresult; /* Numeric result = switch value */
    debug(F101, "ftp get switch", "", n);

    if (!getval && (cmgkwflgs() & CM_ARG)) {
      printf("?This switch requires an argument\n");
      x = -9;
      goto xgetx;
    }
    switch (n) {  /* Process the switch */
    case SND_ASN: /* /AS-NAME: */
      debug(F101, "ftp get /as-name getval", "", getval);
      if (!getval)
        break;
      if ((x = cmfld("Name to store it under", "", &s, NULL)) < 0) {
        if (x == -3) {
          printf("?name required\n");
          x = -9;
        }
        goto xgetx;
      }
      s = brstrip(s);
      if (!*s)
        s = NULL;
      makestr(&(pv[n].sval), s);
      pv[n].ival = 1;
      break;

    case SND_BIN: /* /BINARY */
    case SND_TXT: /* /TEXT or /ASCII */
    case SND_TEN: /* /TENEX */
      pv[SND_BIN].ival = 0;
      pv[SND_TXT].ival = 0;
      pv[SND_TEN].ival = 0;
      pv[n].ival = 1;
      break;

#ifdef PUTPIPE
    case SND_CMD: /* These take no args */
      if (nopush) {
        printf("?Sorry, system command access is disabled\n");
        x = -9;
        goto xgetx;
      }
#ifdef PIPESEND
      else if (rcvfilter) {
        printf("?Sorry, no PUT /COMMAND when SEND FILTER selected\n");
        x = -9;
        goto xgetx;
      }
#endif                                  /* PIPESEND */
      sw.hlpmsg = "Command, or switch"; /* Change help message */
      pv[n].ival = 1;                   /* Just set the flag */
      pv[SND_ARR].ival = 0;
      break;
#endif /* PUTPIPE */

    case SND_SHH:     /* /QUIET */
    case SND_RES:     /* /RECOVER (reget) */
    case SND_NOB:     /* /NOBACKUPFILES */
    case SND_DEL:     /* /DELETE */
    case SND_UPD:     /* /UPDATE */
    case SND_USN:     /* /UNIQUE */
    case SND_NOD:     /* /NODOTFILES */
    case SND_REC:     /* /RECOVER */
    case SND_MAI:     /* /TO-SCREEN */
      pv[n].ival = 1; /* Just set the flag */
      break;

    case SND_DIF:                /* /DATES-DIFFER */
      pv[SND_COL].ival = XYFX_M; /* Now it's a collision option */
      pv[n].ival = 1;
      break;

    case SND_COL: /* /COLLISION: */
      if ((x = cmkey(ftpcolxtab, nftpcolx, "", "", xxstring)) < 0)
        goto xgetx;
      if (x == XYFX_M)
        pv[SND_DIF].ival = 1; /* (phase this out) */
      pv[n].ival = x;         /* this should be sufficient */
      break;

    case SND_ERR: /* /ERROR-ACTION */
      if ((x = cmkey(qorp, 2, "", "", xxstring)) < 0)
        goto xgetx;
      pv[n].ival = x;
      break;

    case SND_EXC: /* Exception list */
      if (!getval)
        break;
      if ((x = cmfld("Pattern", "", &s, xxstring)) < 0) {
        if (x == -3) {
          printf("?Pattern required\n");
          x = -9;
        }
        goto xgetx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s);
      if (pv[n].sval)
        pv[n].ival = 1;
      break;

#ifdef PIPESEND
    case SND_FLT:
      debug(F101, "ftp get /filter getval", "", getval);
      if (!getval)
        break;
      if ((x = cmfld("Filter program to send through", "", &s, NULL)) < 0) {
        if (x == -3)
          s = "";
        else
          goto xgetx;
      }
      s = brstrip(s);
      if (pv[SND_MAI].ival < 1) {
        y = strlen(s);
        /* Make sure they included "\v(...)" */
        for (x = 0; x < y; x++) {
          if (s[x] != '\\')
            continue;
          if (s[x + 1] == 'v')
            break;
        }
        if (x == y) {
          printf("?Filter must contain a replacement variable for filename.\n");
          x = -9;
          goto xgetx;
        }
      }
      if (*s) {
        pv[n].ival = 1;
        makestr(&(pv[n].sval), s);
      } else {
        pv[n].ival = 0;
        makestr(&(pv[n].sval), NULL);
      }
      break;
#endif /* PIPESEND */

    case SND_NAM:
      if (!getval)
        break;
      if ((x = cmkey(fntab, nfntab, "", "automatic", xxstring)) < 0)
        goto xgetx;
      debug(F101, "ftp get /filenames", "", x);
      pv[n].ival = x;
      break;

    case SND_SMA: /* Smaller / larger than */
    case SND_LAR: {
      CK_OFF_T y;
      if (!getval)
        break;
      if ((x = cmnumw("Size in bytes", "0", 10, &y, xxstring)) < 0)
        goto xgetx;
      pv[n].wval = y;
      break;
    }
    case SND_FIL: /* Name of file containing filnames */
      if (!getval)
        break;
      if ((x = cmifi("Name of file containing list of filenames", "", &s, &y,
                     xxstring)) < 0) {
        if (x == -3) {
          printf("?Filename required\n");
          x = -9;
        }
        goto xgetx;
      } else if (y && iswild(s)) {
        printf("?Wildcards not allowed BBB\n");
        x = -9;
        goto xgetx;
      }
      if (s)
        if (!*s)
          s = NULL;
      makestr(&(pv[n].sval), s);
      if (pv[n].sval)
        pv[n].ival = 1;
      break;

    case SND_MOV:   /* MOVE after */
    case SND_REN:   /* RENAME after */
    case SND_SRN: { /* SERVER-RENAME */
      char *m = "";
      switch (n) {
      case SND_MOV:
        m = "Device and/or directory for incoming file after reception";
        break;
      case SND_REN:
        m = "New name for incoming file after reception";
        break;
      case SND_SRN:
        m = "New name for source file on server after reception";
        break;
      }
      if (!getval)
        break;
      if ((x = cmfld(m, "", &s, n == SND_MOV ? xxstring : NULL)) < 0) {
        if (x == -3) {
          printf("%s\n",
                 n == SND_MOV ? "?Destination required" : "?New name required");
          x = -9;
        }
        goto xgetx;
      }
      makestr(&(pv[n].sval), *s ? brstrip(s) : NULL);
      pv[n].ival = (pv[n].sval) ? 1 : 0;
      break;
    }
#ifndef NOCSETS
    case SND_CSL: /* Local character set */
    case SND_CSR: /* Remote (server) charset */
      if ((x = cmkey(fcstab, nfilc, "", "", xxstring)) < 0)
        return ((x == -3) ? -2 : x);
      if (n == SND_CSL)
        x_csl = x;
      else
        x_csr = x;
      x_xla = 1; /* Overrides global OFF setting */
      break;

    case SND_XPA: /* Transparent */
      x_xla = 0;
      x_csr = -1;
      x_csl = -1;
      break;
#endif /* NOCSETS */

    case SND_NML:
      if ((x = cmofi("Local filename", "-", &s, xxstring)) < 0)
        goto xgetx;
      makestr(&ftp_nml, s);
      break;

    case SND_PAT: /* /PATTERN: */
      if (!getval)
        break;
      if ((x = cmfld("Pattern", "*", &s, xxstring)) < 0)
        goto xgetx;
      makestr(&(pv[n].sval), *s ? brstrip(s) : NULL);
      pv[n].ival = (pv[n].sval) ? 1 : 0;
      break;

    case SND_NLS:           /* /NLST */
      pv[n].ival = 1;       /* Use NLST */
      pv[SND_MLS].ival = 0; /* Don't use MLSD */
      break;

    case SND_MLS:           /* /MLSD */
      pv[n].ival = 1;       /* Use MLSD */
      pv[SND_NLS].ival = 0; /* Don't use NLST */
      break;

    default: /* /AFTER, /PERMISSIONS, etc... */
      printf("?Sorry, \"%s\" works only with [M]PUT\n", atmbuf);
      x = -9;
      goto xgetx;
    }
  }
  line[0] = NUL;
  cmarg = line;
  cmarg2 = asnambuf;
  s = line;
  /*
    For GET, we want to parse an optional as-name, like with PUT.
    For MGET, we must parse a list of names, and then send NLST or MLSD
    commands for each name separately.
  */
  switch (cmresult.fcode) { /* How did we get out of switch loop */
  case _CMFLD:              /* Field */
    if (!getone) {
      s = brstrip(cmresult.sresult);
      makestr(&(mgetlist[mgetn++]), s);
      while ((x = cmfld("Remote filename", "", &s, xxstring)) != -3) {
        if (x < 0)
          goto xgetx;
        makestr(&(mgetlist[mgetn++]), brstrip(s));
        if (mgetn >= MGETMAX) {
          printf("?Too many items in MGET list\n");
          goto xgetx;
        }
      }
      if ((x = cmcfm()) < 0)
        goto xgetx;
    } else {
      s = brstrip(cmresult.sresult);
      ckstrncpy(line, s, LINBUFSIZ);
      if ((x = cmfld("Name to store it under", "", &s, xxstring)) < 0)
        if (x != -3)
          goto xgetx;
      s = brstrip(s);
      ckstrncpy(asnambuf, s, CKMAXPATH + 1);
      if ((x = cmcfm()) < 0)
        goto xgetx;
    }
    break;
  case _CMCFM: /* Confirmation */
    break;
  default:
    printf("?Unexpected function code: %d\n", cmresult.fcode);
    x = -9;
    goto xgetx;
  }
  if (pv[SND_REC].ival > 0) /* /RECURSIVE */
    recursive = 2;

  if (pv[SND_BIN].ival > 0) {        /* /BINARY really means binary... */
    forcetype = 1;                   /* So skip the name-pattern match */
    ftp_typ = XYFT_B;                /* Set binary */
  } else if (pv[SND_TXT].ival > 0) { /* Similarly for /TEXT... */
    forcetype = 1;
    ftp_typ = XYFT_T;
  } else if (pv[SND_TEN].ival > 0) { /* and /TENEX*/
    forcetype = 1;
    ftp_typ = FTT_TEN;
  } else if (ftp_cmdlin && ftp_xfermode == XMODE_M) {
    forcetype = 1;
    ftp_typ = binary;
    g_ftp_typ = binary;
  }
  if (pv[SND_ASN].ival > 0 && pv[SND_ASN].sval && !asnambuf[0]) {
    char *p;
    p = brstrip(pv[SND_ASN].sval); /* As-name */
    ckstrncpy(asnambuf, p, CKMAXPATH + 1);
  }
  debug(F110, "ftp get asnambuf", asnambuf, 0);

#ifdef PIPESEND
  if (pv[SND_CMD].ival > 0) { /* /COMMAND - strip any braces */
    char *p;
    p = asnambuf;
    debug(F110, "GET /COMMAND before stripping", p, 0);
    p = brstrip(p);
    debug(F110, "GET /COMMAND after stripping", p, 0);
    if (!*p) {
      printf("?Sorry, a command to write to is required\n");
      x = -9;
      goto xgetx;
    }
    pipename = p;
    pipesend = 1;
  }
#endif /* PIPESEND */

  /* Set up /MOVE and /RENAME */

#ifdef CK_TMPDIR
  if (pv[SND_MOV].ival > 0 && pv[SND_MOV].sval) {
    int len;
    char *p = pv[SND_MOV].sval;
    len = strlen(p);
    if (!isdir(p)) { /* Check directory */
#ifdef CK_MKDIR
      char *s = NULL;
      s = (char *)malloc(len + 4);
      if (s) {
        strcpy(s, p); /* safe */
        if (s[len - 1] != '/') {
          s[len++] = '/';
          s[len] = NUL;
        }
        s[len++] = 'X';
        s[len] = NUL;
#ifdef NOMKDIR
        x = -1;
#else
        x = zmkdir(s);
#endif /* NOMKDIR */
        free(s);
        if (x < 0) {
          printf("?Can't create \"%s\"\n", p);
          x = -9;
          goto xgetx;
        }
      }
#else
      printf("?Directory \"%s\" not found\n", p);
      x = -9;
      goto xgetx;
#endif /* CK_MKDIR */
    }
    makestr(&rcv_move, p);
    moving = 1;
  }
#endif /* CK_TMPDIR */

  if (pv[SND_REN].ival > 0) { /* /RENAME */
    char *p = pv[SND_REN].sval;
    if (!p)
      p = "";
    if (!*p) {
      printf("?New name required for /RENAME\n");
      x = -9;
      goto xgetx;
    }
    p = brstrip(p);
#ifndef NOSPL
    /* If name given is wild, rename string must contain variables */
    if (mget && !getone) {
      char *s = tmpbuf;
      x = TMPBUFSIZ;
      zzstring(p, &s, &x);
      if (!strcmp(tmpbuf, p)) {
        printf("?/RENAME for file group must contain variables such as "
               "\\v(filename)\n");
        x = -9;
        goto xgetx;
      }
    }
#endif /* NOSPL */
    renaming = 1;
    makestr(&rcv_rename, p);
    debug(F110, "FTP rcv_rename", rcv_rename, 0);
  }
  if (!cmarg[0] && mgetn == 0 && getone && pv[SND_FIL].ival < 1) {
    printf("?Filename required but not given\n");
    x = -9;
    goto xgetx;
  } else if ((cmarg[0] || mgetn > 0) && pv[SND_FIL].ival > 0) {
    printf("?You can't give both /LISTFILE and a remote filename\n");
    x = -9;
    goto xgetx;
  }
  CHECKCONN(); /* Check connection */

  if (pv[SND_COL].ival > -1)
    x_fnc = pv[SND_COL].ival;

#ifndef NOSPL
  /* If as-name given for MGET, as-name must contain variables */
  if (mget && !getone && asnambuf[0] && x_fnc != XYFX_A) {
    char *s = tmpbuf;
    x = TMPBUFSIZ;
    zzstring(asnambuf, &s, &x);
    if (!strcmp(tmpbuf, asnambuf)) {
      printf(
          "?As-name for MGET must contain variables such as \\v(filename)\n");
      x = -9;
      goto xgetx;
    }
  }
#endif /* NOSPL */

  /* doget: */

  if (pv[SND_SHH].ival > 0 || ftp_nml) { /* GET /QUIET... */
    fdispla = 0;
  } else {
    displa = 1;
    if (mdel || ftp_deb)
      fdispla = XYFD_B;
  }
  deleting = 0;
  if (pv[SND_DEL].ival > 0) /* /DELETE was specified */
    deleting = 1;
  if (pv[SND_EXC].ival > 0)
    makelist(pv[SND_EXC].sval, rcvexcept, NSNDEXCEPT);
  if (pv[SND_SMA].wval > -1)
    getsmaller = pv[SND_SMA].wval;
  if (pv[SND_LAR].wval > -1)
    getlarger = pv[SND_LAR].wval;
  if (pv[SND_NAM].ival > -1)
    x_cnv = pv[SND_NAM].ival;
  if (pv[SND_ERR].ival > -1)
    geterror = pv[SND_ERR].ival;
  if (pv[SND_MAI].ival > -1)
    toscreen = 1;

  if (pv[SND_NLS].ival > 0) { /* Force NLST or MLSD? */
    mgetmethod = SND_NLS;
    mgetforced = 1;
  } else if (pv[SND_MLS].ival > 0) {
    mgetmethod = SND_MLS;
    mgetforced = 1;
  }

#ifdef FTP_RESTART
  if (pv[SND_RES].ival > 0) {
    if (!ftp_typ) {
      printf("?Sorry, GET /RECOVER requires binary mode\n");
      x = -9;
      goto xgetx;
    }
    restart = 1;
  }
#endif /* FTP_RESTART */

#ifdef PIPESEND
  if (pv[SND_FLT].ival > 0) { /* Have SEND FILTER? */
    if (pipesend) {
      printf("?Switch conflict: /FILTER and /COMMAND\n");
      x = -9;
      goto xgetx;
    }
    makestr(&rcvfilter, pv[SND_FLT].sval);
    debug(F110, "ftp get /FILTER", rcvfilter, 0);
  }
  if (rcvfilter || pipesend) { /* /RESTART */
#ifdef FTP_RESTART
    if (restart) { /* with pipes or filters */
      printf("?Switch conflict: /FILTER or /COMMAND and /RECOVER\n");
      x = -9;
      goto xgetx;
    }
#endif /* FTP_RESTART */
    if (pv[SND_UPD].ival > 0 || x_fnc == XYFX_M || x_fnc == XYFX_U) {
      printf("?Switch conflict: /FILTER or /COMMAND and Date Checking\n");
      x = -9;
      goto xgetx;
    }
  }
#endif /* PIPESEND */

  tfc = (CK_OFF_T)0; /* Initialize stats and counters */
  filcnt = 0;
  pktnum = 0;
  rpackets = 0L;

  if (pv[SND_FIL].ival > 0) {
    if (zopeni(ZMFILE, pv[SND_FIL].sval) < 1) {
      debug(F111, "ftp get can't open listfile", pv[SND_FIL].sval, errno);
      printf("?Failure to open listfile - \"%s\"\n", pv[SND_FIL].sval);
      x = -9;
      goto xgetx;
    }
    if (zsinl(ZMFILE, tmpbuf, CKMAXPATH) < 0) { /* Read a line */
      zclose(ZMFILE);                           /* Failed */
      debug(F110, "ftp get listfile EOF", pv[SND_FIL].sval, 0);
      printf("?Empty listfile - \"%s\"\n", pv[SND_FIL].sval);
      x = -9;
      goto xgetx;
    }
    listfile = 1;
    debug(F110, "ftp get listfile first", tmpbuf, 0);
    makestr(&(mgetlist[0]), tmpbuf);
  }
  t0 = gmstimer(); /* Record starting time */

  updating = 0; /* Checking dates? */
  if (pv[SND_UPD].ival > 0 || (!mdel && x_fnc == XYFX_U))
    updating = 1;
  if (pv[SND_DIF].ival > 0 || x_fnc == XYFX_M)
    updating = 2;
  if (updating) /* These switches force FTP DATES ON */
    ftp_dates |= 2;

  what = mdel ? W_FTP | W_FT_DELE : W_RECV | W_FTP; /* What we're doing */

  cancelgroup = 0; /* Group not canceled yet */
  if (!(ftp_xfermode == XMODE_A && patterns && get_auto && !forcetype))
    changetype(ftp_typ, 0); /* Change to requested type */
  binary = ftp_typ;         /* For file-transfer display */
  first = 1;                /* For MGET list */
  done = 0;                 /* Loop control */

#ifdef CK_TMPDIR
  if (dldir && !f_tmpdir) { /* If they have a download directory */
    if ((s = zgtdir())) {   /* Get current directory */
      if (zchdir(dldir)) {  /* Change to download directory */
        ckstrncpy(savdir, s, TMPDIRLEN);
        f_tmpdir = 1; /* Remember that we did this */
      }
    }
  }
#endif /* CK_TMPDIR */

  if (ftp_nml) { /* /NAMELIST */
    debug(F110, "ftp GET ftp_nml", ftp_nml, 0);
    if (ftp_nml[0] == '-' && ftp_nml[1] == 0)
      fp_nml = stdout;
    else
      fp_nml = fopen(ftp_nml, "wb");
    if (!fp_nml) {
      printf("?%s: %s\n", ftp_nml, ck_errstr());
      goto xgetx;
    }
  }
  while (!done && !cancelgroup) { /* Loop for all files */
                                  /* or until canceled. */
#ifdef FTP_PROXY
    /* do something here if proxy */
#endif /* FTP_PROXY */

    rs_len = (CK_OFF_T)0; /* REGET position */
    cancelfile = 0;       /* This file not canceled yet */
    haspath = 0;          /* Recalculate this each time thru */

    if (getone) { /* GET */
      s = line;
      src = line; /* Server name */
      done = 1;
      debug(F111, "ftp get file", s, 0);
    } else if (mget) { /* MGET */
      src = mgetlist[mgetx];
      debug(F111, "ftp mget remote_files A", src, first);
      s = (char *)remote_files(first, (CHAR *)mgetlist[mgetx],
                               (CHAR *)pv[SND_PAT].sval, 0);
      debug(F110, "ftp mget remote_files B", s, 0);
      if (!s)
        s = "";
      if (!*s) {
        first = 1;
        if (listfile) { /* Names from listfile */
        again:
          tmpbuf[0] = NUL;
          while (!tmpbuf[0]) {
            if (zsinl(ZMFILE, tmpbuf, CKMAXPATH) < 0) {
              zclose(ZMFILE);
              debug(F110, "ftp get listfile EOF", pv[SND_FIL].sval, 0);
              makestr(&(mgetlist[0]), NULL);
              s = NULL;
              done = 1;
              break;
            }
          }
          if (done)
            continue;

          makestr(&(mgetlist[0]), tmpbuf);
          debug(F110, "ftp get listfile next", tmpbuf, 0);
          s = (char *)remote_files(first, (CHAR *)mgetlist[0],
                                   (CHAR *)pv[SND_PAT].sval, 0);
          debug(F110, "ftp mget remote_files C", s, 0);
          if (!s) {
            ftscreen(SCR_FN, 'F', (CK_OFF_T)0, s);
            ftscreen(SCR_ST, ST_MSG, (CK_OFF_T)0, "File not found");
            tlog(F110, "ftp get file not found:", s, 0);
            goto again;
          }
        } else { /* Names from command line */
          mgetx++;
          if (mgetx < mgetn)
            s = (char *)remote_files(first, (CHAR *)mgetlist[mgetx],
                                     (CHAR *)pv[SND_PAT].sval, 0);
          else
            s = NULL;
          if (!s)
            mgetx++;
          debug(F111, "ftp mget remote_files D", s, mgetx);
        }
        if (!s) {
          if (!first || mgetx >= mgetn) {
            done = 1;
            break;
          } else if (geterror) {
            status = 0;
            done = 1;
            break;
          } else {
            continue;
          }
        }
      }
    }
    debug(F111, "ftp mget remote_files E", s, 0);
    /*
      The semantics of NLST are ill-defined.  Suppose we have just sent
      NLST /path/[a-z]*.  Most servers send back names like /path/foo,
      /path/bar, etc.  But some send back only foo and bar, and subsequent
      RETR commands based on the pathless names are not going to work.
    */
    if (servertype == SYS_UNIX && !ckstrchr(s, '/')) {
      char *s3;
      if ((s3 = ckstrrchr(mgetlist[mgetx], '/'))) {
        int len, left = 4096;
        char *tmp = xtmpbuf;
        len = s3 - mgetlist[mgetx] + 1;
        ckstrncpy(tmp, mgetlist[mgetx], left);
        tmp += len;
        left -= len;
        ckstrncpy(tmp, s, left);
        s = xtmpbuf;
        debug(F111, "ftp mget remote_files F", s, 0);
      }
    }
    first = 0;
    skipthis = 0; /* File selection... */
    msg = "";
    nam = s; /* Filename (without path) */
    rc = 0;  /* Initial return code */
    s2 = "";

    if (!getone && !skipthis) { /* For MGET and MDELETE... */
      char c, *p = s;
      int srvpath = 0;
      int usrpath = 0;
      int i, k = 0;

      debug(F111, "ftp mget havetype", s, havetype);
      if (havetype > 0 && havetype != FTYP_FILE) {
        /* Server says it's not file... */
        debug(F110, "ftp mget not-a-file", s, 0);
        continue;
      }
      /*
        Explanation: Some ftp servers (such as wu-ftpd) return a recursive list.
        But if the client did not ask for a recursive list, we have to ignore
        any server files that include a pathname that extends beyond any path
        that was included in the user's request.

        User's filespec is blah or path/blah (or other non-UNIX syntax).  We
        need to get the user's path segment.  Then, for each incoming file, if
        it begins with the same path segment, we must strip it (point past it).
      */
      src = mgetlist[mgetx]; /* In case it moved! */
      if (src) {
        for (i = 0; src[i]; i++) { /* Find rightmost path separator */
          if (ispathsep(src[i]))   /* in user's pathname */
            k = i + 1;
        }
      } else {
        src = "";
      }
      usrpath = k; /* User path segment length */
      debug(F111, "ftp get usrpath", src, usrpath);

      p = s;               /* Server filename */
      while ((c = *p++)) { /* Look for path in server filename */
        if (ispathsep(c)) {
          /* haspath++; */
          nam = p;         /* Pathless name (for ckmatch) */
          srvpath = p - s; /* Server path segment length */
        }
      }
      debug(F111, "ftp get srvpath", s, srvpath);

      if (usrpath == 0) {
        /*
          Here we handle the case where the user said "mget foo" where foo is a
          directory name, and the server is sending back names like "foo/file1",
          "foo/file2", etc.  This is a nasty trick but it's necessary because
          the user can't compensate by typing "mget foo/" because then the
          server is likely to send back "foo//file1, foo//file2" etc, and we
          still won't get a match...
        */
        int srclen = 0, srvlen = 0;
        if (src)
          srclen = strlen(src);
        if (s)
          srvlen = strlen(s);
        if (src && (srvlen > srclen)) {
          if (!strncmp(src, s, srclen) && ispathsep(s[srclen])) {
            char *tmpsrc = NULL;
            tmpsrc = (char *)malloc(srclen + 2);
            strncpy(tmpsrc, src, srclen);
            tmpsrc[srclen] = s[srclen];
            tmpsrc[srclen + 1] = NUL;
            free(mgetlist[mgetx]);
            mgetlist[mgetx] = tmpsrc;
            tmpsrc = NULL;
            src = mgetlist[mgetx];
            usrpath = srclen + 1;
          }
        }
      }
      /*
        If as-name not given and server filename includes path that matches
        the pathname from the user's file specification, we must trim the common
        path prefix from the server's name when constructing the local name.
      */
      if (src &&                        /* Wed Sep 25 17:27:48 2002 */
          !asnambuf[0] && !recursive && /* Thu Sep 19 16:11:59 2002 */
          (srvpath > 0) && !strncmp(src, s, usrpath)) {
        s2 = s + usrpath; /* Local name skips past remote path */
      }
      { /* Count path segments instead */
        int x1 = 0, x2 = 0;
        char *p;
        for (p = s; *p; p++)
          if (ispathsep(*p))
            x1++;
        for (p = src; *p; p++) {
          if (ispathsep(*p))
            x2++;
        }
        haspath = recursive ? x1 || x2 : x1 > x2;
        debug(F111, "ftp get server path segments", s, x1);
        debug(F111, "ftp get user   path segments", src, x2);
      }

      debug(F111, "ftp get haspath", s + usrpath, haspath);

      if (haspath) {      /* Server file has path segments? */
        if (!recursive) { /* [M]GET /RECURSIVE? */
                          /*
                            We did not ask for a recursive listing, but the server is sending us
                            one                 anyway (as wu-ftpd is wont to do).  We get here
                            if the current                 filename                 includes a path segment
                            beyond any path segment we asked                 for in our                 non-recursive [M]GET
                            command.  We MUST skip this file.
                          */
          debug(F111, "ftp get skipping because of path", s, 0);
          continue;
        }
      }
    } else if (getone && !skipthis) { /* GET (not MGET) */
      char *p = nam;
      while ((c = *p++)) { /* Handle path in local name */
        if (ispathsep(c)) {
          if (recursive) { /* If recursive, keep it */
            haspath = 1;
            break;
          } else { /* Otherwise lose it. */
            nam = p;
          }
        }
      }
      s2 = nam;
    }
    if (!*nam) /* Name without path */
      nam = s;

    if (!skipthis && pv[SND_NOD].ival > 0) { /* /NODOTFILES */
      if (nam[0] == '.')
        continue;
    }
    if (!skipthis && rcvexcept[0]) { /* /EXCEPT: list */
      int xx;
      for (i = 0; i < NSNDEXCEPT; i++) {
        if (!rcvexcept[i]) {
          break;
        }
        xx = ckmatch(rcvexcept[i], nam, servertype == SYS_UNIX, 1);
        debug(F111, "ftp mget /except match", rcvexcept[i], xx);
        if (xx) {
          tlog(F100, " refused: exception list", "", 0);
          msg = "Refused: Exception List";
          skipthis++;
          break;
        }
      }
    }
    if (!skipthis && pv[SND_NOB].ival > 0) { /* /NOBACKUPFILES */
      if (ckmatch(
#ifdef CKREGEX
              "*.~[0-9]*~"
#else
              "*.~*~"
#endif /* CKREGEX */
              ,
              nam, 0, 1) > 0)
        continue;
    }
    if (!x_xla) { /* If translation is off */
      x_csl = -2; /* unset the charsets */
      x_csr = -2;
    }
    ckstrncpy(filnam, s, CKMAXPATH); /* For \v(filename) */
    if (!*s2)                        /* Local name */
      s2 = asnambuf;                 /* As-name */

    if (!*s2)                   /* Sat Nov 16 19:19:39 2002 */
      s2 = recursive ? s : nam; /* Fri Jan 10 13:15:19 2003 */

    debug(F110, "ftp get filnam  ", s, 0);
    debug(F110, "ftp get asname A", s2, 0);

    /* Receiving to real file */
    if (!pipesend &&
#ifdef PIPESEND
        !rcvfilter &&
#endif /* PIPESEND */
        !toscreen) {
#ifndef NOSPL
      /* Do this here so we can decide whether to skip */
      if (cmd_quoting && !skipthis && asnambuf[0]) {
        int n;
        char *p;
        n = TMPBUFSIZ;
        p = tmpbuf;
        zzstring(asnambuf, &p, &n);
        s2 = tmpbuf;
        debug(F111, "ftp get asname B", s2, updating);
      }
#endif /* NOSPL */

      local = *s2 ? s2 : s;

      if (!skipthis && x_fnc == XYFX_D) { /* File Collision = Discard */
        CK_OFF_T x;
        x = zchki(local);
        debug(F111, "ftp get DISCARD zchki", local, x);
        if (x > -1) {
          skipthis++;
          debug(F110, "ftp get skip name", local, 0);
          tlog(F100, " refused: name", "", 0);
          msg = "Refused: Name";
        }
      }

#ifdef DOUPDATE
      if (!skipthis && updating) { /* If updating and not yet skipping */
        if (zchki(local) > -1) {
          x = chkmodtime(local, s, 0);
#ifdef DEBUG
          if (deblog) {
            if (updating == 2)
              debug(F111, "ftp get /dates-diff chkmodtime", local, x);
            else
              debug(F111, "ftp get /update chkmodtime", local, x);
          }
#endif                                     /* DEBUG */
          if ((updating == 1 && x > 0) ||  /* /UPDATE */
              (updating == 2 && x == 1)) { /* /DATES-DIFFER */
            skipthis++;
            tlog(F100, " refused: date", "", 0);
            msg = "Refused: Date";
            debug(F110, "ftp get skip date", local, 0);
          }
        }
      }
#endif /* DOUPDATE */
    }
    /* Initialize file size to -1 in case server doesn't understand */
    /* SIZE command, so xxscreen() will know we don't know the size */

    fsize = (CK_OFF_T)-1;

    /* Ask for size now only if we need it for selection */
    /* because if you're going thru a list 100,000 files to select */
    /* a small subset, 100,000 SIZE commands can take hours... */

    gotsize = 0;
    if (!mdel && !skipthis && /* Don't need size for DELE... */
        (getsmaller >= (CK_OFF_T)0 || getlarger >= (CK_OFF_T)0)) {
      if (havesize >= (CK_OFF_T)0) { /* Already have file size? */
        fsize = havesize;
        gotsize = 1;
      } else { /* No - must ask server */
        /*
          Prior to sending the NLST command we necessarily put the
          server into ASCII mode.  We must now put it back into the
          the requested mode so the upcoming SIZE command returns
          right kind of size; this is especially important for
          GET /RECOVER; otherwise the server returns the "ASCII" size
          of the file, rather than its true size.
        */
        changetype(ftp_typ, 0); /* Change to requested type */
        fsize = (CK_OFF_T)-1;
        if (sizeok) {
          x = ftpcmd("SIZE", s, x_csl, x_csr, ftp_vbm);
          if (x == REPLY_COMPLETE) {
            fsize = ckatofs(&ftp_reply_str[4]);
            gotsize = 1;
          }
        }
      }
      if (gotsize) {
        if (getsmaller >= (CK_OFF_T)0 && fsize >= getsmaller)
          skipthis++;
        if (getlarger >= (CK_OFF_T)0 && fsize <= getlarger)
          skipthis++;
        if (skipthis) {
          debug(F111, "ftp get skip size", s, fsize);
          tlog(F100, " refused: size", "", 0);
          msg = "Refused: Size";
        }
      }
    }
    if (skipthis) { /* Skipping this file? */
      ftscreen(SCR_FN, 'F', (CK_OFF_T)0, s);
      if (msg)
        ftscreen(SCR_ST, ST_ERR, (CK_OFF_T)0, msg);
      else
        ftscreen(SCR_ST, ST_SKIP, (CK_OFF_T)0, s);
      continue;
    }
    if (fp_nml) { /* /NAMELIST only - no transfer */
      fprintf(fp_nml, "%s\n", s);
      continue;
    }
    if (recursive && haspath && !pipesend
#ifdef PIPESEND
        && !rcvfilter
#endif /* PIPESEND */
    ) {
      int x;

#ifdef NOMKDIR
      x = -1;
#else
      x = zmkdir(s); /* Try to make the directory */
#endif /* NOMKDIR */

      if (x < 0) {
        rc = -1; /* Failure is fatal */
        if (geterror) {
          status = 0;
          ftscreen(SCR_EM, 0, (CK_OFF_T)0, "Directory creation failure");
          break;
        }
      }
    }

    /* Not skipping */

    selected++; /* Count this file as selected */
    pn = NULL;

    if (!gotsize && !mdel) {         /* Didn't get size yet */
      if (havesize > (CK_OFF_T)-1) { /* Already have file size? */
        fsize = havesize;
        gotsize = 1;
      } else { /* No - must ask server */
        fsize = (CK_OFF_T)-1;
        if (sizeok) {
          x = ftpcmd("SIZE", s, x_csl, x_csr, ftp_vbm);
          if (x == REPLY_COMPLETE) {
            fsize = ckatofs(&ftp_reply_str[4]);
            gotsize = 1;
          }
        }
      }
    }
    if (mdel) { /* [M]DELETE */
      if (displa && !ftp_vbm)
        printf(" %s...", s);
      rc =
          (ftpcmd("DELE", s, x_csl, x_csr, ftp_vbm) == REPLY_COMPLETE) ? 1 : -1;
      if (rc > -1) {
        tlog(F110, "ftp mdelete", s, 0);
        if (displa && !ftp_vbm)
          printf("OK\n");
      } else {
        tlog(F110, "ftp mdelete failed:", s, 0);
        if (displa)
          printf("Failed\n");
      }
#ifndef NOSPL
#ifdef PIPESEND
    } else if (rcvfilter) { /* [M]GET with filter */
      int n;
      char *p;
      n = CKMAXPATH;
      p = tmpbuf; /* Safe - no asname with filter */
      zzstring(rcvfilter, &p, &n);
      if (n > -1)
        pn = tmpbuf;
      debug(F111, "ftp get rcvfilter", pn, n);
#endif /* PIPESEND */
#endif /* NOSPL */
      if (toscreen)
        s2 = "-";
    } else if (pipesend) { /* [M]GET /COMMAND */
      int n;
      char *p;
      n = CKMAXPATH;
      p = tmpbuf; /* Safe - no asname with filter */
      zzstring(pipename, &p, &n);
      if (n > -1)
        pn = tmpbuf;
      debug(F111, "ftp get pipename", pipename, n);
      if (toscreen)
        s2 = "-";
    } else { /* [M]GET with no pipes or filters */
      debug(F111, "ftp get s2 A", s2, x_cnv);
      if (toscreen) {
        s2 = "-";        /* (hokey convention for stdout) */
      } else if (!*s2) { /* No asname? */
        if (x_cnv) {     /* If converting */
          nzrtol(s, tmpbuf, x_cnv, 1, CKMAXPATH); /* convert */
          s2 = tmpbuf;
          debug(F110, "ftp get nzrtol", s2, 0);
        } else    /* otherwise */
          s2 = s; /* use incoming file's name */
      }
      debug(F110, "ftp get s2 B", s2, 0);

      /* If local file already exists, take collision action */

      if (!pipesend &&
#ifdef PIPESEND
          !rcvfilter &&
#endif /* PIPESEND */
          !toscreen) {
        CK_OFF_T x;
        x = zchki(s2);
        debug(F111, "ftp get zchki", s2, x);
        debug(F111, "ftp get x_fnc", s2, x_fnc);

        if (x > (CK_OFF_T)-1 && !restart) {
          int x = -1;
          char *newname = NULL;

          switch (x_fnc) {
          case XYFX_A: /* Append */
            append = 1;
            break;
          case XYFX_R:           /* Rename */
          case XYFX_B:           /* Backup */
            znewn(s2, &newname); /* Make unique name */
            debug(F110, "ftp get znewn", newname, 0);
            if (x_fnc == XYFX_B) { /* Backup existing file */
              x = zrename(s2, newname);
              debug(F111, "ftp get backup zrename", newname, x);
            } else { /* Rename incoming file */
              x = ckstrncpy(tmpbuf, newname, CKMAXPATH + 1);
              s2 = tmpbuf;
              debug(F111, "ftp get rename incoming", newname, x);
            }
            if (x < 0) {
              ftscreen(SCR_EM, 0, (CK_OFF_T)0, "Backup/Rename failed");
              x = 0;
              goto xgetx;
            }
            break;
          case XYFX_D: /* Discard (already handled above) */
          case XYFX_U: /* Update (ditto) */
          case XYFX_M: /* Update (ditto) */
          case XYFX_X: /* Overwrite */
            break;
          }
        }
      }
    }
    if (!mdel) {
#ifdef PIPESEND
      debug(F111, "ftp get pn", pn, rcvfilter ? 1 : 0);
#endif /* PIPESEND */
      if (pipesend && !toscreen)
        s2 = NULL;
#ifdef DEBUG
      if (deblog) {
        debug(F101, "ftp get x_xla", "", x_xla);
        debug(F101, "ftp get x_csl", "", x_csl);
        debug(F101, "ftp get x_csr", "", x_csr);
        debug(F101, "ftp get append", "", append);
      }
#endif /* DEBUG */

      rc = getfile(s, s2, restart, append, pn, x_xla, x_csl, x_csr);

#ifdef DEBUG
      if (deblog) {
        debug(F111, "ftp get rc", s, rc);
        debug(F111, "ftp get ftp_timed_out", s, ftp_timed_out);
        debug(F111, "ftp get cancelfile", s, cancelfile);
        debug(F111, "ftp get cancelgroup", s, cancelgroup);
        debug(F111, "ftp get renaming", s, renaming);
        debug(F111, "ftp get moving", s, moving);
      }
#endif /* DEBUG */
    }
    if (rc > -1) {
      good++;
      status = 1;
      if (!cancelfile) {
        if (deleting) { /* GET /DELETE (source file) */
          rc = (ftpcmd("DELE", s, x_csl, x_csr, ftp_vbm) == REPLY_COMPLETE)
                   ? 1
                   : -1;
          tlog(F110, (rc > -1) ? " deleted" : " failed to delete", s, 0);
        }
        if (renaming && rcv_rename && !toscreen) {
          char *p; /* Rename downloaded file */
#ifndef NOSPL
          char tmpbuf[CKMAXPATH + 1];
          int n;
          n = CKMAXPATH;
          p = tmpbuf;
          debug(F111, "ftp get /rename", rcv_rename, 0);
          zzstring(rcv_rename, &p, &n);
          debug(F111, "ftp get /rename", rcv_rename, 0);
          p = tmpbuf;
#else
          p = rcv_rename;
#endif /* NOSPL */
          rc = (zrename(s2, p) < 0) ? -1 : 1;
          debug(F111, "doftpget /RENAME zrename", p, rc);
          tlog(F110, (rc > -1) ? " renamed to" : " failed to rename to", p, 0);
        } else if (moving && rcv_move && !toscreen) {
          char *p; /* Move downloaded file */
#ifndef NOSPL
          char tmpbuf[CKMAXPATH + 1];
          int n;
          n = TMPBUFSIZ;
          p = tmpbuf;
          debug(F111, "ftp get /move-to", rcv_move, 0);
          zzstring(rcv_move, &p, &n);
          p = tmpbuf;
#else
          p = rcv_move;
#endif /* NOSPL */
          debug(F111, "ftp get /move-to", p, 0);
          rc = (zrename(s2, p) < 0) ? -1 : 1;
          debug(F111, "doftpget /MOVE zrename", p, rc);
          tlog(F110, (rc > -1) ? " moved to" : " failed to move to", p, 0);
        }
        if (pv[SND_SRN].ival > 0 && pv[SND_SRN].sval) {
          char *s = pv[SND_SRN].sval;
          char *srvrn = pv[SND_SRN].sval;
          char tmpbuf[CKMAXPATH + 1];
#ifndef NOSPL
          int y;                  /* Pass it thru the evaluator */
          extern int cmd_quoting; /* for \v(filename) */
          debug(F111, "ftp get srv_renam", s, 1);

          if (cmd_quoting) {
            y = CKMAXPATH;
            s = (char *)tmpbuf;
            zzstring(srvrn, &s, &y);
            s = (char *)tmpbuf;
          }
#endif /* NOSPL */
          debug(F111, "ftp get srv_renam", s, 1);
          if (s)
            if (*s) {
              int x;
              x = ftp_rename(s2, s);
              debug(F111, "ftp get ftp_rename", s2, x);
              tlog(F110,
                   (x > 0) ? " renamed source file to"
                           : " failed to rename source file to",
                   s, 0);
              if (x < 1)
                return (-1);
            }
        }
      }
    }
    if (cancelfile)
      continue;
    if (rc < 0) {
      ftp_fai++;
#ifdef FTP_TIMEOUT
      debug(F101, "ftp get ftp_timed_out", "", ftp_timed_out);
      if (ftp_timed_out) {
        status = 0;
        ftscreen(SCR_EM, 0, (CK_OFF_T)0, "GET timed out");
      }
#endif /* FTP_TIMEOUT */
      if (geterror) {
        status = 0;
        ftscreen(SCR_EM, 0, (CK_OFF_T)0, "Fatal download error");
        done++;
      }
    }
  }
#ifdef DEBUG
  if (deblog) {
    debug(F101, "ftp get status", "", status);
    debug(F101, "ftp get cancelgroup", "", cancelgroup);
    debug(F101, "ftp get cancelfile", "", cancelfile);
    debug(F101, "ftp get selected", "", selected);
    debug(F101, "ftp get good", "", good);
  }
#endif /* DEBUG */

  if (selected == 0) {               /* No files met selection criteria */
    status = 1;                      /* which is a kind of success. */
  } else if (status > 0) {           /* Some files were selected */
    if (cancelgroup)                 /* but MGET was canceled */
      status = 0;                    /* so MGET failed */
    else if (cancelfile && good < 1) /* If file was canceled */
      status = 0;                    /* MGET failed if it got no files */
  }
  success = status;
  x = success;
  debug(F101, "ftp get success", "", success);

xgetx:
  pipesend = pipesave; /* Restore global pipe selection */
  if (fp_nml) {        /* Close /NAMELIST */
    if (fp_nml != stdout)
      fclose(fp_nml);
    fp_nml = NULL;
  }
  if (success) { /* Download successful */
#ifdef GFTIMER
    t1 = gmstimer();                              /* End time */
    sec = (CKFLOAT)((CKFLOAT)(t1 - t0) / 1000.0); /* Stats */
    if (!sec)
      sec = 0.001;
    fptsecs = sec;
#else
    sec = (t1 - t0) / 1000;
    if (!sec)
      sec = 1;
#endif /* GFTIMER */
    tfcps = (long)(tfc / sec);
    tsecs = (int)sec;
    lastxfer = W_FTP | W_RECV;
    xferstat = success;
  }
  if (dpyactive)
    ftscreen(success > 0 ? SCR_TC : SCR_CW, 0, (CK_OFF_T)0, "");
#ifdef CK_TMPDIR
  if (f_tmpdir) {           /* If we changed to download dir */
    zchdir((char *)savdir); /* Go back where we came from */
    f_tmpdir = 0;
  }
#endif /* CK_TMPDIR */

  for (i = 0; i <= SND_MAX; i++) { /* Free malloc'd memory */
    if (pv[i].sval)
      free(pv[i].sval);
  }
  for (i = 0; i < mgetn; i++) /* MGET list too */
    makestr(&(mgetlist[i]), NULL);

  if (cancelgroup) /* Clear temp-file stack */
    mlsreset();

  ftreset(); /* Undo switch effects */
  dpyactive = 0;
  return (x);
}

static struct keytab ftprmt[] = {
    {"cd", XZCWD, 0},     {"cdup", XZCDU, 0},      {"cwd", XZCWD, CM_INV},
    {"delete", XZDEL, 0}, {"directory", XZDIR, 0}, {"exit", XZXIT, 0},
    {"help", XZHLP, 0},   {"login", XZLGI, 0},     {"logout", XZLGO, 0},
    {"mkdir", XZMKD, 0},  {"pwd", XZPWD, 0},       {"rename", XZREN, 0},
    {"rmdir", XZRMD, 0},  {"type", XZTYP, 0},      {"", 0, 0}};
static int nftprmt = (sizeof(ftprmt) / sizeof(struct keytab)) - 1;

int doftpsite() { /* Send a SITE command */
  int reply;
  char *s;
  int lcs = -1, rcs = -1;
  int save_vbm = ftp_vbm;

#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */
  if ((x = cmtxt("Command", "", &s, xxstring)) < 0)
    return (x);
  CHECKCONN();
  ckstrncpy(line, s, LINBUFSIZ);
  if (testing)
    printf(" ftp site \"%s\"...\n", line);
  if (!ftp_vbm)
    ftp_vbm = !ckstrcmp("HELP", line, 4, 0);
  if ((reply = ftpcmd("SITE", line, lcs, rcs, ftp_vbm)) == REPLY_PRELIM) {
    do {
      reply = getreply(0, lcs, rcs, ftp_vbm, 0);
    } while (reply == REPLY_PRELIM);
  }
  ftp_vbm = save_vbm;
  return (success = (reply == REPLY_COMPLETE));
}

int dosetftppsv() { /* Passive mode */
  x = seton(&ftp_psv);
  if (x > 0)
    passivemode = ftp_psv;
  return (x);
}

/*  d o f t p r m t  --  Parse and execute REMOTE commands  */

int doftprmt(int cx, int who) /* who == 1 for ftp, 0 for kermit */
{
  /* cx == 0 means REMOTE */
  /* cx != 0 is a XZxxx value */
  char *s;

  if (who != 0)
    return (0);

  if (cx == 0) {
    if ((x = cmkey(ftprmt, nftprmt, "", "", xxstring)) < 0)
      return (x);
    cx = x;
  }
  switch (cx) {
  case XZCDU: /* CDUP */
    if ((x = cmcfm()) < 0)
      return (x);
    return (doftpcdup());

  case XZCWD: /* RCD */
    if ((x = cmtxt("Remote directory", "", &s, xxstring)) < 0)
      return (x);
    ckstrncpy(line, s, LINBUFSIZ);
    s = brstrip(line);
    return (doftpcwd(s, 1));
  case XZPWD: /* RPWD */
    return (doftppwd());
  case XZDEL: /* RDEL */
    return (doftpget(FTP_MDE, 1));
  case XZDIR: /* RDIR */
    return (doftpdir(FTP_DIR));
  case XZHLP: /* RHELP */
    return (doftpxhlp());
  case XZMKD: /* RMKDIR */
    return (doftpmkd());
  case XZREN: /* RRENAME */
    return (doftpren());
  case XZRMD: /* RRMDIR */
    return (doftprmd());
  case XZLGO: /* LOGOUT */
    return (doftpres());
  case XZXIT: /* EXIT */
    return (ftpbye());
  }
  printf("?Not usable with FTP - \"%s\"\n", atmbuf);
  return (-9);
}

int doxftp() { /* Command parser for built-in FTP */
  int cx;
  struct FDB kw, fl;
  char *s;
  int usetls = 0;
  int lcs = -1, rcs = -1;

#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */

  if (inserver) /* FTP not allowed in IKSD. */
    return (-2);

  if (g_ftp_typ > -1) { /* Restore TYPE if saved */
    ftp_typ = g_ftp_typ;
    /* g_ftp_typ = -1; */
  }

  /* Restore global verbose mode */
  if (ftp_deb)
    ftp_vbm = 1;
  else if (quiet)
    ftp_vbm = 0;
  else
    ftp_vbm = ftp_vbx;

  ftp_dates &= 1; /* Undo any previous /UPDATE switch */

  dpyactive = 0;  /* Reset global transfer-active flag */
  printlines = 0; /* Reset printlines */

  if (fp_nml) { /* Reset /NAMELIST */
    if (fp_nml != stdout)
      fclose(fp_nml);
    fp_nml = NULL;
  }
  makestr(&ftp_nml, NULL);

  cmfdbi(&kw,                        /* First FDB - commands */
         _CMKEY,                     /* fcode */
         "Hostname; or FTP command", /* help */
         "",                         /* default */
         "",                         /* addtl string data */
         nftpcmd,                    /* addtl numeric data 1: tbl size */
         0,                          /* addtl numeric data 2: none */
         xxstring,                   /* Processing function */
         ftpcmdtab,                  /* Keyword table */
         &fl                         /* Pointer to next FDB */
  );
  cmfdbi(&fl,                   /* A host name or address */
         _CMFLD,                /* fcode */
         "Hostname or address", /* help */
         "",                    /* default */
         "",                    /* addtl string data */
         0,                     /* addtl numeric data 1 */
         0,                     /* addtl numeric data 2 */
         xxstring, NULL, NULL);
  x = cmfdb(&kw); /* Parse a hostname or a keyword */
  if (x == -3) {
    printf("?ftp what? \"help ftp\" for hints\n");
    return (-9);
  }
  if (x < 0)
    return (x);
  if (cmresult.fcode == _CMFLD) {          /* If hostname */
    return (openftp(cmresult.sresult, 0)); /* go open the connection */
  } else {
    cx = cmresult.nresult;
  }
  switch (cx) {
  case FTP_ACC: /* ACCOUNT */
    if ((x = cmtxt("Remote account", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    makestr(&ftp_acc, s);
    if (testing)
      printf(" ftp account: \"%s\"\n", ftp_acc);
    success = (ftpcmd("ACCT", ftp_acc, -1, -1, ftp_vbm) == REPLY_COMPLETE);
    return (success);

  case FTP_GUP: /* Go UP */
    if ((x = cmcfm()) < 0)
      return (x);
    CHECKCONN();
    if (testing)
      printf(" ftp cd: \"(up)\"\n");
    return (success = doftpcdup());

  case FTP_CWD: /* CD */
    if ((x = cmtxt("Remote directory", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckstrncpy(line, s, LINBUFSIZ);
    if (testing)
      printf(" ftp cd: \"%s\"\n", line);
    return (success = doftpcwd(line, 1));

  case FTP_CHM: /* CHMOD */
    if ((x = cmfld("Permissions or protection code", "", &s, xxstring)) < 0)
      return (x);
    ckstrncpy(tmpbuf, s, TMPBUFSIZ);
    if ((x = cmtxt("Remote filename", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckmakmsg(ftpcmdbuf, FTP_BUFSIZ, tmpbuf, " ", s, NULL);
    if (testing)
      printf(" ftp chmod: %s\n", ftpcmdbuf);
    success =
        (ftpcmd("SITE CHMOD", ftpcmdbuf, lcs, rcs, ftp_vbm) == REPLY_COMPLETE);
    return (success);

  case FTP_CLS: /* CLOSE FTP connection */
    if ((y = cmcfm()) < 0)
      return (y);
    CHECKCONN();
    if (testing)
      printf(" ftp closing...\n");
    ftpclose();
    return (success = 1);

  case FTP_DIR: /* DIRECTORY of remote files */
  case FTP_VDI:
    return (doftpdir(cx));

  case FTP_GET: /* GET a remote file */
  case FTP_RGE: /* REGET */
  case FTP_MGE: /* MGET */
  case FTP_MDE: /* MDELETE */
    return (doftpget(cx, 1));

  case FTP_IDL: /* IDLE */
    if ((x = cmnum("Number of seconds", "-1", 10, &z, xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    CHECKCONN();
    if (z < 0) { /* Display idle timeout */
      if (testing)
        printf(" ftp query idle timeout...\n");
      success = (ftpcmd("SITE IDLE", NULL, 0, 0, 1) == REPLY_COMPLETE);
    } else { /* Set idle timeout */
      if (testing)
        printf(" ftp idle timeout set: %d...\n", z);
      success = (ftpcmd("SITE IDLE", ckitoa(z), 0, 0, 1) == REPLY_COMPLETE);
    }
    return (success);

  case FTP_MKD: /* MKDIR */
    return (doftpmkd());

  case FTP_MOD: /* MODTIME */
    if ((x = cmtxt("Remote filename", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckstrncpy(line, s, LINBUFSIZ);
    if (testing)
      printf(" ftp modtime \"%s\"...\n", line);
    success = 0;
    if (ftpcmd("MDTM", line, lcs, rcs, ftp_vbm) == REPLY_COMPLETE) {
      success = 1;
      mdtmok = 1;
      if (!quiet) {
        int flag = 0;
        char c, *s;
        struct tm tmremote;

        bzero((char *)&tmremote, sizeof(struct tm));
        s = ftp_reply_str;
        while ((c = *s++)) {
          if (c == SP) {
            flag++;
            break;
          }
        }
        if (flag) {
          if (sscanf(s, "%04d%02d%02d%02d%02d%02d", &tmremote.tm_year,
                     &tmremote.tm_mon, &tmremote.tm_mday, &tmremote.tm_hour,
                     &tmremote.tm_min, &tmremote.tm_sec) == 6) {
            printf(" %s %04d-%02d-%02d %02d:%02d:%02d GMT\n", line,
                   tmremote.tm_year, tmremote.tm_mon, tmremote.tm_mday,
                   tmremote.tm_hour, tmremote.tm_min, tmremote.tm_sec);
          } else {
            success = 0;
          }
        }
      }
    }
    return (success);

  case FTP_OPN: /* OPEN connection */
  {             /* OPEN connection */
    char name[TTNAMLEN + 1], *p;
    extern int network;
    extern char ttname[];
#ifdef USETLSTAB
    int n;
#endif                                    /* USETLSTAB */
    if (network)                          /* If we have a current connection */
      ckstrncpy(name, ttname, LINBUFSIZ); /* get the host name */
    else
      *name = '\0';         /* as default host */
    for (p = name; *p; p++) /* Remove ":service" from end. */
      if (*p == ':') {
        *p = '\0';
        break;
      }
#ifndef USETLSTAB
    x = cmfld("IP hostname or address", name, &s, xxstring);
#else
    cmfdbi(&kw,                  /* First FDB - commands */
           _CMKEY,               /* fcode */
           "Hostname or switch", /* help */
           "",                   /* default */
           "",                   /* addtl string data */
           ntlstab,              /* addtl numeric data 1: tbl size */
           0,                    /* addtl numeric data 2: none */
           xxstring,             /* Processing function */
           tlstab,               /* Keyword table */
           &fl                   /* Pointer to next FDB */
    );
    cmfdbi(&fl,                   /* A host name or address */
           _CMFLD,                /* fcode */
           "Hostname or address", /* help */
           "",                    /* default */
           "",                    /* addtl string data */
           0,                     /* addtl numeric data 1 */
           0,                     /* addtl numeric data 2 */
           xxstring, NULL, NULL);

    for (n = 0;; n++) {
      x = cmfdb(&kw); /* Parse a hostname or a keyword */
      if (x == -3) {
        printf("?ftp open what? \"help ftp\" for hints\n");
        return (-9);
      }
      if (x < 0)
        break;
      if (cmresult.fcode == _CMFLD) { /* Hostname */
        s = cmresult.sresult;
        break;
      } else if (cmresult.nresult == OPN_TLS) {
        usetls = 1;
      }
    }
#endif /* USETLSTAB */
    if (x < 0) {
      success = 0;
      return (x);
    }
    ckstrncpy(line, s, LINBUFSIZ);
    s = line;
    return (openftp(s, usetls));
  }

  case FTP_PUT: /* PUT */
  case FTP_MPU: /* MPUT */
  case FTP_APP: /* APPEND */
  case FTP_REP: /* REPUT */
    return (doftpput(cx, 1));

  case FTP_PWD: /* PWD */
    x = doftppwd();
    if (x > -1)
      success = x;
    return (x);

  case FTP_REN: /* RENAME */
    return (doftpren());

  case FTP_RES: /* RESET */
    return (doftpres());

  case FTP_HLP: /* (remote) HELP */
    return (doftpxhlp());

  case FTP_RMD: /* RMDIR */
    return (doftprmd());

  case FTP_STA: /* STATUS */
    if ((x = cmtxt("Command", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckstrncpy(line, s, LINBUFSIZ);
    if (testing)
      printf(" ftp status \"%s\"...\n", line);
    success = (ftpcmd("STAT", line, lcs, rcs, 1) == REPLY_COMPLETE);
    return (success);

  case FTP_SIT: { /* SITE */
    return (doftpsite());
  }

  case FTP_SIZ: /* (ask for) SIZE */
    if ((x = cmtxt("Remote filename", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckstrncpy(line, s, LINBUFSIZ);
    if (testing)
      printf(" ftp size \"%s\"...\n", line);
    success = (ftpcmd("SIZE", line, lcs, rcs, 1) == REPLY_COMPLETE);
    if (success)
      sizeok = 1;
    return (success);

  case FTP_SYS: /* Ask for server's SYSTEM type */
    if ((x = cmcfm()) < 0)
      return (x);
    CHECKCONN();
    if (testing)
      printf(" ftp system...\n");
    success = (ftpcmd("SYST", NULL, 0, 0, 1) == REPLY_COMPLETE);
    return (success);

  case FTP_UMA: /* Set/query UMASK */
    if ((x = cmfld("Umask to set or nothing to query", "", &s, xxstring)) < 0)
      if (x != -3)
        return (x);
    ckstrncpy(tmpbuf, s, TMPBUFSIZ);
    if ((x = cmcfm()) < 0)
      return (x);
    CHECKCONN();
    if (testing) {
      if (tmpbuf[0])
        printf(" ftp umask \"%s\"...\n", tmpbuf);
      else
        printf(" ftp query umask...\n");
    }
    success = ftp_umask(tmpbuf);
    return (success);

  case FTP_USR:
    return (doftpusr());

  case FTP_QUO:
    if ((x = cmtxt("FTP protocol command", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    success = (ftpcmd(s, NULL, 0, 0, ftp_vbm) == REPLY_COMPLETE);
    return (success);

  case FTP_TYP: /* Type */
    if ((x = cmkey(ftptyp, nftptyp, "", "", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    CHECKCONN();
    ftp_typ = x;
    g_ftp_typ = x;
    tenex = (ftp_typ == FTT_TEN);
    changetype(ftp_typ, ftp_vbm);
    return (1);

  case FTP_CHK: /* Check if remote file(s) exist(s) */
    if ((x = cmtxt("remote filename", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    success = remote_files(1, (CHAR *)s, (CHAR *)s, 0) ? 1 : 0;
    return (success);

  case FTP_FEA: /* RFC2389 */
    if ((y = cmcfm()) < 0)
      return (y);
    CHECKCONN();
    success = (ftpcmd("FEAT", NULL, 0, 0, 1) == REPLY_COMPLETE);
    if (success) {
      if (sfttab[0] > 0) {
        ftp_aut = sfttab[SFT_AUTH];
        sizeok = sfttab[SFT_SIZE];
        mdtmok = sfttab[SFT_MDTM];
        mlstok = sfttab[SFT_MLST];
      }
    }
    return (success);

  case FTP_OPT: /* RFC2389 */
    /* Perhaps this should be a keyword list... */
    if ((x = cmfld("FTP command", "", &s, xxstring)) < 0)
      return (x);
    CHECKCONN();
    ckstrncpy(line, s, LINBUFSIZ);
    if ((x = cmtxt("Options for this command", "", &s, xxstring)) < 0)
      return (x);
    success = (ftpcmd("OPTS", line, lcs, rcs, ftp_vbm) == REPLY_COMPLETE);
    return (success);

  case FTP_ENA: /* FTP ENABLE */
  case FTP_DIS: /* FTP DISABLE */
    if ((x = cmkey(ftpenatab, nftpena, "", "", xxstring)) < 0)
      return (x);
    if ((y = cmcfm()) < 0)
      return (y);
    switch (x) {
    case ENA_AUTH: /* OK to use autoauthentication */
      ftp_aut = (cx == FTP_ENA) ? 1 : 0;
      sfttab[SFT_AUTH] = ftp_aut;
      break;
    case ENA_FEAT: /* OK to send FEAT command */
      featok = (cx == FTP_ENA) ? 1 : 0;
      break;
    case ENA_MLST: /* OK to use MLST/MLSD */
      mlstok = (cx == FTP_ENA) ? 1 : 0;
      sfttab[SFT_MLST] = mlstok;
      break;
    case ENA_MDTM: /* OK to use MDTM */
      mdtmok = (cx == FTP_ENA) ? 1 : 0;
      sfttab[SFT_MDTM] = mdtmok;
      break;
    case ENA_SIZE: /* OK to use SIZE */
      sizeok = (cx == FTP_ENA) ? 1 : 0;
      sfttab[SFT_SIZE] = sizeok;
      break;
    }
    return (success = 1);
  }
  return (-2);
}

#ifndef NOSHOW

int shoftp(int brief) {
  char *s = "?";
  int n, x;

  if (g_ftp_typ > -1) { /* Restore TYPE if saved */
    ftp_typ = g_ftp_typ;
    /* g_ftp_typ = -1; */
  }
  printf("\n");
  printf("FTP connection:                 %s\n",
         connected ? ftp_host : "(none)");
  n = 2;
  if (connected) {
    n++;
    printf("FTP server type:                %s\n",
           ftp_srvtyp[0] ? ftp_srvtyp : "(unknown)");
  }
  if (loggedin)
    printf("Logged in as:                   %s\n",
           strval(ftp_logname, "(unknown)"));
  else
    printf("Not logged in\n");
  n++;
  if (brief)
    return (0);

  printf("\nSET FTP values:\n\n");
  n += 3;

  printf(" ftp anonymous-password:        %s\n",
         ftp_apw ? ftp_apw : "(default)");
  printf(" ftp auto-login:                %s\n", showoff(ftp_log));
  printf(" ftp auto-authentication:       %s\n", showoff(ftp_aut));
  switch (ftp_typ) {
  case FTT_ASC:
    s = "text";
    break;
  case FTT_BIN:
    s = "binary";
    break;
  case FTT_TEN:
    s = "tenex";
    break;
  }
#ifdef FTP_TIMEOUT
  printf(" ftp timeout:                   %ld\n", ftp_timeout);
#endif /* FTP_TIMEOUT */
  printf(" ftp type:                      %s\n", s);
  printf(" ftp get-filetype-switching:    %s\n", showoff(get_auto));
  printf(" ftp dates:                     %s\n", showoff(ftp_dates));
  printf(" ftp error-action:              %s\n", ftp_err ? "quit" : "proceed");
  printf(" ftp filenames:                 %s\n",
         ftp_cnv == CNV_AUTO ? "auto" : (ftp_cnv ? "converted" : "literal"));
  printf(" ftp debug                      %s\n", showoff(ftp_deb));

  printf(" ftp passive-mode:              %s\n", showoff(ftp_psv));
  printf(" ftp permissions:               %s\n", showooa(ftp_prm));
  printf(" ftp verbose-mode:              %s\n", showoff(ftp_vbx));
  printf(" ftp send-port-commands:        %s\n", showoff(ftp_psv));
  printf(" ftp unique-server-names:       %s\n", showoff(ftp_usn));
  printf(" ftp collision:                 %s\n",
         fncnam[ftp_fnc > -1 ? ftp_fnc : fncact]);
  printf(" ftp server-time-offset:        %s\n", fts_sto ? fts_sto : "(none)");
  n += 15;

#ifndef NOCSETS
  printf(" ftp character-set-translation: %s\n", showoff(ftp_xla));
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  printf(" ftp server-character-set:      %s\n", fcsinfo[ftp_csr].keyword);
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  printf(" file character-set:            %s\n", fcsinfo[fcharset].keyword);
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }
#endif /* NOCSETS */

  x = ftp_dis;
  if (x < 0)
    x = fdispla;
  switch (x) {
  case XYFD_N:
    s = "none";
    break;
  case XYFD_R:
    s = "serial";
    break;
  case XYFD_C:
    s = "fullscreen";
    break;
  case XYFD_S:
    s = "crt";
    break;
  case XYFD_B:
    s = "brief";
    break;
  }
  printf(" ftp display:                   %s\n", s);
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  if (mlstok || featok || mdtmok || sizeok || ftp_aut) {
    printf(" enabled:                      ");
    if (ftp_aut)
      printf(" AUTH");
    if (featok)
      printf(" FEAT");
    if (mdtmok)
      printf(" MDTM");
    if (mlstok)
      printf(" MLST");
    if (sizeok)
      printf(" SIZE");
    printf("\n");
    if (++n > cmd_rows - 3) {
      if (!askmore())
        return 0;
      else
        n = 0;
    }
  }
  if (!mlstok || !featok || !mdtmok || !sizeok || !ftp_aut) {
    printf(" disabled:                     ");
    if (!ftp_aut)
      printf(" AUTH");
    if (!featok)
      printf(" FEAT");
    if (!mdtmok)
      printf(" MDTM");
    if (!mlstok)
      printf(" MLST");
    if (!sizeok)
      printf(" SIZE");
    printf("\n");
    if (++n > cmd_rows - 3) {
      if (!askmore())
        return 0;
      else
        n = 0;
    }
  }
  switch (ftpget) {
  case 0:
    s = "kermit";
    break;
  case 1:
    s = "ftp";
    break;
  case 2:
    s = "auto";
    break;
  default:
    s = "?";
  }
  printf(" get-put-remote:                %s\n", s);
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  printf("\n");
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  printf("Available security methods:     (none)\n");
  if (++n > cmd_rows - 3) {
    if (!askmore())
      return 0;
    else
      n = 0;
  }

  if (n <= cmd_rows - 3)
    printf("\n");
  return (0);
}
#endif /* NOSHOW */

#ifndef NOHELP
/* FTP HELP text strings */

static char *fhs_ftp[] = {
    "Syntax: FTP subcommand [ operands ]",
    "  Makes an FTP connection, or sends a command to the FTP server.",
    "  To see a list of available FTP subcommands, type \"ftp ?\".",
    "  and then use HELP FTP xxx to get help about subcommand xxx.",
    "  Also see HELP SET FTP, HELP SET GET-PUT-REMOTE, and HELP FIREWALL.",
    ""};

static char *fhs_acc[] =
    {/* ACCOUNT */
     "Syntax: FTP ACCOUNT text",
     "  Sends an account designator to an FTP server that needs one.",
     "  Most FTP servers do not use accounts; some use them for other",
     "  other purposes, such as disk-access passwords.", ""};
static char *fhs_app[] =
    {/* APPEND */
     "Syntax: FTP APPEND filname",
     "  Equivalent to [ FTP ] PUT /APPEND.  See HELP FTP PUT.", ""};
static char *fhs_cls[] =
    {/* BYE, CLOSE */
     "Syntax: [ FTP ] BYE",
     "  Logs out from the FTP server and closes the FTP connection.",
     "  Also see HELP SET GET-PUT-REMOTE.  Synonym: [ FTP ] CLOSE.", ""};
static char *fhs_cwd[] =
    {/* CD, CWD */
     "Syntax: [ FTP ] CD directory",
     "  Asks the FTP server to change to the given directory.",
     "  Also see HELP SET GET-PUT-REMOTE.  Synonyms: [ FTP ] CWD, RCD, RCWD.",
     ""};
static char *fhs_gup[] =
    {/* CDUP, UP */
     "Syntax: FTP CDUP",
     "  Asks the FTP server to change to the parent directory of its current",
     "  directory.  Also see HELP SET GET-PUT-REMOTE.  Synonym: FTP UP.", ""};
static char *fhs_chm[] =
    {/* CHMOD */
     "Syntax: FTP CHMOD filename permissions",
     "  Asks the FTP server to change the permissions, protection, or mode of",
     "  the given file.  The given permissions must be in the syntax of the",
     "  the server's file system, e.g. an octal number for UNIX.  Also see",
     "  FTP PUT /PERMISSIONS",
     ""};
static char *fhs_mde[] =
    {/* DELETE */
     "Syntax: FTP DELETE [ switches ] filespec",
     "  Asks the FTP server to delete the given file or files.",
     "  Synonym: MDELETE (Kermit makes no distinction between single and",
     "  multiple file deletion).  Optional switches:",
     " ",
     "  /ERROR-ACTION:{PROCEED,QUIT}",
     "  /EXCEPT:pattern",
     "  /FILENAMES:{AUTO,CONVERTED,LITERAL}",
     "  /LARGER-THAN:number",
     "  /NODOTFILES",
     "  /QUIET",
#ifdef RECURSIVE
     "  /RECURSIVE (depends on server)",
     "  /SUBDIRECTORIES",
#endif /* RECURSIVE */
     "  /SMALLER-THAN:number",
     ""};
static char *fhs_dir[] =
    {/* DIRECTORY */
     "Syntax: FTP DIRECTORY [ filespec ]",
     "  Asks the server to send a directory listing of the files that match",
     "  the given filespec, or if none is given, all the files in its current",
     "  directory.  The filespec, including any wildcards, must be in the",
     "  syntax of the server's file system.  Also see HELP SET GET-PUT-REMOTE.",
     "  Synonym: RDIRECTORY.",
     ""};
static char *fhs_vdi[] =
    {/* VDIRECTORY */
     "Syntax: FTP VDIRECTORY [ filespec ]",
     "  Asks the server to send a directory listing of the files that match",
     "  the given filespec, or if none is given, all the files in its current",
     "  directory.  VDIRECTORY is needed for getting verbose directory",
     "  listings from certain FTP servers, such as on TOPS-20.  Try it if",
     "  FTP DIRECTORY lists only filenames without details.",
     ""};
static char *fhs_fea[] =
    {/* FEATURES */
     "Syntax: FTP FEATURES",
     "  Asks the FTP server to list its special features.  Most FTP servers",
     "  do not recognize this command.", ""};
static char *fhs_mge[] =
    {/* MGET */
     "Syntax: [ FTP ] MGET [ options ] filespec [ filespec [ filespec ... ] ]",
     "  Download a single file or multiple files.  Asks the FTP server to send",
     "  the given file or files.  Also see FTP GET.  Optional switches:",
     " ",
     "  /AS-NAME:text",
     "    Name under which to store incoming file.",
     "    Pattern required for for multiple files.",
     "  /BINARY", /* /IMAGE */
     "    Force binary mode.  Synonym: /IMAGE.",
     "  /COLLISION:{BACKUP,RENAME,UPDATE,DISCARD,APPEND,OVERWRITE}",
     "    What to do if an incoming file has the same name as an existing "
     "file.",

#ifdef PUTPIPE
     "  /COMMAND",
     "    Specifies that the as-name is a command to which the incoming file",
     "    is to be piped as standard input.",
#endif /* PUTPIPE */

#ifdef DOUPDATE
     "  /DATES-DIFFER",
     "    Download only those files whose modification date-times differ from",
     "    those of the corresponding local files, or that do not already",
     "    exist on the local computer.",
#endif /* DOUPDATE */

     "  /DELETE",
     "    Specifies that each file is to be deleted from the server after,",
     "    and only if, it is successfully downloaded.",
     "  /ERROR-ACTION:{PROCEED,QUIT}",
     "    When downloading a group of files, what to do upon failure to",
     "    transfer a file: quit or proceed to the next one.",
     "  /EXCEPT:pattern",
     "    Exception list: don't download any files that match this pattern.",
     "    See HELP WILDCARD for pattern syntax.",
     "  /FILENAMES:{AUTOMATIC,CONVERTED,LITERAL}",
     "    Whether to convert incoming filenames to local syntax.",
#ifdef PIPESEND
#ifndef NOSPL
     "  /FILTER:command",
     "    Pass incoming files through the given command.",
#endif /* NOSPL */
#endif /* PIPESEND */
     "  /LARGER-THAN:number",
     "    Only download files that are larger than the given number of bytes.",
     "  /LISTFILE:filename",
     "    Obtain the list of files to download from the given file.",
#ifndef NOCSETS
     "  /LOCAL-CHARACTER-SET:name",
     "    When downloading in text mode and character-set conversion is",
     "    desired, this specifies the target set.",
#endif /* NOCSETS */
     "  /MATCH:pattern",
     "    Specifies a pattern to be used to select filenames locally from the",
     "    server's list.",
     "  /MLSD",
     "    Forces sending of MLSD (rather than NLST) to get the file list.",
#ifdef CK_TMPDIR
     "  /MOVE-TO:directory",
     "    Each file that is downloaded is to be moved to the given local",
     "    directory immediately after, and only if, it has been received",
     "    successfully.",
#endif /* CK_TMPDIR */
     "  /NAMELIST:filename",
     "    Instead of downloading the files, stores the list of files that",
     "    would be downloaded in the given local file, one filename per line.",
     "  /NLST",
     "    Forces sending of NLST (rather than MLSD) to get the file list.",
     "  /NOBACKUPFILES",
     "    Don't download any files whose names end with .~<number>~.",
     "  /NODOTFILES",
     "    Don't download any files whose names begin with period (.).",
     "  /QUIET",
     "    Suppress the file-transfer display.",
#ifdef FTP_RESTART
     "  /RECOVER", /* /RESTART */
     "    Resume a download that was previously interrupted from the point of",
     "    failure.  Works only in binary mode.  Not supported by all servers.",
     "    Synonym: /RESTART.",
#endif /* FTP_RESTART */
#ifdef RECURSIVE
     "  /RECURSIVE", /* /SUBDIRECTORIES */
     "    Create subdirectories automatically if the server sends files",
     "    recursively and includes pathnames (most don't).",
#endif /* RECURSIVE */
     "  /RENAME-TO:text",
     "    Each file that is downloaded is to be renamed as indicated just,",
     "    after, and only if, it has arrived successfully.",
#ifndef NOCSETS
     "  /SERVER-CHARACTER-SET:name",
     "    When downloading in text mode and character-set conversion is "
     "desired",
     "    this specifies the original file's character set on the server.",
#endif /* NOCSETS */
     "  /SERVER-RENAME:text",
     "    Each server source file is to be renamed on the server as indicated",
     "    immediately after, but only if, it has arrived successfully.",
     "  /SMALLER-THAN:number",
     "    Download only those files smaller than the given number of bytes.",
     "  /TEXT", /* /ASCII */
     "    Force text mode.  Synonym: /ASCII.",
     "  /TENEX",
     "    Force TENEX (TOPS-20) mode (see HELP SET FTP TYPE).",
#ifndef NOCSETS
     "  /TRANSPARENT",
     "    When downloading in text mode, do not convert chracter-sets.",
#endif /* NOCSETS */
     "  /TO-SCREEN",
     "    The downloaded file is to be displayed on the screen.",
#ifdef DOUPDATE
     "  /UPDATE",
     "    Equivalent to /COLLISION:UPDATE.  Download only those files that are",
     "    newer than than their local counterparts, or that do not exist on",
     "    the local computer.",
#endif /* DOUPDATE */
     ""};
static char *fhs_hlp[] =
    {/* HELP */
     "Syntax: FTP HELP [ command [ subcommand... ] ]",
     "  Asks the FTP server for help about the given command.  First use",
     "  FTP HELP by itself to get a list of commands, then use HELP FTP xxx",
     "  to get help for command \"xxx\".  Synonyms: REMOTE HELP, RHELP.", ""};
static char *fhs_idl[] =
    {/* IDLE */
     "Syntax: FTP IDLE [ number ]",
     "  If given without a number, this asks the FTP server to tell its",
     "  current idle-time limit.  If given with a number, it asks the server",
     "  to change its idle-time limit to the given number of seconds.", ""};
static char *fhs_usr[] =
    {/* USER, LOGIN */
     "Syntax: FTP USER username [ password [ account ] ]",
     "  Log in to the FTP server.  To be used when connected but not yet",
     "  logged in, e.g. when SET FTP AUTOLOGIN is OFF or autologin failed.",
     "  If you omit the password, and one is required by the server, you are",
     "  prompted for it.  If you omit the account, no account is sent.",
     "  Synonym: FTP LOGIN.",
     ""};
static char *fhs_get[] =
    {/* GET */
     "Syntax: [ FTP ] GET [ options ] filename [ as-name ]",
     "  Download a single file.  Asks the FTP server to send the given file.",
     "  The optional as-name is the name to store it under when it arrives;",
     "  if omitted, the file is stored with the name it arrived with, as",
     "  modified according to the FTP FILENAMES setting or /FILENAMES: switch",
     "  value.  Aside from the file list and as-name, syntax and options are",
     "  the same as for FTP MGET, which is used for downloading multiple "
     "files.",
     ""};
static char *fhs_mkd[] =
    {/* MKDIR */
     "Syntax: FTP MKDIR directory",
     "  Asks the FTP server to create a directory with the given name,",
     "  which must be in the syntax of the server's file system.  Synonyms:",
     "  REMOTE MKDIR, RMKDIR.", ""};
static char *fhs_mod[] =
    {/* MODTIME */
     "Syntax: FTP MODTIME filename",
     "  Asks the FTP server to send the modification time of the given file,",
     "  to be displayed on the screen.  The date-time format is all numeric:",
     "  yyyymmddhhmmssxxx... (where xxx... is 0 or more digits indicating",
     "  fractions of seconds).",
     ""};
static char *fhs_mpu[] =
    {/* MPUT */
     "Syntax: [ FTP ] MPUT [ switches ] filespec [ filespec [ filespec ... ] ]",
     "  Uploads files.  Sends the given file or files to the FTP server.",
     "  Also see FTP PUT.  Optional switches are:",
     " ",
     "  /AFTER:date-time",
     "    Uploads only those files newer than the given date-time.",
     "    HELP DATE for info about date-time formats.  Synonym: /SINCE.",
#ifdef PUTARRAY
     "  /ARRAY:array-designator",
     "    Tells Kermit to upload the contents of the given array, rather than",
     "    a file.",
#endif /* PUTARRAY */
     "  /AS-NAME:text",
     "    Name under which to send files.",
     "    Pattern required for for multiple files.",
     "  /BEFORE:date-time",
     "    Upload only those files older than the given date-time.",
     "  /BINARY",
     "    Force binary mode.  Synonym: /IMAGE.",
#ifdef PUTPIPE
     "  /COMMAND",
     "    Specifies that the filespec is a command whose standard output is",
     "    to be sent.",
#endif /* PUTPIPE */

     "  /DELETE",
     "    Specifies that each source file is to be deleted after, and only if,",
     "    it is successfully uploaded.",
     "  /DOTFILES",
     "    Include files whose names begin with period (.).",
     "  /ERROR-ACTION:{PROCEED,QUIT}",
     "    When uploading a group of files, what to do upon failure to",
     "    transfer a file: quit or proceed to the next one.",
     "  /EXCEPT:pattern",
     "    Exception list: don't upload any files that match this pattern.",
     "    See HELP WILDCARD for pattern syntax.",
     "  /FILENAMES:{AUTOMATIC,CONVERTED,LITERAL}",
     "    Whether to convert outbound filenames to common syntax.",
#ifdef PIPESEND
#ifndef NOSPL
     "  /FILTER:command",
     "    Pass outbound files through the given command.",
#endif /* NOSPL */
#endif /* PIPESEND */
#ifdef CKSYMLINK
     "  /FOLLOWINKS",
     "    Send files that are pointed to by symbolic links.",
     "  /NOFOLLOWINKS",
     "    Skip over symbolic links (default).",
#endif /* CKSYMLINK */
     "  /LARGER-THAN:number",
     "    Only upload files that are larger than the given number of bytes.",
     "  /LISTFILE:filename",
     "    Obtain the list of files to upload from the given file.",
#ifndef NOCSETS
     "  /LOCAL-CHARACTER-SET:name",
     "    When uploading in text mode and character-set conversion is",
     "    desired, this specifies the source-file character set.",
#endif /* NOCSETS */
#ifdef CK_TMPDIR
     "  /MOVE-TO:directory",
     "    Each source file that is uploaded is to be moved to the given local",
     "    directory when, and only if, the transfer is successful.",
#endif /* CK_TMPDIR */
     "  /NOBACKUPFILES",
     "    Don't upload any files whose names end with .~<number>~.",
     "  /NODOTFILES",
     "    Don't upload any files whose names begin with period (.).",
     "  /NOT-AFTER:date-time",
     "    Upload only files that are not newer than the given date-time",
     "  /NOT-BEFORE:date-time",
     "    Upload only files that are not older than the given date-time",
#ifdef UNIX
     "  /PERMISSIONS",
     "    Ask the server to set the permissions of each file it receives",
     "    according to the source file's permissions.",
#endif /* UNIX */
     "  /QUIET",
     "    Suppress the file-transfer display.",
#ifdef FTP_RESTART
     "  /RECOVER",
     "    Resume an upload that was previously interrupted from the point of",
     "    failure.  Synonym: /RESTART.",
#endif /* FTP_RESTART */
#ifdef RECURSIVE
     "  /RECURSIVE",
     "    Send files from the given directory and all the directories beneath",
     "    it.  Synonym: /SUBDIRECTORIES.",
#endif /* RECURSIVE */
     "  /RENAME-TO:text",
     "    Each source file that is uploaded is to be renamed on the local",
     "    local computer as indicated when and only if, the transfer completes",
     "    successfully.",
#ifndef NOCSETS
     "  /SERVER-CHARACTER-SET:name",
     "    When uploading in text mode and character-set conversion is desired,",
     "    this specifies the character set to which the file should be",
     "    converted for storage on the server.",
#endif /* NOCSETS */
     "  /SERVER-RENAME:text",
     "    Each file that is uploaded is to be renamed as indicated on the",
     "    server after, and only if, if arrives successfully.",
     "  /SIMULATE",
     "    Show which files would be sent without actually sending them.",
     "  /SMALLER-THAN:number",
     "    Upload only those files smaller than the given number of bytes.",
     "  /TEXT",
     "    Force text mode.  Synonym: /ASCII.",
     "  /TENEX",
     "    Force TENEX (TOPS-20) mode (see HELP SET FTP TYPE).",
#ifndef NOCSETS
     "  /TRANSPARENT",
     "    When uploading in text mode, do not convert chracter-sets.",
#endif /* NOCSETS */
     "  /TYPE:{TEXT,BINARY}",
     "    Upload only files of the given type.",
#ifdef DOUPDATE
     "  /UPDATE",
     "    If a file of the same name exists on the server, upload only if",
     "    the local file is newer.",
#endif /* DOUPDATE */
     "  /UNIQUE-SERVER-NAMES",
     "    Ask the server to compute new names for any incoming file that has",
     "    the same name as an existing file.",
     ""};
static char *fhs_opn[] =
    {/* OPEN */
     "Syntax: FTP [ OPEN ] hostname [ port ] [ switches ]",
     "  Opens a connection to the FTP server on the given host.  The default",
     "  TCP port is 21, but a different port number can be supplied if",
     "  necessary.  Optional switches are:",
     " ",
     "  /ANONYMOUS",
     "    Logs you in anonymously.",
     "  /USER:text",
     "    Supplies the given text as your username.",
     "  /PASSWORD:text",
     "    Supplies the given text as your password.  If you include a username",
     "    but omit this switch and the server requires a password, you are",
     "    prompted for it.",
     "  /ACCOUNT:text",
     "    Supplies the given text as your account, if required by the server.",
     "  /ACTIVE",
     "    Forces an active (rather than passive) connection.",
     "  /PASSIVE",
     "    Forces a passive (rather than active) connection.",
     "  /NOINIT",
     "    Inhibits sending initial REST, STRU, and MODE commands, which are",
     "    well-known standard commands, but to which some servers react badly.",
     "  /NOLOGIN",
     "    Inhibits autologin for this connection only.",
     ""};
static char *fhs_opt[] =
    {/* OPTS, OPTIONS */
     "Syntax: FTP OPTIONS",
     "  Asks the FTP server to list its current options.  Advanced, new,",
     "  not supported by most FTP servers.", ""};
static char *fhs_put[] =
    {/* PUT, SEND */
     "Syntax: [ FTP ] PUT [ switches ] filespec [ as-name ]",
     "  Like FTP MPUT, but only one filespec is allowed, and if it is followed",
     "  by an additional field, this is interpreted as the name under which",
     "  to send the file or files.  See HELP FTP MPUT.", ""};
static char *fhs_reput[] =
    {/* REPUT, RESEND */
     "Syntax: [ FTP ] REPUT [ switches ] filespec [ as-name ]",
     "  Synonym for FTP PUT /RECOVER.  Recovers an interrupted binary-mode",
     "  upload from the point of failure if the FTP server supports recovery.",
     "  Synonym: [ FTP ] RESEND.  For details see HELP FTP MPUT.", ""};
static char *fhs_pwd[] =
    {/* PWD */
     "Syntax: FTP PWD",
     "  Asks the FTP server to reveal its current working directory.",
     "  Synonyms: REMOTE PWD, RPWD.", ""};
static char *fhs_quo[] =
    {/* QUOTE */
     "Syntax: FTP QUOTE text",
     "  Sends an FTP protocol command to the FTP server.  Use this command",
     "  for sending commands that Kermit might not support.", ""};
static char *fhs_rge[] = {/* REGET */
                          "Syntax: FTP REGET",
                          "  Synonym for FTP GET /RECOVER.", ""};
static char *fhs_ren[] =
    {/* RENAME */
     "Syntax: FTP RENAME name1 name1",
     "  Asks the FTP server to change the name of the file whose name is name1",
     "  and which resides in the FTP server's file system, to name2.  Works",
     "  only for single files; wildcards are not accepted.", ""};
static char *fhs_res[] =
    {/* RESET */
     "Syntax: FTP RESET",
     "  Asks the server to log out your session, terminating your access",
     "  rights, without closing the connection.", ""};
static char *fhs_rmd[] =
    {/* RMDIR */
     "Syntax: FTP RMDIR directory",
     "  Asks the FTP server to remove the directory whose name is given.",
     "  This usually requires the directory to be empty.  Synonyms: REMOTE",
     "  RMDIR, RRMDIR.", ""};
static char *fhs_sit[] = {/* SITE */
                          "Syntax: FTP SITE text",
                          "  Sends a site-specific command to the FTP server.",
                          ""};
static char *fhs_siz[] =
    {/* SIZE */
     "Syntax: FTP SIZE filename",
     "  Asks the FTP server to send a numeric string representing the size",
     "  of the given file.", ""};
static char *fhs_sta[] =
    {/* STATUS */
     "Syntax: FTP STATUS [ filename ]",
     "  Asks the FTP server to report its status.  If a filename is given,",
     "  the FTP server should report details about the file.", ""};
static char *fhs_sys[] =
    {/* SYSTEM */
     "Syntax: FTP SYSTEM",
     "  Asks the FTP server to report its operating system type.", ""};
static char *fhs_typ[] =
    {/* TYPE */
     "Syntax: FTP TYPE { TEXT, BINARY, TENEX }",
     "  Puts the client and server in the indicated transfer mode.",
     "  ASCII is a synonym for TEXT.  TENEX is used only for uploading 8-bit",
     "  binary files to a 36-bit platforms such as TENEX or TOPS-20 and/or",
     "  downloading files from TENEX or TOPS-20 that have been uploaded in",
     "  TENEX mode.",
     ""};
static char *fhs_uma[] =
    {/* UMASK */
     "Syntax: FTP UMASK number",
     "  Asks the FTP server to set its file creation mode mask.  Applies",
     "  only (or mainly) to UNIX-based FTP servers.", ""};
static char *fhs_chk[] =
    {/* CHECK */
     "Syntax: FTP CHECK remote-filespec",
     "  Asks the FTP server if the given file or files exist.  If the",
     "  remote-filespec contains wildcards, this command fails if no server",
     "  files match, and succeeds if at least one file matches.  If the",
     "  remote-filespec does not contain wildcards, this command succeeds if",
     "  the given file exists and fails if it does not.",
     ""};
static char *fhs_ena[] =
    {/* ENABLE */
     "Syntax: FTP ENABLE { AUTH, FEAT, MDTM, MLST, SIZE }",
     "  Enables the use of the given FTP protocol command in case it has been",
     "  disabled (but this is no guarantee that the FTP server understands "
     "it).",
     "  Use SHOW FTP to see which of these commands is enabled and disabled.",
     "  Also see FTP DISABLE.",
     ""};
static char *fhs_dis[] =
    {/* DISABLE */
     "Syntax: FTP DISABLE { AUTH, FEAT, MDTM, MLST, SIZE }",
     "  Disables the use of the given FTP protocol command.",
     "  Also see FTP ENABLE.", ""};

#endif /* NOHELP */

int doftphlp() {
  int cx;
  if ((cx = cmkey(ftpcmdtab, nftpcmd, "", "", xxstring)) < 0)
    if (cx != -3)
      return (cx);
  if ((x = cmcfm()) < 0)
    return (x);

#ifdef NOHELP
  printf("Sorry, no help available\n");
#else
  switch (cx) {
  case -3:
    return (hmsga(fhs_ftp));
  case FTP_ACC: /* ACCOUNT */
    return (hmsga(fhs_acc));
  case FTP_APP: /* APPEND */
    return (hmsga(fhs_app));
  case FTP_CLS: /* BYE, CLOSE */
    return (hmsga(fhs_cls));
  case FTP_CWD: /* CD, CWD */
    return (hmsga(fhs_cwd));
  case FTP_GUP: /* CDUP, UP */
    return (hmsga(fhs_gup));
  case FTP_CHM: /* CHMOD */
    return (hmsga(fhs_chm));
  case FTP_MDE: /* DELETE, MDELETE */
    return (hmsga(fhs_mde));
  case FTP_DIR: /* DIRECTORY */
    return (hmsga(fhs_dir));
  case FTP_VDI: /* VDIRECTORY */
    return (hmsga(fhs_vdi));
  case FTP_FEA: /* FEATURES */
    return (hmsga(fhs_fea));
  case FTP_GET: /* GET */
    return (hmsga(fhs_get));
  case FTP_HLP: /* HELP */
    return (hmsga(fhs_hlp));
  case FTP_IDL: /* IDLE */
    return (hmsga(fhs_idl));
  case FTP_USR: /* USER, LOGIN */
    return (hmsga(fhs_usr));
  case FTP_MGE: /* MGET */
    return (hmsga(fhs_mge));
  case FTP_MKD: /* MKDIR */
    return (hmsga(fhs_mkd));
  case FTP_MOD: /* MODTIME */
    return (hmsga(fhs_mod));
  case FTP_MPU: /* MPUT */
    return (hmsga(fhs_mpu));
  case FTP_OPN: /* OPEN */
    return (hmsga(fhs_opn));
  case FTP_OPT: /* OPTS, OPTIONS */
    return (hmsga(fhs_opt));
  case FTP_PUT: /* PUT, SEND */
    return (hmsga(fhs_put));
  case FTP_REP: /* REPUT, RESEND */
    return (hmsga(fhs_reput));
  case FTP_PWD: /* PWD */
    return (hmsga(fhs_pwd));
  case FTP_QUO: /* QUOTE */
    return (hmsga(fhs_quo));
  case FTP_RGE: /* REGET */
    return (hmsga(fhs_rge));
  case FTP_REN: /* RENAME */
    return (hmsga(fhs_ren));
  case FTP_RES: /* RESET */
    return (hmsga(fhs_res));
  case FTP_RMD: /* RMDIR */
    return (hmsga(fhs_rmd));
  case FTP_SIT: /* SITE */
    return (hmsga(fhs_sit));
  case FTP_SIZ: /* SIZE */
    return (hmsga(fhs_siz));
  case FTP_STA: /* STATUS */
    return (hmsga(fhs_sta));
  case FTP_SYS: /* SYSTEM */
    return (hmsga(fhs_sys));
  case FTP_TYP: /* TYPE */
    return (hmsga(fhs_typ));
  case FTP_UMA: /* UMASK */
    return (hmsga(fhs_uma));
  case FTP_CHK: /* CHECK */
    return (hmsga(fhs_chk));
  case FTP_ENA:
    return (hmsga(fhs_ena));
  case FTP_DIS:
    return (hmsga(fhs_dis));
  default:
    printf("Sorry, help available for this command.\n");
    break;
  }
#endif /* NOHELP */
  return (success = 0);
}

int dosetftphlp() {
  int cx;
  if ((cx = cmkey(ftpset, nftpset, "", "", xxstring)) < 0)
    if (cx != -3)
      return (cx);
  if (cx != -3)
    ckstrncpy(tmpbuf, atmbuf, TMPBUFSIZ);
  if ((x = cmcfm()) < 0)
    return (x);

#ifdef NOHELP
  printf("Sorry, no help available\n");
#else
  switch (cx) {
  case -3:
    printf("\nSyntax: SET FTP parameter value\n");
    printf("  Type \"help set ftp ?\" for a list of parameters.\n");
    printf("  Type \"help set ftp xxx\" for information about setting\n");
    printf("  parameter xxx.  Type \"show ftp\" for current values.\n\n");
    return (0);

  case FTS_BUG:
    printf("\nSyntax: SET FTP BUG <name> {ON, OFF}\n");
    printf("  Activates a workaround for the named bug in the FTP server.\n");
    printf("  Type SET FTP BUG ? for a list of names.\n");
    printf("  For each bug, the default is OFF\n\n");
    return (0);

  case FTS_LOG: /* "autologin" */
    printf("\nSET FTP AUTOLOGIN { ON, OFF }\n");
    printf("  Tells Kermit whether to try to log you in automatically\n");
    printf("  as part of the connection process.\n\n");
    break;

  case FTS_DIS:
    printf("\nSET FTP DISPLAY { BRIEF, FULLSCREEN, CRT, ... }\n");
    printf("  Chooses the file-transfer display style for FTP.\n");
    printf("  Like SET TRANSFER DISPLAY but applies only to FTP.\n\n");
    break;

#ifndef NOCSETS
  case FTS_XLA: /* "character-set-translation" */
    printf("\nSET FTP CHARACTER-SET-TRANSLATION { ON, OFF }\n");
    printf("  Whether to translate character sets when transferring\n");
    printf("  text files with FTP.  OFF by default.\n\n");
    break;

#endif /* NOCSETS */
  case FTS_FNC: /* "collision" */
    printf("\n");
    printf("Syntax: SET FTP COLLISION { "
           "BACKUP,RENAME,UPDATE,DISCARD,APPEND,OVERWRITE }\n");
    printf("  Tells what do when an incoming file has the same name as\n");
    printf("  an existing file when downloading with FTP.\n\n");
    break;

  case FTS_DBG: /* "debug" */
    printf("\nSyntax: SET FTP DEBUG { ON, OFF }\n");
    printf("  Whether to print FTP protocol messages.\n\n");
    return (0);

  case FTS_ERR: /* "error-action" */
    printf("\nSyntax: SET FTP ERROR-ACTION { QUIT, PROCEED }\n");
    printf("  What to do when an error occurs when transferring a group\n");
    printf("  of files: quit and fail, or proceed to the next file.\n\n");
    return (0);

  case FTS_CNV: /* "filenames" */
    printf("\nSyntax: SET FTP FILENAMES { AUTO, CONVERTED, LITERAL }\n");
    printf("  What to do with filenames: convert them, take and use them\n");
    printf("  literally; or choose what to do automatically based on the\n");
    printf("  OS type of the server.  The default is AUTO.\n\n");
    return (0);

  case FTS_PSV: /* "passive-mode" */
    printf("\nSyntax: SET FTP PASSIVE-MODE { ON, OFF }\n");
    printf("  Whether to use passive mode, which helps to get through\n");
    printf("  firewalls.  ON by default.\n\n");
    return (0);

  case FTS_PRM: /* "permissions" */
    printf("\nSyntax: SET FTP PERMISSIONS { AUTO, ON, OFF }\n");
    printf("  Whether to try to send file permissions when uploading.\n");
    printf("  OFF by default.  AUTO means only if client and server\n");
    printf("  have the same OS type.\n\n");
    return (0);

  case FTS_TST: /* "progress-messages" */
    printf("\nSyntax: SET FTP PROGRESS-MESSAGES { ON, OFF }\n");
    printf("  Whether Kermit should print locally-generated feedback\n");
    printf("  messages for each non-file-transfer command.");
    printf("  ON by default.\n\n");
    return (0);

  case FTS_SPC: /* "send-port-commands" */
    printf("\nSyntax: SET FTP SEND-PORT-COMMANDS { ON, OFF }\n");
    printf("  Whether Kermit should send a new PORT command for each");
    printf("  task.\n\n");
    return (0);

#ifndef NOCSETS
  case FTS_CSR: /* "server-character-set" */
    printf("\nSyntax: SET FTP SERVER-CHARACTER-SET name\n");
    printf("  The name of the character set used for text files on the\n");
    printf("  server.  Enter a name of '?' for a menu.\n\n");
    return (0);
#endif /* NOCSETS */

  case FTS_STO: /* "server-time-offset */
    printf("\nSyntax: SET FTP SERVER-TIME-OFFSET +hh[:mm[:ss]] or "
           "-hh[:mm[:ss]]\n");
    printf("  Specifies an offset to apply to the server's file timestamps.\n");
    printf(
        "  Use this to correct for misconfigured server time or timezone.\n");
    printf("  Format: must begin with + or - sign.  Hours must be given; "
           "minutes\n");
    printf(
        "  and seconds are optional: +4 = +4:00 = +4:00:00 (add 4 hours).\n\n");
    return (0);

  case FTS_TYP: /* "type" */
    printf("\nSyntax: SET FTP TYPE { TEXT, BINARY, TENEX }\n");
    printf("  Establishes the default transfer mode.\n");
    printf("  TENEX is used for uploading 8-bit binary files to 36-bit\n");
    printf("  platforms such as TENEX and TOPS-20 and for downloading\n");
    printf("  them again.  ASCII is a synonym for TEXT.  Normally each\n");
    printf("  file's type is determined automatically from its contents\n");
    printf("  or its name; SET FTP TYPE does not prevent that, it only\n");
    printf("  tells which mode to use when the type can't be determined\n");
    printf("  automatically.  To completely disable automatic transfer-\n");
    printf("  mode switching and force either text or binary mode, give\n");
    printf("  the top-level command ASCII or BINARY, as in traditional\n");
    printf("  FTP clients.\n\n");
    return (0);

#ifdef FTP_TIMEOUT
  case FTS_TMO:
    printf("\nSyntax: SET FTP TIMEOUT number-of-seconds\n");
    printf("  Establishes a timeout for FTP transfers.\n");
    printf("  The timeout applies per network read or write on the data\n");
    printf("  connection, not to the whole transfer.  By default the\n");
    printf("  timeout value is 0, meaning no timeout.  Use a positive\n");
    printf("  number to escape gracefully from hung data connections or\n");
    printf("  directory listings.\n\n");
    return (0);
#endif /* FTP_TIMEOUT */

#ifdef PATTERNS
  case FTS_GFT:
    printf("\nSyntax: SET FTP GET-FILETYPE-SWITCHING { ON, OFF }\n");
    printf("  Tells whether GET and MGET should automatically switch\n");
    printf("  the appropriate file type, TEXT, BINARY, or TENEX, by\n");
    printf("  matching the name of each incoming file with its list of\n");
    printf("  FILE TEXT-PATTERNS and FILE BINARY-PATTERNS.  ON by\n");
    printf("  default.  SHOW PATTERNS displays the current pattern\n");
    printf("  list.  HELP SET FILE to see how to change it.\n");
    break;
#endif /* PATTERNS */

  case FTS_USN: /* "unique-server-names" */
    printf("\nSyntax: SET FTP UNIQUE-SERVER-NAMES { ON, OFF }\n");
    printf("  Tells whether to ask the server to create unique names\n");
    printf("  for any uploaded file that has the same name as an\n");
    printf("  existing file.  Default is OFF.\n\n");
    return (0);

  case FTS_VBM: /* "verbose-mode" */
    printf("\nSyntax: SET FTP VERBOSE-MODE { ON, OFF }\n");
    printf("  Whether to display all responses from the FTP server.\n");
    printf("  OFF by default.\n\n");
    return (0);

  case FTS_DAT:
    printf("\nSyntax: SET FTP DATES { ON, OFF }\n");
    printf("  Whether to set date of incoming files from the file date\n");
    printf("  on the server.  ON by default.  Note: there is no way to\n");
    printf("  set the date on files uploaded to the server.  Also note\n");
    printf("  that not all servers support this feature.\n\n");
    return (0);

  case FTS_APW:
    printf("\nSyntax: SET FTP ANONYMOUS-PASSWORD [ text ]\n");
    printf("  Password to supply automatically on anonymous FTP\n");
    printf("  connections instead of the default user@host.\n");
    printf("  Omit optional text to restore default.\n\n");
    return (0);

  default:
    printf("Sorry, help not available for \"set ftp %s\"\n", tmpbuf);
  }
#endif /* NOHELP */
  return (0);
}

#ifndef L_SET
#define L_SET 0
#endif /* L_SET */
#ifndef L_INCR
#define L_INCR 1
#endif /* L_INCR */

static int kerror; /* Needed for all auth types */

static struct sockaddr_in hisctladdr;
static struct sockaddr_in hisdataaddr;
static struct sockaddr_in data_addr;
static int data = -1;
#ifdef FTP_PROXY
static int ptflag = 0;
#endif /* FTP_PROXY */
static struct sockaddr_in myctladdr;

static int cpend = 0; /* No pending replies */

/*  f t p c m d  --  Send a command to the FTP server  */
/*
  Call with:
    char * cmd: The command to send.
    char * arg: The argument (e.g. a filename).
    int lcs: The local character set index.
    int rcs: The remote (server) character set index.
    int vbm: Verbose mode:
      0 = force verbosity off
     >0 = force verbosity on

  If arg is given (not NULL or empty) and lcs != rcs and both are > -1,
  and neither lcs or rcs is UCS-2, the arg is translated from the local
  character set to the remote one before sending the result to the server.

   Returns:
    0 on failure with ftpcode = -1
    >= 0 on success (getreply() result) with ftpcode = 0.
*/
static char xcmdbuf[RFNBUFSIZ];

static int ftpcmd(char *cmd, char *arg, int lcs, int rcs, int vbm) {
  char *s = NULL;
  int r = 0, x = 0, fc = 0, len = 0, cmdlen = 0, q = -1;

  if (ftp_deb) /* DEBUG */
    vbm = 1;
  else if (quiet || dpyactive) /* QUIET or File Transfer Active */
    vbm = 0;
  else if (vbm < 0) /* VERBOSE */
    vbm = ftp_vbm;

  cancelfile = 0;
  if (!cmd)
    cmd = "";
  if (!arg)
    arg = "";
  cmdlen = (int)strlen(cmd);
  len = cmdlen + (int)strlen(arg) + 1;

  if (ftp_deb /* && !dpyactive */) {
#ifdef FTP_PROXY
    if (ftp_prx)
      printf("%s ", ftp_host);
#endif /* FTP_PROXY */
    printf("---> ");
    if (!anonymous && strcmp("PASS", cmd) == 0)
      printf("PASS XXXX");
    else
      printf("%s %s", cmd, arg);
    printf("\n");
  }
  /* bzero(xcmdbuf,RFNBUFSIZ); */
  ckmakmsg(xcmdbuf, RFNBUFSIZ, cmd, *arg ? " " : "", arg, NULL);

#ifdef DEBUG
  if (deblog) {
    debug(F110, "ftpcmd cmd", cmd, 0);
    debug(F110, "ftpcmd arg", arg, 0);
    debug(F101, "ftpcmd lcs", "", lcs);
    debug(F101, "ftpcmd rcs", "", rcs);
  }
#endif /* DEBUG */

  if (csocket == -1) {
    perror("No control connection for command");
    ftpcode = -1;
    return (0);
  }
  havesigint = 0;
  oldintr = ck_signal(SIGINT, cmdcancel);

#ifndef NOCSETS
  if (*arg &&           /* If an arg was given */
      lcs > -1 &&       /* and a local charset */
      rcs > -1 &&       /* and a remote charset */
      lcs != rcs &&     /* and the two are not the same */
      lcs != FC_UCS2 && /* and neither one is UCS-2 */
      rcs != FC_UCS2    /* ... */
  ) {
    initxlate(lcs, rcs); /* Translate arg from lcs to rcs */
    xgnbp = arg;         /* Global pointer to input string */
    rfnptr = rfnbuf;     /* Global pointer to output buffer */

    while (1) {
      if ((c0 = xgnbyte(FC_UCS2, lcs, strgetc)) < 0)
        break;
      if (xpnbyte(c0, TC_UCS2, rcs, strputc) < 0)
        break;
    }
    /*
      We have to copy here instead of translating directly into
      xcmdbuf[] so strputc() can check length.  Alternatively we could
      write yet another xpnbyte() output function.
    */
    if ((int)strlen(rfnbuf) > (RFNBUFSIZ - (cmdlen + 1))) {
      printf("?FTP command too long: %s + arg\n", cmd);
      ftpcode = -1;
      return (0);
    }
    x = ckstrncpy(&xcmdbuf[cmdlen + 1], rfnbuf, RFNBUFSIZ - (cmdlen + 1));
  }
#endif /* NOCSETS */

  s = xcmdbuf; /* Command to send to server */

#ifdef DEBUG
  if (deblog) { /* Log it */
    if (!anonymous && !ckstrcmp(s, "PASS ", 5, 0)) {
      /* But don't log passwords */
      debug(F110, "FTP SENT ", "PASS XXXX", 0);
    } else {
      debug(F110, "FTP SENT ", s, 0);
    }
  }
#endif /* DEBUG */

  if (scommand(s) == 0) { /* Send it. */
    ck_signal(SIGINT, oldintr);
    return (0);
  }
  cpend = 1;
  x = !strcmp(cmd, "QUIT"); /* Is it the QUIT command? */
  if (x)                    /* In case we're interrupted */
    connected = 0;          /* while waiting for the reply... */

  fc = 0;                       /* Function code for getreply() */
  if (!strncmp(cmd, "AUTH ", 5) /* Must parse AUTH reply */
#ifdef FTPHOST
      && strncmp(cmd, "HOST ", 5)
#endif /* FTPHOST */
  ) {
    fc = GRF_AUTH;
  } else if (!ckstrcmp(cmd, "FEAT", -1, 0)) { /* Must parse FEAT reply */
    fc = GRF_FEAT; /* But FEAT not widely understood */
    if (!ftp_deb)  /* So suppress error messages */
      vbm = 9;
  }
  r = getreply(x,        /* Expect connection to close */
               lcs, rcs, /* Charsets */
               vbm,      /* Verbosity */
               fc        /* Function code */
  );
  if (q > -1)
    quiet = q;

  ck_signal(SIGINT, oldintr);
  return (r);
}

static void lostpeer() {
  debug(F100, "lostpeer", "", 0);
  if (connected) {
    if (csocket != -1) {
#ifdef TCPIPLIB
      socket_close(csocket);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
      shutdown(csocket, 1 + 1);
#endif /* USE_SHUTDOWN */
      close(csocket);
#endif /* TCPIPLIB */
      csocket = -1;
    }
    if (data != -1) {
#ifdef TCPIPLIB
      socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
      shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
      close(data);
#endif /* TCPIPLIB */
      data = -1;
      globaldin = -1;
    }
    connected = 0;
    anonymous = 0;
    loggedin = 0;
    auth_type = NULL;
    ftp_cpl = ftp_dpl = FPL_CLR;
#ifdef CKLOGDIAL
    ftplogend();
#endif /* CKLOGDIAL */

#ifdef LOCUS
    if (autolocus)    /* Auotomatic locus switching... */
      setlocus(1, 1); /* Switch locus to local. */
#endif                /* LOCUS */
  }
#ifdef FTP_PROXY
  pswitch(1);
  if (connected) {
    if (csocket != -1) {
#ifdef TCPIPLIB
      socket_close(csocket);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
      shutdown(csocket, 1 + 1);
#endif /* USE_SHUTDOWN */
      close(csocket);
#endif /* TCPIPLIB */
      csocket = -1;
    }
    connected = 0;
    anonymous = 0;
    loggedin = 0;
    auth_type = NULL;
    ftp_cpl = ftp_dpl = FPL_CLR;
  }
  proxflag = 0;
  pswitch(0);
#endif /* FTP_PROXY */
}

int ftpisopen() { return (connected); }

static int ftpclose() {
  extern int quitting;
  if (!connected)
    return (0);
  ftp_xfermode = xfermode;
  if (!ftp_vbm && !quiet)
    printlines = 1;
  ftpcmd("QUIT", NULL, 0, 0, ftp_vbm);
  if (csocket) {
#ifdef TCPIPLIB
    socket_close(csocket);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(csocket, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(csocket);
#endif /* TCPIPLIB */
  }
  csocket = -1;
  connected = 0;
  anonymous = 0;
  loggedin = 0;
  mdtmok = 1;
  sizeok = 1;
  featok = 1;
  stouarg = 1;
  typesent = 0;
  data = -1;
  globaldin = -1;
#ifdef FTP_PROXY
  if (!proxy)
    macnum = 0;
#endif /* FTP_PROXY */
  auth_type = NULL;
  ftp_dpl = FPL_CLR;
#ifdef CKLOGDIAL
  ftplogend();
#endif /* CKLOGDIAL */
#ifdef LOCUS
  /* Unprefixed file management commands are executed locally */
  if (autolocus && !ftp_cmdlin && !quitting) {
    setlocus(1, 1);
  }
#endif /* LOCUS */
  return (0);
}

int ftpopen(char *remote, char *service, int use_tls) {
  char *host;

  if (connected) {
    printf("?Already connected to %s, use FTP CLOSE first.\n", ftp_host);
    ftpcode = -1;
    return (0);
  }
#ifdef FTPHOST
  hostcmd = 0;
#endif /* FTPHOST */
  alike = 0;
  ftp_srvtyp[0] = NUL;
  if (!service)
    service = "";
  if (!*service)
    service = use_tls ? "ftps" : "ftp";

  if (!isdigit(service[0])) {
    struct servent *destsp;
    destsp = getservbyname(service, "tcp");
    if (!destsp) {
      if (!ckstrcmp(service, "ftp", -1, 0)) {
        ftp_port = 21;
      } else if (!ckstrcmp(service, "ftps", -1, 0)) {
        ftp_port = 990;
      } else {
        printf("?Bad port name - \"%s\"\n", service);
        ftpcode = -1;
        return (0);
      }
    } else {
      ftp_port = destsp->s_port;
      ftp_port = ntohs((unsigned short)ftp_port); /* SMS 2007/02/15 */
    }
  } else
    ftp_port = atoi(service);
  if (ftp_port <= 0) {
    printf("?Bad port name - \"%s\"\n", service);
    ftpcode = -1;
    return (0);
  }
  host = ftp_hookup(remote, ftp_port, use_tls);
  if (host) {
    ckstrncpy(ftp_user_host, remote, MAX_DNS_NAMELEN);
    connected = 1; /* Set FTP defaults */
    ftp_cpl = ftp_dpl = FPL_CLR;
    curtype = FTT_ASC; /* Server uses ASCII mode */
    form = FORM_N;
    mode = MODE_S;
    stru = STRU_F;
    strcpy(bytename, "8");
    bytesize = 8;

    if (ftp_log) /* ^^^ */
      ftp_login(remote);

    if (!connected)
      goto fail;

    ftp_xfermode = xfermode;

#ifdef CKLOGDIAL
    dologftp();
#endif /* CKLOGDIAL */
    passivemode = ftp_psv;
    sendport = ftp_spc;
    mdtmok = 1;
    sizeok = 1;
    stouarg = 1;
    typesent = 0;

    if (ucbuf == NULL) {
      actualbuf = DEFAULT_PBSZ;
      while (actualbuf && (ucbuf = (CHAR *)malloc(actualbuf)) == NULL)
        actualbuf >>= 2;
    }
    if (!maxbuf)
      ucbufsiz = actualbuf - FUDGE_FACTOR;
    debug(F101, "ftpopen ucbufsiz", "", ucbufsiz);
    return (1);
  }
fail:
  printf("?Can't FTP connect to %s:%s\n", remote, service);
  ftpcode = -1;
  return (0);
}

static void cmdcancel(int sig) {
  debug(F100, "ftp cmdcancel caught SIGINT ", "", 0);
  fflush(stdout);
  secure_getc(0, 1); /* Initialize net input buffers */
  cancelfile++;
  cancelgroup++;
  mlsreset();
#ifdef FTP_PROXY
  if (ptflag) /* proxy... */
    longjmp(ptcancel, 1);
#endif /* FTP_PROXY */
  debug(F100, "ftp cmdcancel chain to trap()...", "", 0);
  trap(SIGINT);
  /* NOTREACHED */
  debug(F100, "ftp cmdcancel return from trap()...", "", 0);
}

static int scommand(char *s) /* Was secure_command() */
{
  int length = 0, len2;
  char in[FTP_BUFSIZ], out[FTP_BUFSIZ];

  if (auth_type && ftp_cpl != FPL_CLR) {
    /* Other auth types go here ... */

    len2 = FTP_BUFSIZ;
    if ((kerror = radix_encode((CHAR *)out, (CHAR *)in, length, &len2,
                               RADIX_ENCODE))) {
      fprintf(stderr, "Couldn't base 64 encode command (%s)\n",
              radix_error(kerror));
      return (0);
    }
    if (ftp_deb)
      fprintf(stderr, "scommand(%s)\nencoding %d bytes\n", s, length);
    len2 = ckmakmsg(out, FTP_BUFSIZ, ftp_cpl == FPL_PRV ? "ENC " : "MIC ", in,
                    "\r\n", NULL);
    send(csocket, (SENDARG2TYPE)out, len2, 0);
  } else {
    char out[FTP_BUFSIZ];
    int len = ckmakmsg(out, FTP_BUFSIZ, s, "\r\n", NULL, NULL);
    send(csocket, (SENDARG2TYPE)out, len, 0);
  }
  return (1);
}

static int mygetc() {
  static char inbuf[4096];
  static int bp = 0, ep = 0;
  int rc;

  if (bp == ep) {
    bp = ep = 0;
    rc = recv(csocket, (char *)inbuf, 4096, 0);
    if (rc <= 0)
      return (EOF);
    ep = rc;
  }
  return (inbuf[bp++]);
}

/*  x l a t e c  --  Translate a character  */
/*
    Call with:
      fc    = Function code: 0 = translate, 1 = initialize.
      c     = Character (as int).
      incs  = Index of charset to translate from.
      outcs = Index of charset to translate to.

    Returns:
      0: OK
     -1: Error
*/
static int xlatec(int fc, int c, int incs, int outcs) {
#ifdef NOCSETS
  return (c);
#else
  static char buf[128];
  static int cx;
  int c0;

  if (fc == 1) {  /* Initialize */
    cx = 0;       /* Catch-up buffer write index */
    xgnbp = buf;  /* Catch-up buffer read pointer */
    buf[0] = NUL; /* Buffer is empty */
    return (0);
  }
  if (cx >= 127) {                         /* Catch-up buffer full */
    debug(F100, "xlatec overflow", "", 0); /* (shouldn't happen) */
    printf("?Translation buffer overflow\n");
    return (-1);
  }
  /* Add char to buffer. */
  /* The buffer won't grow unless incs is a multibyte set, e.g. UTF-8. */

  debug(F000, "xlatec buf", ckitoa(cx), c);
  buf[cx++] = c;
  buf[cx] = NUL;

  while ((c0 = xgnbyte(FC_UCS2, incs, strgetc)) > -1) {
    if (xpnbyte(c0, TC_UCS2, outcs, NULL) < 0) /* (NULL was xprintc) */
      return (-1);
  }
  /* If we're caught up, reinitialize the buffer */
  return ((cx == (xgnbp - buf)) ? xlatec(1, 0, 0, 0) : 0);
#endif /* NOCSETS */
}

/*  p a r s e f e a t  */

/* Note: for convenience we align keyword values with table indices */
/* If you need to insert a new keyword, adjust the SFT_xxx definitions */

static struct keytab feattab[] = {{"$$$$", 0, 0}, /* Dummy for sfttab[0] */
                                  {"AUTH", SFT_AUTH, 0}, {"LANG", SFT_LANG, 0},
                                  {"MDTM", SFT_MDTM, 0}, {"MLST", SFT_MLST, 0},
                                  {"PBSZ", SFT_PBSZ, 0}, {"PROT", SFT_PROT, 0},
                                  {"REST", SFT_REST, 0}, {"SIZE", SFT_SIZE, 0},
                                  {"TVFS", SFT_TVFS, 0}, {"UTF8", SFT_UTF8, 0}};
static int nfeattab = (sizeof(feattab) / sizeof(struct keytab));

#define FACT_CSET 1
#define FACT_CREA 2
#define FACT_LANG 3
#define FACT_MTYP 4
#define FACT_MDTM 5
#define FACT_PERM 6
#define FACT_SIZE 7
#define FACT_TYPE 8
#define FACT_UNIQ 9

static struct keytab facttab[] = {
    {"CHARSET", FACT_CSET, 0}, {"CREATE", FACT_CREA, 0},
    {"LANG", FACT_LANG, 0},    {"MEDIA-TYPE", FACT_MTYP, 0},
    {"MODIFY", FACT_MDTM, 0},  {"PERM", FACT_PERM, 0},
    {"SIZE", FACT_SIZE, 0},    {"TYPE", FACT_TYPE, 0},
    {"UNIQUE", FACT_UNIQ, 0}};
static int nfacttab = (sizeof(facttab) / sizeof(struct keytab));

static struct keytab ftyptab[] = {{"CDIR", FTYP_CDIR, 0},
                                  {"DIR", FTYP_DIR, 0},
                                  {"FILE", FTYP_FILE, 0},
                                  {"PDIR", FTYP_PDIR, 0}};
static int nftyptab = (sizeof(ftyptab) / sizeof(struct keytab));

static void parsefeat(char *s) /* Parse a FEATURE response */
{
  char kwbuf[8];
  int i, x;
  if (!s)
    return;
  if (!*s)
    return;
  while (*s < '!')
    s++;
  for (i = 0; i < 4; i++) {
    if (s[i] < '!')
      break;
    kwbuf[i] = s[i];
  }
  if (s[i] && s[i] != SP && s[i] != CK_CR && s[i] != LF)
    return;
  kwbuf[i] = NUL;
  /* xlookup requires a full (but case independent) match */
  i = xlookup(feattab, kwbuf, nfeattab, &x);
  debug(F111, "ftp parsefeat", s, i);
  if (i < 0 || i > 15)
    return;

  switch (i) {
  case SFT_MDTM: /* Controlled by ENABLE/DISABLE */
    sfttab[i] = mdtmok;
    if (mdtmok)
      sfttab[0]++;
    break;
  case SFT_MLST: /* ditto */
    sfttab[i] = mlstok;
    if (mlstok)
      sfttab[0]++;
    break;
  case SFT_SIZE: /* ditto */
    sfttab[i] = sizeok;
    if (sizeok)
      sfttab[0]++;
    break;
  case SFT_AUTH: /* ditto */
    sfttab[i] = ftp_aut;
    if (ftp_aut)
      sfttab[0]++;
    break;
  default: /* Others */
    sfttab[0]++;
    sfttab[i]++;
  }
}

static char *parsefacts(char *s) /* Parse MLS[DT] File Facts */
{
  char *p;
  int i, j, x;
  if (!s)
    return (NULL);
  if (!*s)
    return (NULL);

  /* Maybe we should make a copy of s so we can poke it... */

  while ((p = ckstrchr(s, '='))) {
    *p = NUL; /* s points to fact */
    i = xlookup(facttab, s, nfacttab, &x);
    debug(F111, "ftp parsefact fact", s, i);
    *p = '=';
    s = p + 1; /* Now s points to arg */
    p = ckstrchr(s, ';');
    if (!p)
      p = ckstrchr(s, SP);
    if (!p) {
      debug(F110, "ftp parsefact end-of-val search fail", s, 0);
      break;
    }
    *p = NUL;
    debug(F110, "ftp parsefact valu", s, 0);
    switch (i) {
    case FACT_CSET: /* Ignore these for now */
    case FACT_CREA:
    case FACT_LANG:
    case FACT_PERM:
    case FACT_MTYP:
    case FACT_UNIQ:
      break;
    case FACT_MDTM: /* Modtime */
      makestr(&havemdtm, s);
      debug(F110, "ftp parsefact mdtm", havemdtm, 0);
      break;
    case FACT_SIZE: /* Size */
      havesize = ckatofs(s);
      debug(F101, "ftp parsefact size", "", havesize);
      break;
    case FACT_TYPE: /* Type */
      j = xlookup(ftyptab, s, nftyptab, NULL);
      debug(F111, "ftp parsefact type", s, j);
      havetype = (j < 1) ? 0 : j;
      break;
    }
    *p = ';';
    s = p + 1; /* s points next fact or name */
  }
  while (*s == SP) /* Skip past spaces. */
    s++;
  if (!*s) /* Make sure we still have a name */
    s = NULL;
  debug(F110, "ftp parsefact name", s, 0);
  return (s);
}

/*  g e t r e p l y  --  (to an FTP command sent to server)  */

/* vbm = 1 (verbose); 0 (quiet except for error messages); 9 (super quiet) */

static int getreply(int expecteof, int lcs, int rcs, int vbm, int fc) {
  /* lcs, rcs, vbm parameters as in ftpcmd() */
  register int i, c, n;
  register int dig;
  register char *cp;
  int xlate = 0;
  int count = 0;
  int auth = 0;
  int originalcode = 0, continuation = 0;
  int pflag = 0;
  char *pt = pasv;
  char ibuf[FTP_BUFSIZ], obuf[FTP_BUFSIZ]; /* (these are pretty big...) */
  int safe = 0;
  int xquiet = 0;

  auth = (fc == GRF_AUTH);

#ifndef NOCSETS
  debug(F101, "ftp getreply lcs", "", lcs);
  debug(F101, "ftp getreply rcs", "", rcs);
  if (lcs > -1 && rcs > -1 && lcs != rcs) {
    xlate = 1;
    initxlate(rcs, lcs);
    xlatec(1, 0, rcs, lcs);
  }
#endif /* NOCSETS */
  debug(F101, "ftp getreply fc", "", fc);

  if (quiet)
    xquiet = 1;
  if (vbm == 9) {
    xquiet = 1;
    vbm = 0;
  }
  if (ftp_deb) /* DEBUG */
    vbm = 1;
  else if (quiet || dpyactive) /* QUIET or File Transfer Active */
    vbm = 0;
  else if (vbm < 0) /* VERBOSE */
    vbm = ftp_vbm;

  ibuf[0] = '\0';
  if (reply_parse)
    reply_ptr = reply_buf;
  havesigint = 0;
  oldintr = ck_signal(SIGINT, cmdcancel);
  for (count = 0;; count++) {
    obuf[0] = '\0';
    dig = n = ftpcode = i = 0;
    cp = ftp_reply_str;
    while ((c = ibuf[0] ? ibuf[i++] : mygetc()) != '\n') {
      if (c == IAC) { /* Handle telnet commands */
        switch (c = mygetc()) {
        case WILL:
        case WONT:
          c = mygetc();
          obuf[0] = IAC;
          obuf[1] = DONT;
          obuf[2] = c;
          obuf[3] = NUL;
          send(csocket, (SENDARG2TYPE)obuf, 3, 0);
          break;
        case DO:
        case DONT:
          c = mygetc();
          obuf[0] = IAC;
          obuf[1] = WONT;
          obuf[2] = c;
          obuf[3] = NUL;
          send(csocket, (SENDARG2TYPE)obuf, 3, 0);
          break;
        default:
          break;
        }
        continue;
      }
      dig++;
      if (c == EOF) {
        if (expecteof) {
          ck_signal(SIGINT, oldintr);
          ftpcode = 221;
          debug(F101, "ftp getreply EOF", "", ftpcode);
          return (0);
        }
        lostpeer();
        if (!xquiet) {
          if (ftp_deb)
            printf("421 ");
          printf("Service not available, connection closed by server\n");
          fflush(stdout);
        }
        ck_signal(SIGINT, oldintr);
        ftpcode = 421;
        debug(F101, "ftp getreply EOF", "", ftpcode);
        return (4);
      }
      if (n == 0) { /* First digit */
        n = c;      /* Save it */
      }
      if (auth_type && !ibuf[0] && (n == '6' || continuation)) {
        if (c != '\r' && dig > 4)
          obuf[i++] = c;
      } else {
        if (auth_type && !ibuf[0] && dig == 1 && vbm)
          printf("Unauthenticated reply received from server:\n");
        if (reply_parse) {
          *reply_ptr++ = c;
          *reply_ptr = NUL;
        }
        if ((!dpyactive || ftp_deb) && /* Don't mess up xfer display */
            ftp_cmdlin < 2) {
          if ((c != '\r') &&
              (ftp_deb ||
               ((vbm || (!auth && n == '5')) &&
                (dig > 4 || (dig <= 4 && !isdigit(c) && ftpcode == 0))))) {
#ifdef FTP_PROXY
            if (ftp_prx && (dig == 1 || (dig == 5 && vbm == 0)))
              printf("%s:", ftp_host);
#endif /* FTP_PROXY */

            if (!xquiet) {
#ifdef NOCSETS
              printf("%c", c);
#else
              if (xlate) {
                xlatec(0, c, rcs, lcs);
              } else {
                printf("%c", c);
              }
#endif /* NOCSETS */
            }
          }
        }
      }
      if (auth_type && !ibuf[0] && n != '6')
        continue;
      if (dig < 4 && isdigit(c))
        ftpcode = ftpcode * 10 + (c - '0');
      if (!pflag && ftpcode == 227)
        pflag = 1;
      if (dig > 4 && pflag == 1 && isdigit(c))
        pflag = 2;
      if (pflag == 2) {
        if (c != '\r' && c != ')')
          *pt++ = c;
        else {
          *pt = '\0';
          pflag = 3;
        }
      }
      if (dig == 4 && c == '-' && n != '6') {
        if (continuation)
          ftpcode = 0;
        continuation++;
      }
      if (cp < &ftp_reply_str[FTP_BUFSIZ - 1]) {
        *cp++ = c;
        *cp = NUL;
      }
    }
    if (deblog ||
        /* No, we can't be that clever -- it breaks other things like RPWD... */
        (printlines && (ftpcode != 631 && ftpcode != 632 && ftpcode != 633))) {
      char *q = cp;
      char *r = ftp_reply_str;
      *q-- = NUL;               /* NUL-terminate */
      while (*q < '!' && q > r) /* Strip CR, etc */
        *q-- = NUL;
      if (!ftp_deb && printlines) { /* If printing */
        if (ftpcode != 0)           /* strip ftpcode if any */
          r += 4;
#ifdef NOCSETS
        printf("%s\n", r); /* and print */
#else
        if (!xlate) {
          printf("%s\n", r);
        } else {     /* Translating */
          xgnbp = r; /* Set up strgetc() */
          while ((c0 = xgnbyte(FC_UCS2, rcs, strgetc)) > -1) {
            if (xpnbyte(c0, TC_UCS2, lcs, NULL) < 0) { /* (xprintc) */
              ck_signal(SIGINT, oldintr);
              return (-1);
            }
          }
          printf("\n");
        }
#endif /* NOCSETS */
      }
    }
    debug(F110, "FTP RCVD ", ftp_reply_str, 0);

    if (fc == GRF_FEAT) { /* Parsing FEAT command response? */
      if (count == 0 && n == '2') {
        int i; /* (Re)-init server FEATure table */
        debug(F100, "ftp getreply clearing feature table", "", 0);
        for (i = 0; i < 16; i++)
          sfttab[i] = 0;
      } else {
        parsefeat((char *)ftp_reply_str);
      }
    }
    if (auth_type && !ibuf[0] && n != '6') {
      ck_signal(SIGINT, oldintr);
      return (getreply(expecteof, lcs, rcs, vbm, auth));
    }
    ibuf[0] = obuf[i] = '\0';
    if (ftpcode && n == '6') {
      if (ftpcode != 631 && ftpcode != 632 && ftpcode != 633) {
        printf("Unknown reply: %d %s\n", ftpcode, obuf);
        n = '5';
      } else
        safe = (ftpcode == 631);
    }
    if (obuf[0] /* if there is a string to decode */
    ) {
      if (!auth_type) {
        printf("Cannot decode reply:\n%d %s\n", ftpcode, obuf);
        n = '5';
      } else if (ftpcode == 632) {
        printf("Cannot decrypt %d reply: %s\n", ftpcode, obuf);
        n = '5';
      }
#ifdef NOCONFIDENTIAL
      else if (ftpcode == 633) {
        printf("Cannot decrypt %d reply: %s\n", ftpcode, obuf);
        n = '5';
      }
#endif /* NOCONFIDENTIAL */
      else {
        int len = FTP_BUFSIZ;
        if ((kerror = radix_encode((CHAR *)obuf, (CHAR *)ibuf, 0, &len,
                                   RADIX_DECODE))) {
          printf("Can't decode base 64 reply %d (%s)\n\"%s\"\n", ftpcode,
                 radix_error(kerror), obuf);
          n = '5';
        }
        /* Other auth types go here... */
      }
    } else if ((!dpyactive || ftp_deb) && ftp_cmdlin < 2 && !xquiet &&
               (vbm || (!auth && (n == '4' || n == '5')))) {
#ifdef NOCSETS
      printf("%c", c);
#else
      if (xlate) {
        xlatec(0, c, rcs, lcs);
      } else {
        printf("%c", c);
      }
#endif /* NOCSETS */
      fflush(stdout);
    }
    if (continuation && ftpcode != originalcode) {
      if (originalcode == 0)
        originalcode = ftpcode;
      continue;
    }
    *cp = '\0';
    if (n != '1')
      cpend = 0;
    ck_signal(SIGINT, oldintr);
    if (ftpcode == 421 || originalcode == 421) {
      lostpeer();
      if (!xquiet && !ftp_deb)
        printf("%s\n", reply_buf);
    }
    if ((cancelfile != 0) &&
        /* Ultrix 3.0 cc objects violently to this clause */
        (oldintr != cmdcancel) && (oldintr != SIG_IGN)) {
      if (oldintr)
        (*oldintr)(SIGINT);
    }
    if (reply_parse) {
      *reply_ptr = '\0';
      if ((reply_ptr = ckstrstr(reply_buf, reply_parse))) {
        reply_parse = reply_ptr + strlen(reply_parse);
        if ((reply_ptr = ckstrpbrk(reply_parse, " \r")))
          *reply_ptr = '\0';
      } else
        reply_parse = reply_ptr;
    }
    while (*cp < '!' && cp > ftp_reply_str) /* Remove trailing junk */
      *cp-- = NUL;
    debug(F111, "ftp getreply", ftp_reply_str, n - '0');
    return (n - '0');
  } /* for (;;) */
}

#ifdef BSDSELECT
static int empty(fd_set *mask, int sec) {
  struct timeval t;
  t.tv_sec = (long)sec;
  t.tv_usec = 0L;
  debug(F100, "ftp empty calling select...", "", 0);
#ifdef INTSELECT
  x = select(32, (int *)mask, NULL, NULL, &t);
#else
  x = select(32, mask, (fd_set *)0, (fd_set *)0, &t);
#endif /* INTSELECT */
  debug(F101, "ftp empty select", "", x);
  return (x);
}
#else /* BSDSELECT */
#ifdef IBMSELECT
static int empty(mask, cnt, sec)
int *mask, sec;
int cnt;
{
  return (select(mask, cnt, 0, 0, sec * 1000));
}
#endif /* IBMSELECT */
#endif /* BSDSELECT */

static void cancelsend(int sig) {
  havesigint++;
  cancelgroup++;
  cancelfile = 0;
  printf(" Canceled...\n");
  secure_getc(0, 1); /* Initialize net input buffers */
  debug(F100, "ftp cancelsend caught SIGINT ", "", 0);
  fflush(stdout);
  longjmp(sendcancel, 1);
}

static void secure_error(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

/*
 * Internal form of settype; changes current type in use with server
 * without changing our notion of the type for data transfers.
 * Used to change to and from ascii for listings.
 */
static void changetype(int newtype, int show) {
  int rc;
  char *s;

  if ((newtype == curtype) && typesent++)
    return;
  switch (newtype) {
  case FTT_ASC:
    s = "A";
    break;
  case FTT_BIN:
    s = "I";
    break;
  case FTT_TEN:
    s = "L 8";
    break;
  default:
    s = "I";
    break;
  }
  rc = ftpcmd("TYPE", s, -1, -1, show);
  if (rc == REPLY_COMPLETE)
    curtype = newtype;
}

/* PUT a file.  Returns -1 on error, 0 on success, 1 if file skipped */

static void doftpsend(void *threadinfo) {
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  if (initconn()) {
#ifndef NOHTTP
    int y = -1;
    /* debug(F101,"doftpsend","tcp_http_proxy",tcp_http_proxy); */

    /*  If the connection failed and we are using an HTTP Proxy
     *  and the reason for the failure was an authentication
     *  error, then we need to give the user to ability to
     *  enter a username and password, just like a browser.
     *
     *  I tried to do all of this within the netopen() call
     *  but it is much too much work.
     */
    while (y != 0 && tcp_http_proxy != NULL) {

      if (tcp_http_proxy_errno == 401 || tcp_http_proxy_errno == 407) {
        char uid[UIDBUFLEN];
        char pwd[PWDSIZ];
        struct txtbox tb[2];
        int ok;

        tb[0].t_buf = uid;
        tb[0].t_len = UIDBUFLEN;
        tb[0].t_lbl = "Proxy Userid: ";
        tb[0].t_dflt = NULL;
        tb[0].t_echo = 1;
        tb[1].t_buf = pwd;
        tb[1].t_len = 256;
        tb[1].t_lbl = "Proxy Passphrase: ";
        tb[1].t_dflt = NULL;
        tb[1].t_echo = 2;

        ok = uq_mtxt("Proxy Server Authentication Required\n", NULL, 2, tb);
        if (ok && uid[0]) {
          char *proxy_user, *proxy_pwd;

          proxy_user = tcp_http_proxy_user;
          proxy_pwd = tcp_http_proxy_pwd;

          tcp_http_proxy_user = uid;
          tcp_http_proxy_pwd = pwd;

          y = initconn();

          debug(F101, "doftpsend", "initconn", y);
          memset(pwd, 0, PWDSIZ);
          tcp_http_proxy_user = proxy_user;
          tcp_http_proxy_pwd = proxy_pwd;
        } else
          break;
      } else
        break;
    }

    if (y != 0) {
#endif /* NOHTTP */
      ck_signal(SIGINT, ftpsnd.oldintr);
#ifdef SIGPIPE
      if (ftpsnd.oldintp)
        ck_signal(SIGPIPE, ftpsnd.oldintp);
#endif /* SIGPIPE */
      ftpcode = -1;
      zclose(ZIFILE);
      ftpsndret = -1;
      return;
#ifndef NOHTTP
    }
#endif /* NOHTTP */
  }
  ftpsndret = 0;
}

static void failftpsend(void *threadinfo) {
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  while (cpend) {
    ftpsnd.reply = getreply(0, ftpsnd.incs, ftpsnd.outcs, ftp_vbm, 0);
    debug(F111, "ftp sendrequest getreply", "null command", ftpsnd.reply);
  }
  if (data >= 0) {
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
    data = -1;
    globaldin = -1;
  }
  if (ftpsnd.oldintr)
    ck_signal(SIGINT, ftpsnd.oldintr);
#ifdef SIGPIPE
  if (ftpsnd.oldintp)
    ck_signal(SIGPIPE, ftpsnd.oldintp);
#endif /* SIGPIPE */
  ftpcode = -1;
  /* TEST ME IN K95 */
  if (havesigint) {
    havesigint = 0;
    debug(F100, "ftp failftpsend chain to trap()...", "", 0);
    if (ftpsnd.oldintr != SIG_IGN)
      (*ftpsnd.oldintr)(SIGINT);
    /* NOTREACHED (I hope!) */
    debug(F100, "ftp failftpsend return from trap()...", "", 0);
  }
}

static void failftpsend2(void *threadinfo) {
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  debug(F101, "ftp sendrequest canceled", "", ftpsnd.bytes);
  tfc += ffc;
#ifdef GFTIMER
  fpfsecs = gftimer();
#endif /* GFTIMER */
  zclose(ZIFILE);
#ifdef PIPESEND
  if (sndfilter)
    pipesend = 0;
#endif /* PIPESEND */
  ck_signal(SIGINT, ftpsnd.oldintr);
#ifdef SIGPIPE
  if (ftpsnd.oldintp)
    ck_signal(SIGPIPE, ftpsnd.oldintp);
#endif /* SIGPIPE */
  if (!cpend) {
    ftpcode = -1;
    ftpsndret = -1;
    return;
  }
  if (data >= 0) {
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
    data = -1;
    globaldin = -1;
  }
  if (dout) {
#ifdef TCPIPLIB
    socket_close(dout);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(dout, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(dout);
#endif /* TCPIPLIB */
  }
  ftpsnd.reply = getreply(0, ftpsnd.incs, ftpsnd.outcs, ftp_vbm, 0);
  ftpcode = -1;
  ftpsndret = -1;

  /* TEST ME IN K95 */
  if (havesigint) {
    havesigint = 0;
    debug(F100, "ftp failftpsend2 chain to trap()...", "", 0);
    if (ftpsnd.oldintr != SIG_IGN)
      (*ftpsnd.oldintr)(SIGINT);
    /* NOTREACHED (I hope!) */
    debug(F100, "ftp failftpsend2 return from trap()...", "", 0);
  }
}

static void doftpsend2(void *threadinfo) {
  register int c, d = 0;
  int n, x, notafile, unique = 0;
  char *buf, *bufp;

#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  buf = ftpsndbuf; /* (not on stack) */

  unique = strcmp(ftpsnd.cmd, "STOU") ? 0 : 1;
  notafile = sndarray || pipesend;

#ifdef FTP_RESTART
  if (ftpsnd.restart && ((curtype == FTT_BIN) || (alike > 0))) {
    char *p;
    changetype(FTT_BIN, 0); /* Change to binary */

    /* Ask for remote file's size */
    x = ftpcmd("SIZE", ftpsnd.remote, ftpsnd.incs, ftpsnd.outcs, ftp_vbm);

    if (x == REPLY_COMPLETE) { /* Have ftpsnd.reply */
      p = &ftp_reply_str[4];   /* Parse it */
      while (isdigit(*p)) {
        sendstart = sendstart * 10 + (int)(*p - '0');
        p++;
      }
      if (*p && *p != CK_CR) { /* Bad number */
        debug(F110, "doftpsend2 bad size", ftp_reply_str, 0);
        sendstart = (CK_OFF_T)0;
      } else if (sendstart > fsize) { /* Remote file bigger than local */
        debug(F110, "doftpsend2 big size", ckfstoa(fsize), sendstart);
        sendstart = (CK_OFF_T)0;
      }
      /* Local is newer */
      debug(F111, "doftpsend2 size", ftpsnd.remote, sendstart);
      if (chkmodtime(ftpsnd.local, ftpsnd.remote, 0) == 2) {
        debug(F110, "doftpsend2 date mismatch", ftp_reply_str, 0);
        sendstart = (CK_OFF_T)0; /* Send the whole file */
      }
    }
    changetype(ftp_typ, 0);        /* Change back to appropriate type */
    if (sendstart > (CK_OFF_T)0) { /* Still restarting? */
      if (sendstart == fsize) {    /* Same size - no need to send */
        debug(F111, "doftpsend2 /restart SKIP", ckfstoa(fsize), sendstart);
        zclose(ZIFILE);
        ftpsndret = SKP_RES;
        return;
      }
      errno = 0; /* Restart needed, seek to the spot */
      if (zfseek((long)sendstart) < 0) {
        debug(F111, "doftpsend2 zfseek fails", ftpsnd.local, sendstart);
        fprintf(stderr, "FSEEK: %s: %s\n", ftpsnd.local, ck_errstr());
        sendstart = 0;
        zclose(ZIFILE);
        ftpsndret = -1;
        return;
      }
      sendmode = SM_RESEND;
      ftpsnd.cmd = "APPE";
      /* sendstart = (CK_OFF_T)0; */
    }
  }
#endif /* FTP_RESTART */

  if (unique && !stouarg) /* If we know STOU accepts no arg */
    ftpsnd.remote = NULL; /* don't include one. */

  x = ftpcmd(ftpsnd.cmd, ftpsnd.remote, ftpsnd.incs, ftpsnd.outcs, ftp_vbm);
  debug(F111, "doftpsend2 ftpcode", ftpsnd.cmd, ftpcode);
  debug(F101, "doftpsend2 ftpcmd", "", x);

  if (x != REPLY_PRELIM && unique) {
    /*
      RFC959 says STOU does not take an argument.  But every FTP server
      I've encountered but one accepts the arg and constructs the unique
      name from it, which is better than making up a totally random name
      for the file, which is what RFC959 calls for.  Especially because
      there is no way for the client to find out the name chosen by the
      server.  So we try STOU with the argument first, which works with
      most servers, and if it fails we retry it without the arg, for
      the benefit of the one picky server that is not "liberal in what
      it accepts" UNLESS the first STOU got a 502 code ("not implemented")
      which means STOU is not accepted, period.
    */
    if ((x == 5) && stouarg && (ftpcode != 502)) {
      x = ftpcmd(ftpsnd.cmd, NULL, ftpsnd.incs, ftpsnd.outcs, ftp_vbm);
      if (x == REPLY_PRELIM) /* If accepted */
        stouarg = 0;         /* flag no STOU arg for this server */
    }
  }
  if (x != REPLY_PRELIM) {
    ck_signal(SIGINT, ftpsnd.oldintr);
#ifdef SIGPIPE
    if (ftpsnd.oldintp)
      ck_signal(SIGPIPE, ftpsnd.oldintp);
#endif /* SIGPIPE */
    debug(F101, "doftpsend2 not REPLY_PRELIM", "", x);
    zclose(ZIFILE);
#ifdef PIPESEND
    if (sndfilter)
      pipesend = 0;
#endif /* PIPESEND */
    ftpsndret = -1;
    return;
  }
  debug(F100, "doftpsend2 getting data connection...", "", 0);
  dout = dataconn(ftpsnd.lmode); /* Get data connection */
  debug(F101, "doftpsend2 dataconn", "", dout);
  if (dout == -1) {
    failftpsend2(threadinfo);
    return;
  }
  /* Initialize per-file stats */
  ffc = (CK_OFF_T)0; /* Character counter */
  cps = oldcps = 0L; /* Thruput */
  n = 0;
#ifdef GFTIMER
  rftimer(); /* reset f.p. timer */
#endif       /* GFTIMER */

#ifdef SIGPIPE
  ftpsnd.oldintp = ck_signal(SIGPIPE, SIG_IGN);
#endif /* SIGPIPE */
  debug(F101, "doftpsend2 curtype", "", curtype);
  switch (curtype) {
  case FTT_BIN: /* Binary mode */
  case FTT_TEN:
    errno = d = 0;
    while ((n = zxin(ZIFILE, buf, FTP_BUFSIZ - 1)) > 0 && !cancelfile) {
      ftpsnd.bytes += n;
      ffc += n;
      debug(F111, "doftpsend2 zxin", ckltoa(n), ffc);
      ckhexdump("doftpsend2 zxin", buf, 16);
      for (bufp = buf; n > 0; n -= d, bufp += d) {
        if (((d = secure_write(dout, (CHAR *)bufp, n)) <= 0) || iscanceled())
          break;
        spackets++;
        pktnum++;
        if (fdispla != XYFD_B) {
          spktl = d;
          ftscreen(SCR_PT, 'D', (CK_OFF_T)spackets, NULL);
        }
      }
      if (d <= 0)
        break;
    }

    debug(F111, "doftpsend2 XX zxin", ckltoa(n), ffc);
    if (n < 0)
      fprintf(stderr, "local: %s: %s\n", ftpsnd.local, ck_errstr());
    if (d < 0 || (d = secure_flush(dout)) < 0) {
      if (d == -1 && errno && errno != EPIPE)
        perror("netout");
      ftpsnd.bytes = -1;
    }
    break;

  case FTT_ASC: /* Text mode */
#ifndef NOCSETS
    if (ftpsnd.xlate) { /* With translation */
      initxlate(ftpsnd.incs, ftpsnd.outcs);
      while (!cancelfile) {
        if ((c0 = xgnbyte(FC_UCS2, ftpsnd.incs, NULL)) < 0)
          break;
        if ((x = xpnbyte(c0, TC_UCS2, ftpsnd.outcs, xxout)) < 0)
          break;
      }
    } else {
#endif /* NOCSETS */
      /* Text mode, no translation */
      while (((c = zminchar()) > -1) && !cancelfile) {
        ffc++;
        if (xxout(c) < 0)
          break;
      }
      d = 0;
#ifndef NOCSETS
    }
#endif /* NOCSETS */
    if (dout == -1 || (d = secure_flush(dout)) < 0) {
      if (d == -1 && errno && errno != EPIPE)
        perror("netout");
      ftpsnd.bytes = -1;
    }
    break;
  }
  tfc += ffc; /* Total file chars */
#ifdef GFTIMER
  fpfsecs = gftimer();
#endif            /* GFTIMER */
  zclose(ZIFILE); /* Close input file */
#ifdef PIPESEND
  if (sndfilter) /* Undo this (it's per file) */
    pipesend = 0;
#endif /* PIPESEND */

#ifdef TCPIPLIB
  socket_close(dout); /* Close data connection */
#else                 /* TCPIPLIB */
#ifdef USE_SHUTDOWN
  shutdown(dout, 1 + 1);
#endif /* USE_SHUTDOWN */
  close(dout);
#endif /* TCPIPLIB */
  ftpsnd.reply = getreply(0, ftpsnd.incs, ftpsnd.outcs, ftp_vbm, 0);
  ck_signal(SIGINT, ftpsnd.oldintr); /* Put back interrupts */
#ifdef SIGPIPE
  if (ftpsnd.oldintp)
    ck_signal(SIGPIPE, ftpsnd.oldintp);
#endif /* SIGPIPE */
  if (ftpsnd.reply == REPLY_TRANSIENT || ftpsnd.reply == REPLY_ERROR) {
    debug(F101, "doftpsend2 ftpsnd.reply", "", ftpsnd.reply);
    ftpsndret = -1;
    return;
  } else if (cancelfile) {
    debug(F101, "doftpsend2 canceled", "", ftpsnd.bytes);
    ftpsndret = -1;
    return;
  }
  debug(F101, "doftpsend2 ok", "", ftpsnd.bytes);
  ftpsndret = 0;
}

static int sendrequest(char *cmd, char *local, char *remote, int xlate,
                       int incs, int outcs, int restart) {
  if (!remote)
    remote = ""; /* Check args */
  if (!*remote)
    remote = local;
  if (!local)
    local = "";
  if (!*local)
    return (-1);
  if (!cmd)
    cmd = "";
  if (!*cmd)
    cmd = "STOR";

  debug(F111, "ftp sendrequest restart", local, restart);

  nout = 0;         /* Init output buffer count */
  ftpsnd.bytes = 0; /* File input byte count */
  dout = -1;

#ifdef FTP_PROXY
  if (proxy) {
    proxtrans(cmd, local, remote, !strcmp(cmd, "STOU"));
    return (0);
  }
#endif /* FTP_PROXY */

  changetype(ftp_typ, 0); /* Change type for this file */

  ftpsnd.oldintr = NULL; /* Set up interrupt handler */
  ftpsnd.oldintp = NULL;
  ftpsnd.restart = restart;
  ftpsnd.xlate = xlate;
  ftpsnd.lmode = "wb";

#ifdef PIPESEND /* Use Kermit API for file i/o... */
  if (sndfilter) {
    char *p = NULL, *q;
#ifndef NOSPL
    int n = CKMAXPATH;
    if (cmd_quoting && (p = (char *)malloc(n + 1))) {
      q = p;
      debug(F110, "sendrequest pipesend filter", sndfilter, 0);
      zzstring(sndfilter, &p, &n);
      debug(F111, "sendrequest pipename", q, n);
      if (n <= 0) {
        printf("?Sorry, send filter + filename too long, %d max.\n", CKMAXPATH);
        free(q);
        return (-1);
      }
      ckstrncpy(filnam, q, CKMAXPATH + 1);
      free(q);
      local = filnam;
    }
#endif /* NOSPL */
  }

  if (sndfilter)  /* If sending thru a filter */
    pipesend = 1; /* set this for open and i/o */
#endif            /* PIPESEND */

  if (openi(local) == 0) /* Try to open the input file */
    return (-1);

  ftpsndret = 0;
  ftpsnd.incs = incs;
  ftpsnd.outcs = outcs;
  ftpsnd.cmd = cmd;
  ftpsnd.local = local;
  ftpsnd.remote = remote;
  ftpsnd.oldintr = ck_signal(SIGINT, cancelsend);
  havesigint = 0;

  if (cc_execute(ckjaddr(sendcancel), doftpsend, failftpsend) < 0)
    return (-1);
  if (ftpsndret < 0)
    return (-1);
  if (cc_execute(ckjaddr(sendcancel), doftpsend2, failftpsend2) < 0)
    return (-1);

  return (ftpsndret);
}

static void cancelrecv(int sig) {
  havesigint++;
  cancelfile = 0;
  cancelgroup++;
  secure_getc(0, 1); /* Initialize net input buffers */
  printf(" Canceling...\n");
  debug(F100, "ftp cancelrecv caught SIGINT", "", 0);
  fflush(stdout);
  if (fp_nml) {
    if (fp_nml != stdout)
      fclose(fp_nml);
    fp_nml = NULL;
  }
  longjmp(recvcancel, 1);
}

/* Argumentless front-end for secure_getc() */

static int netgetc(void) /* Input function to point to... */
{
  return (secure_getc(globaldin, 0));
}

/* Returns -1 on failure, 0 on success, 1 if file skipped */

/*
  Sets ftpcode < 0 on failure if failure reason is not server reply code:
    -1: interrupted by user.
    -2: error opening or writing output file (reason in errno).
    -3: failure to make data connection.
    -4: network read error (reason in errno).
*/

struct xx_ftprecv {
  int reply;
  int fcs;
  int rcs;
  int recover;
  int xlate;
  int din;
  int is_retr;
  sig_t oldintr, oldintp;
  char *cmd;
  char *local;
  char *remote;
  char *lmode;
  char *pipename;
  int tcrflag;
  CK_OFF_T localsize;
};
static struct xx_ftprecv ftprecv;

static int ftprecvret = 0;

static void failftprecv(void *threadinfo) {

#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  while (cpend) {
    ftprecv.reply = getreply(0, ftprecv.fcs, ftprecv.rcs, ftp_vbm, 0);
  }
  if (data >= 0) {
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
    data = -1;
    globaldin = -1;
  }
  if (ftprecv.oldintr)
    ck_signal(SIGINT, ftprecv.oldintr);
  ftpcode = -1;
  ftprecvret = -1;

  /* TEST ME IN K95 */
  if (havesigint) {
    havesigint = 0;
    debug(F100, "ftp failftprecv chain to trap()...", "", 0);
    if (ftprecv.oldintr != SIG_IGN)
      (*ftprecv.oldintr)(SIGINT);
    /* NOTREACHED (I hope!) */
    debug(F100, "ftp failftprecv return from trap()...", "", 0);
  }
  return;
}

static void doftprecv(void *threadinfo) {
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  if (!out2screen && !ftprecv.pipename) {
    int x;
    char *local;
    local = ftprecv.local;
    x = zchko(local);
    if (x < 0) {
      if ((!dpyactive || ftp_deb))
        fprintf(stderr, "Temporary file %s: %s\n", ftprecv.local, ck_errstr());
      ck_signal(SIGINT, ftprecv.oldintr);
      ftpcode = -2;
      ftprecvret = -1;
      return;
    }
  }
  changetype((!ftprecv.is_retr) ? FTT_ASC : ftp_typ, 0);
  if (initconn()) { /* Initialize the data connection */
    ck_signal(SIGINT, ftprecv.oldintr);
    ftpcode = -1;
    ftprecvret = -3;
    return;
  }
  secure_getc(0, 1); /* Initialize net input buffers */
  ftprecvret = 0;
}

static void failftprecv2(void *threadinfo) {
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  /* Cancel using RFC959 recommended IP,SYNC sequence  */

  debug(F100, "ftp recvrequest CANCEL", "", 0);
#ifdef GFTIMER
  fpfsecs = gftimer();
#endif /* GFTIMER */
#ifdef SIGPIPE
  if (ftprecv.oldintp)
    ck_signal(SIGPIPE, ftprecv.oldintr);
#endif /* SIGPIPE */
  signal(SIGINT, SIG_IGN);
  if (!cpend) {
    ftpcode = -1;
    ck_signal(SIGINT, ftprecv.oldintr);
    ftprecvret = -1;
    return;
  }
  cancel_remote(ftprecv.din);

#ifdef FTP_TIMEOUT
  if (ftp_timed_out && out2screen && !quiet)
    printf("\n?Timed out.\n");
#endif /* FTP_TIMEOUT */

  if (ftpcode > -1)
    ftpcode = -1;
  if (data >= 0) {
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
    data = -1;
    globaldin = -1;
  }
  if (!out2screen) {
    int x = 0;
    debug(F111, "ftp failrecv2 zclose", ftprecv.local, keep);
    zclose(ZOFILE);
    switch (keep) {           /* which is... */
    case SET_AUTO:            /* AUTO */
      if (curtype == FTT_ASC) /* Delete file if TYPE A. */
        x = 1;
      break;
    case SET_OFF: /* DISCARD */
      x = 1;      /* Delete file, period. */
      break;
    default: /* KEEP */
      break;
    }
    if (x) {
      x = zdelet(ftprecv.local);
      debug(F111, "ftp failrecv2 delete incomplete", ftprecv.local, x);
    }
  }
  if (ftprecv.din) {
#ifdef TCPIPLIB
    socket_close(ftprecv.din);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(ftprecv.din, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(ftprecv.din);
#endif /* TCPIPLIB */
  }
  ck_signal(SIGINT, ftprecv.oldintr);
  ftprecvret = -1;

  if (havesigint) {
    havesigint = 0;
    debug(F100, "FTP failftprecv2 chain to trap()...", "", 0);
    if (ftprecv.oldintr != SIG_IGN)
      (*ftprecv.oldintr)(SIGINT);
    /* NOTREACHED (I hope!) */
    debug(F100, "ftp failftprecv2 return from trap()...", "", 0);
  }
}

static void doftprecv2(void *threadinfo) {
  register int c, d;
  CK_OFF_T bytes = (CK_OFF_T)0;
  int bare_lfs = 0;
  int blksize = 0;
  ULONG start = 0L, stop;
  static char *rcvbuf = NULL;
  static int rcvbufsiz = 0;
#ifdef CK_URL
  char newname[CKMAXPATH + 1]; /* For file dialog */
#endif                         /* CK_URL */
  extern int adl_ask;

#ifdef FTP_TIMEOUT
  ftp_timed_out = 0;
#endif /* FTP_TIMEOUT */

  ftprecv.din = -1;
#ifdef CK_LOGIN
#ifdef IKSD
#endif /* IKSD */
#endif /* CK_LOGIN */

  if (ftprecv.recover) { /* Initiate recovery */
    x = ftpcmd("REST", ckfstoa(ftprecv.localsize), -1, -1, ftp_vbm);
    debug(F111, "ftp reply", "REST", x);
    if (x == REPLY_CONTINUE) {
      ftprecv.lmode = "ab";
      rs_len = ftprecv.localsize;
    } else {
      ftprecv.recover = 0;
    }
  }
  /* IMPORTANT: No FTP commands can come between REST and RETR! */

  debug(F111, "ftp recvrequest recover E", ftprecv.remote, ftprecv.recover);

  /* Send the command and get reply */
  debug(F110, "ftp recvrequest cmd", ftprecv.cmd, 0);
  debug(F110, "ftp recvrequest remote", ftprecv.remote, 0);

  if (ftpcmd(ftprecv.cmd, ftprecv.remote, ftprecv.fcs, ftprecv.rcs, ftp_vbm) !=
      REPLY_PRELIM) {
    ck_signal(SIGINT, ftprecv.oldintr); /* Bad reply, fail. */
    ftprecvret = -1;                 /* ftpcode is set by ftpcmd() */
    return;
  }
  ftprecv.din = dataconn("r"); /* Good reply, open data connection */
  globaldin = ftprecv.din;     /* Global copy of file descriptor */
  if (ftprecv.din == -1) {     /* Check for failure */
    ftpcode = -3;              /* Code for no data connection */
    ftprecvret = -1;
    return;
  }
#ifdef CK_URL
  /* In K95 GUI put up a file box */
  if (haveurl && g_url.pth && adl_ask) { /* Downloading from a URL */
    int x;
    char *preface = "\r\nIncoming file from FTP server...\r\n\
Please confirm output file specification or supply an alternative:";

    x = uq_file(preface, /* K95 GUI: Put up file box. */
                NULL, 4, NULL, ftprecv.local ? ftprecv.local : ftprecv.remote,
                newname, CKMAXPATH + 1);
    if (x > 0) {
      ftprecv.local = newname; /* Substitute user's file name */
      if (x == 2)              /* And append if user said to */
        ftprecv.lmode = "ab";
    }
  }
#endif                    /* CK_URL */
  x = 1;                  /* Output file open OK? */
  if (ftprecv.pipename) { /* Command */
    x = zxcmd(ZOFILE, ftprecv.pipename);
    debug(F111, "ftp recvrequest zxcmd", ftprecv.pipename, x);
  } else if (!out2screen) { /* File */
    struct filinfo xx;
    xx.bs = 0;
    xx.cs = 0;
    xx.rl = 0;
    xx.org = 0;
    xx.cc = 0;
    xx.typ = 0;
    xx.os_specific = "";
    xx.lblopts = 0;
    /* Append or New */
    xx.dsp = !strcmp(ftprecv.lmode, "ab") ? XYFZ_A : XYFZ_N;
    x = zopeno(ZOFILE, ftprecv.local, NULL, &xx);
    debug(F111, "ftp recvrequest zopeno", ftprecv.local, x);
  }
  if (x < 1) { /* Failure to open output file */
    if ((!dpyactive || ftp_deb))
      fprintf(stderr, "local(2): %s: %s\n", ftprecv.local, ck_errstr());
    ftprecvret = -1;
    return;
  }
  blksize = FTP_BUFSIZ; /* Allocate input buffer */

  debug(F101, "ftp recvrequest blksize", "", blksize);
  debug(F101, "ftp recvrequest rcvbufsiz", "", rcvbufsiz);

  if (rcvbufsiz < blksize) { /* if necessary */
    if (rcvbuf) {
      free(rcvbuf);
      rcvbuf = NULL;
    }
    rcvbuf = (char *)malloc((unsigned)blksize);
    if (!rcvbuf) {
      debug(F100, "ftp get rcvbuf malloc failed", "", 0);
      ftpcode = -2;
#ifdef ENOMEM
      errno = ENOMEM;
#endif /* ENOMEM */
      if ((!dpyactive || ftp_deb))
        perror("malloc");
      rcvbufsiz = 0;
      ftprecvret = -1;
      return;
    }
    debug(F101, "ftp get rcvbuf malloc ok", "", blksize);
    rcvbufsiz = blksize;
  }
  debug(F111, "ftp get rcvbufsiz", ftprecv.local, rcvbufsiz);

  ffc = (CK_OFF_T)0;  /* Character counter */
  cps = oldcps = 0L;  /* Thruput */
  start = gmstimer(); /* Start time (msecs) */
#ifdef GFTIMER
  rftimer(); /* Start time (float) */
#endif       /* GFTIMER */

  debug(F111, "ftp get type", ftprecv.local, curtype);
  debug(F101, "ftp recvrequest ftp_dpl", "", ftp_dpl);
  switch (curtype) {
  case FTT_BIN: /* Binary mode */
  case FTT_TEN: /* TENEX mode */
    d = 0;
    while (1) {
      errno = 0;
      c = secure_read(ftprecv.din, rcvbuf, rcvbufsiz);
      if (cancelfile) {
        failftprecv2(threadinfo);
        return;
      }
      if (c < 1)
        break;
#ifdef printf /* (What if it isn't?) */
      if (out2screen && !ftprecv.pipename) {
        int i;
        for (i = 0; i < c; i++)
          printf("%c", rcvbuf[i]);
      } else
#endif /* printf */
      {
        register int i;
        i = 0;
        errno = 0;
        while (i < c) {
          if (zmchout(rcvbuf[i++]) < 0) {
            d = i;
            break;
          }
        }
      }
      bytes += c;
      ffc += c;
    }
#ifdef FTP_TIMEOUT
    if (c == -3) {
      debug(F100, "ftp recvrequest timeout", "", 0);
      bytes = (CK_OFF_T)-1;
      ftp_timed_out = 1;
      ftpcode = -3;
    } else
#endif /* FTP_TIMEOUT */
      if (c < 0) {
        debug(F111, "ftp recvrequest errno", ckitoa(c), errno);
        if (c == -1 && errno != EPIPE)
          if ((!dpyactive || ftp_deb))
            perror("netin");
        bytes = (CK_OFF_T)-1;
        ftpcode = -4;
      }
    if (d < c) {
      ftpcode = -2;
      if ((!dpyactive || ftp_deb)) {
        char *p;
        p = ftprecv.local ? ftprecv.local : ftprecv.pipename;
        if (d < 0)
          fprintf(stderr, "local(3): %s: %s\n", ftprecv.local, ck_errstr());
        else
          fprintf(stderr, "%s: short write\n", ftprecv.local);
      }
    }
    break;

  case FTT_ASC: /* Text mode */
    debug(F101, "ftp recvrequest TYPE A xlate", "", ftprecv.xlate);
#ifndef NOCSETS
    if (ftprecv.xlate) {
      int (*fn)(char);
      debug(F110, "ftp recvrequest (data)", "initxlate", 0);
      initxlate(ftprecv.rcs, ftprecv.fcs); /* (From,To) */
      if (ftprecv.pipename) {
        fn = pipeout;
        debug(F110, "ftp recvrequest ASCII", "pipeout", 0);
      } else {
        fn = out2screen ? scrnout : putfil;
        debug(F110, "ftp recvrequest ASCII", out2screen ? "scrnout" : "putfil",
              0);
      }
      while (1) {
        /* Get byte from net */
        c0 = xgnbyte(FC_UCS2, ftprecv.rcs, netgetc);
        if (cancelfile) {
          failftprecv2(threadinfo);
          return;
        }
        if (c0 < 0)
          break;
        /* Second byte from net */
        c1 = xgnbyte(FC_UCS2, ftprecv.rcs, netgetc);
        if (cancelfile) {
          failftprecv2(threadinfo);
          return;
        }
        if (c1 < 0)
          break;

        {
          if ((x = xpnbyte(c0, TC_UCS2, ftprecv.fcs, fn)) < 0)
            break;
          if ((x = xpnbyte(c1, TC_UCS2, ftprecv.fcs, fn)) < 0)
            break;
        }
      }
    } else {
#endif /* NOCSETS */
      while (1) {
        c = secure_getc(ftprecv.din, 0);
        if (cancelfile
#ifdef FTP_TIMEOUT
            || ftp_timed_out
#endif /* FTP_TIMEOUT */
        ) {
          failftprecv2(threadinfo);
          return;
        }
        if (c < 0 || c == EOF)
          break;
#ifdef UNIX
        /* Record format conversion for Unix */
        /* SKIP THIS FOR WINDOWS! */
        if (c == '\n')
          bare_lfs++;
        while (c == '\r') {
          bytes++;
          if ((c = secure_getc(ftprecv.din, 0)) != '\n' || ftprecv.tcrflag) {
            if (cancelfile) {
              failftprecv2(threadinfo);
              return;
            }
            if (c < 0 || c == EOF)
              break;
            if (c == '\0') {
              bytes++;
              break;
            }
          }
        }
        if (c < 0 || c == EOF)
          break;
        if (c == '\0')
          continue;
#endif /* UNX */

        if (out2screen && !ftprecv.pipename)
#ifdef printf
          printf("%c", (char)c);
#else
        putchar((char)c);
#endif /* printf */
        else if ((d = zmchout(c)) < 0)
          break;
        bytes++;
        ffc++;
      }
      if (bare_lfs && (!dpyactive || ftp_deb)) {
        printf("WARNING! %d bare linefeeds received in ASCII mode\n", bare_lfs);
        printf("File might not have transferred correctly.\n");
      }
      if (ftprecv.din == -1) {
        bytes = (CK_OFF_T)-1;
      }
      if (c == -2)
        bytes = (CK_OFF_T)-1;
      break;
#ifndef NOCSETS
    }
#endif /* NOCSETS */
  }
  if (ftprecv.pipename || !out2screen) {
    zclose(ZOFILE); /* Close the file */
    debug(F111, "doftprecv2 zclose ftpcode", ftprecv.local, ftpcode);
    if (ftpcode < 0) { /* If download failed */
      int x = 0;
      switch (keep) {           /* which is... */
      case SET_AUTO:            /* AUTO */
        if (curtype == FTT_ASC) /* Delete file if TYPE A. */
          x = 1;
        break;
      case SET_OFF: /* DISCARD */
        x = 1;      /* Delete file, period. */
        break;
      default: /* KEEP */
        break;
      }
      if (x) {
        x = zdelet(ftprecv.local);
        debug(F111, "ftp get delete incomplete", ftprecv.local, x);
      }
    }
  }
  ck_signal(SIGINT, ftprecv.oldintr);
#ifdef SIGPIPE
  if (ftprecv.oldintp)
    ck_signal(SIGPIPE, ftprecv.oldintp);
#endif /* SIGPIPE */
  stop = gmstimer();
#ifdef GFTIMER
  fpfsecs = gftimer();
#endif /* GFTIMER */
  tfc += ffc;

#ifdef TCPIPLIB
  socket_close(ftprecv.din);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
  shutdown(ftprecv.din, 1 + 1);
#endif /* USE_SHUTDOWN */
  close(ftprecv.din);
#endif /* TCPIPLIB */
  ftprecv.reply = getreply(0, ftprecv.fcs, ftprecv.rcs, ftp_vbm, 0);
  ftprecvret = ((ftpcode < 0 || ftprecv.reply == REPLY_TRANSIENT ||
                 ftprecv.reply == REPLY_ERROR)
                    ? -1
                    : 0);
}

static int recvrequest(char *cmd, char *local, char *remote, char *lmode,
                       int printnames, int recover, char *pipename, int xlate,
                       int fcs, int rcs) {
  struct stat stbuf;

#ifdef DEBUG
  if (deblog) {
    debug(F111, "ftp recvrequest cmd", cmd, recover);
    debug(F110, "ftp recvrequest local ", local, 0);
    debug(F111, "ftp recvrequest remote", remote, ftp_typ);
    debug(F110, "ftp recvrequest pipename ", pipename, 0);
    debug(F101, "ftp recvrequest xlate", "", xlate);
    debug(F101, "ftp recvrequest fcs", "", fcs);
    debug(F101, "ftp recvrequest rcs", "", rcs);
  }
#endif /* DEBUG */

  ftprecv.localsize = (CK_OFF_T)0;

  if (remfile) { /* See remcfm(), remtxt() */
    if (rempipe) {
      pipename = remdest;
    } else {
      local = remdest;
      if (remappd)
        lmode = "ab";
    }
  }
  out2screen = 0;
  if (!cmd)
    cmd = ""; /* Core dump prevention */
  if (!remote)
    remote = "";
  if (!lmode)
    lmode = "";

  if (pipename) { /* No recovery for pipes. */
    recover = 0;
    if (!local)
      local = pipename;
  } else {
    if (!local) /* Output to screen? */
      local = "-";
    out2screen = !strcmp(local, "-");
  }
  debug(F101, "ftp recvrequest out2screen", "", out2screen);

  if (out2screen) /* No recovery to screen */
    recover = 0;
  if (!ftp_typ) /* No recovery in text mode */
    recover = 0;
  ftprecv.is_retr = (strcmp(cmd, "RETR") == 0);

  if (!ftprecv.is_retr) /* No recovery except for RETRieve */
    recover = 0;

  ftprecv.localsize = (CK_OFF_T)0; /* Local file size */
  rs_len = (CK_OFF_T)0;            /* Recovery point */

  debug(F101, "ftp recvrequest recover", "", recover);
  if (recover) {                   /* Recovering... */
    if (stat(local, &stbuf) < 0) { /* Can't stat local file */
      debug(F101, "ftp recvrequest recover stat failed", "", errno);
      recover = 0;                       /* So cancel recovery */
    } else {                             /* Have local file info */
      ftprecv.localsize = stbuf.st_size; /* Get size */
                                         /* Remote file smaller than local */
      if (fsize < ftprecv.localsize) {
        debug(F101, "ftp recvrequest recover remote smaller", "", fsize);
        recover = 0;                           /* Recovery can't work */
      } else if (fsize == ftprecv.localsize) { /* Sizes are equal */
        debug(F111, "ftp recvrequest recover equal size", remote,
              ftprecv.localsize);
        return (1);
      }
    }
    debug(F111, "ftp recvrequest recover", remote, recover);
  }

#ifdef FTP_PROXY
  if (proxy && ftprecv.is_retr)
    return (proxtrans(cmd, local ? local : remote, remote));
#endif /* FTP_PROXY */

  ftprecv.tcrflag = (feol != CK_CR) && ftprecv.is_retr;

  ftprecv.reply = 0;
  ftprecv.fcs = fcs;
  ftprecv.rcs = rcs;
  ftprecv.recover = recover;
  ftprecv.xlate = xlate;
  ftprecv.cmd = cmd;
  ftprecv.local = local;
  ftprecv.remote = remote;
  ftprecv.lmode = lmode;
  ftprecv.pipename = pipename;
  ftprecv.oldintp = NULL;
  ftpcode = 0;

  havesigint = 0;
  ftprecv.oldintr = ck_signal(SIGINT, cancelrecv);
  if (cc_execute(ckjaddr(recvcancel), doftprecv, failftprecv) < 0)
    return -1;

#ifdef FTP_TIMEOUT
  debug(F111, "ftp recvrequest ftprecvret", remote, ftprecvret);
  debug(F111, "ftp recvrequest ftp_timed_out", remote, ftp_timed_out);
  if (ftp_timed_out)
    ftprecvret = -1;
#endif /* FTP_TIMEOUT */

  if (ftprecvret < 0)
    return -1;

  if (cc_execute(ckjaddr(recvcancel), doftprecv2, failftprecv2) < 0)
    return -1;
  return ftprecvret;
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
static int initconn() {
  register char *p, *a;
  int result, tmpno = 0;
  int on = 1;
  GSOCKNAME_T len;

#ifndef NO_PASSIVE_MODE
  int a1, a2, a3, a4, p1, p2;

  if (passivemode) {
    data = socket(AF_INET, SOCK_STREAM, 0);
    globaldin = data;
    if (data < 0) {
      perror("ftp: socket");
      return (-1);
    }
    if (ftpcmd("PASV", NULL, 0, 0, ftp_vbm) != REPLY_COMPLETE) {
      printf("Passive mode refused\n");
      passivemode = 0;
      return (initconn());
    }
    /*
      Now we have a string of comma-separated one-byte unsigned integer values,
      The first four are the an IP address.  The fifth is the MSB of the port
      number, the sixth is the LSB.  From that we can make a sockaddr_in.
    */
    if (sscanf(pasv, "%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, &a4, &p1, &p2) != 6) {
      printf("Passive mode address scan failure\n");
      return (-1);
    };
#ifndef NOHTTP
    if (tcp_http_proxy) {
      char *agent = "C-Kermit";
      register struct hostent *hp = 0;
      struct servent *destsp;
      char host[512], *p, *q;
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
      int tos;
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */
#ifdef DEBUG
      extern int debtim;
      int xdebtim;
      xdebtim = debtim;
      debtim = 1;
#endif /* DEBUG */

      ckmakxmsg(proxyhost, HTTPCPYL, ckuitoa(a1), ".", ckuitoa(a2), ".",
                ckuitoa(a3), ".", ckuitoa(a4), ":", ckuitoa((p1 << 8) | p2),
                NULL, NULL, NULL);
      memset((char *)&hisctladdr, 0, sizeof(hisctladdr));
      for (p = tcp_http_proxy, q = host; *p != '\0' && *p != ':'; p++, q++)
        *q = *p;
      *q = '\0';

      hisctladdr.sin_addr.s_addr = inet_addr(host);
      if (hisctladdr.sin_addr.s_addr != INADDR_NONE) /* 2010-03-29 */
      {
        debug(F110, "initconn A", host, 0);
        hisctladdr.sin_family = AF_INET;
      } else {
        debug(F110, "initconn B", host, 0);
        hp = gethostbyname(host);
#ifdef HADDRLIST
        hp = ck_copyhostent(hp); /* make safe copy that won't change */
#endif                           /* HADDRLIST */
        if (hp == NULL) {
          fprintf(stderr, "ftp: %s: Unknown host\n", host);
          ftpcode = -1;
#ifdef DEBUG
          debtim = xdebtim;
#endif /* DEBUG */
          return (0);
        }
        hisctladdr.sin_family = hp->h_addrtype;
#ifdef HADDRLIST
        memcpy((char *)&hisctladdr.sin_addr, hp->h_addr_list[0],
               sizeof(hisctladdr.sin_addr));
#else  /* HADDRLIST */
        memcpy((char *)&hisctladdr.sin_addr, hp->h_addr,
               sizeof(hisctladdr.sin_addr));
#endif /* HADDRLIST */
      }
      data = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
      debug(F101, "initconn socket", "", data);
      if (data < 0) {
        perror("ftp: socket");
        ftpcode = -1;
#ifdef DEBUG
        debtim = xdebtim;
#endif /* DEBUG */
        return (0);
      }
      if (*p == ':')
        p++;
      else
        p = "http";

      destsp = getservbyname(p, "tcp");
      if (destsp)
        hisctladdr.sin_port = destsp->s_port;
      else if (p)
        hisctladdr.sin_port = htons(atoi(p));
      else
        hisctladdr.sin_port = htons(80);
      errno = 0;
#ifdef HADDRLIST
      debug(F100, "initconn HADDRLIST", "", 0);
      while
#else
      debug(F100, "initconn no HADDRLIST", "", 0);
      if
#endif /* HADDRLIST */
          (connect(data, (struct sockaddr *)&hisctladdr, sizeof(hisctladdr)) <
           0) {
        debug(F101, "initconn connect failed", "", errno);
#ifdef HADDRLIST
        if (hp && hp->h_addr_list[1]) {
          int oerrno = errno;

          fprintf(stderr, "ftp: connect to address %s: ",
                  inet_ntoa(hisctladdr.sin_addr));
          errno = oerrno;
          perror("ftphookup");
          hp->h_addr_list++;
          memcpy((char *)&hisctladdr.sin_addr, hp->h_addr_list[0],
                 sizeof(hisctladdr.sin_addr));
          fprintf(stdout, "Trying %s...\n", inet_ntoa(hisctladdr.sin_addr));
#ifdef TCPIPLIB
          socket_close(data);
#else  /* TCPIPLIB */
          close(data);
#endif /* TCPIPLIB */
          data = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
          if (data < 0) {
            perror("ftp: socket");
            ftpcode = -1;
#ifdef DEBUG
            debtim = xdebtim;
#endif /* DEBUG */
            return (0);
          }
          continue;
        }
#endif /* HADDRLIST */
        perror("ftp: connect");
        ftpcode = -1;
        goto bad;
      }
      if (http_connect(
              data, tcp_http_proxy_agent ? tcp_http_proxy_agent : agent, NULL,
              tcp_http_proxy_user, tcp_http_proxy_pwd, 0, proxyhost) < 0) {
#ifdef TCPIPLIB
        socket_close(data);
#else  /* TCPIPLIB */
        close(data);
#endif /* TCPIPLIB */
        perror("ftp: connect");
        ftpcode = -1;
        goto bad;
      }
    } else
#endif /* NOHTTP */
    {
      data_addr.sin_family = AF_INET;
      data_addr.sin_addr.s_addr =
          htonl((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);
      data_addr.sin_port = htons((p1 << 8) | p2);

      if (connect(data, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("ftp: connect");
        return (-1);
      }
    }
    debug(F100, "initconn connect ok", "", 0);
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
    on = IPTOS_THROUGHPUT;
    if (setsockopt(data, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
      perror("ftp: setsockopt TOS (ignored)");
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */
    memcpy(&hisdataaddr, &data_addr, sizeof(struct sockaddr_in));
    return (0);
  }
#endif /* NO_PASSIVE_MODE */

noport:
  memcpy(&data_addr, &myctladdr, sizeof(struct sockaddr_in));
  if (sendport)
    data_addr.sin_port = 0; /* let system pick one */
  if (data != -1) {
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
  }
  data = socket(AF_INET, SOCK_STREAM, 0);
  globaldin = data;
  if (data < 0) {
    perror("ftp: socket");
    if (tmpno)
      sendport = 1;
    return (-1);
  }
  if (!sendport) {
    if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) <
        0) {
      perror("ftp: setsockopt (reuse address)");
      goto bad;
    }
  }
  if (bind(data, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
    perror("ftp: bind");
    goto bad;
  }
  len = sizeof(data_addr);
  if (getsockname(data, (struct sockaddr *)&data_addr, &len) < 0) {
    perror("ftp: getsockname");
    goto bad;
  }
  if (listen(data, 1) < 0) {
    perror("ftp: listen");
    goto bad;
  }
  if (sendport) {
    a = (char *)&data_addr.sin_addr;
    p = (char *)&data_addr.sin_port;
    ckmakxmsg(ftpcmdbuf, FTP_BUFSIZ, "PORT ", UC(a[0]), ",", UC(a[1]), ",",
              UC(a[2]), ",", UC(a[3]), ",", UC(p[0]), ",", UC(p[1]));
    result = ftpcmd(ftpcmdbuf, NULL, 0, 0, ftp_vbm);
    if (result == REPLY_ERROR && sendport) {
      sendport = 0;
      tmpno = 1;
      goto noport;
    }
    return (result != REPLY_COMPLETE);
  }
  if (tmpno)
    sendport = 1;
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
  on = IPTOS_THROUGHPUT;
  if (setsockopt(data, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
    perror("ftp: setsockopt TOS (ignored)");
#endif
#endif
  return (0);
bad:
#ifdef TCPIPLIB
  socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
  shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
  close(data);
#endif /* TCPIPLIB */
  data = -1;
  globaldin = data;
  if (tmpno)
    sendport = 1;
  return (-1);
}

static int dataconn(char *lmode) {
  int s;
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
  int tos;
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */
  static SOCKOPT_T fromlen;

  fromlen = sizeof(hisdataaddr);

#ifndef NO_PASSIVE_MODE
  if (passivemode) {
    return (data);
  }
#endif /* NO_PASSIVE_MODE */

  s = accept(data, (struct sockaddr *)&hisdataaddr, &fromlen);
  if (s < 0) {
    perror("ftp: accept");
#ifdef TCPIPLIB
    socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
    shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
    close(data);
#endif /* TCPIPLIB */
    data = -1;
    globaldin = data;
    return (-1);
  }
#ifdef TCPIPLIB
  socket_close(data);
#else /* TCPIPLIB */
#ifdef USE_SHUTDOWN
  shutdown(data, 1 + 1);
#endif /* USE_SHUTDOWN */
  close(data);
#endif /* TCPIPLIB */
  data = s;
  globaldin = data;
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
  tos = IPTOS_THROUGHPUT;
  if (setsockopt(data, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
    perror("ftp: setsockopt TOS (ignored)");
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */

  return (data);
}

#ifdef FTP_PROXY
static void pscancel(sig) int sig;
{
  cancelfile++;
}

static void pswitch(flag) int flag;
{
  extern int proxy;
  static struct comvars {
    int connect;
    char name[MAXHOSTNAMELEN];
    struct sockaddr_in mctl;
    struct sockaddr_in hctl;
    FILE *in;
    FILE *out;
    int tpe;
    int curtpe;
    int cpnd;
    int sunqe;
    int runqe;
    int mcse;
    int ntflg;
    char nti[17];
    char nto[17];
    int mapflg;
    char mi[CKMAXPATH];
    char mo[CKMAXPATH];
    char *authtype;
    int clvl;
    int dlvl;
  } proxstruct, tmpstruct;
  struct comvars *ip, *op;

  cancelfile = 0;
  oldintr = ck_signal(SIGINT, pscancel);
  if (flag) {
    if (proxy)
      return;
    ip = &tmpstruct;
    op = &proxstruct;
    proxy++;
  } else {
    if (!proxy)
      return;
    ip = &proxstruct;
    op = &tmpstruct;
    proxy = 0;
  }
  ip->connect = connected;
  connected = op->connect;
  if (ftp_host) {
    strncpy(ip->name, ftp_host, MAXHOSTNAMELEN - 1);
    ip->name[MAXHOSTNAMELEN - 1] = '\0';
    ip->name[strlen(ip->name)] = '\0';
  } else
    ip->name[0] = 0;
  ftp_host = op->name;
  ip->hctl = hisctladdr;
  hisctladdr = op->hctl;
  ip->mctl = myctladdr;
  myctladdr = op->mctl;
  ip->in = csocket;
  csocket = op->in;
  ip->out = csocket;
  csocket = op->out;
  ip->tpe = ftp_typ;
  ftp_typ = op->tpe;
  ip->curtpe = curtype;
  curtype = op->curtpe;
  ip->cpnd = cpend;
  cpend = op->cpnd;
  ip->sunqe = ftp_usn;
  ftp_usn = op->sunqe;
  ip->mcse = mcase;
  mcase = op->mcse;
  ip->ntflg = ntflag;
  ntflag = op->ntflg;
  strncpy(ip->nti, ntin, 16);
  (ip->nti)[strlen(ip->nti)] = '\0';
  strcpy(ntin, op->nti);
  strncpy(ip->nto, ntout, 16);
  (ip->nto)[strlen(ip->nto)] = '\0';
  strcpy(ntout, op->nto);
  ip->mapflg = mapflag;
  mapflag = op->mapflg;
  strncpy(ip->mi, mapin, CKMAXPATH - 1);
  (ip->mi)[strlen(ip->mi)] = '\0';
  strcpy(mapin, op->mi);
  strncpy(ip->mo, mapout, CKMAXPATH - 1);
  (ip->mo)[strlen(ip->mo)] = '\0';
  strcpy(mapout, op->mo);
  ip->authtype = auth_type;
  auth_type = op->authtype;
  ip->clvl = ftp_cpl;
  ftp_cpl = op->clvl;
  ip->dlvl = ftp_dpl;
  ftp_dpl = op->dlvl;
  if (!ftp_cpl)
    ftp_cpl = FPL_CLR;
  if (!ftp_dpl)
    ftp_dpl = FPL_CLR;
  ck_signal(SIGINT, oldintr);
  if (cancelfile) {
    cancelfile = 0;
    debug(F101, "pswitch cancelfile B", "", cancelfile);
    (*oldintr)(SIGINT);
  }
}

static void cancelpt(int sig) {
  printf("\n");
  fflush(stdout);
  ptabflg++;
  cancelfile = 0;
  longjmp(ptcancel, 1);
}

void proxtrans(cmd, local, remote, unique) char *cmd, *local, *remote;
int unique;
{
  int secndflag = 0, prox_type, nfnd;
  char *cmd2;
#ifdef BSDSELECT
  fd_set mask;
#endif /* BSDSELECT */
  void cancelpt();

  if (strcmp(cmd, "RETR"))
    cmd2 = "RETR";
  else
    cmd2 = unique ? "STOU" : "STOR";
  if ((prox_type = type) == 0) {
    if (servertype == SYS_UNIX && unix_proxy)
      prox_type = FTT_BIN;
    else
      prox_type = FTT_ASC;
  }
  if (curtype != prox_type)
    changetype(prox_type, 1);
  if (ftpcmd("PASV", NULL, 0, 0, ftp_vbm) != REPLY_COMPLETE) {
    printf("Proxy server does not support third party transfers.\n");
    return;
  }
  pswitch(0);
  if (!connected) {
    printf("No primary connection\n");
    pswitch(1);
    ftpcode = -1;
    return;
  }
  if (curtype != prox_type)
    changetype(prox_type, 1);

  if (ftpcmd("PORT", pasv, -1, -1, ftp_vbm) != REPLY_COMPLETE) {
    pswitch(1);
    return;
  }

  /* Replace with calls to cc_execute() */
  if (setjmp(ptcancel))
    goto cancel;
  oldintr = ck_signal(SIGINT, cancelpt);
  if (ftpcmd(cmd, remote, -1, -1, ftp_vbm) != PRELIM) {
    ck_signal(SIGINT, oldintr);
    pswitch(1);
    return;
  }
  sleep(2000);
  pswitch(1);
  secndflag++;
  if (ftpcmd(cmd2, local, -1, -1, ftp_vbm) != PRELIM)
    goto cancel;
  ptflag++;
  getreply(0, -1, -1, ftp_vbm, 0);
  pswitch(0);
  getreply(0, -1, -1, ftp_vbm, 0);
  ck_signal(SIGINT, oldintr);
  pswitch(1);
  ptflag = 0;
  return;

cancel:
  signal(SIGINT, SIG_IGN);
  ptflag = 0;
  if (strcmp(cmd, "RETR") && !proxy)
    pswitch(1);
  else if (!strcmp(cmd, "RETR") && proxy)
    pswitch(0);
  if (!cpend && !secndflag) { /* only here if cmd = "STOR" (proxy=1) */
    if (ftpcmd(cmd2, local, -1, -1, ftp_vbm) != PRELIM) {
      pswitch(0);
      if (cpend)
        cancel_remote(0);
    }
    pswitch(1);
    if (ptabflg)
      ftpcode = -1;
    ck_signal(SIGINT, oldintr);
    return;
  }
  if (cpend)
    cancel_remote(0);
  pswitch(!proxy);
  if (!cpend && !secndflag) { /* only if cmd = "RETR" (proxy=1) */
    if (ftpcmd(cmd2, local, -1, -1, ftp_vbm) != PRELIM) {
      pswitch(0);
      if (cpend)
        cancel_remote(0);
      pswitch(1);
      if (ptabflg)
        ftpcode = -1;
      ck_signal(SIGINT, oldintr);
      return;
    }
  }
  if (cpend)
    cancel_remote(0);
  pswitch(!proxy);
  if (cpend) {
#ifdef BSDSELECT
    FD_ZERO(&mask);
    FD_SET(csocket, &mask);
    if ((nfnd = empty(&mask, 10)) <= 0) {
      if (nfnd < 0) {
        perror("cancel");
      }
      if (ptabflg)
        ftpcode = -1;
      lostpeer();
    }
#else /* BSDSELECT */
#ifdef IBMSELECT
    if ((nfnd = empty(&csocket, 1, 10)) <= 0) {
      if (nfnd < 0) {
        perror("cancel");
      }
      if (ptabflg)
        ftpcode = -1;
      lostpeer();
    }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
    getreply(0, -1, -1, ftp_vbm, 0);
    getreply(0, -1, -1, ftp_vbm, 0);
  }
  if (proxy)
    pswitch(0);
  pswitch(1);
  if (ptabflg)
    ftpcode = -1;
  ck_signal(SIGINT, oldintr);
}
#endif /* FTP_PROXY */

static void cancel_remote(int din) {
  CHAR buf[FTP_BUFSIZ];
  int x, nfnd;
#ifdef BSDSELECT
  fd_set mask;
#endif /* BSDSELECT */
#ifdef IBMSELECT
  int fds[2], fdcnt = 0;
#endif /* IBMSELECT */
#ifdef DEBUG
  extern int debtim;
  int xdebtim;
  xdebtim = debtim;
  debtim = 1;
#endif /* DEBUG */
  debug(F100, "ftp cancel_remote entry", "", 0);
  {
    /*
     * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
     * after urgent byte rather than before as is protocol now.
     */
    buf[0] = IAC;
    buf[1] = TN_IP;
    buf[2] = IAC;
    buf[3] = NUL;
    if ((x = send(csocket, (SENDARG2TYPE)buf, 3, MSG_OOB)) != 3)
      perror("cancel");
    debug(F101, "ftp cancel_remote send 1", "", x);
    buf[0] = TN_DM;
    x = send(csocket, (SENDARG2TYPE)buf, 1, 0);
    debug(F101, "ftp cancel_remote send 2", "", x);
  }
  x = scommand("ABOR");
  debug(F101, "ftp cancel_remote scommand", "", x);
#ifdef BSDSELECT
  FD_ZERO(&mask);
  FD_SET(csocket, &mask);
  if (din) {
    FD_SET(din, &mask);
  }
  nfnd = empty(&mask, 10);
  debug(F101, "ftp cancel_remote empty", "", nfnd);
  if ((nfnd) <= 0) {
    if (nfnd < 0) {
      perror("cancel");
    }
#ifdef FTP_PROXY
    if (ptabflg)
      ftpcode = -1;
#endif /* FTP_PROXY */
    lostpeer();
  }
  debug(F110, "ftp cancel_remote", "D", 0);
  if (din && FD_ISSET(din, &mask)) {
    /* Security: No threat associated with this read. */
    /* But you can't simply read the TLS data stream  */
    {
      while (recv(din, (SENDARG2TYPE)buf, FTP_BUFSIZ, 0) > 0)
        /* LOOP */;
    }
  }
  debug(F110, "ftp cancel_remote", "E", 0);
#else /* BSDSELECT */
#ifdef IBMSELECT
  fds[0] = csocket;
  fdcnt++;
  if (din) {
    fds[1] = din;
    fdcnt++;
  }
  nfnd = empty(fds, fdcnt, 10);
  debug(F101, "ftp cancel_remote empty", "", nfnd);
  if ((nfnd) <= 0) {
    if (nfnd < 0) {
      perror("cancel");
    }
#ifdef FTP_PROXY
    if (ptabflg)
      ftpcode = -1;
#endif /* FTP_PROXY */
    lostpeer();
  }
  debug(F110, "ftp cancel_remote", "D", 0);
  if (din && select(&din, 1, 0, 0, 1)) {
    {
      while (recv(din, (SENDARG2TYPE)buf, FTP_BUFSIZ, 0) > 0)
        /* LOOP */;
    }
  }
  debug(F110, "ftp cancel_remote", "E", 0);
#else  /* IBMSELECT */
  Some form of select is required.
#endif /* IBMSELECT */
#endif /* BSDSELECT */
  if (getreply(0, -1, -1, ftp_vbm, 0) == REPLY_ERROR && ftpcode == 552) {
    debug(F110, "ftp cancel_remote", "F", 0);
    /* 552 needed for NIC style cancel */
    getreply(0, -1, -1, ftp_vbm, 0);
    debug(F110, "ftp cancel_remote", "G", 0);
  }
  debug(F110, "ftp cancel_remote", "H", 0);
  getreply(0, -1, -1, ftp_vbm, 0);
  debug(F110, "ftp cancel_remote", "I", 0);
#ifdef DEBUG
  debtim = xdebtim;
#endif /* DEBUG */
}

#ifndef NOMHHOST
#endif /* NOMHHOST */

#ifdef INADDRX
static struct in_addr inaddrx;
#endif /* INADDRX */

static char *ftp_hookup(char *host, int port, int tls) {
  register struct hostent *hp = 0;
#ifdef IP_TOS
#ifdef IPTOS_THROUGHPUT
  int tos;
#endif /* IPTOS_THROUGHPUT */
#endif /* IP_TOS */
  int s;
  GSOCKNAME_T len;
  static char hostnamebuf[MAXHOSTNAMELEN];
  char hostname[MAXHOSTNAMELEN] /* , *p, *q */;
  int cport;
#ifdef DEBUG
  extern int debtim;
  int xdebtim;
  xdebtim = debtim;
  debtim = 1;
#endif /* DEBUG */

  debug(F111, "ftp_hookup", host, port);

#ifndef NOHTTP
  if (tcp_http_proxy) {
    struct servent *destsp;
    char *p, *q;

    ckmakmsg(proxyhost, HTTPCPYL, host, ":", ckuitoa(port), NULL);
    for (p = tcp_http_proxy, q = hostname; *p != '\0' && *p != ':'; p++, q++)
      *q = *p;
    *q = '\0';

    if (*p == ':')
      p++;
    else
      p = "http";

    destsp = getservbyname(p, "tcp");
    if (destsp)
      cport = ntohs(destsp->s_port);
    else if (p) {
      cport = atoi(p);
    } else
      cport = 80;
  } else
#endif /* NOHTTP */
  {
    ckstrncpy(hostname, host, MAXHOSTNAMELEN);
    cport = port;
  }
  memset((char *)&hisctladdr, 0, sizeof(hisctladdr));
  hisctladdr.sin_addr.s_addr = inet_addr(host);
  if (hisctladdr.sin_addr.s_addr != INADDR_NONE) /* 2010-03-29 */
  {
    debug(F110, "ftp hookup A", hostname, 0);
    hisctladdr.sin_family = AF_INET;
    ckstrncpy(hostnamebuf, hostname, MAXHOSTNAMELEN);
  } else {
    debug(F110, "ftp hookup B", hostname, 0);
    hp = gethostbyname(hostname);
#ifdef HADDRLIST
    hp = ck_copyhostent(hp); /* make safe copy that won't change */
#endif                       /* HADDRLIST */
    if (hp == NULL) {
      fprintf(stderr, "ftp: %s: Unknown host\n", host);
      ftpcode = -1;
#ifdef DEBUG
      debtim = xdebtim;
#endif /* DEBUG */
      return ((char *)0);
    }
    hisctladdr.sin_family = hp->h_addrtype;
#ifdef HADDRLIST
    memcpy((char *)&hisctladdr.sin_addr, hp->h_addr_list[0],
           sizeof(hisctladdr.sin_addr));
#else  /* HADDRLIST */
    memcpy((char *)&hisctladdr.sin_addr, hp->h_addr,
           sizeof(hisctladdr.sin_addr));
#endif /* HADDRLIST */
    ckstrncpy(hostnamebuf, hp->h_name, MAXHOSTNAMELEN);
  }
  debug(F110, "ftp hookup C", hostnamebuf, 0);
  ftp_host = hostnamebuf;
  s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
  debug(F101, "ftp hookup socket", "", s);
  if (s < 0) {
    perror("ftp: socket");
    ftpcode = -1;
#ifdef DEBUG
    debtim = xdebtim;
#endif /* DEBUG */
    return (0);
  }
  hisctladdr.sin_port = htons(cport);
  errno = 0;

#ifdef HADDRLIST
  debug(F100, "ftp hookup HADDRLIST", "", 0);
  while
#else
  debug(F100, "ftp hookup no HADDRLIST", "", 0);
  if
#endif /* HADDRLIST */
      (connect(s, (struct sockaddr *)&hisctladdr, sizeof(hisctladdr)) < 0) {
    debug(F101, "ftp hookup connect failed", "", errno);
#ifdef HADDRLIST
    if (hp && hp->h_addr_list[1]) {
      int oerrno = errno;

      fprintf(stderr,
              "ftp: connect to address %s: ", inet_ntoa(hisctladdr.sin_addr));
      errno = oerrno;
      perror((char *)0);
      hp->h_addr_list++;
      memcpy((char *)&hisctladdr.sin_addr, hp->h_addr_list[0],
             sizeof(hisctladdr.sin_addr));
      fprintf(stdout, "Trying %s...\n", inet_ntoa(hisctladdr.sin_addr));
#ifdef TCPIPLIB
      socket_close(s);
#else  /* TCPIPLIB */
      close(s);
#endif /* TCPIPLIB */
      s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
      if (s < 0) {
        perror("ftp: socket");
        ftpcode = -1;
#ifdef DEBUG
        debtim = xdebtim;
#endif /* DEBUG */
        return (0);
      }
      continue;
    }
#endif /* HADDRLIST */
    perror("ftp: connect");
    ftpcode = -1;
    goto bad;
  }
  debug(F100, "ftp hookup connect ok", "", 0);

  len = sizeof(myctladdr);
  errno = 0;
  if (getsockname(s, (struct sockaddr *)&myctladdr, &len) < 0) {
    debug(F101, "ftp hookup getsockname failed", "", errno);
    perror("ftp: getsockname");
    ftpcode = -1;
    goto bad;
  }
  debug(F100, "ftp hookup getsockname ok", "", 0);

#ifndef NOHTTP
  if (tcp_http_proxy) {
    char *agent = "C-Kermit";

    if (http_connect(s, agent, NULL, tcp_http_proxy_user, tcp_http_proxy_pwd, 0,
                     proxyhost) < 0) {
      char *foo = NULL;
#ifdef TCPIPLIB
      socket_close(s);
#else  /* TCPIPLIB */
      close(s);
#endif /* TCPIPLIB */

      while (foo == NULL && tcp_http_proxy != NULL) {

        if (tcp_http_proxy_errno == 401 || tcp_http_proxy_errno == 407) {
          char uid[UIDBUFLEN];
          char pwd[PWDSIZ];
          struct txtbox tb[2];
          int ok;

          tb[0].t_buf = uid;
          tb[0].t_len = UIDBUFLEN;
          tb[0].t_lbl = "Proxy Userid: ";
          tb[0].t_dflt = NULL;
          tb[0].t_echo = 1;
          tb[1].t_buf = pwd;
          tb[1].t_len = 256;
          tb[1].t_lbl = "Proxy Passphrase: ";
          tb[1].t_dflt = NULL;
          tb[1].t_echo = 2;

          ok = uq_mtxt("Proxy Server Authentication Required\n", NULL, 2, tb);

          if (ok && uid[0]) {
            char *proxy_user, *proxy_pwd;

            proxy_user = tcp_http_proxy_user;
            proxy_pwd = tcp_http_proxy_pwd;

            tcp_http_proxy_user = uid;
            tcp_http_proxy_pwd = pwd;

            foo = ftp_hookup(host, port, 0);

            debug(F110, "ftp_hookup()", foo, 0);
            memset(pwd, 0, PWDSIZ);
            tcp_http_proxy_user = proxy_user;
            tcp_http_proxy_pwd = proxy_pwd;
          } else
            break;
        } else
          break;
      }
      if (foo != NULL)
        return (foo);
      perror("ftp: connect");
      ftpcode = -1;
      goto bad;
    }
    ckstrncpy(hostnamebuf, proxyhost, MAXHOSTNAMELEN);
  }
#endif /* NOHTTP */

  csocket = s;

#ifdef IP_TOS
#ifdef IPTOS_LOWDELAY
  tos = IPTOS_LOWDELAY;
  if (setsockopt(csocket, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
    perror("ftp: setsockopt TOS (ignored)");
#endif
#endif
  if (!quiet)
    printf("Connected to %s.\n", host);

  /* Read greeting from server */
  if (getreply(0, ftp_csl, ftp_csr, ftp_vbm, 0) > 2) {
    debug(F100, "ftp hookup bad reply", "", 0);
#ifdef TCPIPLIB
    socket_close(csocket);
#else  /* TCPIPLIB */
    close(csocket);
#endif /* TCPIPLIB */
    ftpcode = -1;
    goto bad;
  }
#ifdef SO_OOBINLINE
  {
    int on = 1;
    errno = 0;
    if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0) {
      perror("ftp: setsockopt");
      debug(F101, "ftp hookup setsockopt failed", "", errno);
    }
#ifdef DEBUG
    else
      debug(F100, "ftp hookup setsockopt ok", "", 0);
#endif /* DEBUG */
  }
#endif /* SO_OOBINLINE */

#ifdef DEBUG
  debtim = xdebtim;
#endif /* DEBUG */
  return (ftp_host);

bad:
  debug(F100, "ftp hookup bad", "", 0);
#ifdef TCPIPLIB
  socket_close(s);
#else  /* TCPIPLIB */
  close(s);
#endif /* TCPIPLIB */
#ifdef DEBUG
  debtim = xdebtim;
#endif /* DEBUG */
  csocket = -1;
  return ((char *)0);
}

static void ftp_init() {
  int i, n;

  /* The purpose of the initial REST 0 is not clear, but other FTP */
  /* clients do it.  In any case, failure of this command is not a */
  /* reliable indication that the server does not support Restart. */

  okrestart = 0;
  if (!noinit) {
    n = ftpcmd("REST 0", NULL, 0, 0, 0);
    if (n == REPLY_COMPLETE)
      okrestart = 1;
  }
  n = ftpcmd("SYST", NULL, 0, 0, 0); /* Get server system type */
  if (n == REPLY_COMPLETE) {
    register char *cp, c = NUL;
    cp = ckstrchr(ftp_reply_str + 4, ' '); /* Get first word of reply */
    if (cp == NULL)
      cp = ckstrchr(ftp_reply_str + 4, '\r');
    if (cp) {
      if (cp[-1] == '.')
        cp--;
      c = *cp;    /* Save this char */
      *cp = '\0'; /* Replace it with NUL */
    }
    if (!quiet)
      printf("Remote system type is %s.\n", ftp_reply_str + 4);
    ckstrncpy(ftp_srvtyp, ftp_reply_str + 4, SRVNAMLEN);
    if (cp) /* Put back saved char */
      *cp = c;
  }
  alike = !ckstrcmp(ftp_srvtyp, myostype, -1, 0);

  if (!ckstrcmp(ftp_srvtyp, "UNIX", -1, 0))
    servertype = SYS_UNIX;
  else if (!ckstrcmp(ftp_srvtyp, "WIN32", -1, 0))
    servertype = SYS_WIN32;
  else if (!ckstrcmp(ftp_srvtyp, "OS/2", -1, 0))
    servertype = SYS_WIN32;
  else if (!ckstrcmp(ftp_srvtyp, "VMS", -1, 0))
    servertype = SYS_VMS;
  else if (!ckstrcmp(ftp_srvtyp, "DOS", -1, 0))
    servertype = SYS_DOS;
  else if (!ckstrcmp(ftp_srvtyp, "TOPS20", -1, 0))
    servertype = SYS_TOPS20;
  else if (!ckstrcmp(ftp_srvtyp, "TOPS10", -1, 0))
    servertype = SYS_TOPS10;

#ifdef FTP_PROXY
  unix_proxy = 0;
  if (servertype == SYS_UNIX && proxy)
    unix_proxy = 1;
#endif /* FTP_PROXY */

  if (ftp_cmdlin && ftp_xfermode == XMODE_M)
    ftp_typ = binary; /* Type given on command line */
  else                /* Otherwise set it automatically */
    ftp_typ = alike ? FTT_BIN : FTT_ASC;
  changetype(ftp_typ, 0); /* Change to this type */
  g_ftp_typ = ftp_typ;    /* Make it the global type */
  if (!quiet)
    printf("Default transfer mode is %s\n",
           ftp_typ ? "BINARY" : "TEXT (\"ASCII\")");
  for (i = 0; i < 16; i++) /* Init server FEATure table */
    sfttab[i] = 0;
  if (!noinit) {
    n = ftpcmd("MODE S", NULL, 0, 0, 0); /* We always send in Stream mode */
    n = ftpcmd("STRU F", NULL, 0, 0, 0); /* STRU File (not Record or Page) */
    if (featok) {
      n = ftpcmd("FEAT", NULL, 0, 0, 0); /* Ask server about features */
      if (n == REPLY_COMPLETE) {
        debug(F101, "ftp_init FEAT", "", sfttab[0]);
        if (deblog || ftp_deb) {
          int i;
          for (i = 1; i < 16 && i < nfeattab; i++) {
            debug(F111, "ftp_init FEAT", feattab[i].kwd, sfttab[i]);
            if (ftp_deb)
              printf("  Server %s %s\n",
                     sfttab[i] ? "supports" : "does not support",
                     feattab[i].kwd);
          }
          /* Deal with disabled MLST opts here if necessary */
          /* But why would it be? */
        }
      }
    }
  }
}

static int ftp_login(char *host) /* (also called from ckuusy.c) */
{
  static char ftppass[PASSBUFSIZ] = "";
  char tmp[PASSBUFSIZ];
  char *user = NULL, *pass = NULL, *acct = NULL;
  int n, aflag = 0;
  extern char uidbuf[];
  extern char pwbuf[];
  extern int pwflg, pwcrypt;

  debug(F111, "ftp_login", ftp_logname, ftp_log);

  if (!ckstrcmp(ftp_logname, "anonymous", -1, 0))
    anonymous = 1;
  if (!ckstrcmp(ftp_logname, "ftp", -1, 0))
    anonymous = 1;

  if (anonymous) {
    user = "anonymous";
    if (ftp_tmp) { /* They gave a password */
      pass = ftp_tmp;
    } else if (ftp_apw) { /* SET FTP ANONYMOUS-PASSWORD */
      pass = ftp_apw;
    } else { /* Supply user@host */
      ckmakmsg(tmp, PASSBUFSIZ, whoami(), "@", myhost, NULL);
      pass = tmp;
    }
    debug(F110, "ftp anonymous", pass, 0);
  } else {
#ifdef USE_RUSERPASS
    if (ruserpass(host, &user, &pass, &acct) < 0) {
      ftpcode = -1;
      return (0);
    }
#endif /* USE_RUSERPASS */
    if (ftp_logname) {
      user = ftp_logname;
      pass = ftp_tmp;
    } else if (uidbuf[0] && (ftp_tmp || pwbuf[0] && pwflg)) {
      user = uidbuf;
      if (ftp_tmp) {
        pass = ftp_tmp;
      } else if (pwbuf[0] && pwflg) {
        ckstrncpy(ftppass, pwbuf, PASSBUFSIZ);
        pass = ftppass;
      }
    }
    acct = ftp_acc;
    while (user == NULL) {
      char *myname, prompt[PROMPTSIZ];
      int ok;

      myname = whoami();
      if (myname)
        ckmakxmsg(prompt, PROMPTSIZ, " Name (", host, ":", myname, "): ", NULL,
                  NULL, NULL, NULL, NULL, NULL, NULL);
      else
        ckmakmsg(prompt, PROMPTSIZ, " Name (", host, "): ", NULL);
      tmp[0] = '\0';

      ok = uq_txt(NULL, prompt, 1, NULL, tmp, PASSBUFSIZ, NULL,
                  DEFAULT_UQ_TIMEOUT);
      if (!ok || *tmp == '\0')
        user = myname;
      else
        user = brstrip(tmp);
    }
  }
  n = ftpcmd("USER", user, -1, -1, ftp_vbm);
  if (n == REPLY_COMPLETE) {
    /* determine if we need to send a dummy password */
    if (ftpcmd("PWD", NULL, 0, 0, 0) != REPLY_COMPLETE)
      ftpcmd("PASS dummy", NULL, 0, 0, 1);
  } else if (n == REPLY_CONTINUE) {

    if (pass == NULL) {
      int ok;
      setint();
      ok = uq_txt(NULL, " Password: ", 2, NULL, ftppass, PASSBUFSIZ, NULL,
                  DEFAULT_UQ_TIMEOUT);
      if (ok)
        pass = brstrip(ftppass);
    }

    n = ftpcmd("PASS", pass, -1, -1, 1);
    if (!anonymous && pass) {
      char *p = pass;
      while (*p++)
        *(p - 1) = NUL;
      makestr(&ftp_tmp, NULL);
    }
  }
  if (n == REPLY_CONTINUE) {
    aflag++;
    if (acct == NULL) {
      static char ftpacct[80];
      int ok;
      setint();
      ok = uq_txt(NULL, " Account: ", 2, NULL, ftpacct, 80, NULL,
                  DEFAULT_UQ_TIMEOUT);
      if (ok)
        acct = brstrip(ftpacct);
    }
    n = ftpcmd("ACCT", acct, -1, -1, ftp_vbm);
  }
  if (n != REPLY_COMPLETE) {
    fprintf(stderr, "FTP login failed.\n");
    if (haveurl)
      doexit(BAD_EXIT, -1);
    return (0);
  }
  if (!aflag && acct != NULL) {
    ftpcmd("ACCT", acct, -1, -1, ftp_vbm);
  }
  makestr(&ftp_logname, user);
  loggedin = 1;
#ifdef LOCUS
  /* Unprefixed file management commands go to server */
  if (autolocus && !ftp_cmdlin) {
    setlocus(0, 1);
  }
#endif /* LOCUS */
  ftp_init();

  if (anonymous && !quiet) {
    printf(" Logged in as anonymous (%s)\n", pass);
    memset(pass, 0, strlen(pass));
  }
  if (ftp_rdir) {
    if (doftpcwd(ftp_rdir, -1) < 1)
      doexit(BAD_EXIT, -1);
  }

#ifdef FTP_PROXY
  if (proxy)
    return (1);
#endif /* FTP_PROXY */
  return (1);
}

static int ftp_reset() {
  int rc;
#ifdef BSDSELECT
  int nfnd = 1;
  fd_set mask;
  FD_ZERO(&mask);
  while (nfnd > 0) {
    FD_SET(csocket, &mask);
    if ((nfnd = empty(&mask, 0)) < 0) {
      perror("reset");
      ftpcode = -1;
      lostpeer();
      return (0);
    } else if (nfnd) {
      getreply(0, -1, -1, ftp_vbm, 0);
    }
  }
#else /* BSDSELECT */
#ifdef IBMSELECT
  int nfnd = 1;
  while (nfnd > 0) {
    if ((nfnd = empty(&csocket, 1, 0)) < 0) {
      perror("reset");
      ftpcode = -1;
      lostpeer();
      return (0);
    } else if (nfnd) {
      getreply(0, -1, -1, ftp_vbm, 0);
    }
  }
#endif /* IBMSELECT */
#endif /* BSDSELECT */
  rc = (ftpcmd("REIN", NULL, 0, 0, ftp_vbm) == REPLY_COMPLETE);
  if (rc > 0)
    loggedin = 0;
  return (rc);
}

static int ftp_rename(char *from, char *to) {
  int lcs = -1, rcs = -1;
#ifndef NOCSETS
  if (ftp_xla) {
    lcs = ftp_csl;
    if (lcs < 0)
      lcs = fcharset;
    rcs = ftp_csx;
    if (rcs < 0)
      rcs = ftp_csr;
  }
#endif /* NOCSETS */
  if (ftpcmd("RNFR", from, lcs, rcs, ftp_vbm) == REPLY_CONTINUE) {
    return (ftpcmd("RNTO", to, lcs, rcs, ftp_vbm) == REPLY_COMPLETE);
  }
  return (0); /* Failure */
}

static int ftp_umask(char *mask) {
  int rc;
  rc = (ftpcmd("SITE UMASK", mask, -1, -1, 1) == REPLY_COMPLETE);
  return (rc);
}

static int ftp_user(char *user, char *pass, char *acct) {
  int n = 0, aflag = 0;
  char pwd[PWDSIZ];

  if (!auth_type && ftp_aut) {
  }
  n = ftpcmd("USER", user, -1, -1, ftp_vbm);
  if (n == REPLY_COMPLETE)
    n = ftpcmd("PASS dummy", NULL, 0, 0, 1);
  else if (n == REPLY_CONTINUE) {
    if (pass == NULL || !pass[0]) {
      int ok;
      pwd[0] = '\0';
      setint();
      ok = uq_txt(NULL, " Password: ", 2, NULL, pwd, PWDSIZ, NULL,
                  DEFAULT_UQ_TIMEOUT);
      if (ok)
        pass = brstrip(pwd);
    }

    n = ftpcmd("PASS", pass, -1, -1, 1);
    memset(pass, 0, strlen(pass));
  }
  if (n == REPLY_CONTINUE) {
    if (acct == NULL || !acct[0]) {
      int ok;
      pwd[0] = '\0';
      setint();
      ok = uq_txt(NULL, " Account: ", 2, NULL, pwd, PWDSIZ, NULL,
                  DEFAULT_UQ_TIMEOUT);
      if (ok)
        acct = pwd;
    }
    n = ftpcmd("ACCT", acct, -1, -1, ftp_vbm);
    aflag++;
  }
  if (n != REPLY_COMPLETE) {
    printf("Login failed.\n");
    return (0);
  }
  if (!aflag && acct != NULL && acct[0]) {
    n = ftpcmd("ACCT", acct, -1, -1, ftp_vbm);
  }
  if (n == REPLY_COMPLETE) {
    makestr(&ftp_logname, user);
    loggedin = 1;
    ftp_init();
    return (1);
  }
  return (0);
}

char *ftp_authtype() {
  if (!connected)
    return ("NULL");
  return (auth_type ? auth_type : "NULL");
}

char *ftp_cpl_mode() {
  switch (ftp_cpl) {
  case FPL_CLR:
    return ("clear");
  case FPL_SAF:
    return ("safe");
  case FPL_PRV:
    return ("private");
  case FPL_CON:
    return ("confidential");
  default:
    return ("(error)");
  }
}

char *ftp_dpl_mode() {
  switch (ftp_dpl) {
  case FPL_CLR:
    return ("clear");
  case FPL_SAF:
    return ("safe");
  case FPL_PRV:
    return ("private");
  case FPL_CON:
    return ("confidential");
  default:
    return ("(error)");
  }
}

/* remote_files() */
/*
   Returns next remote filename on success;
   NULL on error or no more files with global rfrc set to:
     -1: Bad argument
     -2: Server error response to NLST, e.g. file not found
     -3: No more files
     -9: Internal error
*/
#define FTPNAMBUFLEN CKMAXPATH + 1024

/* Check: ckmaxfiles CKMAXOPEN */

#define MLSDEPTH 128     /* Stack of open temp files */
static int mlsdepth = 0; /* Temp file stack depth */
static FILE *tmpfilptr[MLSDEPTH + 1] = {NULL, NULL}; /* Temp file pointers */
static char *tmpfilnam[MLSDEPTH + 1] = {NULL, NULL}; /* Temp file names */

static void mlsreset() { /* Reset MGET temp-file stack */
  int i;
  for (i = 0; i <= mlsdepth; i++) {
    if (tmpfilptr[i]) {
      fclose(tmpfilptr[i]);
      tmpfilptr[i] = NULL;
      if (tmpfilnam[i]) {
        free(tmpfilnam[i]);
      }
    }
  }
  mlsdepth = 0;
}

static CHAR *remote_files(int new_query, CHAR *arg, CHAR *pattern,
                          int proxy_switch)
/* remote_files */ {
  static CHAR buf[FTPNAMBUFLEN];
  CHAR *cp, *whicharg;
  char *cdto = NULL;
  char *p;
  int x, forced = 0;
  int lcs = 0, rcs = 0, xlate = 0;

  debug(F101, "ftp remote_files new_query", "", new_query);
  debug(F110, "ftp remote_files arg", arg, 0);
  debug(F110, "ftp remote_files pattern", pattern, 0);

  rfrc = -1;
  if (pattern) /* Treat empty pattern same as NULL */
    if (!*pattern)
      pattern = NULL;
  if (arg) /* Ditto for arg */
    if (!*arg)
      arg = NULL;

again:

  if (new_query) {
    if (tmpfilptr[mlsdepth]) {
      fclose(tmpfilptr[mlsdepth]);
      tmpfilptr[mlsdepth] = NULL;
    }
  }
  if (tmpfilptr[mlsdepth] == NULL) {
    extern char *tempdir;
    char *p;
    debug(F110, "ftp remote_files tempdir", tempdir, 0);
    if (tempdir) {
      p = tempdir;
    } else {
      p = getenv("CK_TMP");
      if (!p)
        p = getenv("TMPDIR");
      if (!p)
        p = getenv("TEMP");
      if (!p)
        p = getenv("TMP");
      if (p) {
        int len = strlen(p);
        if (p[len - 1] != '/') {
          static char foo[CKMAXPATH];
          ckstrncpy(foo, p, CKMAXPATH);
          ckstrncat(foo, "/", CKMAXPATH);
          p = foo;
        }
      } else
#ifdef UNIX          /* Systems that have a standard */
        p = "/tmp/"; /* temporary directory... */
#else
        p = "";
#endif /* UNIX */
    }
    debug(F110, "ftp remote_files p", p, 0);

    /* Get temp file */

    if ((tmpfilnam[mlsdepth] = (char *)malloc(CKMAXPATH + 1))) {
      ckmakmsg((char *)tmpfilnam[mlsdepth], CKMAXPATH + 1, p, "ckXXXXXX", NULL,
               NULL);
    } else {
      printf("?Malloc failure: remote_files()\n");
      return (NULL);
    }

#ifdef MKTEMP
#ifdef MKSTEMP
    x = mkstemp((char *)tmpfilnam[mlsdepth]);
    if (x > -1)
      close(x); /* We just want the name. */
#else
    mktemp((char *)tmpfilnam[mlsdepth]);
#endif /* MKSTEMP */
       /* if no mktmpnam() the name will just be "ckXXXXXX"... */
#endif /* MKTEMP */

    debug(F111, "ftp remote_files tmpfilnam[mlsdepth]", tmpfilnam[mlsdepth],
          mlsdepth);

#ifdef FTP_PROXY
    if (proxy_switch) {
      pswitch(!proxy);
    }
#endif /* FTP_PROXY */

    debug(F101, "ftp remote_files ftp_xla", "", ftp_xla);
    debug(F101, "ftp remote_files ftp_csl", "", ftp_csl);
    debug(F101, "ftp remote_files ftp_csr", "", ftp_csr);

#ifndef NOCSETS
    xlate = ftp_xla; /* SET FTP CHARACTER-SET-TRANSLATION */
    if (xlate) {     /* ON? */
      lcs = ftp_csl; /* Local charset */
      if (lcs < 0)
        lcs = fcharset;
      if (lcs < 0)
        xlate = 0;
    }
    if (xlate) {     /* Still ON? */
      rcs = ftp_csx; /* Remote (Server) charset */
      if (rcs < 0)
        rcs = ftp_csr;
      if (rcs < 0)
        xlate = 0;
    }
#endif /* NOCSETS */

    forced = mgetforced;                             /* MGET method forced? */
    if (!forced || !mgetmethod)                      /* Not forced... */
      mgetmethod = (sfttab[0] && sfttab[SFT_MLST]) ? /* so pick one */
                       SND_MLS
                                                   : SND_NLS;
    /*
      User's Command:                 Result:
        mget /nlst                     NLST (NULL)
        mget /nlst foo                 NLST foo
        mget /nlst *.txt               NLST *.txt
        mget /nlst /match:*.txt        NLST (NULL)
        mget /nlst /match:*.txt  foo   NLST foo
        mget /mlsd                     MLSD (NULL)
        mget /mlsd foo                 MLSD foo
        mget /mlsd *.txt               MLSD (NULL)
        mget /mlsd /match:*.txt        MLSD (NULL)
        mget /mlsd /match:*.txt  foo   MLSD foo
    */
    x = -1;
    while (x < 0) {
      if (pattern) { /* Don't simplify this! */
        whicharg = arg;
      } else if (mgetmethod == SND_MLS) {
        if (arg)
          whicharg = iswild((char *)arg) ? NULL : arg;
        else
          whicharg = NULL;
      } else {
        whicharg = arg;
      }
      debug(F110, "ftp remote_files mgetmethod",
            mgetmethod == SND_MLS ? "MLSD" : "NLST", 0);
      debug(F110, "ftp remote_files whicharg", whicharg, 0);

      x = recvrequest((mgetmethod == SND_MLS) ? "MLSD" : "NLST",
                      (char *)tmpfilnam[mlsdepth], (char *)whicharg, "wb", 0, 0,
                      NULL, xlate, lcs, rcs);
      if (x < 0) { /* Chosen method wasn't accepted */
        if (forced) {
          if (ftpcode > 500 && ftpcode < 505 && !quiet)
            printf("?%s: Not supported by server\n",
                   mgetmethod == SND_MLS ? "MLSD" : "NLST");
          rfrc = -2; /* Fail */
          return (NULL);
        }
        /* Not forced - if MLSD failed, try NLST */
        if (mgetmethod == SND_MLS) { /* Server lied about MLST */
          sfttab[SFT_MLST] = 0;      /* So disable it */
          mlstok = 0;                /* and */
          mgetmethod = SND_NLS;      /* try NLST */
          continue;
        }
        rfrc = -2;
        return (NULL);
      }
    }
#ifdef FTP_PROXY
    if (proxy_switch) {
      pswitch(!proxy);
    }
#endif /* FTP_PROXY */
    tmpfilptr[mlsdepth] = fopen((char *)tmpfilnam[mlsdepth], "r");
    if (tmpfilptr[mlsdepth]) {
      if (!ftp_deb && !deblog)
        unlink(tmpfilnam[mlsdepth]);
    }
    /*notemp:*/
    if (!tmpfilptr[mlsdepth]) {
      debug(F110, "ftp remote_files open fail", tmpfilnam[mlsdepth], 0);
      if ((!dpyactive || ftp_deb))
        printf("?Can't find list of remote files, oops\n");
      rfrc = -9;
      return (NULL);
    }
    if (ftp_deb)
      printf("LISTFILE: %s\n", tmpfilnam[mlsdepth]);
  }
  buf[0] = NUL;
  buf[FTPNAMBUFLEN - 1] = NUL;
  buf[FTPNAMBUFLEN - 2] = NUL;

  /* We have to redo all this because the first time was only for */
  /* for getting the file list, now it's for getting each file */

  if (arg && mgetmethod == SND_MLS) { /* MLSD */
    if (!pattern && iswild((char *)arg)) {
      pattern = arg; /* Wild arg is really a pattern */
      if (pattern)
        if (!*pattern)
          pattern = NULL;
      arg = NULL; /* and not an arg */
    }
    if (new_query) {      /* Initial query? */
      cdto = (char *)arg; /* (nonwild) arg given? */
      if (cdto)
        if (!*cdto)
          cdto = NULL;
      if (cdto) /* If so, then CD to it */
        doftpcwd(cdto, 0);
    }
  }
  new_query = 0;

  if (fgets((char *)buf, FTPNAMBUFLEN, tmpfilptr[mlsdepth]) == NULL) {
    fclose(tmpfilptr[mlsdepth]);
    tmpfilptr[mlsdepth] = NULL;

    if (ftp_deb && !deblog) {
      printf("(Temporary file %s NOT deleted)\n", (char *)tmpfilnam[mlsdepth]);
    }
    if (mlsdepth <= 0) { /* EOF at depth 0 */
      rfrc = -3;         /* means we're done */
      return (NULL);
    }
    printf("POPPING(%d)...\n", mlsdepth - 1);
    if (tmpfilnam[mlsdepth])
      free(tmpfilnam[mlsdepth]);
    mlsdepth--;
    doftpcdup();
    zchdir(".."); /* <-- Not portable */
    goto again;
  }
  if (buf[FTPNAMBUFLEN - 1]) {
    printf("?BUFFER OVERFLOW -- FTP NLST or MLSD string longer than %d\n",
           FTPNAMBUFLEN);
    debug(F101, "remote_files buffer overrun", "", FTPNAMBUFLEN);
    return (NULL);
  }
  /* debug(F110,"ftp remote_files buf 1",buf,0); */
  if ((cp = (CHAR *)ckstrchr((char *)buf, '\n')) != NULL)
    *cp = '\0';
  if ((cp = (CHAR *)ckstrchr((char *)buf, '\r')) != NULL)
    *cp = '\0';
  debug(F110, "ftp remote_files buf", buf, 0);
  rfrc = 0;

  if (ftp_deb)
    printf("[%s]\n", (char *)buf);

  havesize = (CK_OFF_T)-1; /* Initialize file facts... */
  havetype = 0;
  makestr(&havemdtm, NULL);
  p = (char *)buf;

  if (mgetmethod == SND_NLS) { /* NLST... */
    if (pattern) {
      if (!ckmatch((char *)pattern, p, (servertype == SYS_UNIX), 1))
        goto again;
    }
  } else { /* MLSD... */
    p = parsefacts((char *)buf);
    switch (havetype) {
    case FTYP_FILE: /* File: Get it if it matches */
      if (pattern) {
        if (!ckmatch((char *)pattern, p, (servertype == SYS_UNIX), 1))
          goto again;
      }
      break;
    case FTYP_CDIR:   /* Current directory */
    case FTYP_PDIR:   /* Parent directory */
      goto again;     /* Skip */
    case FTYP_DIR:    /* (Sub)Directory */
      if (!recursive) /* If not /RECURSIVE */
        goto again;   /* Skip */
      if (mlsdepth < MLSDEPTH) {
        char *p2 = NULL;
        mlsdepth++;
        printf("RECURSING [%s](%d)...\n", p, mlsdepth);
        if (doftpcwd(p, 0) > 0) {
          int x;
          if (!ckstrchr(p, '/')) {
            /* zmkdir() needs dirsep */
            if ((p2 = (char *)malloc((int)strlen(p) + 2))) {
              strcpy(p2, p);   /* SAFE */
              strcat(p2, "/"); /* SAFE */
              p = p2;
            }
          }
#ifdef NOMKDIR
          x = -1;
#else
          x = zmkdir(p);
#endif /* NOMKDIR */
          if (x > -1) {
            zchdir(p);
            p = (char *)remote_files(1, arg, pattern, 0);
            if (p2)
              free(p2);
          } else {
            printf("?mkdir failed: [%s] Depth=%d\n", p, mlsdepth);
            mlsreset();
            if (p2)
              free(p2);
            return (NULL);
          }
        } else {
          printf("?CWD failed: [%s] Depth=%d\n", p, mlsdepth);
          mlsreset();
          return (NULL);
        }
      } else {
        printf("MAX DIRECTORY STACK DEPTH EXCEEDED: %d\n", mlsdepth);
        mlsreset();
        return (NULL);
      }
    }
  }

#ifdef DEBUG
  if (deblog) {
    debug(F101, "remote_files havesize", "", havesize);
    debug(F101, "remote_files havetype", "", havetype);
    debug(F110, "remote_files havemdtm", havemdtm, 0);
    debug(F110, "remote_files name", p, 0);
  }
#endif /* DEBUG */
  return ((CHAR *)p);
}

/* N O T  P O R T A B L E !!! */

#if (SIZEOF_SHORT == 4)
typedef unsigned short ftp_uint32;
typedef short ftp_int32;
#else
#if (SIZEOF_INT == 4)
typedef unsigned int ftp_uint32;
typedef int ftp_int32;
#else
#if (SIZEOF_LONG == 4)
typedef ULONG ftp_uint32;
typedef long ftp_int32;
#endif
#endif
#endif

/* Perhaps use these in general, certainly use them for GSSAPI */

#ifndef looping_write
#define ftp_int32 int
#define ftp_uint32 unsigned int
static int looping_write(int fd, register const char *buf, int len) {
  int cc;
  register int wrlen = len;
  do {
    cc = send(fd, (SENDARG2TYPE)buf, wrlen, 0);
    if (cc < 0) {
      if (errno == EINTR)
        continue;
      return (cc);
    } else {
      buf += cc;
      wrlen -= cc;
    }
  } while (wrlen > 0);
  return (len);
}
#endif
#ifndef looping_read
static int looping_read(int fd, register char *buf, register int len) {
  int cc, len2 = 0;

  do {
    cc = recv(fd, (char *)buf, len, 0);
    if (cc < 0) {
      if (errno == EINTR)
        continue;
      return (cc); /* errno is already set */
    } else if (cc == 0) {
      return (len2);
    } else {
      buf += cc;
      len2 += cc;
      len -= cc;
    }
  } while (len > 0);
  return (len2);
}
#endif /* looping_read */

#define ERR -2

/* returns:
 *       0  on success
 *      -1  on error (errno set)
 *      -2  on security error
 */
static int secure_flush(int fd) {
  int rc = 0;
  int len = 0;

  if (nout > 0) {
    len = nout;
    if (!ftpissecure()) {
      rc = send(fd, (SENDARG2TYPE)ucbuf, nout, 0);
      nout = 0;
      goto xflush;
    } else {
      rc = secure_putbuf(fd, ucbuf, nout);
      if (rc)
        goto xflush;
    }
  }
  rc = (!ftpissecure()) ? 0 : secure_putbuf(fd, (CHAR *)"", nout = 0);

xflush:
  if (rc > -1 && len > 0 && fdispla != XYFD_B) {
    spackets++;
    spktl = len;
    ftscreen(SCR_PT, 'D', (CK_OFF_T)spackets, NULL);
  }
  return (rc);
}

/* returns:
 *      nbyte on success
 *      -1  on error (errno set)
 *      -2  on security error
 */
static int secure_write(int fd, CHAR *buf, unsigned int nbyte) {
  int ret;

#ifdef FTP_TIMEOUT
  ftp_timed_out = 0;
  if (check_data_connection(fd, 1) < 0) {
    ftp_timed_out = 1;
    return (-3);
  }
#endif /* FTP_TIMEOUT */

  if (!ftpissecure()) {
    if (nout > 0) {
      if ((ret = send(fd, (SENDARG2TYPE)ucbuf, nout, 0)) < 0)
        return (ret);
      nout = 0;
    }
    return (send(fd, (SENDARG2TYPE)buf, nbyte, 0));
  } else {
    int ucbuflen = (maxbuf ? maxbuf : actualbuf) - FUDGE_FACTOR;
    int bsent = 0;

    while (bsent < nbyte) {
      int b2cp = ((nbyte - bsent) > (ucbuflen - nout) ? (ucbuflen - nout)
                                                      : (nbyte - bsent));
#ifdef DEBUG
      if (deblog) {
        debug(F101, "secure_write ucbuflen", "", ucbuflen);
        debug(F101, "secure_write ucbufsiz", "", ucbufsiz);
        debug(F101, "secure_write bsent", "", bsent);
        debug(F101, "secure_write b2cp", "", b2cp);
      }
#endif /* DEBUG */
      memcpy(&ucbuf[nout], &buf[bsent], b2cp);
      nout += b2cp;
      bsent += b2cp;

      if (nout == ucbuflen) {
        nout = 0;
        ret = secure_putbuf(fd, ucbuf, ucbuflen);
        if (ret < 0)
          return (ret);
      }
    }
    return (bsent);
  }
}

/* returns:
 *       0  on success
 *      -1  on error (errno set)
 *      -2  on security error
 */
static int secure_putbuf(int fd, CHAR *buf, unsigned int nbyte) {
  static char *outbuf = NULL; /* output ciphertext */
  ftp_int32 length = 0;
  ftp_uint32 net_len = 0;

  /* Other auth types go here ... */

  net_len = htonl((ULONG)length);
  if (looping_write(fd, (char *)&net_len, 4) == -1)
    return (-1);
  if (looping_write(fd, outbuf, length) != length)
    return (-1);
  return (0);
}

/* fc = 0 means to get a byte; nonzero means to initialize buffer pointers */

static int secure_getbyte(int fd, int fc) {
  /* number of chars in ucbuf, pointer into ucbuf */
  static unsigned int nin = 0, bufp = 0;
  int kerror;
  ftp_uint32 length;

  if (fc) {
    nin = bufp = 0;
    ucbuf[0] = NUL;
    return (0);
  }
  if (nin == 0) {
    if (iscanceled())
      return (-9);

#ifdef FTP_TIMEOUT
    if (check_data_connection(fd, 0) < 0)
      return (-3);
#endif /* FTP_TIMEOUT */

    {
      kerror = looping_read(fd, (char *)&length, sizeof(length));
      if (kerror != sizeof(length)) {
        secure_error("Couldn't read PROT buffer length: %d/%s", kerror,
                     kerror == -1 ? ck_errstr() : "premature EOF");
        return (ERR);
      }
      debug(F101, "secure_getbyte length", "", length);
      debug(F101, "secure_getbyte ntohl(length)", "", ntohl(length));

      length = (ULONG)ntohl(length);
      if (length > maxbuf) {
        secure_error("Length (%d) of PROT buffer > PBSZ=%u", length, maxbuf);
        return (ERR);
      }
      if ((kerror = looping_read(fd, (char *)ucbuf, length)) != length) {
        secure_error("Couldn't read %u byte PROT buffer: %s", length,
                     kerror == -1 ? ck_errstr() : "premature EOF");
        return (ERR);
      }

      /* Other auth types go here ... */
      /* Other auth types go here ... */

      /* Update file transfer display */
      rpackets++;
      pktnum++;
      if (fdispla != XYFD_B) {
        rpktl = nin;
        ftscreen(SCR_PT, 'D', (CK_OFF_T)rpackets, NULL);
      }
    }
  }
  if (nin == 0)
    return (EOF);
  else
    return (ucbuf[bufp - nin--]);
}

/* secure_getc(fd,fc)
 * Call with:
 *   fd = file descriptor for connection.
 *   fc = 0 to get a character, fc != 0 to initialize buffer pointers.
 * Returns:
 *   c>=0 on success (character value)
 *   -1   on EOF
 *   -2   on security error
 *   -3   on timeout (if built with FTP_TIMEOUT defined)
 */
static int secure_getc(int fd, int fc) /* file descriptor, function code */
{
  if (!ftpissecure()) {
    static unsigned int nin = 0, bufp = 0;
    if (fc) {
      nin = bufp = 0;
      ucbuf[0] = NUL;
      return (0);
    }
    if (nin == 0) {
      if (iscanceled())
        return (-9);

#ifdef FTP_TIMEOUT
      if (check_data_connection(fd, 0) < 0) {
        debug(F100, "secure_getc TIMEOUT", "", 0);
        nin = bufp = 0;
        ftp_timed_out = 1;
        return (-3);
      }
#endif /* FTP_TIMEOUT */

      nin = bufp = recv(fd, (char *)ucbuf, actualbuf, 0);
      if ((nin == 0) || (nin == (unsigned int)-1)) {
        debug(F111, "secure_getc recv errno", ckitoa(nin), errno);
        debug(F101, "secure_getc returns EOF", "", EOF);
        nin = bufp = 0;
        return (EOF);
      }
      debug(F101, "ftp secure_getc recv", "", nin);
      ckhexdump("ftp secure_getc recv", ucbuf, 16);
      rpackets++;
      pktnum++;
      if (fdispla != XYFD_B) {
        rpktl = nin;
        ftscreen(SCR_PT, 'D', (CK_OFF_T)rpackets, NULL);
      }
    }
    return (ucbuf[bufp - nin--]);
  } else
    return (secure_getbyte(fd, fc));
}

/* returns:
 *     n>0  on success (n == # of bytes read)
 *       0  on EOF
 *      -1  on error (errno set), only for FPL_CLR
 *      -2  on security error
 */
static int secure_read(int fd, char *buf, int nbyte) {
  static int c = 0;
  int i;

  debug(F101, "secure_read bytes requested", "", nbyte);
  if (c == EOF)
    return (c = 0);
  for (i = 0; nbyte > 0; nbyte--) {
    c = secure_getc(fd, 0);
    switch (c) {
    case -9: /* Canceled from keyboard */
      debug(F101, "ftp secure_read interrupted", "", c);
      return (0);
    case ERR:
      debug(F101, "ftp secure_read error", "", c);
      return (c);
    case EOF:
      debug(F101, "ftp secure_read EOF", "", c);
      if (!i)
        c = 0;
      return (i);
#ifdef FTP_TIMEOUT
    case -3:
      debug(F101, "ftp secure_read timeout", "", c);
      return (c);
#endif /* FTP_TIMEOUT */
    default:
      buf[i++] = c;
    }
  }
  return (i);
}

#ifdef USE_RUSERPASS
/* BEGIN_RUSERPASS
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)ruserpass.c 5.3 (Berkeley) 3/1/91";
#endif /* not lint */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

char *renvlook();
static FILE *cfile;

#define DEFAULT 1
#define LOGIN 2
#define PASSWD 3
#define ACCOUNT 4
#define MACDEF 5
#define ID 10
#define MACH 11

static char tokval[100];

static struct toktab {
  char *tokstr;
  int tval;
} toktab[] = {"default", DEFAULT, "login",   LOGIN,   "password", PASSWD,
              "passwd",  PASSWD,  "account", ACCOUNT, "machine",  MACH,
              "macdef",  MACDEF,  0,         0};

static int token() {
  char *cp;
  int c;
  struct toktab *t;

  if (feof(cfile))
    return (0);
  while ((c = getc(cfile)) != EOF &&
         (c == '\n' || c == '\t' || c == ' ' || c == ','))
    continue;
  if (c == EOF)
    return (0);
  cp = tokval;
  if (c == '"') {
    while ((c = getc(cfile)) != EOF && c != '"') {
      if (c == '\\')
        c = getc(cfile);
      *cp++ = c;
    }
  } else {
    *cp++ = c;
    while ((c = getc(cfile)) != EOF && c != '\n' && c != '\t' && c != ' ' &&
           c != ',') {
      if (c == '\\')
        c = getc(cfile);
      *cp++ = c;
    }
  }
  *cp = 0;
  if (tokval[0] == 0)
    return (0);
  for (t = toktab; t->tokstr; t++)
    if (!strcmp(t->tokstr, tokval))
      return (t->tval);
  return (ID);
}

ruserpass(host, aname, apass, aacct) char *host, **aname, **apass, **aacct;
{
  char *hdir, buf[FTP_BUFSIZ], *tmp;
  char myname[MAXHOSTNAMELEN], *mydomain;
  int t, i, c, usedefault = 0;
  struct stat stb;

  hdir = getenv("HOME");
  if (hdir == NULL)
    hdir = ".";
  ckmakmsg(buf, FTP_BUFSIZ, hdir, "/.netrc", NULL, NULL);
  cfile = fopen(buf, "r");
  if (cfile == NULL) {
    if (errno != ENOENT)
      perror(buf);
    return (0);
  }
  if (gethostname(myname, MAXHOSTNAMELEN) < 0)
    myname[0] = '\0';
  if ((mydomain = ckstrchr(myname, '.')) == NULL)
    mydomain = "";

next:
  while ((t = token()))
    switch (t) {

    case DEFAULT:
      usedefault = 1;
      /* FALL THROUGH */

    case MACH:
      if (!usedefault) {
        if (token() != ID)
          continue;
        /*
         * Allow match either for user's input host name
         * or official hostname.  Also allow match of
         * incompletely-specified host in local domain.
         */
        if (ckstrcmp(host, tokval, -1, 1) == 0)
          goto match;
        if (ckstrcmp(ftp_host, tokval, -1, 0) == 0)
          goto match;
        if ((tmp = ckstrchr(ftp_host, '.')) != NULL &&
            ckstrcmp(tmp, mydomain, -1, 1) == 0 &&
            ckstrcmp(ftp_host, tokval, tmp - ftp_host, 0) == 0 &&
            tokval[tmp - ftp_host] == '\0')
          goto match;
        if ((tmp = ckstrchr(host, '.')) != NULL &&
            ckstrcmp(tmp, mydomain, -1, 1) == 0 &&
            ckstrcmp(host, tokval, tmp - host, 0) == 0 &&
            tokval[tmp - host] == '\0')
          goto match;
        continue;
      }

    match:
      while ((t = token()) && t != MACH && t != DEFAULT)
        switch (t) {

        case LOGIN:
          if (token())
            if (*aname == 0) {
              *aname = malloc((unsigned)strlen(tokval) + 1);
              strcpy(*aname, tokval); /* safe */
            } else {
              if (strcmp(*aname, tokval))
                goto next;
            }
          break;
        case PASSWD:
          if (strcmp(*aname, "anonymous") && fstat(fileno(cfile), &stb) >= 0 &&
              (stb.st_mode & 077) != 0) {
            fprintf(stderr, "Error - .netrc file not correct mode.\n");
            fprintf(stderr, "Remove password or correct mode.\n");
            goto bad;
          }
          if (token() && *apass == 0) {
            *apass = malloc((unsigned)strlen(tokval) + 1);
            strcpy(*apass, tokval); /* safe */
          }
          break;
        case ACCOUNT:
          if (fstat(fileno(cfile), &stb) >= 0 && (stb.st_mode & 077) != 0) {
            fprintf(stderr, "Error - .netrc file not correct mode.\n");
            fprintf(stderr, "Remove account or correct mode.\n");
            goto bad;
          }
          if (token() && *aacct == 0) {
            *aacct = malloc((unsigned)strlen(tokval) + 1);
            strcpy(*aacct, tokval); /* safe */
          }
          break;

        default:
          fprintf(stderr, "Unknown .netrc keyword %s\n", tokval);
          break;
        }
      goto done;
    }

done:
  fclose(cfile);
  return (0);

bad:
  fclose(cfile);
  return (-1);
}
#endif /* USE_RUSERPASS */

static char *radixN =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char pad = '=';

static int

radix_encode(CHAR inbuf[], CHAR outbuf[], int inlen, int *outlen, int decode) {
  int i, j, D = 0;
  char *p;
  CHAR c = NUL;

  if (decode) {
    for (i = 0, j = 0; inbuf[i] && inbuf[i] != pad; i++) {
      if ((p = ckstrchr(radixN, inbuf[i])) == NULL)
        return (1);
      D = p - radixN;
      switch (i & 3) {
      case 0:
        outbuf[j] = D << 2;
        break;
      case 1:
        outbuf[j++] |= D >> 4;
        outbuf[j] = (D & 15) << 4;
        break;
      case 2:
        outbuf[j++] |= D >> 2;
        outbuf[j] = (D & 3) << 6;
        break;
      case 3:
        outbuf[j++] |= D;
      }
      if (j == *outlen)
        return (4);
    }
    switch (i & 3) {
    case 1:
      return (3);
    case 2:
      if (D & 15)
        return (3);
      if (strcmp((char *)&inbuf[i], "=="))
        return (2);
      break;
    case 3:
      if (D & 3)
        return (3);
      if (strcmp((char *)&inbuf[i], "="))
        return (2);
    }
    *outlen = j;
  } else {
    for (i = 0, j = 0; i < inlen; i++) {
      switch (i % 3) {
      case 0:
        outbuf[j++] = radixN[inbuf[i] >> 2];
        c = (inbuf[i] & 3) << 4;
        break;
      case 1:
        outbuf[j++] = radixN[c | inbuf[i] >> 4];
        c = (inbuf[i] & 15) << 2;
        break;
      case 2:
        outbuf[j++] = radixN[c | inbuf[i] >> 6];
        outbuf[j++] = radixN[inbuf[i] & 63];
        c = 0;
      }
      if (j == *outlen)
        return (4);
    }
    if (i % 3)
      outbuf[j++] = radixN[c];
    switch (i % 3) {
    case 1:
      outbuf[j++] = pad;
    case 2:
      outbuf[j++] = pad;
    }
    outbuf[ *outlen = j] = '\0';
  }
  return (0);
}

static char *radix_error(int e) {
  switch (e) {
  case 0:
    return ("Success");
  case 1:
    return ("Bad character in encoding");
  case 2:
    return ("Encoding not properly padded");
  case 3:
    return ("Decoded # of bits not a multiple of 8");
  case 4:
    return ("Output buffer too small");
  default:
    return ("Unknown error");
  }
}
/* END_RUSERPASS */

#ifdef NOT_USED
/*
  The following code is from the Unix FTP client.  Be sure to
  make sure that the functionality is not lost.  Especially
  the Proxy stuff even though we have not yet implemented it.
*/

/* Send multiple files  */

static int ftp_mput(argc, argv)
int argc;
char **argv;
{
  register int i;
  int ointer;
  char *tp;
  void mcancel();

  if (argc < 2 && !another(&argc, &argv, "local-files")) {
    printf("usage: %s local-files\n", argv[0]);
    ftpcode = -1;
    return;
  }
  mname = argv[0];
  mflag = 1;
  oldintr = ck_signal(SIGINT, mcancel);

  /* Replace with calls to cc_execute() */
  setjmp(jcancel);
#ifdef FTP_PROXY
  if (proxy) {
    char *cp, *tp2, tmpbuf[CKMAXPATH];

    while ((cp = remglob(argv, 0)) != NULL) {
      if (*cp == 0) {
        mflag = 0;
        continue;
      }
      if (mflag && confirm(argv[0], cp)) {
        tp = cp;
        if (mcase) {
          while (*tp && !islower(*tp)) {
            tp++;
          }
          if (!*tp) {
            tp = cp;
            tp2 = tmpbuf;
            while ((*tp2 = *tp) != 0) {
              if (isupper(*tp2)) {
                *tp2 = 'a' + *tp2 - 'A';
              }
              tp++;
              tp2++;
            }
          }
          tp = tmpbuf;
        }
        if (ntflag) {
          tp = dotrans(tp);
        }
        if (mapflag) {
          tp = domap(tp);
        }
        sendrequest((sunique) ? "STOU" : "STOR", cp, tp, 0, -1, -1, 0);
        if (!mflag && fromatty) {
          ointer = interactive;
          interactive = 1;
          if (confirm("Continue with", "mput")) {
            mflag++;
          }
          interactive = ointer;
        }
      }
    }
    ck_signal(SIGINT, oldintr);
    mflag = 0;
    return;
  }
#endif /* FTP_PROXY */
  for (i = 1; i < argc; i++) {
    register char **cpp, **gargs;

    if (mflag && confirm(argv[0], argv[i])) {
      tp = argv[i];
      sendrequest((ftp_usn) ? "STOU" : "STOR", argv[i], tp, 0, -1, -1, 0);
      if (!mflag && fromatty) {
        ointer = interactive;
        interactive = 1;
        if (confirm("Continue with", "mput")) {
          mflag++;
        }
        interactive = ointer;
      }
    }
    continue;

    gargs = ftpglob(argv[i]);
    if (globerr != NULL) {
      printf("%s\n", globerr);
      if (gargs) {
        blkfree(gargs);
        free((char *)gargs);
      }
      continue;
    }
    for (cpp = gargs; cpp && *cpp != NULL; cpp++) {
      if (mflag && confirm(argv[0], *cpp)) {
        tp = *cpp;
        sendrequest((sunique) ? "STOU" : "STOR", *cpp, tp, 0, -1, -1, 0);
        if (!mflag && fromatty) {
          ointer = interactive;
          interactive = 1;
          if (confirm("Continue with", "mput")) {
            mflag++;
          }
          interactive = ointer;
        }
      }
    }
    if (gargs != NULL) {
      blkfree(gargs);
      free((char *)gargs);
    }
  }
  ck_signal(SIGINT, oldintr);
  mflag = 0;
}

/* Get multiple files */

static int ftp_mget(argc, argv)
int argc;
char **argv;
{
  int rc = -1;
  int ointer;
  char *cp, *tp, *tp2, tmpbuf[CKMAXPATH];
  void mcancel();

  if (argc < 2 && !another(&argc, &argv, "remote-files")) {
    printf("usage: %s remote-files\n", argv[0]);
    ftpcode = -1;
    return (-1);
  }
  mname = argv[0];
  mflag = 1;
  oldintr = ck_signal(SIGINT, mcancel);
  /* Replace with calls to cc_execute() */
  setjmp(jcancel);
  while ((cp = remglob(argv, proxy)) != NULL) {
    if (*cp == '\0') {
      mflag = 0;
      continue;
    }
    if (mflag && confirm(argv[0], cp)) {
      tp = cp;
      if (mcase) {
        while (*tp && !islower(*tp)) {
          tp++;
        }
        if (!*tp) {
          tp = cp;
          tp2 = tmpbuf;
          while ((*tp2 = *tp) != 0) {
            if (isupper(*tp2)) {
              *tp2 = 'a' + *tp2 - 'A';
            }
            tp++;
            tp2++;
          }
        }
        tp = tmpbuf;
      }
      rc = (recvrequest("RETR", tp, cp, "wb", tp != cp || !interactive) == 0, 0,
            NULL, 0, 0, 0);
      if (!mflag && fromatty) {
        ointer = interactive;
        interactive = 1;
        if (confirm("Continue with", "mget")) {
          mflag++;
        }
        interactive = ointer;
      }
    }
  }
  ck_signal(SIGINT, oldintr);
  mflag = 0;
  return (rc);
}

/* Delete multiple files */

static int mdelete(argc, argv)
int argc;
char **argv;
{
  int ointer;
  char *cp;
  void mcancel();

  if (argc < 2 && !another(&argc, &argv, "remote-files")) {
    printf("usage: %s remote-files\n", argv[0]);
    ftpcode = -1;
    return (-1);
  }
  mname = argv[0];
  mflag = 1;
  oldintr = ck_signal(SIGINT, mcancel);
  /* Replace with calls to cc_execute() */
  setjmp(jcancel);
  while ((cp = remglob(argv, 0)) != NULL) {
    if (*cp == '\0') {
      mflag = 0;
      continue;
    }
    if (mflag && confirm(argv[0], cp)) {
      rc = (ftpcmd("DELE", cp, -1, -1, ftp_vbm) == REPLY_COMPLETE);
      if (!mflag && fromatty) {
        ointer = interactive;
        interactive = 1;
        if (confirm("Continue with", "mdelete")) {
          mflag++;
        }
        interactive = ointer;
      }
    }
  }
  ck_signal(SIGINT, oldintr);
  mflag = 0;
  return (rc);
}

/* Get a directory listing of multiple remote files */

static int mls(argc, argv)
int argc;
char **argv;
{
  int ointer, i;
  char *cmd, mode[1], *dest;
  void mcancel();
  int rc = -1;

  if (argc < 2 && !another(&argc, &argv, "remote-files"))
    goto usage;
  if (argc < 3 && !another(&argc, &argv, "local-file")) {
  usage:
    printf("usage: %s remote-files local-file\n", argv[0]);
    ftpcode = -1;
    return (-1);
  }
  dest = argv[argc - 1];
  argv[argc - 1] = NULL;
  if (strcmp(dest, "-") && *dest != '|')
    if (!globulize(&dest) || !confirm("output to local-file:", dest)) {
      ftpcode = -1;
      return (-1);
    }
  cmd = argv[0][1] == 'l' ? "NLST" : "LIST";
  mname = argv[0];
  mflag = 1;
  oldintr = ck_signal(SIGINT, mcancel);
  /* Replace with calls to cc_execute() */
  setjmp(jcancel);
  for (i = 1; mflag && i < argc - 1; ++i) {
    *mode = (i == 1) ? 'w' : 'a';
    rc = recvrequest(cmd, dest, argv[i], mode, 0, 0, NULL, 0, 0, 0);
    if (!mflag && fromatty) {
      ointer = interactive;
      interactive = 1;
      if (confirm("Continue with", argv[0])) {
        mflag++;
      }
      interactive = ointer;
    }
  }
  ck_signal(SIGINT, oldintr);
  mflag = 0;
  return (rc);
}

static char *remglob(argv, doswitch)
char *argv[];
int doswitch;
{
  char temp[16];
  static char buf[CKMAXPATH];
  static FILE *ftemp = NULL;
  static char **args;
  int oldhash;
  char *cp, *mode;

  if (!mflag) {
    if (!doglob) {
      args = NULL;
    } else {
      if (ftemp) {
        (void)fclose(ftemp);
        ftemp = NULL;
      }
    }
    return (NULL);
  }
  if (!doglob) {
    if (args == NULL)
      args = argv;
    if ((cp = *++args) == NULL)
      args = NULL;
    return (cp);
  }
  if (ftemp == NULL) {
    (void)strcpy(temp, _PATH_TMP);
#ifdef MKTEMP
#ifndef MKSTEMP
    (void)mktemp(temp);
#endif /* MKSTEMP */
#endif /* MKTEMP */
    verbose = 0;
    oldhash = hash, hash = 0;
#ifdef FTP_PROXY
    if (doswitch) {
      pswitch(!proxy);
    }
#endif /* FTP_PROXY */
    for (mode = "wb"; *++argv != NULL; mode = "ab")
      recvrequest("NLST", temp, *argv, mode, 0);
#ifdef FTP_PROXY
    if (doswitch) {
      pswitch(!proxy);
    }
#endif /* FTP_PROXY */
    hash = oldhash;
    ftemp = fopen(temp, "r");
    unlink(temp);
    if (ftemp == NULL && (!dpyactive || ftp_deb)) {
      printf("Can't find list of remote files, oops\n");
      return (NULL);
    }
  }
  if (fgets(buf, CKMAXPATH, ftemp) == NULL) {
    fclose(ftemp), ftemp = NULL;
    return (NULL);
  }
  if ((cp = ckstrchr(buf, '\n')) != NULL)
    *cp = '\0';
  return (buf);
}
#endif /* NOT_USED */
#endif /* TCPSOCKET (top of file) */
#endif /* SYSFTP (top of file) */
#endif /* NOFTP (top of file) */
