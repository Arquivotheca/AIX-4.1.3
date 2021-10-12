static char sccsid[] = "@(#)91	1.4  src/bos/usr/bin/mh/uip/trmsbr.c, cmdmh, bos411, 9428A410j 6/15/90 22:21:09";
/* 
 * COMPONENT_NAME: CMDMH trmsbr.c
 * 
 * FUNCTIONS: SOprintf, clear_screen, outc, read_termcap, sc_length, 
 *            sc_width 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* trmsbr.c - minor termcap support (load with -ltermlib) */

#include "mh.h"
#include <stdio.h>
#ifndef	SYS5
#include <sgtty.h>
#else	SYS5
#include <sys/types.h>
#include <termio.h>
#include <sys/ioctl.h>
#endif	SYS5


#if	BUFSIZ<2048
#define	TXTSIZ	2048
#else
#define	TXTSIZ	BUFSIZ
#endif

#ifndef	SYS5
extern char PC;
extern short    ospeed;
#else   SYS5
char	PC;
short	ospeed;
#endif	SYS5

int     tgetent (), tgetnum ();
char   *tgetstr ();

/*  */

static int  initLI = 0;
static int  LI = 40;
static int  initCO = 0;
static int  CO = 80;
static char *CL = NULL;
static char *SE = NULL;
static char *SO = NULL;

char *getenv();

static char termcap[TXTSIZ];

/*  */

static  read_termcap () {
    register char  *bp,
                   *term;
    char   *cp,
	    myterm[TXTSIZ];
#ifndef	SYS5
    struct sgttyb   sg;
#else	SYS5
    struct termio   sg;
#endif	SYS5
    static int  inited = 0;

    if (inited++)
	return;

    if ((term = getenv ("TERM")) == NULL || tgetent (myterm, term) <= OK)
	return;

#ifndef	SYS5
    ospeed = ioctl (fileno (stdout), TIOCGETP, (char *) &sg) != NOTOK
		? sg.sg_ospeed : 0;
#else	SYS5
    ospeed = ioctl (fileno (stdout), TCGETA, &sg) != NOTOK
		? sg.c_cflag & CBAUD : 0;
#endif	SYS5

    if (!initCO && (CO = tgetnum ("co")) <= 0)
	CO = 80;
    if (!initLI && (LI = tgetnum ("li")) <= 0)
	LI = 24;

    cp = termcap;
    CL = tgetstr ("cl", &cp);
    if (bp = tgetstr ("pc", &cp))
	PC = *bp;
    if (tgetnum ("sg") <= 0) {
	SE = tgetstr ("se", &cp);
	SO = tgetstr ("so", &cp);
    }
}

/*  */

int     sc_width () {
#ifdef	TIOCGWINSZ
    struct winsize win;

    if (ioctl (fileno (stderr), TIOCGWINSZ, &win) != NOTOK
	    && (CO = win.ws_col) > 0)
	initCO++;
    else
#endif	TIOCGWINSZ
	read_termcap ();

    return CO;
}


int     sc_length () {
#ifdef	TIOCGWINSZ
    struct winsize win;

    if (ioctl (fileno (stderr), TIOCGWINSZ, &win) != NOTOK
	    && (LI = win.ws_row) > 0)
	initLI++;
    else
#endif	TIOCGWINSZ
	read_termcap ();

    return LI;
}

/*  */

static int  outc (c)
register char    c;
{
    (void) putchar (c);
}


void clear_screen () {
    read_termcap ();

    if (CL && ospeed)
	tputs (CL, LI, outc);
    else {
	printf ("\f");
	if (ospeed)
	    printf ("\200");
    }

    (void) fflush (stdout);
}

/*  */

/* VARARGS1 */

int     SOprintf (fmt, a, b, c, d, e, f)
char   *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    read_termcap ();
    if (SO == NULL || SE == NULL)
	return NOTOK;

    tputs (SO, 1, outc);
    printf (fmt, a, b, c, d, e, f);
    tputs (SE, 1, outc);

    return OK;
}
