/* @(#)23	1.2  src/bos/usr/lib/nls/loc/sim/dim.c, cmdims, bos411, 9428A410j 7/29/93 11:16:33 */
/*
 * COMPONENT_NAME : (CMDIMS) SBCS Input Method
 *
 * FUNCTIONS : Dynamic Composing Input Method (DIM)
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------------------------------------------------*
 *	Include Files
 *----------------------------------------------------------------------*/
#include <unistd.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <im.h>
#include <bim.h>
#include "imP.h"
#include "dim.h"

extern int		imerrno;

static int		PutKeysym(DIMObject obj, unsigned int keysym,
					unsigned int state);

#define FEP(obj)	((DIMFep)((obj)->common.imfep))
#define MATCH(val, def)	((val == def) || (def == XK_ALL))
#define MATCHSTATE(val, def)	\
	((val == def) || (def == XK_ALL) || \
		((def & OR_STATE_FLAG) && (val & def)))

#define	ALLOC_UNIT		256	/* has to be larger than 256 */

#define SIZE_CHECK(imb, length)						\
{									\
	if((imb)->len + length > (imb)->siz){				\
		(imb)->siz += ALLOC_UNIT;				\
		(imb)->data =						\
			(unsigned char *)realloc((imb)->data, (imb)->siz);\
	}								\
}

#define	IS_KP_KEY(key)	((XK_KP_0 <= (key)) && ((key) <= XK_KP_9))


/*----------------------------------------------------------------------*
 *	put the compose sequence to the output buffer
 *----------------------------------------------------------------------*/
static void	PutItems(DIMObject obj)
{
	int	i;

	for(i = 0; i < obj->item_num; i++){
		PutKeysym(obj, obj->keysym[i], obj->state[i]);
	}
}


/*----------------------------------------------------------------------*
 *	beep
 *----------------------------------------------------------------------*/
static void	beep(DIMObject obj)
{
	if(obj->common.cb->beep != NULL){
		obj->common.cb->beep(obj, DIM_BEEPPERCENT, obj->common.udata);
	}
}


/*----------------------------------------------------------------------*
 *	put a keysym to the output buffer
 *----------------------------------------------------------------------*/
static int	PutKeysym(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state)
{
	switch(keysym){
	case XK_UNBOUND:	/* output all items in the compose sequence */
		PutItems(obj);
		return 1;
	case XK_BEEP:		/* beep */
		beep(obj);
	case XK_IGNORE:		/* nothing is output for BEEP/IGNORE */
		obj->output.data[0] = '\0';
		obj->output.len = 0;
		return 1;
	default:		/* put a character converted from the keysym */
		_IMSimpleMapping(FEP(obj)->immap, keysym, state, &obj->output);
		return 0;
	}
}


/*----------------------------------------------------------------------*
 *	put a string to the output buffer
 *----------------------------------------------------------------------*/
static void	PutResult(
	DIMObject	obj,
	unsigned char	*str)
{
	int	i, j, len;

	len = *str++;
	SIZE_CHECK(&(obj->output), len);
	for(i = 0, j = -1; i < len; i++){

		/* real '*' */
		if(*str == '\\' && *(str+1) == '*'){
			obj->output.data[obj->output.len++] = '*';
			str++;
			i++;

		/* real backslash */
		}else if(*str == '\\' && *(str+1) == '\\'){
			obj->output.data[obj->output.len++] = '\\';
			str++;
			i++;

		/* normal character */
		}else if(*str != '*'){
			obj->output.data[obj->output.len++] = *str;

		/* '*' outputs corresponding char in the compose sequence */
		}else{
			for(j++; j < obj->item_num; j++){
				if((obj->item[j] != NULL) &&
					(obj->item[j]->keysym == XK_ALL)){
					PutKeysym(obj, obj->keysym[j],
							obj->state[j]);
					break;
				}
			}

			/* cannot find corresponding char in the sequence */
			if(j >= obj->item_num){
				obj->output.data[obj->output.len++] = *str;
			}
		}
		str++;
	}
}


/*----------------------------------------------------------------------*
 *	search brother compose tables
 *----------------------------------------------------------------------*/
static ComposeTable	*Search(
	ComposeTable	*ctp,
	unsigned int	keysym,
	unsigned int	state,
	unsigned int	layer)
{
	for(; ctp != NULL; ctp = ctp->brother){
		if(MATCH(keysym, ctp->keysym) && 
				MATCHSTATE(state, ctp->state) &&
				MATCH(layer, ctp->layer)){
			return ctp;
		}
	}

	return NULL;
}


