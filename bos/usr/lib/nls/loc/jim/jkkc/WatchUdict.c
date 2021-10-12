static char sccsid[] = "@(#)71	1.2  src/bos/usr/lib/nls/loc/jim/jkkc/WatchUdict.c, libKJI, bos411, 9428A410j 8/19/92 20:41:36";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       ReloadUdict, RefreshMcbUdict, WatchUdict
 *
 * DESCRIPTIVE NAME:  LOADING USER DICTIONARY EXCEPT FOR MRU AREA
 *                    CLEANUP MCB STRUCTURE ABOUT USER DICTIONARY
 *		      WATCHING/LOADING USER DICTIONARY EXCEPT FOR MRU AREA
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0388(MEM_MALLOC): err of malloc()
 *                    0x0582(USR_LSEEK):  err of lseek() with usr dict
 *                    0x0682(USR_READ):   err of read() with usr dict
 *                    0x0882(USR_LOCKF):  err of fcntl() with usr dict
 *                    0x0a82(USR_FSTAT):  err of fstat() with usr dict
 *                    0x0a10(UPDATING):   user dict is being updated
 *                    0x0b10(RQ_RECOVER): user dict should be recovered
 *                    0x0c10(MRUBROKEN):  MRU area should be recovered
 *                    0x1082(USR_INCRR):  Usr dictionary incorrected
 *                    0x7FFF(UERROR):     this code is fatal error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#include "_Kcmap.h"                   	/* Define Constant file         */
#include <fcntl.h>                      /* include for open             */
#include <sys/lockf.h>                  /* include for lockf()          */
#include <sys/stat.h>                   /* define for fstat()           */
#include <sys/errno.h>                  /* include for errno            */

#include "_Kcmcb.h"                     /* Monitor Control Block (MCB)  */
#include "_Kcuxe.h"                     /* User dictionary index        */

/************************************************************************
 *      START OF FUNCTION ( ReloadUdict() )
 ************************************************************************/
short 	ReloadUdict(
UDICTINFO	dict 	/* pointer to user dictionary information       */
)
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
    short        _Kcxstat();
    short        _Kcxusrd();

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_RETRY     10
#define   Z_EXLEN     20
#define   Z_READ       1
#define   Z_IQ      0x01

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
    short        z_rt;                   /* set return code             */
    short        z_i;                    /* loop counter                */
    char         z_udlen[2];
    struct stat  z_buf;                  /* infomation of fstat         */
    struct UXE   z_uxe;
    struct flock z_flck;
    long         z_indlen;		/* length of dictinnary buffer 	*/
    long         z_indlen_t;		/* length of dictinnary buffer 	*/
    struct UXE   *z_uxeuxe;		/* Addr of user dict index buff	*/
    struct UXE   *z_uxeuxe_t;		/* Addr of user dict index buff	*/
    uschar       z_dctst;               /* dictionary status            */

/*----------------------------------------------------------------------*
 *      GET INFORMATION OF USER DICTIONARY
 *----------------------------------------------------------------------*/
    if ( fstat( dict->dusfd, &z_buf ) < 0 )
	return ( USR_FSTAT );

/*----------------------------------------------------------------------*
 *  LOCK USER DICTIONARY FILE
 *----------------------------------------------------------------------*/
    z_flck.l_type = F_RDLCK;
    z_flck.l_whence = z_flck.l_start = z_flck.l_len = 0;
    for ( z_i = 0; z_i < Z_RETRY; z_i++ ) {
      	if(( z_rt = fcntl( dict->dusfd, F_SETLK, &z_flck )) != -1 )
            break;
    }
    if ( z_rt == -1 )
        return ( USR_LOCKF );

/*----------------------------------------------------------------------*
 *  CHECK THE SIZE WHETHER IT MATCHES TO TOTAL RECORD LENGTH
 *----------------------------------------------------------------------*/
    if ( lseek( dict->dusfd, MDESIZE, 0 ) == -1 )
   	return ( USR_LSEEK );

    if ( read( dict->dusfd, &z_uxe , 5 ) == -1 )
   	return ( USR_READ );

    if(z_buf.st_size != ( (z_uxe.har+1) * UDESIZE ) )
   	return ( USR_INCRR );

    z_indlen = 0;             /* Initial Index Recode Length  		*/

    /*----- Index Recode is 1 Record(1Kbyte) ---------------------------*/
    if ((z_uxe.har >= U_HAR_V1) && (z_uxe.har <= U_HAR_V2))  z_indlen = 1;

    /*----- Index Recode is 2 Record(2Kbyte) ---------------------------*/
    if ((z_uxe.har >= U_HAR_V3) && (z_uxe.har <= U_HAR_V4))  z_indlen = 2;

    /*----- Index Recode is 3 Record(3Kbyte) ---------------------------*/
    if ((z_uxe.har >= U_HAR_V5) && (z_uxe.har <= U_HAR_V6))  z_indlen = 3;

    /*----- Invalid har Value ------------------------------------------*/
    if ( z_indlen == 0 )  return ( RQ_RECOVER );

