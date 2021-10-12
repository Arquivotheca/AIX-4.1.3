static char sccsid[] = "@(#)31	1.5  src/bos/usr/ccs/lib/libc/getegid.c, libcs, bos411, 9428A410j 3/5/93 10:58:17";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: getegid 
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
#include <sys/id.h>

gid_t
getegid(void)
{
	return(getgidx(ID_EFFECTIVE));
}
