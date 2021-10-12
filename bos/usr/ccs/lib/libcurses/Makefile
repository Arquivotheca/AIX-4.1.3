# @(#)95    1.10  src/bos/usr/ccs/lib/libcurses/Makefile, libcurses, bos411, 9438C411c 9/23/94 19:37:50
#
#   COMPONENT_NAME: LIBCURSES
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27, 4
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved
#
#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.
#
# ident	"@(#)curses:screen/makefile	1.38"

#
# curses package maker
# makefile for OSF build environment
#
# related cmds built here: tic, captoinfo, infocmp, tput
#
#

TMP     = ./tmp

# Sources

SUBDIRS       		= compat
EXPLIB_SUBDIRS		= compat
EXPINC_SUBDIRS		= compat

PROGRAMS            = tic captoinfo infocmp tput

SHARED_LIBRARIES	= libxcurses.a
SHARED_OFILE_NAME   = shr4.o
SHARED_OFILE_LIST   = shr.o
GARBAGE				= term.h term4.h tnames.c tinames.c tifnames.c keycaps \
			  	  	  curses.h keyname.c termcap.c tiget.c tifget.c
MSGHDRS				= libcurses_msg.h tic_msg.h captoinfo_msg.h \
					  printinfo_msg.h infocmp_msg.h tput_msg.h
CATFILES			= libcurses.cat tic.cat captoinfo.cat printinfo.cat \
					  infocmp.cat tput.cat
HFILES              = ${MSGHDRS} curses.h term.h compiler.h object.h \
		   			  curses_inc.h print.h otermcap.h

SCRIPTS 		 	= keyname

EXPLIB_TARGETS		= export_libxcurses.a

TOBJ                = tnames.o tinames.o tifnames.o

tic_OFILES          = tic_captab.o tic_error.o tic_hash.o tic_main.o \
					  tic_parse.o tic_read.o tic_scan.o ${TOBJ}

captoinfo_OFILES    = captoinfo.o otermcap.o iexpand.o print.o ${TOBJ}

infocmp_OFILES      = infocmp.o cexpand.o infotocap.o print.o iexpand.o \
					  ${TOBJ} setupterm.o termcap.o termerr.o reset_sh.o \
					  tcsearch.o

tput_OFILES         = tput.o ${TOBJ} setupterm.o tiget.o tifget.o \
				      termerr.o reset_sh.o longname.o tparm.o putp.o \
					  tputs.o tcsearch.o vidputs.o delay.o vidupdate.o \
					  curses.o

INCLUDES		= curses.h term.h unctrl.h

ILIST			= ${PROGRAMS} curses.h term.h unctrl.h libxcurses.a
curses.h_IDIR	= /usr/include/
term.h_IDIR		= /usr/include/
unctrl.h_IDIR	= /usr/include/
IDIR			= /usr/ccs/lib/

tic_IDIR		= /usr/bin/
captoinfo_IDIR		= /usr/bin/
infocmp_IDIR		= /usr/bin/
tput_IDIR		= /usr/bin/

shr4.o_EXPORTS			= -bE:libcurses.exp
LIBS			= libc.a 

# The LIBS lines below must exist.  The commands cannot use the LIBS line
# above because they will break (at least in a sandbox).

tic_LIBS        = -ls
captoinfo_LIBS  =
infocmp_LIBS    =
tput_LIBS       =

IMOD			= 644
CFLAGS			= -DVIDEO -DKEYPAD -UNONSTANDARD -DSIGPOLL -D_THREAD_SAFE_ERRNO

