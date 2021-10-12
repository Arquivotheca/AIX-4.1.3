static char sccsid[] = "@(#)28	1.12  src/bos/usr/ccs/lib/libc/rand.c, libcgen, bos411, 9428A410j 10/20/93 14:31:08";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: rand, srand
 *
 * ORIGINS: 3,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993 
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

#include <stdlib.h>
#include <errno.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _rand_rmutex;
#endif

/*
 * NAME:	srand
 *
 * FUNCTION:	The srand function uses the arguement as a seed for a new
 *		sequence of pseudo-random numbers to be returned by 
 *		subsequent calls to rand.
 *                                                                    
 * RETURN VALUE DESCRIPTION:	
 *		returns no value
 *
 */                                                                   

static long randx=1;

void	
srand(unsigned int seed)
{
	TS_LOCK(&_rand_rmutex);

	randx = seed;

	TS_UNLOCK(&_rand_rmutex);
}

/*
 * NAME:	rand
 *
 * FUNCTION:	The rand function computes a sequence of pseudo-random
 *		integers in the range 0 to RAND_MAX.
 *                                                                    
 * RETURN VALUE DESCRIPTION:	
 *		returns a psuedo-random integer
 *
 */                                                                   

int 
rand(void)
{
	int	retval;

	TS_LOCK(&_rand_rmutex);
	retval = ((randx = randx * 1103515245L + 12345)>>16) & RAND_MAX;

	TS_UNLOCK(&_rand_rmutex);
	return(retval);
}

#ifdef _THREAD_SAFE
int
rand_r(unsigned int *seed)
{
	if (seed == NULL) {
		errno = EINVAL;
		return(-1);
	}
	return(((*seed = *seed * 1103515245L + 12345)>>16) & RAND_MAX);
}
#endif
