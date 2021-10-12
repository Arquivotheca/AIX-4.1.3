static char sccsid[] = "@(#)92  1.1  src/bos/usr/lpp/kls/dictutil/hudcread.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:18";
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudcread.c
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
 *  Module:       hudcread.c
 *
 *  Description:  Reads User Dictionary 
 *
 *  Functions:    hudcread()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*              	Include Header.					*/
/*----------------------------------------------------------------------*/

#include "hut.h"          /* Utility Define File   */               

/*----------------------------------------------------------------------*/
/*              	Begining of hudcread.				*/
/*----------------------------------------------------------------------*/
void    hudcread ( cbptr, id, rrn )
UDCB          *cbptr;     /* Pointer to control block buffer */
short          id;        /* Dictionary Identify             */
short          rrn;       /* Read Relative Record Number     */
{
  switch (id) {
       /***********************************************/
       /*                                             */
       /* System Dictionary Index    ( Identify = 1 ) */
       /*                                             */
       /***********************************************/
       case  1:  cbptr->rdptr = cbptr->dcptr;
                 break;

       /***********************************************/
       /*                                             */
       /* System Dictionary Data     ( Identify = 2 ) */
       /*                                             */
       /***********************************************/
       case  2:  cbptr->rdptr = cbptr->dcptr +( rrn * U_REC_L );
                 break;

       /***********************************************/
       /*                                             */
       /* User Dictionary Index      ( Identify = 3 ) */
       /*                                             */
       /***********************************************/
       case  3:  cbptr->rdptr = cbptr->dcptr + U_MRU_A;
                 break;

       /***********************************************/
       /*                                             */
       /* User Dictionary Data       ( Identify = 4 ) */
       /*                                             */
       /***********************************************/
       case  4:  cbptr->rdptr = cbptr->dcptr + ( rrn * U_REC_L );
                 break;
  }
  return;
}
/*----------------------------------------------------------------------*/
/*              	End of hudcread.				*/
/*----------------------------------------------------------------------*/
