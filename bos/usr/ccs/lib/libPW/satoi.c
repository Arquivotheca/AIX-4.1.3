static char sccsid[] = "@(#)74	1.5  src/bos/usr/ccs/lib/libPW/satoi.c, libPW, bos411, 9428A410j 6/16/90 00:57:02";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: satoi
 *
 * ORIGINS: 3
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 * 
 */
 
# include	"sys/types.h"
# include	"macros.h"


char *satoi(p,ip)
register char *p;
register int *ip;
{
	register int sum;

	sum = 0;
	while (numeric(*p))
		sum = sum * 10 + (*p++ - '0');
	*ip = sum;
	return(p);
}
