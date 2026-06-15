
/*  C K U U S 7 --  "User Interface" for C-Kermit, part 7  */

/*
  Authors:
    Frank da Cruz <fdc@columbia.edu>,
      The Kermit Project, New York City
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2024,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
    Last big update: 14 April 2023 (ANSI function declarations and prototypes)
    Other updates: 5 May 2023 (change semicolon to comma in extern statement)
    Other updates: 25 Jan 2024 (add semantics of REMOTE CDUP)
*/

/*
  This file created from parts of ckuus3.c, which became too big for
  Mark Williams Coherent compiler to handle.
*/

/*
  Definitions here supersede those from system include files.
*/
/* clang-format off */
#include "ckcdeb.h" /* Debugging & compiler things */
/* clang-format on */
#include "ckcasc.h" /* ASCII character symbols */
#include "ckcfnp.h" /* Prototypes (must be last) */
#include "ckcker.h" /* Kermit application definitions */
#include "ckclib.h"
#include "ckcnet.h" /* Network symbols */
#include "ckcxla.h" /* Character set translation */
#include "ckucmd.h"
#include "ckuusr.h" /* User interface symbols */

#ifdef SSHBUILTIN
#include "ckossh.h"
#endif /* SSHBUILTIN */

char *slmsg = NULL;

static int x, y = 0, z;
static char *s;

extern CHAR feol;
extern int g_matchdot, hints, xcmdsrc, rcdactive;

extern char *k_info_dir;

#ifdef CK_LOGIN
#ifdef CK_PAM
int gotemptypasswd = 0; /* distinguish empty passwd from none given */
#endif                  /* CK_PAM */
#endif                  /* CK_LOGIN */

#ifndef NOSPL
extern int nmac;
extern struct mtab *mactab;
#endif /* NOSPL */

#ifndef NOXFER
#ifdef CK_SPEED
extern short ctlp[]; /* Control-char prefixing table */
#endif               /* CK_SPEED */

#ifdef PIPESEND
extern char *sndfilter, *g_sfilter;
extern char *rcvfilter, *g_rfilter;
#endif /* PIPESEND */

extern char *snd_move;
extern char *snd_rename;
extern char *g_snd_move;
extern char *g_snd_rename;
extern char *rcv_move;
extern char *rcv_rename;
extern char *g_rcv_move;
extern char *g_rcv_rename;

#ifdef PATTERNS
extern char *binpatterns[], *txtpatterns[];
extern int patterns;
#endif /* PATTERNS */

extern char *remdest;
#ifdef CK_TMPDIR
char *dldir = NULL;
#endif /* CK_TMPDIR */

extern struct ck_p ptab[];

extern int protocol, remfile, rempipe, remappd, reliable, xreliable, fmask,
    fncnv, frecl, maxrps, wslotr, bigsbsiz, bigrbsiz, urpsiz, rpsiz, spsiz,
    bctr, npad, timef, timint, spsizr, spsizf, maxsps, spmax, nfils, displa,
    atcapr, pkttim, rtimo, fncact, mypadn, fdispla, f_save, pktpaus,
    setreliable, fnrpath, fnspath, atenci, atenco, atdati, atdato, atleni,
    atleno, atblki, atblko, attypi, attypo, atsidi, atsido, atsysi, atsyso,
    atdisi, atdiso, rpsizf;

extern int stathack;

extern int atfrmi, atfrmo;
#ifdef CK_PERMS
extern int atlpri, atlpro, atgpri, atgpro;
#endif /* CK_PERMS */

extern CHAR sstate, eol, seol, stchr, mystch, mypadc, padch, ctlq, myctlq;

#ifdef IKSD
extern int inserver;
#ifdef IKSDCONF
extern int iksdcf;
#endif /* IKSDCONF */
#endif /* IKSD */

extern char *cmarg, *cmarg2;

#ifndef NOFRILLS
extern char optbuf[]; /* Buffer for MAIL or PRINT options */
extern int rprintf;   /* REMOTE PRINT flag */
#endif                /* NOFRILLS */
#endif                /* NOXFER */

#ifdef CK_TRIGGER
extern char *tt_trigger[];
#endif /* CK_TRIGGER */

extern int tcs_transp;
#ifdef PCTERM
extern int tt_pcterm;
#endif /* PCTERM */

#ifdef SSHBUILTIN
int sl_ssh_xfw = 0;
int sl_ssh_xfw_saved = 0;
int sl_ssh_ver = 0;
int sl_ssh_ver_saved = 0;
#endif /* SSHBUILTIN */

extern char uidbuf[];
static int uidflag = 0;
char sl_uidbuf[UIDBUFLEN] = {NUL, NUL};
int sl_uid_saved = 0;
#ifdef TNCODE
int sl_tn_wait = 0;
int sl_tn_saved = 0;
#endif /* TNCODE */

#ifdef TNCODE
extern int tn_wait_flg;
#endif /* TNCODE */

void slrestor(void) {
  if (sl_uid_saved) {
    ckstrncpy(uidbuf, sl_uidbuf, UIDBUFLEN);
    sl_uid_saved = 0;
  }
#ifdef TNCODE
  if (sl_tn_saved) {
    tn_wait_flg = sl_tn_wait;
    sl_tn_saved = 0;
  }
#endif /* TNCODE */
#ifdef SSHBUILTIN
  if (sl_ssh_xfw_saved) {
    ssh_set_iparam(SSH_IPARAM_XFW, sl_ssh_xfw);
    sl_ssh_xfw_saved = 0;
  }
  if (sl_ssh_ver_saved) {
    ssh_set_iparam(SSH_IPARAM_VER, sl_ssh_ver);
    sl_ssh_ver_saved = 0;
  }
#endif /* SSHBUILTIN */
}

int oldplex = -1; /* Duplex holder around network */

#ifndef NOICP
#ifdef LOCUS
extern int locus, autolocus;
#endif /* LOCUS */
#ifndef NODIAL
extern int dialsta;
#endif /* NODIAL */

/* Note: gcc -Wall wants braces around each keyword table entry. */

static struct keytab psltab[] = {/* SET LINE/PORT command options */
                                 {"/connect", SL_CNX, 0},
                                 {"/server", SL_SRV, 0},
                                 {"", 0, 0}};
static int npsltab = sizeof(psltab) / sizeof(struct keytab) - 1;

#ifdef NETCONN
static struct keytab shtab[] = {/* SET HOST command options */
#ifdef NETCMD
                                /* (COMMAND is also a network type) */
                                {"/command", SL_CMD, CM_INV},
#endif /* NETCMD */
                                {"/connect", SL_CNX, 0},
                                {"/network-type", SL_NET, CM_ARG},
                                {"/nowait", SL_NOWAIT, 0},
#ifndef NOSPL
#endif /* NOSPL */
#ifdef NETCMD
                                {"/pipe", SL_CMD, 0},
#endif /* NETCMD */
#ifdef NETPTY
                                {"/pty", SL_PTY, 0},
#endif /* NETPTY */
                                {"/server", SL_SRV, 0},
                                {"/userid", SL_UID, CM_ARG},
                                {"/wait", SL_WAIT, 0},
                                {"", 0, 0}};
static int nshtab = sizeof(shtab) / sizeof(struct keytab) - 1;

static struct keytab shteltab[] = {/* TELNET command options */
                                   {"/nowait", SL_NOWAIT, 0},
#ifndef NOSPL
#endif /* NOSPL */
                                   {"/timeout", SL_TMO, CM_ARG},
                                   {"/userid", SL_UID, CM_ARG},
                                   {"/wait", SL_WAIT, 0},
                                   {"", 0, 0}};
static int nshteltab = sizeof(shteltab) / sizeof(struct keytab) - 1;

#ifdef RLOGCODE
static struct keytab shrlgtab[] = {/* SET HOST RLOGIN command options */
                                   {"", 0, 0}};
static int nshrlgtab = sizeof(shrlgtab) / sizeof(struct keytab) - 1;
#endif /* RLOGCODE */

extern struct keytab netcmd[];
extern int nnets;
#ifndef NODIAL
extern int dirline;
extern int nnetdir; /* Network services directory */
extern char *netdir[];
void ndreset(void);
char *nh_p[MAXDNUMS + 1];     /* Network directory entry pointers */
char *nh_p2[MAXDNUMS + 1];    /* Network directory entry nettype */
char *nh_px[4][MAXDNUMS + 1]; /* Network-specific stuff... */
#endif                        /* NODIAL */
int nhcount = 0;
int ndinited = 0;
char *n_name = NULL; /* Network name pointer */
#endif               /* NETCONN */

int remtxt(char **);
void rmsg(void);
static int remcfm(void);

extern int nopush;

int mdmsav = -1;    /* Save modem type around network */
extern int isguest; /* Global flag for anonymous login */

extern xx_strp xxstring;

extern int success, binary, b_save, ckwarn, msgflg, quiet, cmask, pflag, local,
    nettype, escape, mdmtyp, duplex, dfloc, network, cdtimo, autoflow, tnlm,
    sosi, tlevel, lf_opts, backgrd, flow, debses, parity, ttnproto, ckxech,
    x_ifnum, cmflgs, haveline, cxtype, cxflow[], maclvl;

#ifdef DCMDBUF
extern struct cmdptr *cmdstk; /* The command stack itself */
#else
extern struct cmdptr cmdstk[]; /* The command stack itself */
#endif /* DCMDBUF */
extern FILE *tfile[];
extern char *macp[];

extern char psave[]; /* For saving & restoring prompt */
extern int sprmlen, rprmlen;

extern int tt_rows, tt_cols;

extern int tt_escape;
extern long speed;

extern char *dftty;

extern char *tp, *lp; /* Temporary buffer & pointers */
extern char ttname[];

#ifdef CK_TAPI
int tttapi = 0; /* is Line TAPI? */
struct keytab *tapilinetab = NULL;
struct keytab *_tapilinetab = NULL;
int ntapiline = 0;
#endif /* CK_TAPI */

#ifdef NETCONN /* Network items */

#ifdef ANYX25
extern int revcall, closgr, cudata, nx25;
extern char udata[];
extern struct keytab x25tab[];
#ifndef IBMX25
extern int npadx3;
extern CHAR padparms[];
extern struct keytab padx3tab[];
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifdef NPIPE
extern char pipename[];
#endif /* NPIPE */

#ifdef TCPSOCKET
static struct keytab tcprawtab[] = {/* SET HOST options */
                                    {"/default", NP_DEFAULT, CM_INV},
                                    {"/no-telnet-init", NP_NONE, 0},
                                    {"/none", NP_NONE, CM_INV},
                                    {"/raw-socket", NP_TCPRAW, 0},
#ifdef RLOGCODE
                                    {"/rlogin", NP_RLOGIN, 0},
#endif /* RLOGCODE */
                                    {"/telnet", NP_TELNET, 0},
                                    {"", 0, 0}};
static int ntcpraw = (sizeof(tcprawtab) / sizeof(struct keytab)) - 1;

#ifdef RLOGCODE
int rlog_naws(void);
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifdef SUPERLAT
extern char slat_pwd[18];
#endif /* SUPERLAT */
#endif /* NETCONN */

#ifndef NOSETKEY
extern KEY *keymap;
extern MACRO *macrotab;
#ifndef NOKVERBS
extern struct keytab kverbs[];
extern int nkverbs;
#endif /* NOKVERBS */
#endif /* NOSETKEY */

/* Keyword tables ... */

extern struct keytab onoff[], rltab[];
extern int nrlt;

#ifndef NOCSETS
static struct keytab fdfltab[] = {{"7bit-character-set", 7, 0},
                                  {"8bit-character-set", 8, 0}};
static int nfdflt = (sizeof(fdfltab) / sizeof(struct keytab));
#endif /* NOCSETS */

/* SET FILE parameters */

static struct keytab filtab[] = {
#ifndef NOXFER
#ifdef PATTERNS
    {"binary-patterns", XYFIBP, 0},
#endif /* PATTERNS */
    {"bytesize", XYFILS, 0},
#ifndef NOCSETS
    {"character-set", XYFILC, 0},
#endif /* NOCSETS */
    {"collision", XYFILX, 0},
    {"default", XYF_DFLT, 0},
    {"destination", XYFILY, 0},
    {"display", XYFILD, CM_INV},
#ifdef CK_TMPDIR
    {"download-directory", XYFILG, 0},
#endif /* CK_TMPDIR */
#endif /* NOXFER */
    {"end-of-line", XYFILA, 0},
    {"eol", XYFILA, CM_INV},
#ifdef CK_CTRLZ
    {"eof", XYFILV, 0},
#endif /* CK_CTRLZ */
#ifndef NOXFER
    {"fastlookups", 9997, CM_INV},
    {"incomplete", XYFILI, 0},
    {"inspection", XYF_INSP, CM_INV},
#ifdef CK_LABELED
    {"label", XYFILL, 0},
#endif /* CK_LABELED */

#ifdef UNIX
#ifdef DYNAMIC
    {"listsize", XYF_LSIZ, 0},
#endif /* DYNAMIC */
#endif /* UNIX */

    {"names", XYFILN, 0},
#ifdef UNIX
    {"output", XYFILH, 0},
#endif /* UNIX */
#ifdef PATTERNS
    {"patterns", XYFIPA, 0},
#endif /* PATTERNS */
    {"scan", XYF_INSP, 0},

#ifdef UNIX
#ifdef DYNAMIC
    {"stringspace", XYF_SSPA, 0},
#endif /* DYNAMIC */
#endif /* UNIX */

#ifdef PATTERNS
    {"t", XYFILT, CM_INV | CM_ABR},
    {"text-patterns", XYFITP, 0},
#endif /* PATTERNS */
#endif /* NOXFER */
    {"type", XYFILT, 0},
#ifdef UNICODE
    {"ucs", XYFILU, 0},
#endif /* UNICODE */
#ifndef NOXFER
    {"warning", XYFILW, CM_INV}
#endif /* NOXFER */
};
static int nfilp = (sizeof(filtab) / sizeof(struct keytab));

struct keytab pathtab[] = {{"absolute", PATH_ABS, 0},
                           {"none", PATH_OFF, CM_INV},
                           {"off", PATH_OFF, 0},
                           {"on", PATH_ABS, CM_INV},
                           {"relative", PATH_REL, 0}};
int npathtab = (sizeof(pathtab) / sizeof(struct keytab));

struct keytab rpathtab[] = {
    {"absolute", PATH_ABS, 0},  {"auto", PATH_AUTO, 0},
    {"none", PATH_OFF, CM_INV}, {"off", PATH_OFF, 0},
    {"on", PATH_ABS, CM_INV},   {"relative", PATH_REL, 0}};
int nrpathtab = (sizeof(rpathtab) / sizeof(struct keytab));

#ifdef CK_CTRLZ
struct keytab eoftab[] = {/* EOF detection method */
                          {"ctrl-z", 1, 0},
                          {"length", 0, 0},
                          {"noctrl-z", 0, CM_INV}};
#endif /* CK_CTRLZ */

struct keytab fttab[] = {/* File types for SET FILE TYPE */
                         {"ascii", XYFT_T, CM_INV},
                         {"binary", XYFT_B, 0},
#ifdef CK_LABELED
                         {"labeled", XYFT_L, 0},
#endif /* CK_LABELED */
                         {"text", XYFT_T, 0}};
int nfttyp = (sizeof(fttab) / sizeof(struct keytab));

static struct keytab rfttab[] = {/* File types for REMOTE SET FILE */
                                 {"ascii", XYFT_T, CM_INV},
                                 {"binary", XYFT_B, 0},
                                 {"text", XYFT_T, 0}};
static int nrfttyp = (sizeof(rfttab) / sizeof(struct keytab));

#ifdef UNIX
#define ZOF_BLK 0
#define ZOF_NBLK 1
#define ZOF_BUF 2
#define ZOF_NBUF 3
static struct keytab zoftab[] = {{"blocking", ZOF_BLK, 0},
                                 {"buffered", ZOF_BUF, 0},
                                 {"nonblocking", ZOF_NBLK, 0},
                                 {"unbuffered", ZOF_NBUF, 0}};
static int nzoftab = (sizeof(zoftab) / sizeof(struct keytab));
#endif /* UNIX */

extern int query; /* Global flag for QUERY active */

#ifndef NOSPL
#ifndef NOXFER
static struct keytab vartyp[] = {/* Variable types for REMOTE QUERY */
                                 {"global", (int)'G', CM_INV},
                                 {"kermit", (int)'K', 0},
                                 {"system", (int)'S', 0},
                                 {"user", (int)'G', 0}};
static int nvartyp = (sizeof(vartyp) / sizeof(struct keytab));
#endif /* NOXFER */
#endif /* NOSPL */

#ifdef CK_TIMERS
static struct keytab timotab[] = {/* Timer types */
                                  {"dynamic", 1, 0},
                                  {"fixed", 0, 0}};
#endif /* CK_TIMERS */

#ifdef DCMDBUF
extern char *atxbuf, *atmbuf; /* Atom buffer */
extern char *cmdbuf;          /* Command buffer */
extern char *line, *tmpbuf;   /* Character buffers for anything */
extern int *intime;           /* INPUT TIMEOUT */

#else /* Not DCMDBUF ... */

extern char atxbuf[], atmbuf[]; /* Atom buffer */
extern char cmdbuf[];           /* Command buffer */
extern char line[], tmpbuf[];   /* Character buffer for anything */
extern int intime[];

#endif /* DCMDBUF */

#ifndef NOCSETS
extern struct keytab fcstab[];  /* For SET FILE CHARACTER-SET */
extern struct csinfo fcsinfo[]; /* File character set info. */
extern struct keytab ttcstab[];
extern int nfilc, fcharset, tcharset, ntermc, tcsr, tcsl, dcset7, dcset8;
#ifdef CKOUNI
extern int tt_utf8;
#endif /* CKOUNI */
#endif /* NOCSETS */

extern int cmdlvl; /* Overall command level */

#ifndef NOSPL
#ifdef DCMDBUF
extern int *inpcas; /* INPUT CASE setting on cmd stack */
#else
extern int inpcas[];
#endif /* DCMDBUF */
#endif /* NOSPL */

#ifdef CK_CURSES
int tgetent(char *, char *);
#endif /* CK_CURSES */

#ifndef NOXMIT
#define XMITF 0 /* SET TRANSMIT values */
#define XMITL 1 /* (Local to this module) */
#define XMITP 2
#define XMITE 3
#define XMITX 4
#define XMITS 5
#define XMITW 6
#define XMITT 7

#define XMBUFL 50
extern int xmitf, xmitl, xmitp, xmitx, xmits, xmitw, xmitt;
char xmitbuf[XMBUFL + 1] = {NUL}; /* TRANSMIT eof string */

struct keytab xmitab[] = {/* SET TRANSMIT */
                          {"echo", XMITX, 0},          {"eof", XMITE, 0},
                          {"fill", XMITF, 0},          {"linefeed", XMITL, 0},
                          {"locking-shift", XMITS, 0}, {"pause", XMITW, 0},
                          {"prompt", XMITP, 0},        {"timeout", XMITT, 0}};
int nxmit = (sizeof(xmitab) / sizeof(struct keytab));
#endif /* NOXMIT */

/* For SET FILE COLLISION */
/* Some of the following may be possible for some C-Kermit implementations */
/* but not others.  Those that are not possible for your implementation */
/* should be ifdef'd out. */

struct keytab colxtab[] =
    {                       /* SET FILE COLLISION options */
     {"append", XYFX_A, 0}, /* append to old file */
     {"backup", XYFX_B, 0}, /* rename old file */
     /* This crashes Mac Kermit. */
     {"discard", XYFX_D, CM_INV},      /* don't accept new file */
     {"no-supersede", XYFX_D, CM_INV}, /* ditto (MSK compatibility) */
     {"overwrite", XYFX_X, 0},         /* overwrite the old file */
     {"reject", XYFX_D, 0},            /* (better word than discard) */
     {"rename", XYFX_R, 0},            /* rename the incoming file */
     {"update", XYFX_U, 0},            /* replace if newer */
     {"", 0, 0}};
int ncolx = (sizeof(colxtab) / sizeof(struct keytab)) - 1;

static struct keytab rfiltab[] = {/* for REMOTE SET FILE */
#ifndef NOCSETS
                                  {"character-set", XYFILC, 0},
#endif /* NOCSETS */
                                  {"collision", XYFILX, 0},
                                  {"incomplete", XYFILI, 0},
                                  {"names", XYFILN, 0},
                                  {"record-length", XYFILR, 0},
                                  {"type", XYFILT, 0}};
int nrfilp = (sizeof(rfiltab) / sizeof(struct keytab));

struct keytab eoltab[] = {/* File eof delimiters */
                          {"cr", XYFA_C, 0},
                          {"crlf", XYFA_2, 0},
                          {"lf", XYFA_L, 0}};
static int neoltab = (sizeof(eoltab) / sizeof(struct keytab));

struct keytab fntab[] = {/* File naming */
                         {"converted", XYFN_C, 0},
                         {"literal", XYFN_L, 0},
                         {"standard", XYFN_C, CM_INV}};
int nfntab = (sizeof(fntab) / sizeof(struct keytab));

#ifndef NOLOCAL
/* Terminal parameters table */
static struct keytab trmtab[] = {
#ifdef CK_APC
    {"apc", XYTAPC, 0},
#endif /* CK_APC */
#ifdef CK_APC
#ifdef CK_AUTODL
    {
        "autodownload",
        XYTAUTODL,
        0,
    },
#endif /* CK_AUTODL */
#endif /* CK_APC */
    {"bytesize", XYTBYT, 0},
#ifndef NOCSETS
    {"character-set", XYTCS, 0},
#endif /* NOCSETS */
    {"cr-display", XYTCRD, 0},
    {"debug", XYTDEB, 0},
    {"echo", XYTEC, 0},
    {"escape-character", XYTESC, 0},
    {"height", XYTHIG, 0},
#ifdef CKTIDLE
    {"idle-action", XYTIACT, 0},
    {"idle-limit", XYTITMO, CM_INV},
    {"idle-send", XYTIDLE, CM_INV},
    {"idle-timeout", XYTITMO, 0},
#endif /* CKTIDLE */
    {"lf-display", XYTLFD, 0},
#ifndef NOCSETS
    {"local-character-set", XYTLCS, CM_INV},
#endif /* NOCSETS */
    {"locking-shift", XYTSO, 0},
    {"newline-mode", XYTNL, 0},
    {"print", XYTPRN, 0},
#ifndef NOCSETS
    {"remote-character-set", XYTRCS, CM_INV},
#endif /* NOCSETS */

    {"transparent-print", XYTPRN, CM_INV},

#ifdef CK_TRIGGER
    {"trigger", XYTRIGGER, 0},
#endif /* CK_TRIGGER */
    {"type", XYTTYP, CM_INV},

#ifndef NOCSETS
#ifdef UNICODE
#ifdef CKOUNI
    {"unicode", XYTUNI, CM_INV},
#endif /* CKOUNI */
#endif /* UNICODE */
#endif /* NOCSETS */
    {"width", XYTWID, 0},
    {"", 0, 0}};
int ntrm = (sizeof(trmtab) / sizeof(struct keytab)) - 1;

struct keytab adltab[] = {/* Autodownload Options */
                          {"ask", TAD_ASK, 0},
                          {"error", TAD_ERR, 0},
                          {"off", TAD_OFF, 0},
                          {"on", TAD_ON, 0},
                          {"", 0, 0}};
int nadltab = (sizeof(adltab) / sizeof(struct keytab)) - 1;

struct keytab adlerrtab[] = {/* Autodownload Error Options */
                             {"continue", 0, 0},
                             {"go", 0, CM_INV},
                             {"stop", 1, 0}};
int nadlerrtab = (sizeof(adlerrtab) / sizeof(struct keytab));

struct keytab crdtab[] = {/* Carriage-return display */
                          {"crlf", 1, 0},
                          {"normal", 0, 0}};
extern int tt_crd; /* Carriage-return display variable */
extern int tt_lfd; /* Linefeed display variable */

#ifdef CK_APC
extern int apcstatus, apcactive;
static struct keytab apctab[] = {/* Terminal APC parameters */
                                 {"no-input", APC_ON | APC_NOINP, 0},
                                 {"off", APC_OFF, 0},
                                 {"on", APC_ON, 0},
                                 {"unchecked", APC_ON | APC_UNCH, 0},
                                 {"unchecked-no-input",
                                  APC_ON | APC_NOINP | APC_UNCH, 0}};
int napctab = (sizeof(apctab) / sizeof(struct keytab));
#endif /* CK_APC */
#endif /* NOLOCAL */

extern int autodl, adl_err, adl_ask;

struct keytab beltab[] = {/* Terminal bell mode */
                          {"audible", XYB_AUD, CM_INV},
                          {"none", XYB_NONE, CM_INV},
                          {"off", XYB_NONE, 0},
                          {"on", XYB_AUD, 0},
                          {"", 0, 0}};
int nbeltab = sizeof(beltab) / sizeof(struct keytab) - 1;

int tt_unicode = 1; /* Use Unicode if possible */
#ifdef CKTIDLE
int tt_idlesnd_tmo = 0;      /* Idle Send Timeout, disabled */
char *tt_idlesnd_str = NULL; /* Idle Send String, none */
char *tt_idlestr = NULL;
extern int tt_idleact, tt_idlelimit;
#endif /* CKTIDLE */

/* #ifdef VMS */
struct keytab fbtab[] = {
    /* Binary record types for VMS */
    {"fixed", XYFT_B, 0},    /* Fixed is normal for binary */
    {"undefined", XYFT_U, 0} /* Undefined if they ask for it */
};
int nfbtyp = (sizeof(fbtab) / sizeof(struct keytab));
/* #endif */

#ifdef CK_CURSES
#ifdef CK_PCT_BAR
static struct keytab fdftab[] = {/* SET FILE DISPLAY FULL options */
                                 {
                                     "thermometer",
                                     1,
                                     0,
                                 },
                                 {"no-thermometer", 0, 0}};
extern int thermometer;
#endif /* CK_PCT_BAR */
#endif /* CK_CURSES */

static struct keytab fdtab[] = {/* SET FILE DISPLAY options */
                                {"brief", XYFD_B, 0}, /* Brief */
                                {"crt", XYFD_S, 0},   /* CRT display */
#ifdef CK_CURSES
                                {"fullscreen", XYFD_C,
                                 0}, /* Full-screen, whatever the method */
#endif                               /* CK_CURSES */
                                {"none", XYFD_N, 0},       /* No display */
                                {"off", XYFD_N, CM_INV},   /* Ditto */
                                {"on", XYFD_R, CM_INV},    /* On = Serial */
                                {"quiet", XYFD_N, CM_INV}, /* No display */
                                {"serial", XYFD_R, 0},     /* Serial */
                                {"", 0, 0}};
int nfdtab = (sizeof(fdtab) / sizeof(struct keytab)) - 1;

struct keytab rsrtab[] = {/* For REMOTE SET RECEIVE */
                          {"packet-length", XYLEN, 0},
                          {"timeout", XYTIMO, 0}};
int nrsrtab = (sizeof(rsrtab) / sizeof(struct keytab));

/* Send/Receive Parameters */

struct keytab srtab[] = {{"backup", XYBUP, 0},
#ifndef NOCSETS
                         {"character-set-selection", XYCSET, 0},
#endif /* NOCSETS */
                         {"control-prefix", XYQCTL, 0},
#ifdef CKXXCHAR
                         {"double-character", XYDBL, 0},
#endif /* CKXXCHAR */
                         {"end-of-packet", XYEOL, 0},
#ifdef PIPESEND
                         {"filter", XYFLTR, 0},
#endif /* PIPESEND */
#ifdef CKXXCHAR
                         {"ignore-character", XYIGN, 0},
#endif /* CKXXCHAR */
                         {"i-packets", 993, 0},
                         {"move-to", XYMOVE, 0},
                         {"negotiation-string-max-length", XYINIL, CM_INV},
                         {"packet-length", XYLEN, 0},
                         {"pad-character", XYPADC, 0},
                         {"padding", XYNPAD, 0},
                         {"pathnames", XYFPATH, 0},
                         {"pause", XYPAUS, 0},
#ifdef CK_PERMS
                         {"permissions", 994, 0},   /* 206 */
#endif                                              /* CK_PERMS */
                         {"quote", XYQCTL, CM_INV}, /* = CONTROL-PREFIX */
                         {"rename-to", XYRENAME, 0},
                         {"start-of-packet", XYMARK, 0},
                         {"timeout", XYTIMO, 0},
                         {"", 0, 0}};
int nsrtab = (sizeof(srtab) / sizeof(struct keytab)) - 1;

#ifdef UNICODE
#define UCS_BOM 1
#define UCS_BYT 2
static struct keytab ucstab[] = {
    {"bom", UCS_BOM, 0}, {"byte-order", UCS_BYT, 0}, {"", 0, 0}};
int nucstab = (sizeof(ucstab) / sizeof(struct keytab)) - 1;

static struct keytab botab[] = {{"big-endian", 0, 0}, {"little-endian", 1, 0}};
static int nbotab = 2;
#endif /* UNICODE */

/* REMOTE SET */

struct keytab rmstab[] = {
    {"attributes", XYATTR, 0},
    {"block-check", XYCHKT, 0},
    {"file", XYFILE, 0},
    {"incomplete", XYIFD, CM_INV}, /* = REMOTE SET FILE INCOMPLETE */
    {"match", XYMATCH, 0},
    {"receive", XYRECV, 0},
    {"retry", XYRETR, 0},
    {"server", XYSERV, 0},
    {"transfer", XYXFER, 0},
    {"window", XYWIND, 0},
    {"xfer", XYXFER, CM_INV}};
int nrms = (sizeof(rmstab) / sizeof(struct keytab));

struct keytab attrtab[] = {{"all", AT_XALL, 0},
#ifndef NOCSETS
                           {"character-set", AT_ENCO, 0},
#endif /* NOCSETS */
                           {"date", AT_DATE, 0},
                           {"disposition", AT_DISP, 0},
                           {"encoding", AT_ENCO, CM_INV},
                           {"format", AT_RECF, CM_INV},
                           {"length", AT_LENK, 0},
                           {"off", AT_ALLN, 0},
                           {"on", AT_ALLY, 0},
#ifdef CK_PERMS
                           {"protection", AT_LPRO, 0},
                           {"permissions", AT_LPRO, CM_INV},
#endif /* CK_PERMS */
                           {"record-format", AT_RECF, 0},
                           {"system-id", AT_SYSI, 0},
                           {"type", AT_FTYP, 0}};
int natr = (sizeof(attrtab) / sizeof(struct keytab)); /* how many attributes */

#ifdef CKTIDLE
struct keytab idlacts[] = {{"exit", IDLE_EXIT, 0},
                           {"hangup", IDLE_HANG, 0},
                           {"output", IDLE_OUT, 0},
                           {"return", IDLE_RET, 0},
#ifdef TNCODE
                           {"telnet-nop", IDLE_TNOP, 0},
                           {"telnet-ayt", IDLE_TAYT, 0},
#endif /* TNCODE */
                           {"", 0, 0}};
int nidlacts = (sizeof(idlacts) / sizeof(struct keytab)) - 1;
#endif /* CKTIDLE */

#ifndef NOSPL
extern int indef, inecho, insilence, inbufsize, inautodl, inintr;
#ifdef CKFLOAT
extern CKFLOAT inscale;
#endif /* CKFLOAT */
extern char *inpbuf, *inpbp;
struct keytab inptab[] = {/* SET INPUT parameters */
#ifdef CK_AUTODL
                          {"autodownload", IN_ADL, 0},
#endif /* CK_AUTODL */
                          {"buffer-length", IN_BUF, 0},
                          {"cancellation", IN_CAN, 0},
                          {"case", IN_CAS, 0},
                          {"default-timeout", IN_DEF,
                           CM_INV}, /* There is no default timeout */
                          {"echo", IN_ECH, 0},
                          {"scale-factor", IN_SCA, 0},
                          {"silence", IN_SIL, 0},
                          {"timeout-action", IN_TIM, 0}};
int ninp = (sizeof(inptab) / sizeof(struct keytab));

struct keytab intimt[] = {
    /* SET INPUT TIMEOUT parameters */
    {"proceed", 0, 0}, /* 0 = proceed */
    {"quit", 1, 0}     /* 1 = quit */
};

struct keytab incast[] = {
    /* SET INPUT CASE parameters */
    {"ignore", 0, 0}, /* 0 = ignore */
    {"observe", 1, 0} /* 1 = observe */
};
#endif /* NOSPL */

struct keytab nabltab[] = {/* For any command that needs */
                           {"disabled", 0, 0},
                           {"enabled", 1, 0},
                           {"off", 0, CM_INV}, /* these keywords... */
                           {"on", 1, CM_INV}};
int nnabltab = sizeof(nabltab) / sizeof(struct keytab);

/* The following routines broken out of doprm() to give compilers a break. */

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */

