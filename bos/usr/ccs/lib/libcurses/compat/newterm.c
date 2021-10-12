static char sccsid[] = "@(#)27  1.6  src/bos/usr/ccs/lib/libcurses/compat/newterm.c, libcurses, bos411, 9428A410j 6/16/90 01:50:27";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   newterm
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

char	*calloc();
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:        newterm
 */

struct screen *
newterm(type, outfd, infd)
char *type;
FILE *outfd, *infd;
{
	int		_tstp();
	struct screen *scp;
	struct screen *_new_tty();
	extern int _endwin;

#ifdef DEBUG
	if(outf) fprintf(outf, "NEWTERM() isatty(2) %d, getenv %s\n",
		isatty(2), getenv("TERM"));
# endif
	SP = (struct screen *) calloc(1, sizeof (struct screen));
	SP->term_file = outfd;
	SP->input_file = infd;
	/*
	 * The default is echo, for upward compatibility, but we do
	 * all echoing in curses to avoid problems with the tty driver
	 * echoing things during critical sections.
	 */
	SP->fl_echoit = 1;
	savetty();
	scp = _new_tty(type, outfd);
	if (scp == NULL)
		return NULL;
#ifdef USG
	(cur_term->Nttyb).c_lflag &= ~ECHO;
#else
	(cur_term->Nttyb).sg_flags &= ~ECHO;
#endif
	reset_prog_mode();
# ifdef SIGTSTP
	signal((int)SIGTSTP, ((void (*)(int))(int) _tstp));
# endif
	if (curscr != NULL) {
# ifdef DEBUG
		if(outf) fprintf(outf,
		"INITSCR: non null curscr = 0%o\n", curscr);
# endif
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	LINES =	lines;
	COLS =	columns;
	curscr = makenew(LINES, COLS, 0, 0);
	stdscr = newwin(LINES, COLS, 0, 0);
# ifdef DEBUG
	if(outf) fprintf(outf,
		"SP %x, stdscr %x, curscr %x\n", SP, stdscr, curscr);
# endif
	SP->std_scr = stdscr;
	SP->cur_scr = curscr;
	/* Maybe should use makewin and glue _y's to DesiredScreen. */
	_endwin = FALSE;
	return scp;
}
