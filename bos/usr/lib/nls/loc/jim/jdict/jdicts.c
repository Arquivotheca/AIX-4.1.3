static char sccsid[] = "@(#)63	1.4.1.3  src/bos/usr/lib/nls/loc/jim/jdict/jdicts.c, libKJI, bos411, 9428A410j 10/22/92 11:50:23";
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jimdict.h>

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
} /* end of searchprofile */

/*-----------------------------------------------------------------------*
*	search pre-defined location for adjunct dictionary
*-----------------------------------------------------------------------*/
char	*findadjdict(amode)
int	amode;		/* access mode */
{
	/*******************/
	/* local variables */
	/*******************/
	char	*path[ADICT_NUMPATHS];
	char	*file;
	int	i;

	/*********************/
	/* set up path names */
	/*********************/
	path[0] = (char *)getenv(ADICT_ENVVAR);

	file = (char *)getenv("HOME");
	path[1] = malloc(strlen(file) + sizeof(ADICT_FILENAME) + 1);
	(void)sprintf(path[1], "%s/%s", file, ADICT_FILENAME);

	path[2] = ADICT_DEFAULTFILE;

	/***********************************/
	/* check to see if the file exists */
	/***********************************/
	for (i = 0; i < ADICT_NUMPATHS; i++) {
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
} /* end of findadjdict */

/*-----------------------------------------------------------------------*
*	search pre-defined location for system dictionary
*-----------------------------------------------------------------------*/
char	**findsysdict(amode)
int	amode;		/* access mode */

{
	/*******************/
	/* local variables */
	/*******************/
	static	char	*file[SDICT_NUM+1];
	char		*path[SDICT_NUMPATHS],*file_path,*subfile,*jimmuldict;
	int		i,j,fno,access_flag;

	/************************/
	/* Initialize Variables */
	/************************/
	jimmuldict = NULL;

	/*********************/
	/* set up path names */
	/*********************/
	if(( path[0] = (char *)getenv(SDICT_ENVVAR))) {
	    if(( jimmuldict = (char *)malloc((int)strlen(path[0]) + 1 )) == NULL ) 
		return NULL;
	    strcpy( jimmuldict, path[0] );
	    path[0] = jimmuldict;
	}
	path[1] = SDICT_DEFAULTFILE;

	/***********************************/
	/* check to see if the file exists */
	/***********************************/
	for (i = 0; i < SDICT_NUMPATHS; i++) {
		if (!path[i] || !*path[i])
			continue;
		for( fno = 0, access_flag = 0, file_path = path[i]; fno < SDICT_NUM; ) { 
		    if(( subfile = strtok( file_path,":")) == (char *)NULL) 
			break;
		    if(( fno ) && ( same_dict( file, subfile, fno )))
			continue;
		    if( access( subfile, amode ) < 0 ) {
			access_flag |= TRUE; 
			break;
		    }
		    if(( file[fno] = (char *)malloc((int)strlen(subfile) + 1 )) == NULL ) {
			if( jimmuldict ) free( jimmuldict );
			return NULL;
		    }
		    (void)strcpy(file[fno++], subfile ); 
		    file_path = (char *)NULL;
		}
		if( access_flag ) 
		    for( j = 0; j < fno ; j++ )
		        free(file[j]);
		else
		    break;
	}
	file[fno] = (char *)NULL;
	if( jimmuldict ) free( jimmuldict );
	return file;
} /* end of findsysdict */

same_dict( char *f1[], char *f2, int fc )
{
	while( --fc >= 0 ) 
	    if( !strcmp( f1[fc], f2 ))
		return( TRUE );
	return( FALSE );
}
