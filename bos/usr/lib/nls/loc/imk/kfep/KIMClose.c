static char sccsid[] = "@(#)52	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMClose.c, libkr, bos411, 9428A410j 5/25/92 15:39:17";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMClose.c
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
 *  Component:    Korean IM FEP
 *
 *  Module:       KIMClose.c
 *
 *  Description:  Korean Input Method Close
 *
 *  Functions:    KIMClose()
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/
#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include "kfep.h"		/* Korean Input Method header file */

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
void KIMClose(imfep)
KIMFEP imfep;
{
    _IMCloseKeymap(imfep->immap);
    free(imfep);
}
/*-----------------------------------------------------------------------*
*	End of procedure
*-----------------------------------------------------------------------*/
