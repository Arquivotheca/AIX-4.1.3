static char sccsid[] = "@(#)62  1.2  src/bos/usr/lib/nls/loc/imk/kfep/KIMsupp.c, libkr, bos411, 9428A410j 7/21/92 00:22:13";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		KIMsupp.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
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
 *  Module:       KIMsupp.c
 *
 *  Description:  Korean Input Method various support routines
 *
 *  Functions:    makeoutputstring()    
 *		  inittextinfo()
 *		  maketextinfo()
 *		  makeauxinfo()
 * 		  makeindinfo()
 *		  makegetstring()
 * 		  makequerystate()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include "kfep.h"
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	Beginning of making string
*-----------------------------------------------------------------------*/

int makeoutputstring(obj)
KIMOBJ obj;
{
    /**********************/
    /*			  */
    /* External Reference */
    /*			  */
    /**********************/
    extern   void 	memcpy();
    register int 	len=0;

    if (obj->output.len == 0) 
    {
       return (0);
    }
    /*************************************/
    /*				 	 */
    /* copy string and return its length */
    /*				 	 */
    /*************************************/
    memcpy(obj->outstr, obj->output.data, obj->output.len); 
    len = obj->output.len;  /* return output string length */
    return (len);
} /* end of makeoutputstring */

/*-----------------------------------------------------------------------*
*	Beginning of text informatin initialization 
*-----------------------------------------------------------------------*/
void inittextinfo(obj)
KIMOBJ obj;
{
    	obj->textinfo.chglen = KIM_TXT_NOCHANGED_LEN;
}

/*-----------------------------------------------------------------------*
*	Beginning of making text information
*-----------------------------------------------------------------------*/
void maketextinfo(obj)
KIMOBJ obj;
{
    /*******************/
    /* local variables */
    /*******************/
    extern EchoBufChanged	kedGetChangeInfoEchoBuf();
    extern int 			kedIsEchoBufChanged();
    extern int 			kedGetEchoBufLen();
    extern int 			kedGetCurPos();
    extern void 		memcpy();
    register int 		i;
    register int 		len;
    EchoBufChanged		chinfo;
    KIMED			*kimed;

        kimed = (KIMED*) obj->kimed;
	/* copy string and attribute data */
	len = kedGetEchoBufLen(kimed);
	/*
	 * These informations are used by IMCallback function.
	 * textinfo.text.str and textinfo.text.att[i] and ......
	 */
	obj->textinfo.text.len = len;
	memcpy(obj->textinfo.text.str, kimed->echobufs, len);
	for(i = 0; i < len; i++) {
	    obj->textinfo.text.att[i] = IMAttHighlight;
	    if(kimed->echobufa[i] & KP_HL_REVERSE)
		obj->textinfo.text.att[i] |= IMAttAttention;
	}
	/* fill in another information */
	obj->textinfo.maxwidth = len;
	obj->textinfo.cur = kedGetCurPos(kimed);
	chinfo = kedGetChangeInfoEchoBuf(kimed);
	obj->textinfo.chgtop =  chinfo.chtoppos;
	obj->textinfo.chglen =  chinfo.chlenbytes;
} /* end of maketextinfo */

