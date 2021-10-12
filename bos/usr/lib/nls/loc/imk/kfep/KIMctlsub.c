static char sccsid[] = "@(#)61	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMctlsub.c, libkr, bos411, 9428A410j 5/25/92 15:40:46";
/*
 * COMPONENT_NAME :	(LIBKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMctlsub.c
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
 *  Module:       KIMctlsub.c
 *
 *  Description:  Korean Input Method Control subroutines
 *
 *  Functions:    kimrefresh()
 *		  kimgetstr()
 *		  kimclear()
 *		  kimreset()
 *	 	  kimchlen()
 *		  kimq_state()
 *		  kimq_text()
 * 		  kimq_aux()
 *		  kimq_indicator()
 *		  kimq_ind_str()
 *		  kimchmode()
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
#include "kfep.h"		/* Korean Input Method header file */
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	external references 
*-----------------------------------------------------------------------*/
extern int     			ketGetEchoBufLen();
extern AuxSize 			kedGetAuxSize();
extern int     			kedFix();
extern int     			kedClear();
extern int     			kedControl();
extern int     			kedIsEchoBufChanged();
extern int     			kedIsAuxBufChanged();
extern int     			kedIsInputModeChanged();
extern int     			kedIsBeepRequested();
extern InputMode 		kedGetInputMode();
extern int     			kedSetInputMode();

/*-----------------------------------------------------------------------*
 *	Redrawing the inputmethod windows on the screen. 
 *-----------------------------------------------------------------------*/
