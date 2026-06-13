# makefile / Makefile / ckuker.mak / CKUKER.MAK
#
# Sun Mar 24 13:21:46 2024
BUILDID=20250322
CKVER= "10.0 Beta.12"
#
# -- Makefile to build C-Kermit for UNIX and UNIX-like platforms --
#
# Copyright (C) 1985, 2024,
#   Trustees of Columbia University in the City of New York.
#   All rights reserved.  See the C-Kermit COPYING.TXT file or the
#   copyright text in the ckcmai.c module for disclaimer and permissions.
#   In case you can't find the COPYING.TXT file, as of 2011 it contains the
#   Simplified 3-Clause BSD License, which is an Open Source license.
#
# Note: As of 3 February 2024, the ckcwart.c and ckcpro.w files have been
# reinstated.  They are not used when building C-Kermit, but any changes to
# to the Kermit protocol itself must be made ckcpro.w, and then you must:
#
#   make wart
#   make ckcpro.c
#
# before building C-Kermit (e.g. with "make linux").
#
# Author: Frank da Cruz (principal author)
# Email:  fdc@kermitproject.org
# Web:    https://kermitproject.org
#
# Example for Linux:
# 1. cd to your C-Kermit source-code directory
# 2. make clean (to delete object files from previous builds)
# 3. rm wermit  (remove any previous binary)
# 4. make linux (to build a version for your Linux machine)
# 5. make check (to check the result - depends on step 3)
# 6. test it and report any any problems by email to the address above
# 7. mv it to where it should reside, e.g. mv wermit /usr/local/bin/kermit
# 8. chmod 755 or 775 if necessary and give appropriate owner and gruop
# .. Many variations possible; see https://www.kermitproject.org/ckccfg.html
#
# Shortcut - Steps 4 and 5 can be combined: "make linux check"
#
# Note: Author is no longer at Columbia University or at the 115th Street
# address as of 1 July 2011.  Even so, C-Kermit remains Copyright Columbia
# University because that is where it was first written in 1985 and further
# developed through mid-2011.
#
# Contributions from many others.  Special thanks to Jeff Altman for the
# secure-build targets, Peter Eichhorn, assyst GmbH, for the consolidated
# HP-UX targets and the "uninstall" target, to Robert Lipe for the updated
# and consolidated SCO UNIX / ODT / OSR5 targets, to Ric Anderson for the
# IRIX 6.x targets, to Seth Theriault for major improvements to the
# Mac OS X targets, and to Alexey Dokuchaev for FreeBSD 9.0.
#
# C-Kermit is written and produced by hand without any external automated
# procedures such as autoconf / automake / configure, although some of the
# targets below (especially the linux target) inspect the environment and make
# some decisions in the most portable way possible. The automated tools are
# not used because (a) C-Kermit predates them, and (b) they are not portable
# to all the platforms where C-Kermit must be (or once was) built, (c) the
# automated tools are always changing, and (d) to keep C-Kermit as independent
# as possible from external tools over which we have no control.
#
# Most entries use the "xermit" target, which uses the select()-based CONNECT
# module, ckucns.c.  The "wermit" target uses the original fork()-based
# CONNECT module, ckucon.c, which has some drawbacks but was portable to every
# Unix variant whether it had TCP/IP or not (select() is part of the TCP/IP
# library, which was not standard on older Unixes).  If your target still uses
# the "wermit" target, please try substituting the "xermit" one and if it
# works, let us know (mailto:fdc@columbia.edu).  When changing a target over
# from wermit to xermit, also remove -DNOLOEARN.
#
# CAREFUL: Don't put the lowercase word "if", "define", or "end" as the first
# word after the "#" comment introducer in the makefile, even if it is
# separated by whitespace.  Some versions of "make" understand these as
# directives, which older make versions do not understand.  Uppercase letters
# remove the danger, e.g. "# If you have..."
# 
# WARNING: This is a huge makefile.  Although it is less likely since the
# turn of the century, some "make" programs might run out of memory.  If this
# happens to you, edit away the parts that do not apply to your platform and
# try again.
#
# WARNING 2: In many cases this file invokes itself recursively, sometimes
# several levels deep (as in the Linux targets); i.e. some targets are used
# as 'subroutines' of other targets, with parameters passed by setting
# environment variables.  For that reason, don't use 'make -e'.
#
# Certain UNIX variations have their own separate makefiles:
#  . For 2.10 or 2.11 BSD on the DEC PDP-11, use ckubs2.mak.
#  . For Plan 9, use ckpker.mk.
#
# Separate build procedures are provided non-UNIX platforms: VMS, VOS,
# AOS/VS, etc.  See the ckaaaa.txt file or the Kermit website for details.
#
#
# DIRECTIONS FOR UNIX
#
# Rename this file to "makefile" or "Makefile" if necessary.  Pick out the
# entry most appropriate for your UNIX version from the list below and then
# give the appropriate "make" command, for example "make linux", "make macos",
# "make freebsd".  If you experience any difficulties with the build procedure,
# then please also read any comments that accompany the make entry itself
# (search for the make entry name on the left margin).
#
# Other targets:
#  'make install' is an installation script (read accompanying comments!).
#  'make uninstall' undoes 'make install' (read accompanying comments!).
#  'make clean' removes intermediate and object files.
#  'make show' tells the default include and lib paths for secure builds.
#
# IMPORTANT:
#   For more detailed installation instructions, read the files ckuins.txt
#   and ckccfg.txt, also available at the Kermit website in HTML form:
#   http://www.columbia.edu/kermit/ckuins.html
#   http://www.columbia.edu/kermit/ckccfg.html
#
#  For descriptions of known problems and limitations,
#   read the files ckcbwr.txt and ckubwr.txt (the "beware files") or:
#   http://www.columbia.edu/kermit/ckcbwr.html
#   http://www.columbia.edu/kermit/ckubwr.html
#
# Most targets build C-Kermit with its symbol table included.  To reduce the
# size of the executable program, add "LNKFLAGS=-s" to the end of your 'make'
# command or to the makefile entry, or 'strip' the executable after
# building.  To further reduce the size after building, use 'mcs -d' if your
# Unix version has such a command.  For further details on size reduction, read
# ckccfg.txt to find out how to remove features that you don't need.
#
# TCP/IP networking support: If your C-Kermit version does not include TCP/IP
# networking, but your UNIX system does, try adding -DTCPSOCKET to the CFLAGS
# of your makefile entry.  If that doesn't work, look at some of the other
# targets that include this flag for ideas about what libraries might need to
# be included (typically -lsocket and/or -lBSD and/or -lnsl and/or -linet).
# NOTE: In some cases (old versions of SCO or HP-UX), you might need not only
# a C compiler, but also a "TCP/IP developers kit" for the required object
# libraries and header files.
#
# Please report modifications, failures (preferably with fixes) or successes
# to the author, fdc@columbia.edu.
#
# TARGETS FOR DIFFERENT UNIX PLATFORMS AND VERSIONS:
#
# + Marks those that have been built successfully for C-Kermit 9.0 or later.
# - Those that once built OK but no longer do (e.g. too big).
# ? Those that worked in a previous version but have not been tested recently.
# --------------------------
# Some commonly used targets:
#
# + "make linux"   - Linux, any version, any architecture (auto-detecting).
#                    Sub-variants: "make linux-clang" (build with clang),
#                    "make linux-nonet" / "make linux-notcp" (no networking),
#                    "make linux-nodeprecated", "make linux-pedantic".
# + "make macos"   - macOS 10.12 (Sierra) and later, Intel or Apple Silicon.
# + "make freebsd" - FreeBSD 4.1 or later (auto-detecting).
# + "make netbsd"  - NetBSD, any version.  Sub-variants: "make netbsd-clang",
#                    "make netbsdnc" (no curses), "make netbsdn" (ncurses),
#                    "make netbsd-nonet" / "make netbsd-notcp",
#                    "make netbsd-nodeprecated", "make netbsd-pedantic".
# + "make openbsd" - OpenBSD 2.3 or later.
# + "make mirbsd"  - MirBSD.
#
# This tree builds only on Linux, macOS, and the modern BSDs.  Targets for
# obsolete platforms (legacy vendor Unixes, old BSD/macOS releases, etc.)
# have been removed; see the SIMPLIFY_*.md / OS_SIMPLIFY_*.md reports.
#
#
# The result should be a runnable program called "wermit" in the current
# directory.  After satisfactory testing, you can rename wermit to "kermit"
# and put it in some directory that's in everybody's PATH, such as
# /usr/local or /opt/local.
#
# To remove intermediate and object files, "make clean".
# If your C compiler produces files with an extension other than "o",
# then "make clean EXT=u", "make clean EXT=s", or whatever.
#
# To run lint on the source files, "make lintbsd".
#
# The following symbols are used to specify library and header file locations.
# prefix statement changed in 10.0 Beta.06 to allow prefix to be specified
# from the make command line e.g. $ env PREFIX=/usr/pkg make install.
# October 2022.
# 
prefix  = $${PREFIX:-/usr/local}
manroot = $(prefix)

