static char sccsid[] = "@(#)94  1.1  src/bos/usr/lpp/kls/dictutil/hudcwrit.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:38";
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudcwrit.c
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
 *  Module:       hudcwrit.c
 *
 *  Description:  Write User Dictionary into Buffer.
 *
 *  Functions:    hudcwrit()
 *
 *  History:      5/22/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/
#include <stdio.h>
#include "hut.h"   				/* Utility Define File  */ 

/*----------------------------------------------------------------------*/
/*                      Begining of hudcwrit.                           */
/*----------------------------------------------------------------------*/
void    hudcwrit ( cbptr, id, rrn )
UDCB    *cbptr;    /* Pointer to Control Block Buffer                 */
short   id;        /* Dictionary Identify                             */
short   rrn;       /* Read Relative Record Number                     */
{
  switch ((int)id)
    {
       /**********************************************/
       /*                                            */
       /* System Dictionary Index   ( Identify = 1 ) */
       /*                                            */
       /**********************************************/
       case  1:  
		 memcpy(cbptr->dcptr, cbptr->wtptr, (int)U_SIX_A);
                 break;

       /**********************************************/
       /*                                            */
       /* System Dictionary Data    ( Identify = 2 ) */
       /*                                            */
       /**********************************************/
       case  2:  
	  	 memcpy(cbptr->dcptr + (rrn * U_REC_L),
		        cbptr->wtptr, U_REC_L);
                 break;

       /**********************************************/
       /*                                            */
       /* User Dictionary Index     ( Identify = 3 ) */
       /*                                            */
       /**********************************************/
       case  3:  
		 memcpy(cbptr->dcptr + U_MRU_A, cbptr->wtptr, U_UIX_A);
                 break;

       /**********************************************/
       /*                                            */
       /* User Dictionary Data      ( Identify = 4 ) */
       /*                                            */
       /**********************************************/
       case  4:  
		 memcpy(cbptr->dcptr+(rrn*U_REC_L), cbptr->wtptr, U_REC_L);
                 break;
    }
  return;
}
/*----------------------------------------------------------------------*/
/*                      End of hudcwrit.                                */
/*----------------------------------------------------------------------*/
