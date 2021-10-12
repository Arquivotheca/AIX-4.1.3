static char sccsid[] = "@(#)80	1.2  src/bos/usr/lib/nls/loc/imk/khhc/Hhcmru.c, libkr, bos411, 9428A410j 6/9/94 09:56:17";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhcmru.c
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
 *  Module:       Hhcmru.c
 *
 *  Description:  The routines of the MRU managements.
 *
 *  Functions:    init_MRU()
 *		  put_it_MRU()
 *		  free_MRU_list()
 *		  update_MRU()
 *		  getcstatus()
 *		  compare()
 *
 *  History:      5/20/90  Initial Creation.     
 * 
 ********************************************************************************/

/*------------------------------------------------------------------------------*/
/*			Include Header Files.					*/
/*------------------------------------------------------------------------------*/

#include	<sys/types.h>
#include        <stdio.h>
#include        <string.h>
#include	<fcntl.h>
#include        <im.h>
#include	"kedconst.h"
#include	"ked.h"
#include	"Hhcim.h"
#include        "Hhclist.h"

/*------------------------------------------------------------------------------*/
/* 				Defines Macro. 					*/
/*------------------------------------------------------------------------------*/

#define  ACTUAL_MRUSIZE	(MRUSIZE-4)

/*------------------------------------------------------------------------------*/
/* 			External Reference.        				*/
/*------------------------------------------------------------------------------*/

extern int  	memicmp();
extern int	compare();
extern int	kimstrcmp();
extern int	makekey2rbn();
extern int	makecand2rbn();
extern void     free();
extern void     set_15and7_bit();
extern char     *malloc();

/*------------------------------------------------------------------------------*/
/* 			Begining of init_MRU.					*/
/* Purpose: 		Initialize mru list. 					*/
/*------------------------------------------------------------------------------*/
int	init_MRU(kimed)
KIMED	*kimed;
{
ushort   mrubuff[(MRUSIZE/2)+10]; /* Local MRU bufer.				*/
register int i=2;       	  /* Search Start position. 			*/
register int bound;		  /* Bound of MRU buffer.			*/
register int len;		  /* A keydata length				*/
uchar    *key;			  /* A pointer of keydata.			*/
uchar    *cand;			  /* A pointer if candidate data.		*/
List	  p, q;			  /* A MRU Entry pointer.			*/

   /*********/
   /* 	    */
   /* Init. */
   /* 	    */
   /*********/
   list_init(&(kimed->mrulist));	

   if (kimed->udict.status == UDP_RDONLY) 
   {
	return (KP_OK);
   }
   else 
   {
	kimlock(kimed->udict.fdesc);
	lseek(kimed->udict.fdesc, 0L, 0);
	if (read(kimed->udict.fdesc, mrubuff, MRUSIZE) <= 0) 
	{
	   return (KP_ERR);
	}
	kimunlock(kimed->udict.fdesc);
	bound = MRUSIZE/2;
	while (i < bound) 
	{
	   if (IsBufEmpty(mrubuff[i])) 		/* check buffer empty. 		*/
	   {
		return (KP_OK);
	   }
	   len = next_key_len(mrubuff, bound, i); /* get a next keydata length. */
	   key = (uchar*)malloc((len*2)+1); 	  
	   memcpy(key, (char*)&mrubuff[i], len*2);	
	   set_15and7_bit(key, len);
	   *(key+(len*2)) = NULL;
	   i += len;
	   len = get_cand_len(mrubuff, bound, i); /* get a cnad length.		*/
	   cand = (uchar*)malloc((len*2)+1); 	  
	   memcpy(cand, (char*)&mrubuff[i], len*2);	
	   set_15and7_bit(cand, len);  /* Conver the string, in order to compare */
	   *(cand+(len*2)) = NULL;
	   i += len;
	   /**************************/
	   /*			     */
	   /* List Append Operation. */
	   /*			     */
	   /**************************/
	   if (alloc_node(&p, key, cand, UD_MRU_OLD) == KP_ERR) 
	   {
		return (KP_ERR);
	   }
	   if ((*(&kimed->mrulist)) != NULL) 
	   {
		NEXT(q) = p;
		q = p;
	   } 
	   else 
	   {
		(*(&kimed->mrulist)) = p;
		q = p;
	   }
	}   
        return (KP_OK);
    }
}

