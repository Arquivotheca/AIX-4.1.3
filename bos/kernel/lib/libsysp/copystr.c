static char sccsid[] = "@(#)41	1.3  src/bos/kernel/lib/libsysp/copystr.c, libsysp, bos411, 9428A410j 6/16/90 02:39:56";
/*
 * COMPONENT_NAME: (LIBSYSP) Private Kernel Library
 *
 * FUNCTIONS: copystr
 *
 * ORIGINS: 3, 9, 26, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/errno.h>

/*
 * NAME: copystr
 *                                                                    
 * FUNCTION:
 * 	copies a string including the terminating null character.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This is bound with the Kernel, and any IBM device drivers
 *	that use it.
 *
 * RETURNS:
 * 	actual is a pointer to a uint which is set to the number of characters
 * 	copied, INCLUDING the null.
 * 	similarly, max is the maximum number of characters to copy, including
 * 	the NULL.
 *	E2BIG is returned if max is hit, otherwise 0 is returned.
 */  

int
copystr(caddr_t from,caddr_t to,uint max,uint *actual)
{
	caddr_t	p;
	for(p=from;max && (*to++=*p++);max--);
	*actual = p-from;
	return max ? 0 : E2BIG;
}

