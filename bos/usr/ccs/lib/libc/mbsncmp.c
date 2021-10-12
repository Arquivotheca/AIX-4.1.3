static char sccsid[] = "@(#)41	1.8  src/bos/usr/ccs/lib/libc/mbsncmp.c, libcnls, bos411, 9428A410j 7/23/91 09:30:19";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: mbsncmp
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
#include <ctype.h>
#include <stdlib.h>

/*
 * NAME: mbsncmp
 *
 * FUNCTION: Compare a specific number of multibyte characters (code points) 
 *  in one multibyte character string to another multibyte character string.
 *
 * PARAMETERS:
 *	char *s1	-	the multibyte character string
 *	char *s2	-	the multibyte character string
 *	size_t    n	-	the number of multibyte characters
 *
 * RETURN VALUE DESCRIPTION: 
 *   s1>s2; >0  s1==s2; 0  s1<s2; <0.
 *
 *
 */

int
mbsncmp(const char *s1, const char *s2, size_t n) 
{
	size_t i;
	wchar_t pc1, pc2;


	if (s1 == s2)
		return(0);
	mbtowc(&pc1,s1,MB_CUR_MAX);
	mbtowc(&pc2,s2,MB_CUR_MAX);
	for (i = 0; i < n && pc1 == pc2; i++) {
		if(*s1 == '\0')
			return(0);
		s1 += mblen(s1,MB_CUR_MAX);
		s2 += mblen(s2,MB_CUR_MAX);
		mbtowc(&pc1,s1,MB_CUR_MAX);
		mbtowc(&pc2,s2,MB_CUR_MAX);
	}
	return( (i == n) ? 0: (pc1 > pc2) ? 1 : -1);
}
