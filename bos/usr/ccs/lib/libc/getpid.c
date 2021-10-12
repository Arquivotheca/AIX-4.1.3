static char sccsid[] = "@(#)11	1.1  src/bos/usr/ccs/lib/libc/getpid.c, libcproc, bos411, 9428A410j 12/7/93 20:35:09";
/*
 *   COMPONENT_NAME: LIBCPROC
 *
 *   FUNCTIONS: getpgid
 *		getsid
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>

extern pid_t kgetpgrp(pid_t);
extern pid_t kgetsid(pid_t);

/*
 * NAME:  getpgid()
 *
 * FUNCTION: Get process group.
 *
 * RETURNS: If unsuccessful, returns -1 and errno = ESRCH.
 *          If successfull, returns process group id.
 */
pid_t
getpgid(pid_t pid)
{
	pid_t rc;
	
	if (rc = kgetpgrp(pid))
		errno = ESRCH;

	return(rc);
}

/*
 * NAME:  getsid()
 *
 * FUNCTION: Get process session id.
 *
 * RETURNS: If unsuccessful, returns -1 and errno = ESRCH.
 *          If successfull, returns session id.
 */
pid_t
getsid(pid_t pid)
{
	pid_t rc;

	if (rc = kgetsid(pid))
		errno = ESRCH;

	return(rc);
}
