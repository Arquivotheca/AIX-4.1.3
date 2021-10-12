static char sccsid[] = "@(#)54	1.13  src/bos/usr/ccs/lib/libc/NLcatgets.c, libcmsg, bos411, 9428A410j 5/13/91 16:58:33";
/*
 * COMPONENT_NAME: LIBCMSG
 *
 * FUNCTIONS: NLcatgets
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

#include <nl_types.h>

/*
 * NAME: NLcatgets
 *                                                                    
 * FUNCTION: Get the text of a message from a message catalog.
 *
 * NOTES: This function is obselete and is being maintained for backward
 *      compatibility with AIX v3.1.  Refer to the catgets() function
 *      for further information.
 */  

char *NLcatgets(nl_catd catd, int setno, int msgno, char *def)
{
	return (catgets(catd, setno, msgno, def));
}
