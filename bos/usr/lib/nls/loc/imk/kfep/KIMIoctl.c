static char sccsid[] = "@(#)57	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMIoctl.c, libkr, bos411, 9428A410j 5/25/92 15:40:07";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMIoctl.c
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
 *  Module:       KIMIoctl.c
 *
 *  Description:  Korean Input Method Control
 *
 *  Functions:    KIMIoctl()
 * 
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	include files 
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include "kimerrno.h"
#include "kfep.h"	/* Korean Input Method header file */

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int 		KIMIoctl(obj, op, arg)
KIMOBJ 		obj;
int 		op;
caddr_t 	arg;
{
    /**********************/
    /* external reference */
    /**********************/
    extern int kimrefresh();
    extern int kimgetstr();
    extern int kimclear();
    extern int kimreset();
    extern int kimchlen();
    extern int kimq_state();
    extern int kimq_text();
    extern int kimq_aux();
    extern int kimq_indicator();
    extern int kimq_ind_str();
    extern int kimchmode();

    /*******************/
    /* local variables */
    /*******************/
    int subret;     /* return from subroutine */
    int retcode;    /* return value for the caller */


    retcode = IMNoError;  /* initialize return code */

    /*************************************************************/
    /* switch to appropriate subroutine depending upon operation */
    /* specified. it is assumed that each subroutine returns	 */
    /* IMNoError on success, otherwise returns value can directly*/
    /* set to imerrno						 */
    /*************************************************************/
    switch(op) {
    case IM_Refresh:
	subret = kimrefresh(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_GetString:
	subret = kimgetstr(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_Clear:
	subret = kimclear(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_Reset:
	subret = kimreset(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_ChangeLength:
	subret = kimchlen(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryState:
	subret = kimq_state(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryText:
	subret = kimq_text(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryAuxiliary:
	subret = kimq_aux(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryIndicator:
	subret = kimq_indicator(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryIndicatorString:

	subret = kimq_ind_str(obj, arg);

	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
	else 

    break;

    case IM_ChangeMode:
	subret = kimchmode(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_GetKeymap:
	*((IMMap *)arg) = 
	(IMMap)((KIMfep *)obj->imobject.imfep)->immap;
    break;

    default:	     
	/* invalid operation specified */
	obj->imobject.imfep->imerrno = subret;
	retcode = IMError;
    } 

  return(retcode);
} /* end of IMIoctl */
/*-----------------------------------------------------------------------*
*	End of procedure
*-----------------------------------------------------------------------*/
