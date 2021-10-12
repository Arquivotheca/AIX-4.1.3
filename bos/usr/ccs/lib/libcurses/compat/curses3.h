/* @(#)42    1.1  src/bos/usr/ccs/lib/libcurses/compat/curses3.h, libcurses, bos411, 9428A410j 2/9/94 07:20:47 */
#ifndef _H_CURSES
#define _H_CURSES

/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: curses3.h
 *
 *   ORIGINS: 3,10,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#ifndef WINDOW

#ifndef _H_NLCHAR
#include <sys/NLchar.h>
#endif

# ifndef 	NONSTANDARD
#  include  <stdio.h>
  /*
   * This is used to distinguish between USG and V7 systems.
   * Assume that L_ctermid is only defined in stdio.h in USG
   * systems, but not in V7 or Berkeley UNIX.
   */
#  ifdef L_ctermid
#  define USG
#  endif
#  include  <unctrl.h>
#  ifdef USG
#   include <termio.h>
   typedef struct termio SGTTY;
#  else
#   include <sgtty.h>
   typedef struct sgttyb SGTTY;
#  endif
# else          /* NONSTANDARD */
/*
 * NONSTANDARD is intended for a standalone program (no UNIX)
 * that manages screens.  The specific program is Alan Hewett's
 * ITC, which runs standalone on an 11/23 (at least for now).
 * It is unclear whether this code needs to be supported anymore.
 */
# define NULL ((void *)0)
# endif		/* NONSTANDARD */

/*
 * chtype is the type used to store a character together with attributes.
 * It can be set to "char" to save space, or "long" to get more attributes.
 */
# ifdef	CHTYPE
	typedef	CHTYPE chtype;
# else
	typedef unsigned long chtype;
# endif /* CHTYPE */

# define        TRUE    1
# define        FALSE   0

# define        ERR     (-1)
# define        OK      0

# define	_SUBWIN		01
# define	_ENDLINE	02
# define	_FULLWIN	04
# define	_SCROLLWIN	010
# define	_FLUSH		020
# define	_ISPAD		040
# define	_STANDOUT	0x80000000
# define        _NOCHANGE       (-1)

typedef char bool ; /* bool type required by X/open */

struct _win_st {
	short	_cury, _curx;
	short	_maxy, _maxx;
	short	_begy, _begx;
	short   _flags;
	chtype  _attrs;
	char    _clear;
	char    _leave;
	char    _scroll;
	char    _use_idl;
	char    _use_keypad;    /* 0=no, 1=yes, 2=yes/timeout */
	char    _use_meta;      /* T=use the meta key */
	char    _nodelay;       /* T=don't wait for tty input */
	chtype	**_y;
	short	*_firstch;
	short	*_lastch;
	short	_tmarg,_bmarg;
};

extern int	LINES, COLS;

typedef struct _win_st	WINDOW;
extern WINDOW	*stdscr, *curscr;

extern char	*Def_term, ttytype[];

typedef struct screen	SCREEN;

# ifndef NOMACROS 
#  ifndef MINICURSES
/*
 * psuedo functions for standard screen
 */
# define	addch(ch)	waddch(stdscr, ch)
# define	getch()		wgetch(stdscr)
# define	NLgetch()	NLwgetch(stdscr)
# define	addstr(str)	waddstr(stdscr, str)
# define	getstr(str)	wgetstr(stdscr, str)
# define	move(y, x)	wmove(stdscr, y, x)
# define	clear()		wclear(stdscr)
# define	erase()		werase(stdscr)
# define	clrtobot()	wclrtobot(stdscr)
# define	clrtoeol()	wclrtoeol(stdscr)
# define	insertln()	winsertln(stdscr)
# define	deleteln()	wdeleteln(stdscr)
# define	refresh()	wrefresh(stdscr)
# define	inch()		winch(stdscr)
# define	insch(c)	winsch(stdscr,c)
# define	delch()		wdelch(stdscr)
# define	standout()	wstandout(stdscr)
# define	standend()	wstandend(stdscr)
# define	attron(at)	wattron(stdscr,at)
# define	attroff(at)	wattroff(stdscr,at)
# define	attrset(at)	wattrset(stdscr,at)

