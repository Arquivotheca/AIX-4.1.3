static char sccsid[] = "@(#)12	1.5.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kczread.c, libKJI, bos411, 9428A410j 7/23/92 03:17:11";
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
 * MODULE NAME:       _Kczread
 *
 * DESCRIPTIVE NAME:  READ DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0584(FZK_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0684(FZK_READ):   error of read()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0884(FZK_LOCKF):  error of lockf()
 *                    0x0b10(RQ_RECOVER): request recovery of user dictionary
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
short   _Kczread( z_kcbptr, z_catego, z_rrn, z_mode, z_file )

struct  KCB     *z_kcbptr;              /* pointer of KCB               */
short           z_catego;               /* category type                */
short           z_rrn;                  /* rrelative record number      */
short           z_mode;                 /* rrelative record number      */
short           z_file;                 /* System File Number           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
#ifdef _AIX
   extern   short   _Kcxstat();
#endif

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#ifdef _AIX
#include <fcntl.h>                      /* include for fcntl            */
#include <sys/lockf.h>                  /* include for lockf()          */
#endif

#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_RETRY          10           /* number of retry for lockf()  */
#define   Z_IQ           0x01           /* request of inquring user dct */

#define   Z_SX_OFFSET    0x06           /* offset of INDEX of SYS DCT   */
#define   Z_SD_OFFSET    4096           /* offset of DATA of SYS DCT    */
#define   Z_MRU_OFFSET      0           /* offset of MRU in USER DCT    */
#define   Z_FD_OFFSET       0           /* offset of DATA of FUZOKU DCT */
#define   Z_SD_RECORD    2048           /* record size of DATA of SYSDCT*/
#define   Z_UD_RECORD    1024           /* record size of DATA of USRDCT*/
#define   Z_MRU_RECORD   7168           /* record size of MRU in USERDCT*/
#define   Z_FD_RECORD     200           /* record size of DATA of FZKDCT*/

#define   Z_INIT            0           /* offset of initial location   */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   long         z_locate;               /* set seek point               */
   short        z_offset;               /* set offset                   */
   short        z_record;               /* set recored size             */
   short        z_ditype;               /* set type of dictionary       */
   char         *z_bufnam;              /* set buffer name              */
   short        z_stat;                 /* buffer of statas             */
   int          z_fildes;               /* set file descripter          */
   short        z_rc;
   short        z_sx_off;               /* offset of INDEX of SYS DCT   */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1  = z_kcbptr;                  /* establish KCB pointer       */
   mcbptr1  = ( struct MCB *)kcb.myarea; /* get MCB pointer             */
   z_sx_off = mcb.mulsys[z_file].poly_p + Z_SX_OFFSET;

/*----------------------------------------------------------------------*
 *      CHECK CATEGORY OF DICTIONARY
 *----------------------------------------------------------------------*/
   switch ( z_catego )                  /* check the category code and  */
   {                                    /* set file descripter, & etc.  */
      case SX : kcb.sxesxe = (struct SXE *)(mcb.dsyseg[z_file]+z_sx_off);
                return ( SUCCESS );       

      case SD : kcb.sdesde = mcb.dsyseg[z_file] + 
			     (z_rrn*mcb.mulsys[z_file].record_size);
                return ( SUCCESS );      

      case TX : kcb.txetxe = mcb.dsyseg[z_file] + z_sx_off;
                return ( SUCCESS );      

      case TD : kcb.tdetde = mcb.dsyseg[z_file] + 
			     (z_rrn*mcb.mulsys[z_file].record_size) + 
			     Z_SD_OFFSET;
                return ( SUCCESS );    

      case UX : kcb.uxeuxe  = mcb.uxeuxe;
                return ( SUCCESS );

      case UD : kcb.udeude 
                    = mcb.udeude + ((z_rrn-8) * Z_UD_RECORD ) ;
                return ( SUCCESS );
#ifdef _AIX
      case MRU: z_fildes = mcb.dusfd;
                z_offset = Z_MRU_OFFSET;
                z_record = Z_MRU_RECORD;
                z_bufnam = ( char *)mcb.mdemde;
                kcb.mdemde = ( uschar *)z_bufnam;
                z_ditype = USR_ERROR;
                break;

      case FD : z_fildes = mcb.dfzfd;
                z_offset = Z_FD_OFFSET;
                z_record = Z_FD_RECORD;
                z_bufnam = ( char *)mcb.dfgdfg;
                kcb.dfgdfg = ( uschar *)z_bufnam;
                z_ditype = FZK_ERROR;
                break;
#else
#   if defined(_AGC) || defined(_GSW)
      case MRU: kcb.mdemde = mcb.mdemde;
                return( SUCCESS );

      case FD : kcb.dfgdfg = mcb.mdemde;
                return( SUCCESS );
#   endif
#endif

      default : return( UERROR );
   }

#ifdef _AIX
/*----------------------------------------------------------------------*
 *      LOCK DICTIONARY FILES
 *----------------------------------------------------------------------*/
   lseek( z_fildes, Z_INIT, 0 );

   switch( z_mode )
   {
      struct flock  z_flck;
      case EXCLUSIVE: if ( lockf( z_fildes, F_TLOCK, 0 ) == -1 )
                         return( LOCKF_ERROR * 0x0100 + z_ditype );
                      break; 
      case SHARE:
		      z_flck.l_type = F_RDLCK;
		      z_flck.l_whence = 1;
		      z_flck.l_start  = 0;
		      z_flck.l_len    = 0;
		      if ( fcntl( z_fildes , F_SETLK , &z_flck) == -1 )
                         return( LOCKF_ERROR * 0x0100 + z_ditype );
		      else
			 break;

      default:        return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      CALUCULATE PHYSICAL LOCATION, AND READ DICTIONARY    
 *----------------------------------------------------------------------*/
   if ( z_catego == MRU )
   {
       z_stat = _Kcxstat( mcbptr1, Z_IQ );
       switch( z_stat )
       {
          case SUCCESS:
          case UPDATING:
                        break;
          case MRUBROKEN:
                        memset(z_bufnam,0,z_record);
                        z_record = 0;   /* if MRU is broken, never use it */
                        break;

          case USR_LSEEK:
          case USR_LOCKF:
          case USR_READ:
          case RQ_RECOVER:
                        return( z_stat );
          default:      return( UERROR );
       }
   }

   z_locate = z_offset + ( z_rrn * z_record );
   z_rc = SUCCESS;

   if( lseek( z_fildes, z_locate, 0 ) == -1 )
      z_rc =  LSEEK_ERROR * 0x0100 + z_ditype;

   else
   {
      if( read( z_fildes, z_bufnam, z_record ) == -1 )
         z_rc = READ_ERROR * 0x0100 + z_ditype;
   }

/*----------------------------------------------------------------------*
 *      UNLOCK DICTIONARY FILES
 *----------------------------------------------------------------------*/
   lseek( z_fildes, Z_INIT, 0 );

   if ( lockf( z_fildes, F_ULOCK, 0 ) == -1 )
      z_rc = LOCKF_ERROR * 0x0100 + z_ditype;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rc );
#endif
}
