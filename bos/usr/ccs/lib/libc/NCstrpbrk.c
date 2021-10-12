static char sccsid[] = "@(#)86	1.2  src/bos/usr/ccs/lib/libc/NCstrpbrk.c, libcnls, bos411, 9428A410j 5/16/91 08:16:16";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrpbrk
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
 *  Return ptr to first occurrence of any character from `brkset'
 *  in the character string `string'; NULL if none exists.  
 */

wchar_t * NCstrpbrk(wchar_t *string, char *brkset)
{
	wchar_t *nlbrkset;
	wchar_t *rc;

	/**********
	** get the space for the nlbrkset
	** and convert it to wchar_t
	**********/
	nlbrkset = (wchar_t *) malloc ((strlen(brkset)+1) * sizeof(wchar_t));
	if (nlbrkset = (wchar_t *) NULL)
		return(0);
	(void)mbstowcs(nlbrkset, brkset, strlen(brkset) + 1);

	rc = wcspbrk(string, nlbrkset);
	(void) free(nlbrkset);
	return (rc);

}
