static char sccsid[] = "@(#)76  1.1  src/bos/usr/lib/nls/loc/imk/khhc/Hhcgetclst.c, libkr, bos411, 9428A410j 5/25/92 15:43:35";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		Hhcgetclst.c
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
 *  Module:       Hhcgetclst.c
 *
 *  Description:  Gets the candidates of a key at MRU, Data Block 
 *		  and system dictionaty.
 *
 *  Functions:    get_candidates()
 *		  get_from_MRU()
 *		  get_from_dict()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ******************************************************************/

/*------------------------------------------------------------------------------*/
/*                      Includes Header Files.                                  */
/*------------------------------------------------------------------------------*/

#include 	<sys/types.h>
#include 	<stdio.h>
#include 	<fcntl.h>
#include	<string.h>
#include        <im.h>
#include	"kedconst.h"
#include        "ked.h"
#include	"Hhcim.h"
#include	"Hhclist.h"

/*------------------------------------------------------------------------------*/
/*                      External reference                                      */
/*------------------------------------------------------------------------------*/

extern int	find_rbn(); 
extern int	find_cand_block();
extern int	get_from_MRU();
extern int	get_from_dict();
extern int	kimstrcmp();
extern void	free();
extern void 	reset_15and7_bit();
extern uchar 	*make_cand();
extern int	list_empty();

/*------------------------------------------------------------------------------*/
/*                      Begining of get_candidates()                            */
/*	PURPOSE : 	Get the candidates of the key.				*/
/*------------------------------------------------------------------------------*/
int	get_candidates(kimed)
KIMED	*kimed;
{
int	        lcount=0;
register int	cmpsz=0;
uchar	*cand;

	/************************************/
	/*				    */
	/* Candidate Search Order. 	    */
	/* (1) MRU Arear	   	    */
	/* (2) User Dictionary Data Block   */
	/* (3) System Dictionary Data Block */
	/*				    */
	/************************************/

	/*********/
	/*	 */
	/* Init. */
	/*	 */
	/*********/
	kimed->candbuf  = (uchar**) calloc((MAX_LIST), sizeof(caddr_t));
	kimed->cand_src = (ushort*) calloc((MAX_LIST), sizeof(ushort));
	memset(kimed->cand_src, UD_FROM_NONE, MAX_LIST);

	/* (1) */
	if (kimed->learn == ON) 
	{
	  cmpsz = get_from_MRU(kimed, &lcount);
	}

	/* (2) */
	get_from_dict(kimed, &lcount, cmpsz, USRDICT);

	/* (3) */
	get_from_dict(kimed, &lcount, cmpsz, SYSDICT);

	cand = (uchar*)malloc(kimed->echosvchlen+1) ;
	memcpy(cand, kimed->echosvch, kimed->echosvchlen+1) ;
	kimed->candbuf[lcount++] = cand ;

	kimed->candbuf[lcount] = NULL;
	kimed->candsize = lcount;

	kimed->candcrpos = 0;	/* Init first of the key's candidates. */ 
	return (lcount);	
}

/*------------------------------------------------------------------------------*/
/*                      Begining of get_from_MRU()                              */
/*	PURPOSE : 	Get candidates from MRU area.				*/
/*------------------------------------------------------------------------------*/
int	get_from_MRU(kimed, lcount)
KIMED 	*kimed;
int	*lcount;
{
register int 	len;
register int 	inc=*lcount;
register int	incsz=*lcount;
List     	entry;
int  		klen;
unsigned char   key[MAXEUMLEN];  

        if (!list_empty(*(&kimed->mrulist))) 
	{
	   klen = kimed->echosvchlen;
           memcpy (key, kimed->echosvch, klen);
	   /****************************************************/
	   /*						       */
	   /* Finds same key of entry and insert into candbuf. */
	   /*						       */
	   /****************************************************/
           entry = (*(&kimed->mrulist));
           while(entry != NULL) 
	   {
                if (kimstrcmp(key, klen/2, KEY(entry), strlen(KEY(entry))/2) == 0) 
		{
                   len = strlen(CAND(entry));
		   /*******************************************/
		   /*					      */
		   /* Uses this flag, in order to update MRU. */
		   /*					      */
		   /*******************************************/
		   kimed->cand_src[inc] |= UD_FROM_MRU;
                   kimed->candbuf[inc]   = (uchar*)malloc(len*2);
                   memcpy(kimed->candbuf[inc], CAND(entry), len);
                   *(kimed->candbuf[inc++]+len) = NULL;
                }
                entry = NEXT(entry);
           }
	   /************************/
 	   /*			   */
	   /* return compare size. */
 	   /*			   */
	   /************************/
	   *lcount = inc;
	   incsz = inc-incsz;
	   return (incsz);
        } 
	else 
	{
	   /************************/
 	   /*			   */
	   /* return compare size. */
 	   /*			   */
	   /************************/
	   return 0;
	}
}

