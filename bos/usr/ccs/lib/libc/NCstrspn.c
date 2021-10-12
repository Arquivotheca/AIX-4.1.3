static char sccsid[] = "@(#)88	1.2  src/bos/usr/ccs/lib/libc/NCstrspn.c, libcnls, bos411, 9428A410j 5/16/91 08:16:20";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrspn
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 *  Return the number of elements (bytes or wchar_ts) in the longest
 *  leading segment of string that consists solely of characters
 *  from charset.  
 */

int NCstrspn(wchar_t *string, char *charset)
{
	wchar_t *nlcharset;
	int	rc;
	/**********
	** get the space for the nlcharset
	** and convert charset to wchar_t
	**********/
	nlcharset = (wchar_t *) malloc(strlen(charset) * sizeof(wchar_t) + 1);
	if (nlcharset == (wchar_t *)NULL)
		return(0);

	(void) mbstowcs(nlcharset, charset, strlen(charset) +1);

	rc = (int)wcsspn(string, nlcharset);
	(void) free(nlcharset);
	return (rc);
}
