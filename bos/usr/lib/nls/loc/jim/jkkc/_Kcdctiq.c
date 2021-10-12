static char sccsid[] = "@(#)50	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcdctiq.c, libKJI, bos411, 9428A410j 6/4/91 10:23:55";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcdctiq
 *
 * DESCRIPTIVE NAME:  REGISTRATION OF WORD ENTRY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0388(MEM_MALLOC): error of malloc()
 *                    0x0582(USR_LSEEK):  error of lseek() with user dict.
 *                    0x0682(USR_READ):   error of read() with user dict.
 *                    0x0882(USR_LOCKF):  error of lockf() with user dict.
 *                    0x1082(USR_INCRR):  user   Dictionary not true
 *                    0x0a10(UPDATING):   being updated
 *                    0x0a82(USR_FSTAT):  error of fstat on User   dict.
 *                    0x0b10(RQ_RECOVER): recovery of user dict. is requested
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


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcdctiq( z_mcbptr )

struct  MCB     *z_mcbptr;              /* pointer of MCB              */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#ifdef _AIX
#include <sys/lockf.h>                  /* header for lockf()          */
#include <sys/stat.h>                   /* define for fstat()           */
#endif
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcuxe.h"                   /* User dictionary index        */

/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                    
 *----------------------------------------------------------------------*/
   extern  short   _Kcofkkc();          /* main of kkc                  */
#ifdef _AIX
   extern  short   _Kcxstat();          /* main of kkc                  */
   extern  short   _Kcxusrd();          /* main of kkc                  */
#endif
#ifdef _AIX
   off_t           lseek();
#endif

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_READ           1
#define   Z_WRITE          2

#define   Z_SET_ON      0xf0
#define   Z_SET_OFF     0x00
#define   Z_IQ          0x01

#define   Z_INIT_OFFSET    0

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rt;                   /* return code                  */
   short        z_rofkkc;               /* return code of _Kcofkkc()    */

#ifdef _AIX
   struct stat     z_buf;               /* infomation of fstat          */
   struct UXE      z_uxe;
   short           z_i;
   uschar          z_udlen[2];
#endif

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* base pointer of MCB          */
   kcbptr1 = mcb.kcbaddr;               /* base pointer of KCB          */

/*----------------------------------------------------------------------* 
 *      SET INPUT PARAMETERS IN KCB
 *----------------------------------------------------------------------*/

#ifdef _OLD
   _Kctalph( mcbptr1 );                /* Matching old monitor interface*/
#endif

   kcb.mode     = mcb.mode;             /* copy conversion mode         */
   kcb.ymill1   = mcb.ymill1;           /* length of input yomi         */
   kcb.seill    = mcb.seill;            /* length of input seisho       */

#ifdef _AIX
/*----------------------------------------------------------------------* 
 *      LOCK USER DICTIONARY WITH LOCKF()
 *----------------------------------------------------------------------*/
   lseek( mcb.dusfd, Z_INIT_OFFSET, 0 ); 

   if ( ( z_rt = lockf( mcb.dusfd, F_TLOCK, 0 ) ) == -1 )
   {
      return( UPDATING );
   }

/*----------------------------------------------------------------------*
 *      CHECK USER DICTIONARY
 *----------------------------------------------------------------------*/
   if( fstat( mcb.dusfd , &z_buf ) == -1 )      /* get file status      */
       return ( USR_FSTAT );

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
 *      REALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
   free( mcb.uxeuxe );
   if(( mcb.uxeuxe =
            ( struct UXE *)malloc( (z_buf.st_size - MDESIZE))) == NULL )
   {
      return( MEM_MALLOC );
   }

   mcb.udeude = ( uschar *)mcb.uxeuxe + (UXESIZE * mcb.indlen );
                                        /* set pointer for DATA of UDCT */

/*----------------------------------------------------------------------* 
 *      DETECT STATUS OF USER DICTIONARY 
 *----------------------------------------------------------------------*/
   z_rt = _Kcxstat( z_mcbptr, Z_IQ );

   switch( z_rt )
   {
      case SUCCESS:   break;

      case UPDATING:  
      case RQ_RECOVER:  
      case USR_LOCKF:  
      case USR_READ:  
      case USR_LSEEK:  
      case USR_INCRR:
                      return( z_rt );
      default:        return( UERROR );
   }

/*----------------------------------------------------------------------* 
 *      READ INDEX & DATA FROM USER DICTIONARY 
 *----------------------------------------------------------------------*/
   z_rt = _Kcxusrd( z_mcbptr, Z_READ ,(z_buf.st_size-MDESIZE));

   switch( z_rt )
   {
      case SUCCESS:   break;

      case USR_LOCKF:  
      case USR_READ:  
      case USR_LSEEK:  
                      return( z_rt );
      default:        return( UERROR );
   }
#endif

/*----------------------------------------------------------------------* 
 *      REGISTRATE WORD ON BUFFER OF USER DICTIONARY 
 *----------------------------------------------------------------------*/
   kcb.func     = FUNDCTIQ;             /* registration of word entry   */

   z_rofkkc = _Kcofkkc( kcbptr1 );

   switch( z_rofkkc )
   {
      case SUCCESS:
         break;

      default:
         if((z_rofkkc & LOCAL_RC) != 0x0010)
            z_rofkkc = UERROR;
         break;
   }

#ifdef _AIX
/*----------------------------------------------------------------------* 
 *      UNLOCK USER DICTIONARY WITH LOCKF()
 *----------------------------------------------------------------------*/
   lseek( mcb.dusfd, Z_INIT_OFFSET, 0 );

   if ( ( z_rt = lockf( mcb.dusfd, F_ULOCK, 0 ) ) == -1 )
      z_rofkkc = USR_LOCKF;
#endif

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rofkkc );
}
