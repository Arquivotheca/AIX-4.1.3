static char sccsid[] = "@(#)09	1.5.1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMctlsub.c, libKJI, bos411, 9428A410j 5/18/93 05:34:53";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMctlsub
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
#include <string.h>
#include "imjimP.h"		/* Japanese Input Method header file */

/*-----------------------------------------------------------------------*
*	external references
*-----------------------------------------------------------------------*/
extern int     jetGetEchoBufLen();
extern AuxSize jedGetAuxSize();
extern int     jedFix();
extern int     jedClear();
extern int     jedControl();
extern EchoBufChanged     jedIsEchoBufChanged();
extern int     jedIsAuxBufChanged();
extern int     jedIsInputModeChanged();
extern int     jedIsBeepRequested();
extern InputMode jedGetInputMode();
extern int     jedSetInputMode();
extern int     jedSetString();

/*-----------------------------------------------------------------------*
*	Beginning of refresh
*-----------------------------------------------------------------------*/
int	jimrefresh(obj, arg)
JIMOBJ	obj;
caddr_t	arg;
{
	AuxSize	auxsize;

	/*
	 *	call Callbacks depending upon the states
	 */
	if (obj->q_state.mode == IMNormalMode &&
		obj->imobject.cb->indicatordraw) {
		if ((*(obj->imobject.cb->indicatordraw))(obj,
			&(obj->indinfo), obj->imobject.udata) == IMError) {
			obj->imobject.imfep->imerrno = IMCallbackError;
			return(IMError);
		}
	}

	if (jedGetEchoBufLen(obj->jedinfo.jeid) > 0 &&
		obj->imobject.cb->textdraw) {
		/*
		 *	On 'refreshing', indicate entire text has been
		 *	changed.
		 */
		unsigned int	chgtop, chglen;

		chgtop = obj->textinfo.chgtop;
		chglen = obj->textinfo.chglen;
		obj->textinfo.chgtop = 0;
		obj->textinfo.chglen = obj->textinfo.text.len;
		if ((*(obj->imobject.cb->textdraw))(obj,
			&(obj->textinfo), obj->imobject.udata) == IMError) {
			obj->imobject.imfep->imerrno = IMCallbackError;
			return(IMError);
		}
		obj->textinfo.chgtop = chgtop;
		obj->textinfo.chglen = chglen;
	}

	auxsize = jedGetAuxSize(obj->jedinfo.jeid);
	if (obj->auxstate & JIM_AUXNOW && obj->imobject.cb->auxdraw) {
		if ((*(obj->imobject.cb->auxdraw))(obj, obj->auxid,
			&(obj->auxinfo), obj->imobject.udata) == IMError) {
			obj->imobject.imfep->imerrno = IMCallbackError;
			return(IMError);
		}
	}

	return(IMNoError);
}

