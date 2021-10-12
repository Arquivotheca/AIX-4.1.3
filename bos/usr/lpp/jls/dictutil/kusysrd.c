static char sccsid[] = "@(#)65	1.1  src/bos/usr/lpp/jls/dictutil/kusysrd.c, cmdKJI, bos411, 9428A410j 7/22/92 23:40:06";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kusysrd
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
 * MODULE NAME:       kusysrd
 *
 * DESCRIPTIVE NAME:  READING SYSTEM DICTIONARIES
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
#include <string.h>   	/*                                              */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kut.h"        /* Kanji Utility Define File.  			*/

/*----------------------------------------------------------------------*
 * Define.
 *----------------------------------------------------------------------*/
#if defined(_OLD_AIX320)
#define	SYSDICT_ENV	"JIMSYSDICT"
#define U_SYSDIC        "/usr/lpp/jls/dict/sysdict"
#else
#define	SYSDICT_ENV	"JIMMULDICT"
#define U_SYSDIC        "/usr/lpp/jls/dict/ibmbase"
#endif /* defined(_OLD_AIX320) */

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";


int	kusysrd( sdcptr, msgid )

SDCB  	*sdcptr;	/* system dictionary file control block		*/
short	*msgid;		/* message ID                           	*/
{
    char   *sysname;		/* system dictionary file name 		*/
    int    sfldes[MAX_SYSDICT];	/* system dictionary file descripter 	*/
    int    i, j;		/* loop counter                         */
    int	   flg=0;		/* flag for finding system dictionary	*/
    char   *sepchar;		/* pointer to ':' in sys dict. names	*/
    char   *names[MAX_SYSDICT];	/* pointer to sys dict. names		*/
    int    fatal_f;         	/* fatal error flag                     */
    int    ret;			/* return code                          */
    struct stat  stbuf;         /* file status work buffer              */
    int    snelem;              /* system dictionary file size buffer   */
    int	   already=0;		/* flag for comparing sys dict. name	*/

    /*----- initialize -------------------------------------------------*/
    sepchar = NULL;
    fatal_f = IUFAIL;
    for ( i=0; i<MAX_SYSDICT; i++ ) {
    	sfldes[i] = U_FILEE;
	sdcptr[i].dcptr = NULL;
	sdcptr[i].rdptr = NULL;
	sdcptr[i].wtptr = NULL;
	names[i] = NULL;
    }

    /*----- get environment variable of system dictionary file ---------*/
    if ( (sysname = getenv( SYSDICT_ENV )) != NULL ) {
    	fatal_f = IUSUCC;     /* initial fatal error flag           	*/

        /*----- open system dictionary file ----------------------------*/
        for ( i=0; i<MAX_SYSDICT; i++ ) {
	    if ( sepchar != NULL )
                sysname = sepchar + 1;
	    else if ( i != 0 )
		break;
            sepchar = strchr( sysname, ':' );
            if ( sepchar != NULL )
                *sepchar = '\0';
	    names[i] = sysname;
	    for ( j=0; j<i; j++ ) {
	    	if ( strcmp( names[j], names[i] ) == 0 ) {
		    already++;
		    break;
		}
	    }
	    if ( already ) {
		i--;
		already = 0;
		continue;
	    }
            sfldes[i] = open( sysname, (O_RDONLY | O_NDELAY) );
	    if ( sfldes[i] == U_FILEE ) {
                for ( j=0;j<i; j++ ) {
		    close( sfldes[j] );
                    sfldes[j] = U_FILEE;
                }
	    	fatal_f = IUFAIL;
		break;
 	    }
        }
    }

    if ( fatal_f == IUFAIL ) {
    	sfldes[0] = open( U_SYSDIC, (O_RDONLY | O_NDELAY));
        if ( sfldes[0] != U_FILEE ) {
	    fatal_f = IUSUCC;
	}
    }

    if ( fatal_f != IUSUCC ) {
    	*msgid = U_AHMSGN;          /* set message number        	*/
    }

    /*----- open successful --------------------------------------------*/
    for ( i=0; i<MAX_SYSDICT; i++ ) {

    	if ( sfldes[i] == U_FILEE )
            break;

        /*----- get system dictionary file status ----------------------*/
        ret = fstat( sfldes[i], &stbuf );
	if ( ret == -1 ) {
            *msgid = U_AHMSGN;   	/* set message number        	*/
	    fatal_f = IUFAIL;
	    break;
	}

        /*----- get system dictionary file size(byte) ------------------*/
        snelem = stbuf.st_size;

        /*----- allocate memory for the system dictionary --------------*/
        sdcptr[i].dcptr = (uchar *)calloc( snelem, sizeof(uchar) );
        if ( sdcptr[i].dcptr == NULL ) {/* cannot allocate memory 	*/
            *msgid  = U_FMSGN;         	/* set message number         	*/
	    fatal_f = IUFAIL;
	    break;
        }
        else if ((read( sfldes[i], sdcptr[i].dcptr, snelem )) == U_FILEE) {
            /*----- system dictionary file read error ------------------*/
            *msgid  = U_AHMSGN;       	/* set message number         	*/
            fatal_f = IUFAIL;         	/* set fatal error            	*/
	    break;
        }
	else {
	    sdcptr[i].st_size = snelem;
            close( sfldes[i] );		/* close system dictionary file	*/
	    sfldes[i] = U_FILEE;
	}
    }

    if ( fatal_f == IUFAIL ) {
	for ( i=0; i<MAX_SYSDICT; i++ ) {
	    if ( sdcptr[i].dcptr != NULL ) {
		free( sdcptr[i].dcptr );
		sdcptr[i].dcptr = NULL;
	    }
	    if ( sfldes[i] != U_FILEE ) {
		close( sfldes[i] );
		sfldes[i] = U_FILEE;
	    }
	}
    }

    /*----- ruturn -----------------------------------------------------*/
    return ( fatal_f );
}

