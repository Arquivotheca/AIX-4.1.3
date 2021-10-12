static char sccsid[] = "@(#)81  1.3  src/bos/usr/lib/nls/loc/imk/khhc/Hhcudict.c, libkr, bos411, 9428A410j 6/9/94 09:56:22";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhcudict.c
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

/******************************************************************
 *
 *  Component:    Korean IM HHC  
 *
 *  Module:       Hhcudict.c
 *
 *  Description:  The routines of the User Dictionary access.
 *
 *  Functions:    get_udstat()
 *		  set_udstat()
 *		  get_udhar()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ******************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <fcntl.h>
#include "kedconst.h"

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern int kimlock();
extern int kimlockWR();
extern int kimunlock();

/*------------------------------------------------------------------------------*/
/*                      Begining of set_udstat()                                */
/* 	Purpose:  	Set User Dictionary Status.				*/
/*------------------------------------------------------------------------------*/
int  	set_udstat(fd, stat)
int	fd;
ushort 	stat;
{
   if (fd >= 0) 
   {
	/****************/
	/* Sets status. */
	/****************/
	kimlockWR(fd);
	lseek(fd, 0L, 0);
	write(fd, (char*)&stat, STATLENGTH);
	kimunlock(fd);
	return (KP_OK);
   } 
   else 
   {
	return (KP_ERR);
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of get_udhar()                                 */
/*	Purpose:	Gets High Allocated Recore number.			*/
/*------------------------------------------------------------------------------*/
short get_udhar(fd)
int	fd;
{
short   har;

   if (fd >= 0)
   {
        kimlock(fd);
        lseek(fd, 2048+2, 0);
        if (read(fd, (char*)&har, 2) <= 0) 
        {
                return (KP_ERR);
        }
        kimunlock(fd);
        return (ushort)har;
   } 
   else
   {
        return (KP_ERR);
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of get_udstat()                                */
/* 	Purpose:	Gets User Dictionary Status.				*/
/*------------------------------------------------------------------------------*/
ushort	get_udstat(fd)
int	fd;
{
ushort stat;

   if (fd >= 0) 
   {
	/****************/
	/* Gets status. */
	/****************/
	kimlock(fd);
	lseek(fd, 0L, 0);
	if (read(fd, (char*)&stat, STATLENGTH) <= 0) 
	{
		return (KP_ERR);
	}
	kimunlock(fd);
	return (ushort)stat;
   } 
   else 
   {
	return (KP_ERR);
   }
}
