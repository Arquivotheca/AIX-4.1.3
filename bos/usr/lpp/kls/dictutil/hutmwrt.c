static char sccsid[] = "@(#)30	1.1  src/bos/usr/lpp/kls/dictutil/hutmwrt.c, cmdkr, bos411, 9428A410j 5/25/92 14:48:38";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hutmwrt.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hutmwrt.c
 *
 *  Description:  write temporaly file
 *
 *  Functions:    hutmwrt()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*      include file.							*/
/*----------------------------------------------------------------------*/

#include <fcntl.h>          /* File Control Package.                    */
#include <stdio.h>
#include <unistd.h>
#include <sys/lockf.h>	    /* added by Maekawa */
#include "hut.h"            /* Utility Define File                      */


/*----------------------------------------------------------------------*/
/*      Begining of hutmwrt.						*/
/*----------------------------------------------------------------------*/
int hutmwrt( flname, dcptr, fsize )
uchar   *flname;        /* pointer to temporaly file for out put        */
uchar   *dcptr;         /* pointer to memory of user dictionary */
long    fsize;          /* user Dictionary file size    */
{
  int           fdesc;  /* file Descripter      */
  int           de_ret; /* return value */
  int           st_ret; /* return value */
  int           i;
  short 	u_ordwr  = 0x00f0; /* Opened for Read/Write */
  short		u_prests;	   /* Previous File Status */

  /* creat temporaly file */
  if((fdesc = creat(flname,(int)(00644))) == U_ERR) {
    /* file creat error */
    return( -1 );
  }

  /* lock tmporaly file */
  for(i=0;((lockf(fdesc,F_TLOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++);
  if(i >= U_TRYLOK) {
    /* file lock error  */
    return( -3 );
  }

  /* 
   * Modify Comments:
   * src - (1) set 0xf0
   *	   (2) write User Dictionary.
   *	   (3) set 0x00
   * our - (1) get previous status.
   *	   (2) set 0x00f0
   *	   (3) write User Dictionary.
   *	   (4) restore previous status.
   */

  /* (1) */
  /* Get previous status */
  getudstat(dcptr, &u_prests);

  /* (2) */
  /* Data Set Opened for Read/Write */
  setudstat(dcptr, u_ordwr); 

  /* (3) */
  /* temporaly file out put     */
  de_ret = write(fdesc, dcptr, (unsigned)fsize) ;

  /* (4) */
  /* write to previous status data on memory */
  setudstat(dcptr, u_prests); 

  if( de_ret == fsize )
  {
     lseek(fdesc, 0L, 0);
     st_ret = write( fdesc, &u_prests, U_STSLEN );
     if( st_ret < U_STSLEN ) return( -2 );
  }

  /* unlock tmporaly file   */
  lseek(fdesc,0L,0);
  for(i=0;((lockf(fdesc,F_ULOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++);
  if(i >= U_TRYLOK) {
    /* file unlock error  */
    return( -4 );
  };

  if(de_ret < (int)fsize) {
    /* write error */
    (void)close(fdesc);
    return( -2 );
  };

  /* success return     */
  (void)close(fdesc);
  return(IUSUCC);
}
/*----------------------------------------------------------------------*/
/*      End of hutmwrt.							*/
/*----------------------------------------------------------------------*/
