static char sccsid[] = "@(#)30	1.4  src/bos/usr/ccs/lib/libc/a64l.c, libccnv, bos411, 9428A410j 6/16/90 01:04:18";
/*
 * COMPONENT_NAME: LIBCCNV a64l
 *
 * FUNCTIONS: a64l
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

#define BITSPERCHAR	6 /* to hold entire character set */

/*
 * NAME: a64l
 *                                                                    
 * FUNCTION: convert base 64 ascii to long int
 *                                                                    
 * NOTES:
 * char set is [./0-9A-Za-z]
 *
 * RETURNS: a long value
 *
 */

long
a64l(s)
register char *s;
{
	register int i, c;
	long lg = 0;

	for (i = 0; (c = *s++) != '\0'; i += BITSPERCHAR) {
		if (c > 'Z')
			c -= 'a' - 'Z' - 1;
		if (c > '9')
			c -= 'A' - '9' - 1;
		lg |= (long)(c - ('0' - 2)) << i;
	}
	return (lg);
}
