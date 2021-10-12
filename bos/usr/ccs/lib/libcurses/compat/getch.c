static char sccsid[] = "@(#)04  1.17  src/bos/usr/ccs/lib/libcurses/compat/getch.c, libcurses, bos411, 9428A410j 4/1/94 14:14:24";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wgetch, _catch_alarm, _fpk
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"
# include	<signal.h>
# include	<errno.h>


static int sig_caught;

static _fpk();

/*
 * NAME:        wgetch
 *
 * FUNCTION:
 *
 *      This routine reads in a character from the window.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      wgetch MUST return an int, not a char, because it can return
 *      things like ERR, meta characters, and function keys > 256.
 */

int
wgetch(win)
register WINDOW	*win;
{

	register int inp;
	register int i, j;
	char c;
	int arg;
	char	weset = FALSE;
	FILE *inf;

	if (SP->fl_echoit && !win->_scroll && (win->_flags&_FULLWIN)
	    && win->_curx == win->_maxx && win->_cury == win->_maxy)
		return ERR;
# ifdef DEBUG
	if(outf) fprintf(outf,
		"WGETCH: SP->fl_echoit = %c, SP->fl_rawmode = %c\n",
			SP->fl_echoit ? 'T' : 'F',SP->fl_rawmode ? 'T' : 'F');
	if (outf) fprintf(outf,
		"_use_keypad %d, kp_state %d\n",
			win->_use_keypad, SP->kp_state);
# endif
	if (SP->fl_echoit && !SP->fl_rawmode) {
		cbreak();
		weset++;
	}

#ifdef KEYPAD
	/* Make sure keypad on is in proper state */
	if (win->_use_keypad != SP->kp_state) {
		_kpmode(win->_use_keypad);
		fflush(stdout);
	}
#endif

	/* Make sure we are in proper nodelay state */
	if (win->_nodelay != SP->fl_nodelay)
		_fixdelay(SP->fl_nodelay, win->_nodelay);

	/* set up input file pointer */
	inf = SP->input_file;
	if (inf == stdout)	/* so output can be teed somewhere */
		inf = stdin;

	/* Check for pushed typeahead. */
	if (SP->input_queue[0] >= 0) {
		inp = SP->input_queue[0];
		for (i=0; i<16; i++) {
			SP->input_queue[i] = SP->input_queue[i+1];
			if (SP->input_queue[i] < 0)
				break;
		}
		goto checkit;
	}

#ifdef FIONREAD
	if (win->_nodelay) {
		ioctl(fileno(inf), FIONREAD, &arg);
#ifdef DEBUG
		if (outf) fprintf(outf, "FIONREAD returns %d\n", arg);
#endif
		if (arg < 1)
			return -1;
	}
#endif

/* 
 *	A012416, IX12416. Peter May.
 *	Changed error check routine in the for loop 
 *	to allow EINTR to cause wgetch to exit. 10/August/1990.
 */

	for (i = -1; i<0; ) {
		extern int errno;
		sig_caught = 0;
		i = read(fileno(inf), &c, 1);
/*		if (i < 0 && errno != EINTR && !sig_caught) { 	Old line */
		if( i < 0 ) {			/* How much will this break? */
			inp = ERR;
			goto gotit;
		}
	}
	if (i > 0) {
		inp = c;
#ifdef NLS
		inp &= 0377;
#else
		if (!win->_use_meta)
			inp &= 0177;
		else
			inp &= 0377;
#endif
	} else {
		inp = ERR;
		goto gotit;
	}
# ifdef DEBUG
	if(outf) fprintf(outf,"WGETCH got '%s'\n",unctrl(inp));
# endif

checkit:
#ifdef KEYPAD
	/* Check for arrow and function keys */
	if (win->_use_keypad) {
		for (j=0; (SP->input_queue[j] >= 0) && (j < 15);j++)
			;
		SP->input_queue[j + 1] = -1;
		for (--j ; j >= 0; j--)
			SP->input_queue[j+1] = SP->input_queue[j];

		SP->input_queue[0] = inp;
		for (i=0; SP->kp[i].keynum > 0; i++) {
			if (SP->kp[i].sends[0] == SP->input_queue[0]) {
				for (j=0; ; j++) {
					if ((int) SP->kp[i].sends[j] <= 0)
						break;	/* found */
					if (SP->input_queue[j] == -1) {
						SP->input_queue[j] =
							_fpk(inf);
						SP->input_queue[j+1] = -1;
					}
					if (SP->kp[i].sends[j] !=
						SP->input_queue[j])
							/* not this one */
						goto contouter;
				}
				/* It matched the function key. */
				inp = SP->kp[i].keynum;
				SP->input_queue[0] = -1;
				goto gotit;
			}
		contouter:;
		}
		/* Didn't match any function keys. */
		inp = SP->input_queue[0];
		for (i=0; i<16; i++) {
			SP->input_queue[i] = SP->input_queue[i+1];
			if (SP->input_queue[i] < 0)
				break;
		}
	}
#endif

	if (SP->fl_echoit) {
		waddch(win, (chtype) inp);
		wrefresh(win);
	}
gotit:
	if (weset)
		nocbreak();
#ifdef DEBUG
	if(outf) fprintf(outf, "getch returns %o, pushed %o %o %o\n",
		inp, SP->input_queue[0], SP->input_queue[1],
			SP->input_queue[2]);
#endif
	return inp;
}
#ifdef NLS
static void
#endif