/*----------------------------------------------------------------------*
 *	search brother and child compose tables
 *----------------------------------------------------------------------*/
static ComposeTable	*SearchAll(
	DIMObject	obj,
	ComposeTable	*head,
	unsigned int	keysym,
	unsigned int	state,
	unsigned int	layer)
{
	ComposeTable	*ctp;

	if(head == NULL){
		return NULL;
	}

	/* search the children */
	ctp = Search(head->child, keysym, state, layer);

	/* search the brothers */
	if(ctp == NULL){
		ctp = Search(head->brother, obj->keysym[obj->item_num-1],
				obj->state[obj->item_num-1], layer);
		if(ctp != NULL){
			ctp = Search(ctp->child, keysym, state, layer);
		}
	}

	return ctp;
}


/*----------------------------------------------------------------------*
 *	search compose tables and output results
 *----------------------------------------------------------------------*/
static int	Compose(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state,
	unsigned int	layer)
{
	ComposeTable	*ctp;
	int		ret;

	if(keysym == XK_VoidSymbol){
		obj->output.data[0] = '\0';
		obj->output.len = 0;
		obj->compose = FEP(obj)->compose_table;
		obj->item_num = 0;
		return IMInputNotUsed;
	}

	/* all keys are mapped before composing */
	/* BIM_PARSEKEYS requres no mapping */
	if(!(layer & BIM_PARSEKEYS)){
		state |= (obj->layer & ~BIM_PARSEKEYS);
		_IMMapKeysym(FEP(obj)->immap, &keysym, &state);
	}

	/* search */
	ctp = SearchAll(obj, obj->compose, keysym, state, layer);

	/* composing failed */
	if(ctp == NULL){

		/* the middle of composing means error */
		if(obj->compose != FEP(obj)->compose_table){
			if(FEP(obj)->compose_error_str != NULL){
				PutResult(obj, FEP(obj)->compose_error_str);
			}else{
				PutKeysym(obj, FEP(obj)->compose_error, 0);
			}

			/* reset internal state */
			obj->compose = FEP(obj)->compose_table;
			obj->item_num = 0;

			/* when beeping, input is thrown away */
			if(FEP(obj)->compose_error == XK_BEEP){
				return IMInputUsed;
			}
		}
		return IMInputNotUsed;
	}

	/* save input key information */
	obj->keysym[obj->item_num] = keysym;
	obj->state[obj->item_num] = state;
	obj->item[obj->item_num++] = ctp;
		
	/* composed successfully. the result is a string or a character */
	if(ctp->result_string != NULL){
		PutResult(obj, ctp->result_string);
		obj->compose = FEP(obj)->compose_table;
		obj->item_num = 0;

	/* composed successfully. the result is a keysym */
	}else if(ctp->result_keysym != XK_NONE){
		ret = PutKeysym(obj, ctp->result_keysym, 0);

		/* nothing was output */
		if(ret == 0 && obj->output.len == 0){

			/* current keysym/state is not used when error occurs */
			obj->item_num--;
			if(FEP(obj)->compose_error_str != NULL){
				PutResult(obj, FEP(obj)->compose_error_str);
			}else{
				PutKeysym(obj, FEP(obj)->compose_error, 0);
			}

			/* reset internal state */
			obj->compose = FEP(obj)->compose_table;
			obj->item_num = 0;

			/* when beeping, input is thrown away */
			if(FEP(obj)->compose_error == XK_BEEP){
				return IMInputUsed;
			}
			return IMInputNotUsed;
		}

		obj->compose = FEP(obj)->compose_table;
		obj->item_num = 0;

	/* in the middle of composing */
	}else{
		obj->compose = ctp;
		obj->output.data[0] = '\0';
		obj->output.len = 0;
	}

	return IMInputUsed;
}


/*----------------------------------------------------------------------*
 *	Check layer switch
 *----------------------------------------------------------------------*/
static int	CheckLayerSwitch(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state,
	unsigned int	layer)
{
	int		i;
	LayerSwitch	*p;

	p = FEP(obj)->layer_switch;
	for(i = 0; i < FEP(obj)->layer_switch_num; i++, p++){
		if(MATCH(keysym, p->keysym) && MATCHSTATE(state, p->state) &&
						MATCH(layer, p->layer)){

			/* reset internal states */
			obj->layer = p->result_layer;
			obj->compose = FEP(obj)->compose_table;
			obj->item_num = 0;
			return IMInputUsed;
		}
	}

	return IMInputNotUsed;
}