###########################################################################
#
#  Compile and Link variables:
#
#  EXT is the extension (file type) for object files, normally o.
#  See MINIX entry for what to do if another filetype must be used.
#
EXT=o
#LNKFLAGS=
SHAREDLIB=
CC= cc
CC2= cc
MAKE= make
SHELL=/bin/sh

###########################################################################
# (Ancient) UNIX V7-specific variables.
# These are set up for Perkin-Elmer 3230 V7 Unix:
#
PROC=proc
DIRECT=
NPROC=nproc
NPTYPE=int
BOOTFILE=/edition7
#
# ( For old Tandy TRS-80 Model 16A or 6000 V7-based Xenix, use PROC=_proc,
#   DIRECT=-DDIRECT, NPROC=_Nproc, NPTYPE=short, BOOTFILE=/xenix )
#

###########################################################################
# SAMPLE INSTALLATION SCRIPT
#
# Modify to suit your own computer's file organization and permissions.  If
# you don't have write access to the destination directories, "make install"
# fails.  In most cases, a real installation also requires you to chown /
# chgrp the Kermit binary for the UUCP lockfile and/or tty devices, and
# perhaps also to chmod +s the corresponding permission fields.
#
# Default binary, man, and doc directories are supplied below.  You can
# override them in your 'make' command.  Examples:
#
#   make install                                   # Accept defaults.
#   make "INFODIR=/usr/share/lib/kermit" install   # Override INFODIR default.
#
# You can also build and install in one step, e.g.:
#
#   make linux install
#
# If you use the 'install' target to install C-Kermit, it creates an
# UNINSTALL script that can be used to uninstall it.
#
WERMIT = makewhat
BINARY = wermit
DESTDIR =
BINDIR = $(prefix)/bin
MANDIR = $(manroot)/man/man1
MANEXT = 1
SRCDIR =
INFODIR =

TEXTFILES = COPYING.TXT ckcbwr.txt ckubwr.txt ckuins.txt ckccfg.txt \
		ckcplm.txt ckermit.ini ckermod.ini ckermit70.txt ckermit80.txt

# How many targets?
count:
	@grep -c '^[^#[:space:]].*:' makefile

# List all targets
list:
	@grep '^[^#[:space:]].*:' makefile | sed 's/:.*$/:/'
	# @grep '^[^#[:space:]].*:' makefile

ALL = $(WERMIT)

all: $(ALL)

.c.o:
	$(CC) $(CFLAGS) -DKTARGET=\"$(KTARGET)\" -c $<

#Clean up intermediate and object files
clean:
	@echo 'Removing object files...'
	-rm -f ckcmai.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckcpro.$(EXT) ckcfns.$(EXT) \
ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) ckucon.$(EXT) ckutio.$(EXT) \
ckufio.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) ckwart.$(EXT) ckuusx.$(EXT) \
ckuusy.$(EXT) ckcnet.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) ckusig.$(EXT) \
ckucns.$(EXT) ckcmdb.$(EXT) ckctel.$(EXT) ckclib.$(EXT) \
ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
ckcpro.c wart

show:
	@echo prefix=$(prefix)
	@echo manroot=$(manroot)
	@exit

