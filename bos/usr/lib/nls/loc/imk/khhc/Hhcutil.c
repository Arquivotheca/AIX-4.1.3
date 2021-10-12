static char sccsid[] = "@(#)82  1.5  src/bos/usr/lib/nls/loc/imk/khhc/Hhcutil.c, libkr, bos411, 9428A410j 5/17/94 10:18:49";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhcutil.c
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
 *  Module:       Hhcutil.c
 *
 *  Description:  The routines of the HHC managements.
 *
 *  Functions:    get_ix_block()
 *		  next_key_len()
 *		  next_key_offset()
 *		  get_cand_len()
 *		  reset_15and7_bit()
 *		  set_15and7_bit()
 *		  free_candidates()
 *		  makekey2rbn()
 * 		  makecand2rbn()
 *		  kimstrcmp()
 * 			
 *  History:      5/20/90  Initial Creation.     
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include	<sys/types.h>
#include	<fcntl.h>
#include	"kedconst.h"	
#include	"ked.h"
#include	"Hhcim.h"

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern   void	free(); 

/*------------------------------------------------------------------------------*/
/*                      Begining of get_ix_block()                              */
/*	Purpose:    Get System or User Dictionary Index Block.			*/
/*------------------------------------------------------------------------------*/
ushort	*get_ix_block(fd, filetype, ixbsize)
int	fd;
int	filetype;
int	*ixbsize;
{
int     status;
int	indexlen;
ushort  *buf;
long	offset;
short   har;

   if (filetype == SYSDICT) 
   {
	*ixbsize = SIXBLOCK_SIZE;
	offset = 0L;
   } 
   else 
   {
	kimlock(fd);
	lseek(fd, 0L, 0);
	har = get_udhar(fd);
	indexlen = 0;
        /* check overflow. */
	if ( (har >= U_HAR_V1) && (har <= U_HAR_V2) )  
	{
	  indexlen = 1;
	}
   	if ( indexlen == 0 )  
	{
	  return ( NULL );
	}
	*ixbsize = (indexlen*1024);
	offset = (2048);
	kimunlock(fd);
   }

   if (fd >= 0 && *ixbsize > 0) 
   {
	buf = (ushort*) malloc(*ixbsize+10);
	kimlock(fd);
	lseek(fd, offset, 0);
	if ((status=read(fd, buf, *ixbsize)) <= 0) 
     	{
	   return (NULL);
	}
	kimunlock(fd);
	return buf;
   } 
   else 
   {
	return (NULL);
   }
}
	