SHARED_OFILES		= \
	V3.box.o V3.initscr.o V3.m_addch.o \
	V3.m_addstr.o V3.m_clear.o V3.m_erase.o V3.m_initscr.o \
	V3.m_move.o V3.m_newterm.o V3.m_refresh.o V3.newterm.o \
	V3.pechochar.o V3.upd_old_y.o V3.vidattr.o V3.vidputs.o \
	V3.waddch.o V3.wattroff.o V3.wattron.o V3.wattrset.o  \
	V3.wechochar.o V3.winsch.o \
	_addch.o _addchnstr.o _addchstr.o _addnstr.o _addstr.o  \
	_attroff.o _attron.o _attrset.o _beep.o _bkgd.o _bkgdset.o  \
	_border.o _box.o _clear.o _clrtobot.o _clrtoeol.o _crmode.o  \
	_del_curterm.o _delch.o _deleteln.o _echo.o _echochar.o  \
	_erase.o _fixterm.o _flash.o _garbagdlins.o _garbagedwin.o  \
	_getch.o _getstr.o _halfdelay.o _hline.o _inch.o _inchnstr.o  \
	_inchstr.o _innstr.o _insch.o _insdelln.o _insertln.o  \
	_insnstr.o _insstr.o _instr.o _intrflush.o _meta.o _move.o  \
	_mvaddch.o _mvaddchnstr.o _mvaddchstr.o _mvaddnstr.o _mvaddstr.o  \
	_mvdelch.o _mvgetch.o _mvgetstr.o _mvhline.o _mvinch.o  \
	_mvinchnstr.o _mvinchstr.o _mvinnstr.o _mvinsch.o _mvinsnstr.o  \
	_mvinsstr.o _mvinstr.o _mvvline.o _mvwaddch.o _mvwaddchnst.o  \
	_mvwaddchstr.o _mvwaddnstr.o _mvwaddstr.o _mvwdelch.o  \
	_mvwgetch.o _mvwgetstr.o _mvwhline.o _mvwinch.o _mvwinchnst.o  \
	_mvwinchstr.o _mvwinnstr.o _mvwinsch.o _mvwinsnstr.o  \
	_mvwinsstr.o _mvwinstr.o _mvwvline.o _newterm.o _nl.o  \
	_nocrmode.o _noecho.o _nonl.o _noqiflush.o _overlay.o  \
	_overwrite.o _qiflush.o _refresh.o _resetterm.o _saveterm.o  \
	_scr_init.o _scr_restore.o _scr_set.o _scrl.o _scroll.o  \
	_set_curterm.o _set_term.o _setscrreg.o _slk_init.o  \
	_standend.o _standout.o _subpad.o _timeout.o _touchline.o  \
	_unctrl.o _vline.o _waddchstr.o _waddstr.o _wclear.o  \
	_wdeleteln.o _werase.o _winsertln.o _winsstr.o \
	_wstandend.o _wstandout.o  \
	baudrate.o can_change.o cbreak.o chkinput.o clearok.o  \
	color_cont.o copywin.o curs_set.o curserr.o curses.o  \
	def_prog.o delay.o delay_out.o delkey.o delkeymap.o  \
	delscreen.o delterm.o delwin.o derwin.o doupdate.o  \
	draino.o dupwin.o endwin.o erasechar.o flushinp.o getattrs.o  \
	getbegyx.o getbkgd.o getmaxyx.o getparyx.o getsyx.o  \
	gettmode.o getwin.o getyx.o has_colors.o has_ic.o has_il.o  \
	idcok.o idlok.o immedok.o init_acs.o init_color.o init_costs.o  \
	init_pair.o initscr.o is_linetou.o is_wintou.o keyname.o  \
	keypad.o killchar.o leaveok.o longname.o makenew.o memSset.o  \
	meta.o mvcur.o mvderwin.o mvprintw.o mvscanw.o mvwin.o  \
	mvwprintw.o mvwscanw.o napms.o newkey.o newpad.o newscreen.o  \
	newwin.o nocbreak.o nodelay.o noraw.o notimeout.o outch.o  \
	overlap.o pair_cont.o pechochar.o pnoutref.o prefresh.o printw.o \
	putp.o putwin.o quick_echo.o raw.o redrawwin.o reset_sh.o  \
	resetty.o restart.o ring.o ripoffline.o savetty.o scanw.o  \
	scr_all.o scr_dump.o scr_ll_dump.o scr_reset.o scrollok.o  \
	setcurscreen.o setcurterm.o setecho.o setkeymap.o setnonl.o  \
	setqiflush.o setsyx.o setterm.o setupterm.o slk_atroff.o \
	slk_atron.o slk_atrset.o slk_clear.o slk_label.o slk_noutref.o  \
	slk_refresh.o slk_restore.o slk_set.o slk_start.o slk_touch.o  \
	start_col.o subwin.o syncok.o tcsearch.o termattrs.o termcap.o  \
	termerr.o termname.o tgetch.o tgoto.o tifget.o tifnames.o tiget.o  \
	tinames.o tinputfd.o tnames.o touchwin.o tparm.o tputs.o \
	trace.o tstp.o ttimeout.o typeahead.o unctrl.o ungetch.o  \
	untouchwin.o vidputs.o vidupdate.o vsscanf.o vwprintw.o  \
	vwscanw.o waddch.o waddchnstr.o waddnstr.o wattroff.o  \
	wattron.o wattrset.o wbkgd.o wbkgdset.o wborder.o wclrtobot.o  \
	wclrtoeol.o wdelch.o wechochar.o wgetch.o wgetstr.o whline.o  \
	winch.o winchnstr.o winchstr.o winnstr.o winsch.o winsdelln.o  \
	winsnstr.o winstr.o wmove.o wnoutref.o wprintw.o wredrawln.o  \
	wrefresh.o wscanw.o wscrl.o wsetscrreg.o wsyncdown.o  \
	wsyncup.o wtimeout.o wtouchln.o wvline.o curs_getchtype.o

