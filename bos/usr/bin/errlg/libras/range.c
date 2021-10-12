static char sccsid[] = "@(#)03	1.2  src/bos/usr/bin/errlg/libras/range.c, cmderrlg, bos411, 9428A410j 3/29/94 19:54:41";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: limit_range, in_range, val_name
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


#define _ILS_MACROS
#include <ctype.h>

int		limit_range(int min,int val,int max);
int		in_range(int min,int val,int max);
int		val_name(char ary[], int max);

/*
 * NAME:      limit_range()
 * FUNCTION:  Limit a value to the range provided.
 * RETURNS:   the limited value.
 */

int
limit_range(int min,int val,int max)
{
	int	limited;

	if(val > 1) 	/* 1 for null */
		limited = val > max ?  max : val;
	else
		limited = 0;

	return(limited);

}

/*
 * NAME:      in_range()
 * FUNCTION:  Check that the provided value is within the range provided.
 * RETURNS:   1 - within range
 *            0 - not in range
 */

int
in_range(int min,int val,int max)
{

	if (val <= max && val >= min)
		return(1);
	else
		return(0);
}

/*
 * NAME:      val_name()
 * FUNCTION:  Check that a given name contains only
 *            printable ascii characters, and is not empty.
 * RETURNS:   1 - only good chars found.
 *            0 - bad chars found, or empty.
 */

int
val_name(char ary[], int max)
{
	int i;
	int	rc = 1;


	for (i=0; i< max; i++) {
		if(!isprint(ary[i])) {		/* not printable... */
			if(ary[i] == '\0') {
				if(i > 0) {		/* good char already encountered */
					break;
				}
				else {		/* 'empty' name */
					rc = 0;
					break;
				}
			}
			else {
				rc = 0;
				break;
			}
		}
	}
	return(rc);
}