# Install C-Kermit after building -- IMPORTANT: Read the instructions above
# (SAMPLE INSTALLATION SCRIPT).
#
# To make sure 'man' notices the new source file and doesn't keep
# showing the old formatted version, remove the old formatted version,
# something like this:
#		rm -f $(MANDIR)/../cat$(MANEXT)/kermit.$(MANEXT)
# or this (which requires CATDIR to be defined):
#		rm -f $(CATDIR)/kermit.$(MANEXT)
#
# As of C-Kermit 8.0.205 this target also builds an UNINSTALL script, and
# so it might be too long for some old Bourne shells, in which case you can
# use a different shell:
#
#   make SHELL=ksh install
#   make SHELL=/bin/posix/sh install
#
# POSTSCRIPT November 2022...  This target can not possibly cover all the
# possible scenarios.  For example, installation on a multiuser timesharing
# system by a sysadmin with root privilege versus installation on a desktop
# by a home user who doesn't even know what "root" is.  Not to mention
# differences among BSD, Linux, macOS, and the hundreds of historical Unix
# versions that this makefile still aims to support.  If there is to be an
# install script at this point, it makes more sense for it to be a Kermit
# script.  I hope to find the the time to write one in time for the C-Kermit
# 10.0 release.  - fdc, Thu Nov 24 08:13:18 2022
#
install:
	@echo Installing C-Kermit version $(CKVER)...;\
	rm -f UNINSTALL;\
	exec 3>./UNINSTALL;\
	echo "# C-Kermit UNINSTALL script" >&3;\
	echo "# `date`\n" >&3;\
	echo "CKVER=$(CKVER)" >&3;\
	echo "PrN Uninstalling C-Kermit version $(CKVER)..." >&3;\
	echo DESTDIR=$(DESTDIR);\
	if test -n "$(DESTDIR)"; then\
		if test -d $(DESTDIR); then\
			echo  "$(DESTDIR) exists...\n";\
		else\
			echo "Creating $(DESTDIR)...";\
			DESTDIR=`echo $(DESTDIR) | sed 's!/*$$!!'`;\
			mkdir $$DESTDIR  || exit 1;\
		fi;\
		chmod 755 $(DESTDIR) || exit 1;\
	fi;\
	echo BINARY=$(BINARY);\
	if test -f $(BINARY); then\
		ls -l $(BINARY);\
	else\
		echo "?$(BINARY) not found";\
		exit 1;\
	fi;\
	if test -z "$(DESTDIR)$(BINDIR)"; then\
		echo "Binary directory not specified";\
		exit 1;\
	fi;\
	if test -d $(DESTDIR)$(BINDIR); then\
		echo  "$(DESTDIR)$(BINDIR) exists...";\
	else\
		echo "Creating $(DESTDIR)$(BINDIR)/...";\
		mkdir     $(DESTDIR)$(BINDIR) || exit 1;\
		chmod 755 $(DESTDIR)$(BINDIR);\
	fi;\
	rm -f $(DESTDIR)$(BINDIR)/kermit;\
	cp $(BINARY) $(DESTDIR)$(BINDIR)/kermit || exit 1;\
	chmod 755    $(DESTDIR)$(BINDIR)/kermit || exit 1;\
	rm -f        $(DESTDIR)$(BINDIR)/kermit-sshsub;\
	ln -s        $(DESTDIR)$(BINDIR)/kermit\
		     $(DESTDIR)$(BINDIR)/kermit-sshsub || exit 1;\
	echo 'set flag=f\nPrC Removing binaries' >&3;\
	echo "RmF $(DESTDIR)$(BINDIR)/kermit-sshsub" >&3;\
	echo "RmF $(DESTDIR)$(BINDIR)/kermit" >&3;\
	if test -f ckermit.ini; then\
		echo "#!$(BINDIR)/kermit" >\
			$(DESTDIR)$(BINDIR)/_tmp.ini;\
		cat ckermit.ini >> $(DESTDIR)$(BINDIR)/_tmp.ini;\
		mv $(DESTDIR)$(BINDIR)/_tmp.ini\
		   $(DESTDIR)$(BINDIR)/ckermit.ini;\
		chmod 755 $(DESTDIR)$(BINDIR)/ckermit.ini;\
		echo "RmF $(DESTDIR)$(BINDIR)/ckermit.ini" >&3;\
	fi;\
	echo;\
	echo 'EfM' >&3;\
	echo "Kermit binary installed:";\
	ls -l $(DESTDIR)$(BINDIR)/kermit\
	      $(DESTDIR)$(BINDIR)/kermit-sshsub\
	      $(DESTDIR)$(BINDIR)/ckermit.ini;\
	echo;\
	echo " WARNING: If C-Kermit is to be used for dialing out,";\
	echo " you must change its owner and group and permissions";\
	echo " to match the 'cu' program.  See the ckuins.txt file";\
	echo " for details.";\
	echo;\
	echo MANDIR=$(MANDIR);\
	if test -n "$(DESTDIR)$(MANDIR)"; then\
		if test -d $(DESTDIR)$(MANDIR); then\
			echo  "$(DESTDIR)$(MANDIR) exists...";\
		else\
			echo "Creating $(MANDIR)...";\
			mkdir $(MANDIR) || exit 1;\
			chmod 755 $(MANDIR) || exit 1;\
		fi;\
		rm -f $(DESTDIR)$(MANDIR)/kermit.$(MANEXT);\
		cp ckuker.nr $(DESTDIR)$(MANDIR)/kermit.$(MANEXT) || exit 1;\
		chmod 644 $(DESTDIR)$(MANDIR)/kermit.$(MANEXT) || exit 1;\
		echo 'set flag=f\nPrC Removing man pages' >&3;\
		echo "RmF $(DESTDIR)$(MANDIR)/kermit.$(MANEXT)" >&3;\
		echo 'EfM' >&3;\
		echo;\
	else\
		echo "Not installing man page!\n";\
	fi;\
	echo SRCDIR=$(DESTDIR)$(SRCDIR);\
	if test -n "$(SRCDIR)"; then\
		echo "Installing source files...";\
		if test -d $(DESTDIR)$(SRCDIR); then\
			echo  "$(DESTDIR)$(SRCDIR) exists...";\
		else\
			echo "Creating $(DESTDIR)$(SRCDIR)/...";\
			mkdir     $(DESTDIR)$(SRCDIR) || exit 1;\
			chmod 755 $(DESTDIR)$(SRCDIR);\
		fi;\
		echo "Copying source files to $(DESTDIR)$(SRCDIR)...";\
		echo 'set flag=f\nPrC Removing source files' >&3;\
		for TextFile in COPYING.TXT ck[cuw_]*.[cwh] makefile; do\
			cp $$TextFile $(DESTDIR)$(SRCDIR)/ && echo ".\c";\
			echo "RmF $(DESTDIR)$(SRCDIR)/$$TextFile" >&3;\
		done; echo;\
		echo 'EfM' >&3;\
		( cd $(DESTDIR)$(SRCDIR)/ &&\
		ls -l COPYING.TXT ck[cuw_]*.[cwh] makefile );echo;\
	else\
		echo "Not installing source code!\n";\
	fi;\
	echo INFODIR=$(DESTDIR)$(INFODIR);\
	if test -n "$(INFODIR)"; then\
		echo "Installing info files...";\
		if test -d $(DESTDIR)$(INFODIR); then\
			echo  "$(DESTDIR)$(INFODIR) exists...";\
		else\
			echo "Creating $(DESTDIR)$(INFODIR)/...";\
			mkdir     $(DESTDIR)$(INFODIR) || exit 1;\
			chmod 755 $(DESTDIR)$(INFODIR);\
		fi;\
		echo "Copying text files to $(DESTDIR)$(INFODIR)...";\
		echo 'set flag=f\nPrC Removing text files' >&3;\
		FileCopyList='';\
		for TextFile in $(TEXTFILES); do\
			test -f $$TextFile || continue;\
			cp $$TextFile $(DESTDIR)$(INFODIR) && echo ".\c" &&\
			FileCopyList="$$FileCopyList $$TextFile";\
			echo "RmF $(DESTDIR)$(INFODIR)/$$TextFile" >&3;\
		done; echo;\
		echo 'EfM' >&3;\
		( cd $(DESTDIR)$(INFODIR)/ && chmod  644   $$FileCopyList );\
		( cd $(DESTDIR)$(INFODIR)/ && pwd && ls -l $$FileCopyList );\
	else\
		echo "Not installing text files!\n";\
	fi;\
	echo "set flag=d\nPrN Removing empty dirs..." >&3;\
	echo "RmD $(DESTDIR)$(BINDIR)" >&3;\
	echo "RmD $(DESTDIR)$(SRCDIR)" >&3;\
	echo "RmD $(DESTDIR)$(INFODIR)" >&3;\
	echo "RmD $(MANDIR)" >&3;\
	echo "RmD $(DESTDIR)" >&3;\
	echo "EfM" >&3;\
	echo "PrN C-Kermit version $(CKVER) is uninstalled!" >&3;\
	echo C-Kermit version $(CKVER) installed!

# UN-Install C-Kermit after building
# Please to not remove the extra blanks before and after '{}' within the
# functions. You would get syntax errors for some older Bourne shells! Best is 
# you don't change or remove anything.
#
uninstall:
	@if test ! -f UNINSTALL; then\
		echo "?C-Kermit UNINSTALL data file not found!";\
		exit 1;\
	fi; \
	X=`grep '^CKVER='$(CKVER)'$$' ./UNINSTALL || :`;\
	if test -z "$$X"; then\
		echo "?UNINSTALL file is not for C-Kermit version $(CKVER)";\
		exit 2;\
	fi;\
	PrN () { echo "$$*"; };\
	PrC () { echo "$$* \c"; };\
	RmF () { test -f "$$1" && rm -f "$$1" && echo ".\c" && flag=F ; };\
	RmD () { \
	dir=$$1;\
	while test -d "$$dir"; do\
		rmdir "$$dir" 2>&- || return && echo "$$dir" && flag=D;\
		dir=`echo "$$dir" | sed 's!/[^/]*/*$$!!'`;\
	done; \
	};\
	EfM () { \
	case "$$flag" in\
		f) echo "- Nothing to remove!";;\
		d) echo "Nothing to remove!";;\
		F) echo " done";;\
		D) echo "done";;\
	esac; \
	};\
	while read Act Args; do\
		case $$Act in\
			EfM) EfM;;\
			RmD) RmD $$Args;;\
			RmF) RmF $$Args;;\
			PrN) PrN $$Args;;\
			PrC) PrC $$Args;;\
			set) eval $$Args;;\
		esac;\
	done < ./UNINSTALL

