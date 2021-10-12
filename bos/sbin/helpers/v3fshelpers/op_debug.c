static char sccsid [] = "@(#)90	1.5.1.2  src/bos/sbin/helpers/v3fshelpers/op_debug.c, cmdfs, bos411, 9428A410j 2/22/93 23:20:29";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: op_debug
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fshelp.h>

/*
** op_debug
**
** debug filesystems.  
**
*/
int
op_debug (devfd, opflags)	/* main */
int devfd;
char *opflags;
{
	return FSHERR_NOTSUP;
}