.include <${RULES_MK}>

# Build Merged Libraries

${SHARED_OFILES}: ${HFILES}

# Rules to make other files

shr.o: compat/shr.o
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}
	${STRIP} -e ${.TARGET}


tnames.c tinames.c tifnames.c:
term.h: caps maketerm.ed copyright.h compat/term3.h
	@if [ ! -d "${TMP}" ] ;\
	then ${MKDIR} ${TMP} ;\
	fi
	${RM} -f term.h term4.h caps copyright.h
	${CP} ${caps:P} caps
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${maketerm.ed:P}
	${ECHO} "#ifndef _AIX32_CURSES\n" > term.h
	${CAT} term4.h >> term.h
	${ECHO} "\n#else /* _AIX32_CURSES */\n" >> term.h
	${CAT} compat/term3.h >> term.h
	${ECHO} "\n#endif /* _AIX32_CURSES */" >> term.h
	${CHMOD} 0664 term.h tnames.c tinames.c tifnames.c
	${RM} -rf caps copyright.h term4.h ${TMP}

keycaps: caps
	${RM} -rf ${TMP}/keycaps keycaps caps
	${GREP} "KEY_" ${caps:P} > keycaps
	@if ${TAIL} -1 keycaps | ${GREP} 'KEY_SUNDO' > /dev/null;then :; \
	else ${ECHO} New keys!!! Must change setkeymap.c ; \
	${RM} -f keycaps ; \
	exit 1 ; \
        fi

curses.h: keycaps curses.ed copyright.h compat/curses3.h
	@if [ ! -d "${TMP}" ] ;\
	then ${MKDIR} ${TMP} ;\
	fi
	${RM} -f curses.h curses4.h copyright.h
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${curses.ed:P}
	${ECHO} "#ifndef _AIX32_CURSES\n" > curses.h
	${CAT} curses4.h >> curses.h
	${ECHO} "\n#else /* _AIX32_CURSES */\n" >> curses.h
	${CAT} ${compat/curses3.h:P} >> curses.h
	${ECHO} "\n#endif /* _AIX32_CURSES */" >> curses.h
	${CHMOD} 0664 curses.h
	${RM} -rf ${TMP}/keycaps copyright.h curses4.h ${TMP}

keyname.c: keycaps keyname copyright.h
	${RM} -f keyname.c copyright.h
	${CP} ${copyright.h:P} copyright.h
	keyname
	${CHMOD} 0664 keyname.c
	${RM} -f copyright.h

termcap.c: caps copyright.h termcap.ed
	@if [ ! -d "${TMP}" ] ;\
	then ${MKDIR} ${TMP} ;\
	fi
	${RM} -f termcap.c caps copyright.h
	${CP} ${caps:P} caps
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${termcap.ed:P}
	${RM} -rf bool num str caps copyright.h ${TMP}
	${CHMOD} 0664 termcap.c

tifget.c: caps copyright.h tifget.ed
	@if [ ! -d "${TMP}" ] ;\
	then ${MKDIR} ${TMP} ;\
	fi
	${RM} -f caps copyright.h tifget.c
	${CP} ${caps:P} caps
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${tifget.ed:P}
	${CHMOD} 0664 tifget.c
	${RM} -rf caps copyright.h ${TMP}

tiget.c: caps copyright.h tiget.ed
	@if [ ! -d "${TMP}" ] ;\
	then ${MKDIR} ${TMP} ;\
	fi
	${RM} -f caps copyright.h tiget.c
	${CP} ${caps:P} caps
	${CP} ${copyright.h:P} copyright.h
	${ED} - < ${tiget.ed:P}
	${CHMOD} 0664 tiget.c
	${RM} -rf caps copyright.h ${TMP}

captoinfo.o:    captoinfo.c curses.h otermcap.h print.h
cexpand.o:      cexpand.c print.h
iexpand.o:      iexpand.c print.h
infotocap.o:    infotocap.c curses.h
infocmp.o:      infocmp.c curses.h term.h print.h
otermcap.o:     otermcap.c otermcap.h
print.o:        print.c curses.h print.h
tic_captab.o:   tic_captab.c compiler.h
tic_error.o:    tic_error.c compiler.h
tic_hash.o:     tic_hash.c compiler.h
tic_main.o:     tic_main.c compiler.h curses.h
tic_parse.o:    tic_parse.c compiler.h object.h
tic_read.o:     tic_read.c object.h
tic_scan.o:     tic_scan.c compiler.h
tput.o:         tput.c

