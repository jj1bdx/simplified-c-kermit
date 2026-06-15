/*  C K U U S 2  --  User interface strings & help text module for C-Kermit  */

/*
  Authors:
    Frank da Cruz <fdc@columbia.edu>,
      The Kermit Project, New York City
    Jeffrey E Altman <jaltman@secure-endpoints.com>
      Secure Endpoints Inc., New York City
    David Goodwin, New Zealand.

  Copyright (C) 1985, 2024,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
    Last updates: 22 Aug 2022 (HELP TYPE adds /INTERPRET switch).
                  20 Sep 2022 (HELP COPY adds /INTERPRET, /TOSCREEN switches).
                  06 Nov 2022 (fixed formatting of HELP SET TELNET).
                  12 Nov 2022 (converted four function help strings to arrays).
                  02 Dec 2022 (changed ssh v2 macs list in windows "help ssh").
                  03 Dec 2022 (fixed misplaced definition of cr_year).
                  12 Apr 2023 (ANSI-ize function definitions)
                  25 Jan 2024 (Added HELP REMOTE STATUS)
                  03 Feb 2024 (Added HELP REMOTE CWD)

  This module contains HELP command and other long text strings.

  IMPORTANT: As of 2022, character string constants longer than about 509 are
  not portable.  Longer strings should be broken up into arrays of strings and
  accessed with hmsga() rather than hmsg().  The length limit might be lower
  in older C compilers.
*/
/* clang-format off */
#include "ckcdeb.h"
/* clang-format on */
#include "ckcnet.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcxla.h"

#include "ckcfnp.h"      /* Prototypes (must be last) */
static int dohfile(int); /* Prototype for static func */

extern char *ck_cryear; /* For copyright notice */
extern xx_strp xxstring;
extern char *ccntab[];
/*
  hlptok contains the string for which the user requested help.  This is
  useful for distinguishing synonyms, in case different help text is needed
  depending on which synonym was given.
*/
extern char *hlptok;

#ifndef NOIKSD
extern int inserver;
#endif /* IKSD */

#ifndef NOICP
extern int cmflgs;

#ifdef DCMDBUF
extern char *cmdbuf, *atmbuf;
#else
extern char cmdbuf[], atmbuf[];
#endif /* DCMDBUF */
#endif /* NOICP */

extern char *xarg0;
extern int nrmt, nprm, dfloc, local, parity, escape;
extern int turn, flow;
extern int binary, quiet, keep;
extern int success, xaskmore;
extern int tt_rows, tt_cols;
extern int cmd_rows, cmd_cols;

extern long speed;
extern char *dftty, *versio, *ckxsys;
#ifndef NOHELP
extern char *helpfile;
#endif /* NOHELP */
extern struct keytab prmtab[];
#ifndef NOXFER
extern struct keytab remcmd[];
#endif /* NOXFER */

#ifndef NOICP

/*  Interactive help strings  */

/* Top-level HELP text.  IMPORTANT: Also see tophlpi[] for IKSD. */

static char *tophlp[] = {
    "Trustees of Columbia University in the City of New York.\n",

#ifndef NOHELP
    "  Type EXIT    to exit.",
    "  Type INTRO   for a brief introduction to C-Kermit.",
    "  Type LICENSE to see the C-Kermit license.",
    "  Type HELP    followed by a command name for help about a specific "
    "command.",
#ifndef NOPUSH
#ifdef UNIX
    "  Type MANUAL  to access the C-Kermit manual page.",
#else
    "  Type MANUAL  to access the C-Kermit manual.",
#endif /* UNIX */
#endif /* NOPUSH */
    "  Type NEWS    for news about new features.",
    "  Type SUPPORT to learn how to get technical support.",
    "  Press ?      (question mark) at the prompt, or anywhere within a "
    "command,",
    "               for a menu (context-sensitive help, menu on demand).",
#else
    "Press ? for a list of commands; see documentation for detailed "
    "descriptions.",
#endif /* NOHELP */

#ifndef NOCMDL
#ifndef NOHELP
    " ",
    "  Type HELP OPTIONS for help with command-line options.",
#endif /* NOHELP */
#endif /* NOCMDL */
    " ",
    "DOCUMENTATION: \"Using C-Kermit\" by Frank da Cruz and Christine M. "
    "Gianone,",
    "2nd Edition, Digital Press / Butterworth-Heinemann 1997, ISBN "
    "1-55558-164-1,",
    "plus supplements at http://www.kermitproject.org/ckermit.html#doc.",
    ""};

#ifndef NOIKSD
#ifdef NOHELP
static char *tophlpi[] =
    {/* Top-level help for IKSD */

     "Trustees of Columbia University in the City of New York.\n",

#ifndef NOHELP
     "  Type INTRO   for a brief introduction to Kermit commands.",
     "  Type VERSION for version and copyright information.",
     "  Type HELP    followed by a command name for help about a specific "
     "command.",
     "  Type SUPPORT to learn how to get technical support.",
     "  Type LOGOUT  (or EXIT) to log out.",
     "  Press ?      (question mark) at the prompt, or anywhere within a "
     "command,",
     "               for a menu (context-sensitive help, menu on demand).",
#else
     "Press ? for a list of commands; see documentation for detailed "
     "descriptions.",
#endif /* NOHELP */
     " ",
     "DOCUMENTATION: \"Using C-Kermit\" by Frank da Cruz and Christine M. "
     "Gianone,",
     "2nd Edition, Digital Press (1997), ISBN 1-55558-164-1.  More info at the",
     "Kermit Project website, http://www.kermitproject.org/.",
     ""};
#endif /* NOHELP */
#endif /* NOIKSD */

#ifndef NOHELP
char *newstxt[] = {
    "Welcome to C-Kermit 10.0.",
    "New features since version 9.0 of 2011 include:",
    /* For 10.0 */
    " . Updated for longevity... Adapted to 2020s compilers and OS's without",
    "    sacrificing compatability with older platforms going back to the "
    "1970s",
    "    and 1980s; at least that's the intention.",
    " . The first new C-Kermit release for Windows in 20 years.",
    " . A simpler version number: 10.0.",
    " . Updated OpenSSL support for built-in HTTP, FTP, and Telnet clients.",
    " . New serial port speeds up to 4000000 bps.",
    " . 'set speed ?' now lists serial speeds in numerical order.",
    " . New functions and built-in variables for the scripting language.",
    " . New ability of Kermit scripts to run in a Unix pipelines.",
    " . New CHANGE command for changing strings in external text files.",
    " . DIRECTORY command fixed to once again allow multiple filespecs.",
    " . TOUCH command fixed after being broken in C-Kermit 9.0.",
    " . Lots more; see https://kermitproject.org/updates.html", " ",
    "Documentation:", " . https://www.kermitproject.org/ckbindex.html",
    "    Online index to C-Kermit documentation.",
    " . https://www.kermitproject.org/ckututor.html", "    C-Kermit tutorial.",
    " ",
    "If the release date shown by the VERSION command is long past, be sure to",
    "check the Kermit website to see if there have been updates:", " ",
    "  https://www.kermitproject.org/ (Kermit Project home page)",
    "  https://www.kermitproject.org/ckermit.html (C-Kermit home page)",
#ifdef BETATEST
    "  https://www.kermitproject.org/ckupdates.html (C-Kermit change log)",
    "  https://www.kermitproject.org/ck10devbuilds.html (Beta test builds "
    "table)",
#endif /* BETATEST */
    " ", "If the Kermit Project website is gone, look on Github:", " ",
    "  https://github.com/KermitProject", ""};
#endif /* NOHELP */

#ifndef NOHELP
char *introtxt[] = {
#ifdef UNIX
    "Welcome to UNIX C-Kermit communications software for:",
#else
#ifdef VOS
    "Welcome to VOS C-Kermit communications software for:",
#else
    "Welcome to C-Kermit communications software for:",
#endif /* VOS */
#endif /* UNIX */
#ifndef NOXFER
    " . Error-free and efficient file transfer",
#endif /* NOXFER */
#ifndef NOLOCAL
    " . Terminal connection",
#endif /* NOLOCAL */
#ifndef NOSPL
    " . Script programming",
#endif /* NOSPL */
#ifndef NOICS
    " . International character set conversion",
#endif /* NOICS */
#ifndef NODIAL
#ifndef NOSPL
    " . Numeric and alphanumeric paging",
#endif /* NOSPL */
#endif /* NODIAL */

#ifndef NOLOCAL
    " ",
    "Supporting:",
    " . Serial connections, direct or dialed.",
#ifndef NODIAL
    " . Automatic modem dialing",
#endif /* NODIAL */
#ifdef TCPSOCKET
    " . TCP/IP network connections:",
#ifdef TNCODE
    "   - Telnet sessions",
#endif /* TNCODE */
#ifdef SSHBUILTIN
    "   - SSH v2 connections",
#else
#ifdef ANYSSH
    "   - SSH connections via external agent",
#endif /* ANYSSH */
#endif /* SSHBUILTIN */
#ifdef RLOGCODE
    "   - Rlogin sessions",
#endif /* RLOGCODE */
#ifdef NEWFTP
    "   - FTP sessions",
#endif /* NEWFTP */
#ifdef CKHTTP
    "   - HTTP 1.1 sessions",
#endif /* CKHTTP */
#ifdef IKSD
    "   - Internet Kermit Service",
#endif /* IKSD */
#endif /* TCPSOCKET */
#ifdef ANYX25
    " . X.25 network connections",
#endif /* ANYX25 */
#endif /* NOLOCAL */

    " ",
    "While typing commands, you may use the following special characters:",
    " . DEL, RUBOUT, BACKSPACE, CTRL-H: Delete the most recent character "
    "typed.",
    " . CTRL-W:      Delete the most recent word typed.",
    " . CTRL-U:      Delete the current line.",
    " . CTRL-R:      Redisplay the current line.",

#ifdef CK_RECALL
    " . CTRL-P:      Command recall - go backwards in command recall buffer.",
    " . CTRL-B:      Command recall - same as Ctrl-P.",
    " . CTRL-N:      Command recall - go forward in command recall buffer.",
#endif /* CK_RECALL */
#ifndef NOLASTFILE
    " . CTRL-K:      Insert the most recently entered local file specifiction.",
#endif /* NOLASTFILE */

    " . ?            (question mark) Display a menu for the current command "
    "field.",
    " . ESC          (or TAB or Ctrl-I) Attempt to complete the current field.",
    " . \\            (backslash) include the following character literally",
#ifndef NOSPL
    "                or introduce a backslash code, variable, or function.",
#else
    "                or introduce a numeric backslash code.",
#endif /* NOSPL */
    " ",

    "IMPORTANT: Since backslash (\\) is Kermit's command-line escape "
    "character,",
    "you must enter DOS, Windows, or OS/2 pathnames using either forward slash "
    "(/)",
    "or double backslash (\\\\) as the directory separator in most contexts.",
    "Examples: C:/TMP/README.TXT, C:\\\\TMP\\\\README.TXT.",
    " ",

    "Command words other than filenames can be abbreviated in most contexts.",
    " ",

    "Basic commands:",
    "  EXIT          Exit from Kermit",
    "  HELP          Request general help",
    "  HELP command  Request help about the given command",
    "  TAKE          Execute commands from a file",
    "  TYPE          Display a file on your screen",
    "  ORIENTATION   Explains directory structure",
    " ",

#ifndef NOXFER
    "Commands for file transfer:",
    "  SEND          Send files",
    "  RECEIVE       Receive files",
    "  GET           Get files from a Kermit server",
#ifdef CK_RESEND
    "  RESEND        Recover an interrupted send",
    "  REGET         Recover an interrupted get from a server",
#endif /* CK_RESEND */
#ifndef NOSERVER
    "  SERVER        Be a Kermit server",
#endif /* NOSERVER */
    " ",
    "File-transfer speed selection:",
    "  FAST          Use fast settings -- THIS IS THE DEFAULT",
    "  CAUTIOUS      Use slower, more cautious settings",
    "  ROBUST        Use extremely slow and cautious settings",
    " ",
    "File-transfer performance fine tuning:",
    "  SET RECEIVE PACKET-LENGTH  Kermit packet size",
    "  SET WINDOW                 Number of sliding window slots",
    "  SET PREFIXING              Amount of control-character prefixing",
#endif /* NOXFER */

#ifndef NOLOCAL
    " ",
    "To make a direct serial connection:",
    "  SET LINE      Select serial communication device",
    "  SET SPEED     Select communication speed",
    "  SET PARITY    Communications parity (if necessary)",
#ifdef CK_RTSCTS
    "  SET FLOW      Communications flow control, such as RTS/CTS",
#else
    "  SET FLOW      Communications flow control, such as XON/XOFF",
#endif /* CK_RTSCTS */
    "  CONNECT       Begin terminal connection",

#ifndef NODIAL
    " ",
    "To dial out with a modem:",
    "  SET DIAL DIRECTORY     Specify dialing directory file (optional)",
    "  SET DIAL COUNTRY-CODE  Country you are dialing from (*)",
    "  SET DIAL AREA-CODE     Area-code you are dialing from (*)",
    "  LOOKUP                 Lookup entries in your dialing directory (*)",
    "  SET MODEM TYPE         Select modem type",
    "  SET LINE               Select serial communication device",
    "  SET SPEED              Select communication speed",
    "  SET PARITY             Communications parity (if necessary)",
    "  DIAL                   Dial the phone number",
    "  CONNECT                Begin terminal connection",
    " ",
    "Further info:   HELP DIAL, HELP SET MODEM, HELP SET LINE, HELP SET DIAL",
    "(*) (For use with optional dialing directory)",
#endif /* NODIAL */

#ifdef NETCONN
    " ",
    "To make a network connection:",
#ifndef NODIAL
    "  SET NETWORK DIRECTORY  Specify a network services directory (optional)",
    "  LOOKUP                 Lookup entries in your network directory",
#endif /* NODIAL */
    "  SET NETWORK TYPE       Select network type (if more than one available)",
    "  SET HOST               Make a network connection but stay in command "
    "mode",
    "  CONNECT                Begin terminal connection",
#ifdef TNCODE
    "  TELNET                 Select a Telnet host and CONNECT to it",
#endif /* TNCODE */
#ifdef RLOGCODE
    "  RLOGIN                 Select an Rlogin host and CONNECT to it",
#endif /* RLOGCODE */
#ifdef ANYSSH
    "  SSH [ OPEN ]           Select an SSH host and CONNECT to it",
#endif /* ANYSSH */
#ifdef NEWFTP
    "  FTP [ OPEN ]           Make an FTP connection",
#endif /* NEWFTP */
#ifdef CKHTTP
    "  HTTP OPEN              Make an HTTP connection",
#endif /* CKHTTP */
#endif /* NETCONN */

    " ",
    "To return from a terminal connection to the C-Kermit prompt:",
    "  Type your escape character followed by the letter C.",
    " ",
    "To display your escape character:",
    "  SHOW ESCAPE",
    " ",
    "To display other settings:",
    "  SHOW COMMUNICATIONS, SHOW TERMINAL, SHOW FILE, SHOW PROTOCOL, etc.",
#else  /* !NOLOCAL */
    " ",
    "To display settings:",
    "  SHOW COMMUNICATIONS, SHOW FILE, SHOW PROTOCOL, etc.",
#endif /* NOLOCAL */
    " ",
    "The manual for C-Kermit is the book \"Using C-Kermit\".  For information",
    "about the manual, visit:",
    "  http://www.kermitproject.org/usingckermit.html",
    " ",
    "For an online C-Kermit tutorial, visit:",
    "  http://www.kermitproject.org/ckututor.html",
    " ",
    "To learn about script programming and automation:",
    "  http://www.kermitproject.org/ckscripts.html",
    " ",
    "For further information about a particular command, type HELP xxx,",
    "where xxx is the name of the command.  For documentation, news of new",
    "releases, and information about other Kermit software, visit the",
    "Kermit Project website:",
    " ",
    "  http://www.kermitproject.org/",
    " ",
    "For information about technical support please visit this page:",
    " ",
    "  http://www.kermitproject.org/support.html",
    ""};

static char *hmfstrcmp[] = {
    "\\fstrcmp(s1,s2[,case[,start[,length]]])",
    "  s1, s2 = strings",
    "  case, start, length = numbers or arithmetic expressions.",
    "    case = 0 [default] means to do a case-independent comparison;",
    "    nonzero case requests a case-sensitive comparison.",
    "    The optional start and length arguments apply to both s1 and s2",
    "    and allow specification of substrings if it is not desired to compare",
    "    the whole strings.  Results for non-ASCII strings are implentation-",
    "    and locale-dependent.",
    "  Returns a number:",
    "    -1: s1 is lexically less than s2;",
    "     0: s1 and s2 are lexically equal;",
    "     2: s1 is lexically greater than s2.",
    ""};

static char *hmffileinfo[] = {
    "\\ffileinfo(s1,&a)",
    "  s1 = file specification string",
    "  &a = array designator for results (required)",
    "  Returns a number:",
    "     0: File not found or not accessible or bad arguments;",
    "    >0: The number of attributes returned in the array, normally 7, 8, or "
    "9",
    " 1. The file's name",
    " 2. The full path of the directory where the file resides",
    " 3. The file's modification date-time yyyymmdd hh:mm:ss",
    " 4. Platform-specific permissions string, e.g. drwxrwxr-x or "
    "RWED,RWE,RE,E",
    " 5. Platform-specific permissions code, e.g. an octal number like 40775",
    " 6. The file's size in bytes",
    " 7. Type: regular file, executable, directory, link, or unknown",
    " 8. If link, the name of the file linked to",
    " 9. Transfer mode for file: text or binary.",
    ""};

static char *hmfdayname[] = {
    "\\fdayname(s1,n)",
    "  s1 = free-format date OR day-of-week number 1-7 OR leave blank.",
    "  n  = function code: 0 to return full name; nonzero to return "
    "abbreviation.",
    "  Returns a string: the name of the weekday for the given date or weekday",
    "    number or, if s1 was omitted, of the current date, in the language "
    "and",
    "    character-set specified by the locale.  If n is nonzero, the result",
    "    is abbreviated in the locale-appropriate way.  If given inappropriate",
    "    arguments, the result is empty and an error message is printed.",
    ""};

static char *hmfmonname[] = {
    "\\fmonthname(s1,n)",
    "  s1 = free-format date OR month-of-year number 1-12 OR leave blank.",
    "  n  = function code: 0 to return full name; nonzero to return "
    "abbreviation.",
    "  Returns a string: the name of the month for the given date or month",
    "    number or, if s1 was omitted, of the current date, in the language "
    "and",
    "    character-set specified by the locale.  If n is nonzero, the result",
    "    is abbreviated in the locale-appropriate way.  If given inappropriate",
    "    arguments, the result is empty and an error message is printed.",
    ""};

static char *hmxymatch[] = {
    "SET MATCH { DOTFILE, FIFO } { ON, OFF }",
    "  Tells whether wildcards should match dotfiles (files whose names begin",
    "  with period) or UNIX FIFO special files.  MATCH FIFO default is OFF.",
    "  MATCH DOTFILE default is OFF in UNIX, ON elsewhere.", ""};

#ifndef NOSEXP
static char *hmxysexp[] = {
    "SET SEXPRESSION { DEPTH-LIMIT, ECHO-RESULT, TRUNCATE-ALL-RESULTS }",
    "  DEPTH-LIMIT sets a limit on the depth of nesting or recursion in",
    "  S-Expressions to prevent the program from crashing from memory "
    "exhaustion.",
    "  ECHO-RESULT tells whether S-Expression results should be displayed as",
    "  a result of evaluating an expression; the default is to display only at",
    "  top (interactive) level; OFF means never display; ON means always "
    "display.",
    "  TRUNCATE-ALL-RESULTS ON means the results of all arithmetic operations",
    "  should be forced to integers (whole numbers) by discarding any "
    "fractional",
    "  part.  The default is OFF.  SHOW SEXPRESSION displays the current "
    "settings.",
    ""};
#endif /* NOSEXP */

static char *hmxxfunc[] = {
    "KERMIT FUNCTIONS",
    "   ",
    "  Functions are part of Kermit's programming language used in writing",
    "  scripts.  They are like functions in other programming languages like C",
    "  and Perl; each function has a name and an argument list, and it returns",
    "  (is replaced by) a value.  In a Kermit script, the function name "
    "preceded",
    "  by a backslash (\\) and then the letter F; for example:",
    " ",
    "     \\Findex(string1,string2).",
    " ",
    "  The argument list is in parentheses.  In this example the name of the",
    "  function is 'index', the arguments are string1 and string2, and the "
    "return",
    "  value is the starting position of string2 in string1; type HELP FUNC "
    "INDEX",
    "  for details.",
    " ",
    "  Type SHOW FUNCTIONS to see a list of available functions.",
    " ",
    "  Type HELP FUNCTION xxx more information about specific functions:",
    " ",
    "   . If xxx matches only one function name the documentation for that",
    "     function is printed; example: HELP FUNCTION INDEX.",
    " ",
    "  . If xxx matches more than one function name, a list of all functions",
    "     whose names contain xxx is printed; example: HELP FUNCTION DATE.",
    ""};

#ifdef SSHBUILTIN
static const char *hmxxskrm[] = {
    "SKERMIT [ OPEN ] host [ port ] /PASSWORD:pwd /USER:username",
    "  This is an approximate synonym for: ",
    "    SSH OPEN host port /PASSWORD:pwd /USER:username /SUBSYSTEM:kermit",
    "  Which opens an SSH connection to the host using the kermit subsystem. "
    "This",
    "  requires kermit to be registered as a subsystem with the remote SSH "
    "server.",
    "  For more details on this, see: https://kermitproject.org/skermit.html",
    ""};
#endif /* SSHBUILTIN */

#ifdef ANYSSH
static char *hmxxssh[] = {
#ifdef SSHBUILTIN
/* In Kermit 95, help content is provided by the currently loaded SSH backend.
 * This help text will only be output when no backend is loaded. If K95 was
 * built with SSHBUILTIN and not SSH_DLL, then the SSH backend is compiled in
 * so can never not be loaded.
 */
#ifdef SSH_DLL
    "SSH LOAD filename",
    "  This command is only available when no SSH backend DLL was loaded on ",
    "  startup, either due to there being no compatible DLL available or due "
    "to",
    "  the loading of optional network libraries being disabled via command "
    "line",
    "  parameter. ",
    " ",
    "  This command takes one or more DLL filenames separated by a semicolon "
    "(;)",
    "  which will attempted in order. The first DLL that loads successfully "
    "will",
    "  enable all SSH commands and be used for all SSH operations until Kermit "
    "is",
    "  restarted.",
    " ",
#endif
#else  /* SSHBUILTIN */
    "Syntax: SSH [ options ] <hostname> [ command ]",
    "  Makes an SSH connection using the external ssh program via the SET SSH",
    "  COMMAND string, which is \"ssh -e none\" by default.  Options for the",
    "  external ssh program may be included.  If the hostname is followed by a",
    "  command, the command is executed on the host instead of an interactive",
    "  shell.",
#endif /* SSHBUILTIN */
    ""};

static char *hmxyssh[] = {
#ifdef SSHBUILTIN
/* In Kermit 95, help content is provided by the currently loaded SSH backend.
 * This help text will only be output when no backend is loaded. If K95 was
 * built with SSHBUILTIN and not SSH_DLL, then the SSH backend is compiled in
 * so can never not be loaded.
 */
#ifdef SSH_DLL
    "No SSH backend loaded. If you have a suitable backend DLL, you can load",
    "it with the SSH LOAD command.",
#endif /* SSH_DLL */
#else  /* SSHBUILTIN */
    "Syntax: SET SSH COMMAND command",
    "  Specifies the external command to be used to make an SSH connection.",
    "  By default it is \"ssh -e none\" (ssh with no escape character).",
#endif /* SSHBUILTIN */
    ""};
#endif /* ANYSSH */

#ifdef NEWFTP
static char *hmxygpr[] = {
    "Syntax: SET GET-PUT-REMOTE { AUTO, FTP, KERMIT}",
    "  Tells Kermit whether GET, PUT, and REMOTE commands should be directed",
    "  at a Kermit server or an FTP server.  The default is AUTO, meaning that",
    "  if you have only one active connection, the appropriate action is taken",
    "  when you give a GET, PUT, or REMOTE command.  SET GET-PUT-REMOTE FTP "
    "forces",
    "  Kermit to treat GET, PUT, and REMOTE as FTP client commands; setting "
    "this",
    "  to KERMIT forces these commands to be treated as Kermit client "
    "commands.",
    "  NOTE: PUT includes SEND, MPUT, MSEND, and all other similar commands.",
    "  Also see HELP REMOTE, HELP SET LOCUS, HELP FTP.",
    ""};
#endif /* NEWFTP */

#ifdef LOCUS
static char *hmxylocus[] = {
    "Syntax: SET LOCUS { AUTO, LOCAL, REMOTE }",
    "  Specifies whether unprefixed file management commands should operate",
    "  locally or (when there is a connection to a remote FTP or Kermit",
    "  server) sent to the server.  The affected commands are: CD (CWD), PWD,",
    "  CDUP, DIRECTORY, DELETE, RENAME, MKDIR, and RMDIR.  To force any of",
    "  these commands to be executed locally, give it an L-prefix: LCD, LDIR,",
    "  etc.  To force remote execution, use the R-prefix: RCD, RDIR, and so",
    "  on.  SHOW COMMAND shows the current Locus.",
    " ",
    "  By default, the Locus for file management commands is switched",
    "  automatically whenever you make or close a connection: if you make an",
    "  FTP connection, the Locus becomes REMOTE; if you close an FTP "
    "connection",
    "  or make any other kind of connection, the Locus becomes LOCAL.",
    " ",
    "  If you give a SET LOCUS LOCAL or SET LOCUS REMOTE command, this sets",
    "  the locus as indicated and disables automatic switching.",
    "  SET LOCUS AUTO restores automatic switching.",
    "",
};
#endif /* LOCUS */

#ifdef UNIX
#ifndef NOPUTENV
static char *hmxxputenv[] = {
    "Syntax: PUTENV name value",
    "  Creates or modifies the environment variable with the given name to "
    "have",
    "  the given value.  Purpose: to pass parameters to subprocesses without",
    "  having them appear on the command line.  If the value is blank (empty),",
    "  the variable is deleted.  The result is visible to this instantiation "
    "of",
    "  C-Kermit via \\$(name) and to any inferior processes by whatever method",
    "  they use to access environment variables.  The value may be enclosed in",
    "  doublequotes or braces, but it need not be; if it is the outermost",
    "  doublequotes or braces are removed.",
    " ",
    "  Note the syntax:",
    "    PUTENV name value",
    "  not:",
    "    PUTENV name=value",
    " ",
    "  There is no equal sign between name and value, and the name itself may",
    "  not include an equal sign.",
    "",
};
#endif /* NOPUTENV */
#endif /* UNIX */

static char *hmxxtak[] = {
    "Syntax: TAKE filename [ arguments ]",
    "  Tells Kermit to execute commands from the named file.  Optional "
    "argument",
    "  words, are automatically assigned to the macro argument variables \\%1",
    "  through \\%9.  Kermit command files may themselves contain TAKE "
    "commands,",
    "  up to any reasonable depth of nesting.",
    ""};

#ifdef TCPSOCKET
static char *hmxxfirew[] = {
    "Firewall Traversal in C-Kermit",
    " ",
#ifndef NEWFTP
#ifndef CKHTTP
#ifndef CK_SOCKS
#define NOFIREWALL
#endif
#endif
#endif
#ifdef NOFIREWALL
    "This version of Kermit was built with no support for firewall traversal",
    "protocols.  Kermit can be built with support for HTTP Proxy Servers,",
    "SOCKS authorized firewall traversal, and FTP Passive connection modes.",
    " ",
#else /* NOFIREWALL */
#ifdef CKHTTP
    "The simplist form of firewall traversal is the HTTP CONNECT command. The",
    "CONNECT command was implemented to allow a public web server which "
    "usually",
    "resides on the boundary between the public and private networks to "
    "forward",
    "HTTP requests from clients on the private network to public web sites.  "
    "To",
    "allow secure web connections, the HTTP CONNECT command authenticates the",
    "client with a username/password and then establishes a tunnel to the",
    "desired host.",

    " ",

    "Web servers that support the CONNECT command can be configured to allow",
    "outbound connections for authenticated users to any TCP/IP hostname-port",
    "combination accessible to the Web server.  HTTP CONNECT can be used only",
    "with TCP-based protocols.  Protocols such as Kerberos authentication that",
    "use UDP/IP cannot be tunneled using HTTP CONNECT.",

    " ",

    "SET TCP HTTP-PROXY [switches] [<hostname or ip-address>[:<port>]]",
    "  If a hostname or ip-address is specified, Kermit uses the given",
    "  proxy server when attempting outgoing TCP connections.  If no hostnamer",
    "  or ip-address is specified, any previously specified Proxy server is",
    "  removed.  If no port number is specified, the \"http\" service is used.",
    "  [switches] can be one or more of:",
    "     /AGENT:<agent> /USER:<user> /PASSWORD:<password>",
    "  Switch parameters are used when connecting to the proxy server and",
    "  override any other values associated with the connection.",
    " ",

#endif /* CKHTTP */
#ifdef CK_SOCKS

    "In the early 1990s as firewalls were becoming prevalent, David Koblas",
    "developed the SOCKS protocol for TCP/IP firewall traversal.  Two versions",
    "of SOCKS are currently in use: Version 4.2 lets TCP/IP client "
    "applications",
    "traverse firewalls, similar to HTTP CONNECT, except that the SOCKS client",
    "is aware of the public source IP address and port, which can be used "
    "within",
    "the application protocol to assist in securing the connection (e.g. FTP",
    "sessions secured with GSSAPI Kerberos 5).",

    " ",

    "In 1995 the IETF issued SOCKS Protocol Version 5 (RFC 1928), which is",
    "significantly more general than version 4.  Besides supporting client-",
    "to-server TCP/IP connections, it also includes:",

    " ",
    " . Authenticated firewall traversal of UDP/IP packets.",
    " . Authenticated binding of incoming public ports on the firewall.",
    " ",

    "This lets a service on the private network offer public services.  It "
    "also",
    "lets client applications like FTP establish a temporary public presence",
    "that can be used by the FTP server to create a data channel.  By allowing",
    "the client to bind to a public port on the firewall and be aware of the",
    "public address, SOCKS 5 lets the application protocol communicate this",
    "information to the server.",

    " ",

#ifdef CK_SOCKS5
    "This version of C-Kermit supports SOCKS version 5.",
#else  /* CK_SOCKS5 */
    "This version of C-Kermit supports SOCKS version 4.",
#endif /* CK_SOCKS5 */

    "See the man page (or other system documentation) for information on",
    "configuring the SOCKS library via the /etc/socks.conf file.",

    " ",
#endif /* CK_SOCKS */

#ifdef NEWFTP

    "FTP is one of the few well-known Internet services that requires",
    "multiple connections.  As described above, FTP originally required the",
    "server to establish the data connection to the client using a destination",
    "address and port provided by the client.  This doesn't work with port",
    "filtering firewalls.",

    " ",

    "Later, FTP protocol added a \"passive\" mode, in which connections for",
    "the data channels are created in the reverse direction.  Instead of the",
    "server establishing a connection to the client, the client makes a second",
    "connection with the server as the destination.  This works just fine as",
    "long as the client is behind the firewall and the server is in public",
    "address space.  If the server is behind a firewall then the traditional",
    "active mode must be used.  If both the client and server are behind their",
    "own port filtering firewalls then data channels cannot be established.",

    " ",

    "In Kermit's FTP client, passive mode is controlled with the command:",

    " ",
    "  SET FTP PASSIVE-MODE { ON, OFF }",
    " ",

    "The default is ON, meaning to use passive mode.",

#endif /* NEWFTP */
#endif /* NOFIREWALL */

    ""};
#endif /* TCPSOCKET */

static char *hmxxsave[] = {
    "Syntax: SAVE item filename { NEW, APPEND }",
    "  Saves the requested material in the given file.  A new file is created",
    "  by default; include APPEND at the end of the command to append to an",
    "  existing file.  Items:",
#ifndef NOSETKEY
    "    KEYMAP               Saves the current key settings.",
#endif /* NOSETKEY */
#ifdef CK_RECALL
    "    COMMAND HISTORY      Saves the current command recall (history) "
    "buffer",
#endif /* CK_RECALL */
    ""};

#ifdef CKROOT
static char *hmxxchroot[] = {
    "Syntax: SET ROOT directoryname",
    "  Sets the root for file access to the given directory and disables "
    "access",
    "  to system and shell commands and external programs.  Once this command",
    "  is given, no files or directories outside the tree rooted by the given",
    "  directory can be opened, read, listed, deleted, renamed, or accessed in",
    "  any other way.  This command can not be undone by a subsequent SET ROOT",
    "  command.  Primarily for use with server mode, to restrict access of",
    "  clients to a particular directory tree.  Synonym: CHROOT.",
    ""};
#endif /* CKROOT */

static char *hmxxscrn[] = {
    "Syntax: SCREEN { CLEAR, CLEOL, MOVE row column }",
    "  Performs screen-formatting actions.  Correct operation of these "
    "commands",
    "  depends on proper terminal setup on both ends of the connection -- "
    "mainly",
    "  that the host terminal type is set to agree with the kind of terminal "
    "or",
    "  the emulation you are viewing C-Kermit through.",
    " ",
    "SCREEN CLEAR",
    "  Moves the cursor to home position and clears the entire screen.",
    "  Synonyms: CLS, CLEAR SCREEN.",
    " ",
    "SCREEN CLEOL",
    "  Clears from the current cursor position to the end of the line.",
    " ",
    "SCREEN MOVE row column",
    "  Moves the cursor to the indicated row and column.  The row and column",
    "  numbers are 1-based so on a 24x80 screen, the home position is 1 1 and",
    "  the lower right corner is 24 80.  If a row or column number is given "
    "that",
    "  too large for what Kermit or the operating system thinks is your screen",
    "  size, the appropriate number is substituted.",
    " ",
    "Also see:",
    "  SHOW VARIABLE TERMINAL, SHOW VARIABLE COLS, SHOW VAR ROWS, SHOW "
    "COMMAND.",
    ""};

#ifndef NOSPL
static char *hmfword[] = {
    "Function \\fword(s1,n1,s2,s3,n2,n3) - Extracts a word from a string.",
    "    s1 = source string.",
    "    n1 = word number (1-based) counting from left; if negative, from "
    "right.",
    "    s2 = optional break set.",
    "    s3 = optional include set (or ALL, CSV, or TSV).",
    "    n2 = optional grouping mask.",
    "    n3 = optional separator flag:",
    "       0 = collapse adjacent separators;",
    "       1 = don't collapse adjacent separators.",
    " ",
    "  \\fword() returns the n1th \"word\" of the string s1, according to the",
    "  criteria specified by the other parameters.",
    " ",
    "  The BREAK SET is the set of all characters that separate words. The",
    "  default break set is all characters except ASCII letters and digits.",
    "  ASCII (C0) control characters are treated as break characters by "
    "default,",
    "  as are spacing and punctuation characters, brackets, and so on, and",
    "  all 8-bit characters.",
    " ",
    "  The INCLUDE SET is the set of characters that are to be treated as ",
    "  parts of words even though they normally would be separators.  The",
    "  default include set is empty.  Three special symbolic include sets are",
    "  also allowed:",
    "   ",
    "    ALL (meaning include all bytes that are not in the break set)",
    "    CSV (special treatment for Comma-Separated-Value records)",
    "    TSV (special treatment for Tab-Separated-Value records)",
    " ",
    "  For operating on 8-bit character sets, the include set should be ALL.",
    " ",
    "  If the GROUPING MASK is given and is nonzero, words can be grouped by",
    "  quotes or brackets selected by the sum of the following:",
    " ",
    "     1 = doublequotes:    \"a b c\"",
    "     2 = braces:          {a b c}",
    "     4 = apostrophes:     'a b c'",
    "     8 = parentheses:     (a b c)",
    "    16 = square brackets: [a b c]",
    "    32 = angle brackets:  <a b c>",
    " ",
    "  Nesting is possible with {}()[]<> but not with quotes or apostrophes.",
    " ",
    "Returns string:",
    "  Word number n1, if there is one, otherwise an empty string.",
    " ",
    "Also see:",
    "  HELP FUNCTION SPLIT",
    ""};

static char *hmxxprompt[] = {
    "Syntax: PROMPT [ text ]",
    "  Enters interactive command level from within a script in such a way "
    "that",
    "  the script can be continued with an END or RETURN command.  STOP, EXIT,",
    "  SHOW STACK, TRACE, and Ctrl-C all have their normal effects.  The "
    "PROMPT",
    "  command allows variables to be examined or changed, or any other "
    "commands",
    "  to be given, in any number, prior to returning to the script, allowing",
    "  Kermit to serve as its own debugger; adding the PROMPT command to a "
    "script",
    "  is like setting a breakpoint.  If the optional text is included, it is",
    "  used as the new prompt for this level, e.g. \"prompt Breakpoint_1>\".",
    ""};

static char *hxxinp[] = {
    "Syntax:  INPUT [ /COUNT:n /CLEAR /NOMATCH /NOWRAP ] \
{ number-of-seconds, time-of-day } [ text ]",
    " ",
    "Examples:",
    "  INPUT 5 Login:",
    "  INPUT 23:59:59 RING",
    "  INPUT /NOMATCH 10",
    "  INPUT /CLEAR 10 \\13\\10",
    "  INPUT /CLEAR 10 \\13\\10$\32",
    "  INPUT /COUNT:256 10",
    "  INPUT 10 \\fpattern(<*@*.*>)",
    " ",
    "  Waits up to the given number of seconds, or until the given time of "
    "day,",
    "  for the given text to arrive on the connection. For use in script "
    "programs",
    "  with IF FAILURE or IF SUCCESS, which tell whether the desired text "
    "arrived",
    "  within the given amount of time.",
    " ",
    "  The text, if given, can be a regular text or it can be a \\fpattern()",
    "  invocation, in which case it is treated as a pattern rather than a "
    "literal",
    "  string (HELP PATTERNS for details).",
    " ",
    "  If the /COUNT: switch is included, INPUT waits for the given number of",
    "  characters to arrive.",
    " ",
    "  If no text is specified, INPUT waits for the first character that "
    "arrives",
    "  and makes it available in the \\v(inchar) variable.  This is equivalent "
    "to",
    "  including a /COUNT: switch with an argument of 1.",
    " ",
    "  If the /NOMATCH switch is included, INPUT does not attempt to match any",
    "  characters, but continues reading from the communication connection "
    "until",
    "  the timeout interval expires.  If the timeout interval is 0, the INPUT",
    "  command does not wait; i.e. the given text must already be available "
    "for",
    "  reading for the INPUT command to succeed.  If the interval is negative,",
    "  the INPUT command waits forever.",
    " ",
    "  The INPUT buffer, \\v(input), is normally circular.  Incoming material "
    "is",
    "  appended to it until it reaches the end, and then it wraps around to "
    "the",
    "  beginning.  If the /CLEAR switch is given, INPUT clears its buffer "
    "before",
    "  reading from the connection.",
    " ",
    "  Typical example of use:",
    " ",
    "    INPUT 10 login:",
    "    IF FAIL EXIT 1 \"Timed out waiting for login prompt\"",
    "    LINEOUT myuserid",
    "    INPUT 10 Password:",
    "    IF FAIL EXIT 1 \"Timed out waiting for Password prompt\"",
    "    LINEOUT xxxxxxx",
    " ",
    "  The /NOWRAP switch means that INPUT should return with failure status",
    "  and with \\v(instatus) set to 6 if its internal buffer fills up before",
    "  any of the other completion criteria are met.  This allows for capture",
    "  of the complete incoming data stream (except NUL bytes, which are",
    "  discarded).  CAUTION: if the search target (if any) spans the INPUT "
    "buffer",
    "  boundary, INPUT will not succeed.",
    " ",
    "  \
\\v(instatus) values are: 0 (success), 1 (timed out), 2 (keyboard interrupt),",
    "  3 (internal error), 4 (I/O error or connection lost), 5 (INPUT "
    "disabled),",
    "  and 6 (buffer filled and /NOWRAP set); these are shown by "
    "\\v(inmessage).",
    " ",
    "  Also see OUTPUT, MINPUT, REINPUT, SET INPUT and CLEAR.  See HELP PAUSE "
    "for",
    "  details on time-of-day format and HELP PATTERNS for pattern syntax.",
    ""};

static char *hxxout[] = {
    "Syntax: OUTPUT text",
    "  Sends the text out the communications connection, as if you had typed "
    "it",
    "  during CONNECT mode.  The text may contain backslash codes, variables,",
    "  etc, plus the following special codes:",
    " ",
    "    \\N - Send a NUL (ASCII 0) character (you can't use \\0 for this).",
    "    \\B - Send a BREAK signal.",
    "    \\L - Send a Long BREAK signal.",
    " ",
    "Also see SET OUTPUT.",
    ""};

static char *hxxcsn[] = {
    "Compact Substring Notation is a shorthand notation for the built-in",
    "\\fsubstring() function; 'name' is the name of any macro-type variable:",
    "  \\s(name[n:m])",
    "      Substring of \\m(name) starting at position n, length m",
    "  \\s(name[n_m])",
    "      Substring of \\m(name) from position n to position m",
    "  \\s(name[n]) or \\s(name[n:])",
    "      Substring of \\m(name) from position n to the end",
    "  \\s(name[n.])",
    "      The character at position n",
    ""};
#endif /* NOSPL */

static char *hxypari[] = {
    "SET PARITY NONE",
    "  Chooses 8 data bits and no parity.",
    " ",
    "SET PARITY { EVEN, ODD, MARK, SPACE }",
    "  Chooses 7 data bits plus the indicated kind of parity.",
    "  Forces 8th-bit prefixing during file transfer.",
    " ",
#ifdef HWPARITY
    "SET PARITY HARDWARE { EVEN, ODD }",
    "  Chooses 8 data bits plus the indicated kind of parity.",
    " ",
    "Also see SET TERMINAL BYTESIZE, SET SERIAL, and SET STOP-BITS.",
#else
    "Also see SET TERMINAL BYTESIZE and SET SERIAL.",
#endif /* HWPARITY */
    ""};

#ifndef NOLOCAL
static char *hxyesc[] = {
    "Syntax: SET ESCAPE number",
    "  Decimal ASCII value for escape character during CONNECT, normally 28",
    "  (Control-\\).  Type the escape character followed by C to get back to "
    "the",
    "  C-Kermit prompt or followed by ? to see other options.",
    " ",
    "You may also enter the escape character as ^X (circumflex followed by a",
    "letter or one of: @, ^, _, [, \\, or ], to indicate a control character;",
    "for example, SET ESC ^_ sets your escape character to Ctrl-Underscore.",
    " ",
    "You can also specify an 8-bit character (128-255) as your escape "
    "character,",
    "either by typing it literally or by entering its numeric code.",
    ""};
#endif /* NOLOCAL */

#ifndef NOSPL
static char *hxyout[] = {
    "SET OUTPUT PACING <number>",
    "  How many milliseconds to pause after sending each OUTPUT character,",
    "  normally 0.",
    " ",
    "SET OUTPUT SPECIAL-ESCAPES { ON, OFF }",
    "  Whether to process the special OUTPUT-only escapes \\B, \\L, and \\N.",
    "  Normally ON (they are processed).",
    ""};

static char *hxyinp[] = {
    "Syntax: SET INPUT parameter value",
    " ",
#ifdef CK_AUTODL
    "SET INPUT AUTODOWNLOAD { ON, OFF }",
    "  Controls whether autodownloads are allowed during INPUT command "
    "execution.",
    " ",
#endif /* CK_AUTODL */
    "SET INPUT BUFFER-LENGTH number-of-bytes",
    "  Removes the old INPUT buffer and creates a new one with the given "
    "length.",
    " ",
    "SET INPUT CANCELLATION { ON, OFF }",
    "  Whether an INPUT in progress can be can interrupted from the keyboard.",
    " ",
    "SET INPUT CASE { IGNORE, OBSERVE }",
    "  Tells whether alphabetic case is to be significant in string "
    "comparisons.",
    "  This setting is local to the current macro or command file, and is",
    "  inherited by subordinate macros and take files.",
    " ",
    "SET INPUT ECHO { ON, OFF }",
    "  Tells whether to display arriving characters read by INPUT on the "
    "screen.",
    " ",
#ifdef CKFLOAT
    "SET INPUT SCALE-FACTOR <number>",
    "  A number to multiply all INPUT timeouts by, which may include a "
    "fractional",
    "  part, e.g. 2.5.  All INPUT commands that specify a timeout in seconds",
    "  (as opposed to a specific time of day) have their time limit adjusted",
    "  automatically by this factor, which is also available in the built-in",
    "  read-only variable \\v(inscale).  The default value is 1.0.",
    " ",

#endif /* CKFLOAT */

    "SET INPUT SILENCE <number>",
    "  The maximum number to seconds of silence (no input at all) before the",
    "  INPUT command times out, 0 for no maximum.",
    " ",
    "SET INPUT TIMEOUT-ACTION { PROCEED, QUIT }",
    "  Tells whether to proceed or quit from a script program if an INPUT "
    "command",
    "  fails.  PROCEED (default) allows use of IF SUCCESS / IF FAILURE "
    "commands.",
    "  This setting is local to the current macro or command file, and is",
    "  inherited by subordinate macros and take files.",
    ""};

static char *hxyfunc[] = {
    "SET FUNCTION DIAGNOSTICS { ON, OFF }",
    "  Whether to issue diagnostic messages for illegal function calls and",
    "  references to nonexistent built-in variables.  ON by default.",
    " ",
    "SET FUNCTION ERROR { ON, OFF }",
    "  Whether an illegal function call or reference to a nonexistent built-in",
    "  variable should cause a command to fail.  OFF by default.",
    ""};
#endif /* NOSPL */

static char *hxyxyz[] = {
#ifdef CK_XYZ
#ifdef XYZ_INTERNAL

    /* This is for built-in protocols */

    "Syntax: SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 [ s3 ] ]",
    "  Selects protocol to use for transferring files.  String s1 is a command "
    "to",
    "  send to the remote host prior to SENDing files with this protocol in",
    "  binary mode; string s2 is the same thing but for text mode.  Use \"%s\" "
    "in",
    "  any of these strings to represent the filename(s).  If the protocol is",
    "  KERMIT, you may also specify a string s3, the command to start a Kermit",
    "  server on the remote host when you give a GET, REGET, REMOTE, or other",
    "  client command.  Use { braces } if any command contains spaces.  "
    "Examples:",
    " ",
    "    set proto xmodem {rx %s} {rx -a %s}",
    "    set proto kermit {kermit -YQir} {kermit -YQTr} {kermit -YQx}",

#else  /* This is for when non-Kermit protocols are external */

    "Syntax: \
SET PROTOCOL { KERMIT, XMODEM, YMODEM, ZMODEM } [ s1 s2 s3 s4 s5 s6 ]",
    "  Selects protocol to use for transferring files.  s1 and s2 are commands "
    "to",
    "  output prior to SENDing with this protocol, to automatically start the",
    "  RECEIVE process on the other end in binary or text mode, respectively.",
    "  If the protocol is KERMIT, s3 is the command to start a Kermit server "
    "on",
    "  the remote computer, and there are no s4-s6 commands.  Otherwise, s3 "
    "and",
    "  s4 are commands used on this computer for sending files with this "
    "protocol",
    "  in binary or text mode, respectively; s5 and s6 are the commands for",
    "  receiving files with this protocol.  Use \"%s\" in any of these strings",
    "  to represent the filename(s).  Use { braces } if any command contains",
    "  spaces.  Examples:",
    " ",
    "    set proto kermit {kermit -YQir} {kermit -YQTr} {kermit -YQx}",
    "    set proto ymodem rb {rb -a} {sb %s} {sb -a %s} rb rb",
    " ",
    "External protocols require REDIRECT and external file transfer programs "
    "that",
    "use redirectable standard input/output.",
#endif /* XYZ_INTERNAL */
#else
    "Syntax: \
SET PROTOCOL KERMIT [ s1 [ s2 [ s3 ] ] ]",
    "  Lets you specify the autoupload binary, autoupload text, and autoserver",
    "  command strings to be sent to the remote system in advance of any SEND",
    "  or GET commands.  By default these are \"kermit -ir\", \"kermit -r\", "
    "and",
    "  \"kermit -x\".  Use { braces } around any command that contains spaces.",
    "  Example:",
    " ",
    "    set proto kermit {kermit -Yir} {kermit -YTr} {kermit -Yx}",
#endif /* CK_XYZ */
    " ",
    "  SHOW PROTOCOL displays the current settings.",
    ""};

static char *hmxxbye = "Syntax: BYE\n\
  Shut down and log out a remote Kermit server";

#ifdef CK_PERMS
#ifdef UNIX
static char *hmxxchmod[] = {
    "Syntax: CHMOD [ switches ] code filespec",
    "  UNIX only.  Changes permissions of the given file(s) to the given code,",
    "  which must be an octal number such as 664 or 775.  Optional switches:",
    " ",
    "   /FILES        Only change permissions of regular files.",
    "   /DIRECTORIES  Only change permissions of directory files.",
    "   /TYPE:BINARY  Only change permissions of binary files.",
    "   /TYPE:TEXT    Only change permissions of text files.",
    "   /DOTFILES     Include files whose names begin with dot (.).",
    "   /RECURSIVE    Change permissions in subdirectories too.",
    "   /LIST         List each file (synonym: /VERBOSE).",
    "   /NOLIST       Operate silently (synonym: /QUIET).",
    "   /PAGE         When listing, pause at end of each screen (implies "
    "/LIST).",
    "   /NOPAGE       When listing, don't pause at end of each screen.",
    "   /SIMULATE     Show what would be done but don't actually do it.",
    ""};
#endif /* UNIX */
#endif /* CK_PERMS */

#ifndef NOSPL
#ifndef NOSEXP
static char *hmxxsexp[] = {
    "Syntax: (operation operand [ operand [ ... ] ])",
    " ",
    "  C-Kermit includes a simple LISP-like S-Expression parser operating on",
    "  numbers only.  An S-Expression is always enclosed in parentheses.  The",
    "  parentheses can contain (a) a number, (b) a variable, (c) a function "
    "that",
    "  returns a number, or (d) an operator followed by one or more operands.",
    "  Operands can be any of (a) through (c) or an S-Expression.  Numbers can "
    "be",
    "  integers or floating-point.  Any operand that is not a number and does "
    "not",
    "  start with backslash (\\) is treated as a Kermit macro name.  "
    "Operators:",
    " ",
    " Operator  Action                                 Example           Value",
    "  EVAL (.)  Returns the contained value            (6)               6",
    "  QUOTE (') Inhibits evaluation of following value (quote a)         a",
    "  SETQ      Assigns a value to a global variable   (setq a 2)        2",
    "  LET       Assigns a value to a local variable    (let b -1.3)     -1.3",
    "  +         Adds all operands (1 or more)          (+ a b)           0.7",
    "  -         Subtracts all operands (1 or more)     (- 9 5 2 1)       1",
    "  *         Multiplies all operands (1 or more)    (* a (+ b 1) 3)  -1.8",
    "  /         Divides all operands (1 or more)       (/ b a 2)        "
    "-0.325",
    "  ^         Raise given number to given power      (^ 3 2)           9",
    "  ++        Increments a variable                  (++ a 1.2)        3.2",
    "  --        Decrements a variable                  (-- a)            1",
    "  ABS       Absolute value of 1 operand            (abs (* a b 3))   7.8",
    "  MAX       Maximum of all operands (1 or more)    (max 1 2 3 4)     4",
    "  MIN       Minimum of all operands (1 or more)    (min 1 2 3 4)     1",
    "  MOD       Modulus of all operands (1 or more)    (mod 7 4 2)       1",
    "  TRUNCATE  Integer part of floating-point operand (truncate 1.333)  1",
    "  CEILING   Ceiling of floating-point operand      (ceiling 1.25)    2",
    "  FLOOR     Floor of floating-point operand        (floor 1.25)      1",
    "  ROUND     Operand rounded to nearest integer     (round 1.75)      2",
    "  ROUND     ...or to given number of decimals      (round 1.7584 2)  1.76",
    "  SQRT      Square root of 1 operand               (sqrt 2)          "
    "1.414..",
    "  EXP       e (2.71828..) to the given power       (exp -1)          "
    "0.367..",
    "  SIN       Sine of angle expressed in radians     (sin (/ pi 2))    1.0",
    "  COS       Cosine of given number                 (cos pi)         -1.0",
    "  TAN       Tangent of given number                (tan pi)          0.0",
    "  LOG       Natural log (base e) of given number   (log 2.7183)      "
    "1.000..",
    "  LOG10     Log base 10 of given number            (log10 1000)      3.0",
    " ",
    "Predicate operators return 0 if false, 1 if true, and if it is the "
    "outermost",
    "operator, sets SUCCESS or FAILURE accordingly:",
    " ",
    "  <         Operands in strictly descending order  (< 6 5 4 3 2 1)   1",
    "  <=        Operands in descending order           (<= 6 6 5 4 3 2)  1",
    "  !=        Operands are not equal                 (!= 1 1 1.0)      0",
    "  =   (==)  All operands are equal                 (= 3 3 3 3)       1",
    "  >         Operands in strictly ascending order   (> 1 2 3 4 5 6)   1",
    "  >=        Operands in ascending order            (> 1 1 2 3 4 5)   1",
    "  AND (&&)  Operands are all true                  (and 1 1 1 1 0)   0",
    "  OR  (||)  At least one operand is true           (or 1 1 1 1 0)    1",
    "  XOR       Logical Exclusive OR                   (xor 3 1)         0",
    "  NOT (!)   Reverses truth value of operand        (not 3)           0",
    " ",
    "Bit-oriented operators:",
    " ",
    "  &         Bitwise AND                            (& 7 2)           2",
    "  |         Bitwise OR                             (| 1 2 3 4)       7",
    "  #         Bitwise Exclusive OR                   (# 3 1)           2",
    "  ~         Reverses all bits                      (~ 3)            -4",
    " ",
    "Operators that work on truth values:",
    " ",
    "  IF        Conditional evaluation                 (if (1) 2 3)      2",
    " ",
    "Operators can also be names of Kermit macros that return either numeric",
    "values or no value at all.",
    " ",
    "Built-in constants are:",
    " ",
    "  t         True (1)",
    "  nil       False (empty)",
    "  pi        The value of Pi (3.1415926...)",
    " ",
    "If SET SEXPRESSION TRUNCATE-ALL-RESULTS is ON, all results are trunctated",
    "to integer values by discarding any fractional part.  Otherwise results",
    "are floating-point if there fractional part and integer otherwise.",
    "If SET SEXPRESSION ECHO-RESULT is AUTO (the default), the value of the",
    "S-Expression is printed if the S-Expression is given at top level; if ON,",
    "it is printed at any level; if OFF it is not printed.  At all levels, the",
    "variable \\v(sexpression) is set to the most recent S-Expression, and",
    "\\v(svalue) is set to its value.  You can use the \\fsexpresssion() "
    "function",
    "to evaluate an S-Expression anywhere in a Kermit command.",
    ""};
#endif /* NOSEXP */
#endif /* NOSPL */

static char *hmxxgrep[] = {
    "Syntax: GREP [ options ] pattern filespec",
    "  Searches through the given file or files for the given character string",
    "  or pattern.  In the normal case, all lines containing any text that "
    "matches",
    "  the pattern are printed.  Pattern syntax is as described in HELP "
    "PATTERNS",
    "  except that '*' is implied at the beginning unless the pattern starts "
    "with",
    "  '^' and also at the end unless the pattern ends with '$'.  Therefore,",
    "  \"grep something *.txt\" lists all lines in all *.txt files that "
    "contain",
    "  the word \"something\", but \"grep ^something *.txt\" lists only the "
    "lines",
    "  that START with \"something\".  The command succeeds if any of the "
    "given",
    "  files contains any lines that match the pattern, otherwise it fails.",
    "  Synonym: FIND.",
    " ",
    "Only one filespec can be given.  To search multiple files that can't",
    "be represented by a wildcard use {file1,file2,file3,...} (in braces).",
    " ",
    "File selection options:",
    "  /NOBACKUPFILES",
    "    Excludes backup files (like oofa.txt.~3~) from the search.",
    "  /DOTFILES",
    "    Includes files whose names start with dot (.) in the search.",
    "  /NODOTFILES",
    "    Excludes files whose names start with dot (.) from the search.",
#ifdef RECURSIVE
    "  /RECURSIVE",
    "    Searches through files in subdirectories too.",
#endif /* RECURSIVE */
    "  /TYPE:TEXT",
    "    Search only text files (requires FILE SCAN ON).",
    "  /TYPE:BINARY",
    "    Search only binary files (requires FILE SCAN ON).",
    " ",
    "Pattern-matching options:",
    "  /NOCASE",
    "    Ignores case of letters when comparing.  Depends on the underlying",
    "    operating system APIs to work for non-ASCII character sets.",
    "  /NOMATCH",
    "    Searches for lines that do NOT match the pattern.",
    "  /EXCEPT:pattern",
    "    Exclude lines that match the main pattern that also match this "
    "pattern.",
    "  /VERBATIM",
    "    The search string is taken literally; variables are not evaluated.",
    "    This allows you to (for example) search Kermit scripts that contain",
    "    variable names, function names, etc (that begin with '\\').",
    " ",
    "Display options:",
    "  /COUNT:variable-name",
    "    For each file, prints only the filename and a count of matching lines",
    "    and assigns the total match count to the variable, if one is given.",
    "  /DISPLAY:number",
    "    How many matching lines to show.  The default is to show all matching",
    "    lines.  Synonym:/SHOW.",
    "  /NAMEONLY",
    "    Prints the name of each file that contains at least one matching "
    "line,",
    "    one name per line, rather than showing each matching line.",
    "  /NOLIST",
    "    Doesn't print anything (but sets SUCCESS or FAILURE appropriately).",
    "  /LINENUMBERS",
    "    Precedes each file line by its line number within the file.",
    "  /PAGE",
    "    Pauses after each screenful.",
    "  /NOPAGE",
    "    Doesn't pause after each screenful.",
    " ",
    "Result disposition options:",
    "  /OUTPUT:name",
    "    Sends results to the given file.",
    "  /ARRAY:&x",
    "    Returns the results in the specified array; one line per array "
    "element.",
    "  /MACRO:name",
    "    Returns the results in the macro whose name is given.  If a macro",
    "    of the same name already exists, the grep results (if there are any)",
    "    replace its previous value.  Synonym: DEFINE.",
    ""};

static char *hmxxdir[] = {
#ifdef DOMYDIR
    "Syntax: DIRECTORY [ switches ] [ filespec [ filespec [ ... ] ] ]",
#ifdef LOCUS
    "  If LOCUS is REMOTE or LOCUS is AUTO and you have an FTP connection,",
    "  this command is equivalent to REMOTE DIRECTORY (RDIR).  Otherwise:",
    " ",
#endif /* LOCUS */
    "  Lists local files.  The filespec may be a filename, possibly containing",
    "  wildcard characters, or a directory name.  If no filespec is given, all",
    "  files in the current directory are listed.  If a directory name is "
    "given,",
    "  all the  files in it are listed.  Multiple filespecs can be given.",
    "  Optional switches:",
    " ",
    "   /BRIEF           List filenames only.",
#ifdef CK_PERMS
    "   /VERBOSE       + Also list permissions, size, and date.",
#else
    "   /VERBOSE       + Also list date and size.",
#endif /* CK_PERMS */
    "   /FILES           Show files but not directories.",
    "   /DIRECTORIES     Show directories but not files.",
    "   /ALL           + Show both files and directories.",
    "   /ARRAY:&a        Store file list in specified array (e.g. \\%a[]).",
    "   /PAGE            Pause after each screenful.",
    "   /NOPAGE          Don't pause after each screenful.",
    "   /TOP:n           Only show the top n lines of the directory listing.",
    "   /DOTFILES        Include files whose names start with dot (period).",
    "   /NODOTFILES    + Don't include files whose names start with dot.",
    "   /FOLLOWLINKS     Follow symbolic links.",
    "   /NOFOLLOWLINKS + Don't follow symbolic links.",
    "   /NOLINKS         Don't list symbolic links at all.",
    "   /BACKUP        + Include backup files (names end with .~n~).",
    "   /NOBACKUPFILES   Don't include backup files.",
    "   /OUTPUT:file     Store directory listing in the given file.",
    "   /HEADING         Include heading and summary.",
    "   /NOHEADING     + Don't include heading or summary.",
    "   /COUNT:var       Put the number of matching files in the given "
    "variable.",
    "   /SUMMARY         Print only count and total size of matching files.",
    "   /XFERMODE        Show pattern-based transfer mode (T=Text, B=Binary).",
    "   /TYPE:           Show only files of the specified type (text or "
    "binary).",
    "   /MESSAGE:text    Add brief message to each listing line.",
    "   /NOMESSAGE     + Don't add message to each listing line.",
    "   /NOXFERMODE    + Don't show pattern-based transfer mode",
    "   /ISODATE       + In verbose listings, show date in ISO 8061 format.",
    "   /ENGLISHDATE     In verbose listings, show date in \"English\" format.",
#ifdef RECURSIVE
    "   /RECURSIVE       Descend through subdirectories.",
    "   /NORECURSIVE   + Don't descend through subdirectories.",
#endif /* RECURSIVE */
    "   /SORT:key        Sort by key, NAME, DATE, or SIZE; default key is "
    "NAME.",
    "   /NOSORT        + Don't sort.",
    "   /ASCENDING     + If sorting, sort in ascending order.",
    "   /REVERSE         If sorting, sort in reverse order.",
    " ",
    "Factory defaults are marked with +.  Default for paging depends on SET",
    "COMMAND MORE-PROMPTING.  Use SET OPTIONS DIRECTORY [ switches ] to change",
    "defaults; use SHOW OPTIONS to display customized defaults.  Also see",
    "WDIRECTORY.",
#else
    "Syntax: DIRECTORY [ filespec ]",
    "  Lists the specified file or files.  If no filespec is given, all files",
    "  in the current directory are listed.",
#endif /* DOMYDIR */
    ""};

static char *hmxxtouch[] = {
    "Syntax: TOUCH [ switches ] filespec",
    "  Updates the modification time of the given file or files to the current",
    "  date and time or to the date and time specified in the /MODTIME: "
    "switch.",
    "  If the filespec is the name of a single file that does not exist, the "
    "file",
    "  is created.  The following switches can be used to restrict the files",
    "  to be touched according to various criteria:",
    " ",
    "   /FILES           Select files but not directories.",
    "   /DIRECTORIES     Select directories but not files.",
    "   /ALL           + Select both files and directories.",
    "   /AFTER:          Select files modified after the given date",
    "   /BEFORE:         Select files modified before the given date",
    "   /LARGER:         Select files larger than the given size in bytes",
    "   /SMALLER:        Select files smaller than the given size in bytes",
    "   /EXCEPT:         Exclude the given files (list or pattern)",
    "   /DOTFILES        Include files whose names start with dot (period).",
    "   /NODOTFILES    + Don't include files whose names start with dot.",
    "   /FOLLOWLINKS     For symbolic link touch the linked-to file, not the "
    "link",
    "   /NOFOLLOWLINKS + Select the link itself, not the file it links to.",
    "   /NOLINKS         Skip over symbolic links.",
    "   /BACKUP        + Include backup files (names end with .~n~).",
    "   /NOBACKUPFILES   Don't include backup files.",
    "   /TYPE:           Select only files of the given type, TEXT or BINARY.",
#ifdef RECURSIVE
    "   /RECURSIVE       Descend through subdirectories.",
    "   /NORECURSIVE   + Don't descend through subdirectories.",
    " ",
    " Action switches:",
    " ",
    "   /MODTIME:        Changes the modification time for the selected files.",
    "                     in numeric yyyy:mm:dd:hh:mm:ss format.",
    "                     if hh:mm:ss omitted time is set to 00:00:00",
    "   /SIMULATE        List files that would be touched, but don't touch "
    "them.",
    "   /LIST            Show which files are being touched.",
#endif /* RECURSIVE */
    " ",
    "Factory defaults are marked with +.  Use HELP DATE to learn the date-time",
    "formats usable with /MODTIME:.  If a /MODTIME: switch is not given, each",
    "selected file gets a modification time equal to the current clock time.",
    "You can use the /SIMULATE switch in combination with other switches to "
    "see",
    "which files will be affected without actually changing their dates.",
    ""};

static char *hmxxchange[] = {
    "Syntax: CHANGE [ switches ] filespec string1 string2",
    "  Changes all occurrences of string1 to string2 in the given file or "
    "files.",
    "  Works line by line, does not do multiline or across-line substitutions.",
    "  To remove strings from files, specify string2 as \"\" or omit string2.",
    "  Temporary files are created in the directory indicated by \\v(tmpdir)",
    "  (show var tmpdir).  You can select a different temporary directory with",
    "  the SET TEMP-DIRECTORY command.  All temporary files are deleted after "
    "use.",
    " ",
    "  String1 and String2 should be enclosed in doublequotes \"\" or braces "
    "{} if",
    "  if they contain spaces.  In the event that they already contain braces "
    "or",
    "  doublequotes, especially if these are not balanced, some quoting may be",
    "  required.  Or you can assign the strings to variables and then use the",
    "  variable names in the CHANGE command; example:",
    " ",
    "    .a = {value=\"./",
    "    .b = {value=\"../",
    "    change *.html \\m(a) \\m(b)",
    " ",
    "  Since the CHANGE command works line by line, only text files can be",
    "  changed; C-Kermit automatically skips over binary files.",
    " ",
    "  File selection switches (factory defaults are marked with +):",
    " ",
    "   /AFTER:         Select files modified after the given date",
    "   /BEFORE:        Select files modified before the given date",
    "   /LARGER:        Select files larger than the given size in bytes",
    "   /SMALLER:       Select files smaller than the given size in bytes",
    "   /EXCEPT:        Exclude the given files (list or pattern)",
    "   /DOTFILES       Include files whose names start with dot (period).",
    "   /NODOTFILES   + Don't include files whose names start with dot.",
#ifdef RECURSIVE
    "   /RECURSIVE      Descend through subdirectories.",
    "   /NORECURSIVE  + Don't descend through subdirectories.",
#endif /* RECURSIVE */
    " ",
    " File disposition switches:",
    " ",
    "   /BACKUP:name       Back up original files to named directory.",
    "   /DESTINATION:name  Store resulting changed files in named directory.",
    "   If neither of these options is given, original files are overwritten.",
    " ",
    " String selection switches:",
    " ",
    "   /CASE:{ON,OFF}  OFF (default) = ignore case in string1; \
ON = don't ignore",
    " ",
    " Action switches:",
    " ",
    "   /COUNT:name     Set named variable to number of files that were "
    "changed.",
    "   /SIMULATE       List files that would be changed, but don't change "
    "them.",
    "   /LIST           Show which files are being changed.",
    "   /MODTIME:       Modification time for change files, PRESERVE or "
    "UPDATE.",
    " ",
    "You can use the /SIMULATE switch in combination with other switches to "
    "see",
    "which files will be affected without actually changing them.",
    ""};

#ifndef NOSPL
static char *hmxxkcd[] = {
    "Syntax: KCD symbolic-directory-name",
    "  Kermit Change Directory: Like CD (q.v.) but (a) always acts locally, "
    "and",
    "  (b) takes a symbolic directory name rather than an actual directory "
    "name.",
    "  The symbolic names correspond to Kermit's directory-valued built-in",
    "  variables, such as \\v(download), \\v(exedir), and so on.  Here's the "
    "list:",
    " ",
    "    download      Your download directory (if any)",
    "    exedir        The directory where the Kermit executable resides",
    "    home          Your home, login, or default directory",
    "    inidir        The directory where Kermit's initialization was found",
#ifdef UNIX
    "    lockdir       The UNIX UUCP lockfile directory on this computer",
#endif /* UNIX */
    "    startup       Your current directory at the time Kermit started",
    "    textdir       The directory where Kermit text files reside, if any",
    "    tmpdir        Your temporary directory",
    " ",
    "  Also see CD, SET FILE DOWNLOAD, SET TEMP-DIRECTORY.",
    ""};
#endif /* NOSPL */

static char *hmxxcwd[] = {
#ifdef LOCUS
    "  If LOCUS is REMOTE or LOCUS is AUTO and you have an FTP connection,",
    "  this command is equivalent to REMOTE CD (RCD).  Otherwise:",
    " ",
#endif /* LOCUS */
#ifdef vms
    "Syntax: CD [ directory or device:directory ]",
    "  Change Working Directory.  Equivalent to VMS SET DEFAULT command.",
#else
    "Syntax: CD [ directory name ]",
    "  Change Directory.  Changes your current, working, default directory to "
    "the",
    "  one given, so that future non-absolute filename references are relative "
    "to",
    "  this directory.  If the directory name is omitted, your home (login)",
    "  directory is supplied.",
#endif /* vms */
    "  C-Kermit's default prompt shows your current directory.",
    "  Synonyms: LCD, CWD.",
#ifdef LOCUS
    "  Also see: SET LOCUS, PWD, CDUP, BACK, REMOTE CD (RCD), SET CD, SET "
    "PROMPT.",
#else
    "  Also see: PWD, CDUP, BACK, REMOTE CD (RCD), SET CD, SET PROMPT.",
#endif /* LOCUS */
#ifndef NOSPL
    "  And see: HELP KCD.",
#endif /* NOSPL */
    "  Relevant environment variables: CDPATH, HOME.",
    ""};

static char *hmxxdel[] = {
    "Syntax: DELETE [ switches... ] filespec",
#ifdef LOCUS
    "  If LOCUS is REMOTE or LOCUS is AUTO and you have an FTP connection,",
    "  this command is equivalent to REMOTE DELETE (RDELETE).  Otherwise:",
    " ",
#endif /* LOCUS */
    "  Deletes a file or files on the computer where C-Kermit is running.",
    "  The filespec may denote a single file or can include wildcard "
    "characters",
    "  to match multiple files.  RM is a synonym for DELETE.  Switches "
    "include:",
    " ",
    "/AFTER:date-time",
    "  Specifies that only those files modified after the given date-time are",
    "  to be deleted.  HELP DATE for info about date-time formats.",
    " ",
    "/BEFORE:date-time",
    "  Specifies that only those files modified before the given date-time",
    "  are to be deleted.",
    " ",
    "/NOT-AFTER:date-time",
    "  Specifies that only those files modified at or before the given "
    "date-time",
    "  are to be deleted.",
    " ",
    "/NOT-BEFORE:date-time",
    "  Specifies that only those files modified at or after the given "
    "date-time",
    "  are to be deleted.",
    " ",
    "/LARGER-THAN:number",
    "  Specifies that only those files longer than the given number of bytes "
    "are",
    "  to be deleted.",
    " ",
    "/SMALLER-THAN:number",
    "  Specifies that only those files smaller than the given number of bytes "
    "are",
    "  to be sent.",
    " ",
    "/EXCEPT:pattern",
    "  Specifies that any files whose names match the pattern, which can be a",
    "  regular filename or may contain wildcards, are not to be deleted.  To",
    "  specify multiple patterns (up to 8), use outer braces around the group",
    "  and inner braces around each pattern:",
    " ",
    "    /EXCEPT:{{pattern1}{pattern2}...}",
    " ",
    "/DOTFILES",
    "  Include (delete) files whose names begin with \".\".",
    " ",
    "/NODOTFILES",
    "  Skip (don't delete) files whose names begin with \".\".",
    " ",
    "/TYPE:TEXT",
    "  Delete only regular text files (requires FILE SCAN ON as it is by "
    "default)",
    " ",
    "/TYPE:BINARY",
    "  Delete only regular binary files (requires FILE SCAN ON)",
    " ",
    "/DIRECTORIES",
    "  Include directories.  If this switch is not given, only regular files",
    "  are deleted.  If it is given, Kermit attempts to delete any directories",
    "  that match the given file specification, which succeeds only if the",
    "  directory is empty.",
    " ",
#ifdef RECURSIVE
    "/RECURSIVE",
    "  The DELETE command applies to the entire directory tree rooted in the",
    "  current or specified directory.  When the /DIRECTORIES switch is also",
    "  given, Kermit deletes all the (matching) files in each directory before",
    "  attempting to delete the directory itself.",
    " ",
#endif /* RECURSIVE */
#ifdef UNIX
#ifdef RECURSIVE
    "/ALL",
    "  This is a shortcut for /RECURSIVE /DIRECTORIES /DOTFILES.",
#else
    "/ALL",
    "  This is a shortcut for /DIRECTORIES /DOTFILES.",
#endif /* RECURSIVE */
#else  /* !UNIX */
#ifdef RECURSIVE
    "/ALL",
    "  This is a shortcut for /RECURSIVE /DIRECTORIES.",
#else
    "/ALL",
    "  This is a synonym for /DIRECTORIES.",
#endif /* RECURSIVE */
#endif /* UNIX */
    " ",
    "/LIST",
    "  List each file and tell whether it was deleted.  Synonyms: /LOG, "
    "/VERBOSE.",
    " ",
    "/NOLIST",
    "  Don't list files while deleting.  Synonyms: /NOLOG, /QUIET.",
    " ",
    "/HEADING",
    "  Print heading and summary information.",
    " ",
    "/NOHEADING",
    "  Don't print heading and summary information.",
    " ",
    "/SUMMARY",
    "  Like /HEADING /NOLIST, but only prints the summary line.",
    " ",
    "/PAGE",
    "  If listing, pause after each screenful.",
    " ",
    "/NOPAGE",
    "  Don't pause after each screenful.",
    " ",
    "/ASK",
    "  Interactively ask permission to delete each file.  Reply Yes or OK to",
    "  delete it, No not to delete it, Quit to cancel the DELETE command, and",
    "  Go to go ahead and delete all the rest of the files without asking.",
    " ",
    "/NOASK",
    "  Delete files without asking permission.",
    " ",
    "/SIMULATE",
    "  Preview files selected for deletion without actually deleting them.",
    "  Implies /LIST.",
    " ",
    "Use SET OPTIONS DELETE to make selected switches effective for every "
    "DELETE",
    "command \
unless you override them; use SHOW OPTIONS to see selections currently",
#ifdef LOCUS
    "in effect.  Also see HELP SET LOCUS, HELP PURGE, HELP WILDCARD.",
#else
    "in effect.  Also see HELP PURGE, HELP WILDCARD.",
#endif /* LOCUS */
    ""};

#ifndef NOHTTP
static char *hmxxhttp[] = {
    "Syntax:",
    "HTTP [ <switches> ] OPEN <hostname> <service/port>",
    "  Instructs Kermit to open a new connection for HTTP communication with",
    "  the specified host on the specified port.  The default port is "
    "\"http\".",
    "  If <switches> are specified, they are applied to all subsequent HTTP",
    "  actions (GET, PUT, ...) until an HTTP CLOSE command is executed.",
    "  A URL can be included in place of the hostname and service or port.",
    " ",
    "HTTP CLOSE",
    "  Instructs Kermit to close any open HTTP connection and clear any saved",
    "  switch values.",
    " ",
    "HTTP [ <switches> ] CONNECT <host>[:<port>]",
    "  Instructs the server to establish a connection with the specified host",
    "  and to redirect all data transmitted between Kermit and the host for "
    "the",
    "  life of the connection.",
    " ",
    "HTTP [ <switches> ] GET <remote-filename> [ <local-filename> ]",
    "  Retrieves the named file on the currently open HTTP connection.  The",
    "  default local filename is the same as the remote filename, but with any",
    "  path stripped.  If you want the file to be displayed on the screen "
    "instead",
    "  of stored on disk, include the /TOSCREEN switch and omit the local",
    "  filename.  If you give a URL instead of a remote filename, this "
    "commands",
    "  opens the connection, GETs the file, and closes the connection; the "
    "same",
    "  is true for the remaining HTTP commands for which you can specify a",
    "  remote filename, directory name, or path.",
    " ",
    "HTTP [ <switches> ] HEAD <remote-filename> [ <local-filename> ]",
    "  Like GET except without actually getting the file; instead it gets only",
    "  the headers, storing them into the given file (if a local filename is",
    "  specified), one line per header item as shown in the /ARRAY: switch",
    "  description.",
    " ",
    "HTTP [ <switches> ] INDEX <remote-directory> [ <local-filename> ]",
    "  Retrieves the file listing for the given server directory.",
    "  NOTE: This command is not supported by most Web servers, and even when",
    "  the server understand it, there is no stardard response format.",
    " ",
    "HTTP [ <switches> ] POST [ /MIME-TYPE:<type> ] <local-file> <remote-path>",
    "     [ <dest-file> ]",
    "  Used to send a response as if it were sent from a form.  The data to be",
    "  posted must be read from a file.",
    " ",
    "HTTP [ <switches> ] PUT [ /MIME-TYPE:<type> ] <local-file> <remote-file>",
    "     [ <dest-file> ]",
    "  Uploads the given local file to server file.  If the remote filename is",
    "  omitted, the local name is used, but with any path stripped.",
    " ",
    "HTTP [ <switches> ] DELETE <remote-filename>",
    "  Instructs the server to delete the specified filename.",
    " ",
    "where <switches> are:",
    "/AGENT:<user-agent>",
    "  Identifies the client to the server; \"C-Kermit\"",
    "  by default.",
    " ",
    "/HEADER:<header-line>",
    "  Used for specifying any optional headers.  A list of headers is "
    "provided",
    "  using braces for grouping:",
    " ",
    "    /HEADER:{{<tag>:<value>}{<tag>:<value>}...}",
    " ",
    "  For a listing of valid <tag> value and <value> formats see RFC 1945:",
    "  \"Hypertext Transfer Protocol -- HTTP/1.0\".  A maximum of eight "
    "headers",
    "  may be specified.",
    " ",
    "/TOSCREEN",
    "  Display server responses on the screen.",
    " ",
    "/USER:<name>",
    "  In case a page requires a username for access.",
    " ",
    "/PASSWORD:<password>",
    "  In case a page requires a password for access.",
    " ",
    "/ARRAY:<arrayname>",
    "  Tells Kermit to store the response headers in the given array, one line",
    "  per element.  The array need not be declared in advance.  Example:",
    " ",
    "    http /array:c get kermit/index.html",
    "    show array c",
    "    Dimension = 9",
    "    1. Date: Fri, 26 Nov 1999 23:12:22 GMT",
    "    2. Server: Apache/1.3.4 (Unix)",
    "    3. Last-Modified: Mon, 06 Sep 1999 22:35:58 GMT",
    "    4. ETag: \"bc049-f72-37d441ce\"",
    "    5. Accept-Ranges: bytes",
    "    6. Content-Length: 3954",
    "    7. Connection: close     ",
    "    8. Content-Type: text/html",
    " ",
    "As you can see, the header lines are like MIME e-mail header lines:",
    "identifier, colon, value.  The /ARRAY switch is the only method available",
    "to a script to process the server responses for a POST or PUT command.",
    " ",
    ""};
#endif /* NOHTTP */

#ifndef NOCSETS
static char *hmxxassoc[] = {
    "ASSOCIATE FILE-CHARACTER-SET <file-character-set> "
    "<transfer-character-set>",
    "  Tells C-Kermit that whenever the given file-character set is selected, "
    "and",
    "  SEND CHARACTER-SET (q.v.) is AUTOMATIC, the given transfer "
    "character-set",
    "  is selected automatically.",
    " ",
    "ASSOCIATE XFER-CHARACTER-SET <xfer-character-set> <file-character-set>",
    "  Tells C-Kermit that whenever the given transfer-character set is "
    "selected,",
    "  either by command or by an announcer attached to an incoming text file,",
    "  and SEND CHARACTER-SET is AUTOMATIC, the specified file character-set "
    "is",
    "  to be selected automatically.  Synonym: ASSOCIATE "
    "TRANSFER-CHARACTER-SET.",
    " ",
    "Use SHOW ASSOCIATIONS to list the current character-set associations, and",
    "SHOW CHARACTER-SETS to list the current settings.",
    ""};
#endif /* NOCSETS */

static char *hmxxpat[] = {
    "A \"pattern\" is notation used in a search string when searching through",
    "text.  C-Kermit uses three kinds of patterns: floating patterns, anchored",
    "patterns, and wildcards.  Wildcards are anchored patterns that are used "
    "to",
    "match file names; type HELP WILDCARD to learn about them.",
    " ",
    "In a pattern, certain characters are special:",
    " ",
    "* Matches any sequence of zero or more characters.  For example, \"k*t\"",
    "  matches all strings that start with \"k\" and end with \"t\" including",
    "  \"kt\", \"kit\", \"knight\", or \"kermit\".",
    " ",
    "? Matches any single character.  For example, \"k????t\" matches all "
    "strings",
    "  that are exactly 6 characters long and start with \"k\" and end with",
    "  with \"t\".  When typing commands at the prompt, you must precede any",
    "  question mark to be used for matching by a backslash (\\) to override "
    "the",
    "  normal function of question mark in interactive commands, which is to",
    "  provide menus and file lists.",
    " ",
#ifdef CKREGEX
    "[abc]",
    "  Square brackets enclosing a list of characters matches any character in",
    "  the list.  Example: h[aou]t matches hat, hot, and hut.",
    " ",
    "[a-z]",
    "  Square brackets enclosing a range of characters matches any character "
    "in",
    "  the range; a hyphen (-) separates the low and high elements of the "
    "range.",
    "  For example, [a-z] matches any character from a to z.",
    " ",
    "[acdm-z]",
    "  Lists and ranges may be combined.  This example matches a, c, d, or any",
    "  letter from m through z.",
    " ",
    "{string1,string2,...}",
    "  Braces enclose a list of strings to be matched.  For example:",
    "  ker{mit,nel,beros} matches kermit, kernel, and kerberos.  The strings",
    "  may themselves contain *, ?, [abc], [a-z], or other lists of strings.",
#endif /* CKREGEX */
#ifndef NOSPL
    " ",
    "To force a special pattern character to be taken literally, precede it "
    "with",
    "a backslash, e.g. [a\\-z] matches a, hyphen, or z rather than a through "
    "z.",
    " ",
    "A floating  pattern can also include the following special characters:",
    " ",
    "^ (First character of pattern) Anchors the pattern at the beginning.",
    "$ (Last character of pattern) Anchors the pattern at the end.",
    " ",
    "If a floating pattern does not start with \"^\", the pattern can match",
    "anywhere in the string instead of only at the beginning; in other words, "
    "a",
    "leading \"*\" is assumed.  Similarly, if the pattern doesn't end with "
    "\"$\",",
    "a trailing \"*\" is assumed.",
    " ",
    "The following commands and functions use floating patterns:",
    "  GREP [ <switches> ] <pattern> <filespec>",
    "  TYPE /MATCH:<pattern> <file>",
    "  \\farraylook(<pattern>,<arrayname>)",
    "  \\fsearch(<pattern>,<string>[,<offset>])",
    "  \\frsearch(<pattern>,<string>[,<offset>])",
    "  The /EXCEPT: clause in SEND, GET, DELETE, etc.",
    " ",
    "Example:",
    "  \\fsearch(abc,xxabcxxx) succeeds because xxabcxx contains abc.",
    "  \\fsearch(^abc,xxabcxx) fails because xxabcxx does not start with abc.",
    " ",

    "All other commands and functions use anchored patterns, meaning that ^ "
    "and $",
    "are not treated specially, and * is not assumed at the beginning or end "
    "of",
    "the pattern.  This is true mainly of filename patterns (wildcards), since",
    "you would not want a command like \"delete x\" to delete all files whose",
    "names contained \"x\"!",

    " ",
    "You can use anchored patterns not only in filenames, but also in SWITCH",
    "case labels, in the INPUT and MINPUT commands, and in file binary- and",
    "text-patterns for filenames.  The IF MATCH pattern is also anchored.",
#endif /* NOSPL */
    ""};

static char *hmxxwild[] = {

    "A \"wildcard\" is a notation used in a filename to match multiple files.",
    "For example, in \"send *.txt\" the asterisk is a wildcard.  Kermit "
    "commands",
    "that accept filenames also accepts wildcards, except commands that are",
    "allowed to operate on only one file, such as TRANSMIT.",
    "This version of Kermit accepts the following wildcards:",
    " ",
    "* Matches any sequence of zero or more characters.  For example, "
    "\"ck*.c\"",
    "  matches all files whose names start with \"ck\" and end with \".c\"",
    "  including \"ck.c\".",
    " ",
    "? Matches any single character.  For example, \"ck?.c\" matches all files",
    "  whose names are exactly 5 characters long and start with \"ck\" and end",
    "  with \".c\".  When typing commands at the prompt, you must precede any",
    "  question mark to be used for matching by a backslash (\\) to override "
    "the",
    "  normal function of question mark in interactive commands, which is to",
    "  provide menus and file lists.  You don't, however, need to quote "
    "filename",
    "  question marks in command files (script programs).",
    " ",
#ifdef CKREGEX
    "[abc]",
    "  Square brackets enclosing a list of characters matches any character in",
    "  the list.  Example: ckuusr.[ch] matches ckuusr.c and ckuusr.h.",
    " ",
    "[a-z]",
    "  Square brackets enclosing a range of characters matches any character "
    "in",
    "  the range; a hyphen (-) separates the low and high elements of the "
    "range.",
    "  For example, [a-z] matches any character from a to z.",
    " ",
    "[acdm-z]",
    "  Lists and ranges may be combined.  This example matches a, c, d, or any",
    "  letter from m through z.",
    " ",
    "{string1,string2,...}",
    "  Braces enclose a list of strings to be matched.  For example:",
    "  ck{ufio,vcon,cmai}.c matches ckufio.c, ckvcon.c, or ckcmai.c.  The "
    "strings",
    "  may themselves contain *, ?, [abc], [a-z], or other lists of strings.",
#endif /* CKREGEX */
    " ",
    "To force a special pattern character to be taken literally, precede it "
    "with",
    "a backslash, e.g. [a\\-z] matches a, hyphen, or z rather than a through "
    "z.",
    "Or tell Kermit to SET WILDCARD-EXPANSION OFF before entering or referring",
    "to the filename.",
    " ",
#ifndef NOSPL
    "Similar notation can be used in general-purpose string matching.  Type "
    "HELP",
    "PATTERNS for details.  Also see HELP SET MATCH.",
#endif /* NOSPL */
    ""};

#ifndef NOXFER
static char *hmxxfast[] = {
    "FAST, CAUTIOUS, and ROBUST are predefined macros that set several",
    "file-transfer parameters at once to achieve the desired file-transfer "
    "goal.",
    "FAST chooses a large packet size, a large window size, and a fair amount "
    "of",
    "control-character unprefixing at the risk of possible failure on some",
    "connections.  FAST is the default tuning in C-Kermit 7.0 and later.  In "
    "case",
    "FAST file transfers fail for you on a particular connection, try "
    "CAUTIOUS.",
    "If that fails too, try ROBUST.  You can also change the definitions of "
    "each",
    "macro with the DEFINE command.  To see the current definitions, type",
    "\"show macro fast\", \"show macro cautious\", or \"show macro robust\".",
    ""};
#endif /* NOXFER */

#ifdef CKPURGE
static char *hmxxpurge[] = {
    "Syntax: PURGE [ switches ] [ filespec ]",
    "  Deletes backup files; that is, files whose names end in \".~n~\", where",
    "  n is a number.  PURGE by itself deletes all backup files in the current",
    "  directory.  Switches:",

    " ",
    "/AFTER:date-time",
    "  Specifies that only those files modified after the given date-time are",
    "  to be purged.  HELP DATE for info about date-time formats.",
    " ",
    "/BEFORE:date-time",
    "  Specifies that only those files modified before the given date-time",
    "  are to be purged.",
    " ",
    "/NOT-AFTER:date-time",
    "  Specifies that only those files modified at or before the given "
    "date-time",
    "  are to be purged.",
    " ",
    "/NOT-BEFORE:date-time",
    "  Specifies that only those files modified at or after the given "
    "date-time",
    "  are to be purged.",
    " ",
    "/LARGER-THAN:number",
    "  Specifies that only those files longer than the given number of bytes "
    "are",
    "  to be purged.",
    " ",
    "/SMALLER-THAN:number",
    "  Specifies that only those files smaller than the given number of bytes "
    "are",
    "  to be purged.",
    " ",
    "/EXCEPT:pattern",
    "  Specifies that any files whose names match the pattern, which can be a",
    "  regular filename or may contain wildcards, are not to be purged.  To",
    "  specify multiple patterns (up to 8), use outer braces around the group",
    "  and inner braces around each pattern:",
    " ",
    "    /EXCEPT:{{pattern1}{pattern2}...}",
    " ",
    "/DOTFILES",
    "  Include (purge) files whose names begin with \".\".",
    " ",
    "/NODOTFILES",
    "  Skip (don't purge) files whose names begin with \".\".",
    " ",
#ifdef RECURSIVE
    "/RECURSIVE",
    "  Descends through the current or specified directory tree.",
    " ",
#endif /* RECURSIVE */
    "/KEEP:n",
    "  Retain the 'n' most recent (highest-numbered) backup files for each "
    "file.",
    "  By default, none are kept.  If /KEEP is given without a number, 1 is "
    "used.",
    " ",
    "/LIST",
    "  Display each file as it is processed and say whether it is purged or "
    "kept.",
    "  Synonyms: /LOG, /VERBOSE.",
    " ",
    "/NOLIST",
    "  The PURGE command should operate silently (default).",
    "  Synonyms: /NOLOG, /QUIET.",
    " ",
    "/HEADING",
    "  Print heading and summary information.",
    " ",
    "/NOHEADING",
    "  Don't print heading and summary information.",
    " ",
    "/PAGE",
    "  When /LIST is in effect, pause at the end of each screenful, even if",
    "  COMMAND MORE-PROMPTING is OFF.",
    " ",
    "/NOPAGE",
    "  Don't pause, even if COMMAND MORE-PROMPTING is ON.",
    " ",
    "/ASK",
    "  Interactively ask permission to delete each backup file.",
    " ",
    "/NOASK",
    "  Purge backup files without asking permission.",
    " ",
    "/SIMULATE",
    "  Inhibits the actual deletion of files; use to preview which files would",
    "  actually be deleted.  Implies /LIST.",
    " ",
    "Use SET OPTIONS PURGE [ switches ] to change defaults; use SHOW OPTIONS "
    "to",
    "display customized defaults.  Also see HELP DELETE, HELP WILDCARD.",
    ""};
#endif /* CKPURGE */

static char *hmxxclo[] = {
    "Syntax:  CLOSE [ item ]",
    "  Close the indicated item.  The default item is CONNECTION, which is the",
    "  current SET LINE or SET HOST connection.  The other items are:",
    " ",
#ifdef CKLOGDIAL
    "    CX-LOG          (connection log, opened with LOG CX)",
#endif /* CKLOGDIAL */
#ifndef NOLOCAL
    "    SESSION-LOG     (opened with LOG SESSION)",
#endif /* NOLOCAL */
#ifdef TLOG
    "    TRANSACTION-LOG (opened with LOG TRANSACTIONS)",
#endif /* TLOG */
    "    PACKET-LOG      (opened with LOG PACKETS)",
#ifdef DEBUG
    "    DEBUG-LOG       (opened with LOG DEBUG)",
#endif /* DEBUG */
#ifndef NOSPL
    "    READ-FILE       (opened with OPEN READ)",
    "    WRITE-FILE      (opened with OPEN WRITE or OPEN APPEND)",
#endif /* NOSPL */
    " ",
    "Type HELP LOG and HELP OPEN for further info.",
    ""};

#ifdef CKLEARN
static char *hmxxlearn[] = {
    "Syntax: LEARN [ /ON /OFF /CLOSE ] [ filename ]",
    "  Records a login script.  If you give a filename, the file is opened for",
    "  subsequent recording.  If you don't give any switches, /ON is assumed.",
    "  /ON enables recording to the current file (if any); /OFF disables",
    "  recording.  /CLOSE closes the current file (if any).  After LEARN "
    "/CLOSE",
    "  or exit from Kermit, your script is available for execution by the TAKE",
    "  command.",
    ""};
#endif /* CKLEARN */

#ifdef CK_MINPUT
static char *hmxxminp[] = {
    "Syntax:  MINPUT [ switches ] n [ string1 [ string2 [ ... ] ] ]",
    "Example: MINPUT 5 Login: {Username: } {NO CARRIER} BUSY RING",
    "  For use in script programs.  Waits up to n seconds for any one of the",
    "  strings to arrive on the communication device.  If no strings are "
    "given,",
    "  the command waits for any character at all to arrive.  Strings are",
    "  separated by spaces; use {braces} or \"doublequotes\" for grouping.  If",
    "  any of the strings is encountered within the timeout interval, the "
    "command",
    "  succeeds and the \\v(minput) variable is set to the number of the "
    "string",
    "  that was matched: 1, 2, 3, etc.  If none of the strings arrives, the",
    "  command times out, fails, and \\v(minput) is set to 0.  In all other",
    "  respects, MINPUT is like INPUT.  See HELP INPUT for the available "
    "switches",
    "  and other details of operation.",
    ""};
#endif /* CK_MINPUT */

#ifndef NOLOCAL
static char *hmxxcon[] = {
    "Syntax: CONNECT (or C, or CQ) [ switches ]",
    "  Connect to a remote computer via the serial communications device given "
    "in",
    "  the most recent SET LINE command, or to the network host named in the "
    "most",
    "  recent SET HOST command.  Type the escape character followed by C to "
    "get",
    "  back to the C-Kermit prompt, or followed by ? for a list of "
    "CONNECT-mode",
    "  escape commands.",
    " ",
    "Include the /QUIETLY switch to suppress the informational message that",
    "tells you how to escape back, etc.  CQ is a synonym for CONNECT /QUIETLY.",
    " ",
    "Other switches include:",
#ifdef CK_TRIGGER
    " ",
    "/TRIGGER:string",
    "  One or more strings to look for that will cause automatic return to",
    "  command mode.  To specify one string, just put it right after the",
    "  colon, e.g. \"/TRIGGER:Goodbye\".  If the string contains any spaces, "
    "you",
    "  must enclose it in braces, e.g. \"/TRIGGER:{READY TO SEND...}\".  To",
    "  specify more than one trigger, use the following format:",
    " ",
    "    /TRIGGER:{{string1}{string2}...{stringn}}",
    " ",
    "  Upon return from CONNECT mode, the variable \\v(trigger) is set to the",
    "  trigger string, if any, that was actually encountered.  This value, "
    "like",
    "  all other CONNECT switches applies only to the CONNECT command with "
    "which",
    "  it is given, and overrides (temporarily) any global SET TERMINAL "
    "TRIGGER",
    "  string that might be in effect.",
#endif /* CK_TRIGGER */
    ""};
#endif /* NOLOCAL */

static char *hmxxmget[] = {
    "Syntax: MGET [ switches... ] remote-filespec [ remote-filespec ... ]", " ",
    "Just like GET (q.v.) except allows a list of remote file specifications,",
    "separated by spaces.", ""};

static char *hmxxget[] = {
    "Syntax: GET [ switches... ] remote-filespec [ as-name ]",
    "  Tells the other Kermit, which must be in (or support autoswitching "
    "into)",
    "  server mode, to send the named file or files.  If the remote-filespec "
    "or",
    "  the as-name contain spaces, they must be enclosed in braces.  If "
    "as-name",
    "  is the name of an existing local directory, incoming files are placed "
    "in",
    "  that directory; if it is the name of directory that does not exist, "
    "Kermit",
    "  tries to create it.  Optional switches include:",
    " ",
    "/AS-NAME:text",
    "  Specifies \"text\" as the name to store the incoming file under, or",
    "  directory to store it in.  You can also specify the as-name as the "
    "second",
    "  filename on the GET command line.",
    " ",
    "/BINARY",
    "  Performs this transfer in binary mode without affecting the global",
    "  transfer mode.",
    " ",
    "/COMMAND",
    "  Receives the file into the standard input of a command, rather than "
    "saving",
    "  it on  disk.  The /AS-NAME or the second \"filename\" on the GET "
    "command",
    "  line is interpreted as the name of a command.",
    " ",
    "/DELETE",
    "  Asks the other Kermit to delete the file (or each file in the group)",
    "  after it has been transferred successfully.",
    " ",
    "/EXCEPT:pattern",
    "  Specifies that any files whose names match the pattern, which can be a",
    "  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
    "  are to be refused.  To specify multiple patterns (up to 8), use outer",
    "  braces around the group, and inner braces around each pattern:",
    " ",
    "    /EXCEPT:{{pattern1}{pattern2}...}",
    " ",
    "/FILENAMES:{CONVERTED,LITERAL}",
    "  Overrides the global SET FILE NAMES setting for this transfer only.",
    " ",
    "/FILTER:command",
    "  Causes the incoming file to passed through the given command (standard",
    "  input/output filter) before being written to disk.",
    " ",
#ifdef CK_LABELED
    "/LABELED",
    "  VMS and OS/2 only: Specifies labeled transfer mode.",
    " ",
#endif /* CK_LABELED */

    "/MOVE-TO:directory-name",
    "  Specifies that each file that arrives should be moved to the specified",
    "  directory after, and only if, it has been received successfully.",
    " ",
    "/PATHNAMES:{OFF,ABSOLUTE,RELATIVE,AUTO}",
    "  Overrides the global SET RECEIVE PATHNAMES setting for this transfer.",
    " ",
    "/PIPES:{ON,OFF}",
    "  Overrides the TRANSFER PIPES setting for this command only.  ON allows",
    "  reception of files with names like \"!tar xf -\" to be automatically",
    "  directed to a pipeline.",
    " ",
    "/QUIET",
    "  Suppresses the file-transfer display.",
    " ",
    "/RECOVER",
    "  Used to recover from a previously interrupted transfer; GET /RECOVER",
    "  is equivalent REGET.  Works only in binary mode.",
    " ",
    "/RECURSIVE",
    "  Tells the server to descend through the directory tree when locating",
    "  the files to be sent.",
    " ",
    "/RENAME-TO:string",
    "  Specifies that each file that arrives should be renamed as specified",
    "  after, and only if, it has been received successfully.  The string can",
    "  be a filename, a directory name, an expression involving variables, "
    "etc.",
    " ",
    "/TEXT",
    "  Performs this transfer in text mode without affecting the global",
    "  transfer mode.",
    " ",
    "/TRANSPARENT",
    "  Inhibits character-set translation of incoming text files for the "
    "duration",
    "  of the GET command without affecting subsequent commands.",
    " ",
    "Also see HELP MGET, HELP SEND, HELP RECEIVE, HELP SERVER, HELP REMOTE.",
    ""};

static char *hmxxlg[] = {
    "Syntax: LOG (or L) log-type [ filename [ { NEW, APPEND } ] ]",
    " ",
    "Record information in a log file:",
    " ",
#ifdef CKLOGDIAL
    "CX",
    "  Connections made with SET LINE, SET PORT, SET HOST, DIAL, TELNET, etc.",
    "  The default filename is CX.LOG in your home directory and APPEND is the",
    "  default mode for opening.",
    " ",
#endif /* CKLOGDIAL */
#ifdef DEBUG
    "DEBUG",
    "  Debugging information, to help track down bugs in the C-Kermit program.",
    "  The default log name is debug.log in current directory.",
    " ",
#endif /* DEBUG */
    "PACKETS",
    "  Kermit packets, to help with protocol problems.  The default filename "
    "is",
    "  packet.log in current directory.",
    " ",
#ifndef NOLOCAL
    "SESSION",
    "  Records your CONNECT session (default: session.log in current "
    "directory).",
    " ",
#endif /* NOLOCAL */
#ifdef TLOG
    "TRANSACTIONS",
    "  Names and statistics about files transferred (default: transact.log in",
    "  current directory; see HELP SET TRANSACTION-LOG for transaction-log "
    "format",
    "  options.)",
    " ",
#endif /* TLOG */
    "If you include the APPEND keyword after the filename, the existing log "
    "file,",
    "if any, is appended to; otherwise a new file is created (except APPEND is",
    "the default for the connection log).  Use CLOSE <keyword> to stop "
    "logging.",
    " ",
    "Note: The filename can also be a pipe, e.g.:",
    " ",
    "  log transactions |lpr",
    "  log debug {| grep \"^TELNET\" > debug.log}",
    " ",
    "Braces are required if the pipeline or filename contains spaces.",
    ""};

#ifndef NOSCRIPT
static char *hmxxlogi[] = {
    "\
Syntax: SCRIPT text",
    "  A limited and cryptic \"login assistant\", carried over from old "
    "C-Kermit",
    "  releases for comptability, but not recommended for use.  Instead, "
    "please",
    "  use the full script programming language described in chapters 17-19 of",
    "  \"Using C-Kermit\".",
    " ",
    "  Login to a remote system using the text provided.  The login script",
    "  is intended to operate similarly to UNIX uucp \"L.sys\" entries.",
    "  A login script is a sequence of the form:",
    " ",
    "    expect send [expect send] . . .",
    " ",
    "  where 'expect' is a prompt or message to be issued by the remote site, "
    "and",
    "  'send' is the names, numbers, etc, to return.  The send may also be the",
    "  keyword EOT to send Control-D, or BREAK (or \\\\b) to send a break "
    "signal.",
    "  Letters in send may be prefixed by ~ to send special characters:",
    " ",
    "  ~b backspace, ~s space, ~q '?', ~n linefeed, ~r return, ~c don\'t",
    "  append a return, and ~o[o[o]] for octal of a character.  As with some",
    "  UUCP systems, sent strings are followed by ~r unless they end with ~c.",
    " ",
    "  Only the last 7 characters in each expect are matched.  A null expect,",
    "  e.g. ~0 or two adjacent dashes, causes a short delay.  If you expect",
    "  that a sequence might not arrive, as with uucp, conditional sequences",
    "  may be expressed in the form:",
    " ",
    "    -send-expect[-send-expect[...]]",
    " ",
    "  where dashed sequences are followed as long as previous expects fail.",
    ""};
#endif /* NOSCRIPT */

#ifndef NOFRILLS
static char *hmxxtyp[] = {
    "Syntax: TYPE [ switches... ] file",
    "  Displays a file on the screen.  Pauses automatically at end of each",
    "  screenful if COMMAND MORE-PROMPTING is ON.  Optional switches:",
    " ",
    "  /PAGE",
    "     Pause at the end of each screenful even if COMMAND MORE-PROMPTING "
    "OFF.",
    "     Synonym: /MORE",
    "  /NOPAGE",
    "     Don't pause at the end of each screen even if COMMAND MORE-PROMPTING "
    "ON.",
    "  /HEAD:n",
    "     Only type the first 'n' lines of the file.",
    "  /TAIL:n",
    "     Only type the last 'n' lines of the file.",
    "  /MATCH:pattern",
    "     Only type lines that match the given pattern.  HELP WILDCARDS for "
    "info",
    "     info about patterns.  /HEAD and /TAIL apply after /MATCH.",
    "  /PREFIX:string",
    "     Print the given string at the beginning of each line.",
    "  /NUMBER",
    "     Add line numbers (conflicts with /PREFIX)",
    "  /WIDTH:number",
    "     Truncate each line at the given column number before printing.",
    "  /COUNT",
    "     Count lines (and matches) and print the count(s) but not the lines.",
#ifdef UNICODE
    "  /CHARACTER-SET:name",
    "     Translates from the named character set.",
    "  /TRANSLATE-TO:name",
    "     Translates to the named character set (default = current file "
    "charset).",
    "  /TRANSPARENT",
    "     Inhibits character-set translation.",
#endif /* UNICODE */
#ifdef TYPEINTERPRET
    "  /INTERPRET",
    "     Shows the file with Kermit backslash escapes interpreted.",
#endif /* TYPEINTERPRET */

    "  /OUTPUT:name",
    "     Sends results to the given file.  If this switch is omitted, the",
    "     results appear on your screen.  This switch overrides any express or",
    "     implied /PAGE switch.",
    " ",
    "You can use SET OPTIONS TYPE to set the defaults for /PAGE or /NOPAGE and",
    "/WIDTH.  Use SHOW OPTIONS to see current TYPE options.",
    ""};

static char *hmxxcle[] = {
    "Syntax: CLEAR [ item-name ]",
    " ",
    "Clears the named item.  If no item is named, DEVICE-AND-INPUT is assumed.",
    " ",
    "  ALARM            Clears any pending alarm (see SET ALARM).",
#ifdef CK_APC
    "  APC-STATUS       Clears Application Program Command status.",
#endif /* CK_APC */
#ifdef PATTERNS
    "  BINARY-PATTERNS  Clears the file binary-patterns list.",
#endif /* PATTERNS */
    "  DEVICE           Clears the current port or network input buffer.",
    "  DEVICE-AND-INPUT Clears both the device and the INPUT buffer.",
    "  DIAL-STATUS      Clears the \\v(dialstatus) variable.",
    "  \
INPUT            Clears the INPUT-command buffer and the \\v(input) variable.",
    "  KEYBOARD-BUFFER  Clears the command terminal keyboard input buffer.",
    "  SEND-LIST        Clears the current SEND list (see ADD).",
#ifdef PATTERNS
    "  TEXT-PATTERNS    Clears the file text-patterns list.",
#endif /* PATTERNS */
    ""};
#endif /* NOFRILLS */

static char *hmxxdate[] = {
    "Syntax: DATE [ date-time [ timezone ] ] [ delta-time ]",
    "  Prints a date-time in standard format: yyyymmdd_hh:mm:ss.",
    "  Various date-time formats are accepted:",
    " ",
    "  . The date, if given, must precede the time.",
    "  . The year must be four digits or else a 2-digit format dd mmm yy,",
    "    in which case if (yy < 50) yyyy = yy + 2000; else yyyy = yy + 1900.",
    "  . If the year comes first, the second field is the month.",
    "  . The day, month, and year may be separated by spaces, /, -, or "
    "underscore.",
    "  . The date and time may be separated by spaces or underscore.",
    "  . The month may be numeric (1 = January) or spelled out or abbreviated "
    "in",
    "    English.",
    "  . The time may be in 24-hour format or 12-hour format.",
    "  . If the hour is 12 or less, AM is assumed unless AM or PM is included.",
    "  . If the date is omitted but a time is given, the current date is "
    "supplied.",
    "  . If the time is given but date omitted, 00:00:00 is supplied.",
    "  . If both the date and time are omitted, the current date and time are",
    "    supplied.",
    " ",
    "  The following shortcuts can also be used in place of dates:",
    " ",
    "  NOW",
    "    Stands for the current date and time.",
    " ",
    "  TODAY",
    "    Today's date, optionally followed by a time; 00:00:00 if no time "
    "given.",
    " ",
    "  YESTERDAY",
    "    Yesterday's date, optionally followed by a time (default 00:00:00).",
    " ",
    "  TOMORROW",
    "    Tomorrows's date, optionally followed by a time (default 00:00:00).",
    " ",
    "  Timezone specifications are similar to those used in e-mail and HTTP",
    "    headers, either a USA timezone name, e.g. EST, or a signed four-digit",
    "    timezone offset, {+,-}hhmm, e.g., -0500; it is used to convert "
    "date-time,",
    "    a local time in that timezone, to GMT which is then converted to the",
    "    local time at the host.  If no timezone is given, the date-time is "
    "local.",
    "    To convert local time (or a time in a specified timezone) to UTC "
    "(GMT),",
    "    use the function \\futcdate().",
    " ",
    "  Delta times are given as {+,-}[number date-units][hh[:mm[:ss]]]",
    "    A date in the future/past relative to the date-time; date-units may "
    "be",
    "    DAYS, WEEKS, MONTHS, YEARS: +3days, -7weeks, +3:00, +1month 8:00.",
    " ",
    "All the formats shown above are acceptable as arguments to date-time "
    "switches",
    "such as /AFTER: or /BEFORE:, and to functions such as \\fcvtdate(),",
    "\\fdiffdate(), and \\futcdate(), that take date-time strings as "
    "arguments.",
    ""};

#ifndef NOXFER
static char *hmxxsen[] = {
    "Syntax: SEND (or S) [ switches...] [ filespec [ as-name ] ]",
    "  Sends the file or files specified by filespec.  If the filespec is "
    "omitted",
    "  the SEND-LIST is used (HELP ADD for more info).  The filespec may "
    "contain",
    "  wildcard characters.  An 'as-name' may be given to specify the name(s)",
    "  the files(s) are sent under; if the as-name is omitted, each file is",
    "  sent under its own name.  Also see HELP MSEND, HELP WILDCARD.",
    "  Optional switches include:",
    " ",
#ifndef NOSPL
    "/ARRAY:<arrayname>",
    "  Specifies that the data to be sent comes from the given array, such as",
    "  \\&a[].  A range may be specified, e.g. SEND /ARRAY:&a[100:199].  Leave",
    "  the brackets empty or omit them altogether to send the whole 1-based "
    "array.",
    "  Include /TEXT to have Kermit supply a line terminator at the end of "
    "each",
    "  array element (and translate character sets if character-set "
    "translations",
    "  are set up), or /BINARY to treat the array as one long string of "
    "characters",
    "  to be sent as-is.  If an as-name is not specified, the array is sent "
    "with",
    "  the name _ARRAY_X_, where \"X\" is replaced by actual array letter.",
    " ",
#endif /* NOSPL */

    "/AS-NAME:<text>",
    "  Specifies <text> as the name to send the file under instead of its real",
    "  name.  This is equivalent to giving an as-name after the filespec.",
    " ",
    "/BINARY",
    "  Performs this transfer in binary mode without affecting the global",
    "  transfer mode.",
    " ",
    "/TEXT",
    "  Performs this transfer in text mode without affecting the global",
    "  transfer mode.",
    " ",
    "/TRANSPARENT",
    "  Inhibits character-set translation for text files for the duration of",
    "  the SEND command without affecting subsequent commands.",
    " ",
    "/NOBACKUPFILES",
    "  Skip (don't send) Kermit or EMACS backup files (files with names that",
    "  end with .~n~, where n is a number).",
    " ",
    "/DOTFILES",
    "  Include (send) files whose names begin with \".\".",
    " ",
    "/NODOTFILES",
    "  Don't send files whose names begin with \".\".",
    " ",
    "/FOLLOWLINKS",
    "  Send files that are pointed to by symbolic links.",
    " ",
    "/NOFOLLOWLINKS",
    "  Skip over symbolic links (default).",
    " ",

#ifdef CK_LABELED
    "/LABELED",
    "  Performs this transfer in labeled mode without affecting the global",
    "  transfer mode.",
    " ",
#endif /* CK_LABELED */
    "/COMMAND",
    "  Sends the output from a command, rather than the contents of a file.",
    "  The first \"filename\" on the SEND command line is interpreted as the "
    "name",
    "  of a command; the second (if any) is the as-name.",
    " ",
    "/FILENAMES:{CONVERTED,LITERAL}",
    "  Overrides the global SET FILE NAMES setting for this transfer only.",
    " ",
    "/PATHNAMES:{OFF,ABSOLUTE,RELATIVE}",
    "  Overrides the global SET SEND PATHNAMES setting for this transfer.",
    " ",
    "/FILTER:command",
    "  Specifies a command \
(standard input/output filter) to pass the file through",
    "  before sending it.",
    " ",
    "/DELETE",
    "  Deletes the file (or each file in the group) after it has been sent",
    "  successfully (applies only to real files).",
    " ",
    "/QUIET",
    "  When sending in local mode, this suppresses the file-transfer display.",
    " ",
    "/RECOVER",
    "  Used to recover from a previously interrupted transfer; SEND /RECOVER",
    "  is equivalent RESEND (use in binary mode only).",
    " ",
    "/RECURSIVE",
    "  Tells C-Kermit to look not only in the given or current directory for",
    "  files that match the filespec, but also in all its subdirectories, and",
    "  all their subdirectories, etc.",
    " ",
    "/RENAME-TO:name",
    "  Tells C-Kermit to rename each source file that is sent successfully to",
    "  the given name (usually you should include \\v(filename) in the new "
    "name,",
    "  which is replaced by the original filename.",
    " ",
    "/MOVE-TO:directory",
    "  Tells C-Kermit to move each source file that is sent successfully to",
    "  the given directory.",
    " ",
    "/STARTING:number",
    "  Starts sending the file from the given byte position.",
    "  SEND /STARTING:n filename is equivalent to PSEND filename n.",
    " ",
    "/SUBJECT:text",
    "  Specifies the subject of an email message, to be used with /MAIL.  If "
    "the",
    "  text contains spaces, it must be enclosed in braces.",
    " ",
    "/MAIL:address",
    "  Sends the file as e-mail to the given address; use with /SUBJECT:.",
    " ",
    "/PRINT:options",
    "  Sends the file to be printed, with optional options for the printer.",
    " ",
#ifdef CK_XYZ
    "/PROTOCOL:name",
    "  Uses the given protocol to send the file (Kermit, Zmodem, etc) for this",
    "  transfer without changing global protocol.",
    " ",
#endif /* CK_XYZ */
    "/AFTER:date-time",
    "  Specifies that only those files modified after the given date-time are",
    "  to be sent.  HELP DATE for info about date-time formats.",
    " ",
    "/BEFORE:date-time",
    "  Specifies that only those files modified before the given date-time",
    "  are to be sent.",
    " ",
    "/NOT-AFTER:date-time",
    "  Specifies that only those files modified at or before the given "
    "date-time",
    "  are to be sent.",
    " ",
    "/NOT-BEFORE:date-time",
    "  Specifies that only those files modified at or after the given "
    "date-time",
    "  are to be sent.",
    " ",
    "/LARGER-THAN:number",
    "  Specifies that only those files longer than the given number of bytes "
    "are",
    "  to be sent.",
    " ",
    "/SMALLER-THAN:number",
    "  Specifies that only those files smaller than the given number of bytes "
    "are",
    "  to be sent.",
    " ",
    "/EXCEPT:pattern",
    "  Specifies that any files whose names match the pattern, which can be a",
    "  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
    "  are not to be sent.  To specify multiple patterns (up to 8), use outer",
    "  braces around the group, and inner braces around each pattern:",
    " ",
    "    /EXCEPT:{{pattern1}{pattern2}...}",
    " ",
    "/TYPE:{ALL,TEXT,BINARY}",
    "  Send only files of the given type (see SET FILE SCAN).",
    " ",
    "/LISTFILE:filename",
    "  Specifies the name of a file that contains the list of names of files",
    "  that are to be sent.  The filenames should be listed one name per line",
    "  in this file (but a name can contain wildcards).",
    " ",
    "Also see HELP RECEIVE, HELP GET, HELP SERVER, HELP REMOTE.",
    ""};

static char *hmxxrc[] = {
    "Syntax: RECEIVE (or R) [ switches... ] [ as-name ]",
    "  Wait for a file to arrive from the other Kermit, which must be given a",
    "  SEND command.  If the optional as-name is given, the incoming file or",
    "  files are stored under that name, otherwise it will be stored under",
#ifndef CK_TMPDIR
    "  the name it arrives with.",
#else
    "  the name it arrives with.  If the filespec denotes a directory, the",
    "  incoming file or files will be placed in that directory.",
#endif /* CK_TMPDIR */
    " ",
    "Optional switches include:",
    " ",
    "/AS-NAME:text",
    "  Specifies \"text\" as the name to store the incoming file under.",
    "  You can also specify the as-name as a filename on the command line.",
    " ",
    "/BINARY",
    "  Skips text-mode conversions unless the incoming file arrives with "
    "binary",
    "  attribute",
    " ",
    "/COMMAND",
    "  Receives the file into the standard input of a command, rather than "
    "saving",
    "  it on disk.  The /AS-NAME or the \"filename\" on the RECEIVE command "
    "line",
    "  is interpreted as the name of a command.",
    " ",
    "/EXCEPT:pattern",
    "  Specifies that any files whose names match the pattern, which can be a",
    "  regular filename, or may contain \"*\" and/or \"?\" metacharacters,",
    "  are to be refused.  To specify multiple patterns (up to 8), use outer",
    "  braces around the group, and inner braces around each pattern:",
    " ",
    "    /EXCEPT:{{pattern1}{pattern2}...}",
    " ",
    "/FILENAMES:{CONVERTED,LITERAL}",
    "  Overrides the global SET FILE NAMES setting for this transfer only.",
    " ",
    "/FILTER:command",
    "  Causes the incoming file to passed through the given command (standard",
    "  input/output filter) before being written to disk.",
    " ",
#ifdef CK_LABELED
    "/LABELED",
    "  Specifies labeled transfer mode.",
    " ",
#endif /* CK_LABELED */

    "/MOVE-TO:directory-name",
    "  Specifies that each file that arrives should be moved to the specified",
    "  directory after, and only if, it has been received successfully.",
    " ",
    "/PATHNAMES:{OFF,ABSOLUTE,RELATIVE,AUTO}",
    "  Overrides the global SET RECEIVE PATHNAMES setting for this transfer.",
    " ",
    "/PIPES:{ON,OFF}",
    "  Overrides the TRANSFER PIPES setting for this command only.  ON allows",
    "  reception of files with names like \"!tar xf -\" to be automatically",
    "  directed to a pipeline.",
    " ",
    "/PROTOCOL:name",
    "  Use the given protocol to receive the incoming file(s).",
    " ",
    "/QUIET",
    "  When receiving in local mode, this suppresses the file-transfer "
    "display.",
    " ",
    "/RECURSIVE",
    "  Equivalent to /PATHNAMES:RELATIVE.",
    " ",
    "/RENAME-TO:string",
    "  Specifies that each file that arrives should be renamed as specified",
    "  after, and only if, it has been received successfully.  The string "
    "should",
    "  normally contain variables like \\v(filename) or \\v(filenum).",
    " ",
    "/TEXT",
    "  Forces text-mode conversions unless the incoming file has the binary",
    "  attribute",
    " ",
    "/TRANSPARENT",
    "  Inhibits character-set translation of incoming text files for the "
    "duration",
    "  of the RECEIVE command without affecting subsequent commands.",
    " ",
    "Also see HELP SEND, HELP GET, HELP SERVER, HELP REMOTE.",
    ""};

#ifndef NORESEND
static char *hmxxrsen = "\
Syntax: RESEND filespec [name]\n\n\
  Resends the file or files, whose previous transfer was interrupted.\n\
  Picks up from where previous transfer left off, IF the receiver kept the\n\
  partially received file.  Works only for binary-mode transfers.\n\
  Requires file-transfer partner to support recovery.  Synonym: REPUT.";

static char *hmxxrget = "\
Syntax: REGET filespec\n\n\
  Ask a server to RESEND a file to C-Kermit.";

static char *hmxxpsen = "\
Syntax: PSEND filespec position [name]\n\n\
  Just like SEND, except sends the file starting at the given byte position.";
#endif /* NORESEND */

#ifndef NOMSEND
static char *hmxxmse[] = {
    "Syntax: MSEND [ switches... ] filespec [ filespec [ ... ] ]",
    "  Sends the files specified by the filespecs.  One or more filespecs may "
    "be",
    "  listed, separated by spaces.  Any or all filespecs may contain "
    "wildcards",
    "  and they may be in different directories.  Alternative names cannot be",
    "  given.  Switches include /BINARY /DELETE /MAIL /PROTOCOL /QUIET "
    "/RECOVER",
    "  /TEXT /TYPE; see HELP SEND for descriptions.",
    ""};
#endif /* NOMSEND */

static char *hmxxadd[] = {
#ifndef NOMSEND
    "ADD SEND-LIST filespec [ <mode> [ <as-name> ] ]",
    "  Adds the specified file or files to the current SEND list.  Use SHOW",
    "  SEND-LIST and CLEAR SEND-LIST to display and clear the list; use SEND",
    "  by itself to send the files from it.",
    " ",
#endif /* NOMSEND */
#ifdef PATTERNS
    "ADD BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Adds the pattern(s), if any, to the SET FILE BINARY-PATTERNS list.",
    " ",
    "ADD TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Adds the pattern(s), if any, to the SET FILE TEXT-PATTERNS list.",
    "  Use SHOW PATTERNS to see the lists.  See HELP SET FILE for further "
    "info.",
#endif /* PATTERNS */
    ""};

static char *hmxxremv[] = {
#ifdef PATTERNS
    "REMOVE BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Removes the pattern(s), if any, from the SET FILE BINARY-PATTERNS list",
    " ",
    "REMOVE TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Removes the given patterns from the SET FILE TEXT-PATTERNS list.",
    "  Use SHOW PATTERNS to see the lists.  See HELP SET FILE for further "
    "info.",
#endif /* PATTERNS */
    ""};
#endif /* NOXFER */

#ifndef NOSERVER
static char *hmxxser = "Syntax: SERVER\n\
  Enter server mode on the current connection.  All further commands\n\
  are taken in packet form from the other Kermit program.  Use FINISH,\n\
  BYE, or REMOTE EXIT to get C-Kermit out of server mode.";
#endif /* NOSERVER */

static char *hmhset[] = {
    "  The SET command establishes communication, file, scripting, or other",
    "  parameters.  The SHOW command can be used to display the values of",
    "  SET parameters.  Help is available for each individual parameter;",
    "  type HELP SET ? to see what's available.", ""};

#ifndef NOSETKEY
static char *hmhskey[] = {
    "Syntax: SET KEY k text",
    "Or:     SET KEY CLEAR",
    "  Configure the key whose \"scan code\" is k to send the given text when",
    "  pressed during CONNECT mode.  SET KEY CLEAR restores all the default",
    "  key mappings.  If there is no text, the default key binding is restored",
#ifndef NOCSETS
    "  for the key k.  SET KEY mappings take place before terminal "
    "character-set",
    "  translation.",
#else
    "  the key k.",
#endif /* NOCSETS */
    " ",
    "  To find out the scan code and mapping for a particular key, use the",
    "  SHOW KEY command.",
    ""};
#endif /* NOSETKEY */

static char *hmxychkt[] = {
    "Syntax: SET BLOCK-CHECK number",
    " ",
    "Type of block check to be used for error detection on file-transfer",
    "packets: 1, 2, 3, 4, or 5.  This command must be given to the file",
    "sender prior to the transfer.",
    " ",
    "Type 1 is standard and supported by all Kermit protocol implementations,",
    "  but it's only a 6-bit checksum, represented in a single printable ASCII",
    "  character.  It's fine for reliable connections (error-correcting "
    "modems,",
    "  TCP/IP, etc) but type 3 is recommended for connections where errors can",
    "  occur.",
    " ",
    "Type 2 is a 12-bit checksum represented in two printable characters.",
    " ",
    "Type 3 is a 16-bit cyclic redundancy check, the strongest error",
    "  detection method supported by Kermit protocol, represented in three",
    "  printable characters.",
    " ",
    "Type 4 (alias \"BLANK-FREE-2\") is a 12-bit checksum guaranteed to",
    "  contain no blanks in its representation; this is needed for connections",
    "  where trailing blanks are stripped from incoming lines of text.",
    " ",
    "Type 5 (alias \"FORCE-3\") means to force a Type 3 block check on",
    "  every packet, including the first packet, which normally has a type 1",
    "  block check.  This is for use in critical applications on noisy",
    "  connections.  As with types 2, 3, and 4, if the Kermit file",
    "  transfer partner does not support this type, the transfer fails",
    "  immediately at the beginning of the transfer.",
    ""};

static char *hmxydeb[] = {
    "Syntax: SET DEBUG { SESSION, ON, OFF, TIMESTAMP, MESSAGES }",
    " ",
    "SET DEBUG ON",
#ifdef DEBUG
    "  Opens a debug log file named debug.log in the current directory.",
    "  Use LOG DEBUG if you want specify a different log file name or path.",
#else
    "  (Has no effect in this version of Kermit.)",
#endif /* DEBUG */
    " ",
    "SET DEBUG OFF",
    "  Stops debug logging and session debugging.",
    " ",
    "SET DEBUG SESSION",
#ifndef NOLOCAL
    "  Displays control and 8-bit characters symbolically during CONNECT mode.",
    "  Equivalent to SET TERMINAL DEBUG ON.",
#else
    "  (Has no effect in this version of Kermit.)",
#endif /* NOLOCAL */
    " ",
    "SET DEBUG TIMESTAMP { ON, OFF }",
    "  Enables/Disables timestamps on debug log entries.",
    " ",
    "SET DEBUG MESSAGES { ON, OFF, STDERR } [C-Kermit 9.0]",
    "  Enables/Disables messages printed by the DEBUG command.",
    "  SET DEBUG OFF causes DEBUG messages not to be printed.",
    "  SET DEBUG ON sends DEBUG messages to standard output (stdout);",
    "  SET DEBUG STDERR sends DEBUG messages to standard error (stderr);",
    ""};

#ifdef CK_SPEED
static char *hmxyqctl[] = {
    "Syntax: SET CONTROL-CHARACTER { PREFIXED, UNPREFIXED } { <code>..., ALL }",
    " ",
    "  <code> is the numeric ASCII code for a control character "
    "1-31,127-159,255.",
    "  The word \"ALL\" means all characters in this range.",
    " ",
    "  PREFIXED <code> means the given control character must be converted to "
    "a",
    "  printable character and prefixed, the default for all control "
    "characters.",
    " ",
    "  UNPREFIXED <code> means you think it is safe to send the given control",
    "  character as-is, without a prefix.  USE THIS OPTION AT YOUR OWN RISK!",
    " ",
    "  SHOW CONTROL to see current settings.  SET CONTROL PREFIXED ALL is",
    "  recommended for safety.  You can include multiple <code> values in one",
    "  command, separated by spaces.",
    ""};
#endif /* CK_SPEED */

#ifndef NODIAL
static char *hxymodm[] = {
    "Syntax: SET MODEM <parameter> <value> ...",
    " ",
    "Note: Many of the SET MODEM parameters are configured automatically when",
    "you SET MODEM TYPE, according to the modem's capabilities.  SHOW MODEM to",
    "see them.  Also see HELP DIAL and HELP SET DIAL.",
    " ",
    "SET MODEM TYPE <name>",

    " Tells Kermit which kind of modem you have, so it can issue the",
    " appropriate modem-specific commands for configuration, dialing, and",
    " hanging up.  For a list of the modem types known to Kermit, type \"set",
    " modem type ?\".  The default modem type is GENERIC, which should work",
    " with any AT command-set modem that is configured for error correction,",
    " data compression, and hardware flow control.  Use SET MODEM TYPE NONE",
    " for direct serial, connections.  Use SET MODEM TYPE USER-DEFINED to use",
    " a type of modem that is not built in to Kermit, and then use SET MODEM",
    " CAPABILITIES, SET MODEM, DIAL-COMMAND, and SET MODEM COMMAND to tell",
    " Kermit how to configure and control it.",

    " ",

    "SET MODEM CAPABILITIES <list>",
    "  Use this command for changing Kermit's idea of your modem's "
    "capabilities,",
    "  for example, if your modem is supposed to have built-in error "
    "correction",
    "  but in fact does not.  Also use this command to define the capabilities",
    "  of a USER-DEFINED modem.  Capabilities are:",
    " ",
    "    AT      AT-commands",
    "    DC      data-compression",
    "    EC      error-correction",
    "    HWFC    hardware-flow",
    "    ITU     v25bis-commands",
    "    SWFC    software-flow",
    "    KS      kermit-spoof",
    "    SB      speed-buffering",
    "    TB      Telebit",
    " ",
    "SET MODEM CARRIER-WATCH { AUTO, ON, OFF }",
    "  Synonym for SET CARRIER-WATCH (q.v.)",
    " ",
    "SET MODEM COMPRESSION { ON, OFF }",
    "  Enables/disables the modem's data compression feature, if any.",
    " ",
    "SET MODEM DIAL-COMMAND <text>",
    "  The text replaces Kermit's built-in modem dialing command.  It must",
    "  include '%s' (percent s) as a place-holder for the telephone numbers",
    "  given in your DIAL commands.",
    " ",
    "SET MODEM ERROR-CORRECTION { ON, OFF }",
    "  Enables/disables the modem's error-correction feature, if any.",
    " ",
    "SET MODEM ESCAPE-CHARACTER number",
    "  Numeric ASCII value of modem's escape character, e.g. 43 for '+'.",
    "  For Hayes-compatible modems, Kermit uses three copies, e.g. \"+++\".",
    " ",
    "SET MODEM FLOW-CONTROL {AUTO, NONE, RTS/CTS, XON/XOFF}",
    "  Selects the type of local flow control to be used by the modem.",
    " ",
    "SET MODEM HANGUP-METHOD { MODEM-COMMAND, RS232-SIGNAL, DTR }",
    "  How hangup operations should be done.  MODEM-COMMAND means try to",
    "  escape back to the modem's command processor and give a modem-specific",
    "  hangup command.  RS232-SIGNAL means turn off the DTR signal.  DTR is a",
    "  synonym for RS232-SIGNAL.",
    " ",
    "SET MODEM KERMIT-SPOOF {ON, OFF}",
    "  If the selected modem type supports the Kermit protocol directly,",
    "  use this command to turn its Kermit protocol function on or off.",
    " ",
    "SET MODEM MAXIMUM-SPEED <number>",
    "  Specify the maximum interface speed for the modem.",
    " ",
    "SET MODEM NAME <text>",
    "  Descriptive name for a USER-DEFINED modem.",
    " ",
    "SET MODEM SPEAKER {ON, OFF}",
    "  Turns the modem's speaker on or off during dialing.",
    " ",
    "SET MODEM SPEED-MATCHING {ON, OFF}",
    "  ON means that C-Kermit changes its serial interface speed to agree with",
    "  the speed reported by the modem's CONNECT message, if any.  OFF means",
    "  Kermit should not change its interface speed.",
    " ",
    "SET MODEM VOLUME {LOW, MEDIUM, HIGH}",
    "  Selects the desired modem speaker volume for when the speaker is ON.",
    " ",
    "SET MODEM COMMAND commands are used to override built-in modem commands "
    "for",
    "each modem type, or to fill in commands for the USER-DEFINED modem type.",
    "Omitting the optional [ text ] restores the built-in modem-specific "
    "command,",
    "if any:",
    " ",
    "SET MODEM COMMAND AUTOANSWER {ON, OFF} [ text ]",
    "  Modem commands to turn autoanswer on and off.",
    " ",
    "SET MODEM COMMAND COMPRESSION {ON, OFF} [ text ]",
    "  Modem commands to turn data compression on and off.",
    " ",
    "SET MODEM COMMAND ERROR-CORRECTION {ON, OFF} [ text ]",
    "  Modem commands to turn error correction on and off.",
    " ",
    "SET MODEM COMMAND HANGUP [ text ]",
    "  Command that tells the modem to hang up the connection.",
    " ",
    "SET MODEM COMMAND IGNORE-DIALTONE [ text ]",
    "  Command that tells the modem not to wait for dialtone before dialing.",
    " ",
    "SET MODEM COMMAND INIT-STRING [ text ]",
    "  The 'text' is a replacement for C-Kermit's built-in initialization "
    "command",
    "  for the modem.",
    " ",
    "SET MODEM COMMAND PREDIAL-INIT [ text ]",
    "  A second INIT-STRING that is to be sent to the modem just prior to \
dialing.",
    " ",
    "SET MODEM COMMAND HARDWARE-FLOW [ text ]",
    "  Modem command to enable hardware flow control (RTS/CTS) in the modem.",
    " ",
    "SET MODEM COMMAND SOFTWARE-FLOW [ text ]",
    "  Modem command to enable local software flow control (Xon/Xoff) in "
    "modem.",
    " ",
    "SET MODEM COMMAND SPEAKER { ON, OFF } [ text ]",
    "  Modem command to turn the modem's speaker on or off.",
    " ",
    "SET MODEM COMMAND NO-FLOW-CONTROL [ text ]",
    "  Modem command to disable local flow control in the modem.",
    " ",
    "SET MODEM COMMAND PULSE [ text ]",
    "  Modem command to select pulse dialing.",
    " ",
    "SET MODEM COMMAND TONE [ text ]",
    "  Modem command to select tone dialing.",
    " ",
    "SET MODEM COMMAND VOLUME { LOW, MEDIUM, HIGH } [ text ]",
    "  Modem command to set the modem's speaker volume.",
    ""};

static char *hmxydial[] = {
    "The SET DIAL command establishes or changes all parameters related to",
    "dialing the telephone.  Also see HELP DIAL and HELP SET MODEM.  Use SHOW",
    "DIAL to display all of the SET DIAL values.",
    " ",
    "SET DIAL COUNTRY-CODE <number>",
    "  Tells Kermit the telephonic country-code of the country you are dialing",
    "  from, so it can tell whether a portable-format phone number from your",
    "  dialing directory will result in a national or an international call.",
    "  Examples: 1 for USA, Canada, Puerto Rico, etc; 7 for Russia, 39 for "
    "Italy,",
    "  351 for Portugal, 47 for Norway, 44 for the UK, 972 for Israel, 81 for",
    "  Japan, ...",
    " ",
    "  If you have not already set your DIAL INTL-PREFIX and LD-PREFIX, then "
    "this",
    "  command sets default values for them: 011 and 1, respectively, for "
    "country",
    "  code 1; 00 and 0, respectively, for all other country codes.  If these "
    "are",
    "  not your true international and long-distance dialing prefixes, then "
    "you",
    "  should follow this command by DIAL INTL-PREFIX and LD-PREFIX to let "
    "Kermit",
    "  know what they really are.",
    " ",
    "SET DIAL AREA-CODE [ <number> ]",
    "  Tells Kermit the area or city code that you are dialing from, so it can",
    "  tell whether a portable-format phone number from the dialing directory "
    "is",
    "  local or long distance.  Be careful not to include your long-distance",
    "  dialing prefix as part of your area code; for example, the area code "
    "for",
    "  central London is 171, not 0171.",
    " ",
    "SET DIAL CONFIRMATION {ON, OFF}",
    "  Kermit does various transformations on a telephone number retrieved "
    "from",
    "  the dialing directory prior to dialing (use LOOKUP <name> to see them).",
    "  In case the result might be wrong, you can use SET DIAL CONFIRM ON to "
    "have",
    "  Kermit ask you if it is OK to dial the number, and if not, to let you "
    "type",
    "  in a replacement.",
    " ",
    "SET DIAL CONNECT { AUTO, ON, OFF }",
    "  Whether to CONNECT (enter terminal mode) automatically after "
    "successfully",
    "  dialing.  ON means to do this; OFF means not to.  AUTO (the default) "
    "means",
    "  do it if the DIAL command was given interactively, but don't do it if "
    "the",
    "  DIAL command was issued from a macro or command file.  If you specify "
    "ON",
    "  or AUTO, you may follow this by one of the keywords VERBOSE or QUIET, "
    "to",
    "  indicate whether the verbose 4-line 'Connecting...' message is to be",
    "  displayed if DIAL succeeds and Kermit goes into CONNECT mode.",
    " ",
    "SET DIAL CONVERT-DIRECTORY {ASK, ON, OFF}",
    "  The format of Kermit's dialing directory changed in version 5A(192).  "
    "This",
    "  command tells Kermit what to do when it encounters an old-style "
    "directory:",
    "  ASK you whether to convert it, or convert it automatically (ON), or "
    "leave",
    "  it alone (OFF).  Old-style directories can still be used without",
    "  conversion, but the parity and speed fields are ignored.",
    " ",
    "SET DIAL DIRECTORY [ filename [ filename [ filename [ ... ] ] ] ]",
    "  The name(s) of your dialing directory file(s).  If you do not supply "
    "any",
    "  filenames, the  dialing directory feature is disabled and all numbers "
    "are",
    "  dialed literally as given in the DIAL command.  If you supply more than",
    "  one directory, all of them are searched.",
    " ",
    "SET DIAL SORT {ON, OFF}",
    "  When multiple entries are obtained from your dialing directory, they "
    "are",
    "  sorted in \"cheapest-first\" order.  If this does not produce the "
    "desired",
    "  effect, SET DIAL SORT OFF to disable sorting, and the numbers will be",
    "  dialed in the order in which they were found.",
    " ",
    "SET DIAL DISPLAY {ON, OFF}",
    "  Whether to display dialing progress on the screen; default is OFF.",
    " ",
    "SET DIAL HANGUP {ON, OFF}",
    "  Whether to hang up the phone prior to dialing; default is ON.",
    " ",
    "SET DIAL IGNORE-DIALTONE {ON, OFF}",
    "  Whether to ignore dialtone when dialing; default is OFF.",
    " ",
#ifndef NOSPL
    "SET DIAL MACRO [ name ]",
    "  Specify the name of a macro to execute on every phone number dialed, "
    "just",
    "  prior to dialing it, in order to perform any last-minute alterations.",
    " ",
#endif /* NOSPL */
    "SET DIAL METHOD {AUTO, DEFAULT, TONE, PULSE}",
    "  Whether to use the modem's DEFAULT dialing method, or to force TONE or",
    "  PULSE dialing.  AUTO (the default) means to choose tone or pulse "
    "dialing",
    "  based on the country code.  (Also see SET DIAL TONE-COUNTRIES and SET "
    "DIAL",
    "  PULSE-COUNTRIES.)",
    " ",
    "SET DIAL PACING number",
    "  How many milliseconds to pause between sending each character to the "
    "modem",
    "  dialer.  The default is -1, meaning to use the number from the built-in",
    " modem database.",
    "  ",
    "SET DIAL PULSE-COUNTRIES [ cc [ cc [ ... ] ] ]",
    "  Sets the list of countries in which pulse dialing is required.  Each cc",
    "  is a country code.",
    " ",
    "SET DIAL TEST { ON, OFF }",
    "  OFF for normal dialing.  Set to ON to test dialing procedures without",
    "  actually dialing.",
    " ",
    "SET DIAL TONE-COUNTRIES [ cc [ cc [ ... ] ] ]",
    "  Sets the list of countries in which tone dialing is available.  Each cc",
    "  is a country code.",
    " ",
    "SET DIAL TIMEOUT number",
    "  How many seconds to wait for a dialed call to complete.  Use this "
    "command",
    "  to override the DIAL command's automatic timeout calculation.  A value",
    "  of 0 turns off this feature and returns to Kermit's automatic dial",
    "  timeout calculation.",
    " ",
    "SET DIAL RESTRICT { INTERNATIONAL, LOCAL, LONG-DISTANCE, NONE }",
    "  Prevents placing calls of the type indicated, or greater.  For example",
    "  SET DIAL RESTRICT LONG prevents placing of long-distance and "
    "international",
    "  calls.  If this command is not given, there are no restrictions.  "
    "Useful",
    "  when dialing a list of numbers fetched from a dialing directory.",
    " ",
    "SET DIAL RETRIES <number>",
    "  How many times to redial each number if the dialing result is busy or "
    "no",
    "  no answer, until the call is successfully answered.  The default is 0",
    "  because automatic redialing is illegal in some countries.",
    " ",
    "SET DIAL INTERVAL <number>",
    "  How many seconds to pause between automatic redial attempts; default "
    "10.",
    " ",
    "The following commands apply to all phone numbers, whether given "
    "literally",
    "or found in the dialing directory:",
    " ",
    "SET DIAL PREFIX [ text ]",
    "  Establish a prefix to be applied to all phone numbers that are dialed,",
    "  for example to disable call waiting.",
    " ",
    "SET DIAL SUFFIX [ text ]",
    "  Establish a suffix to be added after all phone numbers that are dialed.",
    " ",
    "The following commands apply only to portable-format numbers obtained "
    "from",
    "the dialing directory; i.e. numbers that start with a \"+\" sign and",
    "country code, followed by area code in parentheses, followed by the phone",
    "number.",
    " ",
    "SET DIAL LC-AREA-CODES [ <list> ]",
    "  Species a list of area codes to which dialing is local, i.e. does not",
    "  require the LD-PREFIX.  Up to 32 area codes may be listed, separated by",
    "  spaces.  Any area codes in this list will be included in the final dial",
    "  string so do not include your own area code if it should not be dialed.",
    " ",
    "SET DIAL LC-PREFIX [ <text> ]",
    "  Specifies a prefix to be applied to local calls made from portable "
    "dialing",
    "  directory entries.  Normally no prefix is used for local calls.",
    " ",
    "SET DIAL LC-SUFFIX [ <text> ]",
    "  Specifies a suffix to be applied to local calls made from portable "
    "dialing",
    "  directory entries.  Normally no suffix is used for local calls.",
    " ",
    "SET DIAL LD-PREFIX [ <text> ]",
    "  Your long-distance dialing prefix, to be used with portable dialing",
    "  directory entries that result in long-distance calls.",
    " ",
    "SET DIAL LD-SUFFIX [ <text> ]",
    "  Long-distance dialing suffix, if any, to be used with portable dialing",
    "  directory entries that result in long-distance calls.  This would "
    "normally",
    "  be used for appending a calling-card number to the phone number.",
    " ",
    "SET DIAL FORCE-LONG-DISTANCE { ON, OFF }",
    "  Whether to force long-distance dialing for calls that normally would be",
    "  local.  For use (e.g.) in France.",
    " ",
    "SET DIAL TOLL-FREE-AREA-CODE [ <number> [ <number> [ ... ] ] ]",
    "  Tells Kermit the toll-free area code(s) in your country.",
    " ",
    "SET DIAL TOLL-FREE-PREFIX [ <text> ]",
    "  You toll-free dialing prefix, in case it is different from your long-",
    "  distance dialing prefix.",
    " ",
    "SET DIAL INTL-PREFIX <text>",
    "  Your international dialing prefix, to be used with portable dialing",
    "  directory entries that result in international calls.",
    " ",
    "SET DIAL INTL-SUFFIX <text>",
    "  International dialing suffix, if any, to be used with portable dialing",
    "  directory entries that result in international calls.",
    " ",
    "SET DIAL PBX-OUTSIDE-PREFIX <text>",
    "  Use this to tell Kermit how to get an outside line when dialing from a",
    "  Private Branch Exchange (PBX).",
    " ",
    "SET DIAL PBX-EXCHANGE <text> [ <text> [ ... ] ]",
    "  If PBX-OUTSIDE-PREFIX is set, then you can use this command to tell "
    "Kermit",
    "  the leading digits of one or more local phone numbers that identify it "
    "as",
    "  being on your PBX, so it can make an internal call by deleting those "
    "digits",
    "  from the phone number.",
    " ",
    "SET DIAL PBX-INTERNAL-PREFIX <text>",
    "  If PBX-EXCHANGE is set, and Kermit determines from it that a call is",
    "  internal, then this prefix, if any, is added to the number prior to",
    "  \
dialing.  Use this if internal calls from your PBX require a special prefix.",
    ""};
#endif /* NODIAL */

static char *hmxyflo[] = {
    "Syntax: SET FLOW [ switch ] value",
    " ",
#ifndef NOLOCAL
    "  Selects the type of flow control to use during file transfer, terminal",
    "  connection, and script execution.",
#else
    "  Selects the type of flow control to use during file transfer.",
#endif /* NOLOCAL */
    " ",
    "  Switches let you associate a particular kind of flow control with each",
    "  kind of connection: /REMOTE, /MODEM, /DIRECT-SERIAL, /TCPIP, etc; type",
    "  \"set flow ?\" for a list of available switches.  Then whenever you "
    "make",
    "  a connection, the associated flow-control is chosen automatically.",
    "  The flow-control values are NONE, KEEP, XON/XOFF, and possibly RTS/CTS",
    "  and some others; again, type \"set flow ?\" for a list.  KEEP tells "
    "Kermit",
    "  not to try to change the current flow-control method for the "
    "connection.",
    " ",
    "  If you omit the switch and simply supply a value, this value becomes "
    "the",
    "  current flow control type, overriding any default value that might have",
    "  been chosen in your most recent SET LINE, SET PORT, or SET HOST, or "
    "other",
    "  connection-establishment command.",
    " ",
    "  Type SHOW FLOW-CONTROL to see the current defaults for each connection "
    "type",
    "  as well as the current connection type and flow-control setting.  SHOW",
    "  COMMUNICATIONS also shows the current flow-control setting.",
    ""};

static char *hmxyf[] = {
    "Syntax: SET FILE parameter value",
    " ",
    "Sets file-related parameters.  Use SHOW FILE to view them.  Also see SET",
    "(and SHOW) TRANSFER and PROTOCOL.",
    " ",
    "SET FILE TYPE { TEXT, BINARY }",
    "  How file contents are to be treated during file transfer in the absence",
    "  of any other indication.  TYPE can be TEXT for conversion of record "
    "format",
    "  and character set, which is usually needed when transferring text files",
    "  between unlike platforms (such as UNIX and Windows), or BINARY for no",
    "  conversion if TRANSFER MODE is MANUAL, which is not the default.  Use",
    "  BINARY with TRANSFER MODE MANUAL for executable programs or binary data "
    "or",
    "  whenever you wish to duplicate the original contents of the file, byte "
    "for",
    "  byte.  In most modern Kermit programs, the file sender informs the "
    "receiver",
    "  of the file type automatically.  However, when sending files from "
    "C-Kermit",
    "  to an ancient or non-Columbia Kermit implementation, you might need to "
    "set",
    "  the corresponding file type at the receiver as well.",
    " ",
    "  When TRANSFER MODE is AUTOMATIC (as it is by default), various "
    "automatic",
    "  methods (depending on the platform) are used to determine whether a "
    "file",
    "  is transferred in text or binary mode; these methods (which might "
    "include",
    "  content scan (see SET FILE SCAN below), filename pattern matching (SET "
    "FILE",
    "  PATTERNS), client/server \"kindred-spirit\" recognition, or source file",
    "  record format) supersede the FILE TYPE setting but can, themselves, be",
    "  superseded by including a /BINARY or /TEXT switch in the SEND, GET, or",
    "  RECEIVE command.",
    " ",
    "  When TRANSFER MODE is MANUAL, the automatic methods are skipped for "
    "sending",
    "  files; the FILE TYPE setting is used instead, which can be superseded "
    "on",
    "  a per-command basis with a /TEXT or /BINARY switch.",
    " ",

#ifndef NOXFER

    "SET FILE BYTESIZE { 7, 8 }",
    "  Normally 8.  If 7, Kermit truncates the 8th bit of all file bytes.",
    " ",
#ifndef NOCSETS
    "SET FILE CHARACTER-SET name",
    "  Tells the encoding of the local file, ASCII by default.",
    "  The names ITALIAN, PORTUGUESE, NORWEGIAN, etc, refer to 7-bit ISO-646",
    "  national character sets.  LATIN1 is the 8-bit ISO 8859-1 Latin Alphabet "
    "1",
    "  for Western European languages.",
    "  NEXT is the 8-bit character set of the NeXT workstation.",
    "  The CPnnn sets are for PCs.  MACINTOSH-LATIN is for the Macintosh.",
#ifndef NOLATIN2
    "  LATIN2 is ISO 8859-2 for Eastern European languages that are written "
    "with",
    "  Roman letters.  Mazovia is a PC code page used in Poland.",
#endif /* NOLATIN2 */
#ifdef CYRILLIC
    "  KOI-CYRILLIC, CYRILLIC-ISO, and CP866 are 8-bit Cyrillic character "
    "sets.",
    "  SHORT-KOI is a 7-bit ASCII coding for Cyrillic.  BULGARIA-PC is a PC "
    "code",
    "  page used in Bulgaria",
#endif /* CYRILLIC */
#ifdef HEBREW
    "  HEBREW-ISO is ISO 8859-8 Latin/Hebrew.  CP862 is the Hebrew PC code "
    "page.",
    "  HEBREW-7 is like ASCII with the lowercase letters replaced by Hebrew.",
#endif /* HEBREW */
#ifdef GREEK
    "  GREEK-ISO is ISO 8859-7 Latin/Greek.  CP869 is the Greek PC code page.",
    "  ELOT-927 is like ASCII with the lowercase letters replaced by Greek.",
#endif /* GREEK */
#ifdef KANJI
    "  JAPANESE-EUC, JIS7-KANJI, DEC-KANJI, and SHIFT-JIS-KANJI are Japanese",
    "  Kanji character sets.",
#endif /* KANJI */
#ifdef UNICODE
    "  UCS-2 is the 2-byte form of the Universal Character Set.",
    "  UTF-8 is the serialized form of the Universal Character Set.",
#endif /* UNICODE */
    "  Type SET FILE CHAR ? for a complete list of file character sets.",
    " ",
    "SET FILE DEFAULT 7BIT-CHARACTER-SET",
    "  When automatically switching among different kinds of files while "
    "sending",
    "  this tells the character set to be used for 7-bit text files.",
    " ",
    "SET FILE DEFAULT 8BIT-CHARACTER-SET",
    "  This tells the character set to be used for 8-bit text files when",
    "  switching automatically among different kinds of files.",
    " ",
#endif /* NOCSETS */

    "SET FILE COLLISION option",
    "  Tells what to do when a file arrives that has the same name as",
    "  an existing file.  The options are:",
    "   BACKUP (default) - Rename the old file to a new, unique name and store",
    "     the incoming file under the name it was sent with.",
    "   OVERWRITE - Overwrite (replace) the existing file; doesn't work for",
    "     a Kermit server unless you also tell it to ENABLE DELETE.",
    "   APPEND - Append the incoming file to the end of the existing file.",
    "   REJECT - Refuse and/or discard the incoming file (= DISCARD).",
    "   RENAME - Give the incoming file a unique name.",
    "   UPDATE - Accept the incoming file only if newer than the existing "
    "file.",
    " ",

    "SET FILE DESTINATION { DISK, PRINTER, SCREEN, NOWHERE }",
    "  DISK (default): Store incoming files on disk.",
    "  PRINTER:        Send incoming files to SET PRINTER device.",
    "  SCREEN:         Display incoming files on screen (local mode only).",
    "  NOWHERE:        Do not put incoming files anywhere (use for "
    "calibration).",
    " ",
    "SET FILE DISPLAY option",
    "  Selects the format of the file transfer display for local-mode file",
    "  transfer.  The choices are:",
    " ",
    "  BRIEF      A line per file, showing size, mode, status, and throughput.",
    "  SERIAL     One dot is printed for every K bytes transferred.",
    "  CRT        Numbers are continuously updated on a single screen line.",
    "             This format can be used on any video display terminal.",
#ifdef CK_CURSES
    "  FULLSCREEN A fully formatted 24x80 screen showing lots of information.",
    "             This requires a terminal or terminal emulator.",
#endif /* CK_CURSES */
    "  NONE       No file transfer display at all.",
    " ",

    "SET FILE DOWNLOAD-DIRECTORY [ <directory-name> ]",
    "  The directory into which all received files should be placed.  By "
    "default,",
    "  received files go into your current directory.",
    " ",
#endif /* NOXFER */

#ifdef CK_CTRLZ
    "SET FILE EOF { CTRL-Z, LENGTH }",
    "  End-Of-File detection method, normally LENGTH.  Applies only to "
    "text-mode",
    "  transfers.  When set to CTRL-Z, this makes the file sender treat the "
    "first",
    "  Ctrl-Z in the input file as the end of file (EOF), and it makes the "
    "file",
    "  receiver tack a Ctrl-Z onto the end of the output file if it does not",
    "  already end with Ctrl-Z.",
    " ",
#endif /* CK_CTRLZ */

    "SET FILE END-OF-LINE { CR, CRLF, LF }",
    "  Use this command to specify nonstandard line terminators for text "
    "files.",
    " ",

#ifndef NOXFER
    "SET FILE INCOMPLETE { AUTO, KEEP, DISCARD }",
    "  What to do with an incompletely received file: KEEP, DISCARD, or AUTO.",
    "  AUTO (the default) means DISCARD if transfer is in text mode, KEEP if "
    "it",
    "  is in binary mode.",
    " ",

#ifdef UNIX
#ifdef DYNAMIC
    "SET FILE LISTSIZE number",
    "  Changes the size of the internal wildcard expansion list.  Use SHOW "
    "FILE",
    "  to see the current size.  Use this command to increase the size if you "
    "get",
    "  a \"?Too many files\" error.  Also see SET FILE STRINGSPACE.",
    " ",
#endif /* DYNAMIC */
#endif /* UNIX */

    "SET FILE NAMES { CONVERTED, LITERAL }",
    "  File names are normally CONVERTED to \"common form\" during "
    "transmission",
    "  (e.g. lowercase to uppercase, extra periods changed to underscore, "
    "etc).",
    "  LITERAL means use filenames literally (useful between like systems).  "
    "Also",
    "  see SET SEND PATHNAMES and SET RECEIVE PATHNAMES.",
    " ",

#ifdef UNIX
    "SET FILE OUTPUT { { BUFFERED, UNBUFFERED } [ size ], BLOCKING, "
    "NONBLOCKING }",
    "  Lets you control the disk output buffer for incoming files.  Buffered",
    "  blocking writes are normal.  Nonblocking writes might be faster on some",
    "  systems but might also be risky, depending on the underlying file "
    "service.",
    "  Unbuffered writes might be useful in critical applications to ensure "
    "that",
    "  cached disk writes are not lost in a crash, but will probably also be",
    "  slower.  The optional size parameter after BUFFERED or UNBUFFERED lets "
    "you",
    "  change the disk output buffer size; this might make a difference in",
    "  performance.",
    " ",
#endif /* UNIX */

#ifdef PATTERNS
    "SET FILE PATTERNS { ON, OFF, AUTO }",
    "  ON means to use filename pattern lists to determine whether to send a "
    "file",
    "  in text or binary mode.  OFF means to send all files in the prevailing",
    "  mode.  AUTO (the default) is like ON if the other Kermit accepts "
    "Attribute",
    "  packets and like OFF otherwise.  FILE PATTERNS are used only if FILE "
    "SCAN",
    "  is OFF (see SET FILE SCAN).",
    " ",
    "SET FILE BINARY-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Zero or more filename patterns which, if matched, cause a file to be "
    "sent",
    "  in binary mode when FILE PATTERNS are ON.  HELP WILDCARDS for a "
    "description",
    "  of pattern syntax.  SHOW PATTERNS to see the current file pattern "
    "lists.",
    " ",
    "SET FILE TEXT-PATTERNS [ <pattern> [ <pattern> ... ] ]",
    "  Zero or more filename patterns which, if matched, cause a file to be "
    "sent",
    "  in text mode when FILE PATTERNS is ON; if a file does not match a text "
    "or",
    "  binary pattern, the prevailing SET FILE TYPE is used.",
    " ",
#endif /* PATTERNS */

    "SET FILE SCAN { ON [ size ], OFF }",
    "  If TRANSFER MODE is AUTOMATIC and FILE SCAN is ON (as it is by default)",
    "  Kermit peeks at the file's contents to see if it's text or binary.  Use",
    "  SET FILE SCAN OFF to disable file peeking, while still keeping TRANSFER",
    "  MODE automatic to allow name patterns and other methods.  The optional",
    "  size is the number of file bytes to scan, 49152 by default.  -1 means "
    "to",
    "  scan the whole file.  Also see SET FILE PATTERNS.",
    " ",

#ifdef UNIX
#ifdef DYNAMIC
    "SET FILE STRINGSPACE number",
    "  Changes the size (in bytes) of the internal buffer that holds lists of",
    "  filenames such as wildcard expansion lists.  Use SHOW FILE to see the",
    "  current size.  Use this command to increase the size if you get a",
    "  \"?String space exhausted\" error.  Also see SET FILE LISTSIZE.",
    " ",
#endif /* DYNAMIC */
#endif /* UNIX */

#ifdef UNICODE
    "SET FILE UCS BOM { ON, OFF }",
    "  Whether to write a Byte Order Mark when creating a UCS-2 file.",
    " ",
    "SET FILE UCS BYTE-ORDER { BIG-ENDIAN, LITTLE-ENDIAN }",
    "  Byte order to use when creating UCS-2 files, and to use when reading "
    "UCS-2",
    "  files that do not start with a Byte Order Mark.",
    " ",
#endif /* UNICODE */

    "SET FILE WARNING { ON, OFF }",
    "  SET FILE WARNING is superseded by the newer command, SET FILE",
    "  COLLISION.  SET FILE WARNING ON is equivalent to SET FILE COLLISION "
    "RENAME",
    "  and SET FILE WARNING OFF is equivalent to SET FILE COLLISION OVERWRITE.",
#endif /* NOXFER */
    ""};

static char *hmxyhsh[] = {
    "Syntax: SET HANDSHAKE { NONE, XON, LF, BELL, ESC, CODE number }",
    "  Character to use for half duplex line turnaround handshake during file",
    "  transfer.  C-Kermit waits for this character from the other computer",
    "  before sending its next packet.  Default is NONE; you can give one of "
    "the",
    "  other names like BELL or ESC, or use SET HANDSHAKE CODE to specify the",
    "  numeric code value of the handshake character.  Type SET HANDSH ? for a",
    "  complete list of possibilities.",
    ""};

#ifndef NOSERVER
static char *hsetsrv[] = {
    "SET SERVER CD-MESSAGE {ON,OFF}",
    "  Tells whether the server, after successfully executing a REMOTE CD",
    "  command, should send the contents of the new directory's READ.ME",
    "  (or similar) file to your screen.",
    " ",
    "SET SERVER CD-MESSAGE FILE name",
    "  Tells the name of the file to be displayed as a CD-MESSAGE, such as",
    "  READ.ME (SHOW SERVER tells the current CD-MESSAGE FILE name).",
    "  To specify more than one filename to look for, use {{name1}{name2}..}.",
    "  Synonym: SET CD MESSAGE FILE <list>.",
    " ",
    "SET SERVER DISPLAY {ON,OFF}",
    "  Tells whether local-mode C-Kermit during server operation should put a",
    "  file transfer display on the screen.  Default is OFF.",
    " ",
    "SET SERVER GET-PATH [ directory [ directory [ ... ] ] ]",
    "  Tells the C-Kermit server where to look for files whose names it "
    "receives",
    "  from client GET commands when the names are not fully specified "
    "pathnames.",
    "  Default is no GET-PATH, so C-Kermit looks only in its current "
    "directory.",
    " ",
    "SET SERVER IDLE-TIMEOUT seconds",
    "  Idle time limit while in server mode, 0 for no limit.",
    "  NOTE: SERVER IDLE-TIMEOUT and SERVER TIMEOUT are mutually exclusive.",
    " ",
    "SET SERVER KEEPALIVE {ON,OFF}",
    "  Tells whether C-Kermit should send \"keepalive\" packets while "
    "executing",
    "  REMOTE HOST commands, which is useful in case the command takes a long",
    "  time to produce any output and therefore might cause the operation to "
    "time",
    "  out.  ON by default; turn it OFF if it causes trouble with the client "
    "or",
    "  slows down the server too much.",
    " ",
    "SET SERVER LOGIN [ username [ password [ account ] ] ]",
    "  Sets up a username and optional password which must be supplied before",
    "  the server will respond to any commands other than REMOTE LOGIN.  The",
    "  account is ignored.  If you enter SET SERVER LOGIN by itself, then "
    "login",
    "  is no longer required.  Only one SET SERVER LOGIN command can be in "
    "effect",
    "  at a time; C-Kermit does not support multiple user/password pairs.",
    " ",
    "SET SERVER TIMEOUT n",
    "  Server command wait timeout interval, how often the C-Kermit server "
    "issues",
    "  a NAK while waiting for a command packet.  Specify 0 for no NAKs at "
    "all.",
    "  Default is 0.",
    ""};
#endif /* NOSERVER */

static char *hmhrmt[] = {
#ifdef NEWFTP
    "The REMOTE command sends file management instructions or other commands",
    "to a Kermit or FTP server.  If you have a single connection, the command "
    "is",
    "directed to the server you are connected to; if you have multiple "
    "connections",
    "the command is directed according to your GET-PUT-REMOTE setting.",
#else
    "The REMOTE command sends file management instructions or other commands",
    "to a Kermit server.  There should already be a Kermit running in server",
    "mode on the other end of the connection.",
#endif /* NEWFTP */
    "Type REMOTE ? to see a list of available remote commands.  Type HELP "
    "REMOTE",
    "xxx to get further information about a particular remote command xxx.",
    " ",
    "All REMOTE commands except LOGIN and LOGOUT have R-command shortcuts;",
    "for example, RDIR for REMOTE DIR, RCD for REMOTE CD, etc.",
    " ",
#ifdef NEWFTP
#ifdef LOCUS
    "Also see: HELP SET LOCUS, HELP FTP, HELP SET GET-PUT-REMOTE.",
#else
    "Also see: HELP FTP, HELP SET GET-PUT-REMOTE.",
#endif /* LOCUS */
#else
#ifdef LOCUS
    "Also see: HELP SET LOCUS.",
#endif /* LOCUS */
#endif /* NEWFTP */
    ""};

#ifndef NOSPL
static char *ifhlp[] = {
    "Syntax: IF [NOT] condition commandlist",
    " ",
    "If the condition is (is not) true, do the commandlist.  The commandlist",
    "can be a single command, or a list of commands separated by commas or",
    "enclosed in braces.  The condition can be a single condition or a group "
    "of",
    "conditions separated by AND (&&) or OR (||) and enclosed in parentheses.",
    "If parentheses are used they must be surrounded by spaces.  Examples:",
    " ",
    "  IF EXIST oofa.txt <command>",
    "  IF ( EXIST oofa.txt || = \\v(nday) 3 ) <command>",
    "  IF ( EXIST oofa.txt || = \\v(nday) 3 ) { <command>, <command>, ... }",
    " ",
    "The conditions are:",
    " ",
    "  SUCCESS     - The previous command succeeded",
    "  OK          - Synonym for SUCCESS",
    "  FAILURE     - The previous command failed",
    "  ERROR       - Synonym for FAILURE",
    "  FLAG        - Succeeds if SET FLAG ON, fails if SET FLAG OFF",
    "  BACKGROUND  - C-Kermit is running in the background",
#ifdef CK_IFRO
    "  FOREGROUND  - C-Kermit is running in the foreground",
    "  REMOTE-ONLY - C-Kermit was started with the -R command-line option",
#else
    "  FOREGROUND  - C-Kermit is running in the foreground",
#endif /* CK_IFRO */
    "  KERBANG     - A Kerbang script is running",
    "  ALARM       - SET ALARM time has passed",
    "  ASKTIMEOUT  - The most recent ASK, ASKQ, GETC, or GETOK timed out",
    "  EMULATION   - Succeeds if executed while in CONNECT mode",
    " ",
    "  MS-KERMIT   - Program is MS-DOS Kermit",
    "  C-KERMIT    - Program is C-Kermit",
    "  WINDOWS     - Program is Kermit 95",
    "  GUI         - Program runs in a GUI window",
    " ",
    "  AVAILABLE CRYPTO                  - Encryption is available",
    "  AVAILABLE KERBEROS4               - Kerberos 4 authentication is "
    "available",
    "  AVAILABLE KERBEROS5               - Kerberos 5 authentication is "
    "available",
    "  AVAILABLE NTLM                    - NTLM authentication is available",
    "  AVAILABLE SRP                     - SRP authentication is available",
    "  AVAILABLE SSL                     - SSL/TLS authentication is available",
    "  MATCH string pattern              - Succeeds if string matches pattern",
#ifdef CKFLOAT
    "  FLOAT number                      - Succeeds if floating-point number",
#endif /* CKFLOAT */
    "  COMMAND word                      - Succeeds if word is built-in "
    "command",
    "  DEFINED variablename or macroname - The named variable or macro is "
    "defined",
    "  DECLARED arrayname                - The named array is declared",
    "  NUMERIC variable or constant      - The variable or constant is numeric",
    "  FUNCTION name                     - The name is of a built-in function",
    "  EXIST filename                    - The named file exists",
    "  ABSOLUTE filename                 - The filename is absolute, not "
    "relative",
    "  BINARY filename                   - The file is a binary regular file",
    "  TEXT filename                     - The file is a text regular file",
#ifdef CK_TMPDIR
    "  DIRECTORY string                  - The string is the name of a "
    "directory",
#endif /* CK_TMPDIR */
#ifdef UNIX
    "  LINK string                       - The string is a symbolic link",
#endif /* UNIX */
    "  READABLE filename                 - Succeeds if the file is readable",
    "  WRITEABLE filename                - Succeeds if the file is writeable",
#ifdef ZFCDAT
    "  NEWER file1 file2                 - The 1st file is newer than the 2nd "
    "one",
#endif /* ZFCDAT */
    "  OPEN { READ-FILE,SESSION-LOG,...} - The given file or log is open",
#ifndef NOLOCAL
    "  OPEN CONNECTION                   - A connection is open",
#endif /* NOLOCAL */
    "  KBHIT                             - A key has been pressed",
    "  TRUE                              - always succeeds",
    "  FALSE                             - always fails",
    " ",
    "  VERSION - equivalent to \"if >= \\v(version) ...\"",
    " ",
    "  EQUAL s1 s2 - s1 and s2 (character strings or variables) are equal",
    "  LLT   s1 s2 - s1 is lexically (alphabetically) less than s2",
    "  LLE   s1 s2 - s1 is lexically less than or equal to s2",
    "  LGT   s1 s2 - s1 is lexically (alphabetically) greater than s2",
    "  LGE   s1 s2 - s1 is lexically greater than or equal to s2",
    "  NEQ   s1 s2 - s1 is not equal to s2",
    " ",
    "  =  n1 n2 - n1 and n2 (numbers or variables containing numbers) are "
    "equal",
    "  <  n1 n2 - n1 is arithmetically less than n2",
    "  <= n1 n2 - n1 is arithmetically less than or equal to n2",
    "  >  n1 n2 - n1 is arithmetically greater than n2",
    "  >= n1 n2 - n1 is arithmetically greater than or equal to n2",
    "  != n1 n2 - n1 is not equal to n2",
    " ",
    "  (number by itself) - fails if the number is 0, succeeds otherwise",
    " ",
    "  (variable name)    - If value numeric: succeeds if nonzero, fails if "
    "zero",
    "                       NOTE: variable name must not be the same as "
    "keyword",
    " ",
    "The IF command may be followed on the next line by an ELSE command. "
    "Example:",
    " ",
    "  IF < \\%x 10 ECHO It's less",
    "  ELSE echo It's not less",
    " ",
    "It can also include an ELSE part on the same line if braces are used:",
    " ",
    "  IF < \\%x 10 { ECHO It's less } ELSE { ECHO It's not less }",
    " ",
    "Multiple commands can be enclosed in braces, separated by commas:",
    " ",
    "  IF > \\%n \\m(max) { echo \\%n > old max \\m(max), .max := \\%n }",
    " ",
    "When braces are used the command may split onto multiple lines:",
    " ",
    "  IF > \\%n \\m(max) {",
    "      echo \"\\%n greater than old max \\m(max)\"",
    "      .max := \\%n",
    "  } else if < \\%n \\m(min) {",
    "      echo \"\\%n less than old min \\m(min)\"",
    "      .min := \\%n",
    "  }",
    " ",
    "Also see HELP WILDCARD (for IF MATCH pattern syntax).",
    ""};

static char *hmxxeval[] = {
    "Syntax: EVALUATE variable expression",
    "  Evaluates the expression and assigns its value to the given variable.",
    "  The expression can contain numbers and/or numeric-valued variables or",
    "  functions, combined with mathematical operators and parentheses in",
    "  traditional notation.  Operators include +-/*(), etc.  Example:",
    "  EVALUATE \\%n (1+1) * (\\%a / 3).",
    " ",
    "  NOTE: Prior to C-Kermit 7.0, the syntax was \"EVALUATE expression\"",
    "  (no variable), and the result was printed.  Use SET EVAL { OLD, NEW }",
    "  to choose the old or new behavior, which is NEW by default.",
    " ",
    "Alse see: HELP FUNCTION EVAL.",
    ""};
#endif /* NOSPL */

static char *hmxxexit[] = {
    "Syntax: EXIT (or QUIT) [ number [ text ] ]",
    "  Exits from the Kermit program, closing all open files and devices.",
    "  If a number is given it becomes Kermit's exit status code.  If text is",
    "  included, it is printed.  Also see SET EXIT.", ""};

#ifndef NOSPL
static char *ifxhlp[] = {"\
Syntax: XIF condition { commandlist } [ ELSE { commandlist } ]",
                         "  Obsolete.  Same as IF (see HELP IF).", ""};

static char *forhlp[] = {
    "\
Syntax: FOR variablename initial-value final-value increment { commandlist }",
    "  FOR loop.  Execute the comma-separated commands in the commandlist the",
    "  number of times given by the initial value, final value and increment.",
    "  Example:  FOR \\%i 10 1 -1 { pause 1, echo \\%i }", ""};

static char *whihlp[] = {
    "\
Syntax: WHILE condition { commandlist }",
    "  WHILE loop.  Execute the comma-separated commands in the bracketed",
    "  commandlist while the condition is true.  Conditions are the same as "
    "for",
    "  IF commands.", ""};

static char *swihlp[] = {
    "Syntax: SWITCH <variable> { case-list }",
    "  Selects from a group of commands based on the value of a variable.",
    "  The case-list is a series of lines like these:",
    " ",
    "    :x, command, command, ..., break",
    " ",
    "  where \"x\" is a possible value for the variable.  At the end of the",
    "  case-list, you can put a \"default\" label to catch when the variable "
    "does",
    "  not match any of the labels:",
    " ",
    "    :default, command, command, ...",
    " ",
    "The case label \"x\" can be a character, a string, a variable, a function",
    "invocation, a pattern, or any combination of these.  See HELP WILDCARDS",
    "for information about patterns.",
    ""};

static char *openhlp[] = {
    "Syntax:  OPEN mode filename",
    "  For use with READ and WRITE commands.  Open the local file in the",
    "  specified mode: READ, WRITE, or APPEND.  !READ and !WRITE mean to read",
    "  from or write to a system command rather than a file.  Examples:",
    " ",
    "    OPEN READ oofa.txt",
    "    OPEN !READ sort foo.bar",
    ""};

static char *hxxask[] = {
    "Syntax:  ASK [ switches ] variablename [ prompt ]",
    "Example: ASK \\%n { What is your name\\? }",
    "  Issues the prompt and defines the variable to be whatever is typed in",
    "  response, up to the terminating carriage return.  Use braces to "
    "preserve",
    "  leading and/or trailing spaces in the prompt.",
    " ",
    "Syntax:  ASKQ [ switches ] variablename [ prompt ]",
    "Example: ASKQ \\%p { Password:}",
    "  Like ASK except the response does not echo on the screen or, if "
    "specified",
    "  it echoes as asterisks or other specified character.",
    " ",
    "Switches:",
    " /DEFAULT:text",
    "  Text to supply if the user enters a blank response or the /TIMEOUT",
    "  limit expired with no response.",
    " ",
    " /ECHO:char",
    "  (ASKQ only) Character to be echoed each time the user presses a key",
    "  corresponding to a printable character.  This lets users see what they "
    "are",
    "  doing when they are typing (e.g.) passwords, and makes editing easier.",
    " ",
    " /TIMEOUT:number",
    "  If the response is not entered within the given number of seconds, the",
    "  command fails.  This is equivalent to setting ASK-TIMER to a positive",
    "  number, except it applies only to this command.  Also see SET "
    "ASK-TIMER.",
    "  NOTE: If a /DEFAULT: value was also given, it is supplied automatically",
    "  upon timeout and the command does NOT fail.",

    " ",
    " /QUIET",
    "  Suppresses \"?Timed out\" message when /TIMEOUT is given and user "
    "doesn't",
    "  respond within the time limit.",
    ""};
static char *hxxgetc[] = {
    "Syntax:  GETC [ switches] [ variablename [ prompt ] ]",
    "Example: GETC \\%c { Type any character to continue...}",
    "  Issues the prompt and sets the variable to the first character you "
    "type.",
    "  Use braces to preserve leading and/or trailing spaces in the prompt.",
    " ", "Switches:", "  /CHECK",
    "    GETC /CHECK (no variable or prompt is given when /CHECK is used)",
    "   succeeds if characters are waiting to be read and fails if not.", " ",
    "  /QUIET", "    In case of errors, no error message is issued.", " ",
    "  /TIMEOUT:n",
    "    Gives GETC a time limit of n seconds to wait for a character to "
    "appear;",
    "    if no character appears within n seconds, GETC fails and (if a /QUIET",
    "    switch was not given, an error message is printed.",
    /* "Also see SET ASK-TIMER.", */
    ""};

static char *hmxytimer[] = {
    "Syntax: SET ASK-TIMER number",
    "  For use with ASK, ASKQ, GETOK, and GETC.  If ASK-TIMER is set to a "
    "number",
    "  greater than 0, these commands will time out after the given number of",
    "  seconds with no response.  This command is \"sticky\", so to revert to",
    " \
untimed ASKs after a timed one, use SET ASK-TIMER 0.  Also see IF ASKTIMEOUT.",
    ""};

static char *hxxdot[] = {
    "Syntax: .<variable-name> <assignment-operator> <value>",
    "  Assigns the value to the variable in the manner indicated by the",
    "  assignment operator:",
    "  =   Copies without evaluation (like DEFINE).",
    "  :=  Copies with evaluation (like ASSIGN).",
    "  ::= Copies with arithmetic evaluation (like EVALUATE).",
    ""};

static char *hxxdef[] = {
    "Syntax: DEFINE name [ definition ]",
    "  Defines a macro or variable.  Its value is the definition, taken",
    "  literally.  No expansion or evaluation of the definition is done.  Thus",
    "  if the definition includes any variable or function references, their",
    "  names are included, rather than their values (compare with ASSIGN).  If",
    "  the definition is omitted, then the named variable or macro is "
    "undefined.",
    "  If a variable of the same name already exists, its value is replaced by",
    "  the new value.",
    " ",
    "A typical macro definition looks like this:",
    " ",
    "  DEFINE name command, command, command, ...",
    " ",
    "for example:",
    " ",
    "  DEFINE vax set parity even, set duplex full, set flow xon/xoff",
    " ",
    "which defines a Kermit command macro called 'vax'.  The definition is a",
    "comma-separated list of Kermit commands.  Use the DO command to execute",
    "the macro, or just type its name, followed optionally by arguments.",
    " ",
    "The definition of a variable can be anything at all, for example:",
    " ",
    "  DEFINE \\%a Monday",
    "  DEFINE \\%b 3",
    " ",
    "These variables can be used almost anywhere, for example:",
    " ",
    "  ECHO Today is \\%a",
    "  SET BLOCK-CHECK \\%b",
    ""};

static char *hxxass[] = {
    "Syntax:  ASSIGN variablename string.",
    "Example: ASSIGN \\%a My name is \\%b.",
    "  Assigns the current value of the string to the variable (or macro).",
    "  The definition string is fully evaluated before it is assigned, so that",
    "  the values of any variables that are contained are used, rather than "
    "their",
    "  names.  Compare with DEFINE.  To illustrate the difference, try this:",
    " ",
    "    DEFINE \\%a hello",
    "    DEFINE \\%x \\%a",
    "    ASSIGN \\%y \\%a",
    "    DEFINE \\%a goodbye",
    "    ECHO \\%x \\%y",
    " ",
    "  This prints 'goodbye hello'.",
    ""};

static char *hxxdec[] = {
    "Syntax: DECREMENT variablename [ amount ]",
    "  Decrement (subtract one from) the value of a variable if the current "
    "value",
    "  is numeric.  If an optional amount is specified (as a number, a "
    "variable,",
    "  \
or an arithmetic expression that evaluates to a number, or any combination)",
    "  the variable is decremented by that number instead of 1.  The result is",
    "  always an integer.  If floating-point numbers are given, the result is",
    "  truncated.",
    " ",
    "Examples: DECR \\%a, DECR days 7, DECR x \\%n, DECR sum \\%x+12",
    ""};

static char *hxxinc[] = {
    "Syntax: INCREMENT variablename [ number ]",
    "  Increment (add one to) the value of a variable if the current value is",
    "  numeric.  If an optional amount is specified (as a number, a variable,",
    "  \
or an arithmetic expression that evaluates to a number, or any combination)",
    "  the variable is incremented by that number instead of 1.  The result is",
    "  always an integer.  If floating-point numbers are given, the result is",
    "  truncated.",
    " ",
    "Examples: INCR \\%a, INCR dollars 100, INCR size \\%n, INCR total \\%x/10",
    ""};
#endif /* NOSPL */

#ifdef ANYX25
#ifndef IBMX25
static char *hxxpad[] = {
    "Syntax: PAD command",
    "X.25 PAD commands:",
    " ",
    "    PAD CLEAR     - Clear the virtual call",
    "    PAD STATUS    - Return the status of virtual call",
    "    PAD RESET     - Send a reset packet",
    "    PAD INTERRUPT - Send an interrupt packet",
    ""};
#endif /* IBMX25 */

static char *hxyx25[] = {
    "Syntax: SET X.25 option { ON [ data ], OFF }",
    " ",
    "X.25 call options:",
    "  CLOSED-USER-GROUP { ON index, OFF }",
    "    Enable or disable closed user group call, where index is the group",
    "    index, 0 to 99.",
    "  REVERSE-CHARGE { ON, OFF }",
    "    Tell whether you want to reverse the charges for the call.",
    "  CALL-USER-DATA { ON string, OFF }",
    "    Specify call user-data for the X.25 call.",
    ""};
#endif /* ANYX25 */

static char *hxyprtr[] = {
#ifdef PRINTSWI
    "Syntax: SET PRINTER [ switches ] [ name ]",
    " ",
    "  Specifies the printer to be used for transparent-print, autoprint, and",
    "  screen-dump material during terminal emulation, as well as for the "
    "PRINT",
    "  command, plus various options governing print behavior.",
    " ",
    "Switches for specifying the printer by type:",
    " ",
    "/NONE",
    "  Include this switch to specify that all printer actions should simply "
    "be",
    "  skipped.  Use this, for example, if you have no printer.",
    " ",
    "/DOS-DEVICE[:name]",
    "  Include this to declare a DOS printer and to specify its name, such as",
    "  PRN, LPT1, etc.",
    " ",
    "/FILE[:name]",
    "  Specifies that all printer material is to be appended to the named "
    "file,",
    "  rather than being sent to a printer.  If the file does not exist, it is",
    "  created the first time any material is to be printed.",
    " ",
    "/PIPE[:name]",
    "  Specifies that all printer material is to be sent as standard input to",
    "  the program or command whose name is given.  Example:",
    " ",
    "    SET PRINTER /PIPE:{textps > lpt1}",
    " ",
    "If you give a printer name without specifying any of these switches, then "
    "it",
    "is assumed to be a DOS printer device or filename unless the name given",
    "(after removing enclosing braces, if any) starts with \"|\", \
in which case it",
    "is a pipe.  Examples:",
    " ",
    "  SET PRINTER LPT1               <-- DOS device",
    "  SET PRINTER {| textps > lpt1}  <-- Pipe",
    " ",
    "The next group of switches tells whether the printer is one-way or",
    "bidirectional (two-way):",
    " ",
    "/OUTPUT-ONLY",
    "  Include this to declare the printer capable only of receiving material "
    "to",
    "  be printed, but not sending anything back.  This is the normal kind of",
    "  printer, Kermit's default kind, and the opposite of /BIDIRECTIONAL.",
    " ",
    "/BIDIRECTIONAL",
    "  Include this to declare the printer bidirectional.  This is the "
    "opposite ",
    "  of /OUTPUT-ONLY.  You can also use this option with serial printers, "
    "even",
    "  if they aren't bidirectional, in case you need to specify speed, flow",
    "  control, or parity.",
    " ",
    "The next group applies only to bidirectional and/or serial printers:",
    " ",
    "/FLOW-CONTROL:{NONE,XON/XOFF,RTS/CTS,KEEP}",
    "  Flow control to use with a serial bidirectional printer, default KEEP;",
    "  i.e. use whatever the OS/2 driver for the port normally uses.",
    " ",
    "/PARITY:{NONE,EVEN,ODD,SPACE,MARK}",
    "  Parity to use with a serial printer, default NONE; i.e. use 8 data bits",
    "  and no parity.  If you omit the colon and the keyword, NONE is "
    "selected.",
    " ",
    "/SPEED:number",
    "  Interface speed, in bits per second, to use with a serial printer, such "
    "as",
    "  2400, 9600, 19200, etc.  Type SET PRINTER /SPEED:? for a list of "
    "possible",
    "  speeds.",
    " ",
    "The next group deals with print jobs -- how to identify them, how to "
    "start",
    "them, how to terminate them:",
    " ",
    "/TIMEOUT[:number]",
    "  Used with host-directed transparent or auto printing, this is the "
    "number",
    "  of seconds to wait after the host closes the printer before terminating",
    "  the print job if the printer is not opened again during the specified",
    "  amount of time.",
    " ",
    "/JOB-HEADER-FILE[:filename]",
    "  The name of a file to be sent to the printer at the beginning of each",
    "  print job, as a burst page, or to configure the printer.  Normally no "
    "file",
    "  is is sent.",
    " ",
    "/END-OF-JOB-STRING[:string]",
    "  String of characters to be sent to the printer at the end of the print",
    "  job, usually used to force the last or only page out of the printer.  "
    "When",
    "  such a string is needed, it usually consists of a single formfeed: "
    "\"set",
    "  printer /end-of-job:{\\12}\".  No end-of-job string is sent unless you",
    "  specify one with this option.  If the string contains any spaces or",
    "  control characters (even in backslash notation, as above), enclose it "
    "in",
    "  braces.",
    " ",
    "The next group is for use with printers that print only PostScript:",
    " ",
    "/POSTSCRIPT or /PS",
    "  Indicates that Kermit should convert all text to PostScript before "
    "sending",
    "  it to the printer.  The fixed-pitch Courier-11 font is used.",
    " ",
    "/WIDTH:number",
    "  Specifies the width of the page in characters.  If this switch is not",
    "  given, 80 is used.",
    " ",
    "/HEIGHT:number",
    "  Specifies the height of the page in lines.  If this switch is not given",
    "  66 is used.",
    " ",
    "/NOPOSTSCRIPT or /NOPS",
    "  Indicates that Kermit should not convert all text to PostScript before",
    "  sending it to the printer.",
    " ",
    "The final switch is for use with AutoPrint mode and Screen Dumps",
    " ",
    "/CHARACTER-SET:<character-set>",
    "  Specifies the character set used by the printer which may be different",
    "  from both the character set used by the host and by the local computer.",
    "  The default value is CP437.",
    " ",
    "SHOW PRINTER displays your current printer settings.",
#else
#ifdef UNIX
    "Syntax: SET PRINTER [ { |command, filename } ]",
    "  Specifies the command (such as \"|lpr\") or filename to be used by the",
    "  PRINT command.  If a filename is given, each PRINT command appends to "
    "the",
    "  given file.  If the SET PRINTER argument contains spaces, it must be",
    "  enclosed in braces, e.g. \"set printer {| lpr -Plaser}\". If the "
    "argument",
    "  is omitted the default value is restored.  SHOW PRINTER lists the "
    "current",
    "  printer.  See HELP PRINT for further info.",
#else
    "Sorry, SET PRINTER not available yet.",
#endif /* UNIX */
#endif /* PRINTSWI */
    ""};

static char *hxyexit[] = {
    "Syntax: SET EXIT HANGUP { ON, OFF }",
    "  When ON (which is the default), C-Kermit executes an implicit HANGUP "
    "and",
    "  CLOSE command on the communications device or connection when it exits.",
    "  When OFF, Kermit skips this sequence.",
    " ",
    "Syntax: SET EXIT MESSAGE { ON, OFF, STDERR }",
    "  Allows the text (if any) from an EXIT command (see HELP EXIT) to be",
    "  supressed (OFF), printed normally (ON, the default), or sent to STDERR.",
    " ",
    "Syntax: SET EXIT ON-DISCONNECT { ON, OFF }",
    "  When ON, C-Kermit EXITs automatically when a network connection",
    "  is terminated either by the host or by issuing a HANGUP command.",
    " ",
    "Syntax: SET EXIT STATUS number",
#ifdef NOSPL
    "  Set C-Kermit's program return code to the given number.",
#else
    "  Set C-Kermit's program return code to the given number, which can be a",
    "  constant, variable, function result, or arithmetic expression.",
#endif /* NOSPL */
    " ",
    "Syntax: SET EXIT WARNING { ON, OFF, ALWAYS }",
    "  When EXIT WARNING is ON, issue a warning message and ask for "
    "confirmation",
    "  before EXITing if a connection to another computer might still be open.",
    "  When EXIT WARNING is ALWAYS, confirmation is always requested.  When "
    "OFF",
    "  it is never requested.  The default is ON.",
    ""};

#ifndef NOSPL
static char *hxxpau[] = {
    "Syntax:  PAUSE [ { number-of-seconds, hh:mm:ss } ]",
    "Example: PAUSE 3  or  PAUSE 14:52:30",
    "  Do nothing for the specified number of seconds or until the given time "
    "of",
    "  day in 24-hour hh:mm:ss notation.  If the time of day is earlier than "
    "the",
    "  current time, it is assumed to be tomorrow.  If no argument given, one",
    "  second is used.  The pause can be interrupted by typing any character "
    "on",
    "  the keyboard unless SLEEP CANCELLATION is OFF.  If interrupted, PAUSE",
    "  fails, otherwise it succeeds.  Synonym: SLEEP.",
    ""};

static char *hxxmsl[] = {"Syntax:  MSLEEP [ number ]", "Example: MSLEEP 500",
                         "  Do nothing for the specified number of "
                         "milliseconds; if no number given,",
                         "  100 milliseconds.", ""};
#endif /* NOSPL */

#ifndef NOPUSH
extern int nopush;
static char *hxxshe[] = {
    "Syntax: !, @, RUN, PUSH, or SPAWN, optionally followed by a command.",
    "  Gives the command to the local operating system's command processor, "
    "and",
    "  displays the results on the screen.  If the command is omitted, enters "
    "the",
    "  system's command line interpreter or shell; exit from it (the command "
    "for",
    "  this is usually EXIT or QUIT or LOGOUT) to return to Kermit.",
    ""};
#endif /* NOPUSH */

#ifndef NOXMIT
static char *hxxxmit[] = {
    "Syntax: TRANSMIT [ switches ] filename",
    "  Sends the contents of a file, without any error checking or correction,",
    "  to the computer on the other end of your SET LINE or SET HOST "
    "connection",
    "  (or if C-Kermit is in remote mode, displays it on the screen).  The",
    "  filename is the name of a single file (no wildcards) to be sent or, if",
    "  the /PIPE switch is included, the name of a command whose output is to "
    "be",
    "  sent.",
    " ",
    "  The file is sent according to your current FILE TYPE setting (BINARY or",
    "  TEXT), which you can override with a /BINARY or /TEXT switch without",
    "  changing the global setting.  In text mode, it is sent a line at a "
    "time,",
    "  with carriage return at the end of each line (as if you were typing it "
    "at",
    "  your keyboard), and C-Kermit waits for a linefeed to echo before "
    "sending",
    "  the next line; use /NOWAIT to eliminate the feedback requirement.  In",
    "  binary mode, it is sent a character at a time, with no feedback "
    "required.",
    " ",
    "  Normally the transmitted material is echoed to your screen.  Use SET",
    "  TRANSMIT ECHO OFF or the /NOECHO switch to suppress echoing.  Note that",
    "  TRANSMIT /NOECHO /NOWAIT /BINARY is a special case, that more or less",
    "  blasts the file out at full speed.",
    " ",
#ifndef NOCSETS
    "  Character sets are translated according to your current FILE and "
    "TERMINAL",
    "  CHARACTER-SET settings when TRANSMIT is in text mode.  Include "
    "/TRANSPARENT",
    "  to disable character-set translation in text mode (/TRANSPARENT implies",
    "  /TEXT).",
    " ",
#endif /* NOCSETS */
    "  There can be no guarantee that the other computer will receive the file",
    "  correctly and completely.  Before you start the TRANSMIT command, you",
    "  must put the other computer in data collection mode, for example by",
    "  starting a text editor.  TRANSMIT may be interrupted by Ctrl-C.  "
    "Synonym:",
    "  XMIT.  See HELP SET TRANSMIT for further information.",
    ""};
#endif /* NOXMIT */

#ifndef NOCSETS
static char *hxxxla[] = {
    "Syntax: CONVERT file1 cs1 cs2 [ file2 ]",
    "Synonym: TRANSLATE",
    "  Converts file1 from the character set cs1 into the character set cs2",
    "  and stores the result in file2.  The character sets can be any of",
    "  C-Kermit's file character sets.  If file2 is omitted, the translation",
    "  is displayed on the screen.  An appropriate intermediate character-set",
    "  is chosen automatically, if necessary.  Synonym: XLATE.  Example:",
    " ",
    "    CONVERT lasagna.txt latin1 utf8 lasagna-utf8.txt",
    " ",
    "  Multiple files can be translated if file2 is a directory or device "
    "name,",
    "  rather than a filename, or if file2 is omitted.",
    ""};
#endif /* NOCSETS */

#ifndef NOSPL
static char *hxxwai[] = {
    "Syntax: WAIT { number-of-seconds, hh:mm:ss } [ <what> ]",
    " ",
    "Examples:",
    "  wait 5 cd cts",
    "  wait 23:59:59 cd",
    " ",
    "  Waits up to the given number of seconds or the given time of day for "
    "the",
    "  specified item or event, which can be FILE, the name(s) of one or more",
    "  modem signals, or nothing.  If nothing is specified, WAIT acts like "
    "SLEEP.",
    "  If one or more modem signal names are given, Kermit waits for the "
    "specified",
    "  modem signals to appear on the serial communication device.",
    "  Sets FAILURE if the signals do not appear in the given time or "
    "interrupted",
    "  from the keyboard during the waiting period.",
    " ",
    "Signals:",
    "  cd  = Carrier Detect;",
    "  dsr = Dataset Ready;",
    "  cts = Clear To Send;",
    "  ri  = Ring Indicate.",
    " ",
    "If you want Kermit to wait for a file event, then the syntax is:",
    " ",
    "  WAIT <time> FILE { CREATION, DELETION, MODIFICATION } <filename>",
    " ",
    "where <time> is as above, and <filename> is the name of a single file.",
    "Kermit waits up to the given amount of time for the specified event to "
    "occur",
    "with the specified file, succeeds if it does, fails if it doesn't.",
    ""};
#endif /* NOSPL */

static char *hxxwri[] = {
    "Syntax: WRITE name text",
    "  Writes the given text to the named log or file.  The text text may "
    "include",
    "  backslash codes, and is not terminated by a newline unless you include "
    "the",
    "  appropriate code.  The name parameter can be any of the following:",
    " ",
    "   DEBUG-LOG",
    "   ERROR (standard error)",
#ifndef NOSPL
    "   FILE (the OPEN WRITE, OPEN !WRITE, or OPEN APPEND file, see HELP OPEN)",
#endif /* NOSPL */
    "   PACKET-LOG",
    "   SCREEN (compare with ECHO)",
#ifndef NOLOCAL
    "   SESSION-LOG",
#endif /* NOLOCAL */
    "   TRANSACTION-LOG",
    ""};

#ifndef NODIAL
static char *hxxlook[] = {
    "Syntax: LOOKUP name",
    "  Looks up the given name in the dialing directory or directories, if "
    "any,",
    "  specified in the most recent SET DIAL DIRECTORY command.  Each matching",
    "  entry is shown, along with any transformations that would be applied to",
    "  portable-format entries based on your locale.  HELP DIAL, HELP SET DIAL",
    "  for further info.",
    ""};

static char *hxxansw[] = {
    "Syntax:  ANSWER [ <seconds> ]",
    "  Waits for a modem call to come in.  Prior SET MODEM TYPE and SET LINE",
    "  required.  If <seconds> is 0 or not specified, Kermit waits forever or",
    "  until interrupted, otherwise Kermit waits the given number of seconds.",
    "  The ANSWER command puts the modem in autoanswer mode.  Subsequent DIAL",
    "  commands will automatically put it (back) in originate mode.  SHOW "
    "MODEM,",
    "  HELP SET MODEM for more info.",
    ""};

static char *hxxdial[] = {
    "Syntax:  DIAL phonenumber",
    "Example: DIAL 7654321",
    "  \
Dials a number using an autodial modem.  First you must SET MODEM TYPE, then",
    "  SET LINE, then SET SPEED.  Then give the DIAL command, including the "
    "phone",
    "  number, for example:",
    " ",
    "   DIAL 7654321",
#ifdef NETCONN
    " ",
    "  If the modem is on a network modem server, SET HOST first, then SET "
    "MODEM",
    "  TYPE, then DIAL.",
#endif /* NETCONN */
    " ",
    "If you give the DIAL command interactively at the Kermit prompt, and the",
    "call is placed successfully, Kermit automatically enters CONNECT mode.",
    "If the DIAL command is given from a macro or command file, Kermit remains",
    "in command mode after the call is placed, successfully or not.  You can",
    "change this behavior with the SET DIAL CONNECT command.",
    " ",
    "If the phonenumber starts with a letter, and if you have used the SET "
    "DIAL",
    "DIRECTORY command to specify one or more dialing-directory files, Kermit",
    "looks it up in the given file(s); if it is found, the name is replaced by",
    "the number or numbers associated with the name.  If it is not found, the",
    "name is sent to the modem literally.",
    " ",
    "If the phonenumber starts with an equals sign (\"=\"), this forces the "
    "part",
    "after the = to be sent literally to the modem, even if it starts with a",
    "letter, without any directory lookup.",
    " ",
    "You can also give a list of phone numbers enclosed in braces, e.g:",
    " ",
    "  dial {{7654321}{8765432}{+1 (212 555-1212}}",
    " ",
    "(Each number is enclosed in braces and the entire list is also enclosed "
    "in",
    "braces.)  In this case, each number is tried until there is an answer.  "
    "The",
    "phone numbers in this kind of list can not be names of dialing directory",
    "entries.",
    " ",
    "A dialing directory is a plain text file, one entry per line:",
    " ",
    "  name  phonenumber  ;  comments",
    " ",
    "for example:",
    " ",
    "  work    9876543              ; This is a comment",
    "  e-mail  +1  (212) 555 4321   ; My electronic mailbox",
    "  germany +49 (511) 555 1234   ; Our branch in Hanover",
    " ",
    "If a phone number starts with +, then it must include country code and",
    "area code, and C-Kermit will try to handle these appropriately based on",
    "the current locale (HELP SET DIAL for further info); these are called",
    "PORTABLE entries.  If it does not start with +, it is dialed literally.",
    " ",
    "If more than one entry is found with the same name, Kermit dials all of",
    "them until the call is completed; if the entries are in portable format,",
    "Kermit dials them in cheap-to-expensive order: internal, then local, then",
    "long-distance, then international, based on its knowledge of your local",
    "country code and area code (see HELP SET DIAL).",
    " ",
    "Specify your dialing directory file(s) with the SET DIAL DIRECTORY "
    "command.",
    " ",
#ifdef NETCONN
    "See also SET DIAL, SET MODEM, SET LINE, SET HOST, SET SPEED, REDIAL, and",
    "PDIAL.",
#else
    "See also SET DIAL, SET MODEM, SET LINE, SET SPEED, PDIAL, and REDIAL.",
#endif /* NETCONN */
    ""};

#ifdef CK_TAPI
static char *hxxtapi[] = {
    "TAPI CONFIGURE-LINE <tapi-line>",
    "  Displays the TAPI Configure Line Dialog box and allows you to",
    "  alter the default configuration for the specified <tapi-line>.",
    " ",
    "TAPI DIALING-PROPERTIES",
    "  Displays the TAPI Dialing Properties (locations) Dialog box.  The",
    "  Dialing rules may be changed and locations created and deleted.",
    "  When the dialog box is closed, C-Kermit imports the current Dialing",
    "  Properties' Location into the Kermit DIAL command settings.",
    ""};

static char *hxytapi[] = {
    "SET TAPI LINE <tapi-line>",
    "  Opens a TAPI device for use by Kermit.",
    " ",
    "SET TAPI MODEM-DIALING {ON, [OFF]}",
    "  If TAPI MODEM-DIALING is OFF when SET TAPI LINE is issued, Kermit opens",
    "  the TAPI device directly as a \"raw port\".  The device is unavailable "
    "to",
    "  other applications and Kermit performs dialing functions using its",
    "  built-in dialing and modem databases.  If TAPI MODEM-DIALING is ON, "
    "TAPI",
    "  handles all dialing functions and the port may be shared with other",
    "  applications when a call is not active.  When TAPI MODEM-DIALING is "
    "OFF,",
    "  SET MODEM TYPE TAPI Kermit uses the TAPI modem commands imported from "
    "the",
    "  Windows Registry during the previous SET TAPI LINE call.",
    " ",
    "SET TAPI LOCATION <tapi-location>",
    "  Specifies the TAPI location to make current for the entire system.  The",
    "  <tapi-location>'s dialing properties are imported into Kermit's SET "
    "DIAL",
    "  command database.",
    " ",
    "SET TAPI PHONE-NUMBER-CONVERSIONS {ON, OFF, [AUTO]}",
    "  Controls whether the phone number conversions are performed by TAPI "
    "(ON)",
    "  or by Kermit (OFF), or according the type of port that was selected",
    "  (AUTO); AUTO is the default, and is equivalent to ON if the current",
    "  LINE/PORT is a TAPI device and TAPI MODEM-DIALING is ON, OFF otherwise.",
    " ",
    "SET TAPI MODEM-LIGHTS {[ON], OFF}",
    "  Displays a modem lights indicator on the Windows Taskbar.  Does nothing",
    "  in Windows NT 4.0.",
    " ",
    "SET TAPI MANUAL-DIALING {ON, [OFF]}",
    "  Displays a dialog box during dialing requesting that you manually dial "
    "the",
    "  phone before continuing.  Applies only when TAPI MODEM-DIALING is ON.",
    " ",
    "SET TAPI WAIT-FOR-CREDIT-CARD-TONE <seconds>",
    "  Some modems don't support the '$' (BONG) symbol during dialing, which",
    "  means \"wait for credit card tone before continuing.\"  If TAPI "
    "recognizes",
    "  the modem as one that does not support BONG, it replaces the '$' with",
    "  <seconds> worth of pauses.  The default is 8 seconds.  This command",
    "  applies only when TAPI MODEM-DIALING is ON",
    " ",
    "SET TAPI PRE-DIAL-TERMINAL {ON, [OFF]}",
    "SET TAPI POST-DIAL-TERMINAL {ON, [OFF]}",
    "  Displays a small terminal window that may be used to communicate with "
    "the",
    "  modem or the host prior to or immediately after dialing; applies only "
    "when",
    "  TAPI MODEM-DIALING is ON",
    " ",
    "SET TAPI INACTIVITY-TIMEOUT <minutes>",
    "  Specifies the number of minutes of inactivity that may go by before "
    "TAPI",
    "  disconnects the line.  The default is 0 which means disable this "
    "function.",
    "  Applies only when TAPI MODEM-DIALING is ON.",
    " ",
    "SET TAPI USE-WINDOWS-CONFIGURATION {ON, [OFF]}",
    "  Specifies whether the TAPI modem values for speed, parity, stop bits, "
    "flow",
    "  control, etc. are used in preference to the current values specified",
    "  within C-Kermit.",
    " ",
    ""};
#endif /* CK_TAPI */

#endif /* NODIAL */

#ifdef TNCODE
static char *hmxxiks[] = {
    "Syntax: IKS [ switches ] [ host [ service ] ]",
    "  Establishes a new connection to an Internet Kermit Service daemon.",
    "  Equivalent to SET NETWORK TYPE TCP/IP, SET HOST host KERMIT /TELNET,",
    "  IF SUCCESS CONNECT.  If host is omitted, the previous connection (if "
    "any)",
    "  is resumed.  Depending on how Kermit has been built switches may be",
    "  available to require a secure authentication method and bidirectional",
    "  encryption.  See HELP SET TELNET for more info.",
    " ",
    " /USERID:[<name>]",
    "   This switch is equivalent to SET LOGIN USERID <name> or SET TELNET",
    "   ENVIRONMENT USER <name>.  If a string is given, it sent to host during",
    "   Telnet negotiations; if this switch is given but the string is "
    "omitted,",
    "   no user ID is sent to the host.  If this switch is not given, your",
    "   current USERID value, \\v(userid), is sent.  When a userid is sent to "
    "the",
    "   host it is a request to login as the specified user.",
    " ",
    ""};

static char *hmxxtel[] = {
    "Syntax: TELNET [ switches ] [ host [ service ] ]",
    "  Equivalent to SET NETWORK TYPE TCP/IP, SET HOST host [ service ] "
    "/TELNET,",
    "  IF SUCCESS CONNECT.  If host is omitted, the previous connection (if "
    "any)",
    "  is resumed.  Depending on how Kermit has been built switches may be",
    "  available to require a secure authentication method and bidirectional",
    "  encryption.  See HELP SET TELNET for more info.",
    " ",
    " /USERID:[<name>]",
    "   This switch is equivalent to SET LOGIN USERID <name> or SET TELNET",
    "   ENVIRONMENT USER <name>.  If a string is given, it sent to host during",
    "   Telnet negotiations; if this switch is given but the string is "
    "omitted,",
    "   no user ID is sent to the host.  If this switch is not given, your",
    "   current USERID value, \\v(userid), is sent.  When a userid is sent to "
    "the",
    "   host it is a request to login as the specified user.",
    " ",
    ""};

static char *hxtopt[] = {
    "TELOPT { AO, AYT, BREAK, CANCEL, EC, EL, EOF, EOR, GA, IP, DMARK, NOP, "
    "SE,",
    "         SUSP, SB [ option ], DO [ option ], DONT [ option ],",
    "         WILL [ option ], WONT [option] }",
    "  This command lets you send all the Telnet protocol commands.  Note that",
    "  certain commands do not require a response, and therefore can be used "
    "as",
    "  nondestructive \"probes\" to see if the Telnet session is still open;",
    "  e.g.:",
    " ",
    "    set host xyzcorp.com",
    "    ...",
    "    telopt nop",
    "    telopt nop",
    "    if fail stop 1 Connection lost",
    " ",
    "  TELOPT NOP is sent twice because the failure of the connection will not",
    "  be detected until the second send is attempted.  This command is meant",
    "  primarily as a debugging tool for the expert user.",
    ""};
#endif /* TNCODE */

#endif /* NOHELP */

/*  D O H L P  --  Give a help message  */

int dohset(int);
#ifndef NOCMDL
int dohopts(void);
#endif /* NOCMDL */
#ifndef NOSPL
int dohfunc(int);
extern struct keytab fnctab[];
extern int nfuncs;
#endif /* NOSPL */

#ifndef NOSPL
static char *hxxdcl[] = {
    "Syntax: ARRAY verb operands...",
    " ",
    "Declares arrays and performs various operations on them.  Arrays have",
    "the following syntax:",
    " ",
    "  \\&a[n]",
    " ",
    "where \"a\" is a letter and n is a number or a variable with a numeric "
    "value",
    "or an arithmetic expression.  The value of an array element can be "
    "anything",
    "at all -- a number, a character, a string, a filename, etc.",
    " ",
    "The following ARRAY verbs are available:",
    " ",
    "[ ARRAY ] DECLARE arrayname[n] [ = initializers... ]",
    "  Declares an array of the given size, n.  The resulting array has n+1",
    "  elements, 0 through n.  Array elements can be used just like any other",
    "  variables.  Initial values can be given for elements 1, 2, ... by",
    "  including = followed by one or more values separated by spaces.  If you",
    "  omit the size, the array is sized according to the number of "
    "initializers;",
    "  if none are given the array is destroyed and undeclared if it already",
    "  existed.  The ARRAY keyword is optional.  Synonym: [ ARRAY ] DCL.",
    " ",
    "[ ARRAY ] UNDECLARE arrayname",
    "  Destroys and undeclares the given array.  Synonym: ARRAY DESTROY.",
    " ",
    "ARRAY SHOW [ arrayname ]",
    "  Displays the contents of the given array.  A range specifier can be",
    "  included to display a segment of the array, e.g. \"array show "
    "\\&a[1:24].\"",
    "  If the arrayname is omitted, all declared arrays are listed, but their",
    "  contents is not shown.  Synonym: SHOW ARRAY.",
    " ",
    "ARRAY CLEAR arrayname",
    "  Clears all elements of the array, i.e. sets them to empty values.",
    "  You may include a range specifier to clear a segment of the array "
    "rather",
    "  than the whole array, e.g. \"array clear \\%a[22:38]\"",
    " ",
    "ARRAY SET arrayname value",
    "  Sets all elements of the array to the given value.  You may specify a",
    "  range to set a segment of the array, e.g. \"array set \\%a[2:9] 0\"",
    " ",
    "ARRAY RESIZE arrayname number",
    "  Changes the size of the given array, which must already exist, to the",
    "  number given.  If the number is smaller than the current size, the "
    "extra",
    "  elements are discarded; if it is larger, new empty elements are added.",
    " ",
    "ARRAY COPY array1 array2",
    "  Copies array1 to array2.  If array2 has not been declared, it is "
    "created",
    "  automatically.  If it array2 does exist, array1 is copied INTO it, as",
    "  much as will fit.  Range specifiers may be given on one or both arrays.",
    " ",
    "ARRAY LINK array1 arra2",
    "  Makes array1 a link to array2.",
    " ",
    "[ ARRAY ] SORT [ switches ] array-name [ array2 ]",
    "  Sorts the given array lexically according to the switches.  Element 0 "
    "of",
    "  the array is excluded from sorting by default.  The ARRAY keyword is",
    "  optional.  If a second array name is given, that array is sorted "
    "according",
    "  to the first one.  Switches:",
    " ",
    "  /CASE:{ON,OFF}",
    "    If ON, alphabetic case matters; if OFF it is ignored.  If this switch "
    "is",
    "    omitted, the current SET CASE setting applies.",
    " ",
    "  /KEY:number",
    "    \
Position (1-based column number) at which comparisons begin, 1 by default.",
    " ",
    "  /NUMERIC",
    "    Specifies a numeric rather than lexical sort.",
    " ",
    "  /RANGE:low[:high]",
    "    The range of elements, low through high, to be sorted.  If this "
    "switch",
    "    is not given, elements 1 through the dimensioned size are sorted.  If",
    "    :high is omitted, the dimensioned size is used.  To include element 0 "
    "in",
    "    a sort, use /RANGE:0 (to sort the whole array) or /RANGE:0:n (to sort",
    "    elements 0 through n).  You can use a range specifier in the array "
    "name",
    "    instead of the /RANGE switch.",
    " ",
    "  /REVERSE",
    "    Sort in reverse order.  If this switch is not given, the array is "
    "sorted",
    "    in ascending order.",
    " ",
    "Various functions are available for array operations; see HELP FUNCTION "
    "for",
    "details.  These include \\fdimension(), \\farraylook(), \\ffiles(), \
\\fsplit(),",
    "and many more.",
    ""};
#endif /* NOSPL */

#ifdef ZCOPY
static char *hmxxcpy[] = {
    "Syntax: COPY [ switches ] file1 file2",
    "  Copies the source file (file1) to the destination file (file2).  If "
    "file2",
    "  is a directory, file1 can contain wildcards to denote a group of files "
    "to",
    "  be copied to the given directory.  Switches:",
    " ",
    "  /TOSCREEN",
    "    Displays the file on the screen rather than copying to another file.",
    " ",
    "  /LIST",
    "    Print the filenames and status while copying.  Synonyms: /LOG, "
    "/VERBOSE.",
    " ",
    "  /NOLIST",
    "    Copy silently (default). Synonyms: /NOLOG, /QUIET.",
    " ",
    "  /PRESERVE",
    "    Copy timestamp and permissions from source file to destination file.",
    " ",
    "  /OVERWRITE:{ALWAYS,NEVER,NEWER,OLDER}",
    "    When copying from one directory to another, this tells what to do "
    "when",
    "    the destination file already exists: overwrite it always; never; only "
    "if",
    "    the source file is newer; or only if the source file is older.",
    " ",
    "  /APPEND",
    "    Append the source file to the destination file.  In this case the "
    "source",
    "    file specification can contain wildcards, and all the matching source",
    "    files are appended, one after the other in alphabetical order by "
    "name,",
    "    to the destination file.",
    " ",
    "  /SWAP-BYTES",
    "    Swap bytes while copying (e.g. for converting between Big-Endian and",
    "    Little-Endian binary data formats).",
#ifndef NOSPL
    " ",
    "  /FROMB64",
    "    Convert from Base64 encoding while copying.",
    " ",
    "  /TOB64",
    "    Convert to Base64 encoding while copying.",
    " ",
    "  /INTERPRET",
    "    If the file contains Kermit backslash escapes like \\v(date), "
    "\\v(time),",
    "    \\%1, \\%2, \\m(fast), etc, they are interpreted in the new copy of "
    "the",
    "    file or in the screen version, if used in combination with /TOSCREEN.",
    "    This option is not compatible most of the other options.",
    "  ",
#endif /* NOSPL */
    ""};
#endif /* ZCOPY */

#ifndef NOFRILLS
static char *hmxxren[] = {
#ifdef LOCUS
    "  If LOCUS is REMOTE or LOCUS is AUTO and you have an FTP connection,",
    "  this command is equivalent to REMOTE RENAME (RREN).  Otherwise:",
    " ",
#endif /* LOCUS */
#ifndef OLDMOVE
    "Syntax: RENAME or MOVE or MV [ switches ] name1 [ name2 ]",
#else
    "Syntax: RENAME or MV [ switches ] name1 [ name2 ]",
#endif /* OLDMOVE */
    "  Renames the source file (name1) to the target name2.  If name2 is a",
    "  directory, name1 is allowed to contain wildcards, and the file(s) "
    "matching",
    "  name1 are moved to directory name2, subject to rules of the underlying",
    "  operating system regarding renaming across disk boundaries, etc. "
    "Switches:",
    " ",
    "  /LIST",
    "    Print the filenames and status while renaming.  Synonyms: /LOG, "
    "/VERBOSE",
    " ",
    "  /NOLIST",
    "    Rename silently (default). Synonyms: /NOLOG, /QUIET",
    " ",
    "  /COLLISION:{FAIL,SKIP,OVERWRITE}",
    "    Tells what to do if a file with the given (or derived) new name "
    "already",
    "    exists: fail (and stop without renaming any files); skip this file",
    "    without renaming it and go on to the next one, if any; or overwrite "
    "(the",
    "    existing file).  PROCEED is a synonym for SKIP.",
    " ",
    "  /SIMULATE",
    "    Show what the effects of the RENAME command would be without actually",
    "    renaming any files.",
    " ",
    "  When any of the following switches is given, name2 must either be the",
    "  the name of a directory, or else omitted, and name1 is allowed to "
    "contain",
    "  contain wildcards, allowing multiple files to be renamed at once. If "
    "name2",
    "  is given, then all files matching name1 are moved to the name2 "
    "directory",
    "  after being renamed.",
    " ",
    "  /LOWER:{ALL,UPPER}",
    "    Converts letters in the filename to lowercase.  ALL means to convert",
    "    all matching filenames, UPPER means to convert only those filenames",
    "    that contain no lowercase letters.  The switch argument can be "
    "omitted,",
    "    in which case ALL filenames are converted.",
    " ",
    "  /UPPER:{ALL,LOWER}",
    "    Converts letters in the filename to uppercase.  ALL means to convert",
    "    all matching filenames, LOWER means to convert only those filenames",
    "    that contain no uppercase letters.  As with /LOWER, ALL is the "
    "default",
    "    switch argument.",
    " ",
    "  /FIXSPACES:s",
    "    Replaces all spaces in each matching filename by the string s, which "
    "may",
    "    be empty, a single character, or a string of characters.  The default",
    "    replacement (if no argument is given) is underscore (_).",
    " ",
    "  /REPLACE:{{string1}{string2}{options}}",
    "    Replaces all or selected occurrences of string1 with string2 in the",
    "    matching filenames.  The braces are part of the command.  The options",
    "    string can contain the following characters:",
    "     A: String matching is case-sensitive.",
    "     a: String matching is case-insensitive.",
    "     ^: String matching is anchored to the beginning of the filename.",
    "     $: String matching is anchored to the end of the filename.",
    "     1: Only the first occurrence of the string (if any) will be changed.",
    "     2: Only the second occurrence, and so on for all digits up to 9.",
    "     -: (before a digit) Occurrences are counted from the right.",
    "     ~: (before occurrence) All occurences BUT the one given are changed.",
    " ",
    "  /CONVERT:cset1:cset2",
    "    Converts each matching filename from character-set 1 to character-set "
    "2.",
    "    Character sets are the same as for SET FILE CHARACTER-SET.",
    " ",
    "  Global values for /LIST and COLLISION can be set with SET RENAME and",
    "  displayed with SHOW RENAME.",
    ""};
#endif /* NOFRILLS */

static char *cmdlhlp[] = {
    "Command-line options are given after the program name in the system",
    "command that you use to start Kermit.  Example:",
    " ",
    " kermit -i -s oofa.exe",
    " ",
    "tells Kermit to send (-s) the file oofa.exe in binary (-i) mode.",
    " ",
    "Command-line options are case-sensitive; \"-s\" is different from \"-S\".",
    " ",
    "If any \"action options\" are included on the command line, Kermit exits",
    "after executing its command-line options.  If -S is included, or no "
    "action",
    "options were given, Kermit enters its interactive command parser and",
    "issues its prompt.",
    " ",
    "Command-line options are single characters preceded by dash (-).  Some",
    "require an \"argument,\" others do not.  If an argument contains spaces, "
    "it",
    "must be enclosed in doublequotes:",
    " ",
    " kermit -s \"filename with spaces\"",
    " ",
    "\
An option that does not require an argument can be bundled with other options:",
    " ",
    " kermit -Qis oofa.exe",
    " ",
    "Exceptions to the rules:",
    " ",
    " . If the first command-line option is a filename, Kermit executes "
    "commands",
    "   from the file.  Additional command-line options can follow the "
    "filename.",
    " ",
    " . The special option \"=\" (equal sign) or \"--\" (double hyphen) means "
    "to",
    "   treat the rest of the command line as data, rather than commands; this",
    "   data is placed in the argument vector array, \\&@[], along with the "
    "other",
    "   items on the command line, and also in the top-level \\%1..\\%9 "
    "variables.",
    " ",
#ifdef KERBANG
    " . A similar option \"+\" (plus sign) means: the name of a Kermit script",
    "   file follows.  This file is to be executed, and its name assigned to "
    "\\%0",
    "   and \\&_[0].  All subsequent command-line arguments are to be ignored "
    "by",
    "   Kermit but made available to the script as \\%1, \\%2, ..., as well as",
    "   in the argument-vector arrays.  The initialization file is not "
    "executed",
    "   automatically in this case.",
    " ",
#endif /* KERBANG */
    " . The -s option can accept multiple filenames, separated by spaces.",
    " ",
    " . the -j and -J options allow an optional second argument, the TCP port",
    "   name or number.",
    " ",
    "Type \"help options all\" to list all the command-line options.",
    "Type \"help option x\" to see the help message for option x.",
    " ",
    "Kermit also offers a selection of \"extended command-line\" options.",
    "These begin with two dashes, followed by a keyword, and then, if the "
    "option",
    "has arguments, a colon (:) or equal sign (=) followed by the argument.",
    "Unlike single-letter options, extended option keywords aren't case "
    "sensitive",
    "and they can be abbreviated to any length that still distinguishes them "
    "from",
    "other extended-option keywords.  Example:",
    " ",
    "  kermit --banner:oofa.txt",
    " ",
    "which designates the file oofa.txt to be printed upon startup, rather "
    "than",
    "the built-in banner (greeting) text.  To obtain a list of available",
    "extended options, type \"help extended-options ?\".  To get help about "
    "all",
    "extended options, type \"help extended-options\".  To get help about a",
    "particular extended option, type \"help extended-option xxx\", where "
    "\"xxx\"",
    "is the option keyword.",
    ""};

#ifndef NOHELP
#ifndef NOCMDL
int doxopts() {
  extern char *xopthlp[], *xarghlp[];
  extern struct keytab xargtab[];
  extern int nxargs;
  int i, x, y, n = 0;
#ifdef CK_TTGWSIZ
  /* Check whether window size changed */
  if (ttgwsiz() > 0) {
    if (tt_rows > 0 && tt_cols > 0) {
      cmd_rows = tt_rows;
      cmd_cols = tt_cols;
    }
  }
#endif /* CK_TTGWSIZ */
  y = cmkey(xargtab, nxargs, "Extended argument without the \"--\" prefix", "",
            xxstring);
  if (y == -3) {
    printf("\n");
    if ((x = cmcfm()) < 0)
      return (x);
    for (i = 0; i <= XA_MAX; i++) {
      if (xopthlp[i]) {
        printf("%s\n", xopthlp[i]);
        printf("   %s\n", xarghlp[i]);
        printf("\n");
        n += 3;
        if (n > (cmd_rows - 6)) {
          if (!askmore())
            return (0);
          else
            n = 0;
        }
      }
    }
    return (1);
  } else if (y < 0)
    return (y);
  if ((x = cmcfm()) < 0)
    return (x);
  printf("\n%s\n", xopthlp[y]);
  printf("   %s\n\n", xarghlp[y]);
  return (1);
}

int dohopts() {
  int i, j, n, x, y, z, all = 0, msg = 0;
  char *s;
  extern char *opthlp[], *arghlp[];
  extern char *xopthlp[], *xarghlp[];
  extern int optact[];
  if ((x = cmtxt("A command-line option character,\n\
or the word ALL, or carriage return for an overview",
                 "", &s, xxstring)) < 0)
    return (x);
  if (!*s)
    msg = 1;
  else if (!strcmp(s, "all") || (!strcmp(s, "ALL")))
    all = 1;
  else if (*s == '-') /* Be tolerant of leading hyphen */
    s++;
  if (!all && (int)strlen(s) > 1) {
    printf("?A single character, please, or carriage to list them all.\n");
    return (-9);
  }
  if (all) {
    y = 33;
    z = 127;
  } else {
    y = *s;
    z = (y == 0) ? 127 : y;
    if (y == 0)
      y = 33;
  }
#ifdef CK_TTGWSIZ
  /* Check whether window size changed */
  if (ttgwsiz() > 0) {
    if (tt_rows > 0 && tt_cols > 0) {
      cmd_rows = tt_rows;
      cmd_cols = tt_cols;
    }
  }
#endif /* CK_TTGWSIZ */
  printf("\n");
  for (i = 0, n = 1; msg != 0 && *cmdlhlp[i]; i++) {
    printf("%s\n", cmdlhlp[i]);
    if (++n > (cmd_rows - 3)) {
      if (!askmore())
        return (0);
      else
        n = 0;
    }
  }
  if (all) {
    printf("The following command-line options are available:\n\n");
    n += 2;
  }
  for (i = y; msg == 0 && i <= z; i++) {
    if (!opthlp[i])
      continue;
    if (arghlp[i]) { /* Option with arg */
      printf(" -%c <arg>%s\n", (char)i, (optact[i] ? " (action option)" : ""));

      printf("     %s\n", opthlp[i]);
      printf("     Argument: %s\n\n", arghlp[i]);
      x = 4;

      /* Prevent argument help that contains line breaks (such K95s -#)
       * from breaking paging */
      for (j = 0; arghlp[i][j] != '\0'; j++) {
        if (arghlp[i][j] == '\n') {
          n += 1;
        }
      }
    } else { /* Option without arg */
      printf(" -%c  %s%s\n", (char)i, opthlp[i],
             (optact[i] ? " (action option)" : ""));
      printf("     Argument: (none)\n\n");
      x = 3;
    }
    n += x;
    if (n > (cmd_rows - x - 1)) {
      if (!askmore())
        return (0);
      else
        n = 0;
    }
  }
  if (all) { /* Jeff, Jan 2003 */
    printf("\n");
    if (++n >= cmd_rows) {
      if (!askmore())
        return (0);
      else
        n = 0;
    }
    printf("The following extended options are available:\n");
    if (++n >= cmd_rows) {
      if (!askmore())
        return (0);
      else
        n = 0;
    }
    printf("\n");
    if (++n >= cmd_rows) {
      if (!askmore())
        return (0);
      else
        n = 0;
    }
    for (i = 0; i <= XA_MAX; i++) {
      if (xopthlp[i]) {
        printf("%s\n", xopthlp[i]);
        printf("   %s\n", xarghlp[i]);
        printf("\n");
        n += 3;
        if (n > (cmd_rows - 4)) {
          if (!askmore())
            return (0);
          else
            n = 0;
        }
      }
    }
  }
  return (1);
}
#endif /* NOCMDL */
#endif /* NOHELP */

#ifdef CKCHANNELIO
static char *hxxfile[] = {
    "Syntax: FILE <subcommand> [ switches ] <channel> [ <data> ]",
    "  Opens, closes, reads, writes, and manages local files.",
    " ",
    "The FILE commands are:",
    " ",
    "  FILE OPEN   (or FOPEN)   -- Open a local file.",
    "  FILE CLOSE  (or FCLOSE)  -- Close an open file.",
    "  FILE READ   (or FREAD)   -- Read data from an open file.",
    "  FILE WRITE  (or FWRITE)  -- Write data to an open file.",
    "  FILE LIST   (or FLIST)   -- List open files.",
    "  FILE STATUS (or FSTATUS) -- Show status of a channel.",
    "  FILE REWIND (or FREWIND) -- Rewind an open file",
    "  FILE COUNT  (or FCOUNT)  -- Count lines or bytes in an open file",
    "  FILE SEEK   (or FSEEK)   -- Seek to specified spot in an open file.",
    "  FILE FLUSH  (or FFLUSH)  -- Flush output buffers for an open file.",
    " ",
    "Type HELP FILE OPEN or HELP FOPEN for details about FILE OPEN;",
    "type HELP FILE CLOSE or HELP FCLOSE for details about FILE CLOSE, and so "
    "on.",
    " ",
    "The following variables are related to the FILE command:",
    " ",
    "  \\v(f_max)     -- Maximum number of files that can be open at once",
    "  \\v(f_error)   -- Completion code of most recent FILE command or "
    "function",
    "  \\v(f_count)   -- Result of most recent FILE COUNT command",
    " ",
    "The following functions are related to the FILE command:",
    " ",
    "  \\F_eof()      -- Check if channel is at EOF",
    "  \\F_pos()      -- Get channel read/write position (byte number)",
    "  \\F_line()     -- Get channel read/write position (line number)",
    "  \\F_handle()   -- Get file handle",
    "  \\F_status()   -- Get channel status",
    "  \\F_getchar()  -- Read character",
    "  \\F_getline()  -- Read line",
    "  \\F_getblock() -- Read block",
    "  \\F_putchar()  -- Write character",
    "  \\F_putline()  -- Write line",
    "  \\F_putblock() -- Write block",
    "  \\F_errmsg()   -- Error message from most recent FILE command or "
    "function",
    " ",
    "Type HELP <function-name> for information about each one.",
    ""};

static char *hxxf_op[] = {
    "Syntax: FILE OPEN [ switches ] <variable> <filename>",
    "  Opens the file indicated by <filename> in the mode indicated by the",
    "  switches, if any, or if no switches are included, in read-only mode, "
    "and",
    "  assigns a channel number for the file to the given variable.",
    "  Synonym: FOPEN.  Switches:",
    " ",
    "/READ",
    "  Open the file for reading.",
    " ",

#ifdef UNIX
    "/STDIN",
    "  Tells Kermit to read from Standard Input.  In this case you don't "
    "specify",
    "  a filename.",
    " ",
    "/STDOUT",
    "  Tells Kermit to write to Standard Output.  In this case you don't "
    "specify",
    "  a filename.",
    " ",
    "/STDERR",
    "  Tells Kermit to write to Standard Error.  In this case you don't "
    "specify",
    "  a filename.",
    " ",
#endif /* UNIX */

    "/WRITE",
    "  Open the file for writing.  If /READ was not also specified, this "
    "creates",
    "  a new file.  If /READ was specified, the existing file is preserved, "
    "but",
    "  writing is allowed.  In both cases, the read/write pointer is initially",
    "  at the beginning of the file.",
    " ",
    "/APPEND",
    "  If the file does not exist, create a new file and open it for writing.",
    "  If the file exists, open it for writing, but with the write pointer",
    "  positioned at the end.",
    " ",
    "/BINARY",
#ifdef UNIX
    "  This option is ignored in UNIX.",
#else
    "  This option is ignored on this platform.",
#endif /* UNIX */
    " ",
    "Switches can be combined in an way that makes sense and is supported by "
    "the",
    "underlying operating system.",
    ""};

static char *hxxf_cl[] = {
    "Syntax: FILE CLOSE <channel>",
    "  Closes the file on the given channel if it was open.",
    "  Also see HELP FILE OPEN.  Synonym: FCLOSE.", ""};

static char *hxxf_fl[] = {
    "Syntax: FILE FLUSH <channel>",
    "  Flushes output buffers on the given channel if it was open, forcing",
    "  all material previously written to be committed to disk.  Synonym: "
    "FFLUSH.",
    "  Also available as \\F_flush().", ""};

static char *hxxf_li[] = {
    "Syntax: FILE LIST",
    "  Lists the channel number, name, modes, and position of each file opened",
    "  with FILE OPEN.  Synonym: FLIST.", ""};

static char *hxxf_re[] = {
    "Syntax: FILE READ [ switches ] <channel> [ <variable> ]",
    "  Reads data from the file on the given channel number into the "
    "<variable>,",
    "  if one was given; if no variable was given, the result is printed on",
    "  the screen.  The variable should be a macro name rather than a \\%x",
    "  variable or array element if you want backslash characters in the file "
    "to",
    "  be taken literally.  Synonym: FREAD.  Switches:",
    " ",
    "/LINE",
    "  Specifies that a line of text is to be read.  A line is defined "
    "according",
    "  to the underlying operating system's text-file format.  For example, in",
    "  UNIX a line is a sequence of characters up to and including a linefeed.",
    "  The line terminator (if any) is removed before assigning the text to "
    "the",
    "  variable.  If no switches are included with the FILE READ command, "
    "/LINE",
    "  is assumed.",
    " ",
    "/SIZE:number",
    "  Specifies that the given number of bytes (characters) is to be read.",
    "  This gives a semblance of \"record i/o\" for files that do not "
    "necessarily",
    "  contain lines.  The resulting block of characters is assigned to the",
    "  variable without any editing.",
    " ",
    "/CHARACTER",
    "  Equivalent to /SIZE:1.  If FILE READ /CHAR succeeds but the <variable> "
    "is",
    "  empty, this indicates a NUL byte was read.",
    " ",
    "/TRIM",
    "  Trims trailing whitespace from the right when used with /LINE.  Ignored",
    "  if used with /CHAR or /SIZE.",
    " ",
    "/UNTABIFY",
    "  Tells Kermit to convert tabs to spaces (assuming tabs set every 8 "
    "spaces)",
    "  when used with /LINE.  Ignored if used with /CHAR or /SIZE.",
    " ",
    "Synonym: FREAD.",
    "Also available as \\F_getchar(), \\F_getline(), \\F_getblock().",
    ""};

static char *hxxf_rw[] = {
    "Syntax: FILE REWIND <channel>",
    "  If the channel is open, moves the read/write pointer to the beginning "
    "of",
    "  the file.  Equivalent to FILE SEEK <channel> 0.  Synonym: FREWIND.",
    "  Also available as \\F_rewind().", ""};

static char *hxxf_se[] = {
    "Syntax: FILE SEEK [ switches ] <channel> { [{+,-}]<number>, EOF }",
    "  Switches are /BYTE, /LINE, /RELATIVE, /ABSOLUTE, and /FIND:pattern.",
    "  Moves the file pointer for this file to the given position in the",
    "  file.  Subsequent FILE READs or WRITEs will take place at that "
    "position.",
    "  If neither the /RELATIVE nor /ABSOLUTE switch is given, an unsigned",
    "  <number> is absolute; a signed number is relative.  EOF means to move "
    "to",
    "  the end of the file.  If a /FIND: switch is included, Kermit seeks to "
    "the",
    "  specified spot (e.g. 0 for the beginning) and then begins searching "
    "line",
    "  by line for the first line that matches the given pattern.  To start",
    "  searching from the current file position specify a line number of +0.",
    "  To start searching from the line after the current one, use +1 (etc).",
    "  Synonym: FSEEK.",
    ""};

static char *hxxf_st[] = {
    "Syntax: FILE STATUS <channel>",
    "  If the channel is open, this command shows the name of the file, the",
    "  switches it was opened with, and the current read/write position.",
    "  Synonym: FSTATUS", ""};

static char *hxxf_co[] = {
    "Syntax: FILE COUNT [ { /BYTES, /LINES, /LIST, /NOLIST } ] <channel>",
    "  If the channel is open, this command prints the nubmer of bytes "
    "(default)",
    "  or lines in the file if at top level or if /LIST is included; if "
    "/NOLIST",
    "  is given, the result is not printed.  In all cases the result is "
    "assigned",
    "  to \\v(f_count).  Synonym: FCOUNT",
    ""};

static char *hxxf_wr[] = {
    "FILE WRITE [ switches ] <channel> <text>",
    "  Writes the given text to the file on the given channel number.  The "
    "<text>",
    "  can be literal text or a variable, or any combination.  If the text "
    "might",
    "  contain leading or trailing spaces, it must be enclosed in braces if "
    "you",
    "  want to preserve them.  Synonym: FWRITE.  Switches:",
    " ",
    "/LINE",
    "  Specifies that an appropriate line terminator is to be added to the",
    "  end of the <text>.  If no switches are included, /LINE is assumed.",
    " ",
    "/SIZE:number",
    "  Specifies that the given number of bytes (characters) is to be written.",
    "  If the given <text> is longer than the requested size, it is truncated;",
    "  if is shorter, it is padded according /LPAD and /RPAD switches.  "
    "Synonym:",
    "  /BLOCK.",
    " ",
    "/LPAD[:value]",
    "  If /SIZE was given, but the <text> is shorter than the requested size,",
    "  the text is padded on the left with sufficient copies of the character",
    "  whose ASCII value is given to write the given length.  If no value is",
    "  specified, 32 (the code for Space) is used.  The value can also be 0 to",
    "  write the indicated number of NUL bytes.  If /SIZE was not given, this",
    "  switch is ignored.",
    " ",
    "/RPAD[:value]",
    "  Like LPAD, but pads on the right.",
    " ",
    "/STRING",
    "  Specifies that the <text> is to be written as-is, with no terminator "
    "added.",
    " ",
    "/CHARACTER",
    "  Specifies that one character should be written.  If the <text> is empty "
    "or",
    "  not given, a NUL character is written; otherwise the first character of",
    "  <text> is given.",
    " ",
    "Synonym FWRITE.",
    "Also available as \\F_putchar(), \\F_putline(), \\F_putblock().",
    ""};

static int dohfile(int cx) {
  extern struct keytab fctab[];
  extern int nfctab;
  int x;
  if (cx == XXFILE) { /* FILE command was given */
    /* Get subcommand */
    if ((cx = cmkey(fctab, nfctab, "Operation", "", xxstring)) < 0) {
      if (cx == -3) {
        if ((x = cmcfm()) < 0)
          return (x);
        cx = XXFILE;
      } else
        return (cx);
    }
    if ((x = cmcfm()) < 0)
      return (x);
    switch (cx) {
    case FIL_CLS:
      cx = XXF_CL;
      break;
    case FIL_FLU:
      cx = XXF_FL;
      break;
    case FIL_LIS:
      cx = XXF_LI;
      break;
    case FIL_OPN:
      cx = XXF_OP;
      break;
    case FIL_REA:
      cx = XXF_RE;
      break;
    case FIL_REW:
      cx = XXF_RW;
      break;
    case FIL_SEE:
      cx = XXF_SE;
      break;
    case FIL_STA:
      cx = XXF_ST;
      break;
    case FIL_WRI:
      cx = XXF_WR;
      break;
    case FIL_COU:
      cx = XXF_CO;
      break;
    }
  }
  switch (cx) {
  case XXFILE:
    return (hmsga(hxxfile));
  case XXF_CL:
    return (hmsga(hxxf_cl));
  case XXF_FL:
    return (hmsga(hxxf_fl));
  case XXF_LI:
    return (hmsga(hxxf_li));
  case XXF_OP:
    return (hmsga(hxxf_op));
  case XXF_RE:
    return (hmsga(hxxf_re));
  case XXF_RW:
    return (hmsga(hxxf_rw));
  case XXF_SE:
    return (hmsga(hxxf_se));
  case XXF_ST:
    return (hmsga(hxxf_st));
  case XXF_WR:
    return (hmsga(hxxf_wr));
  case XXF_CO:
    return (hmsga(hxxf_co));
  default:
    return (-2);
  }
}
#endif /* CKCHANNELIO */

int dohlp(int xx) {
  int x, y;

  debug(F101, "DOHELP xx", "", xx);
  if (xx < 0) {
    printf("Sorry, that is not a help topic\n");
    return (-9);
  };

#ifdef NOHELP
  if ((x = cmcfm()) < 0)
    return (x);
  printf("\n%s, Copyright (C) 1985, %s,", versio, ck_cryear);
#ifndef NOIKSD
  if (inserver)
    return (hmsga(tophlpi));
  else
#endif /* IKSD */
    return (hmsga(tophlp));

#else /* help is available */

  if (helpfile)
    return (dotype(helpfile, xaskmore, 0, 0, NULL, 0, NULL, 0, 0, NULL, 0));

#ifdef CKCHANNELIO
  if (xx == XXFILE)
    return (dohfile(xx));
  else if (xx == XXF_RE || xx == XXF_WR || xx == XXF_OP || xx == XXF_CL ||
           xx == XXF_SE || xx == XXF_RW || xx == XXF_FL || xx == XXF_LI ||
           xx == XXF_ST || xx == XXF_CO)
    return (dohfile(xx));
#endif /* CKCHANNELIO */

  switch (xx) {

#ifndef NOSPL
  case XXASS: /* ASSIGN */
    return (hmsga(hxxass));

  case XXASK:  /* ASK */
  case XXASKQ: /* ASKQ */
    return (hmsga(hxxask));

  case XXAPC:
    return (hmsg("Syntax: APC text\n\
  Echoes the text within a VT220/320/420 Application Program Command."));
#endif /* NOSPL */

#ifndef NOFRILLS
  case XXBUG:
    return (hmsg("Describes how to get technical support."));
#endif /* NOFRILLS */

#ifndef NOSPL
  case XXBEEP:
    return (hmsg("Syntax: BEEP\n\
Sends a BEL character to your terminal."));
#endif /* NOSPL */

  case XXBYE: /* BYE */
    return (hmsg(hmxxbye));

  case XXCHK: /* check */
    return (hmsg("\
Syntax: CHECK name\n\
  Checks\
  to see if the named feature is included in this version of Kermit.\n\
  To list the features you can check, type \"check ?\"."));

#ifndef NOFRILLS
  case XXCLE: /* clear */
    return (hmsga(hmxxcle));
#endif /* NOFRILLS */

  case XXCLO: /* close */
    return (hmsga(hmxxclo));

  case XXCOM: /* comment */
    return (hmsg("\
Syntax: COMMENT text\n\
Example: COMMENT - this is a comment.\n\
  Introduces a comment.  Beginning of command line only.  Commands may also\n\
  have trailing comments, introduced by ; or #."));

#ifndef NOLOCAL
  case XXCON: /* CONNECT */
  case XXCQ:  /* CQ == CONNECT /QUIETLY */
    hmsga(hmxxcon);
    printf("Your escape character is Ctrl-%c (ASCII %d, %s)\r\n", ctl(escape),
           escape, (escape == 127 ? "DEL" : ccntab[escape]));
    return (0);
#endif /* NOLOCAL */

#ifdef ZCOPY
  case XXCPY:
    return (hmsga(hmxxcpy));
#endif /* ZCOPY */

#ifndef NOFRILLS
  case XXLREN: /* LRENAME */
    return (hmsg(
        "  LRENAME is an alias for the RENAME command forcing it to execute\n\
  on the local computer.  Also see: RENAME, RRENAME, SET LOCUS."));

  case XXREN:
    return (hmsga(hmxxren));
#endif /* NOFRILLS */

  case XXCDUP: /* CDUP */
  case XXLCDU:
    return (hmsg(
        "Change working directory to the one just above the current one."));

  case XXLCWD:
    return (hmsg(
        "  LCD (LCWD) is an alias for the CD (CWD) command forcing it to execute\n\
  on the local computer.  Also see: CD, CDUP, RCD, SET LOCUS."));

  case XXCWD: /* CD / CWD */
    return (hmsga(hmxxcwd));

#ifndef NOSPL
  case XXKCD:
    return (hmsga(hmxxkcd));

  case XXARRAY:
  case XXDCL: /* DECLARE */
  case XXSORT:
    return (hmsga(hxxdcl));

  case XXDEF: /* DEFINE */
#ifndef NOSPL
    if (hlptok) /* What they actually typed... */
      if (hlptok[0] == '.')
        return (hmsga(hxxdot));
#endif /* NOSPL */
    return (hmsga(hxxdef));

  case XXUNDEF: /* UNDEFINE */
    return (hmsg("Syntax:  UNDEFINE variable-name\n\
  Undefines a macro or variable."));
#endif /* NOSPL */

  case XXMSG:
    return (hmsg("Syntax: MESSAGE text-to-print-if-debugging\n\
  Prints the given text to stdout if SET DEBUG MESSAGE is ON; prints it\n\
  to stderr if SET DEBUG MESSAGE is STDERR; doesn't print it at all if SET\n\
  DEBUG MESSAGE is OFF.  Synonym: MSG."));

  case XXXMSG:
    return (hmsg("Syntax: XMESSAGE text-to-print-if-debugging\n\
  Like MESSAGE, except does not include a line terminator at the end.\n\
  Prints the given text to stdout if SET DEBUG MESSAGE is ON; prints it\n\
  to stderr if SET DEBUG MESSAGE is STDERR; doesn't print it at all if SET\n\
  DEBUG MESSAGE is OFF.  Synonym: XMSG."));

#ifndef NOFRILLS
  case XXLDEL:
    return (hmsg(
        "  LDELETE is an alias for the DELETE command forcing it to execute\n\
  on the local computer.  Also see: DELETE, RDELETE, SET LOCUS."));

  case XXDEL: /* delete */
    return (hmsga(hmxxdel));
#endif /* NOFRILLS */

#ifndef NODIAL
  case XXDIAL: /* DIAL, etc... */
    return (hmsga(hxxdial));

  case XXPDIA: /* PDIAL */
    return (hmsg("Syntax: PDIAL phonenumber\n\
  Partially dials a phone number.  Like DIAL but does not wait for carrier\n\
  or CONNECT message."));

  case XXRED:
    return (hmsg("Redial the number given in the most recent DIAL commnd."));

  case XXANSW: /* ANSWER */
    return (hmsga(hxxansw));

  case XXLOOK: /* LOOKUP number in directory */
    return (hmsga(hxxlook));
#endif /* NODIAL */

  case XXLDIR: /* LDIRECTORY */
    return (hmsg(
        "  LDIRIRECTORY is an alias for the DIRECTORY command forcing it to execute\n\
  on the local computer.  Also see: DIRECTORY, SET LOCUS, RDIRECTORY."));

  case XXDIR: /* DIRECTORY */
    return (hmsga(hmxxdir));

  case XXTOUC: /* TOUCH */
    return (hmsga(hmxxtouch));

  case XXCHG: /* CHANGE */
    return (hmsga(hmxxchange));

  case XXWDIR: /* WDIRECTORY */
    return (
        hmsg("  WDIRECTORY is shorthand for DIRECTORY /SORT:DATE /REVERSE;\n\
  it produces a listing in reverse chronological order.  See the DIRECTORY\n\
  command for further information."));

  case XXHDIR: /* HDIRECTORY */
    return (
        hmsg("  HDIRECTORY is shorthand for DIRECTORY /SORT:SIZE /REVERSE;\n\
  it produces a listing showing the biggest files first.  See the DIRECTORY\n\
  command for further information."));

  case XXLMKD: /* LMKDIR */
    return (hmsg(
        "  LMKDIR is an alias for the MKDIR command forcing it to execute\n\
  on the local computer.  Also see: MKDIR, RMKDIR, SET LOCUS."));

  case XXMKDIR: /* MKDIR */
    return (hmsg("Creates a directory.  Also see LRMDIR, RRMDIR, SET LOCUS."));

  case XXLRMD: /* LRMDIR */
    return (hmsg(
        "  LRMDIR is an alias for the RMDIR command forcing it to execute\n\
  on the local computer.  Also see: RMDIR, RRMDIR, SET LOCUS."));

  case XXRMDIR: /* RMDIR */
    return (hmsg("Removes a directory.  Also see LRMDIR, RRMDIR, SET LOCUS."));

  case XXLS:
    return (hmsg("Syntax: LS [ args ]\n\
  Runs \"ls\" with the given arguments."));

#ifndef NOSERVER
#ifndef NOFRILLS
  case XXDIS:
    return (hmsg("Syntax: DISABLE command\n\
  Security for the Kermit server.  Prevents the client Kermit program from\n\
  executing the named REMOTE command, such as CD, DELETE, RECEIVE, etc."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
  case XXDO: /* do */
    return (hmsg("Syntax: [ DO ] macroname [ arguments ]\n\
  Executes a macro that was defined with the DEFINE command.  The word DO\n\
  can be omitted.  Trailing argument words, if any, are automatically\n\
  assigned to the macro argument variables \\%1 through \\%9."));
#endif /* NOSPL */

#ifndef NOSPL
  case XXDEC:
    return (hmsga(hxxdec));
#endif /* NOSPL */

  case XXECH: /* echo */
    return (hmsg("Syntax: ECHO text\n\
  Displays the text on the screen, followed by a line terminator.  The ECHO\n\
  text may contain backslash codes.  Example: ECHO \\7Wake up!\\7.  Also see\n\
  XECHO and WRITE SCREEN."));

  case XXXECH: /* xecho */
    return (hmsg("Syntax: XECHO text\n\
  Just like ECHO but does not add a line terminator to the text.  See ECHO."));

  case XXVOID:
    return (hmsg("Syntax: VOID text\n\
  Like ECHO but doesn't print anything; can be used to invoke functions\n\
  when you don't need to display or use their results."));

#ifndef NOSERVER
#ifndef NOFRILLS
  case XXENA:
    return (hmsg("Syntax: ENABLE capability\n\
  For use with server mode.  Allows the client Kermit program access to the\n\
  named capability, such as CD, DELETE, RECEIVE, etc.  Opposite of DISABLE."));
#endif /* NOFRILLS */
#endif /* NOSERVER */

#ifndef NOSPL
  case XXEND: /* end */
    return (hmsg("Syntax: END [ number [ message ] ]\n\
  Exits from the current macro or TAKE file, back to wherever invoked from.\n\
  Number is return code.  Message, if given, is printed."));

  case XXEVAL: /* evaluate */
    return (hmsga(hmxxeval));
#endif /* NOSPL */

#ifndef NOFRILLS
  case XXERR: /* e-packet */
    return (hmsg("Syntax: E-PACKET\n\
  Sends an Error packet to the other Kermit."));
#endif /* NOFRILLS */

  case XXEXI: /* exit */
  case XXQUI:
    return (hmsga(hmxxexit));

  case XXFIN:
    return (hmsg("Syntax: FINISH\n\
  Tells the remote Kermit server to shut down without logging out."));

#ifndef NOSPL
  case XXFOR:
    return (hmsga(forhlp));
#endif /* NOSPL */

  case XXGET:
    return (hmsga(hmxxget));
  case XXMGET:
    return (hmsga(hmxxmget));

#ifndef NOSPL
#ifndef NOFRILLS
  case XXGOK:
    return (hmsg("Syntax: GETOK [ switches ] prompt\n\
  Prints the prompt, makes user type 'yes', 'no', or 'ok', and sets SUCCESS\n\
  or FAILURE accordingly.  The optional switches are the same as for ASK."));
#endif /* NOFRILLS */
#endif /* NOSPL */

#ifndef NOSPL
  case XXGOTO:
    return (hmsg("Syntax: GOTO label\n\
  In a TAKE file or macro, go to the given label.  A label is a word on the\n\
  left margin that starts with a colon (:).  Example:\n\n\
  :oofa\n\
  echo Hello!\n\
  goto oofa"));
#endif /* NOSPL */

  case XXHAN:
    return (hmsg("Syntax: HANGUP\n\
Hang up the phone or network connection."));

  case XXHLP:
    /*
      We get confirmation here, even though we do it again in hmsga(), to
      prevent the Copyright message from being printed prematurely.  This
      doesn't do any harm, because the first call to cmcfm() sets cmflgs to 1,
      making the second call return immediately.
    */
    if ((x = cmcfm()) < 0)
      return (x);

    if (helpfile) {
      printf("\n%s, Copyright (C) 1985, %s,\n\
Trustees of Columbia University in the City of New York.\n\n",
             versio, ck_cryear);
      return (dotype(helpfile, xaskmore, 3, 0, NULL, 0, NULL, 0, 0, NULL, 0));
    } else {
      printf("\n%s, Copyright (C) 1985, %s,", versio, ck_cryear);
      return (hmsga(tophlp));
    }

  case XXINT:
    return (hmsg("The INTRO command gives a brief introduction to C-Kermit."));

#ifndef NOSPL
  case XXIF:
    return (hmsga(ifhlp));

  case XXINC:
    return (hmsga(hxxinc));

  case XXINP:
    return (hmsga(hxxinp));
#endif /* NOSPL */

#ifdef CK_MINPUT
  case XXMINP:
    return (hmsga(hmxxminp));
#endif /* CK_MINPUT */

#ifndef NOSPL
  case XXREI:
    return (hmsg("Syntax: REINPUT n string\n\
  Looks for the string in the text that has recently been INPUT, set SUCCESS\n\
  or FAILURE accordingly.  Timeout, n, must be specified but is ignored."));
#endif /* NOSPL */

#ifndef NOSPL
  case XXLBL:
    return (hmsg("\
  Introduces a label, like :loop, for use with GOTO in TAKE files or macros.\n\
See GOTO."));
#endif /* NOSPL */

  case XXLOG:
    return (hmsga(hmxxlg));

#ifndef NOSCRIPT
  case XXLOGI:
    return (hmsga(hmxxlogi));
#endif

#ifndef NOFRILLS
  case XXMAI:
    return (hmsg("Syntax: MAIL filename address\n\
  Equivalent to SEND /MAIL:address filename."));
#endif /* NOFRILLS */

#ifndef NOMSEND
  case XXMSE:
    return (hmsga(hmxxmse));

  case XXADD:
    return (hmsga(hmxxadd));

  case XXMMOVE:
    return (hmsg("MMOVE is exactly like MSEND, except each file that is\n\
sent successfully is deleted after it is sent."));
#endif /* NOMSEND */

#ifndef NOSPL
  case XXOPE:
    return (hmsga(openhlp));
#endif /* NOSPL */

  case XXNEW:
    return (hmsg("  Prints news of new features since publication of \"Using "
                 "C-Kermit\"."));

  case XXUPD:
    return (hmsg("  New features are described in the online manual,\n\
   accessible via the MANUAL command."));

#ifndef NOSPL
  case XXOUT:
    return (hmsga(hxxout));
#endif /* NOSPL */

#ifdef ANYX25
#ifndef IBMX25
  case XXPAD:
    return (hmsga(hxxpad));
#endif /* IBMX25 */
#endif /* ANYX25 */

#ifndef NOSPL
  case XXPAU:
    return (hmsga(hxxpau));

  case XXMSL:
    return (hmsga(hxxmsl));
#endif /* NOSPL */

#ifdef TCPSOCKET
  case XXPNG:
    return (hmsg("Syntax: PING [ IP-hostname-or-number ]\n\
  Checks if the given IP network host is reachable.  Default host is from\n\
  most recent SET HOST or TELNET command.  Runs system PING program, if any."));

  case XXFTP:
#ifdef SYSFTP
    return (hmsg("Syntax: FTP [ IP-hostname-or-number ]\n\
  Makes an FTP connection to the given IP host or, if no host specified, to\n\
  the current host.  Uses the system's FTP program, if any."));
#else
#ifndef NOFTP
    return (doftphlp());
#endif /* NOFTP */
#endif /* SYSFTP */
#endif /* TCPSOCKET */

#ifndef NOFRILLS
  case XXPRI:
#ifdef UNIX
    return (hmsg("Syntax: PRINT file [ options ]\n\
  Prints the local file on a local printer with the given options.  Also see\n\
  HELP SET PRINTER."));
#else
    return (hmsg("Syntax: PRINT file\n\
  Prints the local file on a local printer.  Also see HELP SET PRINTER."));
#endif /* VMS */
#endif /* NOFRILLS */

  case XXPWD:
  case XXLPWD:
    return (hmsg("Syntax: PWD\n\
Print the name of the current working directory."));

#ifndef NOSPL
  case XXREA:
    return (hmsg("Syntax: READ variablename\n\
  Reads a line from the currently open READ or !READ file into the variable\n\
  (see OPEN)."));
#endif /* NOSPL */

#ifndef NOXFER
  case XXREC:
    return (hmsga(hmxxrc));

  case XXREM:
    y = cmkey(remcmd, nrmt, "Remote command", "", xxstring);
    return (dohrmt(y));
#endif /* NOXFER */

#ifndef NOSPL
  case XXRET:
    return (hmsg("Syntax: RETURN [ value ]\n\
  Return from a macro.  An optional return value can be given for use with\n\
  \\fexecute(macro), which allows macros to be used like functions."));
#endif /* NOSPL */

#ifndef NOXFER
  case XXSEN:
    return (hmsga(hmxxsen));
#ifdef OLDMOVE
  case XXMOVE:
    return (hmsg("MOVE is exactly like SEND, except each file that is\n\
sent successfully is deleted after it is sent."));
#endif /* OLDMOVE */
#ifndef NORESEND
  case XXRSEN:
    return (hmsg(hmxxrsen));
  case XXREGET:
    return (hmsg(hmxxrget));
  case XXPSEN:
    return (hmsg(hmxxpsen));
#endif /* NORESEND */

#ifndef NOSERVER
  case XXSER:
    return (hmsg(hmxxser));
#endif /* NOSERVER */
#endif /* NOXFER */

#ifndef NOJC
  case XXSUS:
    return (hmsg("Syntax: SUSPEND or Z\n\
  Suspends Kermit.  Continue Kermit with the appropriate system command,\n\
  such as fg."));
#endif /* NOJC */

  case XXSET:
    y = cmkey(prmtab, nprm, "Parameter", "", xxstring);
    debug(F101, "HELP SET y", "", y);
    return (dohset(y));

#ifndef NOPUSH
  case XXSHE:
    if (nopush) {
      if ((x = cmcfm()) < 0)
        return (x);
      printf("Sorry, help not available for \"%s\"\n", cmdbuf);
      break;
    } else
      return (hmsga(hxxshe));
#ifdef CK_REDIR
  case XXFUN:
    return (hmsg("Syntax: REDIRECT command\n\
  Runs the given local command with its standard input and output redirected\n\
  to the current SET LINE or SET HOST communications path.\n\
  Synonym: < (Left angle bracket)."));
#endif /* CK_REDIR */

#ifdef CK_REXX
  case XXREXX:
    return (hmsg("Syntax: REXX text\n\
  The text is a Rexx command to be executed. The \\v(rexx) variable is set\n\
  to the Rexx command's return value.\n\
  To execute a rexx program file, use:  REXX call <filename>\n\
  Rexx programs may call Kermit functions by placing the Kermit command\n\
  in single quotes.  For instance:  'set parity none'."));
#endif /* CK_REXX */
#endif /* NOPUSH */

#ifndef NOSHOW
  case XXSHO:
    return (hmsg("\
  Display current values of various items (SET parameters, variables, etc).\n\
  Type SHOW ? for a list of categories."));
#endif /* NOSHOW */

  case XXSPA:
    return (hmsg("Syntax: SPACE\n\
  Display disk usage in current device and/or directory"));

  case XXSTA:
    return (hmsg("Syntax: STATISTICS [/BRIEF]\n\
  Display statistics about most recent file transfer"));

#ifndef NOSPL
  case XXSTO:
    return (hmsg("Syntax: STOP [ number [ message ] ]\n\
  Stop executing the current macro or TAKE file and return immediately to\n\
  the Kermit prompt.  Number is a return code.  Message printed if given."));
#endif /* NOSPL */

  case XXTAK:
    return (hmsga(hmxxtak));

#ifdef TCPSOCKET
#ifdef TNCODE
  case XXIKSD:
    return (hmsga(hmxxiks));

  case XXTEL:
    return (hmsga(hmxxtel));

  case XXTELOP:
    return (hmsga(hxtopt));
#endif /* TNCODE */

#ifdef RLOGCODE
  case XXRLOG:
    return (hmsg("Syntax: RLOGIN [ switches ] [ host [ username ] ]\n\
  Equivalent to SET NETWORK TYPE TCP/IP, SET HOST host [ service ] /RLOGIN,\n\
  IF SUCCESS CONNECT.  If host is omitted, the previous connection (if any)\n\
  is resumed.  Depending on how Kermit has been built switches may be\n\
  available to require Kerberos authentication and DES encryption."));
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifndef NOXMIT
  case XXTRA:
    return (hmsga(hxxxmit));
#endif /* NOXMIT */

#ifndef NOFRILLS
  case XXTYP:
    return (hmsga(hmxxtyp));
  case XXMORE:
    return (hmsg("Syntax: MORE [ switches ] filename\n\
  Equivalent to TYPE /PAGE filename; see HELP TYPE."));
  case XXCAT:
    return (hmsg("Syntax: MORE [ switches ] filename\n\
  Equivalent to TYPE /NOPAGE filename; see HELP TYPE."));
  case XXHEAD:
    return (hmsg("Syntax: HEAD [ switches ] filename\n\
  Equivalent to TYPE /HEAD filename; see HELP TYPE."));
  case XXTAIL:
    return (hmsg("Syntax: TAIL [ switches ] filename\n\
  Equivalent to TYPE /TAIL filename; see HELP TYPE."));
#endif /* NOFRILLS */

#ifndef NOSPL
  case XXWHI:
    return (hmsga(whihlp));

  case XXSWIT:
    return (hmsga(swihlp));
#endif /* NOSPL */

#ifndef NOCSETS
  case XXXLA:
    return (hmsga(hxxxla));
#endif /* NOCSETS */

  case XXVER:
    return (hmsg("Syntax: VERSION\nDisplays the program version number."));

#ifndef NOSPL
  case XXWAI:
    return (hmsga(hxxwai));
#endif /* NOSPL */

#ifndef NOFRILLS
  case XXWHO:
    return (hmsg("Syntax: WHO [ user ]\nDisplays info about the user."));

  case XXWRI:
    return (hmsga(hxxwri));

  case XXWRL:
    return (hmsg(
        "WRITE-LINE (WRITELN) is just like WRITE, but includes a line terminator\n\
at the end of text.  See WRITE."));
#endif /* NOFRILLS */

#ifndef NOSPL
  case XXIFX:
    return (hmsga(ifxhlp));

  case XXGETC: /* GETC */
    return (hmsga(hxxgetc));

  case XXFWD: /* FORWARD */
    return (
        hmsg("Like GOTO, but searches only forward for the label.  See GOTO."));

  case XXLOCAL: /* LOCAL */
    return (hmsg("Declares a variable to be local to the current macro or "
                 "command file."));
#endif /* NOSPL */

  case XXVIEW:
    return (hmsg("View the terminal emulation screen even when there is no "
                 "connection."));

#ifdef NEWFTP
  case XXASC:
    return (hmsg(
        "Inhibits automatic transfer-mode switching and forces TEXT (ASCII) transfer\n\
mode for all files in both Kermit and FTP protocols."));
  case XXBIN:
    return (hmsg(
        "Inhibits automatic transfer-mode switching and forces BINARY transfer mode\n\
for all files in both Kermit and FTP protocols."));
#else
  case XXASC:
    return (hmsg(
        "Inhibits automatic transfer-mode switching and forces TEXT (ASCII) transfer\n\
mode for all files."));
  case XXBIN:
    return (hmsg(
        "Inhibits automatic transfer-mode switching and forces BINARY transfer mode\n\
for all files."));
#endif /* NEWFTP */

  case XXDATE:
    return (hmsga(hmxxdate));

  case XXRETR:
    return (hmsg(
        "Just like GET but asks the server to delete each file that has been\n\
sent successfully."));

  case XXEIGHT:
    return (hmsg("Equivalent to SET PARITY NONE, SET COMMAND BYTE 8, SET "
                 "TERMINAL BYTE 8."));

  case XXSAVE:
    return (hmsga(hmxxsave));

#ifndef NOFRILLS
#ifndef NOPUSH
  case XXEDIT:
    return (hmsg("Syntax: EDIT [ <file> ]\n\
Starts your preferred editor on the given file, or if none given, the most\n\
recently edited file, if any.  Also see SET EDITOR."));
#endif /* NOPUSH */
#endif /* NOFRILLS */

#ifdef BROWSER
  case XXBROWS:
    return (hmsg("Syntax: BROWSE [ <url> ]\n\
Starts your preferred Web browser on the given URL, or if none given, the\n\
most recently visited URL, if any.  Also see SET BROWSER."));
#endif /* BROWSER */

#ifdef CK_TAPI
  case XXTAPI:
    return (hmsga(hxxtapi));
#endif /* CK_TAPI */

#ifdef PIPESEND
  case XXCSEN:
    return (hmsg("Syntax: CSEND [ switches ] <command> [ <as-name> ]\n\
Sends from the given <command> rather than from a file.  Equivalent to\n\
SEND /COMMAND; see HELP SEND for details."));

  case XXCREC:
    return (hmsg("Syntax: CRECEIVE [ switches ] <command>\n\
Receives to the given <command> rather than to a file.  Equivalent to\n\
RECEIVE /COMMAND; see HELP RECEIVE for details."));

  case XXCGET:
    return (hmsg("Syntax: CGET <remote-file-or-command> <local-command>\n\
Equivalent to GET /COMMAND; see HELP GET for details."));
#endif /* PIPESEND */

#ifndef NOSPL
  case XXCSN: /* Compact Substring Notation */
    return (hmsga(hxxcsn));

  case XXFUNC:
    /*
      Tricky parsing.  We want to let them type the function name in any format
      at all: \fblah(), \fblah, \\fblah(), fblah, blah, blah(), etc, but of
      course only one of these is recognized by cmkey().  So we call cmkeyx()
      (the "no complaints" version of cmkey()), and if it fails, we try the
      other formats silently, and still allow for <no-name-given>, editing and
      reparse, etc.
    */
    y = cmkeyx(fnctab, nfuncs, "Name of function", "", NULL);
    if (y == -1) { /* Reparse needed */
      return (y);
    } else if (y == -3) {
      if ((x = cmcfm()) < 0) /* For recall buffer... */
        return (x);
      return (dohfunc(y)); /* -3 gives general message */
    }
    if (y < 0) { /* Something given but didn't match */
      int dummy;
      char *p;
      for (p = atmbuf; *p; p++) { /* Chop off trailing parens if any */
        if (*p == '(') {
          *p = NUL;
          break;
        }
      }
      /* Chop off leading "\\f" or "\f" or "f" */
      p = atmbuf;
      if (*p == CMDQ) /* Allow for \\f... */
        p++;
      if (*p == CMDQ && (*(p + 1) == 'f' || *(p + 1) == 'F')) { /* or \f */
        p += 2;
      } else if (*p == 'f' || *p == 'F') { /* or just f */
        p++;
      }
      y = lookup(fnctab, p, nfuncs, &dummy); /* Look up the result */
    }
    if (y < 0) {
      printf("?No such function - \"%s\"\n", atmbuf);
      return (-9);
    }
    x = cmgbrk();              /* Find out how user terminated */
    if (x == LF || x == CK_CR) /* if with CR or LF */
      cmflgs = 1;              /* restore cmflgs to say so */
    if ((x = cmcfm()) < 0)     /* And THEN confirm so command will */
      return (x);              /* get into recall buffer. */
    return (dohfunc(y));
#endif /* NOSPL */

#ifndef NOCMDL
  case XXOPTS: /* Command-line options */
    return (dohopts());

  case XXXOPTS: /* Extended command-line options */
    return (doxopts());
#endif /* NOCMDL */

  case XXKERMI:
    return (hmsg("Syntax: KERMIT [command-line-options]\n\
  Lets you give command-line options at the prompt or in a script.\n\
  HELP OPTIONS for more info."));

  case XXBACK:
    return (hmsg("Syntax: BACK\n  Returns to your previous directory."));

  case XXWHERE:
    return (hmsg("Syntax: WHERE\n  Tells where your transferred files went."));

#ifndef NOXFER
  case XXREMV:
    return (hmsga(hmxxremv));
#endif /* NOXFER */

#ifndef NOHTTP
  case XXHTTP:
    return (hmsga(hmxxhttp));
#endif /* NOHTTP */

#ifdef NETCMD
  case XXPIPE:
    return (hmsg("Syntax: PIPE [ command ]\n\
Makes a connection through the program whose command line is given. Example:\n\
\n pipe rlogin xyzcorp.com"));
#endif /* NETCMD */

#ifdef NETPTY
#endif

  case XXSTATUS:
    return (hmsg(
        "STATUS is the same as SHOW STATUS; prints SUCCESS or FAILURE for the\n\
previous command."));

#ifndef NOSPL
  case XXASSER:
    return (hmsg("Syntax: ASSERT <condition>\n\
Succeeds or fails depending on <condition>; see HELP IF for <condition>s."));

  case XXFAIL:
    return (hmsg("Always fails."));

  case XXSUCC:
    return (hmsg("Always succeeds."));
#endif /* NOSPL */

#ifdef CK_LOGIN
  case XXLOGOUT:
    return (hmsg(
        "If you haved logged in to Kermit as an Internet Kermit server, the LOGOUT\n\
command, given at the prompt, logs you out and closes your session."));
#endif /* CK_LOGIN */

  case XXRESET:
    return (hmsg("Closes all open files and logs."));

#ifndef NOCSETS
  case XXASSOC:
    return (hmsga(hmxxassoc));
#endif /* NOCSETS */

#ifndef NOSPL
  case XXSHIFT:
    return (hmsg("Syntax: SHIFT [ n ]\n\
  Shifts script command line or macro or TAKE file argument variables\n\
  \\%1..9 or \\&_[1..n] n places to the left; default n = 1."));
#endif /* NOSPL */

#ifndef NOPUSH
  case XXMAN:
#ifdef UNIX
    return (hmsg("Syntax: MANUAL [ topic ]\n\
  Runs the \"man\" command on the given topic (default \"kermit\")."));
#else
    return (hmsg("Syntax: MANUAL [ topic ]\n\
  Runs the \"help\" command on the given topic (default \"kermit\")."));
#endif /* UNIX */
#endif /* NOPUSH */

  case XXWILD:
    return (hmsga(hmxxwild));

#ifdef LOCUS
  case XXLOCU:
    return (hmsga(hmxylocus));
#endif /* LOCUS */

  case XXPAT:
    return (hmsga(hmxxpat));

#ifndef NOXFER
  case XXFAST:
  case XXCAU:
  case XXROB:
    return (hmsga(hmxxfast));
#endif /* NOXFER */

#ifdef CKPURGE
  case XXPURGE:
    return (hmsga(hmxxpurge));
#else
#endif /* CKPURGE */

#ifndef NOXFER
  case XXRASG:
    return (hmsg("  RASG and RASSIGN are short forms of REMOTE ASSIGN."));
  case XXRCWD:
    return (hmsg("  RCD and RCWD are short forms of REMOTE CD."));
  case XXRCPY:
    return (hmsg("  RCOPY is a short form of REMOTE COPY."));
  case XXRDEL:
    return (hmsg("  RDELETE is a short form of REMOTE RELETE."));
  case XXRDIR:
    return (hmsg("  RDIRECTORY is a short form of REMOTE DIRECTORY."));
  case XXRXIT:
    return (hmsg("  REXIT is a short form of REMOTE EXIT."));
  case XXRHLP:
    return (hmsg("  RHELP is a short form of REMOTE HELP."));
  case XXRHOS:
    return (hmsg("  RHOST is a short form of REMOTE HOST."));
  case XXRKER:
    return (hmsg("  RKERMIT is a short form of REMOTE KERMIT."));
  case XXRMKD:
    return (hmsg("  RMKDIR is a short form of REMOTE MKDIR."));
  case XXRMSG:
    return (hmsg("  RMESSAGE and RMSG are short forms of REMOTE MESSAGE."));
  case XXRPRI:
    return (hmsg("  RPRINT is a short form of REMOTE PRINT."));
  case XXRPWD:
    return (hmsg("  RPWD is a short form of REMOTE PWD."));
  case XXRQUE:
    return (hmsg("  QUERY and RQUERY are short forms of REMOTE QUERY."));
  case XXRREN:
    return (hmsg("  RRENAME is a short form of REMOTE RENAME."));
  case XXRRMD:
    return (hmsg("  RRMDIR is a short form of REMOTE RMDIR."));
  case XXRSET:
    return (hmsg("  RSET is a short form of REMOTE SET."));
  case XXRSPA:
    return (hmsg("  RSPACE is a short form of REMOTE SPACE."));
  case XXRTYP:
    return (hmsg("  RTYPE is a short form of REMOTE TYPE."));
  case XXRWHO:
    return (hmsg("  RWHO is a short form of REMOTE WHO."));
  case XXRCDUP:
    return (hmsg("  RCDUP is a short forms of REMOTE CDUP."));
#endif /* NOXFER */

  case XXSCRN:
    return (hmsga(hmxxscrn));

#ifdef CKEXEC
  case XXEXEC:
    return (hmsg("Syntax: EXEC <command> [ <arg1> [ <arg2> [ ... ] ]\n\
  C-Kermit overlays itself with the given system command and starts it with\n\
  the given arguments.  Upon any error, control returns to C-Kermit."));
#endif /* CKEXEC */

#ifndef NOSPL
  case XXTRACE:
    return (
        hmsg("Syntax: TRACE { /ON, /OFF } { ASSIGNMENTS, COMMAND-LEVEL, ALL }\n\
  Turns tracing of the given object on or off."));
#endif /* NOSPL */

#ifdef CK_PERMS
#ifdef UNIX
  case XXCHMOD:
    return (hmsga(hmxxchmod));
#endif /* UNIX */
#endif /* CK_PERMS */

#ifdef CKROOT
  case XXCHRT:
    return (hmsga(hmxxchroot));
#endif /* CKROOT */

#ifndef NOSPL
  case XXPROMP:
    return (hmsga(hmxxprompt));
#endif /* NOSPL */

  case XXGREP:
    return (hmsga(hmxxgrep));

#ifndef NOSEXP
#ifndef NOSPL
  case XXSEXP:
    return (hmsga(hmxxsexp));
#endif /* NOSPL */
#endif /* NOSEXP */

#ifdef CKLEARN
  case XXLEARN:
    return (hmsga(hmxxlearn));
#endif /* CKLEARN */

#ifdef SSHBUILTIN
  case XXSKRM:
    return (hmsga(hmxxskrm));
#endif /* SSHBUILTIN */

#ifdef ANYSSH
  case XXSSH:
#ifdef SSHBUILTIN
  {
    const char **help_content;
    if (ck_ssh_is_installed()) {
      help_content = ssh_get_help();
    } else {
      help_content = (const char **)hmxxssh;
    }
    return (hmsga((char **)help_content));
  }
#else
    return (hmsga(hmxxssh));
#endif /* SSHBUILTIN */
#endif /* ANYSSH */

#ifdef TCPSOCKET
  case XXFIREW:
    return (hmsga(hmxxfirew));
#endif /* TCPSOCKET */

#ifdef NEWFTP
  case XXUSER:
    return (hmsg(" Equivalent to FTP USER."));
  case XXACCT:
    return (hmsg(" Equivalent to FTP ACCOUNT."));
#endif /* NEWFTP */

  case XXORIE:
    return (hmsg(" Shows the directories important to Kermit."));

  case XXCONT:
    return (hmsg(" In a FOR or WHILE loop: continue the loop.\n\
 At the prompt: continue a script that has \"shelled out\" to the prompt."));

  case XXREDO:
    return (hmsg(" Syntax: REDO xxx (or) ^xxx\n\
 Re-executes the most recent command starting with xxx."));

#ifdef UNIX
#ifndef NOPUTENV
  case XXPUTE:
    return (hmsga(hmxxputenv));
#endif /* NOPUTENV */
#endif /* UNIX */

  case XXNOTAV:
    return (hmsg(" This command is not configured in this version of Kermit."));

  default: {
    char *s;
    if ((x = cmcfm()) < 0)
      return (x);
    s = cmdbuf + (int)strlen(cmdbuf) - 1;
    while (s >= cmdbuf && *s == SP)
      *s-- = NUL;
    while (s >= cmdbuf && *s != SP)
      s--;
    while (*s == SP)
      s++;
    printf("Sorry, help not available for \"%s\"\n", s);
    break;
  }
  } /* switch */
#endif /* NOHELP */

  return (success = 0);
}

/*  H M S G  --  Get confirmation, then print the given message  */

int hmsg(char *s) {
  int x;
  if ((x = cmcfm()) < 0)
    return (x);
  printf("\n%s\n\n", s);
  return (0);
}

#ifdef NOHELP

int              /* Print an array of lines, */
hmsga(char *s[]) /* cheap version. */
{
  int i;
  if ((i = cmcfm()) < 0)
    return (i);
  printf("\n");             /* Start off with a blank line */
  for (i = 0; *s[i]; i++) { /* Print each line. */
    printf("%s\n", s[i]);
  }
  printf("\n");
  return (0);
}

#else /* NOHELP not defined... */

int              /* Print an array of lines, */
hmsga(char *s[]) /* cheap version. */
{
  extern int hmtopline; /* (This should be a parameter...) */
  int x, y, i, j, k, n;
  if ((x = cmcfm()) < 0)
    return (x);

#ifdef CK_TTGWSIZ
  /* Check whether window size changed */
  if (ttgwsiz() > 0) {
    if (tt_rows > 0 && tt_cols > 0) {
      cmd_rows = tt_rows;
      cmd_cols = tt_cols;
    }
  }
#endif /* CK_TTGWSIZ */

  printf("\n");                        /* Start off with a blank line */
  n = (hmtopline > 0) ? hmtopline : 1; /* Line counter */
  for (i = 0; *s[i]; i++) {
    printf("%s\n", s[i]); /* Print a line. */
    y = (int)strlen(s[i]);
    k = 1;
    for (j = 0; j < y; j++) /* See how many newlines were */
      if (s[i][j] == '\n')
        k++; /* in the string... */
    n += k;
    if (n > (cmd_rows - 3) && *s[i + 1]) { /* After a screenful, give them */
      if (!askmore()) {                    /* a "more?" prompt. */
        return (0);
      } else {
        n = 0;
      }
    }
  }
  printf("\n");
  return (0);
}

#ifndef NOFRILLS
static char *supporttext[] = {

    "Live technical support for Kermit software is no longer available",
    "from Columbia University, as it was from mid-1981 until mid-2011 when",
    "the Kermit Project was cancelled.  Beginning with version 9.0, C-Kermit",
    "is Open Source software.  The Kermit project has been moved to:",
    " ",
    "  http://www.kermitproject.org/",
    " ",
    "The C-Kermit home page is here:",
    " ",
    "  http://www.kermitproject.org/ckermit.html",
    " ",
    "The documentation for C-Kermit is listed here:",
    " ",
    "  http://www.kermitproject.org/ckermit.html#doc",
    " ",
    "A C-Kermit tutorial is here:",
    " ",
    "  http://www.kermitproject.org/ckututor.html",
    " ",
    "The C-Kermit Frequently Asked Questions page is here:",
    " ",
    "  http://www.kermitproject.org/ckfaq.html",
    " ",
    "and the Kermit Project Technical Support page is here:",
    " ",
    "  http://www.kermitproject.org/support.html",
    "  ",
    "If you have a problem or question that is not addressed on the website",
    "you can send email to:",
    " ",
    "  support@kermitproject.org",
    " ",
    "and as long as anyone is still at that address, it will be answered",
    "on a best-effort basis.",
    ""};

/* Do the BUG command */
int dobug() { return (hmsga(supporttext)); }
#endif /* NOFRILLS */

#ifndef NOXMIT
static char *hsetxmit[] = {
    "Syntax: SET TRANSMIT parameter value",
    " ",
    "Controls the behavior of the TRANSMIT command (see HELP TRANSMIT):",
    " ",
    "SET TRANSMIT ECHO { ON, OFF }",
    "  Whether to echo text to your screen as it is being transmitted.",
    " ",
    "SET TRANSMIT EOF text",
    "  Text to send after end of file is reached, e.g. \\4 for Ctrl-D",
    " ",
    "SET TRANSMIT FILL number",
    "  ASCII value of a character to insert into blank lines, 0 for none.",
    "  Applies only to text mode.  0 by default.",
    " ",
    "SET TRANSMIT LINEFEED { ON, OFF }",
    "  Transmit Linefeed as well as Carriage Return (CR) at the end of each "
    "line.",
    "  Normally, only CR  is sent.",
    " ",
    "SET TRANSMIT LOCKING-SHIFT { ON, OFF }",
    "  Whether to use SO/SI for transmitting 8-bit data when PARITY is not "
    "NONE.",
    " ",
    "SET TRANSMIT PAUSE number",
    "  How many milliseconds to pause after transmitting each line (text "
    "mode),",
    "  or each character (binary mode).",
    " ",
    "SET TRANSMIT PROMPT number",
    "  ASCII value of character to look for from host before sending next line",
    "  when TRANSMITting in text mode; normally 10 (Linefeed).  0 means none;",
    "  don't wait for a prompt.",
    " ",
    "SET TRANSMIT TIMEOUT number",
    "  Number of seconds to wait for each character to echo when TRANSMIT ECHO",
    "  is ON or TRANSMIT PROMPT is not 0.  If 0 is specified, this means wait",
    "  indefinitely for each echo.",
    " ",
    "Synonym: SET XMIT.  SHOW TRANSMIT displays current settings.",
    ""};
#endif /* NOXMIT */

static char *hsetbkg[] = {
    "Syntax: SET BACKGROUND { OFF, ON }", " ",
    "  SET BACKGROUND OFF forces prompts and messages to appear on your screen",
    "  even though Kermit thinks it is running in the background.", ""};

#ifdef DYNAMIC
static char *hsetbuf[] = {
    "Syntax: SET BUFFERS n1 [ n2 ]",
    " ",
    "  Changes the overall amount of memory allocated for SEND and RECEIVE "
    "packet",
    "  buffers, respectively.  Bigger numbers let you have longer packets and",
    "  more window slots.  If n2 is omitted, the same value as n1 is used.",
#ifdef BIGBUFOK
    " ",
    "  NOTE: This command is not needed in this version of Kermit, which is",
    "  already configured for maximum-size packet buffers.",
#endif /* BIGBUFOK */
    ""};
#endif /* DYNAMIC */

static char *hsetcmd[] = {
    "Syntax: SET COMMAND parameter value",
    " ",

#ifdef CK_AUTODL
    "SET COMMAND AUTODOWNLOAD { ON, OFF }",
    "  Enables/Disables automatic recognition of Kermit packets while in",
    "  command mode.  ON by default.",
    " ",
#endif /* CK_AUTODL */

    "SET COMMAND BYTESIZE { 7, 8 }",
    "  Informs Kermit of the bytesize of the communication path between itself",
    "  and your keyboard and screen.  8 is assumed.  SET COMMAND BYTE 7 only "
    "if",
    "  8-bit characters cannot pass.",
    " ",

    "SET COMMAND ERROR { 0,1,2,3 }",
    "  Sets the verbosity level of command error messages; the higher the "
    "number,",
    "  the more verbose the message.  The default is 1.  Higher values are",
    "  useful only for debugging scripts.",
    " ",

    "SET COMMAND HEIGHT <number>",
    "  Informs Kermit of the number of rows in your command screen for the",
    "  purposes of More?-prompting.",
    " ",
    "SET COMMAND WIDTH <number>",
    "  Informs Kermit of the number of characters across your screen for",
    "  purposes of screen formatting.",
    " ",
    "SET COMMAND MORE-PROMPTING { ON, OFF }",
    "  ON (the default) enables More?-prompting when Kermit needs to display",
    "  text that does not fit vertically on your screen.  OFF allows the text "
    "to",
    "  scroll by without intervention.  If your command window has scroll "
    "bars,",
    "  you might prefer OFF.",
    " ",

#ifdef CK_RECALL
    "SET COMMAND RECALL-BUFFER-SIZE number",
    "  How big you want Kermit's command recall buffer to be.  By default, it",
    "  holds 10 commands.  You can make it any size you like, subject to "
    "memory",
    "  constraints of the computer.  A size of 0 disables command recall.",
    "  Whenever you give this command, previous command history is lost.",
    " ",
#endif /* CK_RECALL */

    "SET COMMAND QUOTING { ON, OFF }",
    "  Whether to treat backslash and question mark as special characters "
    "(ON),",
    "  or as ordinary data characters (OFF) in commands.  ON by default.",
    " ",
#ifdef DOUBLEQUOTING
    "SET COMMAND DOUBLEQUOTING { ON, OFF }",
    "  Whether to allow doublequotes (\") to be used to enclose fields,",
    "  filenames, directory names, and macro arguments that might contain",
    "  spaces.  ON by default; use OFF to force compatibility with older",
    "  versions.",
    " ",
#endif /* DOUBLEQUOTING */

#ifdef CK_RECALL
    "SET COMMAND RETRY { ON, OFF }",
    "  Whether to reprompt you with the correct but incomplete portion of a",
    "  syntactically incorrect command.  ON by default.",
    " ",
#endif /* CK_RECALL */

    "Use SHOW COMMAND to display these settings.",
    ""};

#ifndef NOLOCAL
static char *hsetcar[] = {
    "Syntax: SET CARRIER-WATCH { AUTO, OFF, ON }",
    " ",
    "  Attempts to control treatment of carrier (the Data Carrier Detect "
    "signal)",
    "  on serial communication (SET LINE or SET PORT) devices.  ON means that",
    "  carrier is required at all times.  OFF means carrier is never required.",
    "  AUTO (the default) means carrier is required at all times except during",
    "  the DIAL command.  Correct operation of carrier-watch depends on the",
    "  capabilities of the underlying OS, drivers, devices, and cables.  If "
    "you",
    "  need to CONNECT to a serial device that is not asserting carrier, and",
    "  Kermit won't let you, use SET CARRIER-WATCH OFF.  Use SHOW "
    "COMMUNICATIONS",
    "  to display the CARRIER-WATCH setting.",
    ""};
#endif /* NOLOCAL */

static char *hsetat[] = {
    "Syntax: SET ATTRIBUTES name ON or OFF",
    " ",
    "  Use this command to enable (ON) or disable (OFF) the transmission of",
    "  selected file attributes along with each file, and to handle or ignore",
    "  selected incoming file attributes, including:",
    " ",
#ifndef NOCSETS
    "   CHARACTER-SET:  The transfer character set for text files",
#endif /* NOCSETS */
    "   DATE:           The file's creation date",
    "   DISPOSITION:    Unusual things to do with the file, like MAIL or PRINT",
    "   LENGTH:         The file's length",
    "   PROTECTION:     The file's protection (permissions)",
    "   SYSTEM-ID:      Machine/Operating system of origin",
    "   TYPE:           The file's type (text or binary)",
    " ",
    "You can also specify ALL to select all of them.  Examples:",
    " ",
    "   SET ATTR DATE OFF",
    "   SET ATTR LENGTH ON",
    "   SET ATTR ALL OFF",
    " ",
    "Also see HELP SET SEND and HELP SET RECEIVE.",
    ""};

static char *hxytak[] = {
    "Syntax: SET TAKE parameter value",
    " ",
    "  Controls behavior of TAKE command:",
    " ",
    "SET TAKE ECHO { ON, OFF }",
    "  Tells whether commands read from a TAKE file should be displayed on the",
    "  screen (if so, each command is shown at the time it is read, and "
    "labeled",
    "  with a line number).",
    " ",
    "SET TAKE ERROR { ON, OFF }",
    "  Tells whether a TAKE command file should be automatically terminated "
    "when",
    "  a command fails.  This setting is local to the current command file, "
    "and",
    "  inherited by subordinate command files.",
    ""};

#ifndef NOLOCAL

static char *hxyterm[] = {
    "Syntax: SET TERMINAL parameter value",
    " ",
    "SET TERMINAL TYPE ...",
    "  This command is not available because this version of Kermit does not",
    "  include a terminal emulator.  Instead, it is a \"semitransparent pipe\"",
    "  (or a totally transparent one, if you configure it that way) to the",
    "  computer or service you have made a connection to.  Your console,",
    "  workstation window, or the terminal emulator or terminal from which you",
    "  are running Kermit provides the emulation.",
    " ",

#ifdef CK_APC
    "SET TERMINAL APC { ON, OFF, NO-INPUT, UNCHECKED, UNCHECKED-NO-INPUT }",
    "  Controls execution of Application Program Commands sent by the host "
    "while",
    "  C-Kermit is in CONNECT mode.  ON allows execution of \"safe\" commands "
    "and",
    "  disallows potentially dangerous commands such as DELETE, RENAME, "
    "OUTPUT,",
    "  and RUN.  OFF prevents execution of APCs.  UNCHECKED allows execution "
    "of",
    "  all APCs.  OFF is the default.",
    " ",
#endif /* CK_APC */

    "SET TERMINAL AUTODOWNLOAD { ON, OFF, ERROR { STOP, CONTINUE } }",
    "  Enables/disables automatic switching into file-transfer mode when a "
    "valid",
#ifdef CK_XYZ
    "  Kermit or ZMODEM packet of the appropriate type is received during "
    "CONNECT",
    "  mode.  Default is OFF.",
#else
    "  Kermit packet of the appropriate type is received during CONNECT mode.",
    "  Default is OFF.",
#endif /* CK_XYZ */

    " ",
    "  When TERMINAL AUTODOWNLOAD is ON, the TERMINAL AUTODOWNLOAD ERROR "
    "setting",
    "  tells what to do if an error occurs during a file transfer or other",
    "  protocol operation initiated by the terminal emulator: STOP (the "
    "default)",
    "  means to remain in command mode so you can see what happened; CONTINUE",
    "  means to resume the CONNECT session (e.g. so a far-end script can "
    "continue",
    "  its work).",
    " ",

    "SET TERMINAL BYTESIZE { 7, 8 }",
    "  Use 7- or 8-bit characters between Kermit and the remote computer "
    "during",
    "  terminal sessions.  The default is 8.",
    " ",

#ifndef NOCSETS
    "SET TERMINAL CHARACTER-SET <remote-cs> [ <local-cs> ]",
    "  Specifies the character set used by the remote host, <remote-cs>, and "
    "the",
    "  character set used by C-Kermit locally, <local-cs>.  If you don't "
    "specify",
    "  the local character set, the current FILE CHARACTER-SET is used.  When",
    "  you specify two different character sets, C-Kermit translates between "
    "them",
    "  during CONNECT.  By default, both character sets are TRANSPARENT, and",
    "  no translation is done.",
    " ",
#endif /* NOCSETS */

    "SET TERMINAL CR-DISPLAY { CRLF, NORMAL }",
    "  Specifies how incoming carriage return characters are to be displayed",
    "  on your screen.",
    " ",

    "SET TERMINAL DEBUG { ON, OFF }",
    "  Turns terminal session debugging on and off.  When ON, incoming control",
    "  characters are displayed symbolically, rather than be taken as "
    "formatting",
    "  commands.  SET TERMINAL DEBUG ON implies SET TELNET DEBUG ON.",
    " ",

    "SET TERMINAL ECHO { LOCAL, REMOTE }",
    "  Specifies which side does the echoing during terminal connection.",
    " ",

    "SET TERMINAL ESCAPE-CHARACTER { ENABLED, DISABLED }",
    "  Turns on/off the ability to escape back from CONNECT mode using the SET",
    "  ESCAPE character.  If you disable it, Kermit returns to its prompt only",
    "  when the connection is closed by the other end.  USE WITH EXTREME "
    "CAUTION.",
    "  Also see HELP SET ESCAPE.",
    " ",

    "SET TERMINAL HEIGHT <number>",
    "  Tells C-Kermit how many rows (lines) are on your CONNECT-mode screen.",
    " ",

#ifdef CKTIDLE
    "SET TERMINAL IDLE-TIMEOUT <number>",
    "  Sets the limit on idle time in CONNECT mode to the given number of",
    "  seconds.  0 (the default) means no limit.",
    " ",
    "SET TERMINAL IDLE-ACTION { EXIT, HANGUP, OUTPUT [ text ], RETURN }",
    "  Specifies the action to be taken when a CONNECT session is idle for the",
    "  number of seconds given by SET TERMINAL IDLE-TIMEOUT.  The default "
    "action",
    "  is to RETURN to command mode.  EXIT exits from Kermit; HANGUP hangs up "
    "the",
    "  connection, and OUTPUT sends the given text to the host without leaving",
    "  CONNECT mode; if no text is given a NUL (0) character is sent.",
#ifdef TNCODE
    " ",
    "SET TERMINAL IDLE-ACTION { TELNET-NOP, TELNET-AYT }",
    "  For TELNET connections only: Sends the indicated Telnet protocol "
    "message:",
    "  No Operation (NOP) or \"Are You There?\" (AYT).",
#endif /* TNCODE */
    " ",
#endif /* CKTIDLE */

    "SET TERMINAL LF-DISPLAY { CRLF, NORMAL }",
    "  Specifies how incoming linefeed characters are to be displayed",
    "  on your screen.",
    " ",

#ifndef NOCSETS
#endif /* NOCSETS */

    "SET TERMINAL LOCKING-SHIFT { OFF, ON }",
    "  Tells Kermit whether to use Shift-In/Shift-Out (Ctrl-O and Ctrl-N) to",
    "  switch between 7-bit and 8-bit characters during CONNECT.  OFF by "
    "default.",
    " ",

    "SET TERMINAL NEWLINE-MODE { OFF, ON }",
    "  Tells whether to send CRLF (Carriage Return and Line Feed) when you "
    "type",
    "  CR (press the Return or Enter key) in CONNECT mode.",
    " ",

#ifdef XPRINT
    "SET TERMINAL PRINT { ON, OFF }",
    "  Enables and disables host-initiated transparent printing in CONNECT "
    "mode.",
    " ",
#endif /* XPRINT */

#ifdef CK_TRIGGER
    "SET TERMINAL TRIGGER <string>",
    "  Specifies a string that, when detected during any subsequent CONNECT",
    "  session, is to cause automatic return to command mode.  Give this "
    "command",
    "  without a string to cancel the current trigger.  See HELP CONNECT for",
    "  additional information.",
    " ",
#endif /* CK_TRIGGER */

    "SET TERMINAL WIDTH <number>",
    " \
Tells Kermit how many columns (characters) are on your CONNECT-mode screen.",
    " ",
    "Type SHOW TERMINAL to see current terminal settings.",
    ""};
#endif /* NOLOCAL */

#ifdef NETCONN
static char *hxyhost[] = {
    "SET HOST [ switches ] hostname-or-address [ service ] [ protocol-switch ]",
    "  Establishes a connection to the specified network host on the currently",
    "  selected network type.  For TCP/IP connections, the default service is",
    "  TELNET; specify a different TCP port number or service name to choose a",
    "  different service.  The first set of switches can be:",
    " ",
    " /NETWORK-TYPE:name",
    "   Makes the connection on the given type of network.  Equivalent to SET",
    "   NETWORK TYPE name prior to SET HOST, except that the selected network",
    "   type is used only for this connection.  Type \"set host /net:?\" to "
    "see",
#ifdef NETCMD
    "   a list.  /NETWORK-TYPE:COMMAND means to make the connection through "
    "the",
    "   given system command, such as \"rlogin\" or \"cu\".",
#else
    "   a list.",
#endif /* NETCMD */
    " ",
    " /CONNECT",
    "   \
Enter CONNECT (terminal) mode automatically if the connection is successful.",
    " ",
    " /SERVER",
    "   Enter server mode automatically if the connection is successful.",
    " ",
    " /USERID:[<name>]",
    "   This switch is equivalent to SET LOGIN USERID <name> or SET TELNET",
    "   ENVIRONMENT USER <name>.  If a string is given, it sent to host during",
    "   Telnet negotiations; if this switch is given but the string is "
    "omitted,",
    "   no user ID is sent to the host.  If this switch is not given, your",
    "   current USERID value, \\v(userid), is sent.  When a userid is sent to "
    "the",
    "   host it is a request to login as the specified user.",
    " ",
    "The protocol-switches can be:",
    " ",
    " /NO-TELNET-INIT",
    "   Do not send initial Telnet negotiations even if this is a Telnet port.",
    " ",
    " /RAW-SOCKET",
    "   This is a connection to a raw TCP socket.",
    " ",
#ifdef RLOGCODE
    " /RLOGIN",
    "   Use Rlogin protocol even if this is not an Rlogin port.",
    " ",
#endif /* RLOGCODE */
    " /TELNET",
    "   Send initial Telnet negotiations even if this is not a Telnet port.",
    " ",
    "Examples:",
    "  SET HOST kermit.columbia.edu",
    "  SET HOST /CONNECT kermit.columbia.edu",
    "  SET HOST * 1649",
    "  SET HOST /SERVER * 1649",
    "  SET HOST 128.59.39.2",
    "  SET HOST madlab.sprl.umich.edu 3000",
    "  SET HOST xyzcorp.com 2000 /RAW-SOCKET",
#ifdef SSHBUILTIN
    "  SET HOST /NET:SSH kermit.columbia.edu /x11-forwarding:on",
#endif /* SSHBUILTIN */
#ifdef NETCMD
    "  SET HOST /CONNECT /COMMAND rlogin xyzcorp.com",
#endif /* NETCMD */
    " ",
#ifdef SUPERLAT
    "Notes:",
    " ",
    " . The TELNET command is equivalent to SET NETWORK TYPE TCP/IP,",
    "   SET HOST name [ port ] /TELNET, IF SUCCESS CONNECT",
    " ",
    " . For SUPERLAT connections, the hostname-or-address may be either a "
    "service",
    "   name, or a node/port combination, as required by your LAT host.",
#else
    "The TELNET command is equivalent to SET NETWORK TYPE TCP/IP,",
    "SET HOST name [ port ] /TELNET, IF SUCCESS CONNECT",
#endif /* SUPERLAT */
    " ",
    "Also see SET NETWORK, TELNET, SET TELNET.",
    ""};

static char *hmxyauth[] = {
    "Synatx: SET AUTHENTICATION <auth_type> <parameter> <value>",
    "  Sets defaults for the AUTHENTICATE command:", " ", ""};

static char *hxynet[] = {
    "Syntax: SET NETWORK { TYPE network-type, DIRECTORY [ file(s)... ] }",
    " ",
    "Select the type of network to be used with SET HOST connections:",
    " ",
#ifdef NETCMD
    "  SET NETWORK TYPE COMMAND   ; Make a connection through an external "
    "command",
#endif /* NETCMD */
#ifdef TCPSOCKET
    "  SET NETWORK TYPE TCP/IP    ; Internet: Telnet, Rlogin, etc.",
#endif /* TCPSOCKET */
#ifdef ANYX25
    "  SET NETWORK TYPE X.25      ; X.25 peer-to-peer connections.",
#endif /* ANYX25 */
#ifdef DECNET
    "  SET NETWORK TYPE PATHWORKS { LAT, CTERM } ; DEC LAT or CTERM "
    "connections.",
#endif /* DECNET */
#ifdef NPIPE
    "  SET NETWORK TYPE NAMED-PIPE <pipename>  ; OS/2 Named Pipe connections.",
#endif /* NPIPE */
#ifdef CK_NETBIOS
    "  SET NETWORK TYPE NETBIOS                ; NETBIOS peer-to-peer "
    "connections",
#endif /* CK_NETBIOS */
#ifdef SUPERLAT
    "  SET NETWORK TYPE SUPERLAT ; LAT connections (Meridian Technology "
    "SuperLAT)",
#endif /* SUPERLAT */
    " ",
    "If only one network type is listed above, that is the default network for",
#ifdef RLOGCODE
    "SET HOST commands.  Also see SET HOST, TELNET, RLOGIN.",
#else
#ifdef TNCODE
    "SET HOST commands.  Also see SET HOST, TELNET.",
#else
    "SET HOST commands.  Also see SET HOST.",
#endif /* TNCODE */
#endif /* RLOGCODE */
    " ",
    "SET NETWORK DIRECTORY [ file [ file [ ... ] ] ]",
    "  Specifies the name(s) of zero or more network directory files, similar "
    "to",
    "  dialing directories (HELP DIAL for details).  The general format of a",
    "  network directory entry is:",
    " ",
    "    name network-type address [ network-specific-info ] [ ; comment ]",
    " ",
    "  For TCP/IP, the format is:",
    " ",
    "    name tcp/ip ip-hostname-or-address [ socket ] [ ; comment ]",
    " ",
    "You can have multiple network directories and you can have multiple "
    "entries",
    "with the same name.  SET HOST <name> and TELNET <name> commands look up "
    "the",
    "given <name> in the directory and, if found, fill in the additional items",
    "from the entry, and then try all matching entries until one succeeds.",
    ""};

#ifndef NOTCPOPTS
static char *hxytcp[] = {
#ifdef SOL_SOCKET
    "SET TCP ADDRESS <ip-address>",
    "  This allows a specific IP Address on a multihomed host to be used",
    "  instead of allowing the TCP/IP stack to choose.  This may be necessary",
    "  when using authentication or listening for an incoming connection.",
    "  Specify no <ip-address> to remove the preference.",
    " ",
    "SET TCP KEEPALIVE { ON, OFF }",
    "  Setting this ON might help to detect broken connections more quickly.",
    "  (default is ON.)",
    " ",
    "SET TCP LINGER { ON [timeout], OFF }",
    "  Setting this ON ensures that a connection doesn't close before all",
    "  outstanding data has been transferred and acknowledged.  The timeout is",
    "  measured in 10ths of milliseconds.  The default is ON with a timeout of "
    "0.",
    " ",
    "SET TCP NODELAY { ON, OFF }",
    "  ON means send short TCP packets immediately rather than waiting",
    "  to accumulate a bunch of them before transmitting (Nagle Algorithm).",
    "  (default is OFF.)",
    " ",
    "SET TCP RECVBUF <number>",
    "SET TCP SENDBUF <number>",
    "   TCP receive and send buffer sizes.  (default is -1, use system "
    "defaults.)",
    " ",
    "These items let you tune TCP networking performance on a per-connection",
    "basis by adjusting parameters you normally would not have access to.  You",
    "should use these commands only if you feel that the TCP/IP protocol stack",
    "that Kermit is using is giving you inadequate performance, and then only "
    "if",
    "you understand the concepts (see, for example, the Comer TCP/IP books), "
    "and",
    "then at your own risk.  These settings are displayed by SHOW NETWORK.  "
    "Not",
    "all options are necessarily available in all Kermit versions; it depends "
    "on",
    "the underlying TCP/IP services.",
    " ",
    "The following TCP and/or IP parameter(s) may also be changed:",
    " ",
#endif /* SOL_SOCKET */
    "SET TCP REVERSE-DNS-LOOKUP { AUTO, ON, OFF }",
    "  Tells Kermit whether to perform reverse DNS lookup on TCP/IP "
    "connections",
    "  so Kermit can determine the actual hostname of the host it is connected",
    "  to, which is useful for connections to host pools, and is required for",
    "  Kerberos connections to host pools and for incoming connections.  If "
    "the",
    "  other host does not have a DNS entry, the reverse lookup could take a "
    "long",
    "  time (minutes) to fail, but the connection will still be made.  Turn "
    "this",
    "  option OFF for speedier connections if you do not need to know exactly",
    "  which host you are connected to and you are not using Kerberos.  AUTO, "
    "the",
    "  default, means the lookup is done on hostnames, but not on numeric IP",
    "  addresses unless Kerberos support is installed.",
#ifdef CK_DNS_SRV
    " ",
    "SET TCP DNS-SERVICE-RECORDS {ON, OFF}",
    "  Tells Kermit whether to try to use DNS SRV records to determine the",
    "  host and port number upon which to find an advertised service.  For",
    "  example, if a host wants regular Telnet connections redirected to some",
    "  port other than 23, this feature allows Kermit to ask the host which",
    "  port it should use.  Since not all domain servers are set up to answer",
    "  such requests, this feature is OFF by default.",
#endif /* CK_DNS_SRV */
#ifndef NOHTTP
    " ",
    "SET TCP HTTP-PROXY [<hostname or ip-address>[:<port>]]",
    "  If a hostname or ip-address is specified, Kermit will use the Proxy",
    "  server when attempting outgoing connections.  If no hostname or",
    "  ip-address is specified, any previously specified Proxy server will",
    "  be removed.  If no port number is specified, the \"http\" service",
    "  will be used.",
#endif /* NOHTTP */
    ""};
#endif /* NOTCPOPTS */
#endif /* NETCONN */

#ifdef TNCODE
static char *hxytopt[] = {
    "SET TELOPT [ { /CLIENT, /SERVER } ] <option> -",
    "    { ACCEPTED, REFUSED, REQUESTED, REQUIRED } -",
    "    [ { ACCEPTED, REFUSED, REQUESTED, REQUIRED } ]",
    "  SET TELOPT lets you specify policy requirements for Kermit's handling "
    "of",
    "  Telnet option negotiations.  Setting an option REQUIRED causes Kermit",
    "  to offer the option to the peer and disconnect if the option is "
    "refused.",
    "  REQUESTED causes Kermit to offer an option to the peer.  ACCEPTED "
    "results",
    "  in no offer but Kermit will attempt to negotiate the option if it is",
    "  requested.  REFUSED instructs Kermit to refuse the option if it is",
    "  requested by the peer.",
    " ",
    "  Some options are negotiated in two directions and accept separate "
    "policies",
    "  for each direction; the first keyword applies to Kermit itself, the "
    "second",
    "  applies to Kermit's Telnet partner; if the second keyword is omitted, "
    "an",
    "  appropriate (option-specific) default is applied.  You can also include "
    "a",
    "  /CLIENT or /SERVER switch to indicate whether the given policies apply",
    "  when Kermit is the Telnet client or the Telnet server; if no switch is",
    "  given, the command applies to the client.",
    " ",
    "  Note that some of Kermit's Telnet partners fail to refuse options that",
    "  they do not recognize and instead do not respond at all.  In this case "
    "it",
    "  is possible to use SET TELOPT to instruct Kermit to REFUSE the option",
    "  before connecting to the problem host, thus skipping the problematic",
    "  negotiation.",
    " ",
    "  Use SHOW TELOPT to view current Telnet Option negotiation settings.",
    "  SHOW TELNET displays current Telnet settings.",
    ""};

static char *hxytel[] = {
    "Syntax: SET TELNET parameter value",
    " ",
    "For TCP/IP TELNET connections, which are in NVT (ASCII) mode by default:",
    " ",
    "SET TELNET BINARY-TRANSFER-MODE { ON, OFF }",
    "  When ON (OFF by default) and BINARY negotiations are not REFUSED Kermit",
    "  will attempt to negotiate BINARY mode in each direction before the "
    "start",
    "  of each file transfer.  After the transfer is complete BINARY mode will",
    "  be restored to the pre-transfer state.",
    " ",
    "SET TELNET BINARY-TRANSFER-MODE { ON, OFF }",
    "  Set this command to ON if you want to force Kermit to negotiate",
    "  Telnet Binary in both directions when performing file transfers.",
    "  Default is OFF.  Alias SET TELNET BINARY-XFER-MODE.",
    " ",
    "SET TELNET BUG AUTH-KRB5-DES { ON, OFF }",
    "  Default is ON.  Disable this bug to enable the use of encryption types",
    "  other than DES such as 3DES or CAST-128 when the Kerberos 5 session key",
    "  is longer than 8 bytes.",
    " ",
    "SET TELNET BUG BINARY-ME-MEANS-U-TOO { ON, OFF }",
    "  Set this to ON to try to overcome TELNET binary-mode misnegotiations by",
    "  Kermit's TELNET partner.",
    " ",
    "SET TELNET BUG BINARY-U-MEANS-ME-TOO { ON, OFF }",
    "  Set this to ON to try to overcome TELNET binary-mode misnegotiations by",
    "  Kermit's TELNET partner.",
    " ",
    "SET TELNET BUG INFINITE-LOOP-CHECK { ON, OFF }",
    "  Set this to ON to prevent Kermit from responding to a telnet "
    "negotiation",
    "  sequence that enters an infinite loop.  The default is OFF because this",
    "  should never occur.",
    " ",
    "SET TELNET BUG SB-IMPLIES-WILL-DO { ON, OFF }",
    "  Set this to ON to allow Kermit to respond to telnet sub-negotiations if",
    "  the peer forgets to respond to WILL with DO or to DO with WILL.",
    " ",
    "SET TELNET DEBUG { ON, OFF }",
    "  Set this to ON to display telnet negotiations as they are sent and",
    "  received.",
    " ",
    "SET TELNET DELAY-SB { ON, OFF }",
    "  When ON, telnet subnegotiation responses are delayed until after all",
    "  authentication and encryption options are either successfully "
    "negotiated",
    "  or refused. This ensures that private data is protected.  When OFF, "
    "telnet",
    "  subnegotiation responses are sent immediately.  The default is ON.",
    " ",
    "SET TELNET ECHO { LOCAL, REMOTE }",
    "  Kermit's initial echoing state for TELNET connections, LOCAL by "
    "default.",
    "  After the connection is made, TELNET negotiations determine the "
    "echoing.",
    " ",
#ifdef CK_ENVIRONMENT
    "SET TELNET ENVIRONMENT { variable-name [ value ] }",
    "  This feature lets Kermit send the values of certain environment "
    "variables",
    "  to the other computer if it asks for them.  The variable-name can be "
    "any",
    "  of the \"well-known\" variables \"USER\", \"JOB\", \"ACCT\", "
    "\"PRINTER\",",
    "  \"SYSTEMTYPE\", or \"DISPLAY\".  Some Telnet servers, if given a USER",
    "  value in this way, will accept it and therefore not prompt you for user",
    "  name when you log in.  The default values are taken from your "
    "environment;",
    "  use this command to change or remove them.  See RFC1572 for details.",
    " ",
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    "SET TELNET LOCATION [ text ]",
    "  Location string to send to the Telnet server if it asks.  By default "
    "this",
    "  is picked up from the LOCATION environment variable.  Give this command",
    "  with no text to disable this feature.",
    " ",
#endif /* CK_SNDLOC */
    "SET TELNET NEWLINE-MODE { NVT, BINARY-MODE } { OFF, ON, RAW }",

    "  Determines how carriage returns are handled on TELNET connections.  "
    "There",
    "  are separate settings for NVT (ASCII) mode and binary mode.  ON "
    "(default",
    "  for NVT mode) means CRLF represents CR.  OFF means CR followed by NUL",
    "  represents CR.  RAW (default for BINARY mode) means CR stands for "
    "itself.",
    " ",
#ifdef TCPSOCKET
    "SET TELNET PROMPT-FOR-USERID <prompt>",
    "  Specifies a custom prompt to be used when prompting for a userid.  "
    "Kermit",
    "  prompts for a userid if the command:",
    " ",
    "    SET LOGIN USERID {}",
    " ",
    "  has been issued prior to a Telnet authentication negotiation for an",
    "  authentication type that requires the transmission of a name, such as",
    "  Secure Remote Password.",
    " ",
#endif /* TCPSOCKET */
    "SET TELNET REMOTE-ECHO { ON, OFF }",
    "  Applies only to incoming connections created with:",
    "    SET HOST * <port> /TELNET",
    "  This command determines whether Kermit will actually echo characters",
    "  received from the remote when it has negotiated to do so.  The default",
    "  is ON.  Remote echoing may be turned off when it is necessary to read",
    "  a password with the INPUT command.",
    " ",
    "SET TELNET TERMINAL-TYPE name",
    "  The terminal type to send to the remote TELNET host.  If none is given,",
    "  your local terminal type is sent.",
    " ",
    "SET TELNET WAIT-FOR-NEGOTIATIONS { ON, OFF }",
    "  Each Telnet option must be fully negotiated either On or Off before the",
    "  session can continue.  This is especially true with options that "
    "require",
    "  subnegotiations such as Authentication, Encryption, and Kermit; for",
    "  proper support of these options Kermit must wait for the negotiations "
    "to",
    "  complete.  Of course, Kermit has no way of knowing whether a reply is",
    "  delayed or not coming at all, and so will wait a minute or more for",
    "  required replies before continuing the session.  If you know that "
    "Kermit's",
    "  Telnet partner will not be sending the required replies, you can set "
    "this",
    "  option of OFF to avoid the long timeouts.  Or you can instruct Kermit "
    "to",
    "  REFUSE specific options with the SET TELOPT command.",
    "",
    "Type SHOW TELNET to see the current values of these parameters.",
    ""};
#endif /* TNCODE */

#ifndef NOSPL
static char *hxymacr[] = {
    "Syntax: SET MACRO parameter value",
    "  Controls the behavior of macros.",
    " ",
    "SET MACRO ECHO { ON, OFF }",
    "  Tells whether commands executed from a macro definition should be",
    "  displayed on the screen.  OFF by default; use ON for debugging.",
    " ",
    "SET MACRO ERROR { ON, OFF }",
    "  Tells whether a macro should be automatically terminated upon a command",
    "  error.  This setting is local to the current macro, and inherited by",
    "  subordinate macros.",
    ""};
#endif /* NOSPL */

static char *hmxyprm[] = {
    "Syntax: SET PROMPT [ text ]",
    " ",
    "Prompt text for this program, normally 'C-Kermit>'.  May contain "
    "backslash",
    "codes for special effects.  Surround by { } to preserve leading or "
    "trailing",
    "spaces.  If text omitted, prompt reverts to C-Kermit>.  Prompt can "
    "include",
    "variables like \\v(dir) or \\v(time) to show current directory or time.",
    ""};

#ifdef UNIX
static char *hxywild[] = {
    "Syntax: SET WILDCARD-EXPANSION { KERMIT [ switch ], SHELL, ON, OFF }",
    "  KERMIT (the default) means C-Kermit expands filename wildcards in SEND "
    "and",
    "  similar commands itself, and in incoming GET commands.  Optional "
    "switches",
    "  are /NO-MATCH-DOT-FILES (\"*\" and \"?\" should not match an initial",
    "  period in a filename; this is the default) and /MATCH-DOT-FILES if you",
    "  want files whose names begin with \".\" included.  SET WILDCARD SHELL",
    "  means that Kermit asks your preferred shell to expand wildcards (this",
    "  should not be necessary in C-Kermit 7.0 and later).  HELP WILDCARD for",
    "  further information.",
    " ",
    "  The ON and OFF choices allow you to disable and renable wildcard",
    "  processing independent of the KERMIT / SHELL choice.  Disabling "
    "wildcards",
    "  allows you to process an array or list of filenames without having to",
    "  consider whether the names might contain literal wildcard characters.",
    "  WARNING: SET WILD OFF also disables internal filename pattern-matching,",
    "  used (for example) in creating backup files.",
    ""};
#else
#endif /* UNIX */

#ifndef NOXFER
static char *hxywind[] = {
    "Syntax: SET WINDOW-SIZE number",
    "  Specifies number of slots for sliding windows, i.e. the number of "
    "packets",
    "  that can be transmitted before waiting for acknowledgement.  The "
    "default",
#ifdef XYZ_INTERNAL
    "  for Kermit protocol is one, the maximum is 32; for ZMODEM, the default",
    "  is no windowing (0).  For ZMODEM, the window size is really the packet",
    "  length, and is used only when non-windowed (streaming) transfers fail; "
    "the",
    "  ZMODEM window size should be a largish number, like 1024, and it should "
    "be",
    "  a multiple of 64.",
#else
    "  is one, the maximum is 32.  Increased window size might result in "
    "reduced",
    "  maximum packet length.  Use sliding windows for improved efficiency on",
    "  connections with long delays.  A full duplex connection is required, as",
    "  well as a cooperating Kermit on the other end.",
#endif /* XYZ_INTERNAL */
    ""};

static char *hxyrpt[] = {
    "Syntax: SET REPEAT { COUNTS { ON, OFF }, PREFIX <code> }",
    "  SET REPEAT COUNTS turns the repeat-count compression mechanism ON and "
    "OFF.",
    "  The default is ON.  SET REPEAT PREFIX <code> sets the repeat-count "
    "prefix",
    "  character to the given code.  The default is 126 (tilde).", ""};

static char *hxyrcv[] = {
    "Syntax: SET RECEIVE parameter value",
    "  Specifies parameters for inbound packets:",
    " ",
#ifndef NOCSETS
    "SET RECEIVE CHARACTER-SET { AUTOMATIC, MANUAL }",
    "  Whether to automatically switch to an appropriate file-character set "
    "based",
    "  on the transfer character-set announcer, if any, of an incoming text "
    "file.",
    "  AUTOMATIC by default.  Also see HELP ASSOCIATE.",
    " ",
#endif /* NOCSETS */
    "SET RECEIVE CONTROL-PREFIX number",
    "  ASCII value of prefix character used for quoting control characters in",
    "  packets that Kermit receives, normally 35 (number sign).  Don't change",
    "  this unless something is wrong with the other Kermit program.",
    " ",
    "SET RECEIVE END-OF-PACKET number",
    "  ASCII value of control character that terminates incoming packets,",
    "  normally 13 (carriage return).",
    " ",
#ifdef CKXXCHAR
    "SET RECEIVE IGNORE-CHARACTER number",
    "  ASCII value of character to be discarded when receiving packets, such "
    "as",
    "  line folding characters.",
    " ",
#endif /* CKXXCHAR */
    "SET RECEIVE MOVE-TO [ directory ]",
    "  If a directory name is specified, then every file that is received",
    "  successfully is moved to the given directory immediately after "
    "reception",
    "  is complete.  Omit the directory name to remove any previously set "
    "move-to",
    "  directory.",
    " ",
    "SET RECEIVE PACKET-LENGTH number",
    "  Maximum length packet the other Kermit should send.",
    " ",
    "SET RECEIVE PADDING number",
    "  Number of prepacket padding characters to ask for (normally 0).",
    " ",
    "SET RECEIVE PAD-CHARACTER number",
    "  ASCII value of control character to use for padding (normally 0).",
    " ",
    "SET RECEIVE PATHNAMES {OFF, ABSOLUTE, RELATIVE, AUTO}",
    "  If a recognizable path (directory, device) specification appears in an",
    "  incoming filename, strip it OFF before trying to create the output "
    "file.",
#ifdef CK_MKDIR
    "  Otherwise, then if any of the directories in the path don't exist, "
    "Kermit",
    "  tries to create them, relative to your current or download directory, "
    "or",
    "  absolutely, as specified.  RELATIVE means force all incoming names, "
    "even",
    "  if they are absolute, to be relative to your current or download "
    "directory.",
    "  AUTO, which is the default, means RELATIVE if the file sender indicates "
    "in",
    "  advance that this is a recursive transfer, otherwise OFF.",
#endif /* CK_MKDIR */
    " ",
    "SET RECEIVE PAUSE number",
    "  Milliseconds to pause between packets, normally 0.",
    " ",

#ifdef CK_PERMS
    "SET RECEIVE PERMISSIONS { ON, OFF }",
    "  Whether to copy file permissions from inbound Attribute packets.",
    " ",
#endif /* CK_PERMS */

    "SET RECEIVE RENAME-TO [ template ]",
    "  If a template is specified, then every file that is received "
    "successfully",
    "  \
is renamed according to the given template immediately after it is received.",
    "  \
The template should include variables like \\v(filename) or \\v(filenumber).",
    "  Omit the template to remove any template previously set.",
    " ",
    "SET RECEIVE START-OF-PACKET number",
    "  ASCII value of character that marks start of inbound packet.",
    " ",
    "SET RECEIVE TIMEOUT number",
    "  Number of seconds the other Kermit should wait for a packet before "
    "sending",
    "  a NAK or retransmitting.",
    ""};

static char *hxysnd[] = {
    "Syntax: SET SEND parameter value",
    "  Specifies parameters for outbound files or packets.",
    " ",
    "SET SEND BACKUP { ON, OFF }",
    "  Tells whether to include backup files when sending file groups.  Backup",
    "  files are those created by Kermit, EMACS, etc, when creating a new file",
    "  that has the same name as an existing file.  A backup file has a "
    "version",
    "  appended to its name, e.g. oofa.txt.~23~.  ON is the default, meaning",
    "  don't exclude backup files.  Use OFF to exclude backup files from group",
    "  transfers.",
    " ",
#ifndef NOCSETS
    "SET SEND CHARACTER-SET { AUTOMATIC, MANUAL }",
    "  Whether to automatically switch to an appropriate file-character when a",
    "  SET TRANSFER CHARACTER-SET command is given, or vice versa.  AUTOMATIC "
    "by",
    "  default.  Also see HELP ASSOCIATE.",
    " ",
#endif /* NOCSETS */

    "SET SEND CONTROL-PREFIX number",
    "  ASCII value of prefix character used for quoting control characters in",
    "  packets that Kermit sends, normally 35 (number sign).",
    " ",
#ifdef CKXXCHAR
    "SET SEND DOUBLE-CHARACTER number",
    "  ASCII value of character to be doubled when sending packets, such as an",
    "  X.25 PAD escape character.",
    " ",
#endif /* CKXXCHAR */
    "SET SEND END-OF-PACKET number",
    "  ASCII value of control character to terminate an outbound packet,",
    "  normally 13 (carriage return).",
    " ",
    "SET SEND MOVE-TO [ directory ]",
    "  \
If a directory name is specified, then every file that is sent successfully",
    "  is moved to the given directory immediately after it is sent.",
    "  Omit the directory name to remove any previously set move-to directory.",
    " ",
    "SET SEND PACKET-LENGTH number",
    "  Maximum length packet to send, even if other Kermit asks for longer "
    "ones.",
    "  This command can not be used to force packets to be sent that are "
    "longer",
    "  than the length requested by the receiver.  Use this command only to",
    "  force shorter ones.",
    " ",
    "SET SEND PADDING number",
    "  Number of prepacket padding characters to send.",
    " ",
    "SET SEND PAD-CHARACTER number",
    "  ASCII value of control character to use for padding.",
    " ",
    "SET SEND PATHNAMES {OFF, ABSOLUTE, RELATIVE}",
    "  Include the path (device, directory) portion with the file name when",
    "  sending it as specified; ABSOLUTE means to send the whole pathname,",
    "  RELATIVE means to include the pathname relative to the current "
    "directory.",
    "  Applies to the actual filename, not to the \"as-name\".  The default is",
    "  OFF.",
    " ",
    "SET SEND PAUSE number",
    "  Milliseconds to pause between packets, normally 0.",
    " ",

#ifdef CK_PERMS
    "SET SEND PERMISSIONS { ON, OFF }",
    "  Whether to include file permissions in outbound Attribute packets.",
    " ",
#endif /* CK_PERMS */

    "SET SEND RENAME-TO [ template ]",
    "  If a template is specified, then every file that is sent successfully",
    "  is renamed according to the given template immediately after it is "
    "sent.",
    "  \
The template should include variables like \\v(filename) or \\v(filenumber).",
    "  Omit the template to remove any template previously set.",
    " ",
    "SET SEND START-OF-PACKET number",
    "  ASCII value of character to mark start of outbound packet.",
    " ",
#ifdef CK_TIMERS
    "SET SEND TIMEOUT number [ { DYNAMIC [ min max ] ], FIXED } ]",
#else
    "SET SEND TIMEOUT number",
#endif /* CK_TIMERS */

    "  Number of seconds to wait for a packet before sending NAK or",
#ifdef CK_TIMERS
    "  retransmitting.  Include the word DYNAMIC after the number in the",
    "  SET SEND TIMEOUT command to have Kermit compute the timeouts "
    "dynamically",
    "  throughout the transfer based on the packet rate.  Include the word "
    "FIXED",
    "  to use the \"number\" given throughout the transfer.  DYNAMIC is the",
    "  default.  After DYNAMIC you may include minimum and maximum values.",
    "  SET SEND TIMEOUT -1 FIXED means no timeouts.",
#else
    "  retransmitting.",
#endif /* CK_TIMERS */
    ""};

static char *hxyxfer[] = {
    "Syntax: SET TRANSFER (or XFER) parameter value",
    " ",
    "Choices:",
    " ",
    "SET TRANSFER BELL { OFF, ON }",
    "  Whether to ring the terminal bell at the end of a file transfer.",
    " ",
#ifdef XFRCAN
    "SET TRANSFER CANCELLATION { OFF, ON [ <code> [ <number> ] ] }",
    "  OFF disables remote-mode packet-mode cancellation from the keyboard.",
    "  ON enables it.  The optional <code> is the control character to use for",
    "  cancellation; the optional <number> is how many consecutive occurrences",
    "  of the given control character are required for cancellation.",
    " ",
#endif /* XFRCAN */
    "SET TRANSFER INTERRUPTION { ON, OFF }",
    "  TRANSFER INTERRUPTION is normally ON, allowing for interruption of a "
    "file",
    "  transfer in progress by typing certain characters while the "
    "file-transfer",
    "  display is active.  SET TRANSFER INTERRUPTION OFF disables interruption",
    "  of file transfer from the keyboard in local mode.",
    " ",
#ifndef NOSPL
    "SET TRANSFER CRC-CALCULATION { OFF, ON }",
    "  Tells whether Kermit should accumulate a Cyclic Redundancy Check for ",
    "  each file transfer.  Normally ON, in which case the CRC value is "
    "available",
    "  in the \\v(crc16) variable after the transfer.  Adds some overhead.  "
    "Use",
    "  SET TRANSFER CRC OFF to disable.",
    " ",
#endif /* NOSPL */
#ifndef NOCSETS
    "SET TRANSFER CHARACTER-SET name",
    "  Selects the character set used to represent textual data in Kermit",
    "  packets.  Text characters are translated to/from the FILE "
    "CHARACTER-SET.",
    "  Choices:",
    " ",
    "  TRANSPARENT (no translation, the default)",
    "  ASCII",
    "  LATIN1 (ISO 8859-1 Latin Alphabet 1)",
#ifndef NOLATIN2
    "  LATIN2 (ISO 8859-2 Latin Alphabet 2)",
#endif /* NOLATIN2 */
    "  LATIN9 (ISO 8859-15 Latin Alphabet 9)",
#ifdef CYRILLIC
    "  CYRILLIC-ISO (ISO 8859-5 Latin/Cyrillic)",
#endif /* CYRILLIC */
#ifdef GREEK
    "  GREEK-ISO (ISO 8859-7 Latin/Greek)",
#endif /* GREEK */
#ifdef HEBREW
    "  HEBREW-ISO (ISO 8859-8 Latin/Hebrew)",
#endif /* HEBREW */
#ifdef KANJI
    "  JAPANESE-EUC (JIS X 0208 Kanji + Roman and Katakana)",
#endif /* KANJI */
#ifdef UNICODE
    "  UCS-2 (ISO 10646 / Unicode 2-byte form)",
    "  UTF-8 (ISO 10646 / Unicode 8-bit serialized transformation format)",
#endif /* UNICODE */
    " ",
    "SET TRANSFER TRANSLATION { ON, OFF }",
    "  Enables and disables file-transfer character-set translation.  It's",
    "  enabled by default.",
#endif /* NOCSETS */
    " ",
#ifdef CK_CURSES
    "SET TRANSFER DISPLAY { BRIEF, CRT, FULLSCREEN, NONE, SERIAL }",
#else
    "SET TRANSFER DISPLAY { BRIEF, CRT, NONE, SERIAL }",
#endif /* CK_CURSES */
    "  Choose the desired format for the progress report to be displayed on",
    "  your screen during file transfers when Kermit is in local mode.",
#ifdef CK_CURSES
    "  FULLSCREEN requires your terminal type be set correctly; the others",
    "  are independent of terminal type.",
#else
    "  file transfer.",
#endif /* CK_CURSES */
    " ",
    "SET TRANSFER LOCKING-SHIFT { OFF, ON, FORCED }",
    "  Tell whether locking-shift protocol should be used during file transfer",
    "  to achieve 8-bit transparency on a 7-bit connection.  ON means to "
    "request",
    "  its use if PARITY is not NONE and to use it if the other Kermit agrees,",
    "  OFF means not to use it, FORCED means to use it even if the other "
    "Kermit",
    "  does not agree.",
    " ",
    "SET TRANSFER MODE { AUTOMATIC, MANUAL }",
    "  Automatic (the default) means Kermit should automatically go into "
    "binary",
    "  file-transfer mode and use literal filenames if the other Kermit says "
    "it",
    "  has a compatible file system, e.g. UNIX-to-UNIX, but not UNIX-to-DOS.",
#ifdef PATTERNS
    "  Also, when sending files, Kermit should switch between binary and text",
    "  mode automatically per file based on the SET FILE BINARY-PATTERNS and "
    "SET",
    "  FILE TEXT-PATTERNS.",
#endif /* PATTERNS */
    " ",
#ifdef PIPESEND
    "SET TRANSFER PIPES { ON, OFF }",
    "  Enables/Disables automatic sending from / reception to command pipes "
    "when",
    "  the incoming filename starts with '!'.  Also see CSEND, CRECEIVE.",
    " ",
#endif /* PIPESEND */
#ifdef CK_XYZ
    "SET TRANSFER PROTOCOL { KERMIT, XMODEM, ... }",
    "  Synonym for SET PROTOCOL (q.v.).",
    " ",
#endif /* CK_XYZ */
    "SET TRANSFER REPORT { ON, OFF }",
    "  Enables/Disables the automatic post-transfer message telling what files",
    "  went where from C-Kermit when it is in remote mode.  ON by default.",
    " ",
    "SET TRANSFER SLOW-START { OFF, ON }",
    "  ON (the default) tells Kermit, when sending files, to gradually build "
    "up",
    "  the packet length to the maximum negotiated length.  OFF means start",
    "  sending the maximum length right away.",
    " ",
    "Synonym: SET XFER.  Use SHOW TRANSFER (XFER) to see SET TRANSFER values.",
    ""};
#endif /* NOXFER */

static char *hmxybel[] = {"Syntax: SET BELL { OFF, ON }",
                          "  ON (the default) enables ringing of the terminal "
                          "bell (beep) except where",
                          "  it is disabled in certain circumstances, e.g. by "
                          "SET TRANSFER BELL.  OFF",
                          "  disables ringing of the bell in all "
                          "circumstances, overriding any specific",
                          "  SET xxx BELL selections.",
                          ""};

static char *hmxycd[] = {
    "Syntax: SET CD { HOME <path>, PATH <path>, MESSAGE { ON, OFF, FILE <list> "
    "} }",
    " ",
    "SET CD HOME <path>",
    "  Specified which directory to change to if CD or KCD is given without a",
    "  pathname.  If this command is not given, your login or HOME directory "
    "is",
    "  used.",
    " ",
    "SET CD PATH <path>",
    "  Overrides normal CDPATH environment variable, which tells the CD "
    "command",
    "  where to look for directories to CD to if you don't specify them fully.",
    "  The format is:",
    " ",
    "    set cd path :directory:directory:...",
    " ",
    "  in other words, a list of directories separated by colons, with a colon",
    "  at the beginning, e.g.:",
    " ",
    "    set cd path :/usr/olga:/usr/ivan/public:/tmp",
    " ",
    "SET CD MESSAGE { ON, OFF }",
    "  Default is OFF.  When ON, this tells Kermit to look for a file with a",
    "  certain name in any directory that you CD to, and if it finds one, to",
    "  display it on your screen when you give the CD command.  The filename,",
    "  or list of names, is given in the SET CD MESSAGE FILE command.",
    " ",
    "SET CD MESSAGE FILE name",
    "  or:",
    "SET CD MESSAGE FILE {{name1}{name2}...{name8}}",
    "  Specify up to 8 filenames to look for when when CDing to a new "
    "directory",
    "  and CD MESSAGE is ON.  The first one found, if any, in the new "
    "directory",
#ifndef DFCDMSG
    "  is displayed.",
#else
    "  is displayed.  The default list is:",
    " ",
    "   {{./.readme}{README.TXT}{READ.ME}}",
    " ",
#endif /* DFCDMSG */
#ifndef NOSERVER
    "Synonym: SET SERVER CD-MESSAGE FILE.",
#endif /* NOSERVER */
    " ",
    "Type SHOW CD to view current CD settings.  Also see HELP SET SERVER.",
    ""};

#ifndef NOIKSD
static char *hsetiks[] = {
    "SET IKS ANONYMOUS INITFILE filename",
    "  The initialization file to be executed for anonymous logins.  By "
    "default",
    "  it is .kermrc in the anonymous root directory.  This option is "
    "independent",
    "  of the SET IKS INITFILE command which applies only to real users.",
    " ",
    "SET IKS ANONYMOUS LOGIN { ON, OFF }",
    "  Whether anonymous logins are allowed. By default they are allowed, so "
    "this",
    "  option need be included only to disallow them (or for clarity, to "
    "emphasize",
    "  they are allowed). Anonymous login occurs when the username "
    "\"anonymous\"",
    "  or \"ftp\" is given, with any password (as with ftpd).",
    " ",
    "SET IKS ANONYMOUS ROOT <directory>",
    "  Specifies a directory tree to which anonymous users are restricted "
    "after",
    "  login.",
    " ",
    "SET IKS BANNERFILE <filename>",
    "  The name of a file containing a message to be printed after the user "
    "logs",
    "  in, in place of the normal message (copyright notice, \"Type HELP or ? "
    "for",
    "  help\", etc).",
    " ",
    "SET IKS CDFILE <filelist>",
    "  When cdmessage is on, this is the name of the \"read me\" file to be "
    "shown.",
    "  Normally you would specify a relative (not absolute) name, since the "
    "file",
    "  is opened using the literal name you specified, after changing to the "
    "new",
    "  directory.  Example:",
    " ",
    "    SET IKS CDFILE READ.ME",
    " ",
    "  You can also give a list of up to 8 filenames by (a) enclosing each",
    "  filename in braces, and (b) enclosing the entire list in braces.  "
    "Example:",
    " ",
    "    SET IKS CDFILE {{READ.ME}{aareadme.txt}{README}{read-this-first}}",
    " ",
    "  When a list is given, it is searched from left to right and the first",
    "  file found is displayed.",
    " ",
    "SET IKS CDMESSAGE {ON, OFF, 0, 1, 2}",
    "  For use in the Server-Side Server configuration; whenever the client",
    "  tells the server to change directory, the server sends the contents of "
    "a",
    "  \"read me\" file to the client's screen.  This feature is ON by "
    "default,",
    "  and operates in client/server mode only when ON or 1.  If set to 2 or",
    "  higher, it also operates when the CD command is given at the IKSD> "
    "prompt.",
    "  Synonym: SET IKS CDMSG.",
    " ",
    "SET IKS DATABASE { ON, OFF }",
    "  This command determines whether entries are inserted into the SET IKS",
    "  DBFILE (IKSD active sessions database).",
    " ",
    "SET IKS DBFILE <filename>",
    "  Specifies the file which should be used for storing runtime status",
#ifdef UNIX
    "  information about active connections.  The default is a file called",
    "  \"iksd.db\" in the /var/log directory.",
#else
    "  information about active connections.",
#endif /* UNIX */
    " ",
    " ",
    "SET IKS HELPFILE <filename>",
    "  Specifies the name of a file to be displayed if the user types HELP",
    "  (not followed by a specific command or topic), in place of the built-in",
    "  top-level help text.  The file need not fit on one screen; "
    "more-prompting",
    "  is used if the file is more than one screen long if COMMAND "
    "MORE-PROMPTING",
    "  is ON, as it is by default.",
    " ",
    "SET IKS INITFILE <filename>",
    "  Execute <filename> rather than the normal initialization file for real",
    "  users; this option does not apply to anonymous users.",
    " ",
    "SET IKS NO-INITFILE { ON, OFF }",
    "  Do not execute an initialization file, even if a real user is logging "
    "in.",
    " ",
    "SET IKS SERVER-ONLY { ON, OFF }",
    "  If this option is included on the IKSD command line, the Client Side "
    "Server",
    "  configuration is disabled, and the user will not get a Username: or",
    "  Password: prompt, and will not be able to access the IKSD command "
    "prompt.",
    "  A FINISH command sent to the IKSD will log it out and close the",
    "  connection, rather than returning it to its prompt.",
    " ",
    "SET IKS TIMEOUT <number>",
    "  This sets a limit (in seconds) on the amount of time the client has to "
    "log",
    "  in once the connection is made.  If successful login does not occur "
    "within",
    "  the given number of seconds, the connection is closed.  The default "
    "timeout",
    "  is 300 seconds (5 minutes).  A value of 0 or less indicates there is to "
    "be",
    "  no limit.",
    " ",
    "SET IKS USERFILE <filename>",
#ifdef UNIX
    "  This file contains a list of local usernames that are to be denied "
    "access",
    "  to Internet Kermit Service.  The default is /etc/ftpusers.  This can be "
    "the",
    "  same file that is used by wuftpd, and the syntax is the same: one "
    "username",
    "  per line; lines starting with \"#\" are ignored.  Use this option to",
    "  specify the name of a different forbidden-user file, or use",
    "  \"set iks userfile /dev/null\" to disable this feature in case there is "
    "a",
    "   /etc/ftpusers file but you don't want to use it.",
#else
    "  This file contains a list of local usernames that are to be denied "
    "access",
    "  to Internet Kermit Service.  The syntax is: one username per line; "
    "lines",
    "  starting with \"#\" are ignored.",
#endif /* UNIX */
    " ",
    "SET IKS XFERLOG { ON, OFF }",
#ifdef UNIX
    "  Whether a file-transfer log should be kept.  Off by default.  If "
    "\"on\",",
    "  but no SET IKSD XFERFILE command is given, /var/log/iksd.log is used.",
#else
    "  Whether a file-transfer log should be kept.  Off by default.",
#endif /* UNIX */
    " ",
    "SET IKS XFERFILE <filename>",
    "  Use this option to specify an iksd log file name.  If you include this",
    "  option, it implies SET IKS XFERFILE ON.",
    ""};
#endif /* NOIKSD */

/*  D O H S E T  --  Give help for SET command  */

int dohset(int xx) {
  int x;

  if (xx == -3)
    return (hmsga(hmhset));
  if (xx < 0)
    return (xx);

#ifdef NEWFTP
  if (xx == XYFTPX)
    return (dosetftphlp());
  if (xx == XYGPR)
    return (hmsga(hmxygpr));
#endif /* NEWFTP */

  if ((x = cmcfm()) < 0)
    return (x);
  switch (xx) {
#ifndef NOIKSD
  case XYIKS:
    return (hmsga(hsetiks));
#endif /* NOIKSD */

  case XY_REN:
    return (hmsg("SET RENAME LIST { ON, OFF }\n\
  Tells whether the RENAME command should list its results by default.\n\n\
SET RENAME COLLISION { FAIL, PROCEED, OVERWRITE }\n\
  Establishes the default action when renaming a file would destroy an\n\
  existing file.  See HELP RENAME."));

  case XYATTR:
    return (hmsga(hsetat));

  case XYBACK:
    return (hmsga(hsetbkg));

  case XYBELL:
    return (hmsga(hmxybel));

#ifdef DYNAMIC
  case XYBUF:
    return (hmsga(hsetbuf));
#endif /* DYNAMIC */

#ifndef NOLOCAL
  case XYCARR:
    return (hmsga(hsetcar));
#endif /* NOLOCAL */

#ifndef NOSPL
  case XYCASE:
    return (hmsg("Syntax: SET CASE { ON, OFF }\n\
  Tells whether alphabetic case is significant in string comparisons\n\
  done by INPUT, IF, and other commands.  This setting is local to the\n\
  current macro or command file, and inherited by subordinates."));

#endif /* NOSPL */

  case XYCMD:
    return (hmsga(hsetcmd));

  case XYIFD:
    return (hmsg("Syntax: SET INCOMPLETE { DISCARD, KEEP }\n\
  Whether to discard or keep incompletely received files, default is KEEP."));

#ifndef NOSPL
  case XYINPU:
    return (hmsga(hxyinp));
#endif /* NOSPL */

  case XYCHKT:
    return (hmsga(hmxychkt));

#ifndef NOSPL
  case XYCOUN:
    return (hmsg("Syntax:  SET COUNT number\n\
 Example: SET COUNT 5\n\
  Set up a loop counter, for use with IF COUNT.  Local to current macro\n\
  or command file, inherited by subordinate macros and command files."));
#endif /* NOSPL */

  case XYDEBU:
    return (hmsga(hmxydeb));

  case XYDFLT:
    return (hmsg("Syntax: SET DEFAULT directory\n\
  Change directory.  Equivalent to CD command."));

  case XYDELA:
    return (hmsg("Syntax: SET DELAY number\n\
  Number of seconds to wait before sending first packet after SEND command."));

#ifndef NODIAL
  case XYDIAL:
    return (hmsga(hmxydial));
#endif /* NODIAL */

#ifdef UNIX
  case XYSUSP:
    return (hmsg("Syntax: SET SUSPEND { OFF, ON }\n\
  Disables SUSPEND command, suspend signals, and <esc-char>Z during CONNECT."));
#endif

#ifndef NOSCRIPT
  case XYSCRI:
    return (hmsg("Syntax: SET SCRIPT ECHO { OFF, ON }\n\
  Disables/Enables echoing of SCRIPT command operation."));
#endif /* NOSCRIPT */

  case XYTAKE:
    return (hmsga(hxytak));

#ifndef NOLOCAL
  case XYTERM:
    return (hmsga(hxyterm));

  case XYDUPL:
    return (hmsg("Syntax: SET DUPLEX { FULL, HALF }\n\
  During CONNECT: FULL means remote host echoes, HALF means Kermit\n\
  does its own echoing."));

  case XYLCLE:
    return (hmsg("Syntax: SET LOCAL-ECHO { OFF, ON }\n\
  During CONNECT: OFF means remote host echoes, ON means Kermit\n\
  does its own echoing.  Synonym for SET DUPLEX { FULL, HALF }."));

  case XYESC:
    return (hmsga(hxyesc)); /* SET ESCAPE */
#endif /* NOLOCAL */

  case XYPRTR: /* SET PRINTER */
    return (hmsga(hxyprtr));

  case XYEXIT:
    return (hmsga(hxyexit));

  case XYFILE:
    return (hmsga(hmxyf));

  case XYFLOW:
    return (hmsga(hmxyflo));

  case XYHAND:
    return (hmsga(hmxyhsh));

#ifdef NETCONN

  case XYHOST:
    return (hmsga(hxyhost));

  case XYNET:
    return (hmsga(hxynet));

#ifndef NOTCPOPTS
#ifdef SOL_SOCKET
  case XYTCP:
    return (hmsga(hxytcp));
#endif /* SOL_SOCKET */
#endif /* NOTCPOPTS */

#ifdef ANYX25
  case XYX25:
    return (hmsga(hxyx25));

#ifndef IBMX25
  case XYPAD:
    return (hmsg("Syntax: SET PAD name value\n\
Set a PAD X.3 parameter with a desired value."));
#endif /* IBMX25 */
#endif /* ANYX25 */
#endif /* NETCONN */

#ifndef NOSPL
  case XYOUTP:
    return (hmsga(hxyout));
#endif /* NOSPL */

#ifndef NOSETKEY
  case XYKEY: /* SET KEY */
    return (hmsga(hmhskey));
#endif /* NOSETKEY */

#ifndef NOCSETS
  case XYLANG:
    return (hmsg("Syntax: SET LANGUAGE name\n\
  Selects language-specific translation rules for text-mode file transfers.\n\
  Used with SET FILE CHARACTER-SET and SET TRANSFER CHARACTER-SET when one\n\
  of these is ASCII."));
#endif /* NOCSETS */

  case XYLINE:
    printf("\nSyntax: SET LINE (or SET PORT) [ switches ] [ devicename ]\n\
  Selects a serial-port device to use for making connections.\n");
    printf("  Typical device name for this platform: %s.\n", ttgtpn());
    printf("  The default device name is %s (i.e. none).\n", dftty);
    if (!dfloc) {
      printf("  If you do not give a SET LINE command or if you give a SET "
             "LINE command\n");
      printf(
          "  with no device name, or if you specify %s as the device name,\n",
          dftty);
      printf("  Kermit will be in \"remote mode\", suitable for use on the far "
             "end of a\n");
      printf(
          "  connection, e.g. as the file-transfer partner of your desktop communication\
\n");
      printf(
          "  software.  If you SET LINE to a specific device other than %s,\n",
          dftty);
      printf("  Kermit is in \"local mode\", suitable for making a connection "
             "to another\n");
      printf("  computer.  SET LINE alone resets Kermit to remote mode.\n");
    }
    printf("  To use a modem to dial out, first SET MODEM TYPE (e.g., to USR), "
           "then\n");
    printf("  SET LINE xxx, then SET SPEED, then give a DIAL command.\n");
    printf("  For direct null-modem connections, use SET MODEM TYPE NONE, SET "
           "LINE xxx,\n");
    printf("  then SET FLOW, SET SPEED, and CONNECT.\n");
    printf("\nOptional switches:\n\
  /CONNECT - Enter CONNECT mode automatically if SET LINE succeeds.\n");
    printf(
        "  /SERVER  - Enter server mode automatically if SET LINE succeeds.\n");
    printf("\n");
    printf("Also see HELP SET MODEM, HELP SET DIAL, HELP SET SPEED, HELP SET "
           "FLOW.\n");
    return (0);

#ifndef NOSPL
  case XYMACR:
    return (hmsga(hxymacr));
#endif /* NOSPL */

#ifndef NODIAL
  case XYMODM:
    return (hmsga(hxymodm));
#endif /* NODIAL */

  case XYPARI:
    return (hmsga(hxypari));

  case XYPROM:
    return (hmsga(hmxyprm));

  case XYQUIE:
    return (hmsg("Syntax: SET QUIET {ON, OFF}\n\
  Normally OFF.  ON disables most information messages during interactive\n\
  operation."));

#ifdef CK_SPEED
  case XYQCTL:
    return (hmsga(hmxyqctl));
#endif /* CK_SPEED */

  case XYRETR:
    return (hmsg("Syntax: SET RETRY number\n\
  In Kermit protocol file transfers: How many times to retransmit a\n\
  particular packet before giving up; 0 = no limit."));

#ifndef NOLOCAL
  case XYSESS:
#ifdef UNIX
    return (hmsg("Syntax:\n\
 SET SESSION-LOG { BINARY, DEBUG, NULL-PADDED, TEXT, TIMESTAMPED-TEXT }\n\
  If BINARY, record all CONNECT characters in session log.  If TEXT, strip\n\
  out CR, NUL, and XON/XOFF characters.  DEBUG is the same as BINARY but\n\
  also includes Telnet negotiations on TCP/IP connections."));
#else
#endif /* OS2ORUNIX */

  case XYSPEE:
    return (hmsg("Syntax: SET SPEED number\n\
  Speed for serial-port communication device specified in most recent\n\
  SET LINE command, in bits per second.  Type SET SPEED ? for a list of\n\
  possible speeds.  Some of the speeds shown might not be supported on the\n\
  computer you are using.  Has no effect on job's controlling terminal."));
#endif /* NOLOCAL */

#ifndef NOXFER
  case XYRECV:
    return (hmsga(hxyrcv));
  case XYSEND:
    return (hmsga(hxysnd));
  case XYREPT:
    return (hmsga(hxyrpt));
#endif /* NOXFER */

#ifndef NOSERVER
  case XYSERV:
    return (hmsga(hsetsrv));
#endif /* NOSERVER */

#ifdef TNCODE
  case XYTEL:
    return (hmsga(hxytel));

  case XYTELOP:
    return (hmsga(hxytopt));
#endif /* TNCODE */

#ifndef NOXMIT
  case XYXMIT:
    return (hmsga(hsetxmit));
#endif /* NOXMIT */

#ifndef NOCSETS
  case XYUNCS:
    return (hmsg("Syntax: SET UNKNOWN-CHAR-SET action\n\
  DISCARD (default) means reject any arriving files encoded in unknown\n\
  character sets.  KEEP means to accept them anyway."));
#endif /* NOCSETS */

#ifdef UNIX
  case XYWILD:
    return (hmsga(hxywild));
#endif /* UNIX */

#ifndef NOXFER
  case XYWIND:
    return (hmsga(hxywind));
  case XYXFER:
    return (hmsga(hxyxfer));
#endif /* NOXFER */

#ifndef NOLOCAL
#endif /* NOLOCAL */

  case XYALRM:
    return (hmsg("Syntax: SET ALARM [ { seconds, hh:mm:ss } ]\n\
  Number of seconds from now, or time of day, after which IF ALARM\n\
  will succeed.  0, or no time at all, means no alarm."));

  case XYPROTO:
    return (hmsga(hxyxyz));

#ifdef CK_SPEED
  case XYPREFIX:
    return (hmsg("Syntax: SET PREFIXING { ALL, CAUTIOUS, MINIMAL }\n\
  \
Selects the degree of control-character prefixing.  Also see HELP SET CONTROL."));
#endif /* CK_SPEED */

#ifndef NOSPL
  case XYTMPDIR:
    return (hmsg("Syntax: SET TEMP-DIRECTORY [ <directory-name> ]\n\
  Tells Kermit to use the given directory for creating temporary files.\n\
  These are used (for example) in FTP downloads and by the CHANGE command.\n\
  If you don't issue this command, C-Kermit picks a directory automatically\n\
  based on the operating system and any environment variables you might have\n\
  set.  Use SHOW TEMP-DIRECTORY or SHOW VARIABLE \\v(tmpdir) to see Kermit's\n\
  current temporary directory setting.  Synonym: SET TMP-DIRECTORY."));
#endif /* NOSPL */

#ifndef NOPUSH
#ifndef NOFRILLS
  case XYEDIT:
    return (hmsg("Syntax: SET EDITOR pathname [ options ]\n\
  Specifies the name of your preferred editor, plus any command-line\n\
  options.  SHOW EDITOR displays it."));
#endif /* NOFRILLS */
#endif /* NOPUSH */

#ifdef BROWSER
  case XYBROWSE:
    return (hmsg("Syntax: SET BROWSER [ pathname [ options ] ]\n\
  Specifies the name of your preferred browser plus any command-line\n\
  options.  SHOW BROWSER displays it."));
#endif /* BROWSER */

#ifdef CK_TAPI
  case XYTAPI:
    return (hmsga(hxytapi));
#endif /* CK_TAPI */

#ifndef NOSPL
  case XYFUNC:
    return (hmsga(hxyfunc));
#endif /* NOSPL */

#ifdef BROWSER
  case XYFTP:
    return (hmsg("Syntax: SET FTP [ pathname [ options ] ]\n\
  Specifies the name of your ftp client, plus any command-line options.\n\
  SHOW FTP displays it."));
#endif /* BROWSER */

  case XYSLEEP:
    return (hmsg("Syntax: SET SLEEP CANCELLATION { ON, OFF }\n\
  Tells whether SLEEP (PAUSE) or WAIT commands can be interrupted from the\n\
  keyboard; ON by default."));

  case XYCD:
    return (hmsga(hmxycd));

  case XYSERIAL:
    return (hmsg("Syntax: SET SERIAL dps\n\
  d is data length in bits, 7 or 8; p is first letter of parity; s is stop\n\
  bits, 1 or 2.  Examples: \"set serial 7e1\", \"set serial 8n1\"."));

#ifdef HWPARITY
  case XYSTOP:
    return (hmsg("Syntax: SET STOP-BITS { 1, 2 }\n\
  Number of stop bits to use on SET LINE connections, normally 1."));
#endif /* HWPARITY */

#ifndef NOLOCAL
  case XYDISC:
    return (hmsg("Syntax: SET DISCONNECT { ON, OFF }\n\
  Whether to close and release a SET LINE device automatically upon\n\
  disconnection; OFF = keep device open (default); ON = close and release."));
#endif /* NOLOCAL */

#ifdef STREAMING
  case XYSTREAM:
    return (hmsg("Syntax: SET STREAMING { ON, OFF, AUTO }\n\
  Tells Kermit whether streaming protocol can be used during Kermit file\n\
  transfers.  Default is AUTO, meaning use it if connection is reliable."));
#endif /* STREAMING */

  case XYRELY:
    return (hmsg("Syntax: SET RELIABLE { ON, OFF, AUTO }\n\
  Tells Kermit whether its connection is reliable.  Default is AUTO,\n\
  meaning Kermit should figure it out for itself."));

  case XYCLEAR:
    return (hmsg("Syntax: SET CLEAR-CHANNEL { ON, OFF, AUTO }\n\
  Tells Kermit whether its connection is transparent to all 8-bit bytes.\n\
  Default is AUTO, meaning Kermit figures it out from the connection type.\n\
  When both sender and receiver agree channel is clear, SET PREFIXING NONE\n\
  is used automatically."));

#ifdef TLOG
  case XYTLOG:
    return (hmsg("Syntax: SET TRANSACTION-LOG { BRIEF, FTP, VERBOSE }\n\
  Selects the transaction-log format; BRIEF and FTP have one line per file;\n\
  FTP is compatible with FTP log.  VERBOSE (the default) has more info."));
#endif /* TLOG */

  case XYOPTS:
    return (hmsg("Syntax: SET OPTIONS command [ switches... ]\n\
  For use with commands that have switches; sets the default switches for\n\
  the given command.  Type SET OPTIONS ? for a list of amenable commands."));

#ifndef NOSPL
  case XYTIMER:
    return (hmsga(hmxytimer));
#endif /* NOSPL */

#ifdef CKROOT
  case XYROOT:
    return (hmsga(hmxxchroot));
#endif /* XYROOT */

#ifdef ANYSSH
  case XYSSH:
#ifdef SSHBUILTIN
  {
    const char **help_content;
    if (ck_ssh_is_installed()) {
      help_content = ssh_get_set_help();
    } else {
      help_content = (const char **)hmxyssh;
    }
    return (hmsga((char **)help_content));
  }
#else
    return (hmsga(hmxyssh));
#endif /* SSHBUILTIN */
#endif /* ANYCMD */

#ifdef LOCUS
  case XYLOCUS:
    return (hmsga(hmxylocus));
#endif /* LOCUS */

  case XYMATCH:
    return (hmsga(hmxymatch));

#ifndef NOSEXP
  case XYSEXP:
    return (hmsga(hmxysexp));
#endif /* NOSEXP */

#ifndef NOSPL
  case XYVAREV:
    return (hmsg("Syntax: SET VARIABLE-EVALUATION { RECURSIVE, SIMPLE }\n\
  Tells Kermit weather to evaluate \\%x and \\&x[] variables recursively\n\
  In C-Kermit 10.0 the default is SIMPLE, meaning variables return their\n\
  values like in any other programming language, making life much easier\n\
  when those values happen to be Windows or DOS pathnames, which contain\n\
  backslashes."));
#endif /* NOSPL */

#ifdef HAVE_LOCALE
  case XYLOCALE:
    return (hmsg("Syntax: SET LOCALE [ locale-string ]\n\
  Changes the locale for language and character-set to the one given.  The\n\
  local-string is in the format required by your computer, such as\n\
  en_US.US-ASCII or es_VE.ISO8859-1.  C-Kermit's SET LOCALE command affects\n\
  C-Kermit itself and any subprocesses, but does not affect the environment\n\
  from which C-Kermit was invoked."));
#endif /* HAVE_LOCALE */

  default:
    printf("Not available - \"%s\"\n", cmdbuf);
    return (0);
  }
}

#ifndef NOSPL
static char *hfsplit[] = {
    "Function \\fsplit(s1,&a,s2,s3,n2,n3) - Assigns string words to an array.",
    "  s1 = source string.",
    "  &a = array designator.",
    "  s2 = optional break set.",
    "  s3 = optional include set (or ALL, CSV, or TSV).",
    "  n2 = optional grouping mask.",
    "  n3 = optional separator flag:",
    "   0 = collapse adjacent separators;",
    "   1 = don't collapse adjacent separators.",
    " ",
    "  \\fsplit() breaks the string s1 into \"words\" as indicated by the "
    "other",
    "  parameters, assigning them to given array, if any.  If the specified",
    "  already exists, it is recycled; if no array is specified, the count is",
    "  returned but no array is created.  All arguments are optional",
    "  (\\fsplit() with no arguments returns 0).",
    " ",
    "  The BREAK SET is the set of all characters that separate words. The",
    "  default break set is all characters except ASCII letters and digits.",
    "  ASCII (C0) control characters are treated as break characters by "
    "default,",
    "  as are spacing and punctuation characters, brackets, and so on, and",
    "  all 8-bit characters.",
    " ",
    "  The INCLUDE SET is the set of characters that are to be treated as ",
    "  parts of words even though they normally would be separators.  The",
    "  default include set is empty.  Three special symbolic include sets are",
    "  also allowed:",
    " ",
    "    ALL (meaning include all bytes that are not in the break set)",
    "    CSV (special treatment for Comma-Separated-Value records)",
    "    TSV (special treatment for Tab-Separated-Value records)",
    " ",
    "  For operating on 8-bit character sets, the include set should be ALL.",
    " ",
    "  If the grouping mask is given and is nonzero, words can be grouped by",
    "  quotes or brackets selected by the sum of the following:",
    " ",
    "     1 = doublequotes:    \"a b c\"",
    "     2 = braces:          {a b c}",
    "     4 = apostrophes:     'a b c'",
    "     8 = parentheses:     (a b c)",
    "    16 = square brackets: [a b c]",
    "    32 = angle brackets:  <a b c>",
    " ",
    "  Nesting is possible with {}()[]<> but not with quotes or apostrophes.",
    " ",
    "Returns integer:",
    "  Number of words in source string.",
    " ",
    "Also see:",
    "  HELP FUNCTION WORD",
    ""};

/*  D O H F U N C  --  Give help for a function  */

int dohfunc(int xx) {
  /* int x; */
  if (xx == -3) {
    return (hmsga(hmxxfunc));
  }
  if (xx == FN_WORD) /* Long help message */
    return (hmsga(hmfword));

  printf("\n");
  switch (xx) {
  case FN_IND: /* Index (of string 1 in string 2) */
  case FN_RIX: /* Rindex (index from right) */
    printf("\\f%sindex(s1,s2,n1,n2)\n\
  s1 = string to look for.\n\
  s2 = string to look in.\n\
  n1 = optional 1-based starting position, default = 1.\n\
  n2 = optional desired occurrence number, default = 1.\n",
           xx == FN_RIX ? "r" : "");
    printf("Returns integer:\n\
  1-based position of %smost occurrence of s1 in s2, ignoring the %smost\n\
  (n1-1) characters in s2; returns 0 if s1 not found in s2.\n",
           xx == FN_IND ? "left" : "right", xx == FN_IND ? "left" : "right");
    break;
  case FN_COUNT: /* Count occurrences of s1 in s2 */
    printf("\\fcount(s1,s2,n1)\n\
  s1 = string or character to look for.\n\
  s2 = string to look in.\n\
  n1 = optional 1-based starting position, default = 1.\n");
    printf("Returns integer:\n\
  Number of occurrences of s1 in s2, 0 or more.\n");
    break;

  case FN_SEARCH:  /* Search for pattern */
  case FN_RSEARCH: /* Search for pattern from right */
    printf("\\f%ssearch(s1,s2,n1,n2)\n\
  s1 = pattern to look for.\n\
  s2 = string to look in.\n\
  n1 = optional 1-based offset, default = 1.\n\
  n2 = optional desired occurrence of match, default = 1.\n",
           xx == FN_RSEARCH ? "r" : "");
    printf("Returns integer:\n\
  1-based position of %smost match for s1 in s2, ignoring the %smost\n\
  (n1-1) characters in s2; returns 0 if no match.\n",
           xx == FN_SEARCH ? "left" : "right",
           xx == FN_SEARCH ? "left" : "right");
    printf("  s1 is a \"floating pattern\"; see HELP PATTERNS for details.\n");
    break;
  case FN_LEN: /* Length (of string) */
    printf("\\flength(s1)\n\
  s1 = string.\n");
    printf("Returns integer:\n\
  Length of string s1.\n");
    break;
  case FN_LIT: /* Literal (don't expand the string) */
    printf("\\fliteral(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  s1 literally without evaluation.\n");
    break;
  case FN_LOW: /* Lower (convert to lowercase) */
    printf("\\flower(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  s1 with uppercase letters converted to lowercase.\n");
    break;
  case FN_MAX: /* Max (maximum) */
    printf("\\fmaximum(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
    printf("Returns integer:\n\
  The greater of n1 and n2.\n");
    break;
  case FN_MIN: /* Min (minimum) */
    printf("\\fminimum(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
    printf("Returns integer:\n\
  The lesser of n1 and n2.\n");
    break;
  case FN_MOD: /* Mod (modulus) */
    printf("\\fmodulus(n1,n2)\n\
  n1 = integer.\n\
  n2 = integer.\n");
    printf("Returns integer:\n\
  The remainder after dividing n1 by n2.\n");
    break;
  case FN_EVA: /* Eval (evaluate arith expression) */
    printf("\\fevaluate(e)\n\
  e = arithmetic expression in ordinary algebraic notation.\n");
    printf("Returns integer:\n\
  The result of evaluating the expression.\n");
    break;
  case FN_SUB: /* Substr (substring) */
    printf("\\fsubstring(s1,n1,n2)\n\
  s1 = string.\n\
  n1 = integer, 1-based starting position, default = 1.\n\
  n2 = integer, length, default = length(s1) - n1 + 1.\n");
    printf("Returns string:\n\
  Substring of s1 starting at n1, length n2.\n");
    break;
  case FN_UPP: /* Upper (convert to uppercase) */
    printf("\\fupper(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  s1 with lowercase letters converted to uppercase.\n");
    break;
  case FN_REV: /* Reverse (a string) */
    printf("\\freverse(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  s1 with its characters in reverse order.\n");
    break;
  case FN_REP: /* Repeat (a string) */
    printf("\\frepeat(s1,n1)\n\
  s1 = string.\n\
  n1 = integer.\n");
    printf("Returns string:\n\
  s1 repeated n1 times.\n");
    break;
  case FN_EXE: /* Execute (a macro) */
    printf("\\fexecute(m1,a1,a2,a3,...)\n\
  m1 = macro name.\n\
  a1 = argument 1.\n\
  a2 = argument 2, etc\n");
    printf("Returns string:\n\
  The return value of the macro (HELP RETURN for further info).\n");
    break;
  case FN_LPA: /* LPAD (left pad) */
  case FN_RPA: /* RPAD (right pad) */
    printf("\\f%cpad(s1,n1,c1)\n\
  s1 = string.\n\
  n1 = integer.\n\
  c1 = character, default = space.\n",
           xx == FN_LPA ? 'l' : 'r');
    printf("Returns string:\n\
  s1 %s-padded with character c1 to length n1.\n",
           xx == FN_LPA ? "left" : "right");
    break;
  case FN_DEF: /* Definition of a macro, unexpanded */
    printf("\\fdefinition(m1)\n\
  m1 = macro name.\n");
    printf("Returns string:\n\
  Literal definition of macro m1.\n");
    break;
  case FN_CON: /* Contents of a variable, ditto */
    printf("\\fcontents(v1)\n\
  v1 = variable name such as \\%%a.\n");
    printf("Returns string:\n\
  Literal definition of variable v1, evaluated one level only.\n");
    break;
  case FN_FIL: /* Next file */
    printf("\\fnextfile()\n");
    printf("Returns string:\n\
  Name of next file from list created by most recent \\f[r]files() or\n\
  \\f[r]dir()invocation, or an empty string if there are no more files in\n\
  the list.\n");
    break;
  case FN_FC: /* File count */
    printf("\\ffiles(f1[,&a]) - File list.\n\
  f1 = file specification, possibly containing wildcards.\n\
  &a = optional name of array to assign file list to.\n");
    printf("Returns integer:\n\
  The number of regular files that match f1.  Use with \\fnextfile().\n");
    break;
  case FN_CHR: /* Character (like BASIC CHR$()) */
    printf("\\fcharacter(n1)\n\
  n1 = integer.\n");
    printf("Returns character:\n\
  The character whose numeric code is n1.\n");
    break;
  case FN_RIG: /* Right (like BASIC RIGHT$()) */
    printf("\\fright(s1,n1)\n\
  s1 = string.\n\
  n1 = integer, default = length(s1).\n");
    printf("Returns string:\n\
  The rightmost n1 characters of string s1.\n");
    break;
  case FN_LEF: /* Left (like BASIC LEFT$()) */
    printf("\\fleft(s1,n1)\n\
  s1 = string.\n\
  n1 = integer, default = length(s1).\n");
    printf("Returns string:\n\
  The leftmost n1 characters of string s1.\n");
    break;
  case FN_COD: /* Code value of character */
    printf("\\fcode(s1)\n\
  c1 = character.\n");
    printf("Returns integer:\n\
  The numeric code of the first character in string s1, or 0 if s1 empty.\n");
    break;
  case FN_RPL: /* Replace */
    printf("\\freplace(s1,s2,[s3[,n1[,n2]]])\n\
  s1 = original string.\n\
  s2 = match string.\n\
  s3 = replacement string (may be empty).\n\
  n1 = occurrence (if omitted or 0 does all occurrences).\n");
#ifdef RPLWORDMODE
    printf("  n2 = word mode \
(0 = ignore context; 1 = only if target is delimited).\n");
#endif /* RPLWORDMODE */

    printf("Returns string:\n\
  s1 with occurrence number n1 of s2 replaced by s3.\n\
  If n1 = 0 or omitted, all occurrences are replaced.\n\
  If n1 < 0, occurrences are counted from the right.\n");
    break;

  case FN_FD: /* File date */
    printf("\\fdate(f1)\n\
  f1 = filename.\n");
    printf("Returns string:\n\
  Modification date of file f1, format: yyyymmdd hh:mm:ss.\n");
    break;
  case FN_FS: /* File size */
    printf("\\fsize(f1)\n\
  f1 = filename.\n");
    printf("Returns integer:\n\
  Size of file f1.\n");
    break;
  case FN_VER: /* Verify */
    printf("\\fverify(s1,s2,n1)\n\
  s1 = string of characters to look for.\n\
  s2 = string to look in.\n\
  n1 = starting position in s2.\n");
    printf("Returns integer:\n\
  1-based position of first character in s2 that is not also in s1,\n\
  or -1 if s1 is empty, or 0 if all characters in s2 are also in s1.\n");
    break;
  case FN_IPA: /* Find and return IP address */
    printf("\\fipaddress(s1,n1)\n\
  s1 = string.\n\
  n1 = 1-based integer starting position, default = 1.\n");
    printf("Returns string:\n\
  First IP address in s1, scanning from left starting at position n1.\n");
    break;
  case FN_HEX: /* Hexify */
    printf("\\fhexify(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  The hexadecimal representation of s1.  Also see \\fn2hex().\n");
    break;
  case FN_UNH: /* Unhexify */
    printf("\\funhexify(h1)\n\
  h1 = Hexadecimal string.\n");
    printf("Returns string:\n\
  The result of unhexifying s1, or nothing if s1 is not a hex string.\n");
    break;
  case FN_UNTAB: /* Untabify */
    printf("\\funtabify(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  The result of converting tabs in s1 to spaces assuming tab stops every\n\
  8 spaces.\n");
    break;
  case FN_BRK: /* Break */
  case FN_SPN: /* Span */
    printf("\\f%s(s1,s2,n1)\n\
  s1 = string to look in.\n\
  s2 = string of characters to look for.\n\
  n1 = 1-based integer starting position, default = 1.\n",
           xx == FN_BRK ? "break" : "span");
    printf("Returns string:\n\
  s1 up to the first occurrence of any character%salso in s2,\n\
  scanning from the left starting at position n1.\n",
           xx == FN_SPN ? " not " : " ");
    break;
  case FN_TRM: /* Trim */
  case FN_LTR: /* Left-Trim */
    printf("\\f%s(s1,s2)\n\
  s1 = string to look in.\n\
  s2 = string of characters to look for, default = blanks and tabs.\n",
           xx == FN_TRM ? "trim" : "ltrim");
    printf("Returns string:\n\
  s1 with all characters that are also in s2 trimmed from the %s.\n.",
           xx == FN_TRM ? "right" : "left");
    break;
  case FN_CAP: /* Capitalize */
    printf("\\fcapitalize(s1)\n\
  s1 = string.\n");
    printf("Returns string:\n\
  s1 with its first letter converted to uppercase and the remaining\n\
  letters to lowercase.\n");
    printf("Synonym: \\fcaps(s1)\n");
    break;
  case FN_TOD: /* Time-of-day-to-secs-since-midnite */
    printf("\\ftod2secs(s1)\n\
  s1 = time-of-day string, hh:mm:ss, 24-hour format.\n");
    printf("Returns number:\n\
  Seconds since midnight.\n");
    break;
  case FN_FFN: /* Full file name */
    printf("\\fpathname(f1)\n\
  f1 = filename, possibly wild.\n");
    printf("Returns string:\n\
  Full pathname of f1.\n");
    break;
  case FN_CHK: /* Checksum of text */
    printf("\\fchecksum(s1)\n\
  s1 = string.\n");
    printf("Returns integer:\n\
  16-bit checksum of string s1.\n");
    break;
  case FN_CRC: /* CRC-16 of text */
    printf("\\fcrc16(s1)\n\
  s1 = string.\n");
    printf("Returns integer:\n\
  16-bit cyclic redundancy check of string s1.\n");
    break;
  case FN_BSN: /* Basename of file */
    printf("\\fbasename(f1)\n\
  f1 = filename, possibly wild.\n");
    printf("Returns string:\n\
  Filename f1 stripped of all device and directory information.\n");
    break;
  case FN_CMD: /* Output of a command (cooked) */
    printf("\\fcommand(s1)\n\
  s1 = string\n");
    printf("Returns string:\n\
  Output of system command s1, if any, with final line terminator stripped.\n");
    break;
  case FN_RAW: /* Output of a command (raw) */
    printf("\\frawcommand(s1)\n\
  s1 = string\n");
    printf("Returns string:\n\
  Output of system command s1, if any.\n");
    break;
  case FN_STX: /* Strip from right */
    printf("\\fstripx(s1,c1)\n\
  s1 = string to look in.\n\
  c1 = character to look for, default = \".\".\n");
    printf("Returns string:\n\
  s1 up to the rightmost occurrence of character c1.\n");
    break;

  case FN_STL: /* Strip from left */
    printf("\\flop(s1[,c1[,n1]])\n\
  s1 = string to look in.\n\
  c1 = character to look for, default = \".\".\n\
  n1 = occurrence of c1, default = 1.\n");
    printf("Returns string:\n\
  The part of s1 after the n1th leftmost occurrence of character c1.\n");
    break;

  case FN_LOPX: /* Strip from right */
    printf("\\flopx(s1,c1)\n\
  s1 = string to look in.\n\
  c1 = character to look for, default = \".\".\n\
  n1 = occurrence of c1, default = 1.\n");
    printf("Returns string:\n\
  The part of s1 after the n1th rightmost occurrence of character c1.\n");
    break;

  case FN_STN: /* Strip n chars */
    printf("\\fstripn(s1,n1)\n\
  s1 = string to look in.\n\
  n1 = integer, default = 0.\n");
    printf("Returns string:\n\
  s1 with n1 characters removed from the right.\n");
    break;

  case FN_STB: /* Strip enclosing brackets */
    printf("\\fstripb(s1[,c1[,c2]])\n\
  s1 = original string.\n\
  c1 = optional first character\n");
    printf("\
  c2 = optional final character.\n");
    printf("Returns string:\n\
  s1 with the indicated enclosing characters removed.  If c1 and c2 not\n\
     specified, any matching brackets, braces, parentheses, or quotes are\n");
    printf("\
     assumed.  If c1 is given but not c2, the appropriate c2 is assumed.\n\
     if both c1 and c2 are given, they are used as-is.\n");
    printf("Alternative format:\n\
  Include a grouping mask number in place of c1 and omit c2 to specify more\n\
  than one possibility at once; see \\fword() for details.\n");
    break;

  case FN_2HEX: /* Num to hex */
    printf("\\fn2hex(n1) - Number to hex\n  n1 = integer.\n");
    printf("Returns string:\n  The hexadecimal representation of n1.\n");
    break;

  case FN_2OCT: /* Num to hex */
    printf("\\fn2octal(n1) - Number to octal\n  n1 = integer.\n");
    printf("Returns string:\n  The octal representation of n1.\n");
    break;

#ifdef RECURSIVE
  case FN_DIR: /* Recursive directory count */
    printf("\\fdirectories(f1,&a) - Directory list.\n\
  f1 = directory specification, possibly containing wildcards.\n\
  &a = optional name of array to assign directory list to.\n");
    printf("Returns integer:\n\
  The number of directories that match f1; use with \\fnextfile().\n");
    break;

  case FN_RFIL: /* Recursive file count */
    printf("\\frfiles(f1[,&a]) - Recursive file list.\n\
  f1 = file specification, possibly containing wildcards.\n\
  &a = optional name of array to assign file list to.\n");
    printf("Returns integer:\n\
  The number of files whose names match f1 in the current or given\n\
  directory tree; use with \\fnextfile().\n");
    break;

  case FN_RDIR: /* Recursive directory count */
    printf("\\frdirectories(f1) - Recursive directory list.\n\
  f1 = directory specification, possibly containing wildcards.\n\
  &a = optional name of array to assign directory list to.\n");
    printf("Returns integer:\n\
  The number of directories that match f1 in the current or given directory\n\
  tree.  Use with \\fnextfile().\n");
    break;
#endif /* RECURSIVE */

  case FN_DNAM: /* Directory part of a filename */
    printf("\\fdirname(f) - Directory part of a filename.\n\
  f = a file specification.\n");
    printf("Returns directory name:\n\
  The full name of the directory that the file is in, or if the file is a\n\
  directory, its full name.\n");
    break;

#ifndef NORANDOM
  case FN_RAND: /* Random number */
    printf("\\frandom(n) - Random number.\n\
  n = a positive integer.\n");
    printf("Returns integer:\n\
  A random number between 0 and n-1.\n");
    break;
#endif /* NORANDOM */

  case FN_SPLIT: /* Split */
    hmsga(hfsplit);
    break;

  case FN_DTIM: /* CVTDATE */
    printf("\\fcvtdate([date-time][,n1]) - Date/time conversion.\n");
    printf("  Converts date and/or time to standard format.\n");
    printf("  If no date/time given, returns current date/time.\n");
    printf("  [date-time], if given, is free-format date and/or time.\n");
    printf("  HELP DATE for info about date-time formats.\n");
    printf("Returns string:\n\
  Standard-format date and time: yyyymmdd hh:mm:ss (numeric)\n");
    printf("  If n1 is given:\n\
  n1 = 1: yyyy-mmm-dd hh:mm:ss (mmm = English 3-letter month abbreviation)\n\
  n1 = 2: dd-mmm-yyyy hh:mm:ss (ditto)\n\
  n1 = 3: yyyymmddhhmmss (all numeric)\n\
  n1 = 4: Day Mon dd hh:mm:ss yyyy (asctime)\n\
  n1 = 5: yyyy:mm:dd:hh:mm:ss (all numeric with all fields delimited)\n\
  n1 = 6: dd month-spelled-out yyyy hh:mm:ss\n\
  Other:  yyyymmdd hh:mm:dd\n\
  If n1 is negative (-1 to -6), the result is date only.");
    break;

  case FN_JDATE: /* DOY */
    printf("\\fdoy([date-time]) - Day of Year.\n");
    printf("  Converts date and/or time to day-of-year (DOY) format.\n");
    printf("  If no date/time given, returns current date.\n");
    printf("  [date-time], if given, is free-format date and/or time.\n");
    printf("  HELP DATE for info about date-time formats.\n");
    printf("Returns numeric string:\n\
  DOY: yyyyddd, where ddd is 1-based day number in year.\n");
    break;

  case FN_PNCVT:
    printf("\\fdialconvert(phone-number) - Convert phone number.\n");
    printf("  Converts the given phone number for dialing according\n");
    printf(
        "  to the prevailing dialing rules -- country code, area code, etc.\n");
    printf("Returns string:\n\
  The dial string that would be used if the same phone number had been\n\
  given to the DIAL command.\n");
    break;

  case FN_DATEJ: /* DOY2DATE */
    printf("\\fdoy2date([doy[ time]]) - Day of Year to Date.\n");
    printf("  Converts yyyymmm to yyyymmdd\n");
    printf("  If time included, it is converted to 24-hour format.");
    printf("Returns standard date or date-time string yyyymmdd hh:mm:ss\n");
    break;

  case FN_MJD:
    printf("\\fmjd([[date][ time]]) - Modified Julian Date (MJD).\n");
    printf("  Converts date and/or time to MJD, the number of days since 17 "
           "Nov 1858.\n");
    printf("  HELP DATE for info about date-time formats.\n");
    printf("Returns: integer.\n");
    break;

  case FN_MJD2:
    printf("\\fmjd2date(mjd) - Modified Julian Date (MJD) to Date.\n");
    printf("  Converts MJD to standard-format date.\n");
    printf("Returns: yyyymmdd.\n");
    break;

  case FN_DAY:
    printf("\\fday([[date][ time]]) - Day of Week.\n");
    printf("Returns day of week of given date as Mon, Tue, etc.\n");
    printf("HELP DATE for info about date-time formats.\n");
    printf("Also see HELP FUNCTION DAYNAME.\n");
    break;

  case FN_NDAY:
    printf("\\fnday([[date][ time]]) - Numeric Day of Week.\n");
    printf("Returns numeric day of week of given date, 0=Sun, 1=Mon, ..., "
           "6=Sat.\n");
    printf("HELP DATE for info about date-time formats.\n");
    break;

  case FN_TIME:
    printf("\\ftime([[date][ time]]) - Time.\n");
    printf(
        "Returns time portion of given date and/or time in hh:mm:ss format.\n");
    printf("If no argument given, returns current time.\n");
    printf("HELP DATE for info about date-time formats.\n");
    break;

  case FN_NTIM:
    printf("\\fntime([[date][ time]]) - Numeric Time.\n");
    printf("Returns time portion of given date and/or time as seconds since "
           "midnight.\n");
    printf("If no argument given, returns current time.\n");
    printf("HELP DATE for info about date-time formats.\n");
    break;

  case FN_N2TIM:
    printf("\\fn2time(seconds) - Numeric Time to Time.\n");
    printf("Returns the given number of seconds in hh:mm:ss format.\n");
    break;

  case FN_PERM:
    printf("\\fpermissions(file) - Permissions of File.\n");
    printf(
#ifdef UNIX
        "Returns permissions of given file as they would be shown by \"ls "
        "-l\".\n"
#else
        "Returns the permissions of the given file.\n"
#endif /* UNIX */
    );
    break;

  case FN_ALOOK:
    printf("\\farraylook(pattern,&a) - Lookup pattern in array.\n\
  pattern = String or pattern\n");
    printf("  &a = array designator, can include range specifier.\n");
    printf("Returns number:\n\
  The index of the first matching array element or -1 if none.\n");
    printf("More info:\n\
  HELP PATTERN for pattern syntax.\n  HELP ARRAY for arrays.\n");
    break;

  case FN_TLOOK:
    printf("\\ftablelook(keyword,&a,[c]) \
- Lookup keyword in keyword table.\n");
    printf("  keyword = keyword to look up (can be abbreviated).\n");
    printf("  &a      = array designator, can include range specifier.\n");
    printf("            This array must be in alphabetical order.\n");
    printf("  c       = Optional field delimiter, colon(:) by default.\n");
    printf("Returns number:\n\
  1 or greater, index of array element that uniquely matches given keyword;\n");
    printf("or -2 if keyword was ambiguous, or -1 if keyword empty or not "
           "found.\n");
    printf("Also see:\n\
  HELP FUNC ARRAYLOOK for a similar function.\n  HELP ARRAY for arrays.\n");
    break;

  case FN_ABS: /* Absolute */
    printf("\\fabsolute(n1)\n\
  n1 = integer.\n");
    printf("Returns integer:\n\
  The absolute (unsigned) value of n1.\n");
    break;

#ifdef FNFLOAT
  case FN_FPABS:
    printf("\\ffpabsolute(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The absolute (unsigned) value of f1 to d decimal places.\n");
    break;

  case FN_FPADD:
    printf("\\ffpadd(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  The sum of f1 and f2 to d decimal places.\n");
    break;

  case FN_FPSUB:
    printf("\\ffpsubtract(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  f1 minus f2 to d decimal places.\n");
    break;

  case FN_FPMUL:
    printf("\\ffpmultiply(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  The product of f1 and f2 to d decimal places.\n");
    break;

  case FN_FPDIV:
    printf("\\ffpdivide(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  f1 divided by f2 to d decimal places.\n");
    break;

  case FN_FPMAX:
    printf("\\ffpmaximum(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  The maximum of f1 and f2 to d decimal places.\n");
    break;

  case FN_FPMIN:
    printf("\\ffpminimum(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  The minimum of f1 and f2 to d decimal places.\n");
    break;

  case FN_FPMOD:
    printf("\\ffpmodulus(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  The modulus of f1 and f2 to d decimal places.\n");
    break;

  case FN_FPPOW:
    printf("\\ffpraise(f1,f2,d)\n\
  f1,f2 = floating-point numbers or integers.\n\
      d = integer.\n");
    printf("Returns floating-point number:\n\
  f1 raised to the power f2, to d decimal places.\n");
    break;

  case FN_FPCOS:
    printf("\\ffpcosine(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The cosine of angle f1 (in radians) to d decimal places.\n");
    break;

  case FN_FPSIN:
    printf("\\ffpsine(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The sine of angle f1 (in radians) to d decimal places.\n");
    break;

  case FN_FPTAN:
    printf("\\ffptangent(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The tangent of angle f1 (in radians) to d decimal places.\n");
    break;

  case FN_FPEXP:
    printf("\\ffpexp(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  e (the base of natural logarithms) raised to the f1 power,\n\
  to d decimal places.\n");
    break;

  case FN_FPINT:
    printf("\\ffpint(f1)\n\
  f1 = floating-point number or integer.\n");
    printf("Returns integer:\n\
  The integer part of f1.\n");
    break;

  case FN_FPLOG:
    printf("\\ffplog10(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The logarithm, base 10, of f1 to d decimal places.\n");
    break;

  case FN_FPLN:
    printf("\\ffplogn(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The natural logarithm of f1 to d decimal places.\n");
    break;

  case FN_FPROU:
    printf("\\ffpround(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  f1 rounded to d decimal places.\n");
    break;

  case FN_FPSQR:
    printf("\\ffpsqrt(f1,d)\n\
  f1 = floating-point number or integer.\n\
   d = integer.\n");
    printf("Returns floating-point number:\n\
  The square root of f1 to d decimal places.\n");
    break;
#endif /* FNFLOAT */

#ifdef CKCHANNELIO
  case FN_FEOF:
    printf("\\f_eof(n1)\n\
  n1 = channel number.\n");
    printf("Returns number:\n\
  1 if channel n1 at end of file, 0 otherwise.\n");
    break;
  case FN_FPOS:
    printf("\\f_pos(n1)\n\
  n1 = channel number.\n");
    printf("Returns number:\n\
  Read/write pointer of channel n1 as byte number.\n");
    break;
  case FN_NLINE:
    printf("\\f_line(n1)\n\
  n1 = channel number.\n");
    printf("Returns number:\n\
  Read/write pointer of channel n1 as line number.\n");
    break;
  case FN_FILNO:
    printf("\\f_handle(n1)\n\
  n1 = channel number.\n");
    printf("Returns number:\n\
  File %s of open file on channel n1.\n",
           "descriptor");
    break;
  case FN_FSTAT:
    printf("\\f_status(n1)\n\
  n1 = channel number.\n");
    printf("Returns number:\n\
  Sum of open modes of channel n1: 1 = read; 2 = write; 4 = append, or:\n\
  0 if not open.\n");
    break;
  case FN_FGCHAR:
    printf("\\f_getchar(n1)\n\
  n1 = channel number.\n");
    printf("  Reads a character from channel n1 and returns it.\n");
    break;
  case FN_FGLINE:
    printf("\\f_getline(n1)\n\
  n1 = channel number.\n");
    printf("  Reads a line from channel n1 and returns it.\n");
    break;
  case FN_FGBLK:
    printf("\\f_getblock(n1,n2)\n\
  n1 = channel number, n2 = size\n");
    printf(
        "  Reads a block of n2 characters from channel n1 and returns it.\n");
    break;
  case FN_FPCHAR:
    printf("\\f_putchar(n1,c)\n\
  n1 = channel number, c = character\n");
    printf("  Writes a character to channel n1.\n\
Returns number:\n\
  1 if successful, otherwise a negative error code.\n");
    break;
  case FN_FPLINE:
    printf("\\f_putline(n1,s1)\n\
  n1 = channel number, s1 = string\n");
    printf("  Writes the string s1 to channel n1 and adds a line terminator.\n\
Returns number:\n");
    printf("  How many characters written if successful;\n\
  Otherwise a negative error code.\n");
    break;
  case FN_FPBLK:
    printf("\\f_putblock(n1,s1)\n\
  n1 = channel number, s1 = string\n");
    printf("  Writes the string s1 to channel n1.\n\
  Returns number:\n");
    printf("  How many characters written if successful;\n\
  Otherwise a negative error code.\n");
    break;
  case FN_FERMSG:
    printf("\\f_errmsg([n1])\n\
  n1 = numeric error code, \\v(f_error) by default.\n");
    printf("  Returns the associated error message string.\n");
    break;
#endif /* CKCHANNELIO */

  case FN_AADUMP:
    printf("\\faaconvert(name,&a[,&b])\n\
  name = name of associative array, &a and &b = names of regular arrays.\n");
    printf(
        "  Converts the given associative array into two regular arrays, &a and &b,\n\
  containing the indices and values, respectively:\n");
    printf("Returns number:\n\
  How many elements were converted.\n");
    break;

  case FN_PATTERN:
    printf("\\fpattern(s)\n\
  s = string\n\
  Returns string: s with any variables, etc, evaluated in the normal manner.\n");
    printf("\
  For use with INPUT, MINPUT, and REINPUT to declare that a search target is\n\
  a pattern rather than a literal string.\n");
    break;

  case FN_HEX2N:
    printf("\\fhex2n(s)\n\
  s = hexadecimal number\n\
  Returns decimal equivalent.\n");
    break;

  case FN_HEX2IP:
    printf("\\fhex2ip(s)\n\
  s = 8-digit hexadecimal number\n\
  Returns the equivalent decimal dotted IP address.\n");
    break;

  case FN_IP2HEX:
    printf("\\fip2hex(s)\n\
  s = decimal dotted IP address\n\
  Returns the equivalent 8-digit hexadecimal number.\n");
    break;

  case FN_OCT2N:
    printf("\\foct2n(s)\n\
  s = octal number\n\
  Returns decimal equivalent.\n");
    break;

  case FN_TOB64:
    printf("\\b64encode(s)\n\
  s = string containing no NUL bytes\n\
  Returns Base-64 encoding of string.\n");
    break;

  case FN_FMB64:
    printf("\\b64decode(s)\n\
  s = string in Base-64 notation\n\
  Returns the decoded string or an error code if s not valid.\n");
    break;

  case FN_RADIX:
    printf("\\fradix(s,n1,n2)\n\
  s = number in radix n1\n\
  Returns the number's representation in radix n2.\n");
    break;

  case FN_JOIN:
    printf("\\fjoin(&a[,s[,n1[,n2]]])\n\
  &a = array designator, can include range specifier.\n\
  s  = optional separator.\n");
    printf("\
  n1 = nonzero to put grouping around elements that contain spaces;\n\
       see \\fword() grouping mask for values of n.\n");
    printf("\
  n2 = 0 or omitted to put spaces between elements; nonzero to omit them.\n");
    printf("\
  Returns the (selected) elements of the array joined to together,\n\
  separated by the separator.\n");
    printf("\n\
  If s is CSV (literally), that means the array is to be transformed into a\n\
  comma-separated list.  No other arguments are needed.  If s is TSV, then\n\
  a tab-separated list is created.\n");
    break;

  case FN_SUBST:
    printf("\\fsubstitute(s1,s2,s3)\n\
  s1 = Source string.\n\
  s2 = List of characters to be translated.\n\
  s3 = List of characters to translate them to.\n");
    printf("  Returns: s1, with each character that is in s2 replaced by the\n\
  corresponding character in s3.  s2 and s3 can contain ASCII ranges,\n\
  like [a-z].  Any characters in s2 that don't have corresponding\n\
  characters in s3 (after range expansion) are removed from the result.\n\
  This function works only with single-byte character-sets\n");
    break;

#ifndef NOSEXP
  case FN_SEXP:
    printf("\\fsexpression(s1)\n\
  s1 = S-Expression.\n");
    printf("  Returns: The result of evaluating s1.\n");
    break;

#endif /* NOSEXP */

  case FN_CMDSTK:
    printf("\\fcmdstack(n1,n2)\n\
  n1 = Command-stack level, 0 to \\v(cmdlevel), default \\v(cmdlevel).\n\
  n2 = Function code, 0 or 1.\n");
    printf("Returns:\n");
    printf("  n2 = 0: name of object at stack level n1\n\
  n2 = 1: type of object at stack level n1:\n\
     0 = interactive prompt\n\
     1 = command file\n\
     2 = macro\n");
    break;

#ifdef CKFLOAT
  case FN_DIFDATE:
    printf("\\fdiffdates(d1,d2)\n\
  d1 = free-format date and/or time (default = NOW).\n\
  d2 = ditto.\n");
    printf("Returns:\n");
    printf("  Difference expressed as delta time:\n");
    printf("  Negative if d2 is later than d1, otherwise positive.\n");
    break;
#endif /* CKFLOAT */

  case FN_CMPDATE:
    printf("\\fcmpdates(d1,d2)\n\
  d1 = free-format date and/or time (default = NOW).\n\
  d2 = ditto.\n");
    printf("Returns:\n");
    printf("  0 if d1 is equal to d2;\n\
  1 if d1 is later than d2;\n\
 -1 if d1 is earlier than d2.\n");
    break;

  case FN_TOGMT:
    printf("\\futcdate(d1)\n\
  d1 = free-format date and/or time (default = NOW).\n");
    printf("Returns:\n");
    printf("  Date-time converted to UTC (GMT) yyyymmdd hh:mm:ss.\n");
    break;

#ifdef TCPSOCKET
  case FN_HSTADD:
    printf("\\faddr2name(s)\n\
  s = numeric IP address.\n");
    printf("Returns:\n");
    printf("  Corresponding IP hostname if found, otherwise null.\n");
    break;
  case FN_HSTNAM:
    printf("\\fname2addr(s)\n\
  s = IP host name.\n");
    printf("Returns:\n");
    printf("  Corresponding numeric IP address if found, else null.\n");
    break;
#endif /* TCPSOCKET */

  case FN_DELSEC:
    printf("\\fdelta2secs(dt)\n\
  dt = Delta time, e.g. +3d14:27:52.\n");
    printf("Returns:\n");
    printf("  The corresponding number of seconds.\n");
    break;

  case FN_PC_DU:
    printf("\\fdos2unixpath(p)\n\
  p = string, DOS pathname.\n");
    printf("Returns:\n");
    printf("  The argument converted to a Unix pathname.\n");
    break;

  case FN_PC_UD:
    printf("\\funix2dospath(p)\n\
  p = string, Unix pathname.\n");
    printf("Returns:\n");
    printf("  The argument converted to a DOS pathname.\n");
    break;

#ifdef FN_ERRMSG
  case FN_ERRMSG:
    printf("\\ferrstring(n)\n\
  n = platform-dependent numeric error code.\n");
    printf("Returns:\n");
    printf("  The corresponding error string.\n");
    break;
#endif /* FN_ERRMSG */

  case FN_KWVAL:
    printf("\\fkeywordvalue(s1[,s2])\n\
  s1 = string of the form \"name=value\"\n\
  s2 = one more separator characters (default separator is \"=\")\n");
    printf("    Assigns the value, if any, to the named macro and sets\n");
    printf("    the \\v(lastkeywordvalue) to the macro name.\n");
    printf("    If no value is given, the macro is undefined.\n");
    printf("Returns:\n");
    printf(" -1 on error\n");
    printf("  0 if no keyword or value were found\n");
    printf("  1 if a keyword was found but no value\n");
    printf("  2 if a keyword and a value were found\n");
    printf("Synonym: \\kwvalue(s1[,s2])\n");
    break;

  case FN_SNAME:
    printf("\\fshortpathname(s)\n\
    Synonym for \fpathname()\n");
    break;

  case FN_LNAME:
    printf("\\flongpathname(s)\n\
    Synonym for \fpathname()\n");
    break;

  case FN_EMAIL:
    printf("\\femailaddress(s)\n\
  s = From: or Sender: header from an RFC2822-format email message\n");
    printf("    Extracts and returns the email address.\n");
    break;

  case FN_PICTURE:
    printf("\\fpictureinfo(s[,&a])\n\
  s  = File specification of an image file in JPG or GIF format.\n\
  &a = Optional array name.\n\n");
    printf("Returns integer:\n\
  0 if file not found or not recognized;\n\
  1 if orientation is landscape;\n\
  2 if orientation is portrait;\n\
  3 if the image is square.\n");
    printf("\n\
If an array name is included, and if the function's return value is\n\
greater than 0, element 1 of the array is filled in with the image\n\
width in pixels, element 2 the image height, and element 3 is the image's\n\
'date taken' (if present) in 'yyyy:mm:dd hh:mm:ss' format; for example\n\
2013:05:17 21:14:12.\n");
    break;

  case FN_PID:
    printf("\\fgetpidinfo(n1)\n\
 n1 = Numeric process ID\n");
    printf("Returns integer:\n\
 -1 on failure to get information;\n\
  1 if n1 is the ID of an active process;\n\
  0 if the process does not exist.\n");
    break;

  case FN_FUNC:
    printf("\\ffunction(s1)\n\
 s1 = name of function.\n");
    printf("Returns integer:\n\
  1 if s1 is the name of an available built-in function;\n\
  0 otherwise.\n");
    break;

  case FN_RECURSE:
    printf("\\frecurse(s1)\n\
 s1 = name of \\&x or \\&x[] type variable\n");
    printf("Returns the result of evaluating the variable recursively.\n");
    break;

  case FN_SQUEEZE:
    printf("\\fsqueeze(s)\n\
  s = string\n\
    Returns string with leading and trailing whitespace removed, Tabs\n\
    converted to Spaces, and multiple spaces converted to single spaces.\n");
    break;

#ifndef NOCSETS
  case FN_XLATE:
    printf("\\fcvtcset(s,cset1,cset2)\n\
  s = string\n\
    Returns string converted from character set cset1 to cset2, where cset1\n\
    and cset2 are names of File Character-Sets \
('set file char ?' for a list).\n");
    break;
#endif /* NOCSETS */

  case FN_UNPCT:
    printf("\\fdecodehex(s1[,s2])\n\
  s1, s2 = strings\n\
    Decodes a string s1 that contains prefixed hex bytes.  s2 is the prefix;\n\
    the default is %%%%.  You can specify any other prefix one or two bytes\n\
    long.  If the prefix contains letters (such as 0x), case is ingored.\n\
    Returns string s1 with hex escapes replaced by the bytes they \
represent.\n");
    break;

  case FN_STRINGT:
    printf("\\fstringtype(s)\n\
  s = string\n\
    Returns a string representing the type of its string argument s1:\n\
    7BIT, 8BIT, UTF8, TEXT, or BINARY.  TEXT means some kind of text\n\
    other than 7BIT, 8BIT, or UTF8 (this probably will never appear).\n");
    break;

  case FN_STRCMP:     /* fdc 12 November 2022 */
    hmsga(hmfstrcmp); /* Literal string was too long */
    break;

  case FN_FILEINF:      /* fdc 12 November 2022 */
    hmsga(hmffileinfo); /* Literal string was too long */
    break;

  case FN_FILECMP:
    printf("\\ffilecompare(s1,s2)\n\
  s1 = name of first file\n\
  s1 = name of second file\n\
  Returns a number:\n\
     0: The two files have identical contents and lengths;\n\
     1: The two files have different content or lengths;\n\
    -1: Error opening or reading either file.\n");
    break;

  case FN_DAYNAME:     /* fdc 12 November 2022 */
    hmsga(hmfdayname); /* Literal string was too long */
    break;

  case FN_MONNAME:     /* fdc 12 November 2022 */
    hmsga(hmfmonname); /* Literal string was too long */
    break;

  default:
    printf("Sorry, help not available for \"%s\"\n", cmdbuf);
  }
  printf("\n");
  return (0);
}
#endif /* NOSPL */

#ifndef NOXFER
/*  D O H R M T  --  Give help about REMOTE command  */

static char *hrset[] = {
    "Syntax:  REMOTE SET parameter value",
    "Example: REMOTE SET FILE TYPE BINARY",
    "  Asks the Kermit server to set the named parameter to the given value.",
    "  Equivalent to typing the corresponding SET command directly to the "
    "other",
    "  Kermit if it were in interactive mode.",
    ""};

int dohrmt(int xx) {
  int x;
  if (xx == -3)
    return (hmsga(hmhrmt));
  if (xx < 0)
    return (xx);
  if ((x = cmcfm()) < 0)
    return (x);
  switch (xx) {

  case XZCPY:
    return (hmsg("Syntax: REMOTE COPY source destination\n\
  Asks the Kermit server to copy the source file to destination.\n\
  Synonym: RCOPY."));

  case XZCWD:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE CD [ name ]\n\
  Asks the Kermit or FTP server to change its working directory or device.\n\
  If the device or directory name is omitted, restore the default.\n\
  Synonym: RCD."));
#else
    return (hmsg("Syntax: REMOTE CD [ name ]\n\
  Asks the Kermit server to change its working directory or device.\n\
  If the device or directory name is omitted, restore the default.\n\
  Synonym: RCD."));
#endif /* NEWFTP */

  case XZDEL:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE DELETE filespec\n\
  Asks the Kermit or FTP server to delete the named file(s).\n\
  Synonym: RDEL."));
#else
    return (hmsg("Syntax: REMOTE DELETE filespec\n\
  Asks the Kermit server to delete the named file(s).\n\
  Synonym: RDEL."));
#endif /* NEWFTP */

  case XZMKD:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE MKDIR directory-name\n\
  Asks the Kermit or FTP server to create the named directory.\n\
  Synonym: RMKDIR."));
#else
    return (hmsg("Syntax: REMOTE MKDIR directory-name\n\
  Asks the Kermit server to create the named directory.\n\
  Synonym: RMKDIR."));
#endif /* NEWFTP */

  case XZRMD:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE RMDIR directory-name\n\
  Asks the Kermit or FTP server to remove the named directory.\n\
  Synonym: RRMDIR."));
#else
    return (hmsg("Syntax: REMOTE RMDIR directory-name\n\
  Asks the Kermit server to remove the named directory.\n\
  Synonym: RRMDIR."));
#endif /* NEWFTP */

  case XZDIR:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE DIRECTORY [ filespec ]\n\
  Asks the Kermit or FTP server to provide a directory listing of the named\n\
  file(s) or if no file specification is given, of all files in its current\n\
  directory.  Synonym: RDIR."));
#else
    return (hmsg("Syntax: REMOTE DIRECTORY [ filespec ]\n\
  Asks the Kermit server to provide a directory listing of the named\n\
  file(s) or if no file specification is given, of all files in its current\n\
  directory.  Synonym: RDIR."));
#endif /* NEWFTP */

  case XZHLP:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE HELP\n\
  Asks the Kermit or FTP server to list the services it provides.\n\
  Synonym: RHELP."));
#else
    return (hmsg("Syntax: REMOTE HELP\n\
  Asks the Kermit server to list the services it provides.\n\
  Synonym: RHELP."));
#endif /* NEWFTP */

  case XZHOS:
    return (hmsg("Syntax: REMOTE HOST command\n\
  Sends a command to the other computer in its own command language\n\
  through the Kermit server that is running on that host.  Synonym: RHOST."));

#ifndef NOFRILLS
  case XZKER:
    return (hmsg("Syntax: REMOTE KERMIT command\n\
  Sends a command to the remote Kermit server in its own command language.\n\
  Synonym: RKERMIT."));

  case XZLGI:
    return (hmsg("Syntax: REMOTE LOGIN user password [ account ]\n\
  Logs in to a remote Kermit server that requires you login.  Note: RLOGIN\n\
  is NOT a synonym for REMOTE LOGIN."));

  case XZLGO:
    return (hmsg("Syntax: REMOTE LOGOUT\n\
  Logs out from a remote Kermit server to which you have previously logged in."));

  case XZMSG:
    return (hmsg("Syntax: REMOTE MESSAGE text\n\
  Sends a short text message to the remote Kermit server."));

  case XZPRI:
    return (hmsg("Syntax: REMOTE PRINT filespec [ options ]\n\
  Sends the specified file(s) to the remote Kermit and ask it to have the\n\
  file printed on the remote system's printer, using any specified options.\n\
  Synonym: RPRINT."));
#endif /* NOFRILLS */

  case XZREN:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE RENAME filespec newname\n\
  Asks the Kermit or FTP server to rename the file.  Synonym: RRENAME."));
#else
    return (hmsg("Syntax: REMOTE RENAME filespec newname\n\
  Asks the Kermit server to rename the file.  Synonym: RRENAME."));
#endif /* NEWFTP */

  case XZSET:
    return (hmsga(hrset));

  case XZSPA:
    return (hmsg("Syntax: REMOTE SPACE [ name ]\n\
  Asks the Kermit server to tell you about its disk space on the current\n\
  disk or directory, or in the one that you name.  Synonym: RSPACE."));

#ifndef NOFRILLS
  case XZTYP:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE TYPE file\n\
  Asks the Kermit or FTP server to send the named file to your screen.\n\
  Synonym: RTYPE."));
#else
    return (hmsg("Syntax: REMOTE TYPE file\n\
  Asks the Kermit server to send the named file(s) to your screen.\n\
  Synonym: RTYPE."));
#endif /* NEWFTP */

  case XZWHO:
    return (hmsg("Syntax: REMOTE WHO [ name ]\n\
  Asks the Kermit server to list who's logged in, or to give information\n\
  about the named user.  Synonym: RWHO."));
#endif /* NOFRILLS */

#ifndef NOSPL
  case XZQUE:
    return (
        hmsg("Syntax: [ REMOTE ] QUERY { KERMIT, SYSTEM, USER } variable-name\n\
  Asks the Kermit server to send the value of the named variable of the\n\
  given type, and make it available in the \\v(query) variable.  When the\n\
  type is KERMIT functions may also be specified as if they were variables."));

  case XZASG:
    return (hmsg("Syntax: REMOTE ASSIGN variable-name [ value ]\n\
  Assigns the given value to the named global variable on the server.\n\
  Synonyms: RASG, RASSIGN."));
#endif /* NOSPL */

  case XZPWD:
    return (hmsg(
#ifdef NEWFTP
        "Syntax: REMOTE PWD\n\
  Asks the Kermit server to display its current working directory.\n\
  Synonym: RPWD."));
#else
        "Syntax: REMOTE PWD\n\
  Asks the Kermit or FTP server to display its current working directory.\n\
  Synonym: RPWD."));
#endif /* NEWFTP */

  case XZXIT:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE EXIT\n\
   Asks the Kermit server to exit (without disconnecting), or closes an FTP\n\
   connection.  Synonym: REXIT, and (for FTP only) BYE, FTP BYE."));
#else
    return (hmsg("Syntax: REMOTE EXIT\n\
  Asks the Kermit server to exit.  Synonym: REXIT."));
#endif /* NEWFTP */

  case XZSTA:
    return (hmsg("Syntax: REMOTE STATUS\n\
  Asks the remote Kermit server for information about itself.  Typically\n\
  this would include the name and version of Kermit program,the underlying\n\
  hardware/architecture, operating system, current directory, and the\n\
  details of the most recent file transfer (if any)."));

  case XZCDU:
#ifdef NEWFTP
    return (hmsg("Syntax: REMOTE CDUP\n\
  Asks the Kermit or FTP server to change its working directory to\n\
  the directory above it.  Synonym: RCDUP."));
#else
    return (hmsg("Syntax: REMOTE CDUP\n\
  Asks the Kermit server to change its working directory to the directory\n\
  above it.  Synonym: RCDUP."));
#endif /* NEWFTP */

  default:
    if ((x = cmcfm()) < 0)
      return (x);
    printf("?Sorry, no help available - \"%s\"\n", cmdbuf);
    return (-9);
  }
}
#endif /* NOXFER */
#endif /* NOHELP */
#endif /* NOICP */