/*-----------------------------------------------------------------------*
*	Beginning of getstring
*-----------------------------------------------------------------------*/
int jimgetstr(obj, arg)
JIMOBJ obj;
IMSTR *arg;
{
    /*******************/
    /* local variables */
    /*******************/
    int textstate;

    /********************/
    /* see text, states */
    /********************/
    if(obj->registration != NULL || jedGetEchoBufLen(obj->jedinfo.jeid) == 0)
	textstate = JIM_NOTEXT;
    else
	textstate =  JIM_TEXTON;

    /*********************************/
    /* construct data to be returned */
    /*********************************/
    jedFix(obj->jedinfo.jeid, &obj->output);
    makegetstring(obj);
    if(textstate == JIM_NOTEXT){
        obj->string.len = 0;
    }
    *arg = obj->string;

    /**************************************/
    /* have them cleared, and do CALLBACK */
    /**************************************/
    maketextinfo(obj);
    makeauxinfo(obj);

    if(textstate == JIM_TEXTON) {
	if(obj->imobject.cb->texthide) {
	    if((*(obj->imobject.cb->texthide))(obj,
			obj->imobject.udata) == IMError) {
		obj->imobject.imfep->imerrno = IMCallbackError;
		return(IMError);
	    }
	}
    }
    if(obj->auxstate & JIM_AUXNOW) {
	freelistbox(obj);
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
int jimclear(obj, arg)
JIMOBJ obj;
caddr_t arg;
{
    /*******************/
    /* local variables */
    /*******************/
    int textstate;

    /************************/
    /* see text, aux states */
    /************************/
    if(obj->registration != NULL || jedGetEchoBufLen(obj->jedinfo.jeid) == 0)
	textstate = JIM_NOTEXT;
    else
	textstate =  JIM_TEXTON;

    /**************************************/
    /* have them cleared, and do CALLBACK */
    /**************************************/
    jedClear(obj->jedinfo.jeid);
    maketextinfo(obj);
    makeauxinfo(obj);
    freelistbox(obj);
    if(arg != NULL) { /* do we need to do CALLBACK(s) ? */
	if(textstate == JIM_TEXTON) {
	    if(obj->imobject.cb->texthide) {
		if((*(obj->imobject.cb->texthide))(obj,
				obj->imobject.udata) == IMError) {
		    obj->imobject.imfep->imerrno = IMCallbackError;
		    return(IMError);
		}
	    }
	}
	if(obj->auxstate & JIM_AUXNOW) {
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
    }
    SAVEAUXSTATE(obj->auxstate);
    obj->output.len = 0;
    return(IMNoError);
} /* end of jimclear */

/*-----------------------------------------------------------------------*
*	Beginning of reset
*-----------------------------------------------------------------------*/
int jimreset(obj, arg)
JIMOBJ obj;
caddr_t *arg;
{
    /**********************************/
    /* reset aux area and do CALLBACK */
    /**********************************/
    jedControl(obj->jedinfo.jeid, KP_RESETAUX, arg);
    makeauxinfo(obj);
    freelistbox(obj);
    if(arg != NULL) { /* do we need to do CALLBACK(s) ? */
	if(obj->auxstate & JIM_AUXNOW) {
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
    }
    SAVEAUXSTATE(obj->auxstate);
    return(IMNoError);
} /* end of jimreset */

/*-----------------------------------------------------------------------*
*	Beginning of change length
*-----------------------------------------------------------------------*/
int jimchlen(obj, arg)
JIMOBJ obj;
caddr_t *arg;
{
    /************************************************************/
    /* check passed parameter, change overflow limit when valid */
    /************************************************************/
    if((int)arg > obj->imobject.cb->textmaxwidth) {
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter);
    }
    else if((int)arg < jedGetEchoBufLen(obj->jedinfo.jeid)) {
	obj->imobject.imfep->imerrno = IMCallbackError;
	return(IMInvalidParameter);
    }
    else
	jedControl(obj->jedinfo.jeid, KP_CHANGELEN, arg);
    return(IMNoError);
} /* end of jimchlen */

/*-----------------------------------------------------------------------*
*	Beginning of query state
*-----------------------------------------------------------------------*/
int jimq_state(obj, arg)
JIMOBJ obj;
IMQueryState *arg;
{
    /**********************************************************/
    /* construct state information within my structure, first */
    /**********************************************************/
    makequerystate(obj);

    /***************************************/
    /* return address of above information */
    /***************************************/
    arg->mode = obj->q_state.mode;
    arg->text = obj->q_state.text;
    arg->aux = obj->q_state.aux;
    arg->indicator = obj->q_state.indicator;
    arg->beep = obj->q_state.beep;

    return(IMNoError);
} /* end of jimq_state */

/*-----------------------------------------------------------------------*
*	Beginning of query text
*-----------------------------------------------------------------------*/
int jimq_text(obj, arg)
JIMOBJ obj;
IMQueryText *arg;
{
    arg->textinfo = &(obj->textinfo);
    return(IMNoError);
} /* end jim q text */

/*-----------------------------------------------------------------------*
*	Beginniny of query aux information
*-----------------------------------------------------------------------*/
int jimq_aux(obj, arg)
JIMOBJ obj;
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
} /* end of jim query aux */

/*-----------------------------------------------------------------------*
*	Beginning of query indicator
*-----------------------------------------------------------------------*/
int jimq_indicator(obj, arg)
JIMOBJ obj;
IMQueryIndicator *arg;
{
    if(obj->q_state.mode == IMNormalMode)
	arg->indicatorinfo = &(obj->indinfo);
    else
	arg->indicatorinfo = NULL;
    return(IMNoError);
} /* end of jimq_indinfo */

/*-----------------------------------------------------------------------*
*	Beginning of query indicator string
*-----------------------------------------------------------------------*/
int jimq_ind_str(obj, arg)
JIMOBJ obj;
IMQueryIndicatorString *arg;
{
    char *indstr;
    int  len;			/* EUC Text Length 			*/
    int	codeset;
    JIMFEP	fep;

    fep = (JIMFEP)obj->imobject.imfep;
    codeset = fep->codeset;
    indstr = obj->indstr.str;
    /****************************/
    /* indicator of long format */
    /****************************/
    if(arg->format == IMLongForm) {
	/********************/
	/* code set is SJIS */
	/********************/
	if(codeset == JIM_SJISDBCS || codeset == JIM_SJISMIX) {
	    /*********************************************************/
	    /* first part is one of Alphanumeric, Katakana, Hiragana */
	    /*********************************************************/
	    if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_ALPHA) {
		memcpy(indstr, JIM_SJIND_ALPHA, sizeof(JIM_SJIND_ALPHA) - 1);
	    }
	    else if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_KATA) {
		memcpy(indstr, JIM_SJIND_KATA, sizeof(JIM_SJIND_KATA) - 1);
	    }
	    else  {
		memcpy(indstr, JIM_SJIND_HIRA, sizeof(JIM_SJIND_HIRA) - 1);
	    }
	    indstr = indstr + sizeof(JIM_SJIND_HIRA) - 1;

	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth) {
		memcpy(indstr, JIM_SJIND_ZEN, sizeof(JIM_SJIND_ZEN) - 1);
	    }
	    else {
		memcpy(indstr, JIM_SJIND_HAN, sizeof(JIM_SJIND_HAN) - 1);
	    }
	    indstr = indstr + sizeof(JIM_SJIND_HAN) -1;

	    /*****************************************/
	    /* the last part is one of RKC ON or OFF */
	    /*****************************************/
	    if(codeset == JIM_SJISDBCS) {
		if((obj->indinfo.unique & JIM_SH_RKCMASK) == JIM_SH_RKCON) {
		    memcpy(indstr, JIM_SJIND_RD, sizeof(JIM_SJIND_RD) - 1);
		}
		else {
		    memcpy(indstr, JIM_SJIND_DSP, sizeof(JIM_SJIND_DSP) - 1);
		}
	        indstr = indstr + sizeof(JIM_SJIND_RD) -1;
	    }
	    else { /* SJIS mix */
		if((obj->indinfo.unique & JIM_SH_RKCMASK) & JIM_SH_RKCON) {
		    memcpy(indstr, JIM_SJIND_RS, sizeof(JIM_SJIND_RS) - 1);
		}
		else {
		    memcpy(indstr, JIM_SJIND_SSP, sizeof(JIM_SJIND_SSP) - 1);
		}
	        indstr = indstr + sizeof(JIM_SJIND_RS) -1;
	    }
	    obj->indstr.len = indstr - obj->indstr.str;
	} /* end of if SJIS */
	/*******************/
	/* code set is EUC */
	/*******************/
	else  { /* EUC */
	    /*********************************************************/
	    /* first part is one of Alphanumeric, Katakana, Hiragana */
	    /*********************************************************/
	    if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_ALPHA)
		len = SjisToEuc(fep->cd, JIM_SJIND_ALPHA,
			indstr, sizeof(JIM_SJIND_ALPHA)-1, NULL, NULL, NULL);
	    else if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_KATA)
		len = SjisToEuc(fep->cd, JIM_SJIND_KATA,
			indstr, sizeof(JIM_SJIND_KATA)-1, NULL, NULL, NULL);
	    else
		len = SjisToEuc(fep->cd, JIM_SJIND_HIRA,
			indstr,sizeof(JIM_SJIND_HIRA)-1, NULL, NULL, NULL);
	    indstr = indstr + len;

	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth)
		len = SjisToEuc(fep->cd, JIM_SJIND_ZEN,
			indstr,sizeof(JIM_SJIND_ZEN)-1, NULL, NULL, NULL);
	    else
		len = SjisToEuc(fep->cd, JIM_SJIND_HAN,
			indstr,sizeof(JIM_SJIND_HAN)-1, NULL, NULL, NULL);
    	    indstr = indstr + len;

	    /*****************************************/
	    /* the last part is one of RKC ON or OFF */
	    /*****************************************/
	    if((obj->indinfo.unique & JIM_SH_RKCMASK) & JIM_SH_RKCON)
	        len = SjisToEuc(fep->cd, JIM_SJIND_RS,
			indstr,sizeof(JIM_SJIND_RS)-1, NULL, NULL, NULL);
	    else
	        len = SjisToEuc(fep->cd, JIM_SJIND_SSP,
			indstr,sizeof(JIM_SJIND_SSP)-1, NULL, NULL, NULL);
    	    indstr = indstr + len;
	    obj->indstr.len = indstr - obj->indstr.str;
	} /* end of if EUC  */
    } /* end of long format  */

    /*****************************/
    /* indicator of short format */
    /*****************************/
    else if(arg->format == IMShortForm) {
	/********************/
	/* code set is SJIS */
	/********************/
	if(codeset == JIM_SJISDBCS || codeset == JIM_SJISMIX) {
	    /*********************************************************/
	    /* first part is one of Alphanumeric, Katakana, Hiragana */
	    /*********************************************************/
	    if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_ALPHA) {
		memcpy(indstr, JIM_SJSIND_ALPHA, sizeof(JIM_SJSIND_ALPHA) - 1);
	    }
	    else if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_KATA) {
		memcpy(indstr, JIM_SJSIND_KATA, sizeof(JIM_SJSIND_KATA) - 1);
	    }
	    else  {
		memcpy(indstr, JIM_SJSIND_HIRA, sizeof(JIM_SJSIND_HIRA) - 1);
	    }
	    indstr = indstr + sizeof(JIM_SJSIND_HIRA) - 1;

	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth) {
		memcpy(indstr, JIM_SJSIND_ZEN, sizeof(JIM_SJSIND_ZEN) - 1);
	    }
	    else {
		memcpy(indstr, JIM_SJSIND_HAN, sizeof(JIM_SJSIND_HAN) - 1);
	    }
	    indstr = indstr + sizeof(JIM_SJSIND_HAN) -1;

	    /*****************************************/
	    /* the last part is one of RKC ON or OFF */
	    /*****************************************/
	    if(codeset == JIM_SJISDBCS) {
		if((obj->indinfo.unique & JIM_SH_RKCMASK) == JIM_SH_RKCON) {
		    memcpy(indstr, JIM_SJSIND_RD, sizeof(JIM_SJSIND_RD) - 1);
		}
		else {
		    memcpy(indstr, JIM_SJSIND_DSP, sizeof(JIM_SJSIND_DSP) - 1);
		}
	        indstr = indstr + sizeof(JIM_SJIND_RD) -1;
	    }
	    else { /* SJIS mix */
		if((obj->indinfo.unique & JIM_SH_RKCMASK) & JIM_SH_RKCON) {
		    memcpy(indstr, JIM_SJSIND_RS, sizeof(JIM_SJSIND_RS) - 1);
		}
		else {
		    memcpy(indstr, JIM_SJSIND_SSP, sizeof(JIM_SJSIND_SSP) - 1);
		}
	        indstr = indstr + sizeof(JIM_SJIND_RS) -1;
	    }
	    obj->indstr.len = indstr - obj->indstr.str;
	} /* end of if SJIS */

	/*******************/
	/* code set is EUC */
	/*******************/
	else { /* that is, EUC */
	    /*********************************************************/
	    /* first part is one of Alphanumeric, Katakana, Hiragana */
	    /*********************************************************/
	    if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_ALPHA)
	        len = SjisToEuc(fep->cd, JIM_SJSIND_ALPHA,
			indstr,sizeof(JIM_SJSIND_ALPHA)-1, NULL, NULL, NULL);
	    else if((obj->indinfo.unique & JIM_SH_MASK) == JIM_SH_KATA)
	        len = SjisToEuc(fep->cd, JIM_SJSIND_KATA,
			indstr,sizeof(JIM_SJSIND_KATA)-1, NULL, NULL, NULL);
	    else
	        len = SjisToEuc(fep->cd, JIM_SJSIND_HIRA,
			indstr,sizeof(JIM_SJSIND_HIRA)-1, NULL, NULL, NULL);
	    indstr = indstr + len ;

	    /********************************************/
	    /* second part is one of Single/Double byte */
	    /********************************************/
	    if(obj->indinfo.size == IMFullWidth)
		len = SjisToEuc(fep->cd, JIM_SJSIND_ZEN,
			indstr,sizeof(JIM_SJSIND_ZEN)-1, NULL, NULL, NULL);
	    else
		len = SjisToEuc(fep->cd, JIM_SJSIND_HAN,
			indstr,sizeof(JIM_SJSIND_HAN)-1, NULL, NULL, NULL);
	    indstr = indstr + len ;

	    /*****************************************/
	    /* the last part is one of RKC ON or OFF */
	    /*****************************************/
	    if((obj->indinfo.unique & JIM_SH_RKCMASK) & JIM_SH_RKCON)
	        len = SjisToEuc(fep->cd, JIM_SJSIND_RS,
			indstr,sizeof(JIM_SJSIND_RS)-1, NULL, NULL, NULL);
	    else
	        len = SjisToEuc(fep->cd, JIM_SJSIND_SSP,
			indstr,sizeof(JIM_SJSIND_SSP)-1, NULL, NULL, NULL);
    	    indstr = indstr + len;
	    obj->indstr.len = indstr - obj->indstr.str;
	} /* end of if EUC */
    } /* end of short format */
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
} /* end of jimq_ind_str */

