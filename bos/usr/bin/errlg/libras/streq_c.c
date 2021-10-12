static char sccsid[] = "@(#)00	1.1  src/bos/usr/bin/errlg/libras/streq_c.c, cmderrlg, bos411, 9428A410j 3/2/93 09:03:59";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: streq_c
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
 * NAME:      streq_c
 * FUNCTION:  determine if two strings have the same value
 *            Case insensitive string equality routine
 * INPUTS:    s1,s2
 * RETURNS:   1 if equal (non-zero)
 *            0 if not equal
 */

#include <ctype.h>

streq_c(s1,s2)
char *s1,*s2;
{
	for(; toupper(*s1) == toupper(*s2); ++s1, ++s2) {
		if(*s1 == '\0')
			return(1);
	}
	return(0);
}

