static char sccsid[] = "@(#)89	1.1  src/bos/usr/lib/nls/loc/imk/kpro/kspro.c, libkr, bos411, 9428A410j 5/25/92 15:45:52";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kspro.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM Profile
 *
 *  Module:       kspro.c
 *
 *  Description:  korean Input Method profile
 *                search function
 *
 *  Functions:    searchprofile()
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include "kimpro.h"

/*-----------------------------------------------------------------------*
*	external references
*-----------------------------------------------------------------------*/
extern	char	*getenv(), *malloc(), *strcpy();
extern	void	free();

/*-----------------------------------------------------------------------*
*	search pre-defined location for KIM profile
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
