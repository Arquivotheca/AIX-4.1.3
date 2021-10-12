static char sccsid[] = "@(#)93	1.1  src/bos/usr/lib/learn/C/getline.c, cmdlearn, bos411, 9428A410j 1/24/90 18:03:16";
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

getline(s, lim)	/* get line into s, return length */
char s[];
int lim;
{
	int c, i;

	i = 0;
	while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		s[i++] = c;
	if (c == '\n')
		s[i++] = c;
	s[i] = '\0';
	return(i);
}
