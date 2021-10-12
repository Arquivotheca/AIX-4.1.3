static char sccsid[] = "@(#)77	1.3  src/bos/usr/lib/nls/loc/imk/khhc/Hhclock.c, libkr, bos411, 9428A410j 6/9/94 13:31:10";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhclock.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************************************************************************
 *
 *  Component:    Korean IM HHC  
 *
 *  Module:       Hhclock.c
 *
 *  Description:  Locks and Unlocks the User Dictionary.
 *
 *  Functions:    kimlock()
 *		  kimunlock()
 *
 *  History:      5/20/90  Initial Creation.     
 *		  6/08/94  lock calls changed to use fcntl() since lockf was
 *			   changed in AIX 4.1 to do exclusive locks.
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include <sys/errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "kedconst.h"

/*------------------------------------------------------------------------------*/
/*                      Defines Macros.						*/
/*------------------------------------------------------------------------------*/

#define UD_TRYOK  10

/*------------------------------------------------------------------------------
*/
/*                      Begining of kimlock()
*/
/*      PURPOSE :       Locks the file  - READ LOCK
*/
/*------------------------------------------------------------------------------
*/
int     kimlock(fd)
int     fd;
{
int    i=0;
int    ret;
struct flock flck;          /* flock structure for fcntl()          */


   lseek(fd, 0L, 0);
   flck.l_type = F_RDLCK;
   flck.l_whence = flck.l_start = flck.l_len = 0;
   for ( i=0; i<UD_TRYOK; i++) {
       if ( (ret = fcntl( fd, F_SETLK, &flck )) != -1 )
                break;
   }
   if ( ret == -1 ) {
      /****************/
      /* System error */
      /****************/
      perror("kimlock");
      exit(2);
      return (KP_ERR);
   }
   return (KP_OK);
}


/*------------------------------------------------------------------------------*/
/*                      Begining of kimlockWR()                                   */
/*	PURPOSE : 	Locks the file for writing.						*/
/*------------------------------------------------------------------------------*/
int     kimlockWR(fd)
int	fd;
{
int    i=0;
int    ret;
struct flock flck;          /* flock structure for fcntl()          */


   lseek(fd, 0L, 0);
   flck.l_type = F_WRLCK;
   flck.l_whence = flck.l_start = flck.l_len = 0;
   for ( i=0; i<UD_TRYOK; i++) {
       if ( (ret = fcntl( fd, F_SETLK, &flck )) != -1 )
                break;
   }
   if ( ret == -1 ) {
      /****************/
      /* System error */
      /****************/
      perror("kimlockWR");
      exit(2);
      return (KP_ERR);
   }
   return (KP_OK);
}

/****************/
/* Unlocks file */
/****************/
/*------------------------------------------------------------------------------*/
/*                      Begining of kimunlock()                                 */
/*	PURPOSE :	Unlocks the file.					*/
/*------------------------------------------------------------------------------*/
kimunlock(fd)
int	fd;
{
int    i;
int    ret;
struct flock flck;          /* flock structure for fcntl()          */

   lseek(fd, 0L, 0);
   flck.l_type = F_UNLCK;
   flck.l_whence = flck.l_start = flck.l_len = 0;
   for ( i=0; i<UD_TRYOK; i++) {
       if ( (ret = fcntl( fd, F_SETLK, &flck )) != -1 )
               break;
   }
   if ( ret == -1 ) {
      /****************/
      /* System error */
      /****************/
      perror("kimunlock");
      exit(2);
      return (KP_ERR);
   }
   return (KP_OK);
}