makewhat:
	@echo 'make what?  You must tell which platform to make C-Kermit for.'
	@echo Examples: make linux, make macos, make freebsd, make openbsd.
	@echo Please read the comments at the beginning of the makefile.

###########################################################################
#
# Dependencies Section:

wermit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
	$(CC2) $(LNKFLAGS) -o wermit \
		ckcmai.$(EXT) ckclib.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckcpro.$(EXT) ckucmd.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) \
		ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
		ckuusx.$(EXT) ckuusy.$(EXT) ckuusr.$(EXT) ckucon.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) \
		ckusig.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
		$(LIBS)

# Preferred configuration with select()-based CONNECT

xermit:	ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) ckuus2.$(EXT) \
		ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) \
		ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) ckcpro.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckucns.$(EXT) ckutio.$(EXT) ckufio.$(EXT) ckudia.$(EXT) \
		ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) \
		ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o wermit \
		ckcmai.$(EXT) ckclib.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) ckuxla.$(EXT) \
		ckcpro.$(EXT) ckucmd.$(EXT) ckuus2.$(EXT) ckuus3.$(EXT) \
		ckuus4.$(EXT) ckuus5.$(EXT) ckuus6.$(EXT) ckuus7.$(EXT) \
		ckuusx.$(EXT) ckuusy.$(EXT) ckuusr.$(EXT) ckucns.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckusig.$(EXT) \
		ckctel.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT) \
		$(LIBS)

# Malloc Debugging version

mermit:	ckcmdb.$(EXT) ckcmai.$(EXT) ckclib.$(EXT) ckucmd.$(EXT) ckuusr.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckcpro.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) ckcfn3.$(EXT) \
		ckuxla.$(EXT) ckucon.$(EXT) ckutio.$(EXT) ckufio.$(EXT) \
		ckudia.$(EXT) ckuscr.$(EXT) ckcnet.$(EXT) ckctel.$(EXT) \
		ckusig.$(EXT) ckcuni.$(EXT) ckupty.$(EXT) ckcftp.$(EXT)
	$(CC2) $(LNKFLAGS) -o mermit ckcmdb.$(EXT) ckclib.$(EXT) ckcmai.$(EXT)\
		ckutio.$(EXT) ckufio.$(EXT) ckcfns.$(EXT) ckcfn2.$(EXT) \
		ckcfn3.$(EXT) ckuxla.$(EXT) ckcpro.$(EXT) ckucmd.$(EXT) \
		ckuus2.$(EXT) ckuus3.$(EXT) ckuus4.$(EXT) ckuus5.$(EXT) \
		ckuus6.$(EXT) ckuus7.$(EXT) ckuusx.$(EXT) ckuusy.$(EXT) \
		ckuusr.$(EXT) ckucon.$(EXT) ckudia.$(EXT) ckuscr.$(EXT) \
		ckcnet.$(EXT) ckctel.$(EXT) ckusig.$(EXT) ckcuni.$(EXT) \
		ckupty.$(EXT) ckcftp.$(EXT) $(LIBS)

###########################################################################
# man page...
#
ckuker.nr:
	@echo This target is obsolete.
	@echo The ckuker.nr file no longer needs any preprocessing.

###########################################################################
# Dependencies for each module...
#
ckcmai.$(EXT): ckcmai.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcnet.h ckcsig.h \
		ckuusr.h ckctel.h ckclib.h ckcfnp.h

ckclib.$(EXT): ckclib.c ckclib.h ckcdeb.h ckcasc.h ckcsym.h ckcfnp.h

wart: ckwart.$(EXT)
	$(CC) $(LNKFLAGS) -o wart ckwart.$(EXT) $(LIBS)

ckcpro.c: ckcpro.w wart ckcdeb.h ckcsym.h ckcasc.h ckcker.h ckcnet.h ckctel.h \
	 ckclib.h
	./wart ckcpro.w ckcpro.c

ckcfns.$(EXT): ckcfns.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h ckcuni.h \
		ckuxla.h ckclib.h ckcnet.h ckcfnp.h

ckcfn2.$(EXT): ckcfn2.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h ckctel.h ckclib.h ckcnet.h ckcuni.h ckcfnp.h

ckcfn3.$(EXT): ckcfn3.c ckcker.h ckcdeb.h ckcsym.h ckcasc.h ckcxla.h \
		ckuxla.h ckclib.h ckcuni.h ckcfnp.h

ckuxla.$(EXT): ckuxla.c ckcker.h ckcsym.h ckcdeb.h ckcxla.h ckuxla.h ckclib.h \
		 ckcuni.h ckcfnp.h

ckcuni.$(EXT): ckcuni.c ckcdeb.h ckcker.h ckucmd.h ckcuni.h ckcxla.h ckuxla.h \
		  ckcfnp.h

ckuusr.$(EXT): ckuusr.c ckucmd.h ckcker.h ckuusr.h ckcsym.h ckcdeb.h ckcxla.h \
		ckuxla.h ckcasc.h ckcnet.h ckctel.h ckclib.h ckcuni.h ckcfnp.h

ckuus2.$(EXT): ckuus2.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h ckcfnp.h

ckuus3.$(EXT): ckuus3.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h ckcfnp.h

ckuus4.$(EXT): ckuus4.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckuver.h ckcsym.h ckctel.h ckclib.h \
		ckcuni.h ckcfnp.h

ckuus5.$(EXT): ckuus5.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h ckctel.h ckclib.h ckcxla.h ckuxla.h ckcuni.h ckcfnp.h

ckuus6.$(EXT): ckuus6.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcnet.h \
		 ckcsym.h ckctel.h ckclib.h ckcfnp.h

ckuus7.$(EXT): ckuus7.c ckucmd.h ckcker.h ckuusr.h ckcdeb.h ckcxla.h ckuxla.h \
		ckcasc.h ckcnet.h ckcsym.h ckctel.h ckclib.h ckcuni.h ckcfnp.h

ckuusx.$(EXT): ckuusx.c ckcker.h ckuusr.h ckcdeb.h ckcasc.h ckcsym.h \
		ckcsig.h ckcnet.h ckctel.h ckclib.h ckcxla.h ckuxla.h \
		ckcuni.h ckcfnp.h

ckuusy.$(EXT): ckuusy.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h ckcfnp.h

ckucmd.$(EXT): ckucmd.c ckcasc.h ckucmd.h ckcdeb.h ckcsym.h ckctel.h ckclib.h \
		ckcfnp.h

ckufio.$(EXT): ckufio.c ckcdeb.h ckuver.h ckcsym.h ckclib.h \
		ckcxla.h ckuxla.h ckcuni.h ckcfnp.h

