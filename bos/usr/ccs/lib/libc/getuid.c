static char sccsid[] = "@(#)34	1.5  src/bos/usr/ccs/lib/libc/getuid.c, libcs, bos411, 9428A410j 1/12/93 11:15:46";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: getuid 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/id.h>

uid_t
getuid(void)
{
	return(getuidx(ID_REAL));
}