/*----------------------------------------------------------------------*
 *	Get n-th layer
 *----------------------------------------------------------------------*/
static int	GetLayer(
	DIMObject	obj,
	unsigned int	n)
{
	int		i;
	LayerSwitch	*p;

	p = FEP(obj)->layer_switch;

	/* layers are sorted in ascendand order */
	for(i = 0, n--; i < FEP(obj)->layer_switch_num - 1; i++, p++){
		if(p->layer != (p+1)->layer){
			if(n == 1){
				return((p+1)->layer);
			}
			n--;
		}
	}

	return 0;
}


/*
 *	IMCORE Interface Functions.
 */

/*----------------------------------------------------------------------*
 *	DIMObject corresponds to IMObject
 *----------------------------------------------------------------------*/
static DIMObject	DIMCreate(
	DIMFep		dimfep,
	IMCallback	*cb,
	caddr_t		udata)
{
	DIMObject	obj;

	if(!(obj = (DIMObject)malloc(sizeof(DIMObjectRec))))
		return NULL;
	if(!(obj->output.data =
		(unsigned char *)malloc(obj->output.siz = ALLOC_UNIT))){
		free(obj);
		return NULL;
	}
	obj->common.imfep = (IMFep)dimfep;
	obj->common.cb = cb;
	obj->common.mode = IMNormalMode;
	obj->common.udata = udata;
	obj->output.len = 0;
	obj->layer = 0;
	obj->compose = FEP(obj)->compose_table;
	obj->item_num = 0;

	return obj;
}


/*----------------------------------------------------------------------*
 *	DIMDestroy corresponds to IMDestroy
 *----------------------------------------------------------------------*/
static void	DIMDestroy(DIMObject obj)
{
	if (obj->output.siz != 0)
		free(obj->output.data);
	free(obj);
}


/*----------------------------------------------------------------------*
 *	DIMFilter corresponds to IMFilter
 *----------------------------------------------------------------------*/