ckutio.$(EXT): ckutio.c ckcdeb.h ckcnet.h ckuver.h ckcsym.h ckctel.h ckclib.h \
		ckcfnp.h

ckucon.$(EXT): ckucon.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h ckcfnp.h

ckucns.$(EXT): ckucns.c ckcker.h ckcdeb.h ckcasc.h ckcnet.h ckcsym.h ckctel.h \
		 ckclib.h ckcxla.h ckuxla.h ckcuni.h ckcfnp.h

ckcnet.$(EXT): ckcnet.c ckcdeb.h ckcker.h ckcnet.h ckcsym.h ckcsig.h ckctel.h \
		 ckclib.h ckcfnp.h ckuusr.h

ckctel.$(EXT): ckcsym.h ckcdeb.h ckcker.h ckcnet.h ckctel.h ckclib.h ckcfnp.h

ckcmdb.$(EXT): ckcmdb.c ckcdeb.h ckcsym.h ckclib.h ckcfnp.h

ckudia.$(EXT): ckudia.c ckcker.h ckcdeb.h ckucmd.h ckcasc.h ckcsym.h ckcsig.h \
		ckcnet.h ckctel.h ckclib.h ckcfnp.h

ckuscr.$(EXT): ckuscr.c ckcker.h ckcdeb.h ckcasc.h ckcsym.h ckcsig.h \
		ckcnet.h ckctel.h ckclib.h ckcfnp.h

ckusig.$(EXT): ckusig.c ckcasc.h ckcdeb.h ckcker.h ckcnet.h ckuusr.h \
		ckcsig.h ckctel.h ckclib.h ckcfnp.h

ckcftp.$(EXT): ckcftp.c ckcdeb.h ckcasc.h ckcker.h ckucmd.h ckuusr.h \
		ckcnet.h ckctel.h ckcxla.h ckuxla.h ckcuni.h ckcfnp.h

ckupty.$(EXT): ckupty.c ckupty.h ckcdeb.h ckcfnp.h

###########################################################################
#
# Entries to make C-Kermit for specific systems.
#
# Put the ones that need short makefiles first.

# WARNING: The early BSD entries do not build in version 7.0 with the stock
# BSD compiler: "Too many defines".  Unless you can rebuild cpp to have more
# space for defines, these builds must be accomplished by:
# copying the /usr/include tree to someplace else, preprocessing there with cc
# -E -I./include or whatever (plus all the same -D's, adding any necessary
# -U/-D to override the architecture)), renaming the the resulting files back
# to their original names, bringing them back to the original BSD system, and
# running the make target there.  This technique was used for 4.2 and 4.3 BSD
# on a VAX in C-Kermit 7.0 (later, cpp on that machine was rebuilt to allow
# more symbols, so the C-Kermit 8.0 build proceeds normally).

#FreeBSD 4.1 and above
#Like FreeBSD 4.0 but without the NONOSETBUF hack and with CK_NEWTERM.
#New stanza 27 June 2023 to squelch "sys/timeb.h deprecated" warning.
#Most recently tested on FreeBSD 13.1
#
freebsd freebsd41 freebsd72 freebsd5 freebsd6 freebsd7 freebsd8 freebsd9:
	@echo 'Making C-Kermit $(CKVER) for FreeBSD 4.1 or later...'
	@if test `uname -r | cut -d . -f 1` -ge 8; then \
	   HAVE_FBSD8='-DFREEBSD8'; \
	else HAVE_FBSD8=''; fi; \
	if test `uname -r | cut -d . -f 1` -ge 9; then \
	HAVE_FBSD9='-DFREEBSD9'; \
	else HAVE_FBSD9='';  fi; \
	if test -f /usr/include/utmpx.h ; \
	then HAVE_UTMPX='-DHAVEUTMPX' ; \
	else HAVE_UTMPX='' ; fi; \
	if test -f /usr/include/sys/wait.h ; \
	then HAVE_WAITH='-DHAVEWAITH' ; \
	else HAVE_WAITH='' ; fi; \
	NOSYSTIMEBH="-DNOSYSTIMEBH" ; \
	if test -f /usr/include/sys/timeb.h ; \
	then x=`grep deprecated /usr/include/sys/timeb.h | wc -l` ; \
	if [ $x > 0 ] ; \
	then NOSYSTIMEBH='' ; fi ; fi ; \
	if `grep -q "[[:space:]]utimes" /usr/include/sys/time.h` ; \
	then HAVE_UTIMES='-DHAVE_UTIMES' ; \
	else HAVE_UTIMES=''; fi; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_NCURSES -DCK_NEWTERM -DTCPSOCKET -DNOCOTFMC \
	-DFREEBSD4 $$HAVE_FBSD8 $$HAVE_FBSD9 -DUSE_UU_LOCK -DFNFLOAT \
	$$HAVE_UTMPX $$HAVE_WAITH $$NOSYSTIMEBH $$HAVE_UTIMES \
	-DHERALD=\"\\\" `uname -rs`\\\"\" \
	-funsigned-char -DTPUTSARGTYPE=int -DUSE_STRERROR $(KFLAGS) \
	-O2 -pipe"\
	"LIBS= -lncurses -lcrypt -lutil -lm $(LIBS)"

#NetBSD 1.4.1 or later with vanity banner automated with uname
#and automatic inclusion of large file support if it is available.
#This target tested successfully on NetBSD 1.4.1, 1.5.2, and 2.0.3 (Jan 2006).
#Fails on NetBSD 2.0 on Sun/3 mc68030 with gcc 3.3.3 unless optimization is
#disabled on ckcfn2.c ("KFLAGS=-O0") (Letter O Digit Zero).
#(This could be automated by testing `uname -m` for "sun3".)
#OK: 2011/06/15 on NetBSD 1.5.2 and 5.1.
#NetBSD 4.1: have to include <time.h>.
#OK: 2011/08/21 on 5.1.
#OK: (many more up through NetBSD 9.2)
#OK: NetBSD 8.x
#OK: 2020/08/24 NetBSD 9.0
#OK: 2022/10/02 NetBSD 9.3
# `uname -r | grep "[6789].[0-9]" > /dev/null && echo '-DTIMEH'`
# Note: netbsd15 and 15 are for 1.5 and 1.6 (not 15 and 16).
netbsd netbsd2:
	@echo Making C-Kermit $(CKVER) for NetBSD `uname -r` with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=`grep fseeko /usr/include/stdio.h > /dev/null && \
	echo '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'` \
	-DTIMEH	-DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR \
	-funsigned-char -DHERALD=\"\\\" `uname -s -r`\\\"\" \
	-DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -lcurses -lcrypt -lm -lutil $(LIBS)" ; $(shell ./ckubuildlog)

buildlog:
	@(shell ./ckubuildlog -v)

netbsd-clang netbsdclang:
	# Dummy comment \
	@echo 'Making C-Kermit $(CKVER) for Linux with Clang compiler'
	$(MAKE) CC=clang CC2=clang KTARGET=$${KTARGET:-$(@)} \
	"LNKFLAGS = $(LNKFLAGS)" \
	netbsd

# NetBSD with pedantic warnings (cc is gcc).
netbsd-pedantic:
	@echo Making C-Kermit $(CKVER) for NetBSD `uname -r` gcc pedantic...
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-Wpedantic $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	netbsd