/*
 * NAME:        _catch_alarm
 */

_catch_alarm()
{
	sig_caught = 1;
}


/*
 * NAME:        _fpk
 *
 * FUNCTION:
 *
 *      Fast peek key.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Like getchar but if the right flags are set, times out
 *      quickly if there is nothing waiting, returning -1.
 *      f is an output stdio descriptor, we read from the fileno.  win is
 *      the window this is supposed to have something to do with.
 *
 *      Traditional implementation.  The best resolution we have is 1
 *      second, so we set a 1 second alarm and try to read.  If we fail for
 *      1 second, we assume there is no key waiting.  Problem here is that 1
 *      second is too long, people can type faster than this.
 *
 *      If we have the select system call (FIONREAD), we can do much better.
 *      We wait for long enough for a terminal to send another character
 *      (at 15cps repeat rate, this is 67 ms, I'm using 100ms to allow
 *      a bit of a fudge factor) and time out more quickly.  Even if we
 *      don't have the real 4.2BSD select, we can emulate it with napms
 *      and FIONREAD.  napms might be done with only 1 second resolution,
 *      but this is no worse than what we have above.
 */

#ifndef FIONREAD
static
_fpk(f)
FILE *f;
{
	char c;
	int rc;
	int (*oldsig)();
	int oldalarm;

	oldsig = (FUNC) signal((int)SIGALRM,
				(void (*)(int))(int)_catch_alarm);
	oldalarm = alarm(1);
	sig_caught = 0;
	rc = read(fileno(f), &c, 1);
	if (sig_caught) {
		sig_caught = 0;
		alarm(oldalarm);
		return -2;
	}
	alarm(oldalarm);
	signal((int)SIGALRM, (void (*) (int))(int)oldsig);
	return rc == 1 ? c : -1;
}
#else FIONREAD
static
_fpk(f)
FILE *f;
{
static  long    timeOut = 0;            /* times to read                */
	char    *escDelay;              /* ENV var. ESCDELAY            */
#define max_reads 100000                /* maximum timeout              */

	int infd, rc;
	int *outfd, *exfd;
	char c;
	struct _timeval {
		long tv_sec;
		long tv_usec;
	} t;

	if (!timeOut)                   /* if first time here.          */
					/* if ESCDELAY exists.          */
	    if ((escDelay = (char *) getenv ("ESCDELAY")) != (char *) NULL){
					/* convert it to numeric.       */
		timeOut = (long) atoi (escDelay) * 200;
	    }
	    else
		timeOut = (long) max_reads ;

	infd = 1 << fileno(f);
	outfd = exfd = (int *) NULL;
	t.tv_sec = timeOut / 1000000;
	t.tv_usec = timeOut % 1000000;
	rc = select(fileno(f)+1, &infd, outfd, exfd, &t);
	if (rc <= 0)
		return -2;
	rc = read(fileno(f), &c, 1);
	return rc == 1 ? c : -1;
}
#endif FIONREAD
