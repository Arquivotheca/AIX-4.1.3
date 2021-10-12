static char sccsid[] = "@(#)55	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMFilter.c, libkr, bos411, 9428A410j 5/25/92 15:39:47";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		KIMFilter.c
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
 *  Module:       KIMFilter.c
 *
 *  Description:  Korean Input Method Filtering Process.
 *
 *  Functions:    KIMFilter()
 *		  kimfilter()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <im.h>
#include <imP.h>
#include "kimerrno.h"
#include "kfep.h"		/* Korean Input Method header file */
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	Begining of procedure
*-----------------------------------------------------------------------*/
int     KIMFilter(obj, keysym, state, str, len)
KIMOBJ  	obj;
unsigned int    keysym;
unsigned int    state;
caddr_t 	*str;
unsigned int    *len;
{
        int     ret;

/*********
fprintf(stderr,"KIMFilter.c:  Enter this function with keysym(%x) \n",keysym);
***********/

        if (IsModifierKey(keysym)) 
	{
/*********
fprintf(stderr,"KIMFilter.c: IsModifier(keysym) = %d\n", IsModifier(keysym));
*********/

                *str = 0;
                *len = 0;
/***** Banja/Jeonja mode key : ALT+Caps_Lock *****/
		if (keysym != XK_Caps_Lock)
               		return IMInputNotUsed;
        }

        if (state & Mod5Mask) {
                if (IsKeypadKey(keysym))
                        state ^= ShiftMask;
                state &= ~Mod5Mask;
        }

        state &= KIM_VALIDBITS;

        obj->output.len = 0;
        ret = kimfilter(obj, keysym, state, &obj->output);
        *len = makeoutputstring(obj);
        *str = obj->outstr;

/**********
fprintf(stderr,"KIMFilter.c:  Exit this function with ret (%d)\n",ret);
**********/
        return ret;
}


/*-----------------------------------------------------------------------*
 *	Subtouinte of KIMFilter.
 *-----------------------------------------------------------------------*/
int kimfilter(obj, key, shift, imb)

KIMOBJ 		obj;
uint 		key;
uint 		shift;
IMBuffer	*imb;