/*-----------------------------------------------------------------------*
*	Beginning of making auxiliary information
*-----------------------------------------------------------------------*/
void makeauxinfo(obj)
KIMOBJ obj;
{
    extern AuxSize 	kedGetAuxSize();
    extern AuxCurPos 	kedGetAuxCurPos();
    extern int 		kedGetAuxType();
    extern 		void memcpy();
    /*******************/
    /* local variables */
    /*******************/
    KIMED		*kimed;
    AuxSize 		auxsize;
    AuxCurPos 		auxcurpos;
    int 		auxtype;
    register int 	i, j, k;
    caddr_t 		*froms;
    caddr_t 		*froma;
    int			rsz, rsz_strlen, ix, ix_strlen;
    unsigned char	rsz_str[50], ix_str[50];
    unsigned char 	*rsz_ti, *ix_ti;
    int			maxitemlen=-999, tmp=0;
    int 		len;

    kimed = (KIMED*) obj->kimed;

    obj->auxinfo.title.len = 0 ;  /* no title */
    obj->auxinfo.title.str = NULL ;

	i=0;
	auxtype = kedGetAuxType(kimed);
	if (auxtype == AUXBUF_NOTUSED) {
		return;
	}	
	else if (auxtype == MULTICAND_USED) { 
     		/*
		 ******************************************************
		 * When Hangul to Hanja conversion, the auxbuf used to 
		 * by candidates, thus the title is changed to some 
		 * chars. When Code input, also.
     		 ******************************************************
		 */	
		obj->auxinfo.message.cursor = KIM_MSG_CUROFF; 

		/**************************/
		/* Copys title to message */
		/**************************/
		obj->auxinfo.message.text[i].len = TI_ALLCAND1_LEN;
		memcpy(obj->auxinfo.message.text[i].str, TI_ALLCAND1, TI_ALLCAND1_LEN);
		memset(obj->auxinfo.message.text[i].att, IMAttNone, TI_ALLCAND1_LEN); 
		i++;

		/**************************/
		/* draws remaining size.  */
		/**************************/
		if (kimed->candsize <= kimed->auxsize.itemnum)
			rsz = 0;
		else {
		  if (kimed->candcrpos+10 > kimed->candsize) {
		     rsz = 0;
		  } else {
		     tmp = kimed->candcrpos + 10;
		     if (tmp <= kimed->candsize)
			rsz = kimed->candsize - tmp;
		     else
			rsz = kimed->candcrpos + 1;
		  }
		}
		itoa(rsz, rsz_str, &rsz_strlen);
		rsz_ti = (unsigned char*) atks(rsz_str, rsz_strlen); 
		if (rsz_strlen == 1)
		{
			memcpy(rsz_ti+2, "  " , 2) ;
			rsz_strlen += 1 ;
		}

		obj->auxinfo.message.text[i].len = TI_REMAINSZ_LEN+(rsz_strlen*2);
		memcpy(obj->auxinfo.message.text[i].str, TI_REMAINSZ, TI_REMAINSZ_LEN);
		memcpy(obj->auxinfo.message.text[i].str+14, rsz_ti, (rsz_strlen*2)+1);	
		memset(obj->auxinfo.message.text[i].att, IMAttNone, 
			TI_REMAINSZ_LEN+(rsz_strlen*2)+2) ; 
		i++;
		free(rsz_ti);

		/**************************/
		/* Copys title to message */
		/**************************/
		obj->auxinfo.message.text[i].len = TI_ALLCAND2_LEN;
		memcpy(obj->auxinfo.message.text[i].str, TI_ALLCAND2, TI_ALLCAND2_LEN);
		memset(obj->auxinfo.message.text[i].att, IMAttNone, 
			TI_ALLCAND2_LEN); 
		i++;	
		obj->auxinfo.hint = IM_AtTheEvent;
	}
	else {
		/*******************************/
		/* (auxtype == CODEINPUT_USED) */
		/*******************************/
		obj->auxinfo.message.cursor = KIM_MSG_CURON; 

    		/**********************************************/
    		/* ask editor aux size and cursor information */
    		/**********************************************/
    		auxcurpos = kedGetAuxCurPos(kimed);
		obj->auxinfo.message.cur_row = auxcurpos.rowpos ;
		obj->auxinfo.message.cur_col = auxcurpos.colpos + TI_CODEINP_LEN - 8 ;
		/**************************/
		/* Copys title to message */
		/**************************/
		obj->auxinfo.message.text[i].len = 
			TI_CODEINP_LEN;
		memcpy(obj->auxinfo.message.text[i].str, 
			TI_CODEINP, TI_CODEINP_LEN);
		memset(obj->auxinfo.message.text[i].att, 
			IMAttNone, TI_CODEINP_LEN); 
		obj->auxinfo.message.maxwidth = KIM_CODEINP_WINWIDTH;	

		obj->auxinfo.hint = IM_AtTheEvent;
	}
	/**************************/
	/* Copys title to message */
	/**************************/
	/*
	 * The bellow info are not used to Korean inputmethod.
	 */
    	obj->auxinfo.selection.panel_row = KIM_NOPANEL_ROW; 
    	obj->auxinfo.selection.panel_col = KIM_NOPANEL_COL;
    	obj->auxinfo.selection.panel = KIM_NOPANEL;
    	obj->auxinfo.button = IM_NONE;
    /*
     * The auxinfo.status not used to Callback routines.
     * The auxwindow creation or deletion depends upon obj->auxstate.
     */
    if(obj->auxstate & KIM_AUXBEFORE) { /* there is aux before last IM call */
	if(obj->auxstate & KIM_AUXNOW) { /* there is aux after last IM call */
	    if(kedIsAuxBufChanged(kimed))
		obj->auxinfo.status = IMAuxUpdated;
	    else
		obj->auxinfo.status = IMAuxShowing;
	}
	else
	    obj->auxinfo.status = IMAuxHidden;
    }
    else {  /* there is no aux before the last IM call */
	if(obj->auxstate & KIM_AUXNOW) 
		/* there is aux after the last IM call */
	    obj->auxinfo.status = IMAuxShown;
	else
	    obj->auxinfo.status = IMAuxHiding;
    }
/*
 * Our implementations are different from the Japen's method.
 * Our a candbuf's terminated by NULL character. This is more
 * convenient. The candidates string length are  can different.
 * Each candidates don't have to itemsize. 
 */

	
	froms = kimed->auxbufs;
	froma = kimed->auxbufa;
	/*
	 * This size is that contained candidate by auxbuf(itemnum).
  	 */
	auxsize = kedGetAuxSize(kimed);
	for(ix = 0; ix < auxsize.itemnum; ix++, i++)  {
		k = strlen(*froms);
		/* assuming there is only reverse attribute within aux */
		for(j = 0; j < k; j++) {
			if((*froma)[j] & KP_HL_REVERSE)
			   obj->auxinfo.message.text[i].att[j] = IMAttAttention;
			else
			   obj->auxinfo.message.text[i].att[j] = IMAttNone;
		}
		/*
		 * Null character copyed to interface, auxinfo.message ....
		 */	
		if (auxtype == MULTICAND_USED) {
			itoa(ix, ix_str, &ix_strlen);
			ix_ti = (unsigned char*) atks(ix_str, ix_strlen); 
			obj->auxinfo.message.text[i].len = obj->auxinfo.message.maxwidth ;
			memcpy(obj->auxinfo.message.text[i].str, ix_ti, ix_strlen*2);	
			free(ix_ti);
		        memcpy( obj->auxinfo.message.text[i].str+(ix_strlen*2), 
					"\241\241\241\241", 4);
			memcpy( obj->auxinfo.message.text[i].str+((ix_strlen*2)+4),
					*froms++, k+1);
			obj->auxinfo.message.text[i].len = k+6 ;
			{
				int	j ;
				for ( j = (ix_strlen*2)+4+k ; j < obj->auxinfo.message.maxwidth ; j++)
				{
				memcpy(obj->auxinfo.message.text[i].str+j, " ", 1) ;
				obj->auxinfo.message.text[i].len += 1 ;
				}
			}
		}
		else {
			char pad[7] = {' ', ' ', ' ', ' ', ' ', ' ', ' '};
/******* 13 : code input indicator length ***/
			memcpy(obj->auxinfo.message.text[i].str + 13, *froms++, k);
/*
			if (k<7) memcpy(obj->auxinfo.message.text[i].str + 13 + k, pad, 7-k) ;
*/
			obj->auxinfo.message.text[i].len = obj->auxinfo.message.maxwidth;
		}
	}

	/* fill in another information */
	if (auxtype == CODEINPUT_USED) {
		obj->auxinfo.message.maxwidth = KIM_CODEINP_WINWIDTH;	
		obj->auxinfo.message.nline = KIM_CODEINP_WINMSGNUM + ix; 
	}
	else  {
		for (i=0; i < kimed->candsize; i++)
		{
		  if ((len=strlen(kimed->candbuf[i])) > maxitemlen)
		  {
		     maxitemlen = len;
		  }
		}

                for (k=ix+3; k<13;k++)
		{
		    int	j ;
		    for(j = 0 ; j < obj->auxinfo.message.maxwidth ; j++)
				memcpy(obj->auxinfo.message.text[k].str+j, " ", 1) ;
                    obj->auxinfo.message.text[k].len = obj->auxinfo.message.maxwidth ;
		}


		if (KIM_TITL2_ALLCAND >= maxitemlen) 
	        {
		  obj->auxinfo.message.maxwidth = KIM_TITL2_ALLCAND + 6;
	 	}
		else
		{
		  obj->auxinfo.message.maxwidth = (KIM_TITL1_ALLCAND+maxitemlen)+3;	
		}
		obj->auxinfo.message.nline = KIM_WINMSGNUM_ALLCAND + 10; 
	}
} /* end of makeauxinfo */