# NetBSD with no TCP/IP or any other kind of networking
# but with the external SSH client
netbsd-nonet:
	@echo Making C-Kermit $(CKVER) for NetBSD `uname -r` with no TCP/IP...
	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNONET -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	netbsd

# NetBSD with no TCP/IP but with the external SSH client
netbsd-notcp:
	@echo Making C-Kermit $(CKVER) for NetBSD `uname -r` with no TCP/IP...
	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNOTCPIP -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	netbsd

# NetBSD with "legacy" and deprecated features removed:
# FTP, Telnet, Rlogin, Wtmp logging, and arrow keys, 
# which depend on a deprecated API that has no undeprecated replacement.
netbsd-nodeprecated: \
	# Dummy comment \
	@echo 'Making C-Kermit $(CKVER) for Linux without deprecated features'
	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNODEPRECATED $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)" \
	netbsd

#NetBSD with curses left out (e.g. for use as IKSD).
netbsdnc:
	@echo Making C-Kermit $(CKVER) for NetBSD with no curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) netbsd KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=-DNOCURSES"

#NetBSD with ncurses requested explicitly rather than curses-which-is-ncurses
netbsdn:
	@echo Making C-Kermit $(CKVER) for NetBSD with curses...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=`grep fseeko /usr/include/stdio.h > /dev/null && \
	echo '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'` \
	-DBSD44 -DCK_CURSES -DTCPSOCKET -DUSE_STRERROR \
	-DHERALD=\"\\\" NetBSD `uname -r`\\\"\" \
	-DCK_DTRCD -DCK_DTRCTS -DTPUTSARGTYPE=int -DFNFLOAT $(KFLAGS) -O" \
	"LIBS= -L/usr/pkg/lib -lncurses -lcrypt -lm -lutil $(LIBS)"

#OpenBSD 2.3 or later
#Add -DMAINTYPE=int if you get complaints about main: return type is not int.
#For C-Kermit 8.0 (Christian Weisgerber):
# -ltermlib removed (presumably because -lcurses==ncurses already includes it)
# -DUSE_UU_LOCK and -lutil added for uu_lock()
# -DNDSYSERRLIST changed to -DUSE_STRERROR
#If this gives you trouble use the previous entry.
#29 April 2023 for C-Kermit 10.0: New clauses to account the presence
# or absence of term.h (curses) sys/timeb.h (dates and times).
openbsd:
	@echo Making C-Kermit $(CKVER) for OpenBSD 2.3 or later...
	NOTIMEBH='' ;
	if test -f /usr/include/sys/timeb.h ; \
	then if `grep deprecated /usr/include/sys/timeb.h` ; \
	then NOSYSTIMEBH='-DNOSYSTIMEBH' ; fi ; \
	else NOSYSTIMEBH='-DNOSYSTIMEBH' ; fi; \
	if test -f /usr/include/sys/term.h ; \
	then HAVETERMH='-DHAVETERMH' ; \
	else HAVETERMH='' ; fi; \
	if test -f /usr/include/sys/wait.h ; \
	then HAVE_WAITH='-DHAVEWAITH' ; \
	else HAVE_WAITH='' ; fi; \
	if `grep -q "[[:space:]]utimes" /usr/include/sys/time.h` ; \
	then HAVE_UTIMES='-DHAVE_UTIMES' ; \
	else HAVE_UTIMES=''; fi; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	$$NOSYSTIMEBH -DHERALD=\"\\\" OpenBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR $$HAVETERMH $$HAVE_WAITH \
	$$HAVE_UTIMES \
	$(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm"

#Better to chain to the openbsd target but...
mirbsd:
	@echo Making C-Kermit $(CKVER) for OpenBSD 2.3 or later...
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS= -DBSD44 -DCK_CURSES -DCK_NEWTERM -DTCPSOCKET -DOPENBSD \
	-DHERALD=\"\\\" MirBSD `uname -r`\\\"\" \
	-DUSE_UU_LOCK -DFNFLOAT -DUSE_STRERROR $(KFLAGS) -O" \
	"LIBS= -lcurses -lutil -lm"

# Mac OS X 10 early versions.
# For 10.3.9 and later, use the macosx target below.

# Additional target "macos" by Tony Nicholson 4-Nov-2021.
#
# More recent macOS re-branded releases for Intel (x86_64) Sierra (10,12),
# High Sierra (10,13), Mojave (10.14), Catalina (10.15), and Intel x86_64/ARM
# Big Sur (11), Monterey (12)
#
# Apple's Clang C compiler reports many warnings that can safely be ignored -
# so keep reporting the ones that may have not been detected by other
# compilers by selectively disabling dangling-else, string-compare and
# parentheses related warnings.  -DNOWTMP added because it always provokes
# a "deprecated" warning.  Wtmp logging is only for IKSD, so if you're
# not going to be using macOS C-Kermit as an IKSD server, no worries.
# If you are, and want the server to make Wtmp log entries, do
# 'make macos "KFLAGS=-UNOWTMP".
#
# -Xlinker flags added to LIBS to supress the following warning:
# ld: warning: reducing alignment of section __DATA,__common
#     from 0x8000 to 0x4000 because it exceeds segment maximum alignment
# (Tested on macOS 26.5.1)
#
macos:
	@MACOSNAME=`/usr/bin/sw_vers -productName`; \
	MACOSV=`/usr/bin/sw_vers -productVersion`; \
	echo Making C-Kermit $(CKVER) for $$MACOSNAME $$MACOSV... ; \
	MACCPU=$$HOSTTYPE; \
	HAVE_UTMPX=''; \
	$(MAKE) CC=$(CC) CC2=$(CC2) xermit KTARGET=$${KTARGET:-$(@)} \
	"CFLAGS=-Wno-dangling-else -Wno-string-compare -Wno-parentheses \
	-Wno-pointer-sign -Wno-unused-value -Wdeprecated-declarations \
	-DMACOSX10 -DMACOSX103 -DCK_NCURSES -DTCPSOCKET -DCKHTTP \
	-DUSE_STRERROR -DUSE_NAMESER_COMPAT -DNOCHECKOVERFLOW -DFNFLOAT \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $$HAVE_UTMPX \
	-funsigned-char -DNODCLINITGROUPS \
	-DNOUUCP -O -DHERALD=\"\\\" $${MACOSNAME} $${MACOSV}\\\"\" \
	-DCKCPU=\"\\\"$${MACCPU}\\\"\" \
	$(KFLAGS)" \
	"LIBS= -lncurses -lresolv -Xlinker -max_default_common_align -Xlinker 0x4000 $(LIBS)"

# End of Mac OS X Section

#NeXTSTEP 3.3.
#Includes fullscreen file transfer display and TCP/IP.
# You might have to add 1 line to 1 NeXT header file <ip.h>
# to declare n_long as u_long by adding #include <bsd/netinet/in_systm.h>

#### IBM RT PC - these targets were last verified in C-Kermit 8.0.211.

#### IBM AIX.  The first two targets should work for any version of AIX
#### from 4.2 onwards.  The ones after that are for older versions or
#### specific configurations, and/or with gcc.

# Old AIX versions...

# None of the following aix43gcc attempts work on a gcc-only AIX 4.3.3 box.
# It just plain can't find the math routines (fmod, pow, exp, sqrt, log10,...)
# Which is odd because nm /usr/lib/libC.a finds them...

#The following (old, old) sunosxxx entries are for debugging and testing only.