static int	DIMFilter(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state,
	caddr_t		*str,
	unsigned int	*len)
{
	unsigned int	i, ret_value, save_layer;

	/*
	 *	bidi handling
	 */
	if(obj->layer & BIM_PARSEKEYS){

	 	/* check the layer switch */
		if(CheckLayerSwitch(obj, keysym, state, BIM_PARSEKEYS)
							== IMInputUsed){
			*str = "";
			*len = 0;
			return IMInputUsed;
		}

		/* compose with PareseKeys flag */ 
		obj->output.len = 0;
		if(Compose(obj, keysym, state, BIM_PARSEKEYS) == IMInputUsed){
			*str = obj->output.data;
			*len = obj->output.len;
			return IMInputUsed;
		}
	}

	/*
	 *	SuppressedMode: DIMFilter does nothing.
	 */
	if (obj->common.mode == IMSuppressedMode) {
		*str = "";
		*len = 0;
		return IMInputNotUsed;
	}

	/* check the layer switch */
	save_layer = obj->layer;
	if(CheckLayerSwitch(obj, keysym, state, (obj->layer & ~BIM_PARSEKEYS))
							== IMInputUsed){
		obj->layer |= (save_layer & BIM_PARSEKEYS);
		*str = "";
		*len = 0;
		return IMInputUsed;
	}

	/* Modifier keys are ignored */
	if(IsModifierKey(keysym)){
		*str = "";
		*len = 0;
		return IMInputUsed;
	}

	if(state & Mod5Mask){
		if(IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	/* compose without PareseKeys flag */ 
	obj->output.len = 0;
	ret_value = Compose(obj, keysym, state, (obj->layer & ~BIM_PARSEKEYS));
	*str = obj->output.data;
	*len = obj->output.len;

	return ret_value;
}


/*----------------------------------------------------------------------*
 *	DIMLookup corresponds to IMLookupSting
 *----------------------------------------------------------------------*/
static int	DIMLookup(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state,
	caddr_t		*str,
	unsigned int	*len)
{
	/*
	 *	bidi handling
	 */
	if(obj->layer & BIM_PARSEKEYS){

		/* compose with PareseKeys flag */ 
		obj->output.len = 0;
		if(Compose(obj, keysym, state, BIM_PARSEKEYS) == IMInputUsed){
			*str = obj->output.data;
			*len = obj->output.len;
			return IMInputUsed;
		}
	}

	if(state & Mod5Mask){
		if(IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	/* simple mapping without PareseKeys flag */ 
	state |= (obj->layer & ~BIM_PARSEKEYS);
	_IMMapKeysym(FEP(obj)->immap, &keysym, &state);
	obj->output.len = 0;
	_IMSimpleMapping(FEP(obj)->immap, keysym, state, &obj->output);
	*str = obj->output.data;
	*len = obj->output.len;

	return obj->output.len != 0 ? IMReturnString : IMReturnNothing;
}


/*----------------------------------------------------------------------*
 *	DIMProcess corresponds to IMProcess
 *----------------------------------------------------------------------*/
static int	DIMProcess(
	DIMObject	obj,
	unsigned int	keysym,
	unsigned int	state,
	caddr_t		*str,
	unsigned int	*len)
{
	unsigned int	i, save_layer;

	/*
	 *	bidi handling
	 */
	if(obj->layer & BIM_PARSEKEYS){

	 	/* check the layer switch */
		if(CheckLayerSwitch(obj, keysym, state, BIM_PARSEKEYS)
							== IMInputUsed){
			*str = "";
			*len = 0;
			return IMTextAndAuxiliaryOff;
		}

		/* compose with PareseKeys flag */ 
		obj->output.len = 0;
		if(Compose(obj, keysym, state, BIM_PARSEKEYS) == IMInputUsed){
			*str = obj->output.data;
			*len = obj->output.len;
			return IMTextAndAuxiliaryOff;
		}
	}

	/*
	 *	SuppressedMode: DIMProcess does nothing.
	 */
	if(obj->common.mode == IMSuppressedMode){
		DIMLookup(obj, keysym, state, str, len);
		return IMTextAndAuxiliaryOff;
	}

	/* check the layer switch */
	save_layer = obj->layer;
	if(CheckLayerSwitch(obj, keysym, state, (obj->layer & ~BIM_PARSEKEYS))
							== IMInputUsed){
		obj->layer |= (save_layer & BIM_PARSEKEYS);
		*str = "";
		*len = 0;
		return IMTextAndAuxiliaryOff;
	}

	/* Modifier keys are ignored */
	if(IsModifierKey(keysym)){
		*str = "";
		*len = 0;
		return IMTextAndAuxiliaryOff;
	}

	if(state & Mod5Mask){
		if(IsKeypadKey(keysym))
			state ^= ShiftMask;
		state &= ~Mod5Mask;
	}

	/* compose without PareseKeys flag */ 
	obj->output.len = 0;
	if(Compose(obj, keysym, state, (obj->layer & ~BIM_PARSEKEYS))
							== IMInputNotUsed){

		/* If the input is not used, do simple mapping. */
		state |= (obj->layer & ~BIM_PARSEKEYS);
		_IMMapKeysym(FEP(obj)->immap, &keysym, &state);
		_IMSimpleMapping(FEP(obj)->immap, keysym, state, &obj->output);
	}
	*str = obj->output.data;
	*len = obj->output.len;

	return IMTextAndAuxiliaryOff;
}


/*----------------------------------------------------------------------*
 *	IMProcessAuxiliary (not supported)
 *----------------------------------------------------------------------*/
static int	DIMProcessAuxiliary(
	DIMObject obj,
	caddr_t auxid,
	unsigned int button,
	unsigned int panel_row,
	unsigned int panel_col,
	unsigned int item_row,
	unsigned int item_col,
	caddr_t *str,
	unsigned int *len)
{
	*str = NULL;
	*len = 0;

	return IMNoError;		/* No action is taken. */
}


/*----------------------------------------------------------------------*
 *	DIMIoctl corresponds to IMIoctl
 *----------------------------------------------------------------------*/
static int	DIMIoctl(
	DIMObject obj,
	int cmd,
	caddr_t arg)
{
	switch(cmd){
	case IM_Refresh:
		break;

	case IM_GetString:		/* DIM has no pre-edit text */
		((IMSTR *)arg)->len = 0;
		((IMSTR *)arg)->str = "";
		break;

	case IM_Clear:
	case IM_Reset:
	case IM_ChangeLength:
		break;

	case IM_QueryState:
		((IMQueryState *)arg)->mode = obj->common.mode;
							/* Processing Mode */
		((IMQueryState *)arg)->text = FALSE;	/* No Change */
		((IMQueryState *)arg)->aux = FALSE;	/* No Change */
		((IMQueryState *)arg)->indicator = FALSE;/* No Change */
		((IMQueryState *)arg)->beep = DIM_BEEPPERCENT;/* Beep */
		break;

	case IM_QueryText:		/* DIM has no pre-edit text */
		((IMQueryText *)arg)->textinfo = 0;
		break;

	case IM_QueryAuxiliary:		/* DIM has no auxiliary area */
		((IMQueryAuxiliary *)arg)->auxinfo = 0;
		break;

	case IM_QueryIndicator:		/* DIM has no indicator */
		((IMQueryIndicator *)arg)->indicatorinfo = 0;
		break;

	case IM_QueryIndicatorString:	/* DIM has no indicator */
		((IMQueryIndicatorString *)arg)->indicator.len = 0;
		((IMQueryIndicatorString *)arg)->indicator.str = "";
		break;

	case IM_ChangeMode:
		if((int)arg == IMNormalMode || (int)arg == IMSuppressedMode){
			obj->common.mode = (int)arg;
			break;
		}
		obj->common.imfep->imerrno = IMInvalidParameter;
		return IMError;

	case IM_GetKeymap:
		*((IMKeymap **)arg) = FEP(obj)->immap;
		break;

	/* Bidi specific commands */

	case BIM_ChangeLayer:
		if((int)arg == BIMLatinLayer){
			obj->layer = (obj->layer & BIM_PARSEKEYS);
			break;
		}else if((int)arg == BIMNlsLayer){
			obj->layer = GetLayer(obj, 2) |
					(obj->layer & BIM_PARSEKEYS);
			break;
		}
		obj->common.imfep->imerrno = IMInvalidParameter;
		return IMError;

	case BIM_ChangeParseKeys:
		if((int)arg == BIMParseKeys){
			obj->layer |= BIM_PARSEKEYS;
			break;
		}else if((int)arg == BIMDontParseKeys){
			obj->layer &= ~BIM_PARSEKEYS;
			break;
		}
		obj->common.imfep->imerrno = IMInvalidParameter;
		return IMError;

	case BIM_QueryLayer:
		if(obj->layer & GetLayer(obj, 2)){
			((BIMQueryLayer *)arg)->layer = BIMNlsLayer;
		}else{
			((BIMQueryLayer *)arg)->layer = BIMLatinLayer;
		}
		break;

	case BIM_QueryParseKeys:
		if(obj->layer & BIM_PARSEKEYS){
			((BIMQueryParseKeys *)arg)->parseKeys = BIMParseKeys;
		}else{
			((BIMQueryParseKeys *)arg)->parseKeys
							= BIMDontParseKeys;
		}
		break;

	default:
		obj->common.imfep->imerrno = IMInvalidParameter;
		return IMError;
	}
	return IMNoError;
}


/*----------------------------------------------------------------------*
 *	DIMClose corresponds to IMClose
 *----------------------------------------------------------------------*/
static void	DIMClose(DIMFep dimfep)
{
	_IMCloseKeymap(dimfep->immap);
	if(dimfep->compose != NULL){
		free(dimfep->compose);
	}
	free(dimfep);
	return;
}


/*----------------------------------------------------------------------*
 *	DIMInitialize corresponds to IMInitialize
 *----------------------------------------------------------------------*/
DIMFep	DIMInitialize(IMLanguage language) 
{
	DIMFep		dimfep;
	char		*file, *getenv(char *name);

	if(!(dimfep = (DIMFep)malloc(sizeof(DIMFepRec))))
		return NULL;

	/* fill in the IMFepCommon */
	dimfep->common.iminitialize = (IMFep(*)(void *))DIMInitialize;
	dimfep->common.imclose = DIMClose;
	dimfep->common.imcreate =
			(IMObject(*)(void *, void *, void *))DIMCreate;
	dimfep->common.imdestroy = DIMDestroy;
	dimfep->common.improcess = DIMProcess;
	dimfep->common.improcessaux = DIMProcessAuxiliary;
	dimfep->common.imioctl = DIMIoctl;
	dimfep->common.imfilter = DIMFilter;
	dimfep->common.imlookup = DIMLookup;
	
	/* fill in the private part of DIMFep */
	if (!(dimfep->immap = _IMInitializeKeymap(language))) {
		free(dimfep);
		return NULL;
	}

	/* version stamp */
	dimfep->version = DIMIMVersionNumber;

	/* initialize data for composing */
	dimfep->compose = NULL;
	dimfep->compose_table = NULL;
	dimfep->compose_error = XK_UNBOUND;	 /* default */
	dimfep->compose_error_str = NULL;
	dimfep->layer_switch_num = 0;
	dimfep->layer_switch = NULL;

	/* read compose definition. don't worry about error */
	OpenCompose(language, dimfep);

	return dimfep;
}
