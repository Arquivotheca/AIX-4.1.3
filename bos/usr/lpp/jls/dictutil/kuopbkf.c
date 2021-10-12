static char sccsid[] = "@(#)64	1.1  src/bos/usr/lpp/jls/dictutil/kuopbkf.c, cmdKJI, bos411, 9428A410j 7/22/92 23:37:34";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuopbkf
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
 * MODULE NAME:       kuopbkf
 *
 * DESCRIPTIVE NAME:  OPENING USER DICTIONARY FOR BACKUP
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0    (IUSUCC):exit without any errors
 *                    -6501(IUFAIL):exit with some errors
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
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kut.h"        /* Kanji Utility Define File.  			*/

/*----------------------------------------------------------------------*
 * Define.
 *----------------------------------------------------------------------*/
#define	MAX_FNAME	256

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

char	*kugnbkf();

int	kuopbkf( pathname, udcptr, msgid )
char	*pathname;	/* pathname of user dictionary			*/
UDCB  	*udcptr;	/* user dictionary file control block		*/
short	*msgid;		/* message ID                           	*/

{

    int	   fatal_f;		/* return code				*/
    char   *bakname;		/* ptr to backupping pathname		*/

    /*----- initialize -------------------------------------------------*/
    fatal_f = IUSUCC;           /* initial fatal error flag           	*/
    udcptr->tmpname = NULL;
    udcptr->tmpfd = U_FILEE;

    /*----- get the file name ------------------------------------------*/
    bakname = kugnbkf( pathname );
    if ( bakname == NULL ) {
    	*msgid  = U_FMSGN;         	/* set message number         	*/
	fatal_f = IUFAIL;
	return ( fatal_f );
    }
    udcptr->tmpname = bakname;

    /*----- open pathname.bak ------------------------------------------*/
    udcptr->tmpfd = open( udcptr->tmpname, (O_WRONLY | O_CREAT), 0666 );
    if( udcptr->tmpfd == U_FILEE ) {
    	*msgid  = U_FMSGN;
        fatal_f = IUFAIL;
	free( udcptr->tmpname );
	udcptr->tmpname = NULL;
	return ( fatal_f );
    }

    /*----- return -----------------------------------------------------*/
    return ( fatal_f );

}

