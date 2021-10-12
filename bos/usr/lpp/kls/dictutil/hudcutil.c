static char sccsid[] = "@(#)93	1.1  src/bos/usr/lpp/kls/dictutil/hudcutil.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:28";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudcutil.c
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
 *  Module:       hudcutil.c
 *
 *  Description:  Conveience functions of User Dictionary.
 *
 *  Functions:    getudstat()
 *                setudstat()
 *                getmrulen()
 *                setmrulen()
 *		  getil()
 *		  setil()
 *		  gethar()
 *		  sethar()
 *	 	  getnar()
 * 		  setnar()
 *		  getrl()
 *		  setrl()
 *		  getrrn()
 *		  getlastrrn()
 *		  setrrn()
 *                nxtkeylen()
 *                nxtcandlen()
 *		  nxtkoffset()
 *		  hudstrcmp()
 *                mkrbnk()
 *		  mkrbnc()
 *                mkrbnlastc()
 *		  makeksstr()
 *		  findrrn()
 *		  findcblock()
 *
 *  History:      5/22/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include "hut.h"  				/* Utility Define File  */

/*----------------------------------------------------------------------*/
/*                      Begining of getudstat.                          */
/* 	Purpose: Get the dictionary status from the file.	     	*/
/*----------------------------------------------------------------------*/
short getudstat(dictbuff, status)
uchar  *dictbuff;
short *status;
{
ushort *buff;

   buff = (ushort*)dictbuff;
   *status = (short)(buff[U_STSPOS]);
   if ( *status == 0x0000 ||  /* Normal Close                  */
	*status == 0x000f ||  /* Opened for Read Only  	      */
	*status == 0x00f0 ||  /* Opened for Read Write 	      */
	*status == 0x00ff )   /* Opened for Dictionary Utility */
	return (U_SUCC);
   else
	/* Invalid a dictionary status */
	return (U_FAIL);
}

/*----------------------------------------------------------------------*/
/*                      Begining of setudstat.                          */
/* 	Purpose: Set the dictionary status to the file. 		*/
/*----------------------------------------------------------------------*/
short setudstat(dictbuff, status)
uchar *dictbuff;
short status;
{
ushort *buff;

   buff = (ushort*)dictbuff;
   if ( status == 0x0000 ||  /* Normal Close                  */
        status == 0x000f ||  /* Opened for Read Only         */
        status == 0x00f0 ||  /* Opened for Read Write        */
        status == 0x00ff )   /* Opened for Dictionary Utility */ {
	memcpy(buff+U_STSPOS, &status, U_STSLEN);
        return (U_SUCC);
   } else
	/* Invalid a dictionary status */
        return (U_FAIL);
}

/*----------------------------------------------------------------------*/
/*                      Begining of getrl.                              */
/*	Purpose: Get a active data block length				*/
/*----------------------------------------------------------------------*/
short getrl(databuff)
uchar  *databuff;
{
ushort *buff;
   buff = (ushort*) databuff;
   return (short) (buff[U_RLPOS]);
}	


/*----------------------------------------------------------------------*/
/*                      Begining of setrl.                              */
/* 	Purpose:  	 Set a active data block length			*/
/*----------------------------------------------------------------------*/
short setrl(databuff, rl)
uchar  *databuff;
short rl;
{
ushort *buff;
   buff = (ushort*) databuff;
   memcpy(buff+U_RLPOS, &rl, U_RLLEN);
   return (U_SUCC);
}	

/*----------------------------------------------------------------------*/
/*                      Begining of getlastrrn.                         */
/*	Purpose:	Gets a last rrn number				*/
/*----------------------------------------------------------------------*/
short getlastrrn (dicindex, limit)
uchar	*dicindex;
short   limit;
{
ushort		*buff;
register int	i=(U_ILLEN+U_HARLEN+U_NARLEN)/2;
register int	bound=(limit/2);

   if (limit == (U_ILLEN+U_HARLEN+U_NARLEN))
	return (U_NARMIN-1);
   buff = (ushort*)dicindex;
   while (i < bound && buff[i] != 0xffff) i++;
   return (short) buff[i-1];
}

/*----------------------------------------------------------------------*/
/*                      Begining of nxtkeylen.                          */
/*	Purpose:	Gets a next key length				*/
/*----------------------------------------------------------------------*/
short	nxtkeylen(databuf, start, limit)
uchar	*databuf;
short	start;
short	limit;
{
ushort		*buff;
register int	i=(start/2);
register int	bound=(limit/2);
register int	len=0;

   buff = (ushort*)databuf;
   if (buff[i] == 0xffff) {
	return (-1);
   }
   while (i < bound) {
	len++;
	if (IsLastCharOfKey(buff[i])) {
	   /*********************/
	   /* Finds a last char */
	   /* of  key           */
	   /*********************/
	   return (len*2);
	}
	i++;
   }
   /***********************/ 
   /* 			  */
   /* Cannot find the key */
   /* 			  */
   /***********************/ 
   return (-1);  
}

