/* @(#)20	1.19  src/bos/usr/include/cur01.h, libcurses, bos411, 9428A410j 9/28/93 10:56:27 */
#ifndef _H_CUR01
#define _H_CUR01
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur01.h
 *
 * ORIGINS: 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                cur01.h
 *
 * FUNCTION: This file contains the defines and
 *      declarations needed by most application programs.  It is normally
 *      included by cur00.h for other Ecurses routines.
 *
 * DATA STRUCTURES =    WINDOW (struct _win_st), SGTTY
 *
 * Changes:
 *	1.  8/27/90  add wrefresh function to mvwgetch to move cursor 
 *                   to location in the window where input was expected.
 */

#include	<stdio.h>
#if (IS1|IS2|V7)
#include        <sgtty.h>
#else
#include        <termio.h>
#endif

#include 	<stdlib.h>

#ifndef TRUE
#define TRUE	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

#define ERR	(0)
#define OK	(1)

/* see cur05.h  */
#define  NLSCHAR        wchar_t
#define  NLSBLANK       (NLSCHAR) ' '
#define  NLSNL          (NLSCHAR) '\n'

#define  ATTR           unsigned short

#define WINDOW	struct _win_st

struct _win_st {
    short   _cury,
            _curx;		/* current (y,x) window coordinates */
    short   _maxy,
            _maxx;		/* number of rows and columns */
    short   _begy,
            _begx;		/* starting (y,x) coordinates on glass */
    short   _winy,
            _winx;		/* for a view:	starting (y,x) in original
				   */
    short   _flags;		/* for various #define state flags */
    short  *_firstch;		/* optimization arrays:  first and last   
				*/
    short  *_lastch;		/* position changed on a row by row basis 
				*/
    char    _clear;		/* clear flag */
    char    _leave;		/* cursor leave ok flag */
    char    _scroll;		/* window scroll ok flag */
    ATTR    _csbp;              /* current standout(attribute) bit pattern
				*/
    NLSCHAR **_y;               /* window character array */
    ATTR    **_a;               /* window attribute array */
    struct  _win_st *_view;     /* for a view:  pointer to original window
				*/
    short   _tmarg,
            _bmarg;		/* top and bottom of scrolling region */
};

/* The following #defines are the flags defined within the _flags */
/* field in the WINDOW structure above                            */

#define _SUBWIN 	01
#define _ENDLINE	02
#define _FULLWIN	04
#define _SCROLLWIN	010
#define _FLUSH		020
#define _ISVIEW 	040
#define _HASVIEW	0100
#define _STANDOUT	0200
#define _NOCHANGE	-1


extern  int     LINES,
		COLS;

extern  WINDOW  *stdscr,
		*curscr;

 /* The following external definitions are used by the box drawing     */
 /* functions to specify the normal characters to be used in the boxes */

extern char     *BX,
		*BY;

/* The following integers are used to specify the display attribute */
/* mask to the colorout function.                                   */

extern int  NORMAL,
            STANDOUT,
            REVERSE,
            BOLD,
            UNDERSCORE,
            TOPLINE,
            BOTTOMLINE,
            RIGHTLINE,
            LEFTLINE,
            DIM,
            INVISIBLE,
            PROTECTED,
            BLINK,
            F_WHITE,
            F_RED,
            F_BLUE,
            F_GREEN,
            F_BROWN,
            F_MAGENTA,
            F_CYAN,
            F_BLACK,
            B_BLACK,
            B_RED,
            B_BLUE,
            B_GREEN,
            B_BROWN,
            B_MAGENTA,
            B_CYAN,
            B_WHITE,
            FONT0,
            FONT1,
            FONT2,
            FONT3,
            FONT4,
            FONT5,
            FONT6,
            FONT7;

extern char ESCSTR[];

/*
 * Define VOID to stop lint from generating "null effect" comments.
 */
#ifdef lint
int     __void__;
#define VOID(x) (__void__ = (int) (x))
#else
#define VOID(x) (x)
#endif

/*
 * psuedo functions for standard screen
 */
#define chgat(num, mod) VOID(wchgat(stdscr, num, mod))
#define addch(ch)	VOID(waddch(stdscr, ch))
#define getch() 	VOID(wgetch(stdscr))
#define addstr(str)	VOID(waddstr(stdscr, str))
#define getstr(str)	VOID(wgetstr(stdscr, str))
#define move(y, x)	VOID(wmove(stdscr, y, x))
#define clear() 	VOID(wclear(stdscr))
#define erase() 	VOID(werase(stdscr))
#define clrtobot()	VOID(wclrtobot(stdscr))
#define clrtoeol()	VOID(wclrtoeol(stdscr))
#define insertln()	VOID(winsertln(stdscr))
#define deleteln()	VOID(wdeleteln(stdscr))
#define refresh()	VOID(wrefresh(stdscr))
#define inch()		VOID(winch(stdscr))
#define insch(c)	VOID(winsch(stdscr, c))
#define delch() 	VOID(wdelch(stdscr))
#define standout()	VOID(xstandout(stdscr, 1 ))
#define standend()	VOID(xstandend(stdscr))
#define colorout(a)	VOID(xstandout(stdscr, a))
#define colorend()	VOID(xstandend(stdscr))
#define overwrite(a,b)	VOID(overput(a, b, TRUE))
#define overlay(a, b)	VOID(overput(a, b, FALSE))

#define wcolorout(w,a)	VOID(xstandout(w, a))
#define wstandout(win)	VOID(xstandout(win, 1 ))
#define wstandend(win)	VOID(xstandend(win))
#define wcolorend(win)	VOID(xstandend(win))

