static char sccsid[] = "@(#)15  1.2  src/bos/usr/bin/errlg/libras/num_chk.c, cmderrlg, bos411, 9428A410j 3/29/94 19:54:30";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: num_chk, is_delim
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:        num_chk()
 * FUNCTION:    Given a string this function checks to see if it
 *              contains valid numbers for the provided base.
 *		The string may contain delimiters to construct a
 *		list of numbers.
 * RETURNS:     0       : valid number
 *              1       : not valid number
 *		-1	: invalid base supplied
 */


#define _ILS_MACROS
#include <ctype.h>

int
num_chk(char *str, int base)
{
        int c;
	int rc=1;	/* default to good value */
        char *cp;

        cp = str;
	if (base == 16)
	{
		while(c = *cp++)
		{
			if(is_delim(c))		/* skip char */
				continue;
			else if(!isxdigit(c))
			{
				rc = 0;
				break;
			}
		}
	}
	else if (base == 10)
	{
		while(c = *cp++)
		{
			if(is_delim(c))		/* skip char */
				continue;
			else if(!isdigit(c))
			{
				rc = 0;
				break;
			}
		}
	}
	else if (base == 8)
	{
		while(c = *cp++)
		{
			if(is_delim(c))		/* skip char */
				continue;
			else if(!('0' <= c && c <= '7'))
			{
				rc = 0;
				break;
			}
		}
	}
	else	/* invalid base specified */
		rc = -1;

	return(rc);
}


/*
 * NAME:        is_delim()
 * FUNCTION:    Check if a given character is a valid delimiter.
 *		The valid delimeters are comma ',', and space ' '.
 * RETURNS:     1       : valid delimiter
 *              0       : not valid delimiter
 */

static 	int
is_delim(char c)
{
	int rc;

	if(c == ' ' || c == ',')
		rc = 1;
	else
		rc = 0;

	return(rc);
}