/*----------------------------------------------------------------------*
 *  CHECK THE DATA LENGTH OF EACH RECORD
 *----------------------------------------------------------------------*/
    for ( z_i = U_BASNAR + z_indlen; z_i < z_uxe.nar; z_i++ ) {

        /*----- seek the top of a record -------------------------------*/
	if ( lseek( dict->dusfd, (z_i * UDESIZE) , 0 )  == -1 )
       		return ( USR_LSEEK );

        /*----- read the value of length -------------------------------*/
       	if ( read( dict->dusfd, z_udlen , 2 )  == -1 )
         	return ( USR_READ );

        /*----- compare the value --------------------------------------*/
        if( GETSHORT(z_udlen) > UDESIZE )
         	return ( USR_INCRR );
    }

/*----------------------------------------------------------------------*
 *  CHECK THE MRU OF THE FILE WHETHER VALID OR NOT
 *----------------------------------------------------------------------*/
    if ( lseek( dict->dusfd, 0 , 0 )  == -1 )
      		return ( USR_LSEEK );

    if ( read( dict->dusfd, z_udlen , 2 )  == -1 )
      		return ( USR_READ );

    if ( GETSHORT(z_udlen) > MDESIZE )
      		return ( USR_INCRR );

/*----------------------------------------------------------------------*
 *  ALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
    if (( z_uxeuxe = 			/* get pointer for INDEX of UDCT*/
	( struct UXE *)malloc( (z_buf.st_size - MDESIZE))) == NULL )
      		return ( MEM_MALLOC );

/*----------------------------------------------------------------------*
 * INQUIRE THE STATUS OF USER DICTIONARY FILE
 *   z_rt = _Kcxstat( z_mcbptr, Z_IQ );
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_INIT_OFFSET      0            /* point for lseek()            */
#define Z_STAT_OFFSET   7170            /* point for STAT area          */
#define Z_NORMAL        0x00            /* set record number.index+data */
#define Z_RECOVER       0xf0            /* Dictionary must be recovered */
#define Z_USED          0xf1            /* Dict. being used by Utility  */
#define Z_MRUBRKN       0xf2            /* MRU might be broken          */

    if ( ( lseek( dict->dusfd, Z_STAT_OFFSET, 0 ) ) == -1 )
	z_rt = USR_LSEEK;
    else {
	if ( read( dict->dusfd, &z_dctst, 1 )  == -1 )
            z_rt = USR_READ;
        else {
            switch ( z_dctst ) {
            case Z_NORMAL:    z_rt = SUCCESS;
			      break;
            case Z_USED:      z_rt = UPDATING;
                              break;
            case Z_RECOVER:   z_rt = RQ_RECOVER;
                              break;
            case Z_MRUBRKN:   z_rt = MRUBROKEN;
                              break;
            default:          z_rt = RQ_RECOVER;
                              break;
            }
        }
    }
    if ( z_rt != SUCCESS ) {
      	lseek( dict->dusfd, Z_INIT_OFFSET, 0 );
      	if ( ( z_rt = lockf( mcb.dusfd, F_ULOCK, 0 ) ) == -1 )
    	z_rt = USR_LOCKF;
    }

    switch( z_rt ) {
    case SUCCESS:
	break;

    case MRUBROKEN:
    case UPDATING:
    default:
       	free( z_uxeuxe );
        return( z_rt );
    }
/*----------------------------------------------------------------------*
 *  COPY USER DICTIONARY FROM THE OPENED FILE TO BUFFER
 *   z_rt = _Kcxusrd( z_mcbptr, Z_READ , (z_buf.st_size-MDESIZE) );
 *----------------------------------------------------------------------*/
#define Z_INDEX_OFFSET  7168            /* offset for INDEX area        */
    z_indlen_t = dict->indlen;			/* backup	*/
    z_uxeuxe_t = (struct UXE *)(dict->uxeuxe);	/* backup       */
    dict->indlen = z_indlen;
    dict->uxeuxe = (char *)z_uxeuxe;

    if ( ( lseek( dict->dusfd, Z_INDEX_OFFSET , 0) ) == -1 )
    	z_rt = USR_LSEEK;               /* error of lseek()           	*/
    else if( read( dict->dusfd, dict->uxeuxe, (z_buf.st_size-MDESIZE) ) == -1 )
        z_rt = USR_READ;         	/* error of read()              */
    else
    	z_rt = SUCCESS;

    if ( z_rt != SUCCESS ) {
      	lseek( dict->dusfd, Z_INIT_OFFSET, 0 );
      	if ( ( z_rt = lockf( mcb.dusfd, F_ULOCK, 0 ) ) == -1 )
    	z_rt = USR_LOCKF;
    }

    switch( z_rt ) {
    case SUCCESS:   break;

    default:
      	free( dict->uxeuxe );
	dict->indlen = z_indlen_t;
	dict->uxeuxe = (char *)z_uxeuxe_t;
       	return ( z_rt );
    }

