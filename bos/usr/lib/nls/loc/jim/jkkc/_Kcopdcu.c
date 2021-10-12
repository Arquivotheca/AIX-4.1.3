static char sccsid[] = "@(#)72	1.6.1.2  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopdcu.c, libKJI, bos411, 9428A410j 1/7/93 01:48:19";
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
 * MODULE NAME:       _Kcopdcu
 *
 * DESCRIPTIVE NAME:  OPEN USER DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0108(WSPOVER):    overflow of work space buffer
 *                    0x0182(USR_OPEN):   error of open()
 *                    0x0282(USR_CLOSE):  error of close()
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0a82(USR_FSTAT):  error of fstat on User   dict.
 *                    0x1082(USR_INCRR):  user   Dictionary not true
 *                    0x0388(MEM_MALLOC): error of malloc()
 *                    0x0b10(RQ_RECOVER): request of recovery
 *                    0x7fff(UERROR):     unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include <fcntl.h>                      /* include for open             */
#include <sys/lockf.h>                  /* include for lockf()          */
#include <sys/stat.h>                   /* define for fstat()           */
#include <sys/errno.h>                  /* include for errno 		*/

#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kcuxe.h"                   /* User dictionary index        */

#include   "dict.h"

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcopdcu( z_mcbptr, z_usrnm )

struct  MCB     *z_mcbptr;              /* pointer of MCB               */
char            *z_usrnm;
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   char         *getenv();              /* define getenv()              */
   char         *strcat();              /* define strcat()              */
   char         *strcpy();              /* define strcpy()              */

   short        _Kcxstat();
   short        _Kcxusrd();
   short        _Kccldcu();

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
   short        z_rt;                   /* return code                  */
   short        z_rcldcu;               /* return code                  */
   short        z_length;               /* set length of path name      */
   short        z_i;                    /* loop counter                 */
   char         z_fnmwk[ENVLIM+Z_EXLEN];/* temporaly work area          */
   char         *z_fname;
   char         *z_envnm;
   struct stat   z_buf;                 /* infomation of fstat          */
   struct UXE    z_uxe;
   char          z_udlen[2];
   struct flock  z_flck;
   UDICTINFO    GetCurrentUDICTINFO();  /* get current Udict pointer    */
   UDICTINFO    udictp;                 /* user dict information        */

/*----------------------------------------------------------------------*
 *      Check whether the dictionary is already opened or not
 *----------------------------------------------------------------------*/
   udictp = GetCurrentUDICTINFO();
   if ( udictp->dusfd >= 0 )
      return( _Kcopdcu2( z_mcbptr, z_usrnm ) );

/*----------------------------------------------------------------------* 
 *      SET BASE POINTER
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'th to mcb  */
   z_fname = NULL;

/*----------------------------------------------------------------------* 
 *      OPEN THE FILE REQUESTED BY KJ-MONITOR
 *----------------------------------------------------------------------*/
   if(( mcb.dusfd = OpenFile( z_usrnm, O_RDONLY )) != -1 )
      z_fname = z_usrnm;
   else
      return( USR_OPEN );

/*******************************************************/
/* following portion is removed by #ifdef 7/04/89  $01 */
#ifdef _OLD_VER_70489
   if ( z_fname == NULL )
   {

   /*-------------------------------------------------------------------* 
    *      OPEN THE FILE DEFINED IN ENVIRONMENT VARIABLE
    *-------------------------------------------------------------------*/
      if(( z_envnm = getenv( ENVUSR ) ) != NULL )
      {
         if(( mcb.dusfd = open( z_envnm, O_RDWR )) != -1 )
            z_fname = z_envnm;
         else
            return( USR_OPEN );
      }

      else
      {

      /*----------------------------------------------------------------* 
       *      OPEN THE FILE UNDER THE CURRENT DIRECTRY
       *----------------------------------------------------------------*/
         if(( mcb.dusfd = open( USRDICT, O_RDWR )) != -1 )
            z_fname  = USRDICT;            /* point the path name area  */

         else
         {

         /*-------------------------------------------------------------* 
          *      OPEN THE FILE UNDER THE HOME DIRECTRY
          *-------------------------------------------------------------*/
            if ( ( z_envnm = getenv( "HOME" ) ) != NULL )
            {

               if ( ( strlen(z_envnm) + 1 + strlen( USRDICT ) ) 
                                                >= ( ENVLIM + Z_EXLEN ) )
                  return( WSPOVER );    /*  too long                    */
   
               strcpy( &z_fnmwk[0], z_envnm );
               strcat( &z_fnmwk[0] , "/"  );    /* $HOME + "/"          */
               strcat( &z_fnmwk[0] , USRDICT ); /* $HOME + "/" + dictnm */
               if(( mcb.dusfd = open( &z_fnmwk[0] , O_RDWR )) != -1 )
                  z_fname  = &z_fnmwk[0];       /* set the path name    */
            }

            if ( z_fname == NULL )
            {

            /*----------------------------------------------------------* 
             *      OPEN DEFAULT FILE
             *----------------------------------------------------------*/
               if ( ( strlen( DCTPATH ) + 1 + strlen( USRDICT ) )
                                                >= (ENVLIM + Z_EXLEN ) )
                  return( WSPOVER );

               strcpy( &z_fnmwk[0] , DCTPATH );
               strcat( &z_fnmwk[0] , "/" ); 
               strcat( &z_fnmwk[0] , USRDICT );

               if(( mcb.dusfd = open( &z_fnmwk[0] , O_RDWR )) != -1 )
                  z_fname  = &z_fnmwk[ 0 ];
               else
                  return( USR_OPEN );
            }
         }
      }
   }
