static char sccsid[] = "@(#)10  1.7  src/bos/usr/ccs/lib/libc/svc_run.c, libcrpc, bos411, 9428A410j 10/25/93 20:42:44";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: svc_run
 *		
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)svc_run.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.2 88/02/08 
 */


/*
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <libc_msg.h>

void
svc_run()
{
	fd_set readfds;
	int dtbsize = _rpc_dtablesize();
	extern int errno;

	for (;;) {
		readfds = svc_fdset;
		switch (select(dtbsize, &readfds, (fd_set *)0,
			(fd_set *)0, (struct timeval *)0)) {
		case -1:
			/*
			 * We ignore all other errors except EBADF.  For all
			 * other errors, we just continue with the assumption
			 * that it was set by the signal handlers (or any
			 * other outside event) and not caused by select().
			 */
			if (errno != EBADF) {
				continue;
			}
			(void) syslog(LOG_ERR, 
				      (char *)oncmsg(LIBCRPC,RPC88,
						     "svc_run: - select failed: %m"));
			return;
		case 0:
			continue;
		default:
			svc_getreqset(&readfds);
		}
	}
}
