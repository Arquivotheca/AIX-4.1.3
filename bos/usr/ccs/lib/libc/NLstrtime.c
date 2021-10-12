static char sccsid[] = "@(#)73	1.1  src/bos/usr/ccs/lib/libc/NLstrtime.c, libcnls, bos411, 9428A410j 2/26/91 17:53:43";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrtime
 *
 * ORIGINS:27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 ,1991, 1991
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

#include <sys/localedef.h>
#include <time.h>

char *NLstrtime(char *s, size_t maxsize, const char *format,const struct tm *timeptr)
{
	(void) strftime(s, maxsize, format, timeptr);
	return(s);
}
