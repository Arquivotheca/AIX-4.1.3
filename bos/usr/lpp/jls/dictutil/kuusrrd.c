static char sccsid[] = "@(#)66	1.1  src/bos/usr/lpp/jls/dictutil/kuusrrd.c, cmdKJI, bos411, 9428A410j 7/22/92 23:42:04";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuusrrd
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
 * MODULE NAME:       kuusrrd
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

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kut.h"        /* Kanji Utility Define File.  			*/

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

int	kuusrrd( usrname, udcptr, msgid )
char	*usrname;	/* user dictionary file name			*/
UDCB  	*udcptr;	/* system dictionary file control block		*/
short	*msgid;		/* message ID                           	*/

{

    int    ufldes;		/* user dictionary file descripter 	*/
    int    i, j;		/* loop counter                         */
    int    fatal_f;         	/* fatal error flag                     */
    int    ret;			/* return code                          */
    struct stat  stbuf;         /* file status work buffer              */
    struct flock flck;          /* flock structure for fcntl()          */
    char  nmlsts = 0x00;   	/* normal status code buffer            */
    char  wrtsts = 0xf1;   	/* write status code buffer             */

    /*----- initialize -------------------------------------------------*/
    fatal_f = IUSUCC;           /* initial fatal error flag           	*/
    *msgid  = U_ENDID;          /* set normal message number          	*/
    ufldes  = U_FILEE;
    udcptr->dcptr = NULL;
    udcptr->rdptr = NULL;
    udcptr->wtptr = NULL;
    udcptr->updflg = U_FOF;

    /* ------ Open User dictionary -------------------------------------*/
    if ( (ufldes = open( usrname, (O_RDWR | O_NDELAY))) == U_FILEE ) {
        *msgid   = U_AGMSGN;   /* set error number             		*/
        fatal_f  = IUFAIL;     /* set fatal error              		*/
	return ( fatal_f );
    }
    udcptr->orgfd = ufldes;    /* set user dictionary file discripter  	*/

    /*------- Lock User dictionary -------------------------------------*/
    flck.l_type = F_RDLCK;
    flck.l_whence = flck.l_start = flck.l_len = 0;
    for ( i=0; i<U_TRYLOK; i++) {
        if ( (ret = fcntl( ufldes, F_SETLK, &flck )) != -1 )
                break;
    }
    if ( ret == -1 ) {
        *msgid   = U_AGMSGN; 	/* set message number              	*/
        fatal_f  = IUFAIL;   	/* set fatal error                 	*/
	close( ufldes );
	return ( fatal_f );
    }

    /*------- Get the size of User dictionary  -------------------------*/
    ret = fstat( ufldes, &stbuf );
    if ( ret == -1 ) {
        *msgid   = U_AGMSGN; 	/* set message number              	*/
        fatal_f  = IUFAIL;   	/* set fatal error                 	*/
	close( ufldes );
	return ( fatal_f );
    }
    udcptr->ufilsz = stbuf.st_size;

    /*------- Allocate memory for User dictionary ----------------------*/
    udcptr->dcptr  = (uchar *)calloc(udcptr->ufilsz, sizeof(uchar));
    udcptr->secbuf = (uchar *)calloc(udcptr->ufilsz, sizeof(uchar));
    udcptr->thdbuf = (uchar *)calloc(udcptr->ufilsz, sizeof(uchar));
    if ( udcptr->dcptr == NULL || udcptr->secbuf == NULL || 
		udcptr->thdbuf == NULL ) {
        *msgid   = U_FMSGN; 	/* set message number              	*/
        fatal_f  = IUFAIL;   	/* set fatal error                 	*/
	close( ufldes );
	return ( fatal_f );
    }

    /*------- Read User dictionary -------------------------------------*/
    if ( (read(ufldes, udcptr->dcptr, udcptr->ufilsz)) == U_FILEE ) {
        *msgid   = U_AGMSGN; 	/* set message number              	*/
        fatal_f  = IUFAIL;   	/* set fatal error                 	*/
	free(udcptr->dcptr);
	free(udcptr->secbuf);
	free(udcptr->thdbuf);
	close( ufldes );
	return ( fatal_f );
    }

    /*------- Copy main buffer to third buffer for backup---------------*/
    memcpy( udcptr->thdbuf, udcptr->dcptr, udcptr->ufilsz );

    /*------- Check/Set User dictionary's status -----------------------*/
    if ( *(udcptr->dcptr + U_STATUS) != nmlsts ) {
        if ( *(udcptr->dcptr + U_STATUS) == wrtsts ) {
            /* ----- another updating ----------------------------------*/
            *msgid  = U_AKMSGN;     /* set message number               */
            fatal_f = IUUPDAT;      /* set fatal error                  */
        } else {
            /* ----- status error --------------------------------------*/
            *msgid   = U_DMSGN;     /* set message number               */
            fatal_f  = IURECOV;     /* set recovery status              */
        }
    } else {
        if ( (lseek(ufldes, (long)U_STATUS, 0)) == U_FILEE ) {
            *msgid   = U_AGMSGN;    /* set message number               */
            fatal_f  = IUFAIL;      /* set fatal error                  */
            perror("Seek Error ");
	    free(udcptr->dcptr);
	    free(udcptr->secbuf);
	    free(udcptr->thdbuf);
	    close( ufldes );
	    return ( fatal_f );
        }
        if( (write(ufldes, &wrtsts, 1)) == U_FILEE ) {
            *msgid    = U_AGMSGN;  	/* set message number           */
            fatal_f  = IUFAIL;      	/* set fatal error              */
            perror("Status Write Error ");
	    free(udcptr->dcptr);
	    free(udcptr->secbuf);
	    free(udcptr->thdbuf);
	    close( ufldes );
	    return ( fatal_f );
        }
    	*(udcptr->dcptr + U_STATUS) = wrtsts;
    }

    /*------- Copy main buffer to second buffer ------------------------*/
    memcpy( udcptr->secbuf, udcptr->dcptr, udcptr->ufilsz );

    /*------- Unlock User dictionary -----------------------------------*/
    flck.l_type = F_UNLCK;
    for ( i=0; i<U_TRYLOK; i++) {
        if ( (ret = fcntl( ufldes, F_SETLK, &flck )) != -1 )
                break;
    }
    if ( ret == -1 ) {
        *msgid   = U_AGMSGN; /* set message number              	*/
        fatal_f  = IUFAIL;   /* set fatal error                 	*/
	free(udcptr->dcptr);
	free(udcptr->secbuf);
	free(udcptr->thdbuf);
	close( ufldes );
	return ( fatal_f );
    }

    /*----- ruturn -----------------------------------------------------*/
    return ( fatal_f );
}

