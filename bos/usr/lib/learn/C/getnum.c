static char sccsid[] = "@(#)99	1.1  src/bos/usr/lib/learn/C/getnum.c, cmdlearn, bos411, 9428A410j 1/24/90 18:04:32";
/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online 
 *  		   documentation
 * 
 *  FUNCTIONS: 
 * 
 *  ORIGINS: 27
 * 
 *  Copyright (c) 1980 Regents of the University of California.
 *  All rights reserved.  The Berkeley software License Agreement
 *  specifies the terms and conditions for redistribution.
 */ 

#include <stdio.h>

getnum()
{
	int c, n;

	n = 0;
	while ((c=getchar()) >= '0' && c <= '9')
		n = n*10 + c - '0';
	if (c == EOF)
		return(-1);
	return(n);
}
