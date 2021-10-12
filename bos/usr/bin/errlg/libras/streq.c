static char sccsid[] = "@(#)99	1.1  src/bos/usr/bin/errlg/libras/streq.c, cmderrlg, bos411, 9428A410j 3/2/93 09:03:42";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: streq
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:      streq
 * FUNCTION:  determine if two strings have the same value
 * INPUTS:    s1,s2
 * RETURNS:   1 if equal (non-zero)
 *            0 if not equal
 *
 * Note: this routine is written for expressions like:
 *    if(streq(optarg,"stdin"))
 *           ...
 * which is closer to the way it is thought of than:
 *    if(strcmp(optarg,"stdin") == 0)
 *           ...
 */

streq(s1,s2)
register char *s1,*s2;
{
	register char c;

	while(c = *s1++)
		if(c != *s2++)
			return(0);
	return(*s2 == '\0' ? 1 : 0);
}

