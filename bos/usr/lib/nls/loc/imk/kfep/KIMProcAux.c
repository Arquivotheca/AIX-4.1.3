static char sccsid[] = "@(#)59	1.2  src/bos/usr/lib/nls/loc/imk/kfep/KIMProcAux.c, libkr, bos411, 9428A410j 7/29/93 11:08:54";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMProcAux.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM FEP
 *
 *  Module:       KIMProcAux.c
 *
 *  Description:  Korean Input Method Auxiliary area processing 
 *
 *  Functions:    KIMProcessAuxiliary()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include "kimerrno.h"
#include "kfep.h"		/* Korean Input Method header file */

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/

int KIMProcessAuxiliary(im, aux_id, button, panel_row, panel_col,
			item_row, item_col, str, len)
KIMOBJ im;
caddr_t aux_id;
uint button;
uint panel_row;
uint panel_col;
uint item_row;
uint item_col;
caddr_t *str;
uint *len;
{
    *str = NULL;
    *len = 0;

    /* KIM does not support aux of dialog type */
    return(IMError);
}
