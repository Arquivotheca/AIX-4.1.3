static char sccsid[] = "@(#)51	1.1  src/bos/usr/lib/nls/loc/imk/kdict/kdicts.c, libkr, bos411, 9428A410j 5/25/92 15:38:56";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kdicts.c
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
 *  Component:    Korean IM Dictionary 
 *
 *  Module:       kdicts.c
 *
 *  Description:  korean Input Method dictionary search functions
 *
 *  Functions:    findusrdict()
 * 		  findsysdict()
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include "kimdict.h"

/*-----------------------------------------------------------------------*
*	external references
*-----------------------------------------------------------------------*/
extern	char	*getenv(), *malloc(), *strcpy();
extern	void	free();

/*-----------------------------------------------------------------------*
*	search pre-defined location for user dictionary
*-----------------------------------------------------------------------*/
char	*findusrdict(amode)
int	amode;		/* access mode */
{
	/*******************/
	/* local variables */
	/*******************/
	char	*path[UDICT_NUMPATHS];
	char	*file;
	int	i;

	/*********************/
	/* set up path names */
	/*********************/
	path[0] = (char *)getenv(UDICT_ENVVAR);

	file = (char *)getenv("HOME");
	path[1] = malloc(strlen(file) + sizeof (UDICT_FILENAME) + 1);
	(void)sprintf(path[1], "%s/%s", file, UDICT_FILENAME);

	path[2] = UDICT_DEFAULTFILE;

	/***********************************/
	/* check to see if the file exists */
	/***********************************/
	for (i = 0; i < UDICT_NUMPATHS; i++) {
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
} /* end of findusrdict */


/*-----------------------------------------------------------------------*
*	search pre-defined location for system dictionary
*-----------------------------------------------------------------------*/
char	*findsysdict(amode)
int	amode;		/* access mode */
{
	/*******************/
	/* local variables */
	/*******************/
	char	*path[SDICT_NUMPATHS];
	char	*file;
	int	i;

	/*********************/
	/* set up path names */
	/*********************/
	path[0] = (char *)getenv(SDICT_ENVVAR);

	path[1] = SDICT_DEFAULTFILE;

	/***********************************/
	/* check to see if the file exists */
	/***********************************/
	for (i = 0; i < SDICT_NUMPATHS; i++) {
		if (!path[i] || !*path[i])
			continue;
		file = malloc((unsigned int)strlen(path[i]) + 1);
		(void)strcpy(file, path[i]);
		if (access(file, amode) >= 0)
			break;
		free(file);
		file = NULL;
	}

	return file;
} /* end of findsysdict */