{
    /***********************/
    /* external references */
    /***********************/
    extern int       kedProcess();
    extern int       kedIsEchoBufChanged();
    extern AuxSize   kedGetAuxSize();
    extern InputMode kedGetInputMode();
    extern int	     kedIsAuxBufChanged();
    extern int	     kedControl();
    extern int	     kedIsBeepRequested();
    extern int	     kedIsCurPosChanged();

    /*******************/
    /* local variables */
    /*******************/
    AuxSize 		auxsize;
    int			auxtype;
    int 		ret;
    int 		textstate;
    int 		direction;
    int 		offset;
    InputMode		imode;
    IMFep		imfep;
    KIMED		*kimed;
    IMCallback	 	*imcallback;

    /**************/
    /*		  */
    /* Local Copy */
    /*		  */
    /**************/
    imfep      =   obj->imobject.imfep;
    imcallback =   obj->imobject.cb;
    kimed      =   (KIMED*) obj->kimed;

    /*********************/
    /* initialize states */
    /*********************/
    obj->textauxstate = IMTextAndAuxiliaryOff;
    if(kedGetEchoBufLen(kimed) == 0)
	textstate = KIM_NOTEXT;
    else
	textstate = KIM_TEXTON;

    inittextinfo(obj);

    /******************************/
    /* call IMED process function */
    /******************************/
    ret = kedFilter(kimed, ((KIMFEP)imfep)->immap, key, shift, imb);

#ifdef DEBUG 
    dump_keid(kimed, __LINE__, __FILE__);
    show_all(kimed);
    print_return(ret);
#endif DEBUG 

    /******************************************************************/
    /* perform CALLBACK function depending upon the return from above */
    /******************************************************************/
    /************************************************/
    /* we need something special for cursor up/down */
    /************************************************/
    if(ret == KP_UP || ret == KP_DOWN) 
    {
	if(ret == KP_UP)
	    direction = IM_CursorUp;
	else
	    direction = IM_CursorDown;
	if(imcallback->textcursor)  
	{
	     if((*(imcallback->textcursor))
		(obj, direction, &offset, obj->imobject.udata) == IMError) 
	     {
		imfep->imerrno = IMCallbackError;
		return(IMError);
	     }
	     if(offset != -1) 
	    		kedControl(kimed, KP_SETCURSOR, offset);
        }
	ret = KP_USED;
      } /* end of UP or DOWN */

    /****************************/
    /* process text information */
    /****************************/
    if(kedGetEchoBufLen(kimed) > 0)
	obj->textauxstate = IMTextOn;
    if(kedIsEchoBufChanged(kimed) || kedIsCurPosChanged(kimed)) 
    {
	maketextinfo(obj);
	if(obj->textauxstate == IMTextOn) 
	{
	    /*****************/
	    /*		     */
	    /* there is text */
	    /*		     */
	    /*****************/
	    if(textstate == KIM_NOTEXT) 
	    {
		/* there has been no text so far */
		if(imcallback->textstart) 
		{
		    if((*(imcallback->textstart))
			(obj, &offset, obj->imobject.udata) == IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		}
		else 
		    offset = -1;
		if(offset != -1 && offset <= imcallback->textmaxwidth)
		    kedControl(kimed, KP_CHANGELEN, offset);
	    } 
	    if(imcallback->textdraw) 
	    {
		if((*(imcallback->textdraw))
			(obj, &(obj->textinfo), obj->imobject.udata) == IMError) 
		{
		    imfep->imerrno = IMCallbackError;
		    return(IMError);
		}
	    } 
	}
	else 
	    /*******************/
	    /*		       */
	    /* no data as text */
	    /*		       */
	    /*******************/
	    if(textstate != KIM_NOTEXT) 
	    {
		/***********************************/
		/*				   */
	        /* there has been some text before */
		/*				   */
		/***********************************/
	        if(imcallback->texthide) 
	        {
		    if((*(imcallback->texthide))(obj, obj->imobject.udata) == IMError) 
		    {
		        imfep->imerrno = IMCallbackError;
		        return(IMError);
		    }
	        } 
	    }
    }  /* end of kpisechobufchanged */

    /***************************/
    /* process aux information */
    /***************************/
    auxtype = kedGetAuxType(kimed);
    SAVEAUXSTATE(obj->auxstate);
    if (auxtype == MULTICAND_USED || auxtype == CODEINPUT_USED) 
    {
	obj->auxstate |= KIM_AUXNOW;
	if(obj->textauxstate == IMTextOn)
	    obj->textauxstate = IMTextAndAuxiliaryOn;
	else
	    obj->textauxstate = IMAuxiliaryOn;
    }

    if(kedIsAuxBufChanged(kimed) ||
	  kedIsAuxCurPosChanged(kimed)) 
    {
        makeauxinfo(obj);
	if(obj->auxstate & KIM_AUXNOW) 
	{
 	    /**************************/
	    /*			      */
	    /* there is aux right now */
	    /*			      */
 	    /**************************/
	    if(obj->auxstate & KIM_AUXBEFORE) 
	    {
		/*********************/
		/*		     */
		/* there has been... */
		/*		     */
		/*********************/
		if(imcallback->auxdraw) 
		{
		    if((*(imcallback->auxdraw))
		       (obj, obj->auxid, &(obj->auxinfo), obj->imobject.udata) 
			== IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		}
	    }
	    else 
	    { 
		/*****************/
	 	/*		 */
	        /* no aux before */
	 	/*		 */
		/*****************/
		if(imcallback->auxcreate) 
		{
		    if((*(imcallback->auxcreate))
			(obj, &(obj->auxid), obj->imobject.udata) == IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		    /* now we have valid id */
		    obj->auxidflag = TRUE; 
		}
		if(imcallback->auxdraw) 
		{
		    if((*(imcallback->auxdraw))
			  (obj, obj->auxid, &(obj->auxinfo),
			  obj->imobject.udata) == IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		}
	    }
	}
	else 
	{	
	    /********************/
	    /* 			*/
	    /* no aux right now */
	    /* 			*/
	    /********************/
	    if(obj->auxstate & KIM_AUXBEFORE) 
	    {
		/*********************/
		/* 		     */
		/* there has been... */
		/* 		     */
		/*********************/
		if(imcallback->auxhide) 
	 	{
		    if((*(imcallback->auxhide))
			(obj, obj->auxid, obj->imobject.udata) == IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		}
		if(imcallback->auxdestroy) 
		{
		    if((*(imcallback->auxdestroy))
			(obj, obj->auxid, obj->imobject.udata) == IMError) 
		    {
			imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
		    obj->auxidflag = FALSE;
		}
	    }
	}
    } /* end of kpisauxbufchanged */

    /*********************************/
    /* process indicator information */
    /*********************************/
    if(kedIsInputModeChanged(kimed)) 
    {
	makeindinfo(obj);
	makequerystate(obj);
	if(imcallback->indicatordraw) 
	{
	  if((*(imcallback->indicatordraw))
	   (obj, &(obj->indinfo), obj->imobject.udata) == IMError) 
	  {
		    imfep->imerrno = IMCallbackError;
		    return(IMError);
	  }
	}
    }

    /********/
    /* beep */
    /********/
    if(kedIsBeepRequested(kimed)) 
    {
	if(imcallback->beep) 
	{
	     if((*(imcallback->beep))
		(obj, KIM_BEEPPER, obj->imobject.udata) == IMError) 
		{
		    imfep->imerrno = IMCallbackError; 
		    return(IMError);
		}
        }
    }
    /*********************/
    /* end of processing */
    /*********************/
    return ((ret == KP_USED) ? (IMInputUsed) : (IMInputNotUsed));

} /* end of kimfilter  */