/*------------------------------------------------------------------------------*/
/*                      Begining of next_key_len()                              */
/*	Purpose:  get a key length from start of data buffer.			*/
/*------------------------------------------------------------------------------*/
int	next_key_len(buf, bound, start)
ushort	buf[];
int	bound, start;
{
register int	i=start;
register int	len=0;

   while (i < bound) 
   {
	len++;
	if (IsLastCharOfKey(buf[i])) 
 	{
	   /**********************************/
	   /*	   			     */
	   /* Finds the last char of the key */
	   /*				     */
	   /**********************************/
	   return (len);
	}
	i++;
   }
   return (KP_ERR);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of next_key_offset()                           */
/*	Purpose:  Get a next key offset from start of data buffer.		*/
/*------------------------------------------------------------------------------*/
int	next_key_offset(buf, bound, start)
ushort	buf[];
int	bound, start;
{
register int	i=start;
register int	len=0;

   while(i < bound) 
   {
	len++;
	if (IsLastCCOfKey(buf[i])) 
 	{
	   /*************************************/
	   /*					*/
	   /* Finds a last char of a last cand. */
	   /*					*/
	   /*************************************/
	   return (len);
	}
	i++;
   }
   return (KP_ERR);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of kimstrcmp()                                 */
/*	PURPOSE :	Compare strings.					*/
/*------------------------------------------------------------------------------*/
int     kimstrcmp (wd, wlen, twd, tlen)
int     wlen, tlen;
ushort  wd[], twd[];
{
register int    i=0; 
register int	j=0; 
register int	d;

   if (wlen == tlen) 
   {
      while (i < wlen && (d=CMP(wd[i],twd[j])) == 0) 
	 {i++; j++;}
         return (d == 0) ? 0 : (d > 0) ? 1: -1;
   } 
   else if (wlen > tlen) 
   {
      while (j < tlen && (d=CMP(wd[i],twd[j])) == 0) 
	 {i++; j++;}
         return (d == 0) ? 1: (d > 0) ? 1: -1;
   } 
   else 
   {
      while (i < wlen && (d=CMP(wd[i],twd[j])) == 0) 
	 {i++; j++;}
         return (d == 0) ? -1: (d > 0) ? 1: -1;
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of get_cand_len()                              */
/*	PURPOSE :	Get a candidate length from start of data buffer.	*/
/*------------------------------------------------------------------------------*/
int	get_cand_len(buf, bound, start)
ushort	buf[];
int	bound, start;
{
register int	i=start;
register int	len=0;

    while(i < bound) 
    {
	len++;
	if (IsLastCharOfCand(buf[i])) 
    	{
	   /*********************/
	   /* Finds a last char */
	   /* of a cand.        */
	   /*********************/
	   return (len);
	}
	i++;
    }
    return (KP_ERR);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of reset_15and7_bit()                          */
/*	PURPOSE :	Clear MSB of the string buffer.				*/
/*------------------------------------------------------------------------------*/
void    reset_15and7_bit (word, len)
ushort	word[];
int     len;
{
register int    i;   

   for (i=0; i<len; i++) 
   {
	word[i] &= RESET_15_AND_7_BIT;
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of set_15and7_bit()                            */
/*	PURPOSE :	Set MSB of the string buffer.				*/
/*------------------------------------------------------------------------------*/
void    set_15and7_bit (word, len)
ushort	word[];
int     len;
{
register int    i;              

   for (i=0; i<len; i++) 
   {
	word[i] |= SET_15_AND_7_BIT;
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of free_candidates()                           */
/*	PURPOSE : 	Deallocate the KIMED's candbuffer.			*/
/*------------------------------------------------------------------------------*/
void	free_candidates(kimed)
KIMED	*kimed;
{
register int	i;

   for(i=0; i<MAX_LIST; i++) 
   {
	free(kimed->candbuf[i]);
   }

   free(kimed->candbuf);
   free(kimed->cand_src);
}

/*------------------------------------------------------------------------------*/
/*                      Begining of makekey2rbn()                               */
/* 	PIRPOSE : Convert the code into a Key of RBN structure's code set.	*/
/*------------------------------------------------------------------------------*/
int	makekey2rbn(key, len)
uchar   *key;
int	len;
{
   if (len < 0 || ((len%2)!=0)) 
   {
	/***************/
	/* Invalid key */
	/***************/
	return (KP_ERR);
   } 
   else 
   {
	register int i=0;
	while ((i+2)<len) 
   	{
	   *(key+i) &= RESET_MAKEKEY2RBN;
	   i += 2;
	}
	*(key+i) |= SET_MAKEKEY2RBN;
  	return (KP_OK);
   }
}

/*------------------------------------------------------------------------------*/
/*                      Begining of makecand2rbn()                              */
/* 	PIRPOSE : Convert the code into a Candidate of RBN structure's code set.*/
/*------------------------------------------------------------------------------*/
int     makecand2rbn(cand, len)
uchar   *cand;
int     len;
{
   if (len < 0 || ((len%2)!=0)) 
   {
        /****************/
        /* Invalid cand */
        /****************/
        return (KP_ERR);
   } 
   else 
   {
	register int i=0;
	while ((i+2)<len) 
        {
	   *(cand+i) &= RESET_MAKECAND2RBN; i++;
	   *(cand+i) &= RESET_MAKECAND2RBN; i++;
	}
        *(cand+i+0) |= SET_MAKECAND2RBN;
        *(cand+i+1) |= SET_MAKECAND2RBN;
        return (KP_OK);
   }
}