/*-----------------------------------------------------------------------*
*	Beginning of change mode
*-----------------------------------------------------------------------*/
int jimchmode(obj, arg)
JIMOBJ obj;
int arg;
{
    /*******************/
    /* local variables */
    /*******************/
    InputMode imode;

    /******************************/
    /* ask cuurent mode to editor */
    /******************************/
    imode = jedGetInputMode(obj->jedinfo.jeid);

    /**********************/
    /* set to normal mode */
    /**********************/
    if(arg == IMNormalMode) {
	if(imode.ind3 != KP_NORMALMODE) {
	    /* set inputmode of editor */
	    imode.ind3 = KP_NORMALMODE;
	    jedSetInputMode(obj->jedinfo.jeid, imode);

	    /* update indicator information */
	    imode = jedGetInputMode(obj->jedinfo.jeid);
	    makeindinfo(obj);
	    makequerystate(obj);
	    if(obj->imobject.cb->indicatordraw) {
		if((*(obj->imobject.cb->indicatordraw))(obj,
			  &(obj->indinfo), obj->imobject.udata) == IMError) {
		    obj->imobject.imfep->imerrno = IMCallbackError;
		    return(IMError);
		}
	    }
	}
    }
    /**************************/
    /* set to suppressed mode */
    /**************************/
    else if(arg == IMSuppressedMode) {
	jimclear(obj, 1);
	if(imode.ind3 != KP_SUPPRESSEDMODE) {
	    /* set inputmode of editor */
	    imode.ind3 = KP_SUPPRESSEDMODE;
	    jedSetInputMode(obj->jedinfo.jeid, imode);

	    /* update indicator information */
	    imode = jedGetInputMode(obj->jedinfo.jeid);
	    makeindinfo(obj);
	    makequerystate(obj);
	    if(obj->imobject.cb->indicatorhide) {
		if((*(obj->imobject.cb->indicatorhide))(obj,
			      obj->imobject.udata) == IMError) {
		    obj->imobject.imfep->imerrno = IMCallbackError;
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

/*-----------------------------------------------------------------------*
*	Beginning of set string
*-----------------------------------------------------------------------*/
int jimsetstr(obj, arg)
JIMOBJ obj;
IMSTR *arg;  /* pointer to IMstring to be set */
{
    int	codeset;

    codeset = ((JIMFEP)(obj->imobject.imfep))->codeset;
    /*******************/
    /* code set is EUC */
    /*******************/
    if(codeset == JIM_EUC) {
	/*
	 *	Not implemented
	 *  make SJIS string from passed string
	 *  call kpSetString(kpid, str, len)
	 */
	return(IMNoError);
    }
    /********************/
    /* code set is SJIS */
    /********************/
    else {
	if((jedSetString(obj->jedinfo.jeid, arg->str, arg->len) == KP_OK)) {
	    maketextinfo(obj);
	    if((jedGetEchoBufLen(obj->jedinfo.jeid) > 0) &&
		       (jedIsEchoBufChanged(obj->jedinfo.jeid).flag)) {
		if(obj->imobject.cb->textdraw) {
		    if((*(obj->imobject.cb->textdraw))(obj,
		       &(obj->textinfo), obj->imobject.udata) == IMError) {
			obj->imobject.imfep->imerrno = IMCallbackError;
			return(IMError);
		    }
                }
	    }
	    return(IMNoError);
        }
	else
	    return(IMError);
    }
}


/*-----------------------------------------------------------------------*
*	Change Indicator
*-----------------------------------------------------------------------*/
#include <X11/keysym.h>

int	jimchangeind(obj, arg)
JIMOBJ		obj;	/* pointer to JIM object */
IMIndicatorInfo	*arg;	/* pointer to IMstring to be set */
{
	IMQueryIndicator	qind;
	char			str[256];
	int			len;

	/*
	 *	Get Current Indicator Information
	 */
	jimq_indicator(obj, &qind);
	if (qind.indicatorinfo == NULL) {
		obj->imobject.imfep->imerrno = IMInvalidParameter;
		return(IMError);
	}

	/*
	 *	Change Half/Full-width
	 */
	if (arg->size & JIM_WIDTH_FLAG) {
		if ((arg->size & 0x07) != qind.indicatorinfo->size) {
			JIMFilter(obj, XK_Zenkaku_Hankaku, 0, str, &len);
		}
	}

	/*
	 *	Set Alpha/Kana
	 */
	if (arg->unique & JIM_ALPHA_FLAG) {
		if ((arg->unique & 0x07) !=
				(qind.indicatorinfo->unique & 0x07)) {
			switch (arg->unique & 0x07) {
			case JIM_SH_HIRA:
				JIMFilter(obj, XK_Hiragana, 0, str, &len);
				break;
			case JIM_SH_KATA:
				JIMFilter(obj, XK_Katakana, 0, str, &len);
				break;
			default:
				JIMFilter(obj, XK_Eisu_toggle, 0, str, &len);
			}
		}
	}

	/*
	 *	Set RKC-on/off
	 */
	if (arg->unique & JIM_RKC_FLAG) {
		if ((arg->unique & 0x08) !=
				(qind.indicatorinfo->unique & 0x08)) {
			JIMFilter(obj, XK_Romaji, 0, str, &len);
		}
	}

	return(IMNoError);
}