#endif _OLD_VER_70489
/* above is removed by #ifdef 7/04/89  $01 */
/*******************************************/

/*----------------------------------------------------------------------*
 *      CHECK USER DICTIONARY
 *----------------------------------------------------------------------*/
   if( fstat( mcb.dusfd , &z_buf ) == -1 )      /* get file status      */
       return ( USR_FSTAT );

/*----------------------------------------------------------------------*
 *      LOCK USER DICTIONARY FILE
 *----------------------------------------------------------------------*/
   z_flck.l_type = F_RDLCK;
   z_flck.l_whence = z_flck.l_start = z_flck.l_len = 0;
   for ( z_i = 0; z_i < Z_RETRY; z_i++ ) {
      if(( z_rt = fcntl( mcb.dusfd, F_SETLK, &z_flck )) != -1 )
          break;
   }
   if ( z_rt == -1 ) {
      if ( ( z_rcldcu = _Kccldcu( z_mcbptr ) ) != SUCCESS )
         return( USR_CLOSE );
      else
         return( USR_LOCKF );
   }

   /*-------------------------------------------------------------------*
    *      CHECK THE SIZE WHETHER IT MATCHES TO TOTAL RECORD LENGTH
    *-------------------------------------------------------------------*/
   if ( lseek( mcb.dusfd, MDESIZE, 0 )  == -1 )
      return( USR_LSEEK );

   if ( read( mcb.dusfd, &z_uxe , 5 )  == -1 )
      return( USR_READ );

   if(z_buf.st_size != ( (z_uxe.har+1) * UDESIZE ) )
      return( USR_INCRR );

   mcb.indlen = 0;                      /* Initial Index Recode Length  */
                                     /* Index Recode is 1 Record(1Kbyte)*/
   if ((z_uxe.har >= U_HAR_V1) && (z_uxe.har <= U_HAR_V2))  mcb.indlen = 1;
                                    /* Index Recode is 2 Record(2Kbyte) */
   if ((z_uxe.har >= U_HAR_V3) && (z_uxe.har <= U_HAR_V4))  mcb.indlen = 2;
                                    /* Index Recode is 3 Record(3Kbyte) */
   if ((z_uxe.har >= U_HAR_V5) && (z_uxe.har <= U_HAR_V6))  mcb.indlen = 3;
                                     /* Invalid har Value                */
   if ( mcb.indlen == 0 )  return ( RQ_RECOVER );

   /*-------------------------------------------------------------------*
    *      CHECK THE DATA LENGTH OF EACH RECORD
    *-------------------------------------------------------------------*/
   for( z_i = U_BASNAR + mcb.indlen ; z_i < z_uxe.nar ; z_i++)
   {
                                        /* seek version                 */
      if ( lseek( mcb.dusfd, (z_i * UDESIZE) , 0 )  == -1 )
         return( USR_LSEEK );
                                        /* read version number          */
      if ( read( mcb.dusfd, z_udlen , 2 )  == -1 )
         return( USR_READ );
                                        /* compare version number       */
      if( GETSHORT(z_udlen) > UDESIZE )
         return( USR_INCRR );
   }

   /*-------------------------------------------------------------------*
    *      CHECK THE MRU OF THE FILE WHETHER VALID OR NOT
    *-------------------------------------------------------------------*/
   if ( lseek( mcb.dusfd, 0 , 0 )  == -1 )
      return( USR_LSEEK );


   if ( read( mcb.dusfd, z_udlen , 2 )  == -1 )
      return( USR_READ );

   if (GETSHORT(z_udlen) > MDESIZE )
      return( USR_INCRR );