int seton(int *prm) {
  int x, y;
  if ((y = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
    return (y);
  }
  if ((x = cmcfm()) < 0) {
    return (x);
  }
  *prm = y;
  return (1);
}

/*  S E T O N A U T O --  Parse on/off/auto (default auto) & set result */

struct keytab onoffaut[] = {
    {"auto", SET_AUTO, 0}, /* 2 */
    {"off", SET_OFF, 0},   /* 0 */
    {"on", SET_ON, 0}      /* 1 */
};

int setonaut(int *prm) {
  int x, y;
  if ((y = cmkey(onoffaut, 3, "", "auto", xxstring)) < 0) {
    return (y);
  }
  if ((x = cmcfm()) < 0) {
    return (x);
  }
  *prm = y;
  return (1);
}

/*  S E T N U M  --  Set parameter to result of cmnum() parse.  */
/*
 Call with pointer to integer variable to be set,
   x = number from cnum parse, y = return code from cmnum,
   max = maximum value to accept, -1 if no maximum.
 Returns -9 on failure, after printing a message, or 1 on success.
*/
int setnum(int *prm, int x, int y, int max) {
  debug(F101, "setnum", "", y);
  if (y == -3) {
    printf("\n?Value required\n");
    return (-9);
  }
  if (y == -2) {
    printf("%s?Not a number: %s\n", cmflgs == 1 ? "" : "\n", atxbuf);
    return (-9);
  }
  if (y < 0) {
    return (y);
  }
  if (max > -1 && x > max) {
    printf("?Sorry, %d is the maximum\n", max);
    return (-9);
  }
  if ((y = cmcfm()) < 0) {
    return (y);
  }
  *prm = x;
  return (1);
}

/*  S E T C C  --  Set parameter var to an ASCII control character value.  */
/*
  Parses a number, or a literal control character, or a caret (^) followed
  by an ASCII character whose value is 63-95 or 97-122, then gets confirmation,
  then sets the parameter to the code value of the character given.  If there
  are any parse errors, they are returned, otherwise on success 1 is returned.
*/
int setcc(char *dflt, int *var) {
  int x, y;
  unsigned int c;
  char *hlpmsg = "Control character,\n\
 numeric ASCII value,\n\
 or in ^X notation,\n\
 or preceded by a backslash and entered literally";

  /* This is a hack to turn off complaints from expression evaluator. */
  x_ifnum = 1;
  y = cmnum(hlpmsg, dflt, 10, &x, xxstring); /* Parse a number */
  x_ifnum = 0;                               /* Allow complaints again */
  if (y < 0) {                               /* Parse failed */
    if (y != -2) {                           /* Reparse needed or somesuch */
      return (y); /* Pass failure back up the chain */
    }
  }
  /* Real control character or literal 8-bit character... */

  for (c = strlen(atmbuf) - 1; c > 0; c--) { /* Trim */
    if (atmbuf[c] == SP) {
      atmbuf[c] = NUL;
    }
  }

  if (y < 0) {                             /* It was not a number */
    if (((c = atmbuf[0])) && !atmbuf[1]) { /* Literal character? */
      c &= 0xff;
      if (((c > 31) && (c < 127)) || (c > 255)) {
        printf("\n?%d: Out of range - must be 0-31 or 127-255\n", c);
        return (-9);
      } else {
        if ((y = cmcfm()) < 0) { /* Confirm */
          return (y);
        }
        *var = c; /* Set the variable */
        return (1);
      }
    } else if (atmbuf[0] == '^' && !atmbuf[2]) { /* Or ^X notation? */
      c = atmbuf[1];
      if (islower((char)c)) { /* Uppercase lowercase letters */
        c = toupper(c);
      }
      if (c > 62 && c < 96) { /* Check range */
        if ((y = cmcfm()) < 0) {
          return (y);
        }
        *var = ctl(c); /* OK */
        return (1);
      } else {
        printf("?Not a control character - %s\n", atmbuf);
        return (-9);
      }
    } else { /* Something illegal was typed */
      printf("?Not valid here - '%s'\n", atmbuf);
      return (-9);
    }
  }
  if (((x > 31) && (x < 127)) || (x > 255)) { /* They typed a number */
    printf("\n?%d: Out of range - must be 0-31 or 127-255\n", x);
    return (-9);
  }
  if ((y = cmcfm()) < 0) { /* In range, confirm */
    return (y);
  }
  *var = x; /* Set variable */
  return (1);
}

#ifndef NOSPL /* The SORT command... */

static struct keytab srtswtab[] = {/* SORT command switches */
                                   {"/case", SRT_CAS, CM_ARG},
                                   {"/key", SRT_KEY, CM_ARG},
                                   {"/numeric", SRT_NUM, 0},
                                   {"/range", SRT_RNG, CM_ARG},
                                   {"/reverse", SRT_REV, 0}};
static int nsrtswtab = sizeof(srtswtab) / sizeof(struct keytab);

extern char **a_ptr[]; /* Array pointers */
extern int a_dim[];    /* Array dimensions */

int dosort() { /* Do the SORT command */
  char c, *p = NULL, **ap, **xp = NULL;
  struct FDB sw, fl, cm;
  int hi, lo;
  int xn = 0, xr = -1, xk = -1, xc = -1, xs = 0;
  int getval = 0, range[2], confirmed = 0;

  cmfdbi(&sw,                        /* First FDB - command switches */
         _CMKEY,                     /* fcode */
         "Array name or switch", "", /* default */
         "",                         /* addtl string data */
         nsrtswtab,                  /* addtl numeric data 1: tbl size */
         4,                          /* addtl numeric data 2: 4 = cmswi */
         NULL,                       /* Processing function */
         srtswtab,                   /* Keyword table */
         &fl                         /* Pointer to next FDB */
  );
  cmfdbi(&fl,          /* Anything that doesn't match */
         _CMFLD,       /* fcode */
         "Array name", /* hlpmsg */
         "",           /* default */
         "",           /* addtl string data */
         0,            /* addtl numeric data 1 */
         0,            /* addtl numeric data 2 */
         NULL, NULL, &cm);
  cmfdbi(&cm,    /* Or premature confirmation */
         _CMCFM, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         NULL, NULL, NULL);

  range[0] = -1;
  range[1] = -1;

  while (1) { /* Parse 0 or more switches */
    x = cmfdb(&sw);
    if (x < 0) {
      return (x);
    }
    if (cmresult.fcode != _CMKEY) { /* Break out if not a switch */
      break;
    }
    c = cmgbrk();
    getval = (c == ':' || c == '=');
    if (getval && !(cmresult.kflags & CM_ARG)) {
      printf("?This switch does not take arguments\n");
      return (-9);
    }
    switch (cmresult.nresult) {
    case SRT_REV:
      xr = 1;
      break;
    case SRT_KEY:
      if (getval) {
        if ((y = cmnum("Column for comparison (1-based)", "1", 10, &x,
                       xxstring)) < 0) {
          return (y);
        }
        xk = x - 1;
      } else {
        xk = 0;
      }
      break;
    case SRT_CAS:
      if (getval) {
        if ((y = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
          return (y);
        }
        xc = y;
      } else {
        xc = 1;
      }
      break;
    case SRT_RNG: /* /RANGE */
      if (getval) {
        char buf[32];
        char buf2[16];
        int i;
        char *p, *q;
        if ((y = cmfld("low:high element", "1", &s, NULL)) < 0) {
          return (y);
        }
        s = brstrip(s);
        ckstrncpy(buf, s, 32);
        p = buf;
        for (i = 0; *p && i < 2; i++) { /* Get low and high */
          q = p;                        /* Start of this piece */
          while (*p) {                  /* Find end of this piece */
            if (*p == ':') {
              *p = NUL;
              p++;
              break;
            }
            p++;
          }
          y = 15; /* Evaluate this piece */
          s = buf2;
          zzstring(q, &s, &y);
          s = evalx(buf2);
          if (s) {
            if (*s) {
              ckstrncpy(buf2, s, 16);
            }
          }
          if (!rdigits(buf2)) {
            printf("?Not numeric: %s\n", buf2);
            return (-9);
          }
          range[i] = atoi(buf2);
        }
      }
      break;
    case SRT_NUM: /* /NUMERIC */
      xn = 1;
      break;
    default:
      return (-2);
    }
  }
  switch (cmresult.fcode) {
  case _CMCFM:
    confirmed = 1;
    break;
  case _CMFLD:
    ckstrncpy(line, cmresult.sresult, LINBUFSIZ); /* Safe copy of name */
    s = line;
    break;
  default:
    printf("?Unexpected function code: %d\n", cmresult.fcode);
    return (-9);
  }
  if (confirmed) {
    printf("?Array name required\n");
    return (-9);
  }
  ckmakmsg(tmpbuf, TMPBUFSIZ, "Second array to sort according to ", s, NULL,
           NULL);
  if ((x = cmfld(tmpbuf, "", &p, NULL)) < 0) {
    if (x != -3) {
      return (x);
    }
  }
  tmpbuf[0] = NUL;
  ckstrncpy(tmpbuf, p, TMPBUFSIZ);
  p = tmpbuf;
  if ((x = cmcfm()) < 0) { /* Get confirmation */
    return (x);
  }

  x = arraybounds(s, &lo, &hi); /* Get array index & bounds */
  if (x < 0) {                  /* Check */
    printf("?Bad array name: %s\n", s);
    return (-9);
  }
  if (lo > -1) {
    range[0] = lo; /* Set range */
  }
  if (hi > -1) {
    range[1] = hi;
  }
  ap = a_ptr[x]; /* Get pointer to array element list */
  if (!ap) {     /* Check */
    printf("?Array not declared: %s\n", s);
    return (-9);
  }
  if (range[0] < 0) { /* Starting element */
    range[0] = 1;
  }
  if (range[1] < 0) { /* Final element */
    range[1] = a_dim[x];
  }
  if (range[1] > a_dim[x]) {
    printf("?range %d:%d exceeds array dimension %d\n", range[0], range[1],
           a_dim[x]);
    return (-9);
  }
  ap += range[0];
  xs = range[1] - range[0] + 1; /* Number of elements to sort */
  if (xs < 1) {                 /* Check */
    printf("?Bad range: %d:%d\n", range[0], range[1]);
    return (-9);
  }
  if (xk < 0) {
    xk = 0; /* Key position */
  }
  if (xr < 0) {
    xr = 0; /* Reverse flag */
  }
  if (xn) { /* Numeric flag */
    xc = 2;
  } else if (xc < 0) {   /* Not numeric */
    xc = inpcas[cmdlvl]; /* so alpha case option */
  }

  if (*p) {        /* Parallel array given? */
    y = xarray(p); /* Yes, get its index. */
    if (y < 0) {
      printf("?Bad array name: %s\n", p);
      return (-9);
    }
    if (y != x) {    /* If the 2 arrays are different  */
      xp = a_ptr[y]; /* Pointer to 2nd array element list */
      if (!xp) {
        printf("?Array not declared: %s\n", p);
        return (-9);
      }
      if (a_dim[y] < range[1]) {
        printf("?Array %s smaller than %s\n", p, s);
        return (-9);
      }
      xp += range[0]; /* Set base to same as 1st array */
    }
  }
  sh_sort(ap, xp, xs, xk, xr, xc); /* Sort the array(s) */
  return (success = 1);            /* Always succeeds */
}
#endif /* NOSPL */

#ifdef CKPURGE
static struct keytab purgtab[] = {/* PURGE command switches */
                                  {"/after", PU_AFT, CM_ARG},
                                  {"/ask", PU_ASK, 0},
                                  {"/before", PU_BEF, CM_ARG},
                                  {"/delete", PU_DELE, CM_INV},
                                  {"/dotfiles", PU_DOT, 0},
                                  {"/except", PU_EXC, CM_ARG},
                                  {"/heading", PU_HDG, 0},
                                  {"/keep", PU_KEEP, CM_ARG},
                                  {"/larger-than", PU_LAR, CM_ARG},
                                  {"/list", PU_LIST, 0},
                                  {"/log", PU_LIST, CM_INV},
                                  {"/noask", PU_NASK, 0},
                                  {"/nodelete", PU_NODE, CM_INV},
                                  {"/nodotfiles", PU_NODOT, 0},
                                  {"/noheading", PU_NOH, 0},
                                  {"/nol", PU_NOLI, CM_INV | CM_ABR},
                                  {"/nolist", PU_NOLI, 0},
                                  {"/nolog", PU_NOLI, CM_INV},
#ifdef CK_TTGWSIZ
                                  {"/nopage", PU_NOPA, 0},
#endif /* CK_TTGWSIZ */
                                  {"/not-after", PU_NAF, CM_ARG},
                                  {"/not-before", PU_NBF, CM_ARG},
                                  {"/not-since", PU_NAF, CM_INV | CM_ARG},
#ifdef CK_TTGWSIZ
                                  {"/page", PU_PAGE, 0},
#endif /* CK_TTGWSIZ */
                                  {"/quiet", PU_QUIE, CM_INV},
#ifdef RECURSIVE
                                  {"/recursive", PU_RECU, 0},
#endif /* RECURSIVE */
                                  {"/since", PU_AFT, CM_ARG | CM_INV},
                                  {"/simulate", PU_NODE, 0},
                                  {"/smaller-than", PU_SMA, CM_ARG},
                                  {"/verbose", PU_VERB, CM_INV}};
static int npurgtab = sizeof(purgtab) / sizeof(struct keytab);
#endif /* CKPURGE */

int bkupnum(char *s, int *i) {
  int k = 0, pos = 0;
  char *p = NULL, *q;
  *i = pos;
  if (!s) {
    s = "";
  }
  if (!*s) {
    return (-1);
  }
  if ((k = strlen(s)) < 5) {
    return (-1);
  }

  if (s[k - 1] != '~') {
    return (-1);
  }
  pos = k - 2;
  q = s + pos;
  while (q >= s && isdigit(*q)) {
    p = q--;
    pos--;
  }
  if (!p) {
    return (-1);
  }
  if (q < s + 2) {
    return (-1);
  }
  if (*q != '~' || *(q - 1) != '.') {
    return (-1);
  }
  pos--;
  *i = pos;
  debug(F111, "bkupnum", s + pos, pos);
  return (atoi(p));
}

#ifdef CKPURGE
/* Presently only for UNIX because we need direct access to the file array. */
/* Not needed for VMS anyway, because we don't make backup files there. */

#define MAXKEEP 32 /* Biggest /KEEP: value */

static int pu_keep = 0, pu_list = 0, pu_dot = 0, pu_ask = 0, pu_hdg = 0;

#ifdef CK_TTGWSIZ
static int pu_page = -1;
#else
static int pu_page = 0;
#endif /* CK_TTGWSIZ */

#ifndef NOSHOW
void showpurgopts() { /* SHOW PURGE command options */
  int x = 0;
  extern int optlines;
  prtopt(&optlines, "PURGE");
  if (pu_ask > -1) {
    x++;
    prtopt(&optlines, pu_ask ? "/ASK" : "/NOASK");
  }
  if (pu_dot > -1) {
    x++;
    prtopt(&optlines, pu_dot ? "/DOTFILES" : "/NODOTFILES");
  }
  if (pu_keep > -1) {
    x++;
    ckmakmsg(tmpbuf, TMPBUFSIZ, "/KEEP:", ckitoa(pu_keep), NULL, NULL);
    prtopt(&optlines, tmpbuf);
  }
  if (pu_list > -1) {
    x++;
    prtopt(&optlines, pu_list ? "/LIST" : "/NOLIST");
  }
  if (pu_hdg > -1) {
    x++;
    prtopt(&optlines, pu_hdg ? "/HEADING" : "/NOHEADING");
  }
#ifdef CK_TTGWSIZ
  if (pu_page > -1) {
    x++;
    prtopt(&optlines, pu_page ? "/PAGE" : "/NOPAGE");
  }
#endif /* CK_TTGWSIZ */
  if (!x) {
    prtopt(&optlines, "(no options set)");
  }
  prtopt(&optlines, "");
}
#endif /* NOSHOW */

int setpurgopts() { /* Set PURGE command options */
  int c, z, getval = 0;
  int x_keep = -1, x_list = -1, x_page = -1, x_hdg = -1, x_ask = -1, x_dot = -1;

  while (1) {
    if ((y = cmswi(purgtab, npurgtab, "Switch", "", xxstring)) < 0) {
      if (y == -3) {
        break;
      } else {
        return (y);
      }
    }
    c = cmgbrk();
    if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
      printf("?This switch does not take an argument\n");
      return (-9);
    }
    if (!getval && (cmgkwflgs() & CM_ARG)) {
      printf("?This switch requires an argument\n");
      return (-9);
    }
    switch (y) {
    case PU_KEEP:
      z = 1;
      if (c == ':' || c == '=') {
        if ((y = cmnum("How many backup files to keep", "1", 10, &z,
                       xxstring)) < 0) {
          return (y);
        }
      }
      if (z < 0 || z > MAXKEEP) {
        printf("?Please specify a number between 0 and %d\n", MAXKEEP);
        return (-9);
      }
      x_keep = z;
      break;
    case PU_LIST:
    case PU_VERB:
      x_list = 1;
      break;
    case PU_QUIE:
    case PU_NOLI:
      x_list = 0;
      break;
#ifdef CK_TTGWSIZ
    case PU_PAGE:
      x_page = 1;
      break;
    case PU_NOPA:
      x_page = 0;
      break;
#endif /* CK_TTGWSIZ */
    case PU_HDG:
      x_hdg = 1;
      break;
    case PU_NOH:
      x_hdg = 0;
      break;
    case PU_ASK:
      x_ask = 1;
      break;
    case PU_NASK:
      x_ask = 0;
      break;
    case PU_DOT:
      x_dot = 1;
      break;
    case PU_NODOT:
      x_dot = 0;
      break;
    default:
      printf("?This option can not be set\n");
      return (-9);
    }
  }
  if ((x = cmcfm()) < 0) { /* Get confirmation */
    return (x);
  }
  if (x_keep > -1) { /* Set PURGE defaults. */
    pu_keep = x_keep;
  }
  if (x_list > -1) {
    pu_list = x_list;
  }
#ifdef CK_TTGWSIZ
  if (x_page > -1) {
    pu_page = x_page;
  }
#endif /* CK_TTGWSIZ */
  if (x_hdg > -1) {
    pu_hdg = x_hdg;
  }
  if (x_ask > -1) {
    pu_ask = x_ask;
  }
  if (x_dot > -1) {
    pu_dot = x_dot;
  }
  return (success = 1);
}

int dopurge() { /* Do the PURGE command */
  extern char **mtchs;
  extern int xaskmore, cmd_rows, recursive;
  int simulate = 0, asking = 0;
  int listing = 0, paging = -1, lines = 0, deleting = 1, errors = 0;
  struct FDB sw, sf, cm;
  int g, i, j, k, m = 0, n, x, y, z, done = 0, count = 0, flags = 0;
  int tokeep = 0, getval = 0, havename = 0, confirmed = 0;
  int xx[MAXKEEP + 1]; /* Array of numbers to keep */
  int min = -1;
  int x_hdg = 0, fs = 0, rc = 0;
  CK_OFF_T minsize = -1L, maxsize = -1L;
  char namebuf[CKMAXPATH + 4];
  char basebuf[CKMAXPATH + 4];
  char *pu_aft = NULL, *pu_bef = NULL, *pu_naf = NULL, *pu_nbf = NULL,
       *pu_exc = NULL;
  char *pxlist[8]; /* Exception list */

  if (pu_keep > -1) { /* Set PURGE defaults. */
    tokeep = pu_keep;
  }
  if (pu_list > -1) {
    listing = pu_list;
  }
#ifdef CK_TTGWSIZ
  if (pu_page > -1) {
    paging = pu_page;
  }
#endif /* CK_TTGWSIZ */

  for (i = 0; i <= MAXKEEP; i++) { /* Clear this number buffer */
    xx[i] = 0;
  }
  for (i = 0; i < 8; i++) { /* Initialize these... */
    pxlist[i] = NULL;
  }

  g_matchdot = matchdot; /* Save these... */

  cmfdbi(&sw,                  /* 1st FDB - PURGE switches */
         _CMKEY,               /* fcode */
         "Filename or switch", /* hlpmsg */
         "",                   /* default */
         "",                   /* addtl string data */
         npurgtab,             /* addtl numeric data 1: tbl size */
         4,                    /* addtl numeric data 2: 4 = cmswi */
         xxstring,             /* Processing function */
         purgtab,              /* Keyword table */
         &sf                   /* Pointer to next FDB */
  );
  cmfdbi(&sf,    /* 2nd FDB - filespec to purge */
         _CMIFI, /* fcode */
         "", "", /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         xxstring, NULL, &cm);
  cmfdbi(&cm,    /* Or premature confirmation */
         _CMCFM, /* fcode */
         "",     /* hlpmsg */
         "",     /* default */
         "",     /* addtl string data */
         0,      /* addtl numeric data 1 */
         0,      /* addtl numeric data 2 */
         NULL, NULL, NULL);

  while (!havename && !confirmed) {
    x = cmfdb(&sw); /* Parse something */
    if (x < 0) {    /* Error */
      rc = x;
      goto xpurge;
    } else if (cmresult.fcode == _CMKEY) {
      char c;
      c = cmgbrk();
      if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
        printf("?This switch does not take an argument\n");
        rc = -9;
        goto xpurge;
      }
      if (!getval && (cmgkwflgs() & CM_ARG)) {
        printf("?This switch requires an argument\n");
        rc = -9;
        goto xpurge;
      }
      switch (k = cmresult.nresult) {
      case PU_KEEP:
        z = 1;
        if (c == ':' || c == '=') {
          if ((y = cmnum("How many backup files to keep", "1", 10, &z,
                         xxstring)) < 0) {
            rc = y;
            goto xpurge;
          }
        }
        if (z < 0 || z > MAXKEEP) {
          printf("?Please specify a number between 0 and %d\n", MAXKEEP);
          rc = -9;
          goto xpurge;
        }
        tokeep = z;
        break;
      case PU_LIST:
        listing = 1;
        break;
      case PU_NOLI:
        listing = 0;
        break;
#ifdef CK_TTGWSIZ
      case PU_PAGE:
        paging = 1;
        break;
      case PU_NOPA:
        paging = 0;
        break;
#endif /* CK_TTGWSIZ */
      case PU_DELE:
        deleting = 1;
        break;
      case PU_NODE:
        deleting = 0;
        simulate = 1;
        listing = 1;
        break;
      case PU_ASK:
        asking = 1;
        break;
      case PU_NASK:
        asking = 0;
        break;
      case PU_AFT:
      case PU_BEF:
      case PU_NAF:
      case PU_NBF:
        if ((x = cmdate("File-time", "", &s, 0, xxstring)) < 0) {
          if (x == -3) {
            printf("?Date-time required\n");
            rc = -9;
          } else {
            rc = x;
          }
          goto xpurge;
        }
        fs++;
        switch (k) {
        case PU_AFT:
          makestr(&pu_aft, s);
          break;
        case PU_BEF:
          makestr(&pu_bef, s);
          break;
        case PU_NAF:
          makestr(&pu_naf, s);
          break;
        case PU_NBF:
          makestr(&pu_nbf, s);
          break;
        }
        break;
      case PU_SMA:
      case PU_LAR:
        if ((x = cmnum("File size in bytes", "0", 10, &y, xxstring)) < 0) {
          rc = x;
          goto xpurge;
        }
        fs++;
        switch (cmresult.nresult) {
        case PU_SMA:
          minsize = y;
          break;
        case PU_LAR:
          maxsize = y;
          break;
        }
        break;
      case PU_DOT:
        matchdot = 1;
        break;
      case PU_NODOT:
        matchdot = 0;
        break;
      case PU_EXC:
        if ((x = cmfld("Pattern", "", &s, xxstring)) < 0) {
          if (x == -3) {
            printf("?Pattern required\n");
            rc = -9;
          } else {
            rc = x;
          }
          goto xpurge;
        }
        fs++;
        makestr(&pu_exc, s);
        break;
      case PU_HDG:
        x_hdg = 1;
        break;
#ifdef RECURSIVE
      case PU_RECU: /* /RECURSIVE */
        recursive = 2;
        break;
#endif /* RECURSIVE */
      default:
        printf("?Not implemented yet - \"%s\"\n", atmbuf);
        rc = -9;
        goto xpurge;
      }
    } else if (cmresult.fcode == _CMIFI) {
      havename = 1;
    } else if (cmresult.fcode == _CMCFM) {
      confirmed = 1;
    } else {
      rc = -2;
      goto xpurge;
    }
  }
  if (havename) {
#ifdef CKREGEX
    ckmakmsg(line, LINBUFSIZ, cmresult.sresult, ".~[1-9]*~", NULL, NULL);
#else
    ckmakmsg(line, LINBUFSIZ, cmresult.sresult, ".~*~", NULL, NULL);
#endif /* CKREGEX */
  } else {
#ifdef CKREGEX
    ckstrncpy(line, "*.~[1-9]*~", LINBUFSIZ);
#else
    ckstrncpy(line, "*.~*~", LINBUFSIZ);
#endif /* CKREGEX */
  }
  if (!confirmed) {
    if ((x = cmcfm()) < 0) {
      rc = x;
      goto xpurge;
    }
  }
  /* Parse finished - now action */

#ifdef CK_LOGIN
  if (isguest) {
    printf("?File deletion by guests not permitted.\n");
    rc = -9;
    goto xpurge;
  }
#endif /* CK_LOGIN */

#ifdef CK_TTGWSIZ
  if (paging < 0) {    /* /[NO]PAGE not given */
    paging = xaskmore; /* so use prevailing */
  }
#endif /* CK_TTGWSIZ */

  lines = 0;
  if (x_hdg > 0) {
    printf("Purging %s, keeping %d...%s\n", s, tokeep,
           simulate ? " (SIMULATION)" : "");
    lines += 2;
  }
  flags = ZX_FILONLY;
  if (recursive) {
    flags |= ZX_RECURSE;
  }
  n = nzxpand(line, flags); /* Get list of backup files */
  if (tokeep < 1) {         /* Deleting all of them... */
    for (i = 0; i < n; i++) {
      if (fs) {
        if (fileselect(mtchs[i], pu_aft, pu_bef, pu_naf, pu_nbf, minsize,
                       maxsize, 0, 8, pxlist) < 1) {
          if (listing > 0) {
            printf(" %s (SKIPPED)\n", mtchs[i]);
#ifdef CK_TTGWSIZ
            if (paging) {
              if (++lines > cmd_rows - 3) {
                if (!askmore()) {
                  goto xpurge;
                } else {
                  lines = 0;
                }
              }
            }
#endif /* CK_TTGWSIZ */
          }
          continue;
        }
      }
      if (asking) {
        int x;
        ckmakmsg(tmpbuf, TMPBUFSIZ, " Delete ", mtchs[i], "?", NULL);
        x = getyesno(tmpbuf, 1);
        switch (x) {
        case 0:
          continue;
        case 1:
          break;
        case 2:
          goto xpurge;
        }
      }
      x = deleting ? zdelet(mtchs[i]) : 0;
      if (x > -1) {
        if (listing) {
          printf(" %s (%s)\n", mtchs[i], deleting ? "OK" : "SELECTED");
        }
        count++;
      } else {
        errors++;
        if (listing) {
          printf(" %s (FAILED)\n", mtchs[i]);
        }
      }
#ifdef CK_TTGWSIZ
      if (listing && paging) {
        if (++lines > cmd_rows - 3) {
          if (!askmore()) {
            goto xpurge;
          } else {
            lines = 0;
          }
        }
      }
#endif /* CK_TTGWSIZ */
    }
    goto xpurge;
  }
  if (n < tokeep) { /* Not deleting any */
    count = 0;
    if (listing) {
      printf(" Matches = %d: Not enough to purge.\n", n);
    }
    goto xpurge;
  }

  /* General case - delete some but not others */

  sh_sort(mtchs, NULL, n, 0, 0,
          filecase); /* Alphabetize the list (ESSENTIAL) */

  g = 0;                                      /* Start of current group */
  for (i = 0; i < n; i++) {                   /* Go thru sorted file list */
    x = znext(namebuf);                       /* Get next file */
    if (x < 1 || !namebuf[0] || i == n - 1) { /* No more? */
      done = 1; /* NOTE: 'done' must be 0 or 1 only */
    }
    if (fs) {
      if (fileselect(namebuf, pu_aft, pu_bef, pu_naf, pu_nbf, minsize, maxsize,
                     0, 8, pxlist) < 1) {
        if (listing > 0) {
          printf(" %s (SKIPPED)\n", namebuf);
          if (++lines > cmd_rows - 3) {
            if (!askmore()) {
              goto xpurge;
            } else {
              lines = 0;
            }
          }
        }
        continue;
      }
    }
    if (x > 0) {
      if ((m = bkupnum(namebuf, &z)) < 0) { /* This file's backup number. */
        continue;
      }
    }
    for (j = 0; j < tokeep; j++) { /* Insert in list. */
      if (m > xx[j]) {
        for (k = tokeep - 1; k > j; k--) {
          xx[k] = xx[k - 1];
        }
        xx[j] = m;
        break;
      }
    }
    /* New group? */
    if (done || (i > 0 && ckstrcmp(namebuf, basebuf, z, 1))) {
      if (i + done - g > tokeep) { /* Do we have enough to purge? */
        min = xx[tokeep - 1];      /* Yes, lowest backup number to keep */
        debug(F111, "dopurge group", basebuf, min);
        for (j = g; j < i + done; j++) { /* Go through this group */
          x = bkupnum(mtchs[j], &z);     /* Get file backup number */
          if (x > 0 && x < min) {        /* Below minimum? */
            x = deleting ? zdelet(mtchs[j]) : 0;
            if (x < 0) {
              errors++;
            }
            if (listing) {
              printf(" %s (%s)\n", mtchs[j],
                     ((x < 0) ? "ERROR" : (deleting ? "DELETED" : "SELECTED")));
            }
            count++;
          } else if (listing) { /* Not below minimum - keep this one */
            printf(" %s (KEPT)\n", mtchs[j]);
          }
#ifdef CK_TTGWSIZ
          if (listing && paging) {
            if (++lines > cmd_rows - 3) {
              if (!askmore()) {
                goto xpurge;
              } else {
                lines = 0;
              }
            }
          }
#endif /* CK_TTGWSIZ */
        }
      } else if (listing && paging) { /* Not enough to purge */
        printf(" %s.~*~ (KEPT)\n", basebuf);
#ifdef CK_TTGWSIZ
        if (++lines > cmd_rows - 3) {
          if (!askmore()) {
            goto xpurge;
          } else {
            lines = 0;
          }
        }
#endif /* CK_TTGWSIZ */
      }
      for (j = 0; j < tokeep; j++) { /* Clear the backup number list */
        xx[j] = 0;
      }
      g = i; /* Reset the group pointer */
    }
    if (done) { /* No more files, done. */
      break;
    }
    strncpy(basebuf, namebuf, z); /* Set basename of this file */
    basebuf[z] = NUL;
  }
xpurge: /* Common exit point */
  if (g_matchdot > -1) {
    matchdot = g_matchdot; /* Restore these... */
    g_matchdot = -1;
  }
  if (rc < 0) {
    return (rc); /* Parse error */
  }
  if (x_hdg) {
    printf("Files purged: %d%s\n", count, deleting ? "" : " (not really)");
  }
  return (success = count > 0 ? 1 : (errors > 0) ? 0 : 1);
}
#endif /* CKPURGE */

#ifndef NOXFER
#ifndef NOLOCAL
int doxdis(int which) /* 1 = Kermit, 2 = FTP */
{
  extern int nolocal;
  int x, y = 0, z;
#ifdef NEWFTP
  extern int ftp_dis;
#endif /* NEWFTP */

  if ((x = cmkey(fdtab, nfdtab, "file transfer display style", "", xxstring)) <
      0) {
    return (x);
  }
#ifdef CK_PCT_BAR
  if ((y = cmkey(fdftab, 2, "", "thermometer", xxstring)) < 0) {
    return (y);
  }
#endif /* CK_PCT_BAR */
  if ((z = cmcfm()) < 0) {
    return (z);
  }
#ifdef CK_CURSES
  if (x == XYFD_C) { /* FULLSCREEN */

    if (nolocal) { /* Nothing to do in this case */
      return (success = 1);
    }

    fxdinit(x);

#ifdef CK_PCT_BAR
    thermometer = y;
#endif /* CK_PCT_BAR */

    line[0] = '\0'; /* (What's this for?) */
  }
#endif              /* CK_CURSES */
  if (which == 1) { /* It's OK. */
    fdispla = x;
  }
#ifdef NEWFTP
  else if (which == 2) {
    ftp_dis = x;
  }
#endif /* NEWFTP */
  return (success = 1);
}
#endif /* NOLOCAL */
#endif /* NOXFER */