#define setscrreg(t,b)          VOID(wsetscrreg(stdscr, t, b))
#define wsetscrreg(win,t,b)     VOID((win->_tmarg=(t),win->_bmarg=(b)))

/*
 * mv functions
 */
#define mvwchgat(win,y,x,n,m)	VOID(wmove(win,y,x)==ERR?ERR:wchgat(win,n,m))
#define mvwaddch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define mvwgetch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:(wrefresh(win),wgetch(win)))
#define mvwaddstr(win,y,x,str)	VOID(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define mvwgetstr(win,y,x,s)	VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win,s))
#define mvwinch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:winch(win))
#define mvwdelch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:wdelch(win))
#define mvwinsch(win,y,x,c)	VOID(wmove(win,y,x)==ERR?ERR:winsch(win,c))
#define mvchgat(y,x,n,m)        mvwchgat(stdscr,y,x,n,m)
#define mvaddch(y,x,ch) 	mvwaddch(stdscr,y,x,ch)
#define mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define mvgetstr(y,x,str)	mvwgetstr(stdscr,y,x,str)
#define mvinch(y,x)		mvwinch(stdscr,y,x)
#define mvdelch(y,x)            mvwdelch(stdscr,y,x)
#define mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

/*
 * psuedo functions
 */

#define box(w,b,c)                                                          \
	superbox1(w,0,0,(w)->_maxy,(w)->_maxx,b,c,c,c,c,c,0)
#define fullbox(w,b,c,d,e,f,g)                                              \
	superbox1(w,0,0,(w)->_maxy,(w)->_maxx,b,c,d,e,f,g,0)
#define cbox(w)                                                             \
	superbox1(w,0,0,(w)->_maxy,(w)->_maxx,BX[3],BX[1],BX[0],BX[2],BX[5], \
	BX[4],Bxa)
#define drawbox(w,y,x,l,c)                                                  \
	superbox1(w,(y),(x),(l),(c),BX[3],BX[1],BX[0],BX[2],BX[5],BX[4],Bxa)

#define flash() bell(2)
#define beep()	bell(1)

#define werase(win)         ((win)->_curx = (win)->_cury = 0, wclrtobot(win))
#define wclear(win)         ((win)->_curx = (win)->_cury = 0, wclrtobot(win), (win)->_clear = TRUE)
#define clearok(win,bf)     ((win)->_clear = bf)
#define leaveok(win,bf)     ((win)->_leave = bf)
#define scrollok(win,bf)    ((win)->_scroll = bf)
#define flushok(win,bf)                                                     \
	(bf ? ((win)->_flags |= _FLUSH):((win)->_flags &= ~_FLUSH))
#define getyx(win,y,x)      y = (win)->_cury, x = (win)->_curx

/* psuedo function to draw Box Character */
#define cboxalt(w)                                                         \
        superbox1(w,0,0,(w)->_maxy,(w)->_maxx,BY[3],BY[1],BY[0],BY[2],BY[5],BY[4],Bya)
#define drawboxalt(w,y,x,l,c)                                              \
        superbox1(w,(y),(x),(l),(c),BY[3],BY[1],BY[0],BY[2],BY[5],BY[4],Bya)

#if (IS1|IS2|V7)
#define raw()                                                               \
	(_tty.sg_flags|=RAW, _pfast=_rawmode=TRUE, stty(_tty_ch,&_tty))
#define noraw()                                                             \
	(_tty.sg_flags&=~RAW,_rawmode=FALSE,_pfast=!(_tty.sg_flags&CRMOD),  \
	stty(_tty_ch,&_tty))
#define crmode()                                                            \
	(_tty.sg_flags |= CBREAK, _rawmode = TRUE, stty(_tty_ch,&_tty))
#define nocrmode()                                                          \
	(_tty.sg_flags &= ~CBREAK,_rawmode=FALSE,stty(_tty_ch,&_tty))
#define echo()                                                              \
	(_tty.sg_flags |= ECHO, _echoit = TRUE, stty(_tty_ch, &_tty))
#define noecho()                                                            \
	(_tty.sg_flags &= ~ECHO, _echoit = FALSE, stty(_tty_ch, &_tty))
#define nl()                                                                \
	(_tty.sg_flags |= CRMOD,_pfast = _rawmode,stty(_tty_ch, &_tty))
#define nonl()                                                              \
	(_tty.sg_flags &= ~CRMOD, _pfast = TRUE, stty(_tty_ch, &_tty))
#define savetty()                                                           \
	(gtty(_tty_ch, &_tty), _res_flg = _tty.sg_flags)
#define resetty()                                                           \
	(_tty.sg_flags = _res_flg, stty(_tty_ch, &_tty))
#else
#define Stty(chan, data)	ioctl(chan, TCSETAW, data)
#define Gtty(chan, data)	ioctl(chan, TCGETA, data)
extern  raw (), noraw (), crmode (), nocrmode ();
extern  nl (), nonl (), echo (), noecho ();
extern  savetty ();
extern  resetty ();
#endif

WINDOW  *newview(), *initscr(), *newwin(), *subwin();
char    *longname (), *getcap ();

extern  char    do_colors;      /* If true before calling initscr(), the
				   screen foreground and background will
				   be saved and restored during endwin() 
				*/
extern  char    do_cursor;      /* If false before calling initscr(), the
				   cursor shape will not be set to normal
				   (i.e. underscore shap).
				*/
#endif				/* _H_CUR01 */