/*----------------------------------------------------------------------*
 *  UNLOCK USER DICTIONARY FILE
 *----------------------------------------------------------------------*/
    z_flck.l_type = F_UNLCK;
    for ( z_i = 0; z_i < Z_RETRY; z_i++ ) {
      	if(( z_rt = fcntl( dict->dusfd, F_SETLK, &z_flck )) != -1 )
	    break;
    }
    if ( z_rt == -1 ) {
        free( dict->uxeuxe );
	dict->indlen = z_indlen_t;
	dict->uxeuxe = (char *)z_uxeuxe_t;
        return ( USR_LOCKF );
    }

/*----------------------------------------------------------------------*
 *  FREE THE OLD BUFFER
 *----------------------------------------------------------------------*/
    free( z_uxeuxe_t );

/*----------------------------------------------------------------------*
 *  SET THE NEW MODIFICATION TIME
 *----------------------------------------------------------------------*/
    dict->dumtime = z_buf.st_mtime;
    dict->ductime = z_buf.st_ctime;

/*----------------------------------------------------------------------*
 *  RETURN
 *----------------------------------------------------------------------*/
    return ( SUCCESS );
}

/************************************************************************
 *      START OF FUNCTION ( RefreshMcbUdict() )
 ************************************************************************/
void 	RefreshMcbUdict(
struct	MCB	*z_mcbptr,	/* pointer to MCB structure            	*/
UDICTINFO	dict	/* pointer to user dictionary information       */
)
{
/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
    short        z_i;                    /* loop counter                */

/*----------------------------------------------------------------------*
 *  Set MCB structure from UDICTINFO structure
 *----------------------------------------------------------------------*/
    /*
     * These are set by _Kcopdcu().
     * strncpy( z_mcbptr->dusnm, dict->udictname, 80);
     * for ( z_i=0; z_i<80; z_i++ ) {
     *	if ( z_mcbptr->dusnm[z_i] == 0 )
     *	    break;
     * }
     * z_mcbptr->dusnmll = z_i;
     */
    z_mcbptr->dusfd  = dict->dusfd;
    z_mcbptr->indlen = dict->indlen;
    z_mcbptr->mdemde = dict->mdemde;
    z_mcbptr->uxeuxe = (struct UXE *)(dict->uxeuxe);
    z_mcbptr->udeude =
	(uschar *)z_mcbptr->uxeuxe + (UXESIZE * z_mcbptr->indlen);
    z_mcbptr->dumtime = dict->dumtime;
    z_mcbptr->ductime = dict->ductime;
    z_mcbptr->kcbaddr->uxeuxe = z_mcbptr->uxeuxe;
    z_mcbptr->kcbaddr->udeude = z_mcbptr->udeude;
    z_mcbptr->kcbaddr->mdemde = z_mcbptr->mdemde;

/*----------------------------------------------------------------------*
 *  RETURN
 *----------------------------------------------------------------------*/
    return;
}

/************************************************************************
 *      START OF FUNCTION ( WatchUdict() )
 ************************************************************************/
short 	WatchUdict(
struct  MCB *z_mcbptr  /* pointer to MCB structure              	*/
)
{

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
    UDICTINFO	GetCurrentUDICTINFO();
    static  UDICTINFO  z_udictptr;      /* ptr to user dict information */
    short        z_rt;          /* set return code              	*/
    struct stat  z_buf;         /* infomation of fstat          	*/

/*----------------------------------------------------------------------*
 *      SET UDICTINFO structure
 *----------------------------------------------------------------------*/
    z_udictptr = GetCurrentUDICTINFO();

/*----------------------------------------------------------------------*
 *      GET INFORMATION OF USER DICTIONARY
 *----------------------------------------------------------------------*/
    if ( fstat( z_udictptr->dusfd, &z_buf ) < 0 )
        return ( USR_FSTAT );

/*----------------------------------------------------------------------*
 *      READ USER DICTIONARY AGAIN IF NEED
 *----------------------------------------------------------------------*/
    if ( z_udictptr->dumtime < z_buf.st_mtime ||
                z_udictptr->ductime < z_buf.st_ctime ) {

        /*--------------------------------------------------------------*
         *  LOAD DATA AREA IN USER DICTIONARY
         *--------------------------------------------------------------*/
        z_rt = ReloadUdict( z_udictptr );
	RefreshMcbUdict( z_mcbptr, z_udictptr );

        /*--------------------------------------------------------------*
         *  RETURN
         *--------------------------------------------------------------*/
        return( z_rt );

    }
    else if ( z_mcbptr->dumtime < z_udictptr->dumtime ||
    		z_mcbptr->ductime < z_udictptr->ductime ) {
	RefreshMcbUdict( z_mcbptr, z_udictptr );
    }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return ( UNREAD_UDCT );

}