# (End of SunOS test entries...)

# AT&T 7300 UNIX PC.  As of C-Kermit 6.1, many of these entries don't work
# any more due to "Out of memory" or "Too many defines" errors during
# compilation, at least not on systems without lots of memory.  The sys3upcgc
# entry works (using gcc) with optimization removed, and might also work
# with optimization enabled on machines with larger memories.

# The next two are likely not to work as-is.

# Peter E's updated HP-UX 5.xx entries Oct 2001.

# Linux 1.2 or later with gcc, dynamic libraries, ncurses, TCP/IP.
#
# If your Linux system has curses rather than ncurses, use the linuxc
# entry, or if that doesn't work, linuxnc.
#
# The Kermit "large memory model" is used by default to configure big packet
# and script buffers, etc.  For small-memory or limited-resource systems,
# "make linux KFLAGS=-DNOBIGBUF".
#
# -DLINUXFSSTND (Linux File System Standard 1.2) gives UUCP lockfile /var/lock
# with string pid.  Remove this to get /usr/spool/uucp with int pid, used in
# very early Linux versions.  FSSTND 1.2 also says that the PID string in the
# UUCP lock file has leading spaces.  This is a change from FSSTND 1.0, which
# used leading zeros.  Add -DFSSTND10 to support FSSTND 1.0 instead of 1.2.
# I hope subsequent editions of the file-system standard did not change these
# again.
#
# Add -DOLINUXHISPEED (Old Linux High Speed support) to turn on an ugly kludge
# in Linux 1.0 and earlier to support speeds of 57600 and 115200.  Extremely
# old Linux systems (pre-0.99pl15) will not support this.  If OLINUXHISPEED is
# not defined, then only the standard POSIX termios methods of setting the port
# speed will be used, and in this case speeds can be as high as 460800 in most
# modern Linux versions.
#
# -DCK_POSIX_SIG (POSIX signal handling) is good for Linux releases back to at
# least 0.99.14; if it causes trouble for you, remove it from the CFLAGS.
#
# -pipe removes the need for temp files - remove it if it causes trouble.
#
# -funsigned-char makes all characters unsigned, as they should have been
#  in the first place.
#
# Add -DCK_DSYSINI if you want a shared system-wide init file.
#
# See http://www.columbia.edu/kermit/ckubwr.html about -DNOCOTFMC.
# Better still, should read the entire Linux section of that document.
#
# The "linuxa" entry can be referenced directly on LIBC systems, but not
# GLIBC, where -lcrypt is required.  The "make linux" entry should normally
# be used for all builds on all Linux distributions unless you have special
# requirements, in which case keep reading.  CK_NEWTERM added after 7.0b04
# due to new complaints about ncurses changing buffering of tty.
#
# By the way, the trick for testing if a lib exists ("if ld -lncurses ...")
# might seem crazy but it works everywhere, whereas the more appropriate test
# ("if locate libncurses") is not necessarily available on all Linuxes.
linuxa:
	@echo 'Making C-Kermit $(CKVER) for Linux 1.2 or later...'
	@echo 'IMPORTANT: Read the comments in the linux section of the'
	@echo 'makefile if you have trouble.'
	$(MAKE) xermit KTARGET=$${KTARGET:-$(@)} "CC=$(CC)" "CC2=$(CC)" \
	"CFLAGS = -std=gnu17 -O -DLINUX -pipe -funsigned-char -DFNFLOAT -DCK_POSIX_SIG \
	-DCK_NEWTERM -DTCPSOCKET -DLINUXFSSTND -DNOCOTFMC -DPOSIX \
	-DUSE_STRERROR $(KFLAGS)" "LNKFLAGS = $(LNKFLAGS)" \
	"LIBS = $(LIBS) -lm"