/*------------------------------------------------------------------------------*/
/* 			Begining of put_it_MRU					*/
/* 			Purpose: Put a entry into list. 			*/
/*------------------------------------------------------------------------------*/
int	put_it_MRU(kimed)
KIMED	*kimed;
{
register int	len;
List	curentry;
List	backentry;
List	modify_entry;
uchar	*key, *cand;
ushort	src;
extern  ushort getcandsrc();

	len = kimed->echosvchlen;
	key = (uchar*) malloc(len*2);
	memcpy(key, kimed->echosvch, len);
	*(key+len) = NULL;	

	len  = strlen(kimed->candbuf[kimed->candcrpos]);
	cand = (uchar*) malloc(len*2);
	memcpy(cand, kimed->candbuf[kimed->candcrpos], len);
	*(cand+len) = NULL;	

	/* (1) */
	/********************************************/
	/*					    */
	/* Inserts a entry at the front of the list.*/
	/*					    */
	/********************************************/
	if (list_insert(&(kimed->mrulist), (caddr_t)key, (caddr_t)cand) == KP_ERR)
	{
	  return (KP_ERR);
	}

	backentry = (*(&kimed->mrulist));
	modify_entry = (*(&kimed->mrulist));

	src = getcandsrc(kimed, cand);
        M_SOURCE(modify_entry) = src;

	curentry = NEXT(backentry);

	/* (2) */
	/********************************************/
	/*					    */
	/* Finds a same entry and then delete it.   */
	/*					    */
	/********************************************/
	while(curentry != NULL) 
	{
	  if (compare((caddr_t)key,  (caddr_t)KEY(curentry)) && 
	      compare((caddr_t)cand, (caddr_t)CAND(curentry))) 
	  {
	        /* There is the same entry. It is removed here.	*/
		M_STATUS(modify_entry) = UD_MRU_OLD;
		NEXT(backentry) = NEXT(curentry);
		free_node(&curentry);
		return (KP_OK);
	  } 
	  else 
 	  {
	 	backentry = curentry;	
		curentry = NEXT(curentry);
	  }
	}
	/*******************************/
	/*			       */
	/* There isn't the same entry. */
	/*			       */
	/*******************************/
	return (KP_OK);    
}

/*------------------------------------------------------------------------------*/
/* 			Begining of getcandsrc					*/
/*	PURPOSE : 	Get info, that where is this candidate come from.	*/ 
/*------------------------------------------------------------------------------*/
ushort getcandsrc(kimed, candidate)
KIMED	*kimed;
uchar	*candidate;
{
register int i=0; 
register int cand_len;

   cand_len = strlen(candidate);
   for(i=0; i < kimed->candsize; i++) 
   {
     if (strncmp(candidate, kimed->candbuf[i], cand_len) == 0) 
     { 
	return (ushort)(kimed->cand_src);
     }
   }
   return (ushort)(UD_FROM_NONE);
}