# define	setscrreg(t,b)	wsetscrreg(stdscr, t, b)
# define	wsetscrreg(win,t,b)	(win->_tmarg=(t),win->_bmarg=(b))

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define	mvwgetstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	(wmove(win,y,x)==ERR?ERR:winch(win))
#define	mvwdelch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wdelch(win))
#define	mvwinsch(win,y,x,c)	(wmove(win,y,x)==ERR?ERR:winsch(win,c))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define	mvgetstr(y,x,str)	mvwgetstr(stdscr,y,x,str)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)
#define	mvdelch(y,x)		mvwdelch(stdscr,y,x)
#define	mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

#  else /* MINICURSES */

# define	addch(ch)		m_addch(ch)
# define	addstr(str)		m_addstr(str)
# define	move(y, x)		m_move(y, x)
# define	clear()			m_clear()
# define	erase()			m_erase()
# define	refresh()		m_refresh()
# define	standout()		wstandout(stdscr)
# define	standend()		wstandend(stdscr)
# define	attron(at)		wattron(stdscr,at)
# define	attroff(at)		wattroff(stdscr,at)
# define	attrset(at)		wattrset(stdscr,at)
# define	mvaddch(y,x,ch)		move(y, x), addch(ch)
# define	mvaddstr(y,x,str)	move(y, x), addstr(str)
# define	initscr			m_initscr
# define	newterm			m_newterm

/*
 * These functions don't exist in minicurses, so we define them
 * to nonexistent functions to help the user catch the error.
 */
#define	getch		m_getch
#define	getstr		m_getstr
#define	clrtobot	m_clrtobot
#define	clrtoeol	m_clrtoeol
#define	insertln	m_insertln
#define	deleteln	m_deleteln
#define	inch		m_inch
#define	insch		m_insch
#define	delch		m_delch
/* mv functions that aren't valid */
#define	mvwaddch	m_mvwaddch
#define	mvwgetch	m_mvwgetch
#define	mvwaddstr	m_mvaddstr
#define	mvwgetstr	m_mvwgetstr
#define	mvwinch		m_mvwinch
#define	mvwdelch	m_mvwdelch
#define	mvwinsch	m_mvwinsch
#define	mvgetch		m_mvwgetch
#define	mvgetstr	m_mvwgetstr
#define	mvinch		m_mvwinch
#define	mvdelch		m_mvwdelch
#define	mvinsch		m_mvwinsch
/* Real functions that aren't valid */
#define box		m_box
#define delwin		m_delwin
#define longname	m_longname
#define makenew		m_makenew
#define mvprintw	m_mvprintw
#define mvscanw		m_mvscanw
#define mvwin		m_mvwin
#define mvwprintw	m_mvwprintw
#define mvwscanw	m_mvwscanw
#define newwin		m_newwin
#define _outchar        m_outchar
#define overlay		m_overlay
#define overwrite	m_overwrite
#define printw		m_printw
#define putp		m_putp
#define scanw		m_scanw
#define scroll		m_scroll
#define subwin		m_subwin
#define touchwin	m_touchwin
#define _tscroll        m_tscroll
#define _tstp		m_tstp
#define vidattr		m_vidattr
#define waddch		m_waddch
#define waddstr		m_waddstr
#define wclear		m_wclear
#define wclrtobot	m_wclrtobot
#define wclrtoeol	m_wclrtoeol
#define wdelch		m_wdelch
#define wdeleteln	m_wdeleteln
#define werase		m_werase
#define wgetch		m_wgetch
#define wgetstr		m_wgetstr
#define winsch		m_winsch
#define winsertln	m_winsertln
#define wmove		m_wmove
#define wprintw		m_wprintw
#define wrefresh	m_wrefresh
#define wscanw		m_wscanw
#define setscrreg	m_setscrreg
#define wsetscrreg	m_wsetscrreg

