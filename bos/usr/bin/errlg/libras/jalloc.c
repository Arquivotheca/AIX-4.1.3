static char sccsid[] = "@(#)89	1.1  src/bos/usr/bin/errlg/libras/jalloc.c, cmderrlg, bos411, 9428A410j 3/2/93 09:00:08";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: jalloc, jfree
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
 * Memory allocation. (interface to malloc)
 *
 * NAME:     jalloc
 * FUNCTION: allocate 'n' bytes by calling calloc.
 *           If allocation fails (which should be never), the routine
 *           exits.
 *           This relieves callers of having to check for an error which
 *           should never happen, and if it does, nothing can be done about it.
 * INPUTS:   'n'   Number of bytes.
 * RETURNS:  character pointer (int aligned) to allocated buffer.
 * 
 * NOTE:     calloc is malloc with a zeroed-out buffer
 *
 *
 * NAME:     jfree
 * FUNCTION: free jalloc-ed buffer
 * INPUTSL   'ptr'   buffer returned by malloc/jalloc
 * RETURNS:  none
 */

#include <libras.h>

extern char *calloc();

char *jalloc(n)
{
	char *cp;

	if(n == 0)
		n = 1;
	cp = calloc(n,1);
	if(cp == 0) 
		perror("malloc");
	return(cp);
}

jfree(ptr)
char *ptr;
{

	free(ptr);
}


