static char sccsid[] = "@(#)75  1.2  src/bos/usr/lib/nls/loc/imk/khhc/Hhcfindrbn.c, libkr, bos411, 9428A410j 7/21/92 00:37:49";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		Hhcfindrbn.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
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
 *  Module:       Hhcfindrbn.c
 *
 *  Description:  Finds the Relative Block Number(RBN) of a key
 *                at the Index Block.
 *
 *  Functions:    find_rbn()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include	<stdio.h>
#include	"kedconst.h"
#include	"Hhcim.h"	

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern int	next_key_len();
extern int	kimstrcmp(); 
extern void	reset_15and7_bit();

/*------------------------------------------------------------------------------*/
/*                      Begining of find_rbn()                                  */
/* 	Purpose : 	Finds a proper RBN of the key data.			*/
/*------------------------------------------------------------------------------*/
int		find_rbn (ixblock, bound, key, klen, filetype)
ushort		ixblock[], key[];
int		bound, klen, filetype;
{
ushort		*nextkey;
int		p_rbn_setted=FALSE, len=0, result; 
register int	i=0; 

	/* Init, Search start position. */
	if (filetype == SYSDICT) i=0; 
	else i=3;

	/* Is input parameter vaild ? */
	if (ixblock == NULL || bound <= 0 || key == NULL || klen <= 0) 
	{
	   return (KP_ERR);
	}
	while (i < bound) 
	{
		if (IsBufEmpty(ixblock[i])) 
		{
 		   if (p_rbn_setted)
		   {
                       /* The previous rbn is closest to our rbn  */
                       return ixblock[i-1];
		   }
		   else 
		   {
		       /* buffer empty */
		       return (KP_ERR);
		   }
		}
		/* get a next keydata length */
                len = next_key_len(ixblock, bound, i);
		if (len <= 0) 
		{
		   /* There isn't a proper next key */
		   return (KP_ERR);
		}
                nextkey = (ushort*)malloc((len*2)*2);
                memcpy((unsigned char*)nextkey, (unsigned char*)&ixblock[i], len*2);
		/* Convert the string, in order to compare */
		reset_15and7_bit (nextkey, len);
		result = kimstrcmp (key, klen, nextkey, len);
		free(nextkey);
		if (result == 0) 
		{
		   /* find it */
		   return ixblock[i+len];
		} 
		else if (result == 1) 
		{
		   p_rbn_setted = TRUE;
		   i += (len+1);	
		} 
		else 
		{
		   if (p_rbn_setted) 
		   {
		      return ixblock[i-1];
		   }
		   else 
		   {
		      return (KP_ERR);
		   }
		}
	}	
	return (KP_ERR);	
}
/*------------------------------------------------------------------------------*/
/*                      End of find_rbn()                                       */
/*------------------------------------------------------------------------------*/
