static char sccsid[] = "@(#)10	1.6.1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMsupp.c, libKJI, bos411, 9428A410j 5/18/93 05:35:12";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMsupp
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
*	Beginning of making string
*-----------------------------------------------------------------------*/
int	makeoutputstring(obj)
JIMOBJ	obj;
{
	int	len;
	int	codeset;
	JIMFEP	fep;

	if (obj->output.len == 0)
		return 0;
	fep = (JIMFEP)obj->imobject.imfep;
	codeset = fep->codeset;
	if (codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
		(void)memcpy(obj->outstr, obj->output.data, obj->output.len); 
		len = obj->output.len;
	}
	else	/* EUC */
		len = SjisToEuc(fep->cd,
			obj->output.data, obj->outstr, obj->output.len,
			NULL, NULL, NULL);
	return len;
}

/*-----------------------------------------------------------------------*
*	Beginning of text informatin initialization 
*-----------------------------------------------------------------------*/
void	inittextinfo(obj)
JIMOBJ	obj;
{
	obj->textinfo.chglen = 0;
}

/*-----------------------------------------------------------------------*
*	Beginning of making text information
*-----------------------------------------------------------------------*/
void	maketextinfo(obj)
JIMOBJ	obj;
{
    extern EchoBufChanged jedIsEchoBufChanged();
    extern int jedGetEchoBufLen();
    extern int jedGetCurPos();
    int i;
    int len;
    int	codeset;
    JIMFEP	fep;
    unsigned int	cur, chgtop, chgbot;

    fep = (JIMFEP)obj->imobject.imfep;
    codeset = fep->codeset;
    cur = jedGetCurPos(obj->jedinfo.jeid);
    chgtop = jedIsEchoBufChanged(obj->jedinfo.jeid).chtoppos;
    chgbot = jedIsEchoBufChanged(obj->jedinfo.jeid).chlenbytes + chgtop;
    len = jedGetEchoBufLen(obj->jedinfo.jeid);
    /********************/
    /* code set is SJIS */
    /********************/
    if (codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
	/* copy string and attribute data */
	obj->textinfo.text.len = len;
	(void)memcpy(obj->textinfo.text.str, obj->jedinfo.echobufs, len);
	for(i = 0; i < len; i++) {
	    obj->textinfo.text.att[i] = IMAttHighlight;
	    if(obj->jedinfo.echobufa[i] & KP_HL_REVERSE)
		obj->textinfo.text.att[i] |= IMAttAttention;
	}
    }
    else /* code set is EUC */ {
	/* copy string and attribute data */
	len = SjisToEucAtt(fep->cd,
		obj->jedinfo.echobufs, obj->jedinfo.echobufa,
		obj->textinfo.text.str, obj->textinfo.text.att, len,
		&cur, &chgtop, &chgbot);
	obj->textinfo.text.len = len;
	for(i = 0; i < len; i++) {
	    if (obj->textinfo.text.att[i] & KP_HL_REVERSE)
		obj->textinfo.text.att[i] = IMAttHighlight | IMAttAttention;
	    else
		obj->textinfo.text.att[i] = IMAttHighlight;
	}
    }
    obj->textinfo.maxwidth = obj->imobject.cb->textmaxwidth;
    obj->textinfo.cur = cur;
    obj->textinfo.chgtop = chgtop;
    obj->textinfo.chglen = chgbot - chgtop;
}