/*------------------------------------------------------------------------------*/
/*                      Begining of get_from_dict()                             */
/*	PURPOSE: 	Get candidates from User and System Dictionary.		*/
/*------------------------------------------------------------------------------*/
int	get_from_dict(kimed, lcount, cmpsz, filetype)
KIMED	*kimed;
int	*lcount;
int	cmpsz;
int	filetype;
{
register int	klen;
register int	inc=*lcount;
register int 	incsz=*lcount;
ulong	offset;
uchar   key[MAXSTR]; 
ushort		*ixblock, *dblock;
int		cbix;
int		n;
int		fd;
int		rbn;
int		ixblocksize;
int		dblocksize;
int  		bound;
int  		len;
uchar    	*cand;
register int 	i;
register int 	j;
register int 	thereisno;

	if (filetype == SYSDICT) 
 	{
	   fd          = kimed->sdict.fdesc;
	   ixblocksize = kimed->sdict.ixsz;
	   dblocksize  = kimed->sdict.dbunit; 
	   ixblock     = kimed->sdict.ixb;
	   dblock      = (ushort*) malloc(dblocksize+10);
	} 
	else 
	{
	
	   if (kimed->udict.fdstat == INVALID_FDESC) 
           {	/* IM don't have User Dictionary */
		return (0);
	   }
	   fd          = kimed->udict.fdesc;
	   dblocksize  = kimed->udict.dbunit; 
	   ixblock = get_ix_block(fd, USRDICT, &ixblocksize);
           dblock  = (ushort*) malloc(dblocksize+10);
	} 
	klen = kimed->echosvchlen;
        memcpy (key, kimed->echosvch, klen);
	/* Convert the string, in order to compare */
        reset_15and7_bit(key, klen/2);

	if (ixblock != NULL) 
	{
           rbn = find_rbn(ixblock, ixblocksize/2, key, klen/2, filetype);
	} 
	else 
	{
	   return (KP_ERR);
	}
        if (rbn <= 0)  
	{
		(void)free(dblock);
		return (KP_ERR);
	}
	offset = (rbn*1024);
	kimlock(fd);
	lseek(fd, offset, 0);
        n=read (fd, dblock, 1024);
	kimunlock(fd);
        if (n <= 0) 
	{
		(void)free(dblock);
		return (KP_ERR);
        }
        cbix = find_cand_block(dblock, 1024/2, key, klen/2, filetype);
        if (cbix > 0) 
	{
	   bound = dblocksize/2;   
	   i=cbix;
		while (i < bound) 
	        {
		  thereisno = TRUE;
		  if (IsBufEmpty(dblock[i])) break;
		  cand = make_cand(dblock, bound, i, &len);
		  if (kimed->learn == ON && cand != NULL) 
		  {
		     /*********************************/
		     /* Finds there is aleady a cand. */
		     /*********************************/
		     for(j=0; j < cmpsz; j++) 
		     {
			  if (kimstrcmp(cand, len, kimed->candbuf[j], 
			      strlen(kimed->candbuf[j])/2) == 0) 
			  {
			      /************************************/
			      /*				  */
			      /* Aleady exist, don't need to copy */
			      /*				  */
			      /************************************/
			      thereisno = FALSE;
			      break;
			  }
		     }
		  }
		  if (thereisno == TRUE && cand != NULL) 
		  {
		     /*******************************************/
		     /*					        */
		     /* Uses this flag, in order to update MRU. */
		     /*					        */
		     /*******************************************/
		     if (filetype == USRDICT) 
		     {
			kimed->cand_src[inc] |= UD_FROM_USR;
		     }
		     else 
		     {
			kimed->cand_src[inc] |= UD_FROM_SYS;
		     }
		     kimed->candbuf[inc++] = cand; 
		  }
		  if (IsLastCCOfKey(dblock[i+len-1])) break;
		  i += len;
		} /* while */

		if (filetype == USRDICT) 
		{
		   (void)free(ixblock);
		}
		(void)free(dblock);
		*lcount = inc;
		incsz = inc-incsz;
		return (incsz);
	}
        else 
	{
	   /****************************************/
	   /* There isn't proper candidates block. */
	   /****************************************/
	   if (filetype == USRDICT) 
	   {
	      (void)free(ixblock);
	   }
	   (void)free(dblock);
	   return (KP_ERR);
        }
}
