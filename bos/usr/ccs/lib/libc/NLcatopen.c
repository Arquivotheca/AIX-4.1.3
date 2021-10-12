static char sccsid[] = "@(#)65	1.7  src/bos/usr/ccs/lib/libc/NLcatopen.c, libcmsg, bos411, 9428A410j 5/13/91 16:58:40";
/*
 * COMPONENT_NAME: (LIBCMSG) LIBC Message Catalog Functions
 *
 * FUNCTIONS: NLcatopen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>

/*
 * NAME: NLcatopen
 *
 * FUNCTION: Open a message catalog.
 *
 * NOTES: This function is obselete and is being maintained for backward
 *      compatibility with AIX v3.1.  Refer to the catopen() function
 *      for further information.
 */


nl_catd NLcatopen (char *catname, int oflag)
{
	return (catopen(catname, oflag));
}
