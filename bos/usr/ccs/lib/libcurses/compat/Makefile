# @(#)35    1.5  src/bos/usr/ccs/lib/libcurses/compat/Makefile, libcurses, bos411, 9439B411a 9/28/94 12:32:05
#
#   COMPONENT_NAME: LIBCURSES
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 10, 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SHARED_LIBRARIES	= libcurses.a

MSGHDRS			= setupterm_msg.h showstring_msg.h
CATFILES		= setupterm.cat	showstring.cat
HFILES			= ${MSGHDRS}
EXPINC_TARGETS	= export_term3.h

EXPLIB_TARGETS  = export_libcurses.a
EXPDIR          = /usr/aix3/lib/

INCLUDES     	= term3.h

EXPORTS			 = -bE:libcurses.exp
LIBS			 = libc.a

CFLAGS=	-DNLS -DVIDEO -DKEYPAD -D_AIX32_CURSES -UNONSTANDARD -DDONOTUSELOCALE

SHARED_OFILES=\
	capnames.o NLgetch.o  __cflush.o __sscans.o   \
	_blanks.o _c_clean.o _clearhl.o _clearline.o \
	_comphash.o _delay.o _delchars.o _dellines.o  \
	_dumpwin.o _ec_quit.o _fixdelay.o _forcehl.o   \
	_hlmode.o _id_char.o _init_cost.o _inschars.o  \
	_insmode.o _kpmode.o _line_free.o _ll_move.o   \
	_outch.o _outchar.o _pos.o _reset.o _scrdown.o   \
	_scrollf.o _sethl.o _setmode.o _setwind.o   \
	_shove.o _sprintw.o _sputc.o _syncmodes.o \
	_tscroll.o _window.o addch.o addstr.o baudrate.o   \
	beep.o box.o cbreak.o chktypeahd.o clear.o clearok.o    \
	clreolinln.o clrtobot.o clrtoeol.o cntcostfn.o crmode.o     \
	curses.o def_prog.o def_shell.o delayoutpt.o delch.o	     \
	deleteln.o delwin.o doupdate.o draino.o echo.o endwin.o     \
	erase.o	erasechar.o fixterm.o flash.o flushinp.o getcap.o     \
	getch.o	getstr.o gettmode.o has_ic.o has_il.o idln.getst.o \
	idlok.o	initkeypad.o initscr.o insch.o insertln.o intrflush.o  \
	keypad.o killchar.o leaveok.o line_alloc.o ll_refresh.o \
	longname.o m_addch.o m_addstr.o m_clear.o m_erase.o m_move.o     \
	m_refresh.o m_tstp.o makenew.o meta.o miniinit.o move.o	     \
	mvcur.o	mvprintw.o mvscanw.o mvwin.o mvwprintw.o mvwscanw.o   \
	naps.o newpad.o newterm.o newwin.o nl.o	nocbreak.o   \
	nocrmode.o nodelay.o noecho.o nonl.o noraw.o nttychktrm.o \
	overlay.o overwrite.o pnoutrfrsh.o prefresh.o printw.o     \
	putp.o raw.o reset_prog.o resetshell.o resetterm.o  \
	resetty.o restarttrm.o saveterm.o savetty.o scanw.o     \
	scroll.o scrollok.o select.o set_term.o setbuffred.o \
	setterm.o setupterm.o showstring.o subwin.o termcap.o    \
	tgoto.o	touchwin.o toucholap.o tparm.o tputs.o traceonoff.o \
	tstp.o two.twostr.o typeahead.o unctrl.o vidattr.o    \
	vidputs.o vsscanf.o wattroff.o wattron.o wattrset.o   \
	wnoutrfrsh.o wprintw.o wrefresh.o writechars.o \
	wscanw.o wstandend.o wstandout.o


.include <${RULES_MK}>



setupterm.o : setupterm_msg.h
showstring.o: showstring_msg.h

# modified make	logic for tic, ntic, capnames.c, termcap.c and term3.h to
# include the update logic in this make	file and automate the update of
# term3.h, termcap.c and capnames.c
#
# - caps controls making both term3.h and capnames.c via	the ed script
#   contained in maketerm.ex
# - term3.h is generated	here and copied	to the include directory for use
#   by other compiles (e.g. lib/cur/initscr.c).
#

capnames.c term3.h: caps maketerm.ex
	${RM} -f caps.e capnames.c term3.h boolcap strcap numcap
	${CP} ${caps:P} caps.e
	${CHMOD} u+w caps.e
	${ED} - < ${maketerm.ex:P}
	${RM} -f caps.e

# - caps also controls the creation of termcap.c via the ed script
# - contained in termcap.ed and	the skeleton file termcap.form
#
termcap.c: caps	termcap.form termcap.ed
	${RM} -f caps.e termcap.form.e termcap.c bool num str
	${CP} ${caps:P} caps.e
	${CP} ${termcap.form:P} termcap.form.e
	${CHMOD} +w termcap.form.e	caps.e
	${ED} - < ${termcap.ed:P}