int kimrefresh(obj, arg)
KIMOBJ 		obj;
caddr_t 	arg;
{
    /*******************/
    /* local variables */
    /*******************/
    AuxSize  auxsize;
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;

    /********************************************/
    /* call Callbacks depending upon the states */
    /********************************************/
    if(obj->q_state.mode == IMNormalMode) {
	if(obj->imobject.cb->indicatordraw) { 
	    if((*(obj->imobject.cb->indicatordraw))
		   (obj, &(obj->indinfo), obj->imobject.udata) 
			== IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
    }

    if(kedGetEchoBufLen(kimed) > 0) {
	if(obj->imobject.cb->textdraw) {
	    if((*(obj->imobject.cb->textdraw))
		(obj, &(obj->textinfo), obj->imobject.udata) 
			== IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
    }

    auxsize = kedGetAuxSize(kimed);
    if(obj->auxstate & KIM_AUXNOW) {
	if(obj->imobject.cb->auxdraw) {
	    if((*(obj->imobject.cb->auxdraw))
		(obj, obj->auxid, 
		&(obj->auxinfo), obj->imobject.udata) 
			== IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
    }
    return(IMNoError);
} /* end of kimrefresh */

/*-----------------------------------------------------------------------*
*	Beginning of getstring
*       Makes the string into fixbuf, clears all other buffers.
*	Applications gets it.
*-----------------------------------------------------------------------*/
int kimgetstr(obj, arg)
KIMOBJ 		obj;
IMSTR 		*arg;
{
    /*******************/
    /* local variables */
    /*******************/
    int textstate;
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;

    /********************/
    /* see text, states */
    /********************/
    if(kedGetEchoBufLen(kimed) == 0)
	textstate = KIM_NOTEXT;
    else
	textstate =  KIM_TEXTON;

    /*********************************/
    /* construct data to be returned */
    /*********************************/
    kedFix(kimed, &obj->output);
    makegetstring(obj); 
    *arg = obj->string;

    /**************************************/
    /* have them cleared, and do CALLBACK */
    /**************************************/
    maketextinfo(obj);
    makeauxinfo(obj);

    if(textstate == KIM_TEXTON) {
	if(obj->imobject.cb->texthide) {
	    if((*(obj->imobject.cb->texthide))(obj, 
			obj->imobject.udata) == IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
    }
    if(obj->auxstate & KIM_AUXNOW) {
	if(obj->imobject.cb->auxhide) {
	    if((*(obj->imobject.cb->auxhide))(obj, 
		       obj->auxid, obj->imobject.udata) == IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
	if(obj->imobject.cb->auxdestroy) {
	    if((*(obj->imobject.cb->auxdestroy))(obj, 
		       obj->auxid, obj->imobject.udata) == IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	    obj->auxidflag = FALSE;
	}
    }
    SAVEAUXSTATE(obj->auxstate);
    return(IMNoError);
} /* end of getstring */

/*-----------------------------------------------------------------------*
*	Beginning of clear
*-----------------------------------------------------------------------*/
int kimclear(obj, arg)
KIMOBJ 		obj;
caddr_t 	arg;
{
    /*******************/
    /* local variables */
    /*******************/
    int textstate;
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;

    /************************/
    /* see text, aux states */
    /************************/
    if(kedGetEchoBufLen(kimed) == 0)
	textstate = KIM_NOTEXT;
    else
	textstate =  KIM_TEXTON;

    /**************************************/
    /* have them cleared, and do CALLBACK */
    /**************************************/
    kedClear(kimed);
    maketextinfo(obj);
    makeauxinfo(obj);
    if(arg != NULL) { 
	if(textstate == KIM_TEXTON) {
	    if(obj->imobject.cb->texthide)
		if((*(obj->imobject.cb->texthide))
			(obj, obj->imobject.udata) 
				== IMError) {
		    obj->imobject.imfep->imerrno = 
				IMCallbackError;
		    return(IMError);
		}
	    }
	if(obj->auxstate & KIM_AUXNOW) {
	    if(obj->imobject.cb->auxhide) {
		if((*(obj->imobject.cb->auxhide))
			(obj, obj->auxid, obj->imobject.udata) 
				== IMError) {
		    obj->imobject.imfep->imerrno = 
				IMCallbackError;
		    return(IMError);
		}
	    }
	    if(obj->imobject.cb->auxdestroy) {
		if((*(obj->imobject.cb->auxdestroy))
			(obj, obj->auxid, obj->imobject.udata) 
				== IMError) {
		    obj->imobject.imfep->imerrno = 
				IMCallbackError;
		    return(IMError);
		    }
		obj->auxidflag = FALSE;
	    }
	}
    }
    SAVEAUXSTATE(obj->auxstate);
    return(IMNoError);
} /* end of kimclear */

/*-----------------------------------------------------------------------*
*	Beginning of reset
*-----------------------------------------------------------------------*/
int kimreset(obj, arg)
KIMOBJ obj;
caddr_t *arg;
{
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;

    /**********************************/
    /* reset aux area and do CALLBACK */
    /**********************************/
    kedControl(kimed, KP_RESETAUX);
    makeauxinfo(obj);
    if(arg != NULL) { 
	if(obj->auxstate & KIM_AUXNOW) {
	    if(obj->imobject.cb->auxhide) {
		if((*(obj->imobject.cb->auxhide))
			(obj, obj->auxid, obj->imobject.udata) 
			== IMError) {
		    obj->imobject.imfep->imerrno = 
			IMCallbackError;
		    return(IMError);
		}
	    }
	    if(obj->imobject.cb->auxdestroy) {
		if((*(obj->imobject.cb->auxdestroy))
			(obj, obj->auxid, obj->imobject.udata) 
				== IMError) {
		    obj->imobject.imfep->imerrno = 
			IMCallbackError;
		    return(IMError);
		}
		obj->auxidflag = FALSE;
	    }
	}
    }

    SAVEAUXSTATE(obj->auxstate);
    return(IMNoError);
} /* end of kimreset */

/*-----------------------------------------------------------------------*
*	Beginning of change length
*-----------------------------------------------------------------------*/
int kimchlen(obj, arg)
KIMOBJ 		obj;
caddr_t 	*arg;
{
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;

    /************************************************************/
    /* check passed parameter, change overflow limit when valid */
    /************************************************************/
    if((int)arg > obj->imobject.cb->textmaxwidth) {
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter);
    }
    else if((int)arg < kedGetEchoBufLen(kimed)) {
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter);
    }
    else 
	kedControl(kimed, KP_CHANGELEN, arg);
    return(IMNoError);
} /* end of kimchlen */


/*-----------------------------------------------------------------------*
*	Beginning of query state
*-----------------------------------------------------------------------*/
int kimq_state(obj, arg)
KIMOBJ obj;
IMQueryState *arg;
{
    /**********************************************************/
    /* construct state information within my structure, first */
    /**********************************************************/
    makequerystate(obj);
    /***************************************/
    /* return address of above information */
    /***************************************/
    arg->mode 		= obj->q_state.mode;
    arg->text 		= obj->q_state.text;
    arg->aux 		= obj->q_state.aux;
    arg->indicator 	= obj->q_state.indicator;
    arg->beep 		= obj->q_state.beep;
    return(IMNoError);
} /* end of kimq_state */

/*-----------------------------------------------------------------------*
*	Beginning of query text
*-----------------------------------------------------------------------*/
int kimq_text(obj, arg)
KIMOBJ obj;
IMQueryText *arg;
{
    arg->textinfo = &(obj->textinfo);
    return(IMNoError);
} /* end kim q text */

/*-----------------------------------------------------------------------*
*	Beginniny of query aux information 
*-----------------------------------------------------------------------*/
int kimq_aux(obj, arg)
KIMOBJ obj;
IMQueryAuxiliary *arg;
{
    makeauxinfo(obj);
    if(obj->auxidflag == TRUE && arg->auxid == obj->auxid) {
	arg->auxinfo = &(obj->auxinfo);
	return(IMNoError);
    }
    else {
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter);
    }
} /* end of kim query aux */

/*-----------------------------------------------------------------------*
*	Beginning of query indicator
*-----------------------------------------------------------------------*/
int kimq_indicator(obj, arg)
KIMOBJ obj;
IMQueryIndicator *arg;
{
    if(obj->q_state.mode == IMNormalMode)
	arg->indicatorinfo = &(obj->indinfo);
    else
	arg->indicatorinfo = NULL;
    return(IMNoError);
} /* end of kimq_indinfo */


int 		kimq_ind_str(obj, arg)
KIMOBJ 				obj;
IMQueryIndicatorString 		*arg;
{
    KIMED	*kimed;
    char *indstr = obj->indstr.str;

    kimed = (KIMED*)obj->kimed;
    /****************************/
    /* indicator of long format */
    /****************************/
    if(arg->format == IMLongForm) {
	    /*****************************************************/
	    /* first part is one of Alphanumeric, Hangul, Jamo 	 */
	    /*****************************************************/
	    if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_ALPHA) {
		memcpy(indstr, KIM_SJIND_ALPHA, sizeof(KIM_SJIND_ALPHA) - 1);
	    }
	    else if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_HANGUL) {
		memcpy(indstr, KIM_SJIND_HANGUL, 
			    sizeof(KIM_SJIND_HANGUL) - 1);
	    }
	    else  if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_JAMO) {
		memcpy(indstr, KIM_SJIND_JAMO, 
			    sizeof(KIM_SJIND_JAMO) - 1);
	    }
	    indstr = indstr + sizeof(KIM_SJIND_ALPHA) - 1;
	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth) {
		memcpy(indstr, KIM_SJIND_JEON, 
			    sizeof(KIM_SJIND_JEON) - 1);
	    }
	    else {
		memcpy(indstr, KIM_SJIND_BAN, 
			    sizeof(KIM_SJIND_BAN) - 1);
	    }
	    indstr = indstr + sizeof(KIM_SJIND_BAN) -1;
	    /********************************************/
	    /* third part is one of Insert/Replace byte */
	    /********************************************/
            if (kedGetEchoBufLen(kimed) == 0) {
			memcpy(indstr, KIM_SJIND_SPACE,
				sizeof(KIM_SJIND_SPACE) - 1);
		    	indstr = indstr + sizeof(KIM_SJIND_SPACE) -1;
			kedSetNeedIndRedraw(kimed);
	    } else {
		    if(obj->indinfo.insert == IMReplaceMode) {
			memcpy(indstr, KIM_SJIND_REP, 
				    sizeof(KIM_SJIND_REP) - 1);
		    }
		    else {
			memcpy(indstr, KIM_SJIND_INS, 
				    sizeof(KIM_SJIND_INS) - 1);
		    }
		    indstr = indstr + sizeof(KIM_SJIND_INS) -1;
	    }
	    /*******************************************/
	    /* the last part is one of Hanja On or Off */
	    /*******************************************/
	    if((obj->indinfo.unique &
			KIM_SH_HANJAMASK) == KIM_SH_HANJAON) {
		    memcpy(indstr, KIM_SJIND_RD, 
			sizeof(KIM_SJIND_RD) - 1);
	    }
	    else {
	    	    memcpy(indstr, KIM_SJIND_DSP, 
	    			sizeof(KIM_SJIND_DSP) - 1);
	    }
	    obj->indstr.len = KIM_SJD_LONGINDLEN;
    } /* end of long format  */
    else if(arg->format == IMShortForm) {
	    /*********************************************************/
            /* first part is one of Alphanumeric, Hangul, Jamo       */
	    /*********************************************************/
	    if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_ALPHA) {
		memcpy(indstr, KIM_SJIND_ALPHA, 
			    sizeof(KIM_SJIND_ALPHA) - 1);
	    }
	    else if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_HANGUL) {
		memcpy(indstr, KIM_SJIND_HANGUL, 
			    sizeof(KIM_SJIND_HANGUL) - 1);
	    }
	    else  if((obj->indinfo.unique & KIM_SH_MASK) 
			== KIM_SH_JAMO) {
		memcpy(indstr, KIM_SJIND_JAMO, 
			    sizeof(KIM_SJIND_JAMO) - 1);
	    }
	    indstr = indstr + sizeof(KIM_SJIND_ALPHA) - 1;
	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth) {
		memcpy(indstr, KIM_SJIND_JEON, 
			    sizeof(KIM_SJIND_JEON) - 1);
	    }
	    else {
		memcpy(indstr, KIM_SJIND_BAN, 
			    sizeof(KIM_SJIND_BAN) - 1);
	    }
	    indstr = indstr + sizeof(KIM_SJIND_BAN) -1;
            /********************************************/
            /* third part is one of Insert/Replace byte */
            /********************************************/
            if (kedGetEchoBufLen(kimed) == 0) {
                        memcpy(indstr, KIM_SJIND_SPACE,
                                sizeof(KIM_SJIND_SPACE) - 1);
                        indstr = indstr + sizeof(KIM_SJIND_SPACE) -1;
            } else {
                    if(obj->indinfo.insert == IMReplaceMode) {
                        memcpy(indstr, KIM_SJIND_REP,
                                    sizeof(KIM_SJIND_REP) - 1);
                    }
                    else {
                        memcpy(indstr, KIM_SJIND_INS,
                                    sizeof(KIM_SJIND_INS) - 1);
                    }
                    indstr = indstr + sizeof(KIM_SJIND_INS) -1;
            }
	    /*******************************************/
	    /* the last part is one of Hanja On or Off */
	    /*******************************************/
	    if((obj->indinfo.unique &
			KIM_SH_HANJAMASK) == KIM_SH_HANJAON) {
		    memcpy(indstr, KIM_SJIND_RD, 
			sizeof(KIM_SJIND_RD) - 1);
	    }
	    else {
	    	    memcpy(indstr, KIM_SJIND_DSP, 
	    			sizeof(KIM_SJIND_DSP) - 1);
	    }
	    obj->indstr.len = KIM_SJD_LONGINDLEN;
    } /* end of short format  */
    else { /* invalid format specified */
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter); 
    }

    /***********************/
    /* return successfully */
    /***********************/
    arg->indicator.len = obj->indstr.len;
    arg->indicator.str = obj->indstr.str;
    return(IMNoError);
} /* end of kimq_ind_str */