/*----------------------------------------------------------------------*/
/*                      Begining of getil.                              */
/*	Purpose: Get the active index block data length.		*/
/*----------------------------------------------------------------------*/
short getil(uindexbuff)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	return (short)(buff[U_ILPOS]);
}

/*----------------------------------------------------------------------*/
/*                      Begining of setil.                              */
/*	Purpose: Set the active index block data length.		*/
/*----------------------------------------------------------------------*/
void setil(uindexbuff, il)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
short il;
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	memcpy(buff+U_ILPOS, &il, U_ILLEN);
}

/*----------------------------------------------------------------------*/
/*                      Begining of gethar.                             */
/* 	Purpose:  Get the higest allocated  record block		*/
/*----------------------------------------------------------------------*/
short gethar(uindexbuff)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	return (short)(buff[U_HARPOS]);
}

/*----------------------------------------------------------------------*/
/*                      Begining of sethar.                             */
/* 	Purpose: Set the higest allocated record block number.		*/
/*----------------------------------------------------------------------*/
void  sethar(uindexbuff, har)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
short har;
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	memcpy(buff+U_HARPOS, &har, U_HARLEN);
}

/*----------------------------------------------------------------------*/
/*                      Begining of getnar.                             */
/*	Purpose: Get the next allocatable record block.			*/
/*----------------------------------------------------------------------*/
short getnar(uindexbuff)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	return (short)(buff[U_NARPOS]);
}

/*----------------------------------------------------------------------*/
/*                      Begining of setnar.                             */
/*	Purpose: Set the next allocatable record block.			*/
/*----------------------------------------------------------------------*/
void setnar(uindexbuff, nar)
uchar  *uindexbuff;	/* User Dictionary Index Buffer Pointer */
short nar;
{
ushort *buff;
	buff = (ushort *)(uindexbuff);
	memcpy(buff+U_NARPOS, &nar, U_NARLEN);
}

/*----------------------------------------------------------------------*/
/*                      Begining of getrrn.                             */
/*	Purpose: Get a Relative Record(Block) Number of dictionary	*/
/*----------------------------------------------------------------------*/
void   getrrn(buff, rrn)
uchar *buff; /* Pointer of the buffer start */
short *rrn;
{
	memcpy(rrn, buff, U_RRNLEN);
}
	
/*----------------------------------------------------------------------*/
/*                      Begining of setrrn.                             */
/* 	Purpose: Set a Relative Record(Block) Number of dictionary	*/
/*----------------------------------------------------------------------*/
void setrrn(buff, rrn)
uchar *buff; /* Pointer of the buffer start */
short rrn;
{
	memcpy(buff, &rrn, U_RRNLEN);
}

/*----------------------------------------------------------------------*/
/*                      Begining of setmrulen.                          */
/* 	Purpose:  	Set Mru data block length			*/
/*----------------------------------------------------------------------*/
void setmrulen(mrubuff, mrulen)
uchar  *mrubuff;	/* User Dictionary Buffer Pointer */
short  mrulen;
{
ushort *buff;
	buff = (ushort*)mrubuff;
	memcpy(buff+U_MRUPOS, (uchar*)&mrulen, U_MRULEN);
}


/*----------------------------------------------------------------------*/
/*                      Begining of getmrulen.                          */
/*	Purpose: 	Get mru length of MRU area			*/
/*----------------------------------------------------------------------*/
short getmrulen(mrubuff)
uchar  *mrubuff;	/* User Dictionary Buffer Pointer */
{
ushort *buff;
	buff = (ushort *)(mrubuff);
	return (short)(buff[U_MRUPOS]);
} 		       


/*----------------------------------------------------------------------*/
/*                      Begining of nxtkoffset.                         */
/*	Purpose:	Get next key offset from a buffer		*/ 
/*----------------------------------------------------------------------*/
short	nxtkoffset(databuf, start, limit)
uchar	*databuf;
short	start;
short	limit;
{
ushort		*buff;
register int	i=(start/2);
register int	bound=(limit/2);
register int	len=0;

   buff = (ushort*) databuf;
   if (buff[i] == 0xffff) {
	return (-1);
   }
   while(i < bound) {
	len++;
	if (IsLastCCOfKey(buff[i])) {
	   /*********************/
	   /* Finds a last char */
	   /* of a last cand.   */
	   /*********************/
	   return (len*2);
	}
	i++;
   }
   /****************************/ 
   /* 			       */
   /* Cannot find the next key */
   /* 			       */
   /****************************/ 
   return (-1);
}

