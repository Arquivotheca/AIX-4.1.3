# @(#)20    1.5  src/bos/usr/ccs/lib/libcur/Makefile, libcur, bos411, 9428A410j 3/30/94 10:38:46
#
# COMPONENT_NAME:  (LIBCUR) Extended Curses Library
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ${MAKETOP}/bos/usr/ccs/lib/libcurses/compat

SHARED_LIBRARIES	= libcur.a
SHARED_OFILE_NAME   = shr.o
SHARED_OFILE_LIST   = shr32c.o

MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libcur.a

ILIST           = libcur.a
IDIR            = /usr/lib/

#LIBS		= libcurses.a libc.a
LIBS		= libc.a
shr.o_IMPORTS		= -bI:libcurses.imp
shr.o_EXPORTS		= -bE:libcur.exp

CFLAGS          +=  ${_AIX3CURSESFLAGS}
#LIBFLAGS        +=  ${_AIX3CURSESLIBFLAG}

SHARED_OFILES	= \
	addch.o addfld.o addstr.o bell.o box.o chgat.o \
	chkscroll.o clrtobot.o clrtoeol.o cr_put.o cr_tty.o \
	curses.o delch.o deleteln.o delwin.o do_attr.o \
	ecactp.o ecadpn.o ecaspn.o ecblks.o ecbpls.o ecbpns.o \
	ecdfpl.o ecdppn.o ecdspl.o ecdvpl.o ecflin.o \
	ecgtco.o eciorc.o ecloct.o ecmove.o ecobsc.o \
	ecpnin.o ecpspn.o ecrefpan.o ecrfpl.o ecrfpn.o \
	ecrlpl.o ecrmpl.o ecscpn.o ecshpl.o endwin.o \
	getch.o getstr.o initscr.o inch.o insch.o \
	insertln.o keypad.o longname.o makech.o move.o \
	mvprintw.o mvscanw.o mvwin.o newview.o newwin.o overput.o \
	printw.o refresh.o scanw.o scroll.o set_attr.o \
	standout.o touchwin.o tstp.o ttyctl.o unctrl.o \
	vscroll.o

.include <${RULES_MK}>

# Rules to make other files

shr32c.o: ${MAKETOP}/bos/usr/ccs/lib/libcurses/compat/shr.o
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

