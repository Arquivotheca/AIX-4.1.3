static char sccsid[] = "@(#)97	1.1  src/bos/usr/lpp/kls/dictutil/hudeixsc.c, cmdkr, bos411, 9428A410j 5/25/92 14:42:07";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudeixsc.c
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
 *  Module:       hudeixsc.c
 *
 *  Description:  User Dictionary Index Area Search
 *
 *  Functions:    hudeixsc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*              	Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* System memory operation uty.                 */
#include "hut.h"        /* Utility Define File.   */

/*----------------------------------------------------------------------*/
/*              	Begining of hudeixsc.				*/
/*		Finds our key position of Index Block.			*/
/*----------------------------------------------------------------------*/
void  hudeixsc ( dicindex, key, keylen, indxpos, indxpos1 )
uchar   *dicindex;      /* Pointer to Index Data Area                   */
uchar   *key;           /* Pointer to Key  Data                         */
short   keylen;         /* Length of Key  Data                          */
short   *indxpos;       /* Index Position.                              */
short   *indxpos1;      /* Index Position 1.                            */
{
  int    rc;             
  short i_len,		/* Active index block size 		 	*/
	 length,	/* Comparision Length 				*/
	 lst_indxpos,	
	 lst_indxpos1,	
	 i_keylen;	/* Key length of index block 			*/
  uchar	 l_keydata [U_KEY_MX]; 
	  
  /**************/
  /*		*/
  /* Local Copy */
  /*		*/
  /**************/
   memcpy(l_keydata, key, (int)keylen);
   mkrbnk(l_keydata, keylen);

   i_len = getil(dicindex);
   lst_indxpos = lst_indxpos1 = *indxpos = *indxpos1 = (U_ILLEN + U_HARLEN + U_NARLEN);

   while ( 1 ) {
	/***************************/
	/*			   */
	/* Gets a next key length. */
	/*			   */
	/***************************/

	i_keylen = nxtkeylen(dicindex, *indxpos, U_UIX_A); 
        length = ( keylen < i_keylen ) ? keylen : i_keylen;
        rc = memcmp ( l_keydata, (char *)(dicindex + *indxpos), length );
        if ( ( rc == 0 ) && ( keylen <= length ) )  break;
        if ( rc < 0 ) 
	{
	   *indxpos = lst_indxpos;
	   *indxpos1 = lst_indxpos1;
	   break;
	}
	/*****************************/
	/*			     */
	/* There isn't our key data. */
	/*			     */
	/*****************************/

        if ( ( *indxpos + i_keylen + U_RRNLEN ) >= i_len ) {
            break;
        }
	lst_indxpos = *indxpos;
 	lst_indxpos1 = *indxpos1;
	*indxpos1  = *indxpos;         
        *indxpos  += (i_keylen + U_RRNLEN);
   }
   return;
}
/*----------------------------------------------------------------------*/
/*              	End of hudeixsc.				*/
/*----------------------------------------------------------------------*/
