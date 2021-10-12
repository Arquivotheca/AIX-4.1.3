static char sccsid[] = "@(#)60	1.1  src/bos/usr/lpp/jls/dictutil/kugnbkf.c, cmdKJI, bos411, 9428A410j 7/22/92 23:27:44";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kugnbkf
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       kugnbkf
 *
 * DESCRIPTIVE NAME:  GETTING THE NAME OF BAKUP FILE
 *
 * INPUT:	      file name of user dictionary
 *
 * OUTPUT:	      backup file name of user dictionary
 *
 * RETURN CODE:       ADDRESS   : pointer to the name string
 *                    NULL	: failed with some errors
 *
 ******************** END OF SPECIFICATIONS *****************************/

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>      /* Standard I/O Package.                        */
#include <fcntl.h>      /* File Control Package                         */
#include <sys/stat.h>   /*                                              */
#include <limits.h>   	/*                                              */
#include <stdlib.h>   	/*                                              */
#include <string.h>   	/*                                              */

/*----------------------------------------------------------------------*
 * Define.
 *----------------------------------------------------------------------*/
#define	MAX_FNAME	256

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

char	*kugnbkf( pathname )

char	*pathname;	/* pathname of user dictionary			*/
{
    int    i, j;		/* loop counter                         */
    int	   baselen, flen;	/* length of basename and filename	*/
    int	   ret;			/* return code 				*/
    wchar_t	*pwcs;		/* work buffer				*/
    char   *fname;		/* ptr to file name in pathname		*/
    char   *bakname;		/* ptr to backupping pathname		*/
    int    baklen;		/* length of backupping pathname	*/
    char   fname_area[MAX_FNAME+1];
    char   base_area[PATH_MAX+1];
    char   back_pathname[PATH_MAX+MAX_FNAME+1];

    /*----- check the length of pathname -------------------------------*/
    if ( strlen( pathname ) > PATH_MAX )
	return ( (char *)NULL );

    /*----- get the original file name and the base name ---------------*/
    fname = strrchr( pathname, '/' );
    if ( fname == NULL ) {
	fname = pathname;
	base_area[0] = '\0';
    }
    else {
	baselen = (size_t)(fname - pathname);
	strncpy( base_area, pathname, baselen );
	base_area[ baselen ] = '\0';
	fname++;
    }
    strcpy( fname_area, fname );

    /*----- make backup file name --------------------------------------*/
    while ( TRUE ) {
    	flen = strlen( fname_area );
    	if ( flen <= (MAX_FNAME - 4) ) {
    	    strcat( fname_area, ".bak");
	    break;
	}
    	else {
	    pwcs = (wchar_t *)malloc( (flen+1) * sizeof(wchar_t) );
    	    if ( pwcs == NULL )
	    	return ( (char *)NULL );

	    ret = mbstowcs( pwcs, fname_area, strlen(fname_area)+1 );
	    if ( ret == -1 ) {
	    	free( pwcs );
	    	return ( (char *)NULL );
	    }

	    memset( fname_area, '\0', MAX_FNAME+1 );
	    ret = wcstombs( fname_area, pwcs, ret-1 );
	    if ( ret == -1 ) {
	    	free( pwcs );
	    	return ( (char *)NULL );
	    }

	    free( pwcs );
	}
    }

    /*----- check the length of pathname.bak ---------------------------*/
    if ( (strlen( base_area ) + strlen( fname_area ) + 1) > PATH_MAX )
	return ( (char *)NULL );

    /*----- allocate a memory for pathname.bak -------------------------*/
    baklen = strlen( base_area ) + strlen( fname_area );
    bakname = malloc( baklen + 1 );
    if ( bakname == NULL ) {
	return ( (char *)NULL );
    }
    bakname[0] = '\0';

    /*----- make the name (pathname.bak) -------------------------------*/
    if ( base_area[0] != 0x00 ) {
    	strcat( bakname, base_area );
    	strcat( bakname, "/" );
    }
    strcat( bakname, fname_area );

    /*----- return -----------------------------------------------------*/
    return ( bakname );

}