/*------------------------------------------------------------------------------*/
/* 			Begining of free_MRU_list				*/
/* 			Purpose: Deallocates mru list's nodes.			*/
/*------------------------------------------------------------------------------*/
int	free_MRU_list(kimed)
KIMED	*kimed;
{
List cur;
List next;

   cur = (*(&kimed->mrulist));
   while (cur != NULL) 
   {
	next = NEXT(cur);
	free_node(&cur);
	cur = next;
   }
   return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*			Begining of update_MRU.					*/
/* Purpose:     Updates MRU arear into IM ED's mru list entry.			*/
/*------------------------------------------------------------------------------*/
int	update_MRU(kimed)
KIMED	*kimed;
{
register int	i=0;
register int	klen;
register int	clen;
register int	acptr=4;
register int	j,
		bound;
uchar	 *key;
uchar	 *cand;
uchar    o_mrubuff[ACTUAL_MRUSIZE];
uchar    n_mrubuff[ACTUAL_MRUSIZE];
ushort   thereis;
ushort	 *short_buff;
ushort   mrulen=4; /* 4 = (U_STSLEN+U_MRULEN) */
List	 entry;

   if (kimed->udict.fdesc < 0) 
   {
	return (KP_ERR);
   }

   memset(o_mrubuff, 0xff, (int)(ACTUAL_MRUSIZE));
   memset(n_mrubuff, 0xff, (int)(ACTUAL_MRUSIZE));

   kimlockWR(kimed->udict.fdesc);

   /* 4 = (U_STSLEN+U_MRULEN) */
   lseek(kimed->udict.fdesc, (long)4, 0);
   if (read(kimed->udict.fdesc, o_mrubuff, ACTUAL_MRUSIZE) != ACTUAL_MRUSIZE) 
   {
	return (KP_ERR);
   }

   entry = (List) *(&kimed->mrulist);
   while (entry != NULL) 
   {
	key  = KEY(entry);  klen = strlen(key);
	cand = CAND(entry); clen = strlen(cand);
	/*--------------------------------------------------------*/
	/*							  */
	/* Converts the code, In order to store at User Dictionary*/
	/*							  */
	/*--------------------------------------------------------*/
	makekey2rbn(key, klen);
	makecand2rbn(cand, clen);
	
	getcstatus(&o_mrubuff[0], key, (ushort)klen, cand, (ushort)clen, &thereis);

	/*---------------------------------------------------------*/
	/*							   */
	/* Now, Dictionary Utility didedn't delete this candidate. */
	/* Thus, Save this entry in order to update MRU arear.	   */
	/*							   */
	/*---------------------------------------------------------*/
	if ( (M_STATUS(entry) == UD_MRU_OLD && thereis == ON) ||
	     (M_STATUS(entry) == UD_MRU_NEW && thereis == ON  && 
	     (M_SOURCE(entry) == UD_FROM_USR || M_SOURCE(entry) == UD_FROM_MRU)) || 
	     (M_STATUS(entry) == UD_MRU_NEW && M_SOURCE(entry) == UD_FROM_SYS) )
	{
	   acptr += (klen+clen);
	   /* Buffer overflow */
	   if (acptr > MRUSIZE) 
 	   {
	      break;
	   }
	   memcpy(n_mrubuff+i, key, klen);	
	   memcpy(n_mrubuff+i+klen, cand, clen);	
	   i += (klen+clen);
	   mrulen += (klen+clen);
	}
	entry = NEXT(entry);
   }

   /*****************************************/
   /*					    */
   /* Save o_mrubuff's a key and candidate. */
   /*					    */
   /*****************************************/
   j = 0;
   bound = (ACTUAL_MRUSIZE/2);
   short_buff = (ushort*)&o_mrubuff[0];
   while (j < bound)
   {
      if (IsBufEmpty(short_buff[j])) 
      {
         break;
      }
      klen = next_key_len(short_buff, bound, j);
      clen = get_cand_len(short_buff, bound, j+klen);
     (void)getcstatus(&n_mrubuff[0], &short_buff[j],
            (ushort)klen*2, &short_buff[j+klen], (ushort)clen*2, &thereis);
      if (thereis == OFF) 
      {
	  acptr += (klen+clen)*2;
          /* check buffer overflow */
          if (acptr > MRUSIZE)
          {
              break;
 	  }
          memcpy(n_mrubuff+i, &short_buff[j], klen*2);
          memcpy(n_mrubuff+i+(klen*2), &short_buff[j+klen], clen*2);
          i += (klen+clen)*2;
          mrulen += (klen+clen)*2;
      }
      j += (klen+clen);
   }

   lseek(kimed->udict.fdesc, (long)4L, 0);
   if (write(kimed->udict.fdesc, n_mrubuff, ACTUAL_MRUSIZE) 
	!= ACTUAL_MRUSIZE) 
   {
	return (KP_ERR);
   }
   lseek(kimed->udict.fdesc, (long)2L, 0);
   if (write(kimed->udict.fdesc, &mrulen, 2) != 2) 
   {
	return (KP_ERR);
   }

   kimunlock(kimed->udict.fdesc);

   return (KP_OK);
}

/*------------------------------------------------------------------------------*/
/*			Begining of getcstatus.					*/
/* Purpose:     Is there a key and candidate exist.				*/
/*------------------------------------------------------------------------------*/
getcstatus(buff1, key, keylen, cand, candlen, thereis)
uchar  *buff1;
uchar  *key;
ushort keylen;
uchar  *cand;
ushort candlen;
ushort *thereis;
{
ushort *buff2;
short  i=0;
short  bound=(ACTUAL_MRUSIZE/2);
short  l_keylen, l_candlen;

   *thereis = OFF; 
   buff2 = (ushort*)buff1;   

   while (i < bound) 
   {
	if (IsBufEmpty(buff2[i])) 
	{
	   break; 
	}
        l_keylen = (short)next_key_len(&buff2[0], bound, i);
	if (l_keylen <=0) 
	{
	  break;
	}
	l_candlen = (short)get_cand_len(&buff2[0], bound, i+l_keylen);
	if (l_candlen <= 0) 
	{
	  break;
	}
	if ((kimstrcmp(key, keylen/2, &buff2[i], l_keylen) == 0) && 
	    (kimstrcmp(cand, candlen/2, &buff2[i+l_keylen], l_candlen) == 0)) 
	{
	   *thereis = ON;
	   break;
	}
	i += (l_keylen+l_candlen);
   }
}

/*------------------------------------------------------------------------------*/
/*			Begining of update_MRU.					*/
/* Purpose:     	Compares between two string. 				*/
/*------------------------------------------------------------------------------*/
int	compare(str1, str2)
caddr_t	str1; 
caddr_t	str2; 
{
	if (kimstrcmp(str1, strlen(str1)/2, str2, strlen(str2)/2) == 0)
  	{
	   return (TRUE);
	}
	else 
	{
    	   return (FALSE);
	}
}
