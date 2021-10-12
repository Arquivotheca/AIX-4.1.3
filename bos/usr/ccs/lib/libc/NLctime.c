static char sccsid[] = "@(#)81	1.10  src/bos/usr/ccs/lib/libc/NLctime.c, libctime, bos411, 9428A410j 6/16/90 01:33:51";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: NLctime, NLasctime 
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

/*LINTLIBRARY*/
/*
 *  NLS-parameterized versions of ctime and asctime:
 *	NLctime(clk) just does the call NLasctime(localtime(clk)).
 *	NLasctime(tm) just makes the appropriate call to NLstrtime
 *	to get the current NLS equivalent of the traditional Unix
 *	date/time string.
 */
#include <time.h>

/*  Length of buffer to hold return value of NLasctime()
 */
#define BLEN	64

unsigned char *
NLctime(long *t)
{
	return(NLasctime(localtime(t)));
}

unsigned char *
NLasctime(struct tm *t)
{
	static unsigned char strbuf[BLEN];

	strftime((char *)strbuf, (size_t) BLEN, "%a %sD %X %Y\n\0", t);
	return(strbuf);
}