/*-----------------------------------------------------------------------*
*	Beginning of indicator information
*-----------------------------------------------------------------------*/
void makeindinfo(obj)
KIMOBJ obj;
{
    /*******************/
    /* local variables */
    /*******************/
    extern InputMode 	kedGetInputMode();
    InputMode 		imode;
    KIMED		*kimed;

	kimed = (KIMED*) obj->kimed;
    	/***********************************************/
    	/* retrieve input mode information from editor */
    	/***********************************************/
    	imode = kedGetInputMode(kimed);

    	/*********************************/
    	/* fill in indicator information */
    	/*********************************/
	switch(imode.sizemode) {
		case MD_BANJA:
			obj->indinfo.size = IMHalfWidth;
			break;
		case MD_JEONJA:
		default:
			obj->indinfo.size = IMFullWidth;
	}
	switch(imode.insrepmode) {
		case MD_INSERT:
			obj->indinfo.insert = IMInsertMode;
			break;
		case MD_REPLACE:
		default:
			obj->indinfo.insert = IMReplaceMode;
			break;
	}
	switch(imode.basemode) {
		case MD_ENG:
			obj->indinfo.unique = KIM_SH_ALPHA;
			break;
		case MD_HAN:
			obj->indinfo.unique = KIM_SH_HANGUL;
			break;
		case MD_JAMO:
			obj->indinfo.unique = KIM_SH_JAMO;
			break;
	}
	switch(imode.hjmode) {
		case MD_HJOFF:
			obj->indinfo.unique |= KIM_SH_HANJAOFF;
			break;
		case MD_HJON:
		default:
			obj->indinfo.unique |= KIM_SH_HANJAON;
	}
} /* end of makeindicatorinfo */