/*-----------------------------------------------------------------------*
*	Beginning of making auxiliary information
*-----------------------------------------------------------------------*/
void	makeauxinfo(obj)
JIMOBJ	obj;
{
    /*******************/
    /* local variables */
    /*******************/
    extern AuxSize jedGetAuxSize();
    extern AuxCurPos jedGetAuxCurPos();
    extern int jedGetAuxType();
    extern int	jedHasSelection();
    void	makelistbox();
    AuxSize auxsize;
    AuxCurPos auxcurpos;
    int auxtype;
    int i, j;
    caddr_t *from, *froma;
    int	codeset;
    JIMFEP fep;
    unsigned int	curcol;

    if(obj->registration != NULL){
	return;
    }

    if(jedHasSelection(obj->jedinfo.jeid)){
	makelistbox(obj);
	return;
    }

    fep = (JIMFEP)obj->imobject.imfep;
    codeset = fep->codeset;
    /**********************************************/
    /* ask editor aux size and cursor information */
    /**********************************************/
    auxsize = jedGetAuxSize(obj->jedinfo.jeid);
    auxcurpos = jedGetAuxCurPos(obj->jedinfo.jeid);

    /***********************************************/
    /* fill code-set-independent information first */
    /***********************************************/
    obj->auxinfo.title.len = 0;     /* no title there */
    obj->auxinfo.title.str = NULL;   
    obj->auxinfo.selection.panel_row = 0; /* no panel there */
    obj->auxinfo.selection.panel_col = 0;
    obj->auxinfo.selection.panel = NULL;
    obj->auxinfo.button = IM_NONE;
    if(((auxtype = jedGetAuxType(obj->jedinfo.jeid)) == KP_AUX_ALLCAND) ||
       (auxtype == KP_AUX_KANJINO))
	obj->auxinfo.hint = IM_AtTheEvent;
    else
	obj->auxinfo.hint = IM_UpperRight;

    /* aux state  */
    if(obj->auxstate & JIM_AUXBEFORE) { /* there is aux before last IM call */
	if(obj->auxstate & JIM_AUXNOW) { /* there is aux after last IM call */
	    if(jedIsAuxBufChanged(obj->jedinfo.jeid))
		obj->auxinfo.status = IMAuxUpdated;
	    else
		obj->auxinfo.status = IMAuxShowing;
	}
	else
	    obj->auxinfo.status = IMAuxHidden;
    }
    else {  /* there is no aux before the last IM call */
	if(obj->auxstate & JIM_AUXNOW) /* there is aux after the last IM call */
	    obj->auxinfo.status = IMAuxShown;
	else
	    obj->auxinfo.status = IMAuxHiding;
    }

    /********************/
    /* code set is SJIS */
    /********************/
    if(codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
	/* copy string data */
	from = obj->jedinfo.auxbufs;
	for(i = 0; i < auxsize.itemnum; i++)  
	    (void)memcpy(obj->auxinfo.message.text[i].str,
		*from++, auxsize.itemsize);

	/* assuming there is only reverse attribute within aux */
	from = obj->jedinfo.auxbufa;
	for(i = 0; i < auxsize.itemnum; i++)  {
	    obj->auxinfo.message.text[i].len = auxsize.itemsize;
	    for(j = 0; j < auxsize.itemsize; j++) {
		if((*from)[j] & KP_HL_REVERSE)
		    obj->auxinfo.message.text[i].att[j] = IMAttAttention;
		else
		    obj->auxinfo.message.text[i].att[j] = IMAttNone;
	    }
	    from++;
	}
    }
    /*******************/
    /* code set is EUC */
    /*******************/
    else { 
	/* copy string data */
	from = obj->jedinfo.auxbufs;
	froma = obj->jedinfo.auxbufa;
	curcol = auxcurpos.colpos;
	for (i = 0; i < auxsize.itemnum; i++, from++, froma++) {
	    if (i == auxcurpos.rowpos)
		obj->auxinfo.message.text[i].len = SjisToEucAtt(fep->cd,
		    *from, *froma,
		    obj->auxinfo.message.text[i].str,
		    obj->auxinfo.message.text[i].att, auxsize.itemsize,
		    &curcol, NULL, NULL);
	    else
		obj->auxinfo.message.text[i].len = SjisToEucAtt(fep->cd,
		    *from, *froma,
		    obj->auxinfo.message.text[i].str,
		    obj->auxinfo.message.text[i].att, auxsize.itemsize,
		    NULL, NULL, NULL);
	}
	auxcurpos.colpos = curcol;

	/* assuming there is only reverse attribute within aux */
	for (i = 0; i < auxsize.itemnum; i++)  {
	    for (j = 0; j < obj->auxinfo.message.text[i].len; j++) {
		if (obj->auxinfo.message.text[i].att[j] & KP_HL_REVERSE)
		    obj->auxinfo.message.text[i].att[j] = IMAttAttention;
		else
		    obj->auxinfo.message.text[i].att[j] = IMAttNone;
	    }
	}
    }
    /* fill in another information */
    obj->auxinfo.message.maxwidth = auxsize.itemsize;
    obj->auxinfo.message.nline = auxsize.itemnum;
    if(auxcurpos.rowpos >= 0 && auxcurpos.colpos >= 0)  
	obj->auxinfo.message.cursor = TRUE;
    else 
	obj->auxinfo.message.cursor = FALSE;
    obj->auxinfo.message.cur_row = auxcurpos.rowpos;
    obj->auxinfo.message.cur_col = auxcurpos.colpos;
} /* end of makeauxinfo */

