static char sccsid[] = "@(#)06	1.4.1.3  src/bos/usr/lib/nls/loc/jim/jfep/JIMIoctl.c, libKJI, bos411, 9439A411b 9/25/94 21:22:35";
/*
 * COMPONENT_NAME : (LIBKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMIoctl
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <X11/X.h>
#include "imjimP.h"		/* Japanese Input Method header file */

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int JIMIoctl(obj, op, arg)
JIMOBJ obj;
int op;
caddr_t arg;
{
    /**********************/
    /* external reference */
    /**********************/
    extern int jimrefresh();
    extern int jimgetstr();
    extern int jimclear();
    extern int jimreset();
    extern int jimchlen();
    extern int jimq_state();
    extern int jimq_text();
    extern int jimq_aux();
    extern int jimq_indicator();
    extern int jimq_ind_str();
    extern int jimchmode();
    extern int jimsetstr();
    extern int jimchangeind();

    /*******************/
    /* local variables */
    /*******************/
    int subret;     /* return from subroutine */
    int retcode;    /* return value for the caller */

    SetCurrentSDICTDATA(&(((JIMfep *)(obj->imobject.imfep))->sdictdata));
    SetCurrentUDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->udictinfo));
    SetCurrentFDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->fdictinfo));

    retcode = IMNoError;  /* initialize return code */

    /*************************************************************/
    /* switch to appropriate subroutine depending upon operation */
    /* specified. it is assumed that each subroutine returns	 */
    /* IMNoError on success, otherwise returns value can directly*/
    /* set to imerrno						 */
    /*************************************************************/
    switch(op) {
    case IM_Refresh:
	subret = jimrefresh(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_GetString:
	subret = jimgetstr(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_Clear:
	subret = jimclear(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_Reset:
	subret = jimreset(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_ChangeLength:
	subret = jimchlen(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryState:
	subret = jimq_state(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryText:
	subret = jimq_text(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryAuxiliary:
	subret = jimq_aux(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryIndicator:
	subret = jimq_indicator(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_QueryIndicatorString:
	subret = jimq_ind_str(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_ChangeMode:
	subret = jimchmode(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_GetKeymap:
	*((IMMap *)arg) = (IMMap)((JIMfep *)obj->imobject.imfep)->immap;
    break;

    case IM_SetString:	/* this is of Japanese specific operation */
	subret = jimsetstr(obj, arg);
	if(subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_ChangeIndicator:	/* this is a Japanese specific operation */
	subret = jimchangeind(obj, arg);
	if (subret != IMNoError) {
	    obj->imobject.imfep->imerrno = subret;
	    retcode = IMError;
	}
    break;

    case IM_SupportSelection:	/* this is for selection support */
	if(obj->supportselection == 0 ||
		obj->supportselection == JIM_SELECTIONON){
		jedSetSelectionFlag(obj->jedinfo.jeid, TRUE);
	}
    break;

    case IM_QueryKeyboard:	/* this is for CapsLock special handling */
	if (arg == NULL) {
		return 113;		/* X11R3 */
	} else {
		*((int *)arg) = LockMask;
		return 1141;		/* X11R4ModifierMask */
	}

    default:	     /* invalid operation specified */
	obj->imobject.imfep->imerrno = subret;
	retcode = IMError;

    } /* end of switch(op) */
  return(retcode);
} /* end of IMIoctl */