/*-----------------------------------------------------------------------*
*	Beginning of make string for getstring 
*-----------------------------------------------------------------------*/
void makegetstring(obj)
KIMOBJ obj;
{
    /*******************/
    /* local variables */
    /*******************/
    KIMED 	*kimed;
    extern 	void memcpy();
    extern 	int kedGetFixBuflen();
    register 	int len;

    if (obj->output.len == 0) 
    {
       obj->string.len = 0;
       return;
    }
    memcpy(obj->string.str, obj->output.data, obj->output.len); 
    obj->string.len = obj->output.len;
} /* end of makegetstring */

/*-----------------------------------------------------------------------*
*	Beginning of make information for query state
*-----------------------------------------------------------------------*/
void makequerystate(obj)
KIMOBJ obj;
{
    extern InputMode kedGetInputMode();
    extern int kedIsEchoBufChanged();
    extern int kedIsAuxBufChanged();
    extern int kedIsInputModeChanged();
    InputMode imode;
    KIMED       *kimed;

    kimed = (KIMED*) obj->kimed;

    /***********************************************/
    /* retrieve input mode information from editor */
    /***********************************************/
    imode = kedGetInputMode(kimed);

    /*********************************/
    /* fill in query state structure */
    /*********************************/
    /* mode state */
    if(imode.supnormode == MD_NORM)
	obj->q_state.mode = IMNormalMode;
    else
	obj->q_state.mode = IMSuppressedMode;

    /* text state */
    obj->q_state.text = (uint)kedIsEchoBufChanged(kimed);

    /* aux state  */
    obj->q_state.aux = (uint)kedIsAuxBufChanged(kimed);

    /* indicator state */
    obj->q_state.indicator = (uint)kedIsInputModeChanged(kimed);

    /* beep state */
    if(kedIsBeepRequested(kimed))
	obj->q_state.beep = KIM_BEEPPER;
    else
	obj->q_state.beep = KIM_NOBEEP;

} /* end of makequerystate */
