static char sccsid[] = "@(#)08	1.3.1.3  src/bos/usr/lib/nls/loc/jim/jfep/JIMProcess.c, libKJI, bos411, 9428A410j 9/30/93 20:47:07";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMProcess
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
#include <stdlib.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "imjimP.h"		/* Japanese Input Method header file */

/*-----------------------------------------------------------------------*
*	Begining of procedure
*-----------------------------------------------------------------------*/

static int	jimfilter(obj, key, shift, imb)
JIMOBJ	obj;
unsigned int	key;
unsigned int	shift;
IMBuffer	*imb;
{
	extern EchoBufChanged	jedIsEchoBufChanged();
	extern AuxSize	jedGetAuxSize();
	extern InputMode	jedGetInputMode();
	extern int	jedIsAuxBufChanged();
	extern int	jedControl();
	extern int	jedIsBeepRequested();
	extern int	jedIsCurPosChanged();

	AuxSize	auxsize;
	int	ret;
	int	textstate;
	int	direction;
	int	offset;
	IMCallback	*cb;
	IMFep	imfep;
	int	jeid;

	/* now registration window is displayed */
	if(obj->registration != NULL){
		return(Registration(obj, key, shift, imb));
	}

	/* while selection panel is displayed, no input is handled */
	if(obj->auxinfo.selection.panel != NULL){
		if(key != XK_Escape || obj->modereset != JIM_RESETON) {
			return(IMInputNotUsed);
		}
	}

	imfep = obj->imobject.imfep;
	cb = obj->imobject.cb;
	jeid = obj->jedinfo.jeid;
	/*********************/
	/* initialize states */
	/*********************/
	obj->textauxstate = IMTextAndAuxiliaryOff;
	if (jedGetEchoBufLen(jeid) == 0)
		textstate = JIM_NOTEXT;
	else
		textstate = JIM_TEXTON;
	inittextinfo(obj);

	/******************************/
	/* call IMED process function */
	/******************************/
						/* ESC key handling */
	if(key == XK_Escape && obj->modereset == JIM_RESETON) {
		IMIndicatorInfo		ind;

		jimclear(obj, 1);
		ind.size = IMHalfWidth | JIM_WIDTH_FLAG;
		ind.unique = JIM_SH_ALPHA | JIM_ALPHA_FLAG;
		IMIoctl(obj, IM_ChangeIndicator, &ind);
		ret = XK_Escape;
	}else{
		ret = jedFilter(jeid, ((JIMFEP)imfep)->immap,
						key, shift, imb);
	}

	/******************************/
	/* start runtime registration */
	/******************************/

	if (ret == KP_REGISTRATION) {
		auxsize = jedGetAuxSize(jeid);
		if(auxsize.itemnum > 0){
			jimclear(obj, 1);
		}
		if((obj->registration = JIMRegisterCreate(obj)) == NULL){
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}
		if(StartRegistration(obj) != IMNoError){
			obj->registration = NULL;
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}else{
			return(IMInputUsed);
		}
	}

	/******************************************************************/
	/* perform CALLBACK function depending upon the return from above */
	/******************************************************************/

	if (ret == KP_UP || ret == KP_DOWN) {
		/************************************************/
		/* we need something special for cursor up/down */
		/************************************************/
		if (ret == KP_UP)
			direction = IM_CursorUp;
		else
			direction = IM_CursorDown;
		if (cb->textcursor)  {
			if ((*(cb->textcursor))(obj, direction, &offset,
				obj->imobject.udata) == IMError) {
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
			if (offset != -1)
				jedControl(jeid, KP_SETCURSOR, offset);
		}
		ret = KP_USED;
	}

	/****************************/
	/* process text information */
	/****************************/
	if (jedGetEchoBufLen(jeid) > 0)
		obj->textauxstate = IMTextOn;
	if (jedIsEchoBufChanged(jeid).flag || jedIsCurPosChanged(jeid)) {
		maketextinfo(obj);
		if (obj->textauxstate == IMTextOn) { /* there is text */
			if (textstate == JIM_NOTEXT) {
				/* there has been no text so far */
				if (cb->textstart) {
					if ((*cb->textstart)(obj, &offset,
						obj->imobject.udata) == IMError) {
						imfep->imerrno = IMCallbackError;
						return(IMError);
					}
					if (offset != -1 &&
						offset <= cb->textmaxwidth)
						jedControl(jeid, KP_CHANGELEN,
						offset);
				}
			}
			if (cb->textdraw && (*cb->textdraw)(obj, &obj->textinfo,
				obj->imobject.udata) == IMError) {
				imfep->imerrno = IMCallbackError;
				return(IMError);
			}
		}
		else /* no data as text */
			if (textstate != JIM_NOTEXT) {
				/* there has been some text before */
				if (cb->texthide && (*cb->texthide)(obj,
					obj->imobject.udata) == IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
			}
	}  /* end of kpisechobufchanged */

	/***************************/
	/* process aux information */
	/***************************/
	auxsize = jedGetAuxSize(jeid);
	SAVEAUXSTATE(obj->auxstate);
	if (auxsize.itemnum > 0 && auxsize.itemsize > 0) {
		obj->auxstate |= JIM_AUXNOW;
		if (obj->textauxstate == IMTextOn)
			obj->textauxstate = IMTextAndAuxiliaryOn;
		else
			obj->textauxstate = IMAuxiliaryOn;
	}

	makeauxinfo(obj);
	if (jedIsAuxBufChanged(jeid) || jedIsAuxCurPosChanged(jeid)) {
		if (obj->auxstate & JIM_AUXNOW) { /* ther is aux right now */
			if (obj->auxstate & JIM_AUXBEFORE) {
				/* there has been... */
				if (cb->auxdraw && (*cb->auxdraw)(obj,
					obj->auxid, &obj->auxinfo,
					obj->imobject.udata) == IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
			}
			else { /* no aux before */
				if (cb->auxcreate && (*cb->auxcreate)(obj,
					&obj->auxid, obj->imobject.udata) ==
					IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
				/* now we have valid id */
				obj->auxidflag = TRUE;
				if (cb->auxdraw && (*cb->auxdraw)(obj,
					obj->auxid, &obj->auxinfo,
					obj->imobject.udata) == IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
			}
		}
		else {	/* no aux right now */
			if (obj->auxstate & JIM_AUXBEFORE) {
				/* there has been... */
				if (cb->auxhide && (*cb->auxhide)(obj,
					obj->auxid, obj->imobject.udata) ==
					IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
				if (cb->auxdestroy && (*cb->auxdestroy)(obj,
					obj->auxid, obj->imobject.udata) ==
					IMError) {
					imfep->imerrno = IMCallbackError;
					return(IMError);
				}
				obj->auxidflag = FALSE;
			}
		}
	} /* end of kpisauxbufchanged */

	/*********************************/
	/* process indicator information */
	/*********************************/
	if (jedIsInputModeChanged(jeid)) {
		makeindinfo(obj);
		makequerystate(obj);
		if (jedGetInputMode(jeid).ind3 == KP_NORMALMODE &&
			cb->indicatordraw && (*cb->indicatordraw)(obj,
			&obj->indinfo, obj->imobject.udata) == IMError) {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		}
	}

	/********/
	/* beep */
	/********/
	if (jedIsBeepRequested(jeid) && cb->beep &&
		(*cb->beep)(obj, JIM_BEEPPER, obj->imobject.udata) == IMError) {
			imfep->imerrno = IMCallbackError;
			return(IMError);
	}
	return ret == KP_USED ? IMInputUsed : IMInputNotUsed;
}

static void	jimlookup(obj, key, shift, imb)
JIMOBJ	obj;
unsigned int	key;
unsigned int	shift;
IMBuffer	*imb;
{
	jedLookup(obj->jedinfo.jeid, ((JIMFEP)obj->imobject.imfep)->immap,
	       key, shift, imb);
}

/*
 *
 */

int	JIMFilter(obj, keysym, state, str, len)
JIMOBJ	obj;
unsigned int	keysym;
unsigned int	state;
caddr_t	*str;
unsigned int	*len;
{
	int	ret;

	SetCurrentSDICTDATA(&(((JIMfep *)(obj->imobject.imfep))->sdictdata));
	SetCurrentUDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->udictinfo));
	SetCurrentFDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->fdictinfo));

	if (IsModifierKey(keysym)) {
		*str = 0;
		*len = 0;
		return IMInputNotUsed;
	}

	if (state & Mod5Mask) {
		if (IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	state &= JIM_VALIDBITS;

	obj->output.len = 0;
	ret = jimfilter(obj, keysym, state, &obj->output);
	*len = makeoutputstring(obj);
	*str = obj->outstr;
	return ret;
}

int	JIMLookup(obj, keysym, state, str, len)
JIMOBJ	obj;
unsigned int	keysym;
unsigned int	state;
caddr_t	*str;
unsigned int	*len;
{
	SetCurrentSDICTDATA(&(((JIMfep *)(obj->imobject.imfep))->sdictdata));
	SetCurrentUDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->udictinfo));
	SetCurrentFDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->fdictinfo));

	if (state & Mod5Mask) {
		if (IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	state &= JIM_VALIDBITS;

	obj->output.len = 0;
	jimlookup(obj, keysym, state, &obj->output);
	*len = makeoutputstring(obj);
	*str = obj->outstr;
	return *len != 0 ? IMReturnString : IMReturnNothing;
}

int	JIMProcess(obj, keysym, state, str, len)
JIMOBJ	obj;
unsigned int	keysym;
unsigned int	state;
caddr_t	*str;
unsigned int	*len;
{
	SetCurrentSDICTDATA(&(((JIMfep *)(obj->imobject.imfep))->sdictdata));
	SetCurrentUDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->udictinfo));
	SetCurrentFDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->fdictinfo));

	if (IsModifierKey(keysym)) {
		*str = 0;
		*len = 0;
		return obj->textauxstate;
	}

	if (state & Mod5Mask) {
		if (IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	state &= JIM_VALIDBITS;

	obj->output.len = 0;
	if (jimfilter(obj, keysym, state, &obj->output) == IMInputNotUsed)
		jimlookup(obj, keysym, state, &obj->output);
	*len = makeoutputstring(obj);
	*str = obj->outstr;
	return obj->textauxstate;
}
