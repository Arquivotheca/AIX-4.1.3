static char sccsid[] = "@(#)01	1.2  src/bos/usr/bin/errlg/libras/streq_cn.c, cmderrlg, bos411, 9428A410j 3/29/94 19:55:09";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: streq_cn
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
 * NAME:     streq_cn
 * FUNCTION: return true if strings match.
 *           case insensitive.
 *           If s1 is shorter than s2 (and s1 matches s2), this is a match.
 * INPUTS:   character strings s1 and s2
 * RETURNS:  0 if no match. non-zero if match.
 */

#define _ILS_MACROS
#include <ctype.h>

streq_cn(s1,s2)
char *s1,*s2;
{
	char c1;
	char c2;

	while(c1 = *s1++) {
		c2 = *s2++;
		if(c1 == c2)
			continue;
		if(isupper(c1))
			c1 = _tolower(c1);
		if(isupper(c2))
			c2 = _tolower(c2);
		if(c1 == c2)
			continue;
		return(0);
	}
	return(1);
}

