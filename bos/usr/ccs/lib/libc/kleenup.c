static char sccsid[] = "@(#)10	1.7  src/bos/usr/ccs/lib/libc/kleenup.c, libcs, bos411, 9428A410j 11/17/93 15:13:39";

/*
 * COMPONENT_NAME:  LIBCS
 *
 * FUNCTIONS:  kleenup()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <signal.h>
#include <ulimit.h>
#include <stdio.h>
#include <sys/errno.h>

#define MIN_KULIMIT	8192

void setsiglist(int  *ign, int  *keep);

int
kleenup(int  fd,
	int  *sig_ign,
	int  *sig_keep)
{
	int	curlim;

	fcntl(fd, F_CLOSEM, 0);

	curlim = ulimit(GET_FSIZE, 0);
	if (curlim < MIN_KULIMIT)
		ulimit(SET_FSIZE, MIN_KULIMIT);
	ulimit(SET_REALDIR, 0);

	alarm(0);

	setsiglist(sig_ign, sig_keep);
	
	errno = 0;
	return(0);
}


void
setsiglist(int  *ign,
	   int  *keep)
{
	int     *sp;
	int     i;
	int     sig_tab[NSIG];
	struct sigaction nact;

	/*
	 * Initialize the mask and flag fields for sigaction.
	 */
	sigemptyset(&(nact.sa_mask));
	nact.sa_flags=0;

	/*
	 * initialize sig_tab array
	 */
	for (i=0; i<NSIG; i++)
		sig_tab[i] = 1;

	/*
	 * now go through the ignore list and set those to SIG_IGN
	 * and put a tick mark in the sig_tab array
	 */
	if (ign)	/* special case: if pointer is NULL don't process */
	{
		nact.sa_handler=SIG_IGN;
		for (sp=ign; *sp; sp++)
		{
			if ((*sp < 0) || (*sp >= NSIG))
				continue;
			sigaction(*sp, &nact, NULL);
			sig_tab[*sp] = 0;
		}
	}

	/*
	 * now go through the keep list and set those to SIGKEEP 
	 */
	if (keep)       /* special case: if pointer is NULL don't process */
		for (sp=keep; *sp; sp++)
		{
			if ((*sp < 0) || (*sp >= NSIG))
				continue;
			sig_tab[*sp] = 0;
		}


	/* we can't do anything with SIGKILL */
	sig_tab[SIGKILL] = 0;

	/*
	 * now go through our list of signals and set those 
	 * that aren't "ticked"
	 */
	nact.sa_handler=SIG_DFL;
	for (i=0; i<NSIG; i++)
		if (sig_tab[i])
			sigaction(i, &nact, NULL); 
}