#Linux.  Completely new target: 18 January 2016.  No more looking in 100
#different places for libraries, let ld do it, since it knows what libraries
#it's going to use.  If this target fails to work somewhere, use the
#'linux-2015' target just below this one.
#
#This entry should work for any Linux distribution on any platform,
#32-bit or 64-bit, except for extremely ancient ones.  Automatically detects:
# . curses, ncurses, or no curses
# . Old versus new pty handling (new == glibc 2.1++)
# . Presence or absence of libcrypt and <crypt.h>
# . Presence or absence of libresolv
# . Presence of various serial port locking schemes
# . Transitional Long File API for 32-bit platforms (SUS V2 UNIX 98 LFS).
#
#Long file support for 32-bit builds added in 8.0.212 - if features.h contains
#__USE_LARGEFILE64 then we set the flags that must be set before reading any
#header files; on 32-bit platforms such as i386, this produces a 32-bit build
#capable of accessing, sending, receiving, and managing long (> 2GB) files.
#On 64-bit platforms, it does no harm.
#
# The clause regarding errno includes "extern int errno" in the source files
# by supplying a symbol DCL_ERRNO if errno is not declared or defined in any
# header files in the /usr/include tree.
#
# This target uses the computer's default C compiler, whatever it is
# (usually gcc, but beware: it might also be clang, which is extremely
# hostile to "old" code.
#
linux gnu-linux:
	@echo "Making C-Kermit for `uname -spm` $(CC) \
`$(CC) -dumpversion`..."; \
	# Dummy comment \
	DCL_ERRNO='-DDCL_ERRNO';  \
	if egrep -r \
	  "(int *errno|\#define *errno|\# *define *errno)" /usr/include/* \
	  > /dev/null 2> /dev/null; \
	then DCL_ERRNO=''; \
	fi ; \
	if test `grep grantpt /usr/include/*.h | wc -l` -gt 0; \
	  then if test -c /dev/ptmx; \
	    then HAVE_PTMX='-DHAVE_PTMX'; \
	    else HAVE_PTMX=''; \
	  fi; \
	fi ; \
	HAVE_OPENPTY=''; \
	if test `grep openpty /usr/include/*.h | wc -l` -gt 0; then \
	  HAVE_OPENPTY='-DHAVE_OPENPTY';  \
	fi; \
	if test -n '$$HAVE_OPENPTY'; \
	  then if ld -lutil > /dev/null 2> /dev/null; then \
	    LIB_UTIL='-lutil'; \
	  else \
	    LIB_UTIL=''; \
	  fi; \
	fi; \
	HAVE_LIBCURSES=''; \
	HAVE_CURSES=''; \
	if ld -lncurses > /dev/null 2> /dev/null; then \
	  HAVE_LIBCURSES='-lncurses'; \
	  if test -f /usr/include/ncurses.h; then \
	    HAVE_CURSES='-DCK_NCURSES  -I/usr/include/ncurses'; \
	  else \
	    HAVE_LIBCURSES=''; \
	  fi; \
	fi; \
	if test -z '$$HAVE_LIBCURSES'; then \
	  if ld -lcurses > /dev/null 2> /dev/null; then \
	    HAVE_LIBCURSES='-lcurses'; \
	    if test -f /usr/include/curses.h; then \
	      HAVE_CURSES='-DCK_CURSES  -I/usr/include/curses'; \
	    else \
	      HAVE_LIBCURSES=''; \
	    fi; \
	  fi; \
	fi; \
	HAVE_RESOLV=''; \
	if ld -lresolv > /dev/null 2> /dev/null; then \
	  HAVE_RESOLV='-lresolv'; \
	fi; \
	HAVE_CRYPT=''; \
	HAVE_CRYPT_H=''; \
	if ld -lcrypt > /dev/null 2> /dev/null; then \
	  if test -f /usr/include/crypt.h; then \
	    HAVE_CRYPT_H='-DHAVE_CRYPT_H'; \
	    HAVE_CRYPT='-lcrypt'; \
	  fi; \
	fi; \
	if test -f /usr/include/baudboy.h ; \
	  then HAVE_BAUDBOY='-DHAVE_BAUDBOY' ; \
	  else HAVE_BAUDBOY=''; \
	fi; \
	if test -n '$$HAVE_BAUDBOY' || test -f /usr/include/ttylock.h; \
	  then HAVE_LOCKDEV='-DHAVE_LOCKDEV' ; \
	  else HAVE_LOCKDEV='' ; \
	fi ; \
	if test -n '$$HAVE_LOCKDEV'; then \
	  if ld -llockdev > /dev/null 2> /dev/null; then \
	    HAVE_LIBLOCKDEV='-llockdev'; \
	  else \
	    HAVE_LOCKDEV=''; \
	  fi; \
	fi; \
	if grep __USE_LARGEFILE64 /usr/include/features.h > /dev/null; \
	  then HAVE_LARGEFILES='-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64'; \
	  else HAVE_LARGEFILES=''; \
	fi; \
	$(MAKE) KTARGET=$${KTARGET:-$(@)} \
	"KFLAGS=$$HAVE_CURSES $$HAVE_PTMX $$HAVE_LOCKDEV $$HAVE_CRYPT_H \
	$$HAVE_BAUDBOY $$HAVE_OPENPTY $$HAVE_LARGEFILES $(KFLAGS)" \
	"LIBS=$(LIBS) $$LIB_UTIL \
	  $$HAVE_LIBCURSES $$HAVE_RESOLV $$HAVE_CRYPT $$HAVE_LOCKDEV" \
	linuxa

# Force compilation with clang
linux-clang linuxclang:
	$(MAKE) CC=clang CC2=clang linux

# Linux with "legacy" and "deprecated" features removed:
# FTP, Telnet, Rlogin, Wtmp logging.  And arrow keys, which depend on
# a deprecated API that as yet has no undeprecated replacement. 
linux-nodeprecated: \
	# Dummy comment \
	@echo 'Making C-Kermit $(CKVER) for Linux without deprecated features'
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNODEPRECATED $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"

# Linux with pedantic warnings (force gcc)
linux-pedantic:
	@echo Making C-Kermit $(CKVER) for Linux `uname -r` gcc pedantic...
	$(MAKE) "CC=gcc" "CC2=gcc" linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-pedantic $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"

# Linux with no TCP/IP support but with the external ssh client
linux-notcp:
	@echo Making C-Kermit $(CKVER) for Linux `uname -r` NO TCP/IP...
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNOTCPIP -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"

# Linux with no TCP/IP support but with the external ssh client
linux-nonet:
	@echo Making C-Kermit $(CKVER) for Linux `uname -r` NO NETWORKING...
	$(MAKE) linux KTARGET=$${KTARGET:-$(@)} \
	KFLAGS="-DNONET -DNONETDIR -DSSHCMD -DANYSSH $(KFLAGS)" \
	"LNKFLAGS = $(LNKFLAGS)"

# Secure targets for Linux.  These work on RHAS4, RHEL4, and RHEL5,
# unlike some of the older targets that follow.  They hook into the main Linux
# target so we pick up all the other new stuff - large files, baudboy.h, the
# appropriate pty interface, etc.

# ::BEGIN_OLD_LINUX_TARGETS::

# The remaining Linux entries are for special or customized builds.  They have
# not been generalized ("subroutinized") like the ones above.  Ideally, we
# should allow for every combination of libc vs glibc, gcc vs egcs, curses vs
# ncurses, and so on.
# The best way to do this is to set KFLAGS and LIBS values and then chain to
# the main "linux" target, as in the examples just above.  To skip past all of
# these old targets (and there are many) search for ::END_OLD_LINUX_TARGETS::
# (after this line).

# ::END_OLD_LINUX_TARGETS::

#SCO UNIX (and ODT) entries...
#
#NOTE: All SCO UNIX entry LIBS should have "-lc_s -lc -lx" IN THAT ORDER (if
#shared C library is desired), or else "-lc -lx" IN THAT ORDER.  Use shared C
#libraries to save memory, but then don't expect to run the resulting binary
#on a different machine.  When using -lc_s, you must also use -lc, because the
#shared C library does not contain all of libc.a.  And in all cases, -lc must
#ALWAYS precede -lx.
#
#ANOTHER NOTE: -DRENAME is included in all SCO UNIX entries.  Remove it if it
#causes trouble.  No harm is done by removing it (see ckuins.txt).
#
#AND ANOTHER: In theory, it should be possible to run SCO UNIX binaries on
#SCO Xenix 2.3 and later.  In practice, this might not work because of the
#libraries, etc.  Also, don't add the -link -z switch (which is supposed to
#root out references to null pointers) because it makes UNIX binaries core
#dump when they are run under Xenix.

# SCO 3.2v4.x targets...

#  NOTE: Add -DDCLPOPEN and/or -DDCLFDOPEN to anySCO 3.2v4.x non-gcc entries
#  that complain about fdopen() or popen() at compile time.  They compile OK
#  without these flags as of July 1999.  However, the gcc entries seem to
#  need them, at least for gcc 2.7.2.2.

#  NOTE 2: To enable IKSD support, add:
#  -DCK_LOGIN -DNOGETUSERSHELL -DNOINITGROUPS
#  to CFLAGS (not tested).

#As above but with floating-point math library support \ffp...() functions
#and S-Expressions.

ckuuid:
	@echo 'building C-Kermit $(CKVER) set-UID/set-GID test programs'
	$(CC) -DANYBSD -DSAVEDUID -o ckuuid1 ckuuid.c
	$(CC) -DANYBSD -o ckuuid2 ckuuid.c
	$(CC) -DANYBSD -DNOSETREU -o ckuuid3 ckuuid.c
	$(CC) -DANYBSD -DSETEUID -DNOSETREU -o ckuuid4 ckuuid.c
	$(CC) -o ckuuid5 ckuuid.c
	@echo 'Read the top of ckuuid.c for directions...for testing'
	@echo 'you must make these programs setuid and setgid'

############################################################################
# A N T I Q U I T I E S
#
# The following are antique targets from C-Kermit 5A or earlier.  They have
# not been updated or tested in years.  Most of them will need recent features
# disabled, usually with some combination of -DNOUNICODE, -DNOIKSD, -DNOANSI,
# -DNOCKGHNLHOST, -DNO_DNS_SRV, -DNOREDIRECT, -DNOREALPATH, -DNOCURSES, etc.
# They are also missing the KTARGET=$${KTARGET:-$(@)} business.
# For details see ckuins.txt and ckccfg.txt.
#
############################################################################

lintbsd:
	@echo 'Running Lint on C-Kermit $(CKVER) sources for BSD 4.2 version..'
	lint -x -DBSD4 -DTCPSOCKET ck[cu]*.c > ckuker.lint.bsd42

#Who remembers TECO?
love:
	@echo 'Not war?'

#Check the most recent build (assuming any previous wermit had been deleted)
check:
	@if test -s wermit -a -x wermit ; then \
	echo SUCCESS:; ls -log wermit ; else \
	echo FAILED; \
	fi