int setfil(int rmsflg) {
#ifndef NOXFER
  if (rmsflg) {
    if ((y = cmkey(rfiltab, nrfilp, "Remote file parameter", "", xxstring)) <
        0) {
      if (y == -3) {
        printf("?Remote file parameter required\n");
        return (-9);
      } else {
        return (y);
      }
    }
  } else {
#endif /* NOXFER */
    if ((y = cmkey(filtab, nfilp, "File parameter", "", xxstring)) < 0) {
      return (y);
    }
#ifndef NOXFER
  }
#endif /* NOXFER */
  switch (y) {

#ifndef NOXFER
  case XYFILS: /* Byte size */
    if ((y = cmnum("file byte size (7 or 8)", "8", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 7 && z != 8) {
      printf("\n?The choices are 7 and 8\n");
      return (0);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    if (z == 7) {
      fmask = 0177;
    } else if (z == 8) {
      fmask = 0377;
    }
    return (success = 1);

#ifndef NOCSETS
  case XYFILC: { /* Character set */
    char *csetname = NULL;
    extern int r_cset, s_cset, afcset[]; /* SEND CHARACTER-SET AUTO or MANUAL */

    struct FDB kw, fl;
    cmfdbi(&kw,    /* First FDB - command switches */
           _CMKEY, /* fcode */
           rmsflg ? "server character-set name" : "", /* help */
           "",                                        /* default */
           "",                                        /* addtl string data */
           nfilc,              /* addtl numeric data 1: tbl size */
           0,                  /* addtl numeric data 2: 0 = keyword */
           xxstring,           /* Processing function */
           fcstab,             /* Keyword table */
           rmsflg ? &fl : NULL /* Pointer to next FDB */
    );
    cmfdbi(&fl,    /* Anything that doesn't match */
           _CMFLD, /* fcode */
           "",     /* hlpmsg */
           "",     /* default */
           "",     /* addtl string data */
           0,      /* addtl numeric data 1 */
           0,      /* addtl numeric data 2 */
           xxstring, NULL, NULL);
    if ((x = cmfdb(&kw)) < 0) {
      return (x);
    }
    if (cmresult.fcode == _CMKEY) {
      x = cmresult.nresult;
      csetname = fcsinfo[x].keyword;
    } else {
      ckstrncpy(line, cmresult.sresult, LINBUFSIZ);
      csetname = line;
    }
    if ((z = cmcfm()) < 0) {
      return (z);
    }
    if (rmsflg) {
      sstate = setgen('S', "320", csetname, "");
      return ((int)sstate);
    }
    fcharset = x;
    if (s_cset == XMODE_A) { /* If SEND CHARACTER-SET is AUTO */
      if (x > -1 && x <= MAXFCSETS) {
        if (afcset[x] > -1 && afcset[x] <= MAXTCSETS) {
          tcharset = afcset[x]; /* Pick corresponding xfer charset */
        }
      }
    }
    setxlatype(tcharset, fcharset); /* Translation type */
    /* If I say SET FILE CHARACTER-SET blah, I want to be blah! */
    r_cset = XMODE_M;           /* Don't switch incoming set! */
    x = fcsinfo[fcharset].size; /* Also set default x-bit charset */
    if (x == 128) {             /* 7-bit... */
      dcset7 = fcharset;
    } else if (x == 256) { /* 8-bit... */
      dcset8 = fcharset;
    }
    return (success = 1);
  }
#endif /* NOCSETS */

#ifndef NOLOCAL
  case XYFILD:          /* Display */
    return (doxdis(1)); /* 1 == kermit */
#endif                  /* NOLOCAL */
#endif                  /* NOXFER */

  case XYFILA: /* End-of-line */
#ifdef NLCHAR
    s = "";
    if (NLCHAR == 015) {
      s = "cr";
    } else if (NLCHAR == 012) {
      s = "lf";
    }
    if ((x = cmkey(eoltab, neoltab, "local text-file line terminator", s,
                   xxstring)) < 0) {
      return (x);
    }
#else
    if ((x = cmkey(eoltab, neoltab, "local text-file line terminator", "crlf",
                   xxstring)) < 0) {
      return (x);
    }
#endif /* NLCHAR */
    if ((z = cmcfm()) < 0) {
      return (z);
    }
    feol = (CHAR)x;
    return (success = 1);

#ifndef NOXFER
  case XYFILN: /* Names */
    if ((x = cmkey(fntab, nfntab, "how to handle filenames", "converted",
                   xxstring)) < 0) {
      return (x);
    }
    if ((z = cmcfm()) < 0) {
      return (z);
    }
    if (rmsflg) {
      sstate = setgen('S', "301", ckitoa(1 - x), "");
      return ((int)sstate);
    } else {
      ptab[protocol].fncn = x; /* Set structure */
      fncnv = x;               /* Set variable */
      f_save = x;              /* And set "permanent" variable */
      return (success = 1);
    }

  case XYFILR: /* Record length */
    if ((y = cmnum("file record length", ckitoa(DLRECL), 10, &z, xxstring)) <
        0) {
      return (y);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (rmsflg) {
      sstate = setgen('S', "312", ckitoa(z), "");
      return ((int)sstate);
    } else {
      frecl = z;
      return (success = 1);
    }

#endif /* NOXFER */

  case XYFILT: /* Type */
    if ((x = cmkey(rmsflg ? rfttab : fttab, rmsflg ? nrfttyp : nfttyp,
                   "type of file transfer", "text", xxstring)) < 0) {
      return (x);
    }

    if ((y = cmcfm()) < 0) {
      return (y);
    }
    binary = x;
    b_save = x;
#ifndef NOXFER
    if (rmsflg) {
      /* Allow for LABELED in VMS & OS/2 */
      sstate = setgen('S', "300", ckitoa(x), "");
      return ((int)sstate);
    } else {
#endif /* NOXFER */
      return (success = 1);
#ifndef NOXFER
    }
#endif /* NOXFER */

#ifndef NOXFER
  case XYFILX: /* Collision Action */
    if ((x = cmkey(colxtab, ncolx, "Filename collision action", "backup",
                   xxstring)) < 0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
#ifdef CK_LOGIN
    if (isguest) {
      /* Don't let guests change existing files */
      printf("?This command not valid for guests\n");
      return (-9);
    }
#endif /* CK_LOGIN */
    fncact = x;
    ptab[protocol].fnca = x;
    if (rmsflg) {
      sstate = setgen('S', "302", ckitoa(fncact), "");
      return ((int)sstate);
    } else {
      if (fncact == XYFX_R) {
        ckwarn = 1; /* FILE WARNING implications */
      }
      if (fncact == XYFX_X) {
        ckwarn = 0; /* ... */
      }
      return (success = 1);
    }

  case XYFILW: /* Warning/Write-Protect */
    if ((x = seton(&ckwarn)) < 0) {
      return (x);
    }
    if (ckwarn) {
      fncact = XYFX_R;
    } else {
      fncact = XYFX_X;
    }
    return (success = 1);

#ifdef CK_LABELED
  case XYFILL: /* LABELED FILE parameters */
    if ((x = cmkey(lbltab, nlblp, "Labeled file feature", "", xxstring)) < 0) {
      return (x);
    }
    if ((success = seton(&y)) < 0) {
      return (success);
    }
    if (y) {        /* Set or reset the selected bit */
      lf_opts |= x; /* in the options bitmask. */
    } else {
      lf_opts &= ~x;
    }
    return (success);
#endif /* CK_LABELED */

  case XYFILI: { /* INCOMPLETE */
    extern struct keytab ifdatab[];
    extern int keep;
    if ((y = cmkey(ifdatab, 3, "", "auto", xxstring)) < 0) {
      return (y);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (rmsflg) {
      sstate = setgen('S', "310", y == 0 ? "0" : (y == 1 ? "1" : "2"), "");
      return ((int)sstate);
    } else {
      keep = y;
      return (success = 1);
    }
  }

#ifdef CK_TMPDIR
  case XYFILG: { /* Download directory */
    int x;
    char *s;
#ifdef ZFNQFP
    struct zfnfp *fnp;
#endif /* ZFNQFP */

    if ((x = cmdir("Name of local directory, or carriage return", "", &s,
                   xxstring)) < 0) {
      if (x != -3) {
        return (x);
      }
    }
    debug(F110, "download dir", s, 0);

    if (x == 2) {
      printf("?Wildcards not allowed in directory name\n");
      return (-9);
    }

#ifdef ZFNQFP
    if ((fnp = zfnqfp(s, TMPBUFSIZ - 1, tmpbuf))) {
      if (fnp->fpath) {
        if ((int)strlen(fnp->fpath) > 0) {
          s = fnp->fpath;
        }
      }
    }
    debug(F110, "download zfnqfp", s, 0);
#endif /* ZFNQFP */

    ckstrncpy(line, s, LINBUFSIZ); /* Make a safe copy */
    if ((x = cmcfm()) < 0) {       /* Get confirmation */
      return (x);
    }

#ifdef CK_LOGIN
    if (isguest) {
      /* Don't let guests change existing files */
      printf("?This command not valid for guests\n");
      return (-9);
    }
#endif /* CK_LOGIN */
    x = strlen(s);

    if (x) {
      if ((x < (LINBUFSIZ - 2)) && /* Add trailing dirsep */
          (s[x - 1] != '/')) {     /* if none present.  */
        s[x] = '/';                /* Note that Windows path has */
        s[x + 1] = NUL;            /* been canonicalized to forward */
      } /* slashes at this point. */
      makestr(&dldir, s);
    } else {
      makestr(&dldir, NULL); /* dldir is NULL when not assigned */
    }

    return (success = 1);
  }
#endif /* CK_TMPDIR */
  case XYFILY:
    return (setdest());
#endif /* NOXFER */

#ifdef CK_CTRLZ
  case XYFILV: { /* EOF */
    extern int eofmethod;
    if ((x = cmkey(eoftab, 3, "end-of-file detection method", "", xxstring)) <
        0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    eofmethod = x;
    return (success = 1);
  }
#endif /* CK_CTRLZ */

#ifndef NOXFER
#ifdef UNIX
  case XYFILH: { /* OUTPUT */
    extern int zofbuffer, zobufsize, zofblock;
#ifdef DYNAMIC
    extern char *zoutbuffer;
#endif /* DYNAMIC */

    if ((x = cmkey(zoftab, nzoftab, "output file writing method", "",
                   xxstring)) < 0) {
      return (x);
    }
    if (x == ZOF_BUF || x == ZOF_NBUF) {
      if ((y = cmnum("output buffer size", "32768", 10, &z, xxstring)) < 0) {
        return (y);
      }
      if (z < 1) {
        printf("?Bad size - %d\n", z);
        return (-9);
      }
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    switch (x) {
    case ZOF_BUF:
    case ZOF_NBUF:
      zofbuffer = (x == ZOF_BUF);
      zobufsize = z;
      break;
    case ZOF_BLK:
    case ZOF_NBLK:
      zofblock = (x == ZOF_BLK);
      break;
    }
#ifdef DYNAMIC
    if (zoutbuffer) {
      free(zoutbuffer);
    }
    if (!(zoutbuffer = (char *)malloc(z))) {
      printf("MEMORY ALLOCATION ERROR - FATAL\n");
      doexit(BAD_EXIT, -1);
    } else {
      zobufsize = z;
    }
#else
    if (z <= OBUFSIZE) {
      zobufsize = z;
    } else {
      printf("?Sorry, %d is too big - %d is the maximum\n", z, OBUFSIZE);
      return (-9);
    }
#endif /* DYNAMIC */
    return (success = 1);
  }
#endif /* UNIX */

#ifdef PATTERNS
  case XYFIBP:   /* BINARY-PATTERN */
  case XYFITP: { /* TEXT-PATTERN */
    char *tmp[FTPATTERNS];
    int i, n = 0;
    while (n < FTPATTERNS) {
      tmp[n] = NULL;
      if ((x = cmfld("Pattern", "", &s, xxstring)) < 0) {
        break;
      }
      ckstrncpy(line, s, LINBUFSIZ);
      s = brstrip(line);
      makestr(&(tmp[n++]), s);
    }
    if (x == -3) {
      x = cmcfm();
    }
    for (i = 0; i <= n; i++) {
      if (x > -1) {
        if (y == XYFIBP) {
          makestr(&(binpatterns[i]), tmp[i]);
        } else {
          makestr(&(txtpatterns[i]), tmp[i]);
        }
      }
      free(tmp[i]);
    }
    if (y == XYFIBP) { /* Null-terminate the list */
      makestr(&(binpatterns[i]), NULL);
    } else {
      makestr(&(txtpatterns[i]), NULL);
    }
    return (x);
  }

  case XYFIPA: /* PATTERNS */
    if ((x = setonaut(&patterns)) < 0) {
      return (x);
    }
    return (success = 1);
#endif /* PATTERNS */
#endif /* NOXFER */

#ifdef UNICODE
  case XYFILU: { /* UCS */
    extern int ucsorder, ucsbom, byteorder;
    if ((x = cmkey(ucstab, nucstab, "", "", xxstring)) < 0) {
      return (x);
    }
    switch (x) {
    case UCS_BYT:
      if ((y = cmkey(botab, nbotab, "Byte order",
                     byteorder ? "little-endian" : "big-endian", xxstring)) <
          0) {
        return (y);
      }
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      ucsorder = y;
      return (success = 1);
    case UCS_BOM:
      if ((y = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
        return (y);
      }
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      ucsbom = y;
      return (success = 1);
    default:
      return (-2);
    }
  }
#endif /* UNICODE */

  case XYF_INSP: { /* SCAN (INSPECTION) */
    extern int filepeek, nscanfile;
    if ((x = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
      return (x);
    }
    if (y) {
      if ((y = cmnum("How much to scan", ckitoa(SCANFILEBUF), 10, &z,
                     xxstring)) < 0) {
        return (y);
      }
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    filepeek = x;
    nscanfile = z;
    return (success = 1);
  }

  case XYF_DFLT:
    y = 0;
#ifndef NOCSETS
    if ((y = cmkey(fdfltab, nfdflt, "", "", xxstring)) < 0) {
      return (y);
    }
    if (y == 7 || y == 8) {
      if (y == 7) {
        s = fcsinfo[dcset7].keyword;
      } else {
        s = fcsinfo[dcset8].keyword;
      }
      if ((x = cmkey(fcstab, nfilc, "character-set", s, xxstring)) < 0) {
        return (x);
      }
    }
    ckstrncpy(line, fcsinfo[x].keyword, LINBUFSIZ);
    s = line;
#endif /* NOCSETS */
    if ((z = cmcfm()) < 0) {
      return (z);
    }
    switch (y) {
#ifndef NOCSETS
    case 7:
      if (fcsinfo[x].size != 128) {
        printf("%s - Not a 7-bit set\n", s);
        return (-9);
      }
      dcset7 = x;
      break;
    case 8:
      if (fcsinfo[x].size != 256) {
        printf("%s - Not an 8-bit set\n", s);
        return (-9);
      }
      dcset8 = x;
      break;
#endif /* NOCSETS */
    default:
      return (-2);
    }
    return (success = 1);

#ifndef NOXFER
  case 9997: /* FASTLOOKUPS */
    return (success = seton(&stathack));
#endif /* NOXFER */

#ifdef UNIX
#ifdef DYNAMIC
  case XYF_LSIZ: { /* LISTSIZE */
    int zz;
    y = cmnum("Maximum number of filenames", "", 10, &x, xxstring);
    if ((x = setnum(&zz, x, y, -1)) < 0) {
      return (x);
    }
    if (zsetfil(zz, 3) < 0) {
      printf("?Memory allocation failure\n");
      return (-9);
    }
    return (success = 1);
  }
  case XYF_SSPA: { /* STRINGSPACE */
    int zz;
    y = cmnum("Number of characters for filename list", "", 10, &x, xxstring);
    if ((x = setnum(&zz, x, y, -1)) < 0) {
      return (x);
    }
    if (zsetfil(zz, 1) < 0) {
      printf("?Memory allocation failure\n");
      return (-9);
    }
    return (success = 1);
  }

#endif /* DYNAMIC */
#endif /* UNIX */

  default:
    printf("?unexpected file parameter\n");
    return (-2);
  }
}

#ifdef UNIX
#ifndef NOPUTENV
#ifdef BIGBUFOK
#define NPUTENVS 4096
#else
#define NPUTENVS 128
#endif /* BIGBUFOK */
/* environment variables must be static, not automatic */

static char *putenvs[NPUTENVS]; /* Array of environment var strings */
static int nputenvs = -1;       /* Pointer into array */
/*
  If anyone ever notices the limitation on the number of PUTENVs, the list
  can be made dynamic, we can recycle entries with the same name, etc.
*/
int doputenv(char *s1, char *s2) {
  char *s, *t = tmpbuf; /* Create or alter environment var */

  if (nputenvs == -1) { /* Table not used yet */
    int i;              /* Initialize the pointers */
    for (i = 0; i < NPUTENVS; i++) {
      putenvs[i] = NULL;
    }
    nputenvs = 0;
  }
  if (!s1) {
    return (1); /* Nothing to do */
  }
  if (!*s1) {
    return (1); /* ditto */
  }

  if (ckindex("=", s1, 0, 0, 0)) { /* Does the name contain an '='? */
    printf(                        /* putenv() does not allow this. */
           /* This also catches the 'putenv name=value' case */
           "?PUTENV - Equal sign in variable name - 'help putenv' for info.\n");
    return (-9);
  }
  nputenvs++; /* Point to next free string */

  debug(F111, "doputenv s1", s1, nputenvs);
  debug(F111, "doputenv s2", s2, nputenvs);

  if (nputenvs > NPUTENVS - 1) { /* Notice the end */
    printf("?PUTENV - static buffer space exhausted\n");
    return (-9);
  }
  /* Quotes are not needed but we allow them for familiarity */
  /* but then we strip them, so syntax is same as for Unix shell */

  if (s2) {
    s2 = brstrip(s2);
  } else {
    s2 = (char *)"";
  }
  ckmakmsg(t, TMPBUFSIZ, s1, "=", s2, NULL);
  debug(F111, "doputenv", t, nputenvs);
  (void)makestr(&(putenvs[nputenvs]), t); /* Make a safe permananent copy */
  if (!putenvs[nputenvs]) {
    printf("?PUTENV - memory allocation failure\n");
    return (-9);
  }
  if (putenv(putenvs[nputenvs])) {
    printf("?PUTENV - %s\n", ck_errstr());
    return (-9);
  } else {
    return (success = 1);
  }
}
#endif /* NOPUTENV */
#endif /* UNIX */

int settrmtyp() {
#ifdef UNIX
  extern int fxd_inited;
  x = cmtxt("Terminal type name, case sensitive", "", &s, NULL);
#ifdef NOPUTENV
  success = 1;
#else
  success = doputenv("TERM", s); /* Set the TERM variable */
#ifdef CK_CURSES
  fxd_inited = 0;      /* Force reinitialization of curses database */
  (void)doxdis(0);     /* Re-initialize file transfer display */
  concb((char)escape); /* Fix command terminal */
#endif /* CK_CURSES */
#endif /* NOPUTENV */
  return (success);
#else
  printf("\n Sorry, this version of C-Kermit does not support the SET TERMINAL "
         "TYPE\n");
  printf(" command.  Type \"help set terminal\" for further information.\n");
  return (success = 0);
#endif /* UNIX */
}

#ifndef NOLOCAL

#ifdef CKTIDLE
static char iactbuf[132];

char *getiact() {
  switch (tt_idleact) {
  case IDLE_RET:
    return ("return");
  case IDLE_EXIT:
    return ("exit");
  case IDLE_HANG:
    return ("hangup");
#ifdef TNCODE
  case IDLE_TNOP:
    return ("Telnet NOP");
  case IDLE_TAYT:
    return ("Telnet AYT");
#endif /* TNCODE */

  case IDLE_OUT: {
    int c, k, n;
    char *p, *q;
    k = ckstrncpy(iactbuf, "output ", 132);
    n = k;
    q = &iactbuf[k];
    p = tt_idlestr;
    if (!p) {
      p = "";
    }
    if (!*p) {
      return ("output NUL");
    }
    while ((c = *p++) && n < 131) {
      c &= 0xff;
      if (c == '\\') {
        if (n > 130) {
          break;
        }
        *q++ = '\\';
        *q++ = '\\';
        *q = NUL;
        n += 2;
      } else if ((c > 32 && c < 127) || c > 159) {
        *q++ = c;
        *q = NUL;
        n++;
      } else {
        if (n > (131 - 6)) {
          break;
        }
        sprintf(q, "\\{%d}", c);
        k = strlen(q);
        q += k;
        n += k;
        *q = NUL;
      }
    }
    *q = NUL;
    k = tt_cols;
    if (n > k - 52) {
      n = k - 52;
      iactbuf[n - 2] = '.';
      iactbuf[n - 1] = '.';
      iactbuf[n] = NUL;
    }
    return (iactbuf);
  }
  default:
    return ("unknown");
  }
}
#endif /* CKTIDLE */

#ifndef NOCSETS
void setlclcharset(int x) {
  int i;
  tcsl = y; /* Local character set */
}

void setremcharset(int x, int z) {
  int i;

#ifdef UNICODE
  if (x == TX_TRANSP)
#else            /* UNICODE */
  if (x == FC_TRANSP)
#endif           /* UNICODE */
  {              /* TRANSPARENT? */
    tcsr = tcsl; /* Make both sets the same */
    return;
  }
  tcsr = x; /* Remote character set */
}
#endif /* NOCSETS */

void setcmask(int x) {
  if (x == 7) {
    cmask = 0177;
  } else if (x == 8) {
    cmask = 0377;
    parity = 0;
  }
}

#ifdef CK_AUTODL
void setautodl(int x, int y) {
  autodl = x;
  adl_ask = y;
}
#endif /* CK_AUTODL */

int settrm() {
  int i = 0;
  if ((y = cmkey(trmtab, ntrm, "", "", xxstring)) < 0) {
    return (y);
  }
#ifdef IKSD
  if (inserver) {
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    printf("?Sorry, command disabled.\r\n");
    return (success = 0);
  }
#endif /* IKSD */

  switch (y) {
  case XYTBYT: /* SET TERMINAL BYTESIZE */
    if ((y = cmnum("bytesize for terminal connection", "8", 10, &x, xxstring)) <
        0) {
      return (y);
    }
    if (x != 7 && x != 8) {
      printf("\n?The choices are 7 and 8\n");
      return (success = 0);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    setcmask(x);
    return (success = 1);

  case XYTSO: /* SET TERMINAL LOCKING-SHIFT */
    return (seton(&sosi));

  case XYTNL: /* SET TERMINAL NEWLINE-MODE */
    return (seton(&tnlm));

  case XYTTYP: /* SET TERMINAL TYPE */
    return (settrmtyp());

#ifndef NOCSETS
  case XYTCS: { /* SET TERMINAL CHARACTER-SET */
    int eol;
    /* set terminal character-set <remote> <local> */
    if ((x = cmkey(
#ifdef CKOUNI
             txrtab, ntxrtab,
#else  /* CKOUNI */
             ttcstab, ntermc,
#endif /* CKOUNI */
             "remote terminal character-set", "", xxstring)) < 0)
      return (x);

#ifdef UNICODE
    if (x == TX_TRANSP
#ifdef CKOUNI
        || x == TX_UTF8
#endif /* CKOUNI */
    ) {
      if ((y = cmcfm()) < 0) { /* Confirm the command */
        return (y);
      }
      y = x;
    }
#else  /* UNICODE */
    if (x == FC_TRANSP) {
      if ((y = cmcfm()) < 0) { /* Confirm the command */
        return (y);
      }
      y = x;
    }
#endif /* UNICODE */

    /* Not transparent or UTF8, so get local set to translate it into */
    s = "";
    s = fcsinfo[fcharset].keyword;

    if ((y = cmkey(
#ifdef CKOUNI
             txrtab, ntxrtab,
#else  /* CKOUNI */
             ttcstab, ntermc,
#endif /* CKOUNI */
             "local character-set", s, xxstring)) < 0)
      return (y);

#ifdef UNICODE
    if (y == TX_UTF8) {
      printf("?UTF8 may not be used as a local character set.\r\n");
      return (-9);
    }
#endif /* UNICODE */
    if ((eol = cmcfm()) < 0) {
      return (eol); /* Confirm the command */
    }

    /* End of command parsing - actions begin */
    setlclcharset(y);
    setremcharset(x, z);
    return (success = 1);
  }
#endif /* NOCSETS */

#ifndef NOCSETS
  case XYTLCS: /* SET TERMINAL LOCAL-CHARACTER-SET */
    /* set terminal character-set <local> */
    s = getdcset(); /* Get display character-set name */
    if ((y = cmkey(
#ifdef CKOUNI
             txrtab, ntxrtab,
#else  /* CKOUNI */
             fcstab, nfilc,
#endif /* CKOUNI */
             "local character-set", s, xxstring)) < 0)
      return (y);

#ifdef UNICODE
    if (y == TX_UTF8) {
      printf("?UTF8 may not be used as a local character set.\r\n");
      return (-9);
    }
#endif /* UNICODE */
    if ((z = cmcfm()) < 0) {
      return (z); /* Confirm the command */
    }

    /* End of command parsing - action begins */

    setlclcharset(y);
    return (success = 1);
#endif /* NOCSETS */

#ifndef NOCSETS
#ifdef UNICODE
  case XYTUNI: /* SET TERMINAL UNICODE */
    return (seton(&tt_unicode));
#endif /* UNICODE */

  case XYTRCS: /* SET TERMINAL REMOTE-CHARACTER-SET */
    /* set terminal character-set <remote> <Graphic-set> */
    if ((x = cmkey(
#ifdef CKOUNI
             txrtab, ntxrtab,
#else  /* CKOUNI */
             ttcstab, ntermc,
#endif /* CKOUNI */
             "remote terminal character-set", "", xxstring)) < 0)
      return (x);

#ifdef UNICODE
    if (x == TX_TRANSP
#ifdef CKOUNI
        || x == TX_UTF8
#endif /* CKOUNI */
    ) {
      if ((y = cmcfm()) < 0) { /* Confirm the command */
        return (y);
      }
    }
#else  /* UNICODE */
    if (x == FC_TRANSP) {
      if ((y = cmcfm()) < 0) { /* Confirm the command */
        return (y);
      }
    }
#endif /* UNICODE */
    else {
      if ((y = cmcfm()) < 0) { /* Confirm the command */
        return (y);
      }
    }
    /* Command parsing ends here */

    setremcharset(x, z);
    return (success = 1);
#endif /* NOCSETS */

  case XYTEC: /* SET TERMINAL ECHO */
    if ((x = cmkey(rltab, nrlt, "which side echos during CONNECT", "remote",
                   xxstring)) < 0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
#ifdef NETCONN
    oldplex = x;
#endif /* NETCONN */
    duplex = x;
    return (success = 1);

  case XYTESC: /* SET TERM ESC */
    if ((x = cmkey(nabltab, nnabltab, "", "enabled", xxstring)) < 0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    tt_escape = x;
    return (1);

  case XYTCRD: /* SET TERMINAL CR-DISPLAY */
    if ((x = cmkey(crdtab, 2, "", "normal", xxstring)) < 0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    tt_crd = x;
    return (success = 1);

  case XYTLFD: /* SET TERMINAL LF-DISPLAY */
    if ((x = cmkey(crdtab, 2, "", "normal", xxstring)) < 0) {
      return (x);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    tt_lfd = x;
    return (success = 1);

#ifdef CK_APC
  case XYTAPC:
    if ((y = cmkey(apctab, napctab, "application program command execution", "",
                   xxstring)) < 0) {
      return (y);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (apcactive == APC_LOCAL ||
        (apcactive == APC_REMOTE && !(apcstatus & APC_UNCH))) {
      return (success = 0);
    }
    apcstatus = y;
    return (success = 1);

#ifdef CK_AUTODL
  case XYTAUTODL: /* AUTODOWNLOAD */
    if ((y = cmkey(adltab, nadltab, "Auto-download options", "", xxstring)) <
        0) {
      return (y);
    }
    switch (y) {
    case TAD_ON:
    case TAD_OFF:
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      setautodl(y, 0);
      break;
    case TAD_ASK:
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      setautodl(TAD_ON, 1);
      break;
    case TAD_ERR:
      if ((y = cmkey(adlerrtab, nadlerrtab, "", "", xxstring)) < 0) {
        return (y);
      }
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      adl_err = y;
      break;
    }
    return (success = 1);

#endif /* CK_AUTODL */
#endif /* CK_APC */

#ifdef CKTIDLE
  case XYTIDLE: /* IDLE-SEND */
  case XYTITMO: /* IDLE-TIMEOUT */
    if ((z = cmnum("seconds of idle time to wait, or 0 to disable", "0", 10, &x,
                   xxstring)) < 0) {
      return (z);
    }
    if (y == XYTIDLE) {
      if ((y = cmtxt("string to send, may contain kverbs and variables",
                     "\\v(newline)", &s, xxstring)) < 0) {
        return (y);
      }
      tt_idlesnd_tmo = x;               /* (old) */
      tt_idlelimit = x;                 /* (new) */
      makestr(&tt_idlestr, brstrip(s)); /* (new) */
      tt_idlesnd_str = tt_idlestr;      /* (old) */
      tt_idleact = IDLE_OUT;            /* (new) */
    } else {
      if ((y = cmcfm()) < 0) {
        return (y);
      }
      tt_idlelimit = x;
    }
    return (success = 1);

  case XYTIACT: { /* SET TERM IDLE-ACTION */
    if ((y = cmkey(idlacts, nidlacts, "", "", xxstring)) < 0) {
      return (y);
    }
    if (y == IDLE_OUT) {
      if ((x = cmtxt("string to send, may contain kverbs and variables", "", &s,
                     xxstring)) < 0) {
        return (x);
      }
      makestr(&tt_idlestr, brstrip(s)); /* (new) */
      tt_idlesnd_str = tt_idlestr;      /* (old) */
    } else {
      if ((x = cmcfm()) < 0) {
        return (x);
      }
    }
    tt_idleact = y;
    return (success = 1);
  }
#endif /* CKTIDLE */

  case XYTDEB:     /* TERMINAL DEBUG */
    y = seton(&x); /* Go parse ON or OFF */
    if (y > 0) {   /* Command succeeded? */
      setdebses(x);
    }
    return (y);

  case XYTWID: {
    if ((y = cmnum("number of columns on your screen", "80", 10, &x,
                   xxstring)) < 0) {
      return (y);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    tt_cols = x;
    return (success = 1);
  }

  case XYTHIG:
    if ((y = cmnum("24", "number of rows on your screen", 10, &x, xxstring)) <
        0) {
      return (y);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }

    tt_rows = x;
    return (success = 1);

#ifdef XPRINT
  case XYTPRN: {
    extern int tt_print;
    if ((x = seton(&tt_print)) < 0) {
      return (x);
    }
    return (success = 1);
  }
#endif /* XPRINT */

#ifdef CK_TRIGGER
  case XYTRIGGER:
    if ((y = cmtxt("String to trigger automatic return to command mode", "", &s,
                   xxstring)) < 0) {
      return (y);
    }
    makelist(s, tt_trigger, TRIGGERS);
    return (1);
#endif /* CK_TRIGGER */

  default: /* Shouldn't get here. */
    return (-2);
  }
}

int setbell() {
  int y, x;

  if ((y = cmkey(beltab, nbeltab,
                 "Whether Kermit should ring the terminal bell (beep)", "on",
                 xxstring)) < 0) {
    return (y);
  }

#ifdef IKSD
  if (inserver) {
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    printf("?Sorry, command disabled.\r\n");
    return (success = 0);
  }
#endif /* IKSD */

  switch (y) { /* SET BELL */
  case XYB_NONE:
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    tt_bell = 0;
    break;

  case XYB_AUD:
    /* This lets C-Kermit accept but ignore trailing K95 keywords */
    if ((x = cmtxt("Confirm with carriage return", "", &s, xxstring)) < 0) {
      return (x);
    }
    tt_bell = 1;
    break;
  }
  return (1);
}

#endif /* NOLOCAL */

#ifndef NOXFER
int /* SET SEND/RECEIVE */
setsr(int xx, int rmsflg) {
  if (xx == XYRECV) {
    ckstrncpy(line, "Parameter for inbound packets", LINBUFSIZ);
  } else {
    ckstrncpy(line, "Parameter for outbound packets", LINBUFSIZ);
  }

  if (rmsflg) {
    if ((y = cmkey(rsrtab, nrsrtab, line, "", xxstring)) < 0) {
      if (y == -3) {
        printf("?Remote receive parameter required\n");
        return (-9);
      } else {
        return (y);
      }
    }
  } else {
    if ((y = cmkey(srtab, nsrtab, line, "", xxstring)) < 0) {
      return (y);
    }
  }
  switch (y) {
  case XYQCTL: /* CONTROL-PREFIX */
    if ((x = cmnum("ASCII value of control prefix", "", 10, &y, xxstring)) <
        0) {
      return (x);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if ((y > 32 && y < 63) || (y > 95 && y < 127)) {
      if (xx == XYRECV) {
        ctlq = (CHAR)y; /* RECEIVE prefix, use with caution! */
      } else {
        myctlq = (CHAR)y; /* SEND prefix, OK to change */
      }
      return (success = 1);
    } else {
      printf("?Illegal value for prefix character\n");
      return (-9);
    }

  case XYEOL:
    if ((y = setcc("13", &z)) < 0) {
      return (y);
    }
    if (z > 31) {
      printf("Sorry, the legal values are 0-31\n");
      return (-9);
    }
    if (xx == XYRECV) {
      eol = (CHAR)z;
    } else {
      seol = (CHAR)z;
    }
    return (success = y);

  case XYLEN:
    y = cmnum("Maximum number of characters in a packet", "90", 10, &x,
              xxstring);
    if (xx == XYRECV) { /* Receive... */
      if ((y = setnum(&z, x, y, maxrps)) < 0) {
        return (y);
      }
      if (protocol != PROTO_K) {
        printf("?Sorry, this command does not apply to %s protocol.\n",
               ptab[protocol].p_name);
        printf("Use SET SEND PACKET-LENGTH for XYZMODEM\n");
        return (-9);
      }
      if (z < 10) {
        printf("Sorry, 10 is the minimum\n");
        return (-9);
      }
      if (rmsflg) {
        sstate = setgen('S', "401", ckitoa(z), "");
        return ((int)sstate);
      } else {
        if (protocol == PROTO_K) {
          if (z > MAXRP) {
            z = MAXRP;
          }
          y = adjpkl(z, wslotr, bigrbsiz);
          rpsizf = 1; /* Packet-size override flag fdc 20220917 */
          if (y != z) {
            urpsiz = y;
            if (!xcmdsrc) {
              if (msgflg) {
                printf(" Adjusting receive packet-length to %d for %d window "
                       "slots\n",
                       y, wslotr);
              }
            }
          }
          urpsiz = y;
          ptab[protocol].rpktlen = urpsiz;
          rpsiz = (y > 94) ? 94 : y;
        } else {
#ifdef CK_XYZ
          if ((protocol == PROTO_X || protocol == PROTO_XC) && z != 128 &&
              z != 1024) {
            printf("Sorry, bad packet length for XMODEM.\n");
            printf("Please use 128 or 1024.\n");
            return (-9);
          }
#endif /* CK_XYZ */
          urpsiz = rpsiz = z;
        }
      }
    } else { /* Send... */
      if ((y = setnum(&z, x, y, maxsps)) < 0) {
        return (y);
      }
      if (z < 10) {
        printf("Sorry, 10 is the minimum\n");
        return (-9);
      }
      if (protocol == PROTO_K) {
        if (z > MAXSP) {
          z = MAXSP;
        }
        spsiz = z; /* Set it */
        y = adjpkl(spsiz, wslotr, bigsbsiz);
        if (y != spsiz && !xcmdsrc) {
          if (msgflg) {
            printf("Adjusting packet size to %d for %d window slots\n", y,
                   wslotr);
          }
        }
      } else {
        y = z;
      }
#ifdef CK_XYZ
      if ((protocol == PROTO_X || protocol == PROTO_XC) && z != 128 &&
          z != 1024) {
        printf("Sorry, bad packet length for XMODEM.\n");
        printf("Please use 128 or 1024.\n");
        return (-9);
      }
#endif                            /* CK_XYZ */
      spsiz = spmax = spsizr = y; /* Set it and flag that it was set */
      spsizf = 1;                 /* to allow overriding Send-Init. */
      ptab[protocol].spktflg = spsizf;
      ptab[protocol].spktlen = spsiz;
    }
    if (pflag && protocol == PROTO_K && !xcmdsrc) {
      if (z > 94 && !reliable && msgflg) {
        /* printf("Extended-length packets requested.\n"); */
        if (bctr < 2 && z > 200) {
          printf("\
Remember to SET BLOCK 2 or 3 for long packets.\n");
        }
      }
      if (speed <= 0L) {
        speed = ttgspd();
      }
    }
    return (success = y);

  case XYMARK:
#ifdef DOOMSDAY
    /*
      Printable start-of-packet works for UNIX and VMS only!
    */
    x_ifnum = 1;
    y = cmnum("Code for packet-start character", "1", 10, &x, xxstring);
    x_ifnum = 0;
    if ((y = setnum(&z, x, y, 126)) < 0) {
      return (y);
    }
#else
    if ((y = setcc("1", &z)) < 0) {
      return (y);
    }
#endif /* DOOMSDAY */
    if (xx == XYRECV) {
      stchr = (CHAR)z;
    } else {
      mystch = (CHAR)z;
#ifdef IKS_OPTION
      /* If IKS negotiation in use   */
      if (TELOPT_U(TELOPT_KERMIT) || TELOPT_ME(TELOPT_KERMIT)) {
        tn_siks(KERMIT_SOP); /* Report change to other side */
      }
#endif /* IKS_OPTION */
    }
    return (success = y);

  case XYNPAD: /* PADDING */
    y = cmnum("How many padding characters for inbound packets", "0", 10, &x,
              xxstring);
    if ((y = setnum(&z, x, y, 94)) < 0) {
      return (y);
    }
    if (xx == XYRECV) {
      mypadn = (CHAR)z;
    } else {
      npad = (CHAR)z;
    }
    return (success = y);

  case XYPADC: /* PAD-CHARACTER */
    if ((y = setcc("0", &z)) < 0) {
      return (y);
    }
    if (xx == XYRECV) {
      mypadc = z;
    } else {
      padch = z;
    }
    return (success = y);

  case XYTIMO: /* TIMEOUT */
    if (xx == XYRECV) {
      y = cmnum("Packet timeout interval", ckitoa(URTIME), 10, &x, xxstring);
      if ((y = setnum(&z, x, y, 94)) < 0) {
        return (y);
      }

      if (rmsflg) { /* REMOTE SET RECEIVE TIMEOUT */
        sstate = setgen('S', "402", ckitoa(z), "");
        return ((int)sstate);
      } else {      /* SET RECEIVE TIMEOUT */
        pkttim = z; /*   Value to put in my negotiation */
      } /*   packet for other Kermit to use */

    } else { /* SET SEND TIMEOUT */
#ifdef CK_TIMERS
      extern int rttflg, mintime, maxtime;
      int tmin = 0, tmax = 0;
#endif /* CK_TIMERS */
      y = cmnum("Packet timeout interval", ckitoa(DMYTIM), 10, &x, xxstring);
      if (y == -3) { /* They cancelled a previous */
        x = DMYTIM;  /* SET SEND command, so restore */
        timef = 0;   /* and turn off the override flag */
        y = cmcfm();
      }
#ifdef CK_TIMERS
      if (y < 0) {
        return (y);
      }
      if (x < 0) {
        printf("?Out of range - %d\n", x);
        return (-9);
      }
      if ((z = cmkey(timotab, 2, "", "dynamic", xxstring)) < 0) {
        return (z);
      }
      if (z) {
        if ((y = cmnum("Minimum timeout to allow", "1", 10, &tmin, xxstring)) <
            0) {
          return (y);
        }
        if (tmin < 1) {
          printf("?Out of range - %d\n", tmin);
          return (-9);
        }
        if ((y = cmnum("Maximum timeout to allow", "0", 10, &tmax, xxstring)) <
            0) {
          return (y);
        }
        /* 0 means let Kermit choose, < 0 means no maximum */
      }
      if ((y = cmcfm()) < 0) {
        return (y);
      }
      rttflg = z; /* Round-trip timer flag */
      z = x;
#else
      if ((y = setnum(&z, x, y, 94)) < 0) {
        return (y);
      }
#endif                    /* CK_TIMERS */
      timef = 1;          /* Turn on the override flag */
      timint = rtimo = z; /* Override value for me to use */
#ifdef CK_TIMERS
      if (rttflg) { /* Lower and upper bounds */
        mintime = tmin;
        maxtime = tmax;
      }
#endif /* CK_TIMERS */
    }
    return (success = 1);

  case XYFPATH: /* PATHNAMES */
    if (xx == XYRECV) {
      y = cmkey(rpathtab, nrpathtab, "", "auto", xxstring);
    } else {
      y = cmkey(pathtab, npathtab, "", "off", xxstring);
    }
    if (y < 0) {
      return (y);
    }

    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (xx == XYRECV) { /* SET RECEIVE PATHNAMES */
      fnrpath = y;
      ptab[protocol].fnrp = fnrpath;
    } else { /* SET SEND PATHNAMES */
      fnspath = y;
      ptab[protocol].fnsp = fnspath;
    }
    return (success = 1); /* Note: 0 = ON, 1 = OFF */
    /* In other words, ON = leave pathnames ON, OFF = take them off. */

  case XYPAUS: /* SET SEND/RECEIVE PAUSE */
    y = cmnum("Milliseconds to pause between packets", "0", 10, &x, xxstring);
    if ((y = setnum(&z, x, y, 15000)) < 0) {
      return (y);
    }
    pktpaus = z;
    return (success = 1);

#ifdef CKXXCHAR /* SET SEND/RECEIVE IGNORE/DOUBLE */
  case XYIGN:
  case XYDBL: {
    int i, zz;
    short *p;
    extern short dblt[];
    extern int dblflag, ignflag;

    /* Make space for a temporary copy of the ignore/double table */

    zz = y;
    p = (short *)malloc(256 * sizeof(short));
    if (!p) {
      printf("?Internal error - malloc failure\n");
      return (-9);
    }
    for (i = 0; i < 256; i++) {
      p[i] = dblt[i]; /* Copy current table */
    }

    while (1) { /* Collect a list of numbers */
#ifndef NOSPL
      x_ifnum = 1; /* Turn off complaints from eval() */
#endif             /* NOSPL */
      if ((x = cmnum(zz == XYDBL ? "Character to double"
                                 : "Character to ignore",
                     "", 10, &y, xxstring)) < 0) {
#ifndef NOSPL
        x_ifnum = 0;
#endif                 /* NOSPL */
        if (x == -3) { /* Done */
          break;
        }
        if (x == -2) {
          if (p) {
            free(p);
            p = NULL;
          }
          debug(F110, "SET S/R DOUBLE/IGNORE atmbuf", atmbuf, 0);
          if (!ckstrcmp(atmbuf, "none", 4, 0) ||
              !ckstrcmp(atmbuf, "non", 3, 0) || !ckstrcmp(atmbuf, "no", 2, 0) ||
              !ckstrcmp(atmbuf, "n", 1, 0)) {
            if ((x = cmcfm()) < 0) { /* Get confirmation */
              return (x);
            }
            for (y = 0; y < 256; y++) {
              dblt[y] &= (zz == XYDBL) ? 1 : 2;
            }
            if (zz == XYDBL) {
              dblflag = 0;
            }
            if (zz == XYIGN) {
              ignflag = 0;
            }
            return (success = 1);
          } else {
            printf("?Please specify a number or the word NONE\n");
            return (-9);
          }
        } else {
          free(p);
          p = NULL;
          return (x);
        }
      }
#ifndef NOSPL
      x_ifnum = 0;
#endif /* NOSPL */
      if (y < 0 || y > 255) {
        printf("?Please enter a character code in range 0-255\n");
        free(p);
        p = NULL;
        return (-9);
      }
      p[y] |= (zz == XYDBL) ? 2 : 1;
      if (zz == XYDBL) {
        dblflag = 1;
      }
      if (zz == XYIGN) {
        ignflag = 1;
      }
    } /* End of while loop */

    if ((x = cmcfm()) < 0) {
      return (x);
    }
    /*
      Get here only if they have made no mistakes.  Copy temporary table back to
      permanent one, then free temporary table and return successfully.
    */
    if (p) {
      for (i = 0; i < 256; i++) {
        dblt[i] = p[i];
      }
      free(p);
      p = NULL;
    }
    return (success = 1);
  }
#endif /* CKXXCHAR */

#ifdef PIPESEND
  case XYFLTR: { /* SET { SEND, RECEIVE } FILTER */
    if ((y = cmtxt((xx == XYSEND) ? "Filter program for sending files -\n\
 use \\v(filename) to substitute filename"
                                  : "Filter program for receiving files -\n\
 use \\v(filename) to substitute filename",
                   "", &s, NULL)) < 0) {
      return (y);
    }
    if (!*s) { /* Removing a filter... */
      if (xx == XYSEND && sndfilter) {
        makestr(&g_sfilter, NULL);
        makestr(&sndfilter, NULL);
      } else if (rcvfilter) {
        makestr(&g_rfilter, NULL);
        makestr(&rcvfilter, NULL);
      }
      return (success = 1);
    } /* Adding a filter... */
    s = brstrip(s); /* Strip any braces */
    y = strlen(s);
    if (xx == XYSEND) {         /* For SEND filter... */
      for (x = 0; x < y; x++) { /* make sure they included "\v(...)" */
        if (s[x] != '\\') {
          continue;
        }
        if (s[x + 1] == 'v') {
          break;
        }
      }
      if (x == y) {
        printf("?Filter must contain a replacement variable for filename.\n");
        return (-9);
      }
    }
    if (xx == XYSEND) {
      makestr(&sndfilter, s);
      makestr(&g_sfilter, s);
    } else {
      makestr(&rcvfilter, s);
      makestr(&g_rfilter, s);
    }
    return (success = 1);
  }
#endif /* PIPESEND */

  case XYINIL:
    y = cmnum("Max length for protocol init string", "-1", 10, &x, xxstring);
    if ((y = setnum(&z, x, y, -1)) < 0) {
      return (y);
    }
    if (xx == XYSEND) {
      sprmlen = z;
    } else {
      rprmlen = z;
    }
    return (success = 1);

  case 993: {
    extern int sendipkts;
    if (xx == XYSEND) {
      if ((x = seton(&sendipkts)) < 0) {
        return (x);
      }
    }
    return (1);
  }
#ifdef CK_PERMS
  case 994:
    switch (xx) {
    case XYSEND:
      if ((x = seton(&atlpro)) < 0) {
        return (x);
      }
      atgpro = atlpro;
      return (1);
    case XYRECV:
      if ((x = seton(&atlpri)) < 0) {
        return (x);
      }
      atgpri = atlpri;
      return (1);
    default:
      return (-2);
    }
#endif /* CK_PERMS */

#ifndef NOCSETS
  case XYCSET: { /* CHARACTER-SET-SELECTION */
    extern struct keytab xfrmtab[];
    extern int r_cset, s_cset;
    if ((y = cmkey(xfrmtab, 2, "", "automatic", xxstring)) < 0) {
      return (y);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (xx == XYSEND) {
      s_cset = y;
    } else {
      r_cset = y;
    }
    return (success = 1);
  }
#endif /* NOCSETS */

  case XYBUP:
    if ((y = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
      return (y);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (xx == XYSEND) {
      extern int skipbup;
      skipbup = (y == 0) ? 1 : 0;
      return (success = 1);
    } else {
      printf("?Please use SET FILE COLLISION to choose the desired action\n");
      return (-9);
    }

  case XYMOVE:
    y = cmtxt("Directory to move file(s) to after successful transfer", "", &s,
              xxstring);

    if (y < 0 && y != -3) {
      return (y);
    }
    ckstrncpy(line, s, LINBUFSIZ);
    s = brstrip(line);

    /* Check directory existence if absolute */
    /* THIS MEANS IT CAN'T INCLUDE ANY DEFERRED VARIABLES! */
    if (s) {
      if (*s) {
        if (isabsolute(s) && !isdir(s)) {
          printf("?Directory does not exist - %s\n", s);
          return (-9);
        }
      }
    }
    if (xx == XYSEND) {
      if (*s) {
        makestr(&snd_move, line);
        makestr(&g_snd_move, line);
      } else {
        makestr(&snd_move, NULL);
        makestr(&g_snd_move, NULL);
      }
    } else {
      if (*s) {
        makestr(&rcv_move, line);
        makestr(&g_rcv_move, line);
      } else {
        makestr(&rcv_move, NULL);
        makestr(&g_rcv_move, NULL);
      }
    }
    return (success = 1);

  case XYRENAME:
    y = cmtxt("Template to rename file(s) to after successful transfer", "", &s,
              NULL);        /* NOTE: no xxstring */
    if (y < 0 && y != -3) { /* Evaluation is deferred */
      return (y);
    }
    ckstrncpy(line, s, LINBUFSIZ);
    s = brstrip(line);
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (xx == XYSEND) {
      if (*s) {
        makestr(&snd_rename, s);
        makestr(&g_snd_rename, s);
      } else {
        makestr(&snd_rename, NULL);
        makestr(&g_snd_rename, NULL);
      }
    } else {
      if (*s) {
        makestr(&rcv_rename, s);
        makestr(&g_rcv_rename, s);
      } else {
        makestr(&rcv_rename, NULL);
        makestr(&g_rcv_rename, NULL);
      }
    }
    return (success = 1);

  default:
    return (-2);
  } /* End of SET SEND/RECEIVE... */
}
#endif /* NOXFER */

#ifndef NOXMIT
int setxmit() {
  if ((y = cmkey(xmitab, nxmit, "", "", xxstring)) < 0) {
    return (y);
  }
  switch (y) {
  case XMITE: /* EOF */
    y = cmtxt("Characters to send at end of file,\n\
 Use backslash codes for control characters",
              "", &s, xxstring);
    if (y < 0) {
      return (y);
    }
    if ((int)strlen(s) > XMBUFL) {
      printf("?Too many characters, %d maximum\n", XMBUFL);
      return (-2);
    }
    ckstrncpy(xmitbuf, s, XMBUFL);
    return (success = 1);

  case XMITF: /* Fill */
    y = cmnum("Numeric code for blank-line fill character", "0", 10, &x,
              xxstring);
    if ((y = setnum(&z, x, y, 127)) < 0) {
      return (y);
    }
    xmitf = z;
    return (success = 1);
  case XMITL: /* Linefeed */
    return (seton(&xmitl));
  case XMITS: /* Locking-Shift */
    return (seton(&xmits));
  case XMITP: /* Prompt */
    y = cmnum("Numeric code for host's prompt character, 0 for none", "10", 10,
              &x, xxstring);
    if ((y = setnum(&z, x, y, 127)) < 0) {
      return (y);
    }
    xmitp = z;
    return (success = 1);
  case XMITX: /* Echo */
    return (seton(&xmitx));
  case XMITW: /* Pause */
    y = cmnum("Number of milliseconds to pause between binary characters\n\
or text lines during transmission",
              "0", 10, &x, xxstring);
    if ((y = setnum(&z, x, y, 1000)) < 0) {
      return (y);
    }
    xmitw = z;
    return (success = 1);
  case XMITT: /* Timeout */
    y = cmnum("Seconds to wait for each character to echo", "1", 10, &x,
              xxstring);
    if ((y = setnum(&z, x, y, 1000)) < 0) {
      return (y);
    }
    xmitt = z;
    return (success = 1);
  default:
    return (-2);
  }
}
#endif /* NOXMIT */

#ifndef NOXFER
/*  D O R M T  --  Do a remote command  */

void rmsg() {
  if (pflag && !quiet && fdispla != XYFD_N) {
    printf(
#ifdef CK_NEED_SIG
        " Type your escape character, %s, followed by X or E to cancel.\n",
        dbchr(escape)
#else
        " Press the X or E key to cancel.\n"
#endif /* CK_NEED_SIG */
    );
  }
}

static int xzcmd = 0; /* Global copy of REMOTE cmd index */

/*  R E M C F M  --  Confirm a REMOTE command  */
/*
  Like cmcfm(), but allows for a redirection indicator on the end,
  like "> filename" or "| command".  Returns what cmcfm() would have
  returned: -1 if reparse needed, etc etc blah blah.  On success,
  returns 1 with:

    char * remdest containing the name of the file or command.
    int remfile set to 1 if there is to be any redirection.
    int remappd set to 1 if output file is to be appended to.
    int rempipe set to 1 if remdest is a command, 0 if it is a file.
*/
static int remcfm() {
  int x = 0;
  char *s;
  char *helptxt = "> filename, | command,\n\
or type carriage return to confirm the command";
  char c;

  remfile = 0;
  rempipe = 0;
  remappd = 0;

  if ((x = cmtxt(helptxt, "", &s, xxstring)) < 0) {
    return (x);
  }
  if (remdest) {
    free(remdest);
    remdest = NULL;
  }
  debug(F101, "remcfm local", "", local);
  debug(F110, "remcfm s", s, 0);
  debug(F101, "remcfm cmd", "", xzcmd);
  /*
    This check was added in C-Kermit 6.0 or 7.0 but it turns out to be
    unhelpful in the situation where the remote is running a script that sends
    REMOTE commands to the local workstation.  What happens is, the local
    server executes the command and sends the result back as screen text, which
    is indicated by using an X packet instead of an F packet as the file
    header.  There are two parts to this: executing the command under control
    of the remote Kermit, which is desirable (and in fact some big applications
    depend on it, and therefore never installed any new C-Kermit versions after
    5A), and displaying the result.  Commenting out the check allows the
    command to be executed, but the result is still sent back to the remote in
    a file transfer, where it vanishes into the ether.  Actually it's on the
    communication connection, mixed in with the packets.  Pretty amazing that
    the file transfer still works, right?
  */

  if (!s) {
    s = ""; /* 2014-11-03 */
  }
  if (!*s) {
    return (1); /* 2014-11-03 */
  }

  c = *s;                       /* We have something */
  if (c != '>' && c != '|') {   /* Is it > or | ? */
    printf("?Not confirmed\n"); /* No */
    return (-9);
  }
  s++;                         /* See what follows */
  if (c == '>' && *s == '>') { /* Allow for ">>" too */
    s++;
    remappd = 1; /* Append to output file */
  }
  while (*s == SP || *s == HT) {
    s++; /* Strip intervening whitespace */
  }
  if (!*s) {
    printf("?%s missing\n", c == '>' ? "Filename" : "Command");
    return (-9);
  }
  if (c == '>' && zchko(s) < 0) { /* Check accessibility */
    printf("?Access denied - %s\n", s);
    return (-9);
  }
  remfile = 1; /* Set global results */
  rempipe = (c == '|');
  if (rempipe
#ifndef NOPUSH
      && nopush
#endif /* NOPUSH */
  ) {
    printf("?Sorry, access to external commands is disabled.\n");
    return (-9);
  }
  makestr(&remdest, s);
#ifndef NODEBUG
  if (deblog) {
    debug(F101, "remcfm remfile", "", remfile);
    debug(F101, "remcfm remappd", "", remappd);
    debug(F101, "remcfm rempipe", "", rempipe);
    debug(F110, "remcfm remdest", remdest, 0);
  }
#endif /* NODEBUG */
  return (1);
}

/*  R E M T X T  --  Like remcfm()...  */
/*
   ... but for REMOTE commands that end with cmtxt().
   Here we must decipher braces to discover whether the trailing
   redirection indicator is intended for local use, or to be sent out
   to the server, as in:

     remote host blah blah > file                 This end
     remote host { blah blah } > file             This end
     remote host { blah blah > file }             That end
     remote host { blah blah > file } > file      Both ends

   Pipes too:

     remote host blah blah | cmd                  This end
     remote host { blah blah } | cmd              This end
     remote host { blah blah | cmd }              That end
     remote host { blah blah | cmd } | cmd        Both ends

   Or both:

     remote host blah blah | cmd > file           This end, etc etc...

   Note: this really only makes sense for REMOTE HOST, but why be picky?
   Call after calling cmtxt(), with pointer to string that cmtxt() parsed,
   as in "remtxt(&s);".

   Returns:
    1 on success with braces & redirection things removed & pointer updated,
   -9 on failure (bad indirection), after printing error message.
*/
int remtxt(char **p) {
  int i, x, bpos, ppos;
  char c, *s, *q;

  remfile = 0; /* Initialize global results */
  rempipe = 0;
  remappd = 0;
  if (remdest) {
    free(remdest);
    remdest = NULL;
  }
  s = *p;
  if (!s) { /* No redirection indicator */
    s = "";
  }
  bpos = -1;     /* Position of > (bracket) */
  ppos = -1;     /* Position of | (pipe) */
  x = strlen(s); /* Length of cmtxt() string */

  for (i = x - 1; i >= 0; i--) { /* Search right to left. */
    c = s[i];
    if (c == '}') {        /* Break on first right brace */
      break;               /* Don't look at contents of braces */
    } else if (c == '>') { /* Record position of > */
      bpos = i;
    } else if (c == '|') { /* and of | */
      ppos = i;
    }
  }
  if (bpos < 0 && ppos < 0) { /* No redirectors. */
    s = brstrip(s);           /* Remove outer braces if any. */
    *p = s;                   /* Point to result */
    return (1);               /* and return. */
  }
  remfile = 1;     /* It's | or > */
  i = -1;          /* Get leftmost symbol */
  if (bpos > -1) { /* Bracket */
    i = bpos;
  }
  if (ppos > -1 && (ppos < bpos || bpos < 0)) { /* or pipe */
    i = ppos;
    rempipe = 1;
  }
  if (rempipe
#ifndef NOPUSH
      && nopush
#endif /* NOPUSH */
  ) {
    printf("?Sorry, access to external commands is disabled.\n");
    return (-9);
  }
  c = s[i]; /* Copy of symbol */

  if (c == '>' && s[i + 1] == '>') { /* ">>" for append? */
    remappd = 1;                     /* It's not just a flag it's a number */
  }

  q = s + i + 1 + remappd; /* Point past symbol in string */
  while (*q == SP || *q == HT) {
    q++; /* and any intervening whitespace */
  }
  if (!*q) {
    printf("?%s missing\n", c == '>' ? "Filename" : "Command");
    return (-9);
  }
  if (c == '>' && zchko(q) < 0) { /* (Doesn't work for | cmd > file) */
    printf("?Access denied - %s\n", q);
    return (-9);
  }
  makestr(&remdest, q);                     /* Create the destination string */
  q = s + i - 1;                            /* Point before symbol */
  while (q > s && (*q == SP || *q == HT)) { /* Strip trailing whitespace */
    q--;
  }
  *(q + 1) = NUL; /* Terminate the string. */
  s = brstrip(s); /* Remove any braces */
  *p = s;         /* Set return value */

#ifndef NODEBUG
  if (deblog) {
    debug(F101, "remtxt remfile", "", remfile);
    debug(F101, "remtxt remappd", "", remappd);
    debug(F101, "remtxt rempipe", "", rempipe);
    debug(F110, "remtxt remdest", remdest, 0);
    debug(F110, "remtxt command", s, 0);
  }
#endif /* NODEBUG */

  return (1);
}

int plogin(int xx) {
  char *p1 = NULL, *p2 = NULL, *p3 = NULL;
  int psaved = 0, rc = 0;
#ifdef CK_RECALL
  extern int on_recall; /* around Password prompting */
#endif                  /* CK_RECALL */
  debug(F101, "plogin local", "", local);

  if (!local || (network && ttchk() < 0)) {
    printf("?No connection\n");
    return (-9);
  }
  if ((x = cmfld("User ID", "", &s, xxstring)) < 0) { /* Get User ID */
    if (x != -3) {
      return (x);
    }
  }
  y = strlen(s);
  if (y > 0) {
    if ((p1 = malloc(y + 1)) == NULL) {
      printf("?Internal error: malloc\n");
      rc = -9;
      goto XZXLGI;
    } else {
      strcpy(p1, s); /* safe */
    }
    if ((rc = cmfld("Password", "", &s, xxstring)) < 0) {
      if (rc != -3) {
        goto XZXLGI;
      }
    }
    y = strlen(s);
    if (y > 0) {
      if ((p2 = malloc(y + 1)) == NULL) {
        printf("?Internal error: malloc\n");
        rc = -9;
        goto XZXLGI;
      } else {
        strcpy(p2, s); /* safe */
      }
      if ((rc = cmfld("Account", "", &s, xxstring)) < 0) {
        if (rc != -3) {
          goto XZXLGI;
        }
      }
      y = strlen(s);
      if (y > 0) {
        if ((p3 = malloc(y + 1)) == NULL) {
          printf("?Internal error: malloc\n");
          rc = -9;
          goto XZXLGI;
        } else {
          strcpy(p3, s); /* safe */
        }
      }
    }
  }
  if ((rc = remtxt(&s)) < 0) { /* Confirm & handle redirectors */
    goto XZXLGI;
  }

  if (!p1) {   /* No Userid specified... */
    debok = 0; /* Don't log this */
               /* Prompt for username, password, and account */
#ifdef CK_RECALL
    on_recall = 0;
#endif                      /* CK_RECALL */
    cmsavp(psave, PROMPTL); /* Save old prompt */
    psaved = 1;
    debug(F110, "REMOTE LOGIN saved", psave, 0);

    cmsetp("Username: "); /* Make new prompt */
    concb((char)escape);  /* Put console in cbreak mode */
    cmini(1);
    prompt(xxstring);
    rc = -9;
    for (x = -1; x < 0;) {         /* Prompt till they answer */
      cmres();                     /* Reset the parser */
      x = cmtxt("", "", &s, NULL); /* Get a literal line of text */
    }
    y = strlen(s);
    if (y < 1) {
      printf("?Canceled\n");
      goto XZXLGI;
    }
    if ((p1 = malloc(y + 1)) == NULL) {
      printf("?Internal error: malloc\n");
      goto XZXLGI;
    } else {
      strcpy(p1, s); /* safe */
    }

    cmsetp("Password: "); /* Make new prompt */
    concb((char)escape);  /* Put console in cbreak mode */
    cmini(0);             /* No echo */
    prompt(xxstring);
    debok = 0;
    for (x = -1; x < 0 && x != -3;) { /* Get answer */
      cmres();                        /* Reset the parser */
      x = cmtxt("", "", &s, NULL);    /* Get literal line of text */
    }
    if ((p2 = malloc((int)strlen(s) + 1)) == NULL) {
      printf("?Internal error: malloc\n");
      goto XZXLGI;
    } else {
      strcpy(p2, s); /* safe */
    }
    printf("\r\n");
    if ((rc = cmcfm()) < 0) {
      goto XZXLGI;
    }
  }
  sstate = setgen('I', p1, p2, p3); /* Get here with at least user ID */
  rc = 0;

XZXLGI: /* Common exit point */
  if (psaved) {
    cmsetp(psave); /* Restore original prompt */
  }
  if (p3) {
    free(p3);
    p3 = NULL;
  } /* Free malloc'd storage */
  if (p2) {
    free(p2);
    p2 = NULL;
  }
  if (p1) {
    free(p1);
    p1 = NULL;
  }
  if (rc > -1) {
    if (local && rc > -1) { /* If local, flush tty input buffer */
      ttflui();
    }
  }
  return (rc);
}

int dormt(int xx)

{ /* REMOTE commands */
  int x, y, retcode;
  char *s, sbuf[50], *s2;

#ifdef NEWFTP
  extern int ftpget, ftpisopen();
  if ((ftpget == 1) || ((ftpget == 2) && ftpisopen())) {
    return (doftprmt(xx, 0));
  }
#endif /* NEWFTP */

  remfile = 0; /* Clear these */
  rempipe = 0;
  remappd = 0;

  debug(F101, "XXX xxdormt xx", "", xx);

  if (xx < 0) {
    return (xx); /* REMOTE what? */
  }

  xzcmd = xx; /* Make global copy of arg */

  if (xx == XZSET) { /* REMOTE SET */
    if ((y = cmkey(rmstab, nrms, "", "", xxstring)) < 0) {
      if (y == -3) {
        printf("?Parameter name required\n");
        return (-9);
      } else {
        return (y);
      }
    }
    return (doprm(y, 1));
  }
  switch (xx) { /* Others... */

  case XZCDU:
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    s = "..";
    rcdactive = 1;
    sstate = setgen('C', s, "", "");
    retcode = 0;
    break;

  case XZSTA: /* Remote Status (2024) */
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    sstate = setgen('Q', "", "", "");
    retcode = 0;
    break;

  case XZCWD: /* CWD (CD) */
    if ((x = cmtxt("Remote directory name", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    debug(F111, "XZCWD: ", s, x);
    *sbuf = NUL;
    s2 = sbuf;
/*
  The following is commented out because since the disappearance of the
  DECSYSTEM-20 from the planet, no known computer requires a password for
  changing directory.
*/
#ifdef DIRPWDPR
    if (*s != NUL) {     /* If directory name given, */
                         /* get password on separate line. */
      if (tlevel > -1) { /* From take file... */

        if (fgets(sbuf, 50, tfile[tlevel]) == NULL) {
          fatal("take file ends prematurely in 'remote cwd'");
        }
        debug(F110, " pswd from take file", s2, 0);
        for (x = (int)strlen(sbuf);
             x > 0 && (sbuf[x - 1] == NL || sbuf[x - 1] == CR); x--) {
          sbuf[x - 1] = '\0';
        }

      } else { /* From terminal... */

        printf(" Password: "); /* get a password */
#ifdef IKSD
        if (!local && inserver) {
          x = coninc(0);
        } else
#endif /* IKSD */
          x = getchar();
        while ((x != NL) && (x != CR)) {
          if ((x &= 0177) == '?') {
            printf("? Password of remote directory\n Password: ");
            s2 = sbuf;
            *sbuf = NUL;
          } else if (x == ESC) { /* Mini command line editor... */
            bleep(BP_WARN);
          } else if (x == BS || x == 0177) {
            s2--;
          } else if (x == 025) { /* Ctrl-U */
            s2 = sbuf;
            *sbuf = NUL;
          } else {
            *s2++ = x;
          }

          /* Get the next character */
#ifdef IKSD
          if (!local && inserver) {
            x = coninc(0);
          } else
#endif /* IKSD */
            x = getchar();
        }
        *s2 = NUL;
        putchar('\n');
      }
      s2 = sbuf;
    } else {
      s2 = "";
    }
    debug(F110, " password", s2, 0);
#endif /* DIRPWDPR */

    rcdactive = 1;
    sstate = setgen('C', s, s2, "");
    retcode = 0;
    break;

  case XZDEL: /* Delete */
    if ((x = cmtxt("Name of remote file(s) to delete", "", &s, xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of remote file(s) required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if (local) {
      ttflui(); /* If local, flush tty input buffer */
    }
    retcode = sstate = rfilop(s, 'E');
    break;

  case XZDIR: /* Directory */
    if ((x = cmtxt("Remote directory or file specification", "", &s,
                   xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if (local) {
      ttflui(); /* If local, flush tty input buffer */
    }
    rmsg();
    retcode = sstate = setgen('D', s, "", "");
    break;

  case XZHLP: /* Help */
    if ((x = remcfm()) < 0) {
      return (x);
    }
    sstate = setgen('H', "", "", "");
    retcode = 0;
    break;

  case XZHOS: /* Host */
    if ((x = cmtxt("Command for remote system", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if ((y = (int)strlen(s)) < 1) {
      return (x);
    }
    ckstrncpy(line, s, LINBUFSIZ);
    cmarg = line;
    rmsg();
    retcode = sstate = 'c';
    break;

#ifndef NOFRILLS
  case XZKER:
    if ((x = cmtxt("Command for remote Kermit", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if ((int)strlen(s) < 1) {
      if (x == -3) {
        printf("?Remote Kermit command required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    ckstrncpy(line, s, LINBUFSIZ);
    cmarg = line;
    retcode = sstate = 'k';
    rmsg();
    break;

  case XZLGI:      /* Login */
    rcdactive = 1; /* Suppress "Logged in" msg if quiet */
    return (plogin(XXREM));

  case XZLGO: { /* Logout */
    extern int bye_active;
    if ((x = remcfm()) < 0) {
      return (x);
    }
    sstate = setgen('I', "", "", "");
    retcode = 0;
    bye_active = 1; /* Close connection when done */
    break;
  }

  case XZPRI:                 /* Print */
    if (!atdiso || !atcapr) { /* Disposition attribute off? */
      printf("?Disposition Attribute is Off\n");
      return (-2);
    }
    cmarg = "";
    cmarg2 = "";
    if ((x = cmifi("Local file(s) to print on remote printer", "", &s, &y,
                   xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of local file(s) required\n");
        return (-9);
      }
      return (x);
    }
    ckstrncpy(line, s, LINBUFSIZ); /* Make a safe copy of filename */
    *optbuf = NUL;                 /* Wipe out any old options */
    if ((x = cmtxt("Options for remote print command", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if ((int)strlen(optbuf) > 94) { /* Make sure this is legal */
      printf("?Option string too long\n");
      return (-9);
    }
    ckstrncpy(optbuf, s, OPTBUFLEN); /* Make a safe copy of options */
    nfils = -1;                      /* Expand file list internally */
    cmarg = line;                    /* Point to file list. */
    rprintf = 1;                     /* REMOTE PRINT modifier for SEND */
    sstate = 's';                    /* Set start state to SEND */
    if (local) {
      displa = 1;
    }
    retcode = 0;
    break;
#endif /* NOFRILLS */

  case XZSPA: /* Space */
    if ((x = cmtxt("Confirm, or remote directory name", "", &s, xxstring)) <
        0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    retcode = sstate = setgen('U', s, "", "");
    break;

  case XZMSG: /* Message */
    if ((x = cmtxt("Short text message for server", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    retcode = sstate = setgen('M', s, "", "");
    break;

#ifndef NOFRILLS
  case XZTYP: /* Type */
    if ((x = cmtxt("Remote file specification", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((int)strlen(s) < 1) {
      printf("?Remote filename required\n");
      return (-9);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    rmsg();
    retcode = sstate = rfilop(s, 'T');
    break;
#endif /* NOFRILLS */

#ifndef NOFRILLS
  case XZWHO:
    if ((x = cmtxt("Remote user name, or carriage return", "", &s, xxstring)) <
        0) {
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    retcode = sstate = setgen('W', s, "", "");
    break;
#endif /* NOFRILLS */

  case XZPWD: /* PWD */
    if ((x = remcfm()) < 0) {
      return (x);
    }
    sstate = setgen('A', "", "", "");
    retcode = 0;
    break;

#ifndef NOSPL
  case XZQUE: { /* Query */
    char buf[2];
    extern char querybuf[], *qbufp;
    extern int qbufn;
    if ((y = cmkey(vartyp, nvartyp, "", "", xxstring)) < 0) {
      return (y);
    }
    if ((x = cmtxt(y == 'F' ? "Remote function invocation"
                            : ('K' ? "Remote variable name or function"
                                   : "Remote variable name"),
                   "", &s, (y == 'K') ? xxstring : NULL)) <
        0) { /* Don't evaluate */
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    query = 1;        /* QUERY is active */
    qbufp = querybuf; /* Initialize query response buffer */
    qbufn = 0;
    querybuf[0] = NUL;
    buf[0] = (char)(y & 127);
    buf[1] = NUL;
    retcode = sstate = setgen('V', "Q", (char *)buf, s);
    break;
  }

  case XZASG: { /* Assign */
    char buf[VNAML];
    if ((y = cmfld("Remote variable name", "", &s, NULL)) < 0) { /* No eval */
      return (y);
    }
    if ((int)strlen(s) >= VNAML) {
      printf("?Too long\n");
      return (-9);
    }
    ckstrncpy(buf, s, VNAML);
    if ((x = cmtxt("Assignment for remote variable", "", &s, xxstring)) <
        0) { /* Evaluate this one */
      return (x);
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    retcode = sstate = setgen('V', "S", (char *)buf, s);
    break;
  }
#endif /* NOSPL */

  case XZCPY: { /* COPY */
    char buf[TMPBUFSIZ];
    buf[TMPBUFSIZ - 1] = '\0';
    if ((x = cmfld("Name of remote file to copy", "", &s, xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of remote file required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    ckstrncpy(buf, s, TMPBUFSIZ);
    if ((x = cmfld("Name of remote destination file or directory", "", &s,
                   xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of remote file or directory required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    ckstrncpy(tmpbuf, s, TMPBUFSIZ);
    if ((x = remcfm()) < 0) {
      return (x);
    }
    if (local) {
      ttflui(); /* If local, flush tty input buffer */
    }
    retcode = sstate = setgen('K', buf, tmpbuf, "");
    break;
  }
  case XZREN: { /* Rename */
    char buf[TMPBUFSIZ];
    buf[TMPBUFSIZ - 1] = '\0';
    if ((x = cmfld("Name of remote file to rename", "", &s, xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of remote file required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    ckstrncpy(buf, s, TMPBUFSIZ);
    if ((x = cmfld("New name of remote file", "", &s, xxstring)) < 0) {
      if (x == -3) {
        printf("?Name of remote file required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    ckstrncpy(tmpbuf, s, TMPBUFSIZ);
    if ((x = remcfm()) < 0) {
      return (x);
    }
    if (local) {
      ttflui(); /* If local, flush device buffer */
    }
    retcode = sstate = setgen('R', buf, tmpbuf, "");
    break;
  }
  case XZMKD: /* mkdir */
  case XZRMD: /* rmdir */
    if ((x = cmtxt((xx == XZMKD) ? "Name of remote directory to create"
                                 : "Name of remote directory to delete",
                   "", &s, xxstring)) < 0) {
      if (x == -3) {
        printf("?Name required\n");
        return (-9);
      } else {
        return (x);
      }
    }
    if ((x = remtxt(&s)) < 0) {
      return (x);
    }
    if (local) {
      ttflui(); /* If local, flush tty input buffer */
    }
    retcode = sstate = rfilop(s, (char)(xx == XZMKD ? 'm' : 'd'));
    break;

  case XZXIT: /* Exit */
    if ((x = remcfm()) < 0) {
      return (x);
    }
    sstate = setgen('X', "", "", "");
    retcode = 0;
    break;

  default:
    if ((x = remcfm()) < 0) {
      return (x);
    }
    printf("?Not implemented - %s\n", cmdbuf);
    return (-2);
  }
  if (local && retcode > -1) { /* If local, flush tty input buffer */
    ttflui();
  }
  return (retcode);
}

/*  R F I L O P  --  Remote File Operation  */

CHAR rfilop(char *s, char t)
/* rfilop */ {
  if (*s == NUL) {
    printf("?File specification required\n");
    return ((CHAR)0);
  }
  debug(F111, "rfilop", s, t);
  return (setgen(t, s, "", ""));
}
#endif /* NOXFER */

#ifdef ANYX25
int setx25() {
  if ((y = cmkey(x25tab, nx25, "X.25 call options", "", xxstring)) < 0) {
    return (y);
  }
  switch (y) {
  case XYUDAT:
    if ((z = cmkey(onoff, 2, "X.25 call user data", "", xxstring)) < 0) {
      return (z);
    }
    if (z == 0) {
      if ((z = cmcfm()) < 0) {
        return (z);
      }
      cudata = 0; /* disable call user data */
      return (success = 1);
    }
    if ((x = cmtxt("X.25 call user data string", "", &s, xxstring)) < 0) {
      return (x);
    }
    if ((int)strlen(s) == 0) {
      return (-3);
    } else if ((int)strlen(s) > MAXCUDATA) {
      printf("?The length must be > 0 and <= %d\n", MAXCUDATA);
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    ckstrncpy(udata, s, MAXCUDATA);
    cudata = 1; /* X.25 call user data specified */
    return (success = 1);
  case XYCLOS:
    if ((z = cmkey(onoff, 2, "X.25 closed user group call", "", xxstring)) <
        0) {
      return (z);
    }
    if (z == 0) {
      if ((z = cmcfm()) < 0) {
        return (z);
      }
      closgr = -1; /* disable closed user group */
      return (success = 1);
    }
    if ((y = cmnum("0 <= cug index >= 99", "", 10, &x, xxstring)) < 0) {
      return (y);
    }
    if (x < 0 || x > 99) {
      printf("?The choices are 0 <= cug index >= 99\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    closgr = x; /* closed user group selected */
    return (success = 1);

  case XYREVC:
    if ((z = cmkey(onoff, 2, "X.25 reverse charge call", "", xxstring)) < 0) {
      return (z);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    revcall = z;
    return (success = 1);
  }
}

#ifndef IBMX25
int setpadp() {
  if ((y = cmkey(padx3tab, npadx3, "PAD X.3 parameter name", "", xxstring)) <
      0) {
    return (y);
  }
  x = y;
  switch (x) {
  case PAD_BREAK_CHARACTER:
    if ((y = cmnum("PAD break character value", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_ESCAPE:
    if ((y = cmnum("PAD escape", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_ECHO:
    if ((y = cmnum("PAD echo", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_DATA_FORWARD_CHAR:
    if ((y = cmnum("PAD data forward char", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 2) {
      printf("?The choices are 0 or 2\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_DATA_FORWARD_TIMEOUT:
    if ((y = cmnum("PAD data forward timeout", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 255) {
      printf("?The choices are 0 or 1 <= timeout <= 255\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_FLOW_CONTROL_BY_PAD:
    if ((y = cmnum("PAD pad flow control", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  case PAD_SUPPRESSION_OF_SIGNALS:
    if ((y = cmnum("PAD service", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_BREAK_ACTION:
    if ((y = cmnum("PAD break action", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1 && z != 2 && z != 5 && z != 8 && z != 21) {
      printf("?The choices are 0, 1, 2, 5, 8 or 21\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_SUPPRESSION_OF_DATA:
    if ((y = cmnum("PAD data delivery", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_PADDING_AFTER_CR:
    if ((y = cmnum("PAD crpad", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 7) {
      printf("?The choices are 0 or 1 <= crpad <= 7\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_LINE_FOLDING:
    if ((y = cmnum("PAD linefold", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 255) {
      printf("?The choices are 0 or 1 <= linefold <= 255\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_LINE_SPEED:
    if ((y = cmnum("PAD baudrate", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 18) {
      printf("?The choices are 0 <= baudrate <= 18\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_FLOW_CONTROL_BY_USER:
    if ((y = cmnum("PAD terminal flow control", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_LF_AFTER_CR:
    if ((y = cmnum("PAD crpad", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z == 3 || z > 7) {
      printf("?The choices are 0, 1, 2, 4, 5, 6 or 7\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_PADDING_AFTER_LF:
    if ((y = cmnum("PAD lfpad", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 7) {
      printf("?The choices are 0 or 1 <= lfpad <= 7\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_EDITING:
    if ((y = cmnum("PAD edit control", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z != 0 && z != 1) {
      printf("?The choices are 0 or 1\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_CHAR_DELETE_CHAR:
    if ((y = cmnum("PAD char delete char", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 127) {
      printf("?The choices are 0 or 1 <= chardelete <= 127\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_BUFFER_DELETE_CHAR:
    if ((y = cmnum("PAD buffer delete char", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 127) {
      printf("?The choices are 0 or 1 <= bufferdelete <= 127\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;

  case PAD_BUFFER_DISPLAY_CHAR:
    if ((y = cmnum("PAD display line char", "", 10, &z, xxstring)) < 0) {
      return (y);
    }
    if (z < 0 || z > 127) {
      printf("?The choices are 0 or 1 <= displayline <= 127\n");
      return (-2);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    break;
  }
  padparms[x] = z;
  return (success = 1);
}
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOXFER
int setat(int rmsflg) {
  int xx;
  if ((y = cmkey(attrtab, natr, "File Attribute packets", "", xxstring)) < 0) {
    return (y);
  }
  if (y == AT_XALL) { /* ATTRIBUTES ALL ON or ALL OFF */
    if ((z = seton(&xx)) < 0) {
      return (z);
    }
    if (rmsflg) {
      printf("Sorry, command not available\n");
      return (-9);
    } else {
      atenci = xx; /* Encoding in */
      atenco = xx; /* Encoding out */
      atdati = xx; /* Date in */
      atdato = xx; /* Date out */
      atdisi = xx; /* Disposition in/out */
      atdiso = xx;
      atleni = xx; /* Length in/out (both kinds) */
      atleno = xx;
      atblki = xx; /* Blocksize in/out */
      atblko = xx;
      attypi = xx; /* File type in/out */
      attypo = xx;
      atsidi = xx; /* System ID in/out */
      atsido = xx;
      atsysi = xx; /* System-dependent params in/out */
      atsyso = xx;
#ifdef CK_PERMS    /* Protection */
      atlpri = xx; /* Local in */
      atlpro = xx; /* Local out */
      atgpri = xx; /* Generic in */
      atgpro = xx; /* Generic out */
#endif             /* CK_PERMS */
    }
    return (z);
  } else if (y == AT_ALLY || y == AT_ALLN) { /* ATTRIBUTES ON or OFF */
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    atcapr = (y == AT_ALLY) ? 1 : 0;
    if (rmsflg) {
      sstate = setgen('S', "132", atcapr ? "1" : "0", "");
      return ((int)sstate);
    } else {
      return (success = 1);
    }
  }
  /* Otherwise, it's an individual attribute that wants turning off/on */

  if ((z = cmkey(onoff, 2, "", "", xxstring)) < 0) {
    return (z);
  }
  if ((x = cmcfm()) < 0) {
    return (x);
  }

  /* There are better ways to do this... */
  /* The real problem is that we're not separating the in and out cases */
  /* and so we have to arbitrarily pick the "in" case, i.e tell the remote */
  /* server to ignore incoming attributes of the specified type, rather */
  /* than telling it not to send them.  The protocol does not (yet) define */
  /* codes for "in-and-out-at-the-same-time". */

  switch (y) {
#ifdef CK_PERMS
    /* We're lumping local and generic protection together for now... */
  case AT_LPRO:
  case AT_GPRO:
    if (rmsflg) {
      sstate = setgen('S', "143", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atlpri = atlpro = atgpri = atgpro = z;
    break;
#endif /* CK_PERMS */
  case AT_DISP:
    if (rmsflg) {
      sstate = setgen('S', "142", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atdisi = atdiso = z;
    break;
  case AT_ENCO:
    if (rmsflg) {
      sstate = setgen('S', "141", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atenci = atenco = z;
    break;
  case AT_DATE:
    if (rmsflg) {
      sstate = setgen('S', "135", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atdati = atdato = z;
    break;
  case AT_LENB:
  case AT_LENK:
    if (rmsflg) {
      sstate = setgen('S', "133", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atleni = atleno = z;
    break;
  case AT_BLKS:
    if (rmsflg) {
      sstate = setgen('S', "139", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atblki = atblko = z;
    break;
  case AT_FTYP:
    if (rmsflg) {
      sstate = setgen('S', "134", z ? "1" : "0", "");
      return ((int)sstate);
    }
    attypi = attypo = z;
    break;
  case AT_SYSI:
    if (rmsflg) {
      sstate = setgen('S', "145", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atsidi = atsido = z;
    break;
  case AT_RECF:
    if (rmsflg) {
      sstate = setgen('S', "146", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atfrmi = atfrmo = z;
    break;
  case AT_SYSP:
    if (rmsflg) {
      sstate = setgen('S', "147", z ? "1" : "0", "");
      return ((int)sstate);
    }
    atsysi = atsyso = z;
    break;
  default:
    printf("?Not available\n");
    return (-2);
  }
  return (1);
}
#endif /* NOXFER */

#ifndef NOSPL
int setinp() {
  if ((y = cmkey(inptab, ninp, "", "", xxstring)) < 0) {
    return (y);
  }
  switch (y) {
  case IN_DEF: /* SET INPUT DEFAULT-TIMEOUT */
    z = cmnum("Positive number", "", 10, &x, xxstring);
    return (setnum(&indef, x, z, 94));
#ifdef CKFLOAT
  case IN_SCA: /* SET INPUT SCALE-FACTOR */
    if ((x = cmfld("Number such as 2 or 0.5", "1.0", &s, xxstring)) < 0) {
      return (x);
    }
    if (isfloat(s, 0)) { /* A floating-point number? */
      extern char *inpscale;
      inscale = floatval;    /* Yes, get its value */
      makestr(&inpscale, s); /* Save it as \v(inscale) */
      return (success = 1);
    } else {
      return (-2);
    }
#endif         /* CKFLOAT */
  case IN_TIM: /* SET INPUT TIMEOUT-ACTION */
    if ((z = cmkey(intimt, 2, "", "", xxstring)) < 0) {
      return (z);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    intime[cmdlvl] = z;
    return (success = 1);
  case IN_CAS: /* SET INPUT CASE */
    if ((z = cmkey(incast, 2, "", "", xxstring)) < 0) {
      return (z);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    inpcas[cmdlvl] = z;
    return (success = 1);
  case IN_ECH: /* SET INPUT ECHO */
    return (seton(&inecho));
  case IN_SIL: /* SET INPUT SILENCE */
    z = cmnum("Seconds of inactivity before INPUT fails", "", 10, &x, xxstring);
    return (setnum(&insilence, x, z, -1));

  case IN_BUF: /* SET INPUT BUFFER-SIZE */
    if ((z = cmnum("Number of bytes in INPUT buffer", ckitoa(INPBUFSIZ), 10, &x,
                   xxstring)) < 0) {
      return (z);
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    inbufsize = 0;
    if (inpbuf) {
      free(inpbuf);
      inpbuf = NULL;
      inpbp = NULL;
    }
    if (!(s = (char *)malloc(x + 1))) {
      return (0);
    }
    inpbuf = s;
    inpbp = s;
    inbufsize = x;
    for (x = 0; x <= inbufsize; x++) {
      inpbuf[x] = NUL;
    }
    return (success = 1);

#ifdef CK_AUTODL
  case IN_ADL: /* AUTODOWNLOAD */
    return (seton(&inautodl));
#endif /* CK_AUTODL */

  case IN_CAN: /* SET INPUT INTERRUPTS */
    return (seton(&inintr));
  }
  return (0);
}
#endif /* NOSPL */

#ifdef NETCONN
void ndreset() {
#ifndef NODIAL /* This depends on DIAL... */
  int i = 0, j = 0;
  if (!ndinited) { /* Don't free garbage... */
    return;
  }
  for (i = 0; i < nhcount; i++) { /* Clean out previous list */
    if (nh_p[i]) {
      free(nh_p[i]);
    }
    nh_p[i] = NULL;
    if (nh_p2[i]) {
      free(nh_p2[i]);
    }
    nh_p2[i] = NULL;
    for (j = 0; j < 4; j++) {
      if (nh_px[j][i]) {
        free(nh_px[j][i]);
      }
      nh_px[j][i] = NULL;
    }
  }
#endif /* NODIAL */
}

void ndinit() { /* Net directory pointers */
#ifndef NODIAL  /* This depends on DIAL... */
  int i, j;
  if (ndinited++) { /* Don't do this more than once. */
    return;
  }
  for (i = 0; i < MAXDDIR; i++) { /* Init all pointers to NULL */
    netdir[i] = NULL;
  }
  for (i = 0; i < MAXDNUMS; i++) {
    nh_p[i] = NULL;
    nh_p2[i] = NULL;
    for (j = 0; j < 4; j++) {
      nh_px[j][i] = NULL;
    }
  }
#endif /* NODIAL */
}

#ifndef NODIAL
#ifdef NETCONN
void /* Get net defaults from environment */
getnetenv() {
  char *p = NULL;

  makestr(&p, getenv("K_NET_DIRECTORY")); /* Dialing directories */
  if (p) {
    int i;
    xwords(p, MAXDDIR, netdir, 0);
    for (i = 0; i < MAXDDIR; i++) { /* Fill in any gaps... */
      if (!netdir[i + 1]) {
        break;
      } else {
        netdir[i] = netdir[i + 1];
      }
      debug(F111, "netdir[i]", netdir[i], i);
    }
    nnetdir = i;
  }
}
#endif /* NETCONN */
#endif /* NODIAL */

int lunet(char *s) /* s = name to look up   */
/* lunet */ {
#ifndef NODIAL /* This depends on DIAL... */
  int n, n1, t, dd = 0;
  int ambiguous = 0;
  FILE *f;
  char *line = NULL;
  extern int dialdpy;
  int netdpy = dialdpy;
  char *info[8];

  nhcount = 0; /* Set this before returning */

  if (!s || nnetdir < 1) { /* Validate arguments */
    return (-1);
  }

  if (isdigit(*s) || *s == '*' || *s == '.') {
    return (0);
  }

  if ((n1 = (int)strlen(s)) < 1) { /* Length of string to look up */
    return (-1);
  }

  if (!(line = malloc(1024))) { /* Allocate input buffer */
    return (-1);
  }

lu_again:
  f = NULL;        /* Network directory file descriptor */
  t = nhcount = 0; /* Match count */
  dd = 0;          /* Directory counter */

  dirline = 0;
  while (1) {              /* We make one pass */
    if (!f) {              /* Directory not open */
      if (dd >= nnetdir) { /* No directories left? */
        break;             /* Done. */
      }
      if ((f = fopen(netdir[dd], "r")) == NULL) { /* Open it */
        perror(netdir[dd]); /* Can't, print message saying why */
        dd++;
        continue; /* But go on to next one. */
      }
      if (netdpy) {
        printf("Opening %s...\n", netdir[dd]);
      }
      dd++;
    }
    line[0] = NUL;
    if (getnct(line, 1023, f, 1) < 0) { /* Read a line */
      if (f) {                          /* f can be clobbered! */
        fclose(f);                      /* Close the file */
        f = NULL;                       /* Indicate next one needs opening */
      }
      continue;
    }
    if (!line[0]) { /* Empty line */
      continue;
    }

    xwords(line, 7, info, 0); /* Parse it */

    if (!info[1] || !info[2] || !info[3]) { /* Required fields */
      continue;
    }
    if (*info[1] == ';') { /* Full-line comment */
      continue;
    }
    if ((n = (int)strlen(info[1])) < 1) { /* Length of name-tag */
      continue;
    }
    if (n < n1) { /* Search name is longer */
      continue;   /* Can't possibly match */
    }
    if (ambiguous && n != n1) {
      continue;
    }
    if (ckstrcmp(s, info[1], n1, 0)) { /* Compare using length of */
      continue;                        /* search string s. */
    }

    /* Have a match */

    makestr(&(nh_p[nhcount]), info[3]);     /* address */
    makestr(&(nh_p2[nhcount]), info[2]);    /* net type */
    makestr(&(nh_px[0][nhcount]), info[4]); /* net-specific stuff... */
    makestr(&(nh_px[1][nhcount]), info[5]);
    makestr(&(nh_px[2][nhcount]), info[6]);
    makestr(&(nh_px[3][nhcount]), info[7]);

    nhcount++;                /* Count this match */
    if (nhcount > MAXDNUMS) { /* Watch out for too many */
      printf("Warning: %d matches found, %d max\n", nhcount, MAXDNUMS);
      nhcount = MAXDNUMS;
      break;
    }
    if (nhcount == 1) { /* First one - save entry name */
      if (n_name) {     /* Free the one from before if any */
        free(n_name);
        n_name = NULL;
      }
      if (!(n_name = (char *)malloc(n + 1))) { /* Allocate new storage */
        printf("?memory allocation error - lunet:3\n");
        if (line) {
          free(line);
          line = NULL;
        }
        nhcount = 0;
        return (-1);
      }
      t = n;                                    /* Remember its length */
      strcpy(n_name, info[1]);                  /* safe */
    } else {                                    /* Second or subsequent one */
      if ((int)strlen(info[1]) == t) {          /* Lengths compare */
        if (!ckstrcmp(n_name, info[1], t, 0)) { /* Caseless compare OK */
          continue;
        }
      }

      /* Name given by user matches entries with different names */

      if (ambiguous) { /* Been here before */
        break;
      }

      ambiguous = 1; /* Now an exact match is required */
      ndreset();     /* Clear out previous list */
      goto lu_again; /* Do it all over again. */
    }
  }
  if (line) {
    free(line);
    line = NULL;
  }
  if (nhcount == 0 && ambiguous) {
    printf("?\"%s\" - ambiguous in network directory\n", s);
  }
#else
  nhcount = 0;
#endif /* NODIAL */
  return (nhcount);
}
#endif /* NETCONN */

#ifndef NOLOCAL
/*  C L S C O N N X  --  Close connection  */

int clsconnx(int ask) {
  int x, rc = 0;
#ifdef NEWFTP
  extern int ftpget, ftpisopen();
  if ((ftpget == 1) || ((ftpget == 2) && !local && ftpisopen())) {
    return (success = ftpbye());
  }
#endif /* NEWFTP */
  debug(F101, "clsconnx local", "", local);
  if (local) {
    x = ask ? hupok(1) : 1; /* Make sure it's OK to close */
    if (!x) {
      rc = -1;
      debug(F101, "clsconnx hupok says no", "", rc);
      return (rc);
    }
    ttflui(); /* Clear away buffered up junk */
#ifndef NODIAL
    mdmhup();
#endif /* NODIAL */
    if (network && msgflg) {
      printf(" Closing connection\n");
    }
    ttclos(0); /* Close old connection, if any */
    rc = 1;
    {
      extern int wasclosed, whyclosed;
      if (wasclosed) {
        whyclosed = WC_CLOS;
#ifndef NOSPL
        if (nmac) { /* Any macros defined? */
          int k;    /* Yes */
          /* printf("ON_CLOSE CLSCONNX\n"); */
          wasclosed = 0;
          k = mlook(mactab, "on_close", nmac);        /* Look this up */
          if (k >= 0) {                               /* If found, */
            if (dodo(k, ckitoa(whyclosed), 0) > -1) { /* set it up, */
              parser(1);                              /* and execute it */
            }
          }
        }
#endif /* NOSPL */
        whyclosed = WC_REMO;
        wasclosed = 0;
      }
    }
  }
  dologend();
  haveline = 0;
  if (mdmtyp < 0) {    /* Switching from net to async? */
    if (mdmsav > -1) { /* Restore modem type from last */
      mdmtyp = mdmsav; /* SET MODEM command, if any. */
    } else {
      mdmtyp = 0;
    }
    mdmsav = -1;
  }
  if (network) {
    network = 0;
  }
#ifdef NETCONN
  if (oldplex > -1) { /* Restore previous duplex setting. */
    duplex = oldplex;
    oldplex = -1;
  }
#endif                                /* NETCONN */
  ckstrncpy(ttname, dftty, TTNAMLEN); /* Restore default communication */
  local = dfloc;                      /* device and local/remote status */
  if (local) {
    cxtype = CXT_DIRECT; /* Something reasonable */
    speed = ttgspd();    /* Get the current speed */
  } else {
    cxtype = CXT_REMOTE;
    speed = -1L;
  }
#ifndef NOXFER
  if (xreliable > -1 && !setreliable) {
    reliable = xreliable;
    debug(F101, "clsconnx reliable A", "", reliable);
  } else if (!setreliable) {
    reliable = SET_AUTO;
    debug(F101, "clsconnx reliable B", "", reliable);
  }
#endif       /* NOXFER */
  setflow(); /* Revert flow control */
  return (rc);
}

int clskconnx(int x) /* Close Kermit connection only */
{
  int t, rc; /* (not FTP) */
#ifdef NEWFTP
  extern int ftpget;
  t = ftpget;
  ftpget = 0;
#endif /* NEWFTP */
  rc = clsconnx(x);
#ifdef NEWFTP
  ftpget = t;
#endif /* NEWFTP */
  return (rc);
}

/* May 2002: setlin() decomposition starts here ... */

#define SRVBUFSIZ 63
#define HOSTNAMLEN 15 * 65

int netsave = -1;
static char *tmpstring = NULL;
static char *tmpusrid = NULL;

#ifdef SSHCMD
char *sshcmd = NULL;
char *defsshcmd = "ssh -e none";
#else
#ifdef SSHBUILTIN
char *sshrcmd = NULL;
char *sshtmpcmd = NULL;
#endif /* SSHBUILTIN */
#endif /* SSHCMD */

/* c x _ f a i l  --  Common error exit routine for cx_net, cx_line */

int cx_fail(int msg, char *text) {
  makestr(&slmsg, text);   /* For the record (or GUI) */
  if (msg) {               /* Not GUI, not quiet, etc */
    printf("?%s\n", text); /* Print error message */
  }
  slrestor();                        /* Restore LINE/HOST to known state */
  return (msg ? -9 : (success = 0)); /* Return appropriate code */
}
/* c x _ n e t  --  Make a network connection */

/*
  Call with:
    net      = network type
    protocol = protocol type
    host     = string pointer to host name.
    svc      = string pointer to service or port on host.
    username = username for connection
    password = password for connection
    command  = command to execute
    param1   = Telnet: Authentication type
               SSH:    Version
    param2   = Telnet: Encryption type
               SSH:    Command as Subsystem
    param3   = Telnet: 1 to wait for negotiations, 0 otherwise
               SSH:    X11 Forwarding
    cx       = 1 to automatically enter Connect mode, 0 otherwise.
    sx       = 1 to automatically enter Server mode, 0 otherwise.
    flag     = if no host name given, 1 = close current connection, 0 = resume
    gui      = 1 if called from GUI dialog, 0 otherwise.
  Returns:
    1 on success
    0 on failure and no message printed, slmsg set to failure message.
   -9 on failure and message printed, ditto.
*/
int cx_net(int net, int protocol, char *xhost, char *svc, char *username,
           char *password, char *command, int param1, int param2, int param3,
           int cx, int sx, int flag, int gui)
/* cx_net */ {

  int i, n = 1, x, msg;
  int _local = -1;
  int did_ttopen = 0;

  extern char pwbuf[], *g_pswd;
  extern int pwflg, pwcrypt, g_pflg, g_pcpt, nolocal;

  char srvbuf[SRVBUFSIZ + 1]; /* Service */
  char hostbuf[HOSTNAMLEN];   /* Host buffer to manipulate */
  char hostname[HOSTNAMLEN];  /* Copy of host parameter */
  char *host = hostbuf;       /* Pointer to copy of host param */

  if (!xhost) {
    xhost = ""; /* Watch out for null pointers */
  }
  if (!svc) {
    svc = "";
  }
  ckstrncpy(host, xhost, HOSTNAMLEN); /* Avoid buffer confusion */

  debug(F110, "cx_net host", host, 0);
  debug(F111, "cx_net service buffer size", svc, SRVBUFSIZ);
  debug(F101, "cx_net network type", "", net);

  msg = (gui == 0) && msgflg; /* Whether to print messages */

#ifndef NODIAL
#ifndef NONETDIR
  debug(F101, "cx_net nnetdir", "", nnetdir);
  x = 0;              /* Look up in network directory */
  if (*host == '=') { /* If number starts with = sign */
    host++;           /* strip it */
    while (*host == SP) {
      host++; /* and any leading spaces */
    }
    debug(F110, "cx_net host 2", host, 0);
    nhcount = 0;
  } else if (*host) {  /* We want to look it up. */
    if (nnetdir > 0) { /* If there is a directory... */
      x = lunet(host); /* (sets nhcount) */
    } else {           /* otherwise */
      nhcount = 0;     /* we didn't find any there */
    }
    if (x < 0) { /* Internal error? */
      return (cx_fail(msg, "Network directory lookup error"));
    }
    debug(F111, "cx_net lunet nhcount", host, nhcount);
  }
#endif /* NONETDIR */
#endif /* NODIAL */

  /* New connection wanted.  Make a copy of the host name/address... */

  debug(F100, "cx_net A", "", 0);
  if (clskconnx(1) < 0) { /* Close current Kermit connection */
    return (cx_fail(msg, "Error closing previous connection"));
  }

  debug(F100, "cx_net B", "", 0);
  if (*host) {  /* They gave a hostname */
    _local = 1; /* Network connection always local */
    if (mdmsav < 0) {
      mdmsav = mdmtyp; /* Remember old modem type */
    }
    mdmtyp = -net;  /* Special code for network */
  } else {          /* They just said "set host" */
    host = dftty;   /* So go back to normal */
    _local = dfloc; /* default tty, location, */
    if (flag) {     /* Close current connection */
      setflow();    /* Maybe change flow control */
      haveline = 1; /* (* is this right? *) */
      dologend();
#ifndef NODIAL
      dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
      if (autolocus) {
        setlocus(1, 1);
      }
#endif /* LOCUS */
       /* XXX - Is this right? */
      /* Should we be returning without doing anything ? */
      /* Yes it's right -- we closed the old connection just above. */
      return (success = 1);
    }
  }
  success = 0;
  if (host != line) { /* line[] is a global */
    ckstrncpy(line, host, LINBUFSIZ);
  }
  ckstrncpy(hostname, host, HOSTNAMLEN);
  ckstrncpy(srvbuf, svc, SRVBUFSIZ + 1);
  debug(F110, "cx_net hostname", host, 0);
  debug(F110, "cx_net srvbuf", srvbuf, 0);

#ifndef NONETDIR
#ifndef NODIAL
  if ((nhcount > 1) && msg) {
    int k;
    printf("%d entr%s found for \"%s\"%s\n", nhcount,
           (nhcount == 1) ? "y" : "ies", s, (nhcount > 0) ? ":" : ".");
    for (i = 0; i < nhcount; i++) {
      printf("%3d. %-12s => %-9s %s", i + 1, n_name, nh_p2[i], nh_p[i]);
      for (k = 0; k < 4; k++) { /* Also list net-specific items */
        if (nh_px[k][i]) {      /* free format... */
          printf(" %s", nh_px[k][i]);
        } else {
          break;
        }
      }
      printf("\n");
    }
  }
  if (nhcount == 0) {
    n = 1;
  } else {
    n = nhcount;
  }
#else
  n = 1;
  nhcount = 0;
#endif /* NODIAL */
  n = 1;
#endif /* NONETDIR */

  for (i = 0; i < n; i++) { /* Loop for each entry found */
    debug(F101, "cx_net loop i", "", i);
#ifndef NODIAL
#ifndef NONETDIR
    if (nhcount > 0) { /* If we found at least one entry... */
      ckstrncpy(line, nh_p[i], LINBUFSIZ);            /* Copy current entry */
      if (lookup(netcmd, nh_p2[i], nnets, &x) > -1) { /* Net type */
        int xx;
        xx = netcmd[x].kwval;
        /* User specified SSH so don't let net directory override */
        if (net != NET_SSH || xx != NET_TCPB) {
          net = xx;
          mdmtyp = 0 - net;
        }
      } else {
        makestr(&slmsg, "Network type not supported");
        if (msg) {
          printf("Error - network type \"%s\" not supported\n", nh_p2[i]);
        }
        continue;
      }
      switch (net) { /* Net-specific directory things */
#ifdef SSHBUILTIN
      case NET_SSH: /* SSH */
        /* Any SSH specific network directory stuff? */
        break; /* NET_SSH */
#endif         /* SSHBUILTIN */

      case NET_TCPB: { /* TCP/IP TELNET,RLOGIN,... */
#ifdef TCPSOCKET
        char *q;
        int flag = 0;

        /* Extract ":service", if any, from host string */
        debug(F110, "cx_net service 1", line, 0);
        for (q = line; (*q != '\0') && (*q != ':'); q++)
          ;
        if (*q == ':') {
          *q++ = NUL;
          flag = 1;
        }
        debug(F111, "cx_net service 2", line, flag);

        /* Get service, if any, from directory entry */

        if (!*srvbuf) {
          if (nh_px[0][i]) {
            ckstrncpy(srvbuf, nh_px[0][i], SRVBUFSIZ);
            debug(F110, "cx_net service 3", srvbuf, 0);
          }
          if (flag) {
            ckstrncpy(srvbuf, q, SRVBUFSIZ);
            debug(F110, "cx_net service 4", srvbuf, 0);
          }
        }
        ckstrncpy(hostname, line, HOSTNAMLEN);

        /* If we have a service, append to host name/address */
        if (*srvbuf) {
          ckstrncat(line, ":", LINBUFSIZ);
          ckstrncat(line, srvbuf, LINBUFSIZ);
          debug(F110, "cx_net service 5", line, 0);
        }
#ifdef RLOGCODE
        /* If no service given but command was RLOGIN */
        else if (ttnproto == NP_RLOGIN) { /* add this... */
          ckstrncat(line, ":login", LINBUFSIZ);
          debug(F110, "cx_net service 6", line, 0);
        }
#endif         /* RLOGCODE */
        else { /* Otherwise, add ":telnet". */
          ckstrncat(line, ":telnet", LINBUFSIZ);
          debug(F110, "cx_net service 9", line, 0);
        }
        if (username) { /* This is a parameter... */
          ckstrncpy(uidbuf, username, UIDBUFLEN);
          uidflag = 1;
        }
        /* Fifth field, if any, is user ID (for rlogin) */

        if (nh_px[1][i] && !uidflag) {
          ckstrncpy(uidbuf, username, UIDBUFLEN);
        }
#ifdef RLOGCODE
        if (IS_RLOGIN() && !uidbuf[0]) {
          return (cx_fail(msg, "Username required"));
        }
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
        break;
      }
      case NET_PIPE: /* Pipe */
#ifdef NPIPE
        if (!pipename[0]) {  /* User didn't give a pipename */
          if (nh_px[0][i]) { /* But directory entry has one */
            if (strcmp(pipename, "\\pipe\\")) {
              ckstrncpy(pipename, "\\pipe\\", LINBUFSIZ);
              ckstrncat(srvbuf, nh_px[0][i], PIPENAML - 6);
            } else {
              ckstrncpy(pipename, nh_px[0][i], PIPENAML);
            }
            debug(F110, "cx_net pipeneme", pipename, 0);
          }
        }
#endif /* NPIPE */
        break;

      case NET_SLAT: /* LAT / CTERM */
#ifdef SUPERLAT
        if (!slat_pwd[0]) {  /* User didn't give a password */
          if (nh_px[0][i]) { /* But directory entry has one */
            ckstrncpy(slat_pwd, nh_px[0][i], 18);
            debug(F110, "cx_net SuperLAT password", slat_pwd, 0);
          }
        }
#endif /* SUPERLAT */
        break;

      case NET_SX25: /* X.25 keyword parameters */
      case NET_IX25:
      case NET_VX25: {
#ifdef ANYX25
        int k; /* Cycle through the four fields */
        for (k = 0; k < 4; k++) {
          if (!nh_px[k][i]) { /* Bail out if none left */
            break;
          }
          if (!ckstrcmp(nh_px[k][i], "cug=", 4, 0)) {
            closgr = atoi(nh_px[k][i] + 4);
            debug(F101, "X25 CUG", "", closgr);
          } else if (!ckstrcmp(nh_px[k][i], "cud=", 4, 0)) {
            cudata = 1;
            ckstrncpy(udata, nh_px[k][i] + 4, MAXCUDATA);
            debug(F110, "X25 CUD", cudata, 0);
          } else if (!ckstrcmp(nh_px[k][i], "rev=", 4, 0)) {
            revcall = !ckstrcmp(nh_px[k][i] + 4, "=on", 3, 0);
            debug(F101, "X25 REV", "", revcall);
#ifndef IBMX25
          } else if (!ckstrcmp(nh_px[k][i], "pad=", 4, 0)) {
            int x3par, x3val;
            char *s1, *s2;
            s1 = s2 = nh_px[k][i] + 4; /* PAD parameters */
            while (*s2) {              /* Pick them apart */
              if (*s2 == ':') {
                *s2 = NUL;
                x3par = atoi(s1);
                s1 = ++s2;
                continue;
              } else if (*s2 == ',') {
                *s2 = NUL;
                x3val = atoi(s1);
                s1 = ++s2;
                debug(F111, "X25 PAD", x3par, x3val);
                if (x3par > -1 && x3par <= MAXPADPARMS) {
                  padparms[x3par] = x3val;
                }
                continue;
              } else {
                s2++;
              }
            }
#endif /* IBMX25 */
          }
        }
#endif /* ANYX25 */
        break;
      }
      default: /* Nothing special for other nets */
        break;
      }
    } else
#endif /* NODIAL */
#endif /* NONETDIR */

    {                                       /* No directory entries found. */
      ckstrncpy(line, hostname, LINBUFSIZ); /* Put this back... */
      debug(F110, "cx_net after loop loop", line, 0);

      /* If the user gave a TCP service */
      if (net == NET_TCPB || net == NET_SSH) {
        if (*srvbuf) { /* Append it to host name/address */
          ckstrncat(line, ":", LINBUFSIZ);
          ckstrncat(line, srvbuf, LINBUFSIZ);
        }
      }
    }
    /*
       Get here with host name/address and all net-specific
       parameters set, ready to open the connection.
    */
    mdmtyp = -net; /* This should have been done */
                   /* already but just in case ... */

    debug(F110, "cx_net net line[] before ttopen", line, 0);
    debug(F101, "cx_net net mdmtyp before ttopen", "", mdmtyp);
    debug(F101, "cx_net net ttnproto", "", ttnproto);

#ifdef SSHBUILTIN
    if (net == NET_SSH) {
      ssh_set_sparam(SSH_SPARAM_HST, hostname); /* Stash everything */
      if (username) {
        if (!sl_uid_saved) {
          ckstrncpy(sl_uidbuf, uidbuf, UIDBUFLEN);
          sl_uid_saved = 1;
        }
        ckstrncpy(uidbuf, username, UIDBUFLEN);
      }
      if (srvbuf[0]) {
        ssh_set_sparam(SSH_SPARAM_PRT, srvbuf);
      } else {
        ssh_set_sparam(SSH_SPARAM_PRT, NULL);
      }

      if (command) {
        ssh_set_sparam(SSH_SPARAM_CMD, brstrip(command));
        ssh_set_iparam(SSH_IPARAM_CAS, param2);
      } else {
        ssh_set_sparam(SSH_SPARAM_CMD, NULL);
      }

      if (param1 > -1) {
#ifndef SSHTEST
        if (!sl_ssh_ver_saved) {
          sl_ssh_ver = ssh_get_iparam(SSH_IPARAM_VER);
          sl_ssh_ver_saved = 1;
        }
#endif /* SSHTEST */
        ssh_set_iparam(SSH_IPARAM_VER, param1);
      }
      if (param3 > -1) {
#ifndef SSHTEST
        if (!sl_ssh_xfw_saved) {
          sl_ssh_xfw = ssh_get_iparam(SSH_IPARAM_XFW);
          sl_ssh_xfw_saved = 1;
        }
#endif /* SSHTEST */
        ssh_set_iparam(SSH_IPARAM_XFW, param3);
      }
    } else /* NET_SSH */
#endif     /* SSHBUILTIN */
#ifdef TCPSOCKET
        if (net == NET_TCPB) {
      switch (protocol) {

      case NP_NONE:
      case NP_TCPRAW:
      case NP_RLOGIN:
      case NP_K4LOGIN:
      case NP_K5LOGIN:
      case NP_EK4LOGIN:
      case NP_EK5LOGIN:
      case NP_TELNET:
      case NP_KERMIT:
      default:
        ttnproto = protocol;
        break;
      }
#ifdef RLOGCODE
#endif /* RLOGCODE */

#ifndef NOSPL
#ifdef RLOGCODE
      if (username) {
        if (!sl_uid_saved) {
          ckstrncpy(sl_uidbuf, uidbuf, UIDBUFLEN);
          sl_uid_saved = 1;
        }
        ckstrncpy(uidbuf, username, UIDBUFLEN);
        uidflag = 1;
      }
#endif /* RLOGCODE */
#ifdef TNCODE
      if (!sl_tn_saved) {
        sl_tn_wait = tn_wait_flg;
        sl_tn_saved = 1;
      }
      tn_wait_flg = param3;
#endif /* TNCODE */
#endif /* NOSPL */
    } /* if (net == NET_TCPB) */
#endif /* TCPSOCKET */

#ifndef NOSPL
#endif /* NOSPL */

    /* Try to open - network */
    ckstrncpy(ttname, line, TTNAMLEN);
    y = ttopen(line, &_local, mdmtyp, 0);
    did_ttopen++;
    debug(F101, "cx_net did_ttopen A", "", did_ttopen);

#ifndef NOHTTP
    /*  If the connection failed and we are using an HTTP Proxy
     *  and the reason for the failure was an authentication
     *  error, then we need to give the user to ability to
     *  enter a username and password, just like a browser.
     *
     *  I tried to do all of this within the netopen() call
     *  but it is much too much work.
     */
    while (y < 0 && tcp_http_proxy != NULL) {

      if (tcp_http_proxy_errno == 401 || tcp_http_proxy_errno == 407) {
        char uid[UIDBUFLEN];
        char pwd[256];
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

          ckstrncpy(ttname, line, TTNAMLEN);
          y = ttopen(line, &_local, mdmtyp, 0);
          debug(F101, "cx_net did_ttopen B", "", did_ttopen);
          memset(pwd, 0, sizeof(pwd));
          tcp_http_proxy_user = proxy_user;
          tcp_http_proxy_pwd = proxy_pwd;
        } else {
          break;
        }
      } else {
        break;
      }
    }
#endif /* NOHTTP */
    if (y < 0) {
      slrestor();
      makestr(&slmsg, "Network connection failure");
      if (errno) {
        debug(F111, "set host line, errno", "", errno);
        makestr(&slmsg, ck_errstr());
        if (msg) {
#ifdef UNIX
          if (hints && !xcmdsrc && IS_RLOGIN()) {
            makestr(&slmsg, "RLOGIN failure");
            printf("*************************\n");
            printf(
                "Hint: RLOGIN requires privileges to open an outbound port.\n");
            printf("(Use SET HINTS OFF to suppress future hints.)\n");
            printf("*************************\n");
          }
#endif /* UNIX */
        } else {
          printf("Can't connect to %s\n", line);
        }
      } else if (msg) {
        printf("Can't open connection to %s\n", line);
      }
      continue;
    } else {
      success = 1;
#ifndef NODIAL
      dialsta = DIA_UNK;
#endif /* NODIAL */
      switch (net) {
      case NET_TCPA:
      case NET_TCPB:
        cxtype = CXT_TCPIP;
        break;
      case NET_SSH:
        cxtype = CXT_SSH;
        duplex = 0; /* Remote echo */
        break;
      case NET_SLAT:
        cxtype = CXT_LAT;
        break;
      case NET_SX25:
      case NET_IX25:
      case NET_HX25:
      case NET_VX25:
        cxtype = CXT_X25;
        break;
      case NET_BIOS:
        cxtype = CXT_NETBIOS;
        break;
      case NET_FILE:
      case NET_PIPE:
      case NET_CMD:
      case NET_DLL:
      case NET_PTY:
        cxtype = CXT_PIPE;
        break;
      default:
        cxtype = CXT_PIPE;
        break;
      }
      break;
    }
  } /* for-loop */
  s = line;

  debug(F101, "cx_net after for-loop did_ttopen", "", did_ttopen);
  if (did_ttopen == 0) {
    debug(F100, "cx_net didn't call ttopen - calling it now", "", 0);
    y = ttopen(line, &_local, mdmtyp, 0);
    debug(F101, "cx_net ttopen return code", "", y);
    debug(F101, "cx_net ttopen _local", "", _local);
    did_ttopen++;
    ckstrncpy(ttname, line, TTNAMLEN);
    success = 0;
    if (y > 0) {
      success = 1;
    }
  }
  debug(F101, "cx_net post ttopen success", "", success);

  if (!success) {
    local = dfloc;                      /* Go back to normal */
    ckstrncpy(ttname, dftty, TTNAMLEN); /* Restore default tty name */
    speed = ttgspd();
    network = 0; /* No network connection active */
    haveline = 0;
    if (mdmtyp < 0) {    /* Switching from net to async? */
      if (mdmsav > -1) { /* Restore modem type from last */
        mdmtyp = mdmsav; /* SET MODEM command, if any. */
      } else {
        mdmtyp = 0;
      }
      mdmsav = -1;
    }
    return (0); /* Return failure */
  }
  if (_local > -1) {
    local = _local; /* Opened ok, set local/remote. */
  }
  makestr(&slmsg, NULL);
  network = (mdmtyp < 0);         /* Remember connection type. */
  ckstrncpy(ttname, s, TTNAMLEN); /* Copy name into real place. */
  debug(F110, "cx_net ok", ttname, 0);
  debug(F101, "cx_net network", "", network);
#ifndef NOXFER
  if ((reliable != SET_OFF || !setreliable)) { /* Assume not reliable. */
    reliable = SET_OFF;
  }
#endif /* NOXFER */
  if (!network
#ifdef NETCOMM
      || istncomport()
#endif /* NETCOMM */
  )
    speed = ttgspd(); /* Get the current speed. */
  debug(F101, "cx_net local", "", local);
  if (network) {
    debug(F101, "cx_net net", "", net);
#ifndef NOXFER
    /* Force prefixing of 255 on TCP/IP connections... */
    if (net == NET_TCPB
#ifdef SSHBUILTIN
        || net == NET_SSH
#endif /* SSHBUILTIN */
    ) {
      debug(F101, "cx_net reliable A", "", reliable);
#ifdef CK_SPEED
      ctlp[(unsigned)255] = 1;
#endif /* CK_SPEED */
      if ((reliable != SET_OFF || !setreliable)) {
#ifdef TN_COMPORT
        if (istncomport()) {  /* Telnet communication port */
          reliable = SET_OFF; /* Transport is not reliable */
          debug(F101, "cx_net reliable istncomport()", "", 1);
        } else {
          reliable = SET_ON; /* Transport is reliable end to end */
          debug(F101, "cx_net reliable istncomport()", "", 0);
        }
#else
        reliable = SET_ON; /* Transport is reliable end to end */
#endif /* ifdef TN_COMPORT */
      }
      debug(F101, "cx_net reliable B", "", reliable);
    } else if (net == NET_SX25 || net == NET_VX25 || net == NET_IX25 ||
               net == NET_HX25) {
      duplex = 1; /* Local echo for X.25 */
      if (reliable != SET_OFF || !setreliable) {
        reliable = SET_ON; /* Transport is reliable end to end */
      }
    }
#endif /* NOXFER */
  }
#ifndef NOXFER
  debug(F101, "cx_net reliable", "", reliable);
#endif /* NOXFER */

  /*xcx_net:*/

  setflow(); /* Set appropriate flow control */

  haveline = 1;
#ifdef NETCONN
#ifdef CKLOGDIAL
  dolognet();
#endif /* CKLOGDIAL */
#endif /* NETCONN */

#ifndef NOSPL
  if (local) {
    if (nmac) {                           /* Any macros defined? */
      int k;                              /* Yes */
      k = mlook(mactab, "on_open", nmac); /* Look this up */
      if (k >= 0) {                       /* If found, */
        if (dodo(k, ttname, 0) > -1) {    /* set it up, */
          parser(1);                      /* and execute it */
        }
      }
    }
  }
#endif /* NOSPL */

  if (local && (cx || sx)) { /* /CONNECT or /SERVER switch given */
    if (cx) {                /* /CONNECT */
      if (!gui) {
        /* Command was confirmed so we can pre-pop command level.  */
        /* This is so CONNECT module won't think we're executing a */
        /* script if CONNECT was the final command in the script.  */
        if (cmdlvl > 0) {
          prepop();
        }
      }
#ifndef NODIAL
      dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
      if (autolocus) {
        setlocus(1, 1);
      }
#endif /* LOCUS */
      success = doconect(0, cmdlvl == 0 ? 1 : 0);
      if (ttchk() < 0) {
        dologend();
      }
      debug(F101, "cx_net post doconect success", "", success);
      return (success);
#ifndef NOXFER
    } else if (sx) { /* /SERVER */
      sstate = 'x';
      if (local) {
        displa = 1;
      }
#endif /* NOXFER */
    }
  }
#ifndef NODIAL
  dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
  if (autolocus) {
    setlocus(1, 1);
  }
#endif /* LOCUS */
  return (success = 1);
}

/* c x _ s e r i a l  --  Make a serial connection */

/*
  Call with:
    device  = string pointer to device name.
    cx      = 1 to automatically enter Connect mode, 0 otherwise.
    sx      = 1 to automatically enter Server mode, 0 otherwise.
    shr     = 1 if device should be opened in shareable mode, 0 otherwise.
    flag    = if no dev name given: 1 = close current connection, 0 = resume.
    gui     = 1 if called from GUI dialog, 0 otherwise.
  Returns:
    1 on success
    0 on failure and no message printed, slmsg set to failure message.
   -9 on failure and message printed, ditto.
*/

/* these are bit flags */
#define CX_TAPI 1
#define CX_PPP 2
#define CX_SLIP 4

int cx_serial(char *device, int cx, int sx, int shr, int flag, int gui,
              int special)
/* cx_serial */ {
  int y, msg;
  int _local = -1;
  char *s;

  debug(F110, "cx_serial device", device, 0);
  s = device;
  msg = (gui == 0) && msgflg; /* Whether to print messages */
  success = 0;

#ifndef NODIAL
  dialsta = DIA_UNK;
#endif /* NODIAL */
  debug(F101, "cx_serial mdmtyp", "", mdmtyp);
  if (clskconnx(1) < 0) { /* Close the Kermit connection */
    return (success = 0);
  }
  if (*s) {         /* They gave a device name */
    _local = -1;    /* Let ttopen decide about it */
  } else {          /* They just said "set line" */
    s = dftty;      /* so go back to normal tty */
    _local = dfloc; /* and mode. */
  }

  /* Open the new line */

  ckstrncpy(ttname, s, TTNAMLEN);
  if ((y = ttopen(s, &_local, mdmtyp, cdtimo)) > -1) {
    cxtype = (mdmtyp > 0) ? CXT_MODEM : CXT_DIRECT;
#ifndef NODIAL
    dialsta = DIA_UNK;
#ifdef CK_TAPI
    /* if the line is a tapi device, then we need to auto-execute */
    /* SET MODEM TYPE TAPI - which we do the equivalent of here.  */
    if (tttapi) {
      extern int usermdm;
      usermdm = 0;
      initmdm(38); /* From ckudia.c n_TAPI == 38 */
    }
#endif /* CK_TAPI */
#endif /* NODIAL */
    success = 1;
  } else { /* Failed */
    if (y == -2) {
      makestr(&slmsg, "Timed out - no carrier");
      if (msg) {
        printf("?%s\n", slmsg);
        if (hints) {
          printf("\n*************************\n");
          printf("HINT (Use SET HINTS OFF to suppress future hints):\n");
          printf("Try SET CARRIER OFF and SET LINE again, or else\n");
          printf("SET MODEM, SET LINE, and then DIAL.\n");
          printf("*************************\n\n");
        }
      }
    } else if (y == -3) {
      makestr(&slmsg, "Access to lock denied");
      if (msg) {
#ifdef UNIX
        printf("Sorry, write access to UUCP lockfile directory denied.\n");
#ifndef NOHINTS
        if (hints) {
          printf("\n*************************\n");
          printf("HINT (Use SET HINTS OFF to suppress future hints):\n");
          printf(
              "Please read the installation instructions file, %sckuins.txt,\n",
              k_info_dir ? k_info_dir : "");
          printf("or the UNIX appendix of the manual, \"Using C-Kermit\"\n");
          printf("or visit http://www.kermitproject.org/ckuins.html \n");
          printf("*************************\n\n");
        }
#endif /* NOHINTS */
#else
        printf("Sorry, access to lock denied: %s\n", s);
#endif /* UNIX */
      }
    } else if (y == -4) {
      makestr(&slmsg, "Access to device denied");
      if (msg) {
        printf("Sorry, access to device denied: %s\n", s);
#ifdef UNIX
#ifndef NOHINTS
        if (hints) {
          printf("\n*************************\n");
          printf("HINT (Use SET HINTS OFF to suppress future hints):\n");
          printf(
              "Please read the installation instructions file, %sckuins.txt,\n",
              k_info_dir ? k_info_dir : "");
          printf("or the UNIX appendix of the manual, \"Using C-Kermit\".\n");
          printf("*************************\n\n");
        }
#endif /* NOHINTS */
#endif /* UNIX */
      }
    } else if (y == -5) {
      makestr(&slmsg, "Device is in use or unavailable");
      if (msg) {
        printf("Sorry, device is in use: %s\n", s);
      }
    } else { /* Other error. */
      makestr(&slmsg, "Device open failed");
      if (errno) {
        makestr(&slmsg, ck_errstr());
        debug(F111, "cx_serial serial errno", slmsg, errno);
        if (msg) {
          printf("Connection to %s failed: %s\n", s, slmsg);
        }
      } else if (msg) {
        printf("Sorry, can't open connection: %s\n", s);
      }
    }
  }
  network = 0; /* No network connection active */
  speed = ttgspd();
  if (!success) {
    local = dfloc;                      /* Go back to normal */
    ckstrncpy(ttname, dftty, TTNAMLEN); /* Restore default tty name */
    haveline = 0;
    if (mdmtyp < 0) {    /* Switching from net to async? */
      if (mdmsav > -1) { /* Restore modem type from last */
        mdmtyp = mdmsav; /* SET MODEM command, if any. */
      } else {
        mdmtyp = 0;
      }
      mdmsav = -1;
    }
    return (msg ? -9 : 0); /* Return failure */
  }
  if (_local > -1) {
    local = _local; /* Opened ok, set local/remote. */
  }
  makestr(&slmsg, NULL);          /* Erase SET LINE message */
  ckstrncpy(ttname, s, TTNAMLEN); /* Copy name into real place. */
  debug(F110, "cx_serial ok", ttname, 0);
#ifndef NOXFER
  if ((reliable != SET_OFF || !setreliable)) { /* Assume not reliable. */
    reliable = SET_OFF;
  }
#endif /* NOXFER */

  /*xcx_serial:*/
  setflow(); /* Set appropriate flow control */
  haveline = 1;
#ifdef CKLOGDIAL
  dologline();
#endif /* CKLOGDIAL */

#ifndef NOSPL
  if (local) {
    if (nmac) {                           /* Any macros defined? */
      int k;                              /* Yes */
      k = mlook(mactab, "on_open", nmac); /* Look this up */
      if (k >= 0) {                       /* If found, */
        if (dodo(k, ttname, 0) > -1) {    /* set it up, */
          parser(1);                      /* and execute it */
        }
      }
    }
  }
#endif /* NOSPL */

  if (local && (cx || sx)) { /* /CONNECT or /SERVER switch given */
    extern int carrier;
    if (carrier != CAR_OFF) { /* Looking for carrier? */
      /* Open() turns on DTR -- wait up to a second for CD to come up */
      int i, x;
      for (i = 0; i < 10; i++) { /* WAIT 1 CD... */
        x = ttgmdm();
        if (x < 0 || x & BM_DCD) {
          break;
        }
        msleep(100);
      }
    }
    if (cx) { /* /CONNECT */
      /* Command was confirmed so we can pre-pop command level. */
      /* This is so CONNECT module won't think we're executing a */
      /* script if CONNECT was the final command in the script. */

      if (cmdlvl > 0) {
        prepop();
      }
#ifndef NODIAL
      dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
      if (autolocus) {
        setlocus(1, 1);
      }
#endif /* LOCUS */
      success = doconect(0, cmdlvl == 0 ? 1 : 0);
      if (ttchk() < 0) {
        dologend();
      }
      return (success);
#ifndef NOXFER
    } else if (sx) { /* /SERVER */
      sstate = 'x';
      if (local) {
        displa = 1;
      }
#endif /* NOXFER */
    }
  }
#ifndef NODIAL
  dialsta = DIA_UNK;
#endif /* NODIAL */
#ifdef LOCUS
  if (autolocus) {
    setlocus(1, 1);
  }
#endif /* LOCUS */
  return (success = 1);
}

/* S E T L I N -- parse name of and then open communication device. */
/*
  Call with:
    xx == XYLINE for a serial (tty) line, XYHOST for a network host,
    zz == 0 means if user doesn't give a device name, continue current
            active connection (if any);
    zz != 0 means if user doesn't give a device name, then close the
            current connection and restore the default communication device.
    fc == 0 to just make the connection, 1 to also CONNECT (e.g. "telnet").
*/
int setlin(int xx, int zz, int fc) {
  extern char pwbuf[], *g_pswd;
  extern int pwflg, pwcrypt, g_pflg, g_pcpt, nolocal;
  int wait;
  /* int tn_wait_sv; */
  int mynet;
  int c, i, haveswitch = 0;
  int haveuser = 0;
  int getval = 0;
  int wild = 0;      /* Filespec has wildcards */
  int cx = 0;        /* Connect after */
  int sx = 0;        /* Become server after */
  int a_type = -1;   /* Authentication type */
  int e_type = -1;   /* Telnet /ENCRYPT type */
  int shr = 0;       /* Share serial device */
  int confirmed = 0; /* Command has been entered */
  struct FDB sw, nx;
  struct FDB tx;

  char *ss;
#ifdef TCPSOCKET
  int rawflg = 0;
#endif /* TCPSOCKET */

  char srvbuf[SRVBUFSIZ + 1];

  int dossh = 0;

  debug(F101, "setlin fc", "", fc);
  debug(F101, "setlin zz", "", zz);
  debug(F101, "setlin xx", "", xx);

#ifdef SSHCMD
  if (xx == XXSSH) { /* SSH becomes PTY SSH ... */
    dossh = 1;
    xx = XYHOST;
  } else if (!ckstrcmp("ssh ", line, 4, 0)) { /* 2010/03/01 */
    dossh = 1;
    xx = XYHOST;
  }
  debug(F101, "setlin dossh", "", dossh);
#endif /* SSHCMD */

#ifdef TNCODE
  /* tn_wait_sv = tn_wait_flg; */
  wait = tn_wait_flg;
#else
  /* tn_wait_sv = 0; */
  wait = 0;
#endif /* TNCODE */

  mynet = nettype;

  if (nolocal) {
    makestr(&slmsg, "Making connections is disabled");
    printf("?Sorry, making connections is disabled\n");
    return (-9);
  }
  if (netsave > -1) {
    nettype = netsave;
  }

  if (fc != 0 || zz == 0) { /* Preset /CONNECT switch */
    cx = 1;
  }

  debug(F101, "setlin cx", "", cx);

  *srvbuf = NUL;

  line[0] = NUL;
  s = line;

#ifdef NETCONN
  if (tmpusrid) {
    makestr(&tmpusrid, NULL);
  }
#endif /* NETCONN */

  autoflow = 1; /* Enable automatic flow setting */
  debug(F101, "setlin xx", "", xx);

#ifdef SSHCMD
  debug(F100, "setlin SSHCMD", "", 0);
#endif                /* SSHCMD */
  if (xx == XYHOST) { /* SET HOST <hostname> */
    debug(F100, "setlin XYHOST", "", 0);
#ifndef NETCONN
#ifndef SSHCMD
    debug(F100, "setlin XXX", "", 0);
    makestr(&slmsg, "Network connections not configured");
    printf("?%s\n", slmsg);
    return (-9);
#endif /* SSHCMD */
#endif /* NETCONN */

#ifndef NOPUSH
    debug(F101, "setlin mynet", "", mynet); /* NET_CMD = 11, NET_PTY = 15 */
    if ((mynet == NET_CMD || mynet == NET_PTY || dossh) && nopush) {
      makestr(&slmsg, "Access to external commands is disabled");
      printf("?Sorry, access to external commands is disabled\n");
      return (-9);
    }
#endif /* NOPUSH */

    debug(F101, "setlin dossh abc", "", dossh);
#ifdef SSHCMD
    debug(F101, "setlin dossh def", "", dossh);
    if (dossh) { /* SSH connection via pty */
      int k, q;
      int have_host = 0;
      extern int ttyfd; /* 2010/03/01 */
      k = ckstrncpy(line, sshcmd ? sshcmd : defsshcmd, LINBUFSIZ);
      debug(F111, "setlin sshcmd 1", line, k);
      if ((x = cmtxt("Optional switches and hostname", "", &s, xxstring)) < 0) {
        return (x);
      }
      debug(F111, "setlin dossh cmtxt", s, 1);
      debug(F110, "setlin dossh ttname", ttname, 0);
      if (!*s) {
        debug(F111, "setlin dossh cmtxt is EMPTY", s, x);
      }
      q = (int)strlen(s);
      debug(F111, "setlin dossh IF strlen(s)", s, q);
      if (q > 0) {
        have_host = 1;
      }

      /* 2010-03-30 */
      if ((!q && (ttyfd < 0)) && !ckstrcmp("ssh ", ttname, 4, 0)) {
        x = ckstrncpy(line, ttname, LINBUFSIZ);
        debug(F110, "setlin dossh ttname *s == 0", s, 0);
      } else {
        debug(F111, "setlin dossh ELSE have_host", s, have_host);
        if (have_host == 0) {
          debug(F101, "setlin dossh have_host IS ZERO", "", have_host);
          printf("?SSH to where?\n");
          return (-9);
        }
        if (k < LINBUFSIZ) {
          line[k++] = SP;
          line[k] = NUL;
          debug(F111, "setlin sshcmd 2", line, k);
        }
        if (k < LINBUFSIZ) {
          ckstrncpy(&line[k], s, LINBUFSIZ - k);
          debug(F111, "setlin sshcmd 3", line, k);
        } else {
          printf("?Too long\n");
          return (-9);
        }
      }
      debug(F110, "setlin sshcmd calling cx_net", line, 0);
      x = cx_net(NET_PTY,    /* network type */
                 0,          /* protocol (not used) */
                 line,       /* host */
                 NULL,       /* service (not used) */
                 NULL,       /* username (not used) */
                 NULL,       /* password (not used) */
                 NULL,       /* command (not used) */
                 -1, -1, -1, /* params 1-3 (not used) */
                 1,          /* connect immediately */
                 sx,         /* server? */
                 zz,         /* close current? */
                 0);         /* not gui */
      debug(F111, "setlin cx_net", line, x);
      debug(F101, "setlin cx_net ttyfd", "", ttyfd);
      return (x);
    }
#endif /* SSHCMD */

/*
  Here we parse optional switches and then the hostname or whatever,
  which depends on the network type.  The tricky part is, the network type
  can be set by a switch.
*/
#ifndef NOSPL
    makestr(&g_pswd, pwbuf); /* Save global pwbuf */
    g_pflg = pwflg;          /* and flag */
    g_pcpt = pwcrypt;
#endif /* NOSPL */

    confirmed = 0;
    haveswitch = 0;

#ifdef NETCONN
#ifdef NETFILE
    if (mynet != NET_FILE) {
#endif /* NETFILE */
      ss = (mynet == NET_CMD || mynet == NET_PTY) ? "Command, or switch"
           : (mynet == NET_TCPA || mynet == NET_TCPB || mynet == NET_SSH)
               ? "Hostname, ip-address, or switch"
           : (mynet == NET_DLL) ? "Parameters, or switch"
                                : "Host or switch";
      if (fc) {
        if (mynet == NET_TCPB &&
            (ttnproto == NP_TELNET || ttnproto == NP_KERMIT)) {
          if (nshteltab) {
            haveswitch++;
            cmfdbi(&sw, _CMKEY, ss, "", "", nshteltab, 4, xxstring, shteltab,
                   &nx);
          }
        }
#ifdef RLOGCODE
        else if (mynet == NET_TCPB && ttnproto == NP_RLOGIN) {
          if (nshrlgtab) {
            haveswitch++;
            cmfdbi(&sw, _CMKEY, ss, "", "", nshrlgtab, 4, xxstring, shrlgtab,
                   &nx);
          }
        }
#endif /* RLOGCODE */
      } else {
        haveswitch++;
        cmfdbi(&sw, _CMKEY, ss, "", "", nshtab, 4, xxstring, shtab, &nx);
      }
#ifdef NETFILE
    }
#endif /* NETFILE */
    if (mynet == NET_TCPB || mynet == NET_SLAT || mynet == NET_SSH ||
        mynet == NET_DEC) {
      cmfdbi(&nx, _CMFLD, "Host", "", "", 0, 0, xxstring, NULL, NULL);
#ifdef NETFILE
    } else if (mynet == NET_FILE) {
      cmfdbi(&nx, _CMIFI, "Filename", "", "", 0, 0, xxstring, NULL, NULL);
#endif /* NETFILE */
#ifdef PTYORPIPE
    } else if (mynet == NET_CMD || mynet == NET_PTY) {
      cmfdbi(&nx, _CMTXT, "Command", "", "", 0, 0, xxstring, NULL, NULL);
#endif /* PTYORPIPE */
#ifdef NETDLL
    } else if (mynet == NET_DLL) {
      cmfdbi(&nx, _CMTXT, "Parameters", "", "", 0, 0, xxstring, NULL, NULL);
#endif /* NETFILE */
    } else {
      cmfdbi(&nx, _CMTXT, "Host", "", "", 0, 0, xxstring, NULL, NULL);
    }
    while (1) {
      x = cmfdb(haveswitch ? &sw : &nx);
      debug(F101, "setlin cmfdb", "", x);
      if (x < 0) {
        if (x != -3) {
          return (x);
        }
      }
      if (x == -3) {
        if ((x = cmcfm()) < 0) {
          return (x);
        } else {
          confirmed = 1;
          break;
        }
      }
      if (cmresult.fcode != _CMKEY) {                 /* Not a switch */
        ckstrncpy(line, cmresult.sresult, LINBUFSIZ); /* Save the data */
        s = line;                                     /* that was parsed... */
        if (cmresult.fcode == _CMIFI) {
          wild = cmresult.nresult;
        } else if (cmresult.fcode == _CMTXT) {
          confirmed = 1;
        }
        break; /* and break out of this loop */
      }
      c = cmgbrk();                    /* Have switch - get break character */
      getval = (c == ':' || c == '='); /* Must parse an agument? */
      if (getval && !(cmresult.kflags & CM_ARG)) {
        printf("?This switch does not take arguments\n");
        return (-9);
      }
      if (!getval && (cmgkwflgs() & CM_ARG)) {
        printf("?This switch requires an argument\n");
        return (-9);
      }
      switch (cmresult.nresult) { /* It's a switch.. */
      case SL_CNX:                /* /CONNECT */
        cx = 1;
        sx = 0;
        break;
      case SL_SRV: /* /SERVER */
        cx = 0;
        sx = 1;
        break;
#ifdef NETCMD
      case SL_CMD: /* /COMMAND */
        netsave = mynet;
        mynet = NET_CMD;
        break;
#endif /* NETCMD */
#ifdef NETPTY
      case SL_PTY: /* /PTY */
        netsave = mynet;
        mynet = NET_PTY;
        break;
#endif             /* NETPTY */
      case SL_NET: /* /NETWORK-TYPE */
        if ((x = cmkey(netcmd, nnets, "", "", xxstring)) < 0) {
          return (x);
        }
        mynet = x;
        break;

      case SL_UID: /* /USERID: */
        if (!getval) {
          break;
        }
        if ((x = cmfld("Userid", "", &s, xxstring)) < 0) {
          if (x == -3) {
            makestr(&tmpusrid, "");
          } else {
            return (x);
          }
        } else {
          s = brstrip(s);
          if ((x = (int)strlen(s)) > 63) {
            makestr(&slmsg, "Internal error");
            printf("?Sorry, too long - max = %d\n", 63);
            return (-9);
          }
          makestr(&tmpusrid, s);
          haveuser = 1;
        }
        break;

      case SL_WAIT:
        wait = 1;
        break;
      case SL_NOWAIT:
        wait = 0;
        break;
      }
    }

#ifdef NETFILE
    if (mynet == NET_FILE) {   /* Parsed by cmifi() */
      if ((x = cmcfm()) < 0) { /* Needs confirmation */
        return (x);
      }
      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif /* NETFILE */

#ifdef NETCMD
    if (mynet == NET_CMD || mynet == NET_PTY) {
      char *p = NULL;
      if (!confirmed) {
        if ((x = cmtxt("Rest of command", "", &s, xxstring)) < 0) {
          return (x);
        }
        if (*s) {
          ckstrncat(line, " ", LINBUFSIZ);
          ckstrncat(line, s, LINBUFSIZ);
        }
        s = line;
      }
      /* s == line - so we must protect the line buffer */
      s = brstrip(s);
      makestr(&p, s);
      ckstrncpy(line, p, LINBUFSIZ);
      makestr(&p, NULL);

      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif /* NETCMD */

#ifdef NETDLL
    if (mynet == NET_DLL) {
      char *p = NULL;
      if (!confirmed) {
        if ((x = cmtxt("Rest of command", "", &s, xxstring)) < 0) {
          return (x);
        }

        if (*s) {
          ckstrncat(line, " ", LINBUFSIZ);
          ckstrncat(line, s, LINBUFSIZ);
        }
        s = line;
      }
      /* s == line - so we must protect the line buffer */
      s = brstrip(s);
      makestr(&p, s);
      ckstrncpy(line, p, LINBUFSIZ);
      makestr(&p, NULL);

      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif /* NETDLL */
#ifdef CK_NETBIOS
    if (mynet == NET_BIOS) {
      /*
       * TODO:
       *   "server name, *,\n or carriage return to close an open connection" :
       *   "server name, *,\n or carriage return to resume an open connection",
       */
      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif                       /* CK_NETBIOS */
#ifdef NPIPE                 /* Named pipe */
    if (mynet == NET_PIPE) { /* Needs backslash twiddling */
      if (line[0]) {
        if (strcmp(line, "*")) { /* If remote, begin with */
          char *p = NULL;
          makestr(&p, line);
          ckstrncpy(line, "\\\\", LINBUFSIZ); /* server name */
          ckstrncat(line, p, LINBUFSIZ);
          makestr(&p, NULL);
        } else {
          line[0] = '\0';
        }
        ckstrncat(line, "\\pipe\\", LINBUFSIZ); /* Make pipe name */
        ckstrncat(line, pipename, LINBUFSIZ);   /* Add name of pipe */

        x = cx_net(mynet, /* nettype */
                   0,     /* protocol (not used) */
                   line,  /* host */
                   "",    /* port */
                   NULL,  /* alternate username */
                   NULL,  /* password */
                   NULL,  /* command to execute */
                   0,     /* param1 */
                   0,     /* param2 */
                   0,     /* param3 */
                   cx,    /* enter CONNECT mode */
                   sx,    /* enter SERVER mode */
                   zz,    /* close connection if open */
                   0      /* gui */
        );
      }
    }
#endif /* NPIPE */

#ifdef SUPERLAT
    if (mynet == NET_SLAT) { /* Needs password, etc. */
      slat_pwd[0] = NUL;     /* Erase any previous password */
      debok = 0;
      if (*line) { /* If they gave a host name... */
        if ((x = cmfld("password,\n or carriage return if no password required",
                       "", &s, xxstring)) < 0 &&
            x != -3) {
          return (x);
        }
        ckstrncpy(slat_pwd, s, 18); /* Set the password, if any */
      }
      if ((x = cmcfm()) < 0) {
        return (x); /* Confirm the command */
      }

      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif /* SUPERLAT */

#ifdef DECNET
    if (mynet == NET_DEC) {
      if (!line[0]) { /* If they gave a host name... */
        printf("?hostname required\n");
        return (-3);
      }
      if ((x = cmcfm()) < 0) {
        return (x); /* Confirm the command */
      }

      x = cx_net(mynet, /* nettype */
                 0,     /* protocol (not used) */
                 line,  /* host */
                 "",    /* port */
                 NULL,  /* alternate username */
                 NULL,  /* password */
                 NULL,  /* command to execute */
                 0,     /* param1 */
                 0,     /* param2 */
                 0,     /* param3 */
                 cx,    /* enter CONNECT mode */
                 sx,    /* enter SERVER mode */
                 zz,    /* close connection if open */
                 0      /* gui */
      );
    }
#endif /* DECNET */

#ifdef SSHBUILTIN
    if (mynet == NET_SSH) { /* SSH connection */
      int k, havehost = 0, trips = 0;
      int tmpver = -1, tmpxfw = -1, tmpssh_cas;
#ifndef SSHTEST
      extern int sl_ssh_xfw, sl_ssh_xfw_saved;
      extern int sl_ssh_ver, sl_ssh_ver_saved;
#endif /* SSHTEST */
      extern struct keytab sshopnsw[];
      extern int nsshopnsw;
      extern char *ssh_tmpcmd, *ssh_tmpport;
      struct FDB sw, kw, fl;

      debug(F110, "setlin SSH service 0", srvbuf, 0);
      debug(F110, "setlin SSH host s 2", s, 0);
      if (*s) { /* If they gave a host name... */
        debug(F110, "setlin SSH host s 1", s, 0);
        if (*s == '*') {
          makestr(&slmsg, "Incoming connections not supported");
          printf("?Sorry, incoming connections not supported for SSH.\n");
          return (-9);
        }
        ckstrncpy(line, s, LINBUFSIZ);
      } else {
        printf("?hostname required\n");
        return (-3);
      }

      /* Parse [ port ] [ switches ] */
      cmfdbi(&kw, /* Switches */
             _CMKEY, "Port number or service name,\nor switch", "", "",
             nsshopnsw, 4, xxstring, sshopnsw, &fl);
      cmfdbi(&fl, /* Port number or service name */
             _CMFLD, "", "", "", 0, 0, xxstring, NULL, NULL);
      trips = 0;        /* Explained below */
      while (1) {       /* Parse port and switches */
        y = cmfdb(&kw); /* Get a field */
        if (y == -3) {  /* User typed CR so quit from loop */
          break;
        }
        if (y < 0) { /* Other parse error, pass it back */
          return (y);
        }
        switch (cmresult.fcode) { /* Field or Keyword? */
        case _CMFLD:              /* Field */
          ckstrncpy(srvbuf, cmresult.sresult, SRVBUFSIZ);
          break;
        case _CMKEY:                  /* Keyword */
          switch (cmresult.nresult) { /* Which one? */
          case SSHSW_PWD:
            if (!cmgbrk()) {
              printf("?This switch requires an argument\n");
              return (-9);
            }
            debok = 0;
            if ((y = cmfld("Password", "", &s, xxstring)) < 0) {
              if (y == -3) {
                makestr(&tmpstring, "");
              } else {
                return (y);
              }
            } else {
              s = brstrip(s);
              if ((y = (int)strlen(s)) > PWBUFL) {
                makestr(&slmsg, "Internal error");
                printf("?Sorry, too long - max = %d\n", PWBUFL);
                return (-9);
              }
              makestr(&tmpstring, s);
            }
            break;
          case SSHSW_USR: /* /USER: */
            if (!cmgbrk()) {
              printf("?This switch requires an argument\n");
              return (-9);
            }
            if ((y = cmfld("Username", "", &s, xxstring)) < 0) {
              return (y);
            }
            s = brstrip(s);
            makestr(&tmpusrid, s);
            break;
          case SSHSW_VER:
            if ((y = cmnum("Number", "", 10, &z, xxstring)) < 0) {
              return (y);
            }
            if (z < 1 || z > 2) {
              printf("?Out of range: %d\n", z);
              return (-9);
            }
            tmpver = z;
            break;
          case SSHSW_CMD:
          case SSHSW_SUB:
            if ((y = cmfld("Text", "", &s, xxstring)) < 0) {
              return (y);
            }
            makestr(&ssh_tmpcmd, s);
            tmpssh_cas = (cmresult.nresult == SSHSW_SUB);
            break;
          case SSHSW_X11:
            if ((y = cmkey(onoff, 2, "", "on", xxstring)) < 0) {
              return (y);
            }
            tmpxfw = y;
            break;
          default:
            return (-2);
          }
        }
        if (trips++ == 0) { /* After first time through */
          cmfdbi(&kw,       /* only parse switches, not port. */
                 _CMKEY, "Switch", "", "", nsshopnsw, 4, xxstring, sshopnsw,
                 NULL);
        }
      }
      if ((y = cmcfm()) < 0) { /* Get confirmation */
        return (y);
      }

      debug(F110, "setlin pre-cx_net line", line, 0);
      debug(F110, "setlin pre-cx_net srvbuf", srvbuf, 0);
      x = cx_net(mynet,      /* nettype */
                 0,          /* protocol (not used) */
                 line,       /* host */
                 srvbuf,     /* port */
                 tmpusrid,   /* alternate username */
                 tmpstring,  /* password */
                 ssh_tmpcmd, /* command to execute */
                 tmpver,     /* param1 - ssh version */
                 tmpssh_cas, /* param2 - ssh cas  */
                 tmpxfw,     /* param3 - ssh x11fwd */
                 cx,         /* enter CONNECT mode */
                 sx,         /* enter SERVER mode */
                 zz,         /* close connection if open */
                 0           /* gui */
      );
      if (tmpusrid) {
        makestr(&tmpusrid, NULL);
      }
      if (ssh_tmpcmd) {
        makestr(&ssh_tmpcmd, NULL);
      }
    }
#endif /* SSHBUILTIN */

#ifdef TCPSOCKET
    if (mynet == NET_TCPB) { /* TCP/IP connection */
      debug(F110, "setlin service 0", srvbuf, 0);
      debug(F110, "setlin host s 2", s, 0);
      if (*s) { /* If they gave a host name... */
        debug(F110, "setlin host s 1", s, 0);
#ifdef NOLISTEN
        if (*s == '*') {
          makestr(&slmsg, "Incoming connections not supported");
          printf("?Sorry, incoming connections not supported in this version "
                 "of Kermit.\n");
          return (-9);
        }
#endif /* NOLISTEN */
#ifdef RLOGCODE
        /* Allow a username if rlogin is requested */
        if (mynet == NET_TCPB &&
            (ttnproto == NP_RLOGIN || ttnproto == NP_K5LOGIN ||
             ttnproto == NP_EK5LOGIN || ttnproto == NP_K4LOGIN ||
             ttnproto == NP_EK4LOGIN)) {
          int y;
          uidflag = 0;
          /* Check for "host:service" */
          for (; (*s != '\0') && (*s != ':'); s++)
            ;
          if (*s) { /* Service, save it */
            *s = NUL;
            ckstrncpy(srvbuf, ++s, SRVBUFSIZ);
          } else { /* No :service, then use default. */
            switch (ttnproto) {
            case NP_RLOGIN:
              ckstrncpy(srvbuf, "login", SRVBUFSIZ);
              break;
            case NP_K4LOGIN:
            case NP_K5LOGIN:
              ckstrncpy(srvbuf, "klogin", SRVBUFSIZ);
              break;
            case NP_EK4LOGIN:
            case NP_EK5LOGIN:
              ckstrncpy(srvbuf, "eklogin", SRVBUFSIZ);
              break;
            }
          }
          if (!confirmed) {
            y = cmfld("Userid on remote system", uidbuf, &s, xxstring);
            if (y < 0 && y != -3) {
              return (y);
            }
            if ((int)strlen(s) > 63) {
              makestr(&slmsg, "Internal error");
              printf("Sorry, too long\n");
              return (-9);
            }
            makestr(&tmpusrid, s);
          }
        } else { /* TELNET or SET HOST */
#endif           /* RLOGCODE */
          /* Check for "host:service" */
          for (; (*s != '\0') && (*s != ':'); s++)
            ;
          if (*s) { /* Service, save it */
            *s = NUL;
            ckstrncpy(srvbuf, ++s, SRVBUFSIZ);
          } else if (!confirmed) {
            /* No :service, let them type one. */
            if (*line != '*') { /* Not incoming */
              if (mynet == NET_TCPB && ttnproto == NP_KERMIT) {
                if ((x = cmfld("TCP service name or number", "kermit", &s,
                               xxstring)) < 0 &&
                    x != -3) {
                  return (x);
                }
#ifdef RLOGCODE
              } else if (mynet == NET_TCPB && ttnproto == NP_RLOGIN) {
                if ((x = cmfld("TCP service name or number,\n or carriage "
                               "return for rlogin (513)",
                               "login", &s, xxstring)) < 0 &&
                    x != -3) {
                  return (x);
                }
#endif /* RLOGCODE */
              } else {
                /* Do not set a default value in this call */
                /* If you do then it will prevent entries  */
                /* in the network directory from accessing */
                /* alternate ports.                        */

                if ((x = cmfld("TCP service name or number", "", &s,
                               xxstring)) < 0 &&
                    x != -3) {
                  return (x);
                }
              }
            } else { /* Incoming connection */
              if ((x = cmfld("TCP service name or number", "", &s, xxstring)) <
                      0 &&
                  x != -3) {
                return (x);
              }
            }
            if (*s) {                          /* If they gave a service, */
              ckstrncpy(srvbuf, s, SRVBUFSIZ); /* copy it */
            }
            debug(F110, "setlin service 0.5", srvbuf, 0);
          }
#ifdef RLOGCODE
        }
#endif /* RLOGCODE */
        if (!confirmed) {
          char *defproto;
          switch (ttnproto) {
          case NP_RLOGIN:
            defproto = "/rlogin";
            break;
          case NP_K4LOGIN:
            defproto = "/k4login";
            break;
          case NP_K5LOGIN:
            defproto = "/k5login";
            break;
          case NP_EK4LOGIN:
            defproto = "/ek4login";
            break;
          case NP_EK5LOGIN:
            defproto = "/ek5login";
            break;
          case NP_KERMIT:
          case NP_TELNET:
            defproto = "/telnet";
            break;
          default:
            defproto = "/default";
          }
          if ((x = cmkey(tcprawtab, ntcpraw, "Switch", defproto, xxstring)) <
              0) {
            if (x != -3) {
              return (x);
            } else if ((x = cmcfm()) < 0) {
              return (x);
            }
          } else {
            rawflg = x;
            if ((x = cmcfm()) < 0) {
              return (x);
            }
          }
        }
      }
      debug(F110, "setlin pre-cx_net line", line, 0);
      debug(F110, "setlin pre-cx_net srvbuf", srvbuf, 0);
      x = cx_net(mynet,                       /* nettype */
                 rawflg /* protocol */, line, /* host */
                 srvbuf,                      /* port */
                 tmpusrid,                    /* alternate username */
                 tmpstring,                   /* password */
                 NULL,                        /* command to execute */
                 a_type,                      /* param1 - telnet authtype */
                 e_type,                      /* param2 - telnet enctype  */
                 wait,                        /* param3 - telnet wait */
                 cx,                          /* enter CONNECT mode */
                 sx,                          /* enter SERVER mode */
                 zz,                          /* close connection if open */
                 0                            /* gui */
      );
    }
#endif /* TCPSOCKET */

    if (tmpusrid) {
      makestr(&tmpusrid, NULL);
    }
    debug(F111, "setlin cx_net", line, x);
    return (x);
#endif /* NETCONN */
  }

  /* Serial tty device, possibly modem, connection... */

  cmfdbi(&sw, _CMKEY, "Device name, or switch", "", "", npsltab, 4, xxstring,
         psltab, &tx);
  cmfdbi(&tx, _CMTXT, "", dftty, "", 0, 0, xxstring, NULL, NULL);
  while (!confirmed) {
    x = cmfdb(&sw);
    debug(F101, "setlin cmfdb", "", x);
    if (x < 0) {
      if (x != -3) {
        return (x);
      }
    }
    if (x == -3) {
      if ((x = cmcfm()) < 0) {
        return (x);
      } else {
        confirmed = 1;
        break;
      }
    }
    switch (cmresult.fcode) {
    case _CMTXT:
      ckstrncpy(tmpbuf, cmresult.sresult, TMPBUFSIZ);
      s = tmpbuf;
      debug(F110, "setlin CMTXT", tmpbuf, 0);
      confirmed = 1;
      break;
    case _CMKEY: /* Switch */
      debug(F101, "setlin CMKEY", tmpbuf, cmresult.nresult);
      switch (cmresult.nresult) {
      case SL_CNX: /* /CONNECT */
        cx = 1;
        sx = 0;
        break;
      case SL_SRV: /* /SERVER */
        cx = 0;
        sx = 1;
        break;
      }
      continue;
    default:
      debug(F101, "setlin bad cmfdb result", "", cmresult.fcode);
      makestr(&slmsg, "Internal error");
      printf("?Internal parsing error\n");
      return (-9);
    }
  }
  if (!confirmed) {
    if ((x = cmcfm()) < 0) {
      return (x);
    }
  }

  debug(F110, "setlin pre-cx_serial s", s, 0);
  debug(F110, "setlin pre-cx_serial line", line, 0);
  x = cx_serial(s, cx, sx, shr, zz, 0, 0);
  debug(F111, "setlin cx_serial", line, x);
  return (x);
}
#endif /* NOLOCAL */

#ifdef CKCHANNELIO
/*
  C-Library based file-i/o package for scripts.  This should be portable to
  all C-Kermit versions since it uses the same APIs we have always used for
  processing command files.  The entire channel i/o package is contained
  herein, apart from some keyword table entries in the main keyword table
  and the help text in the HELP command module.

  On platforms like VMS and VOS, this package handles only UNIX-style
  stream files.  If desired, it can be replaced for those platforms by
  <#>ifdef'ing out this code and adding the equivalent replacement routines
  to the ck?fio.c module, e.g. for RMS-based file i/o in ckvfio.c.
*/
#ifndef NOSTAT
#include <sys/stat.h>
#endif /* NOSTAT */

#ifdef NLCHAR
static int z_lt = 1; /* Length of line terminator */
#else
static int z_lt = 2;
#endif /* NLCHAR */

struct ckz_file {             /* C-Kermit file struct */
  FILE *z_fp;                 /* Includes the C-Lib file struct */
  unsigned int z_flags;       /* Plus C-Kermit mode flags, */
  CK_OFF_T z_nline;           /* current line number if known, */
  char z_name[CKMAXPATH + 2]; /* and the file's name. */
};
static struct ckz_file **z_file = NULL; /* Array of C-Kermit file structs */
static int z_inited = 0;                /* Flag for array initialized */
int z_maxchan = Z_MAXCHAN;              /* Max number of C-Kermit channels */
int z_openmax = CKMAXOPEN;              /* Max number of open files overall */
int z_nopen = 0;                        /* How many channels presently open */
int z_error = 0;                        /* Most recent error */
int z_filcount = -1;                    /* Most recent FILE COUNT result */

#define RD_LINE 0 /* FILE READ options */
#define RD_CHAR 1
#define RD_SIZE 2
#define RD_TRIM 8 /* Like Snobol &TRIM = 1 */
#define RD_UNTA 9 /* Untabify */

#define WR_LINE RD_LINE /* FILE WRITE options */
#define WR_CHAR RD_CHAR
#define WR_SIZE RD_SIZE
#define WR_STRI 3
#define WR_LPAD 4
#define WR_RPAD 5

#ifdef UNIX
extern int ckmaxfiles; /* Filled in by sysinit(). */
#endif                 /* UNIX */

/* See ckcker.h for error numbers */
/* See ckcdeb.h for Z_MAXCHAN and CKMAXOPEN definitions */
/* NOTE: For VMS we might be able to fill in ckmaxfiles */
/* from FILLM and CHANNELCNT -- find out about these... */

static char *fopnargs[] = {
    /* Mode combinations for fopen() */
    /* Combinations and syntax permitted by C libraries... */
    "", "r", "w", "r+", "a", "", "a", "", /* Text mode */
    "", "r", "w", "r+", "a", "", "a", ""  /* Binary modes for UNIX */
};
static int nfopnargs = sizeof(fopnargs) / sizeof(char *);

char * /* Error messages */

ckferror(int n) {
  switch (n) {
  case FX_NER:
    return ("No error");
  case FX_SYS:
    return (ck_errstr());
  case FX_EOF:
    return ("End of file");
  case FX_NOP:
    return ("File not open");
  case FX_CHN:
    return ("Channel out of range");
  case FX_RNG:
    return ("Parameter out of range");
  case FX_NMF:
    return ("Too many files open");
  case FX_FOP:
    return ("Operation conflicts with OPEN mode");
  case FX_NYI:
    return ("OPEN mode not supported");
  case FX_BOM:
    return ("Illegal combination of OPEN modes");
  case FX_ACC:
    return ("Access denied");
  case FX_FNF:
    return ("File not found");
  case FX_OFL:
    return ("Buffer overflow");
  case FX_LNU:
    return ("Current line number unknown");
  case FX_ROO:
    return ("Off limits");
  case FX_UNK:
    return ("Operation fails - reason unknown");
  default:
    return ("Error number out of range");
  }
}

/*
  Z _ O P E N --  Open a file for the requested type of access.

  Call with:
    name:  Name of file to be opened.
    flags: Any combination of FM_xxx values except FM_EOF (ckcker.h).
  Returns:
    >= 0 on success: The assigned channel number
    <  0 on failure: A negative FX_xxx error code (ckcker.h).
*/
int z_open(char *name, int flags) {
  int i, n;
  FILE *t;
  char *mode;
  debug(F111, "z_open", name, flags);
  if (!name) {
    name = ""; /* Check name argument */
  }
  if (!name[0]) {
    return (z_error = FX_BFN);
  }
  if (flags & FM_CMD) {        /* Opening pipes not implemented yet */
    return (z_error = FX_NYI); /* (and not portable either) */
  }
  debug(F101, "z_open nfopnargs", "", nfopnargs);

  if (flags & FM_STDIN) { /* Read from standard input */
    mode = "r";
  } else if (flags & (FM_STDOUT | FM_STDERR)) {
    mode = "w";
  } else {                                 /* If regular file, not stdin.. */
    if (flags < 0 || flags >= nfopnargs) { /* Range check flags */
      return (z_error = FX_RNG);
    }
    mode = fopnargs[flags]; /* Get fopen() arg */
    debug(F111, "z_open fopen args", mode, flags);
    if (!mode[0]) { /* Check for illegal combinations */
      return (z_error = FX_BOM);
    }
  }

  if (!z_inited) { /* If file structs not inited */
    debug(F101, "z_open z_maxchan 1", "", z_maxchan);
#ifdef UNIX
    debug(F101, "z_open ckmaxfiles", "", ckmaxfiles);
    if (ckmaxfiles > 0) { /* Set in ck?tio.c: sysinit() */
      int x;
      x = ckmaxfiles - ZNFILS - 5;
      if (x > z_maxchan) { /* sysconf() value greater than */
        z_maxchan = x;     /* value from header files. */
      }
      debug(F101, "z_open z_maxchan 2", "", z_maxchan);
    }
#endif                           /* UNIX */
    if (z_maxchan < Z_MINCHAN) { /* Allocate at least this many. */
      z_maxchan = Z_MINCHAN;
    }
    debug(F101, "z_open z_maxchan 3", "", z_maxchan);
    /* Note: This could be a pretty big chunk of memory */
    /* if z_maxchan is a big number.  If this becomes a problem */
    /* we'll need to malloc and free each element at open/close time */
    /* New economical way, allocate storage for each channel as needed */
    if (!z_file) {
      debug(F100, "z_file[] is NULL", "", 0);
      debug(F101, "sizeof(struct ckz_file *)", "", sizeof(struct ckz_file *));
      z_file = (struct ckz_file **)malloc((z_maxchan + 1) *
                                          sizeof(struct ckz_file *));
      debug(F101, "z_open z_maxchan 4", "", z_maxchan);
      if (!z_file) {
        return (z_error = FX_NMF);
      }
      for (i = 0; i < z_maxchan; i++) {
        z_file[i] = NULL;
      }
      debug(F101, "z_open z_maxchan 5", "", z_maxchan);
    }
    debug(F101, "z_open z_maxchan 6", "", z_maxchan);
    z_inited = 1; /* Remember we initialized */
  }
  for (n = -1, i = 0; i < z_maxchan; i++) { /* Find a free channel */
    debug(F101, "z_open find-free-channel loop", "", i);
    if (!z_file[i]) {
      z_file[i] = (struct ckz_file *)malloc(sizeof(struct ckz_file));
      if (!z_file[i]) {
        return (z_error = FX_NMF);
      }
      n = i;
      break;
    }
  }
  debug(F101, "z_open found free channel", "", n);
  if (n < 0 || n >= z_maxchan) { /* Any free channels? */
    return (z_error = FX_NMF);   /* No, fail. */
  }
  debug(F100, "z_open check n ok", "", 0);
  errno = 0;
  debug(F100, "z_open errno ok", "", 0);
  z_file[n]->z_flags = 0; /* In case of failure... */
  debug(F100, "z_open z_file[n] flags ok", "", 0);
  z_file[n]->z_fp = NULL; /* Set file pointer to NULL */
  debug(F100, "z_open z_file[n] fps ok", "", 0);

#ifdef UNIX
  if (flags & FM_STDIN) {       /* Standard input */
    t = (FILE *)stdin;          /* We just use the ready-made stream */
    z_nopen++;                  /* Count it. */
    z_file[n]->z_fp = t;        /* Stash the file pointer */
    z_file[n]->z_flags = flags; /* and the flags */
    z_file[n]->z_nline = 0;     /* Current line number is 0 */
    ckstrncpy(z_file[n]->z_name, name, CKMAXPATH); /* "filename" */
    z_error = 0;                                   /* No error so far */
    return (n); /* Return the channel number */
  }
  if (flags & FM_STDOUT) { /* Standard output */
    t = (FILE *)stdout;    /* Same deal */
    z_nopen++;
    z_file[n]->z_fp = t;
    z_file[n]->z_flags = flags;
    z_file[n]->z_nline = 0;
    ckstrncpy(z_file[n]->z_name, name, CKMAXPATH);
    z_error = 0;
    return (n);
  }
  if (flags & FM_STDERR) { /* Standard error */
    t = (FILE *)stderr;
    z_nopen++;
    z_file[n]->z_fp = t;
    z_file[n]->z_flags = flags;
    z_file[n]->z_nline = 0;
    ckstrncpy(z_file[n]->z_name, name, CKMAXPATH);
    z_error = 0;
    return (n);
  }
#endif                   /* UNIX */
  t = fopen(name, mode); /* Try to open the file. */
  if (!t) {              /* Failed... */
    debug(F111, "z_open error", name, errno);
#ifdef EMFILE
    if (errno == EMFILE) {
      return (z_error = FX_NMF);
    }
#endif /* EMFILE */
    free(z_file[n]);
    z_file[n] = NULL;
    return (z_error = (errno ? FX_SYS : FX_UNK)); /* Return error code */
  }

  z_nopen++;                  /* Open, count it. */
  z_file[n]->z_fp = t;        /* Stash the file pointer */
  z_file[n]->z_flags = flags; /* and the flags */
  z_file[n]->z_nline = 0;     /* Current line number is 0 */
  z_error = 0;
  zfnqfp(name, CKMAXPATH, z_file[n]->z_name); /* and the file's full name */
  return (n);                                 /* Return the channel number */
}

int z_close(int channel) /* Close file on given channel */
{
  int x;
  FILE *t;
  if (!z_inited) { /* Called before any files are open? */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) { /* Channel out of range? */
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) { /* Channel wasn't open? */
    return (z_error = FX_NOP);
  }
  errno = 0; /* Set errno 0 to get a good reading */
  if (!(z_file[channel]->z_flags & FM_STDM)) { /* If not stdin/out/err... */
    x = fclose(t);                             /* Try to close */
  }
  if (x == EOF) {              /* On failure */
    return (z_error = FX_SYS); /* indicate system error. */
  }
  z_nopen--;                         /* Closed OK, decrement open count */
  z_file[channel]->z_fp = NULL;      /* Set file pointer to NULL */
  z_file[channel]->z_nline = 0;      /* Current line number is 0 */
  z_file[channel]->z_flags = 0;      /* Set flags to 0 */
  *(z_file[channel]->z_name) = '\0'; /* Clear name */
  free(z_file[channel]);
  z_file[channel] = NULL;
  return (z_error = 0);
}

/*
  Z _ O U T  --  Output string to channel.

  Call with:
    channel:     Channel number to write to.
    s:           String to write.
    length > -1: How many characters of s to write.
    length < 0:  Write entire NUL-terminated string.
    flags == 0:  Supply line termination.
    flags >  0:  Don't supply line termination.
    flags <  0:  Write 'length' NUL characters.
  Special case:
    If flags > -1 and s is empty or NULL and length == 1, write 1 NUL.
  Returns:
    Number of characters written to channel on success, or
    negative FX_xxx error code on failure.
*/
int z_out(int channel, char *s, int length, int flags) {
  FILE *t;
  int x, n;
  char c = '\0';

  if (!s) {
    s = ""; /* Guard against null pointer */
  }
#ifdef DEBUG
  if (deblog) {
    debug(F111, "z_out", s, channel);
    debug(F101, "z_out length", "", length);
    debug(F101, "z_out flags", "", flags);
  }
#endif             /* DEBUG */
  if (!z_inited) { /* File i/o inited? */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) { /* Channel in range? */
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) { /* File open? */
    return (z_error = FX_NOP);
  }
  if (!((z_file[channel]->z_flags) & (FM_WRI | FM_APP))) { /* In write mode? */
    return (z_error = FX_FOP);
  }
  n = length;                    /* Length of string to write */
  if (n < 0) {                   /* Negative means get it ourselves */
    if (flags < 0) {             /* Except when told to write NULs in */
      return (z_error = FX_RNG); /* which case args are inconsistent */
    }
    n = strlen(s); /* Get length of string arg */
  }
  errno = 0; /* Reset errno */
  debug(F101, "z_out n", "", n);
  if (flags < 0) { /* Writing NULs... */
    int i;
    for (i = 0; i < n; i++) {
      x = fwrite(&c, 1, 1, t);
      if (x < 1) {
        return (z_error = (errno ? FX_SYS : FX_UNK));
      }
    }
    z_file[channel]->z_nline = -1; /* Current line no longer known */
    z_error = 0;
    return (i);
  } else {                 /* Writing string arg */
    if (n == 1 && !s[0]) { /* Writing one char but it's NUL */
      x = fwrite(&c, 1, 1, t);
    } else { /* Writing non-NUL char or string */
      x = fwrite(s, 1, n, t);
    }
    debug(F101, "z_out fwrite", ckitoa(x), errno);
    if (x < n) { /* Failure to write requested amount */
      return (z_error = (errno ? FX_SYS : FX_UNK)); /* Return error */
    }
    if (flags == 0) {              /* If supplying line termination */
      if (fwrite("\n", 1, 1, t)) { /* do that  */
        x += z_lt;                 /* count the terminator */
      }
      if (z_file[channel]->z_nline > -1) { /* count this line */
        z_file[channel]->z_nline++;
      }
    } else {
      z_file[channel]->z_nline = -1; /* Current line no longer known */
    }
  }
  z_error = 0;
  return (x);
}

#define Z_INBUFLEN 64

/*
  Z _ I N  --  Multichannel i/o file input function.

  Call with:
    channel number to read from.
    s = address of destination buffer.
    buflen = destination buffer length.
    length = Number of bytes to read, must be < buflen.
    flags: 0 = read a line; nonzero = read the given number of bytes.
  Returns:
    Number of bytes read into buffer or a negative error code.
    A terminating NUL is deposited after the last byte that was read.
*/
int z_in(int channel, char *s, int buflen, int length, int flags) {
  int i, x;
  FILE *t;

  if (!z_inited) { /* Check everything... */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (channel < 0) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  if (!((z_file[channel]->z_flags) & FM_REA)) {
    return (z_error = FX_FOP);
  }
  if (!s) { /* Check destination */
    return (z_error = FX_RNG);
  }
  s[0] = NUL;
  if (length == 0) { /* Read 0 bytes - easy. */
    return (z_error = 0);
  }
  debug(F101, "z_in channel", "", channel);
  debug(F101, "z_in buflen", "", buflen);
  debug(F101, "z_in length", "", length);
  debug(F101, "z_in flags", "", flags);
  if (length < 0 || buflen < 0) { /* Check length args */
    return (z_error = FX_RNG);
  }
  if (buflen <= length) {
    return (z_error = FX_RNG);
  }
  errno = 0;                 /* Reset errno */
  if (flags) {               /* Read block or byte */
    int n;                   /* 20050912 */
    n = length;              /* 20050912 */
    i = 0;                   /* 20050912 */
    while (n > 0) {          /* 20050912 */
      i = fread(s, 1, n, t); /* 20050912 */
#ifdef DEBUG
      if (deblog) {
        debug(F111, "z_in block", s, i);
        debug(F101, "z_in block errno", "", errno);
        debug(F101, "z_in block ferror", "", ferror(t));
        debug(F101, "z_in block feof", "", feof(t));
      }
#endif /* DEBUG */
      if (i == 0) {
        break; /* 20050912 */
      }
      s += i; /* 20050912 */
      n -= i; /* 20050912 */
    }
    /* Current line no longer known */
    z_file[channel]->z_nline = (CK_OFF_T)-1;
  } else { /* Read line */
    /* This method is used because it's simpler than the others */
    /* and also marginally faster. */
    debug(F101, "z_in getc loop", "", CKFTELL(t));
    for (i = 0; i < length; i++) {
      if ((x = getc(t)) == EOF) {
        debug(F101, "z_in getc error", "", CKFTELL(t));
        s[i] = '\0';
        break;
      }
      s[i] = x;
      if (s[i] == '\n') {
        s[i] = '\0';
        break;
      }
    }
    debug(F111, "z_in line byte loop", ckitoa(errno), i);
    debug(F111, "z_in line got", s, z_file[channel]->z_nline);
    if (z_file[channel]->z_nline > -1) {
      z_file[channel]->z_nline++;
    }
  }
  debug(F111, "z_in i", ckitoa(errno), i);
  if (i < 0) {
    i = 0; /* NUL-terminate result */
  }
  s[i] = '\0';
  if (i > 0) {
    z_error = 0;
    return (i);
  }
  if (i == 0 && feof(t)) {     /* EOF on reading? */
    return (z_error = FX_EOF); /* Return EOF code */
  }
  return (errno ? (z_error = -1) : i); /* Return length or system error */
}

int z_flush(int channel) /* Flush output channel */
{
  FILE *t;
  int x;
  if (!z_inited) { /* Regular checks */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  if (!((z_file[channel]->z_flags) & (FM_WRI | FM_APP))) { /* Write access? */
    return (z_error = FX_FOP);
  }
  errno = 0;                           /* Reset errno */
  x = fflush(t);                       /* Try to flush */
  return (x ? (z_error = FX_SYS) : 0); /* Return system error or 0 if OK */
}

int z_seek(int channel, CK_OFF_T pos) /* Move file pointer to byte */
{
  int x = 0, rc;
  FILE *t;
  if (!z_inited) { /* Check... */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  if (pos < 0L) {
    x = 2;
    pos = (pos == -2) ? -1L : 0L;
  }
  errno = 0;
  rc = CKFSEEK(t, pos, x); /* Try to seek */
  debug(F111, "z_seek", ckitoa(errno), rc);
  if (rc < 0) {                /* OK? */
    return (z_error = FX_SYS); /* No. */
  }
  z_file[channel]->z_nline = ((pos || x) ? -1 : 0);
  return (z_error = 0);
}

int z_line(int channel, CK_OFF_T pos) /* Move file pointer to line */
{
  int len, x = 0;
  CK_OFF_T current = (CK_OFF_T)0, prev = (CK_OFF_T)-1, old = (CK_OFF_T)-1;
  FILE *t;
  char tmpbuf[256];
  if (!z_inited) { /* Check... */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  debug(F101, "z_line pos", "", pos);
  if (pos < 0L) { /* EOF wanted */
    CK_OFF_T n;
    n = z_file[channel]->z_nline;
    debug(F101, "z_line n", "", n);
    if (n < 0 || pos < 0) {
      rewind(t);
      n = 0;
    }
    while (1) { /* This could take a while... */
      if ((x = getc(t)) == EOF) {
        break;
      }
      if (x == '\n') {
        n++;
        if (pos == -2) {
          old = prev;
          prev = CKFTELL(t);
        }
      }
    }
    debug(F101, "z_line old", "", old);
    debug(F101, "z_line prev", "", prev);
    if (pos == -2) {
      if ((x = z_seek(channel, old)) < 0) {
        return (z_error = x);
      } else {
        n--;
      }
    }
    z_file[channel]->z_nline = n;
    return (z_error = 0);
  }
  if (pos == 0L) { /* Rewind wanted */
    z_file[channel]->z_nline = 0L;
    rewind(t);
    debug(F100, "z_line rewind", "", 0);
    return (0L);
  }
  tmpbuf[255] = NUL;                  /* Make sure buf is NUL terminated */
  current = z_file[channel]->z_nline; /* Current line */
  /*
    If necessary the following could be optimized, e.g. for positioning
    to a previous line in a large file without starting over.
  */
  if (current < 0 || pos < current) { /* Not known or behind us... */
    debug(F101, "z_line rewinding", "", pos);
    if ((x = z_seek(channel, 0L)) < 0) { /* Rewind */
      return (z_error = x);
    }
    if (pos == 0) { /* If 0th line wanted we're done */
      return (z_error = 0);
    }
    current = 0;
  }
  while (current < pos) { /* Search for specified line */
    if (fgets(tmpbuf, 255, t)) {
      len = strlen(tmpbuf);
      if (len > 0 && tmpbuf[len - 1] == '\n') {
        current++;
        debug(F111, "z_line read", ckitoa(len), current);
      } else if (len == 0) {
        return (z_error = FX_UNK);
      }
    } else {
      z_file[channel]->z_nline = -1L;
      debug(F101, "z_line premature EOF", "", current);
      return (z_error = FX_EOF);
    }
  }
  z_file[channel]->z_nline = current;
  debug(F101, "z_line result", "", current);
  z_error = 0;
  return (current);
}

char *z_getname(int channel) /* Return name of file on channel */
{
  FILE *t;
  if (!z_inited) {
    z_error = FX_NOP;
    return (NULL);
  }
  if (channel >= z_maxchan) {
    z_error = FX_CHN;
    return (NULL);
  }
  if (!z_file[channel]) {
    z_error = FX_NOP;
    return (NULL);
  }
  if (!(t = z_file[channel]->z_fp)) {
    z_error = FX_NOP;
    return (NULL);
  }
  return ((char *)(z_file[channel]->z_name));
}

int z_getmode(int channel) /* Return OPEN modes of channel */
{
  FILE *t; /* 0 if file not open */
#ifndef NOSTAT
  struct stat statbuf;
#endif /* NOSTAT */
  int x;
  if (!z_inited) {
    return (0);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (0);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (0);
  }
  x = z_file[channel]->z_flags;
  if (feof(t)) { /* This might not work for */
    x |= FM_EOF; /* output files */
#ifndef NOSTAT
    /* But this does if we can use it. */
  } else if (stat(z_file[channel]->z_name, &statbuf) > -1) {
    if (CKFTELL(t) == statbuf.st_size) {
      x |= FM_EOF;
    }
#endif /* NOSTAT */
  }
  return (x);
}

CK_OFF_T
z_getpos(int channel) {
  /* Get file pointer position on this channel */
  FILE *t;
  CK_OFF_T x;
  if (!z_inited) {
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  x = CKFTELL(t);
  return ((x < 0L) ? (z_error = FX_SYS) : x);
}

CK_OFF_T
z_getline(int channel) {
  /* Get current line number  in file on this channel */
  FILE *t;
  CK_OFF_T rc;
  if (!z_inited) {
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  debug(F101, "z_getline", "", z_file[channel]->z_nline);
  rc = z_file[channel]->z_nline;
  return ((rc < 0) ? (z_error = FX_LNU) : rc);
}

int z_getfnum(int channel) {
  /* Get file number / handle for file on this channel */
  FILE *t;
  if (!z_inited) {
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  z_error = 0;
  return (fileno(t));
}

/*
  Line-oriented counts and seeks are as dumb as they can be at the moment.
  Later we can speed them up by building little indexes.
*/
CK_OFF_T
z_count(int channel, int what) {
  /* Count bytes or lines in file */
  FILE *t;
  int x;
  CK_OFF_T pos, count = (CK_OFF_T)0;
  if (!z_inited) { /* Check stuff... */
    return (z_error = FX_NOP);
  }
  if (channel >= z_maxchan) {
    return (z_error = FX_CHN);
  }
  if (!z_file[channel]) {
    return (z_error = FX_NOP);
  }
  if (!(t = z_file[channel]->z_fp)) {
    return (z_error = FX_NOP);
  }
  pos = CKFTELL(t); /* Save current file pointer */
  errno = 0;
  z_error = 0;
  if (what == RD_CHAR) { /* Size in bytes requested */
    return (zgetfs(z_file[channel]->z_name));
  }
  rewind(t);                    /* Line count requested - rewind. */
  while (1) {                   /* Count lines. */
    if ((x = getc(t)) == EOF) { /* Stupid byte loop */
      break;                    /* but it works as well as anything */
    }
    if (x == '\n') { /* else... */
      count++;
    }
  }
  x = CKFSEEK(t, pos, 0); /* Restore file pointer */
  return (count);
}

/* User interface for generalized channel-oriented file i/o */

struct keytab fctab[] = {/* FILE subcommands */
                         {"close", FIL_CLS, 0},  {"count", FIL_COU, 0},
                         {"flush", FIL_FLU, 0},  {"list", FIL_LIS, 0},
                         {"open", FIL_OPN, 0},   {"read", FIL_REA, 0},
                         {"rewind", FIL_REW, 0}, {"seek", FIL_SEE, 0},
                         {"status", FIL_STA, 0}, {"write", FIL_WRI, 0}};
int nfctab = (sizeof(fctab) / sizeof(struct keytab));

static struct keytab fcswtab[] = {/* OPEN modes */
                                  {"/append", FM_APP, 0},
                                  {"/binary", FM_BIN, 0},
                                  {"/read", FM_REA, 0},
#ifdef UNIX /* Could be expanded to VMS etc.. */
                                  {"/stderr", FM_STDERR, 0},
                                  {"/stdin", FM_STDIN, 0},
                                  {"/stdout", FM_STDOUT, 0},
#endif /* UNIX */
                                  {"/write", FM_WRI, 0}};
static int nfcswtab = (sizeof(fcswtab) / sizeof(struct keytab));

static struct keytab fclkwtab[] = {/* CLOSE options */
                                   {"all", 1, 0}};

static struct keytab fsekwtab[] = {/* SEEK symbols */
                                   {"eof", 1, 0},
                                   {"last", 2, 0}};
static int nfsekwtab = (sizeof(fsekwtab) / sizeof(struct keytab));

#define SEE_LINE RD_LINE /* SEEK options */
#define SEE_CHAR RD_CHAR
#define SEE_REL 3
#define SEE_ABS 4
#define SEE_FIND 5

static struct keytab fskswtab[] = {
    {"/absolute", SEE_ABS, 0},        {"/byte", SEE_CHAR, 0},
    {"/character", SEE_CHAR, CM_INV}, {"/find", SEE_FIND, CM_ARG},
    {"/line", SEE_LINE, 0},           {"/relative", SEE_REL, 0}};
static int nfskswtab = (sizeof(fskswtab) / sizeof(struct keytab));

#define COU_LINE RD_LINE /* COUNT options */
#define COU_CHAR RD_CHAR
#define COU_LIS 3
#define COU_NOL 4

static struct keytab fcoswtab[] = {
    {"/bytes", COU_CHAR, 0}, {"/characters", COU_CHAR, CM_INV},
    {"/lines", COU_LINE, 0}, {"/list", COU_LIS, 0},
    {"/nolist", COU_NOL, 0}, {"/quiet", COU_NOL, CM_INV}};
static int nfcoswtab = (sizeof(fcoswtab) / sizeof(struct keytab));

static struct keytab frdtab[] = {/* READ types */
                                 {"/block", RD_SIZE, CM_INV | CM_ARG},
                                 {"/byte", RD_CHAR, CM_INV},
                                 {"/character", RD_CHAR, 0},
                                 {"/line", RD_LINE, 0},
                                 {"/size", RD_SIZE, CM_ARG},
                                 {"/trim", RD_TRIM, 0},
                                 {"/untabify", RD_UNTA, 0}};
static int nfrdtab = (sizeof(frdtab) / sizeof(struct keytab));

static struct keytab fwrtab[] = {/* WRITE types */
                                 {"/block", WR_SIZE, CM_INV | CM_ARG},
                                 {"/byte", WR_CHAR, CM_INV},
                                 {"/character", WR_CHAR, 0},
                                 {"/line", WR_LINE, 0},
                                 {"/lpad", WR_LPAD, CM_ARG},
                                 {"/rpad", WR_RPAD, CM_ARG},
                                 {"/size", WR_SIZE, CM_ARG},
                                 {"/string", WR_STRI, 0}};
static int nfwrtab = (sizeof(fwrtab) / sizeof(struct keytab));

static char blanks[] = "\040\040\040\040"; /* Some blanks for formatting */
static char *seek_target = NULL;

int dofile(int op) /* Do the FILE command */
{
  char vnambuf[VNAML]; /* Buffer for variable names */
  char *vnp = NULL;    /* Pointer to same */
  char zfilnam[CKMAXPATH + 2];
  char *p, *m;
  struct FDB fl, sw, nu;
  CK_OFF_T z;
  int rsize, filmode = 0, relative = -1, eofflg = 0;
  int rc, x, y, cx, n, getval, dummy, confirmed, listing = -1;
  int charflag = 0, sizeflag = 0;
  int pad = 32, wr_lpad = 0, wr_rpad = 0, rd_trim = 0, rd_untab = 0;

  makestr(&seek_target, NULL);

  if (op == XXFILE) { /* FILE command was given */
    /* Get subcommand */
    if ((cx = cmkey(fctab, nfctab, "Operation", "", xxstring)) < 0) {
      if (cx == -3) {
        printf("?File operation required\n");
        x = -9;
      }
      return (cx);
    }
  } else { /* Shorthand command was given */
    switch (op) {
    case XXF_CL:
      cx = FIL_CLS;
      break; /* FCLOSE */
    case XXF_FL:
      cx = FIL_FLU;
      break; /* FFLUSH */
    case XXF_LI:
      cx = FIL_LIS;
      break; /* FLIST */
    case XXF_OP:
      cx = FIL_OPN;
      break; /* etc... */
    case XXF_RE:
      cx = FIL_REA;
      break;
    case XXF_RW:
      cx = FIL_REW;
      break;
    case XXF_SE:
      cx = FIL_SEE;
      break;
    case XXF_ST:
      cx = FIL_STA;
      break;
    case XXF_WR:
      cx = FIL_WRI;
      break;
    case XXF_CO:
      cx = FIL_COU;
      break;
    default:
      return (-2);
    }
  }
  switch (cx) {                  /* Do requested subcommand */
  case FIL_OPN:                  /* OPEN */
    cmfdbi(&sw,                  /* Switches */
           _CMKEY,               /* fcode */
           "Variable or switch", /* hlpmsg */
           "",                   /* default */
           "",                   /* addtl string data */
           nfcswtab,             /* addtl numeric data 1: tbl size */
           4,                    /* addtl numeric data 2: 4 = cmswi */
           xxstring,             /* Processing function */
           fcswtab,              /* Keyword table */
           &fl                   /* Pointer to next FDB */
    );
    cmfdbi(&fl,        /* Anything that doesn't match */
           _CMFLD,     /* fcode */
           "Variable", /* hlpmsg */
           "", "", 0, 0, NULL, NULL, NULL);
    while (1) {
      x = cmfdb(&sw); /* Parse something */
      if (x < 0) {
        if (x == -3) {
          printf("?Variable name and file name required\n");
          x = -9;
        }
        return (x);
      }
      if (cmresult.fcode == _CMFLD) {
        break;
      } else if (cmresult.fcode == _CMKEY) {
        char c;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
          printf("?This switch does not take an argument\n");
          return (-9);
        }
        debug(F101, "filmode A", "", filmode);
        filmode |= cmresult.nresult; /* OR in the file mode */
        debug(F101, "filmode B", "", filmode);
        debug(F101, "filmode & (FM_REA|FM_STDIN)", "",
              filmode & (FM_REA | FM_STDIN));
        debug(F101, "filmode & (FM_WRI|FM_STDOUT|FM_STDERR)", "",
              filmode & (FM_WRI | FM_STDOUT | FM_STDERR));
        if ((filmode & (FM_REA | FM_STDIN)) &&
            (filmode & (FM_WRI | FM_STDOUT | FM_STDERR))) {
          printf("?Conflicting file modes\n");
          return (-9);
        }
      } else {
        return (-2);
      }
    }
    /* Not a switch - get the string */
    ckstrncpy(vnambuf, cmresult.sresult, VNAML);
    if (!vnambuf[0] || chknum(vnambuf)) { /* (if there is one...) */
      printf("?Variable name required\n");
      return (-9);
    }
    vnp = vnambuf; /* Check variable-name syntax */
    if (vnambuf[0] == CMDQ && (vnambuf[1] == '%' || vnambuf[1] == '&')) {
      vnp++;
    }
    y = 0;
    if (*vnp == '%' || *vnp == '&') {
      if ((y = parsevar(vnp, &x, &dummy)) < 0) {
        printf("?Syntax error in variable name\n");
        return (-9);
      }
    }
    /* Assign a negative channel number in case we fail */
    addmac(vnambuf, "-1");

    if (!(filmode & FM_RWA)) { /* If no access mode specified */
      filmode |= FM_REA;       /* default to /READ. */
    }

#ifdef UNIX
    if (filmode & FM_STDIN) { /* If STDIN specified */
      filmode |= FM_REA;      /* it implies /READ */
      /* We don't need to parse anything further */
      s = "(stdin)";
      goto xdofile; /* Skip around the following */
    }
    if (filmode & FM_STDOUT) { /* If STDOUT specified */
      filmode |= FM_WRI;       /* it implies /WRITE */
      /* We don't need to parse anything further */
      s = "(stdout)";
      goto xdofile; /* Skip around the following */
    }
    if (filmode & FM_STDIN) { /* If STDIN specified */
      filmode |= FM_WRI;      /* it implies /WRITE */
      /* We don't need to parse anything further */
      s = "(stderr)";
      goto xdofile; /* Skip around the following */
    }
#endif     /* UNIX */
    y = 0; /* Now parse the filename */
    if ((filmode & FM_RWA) == FM_WRI) {
      x = cmofi("Name of new file", "", &s, xxstring);
    } else if ((filmode & FM_RWA) == FM_REA) {
      x = cmifi("Name of existing file", "", &s, &y, xxstring);
    } else {
      x = cmiofi("Filename", "", &s, &y, xxstring);
      debug(F111, "fopen /append x", s, x);
    }
    if (x < 0) {
      if (x == -3) {
        printf("?Filename required\n");
        x = -9;
      }
      return (x);
    }
    if (y) { /* No wildcards */
      printf("?Wildcards not allowed here\n");
      return (-9);
    }
    if (filmode & (FM_APP | FM_WRI)) { /* Check output access */
      if (zchko(s) < 0) {              /* and set error code if denied */
        z_error = FX_ACC;
        printf("?Write access denied - \"%s\"\n", s);
        return (-9);
      }
    }

#ifdef UNIX
  xdofile:
#endif                                /* UNIX */
    ckstrncpy(zfilnam, s, CKMAXPATH); /* Is OK - make safe copy */
    if ((x = cmcfm()) < 0) {          /* Get confirmation of command */
      return (x);
    }
    if ((n = z_open(zfilnam, filmode)) < 0) {
      printf("?OPEN failed - %s: %s\n", zfilnam, ckferror(n));
      return (-9);
    }
    addmac(vnambuf, ckitoa(n)); /* Assign channel number to variable */
    return (success = 1);

  case FIL_REW: /* REWIND */
    if ((x = cmnum("Channel number", "", 10, &n, xxstring)) < 0) {
      if (x == -3) {
        printf("?Channel number required\n");
        x = -9;
      }
      return (x);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (n == -9) {
      return (success = 0);
    }
    if (n == -8) {
      return (success = 1);
    }

    if ((rc = z_seek(n, 0L)) < 0) {
      printf("?REWIND failed - Channel %d: %s\n", n, ckferror(rc));
      return (-9);
    }
    return (success = 1);

  case FIL_CLS:                     /* CLOSE */
    cmfdbi(&nu,                     /* Second FDB - channel number */
           _CMNUM,                  /* fcode */
           "Channel number or ALL", /* Help message */
           s,                       /* default */
           "",                      /* addtl string data */
           10,                      /* addtl numeric data 1: radix */
           0,                       /* addtl numeric data 2: 0 */
           xxstring,                /* Processing function */
           NULL,                    /* Keyword table */
           &sw                      /* Pointer to next FDB */
    );                              /* Pointer to next FDB */
    cmfdbi(&sw,                     /* First FDB - command switches */
           _CMKEY,                  /* fcode */
           "",                      /* help message */
           "",                      /* Default */
           "",                      /* No addtl string data */
           1,                       /* addtl numeric data 1: tbl size */
           0,                       /* addtl numeric data 2: 4 = cmswi */
           xxstring,                /* Processing function */
           fclkwtab,                /* Keyword table */
           NULL                     /* Last in chain */
    );
    x = cmfdb(&nu); /* Parse something */
    if (x < 0) {
      if (x == -3) {
        printf("?Channel number or ALL required\n");
        x = -9;
      }
      return (x);
    }
    if (cmresult.fcode == _CMNUM) {
      n = cmresult.nresult;
    } else if (cmresult.fcode == _CMKEY) {
      n = -1;
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (n == -9) {
      return (success = 0);
    }
    if (n == -8) {
      return (success = 1);
    }

    rc = 1;
    if (n < 0) {
      int count = 0;
      int i;
      for (i = 0; i < z_maxchan; i++) {
        x = z_close(i);
        if (x == FX_SYS) {
          printf("?CLOSE failed - Channel %d: %s\n", n, ckferror(x));
          rc = 0;
        } else if (x > -1) {
          count++;
        }
      }
      debug(F101, "FILE CLOSE ALL", "", count);
    } else if ((x = z_close(n)) < 0) {
      printf("?CLOSE failed - Channel %d: %s\n", n, ckferror(x));
      return (-9);
    }
    return (success = rc);

  case FIL_REA: /* READ */
  case FIL_WRI: /* WRITE */
    rsize = 0;
    cmfdbi(&sw,                 /* Switches */
           _CMKEY,              /* fcode */
           "Channel or switch", /* hlpmsg */
           "",                  /* default */
           "",                  /* addtl string data */
           (cx == FIL_REA) ? nfrdtab : nfwrtab,
           4,        /* addtl numeric data 2: 4 = cmswi */
           xxstring, /* Processing function */
           (cx == FIL_REA) ? frdtab : fwrtab, /* Keyword table */
           &nu                                /* Pointer to next FDB */
    );
    cmfdbi(&nu,           /* Channel number */
           _CMNUM,        /* fcode */
           "Channel", "", /* default */
           "",            /* addtl string data */
           10,            /* addtl numeric data 1: radix */
           0,             /* addtl numeric data 2: 0 */
           xxstring,      /* Processing function */
           NULL,          /* Keyword table */
           NULL           /* Pointer to next FDB */
    );
    do {
      x = cmfdb(&sw); /* Parse something */
      if (x < 0) {
        if (x == -3) {
          printf("?Channel number required\n");
          x = -9;
        }
        return (x);
      }
      if (cmresult.fcode == _CMNUM) { /* Channel number */
        break;
      } else if (cmresult.fcode == _CMKEY) { /* Switch */
        char c;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
          printf("?This switch does not take an argument\n");
          return (-9);
        }
        if (!getval && (cmgkwflgs() & CM_ARG)) {
          printf("?This switch requires an argument\n");
          return (-9);
        }
        switch (cmresult.nresult) {
        case WR_LINE:
          charflag = 0;
          sizeflag = 0;
          rsize = 0;
          break;
        case WR_CHAR:
          rsize = 1;
          charflag = 1;
          sizeflag = 1;
          break;
        case WR_SIZE:
          if ((x = cmnum("Bytes", "", 10, &rsize, xxstring)) < 0) {
            if (x == -3) {
              printf("?Number required\n");
              x = -9;
            }
            return (x);
          }
          if (rsize > LINBUFSIZ) {
            printf("?Maximum FREAD/FWRITE size is %d\n", LINBUFSIZ);
            rsize = 0;
            return (-9);
          }
          charflag = 0;
          sizeflag = 1;
          break;
        case WR_STRI:
          rsize = 1;
          charflag = 0;
          sizeflag = 0;
          break;
        case WR_LPAD:
        case WR_RPAD:
          if ((x = cmnum("Numeric ASCII character value", "32", 10, &pad,
                         xxstring)) < 0) {
            return (x);
          }
          if (cmresult.nresult == WR_LPAD) {
            wr_lpad = 1;
          } else {
            wr_rpad = 1;
          }
          break;
        case RD_TRIM:
          rd_trim = 1;
          break;
        case RD_UNTA:
          rd_untab = 1;
          break;
        }
        debug(F101, "FILE READ rsize 2", "", rsize);
      } else {
        return (-2);
      }
    } while (cmresult.fcode == _CMKEY);

    n = cmresult.nresult; /* Channel */
    debug(F101, "FILE READ/WRITE channel", "", n);

    if (cx == FIL_WRI) { /* WRITE */
      int len = 0;
      if ((x = cmtxt("Text", "", &s, xxstring)) < 0) {
        return (x);
      }
      if (n == -9) {
        return (success = 0);
      }
      if (n == -8) {
        return (success = 1);
      }

      ckstrncpy(line, s, LINBUFSIZ); /* Make a safe copy */
      s = line;
      s = brstrip(s);         /* Strip braces */
      if (charflag) {         /* Write one char */
        len = 1;              /* So length = 1 */
        rsize = 1;            /* Don't supply terminator */
      } else if (!sizeflag) { /* Write a string */
        len = -1;             /* So length is unspecified */
      } else {                /* Write a block of given size */
        int i, xx;
        if (rsize > TMPBUFSIZ) {
          z_error = FX_OFL;
          printf("?Buffer overflow\n");
          return (-9);
        }
        len = rsize;     /* rsize is really length */
        rsize = 1;       /* Don't supply a terminator */
        xx = strlen(s);  /* Size of given string */
        if (xx >= len) { /* Bigger or equal */
          s[len] = NUL;
        } else if (wr_lpad) {              /* Smaller, left-padding requested */
          for (i = 0; i < len - xx; i++) { /* Must make a copy */
            tmpbuf[i] = pad;
          }
          ckstrncpy(tmpbuf + i, s, TMPBUFSIZ - i);
          tmpbuf[len] = NUL;
          s = tmpbuf;         /* Redirect write source */
        } else if (wr_rpad) { /* Smaller with right-padding */
          for (i = xx; i < len; i++) {
            s[i] = pad;
          }
          s[len] = NUL;
        }
      }
      if ((rc = z_out(n, s, len, rsize)) < 0) { /* Try to write */
        printf("?Channel %d WRITE error: %s\n", n, ckferror(rc));
        return (-9);
      }
    } else { /* FIL_REA READ */
      confirmed = 0;
      vnambuf[0] = NUL;
      x = cmfld("Variable name", "", &s, NULL);
      debug(F111, "FILE READ cmfld", s, x);
      if (x < 0) {
        if (x == -3 || !*s) {
          if ((x = cmcfm()) < 0) {
            return (x);
          } else {
            confirmed++;
          }
        } else {
          return (x);
        }
      }
      ckstrncpy(vnambuf, s, VNAML);
      debug(F111, "FILE READ vnambuf", vnambuf, confirmed);
      if (vnambuf[0]) { /* Variable name given, check it */
        if (!confirmed) {
          x = cmcfm();
          if (x < 0) {
            return (x);
          } else {
            confirmed++;
          }
        }
        vnp = vnambuf;
        if (vnambuf[0] == CMDQ && (vnambuf[1] == '%' || vnambuf[1] == '&')) {
          vnp++;
        }
        y = 0;
        if (*vnp == '%' || *vnp == '&') {
          if ((y = parsevar(vnp, &x, &dummy)) < 0) {
            printf("?Syntax error in variable name\n");
            return (-9);
          }
        }
      }
      debug(F111, "FILE READ variable", vnambuf, confirmed);

      if (!confirmed) {
        if ((x = cmcfm()) < 0) {
          return (x);
        }
      }

      if (n == -9) {
        return (success = 0);
      }
      if (n == -8) {
        return (success = 1);
      }

      line[0] = NUL; /* Clear destination buffer */

      if (rsize == 0) { /* Read a line */
        rc = z_in(n, line, LINBUFSIZ, LINBUFSIZ - 1, 0);
      } else {
        rc = z_in(n, line, LINBUFSIZ, rsize, 1); /* Read a block */
      }
      if (rc < 0) { /* Error... */
        debug(F101, "FILE READ error", "", rc);
        debug(F101, "FILE READ errno", "", errno);
        if (rc == FX_EOF) { /* EOF - fail but no error message */
          return (success = 0);
        } else { /* Other error - fail and print msg */
          printf("?READ error: %s\n", ckferror(rc));
          return (-9);
        }
      }
      if (rsize == 0) { /* FREAD /LINE postprocessing */
        if (rd_trim) {  /* Trim */
          int i, k;
          k = strlen(line);
          if (k > 0) {
            for (i = k - 1; i > 0; i--) {
              if (line[i] == SP || line[i] == '\t') {
                line[i] = NUL;
              } else {
                break;
              }
            }
          }
        }
        if (rd_untab) { /* Untabify */
          if (untabify(line, tmpbuf, TMPBUFSIZ) > -1) {
            ckstrncpy(line, tmpbuf, LINBUFSIZ);
          }
        }
      }
      debug(F110, "FILE READ data", line, 0);
      if (vnambuf[0]) {        /* Read OK - If variable name given */
        addmac(vnambuf, line); /* Assign result to variable */
      } else {                 /* otherwise */
        printf("%s\n", line);  /* just print it */
      }
    }
    return (success = 1);

  case FIL_SEE:                 /* SEEK */
  case FIL_COU:                 /* COUNT */
    rsize = RD_CHAR;            /* Defaults to /BYTE */
    cmfdbi(&sw,                 /* Switches */
           _CMKEY,              /* fcode */
           "Channel or switch", /* hlpmsg */
           "",                  /* default */
           "",                  /* addtl string data */
           ((cx == FIL_SEE) ? nfskswtab : nfcoswtab),
           4,        /* addtl numeric data 2: 4 = cmswi */
           xxstring, /* Processing function */
           ((cx == FIL_SEE) ? fskswtab : fcoswtab),
           &nu /* Pointer to next FDB */
    );
    cmfdbi(&nu,           /* Channel number */
           _CMNUM,        /* fcode */
           "Channel", "", /* default */
           "",            /* addtl string data */
           10,            /* addtl numeric data 1: radix */
           0,             /* addtl numeric data 2: 0 */
           xxstring,      /* Processing function */
           NULL,          /* Keyword table */
           NULL           /* Pointer to next FDB */
    );
    do {
      x = cmfdb(&sw); /* Parse something */
      if (x < 0) {
        if (x == -3) {
          printf("?Channel number required\n");
          x = -9;
        }
        return (x);
      }
      if (cmresult.fcode == _CMNUM) { /* Channel number */
        break;
      } else if (cmresult.fcode == _CMKEY) { /* Switch */
        char c;
        c = cmgbrk();
        if ((getval = (c == ':' || c == '=')) && !(cmgkwflgs() & CM_ARG)) {
          printf("?This switch does not take an argument\n");
          return (-9);
        }
        if (cx == FIL_SEE) {
          switch (cmresult.nresult) {
          case SEE_REL:
            relative = 1;
            break;
          case SEE_ABS:
            relative = 0;
            break;
          case SEE_FIND: {
            if (getval) {
              y = cmfld("string or pattern", "", &s, xxstring);
              if (y < 0) {
                return (y);
              }
              makestr(&seek_target, brstrip(s));
              break;
            }
          }
          default:
            rsize = cmresult.nresult;
          }
        } else if (cx == FIL_COU) {
          switch (cmresult.nresult) {
          case COU_LIS:
            listing = 1;
            break;
          case COU_NOL:
            listing = 0;
            break;
          default:
            rsize = cmresult.nresult;
          }
        }
      }
    } while (cmresult.fcode == _CMKEY);

    n = cmresult.nresult; /* Channel */
    debug(F101, "FILE SEEK/COUNT channel", "", n);
    if (cx == FIL_COU) {
      if ((x = cmcfm()) < 0) {
        return (x);
      }
      if (n == -9) {
        return (success = 0);
      }
      if (n == -8) {
        return (success = 1);
      }

      z_filcount = z_count(n, rsize);
      if (z_filcount < 0) {
        rc = z_filcount;
        printf("?COUNT error: %s\n", ckferror(rc));
        return (-9);
      }
      if (listing < 0) {
        listing = !xcmdsrc;
      }
      if (listing) {
        printf(" %d %s%s\n", z_filcount, ((rsize == RD_CHAR) ? "byte" : "line"),
               ((z_filcount == 1L) ? "" : "s"));
      }
      return (success = (z_filcount > -1) ? 1 : 0);
    }
    m = (rsize == RD_CHAR) ? "Number of bytes;\n or keyword"
                           : "Number of lines;\n or keyword";
    cmfdbi(&sw,       /* SEEK symbolic targets (EOF) */
           _CMKEY,    /* fcode */
           m, "", "", /* addtl string data */
           nfsekwtab, /* addtl numeric data 1: table size */
           0,         /* addtl numeric data 2: 4 = cmswi */
           xxstring,  /* Processing function */
           fsekwtab,  /* Keyword table */
           &nu        /* Pointer to next FDB */
    );
    cmfdbi(&nu,      /* Byte or line number */
           _CMNUW,   /* fcode */
           "", "",   /* default */
           "",       /* addtl string data */
           10,       /* addtl numeric data 1: radix */
           0,        /* addtl numeric data 2: 0 */
           xxstring, /* Processing function */
           NULL,     /* Keyword table */
           NULL      /* Pointer to next FDB */
    );
    x = cmfdb(&sw); /* Parse something */
    if (x < 0) {
      if (x == -3) {
        printf("?Channel number or EOF required\n");
        x = -9;
      }
      return (x);
    }
    if (cmresult.fcode == _CMNUW) {
      z = cmresult.wresult;
      debug(F110, "FILE SEEK atmbuf", atmbuf, 0);
      if (relative < 0) {
        if (cx == FIL_SEE && (atmbuf[0] == '+' || atmbuf[0] == '-')) {
          relative = 1;
        } else {
          relative = 0;
        }
      }
    } else if (cmresult.fcode == _CMKEY) {
      eofflg = cmresult.nresult;
      relative = 0;
      y = 0 - eofflg;
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (n == -9) {
      return (success = 0);
    }
    if (n == -8) {
      return (success = 1);
    }
    y = 1; /* Recycle this */
    z_flush(n);
    debug(F101, "FILE SEEK relative", "", relative);
    debug(F101, "FILE SEEK rsize", "", rsize);

    if (rsize == RD_CHAR) { /* Seek to byte position */
      if (relative > 0) {
        CK_OFF_T pos;
        pos = z_getpos(n);
        if (pos < (CK_OFF_T)0) {
          rc = pos;
          printf("?Relative SEEK failed: %s\n", ckferror(rc));
          return (-9);
        }
        z += pos;
      } else {
        if (z < 0 && !eofflg) { /* Negative arg but not relative */
          y = 0;                /* Remember this was bad */
          z = 0;                /* but substitute 0 */
        }
      }
      debug(F101, "FILE SEEK /CHAR z", "", z);
      if (z < 0 && !eofflg) {
        z_error = FX_RNG;
        return (success = 0);
      }
      if ((rc = z_seek(n, z)) < 0) {
        if (rc == FX_EOF) {
          return (success = 0);
        }
        printf("?SEEK /BYTE failed - Channel %d: %s\n", n, ckferror(rc));
        return (-9);
      }
    } else { /* Seek to line */
      if (relative > 0) {
        CK_OFF_T pos;
        pos = z_getline(n);
        debug(F101, "FILE SEEK /LINE pos", "", pos);
        if (pos < 0) {
          rc = pos;
          printf("?Relative SEEK failed: %s\n", ckferror(rc));
          return (-9);
        }
        z += pos;
      }
      debug(F101, "FILE SEEK /LINE z", "", z);
      debug(F101, "FILE SEEK /LINE eofflg", "", eofflg);
      if (z < 0 && !eofflg) {
        z_error = FX_RNG;
        return (success = 0);
      }
      if ((rc = z_line(n, z)) < 0) {
        if (rc == FX_EOF) {
          return (success = 0);
        }
        printf("?SEEK /LINE failed - Channel %d: %s\n", n, ckferror(rc));
        return (-9);
      }
    }
    /*
      Now, having sought to the desired starting spot, if a /FIND:
      target was specified, look for it now.
    */
    if (seek_target) {
      int flag = 0, matchresult = 0;
      while (!flag) {
        y = z_in(n, line, LINBUFSIZ, LINBUFSIZ - 1, 0);
        if (y < 0) {
          y = 0;
          break;
        }
        if (ispattern(seek_target)) {
          matchresult = ckmatch(seek_target, line, inpcas[cmdlvl], 1 + 4);
        } else {
          /* This is faster */
          matchresult = ckindex(seek_target, line, 0, 0, inpcas[cmdlvl]);
        }
        if (matchresult) {
          flag = 1;
          break;
        }
      }
      if (flag) {
        debug(F111, "FSEEK HAVE MATCH", seek_target, z_getline(n));
        /* Back up to beginning of line where target found */
        if ((y = z_line(n, z_getline(n) - 1)) < 0) {
          if (rc == FX_EOF) {
            return (success = 0);
          }
          printf("?SEEK /LINE failed - Channel %d: %s\n", n, ckferror(rc));
          return (-9);
        }
        debug(F101, "FSEEK LINE", "", y);
      }
    }
    return (success = (y < 0) ? 0 : 1);

  case FIL_LIS: { /* LIST open files */
#ifdef CK_TTGWSIZ
    extern int cmd_rows, cmd_cols;
#endif /* CK_TTGWSIZ */
    extern int xaskmore;
    int i, x, n = 0, paging = 0;
    char *s;

    if ((x = cmcfm()) < 0) {
      return (x);
    }

#ifdef CK_TTGWSIZ
    if (cmd_rows > 0 && cmd_cols > 0)
#endif /* CK_TTGWSIZ */
      paging = xaskmore;

    printf("System open file limit:%5d\n", z_openmax);
    printf("Maximum for FILE OPEN: %5d\n", z_maxchan);
    printf("Files currently open:  %5d\n\n", z_nopen);
    n = 4;
    for (i = 0; i < z_maxchan; i++) {
      s = z_getname(i); /* Got one? */
      if (s) {          /* Yes */
        char m[8];
        m[0] = NUL;
        printf("%2d. %s", i, s); /* Print name */
        n++;                     /* Count it */
        x = z_getmode(i);        /* Get modes & print them */
        if (x > 0) {
          if (x & FM_REA) {
            ckstrncat(m, "R", 8);
          }
          if (x & FM_WRI) {
            ckstrncat(m, "W", 8);
          }
          if (x & FM_APP) {
            ckstrncat(m, "A", 8);
          }
          if (x & FM_BIN) {
            ckstrncat(m, "B", 8);
          }
          if (m[0]) {
            printf(" (%s)", m);
          }
          if (x & FM_EOF) {
            printf(" [EOF]");
          } else { /* And file position too */
            printf(" %s", ckfstoa(z_getpos(i)));
          }
        }
        printf("\n");
#ifdef CK_TTGWSIZ
        if (paging > 0) { /* Pause at end of screen */
          if (n > cmd_rows - 3) {
            if (!askmore()) {
              break;
            } else {
              n = 0;
            }
          }
        }
#endif /* CK_TTGWSIZ */
      }
    }
    return (success = 1);
  }

  case FIL_FLU: /* FLUSH */
    if ((x = cmnum("Channel number", "", 10, &n, xxstring)) < 0) {
      if (x == -3) {
        printf("?Channel number required\n");
        x = -9;
      }
      return (x);
    }
    if ((x = cmcfm()) < 0) {
      return (x);
    }
    if (n == -9) {
      return (success = 0);
    }
    if (n == -8) {
      return (success = 1);
    }
    if ((rc = z_flush(n)) < 0) {
      printf("?FLUSH failed - Channel %d: %s\n", n, ckferror(rc));
      return (-9);
    }
    return (success = 1);

  case FIL_STA: /* STATUS */
  {
    int i, j, k; /* Supply default if only one open */
    s = "";
    for (k = 0, j = 0, i = 0; i < z_maxchan; i++) {
      if (z_file) {
        if (z_file[i]) {
          if (z_file[i]->z_fp) {
            k++;
            j = i;
          }
        }
      }
    }
    if (k == 1) {
      s = ckitoa(j);
    }
  }
    if ((x = cmnum("Channel number", s, 10, &n, xxstring)) < 0) {
      if (x == -3) {
        if (z_nopen > 1) {
          printf("?%d files open - please supply channel number\n", z_nopen);
          return (-9);
        }
      } else {
        return (x);
      }
    }
    if ((y = cmcfm()) < 0) {
      return (y);
    }
    if ((!z_file || z_nopen == 0) && x == -3) {
      printf("No files open\n");
      return (success = 1);
    }
    p = blanks + 3; /* Tricky formatting... */
    if (n < 1000) {
      p--;
    }
    if (n < 100) {
      p--;
    }
    if (n < 10) {
      p--;
    }
    if ((rc = z_getmode(n)) < 0) {
      printf("Channel %d:%s%s\n", n, p, ckferror(rc));
      return (success = 0);
    } else if (!rc) {
      printf("Channel %d:%sNot open\n", n, p);
      return (success = 0);
    } else {
      CK_OFF_T xx;
      s = z_getname(n);
      if (!s) {
        s = "(name unknown)";
      }
      printf("Channel %d:%sOpen\n", n, p);
      printf(" File:        %s\n Modes:      ", s);
      if (rc & FM_REA) {
        printf(" /READ");
      }
      if (rc & FM_WRI) {
        printf(" /WRITE");
      }
      if (rc & FM_APP) {
        printf(" /APPEND");
      }
      if (rc & FM_BIN) {
        printf(" /BINARY");
      }
      if (rc & FM_CMD) {
        printf(" /COMMAND");
      }
      if (rc & FM_EOF) {
        printf(" [EOF]");
      }
      printf("\n Size:        %s\n", ckfstoa(z_count(n, RD_CHAR)));
      printf(" At byte:     %s\n", ckfstoa(z_getpos(n)));
      xx = z_getline(n);
      if (xx > (CK_OFF_T)-1) {
        printf(" At line:     %s\n", ckfstoa(xx));
      }
      return (success = 1);
    }
  default:
    return (-2);
  }
}
#endif /* CKCHANNELIO */

#ifndef NOSETKEY
/* Save Key maps and in OS/2 Mouse maps */
int savkeys(char *name, int disp) {
  char *tp;
  static struct filinfo xx;
  int savfil, i, j, k;
  char buf[1024];

  zclose(ZMFILE);

  if (disp) {
    xx.bs = 0;
    xx.cs = 0;
    xx.rl = 0;
    xx.org = 0;
    xx.cc = 0;
    xx.typ = 0;
    xx.dsp = XYFZ_A;
    xx.os_specific = "";
    xx.lblopts = 0;
    savfil = zopeno(ZMFILE, name, NULL, &xx);
  } else {
    savfil = zopeno(ZMFILE, name, NULL, NULL);
  }

  if (savfil) {
    ztime(&tp);
    zsout(ZMFILE, "; C-Kermit SAVE KEYMAP file: ");
    zsoutl(ZMFILE, tp);

    zsoutl(ZMFILE, "; Clear previous keyboard mappings ");
    zsoutl(ZMFILE, "set key clear");
    zsoutl(ZMFILE, "");

    for (i = 0; i < KMSIZE; i++) {
      if (macrotab[i]) {
        int len = strlen((char *)macrotab[i]);
        ckmakmsg(buf, 1024, "set key \\", ckitoa(i), NULL, NULL);
        zsout(ZMFILE, buf);

        for (j = 0; j < len; j++) {
          char ch = macrotab[i][j];
          if (ch <= SP || ch >= DEL || ch == '-' || ch == ',' || ch == '{' ||
              ch == '}' || ch == ';' || ch == '?' || ch == '.' || ch == '\'' ||
              ch == '\\' || ch == '/' || ch == '#') {
            ckmakmsg(buf, 1024, "\\{", ckitoa((int)ch), "}", NULL);
            zsout(ZMFILE, buf);
          } else {
            ckmakmsg(buf, 1024, ckctoa((char)ch), NULL, NULL, NULL);
            zsout(ZMFILE, buf);
          }
        }
        zsoutl(ZMFILE, "");
      } else if (keymap[i] != i) {
#ifndef NOKVERBS
        if (IS_KVERB(keymap[i])) {
          for (j = 0; j < nkverbs; j++) {
            if (kverbs[j].kwval == (keymap[i] & ~F_KVERB)) {
              break;
            }
          }
          if (j != nkverbs) {
            ckmakmsg(buf, 1024, "set key \\", ckitoa(i), " \\K", kverbs[j].kwd);
            zsoutl(ZMFILE, buf);
          }
        } else
#endif /* NOKVERBS */
        {
          ckmakxmsg(buf, 1024, "set key \\", ckitoa(i), " \\{",
                    ckitoa(keymap[i]), "}", NULL, NULL, NULL, NULL, NULL, NULL,
                    NULL);
          zsoutl(ZMFILE, buf);
        }
      }
    }

    zsoutl(ZMFILE, "");
    zsoutl(ZMFILE, "; End");
    zclose(ZMFILE);
    return (success = 1);
  } else {
    return (success = 0);
  }
}
#endif /* NOSETKEY */

#define SV_SCRL 0
#define SV_HIST 1

static struct keytab cmdtrmopt[] = {
#ifdef CK_RECALL
    {"history", SV_HIST, 0},
#endif /* CK_RECALL */
    {"", 0, 0}};
static int ncmdtrmopt = (sizeof(cmdtrmopt) / sizeof(struct keytab)) - 1;

int dosave(int xx) {
  int x, y = 0, disp;
  char *s = NULL;
  extern struct keytab disptb[];
#ifdef ZFNQFP
  struct zfnfp *fnp;
#endif /* ZFNQFP */

#ifndef NOSETKEY
  if (xx == XSKEY) { /* SAVE KEYMAP.. */
    z = cmofi("Name of Kermit command file", "keymap.ksc", &s, xxstring);
  } else {
#endif /* NOSETKEY */
    switch (xx) {
    case XSCMD: /* SAVE COMMAND.. */
      if ((y = cmkey(cmdtrmopt, ncmdtrmopt, "What to save", "history",
                     xxstring)) < 0) {
        return (y);
      }
      break;
    }
    z = cmofi("Filename", ((y == SV_SCRL) ? "scrollbk.txt" : "history.txt"), &s,
              xxstring);
#ifndef NOSETKEY
  }
#endif         /* NOSETKEY */
  if (z < 0) { /* Check output-file parse results */
    return (z);
  }
  if (z == 2) {
    printf("?Sorry, %s is a directory name\n", s);
    return (-9);
  }
#ifdef ZFNQFP
  if ((fnp = zfnqfp(s, TMPBUFSIZ - 1, tmpbuf))) { /* Convert to full pathname */
    if (fnp->fpath) {
      if ((int)strlen(fnp->fpath) > 0) {
        s = fnp->fpath;
      }
    }
  }
#endif /* ZFNQFP */

  ckstrncpy(line, s, LINBUFSIZ); /* Make safe copy of pathname */
  s = line;
  /* Get NEW/APPEND disposition */
  if ((z = cmkey(disptb, 2, "Disposition", "new", xxstring)) < 0) {
    return (z);
  }
  disp = z;
  if ((x = cmcfm()) < 0) { /* Get confirmation */
    return (x);
  }

  switch (xx) { /* Do action.. */
#ifndef NOSETKEY
  case XSKEY: /* SAVE KEYMAP */
    return (savkeys(s, disp));
#endif /* NOSETKEY */

  case XSCMD: /* SAVE COMMAND.. */
#ifndef NORECALL
    if (y == SV_HIST) { /* .. HISTORY */
      return (success = savhistory(s, disp));
    }
#endif /* NORECALL */
    break;
  }
  success = 0;
  return (-2);
}

/*
  R E A D T E X T

  Read text with a custom prompt into given buffer using command parser but
  with no echoing or entry into recall buffer.
*/
int readtext(char *prmpt, char *buffer, int bufsiz) {
#ifdef CK_RECALL
  extern int on_recall; /* Around Password prompting */
#endif                  /* CK_RECALL */
  int rc;
#ifndef NOLOCAL
#endif /* NOLOCAL */

#ifdef CK_RECALL
  on_recall = 0;
#endif                    /* CK_RECALL */
  cmsavp(psave, PROMPTL); /* Save old prompt */
  cmsetp(prmpt);          /* Make new prompt */
  concb((char)escape);    /* Put console in cbreak mode */
  cmini(1);               /* and echo mode */
  if (pflag) {
    prompt(xxstring); /* Issue prompt if at top level */
  }
  cmres();                        /* Reset the parser */
  for (rc = -1; rc < 0;) {        /* Prompt till they answer */
    rc = cmtxt("", "", &s, NULL); /* Get a literal line of text */
    cmres();                      /* Reset the parser again */
  }
  ckstrncpy(buffer, s, bufsiz);
  cmsetp(psave); /* Restore original prompt */

#ifndef NOLOCAL
#endif /* NOLOCAL */
  return (0);
}
#endif /* NOICP */

/* A general function to allow a Password or other information  */
/* to be read from the command prompt without it going into     */
/* the recall buffer or being echo'd.                           */

int readpass(char *prmpt, char *buffer, int bufsiz) {
  int x;
#ifdef NOICP
  if (!prmpt) {
    prmpt = "";
  }
  printf("%s", prmpt);
  {
    int c, i;
    char *p;
    p = buffer;
    for (i = 0; i < bufsiz - 1; i++) {
      if ((c = getchar()) == EOF) {
        break;
      }
      if (c < SP) {
        break;
      }
      buffer[i] = c;
    }
    buffer[i] = NUL;
  }
  return (1);
#else /* NOICP */
#ifdef CK_RECALL
  extern int on_recall; /* around Password prompting */
#endif /* CK_RECALL */
  int rc;
#ifndef NOLOCAL
#endif /* NOLOCAL */
#ifdef CKSYSLOG
  int savlog;
#endif /* CKSYSLOG */
  if (!prmpt) {
    prmpt = "";
  }
#ifndef NOLOCAL
  debok = 0; /* Don't log */
#endif /* NOLOCAL */

#ifdef CKSYSLOG
  savlog = ckxsyslog; /* Save and turn off syslogging */
  ckxsyslog = 0;
#endif /* CKSYSLOG */
#ifndef NOLOCAL
#endif /* NOLOCAL */
#ifdef CK_RECALL
  on_recall = 0;
#endif /* CK_RECALL */
  cmsavp(psave, PROMPTL); /* Save old prompt */
  cmsetp(prmpt);          /* Make new prompt */
  concb((char)escape);    /* Put console in cbreak mode */
  cmini(0);               /* and no-echo mode */
  if (pflag) {
    prompt(xxstring); /* Issue prompt if at top level */
  }
  cmres();                        /* Reset the parser */
  for (rc = -1; rc < 0;) {        /* Prompt till they answer */
    rc = cmtxt("", "", &s, NULL); /* Get a literal line of text */
    cmres();                      /* Reset the parser again */
  }
  ckstrncpy(buffer, s, bufsiz);
  printf("\r\n"); /* Echo a CRLF */
  cmsetp(psave);  /* Restore original prompt */
  cmini(1);       /* Restore echo mode */
#ifndef NOLOCAL
#endif /* NOLOCAL */
#ifdef CKSYSLOG
  ckxsyslog = savlog; /* Restore syslogging */
#endif /* CKSYSLOG */
  debok = 1;
  return (0);
#endif /* NOICP */
}

#ifndef NOICP
struct keytab authtab[] = {/* Available authentication types */
                           {"", 0, 0}};
int authtabn = sizeof(authtab) / sizeof(struct keytab) - 1;

#ifndef NOSHOW
int sho_iks() {
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
#ifdef IKSDB
  extern char *dbfile;
  extern int dbenabled;
#endif /* IKSDB */
#ifdef CK_LOGIN
  extern int logintimo;
#endif /* CK_LOGIN */
  extern int srvcdmsg, success, iksdcf, noinit, arg_x;
  extern char *cdmsgfile[], *cdmsgstr, *kermrc;
  char *bannerfile = NULL;
  char *helpfile = NULL;
  extern int xferlog;
  extern char *xferfile;
  int i;

  if (isguest) {
    printf("?Command disabled\r\n");
    return (success = 0);
  }

  printf("IKS Settings\r\n");
#ifdef CK_LOGIN
  printf("  Anonymous Initfile:  %s\r\n", anonfile ? anonfile : "<none>");
  printf("  Anonymous Login:     %d\r\n", ckxanon);
  printf("  Anonymous Root:      %s\r\n", anonroot ? anonroot : "<none>");
#endif /* CK_LOGIN */
  printf("  Bannerfile:          %s\r\n", bannerfile ? bannerfile : "<none>");
  printf("  CDfile:              %s\r\n",
         cdmsgfile[0] ? cdmsgfile[0] : "<none>");
  for (i = 1; i < 16 && cdmsgfile[i]; i++) {
    printf("  CDfile:              %s\r\n", cdmsgfile[i]);
  }
  printf("  CDMessage:           %d\r\n", srvcdmsg);
#ifdef IKSDB
  printf("  DBfile:              %s\r\n", dbfile ? dbfile : "<none>");
  printf("  DBenabled:           %d\r\n", dbenabled);
#endif /* IKSDB */
#ifdef CK_LOGIN
#endif /* CK_LOGIN */
  printf("  Helpfile:            %s\r\n", helpfile ? helpfile : "<none>");
  printf("  Initfile:            %s\r\n", kermrc ? kermrc : "<none>");
  printf("  No-Initfile:         %d\r\n", noinit);
#ifdef CK_LOGIN
#ifdef CK_PERM
  printf("  Permission code:     %0d\r\n", ckxperms);
#endif /* CK_PERM */
#ifdef UNIX
  printf("  Privileged Login:    %d\r\n", ckxpriv);
#endif /* UNIX */
#endif /* CK_LOGIN */
  printf("  Server-only:         %d\r\n", arg_x);
  printf("  Syslog:              %d\r\n", ckxsyslog);
#ifdef CK_LOGIN
  printf("  Timeout (seconds):   %d\r\n", logintimo);
  printf("  Userfile:            %s\r\n", userfile ? userfile : "<none>");
#ifdef CKWTMP
  printf("  Wtmplog:             %d\r\n", ckxwtmp);
  printf("  Wtmpfile:            %s\r\n", wtmpfile ? wtmpfile : "<none>");
#endif /* CKWTMP */
#endif /* CK_LOGIN */
  printf("  Xferfile:            %s\r\n", xferfile ? xferfile : "<none>");
  printf("  Xferlog:             %d\r\n", xferlog);
#else  /* IKSDCONF */
  printf("?Nothing to show.\r\n");
#endif /* IKSDCONF */
  return (success = 1);
}

#endif /* NOSHOW */

#ifdef CK_LOGIN
int ckxlogin(CHAR *userid, CHAR *passwd, CHAR *acct, int promptok)
/* ckxlogin */ {
#ifdef CK_RECALL
  extern int on_recall; /* around Password prompting */
#endif                  /* CK_RECALL */
  int rprompt = 0;      /* Restore prompt */
#ifdef CKSYSLOG
  int savlog;
#endif /* CKSYSLOG */

  extern int what, srvcdmsg;

  int x = 0, ok = 0, rc = 0;
  CHAR *_u = NULL, *_p = NULL, *_a = NULL;

  debug(F111, "ckxlogin userid", userid, promptok);
  debug(F110, "ckxlogin passwd", passwd, 0);

  isguest = 0; /* Global "anonymous" flag */

  if (!userid) {
    userid = (CHAR *)"";
  }
  if (!passwd) {
    passwd = (CHAR *)"";
  }

  debug(F111, "ckxlogin userid", userid, what);

#ifdef CK_RECALL
  on_recall = 0;
#endif /* CK_RECALL */

#ifdef CKSYSLOG
  savlog = ckxsyslog; /* Save and turn off syslogging */
#endif                /* CKSYSLOG */

  if ((!*userid || !*passwd) && /* Need to prompt for missing info */
      promptok) {
    cmsavp(psave, PROMPTL); /* Save old prompt */
    debug(F110, "ckxlogin saved", psave, 0);
    rprompt = 1;
  }
  if (!*userid) {
    if (!promptok) {
      return (0);
    }
    cmsetp("Username: "); /* Make new prompt */
    concb((char)escape);  /* Put console in cbreak mode */
    cmini(1);

    /* Flush typeahead */

#ifdef IKS_OPTION
    debug(F101, "ckxlogin TELOPT_SB(TELOPT_KERMIT).kermit.me_start", "",
          TELOPT_SB(TELOPT_KERMIT).kermit.me_start);
#endif /* IKS_OPTION */

    while (ttchk() > 0) {
      x = ttinc(0);
      debug(F101, "ckxlogin flush user x", "", x);
      if (x < 0) {
        doexit(GOOD_EXIT, 0); /* Connection lost */
      }
#ifdef TNCODE
      if (sstelnet) {
        if (x == IAC) {
          x = tn_doop((CHAR)(x & 0xff), ckxech, ttinc);
          debug(F101, "ckxlogin user tn_doop", "", x);
#ifdef IKS_OPTION
          debug(F101, "ckxlogin user TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
                "", TELOPT_SB(TELOPT_KERMIT).kermit.me_start);
#endif /* IKS_OPTION */

          if (x < 0) {
            goto XCKXLOG;
          }
          switch (x) {
          case 1:
            ckxech = 1;
            break; /* Turn on echoing */
          case 2:
            ckxech = 0;
            break; /* Turn off echoing */
#ifdef IKS_OPTION
          case 4: /* IKS event */
            if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
              break; /* else fall thru... */
            }
#endif            /* IKS_OPTION */
          case 6: /* Logout */
            goto XCKXLOG;
          }
        }
      }
#endif /* TNCODE */
    }
    if (pflag) {
      prompt(xxstring); /* Issue prompt if at top level */
    }
    cmres();               /* Reset the parser */
    for (x = -1; x < 0;) { /* Prompt till they answer */
      /* Get a literal line of text */
      x = cmtxt("Your username, or \"ftp\", or \"anonymous\"", "", &s, NULL);
      if (x == -4 || x == -10) {
        printf("\r\n%sLogin cancelled\n", x == -10 ? "Timed out: " : "");
#ifdef CKSYSLOG
        ckxsyslog = savlog;
#endif /* CKSYSLOG */
        doexit(GOOD_EXIT, 0);
      }
      if (sstate) { /* Did a packet come instead? */
        goto XCKXLOG;
      }
      cmres(); /* Reset the parser again */
    }
    if ((_u = (CHAR *)malloc((int)strlen(s) + 1)) == NULL) {
      printf("?Internal error: malloc\n");
      goto XCKXLOG;
    } else {
      strcpy((char *)_u, s); /* safe */
      userid = _u;
    }
  }
  ok = zvuser((char *)userid); /* Verify username */
  debug(F111, "ckxlogin zvuser", userid, ok);

  if (!*passwd && promptok) {
    char prmpt[80];

#ifdef CKSYSLOG
    savlog = ckxsyslog; /* Save and turn off syslogging */
    ckxsyslog = 0;
#endif /* CKSYSLOG */

    /* Flush typeahead again */

    while (ttchk() > 0) {
      x = ttinc(0);
      debug(F101, "ckxlogin flush user x", "", x);
#ifdef TNCODE
      if (sstelnet) {
        if (x == IAC) {
          x = tn_doop((CHAR)(x & 0xff), ckxech, ttinc);
          debug(F101, "ckxlogin pass tn_doop", "", x);
#ifdef IKS_OPTION
          debug(F101, "ckxlogin pass TELOPT_SB(TELOPT_KERMIT).kermit.me_start",
                "", TELOPT_SB(TELOPT_KERMIT).kermit.me_start);
#endif /* IKS_OPTION */
          if (x < 0) {
            goto XCKXLOG;
          }
          switch (x) {
          case 1:
            ckxech = 1;
            break; /* Turn on echoing */
          case 2:
            ckxech = 0;
            break; /* Turn off echoing */
          case 4:  /* IKS event */
            if (!TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
              break; /* else fall thru... */
            }
          case 6: /* Logout */
            goto XCKXLOG;
          }
        }
      }
#endif /* TNCODE */
    }
    if (!strcmp((char *)userid, "anonymous") ||
        !strcmp((char *)userid, "ftp")) {
      if (!ok) {
        goto XCKXLOG;
      }
      ckstrncpy(prmpt, "Enter e-mail address as Password: ", 80);
    } else if (*userid && strlen((char *)userid) < 60) {
      ckmakmsg(prmpt, 80, "Enter ", (char *)userid, "'s Password: ", NULL);
    } else {
      ckstrncpy(prmpt, "Enter Password: ", 80);
    }
    cmsetp(prmpt);       /* Make new prompt */
    concb((char)escape); /* Put console in cbreak mode */
    if (strcmp((char *)userid, "anonymous") &&
        strcmp((char *)userid, "ftp")) { /* and if not anonymous */
      debok = 0;
      cmini(0); /* and no-echo mode */
    } else {
      cmini(1);
    }
    if (pflag) {
      prompt(xxstring); /* Issue prompt if at top level */
    }
    cmres();               /* Reset the parser */
    for (x = -1; x < 0;) { /* Prompt till they answer */
#ifdef CK_PAM
      gotemptypasswd = 0;
#endif                             /* CK_PAM */
      x = cmtxt("", "", &s, NULL); /* Get a literal line of text */
      if (x == -4 || x == -10) {
        printf("\r\n%sLogin cancelled\n", x == -10 ? "Timed out: " : "");
#ifdef CKSYSLOG
        ckxsyslog = savlog;
#endif /* CKSYSLOG */
        doexit(GOOD_EXIT, 0);
      }
#ifdef CK_PAM
      if (!*s) {
        gotemptypasswd = 1;
      }
#endif              /* CK_PAM */
      if (sstate) { /* In case of a Kermit packet */
        goto XCKXLOG;
      }
      cmres(); /* Reset the parser again */
    }
    printf("\r\n"); /* Echo a CRLF */
    if ((_p = (CHAR *)malloc((int)strlen(s) + 1)) == NULL) {
      printf("?Internal error: malloc\n");
      goto XCKXLOG;
    } else {
      strcpy((char *)_p, s); /* safe */
      passwd = _p;
    }
  }
#ifdef CK_PAM
  else {
    cmres(); /* Reset the parser */

    /* We restore the prompt now because the PAM engine will call  */
    /* readpass() which will overwrite psave. */
    if (rprompt) {
      cmsetp(psave); /* Restore original prompt */
      debug(F110, "ckxlogin restored", psave, 0);
      rprompt = 0;
    }
  }
#endif /* CK_PAM */

#ifdef CKSYSLOG
  ckxsyslog = savlog;
#endif /* CKSYSLOG */

  if (ok) {
    ok = zvpass((char *)passwd); /* Check password */
    debug(F101, "ckxlogin zvpass", "", ok);
#ifdef CK_PAM
  } else {
    /* Fake pam password failure for nonexistent users */
    sleep(1);
    printf("Authentication failure\n");
#endif /* CK_PAM */
  }

  if (ok > 0 && isguest) {
#ifndef NOPUSH
    nopush = 1;
#endif /* NOPUSH */
    srvcdmsg = 1;
  }
  rc = ok; /* Set the return code */
  if ((char *)uidbuf != (char *)userid) {
    ckstrncpy(uidbuf, (char *)userid, UIDBUFLEN); /* Remember username */
  }

XCKXLOG: /* Common exit */
#ifdef CKSYSLOG
  ckxsyslog = savlog; /* In case of GOTO above */
#endif                /* CKSYSLOG */
  if (rprompt) {
    cmsetp(psave); /* Restore original prompt */
    debug(F110, "ckxlogin restored", psave, 0);
  }
  if (_u || _p || _a) {
    if (_u) {
      free(_u);
    }
    if (_p) {
      free(_p);
    }
    if (_a) {
      free(_a);
    }
  }
  return (rc);
}

int ckxlogout() {
  doexit(GOOD_EXIT, 0); /* doexit calls zvlogout */
  return (0);           /* not reached */
}
#endif /* CK_LOGIN */

#endif /* NOICP */