/*-----------------------------------------------------------------------*
*	Beginning of indicator information
*-----------------------------------------------------------------------*/
void	makeindinfo(obj)
JIMOBJ	obj;
{
    /*******************/
    /* local variables */
    /*******************/
    extern InputMode jedGetInputMode();
    InputMode imode;

    /***********************************************/
    /* retrieve input mode information from editor */
    /***********************************************/
    imode = jedGetInputMode(obj->jedinfo.jeid);

    /*********************************/
    /* fill in indicator information */
    /*********************************/
    if(imode.ind1 == KP_SINGLE)
	obj->indinfo.size = IMHalfWidth;
    else
	obj->indinfo.size = IMFullWidth;

    if(imode.ind4 == KP_INSERTMODE)
	obj->indinfo.insert = IMInsertMode;
    else
	obj->indinfo.insert = IMReplaceMode;

    if(imode.ind0 == KP_ALPHANUM)
	obj->indinfo.unique = JIM_SH_ALPHA;
    else if(imode.ind0 == KP_KATAKANA)
	obj->indinfo.unique = JIM_SH_KATA;
    else
	obj->indinfo.unique = JIM_SH_HIRA;
    if(imode.ind2 == KP_ROMAJI_OFF)
	obj->indinfo.unique |= JIM_SH_RKCOFF;
    else
	obj->indinfo.unique |= JIM_SH_RKCON;
} /* end of makeindicatorinfo */

/*-----------------------------------------------------------------------*
*	Beginning of make string for getstring 
*-----------------------------------------------------------------------*/
void	makegetstring(obj)
JIMOBJ	obj;
{
	int	codeset;
	JIMFEP	fep;

	if (obj->output.len == 0) {
		obj->string.len = 0;
		return;
	}
	fep = (JIMFEP)obj->imobject.imfep;
	codeset = fep->codeset;
	if (codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
		(void)memcpy(obj->string.str,
			obj->output.data, obj->output.len);
		obj->string.len = obj->output.len;
	}
	else	/* EUC */
		obj->string.len = SjisToEuc(fep->cd,
			obj->output.data, obj->string.str, obj->output.len,
			NULL, NULL, NULL);
}

/*-----------------------------------------------------------------------*
*	Beginning of make information for query state
*-----------------------------------------------------------------------*/
void	makequerystate(obj)
JIMOBJ	obj;
{
    extern InputMode jedGetInputMode();
    extern EchoBufChanged jedIsEchoBufChanged();
    extern int jedIsAuxBufChanged();
    extern int jedIsInputModeChanged();
    InputMode imode;

    /***********************************************/
    /* retrieve input mode information from editor */
    /***********************************************/
    imode = jedGetInputMode(obj->jedinfo.jeid);

    /*********************************/
    /* fill in query state structure */
    /*********************************/
    /* mode state */
    if(imode.ind3 == KP_NORMALMODE)
	obj->q_state.mode = IMNormalMode;
    else
	obj->q_state.mode = IMSuppressedMode;

    /* text state */
    obj->q_state.text =
	(unsigned int)jedIsEchoBufChanged(obj->jedinfo.jeid).flag;

    /* aux state  */
    obj->q_state.aux =
	(unsigned int)jedIsAuxBufChanged(obj->jedinfo.jeid);

    /* indicator state */
    obj->q_state.indicator =
	(unsigned int)jedIsInputModeChanged(obj->jedinfo.jeid);

    /* beep state */
    if(jedIsBeepRequested(obj->jedinfo.jeid))
	obj->q_state.beep = JIM_BEEPPER;
    else
	obj->q_state.beep = JIM_NOBEEP;

} /* end of makequerystate */

