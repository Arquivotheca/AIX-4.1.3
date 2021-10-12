static char sccsid[] = "@(#)20	1.14  src/bos/usr/bin/csh/err.c, cmdcsh, bos411, 9428A410j 11/12/92 13:32:19";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: error Perror_work Perror Perror_free bferr seterr seterr2 seterrc
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include "sh.h"
#include <sys/ioctl.h>


bool	errspl;			/* Argument to error was spliced by seterr2 */
uchar_t	one[2] = { '1', 0 };
uchar_t	*onev[2] = { one, NOSTR };
/*
 * Print error string s
 * This routine always resets or exits.  The flag haderr
 * is set so the routine who catches the unwind can propogate
 * it if they want.
 *
 * Note that any open files at the point of error will eventually
 * be closed in the routine process in sh.c which is the only
 * place error unwinds are ever caught.
 */
void
error(char *s)
{
	register uchar_t **v;
	register uchar_t *ep;

	/*
	 * Must flush before we print as we wish output before the error
	 * to go on (some form of) standard output, while output after
	 * goes on (some form of) diagnostic output.
	 * If didfds then output will go to 1/2 else to FSHOUT/FSHDIAG.
	 * See flush in print.c.
	 */
	flush();
	haderr = 1;		/* Now to diagnostic output */
	timflg = 0;		/* This isn't otherwise reset */
	if(v = pargv) {
		pargv = 0;
		blkfree(v);
	}
	if(v = gargv) {
		gargv = 0;
		blkfree(v);
	}

	/*
	 * A zero arguments causes no printing, else print
	 * an error diagnostic here.
	 */
	if (s){
		printf("%s.\n", (char *)s); 
	}

	didfds = 0;		/* Forget about 0,1,2 */
	if ((ep = err) && errspl) {
		errspl = 0;
		xfree(ep);
	}
	errspl = 0;

	/*
	 * Go away if -e or we are a child shell
	 */

	if (exiterr || child)
		exitcsh(1);

	/*
	 * Reset the state of the input.
	 * This buffered seek to end of file will also
	 * clear the while/foreach stack.
	 */
	btoeof();

	setq("status", (uchar_t **)onev, &shvhed);
	if (tpgrp > 0)  {
	    struct sigvec nsv, osv; 
		
	    nsv.sv_handler = SIG_IGN;
	    nsv.sv_mask = SA_RESTART;
	    nsv.sv_onstack = 0;
	    (void)sigvec(SIGTTOU, &nsv, &osv);
	    IOCTL(FSHTTY, TIOCSPGRP, &tpgrp,"1");
	    (void)sigvec(SIGTTOU, &osv, (struct sigvec *)0);
	}
	reset();		/* Unwind */
}

/*
 * Perror is the shells version of perror which should otherwise
 * never be called.
 */
static void
Perror_work(char *s, int free)
{
	/*
	 * Perror uses unit 2, thus if we didn't set up the fd's
	 * we must set up unit 2 now else the diagnostic will disappear
	 */
	if (!didfds) {
		register int oerrno = errno;

		dcopy(SHDIAG, 2);
		errno = oerrno;
	}
	perror(s);
	if (free)
		xfree((uchar_t *)s);
	error((char *)NOSTR);		/* To exit or unwind */
}

/*
 * Two version of Perror, one that will free its argument
 */
void Perror(char *s)
{
	Perror_work(s, FALSE);
}

void
Perror_free(char *s)
{
	Perror_work(s, TRUE);
}

void
bferr(char *cp)
{
	flush();
	haderr = 1;
	printf("%s: ", bname);
	error(cp);
}

/*
 * The parser and scanner set up errors for later by calling seterr,
 * which sets the variable err as a side effect; later to be tested,
 * e.g. in process.
 */
void
seterr(char *s)
{

	if (err == 0) {
		err = (uchar_t *)s;
		errspl = 0;
	}
}

/*
 * Set err to a splice of cp and dp, to be freed later in error()
 */
void
seterr2(char *cp, uchar_t *dp)
{

	if (err)
		return;
	err = strspl((uchar_t *)cp, dp);
	errspl++;
}

/*
 * Set err to a splice of cp with a string form of character d
 */
void
seterrc(char *cp, uchar_t d)
{
	uchar_t chbuf[2];

	chbuf[0] = d;
	chbuf[1] = 0;
	seterr2(cp, chbuf);
}