#  endif /* MINICURSES */

/*
 * psuedo functions
 */

#define	getyx(win,y,x)	 y = win->_cury, x = win->_curx
#define	winch(win)	 (win->_y[win->_cury][win->_curx])
#define flushok(win,bf)  (bf ? (win->_flags|=_FLUSH):(win->_flags&=~_FLUSH))

WINDOW	*initscr(), *newwin(), *subwin(), *newpad();
char	*longname();
char	erasechar(), killchar();
int	wgetch();	/* because it can return KEY_*, for instance. */
SCREEN	*newterm();

/* Various video attributes */
/*
   We start from the left and attributes bits from left to right to permit
   larger collating sequences.  Add attributes in this fashion if necessary.
*/

#define A_STANDOUT	0x80000000
#define A_UNDERLINE	0x40000000
#define A_REVERSE	0x20000000
#define A_BLINK		0x10000000
#define A_DIM		0x08000000
#define A_BOLD		0x04000000

#define A_INVIS		0x02000000
#define A_PROTECT	0x01000000
#define A_ALTCHARSET	0x00800000

#define A_NORMAL	0x00000000
#define A_ATTRIBUTES	0xff800000
#define A_CHARTEXT	0x007fffff

/* Funny "characters" enabled for various special function keys for input */
/*
 * Under NLS, we start their collation around 3500 octal (1856 decimal)
 * to permit room for NLS characters with large values
*/
#define KEY_BREAK	03501		/* break key (unreliable) */
#define KEY_DOWN	03502		/* The four arrow keys ... */
#define KEY_UP		03503
#define KEY_LEFT	03504
#define KEY_RIGHT	03505		/* ... */
#define KEY_HOME	03506		/* Home key (upward+left arrow) */
#define KEY_BACKSPACE	03507		/* backspace (unreliable) */
#define KEY_F0		03510		/* Function keys.  Space for 64 */
#define KEY_F(n)	(KEY_F0+(n))	/* keys is reserved. */
#define KEY_DL		03610		/* Delete line */
#define KEY_IL		03611		/* Insert line */
#define KEY_DC		03612		/* Delete character */
#define KEY_IC		03613		/* Insert char or enter insert mode */
#define KEY_EIC		03614		/* Exit insert char mode */
#define KEY_CLEAR	03615		/* Clear screen */
#define KEY_EOS		03616		/* Clear to end of screen */
#define KEY_EOL		03617		/* Clear to end of line */
#define KEY_SF		03620		/* Scroll 1 line forward */
#define KEY_SR          03621           /* Scroll 1 line backwards (reverse)*/
#define KEY_NPAGE	03622		/* Next page */
#define KEY_PPAGE	03623		/* Previous page */
#define KEY_STAB	03624		/* Set tab */
#define KEY_CTAB	03625		/* Clear tab */
#define KEY_CATAB	03626		/* Clear all tabs */
#define KEY_ENTER	03627		/* Enter or send (unreliable) */
#define KEY_SRESET      03630           /* soft (partial) reset (unreliable)*/
#define KEY_RESET	03631		/* reset or hard reset (unreliable) */
#define KEY_PRINT	03632		/* print or copy */
#define KEY_LL		03633		/* home down or bottom (lower left) */
					/* The keypad is arranged like this:*/
					/*    a1    up    a3   */
					/*   left   b2  right  */
					/*    c1   down   c3   */
#define KEY_A1		03634		/* upper left of keypad */
#define KEY_A3		03635		/* upper right of keypad */
#define KEY_B2		03636		/* center of keypad */
#define KEY_C1		03637		/* lower left of keypad */
#define KEY_C3		03640		/* lower right of keypad */
#define KEY_ACTION      03641           /* Action key            */
#define KEY_END         03642           /* End key               */
#define KEY_BTAB        03643           /* Back-tab key          */

SCREEN * set_term (SCREEN * n) ; /* prototype for set_term */

# endif /* NOMACROS */
#endif /* WINDOW */

#endif /* _H_CURSES */

