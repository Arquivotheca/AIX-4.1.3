static char sccsid[] = "@(#)38	1.5  src/bos/usr/ccs/lib/libc/setrgid.c, libcs, bos411, 9428A410j 11/17/93 15:13:44";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: setrgid 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>

/* this function is in violation to the security policy of AIX */

int
setrgid(gid)
gid_t	gid;
{
	errno = EPERM;
	return(-1);
}
