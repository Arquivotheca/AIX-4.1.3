static char sccsid[] = "@(#)79	1.1  src/bos/usr/lib/nls/loc/imk/khhc/Hhcmkclst.c, libkr, bos411, 9428A410j 5/25/92 15:44:05";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhcmkclst.c
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
 *  Module:       Hhcmkclst.c
 *
 *  Description:  Makes a candidate of a key.
 *
 *  Functions:    make_cand()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include	<sys/types.h>
#include	<stdio.h>
#include	"Hhcim.h"	
#include	"kedconst.h"

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern char 	*calloc();
extern char 	*memcpy();
extern void	set_15and7_bit();
extern int	get_cand_len();

/*------------------------------------------------------------------------------*/
/*                      Begining of make_cand()                                 */
/* 	Purpose:  Makes a candidate from candidate block.	  		*/
/*------------------------------------------------------------------------------*/
uchar	*make_cand(dblock, bound, start, clen)
ushort 	dblock[];
int	bound, start, *clen;
{
register int	size=0;
register int	i=start;
register int	len=0;
register int	j=0; 
ushort		*cand;

	len = get_cand_len(dblock, bound, i);
	if (len <= 0) 
	{
	   return (NULL);  /* there isn't a proper candidate. */
	}
	while(j < len) 
	{
	   if (IsIBMUniqueCode(dblock[i+j])) 
	   {
		return (NULL); /* IBM unique code isn't supported. */
	   }
	   j++;
	}	
	size = len*2;
	cand = (ushort*) calloc ((size+1),2); 
	if (cand == NULL) 
	{
	   return NULL;
	}
	if (size <= IMACAND_MAXLEN) 
	{
	   memcpy (cand, (char*)&dblock[i], size);
	}
	set_15and7_bit (cand, len); /* in order to save KS string. */
	*(cand+size) = NULL;
	*clen = len;
	return (unsigned char*) cand;
}		
/*------------------------------------------------------------------------------*/
/*                      End of make_cand()                                      */
/*------------------------------------------------------------------------------*/