/*----------------------------------------------------------------------* 
 *      SET INFORMATION OF USER DICTIONARY
 *----------------------------------------------------------------------*/
   mcb.dusnm[ 0 ] = 0x00;               /* reset the path name area     */

   if ( ( z_length = strlen( z_fname ) ) >= ENVLIM )
   {
      strncpy( mcb.dusnm , z_fname, ( ENVLIM - 1 ) );
      mcb.dusnm[ENVLIM-1] = '\0';
      mcb.dusnmll = ENVLIM - 1;         /* set length of path name      */
   }
   else
   {
      strcpy( mcb.dusnm , z_fname );    /* set path name                */
      mcb.dusnmll = z_length;           /* set length of path name      */
   }

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
   if(( mcb.mdemde = (uschar *)malloc( MDESIZE )) == NULL )
   {
      return( MEM_MALLOC );
   }

                                        /* get pointer for INDEX of UDCT*/
   if(( mcb.uxeuxe =
            ( struct UXE *)malloc( (z_buf.st_size - MDESIZE))) == NULL )
   {
      return( MEM_MALLOC );
   }

   mcb.udeude = ( uschar *)mcb.uxeuxe + (UXESIZE * mcb.indlen );
                                        /* set pointer for DATA of UDCT */

/*----------------------------------------------------------------------* 
 *      INQUIRE THE STATUS OF USER DICTIONARY FILE
 *----------------------------------------------------------------------*/
   z_rt = _Kcxstat( z_mcbptr, Z_IQ );

   switch( z_rt )
   {
      case SUCCESS:
      case MRUBROKEN:
      case UPDATING: break;

      default:
         free( mcb.mdemde );
         if ( ( z_rcldcu = _Kccldcu( z_mcbptr ) ) != SUCCESS )
            return( USR_CLOSE );
         else
            return( z_rt );
   }

/*----------------------------------------------------------------------* 
 *      COPY USER DICTIONARY FROM THE OPENED FILE TO BUFFER
 *----------------------------------------------------------------------*/
   z_rt = _Kcxusrd( z_mcbptr, Z_READ , (z_buf.st_size-MDESIZE) );

   switch( z_rt )
   {
      case SUCCESS:   break;

      default:
         free( mcb.mdemde );
         if ( ( z_rcldcu = _Kccldcu( z_mcbptr ) ) != SUCCESS )
            return( USR_CLOSE );
         else
            return( z_rt );
   }

/*----------------------------------------------------------------------*
 *  SET MODIFICATION TIME
 *----------------------------------------------------------------------*/
   mcb.dumtime = z_buf.st_mtime;
   mcb.ductime = z_buf.st_ctime;

/*----------------------------------------------------------------------* 
 *      UNLOCK USER DICTIONARY FILE
 *----------------------------------------------------------------------*/
   lseek( mcb.dusfd, 0 , 0 );

   z_flck.l_type = F_UNLCK;
   if(( z_rt = fcntl(mcb.dusfd,F_SETLK,&z_flck)) == -1 )
   {
      free( mcb.mdemde );
      if ( ( z_rcldcu = _Kccldcu( z_mcbptr ) ) != SUCCESS )
          return( USR_CLOSE );
      else
          return( USR_LOCKF );
   }

/*----------------------------------------------------------------------*
 *      Copy dictionary information to Udict structure
 *----------------------------------------------------------------------*/
   if (( udictp->udictname = (char *)malloc( strlen( z_fname ) + 1 )) != NULL )
      strcpy( udictp->udictname, z_fname );
   udictp->dusfd   = mcb.dusfd;
   udictp->indlen  = mcb.indlen;
   udictp->mdemde  = mcb.mdemde;
   udictp->uxeuxe  = (char *)mcb.uxeuxe;
   udictp->dumtime = mcb.dumtime;
   udictp->ductime = mcb.ductime;

/*----------------------------------------------------------------------*
 *       READ MRU IN USRDICT
 *----------------------------------------------------------------------*/
   z_rt = _Kczread(mcb.kcbaddr,(short)MRU,(short)0,(short)SHARE,(short)0);
   if ( z_rt != SUCCESS ) {
      free( mcb.mdemde );
      free( mcb.uxeuxe );
      if ( ( z_rcldcu = _Kccldcu( z_mcbptr ) ) != SUCCESS )
          return( USR_CLOSE );
      else
          return( z_rt );
   }

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}

/************************************************************************
 *	Open User Dictionary
 ************************************************************************/
int	OpenUdict
(
int	mode				/* file open mode		*/
)
{
/*----------------------------------------------------------------------* 
 *	DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
	UDICTINFO	udictp;		/* user dict information	*/
	int		fd;		/* file descriptor		*/

/*----------------------------------------------------------------------*
 *	Get the user dictionary name
 *----------------------------------------------------------------------*/
	udictp = GetCurrentUDICTINFO();
	if((udictp->dusfd < 0) || (udictp->udictname == NULL)){
		return(-1);
	}

/*----------------------------------------------------------------------* 
 *	Open the user dictionary file
 *----------------------------------------------------------------------*/
	if((fd = OpenFile(udictp->udictname, mode)) == -1){
		return(-1);
	}

/*----------------------------------------------------------------------*
 *	Return file descriptor
 *----------------------------------------------------------------------*/
	return(fd);
}
