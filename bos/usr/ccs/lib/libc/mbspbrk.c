static char sccsid[] = "@(#)44	1.6  src/bos/usr/ccs/lib/libc/mbspbrk.c, libcnls, bos411, 9428A410j 8/2/91 14:05:20";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbspbrk
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <NLchar.h>

/*
 * NAME: mbspbrk
 *
 * FUNCTION: Locate the first occurrence of multibyte characters (code 
 *           points) in a string
 *
 * PARAMETERS: 
 *	     char *string - compared string
 *	     char *brkset - comparing multibyte character set
 *
 * RETURN VALUE DESCRIPTION: 
 *  Return ptr to first occurrence of any character from `brkset'
 *  in the character string `string'; NULL if none exists.
 */

char *
mbspbrk(char *string, char *brkset)
{
	register wchar_t *p;
	wchar_t sc;
	wchar_t *nlbrkset;
	register int i;

	/**********
	  if in a single byte codeset, mbspbrk == strpbrk
	*********/
	if (MB_CUR_MAX == 1)
	    return (strpbrk(string, brkset));

	/**********
	  get the space for the process code version of the brkset
	**********/
	if ((nlbrkset=(wchar_t *)malloc(sizeof(wchar_t)*(strlen(brkset)+1)))
	    == (wchar_t *)NULL)
	    return((char *)NULL);

	/*********
	  convert the brkset to process code
	**********/
	if((mbstowcs(nlbrkset, brkset, strlen(brkset)+1)) == -1) {
	    free (nlbrkset);
	    return((char *)NULL);
	}
		    
	for (; ; ) {
		if (!*string)
			break;
		/**********
		  if at any time an invalid character is found,
		  stop processing
		**********/
		if ((i=mbtowc(&sc, string, MB_CUR_MAX)) == -1) {
		    free(nlbrkset);
		    return((char *)NULL);
		}
		for(p = nlbrkset; *p != 0 && *p != sc; ++p)
			;
		if(*p != 0) {
			free(nlbrkset);
			return(string);
		}
		string += i;
	}
	free(nlbrkset);
	return(NULL);
}