/*-----------------------------------------------------------------------*
*	Beginning of change mode
*-----------------------------------------------------------------------*/
int kimchmode(obj, arg)
KIMOBJ 		obj;
int 		arg;
{
    InputMode imode;
    KIMED    *kimed;

    kimed = (KIMED *)obj->kimed;


    /******************************/
    /* ask cuurent mode to editor */
    /******************************/
    imode = kedGetInputMode(kimed);

    /**********************/
    /* set to normal mode */
    /**********************/
    if(arg == IMNormalMode) {
	if(imode.supnormode != MD_NORM) {
	    /* set inputmode of editor */
	    imode.supnormode = MD_NORM;
	    kedSetInputMode(kimed, imode);

	    /* update indicator information */
	    imode = kedGetInputMode(kimed);
	    makeindinfo(obj);
	    makequerystate(obj);
	    if(obj->imobject.cb->indicatordraw) {
		if((*(obj->imobject.cb->indicatordraw))
		  (obj, &(obj->indinfo), obj->imobject.udata) 
			== IMError) {
		    obj->imobject.imfep->imerrno = 
				IMCallbackError;
		    return(IMError);
		}
	    }
	}
    }
    /**************************/
    /* set to suppressed mode */
    /**************************/
    else if(arg == IMSuppressedMode) {
	if(imode.supnormode != MD_SUPP) {
	    		/* set inputmode of editor */
	    		imode.supnormode = MD_SUPP;
	    		kedSetInputMode(kimed, imode);
	    		imode = kedGetInputMode(kimed);
	    		makeindinfo(obj);
	    		makequerystate(obj);
	    		if(obj->imobject.cb->indicatorhide) {
				if((*(obj->imobject.cb->indicatorhide))
					(obj, obj->imobject.udata) 
					== IMError) {
		    		obj->imobject.imfep->imerrno = 
						IMCallbackError;
		    		return(IMError);
				}
	    		}
			}
    		}
    /**************************/
    /* invaild mode specified */
    /**************************/
    else  /* invalid mode specified */
      return(IMError);
    
    return(IMNoError);
} /* end of imchangemode */

