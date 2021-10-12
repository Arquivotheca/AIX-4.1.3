static char sccsid[] = "@(#)07	1.3.1.4  src/bos/usr/lib/nls/loc/jim/jfep/JIMProcAux.c, libKJI, bos411, 9428A410j 9/19/93 20:03:38";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMProcAux
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

#include <stdio.h>
#include <sys/types.h>
#include "imjimP.h"		/* Japanese Input Method header file */
#include <X11/keysym.h>

#define PCONVERT	0x05	/* Convert key (from jed/jedexm.h)	*/

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int JIMProcessAuxiliary(im, aux_id, button, panel_row, panel_col,
			item_row, item_col, Str, Len)
JIMOBJ		im;			/* JIMOBJ pointer	*/
caddr_t		aux_id;			/* Auxiliary ID		*/
unsigned int	button;			/* buttons pushed	*/
unsigned int	panel_row;		/* must be 0		*/
unsigned int	panel_col;		/* must be 0		*/
unsigned int	item_row;		/* must be >= 0		*/
unsigned int	item_col;		/* must be >= 0		*/
caddr_t 	*Str;			/* committed string 	*/
unsigned int	*Len;			/* committed string len */
{
    IMPanel	*panelp;
    char	buff[256];
    int		len;
    IMFep	imfep = im->imobject.imfep;

    *Str = NULL;
    *Len = 0;
    /*
     *	No Selection List
     */
    if(!jedHasSelection(im->jedinfo.jeid)){
	imfep->imerrno = IMInvalidParameter;
	return(IMError);
    }

    /*
     *	Error Check
     */
    if(aux_id != im->auxid || panel_row != 0 || panel_col != 0){
	imfep->imerrno = IMInvalidParameter;
	return(IMError);
    }

    switch(button){

    case IM_SELECTED:

	/*
	 *	Error Check
	 */
	if(item_row >= im->auxinfo.selection.panel->item_row){
	    imfep->imerrno = IMInvalidParameter;
	    return(IMError);
	}
	if(item_col >= im->auxinfo.selection.panel->item_col){
	    imfep->imerrno = IMInvalidParameter;
	    return(IMError);
	}

	/*
	 *	Set the Position at the Right Page and Select the Item
	 */
	jedPositionPage(im->jedinfo.jeid,
	    (item_row+item_col*im->auxinfo.selection.panel->item_col)/10);
	freelistbox(im);
	if(JIMFilter(im, 0x30 + 
	    (item_row+item_col*im->auxinfo.selection.panel->item_col)%10,
	    0, buff, &len) == IMError){
	    return(IMError);
	}else{
	    return(IMNoError);
	}

    case IM_OK:
	if(item_row >= im->auxinfo.selection.panel->item_row ||
	   item_col >= im->auxinfo.selection.panel->item_col){
		;	/* Do the same as IM_CANCEL */
	}else{
	    /*
	     *	Set the Position at the Right Page and Select the Item
	     */
	    jedPositionPage(im->jedinfo.jeid,
		(item_row+item_col*im->auxinfo.selection.panel->item_col)/10);
	    freelistbox(im);
	    if(JIMFilter(im, 0x30 + 
		(item_row+item_col*im->auxinfo.selection.panel->item_col)%10,
		0, buff, &len) == IMError){
		return(IMError);
	    }else{
		return(IMNoError);
	    }
	}

    case IM_CANCEL:
	/*
	 *	If TANKAN, then Terminate the Listbox
	 */
	if(IsTankanList(im->jedinfo.jeid)){
	    freelistbox(im);
	    if(JIMFilter(im, XK_Return, 0, buff, &len) == IMError){
		return(IMError);
	    }else{
		return(IMNoError);
	    }

	}else{
	    IMCallback	*cb = im->imobject.cb;

	    /*
	     *	If it does not have any TANKAN candidates, then Terminate
	     */
	    freelistbox(im);
	    jedMakeListbox(im->jedinfo.jeid, PCONVERT);
	    if(!IsTankanList(im->jedinfo.jeid)){
		jedFreeListbox(im->jedinfo.jeid);
		if(JIMFilter(im, XK_Return, 0, buff, &len) == IMError){
		    return(IMError);
		}else{
		    return(IMNoError);
		}
	    }

	    /*
	     *	Remove the Previous Listbox
	     */
/*	    if(cb->auxhide &&
		(*cb->auxhide)(im, im->auxid, im->imobject.udata)
			== IMError){
		im->imobject.imfep->imerrno = IMCallbackError;
	    	return(IMError);
	    }
	    if(cb->auxdestroy &&
		(*cb->auxdestroy)(im, im->auxid, im->imobject.udata)
			== IMError){
		im->imobject.imfep->imerrno = IMCallbackError;
		freelistbox(im);
	    	return(IMError);
	    }
*/	    /*
	     *	Make a New Listbox and Display It
	     */
	    makelistbox(im);
	    if(im->registration != NULL){
		RegAuxListbox(&(im->auxinfo),
				im->registration, &(im->jedinfo));
	    }
/*	    if(cb->auxcreate &&
		(*cb->auxcreate)(im, &im->auxid, im->imobject.udata)
			== IMError){
		im->imobject.imfep->imerrno = IMCallbackError;
	    	return(IMError);
	    }
*/	    if(cb->auxdraw &&
		(*cb->auxdraw)(im, im->auxid, &im->auxinfo,
				im->imobject.udata) == IMError){
		im->imobject.imfep->imerrno = IMCallbackError;
	    	return(IMError);
	    }
	    return(IMNoError);
	}

    case IM_NEXT:
    case IM_PREV:
	{
	    IMCallback	*cb = im->imobject.cb;

	    freelistbox(im);
	    jedMakeListbox(im->jedinfo.jeid, PCONVERT);
	    makelistbox(im);
	    if(im->registration != NULL){
		RegAuxListbox(&(im->auxinfo),
				im->registration, &(im->jedinfo));
	    }
	    if(cb->auxdraw &&
		(*cb->auxdraw)(im, im->auxid, &im->auxinfo,
				im->imobject.udata) == IMError){
		im->imobject.imfep->imerrno = IMCallbackError;
	    	return(IMError);
	    }
	}
	break;

    default:

	imfep->imerrno = IMInvalidParameter;
	return(IMError);

    }
}
