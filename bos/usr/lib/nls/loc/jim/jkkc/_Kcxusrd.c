static char sccsid[] = "@(#)07	1.4 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxusrd.c, libKJI, bos411, 9428A410j 1/7/93 03:40:09";
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
 * MODULE NAME:       _Kcxusrd
 *
 * DESCRIPTIVE NAME:  READ & WRITE CONTENT OF USER DICTIONARY ON MEMORY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x0582(USR_LSEEK): error of lseek()
 *                    0x0682(USR_READ):  error of read() during reading 
 *                    0x0782(USR_WRITE): error of write() during writing
 *                    0x0888(USR_LOCKF): error of lockf()
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
short _Kcxusrd( z_mcbptr, z_task , z_length )

struct MCB   *z_mcbptr;                 /* pointer of MCB               */
short        z_task;                    /* 1: read data of usrdict      
                                         * 2: write data of usrdict
                                         */
int          z_length;                  /* User dictionary Index and    */
                                        /* Data Length                  */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include <fcntl.h>                      /* include for open()           */
#include <sys/types.h>                  /* header for lseek()           */
#include <sys/lockf.h>                  /* header for lockf()           */
#include <sys/errno.h>                  /* include for errno            */

#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */

/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                    
 *----------------------------------------------------------------------*/
   off_t           lseek();

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_INIT_OFFSET      0            /* offset for initial point     */
#define Z_INDEX_OFFSET  7168            /* offset for INDEX area        */

#define Z_RECORD        1024            /* set record length            */
#define Z_NO              51            /* set record number.index+data */

#define Z_READ             1            /* request to read index & data */
#define Z_WRITE            2            /* request to write index & data*/

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   off_t        z_rtlseek;              /* return code for lseek()      */
   short        z_rt;                   /* return code                  */
   int          fd;                     /* file descriptor              */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* pointer of MCB               */

/*----------------------------------------------------------------------* 
 *      READ INDEX & DATA FROM USER DICTIONARY 
 *      WRITE INDEX & DATA TO USER DICTIONARY 
 *----------------------------------------------------------------------*/
   z_rt = SUCCESS;

   if ( z_task == Z_WRITE )
   {
      if ( (fd = OpenUdict(O_RDWR)) == -1 )
         z_rt = USR_LSEEK;
   }
   else
      fd = mcb.dusfd;

   if ( z_rt != SUCCESS ||
        ( z_rtlseek = lseek( fd, Z_INDEX_OFFSET , 0) ) == -1 )
      z_rt = USR_LSEEK;                 /* error of lseek()             */

   else
   {
      switch( z_task )
      {
         case Z_READ:
            if( read( fd, mcb.uxeuxe, z_length  ) == -1 )
               z_rt = USR_READ;         /* error of read()              */
            break;

         case Z_WRITE:
            errno = 0;
            if( write( fd, mcb.uxeuxe,z_length ) == -1 )
               if ( errno != EBADF )    /* if dictionary is read only   */ 
                  z_rt = USR_WRITE;     /* error of write()             */
            close ( fd );
            break;
   
         default:
            z_rt = UERROR;              /* unpredictable error          */
            break;
      }
   }

/*----------------------------------------------------------------------* 
 *      IF ERRORS OCCURES
 *----------------------------------------------------------------------*/
   if ( z_rt != SUCCESS )
   {
      lseek( mcb.dusfd, Z_INIT_OFFSET, 0 );

      if ( ( z_rt = lockf( mcb.dusfd, F_ULOCK, 0 ) ) == -1 )
         z_rt = USR_LOCKF;
   }

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rt );
}