/*----------------------------------------------------------------------*/
/*                      Begining of hudstrcmp.                          */
/*----------------------------------------------------------------------*/
short	hudstrcmp(str1, len1, str2, len2)
short  len1, len2;
uchar   *str1, *str2;
{
ushort	 *str1buf, *str2buf;
register int    i=0; 
register int	j=0; 
short    ret=-1;

   len1 /= 2;
   len2 /= 2;
   str1buf  = (ushort*)str1;
   str2buf  = (ushort*)str2;
   if (len1 == len2) {
      while (i < len1 && (ret=CMP(str1buf[i],str2buf[j])) == 0) 
	 {i++; j++;}
         return (short)(ret == 0) ? 0 : (ret > 0) ? 1: -1;
   } else if (len1 > len2) {
      while (j < len2 && (ret=CMP(str1buf[i],str2buf[j])) == 0) 
	 {i++; j++;}
         return (short)(ret == 0) ? 1: (ret > 0) ? 1: -1;
   } else {
      while (i < len1 && (ret=CMP(str1buf[i],str2buf[j])) == 0) 
	 {i++; j++;}
         return (short)(ret == 0) ? -1: (ret > 0) ? 1: -1;
   }
}

/*----------------------------------------------------------------------*/
/*                      Begining of nxtcandlen.                         */
/*	Purpose: 	 Get a candidate string length.			*/
/*----------------------------------------------------------------------*/
short	nxtcandlen(databuf, start, lastcflag, limit)
uchar	*databuf;
short	start;
short	*lastcflag;
short	limit;
{
ushort	 *buff;
register int	i=(start/2);
register int	bound=(limit/2);
register int	len=0;

    buff = (ushort*)databuf;
    *lastcflag = U_FOF;
    if (buff[i] == 0xffff) {
	return (-1);
    }
    while(i < bound) {
	len++;
	if (IsLastCharOfCand(buff[i])) {
	   /*********************/
	   /* Finds a last char */
	   /* of a cand.        */
	   /*********************/
	   if (IsLastCCOfKey(buff[i])) {
		*lastcflag = U_FON;
	   }
	   return (len*2);
	}
	i++;
    }
   /************************/ 
   /* 			   */
   /* Cannot find the cand */
   /* 			   */
   /************************/ 
   return (-1);  
}

/*----------------------------------------------------------------------*/
/*                      Begining of mkrbnk.                             */
/* Purpose: Makes a key of the RBN file structure. Len must be even here*/
/*----------------------------------------------------------------------*/
short  mkrbnk(keybuf, buflen)
uchar   *keybuf;
short	buflen;
{
register int i;

   i=0;
   if (buflen < 0 || ((buflen%2)!=0)) {
	/* Invalid key buffer */
	return (U_FAIL);
   } else {
	while ((i+2)<buflen) {
	   *(keybuf+i) &= 0x7f;
	   i += 2;
	}
	*(keybuf+i) |= 0x80;
  	return (U_SUCC);
   }
}

/*----------------------------------------------------------------------*/
/*                      Begining of mkrbnlastc.                         */
/* Purpose: Makes a cand of the RBN file structure.  Len must be even   */
/*----------------------------------------------------------------------*/
short  mkrbnlastc(candbuf, buflen)
uchar   *candbuf;
short  buflen;
{
register int i=0;
   
   if (buflen < 0 || ((buflen%2)!=0)) {
        /* Invalid candidate buffer */
        return (U_FAIL);
   } else {
	while ((i+2)<buflen) {
	   *(candbuf+i) &= 0x7f; i++;
	   *(candbuf+i) &= 0x7f; i++;
	}
        *(candbuf+i+0) |= 0x80; /* Means Last Candidate */
        *(candbuf+i+1) |= 0x80; /* Means Last Char of a Candidats */
        return (U_SUCC);
   }
}

/*----------------------------------------------------------------------*/
/*                      Begining of mkrbnc.                             */
/* 	Purpose: 	 Make a normal candidate			*/
/*----------------------------------------------------------------------*/
short mkrbnc(candbuf, buflen)
uchar *candbuf;
short buflen;
{
register int i=0;
   
   if (buflen < 0 || ((buflen%2)!=0)) {
        /* Invalid candidate buffer */
        return (U_FAIL);
   } else {
	while ((i+2)<buflen) {
	   *(candbuf+i) &= 0x7f; i++;
	   *(candbuf+i) &= 0x7f; i++;
	}
        *(candbuf+i+0) &= 0x7f; 
        *(candbuf+i+1) |= 0x80; /* Means Last Char of a Candidats */
        return (U_SUCC);
   }
}