/*-----------------------------------------------------------------------*
*	make auxiliary information for list box
*-----------------------------------------------------------------------*/
void	makelistbox(obj)
JIMOBJ	obj;
{
    /*******************/
    /* local variables */
    /*******************/
    extern AuxSize jedGetAuxSize();
    extern AuxCurPos jedGetAuxCurPos();
    extern int jedGetAuxType();
    void	freelistbox2();
    AuxSize auxsize;
    AuxCurPos auxcurpos;
    int auxtype;
    int i, j;
    caddr_t *from, *froma;
    int	codeset;
    JIMFEP fep;
    IMPanel	*panelp, *jedGetPanel();
    char	*str;

    freelistbox2(obj);
    fep = (JIMFEP)obj->imobject.imfep;
    codeset = fep->codeset;
    panelp = jedGetPanel(obj->jedinfo.jeid);
    /**********************************************/
    /* ask editor aux size and cursor information */
    /**********************************************/
    auxsize = jedGetAuxSize(obj->jedinfo.jeid);
    auxcurpos = jedGetAuxCurPos(obj->jedinfo.jeid);

    /***********************************************/
    /* fill code-set-independent information first */
    /***********************************************/
    obj->auxinfo.title.len = 20;
    obj->auxinfo.title.str = malloc(40);
    obj->auxinfo.selection.panel_row = 1;
    obj->auxinfo.selection.panel_col = 1;
    obj->auxinfo.selection.panel = malloc(sizeof(IMPanel));
    obj->auxinfo.selection.panel->item =
		 malloc(sizeof(IMItem) * (panelp->item_row));
    obj->auxinfo.selection.panel->maxwidth = panelp->maxwidth;
    obj->auxinfo.selection.panel->item_row = panelp->item_row;
    obj->auxinfo.selection.panel->item_col = panelp->item_col;
    str = malloc(panelp->maxwidth * panelp->item_row * 2);
    obj->auxinfo.button = IM_OK | IM_CANCEL | IM_NEXT | IM_PREV;
    if(((auxtype = jedGetAuxType(obj->jedinfo.jeid)) == KP_AUX_ALLCAND) ||
       (auxtype == KP_AUX_KANJINO))
	obj->auxinfo.hint = IM_AtTheEvent;
    else
	obj->auxinfo.hint = IM_UpperRight;

    /* aux state  */
    if(obj->auxstate & JIM_AUXBEFORE) { /* there is aux before last IM call */
	if(obj->auxstate & JIM_AUXNOW) { /* there is aux after last IM call */
	    if(jedIsAuxBufChanged(obj->jedinfo.jeid))
		obj->auxinfo.status = IMAuxUpdated;
	    else
		obj->auxinfo.status = IMAuxShowing;
	}
	else
	    obj->auxinfo.status = IMAuxHidden;
    }
    else {  /* there is no aux before the last IM call */
	if(obj->auxstate & JIM_AUXNOW) /* there is aux after the last IM call */
	    obj->auxinfo.status = IMAuxShown;
	else
	    obj->auxinfo.status = IMAuxHiding;
    }

    /********************/
    /* code set is SJIS */
    /********************/
    if(codeset == JIM_SJISMIX || codeset == JIM_SJISDBCS) {
	/* copy string data */
	strcpy(obj->auxinfo.title.str, jedGetTitle(obj->jedinfo.jeid));
	obj->auxinfo.title.len = strlen(obj->auxinfo.title.str);
	for(i = 0; i < panelp->item_row; i++) {
	    obj->auxinfo.selection.panel->item[i].text.str = str;
	    (void)memcpy(str, panelp->item[i].text.str, panelp->maxwidth);
	    obj->auxinfo.selection.panel->item[i].text.len = panelp->maxwidth;
	    obj->auxinfo.selection.panel->item[i].selectable = TRUE;
	    str += panelp->maxwidth;
	}
    }
    /*******************/
    /* code set is EUC */
    /*******************/
    else { 
	/* copy string data */
	obj->auxinfo.title.len =
		SjisToEuc(fep->cd, jedGetTitle(obj->jedinfo.jeid),
			obj->auxinfo.title.str,
			strlen(jedGetTitle(obj->jedinfo.jeid)),
			NULL, NULL, NULL);
	for (i = 0; i < panelp->item_row; i++) {
	    obj->auxinfo.selection.panel->item[i].text.str = str;
	    obj->auxinfo.selection.panel->item[i].text.len
		= SjisToEuc(fep->cd,
		    panelp->item[i].text.str, str, panelp->item[i].text.len,
		    NULL, NULL, NULL);
	    obj->auxinfo.selection.panel->item[i].selectable = TRUE;
	    str += obj->auxinfo.selection.panel->item[i].text.len;
	}
    }
    /* fill in another information */
    obj->auxinfo.message.maxwidth = auxsize.itemsize;
    obj->auxinfo.message.nline = 0;
    if(auxcurpos.rowpos >= 0 && auxcurpos.colpos >= 0)  
	obj->auxinfo.message.cursor = TRUE;
    else 
	obj->auxinfo.message.cursor = FALSE;
    obj->auxinfo.message.cur_row = -1;
    obj->auxinfo.message.cur_col = -1;
} /* end of makelistbox */

/*-----------------------------------------------------------------------*
*	free auxiliary information for list box (jfep)
*-----------------------------------------------------------------------*/
static	void	freelistbox2(obj)
JIMOBJ	obj;
{
	if(obj->auxinfo.selection.panel != NULL) {
		if(obj->auxinfo.selection.panel->item[0].text.str != NULL)
			free(obj->auxinfo.selection.panel->item[0].text.str);
		if(obj->auxinfo.selection.panel->item != NULL)
			free(obj->auxinfo.selection.panel->item);
		free(obj->auxinfo.selection.panel);
		obj->auxinfo.selection.panel = NULL;
	}
	if(obj->auxinfo.title.str != NULL)
		free(obj->auxinfo.title.str);
}

/*-----------------------------------------------------------------------*
*	free auxiliary information for list box (jfep and jed)
*-----------------------------------------------------------------------*/
void	freelistbox(obj)
JIMOBJ	obj;
{
	jedFreeListbox(obj->jedinfo.jeid);
	freelistbox2(obj);
}
