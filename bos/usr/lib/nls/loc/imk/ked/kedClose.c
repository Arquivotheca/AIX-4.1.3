static char sccsid[] = "@(#)67	1.3  src/bos/usr/lib/nls/loc/imk/ked/kedClose.c, libkr, bos411, 9428A410j 9/16/92 13:42:31";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kedClose.c
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

/******************************************************************
 *
 *  Component:    Korean IM ED
 *
 *  Module:       kedClose.c
 *
 *  Description:  Frees all resources being used
 *
 *  Functions:    kedClose()
 *
 *  History:     
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <fcntl.h>
#include <im.h>
#include <imP.h>
#include "kfep.h"
#include "kedconst.h"`
#include "ked.h"
#include "Hhcim.h"

/*-----------------------------------------------------------------------*
*	Begining of kedClose
*-----------------------------------------------------------------------*/
int     kedClose(kimed)
KIMED   *kimed ;
{
register int	i;
uchar		**from;

	/************************************/
	/* Frees echobufs, echobufa         */
	/* Frees echosvch.                  */
	/************************************/
	free(kimed->echobufs);
	free(kimed->echobufa);
	free(kimed->echosvch);
	free(kimed->fixbuf);
	
	/************************/
	/* Frees Auxbufs and a.	*/
	/************************/
	from = kimed->auxbufs;
	for(i=0; i < KIM_AUXROWMAX; i++) free(*from++);
	from = kimed->auxbufa;
	for(i=0; i < KIM_AUXROWMAX; i++) free(*from++);
	free(kimed->auxbufs);
	free(kimed->auxbufa);

	/***************************/
	/* Close System dictioary. */
	/***************************/
	if (kimed->sdict.fdstat == VALID_FDESC) {
	   close(kimed->sdict.fdesc);
	}
	/*************************/
	/* Close User dictioary. */
	/*************************/
	if (kimed->udict.fdstat == VALID_FDESC) {
	    if (kimed->udict.status == UDP_UPDATE) {
	      /*****************************/
	      /* Updates MRU area into new */
	      /*****************************/
	      update_MRU(kimed);
	      /***************************/
	      /* After other process can */
	      /* update mru area. 	 */
	      /***************************/
	      set_udstat(kimed->udict.fdesc, UD_NORMAL);
            }
	    close(kimed->udict.fdesc);
	}
	free_MRU_list(kimed);
	free(kimed->sdict.ixb);
	free(kimed) ;
	return (KP_OK);
} /* end of kedClose */

/*-----------------------------------------------------------------------*
*	End of kedClose
*-----------------------------------------------------------------------*/
