static char sccsid[] = "@(#)28  1.5  src/bos/usr/ccs/lib/libcurses/compat/select.c, libcurses, bos411, 9428A410j 6/16/90 01:52:26";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   select
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

#include "cursesext.h"
#include <signal.h>

#define NAPINTERVAL 100

struct _timeval {
	long tv_sec;
	long tv_usec;
};

#ifdef NEEDSELECT

/*
 * NAME:        select
 *
 * FUNCTION:
 *
 *      Emulation of 4.2BSD select system call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Code for various kinds of delays.  Most of this is nonportable and
 *      requires various enhancements to the operating system, so it won't
 *      work on all systems.  It is included in curses to provide a portable
 *      interface, and so curses itself can use it for function keys.
 *
 *      This is somewhat crude but
 *      better than nothing.  We do FIONREAD on each fd, and if we have to
 *      wait we use nap to avoid a busy wait.  The resolution of the nap
 *      will hurt response - so will the fact that we ignore the write fds.
 *      If we are simulating nap with a 1 second sleep, this will be very
 *      poor.
 *
 *      nfds is the number of fds to check - this is usually 20.
 *      prfds is a pointer to a bit vector of file descriptors - in the case
 *      where nfds < 32, prfds points to an integer, where bit 1<<fd
 *      is 1 if we are supposed to check file descriptor fd.
 *      pwfds is like prfds but for write checks instead of read checks.
 *      ms is the max number of milliseconds to wait before returning
 *      failure.
 *
 *      The value returned is the number of file descriptors ready for input.
 *      The bit vectors are updated in place.
 */

int
select(nfds, prfds, pwfds, pefds, timeout)
register int nfds;
int *prfds, *pwfds, *pefds;
struct _timeval *timeout;
{
	register int fd;
	register int rfds = *prfds;
	register int n;
	int nwaiting, rv = 0;
	long ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

	for (;;) {
		/* check the fds */
		for (fd=0; fd<nfds; fd++)
			if (1<<fd & rfds) {
				ioctl(fd, FIONREAD, &nwaiting);
				if (nwaiting > 0) {
					rv++;
				} else
					*prfds &= ~(1<<fd);
			}
		if (rv)
			return rv;

		/* Nothing ready.  Should we give up? */
		if (ms <= 0)
			return 0;

		*prfds = rfds;	/* we clobbered it, so restore. */

		/* Wait a bit */
		n = NAPINTERVAL;
		if (ms < NAPINTERVAL)
			n = ms;
		ms -= n;
		napms(n);
	}
}
#endif NEEDSELECT