/*----------------------------------------------------------------------*/
/*                      Begining of makeksstr.                          */
/* Purpose: Makes a KS Code set string of rbn structure's code		*/
/*----------------------------------------------------------------------*/
short makeksstr(code_buf, buflen)
uchar  *code_buf;
short buflen;
{
register int i=0;

   if (buflen < 0 || ((buflen%2)!=0)) {
        /* Invalid code buffer */
        return (U_FAIL);
   } else {
	while ((i+2)<buflen) {
	   *(code_buf+i) |= 0x80; i++;
	   *(code_buf+i) |= 0x80; i++;
	}
        *(code_buf+i+0) |= 0x80;
        *(code_buf+i+1) |= 0x80;
        return (U_SUCC);
   }
}

/*----------------------------------------------------------------------*/
/*                      Begining of findcblock.                         */
/* Purpose:	Finds candidates block  in data block.			*/
/*----------------------------------------------------------------------*/
/* 
	(Routine Description):
	This routine uses internally unsigned short buffer.
	It has a advantage. Do you notify it ? 
	It returns the index of the candidates block, it is
	char buffer's.   
*/	
short  findcblock(databuf, keydata, keylen, dicttype)
uchar	*databuf;
uchar   *keydata;
short  keylen;
short  dicttype;
{	
ushort  *buff;	/* Local Buffer */
uchar	l_keydata[U_KEY_MX];
short  d_cbsz, d_keylen, result, bound;	
register int i; 
   
   /***************************/
   /* Initialize this routine */
   /***************************/
   buff = (ushort*) databuf;
   if (dicttype == USRDICT) {
	i = (U_RLLEN/2);
	bound = getrl(databuf)/2;
   } else {
	bound = (U_REC_L/2);
	i = 0;
   }
   /*******************************/
   /* Local Copy                  */
   /* In order to compare with    */
   /* data block, convert keydata */
   /*******************************/
   memcpy(l_keydata, keydata, keylen);
/*****
   mkrbnk(l_keydata, keylen);
******/
   makeksstr(l_keydata, keylen);

   while (i < bound) {
	if (IsBufEmpty(buff[i])) {
	   return (U_FAIL);
	}
	d_keylen = nxtkeylen(databuf, (i*2), bound*2);
	if (d_keylen <= 0) return (U_FAIL);
	/* in order to compare */
	makeksstr(&databuf[i*2], d_keylen);
	result = hudstrcmp(&l_keydata[0], keylen, &databuf[i*2], d_keylen);
	/* restore the code */
	mkrbnk(&databuf[i*2], d_keylen);
	i += (d_keylen/2);
	if (result == 0) return (i*2);
	else if (result == 1) {
	   d_cbsz = nxtkoffset(databuf, (i*2), bound*2);
	   if (d_cbsz < 0) return (U_FAIL);
	   i += (d_cbsz/2);
	} else 
 	   return (U_FAIL);
   }
   return (U_FAIL);
} 

/*----------------------------------------------------------------------*/
/*                      Begining of findrrn.                            */
/*	Purpose:	Finds RRN in Index block.			*/
/*----------------------------------------------------------------------*/
short  findrrn(indexbuff, keydata, keylen, dicttype)
uchar  *indexbuff;
uchar  *keydata;
short keylen;
short dicttype;
{
register int i;
short   p_rrn_setted=FALSE, 
	 i_keylen,
	 result;
ushort   *buff;  /* Local Ushort buffer */
short   bound;
uchar    l_keydata [U_KEY_MX];

   /* Init. */
   buff = (ushort*)indexbuff;
   if (dicttype == USRDICT) {
	/* Init. Start position & boundary */
	i = ((U_ILLEN+U_HARLEN+U_NARLEN)/2);
	bound = (getil(indexbuff)/2);
   } else {
	/* Init. Start position & boundary */
	i = 0;
	bound = (2048/2);
   }
   memcpy(l_keydata, keydata, keylen);
/*****
   mkrbnk(l_keydata, keylen);
******/
   makeksstr(l_keydata, keylen);

   while (i < bound) {
	if (IsBufEmpty(buff[i])) {
	   if (p_rrn_setted)
               return (short)buff[i-1];
	   else 
	       return (U_FAIL);
	}
        i_keylen = nxtkeylen(indexbuff, (i*2), bound*2);
	if (i_keylen <= 0) return (U_FAIL);
	/* in order to compare */
	makeksstr(&indexbuff[i*2], i_keylen);
	result = hudstrcmp (&l_keydata[0], keylen, &indexbuff[i*2], i_keylen);
	/* restore */
	mkrbnk(&indexbuff[i*2], i_keylen);
	if (result == 0) {
	   return (short)buff[i+(i_keylen/2)];
	} else if (result == 1) {
	   p_rrn_setted = TRUE;
	   i += ((i_keylen/2)+1);	
	} else {
	   if (p_rrn_setted) return (short)buff[i-1];
	   else return (U_FAIL);
	}
   }	
   return (U_FAIL);	
}
