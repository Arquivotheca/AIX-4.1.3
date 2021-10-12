static char sccsid[] = "@(#)35	1.4  src/bos/usr/ccs/lib/libc/setegid.c, libcs, bos411, 9428A410j 6/16/90 01:03:47";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: setegid 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/id.h>

int
setegid(gid)
gid_t	gid;
{
	return(setgidx(ID_EFFECTIVE, gid));
}
