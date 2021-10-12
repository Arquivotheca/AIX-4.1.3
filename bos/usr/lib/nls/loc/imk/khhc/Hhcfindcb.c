static char sccsid[] = "@(#)74  1.2  src/bos/usr/lib/nls/loc/imk/khhc/Hhcfindcb.c, libkr, bos411, 9428A410j 7/21/92 00:37:20";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		Hhcfindcb.c
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
 *  Module:       Hhcfindcb.c
 *
 *  Description:  Finds the index of a key's candidates
 *		  at a data block of User Dictionary.
 *
 *  Functions:    find_cand_block()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*			Includes Header Files.					*/
/*------------------------------------------------------------------------------*/

#include 	<sys/types.h>
#include	"kedconst.h"
#include	"Hhcim.h"

/*------------------------------------------------------------------------------*/
/* 			External reference 					*/
/*------------------------------------------------------------------------------*/

extern int	next_key_offset();
extern int	next_key_len();
extern int	kimstrcmp();
extern void	reset_15and7_bit ();

/*------------------------------------------------------------------------------*/
/* 			Begining of find_cand_block()				*/
/* Purpose:		Finds candidates block in data block.			*/
/*------------------------------------------------------------------------------*/
int		find_cand_block (buf, bound, key, klen, filetype)
ushort		buf[], key[];
int		bound, klen, filetype;
{	
ushort		*nextkey;
register int	i=0; 
int		len, result, offset=-1; 

	/* Init. Search start position */
	if (filetype == SYSDICT) 
	{
	   i=0;
	}
	else 
	{
	   i=1;
	}
	while (i < bound) 
	{
		/* check buffer empty */
		if (IsBufEmpty(buf[i])) 
		{
		   return (KP_ERR);
		}
		/* get n next keydata length */
		len = next_key_len(buf, bound, i);
		if (len <= 0) 
		{
		   /* there isn't a proper keydata. */
		   return (KP_ERR);
		}
		nextkey = (ushort*)malloc((len*2)*2);
		memcpy((unsigned char*)nextkey, (unsigned char*)&buf[i], len*2);
		/* Convert the string, in order to compare */
		reset_15and7_bit (nextkey, len);	
		result = kimstrcmp(key, klen, nextkey, len);
		free(nextkey);
		i += len;
		if (result == 0) 
		{
		   return i;	/* find it */
		}
		else if (result == 1) 
		{
			/* look for a next key */
			offset = next_key_offset(buf, bound, i);
			if (offset < 0) 
			{
		           /* there isn't a proper keydata. */
			   return (KP_ERR);
			}	
			i += offset;
		}
		else 
		{
		        /* there isn't our keydata. */
			return (KP_ERR);
		}
	}		
	return (KP_ERR);
} 
/*------------------------------------------------------------------------------*/
/* 			End of find_cand_block()				*/
/*------------------------------------------------------------------------------*/
