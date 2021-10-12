static char sccsid[] = "@(#)80	1.2  src/bos/usr/ccs/lib/libc/NCstrcspn.c, libcnls, bos411, 9428A410j 5/16/91 08:16:11";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrcspn
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
 * NAME: NCstrcspn
 *
 * FUNCTION: Return the number of array elements (bytes or wchar_ts) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on wchar_ts).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  wchar_ts.
 *
 * RETURN VALUE DESCRIPTION: see FUNCTION
 */
int NCstrcspn(wchar_t *string, char *charset)
{
	
	wchar_t *nlcharset;
	int	rc;
	
	/**********
	** get the space to convert the char charset to wchar_t
	**********/
	nlcharset = (wchar_t *) malloc((strlen(charset)+1) * sizeof(wchar_t));
	if (nlcharset == (wchar_t *)NULL)
		return(0);

	(void)mbstowcs(nlcharset, charset, strlen(charset) + 1 );
	rc = wcscspn(string, nlcharset);
	(void) free(nlcharset);
	return (rc);
}
