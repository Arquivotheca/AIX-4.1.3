/* @(#)22	1.4  src/bos/usr/lib/nls/loc/jim/jpro/jspro.c, libKJI, bos411, 9428A410j 6/6/91 14:49:52 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jimpro.h>

/*-----------------------------------------------------------------------*
*	search pre-defined location for JIM profile
*-----------------------------------------------------------------------*/
char	*searchprofile(amode)
int	amode;		/* access mode */
{
	/*******************/
	/* local variables */
	/*******************/
	char	*path[NUMPATHS];
	char	*file;
	int	i;

	/*********************/
	/* set up path names */
	/*********************/
	path[0] = (char *)getenv(ENVVAR);

	file = (char *)getenv("HOME");
	path[1] = malloc(strlen(file) + sizeof (FILENAME) + 1);
	(void)sprintf(path[1], "%s/%s", file, FILENAME);

	path[2] = DEFAULTFILE;

	/***********************************/
	/* check to see if the file exists */
	/***********************************/
	for (i = 0; i < NUMPATHS; i++) {
		if (!path[i] || !*path[i])
			continue;
		file = malloc((unsigned int)strlen(path[i]) + 1);
		(void)strcpy(file, path[i]);
		if (access(file, amode) >= 0)
			break;
		free(file);
		file = NULL;
	}

	/************************************************/
	/* free memory allocted to store $HOME/FILENAME */
	/************************************************/
	free(path[1]);

	return file;
} /* end of searchprofile */
