static char sccsid[] = "@(#)06	1.5 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxstat.c, libKJI, bos411, 9428A410j 1/7/93 03:39:45";
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
 * MODULE NAME:       _Kcxstat
 *
 * DESCRIPTIVE NAME:  REGISTRATION OF WORD ENTRY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x0582(USR_LSEEK): error of lseek()
 *                    0x0888(USR_LOCKF): error of lockf()
 *
 *                    0x0782(USR_WRITE): error of write() during setting status
 *
 *                    0x0682(USR_READ):  error of read() during inquiring 
 *                    0x0a10(UPDATING):  being updated during inquiring 
 *                    0x0b10(RQ_RECOVER):request of recovery during inquiring 
 *
 *                    0x7fff(UERROR):    unpredictable error
 *
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
short   _Kcxstat( z_mcbptr, z_stat )

struct MCB   *z_mcbptr;                 /* pointer of MCB               */
char          z_stat;                   /* 0xf0: set status ON(0xf0)
                                         * 0x00: set status OFF(0x00)
                                         * 0x01: inquire status
                                         */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include <fcntl.h>                      /* define for open()            */
#include <sys/lockf.h>                  /* header for lockf()           */
#include <sys/errno.h>                  /* include for errno            */
#include   "_Kcmcb.h"   /* Monitor Control Block (MCB)                  */
#include   "_Kcuxe.h"   /* User dictionary indeX Entry (UXE)            */

/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                    
 *----------------------------------------------------------------------*/
   off_t           lseek();

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_INIT_OFFSET      0            /* point for lseek()            */
#define Z_STAT_OFFSET   7170            /* point for STAT area          */

#define Z_NORMAL        0x00            /* set record number.index+data */
#define Z_RECOVER       0xf0            /* Dictionary must be recovered */
#define Z_USED          0xf1            /* Dict. being used by Utility  */
#define Z_MRUBRKN       0xf2            /* MRU might be broken          */

#define Z_IQ            0x01            /* set record number.index+data */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rt;                   /* return code                  */
   short        z_ret;                  /* return code  of own          */
   uschar       z_dctst;                /* dictionary status            */
   off_t        z_rlseek;
   int          fd;                     /* file descriptor              */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* pointer of MCB               */

/*----------------------------------------------------------------------* 
 *      DETECT STATUS OF USER DICTIONARY 
 *----------------------------------------------------------------------*/
   z_ret = SUCCESS;

   if ( z_stat != Z_IQ )
   {
      if ( (fd = OpenUdict(O_RDWR)) == -1 )
         z_ret = USR_LSEEK;
   }
   else
      fd = mcb.dusfd;

   if ( z_ret != SUCCESS ||
        ( z_rlseek = lseek( fd, Z_STAT_OFFSET, 0 ) ) == -1 )
      z_ret = USR_LSEEK;

   else
   {
      switch( z_stat )
      {
         case Z_IQ:
            if ( read( fd, &z_dctst, 1 )  == -1 )
            {
               z_ret = USR_READ;
            }
            else
            {
               switch ( z_dctst )
               {
                  case Z_NORMAL:    break;
                  case Z_USED:      z_ret = UPDATING;
                                    break;
                  case Z_RECOVER:   z_ret = RQ_RECOVER;
                                    break;
                  case Z_MRUBRKN:   z_ret = MRUBROKEN;
                                    break;
                  default:          z_ret = RQ_RECOVER;
                                    break;
               }
            }
            break;

         default:
            errno = 0;
            if ( write( fd, &z_stat, 1 ) == -1 )
               if ( errno != EBADF )
                  z_ret = USR_WRITE;
            close ( fd );
            uxeptr1 = mcb.uxeuxe;
            uxe.sts = z_stat;
            break;

      }
   }

/*----------------------------------------------------------------------* 
 *      ERRORS
 *----------------------------------------------------------------------*/
   if ( z_ret != SUCCESS )
   {
      lseek( mcb.dusfd, Z_INIT_OFFSET, 0 );

      if (  lockf( mcb.dusfd, F_ULOCK, 0 )  == -1 )
         z_ret = USR_LOCKF;
   }

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_ret );
}
