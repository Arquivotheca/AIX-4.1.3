static char sccsid[] = "@(#)87	1.6  src/bos/usr/ccs/lib/libPW/curdir.c, libPW, bos411, 9428A410j 6/16/90 00:55:53";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: curdir
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

/*
 * FUNCTION: places the full pathname of the current directory
 *           in the caller's supplied string
 *
 * RETURN VALUE DESCRIPTIONS:
 *		0 if successful
 *		-1 on ERROR
 */
#include	<sys/types.h>	/* for NULL */

curdir(str)
char *str;
{
	if (getwd(str) == NULL) {
		*str = '\0';
		return(-1);
	}
	return(0);
}
