static char sccsid[] = "@(#)89	1.2  src/bos/usr/ccs/lib/libc/NCstrtok.c, libcnls, bos411, 9428A410j 5/16/91 08:16:28";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrtok
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
 *
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 *  Uses strpbrk and strspn to break string into tokens on sequentially
 *  subsequent calls.  Returns NULL when no non-separator characters
 *  remain.  `Subsequent' calls are calls with first argument NULL.
 */

wchar_t *NCstrtok(wchar_t *string, char *sepset)
{
	wchar_t *nlsepset;
	wchar_t *rc;

	/**********
	** convert sepset to wchar_t
	***********/	
	nlsepset = (wchar_t *) malloc ((strlen(sepset)+1) * sizeof(wchar_t));
	if (nlsepset == (wchar_t *)NULL)
		return(0);
	(void)mbstowcs(nlsepset, sepset, strlen(sepset) +1 );

	rc = wcstok(string, nlsepset);
	(void) free(nlsepset);
	return(rc);
}
