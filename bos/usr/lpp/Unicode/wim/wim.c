static char sccsid[] = "@(#)64	1.2  src/bos/usr/lpp/Unicode/wim/wim.c, cfgnls, bos411, 9428A410j 3/4/94 10:16:42";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: WIMAuxCreate
 *		WIMAuxDestroy
 *		WIMAuxDraw
 *		WIMAuxHide
 *		WIMBeep
 *		WIMClose
 *		WIMCreate
 *		WIMDestroy
 *		WIMFilter
 *		WIMIndicatorDraw
 *		WIMIndicatorHide
 *		WIMInitialize
 *		WIMIoctl
 *		WIMLookup
 *		WIMProcess
 *		WIMProcessAuxiliary
 *		WIMQueryAuxiliary
 *		WIMQueryIndicatorString
 *		WIMRefresh
 *		WIMTextCursor
 *		WIMTextDraw
 *		WIMTextHide
 *		WIMTextStart
 *		clist_fill
 *		find_clist
 *		find_loc
 *		get_data_recs
 *		get_locpath
 *		iconv_auxinfo
 *		iconv_len
 *		iconv_str
 *		iconv_stratt
 *		iconv_textinfo
 *		imstr_append
 *		imstr_copy
 *		imstratt_append
 *		imstratt_copy
 *		menu_clear
 *		menu_fill
 *		menu_filter
 *		menu_hide
 *		menu_init
 *		menu_process_auxiliary
 *		menu_reorder
 *		menu_show
 *		menu_supp_selection
 *		parse_clistline
 *		parse_locline
 *		preedit_commit
 *		preedit_init
 *		read_config
 *		wim_filter
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "wim.h"

/************************************************************************
 * WIM Callbacks
 ************************************************************************/

static int WIMTextDraw(IMObjectRec *obj, IMTextInfo *textinfo, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;
    ObjDataRec *orec;
    FepDataRec *frec;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    /* If in a WIM state don't allow child IM to preedit text. */
    if ((textinfo != &(wobj->textinfo)) && (wobj->state & WIM_STATE)) {
	return IMCallbackError;
    }

    if (get_data_recs(wobj, &frec, &orec) == IMError) {
	return IMCallbackError;
    }

    /* NOTE: I might not want to share the textinfo structure. Each child
     * IM might need to have it's own textinfo data structure. Otherwise,
     * I should make sure that only one IM (including the WIM), is using
     * the textinfo structure at a time. */

    /* Convert only if this is not called by the WIM. */
    if (textinfo != &(wobj->textinfo)) {
	if (iconv_textinfo(frec->cd, textinfo, &(wobj->textinfo), 
	    &(wobj->textinfo_siz))) {
	    return IMCallbackError;
	}
    }

    return wobj->common.cb->textdraw(wobj, &(wobj->textinfo), 
	wobj->common.udata);
}

static int WIMTextHide(IMObjectRec *obj, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->texthide(wobj, wobj->common.udata);
}
 
static int WIMTextStart(IMObjectRec *obj, int *space, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* If we are in a WIM special state, don't allow preeding by child IM. */
    if (wobj->state & WIM_STATE) {
	return IMCallbackError;
    }

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->textstart(wobj, space, wobj->common.udata);
}
 
static int WIMTextCursor(IMObjectRec *obj, uint_t direction, int *cur, 
    caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->textcursor(wobj, direction, cur, 
	    wobj->common.udata);
}
 
static int WIMAuxCreate(IMObjectRec *obj, caddr_t *auxidp, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->auxcreate(wobj, auxidp, wobj->common.udata);
}
 
static int WIMAuxDraw(IMObjectRec *obj, caddr_t auxid, IMAuxInfo *auxinfo,
	caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;
    ObjDataRec *orec;
    FepDataRec *frec;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    /* If this is for one of the WIM's menus ... */
    if (wobj->auxid == auxid) {
	/* No conversion necessary. */
	return wobj->common.cb->auxdraw(wobj, auxid, auxinfo, 
	    wobj->common.udata);
    }

    else { /* This is from the child IM. */
	if (get_data_recs(wobj, &frec, &orec) == IMError) {
	    /* NOTE: Might want to call application's callback anyway. */
	    return IMCallbackError;
	}

	/* Try to convert all strings in auxiliary. Change or copy other
	 * fields, as necessary. */
	if (iconv_auxinfo(frec->cd, auxinfo, &(wobj->auxinfo), 
	    &(wobj->auxinfo_siz), &(wobj->auxinfo_nline), 
	    &(wobj->auxinfo_nitem))) {
	    /* NOTE: Might want to call application's callback anyway. */
	    return IMCallbackError;
	}

	return wobj->common.cb->auxdraw(wobj, auxid, &(wobj->auxinfo), 
	    wobj->common.udata);
    }

}
 
static int WIMAuxHide(IMObjectRec *obj, caddr_t auxid, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->auxhide(wobj, auxid, wobj->common.udata);
}
 
static int WIMAuxDestroy(IMObjectRec *obj, caddr_t auxid, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->auxdestroy(wobj, auxid, wobj->common.udata);
}
 
static int WIMIndicatorDraw(IMObjectRec *obj, IMIndicatorInfo *indicatorinfo, 
	caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->indicatordraw(wobj, indicatorinfo, 
	wobj->common.udata);
}
 
static int WIMIndicatorHide(IMObjectRec *obj, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->indicatorhide(wobj, wobj->common.udata);
}
 
static int WIMBeep(IMObjectRec *obj, int percent, caddr_t udata)
{
    WIMObjectRec *wobj = (WIMObjectRec *)udata;

    /* Do this in case child input method calls callback during IMCreate. */
    wobj->objtbl[wobj->curloc].obj = obj;

    return wobj->common.cb->beep(wobj, percent, wobj->common.udata);
}
 
/************************************************************************
 * WIM IMFep Functions
 ************************************************************************/

WIMFepRec *WIMInitialize(IMLanguage language) 
{
    int i;
    WIMFepRec *fep;
    char **labels;
    int blen;
    int clen;
    char *pathname;

    /* Allocate WIMFepRec. */
    if (!(fep = (WIMFepRec *)calloc(1, sizeof(WIMFepRec)))) {
	return NULL;
    }

    /* MAXPATHLEN defined in /usr/include/sys/param.h */
    pathname = malloc(MAXPATHLEN);

    /* NOTE: These tables are stored in the FEP, instead of being kept global
     * to allow different processes and users to have different customized
     * versions of them. */

    get_locpath(language, IMCFG_SUFFIX, pathname);
    read_config(fep, pathname);

    if (fep->numlocs == 0) {
	/* Make sure there is at lease one IM. */
	fep->feptbl[0] = DefFepRec;
	fep->numlocs = 1;
    }

    if (fep->numclists == 0) {
	/* Make sure there is at lease one clist. */
	ClistRec *clist;
	CrangeRec *range;
	/* Default clist for ASCII */
	clist = &(fep->clisttbl[0]);
	clist->chars = NULL;
	clist->charbuf = NULL;
	strcpy(clist->label, "ASCII");
	clist->ranges = (CrangeRec *)malloc(1 * sizeof(CrangeRec));
	range = &(clist->ranges[0]);
	range->first = 0x0020;
	range->last = 0x007e;
	clist->numranges = 1;
	menu_init(&(clist->menu), clist->label, CLIST_NUM_COLS);
	fep->numclists = 1;
    }

    free(pathname);

    i = find_loc(fep, language);

    if (i >= fep->numlocs) {
	/* Use first locale by default. */
	i = 0;
    }

    /* Allocate a FEP for the current (default) locale. */
    if (!(fep->feptbl[i].fep = IMInitialize(fep->feptbl[i].imlocale))) { 
	i = find_loc(fep, "C");
	if (i >= fep->numlocs) {
	    /* Use first locale by default. */
	    i = 0;
	    fep->feptbl[i] = DefFepRec;
	    fep->feptbl[i].fep = IMInitialize(fep->feptbl[i].imlocale);
	}
    }

    fep->defloc = i;

    fep->common.iminitialize = (IMFep (*)())WIMInitialize;
    fep->common.imclose = WIMClose;
    fep->common.imcreate = (IMObject (*)())WIMCreate;
    fep->common.imdestroy = WIMDestroy;
    fep->common.improcess = WIMProcess;
    fep->common.improcessaux = WIMProcessAuxiliary;
    fep->common.imioctl = WIMIoctl;
    fep->common.imfilter = WIMFilter;
    fep->common.imlookup = WIMLookup;

    /* Create master copy of the locale menu. */
    /* NOTE: Have to use same number of columns for all menus, apparently. */
    menu_init(&(fep->locmenu), "Input Methods", 1);
    /* NOTE: read_config could create this table directly. */
    fep->loclabels = (char **)malloc(fep->numlocs * sizeof(char *));
    for (i = 0; i < fep->numlocs; i++) {
	fep->loclabels[i] = fep->feptbl[i].description;
    }

    /* NOTE: This is a hack to make sure that locale labels are always at least
     * the width of 16 spaces. XIM bug causes the width of items in
     * row-column to be the width of the first item. */
    clen = mbslen(fep->loclabels[0]);
    blen = strlen(fep->loclabels[0]) + 1; /* Add 1 for terminal NULL. */
    if (clen < 16) {
	/* Append nulls only up to the minimum total 16 characters or 
	 * MAX_CFG_FIELD bytes. */
	strncat(fep->loclabels[0], "                ", 
	    MIN((16 - clen), (MAX_CFG_FIELD - blen)));
    }

    menu_fill(&(fep->locmenu), fep->loclabels, fep->numlocs);

    /* Create master copy of the clist menu. */
    /* NOTE: Have to use same number of columns for all menus, apparently. */
    menu_init(&(fep->clistmenu), "Char Lists", 1);
    /* NOTE: read_clists could create this table directly. */
    fep->clistlabels = (char **)malloc(fep->numclists * sizeof(char *));
    for (i = 0; i < fep->numclists; i++) {
	fep->clistlabels[i] = fep->clisttbl[i].label;
    }

    /* NOTE: This is a hack to make sure that clist labels are always at least
     * the width of 16 spaces. XIM bug causes the width of items in
     * row-column to be the width of the first item. */
    clen = mbslen(fep->clistlabels[0]);
    blen = strlen(fep->clistlabels[0]) + 1; /* Add 1 for terminal NULL. */
    if (clen < 16) {
	/* Append nulls only up to the minimum total 16 characters or 
	 * MAX_CFG_FIELD bytes. */
	strncat(fep->clistlabels[0], "                ", 
	    MIN((16 - clen), (MAX_CFG_FIELD - blen)));
    }

    menu_fill(&(fep->clistmenu), fep->clistlabels, fep->numclists);

    return fep;
}

static WIMObjectRec *WIMCreate(WIMFepRec *fep, IMCallback *cb, caddr_t udata)
{
    WIMObjectRec *obj; 
    ObjDataRec *orec;
    FepDataRec *frec;

    /* Allocate WIMObjetRec. */
    if (!(obj = (WIMObjectRec *)calloc(1, sizeof(WIMObjectRec)))) {
	return NULL;
    }

    /* Initialize wimcb structure. */
    obj->wimcb.textmaxwidth = cb->textmaxwidth;
    obj->wimcb.textdraw = WIMTextDraw;
    obj->wimcb.texthide = WIMTextHide;
    obj->wimcb.textstart = WIMTextStart;
    obj->wimcb.textcursor = WIMTextCursor;
    obj->wimcb.auxcreate = WIMAuxCreate;
    obj->wimcb.auxdraw = WIMAuxDraw;
    obj->wimcb.auxhide = WIMAuxHide;
    obj->wimcb.auxdestroy = WIMAuxDestroy;
    obj->wimcb.indicatordraw = WIMIndicatorDraw;
    obj->wimcb.indicatorhide = WIMIndicatorHide;
    obj->wimcb.beep = WIMBeep;

    /* NOTE: I am not checking if malloc or calloc fail. They shouldn't 
     * fail. If I need to check, I can check all pointers at the end. */

    /* Allocate IMObject Table. Since we use calloc, everything is already 
     * initialized to 0. */
    obj->objtbl = (ObjDataRec *)calloc(fep->numlocs, sizeof(ObjDataRec));
    obj->numlocs = fep->numlocs;
    obj->curloc = fep->defloc;

    /* Copy menu structures to Obj record. The actual menu
     * data are not copied. Only the pointer to the data is copied. */
    obj->locmenu = fep->locmenu;
    obj->clistmenu = fep->clistmenu;

    /* Initialize curmenu field. */
    obj->curmenu = NULL;
    obj->auxid = NULL;

    /* Allocate buffer for storing misc temporary string data. */
    obj->str.str = (uchar_t *)malloc(obj->str_siz = 256);
    obj->str.len = 0;

    /* Allocate buffer for storing IMFilter string data. */
    obj->filtstr.str = (uchar_t *)malloc(obj->filtstr_siz = 256);
    obj->filtstr.len = 0;

    /* Allocate indicator string buffer. */
    obj->indstr_siz = 128;
    obj->indstr.str = malloc(obj->indstr_siz);

    /* Allocate panel for auxinfo structure. */
    /* NOTE: Currently limited to one panel. */
    obj->auxinfo.selection.panel = (IMPanel *)malloc(sizeof(IMPanel));

    /* Initialize fields. */
    obj->common.imfep = (IMFepRec *)fep;
    obj->common.cb = cb;
    obj->common.mode = IMNormalMode;
    obj->common.udata = udata;
    obj->state = BASE_STATE;

    /* Allocate an IM Obj (and FEP, if necessary) for current locale. */
    /* If this fails, have to fail everything, because ALWAYS must have an
     * active valid child IM. */
    if (get_data_recs(obj, &frec, &orec) != IMNoError) {
	(void)WIMDestroy(obj);
	return NULL;
    }

    obj->textinfo_siz = 128;
    obj->textinfo.text.str = malloc(obj->textinfo_siz);
    obj->textinfo.text.att = malloc(obj->textinfo_siz);

    /* Call First Callback. This is necessary to inform the application that
     * this input method will use callbacks. */
    if (!cb || (WIMIndicatorDraw(orec->obj, &(obj->indinfo), obj) != 
	IMNoError)) {
	(void)WIMDestroy(obj);
	fep->common.imerrno = IMCallbackError;
	return NULL;
    }

    return obj;
}
	
static void WIMDestroy(WIMObjectRec *obj)
{
    int i;

    free(obj->str.str);
    free(obj->filtstr.str);
    free(obj->indstr.str);
    free(obj->textinfo.text.str);
    free(obj->textinfo.text.att);

    /* Free space for lines in auxinfo. */
    for (i = 0; i < obj->auxinfo_nline; i++) {
	free(obj->auxinfo.message.text[i].str);
	free(obj->auxinfo.message.text[i].att);
    }
    free(obj->auxinfo.message.text);

    /* Free space for items in auxinfo. */
    /* NOTE: Assuming only one panel. */
    for (i = 0; i < obj->auxinfo_nitem; i++) {
	free(obj->auxinfo.selection.panel[0].item[i].text.str);
    }
    free(obj->auxinfo.selection.panel[0].item);
    free(obj->auxinfo.selection.panel);

    for (i = 0; i < obj->numlocs; i++) {
	if (obj->objtbl[i].obj) {
	    IMDestroy(obj->objtbl[i].obj);
	}
    }
    free(obj->objtbl);

    free(obj);
}

static int WIMFilter(WIMObjectRec *obj, uint_t keysym, uint_t state,
    caddr_t *str, uint_t *len)
{
    ObjDataRec *orec;
    FepDataRec *frec;
    IMSTR str1;
    int rc;

    if (get_data_recs(obj, &frec, &orec) == IMError) {
	*str = "";
	*len = 0;
	return IMInputUsed;
    }

    /* Call IMFilter for the child IM.
     * Always do this first, because WIM may not know how to interpret
     * keysyms correctly in certain environments. */
    rc = IMFilter(orec->obj, keysym, state, &(str1.str), &(str1.len));
    
    /* If child IM returned a string, convert it. */
    if (str1.len) {
	if (iconv_str(frec->cd, &(str1), &(obj->filtstr), 
	    &(obj->filtstr_siz))) {
	    obj->filtstr.len = 0;
	}
    }
    else { 
	obj->filtstr.len = 0;
    }

    /* If input was not used by child IM and not in a special child IM
     * call wim_filter. */
    if ((rc != IMInputUsed) && !(obj->state & CHILD_STATE)) {
	rc = wim_filter(obj, keysym, state, &(obj->filtstr), 
	    &(obj->filtstr_siz), orec, frec);
    }

    *str = obj->filtstr.str;
    *len = obj->filtstr.len;

    return (rc);
}

static int WIMLookup(WIMObjectRec *obj, uint_t keysym, uint_t state,
	caddr_t *str, uint_t *len)
{
    ObjDataRec *orec;
    FepDataRec *frec;
    IMSTR str1;
    int rc;

    if (get_data_recs(obj, &frec, &orec) == IMError) {
	*str = "";
	*len = 0;
	return IMReturnNothing;
    }

    /* Call IMLookup for the current IM. */
    rc = IMLookupString(orec->obj, keysym, state, &(str1.str), &(str1.len));
    if (!(str1.len)) {
	*str = "";
	*len = 0;
	return rc;
    }
    
    if (iconv_str(frec->cd, &(str1), &(obj->filtstr), &(obj->filtstr_siz))) {
	*str = "";
	*len = 0;
	return IMReturnNothing;
    }

    *str = obj->filtstr.str;
    *len = obj->filtstr.len;

    /* Otherwise, just pass the the string and rc back to the app. */

    return (rc);
}

static int WIMProcess(WIMObjectRec *obj, uint_t keysym, uint_t state,
	caddr_t *str, uint_t *len)
{
    /* NOTE: This might not generate the correct return values for 
     * IMProcess. */
    if (WIMFilter(obj, keysym, state, str, len) == IMInputNotUsed) {
	if (WIMLookup(obj, keysym, state, str, len) == IMReturnNothing) {
	    return IMTextAndAuxiliaryOff;
	}
	else {
	    return IMTextOn;
	}
    }
    else {
	if (*len) {
	    return IMTextAndAuxiliaryOn;
	}
	else {
	    return IMAuxiliaryOn;
	}
    }
}

static int WIMProcessAuxiliary(WIMObjectRec *obj, caddr_t auxid, uint_t button,
	uint_t panel_row, uint_t panel_col, uint_t item_row, uint_t item_col,
	caddr_t *str, uint_t *str_len)
{
    WIMFepRec *fep = (WIMFepRec *)obj->common.imfep;
    ObjDataRec *orec;
    FepDataRec *frec;
    ObjDataRec *neworec;
    FepDataRec *newfrec;
    int oldloc;
    int selection;
    int len; 

    /* WIM doesn't yet use these. */
    *str = NULL;
    *str_len = 0;

    if (get_data_recs(obj, &frec, &orec) == IMError) {
	return IMError;
    }

    if (obj->state & WIM_STATE) {

	switch(obj->state) {

	case LOCSEL_STATE:
	    if (menu_process_auxiliary(&(obj->locmenu), button, panel_row,
		panel_col, item_row, item_col, &selection)) {
		return IMError;
	    }
	    else if (selection == -1) {
		return IMError;
	    }
	    else {
		/* A valid selection was returned by 
		 * menu_process_auxiliary.  */
		/* Try to load this locale. */
		oldloc = obj->curloc;
		obj->curloc = selection;
		if (get_data_recs(obj, &newfrec, &neworec) != IMError) {
		    orec = neworec;
		    frec = newfrec;
		    /* Change default locale for Fep. */
		    fep->defloc = obj->curloc;
		    /* Draw indicator for new locale. */
		    WIMIndicatorDraw(orec->obj, &(obj->indinfo), obj);
		}
		else {
		    obj->curloc = oldloc;
		    WIMBeep(orec->obj, 50, obj);
		}
		if (!menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		    obj->state = BASE_STATE;
		}
	    }
	    break;

	case CLSEL_STATE:
	    if (menu_process_auxiliary(&(obj->clistmenu), button, panel_row,
		panel_col, item_row, item_col, &selection)) {
		return IMError;
	    }
	    else if (selection == -1) {
		return IMError;
	    }
	    else {
		/* A valid selection was returned by 
		 * menu_process_auxiliary.  */
		fep->clist = &(fep->clisttbl[selection]);
		if (!menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		    obj->state = CSEL_STATE;
		    /* Put up clist. */
		    if (!(fep->clist->chars)) {
			clist_fill(fep->clist, obj->supp_selection);
		    }
		    if (menu_show(&(fep->clist->menu), fep, obj, 
			orec->obj)) {
			/* menu_show failed. */
			obj->state = BASE_STATE;
		    }
		    else { /* menu_show succeeded. */
			preedit_init(obj, orec);
		    }
		}
	    }
	    break; /* CLSEL_STATE. */

	case CSEL_STATE:
	    if (menu_process_auxiliary(&(fep->clist->menu), button, panel_row,
		panel_col, item_row, item_col, &selection)) {
		return IMError;
	    }
	    else if (selection == -1) {
		return IMError;
	    }
	    else {
		/* A valid selection was returned by 
		 * menu_process_auxiliary.  */
		len = mblen(fep->clist->chars[selection], MB_CUR_MAX);
		obj->textinfo.cur += len;
		obj->textinfo.chgtop += len;
		obj->textinfo.chglen = len;
		imstratt_append(fep->clist->chars[selection],
		    mblen(fep->clist->chars[selection], MB_CUR_MAX),
		    IMAttAttention, &(obj->textinfo.text), 
		    &(obj->textinfo_siz));
		WIMTextDraw(orec->obj, &(obj->textinfo), obj);
	    }
	}

	return IMNoError;
    }

    else {
	/* Call IMProcessAux for the current IM. */
	return IMProcessAuxiliary(orec->obj, auxid, button, panel_row, 
	    panel_col, item_row, item_col, str, str_len);
    }
}

static int WIMIoctl(WIMObjectRec *obj, int cmd, caddr_t arg)
{
    ObjDataRec *orec;
    FepDataRec *frec;
    WIMFepRec *fep;
    int rc;
    int wrc;
    int i;

    fep = (WIMFepRec *)obj->common.imfep;

    if (get_data_recs(obj, &frec, &orec) == IMError) {
	return IMError;
    }

    switch (cmd) {

    case IM_Refresh: 
	return WIMRefresh(obj, arg, orec, frec);
	
    /* NOTE: Some of these may require conversion. */
    case IM_Clear:
    case IM_Reset:
    case IM_ChangeLength:
    case IM_QueryState:
    case IM_ChangeMode:
    case IM_GetKeymap:
    case IM_GetString:
    case IM_QueryText:
    case IM_QueryIndicator:
	rc = IMIoctl(orec->obj, cmd, arg);
	if (rc != IMNoError) {
	    fep->common.imerrno = frec->fep->imerrno;
	}
	return rc;

    case IM_QueryAuxiliary:
	return WIMQueryAuxiliary(obj, arg, orec, frec);

    case IM_QueryIndicatorString: 
	return WIMQueryIndicatorString(obj, arg, orec, frec);

    case IM_SupportSelection:
	/* Set flag in object. */
	obj->supp_selection = 1;
	/* Set menu to support selection. */
	menu_supp_selection(&(obj->locmenu), obj->supp_selection);
	menu_supp_selection(&(obj->clistmenu), obj->supp_selection);
	/* Pass info to existing child IMs. */
	wrc = IMNoError;
	for (i = 0; i < fep->numlocs; i++) {
	    if (obj->objtbl[i].obj) {
		rc = IMIoctl(orec->obj, IM_SupportSelection, arg);
		if (rc != IMNoError) {
		    fep->common.imerrno = frec->fep->imerrno;
		    wrc = rc;
		}
	    }
	}
	return wrc;
	
    default:
	/* NOTE: Some of these may require conversion. */
	/* Let the IM handle this situation. */
	rc = IMIoctl(orec->obj, cmd, arg);
	if (rc != IMNoError) {
	    fep->common.imerrno = frec->fep->imerrno;
	}
	return rc;
    }

    return IMNoError;
}

static void WIMClose(WIMFepRec *fep)
{
    uint_t i;

    for (i = 0; i < fep->numlocs; i++) {
	if (fep->feptbl[i].fep) {
	    IMClose(fep->feptbl[i].fep);
	}
	if (fep->feptbl[i].cd) {
	    iconv_close(fep->feptbl[i].cd);
	}
    }
    free(fep->feptbl);
    free(fep->loclabels);
    free(fep->clistlabels);

    menu_clear(&(fep->locmenu));
    menu_clear(&(fep->clistmenu));

    for (i = 0; i < fep->numclists; i++) {
	/* If a character menu has been created for this clist, free it. */
	if (fep->clisttbl[i].menu.num_choices) {
	    menu_clear(&(fep->clisttbl[i].menu));
	    free(fep->clisttbl[i].charbuf);
	    free(fep->clisttbl[i].chars);
	}
	/* Free the array of ranges. */
	free (fep->clisttbl[i].ranges);
    }
    free(fep->clisttbl);

    free(fep);
    return;
}

/************************************************************************
 * Ioctl handling functions
 ************************************************************************/

static int WIMRefresh(WIMObjectRec *obj, caddr_t arg, ObjDataRec *orec, 
	FepDataRec *frec)
{
    int rc;
    WIMFepRec *fep;

    fep = (WIMFepRec *)obj->common.imfep;

    /* Redraw indicator. */
    if (WIMIndicatorDraw(orec->obj, &(obj->indinfo), obj) != IMNoError) {
	fep->common.imerrno = IMCallbackError;
    }
    /* Redraw locale selection menu, if appropriate. */
    if (obj->state & LOCSEL_STATE) {
	menu_show(&(obj->locmenu), fep, obj, orec->obj);
    }
    /* Redraw clist selection menu, if appropriate. */
    if (obj->state & CLSEL_STATE) {
	menu_show(&(obj->clistmenu), fep, obj, orec->obj);
    }
    /* Redraw clist menu, if appropriate. */
    if (obj->state & CSEL_STATE) {
	menu_show(&(fep->clist->menu), fep, obj, orec->obj);
    }
    /* Call child IM Ioctl. */
    rc = IMIoctl(orec->obj, IM_Refresh, arg);
    if (rc != IMNoError) {
	fep->common.imerrno = frec->fep->imerrno;
    }
    return rc;
}

static int WIMQueryIndicatorString(WIMObjectRec *obj, 
	IMQueryIndicatorString *qindstr, ObjDataRec *orec, FepDataRec *frec)
{
    /* NOTE: Storage for actual string is allocated by input method. */
    char *locstr = frec->locale;
    uint_t loclen = strlen(locstr);
    int rc;
    char *p;

    /* Copy the locale name to the indicator string. obj->indstr is 
     * initialized with sufficient space to hold the locale name, when 
     * obj is created. */
    p = obj->indstr.str;
    /* NOTE: Motif1.2 workaround since indicator string can't seem to grow.
     * Pad with blanks. */
#define INDSTR_LEN 72
    memset(p, ' ', INDSTR_LEN);
    memcpy(p, locstr, loclen);
    p += loclen;
    *p = ' ';
    p += sizeof(' ');
    obj->indstr.len = (p - obj->indstr.str);

    /* Get Indicator string from child IM. */
    rc = IMIoctl(orec->obj, IM_QueryIndicatorString, qindstr);
    if (rc != IMNoError) {
	/* Return only WIM indicator. */

	/* NOTE: Motif1.2 workaround */
	obj->indstr.len = INDSTR_LEN;

	/* Copy IMSTR structure */
	qindstr->indicator = obj->indstr;
	obj->common.imfep->imerrno = frec->fep->imerrno;
	return rc;
    }

    /* Convert string returned by child IM. */
    if (iconv_str(frec->cd, &(qindstr->indicator), &(obj->str),
	&(obj->str_siz))) {
	/* Return only WIM indicator. */

	/* NOTE: Motif1.2 workaround */
	obj->indstr.len = INDSTR_LEN;

	/* Copy IMSTR structure */
	qindstr->indicator = obj->indstr;
	return IMError;
    }

    imstr_append(obj->str.str, obj->str.len, &(obj->indstr),
	&(obj->indstr_siz));

    /* NOTE: Motif1.2 workaround */
    obj->indstr.len = INDSTR_LEN;

    /* Copy IMSTR structure */
    qindstr->indicator = obj->indstr;

    return rc;
}

static int WIMQueryAuxiliary(WIMObjectRec *obj, IMQueryAuxiliary *qaux,
	ObjDataRec *orec, FepDataRec *frec)
{
    int rc;
    WIMFepRec *fep;

    /* NOTE: Storage for actual auxinfo struct is provided by input 
     * method. */

    fep = (WIMFepRec *)obj->common.imfep;

    /* If this is for one of the WIM's menus ... */
    if (obj->auxid == qaux->auxid) {
	if (obj->curmenu) {
	    qaux->auxinfo = &(obj->curmenu->auxinfo);
        }
	else {
	    /* Default to the locale menu. */
	    qaux->auxinfo = &(obj->locmenu.auxinfo);
	}
	return(IMNoError);
    }
	
    else { /* This is for the child IM. */
	rc = IMIoctl(orec->obj, IM_QueryAuxiliary, qaux);
	if (rc != IMNoError) {
	    obj->common.imfep->imerrno = frec->fep->imerrno;
	}

	if (iconv_auxinfo(frec->cd, qaux->auxinfo, &(obj->auxinfo), 
	    &(obj->auxinfo_siz), &(obj->auxinfo_nline), 
	    &(obj->auxinfo_nitem))) {
	    rc = IMError;
	}

	/* NOTE: Reuse of this structure may cause problems. May need
	 * to create one for each one that is created. */
	/* Return pointer to WIM's converted auxinfo structure. */
	qaux->auxinfo = &(obj->auxinfo);

	return rc;
    }
}

/************************************************************************
 * iconv() Conversion Functions
 ************************************************************************/

static int iconv_str(iconv_t cd, IMSTR *istr, 
    IMSTR *ostr, uint_t *osiz)
{
    uint_t ilen, olen;
    uchar_t *ip, *op;
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < (istr->len * MB_CUR_MAX)) {
	*osiz = istr->len * MB_CUR_MAX;
	ostr->str = (uchar_t *)realloc(ostr->str, *osiz);
    }

    /* Remember the output buffer size (this is decremented by iconv). */
    olen = *osiz;

    /* Initialize string pointers. */
    ip = istr->str;
    op = ostr->str;

    ilen = istr->len;

    /* Try to convert string. This increments ip & op and
     * decrements ilen & olen. */
    if (iconv(cd, &ip, &ilen, &op, &olen) == -1) {
	err = 1;
    }

    ostr->len = *osiz - olen;

    return err;
}

static int iconv_stratt(iconv_t cd, IMSTRATT *istratt, IMSTRATT *ostratt, 
        int *osiz)
{
    uint_t n1, n2, ilen, olen;
    uchar_t *istr, *ostr, *iatt, *oatt, *iattend;
    char attchr;
    char *p;
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < (istratt->len * MB_CUR_MAX)) {
	*osiz = istratt->len * MB_CUR_MAX;
	ostratt->str = (uchar_t *)realloc(ostratt->str, *osiz);
	ostratt->att = (uchar_t *)realloc(ostratt->att, *osiz);
    }

    /* Remember the output buffer size (this is decremented by iconv). */
    n2 = *osiz;

    /* Initialize string pointers. */
    istr = istratt->str;
    ostr = ostratt->str;
    iatt = istratt->att;
    oatt = ostratt->att;

    iattend = istratt->att + istratt->len;
    while ((iatt < iattend) && !err) {
	/* Find length of string with same attributes. */
	attchr = *iatt;
	ilen = 0;
	for (p = iatt; (*p == attchr) && (p < iattend); p++) {
	    ilen ++;
	}

	/* Check that this will not put us past the end of the string. */
	if (ilen > (iattend - iatt)) {
	    ilen = iattend - iatt;
	}

	n1 = ilen;
	olen = n2;
	/* Try to convert string. This increments istr & ostr and
	 * decrements n1 & n2. */
	if (iconv(cd, &istr, &n1, &ostr, &n2) == -1) {
	    err = 1;
	}
	olen = olen - n2;
	ilen = ilen - n1;

	/* Set the attributes for the output string. */
	memset(oatt, attchr, olen);

	/* Advance the attribute string pointers. */
	iatt += ilen;
	oatt += olen;
    }

    ostratt->len = *osiz - n2;

    return err;
}

static int iconv_len(iconv_t cd, uchar_t *istr, uint_t ilen, uint_t *olen)
{
    uchar_t *ip, *op;
    uint_t il, ol;
    int err = 0;
    static uint_t osiz = 0;
    static uchar_t *ostr = NULL;

    if (osiz < (ilen * MB_CUR_MAX)) {
	ostr = realloc(ostr, (ilen * MB_CUR_MAX));
    }
    osiz = ilen * MB_CUR_MAX;

    /* Initialize string pointers. */
    ip = istr;
    op = ostr;

    /* Initialize lengths. */
    ol = osiz;
    il = ilen;

    /* Try to convert string. This increments ip & op and
     * decrements ilen & olen. */
    if (iconv(cd, &ip, &il, &op, &ol) == -1) {
	err = 1;
    }

    *olen = osiz - ol;

    return err;
}

static int iconv_textinfo(iconv_t cd, IMTextInfo *itext, IMTextInfo *otext, 
	int *osiz)
{
    int err = 0;

    otext->maxwidth = itext->maxwidth;
    otext->cur = itext->cur;
    otext->chgtop = itext->chgtop;

    /* Determine length of unchanged part, in converted string. */
    err |= iconv_len(cd, itext->text.str, itext->chgtop, &(otext->chgtop));

    /* Determine cursor position and maxwidth for new string. */ 
    err |= iconv_len(cd, itext->text.str, itext->cur, &(otext->cur));
    /* NOTE: Shouldn't maxwidth really be in columns? */
    err |= iconv_len(cd, itext->text.str, itext->maxwidth, 
	    &(otext->maxwidth));

    /* Try to convert text. Convert the whole string, to avoid 
     * possible problems and keep things simple. */
    err |= iconv_stratt(cd, &(itext->text), &(otext->text), osiz);

    otext->chglen = otext->text.len - otext->chgtop;

    return err;
}

/* NOTE: Should probably malloc everything once before reallocing. */

static int iconv_auxinfo(iconv_t cd, IMAuxInfo *iaux, 
	IMAuxInfo *oaux, int *osiz, int *online, int *onitem)
{
    int err = 0;
    uint_t tmp_siz;
    uint_t max_siz;
    uint_t nitem;
    int i;

    /* Copy fields which don't need to be converted. */
    oaux->button = iaux->button;
    oaux->hint = iaux->hint;
    oaux->status = iaux->status;
    /* Maxwidth is in columns, not bytes. */
    oaux->message.maxwidth = iaux->message.maxwidth;
    oaux->message.nline = iaux->message.nline;
    oaux->message.cursor = iaux->message.cursor;
    oaux->message.cur_row = iaux->message.cur_row;
    /* cur_col is in bytes. */
    /* NOTE: This should be converted using iconv_len, on each string.
     * Is cur_col the size of the maximum length of a line in bytes?
     * Maybe we could use onsiz. */
    oaux->message.cur_col = iaux->message.cur_col;

    /* Make sure that there are enough lines. */
    if (oaux->message.nline > *online) {
	oaux->message.text = realloc(oaux->message.text, 
	    (oaux->message.nline * sizeof(IMMessage)));

	/* Malloc space for the new lines. */
	for (i = *online; i < oaux->message.nline; i++) {
	    oaux->message.text[i].str = malloc(*osiz);
	    oaux->message.text[i].att = malloc(*osiz);
	}
	*online = oaux->message.nline;
    }

    /* Initialize max_siz to current size. */
    max_siz = *osiz;

    if (iconv_str(cd, &(iaux->title), &(oaux->title), &max_siz)) {
	err = 1;
    }

    /* Convert all message lines. */
    for (i = 0; i < oaux->message.nline; i++) {
        tmp_siz = *osiz;
	if (iconv_stratt(cd, &(iaux->message.text[i]), 
	    &(oaux->message.text[i]), &tmp_siz)) {
	    err = 1;
	}
	max_siz = MAX(max_siz, tmp_siz);
    }

    /* If input auxinfo has a panel ... */
    if (iaux->selection.panel_row) {

	/* NOTE: Restrict number of panels to 1, for now. If more than one, 
	 * only convert the first one. */
	if ((iaux->selection.panel_row != 1) || 
	    (iaux->selection.panel_col != 1)) {
	    err = 1;
	}

	oaux->selection.panel_row = 1;
	oaux->selection.panel_col = 1;
	oaux->selection.panel[0].item_row = iaux->selection.panel[0].item_row;
	oaux->selection.panel[0].item_col = iaux->selection.panel[0].item_col;
	/* Maxwidth is in columns, not bytes. */
	oaux->selection.panel[0].maxwidth = iaux->selection.panel[0].maxwidth;

	/* Compute number of items required. */
	nitem = iaux->selection.panel[0].item_row *
	    iaux->selection.panel[0].item_col;

	/* If number of items in output array is not big enough ... */
	if (nitem > *onitem) {

	    /* Grow the array. */
	    oaux->selection.panel[0].item = 
		realloc(oaux->selection.panel[0].item,
		(nitem * sizeof(IMItem)));

	    /* Malloc space for the new item strings. */
	    for (i = *onitem; i < nitem; i++) {
		oaux->selection.panel[0].item[i].text.str = malloc(*osiz);
	    }

	    /* Update output array item count. */
	    *onitem = nitem;

	}

	for (i = 0; i < nitem; i++) {
	    oaux->selection.panel[0].item[i].selectable =
		iaux->selection.panel[0].item[i].selectable;
	}

	/* Convert all item strings. */
	for (i = 0; i < nitem; i++) {
	    tmp_siz = *osiz;
	    if (iconv_str(cd, &(iaux->selection.panel[0].item[i].text), 
		&(oaux->selection.panel[0].item[i].text), &tmp_siz)) {
		err = 1;
	    }
	    max_siz = MAX(max_siz, tmp_siz);
	}

	/* Realloc all buffers to the current max size. */
	oaux->title.str = realloc(oaux->title.str, max_siz);
	for (i = 0; i < *online; i++) {
	    oaux->message.text[i].str = realloc(oaux->message.text[i].str, 
		    max_siz);
	    oaux->message.text[i].att = realloc(oaux->message.text[i].att, 
		    max_siz);
	}
	for (i = 0; i < *onitem; i++) {
	    oaux->selection.panel[0].item[i].text.str = 
		realloc(oaux->selection.panel[0].item[i].text.str, max_siz);
	}
	*osiz = max_siz;
    }

    return err;
}
    
/************************************************************************
 * Preedit manipulation functions
 ************************************************************************/

static int preedit_init(WIMObjectRec *obj, ObjDataRec *orec)
{
    obj->textinfo.maxwidth = obj->wimcb.textmaxwidth;
    obj->textinfo.cur = 0;
    obj->textinfo.chgtop = 0;
    obj->textinfo.chglen = 0;
    obj->textinfo.text.len = 0;
    return WIMTextStart(orec->obj, &(obj->textinfo_space), obj);
}

static int preedit_commit(WIMObjectRec *obj, IMSTR *str, uint_t *str_siz,
    ObjDataRec *orec)
{
    imstr_copy(obj->textinfo.text.str, obj->textinfo.text.len, str, str_siz);
    return WIMTextHide(orec->obj, obj);
}

/************************************************************************
 * Menu manipulation functions
 ************************************************************************/

static int menu_init(MenuRec *rec, char *title, uint_t num_cols) 
{

    rec->auxinfo.title.str = title;
    rec->auxinfo.title.len = strlen(rec->auxinfo.title.str);

    rec->num_cols = num_cols;
    rec->auxinfo.selection.panel_row = 0;
    rec->auxinfo.selection.panel_col = 0;
    rec->auxinfo.selection.panel = NULL;

    rec->auxinfo.button = IM_NONE;
    rec->auxinfo.hint = IM_LowerRight;
    rec->auxinfo.status = IMAuxShowing;
    rec->auxinfo.message.cursor = FALSE;
    rec->auxinfo.message.cur_row = 0;
    rec->auxinfo.message.cur_col = 0;
    rec->auxinfo.message.text = NULL;
    rec->pages = NULL;
    rec->num_pages = 0;

    /* Use messages by default. */
    rec->supp_selection = FALSE;
    rec->auxinfo.message.maxwidth = MENU_LINE_LEN;
    rec->auxinfo.message.nline = LINES_PER_PAGE;

    return 0;
}

static int menu_supp_selection(MenuRec *rec, int supp_selection)
{
    rec->supp_selection = supp_selection;
    if (supp_selection) { /* Use selection. */
	rec->auxinfo.selection.panel_col = 1;
	rec->auxinfo.selection.panel_row = 1;
	rec->auxinfo.message.maxwidth = 0;
	rec->auxinfo.message.nline = 0;
    }
    else { /* Use message. */
	rec->auxinfo.message.maxwidth = MENU_LINE_LEN;
	rec->auxinfo.message.nline = LINES_PER_PAGE;
	rec->auxinfo.selection.panel_col = 0;
	rec->auxinfo.selection.panel_row = 0;
    }
}

/* NOTE: This should be called when the first copy of a menu is created (e.g.
 * in the FEP). Other copies of the menu will then get their own auxinfo
 * structure, etc. but will share the items array and the pages array.
 * This probably needs to be thought out more carefully, since much of
 * the same data is duplicated between the pages and items arrays. */
/* NOTE: In the RowColumn style of auxiliary, the length of the upper
 * left-hand item string of the first menu which is put up, is used as the
 * length for all items in all subsequent menus (since we are using the
 * same Auxiliary for all menus). The real problem is that we are limited
 * to one auxiliary per IMObject. */
static int menu_fill(MenuRec *rec, char **labels, uint_t num_labels)
{
    uint_t i;
    IMItem *item;
    uint_t page, line, len, maxlen;
    uint_t num_items;
    IMSTRATT *stratt;
    char label[MENU_LINE_LEN];

    if (!num_labels) {
	return 0;
    }

    rec->num_choices = num_labels;
    /* Space for the items array must be allocated and freed by the calling 
     * environment. */
    rec->items = labels;

    /* Initialize array of panels, regardless of whether selection is
     * supported or not. This permits objects to use or not use selection,
     * as desired. */
    rec->auxinfo.selection.panel = 
	(IMPanel *)malloc(sizeof(IMPanel));
    rec->auxinfo.selection.panel[0].item_col = rec->num_cols;
    rec->auxinfo.selection.panel[0].item_row = 
	(num_labels + rec->num_cols - 1) / rec->num_cols;
    rec->auxinfo.selection.panel[0].maxwidth = 16;
    /* Need to malloc enough for to fill out last row of 2d array. */
    num_items = rec->num_cols * rec->auxinfo.selection.panel[0].item_row;
    rec->auxinfo.selection.panel[0].item = 
	(IMItem *)malloc(num_items * sizeof(IMItem));

    for (i = 0; i < num_labels; i++)  {
	item = &(rec->auxinfo.selection.panel[0].item[i]);
	item->selectable = TRUE;
	item->text.str = labels[i];
	item->text.len = strlen(item->text.str);
    }

    /* Fill out rest of matrix, for row-column menus. */
    for (i = num_labels; i < num_items; i++) {
	item = &(rec->auxinfo.selection.panel[0].item[i]);
	item->selectable = FALSE;
	item->text.str = "";
	item->text.len = 0;
    }

    /* Initialize array of pages, regardless of whether selection is
     * supported or not. This permits objects to use or not use selection,
     * as desired. */

    rec->num_pages = (num_labels + MENU_PAGE_SIZE - 1) / MENU_PAGE_SIZE;
    rec->pages = (MenuPageRec *)malloc(sizeof(MenuPageRec) * rec->num_pages);

    /* Initialize empty pages. */
    /* For every page... */
    for (page = 0; page < rec->num_pages; page++) {
	/* For every line... */
	for (line = 0; line < LINES_PER_PAGE; line++) {
	    rec->pages[page].lines[line].str = 
		rec->pages[page].strbuf[line];
	    /* Fill strbuf with blanks. */
	    memset(rec->pages[page].strbuf[line], ' ', MENU_LINE_LEN);
	    rec->pages[page].lines[line].att = 
		rec->pages[page].attbuf[line];
	    /* Fill attbuff with IMAttNone. */
	    memset(rec->pages[page].attbuf[line], IMAttNone, MENU_LINE_LEN);
	}
    }

    maxlen = 0;

    page = 0;
    line = 0;
    for (i = 0; i < num_labels; i++) {

	/* If this is the start of a page... */
	if (!line) {
	    stratt = &(rec->pages[page].lines[line]);
	    /* If this is not the first page... */
	    if (page) {
		sprintf(stratt->str, "%c  %s", 'u', "<page up>");
		len = strlen(stratt->str);
		/* Replace NULL with blank. */
		stratt->str[len] = ' ';
		maxlen = MAX(len, maxlen);
	    }
	    line++;
	}

	stratt = &(rec->pages[page].lines[line]);
	/* Make sure that label will fit.  Subtract characters
	 * for the key, spaces & terminal null. */
	/* NOTE: This code assumes that digits, spaces, and nulls are
	 * single-byte characters. */
	memcpy(label, labels[i], MENU_LINE_LEN - (MENU_DIGITS + 3));
	label[MENU_LINE_LEN - (MENU_DIGITS + 3)] = '\0';

	sprintf(stratt->str, "%d  %s",  (line - 1), label);
	len = strlen(stratt->str);
	/* Replace NULL with space. */
	stratt->str[len] = ' ';
	maxlen = MAX(len, maxlen);
	line++;

	/* If this is the end of a page... */
	if (line == (MENU_PAGE_SIZE + 1)) {
	    stratt = &(rec->pages[page].lines[line]);
	    /* If this is not the last page... */
	    if ((page + 1) < rec->num_pages) {
		sprintf(stratt->str, "%c  %s", 'd', "<page down>");
		len = strlen(stratt->str);
		/* Replace NULL with blank. */
		stratt->str[len] = ' ';
		maxlen = MAX(len, maxlen);
		line = 0;
		page++;
	    }
	}
    }

    /* Making all lines filled in with blanks and the same length avoids
     * the need for hiding the menu to change page. */

    /* For every page... */
    for (page = 0; page < rec->num_pages; page++) {
	/* For every line... */
	for (line = 0; line < LINES_PER_PAGE; line++) {
	    /* Set length to maximum line length. */
	    rec->pages[page].lines[line].len = maxlen;
	}
    }

    /* Initialize menu to the first page. */
    rec->auxinfo.message.text = rec->pages[0].lines;
    rec->page = 0;

    return 0;
}

/* This sets the menu back to the state it was in after menu_init. It
 * should be called only at the FEP menu level. */
static int menu_clear(MenuRec *rec)
{

    /* Reset auxinfo message fields. */
    rec->num_choices = 0;
    rec->auxinfo.message.text = NULL;

    /* Use messages by default. */
    rec->auxinfo.message.maxwidth = MENU_LINE_LEN;
    rec->auxinfo.message.nline = LINES_PER_PAGE;

    /* Clear page data if it has been created. */
    if (rec->num_pages) {
	free(rec->pages);
	rec->pages = NULL;
	rec->num_pages = 0;
    }

    /* Clear selection panel, if it has been created. */
    if (rec->auxinfo.selection.panel_col) {
	if (rec->auxinfo.selection.panel[0].item_col) {
	    free(rec->auxinfo.selection.panel[0].item);
	    rec->auxinfo.selection.panel[0].item = NULL;
	    rec->auxinfo.selection.panel[0].item_col = 0;
	}
        free(rec->auxinfo.selection.panel);
	rec->auxinfo.selection.panel = NULL;
	rec->auxinfo.selection.panel_col = 0;
	rec->auxinfo.selection.panel_row = 0;
    }
    return 0;
}

static int menu_process_auxiliary(MenuRec *rec, uint_t button,
	uint_t panel_row, uint_t panel_col, uint_t item_row, 
	uint_t item_col, int *selection)
{
    if (!(rec->supp_selection)) {
	return -1;
    }

    /* Only one panel and, so these should be 0. */
    if (panel_row || panel_col) {
	return -1;
    }

    /* Selection is ((row * number_of_columns) + columnn). */
    *selection = (item_row * rec->auxinfo.selection.panel[0].item_col) +
	item_col;

    if (*selection >= rec->num_choices) {
	*selection = -1;
	return -1;
    }

    return 0;
}

static int menu_filter(MenuRec *rec, uint_t keysym, uint_t state, 
    WIMFepRec *wfep, WIMObjectRec *wobj, IMObject *obj, int *selection)
{
    int i;

    *selection = -1;

    if (rec->supp_selection) {
	return -1;
    }

    switch (keysym) {
    case XK_u:
    case XK_U:
    case XK_Prior:
	if (rec->page) {
	    rec->page--;
	    rec->auxinfo.message.text = rec->pages[rec->page].lines;
	    if (wobj->auxid) {
		/* This is necessary for aixterm. */
		if (WIMAuxHide(obj, wobj->auxid, wobj) != IMNoError) {
		    wfep->common.imerrno = IMCallbackError;
		}
		if (WIMAuxDraw(obj, wobj->auxid, &rec->auxinfo, wobj) != 
		    IMNoError) {
		    wfep->common.imerrno = IMCallbackError;
		}
	    }
	}
	else {
	    WIMBeep(obj, 50, wobj);
	    return -1;
	}
	break;
    case XK_d:
    case XK_D:
    case XK_Next:
	if ((rec->page + 1) < rec->num_pages) {
	    rec->page++;
	    rec->auxinfo.message.text = rec->pages[rec->page].lines;
	    if (wobj->auxid) {
		/* This is necessary for aixterm. */
		if (WIMAuxHide(obj, wobj->auxid, wobj) != IMNoError) {
		    wfep->common.imerrno = IMCallbackError;
		}
		if (WIMAuxDraw(obj, wobj->auxid, &rec->auxinfo, wobj) != 
		    IMNoError) {
		    wfep->common.imerrno = IMCallbackError;
		}
	    }
	}
	else {
	    WIMBeep(obj, 50, wobj);
	    return -1;
	}
	break;

    case XK_Shift_L:
    case XK_Shift_R:
    case XK_Control_L:
    case XK_Control_R:
    case XK_Caps_Lock:
    case XK_Shift_Lock:
    case XK_Meta_L:
    case XK_Meta_R:
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Super_L:
    case XK_Super_R:
    case XK_Hyper_L:
    case XK_Hyper_R:
    case XK_Mode_switch:
    case XK_Num_Lock:
	/* Ignore modifier keysyms. */
	return -1;
	break;

    default:
	/* NOTE: This assumes that number keysyms are contiguous. */
	if ((XK_0 <= keysym) && (XK_9 >= keysym)) {
	    i = keysym - XK_0;
	}
	else if ((XK_KP_0 <= keysym) && (XK_KP_9 >= keysym)) {
	    i = keysym - XK_KP_0;
	}
	else {
	    WIMBeep(obj, 50, wobj);
	    return -1;
	}

	if ((i > -1) && (i < MENU_PAGE_SIZE)) { 
	    i += (rec->page * MENU_PAGE_SIZE);
	    if (i > rec->num_choices) {
		WIMBeep(obj, 50, wobj);
		return -1;
	    }
	}
	else {
	    WIMBeep(obj, 50, wobj);
	    return -1;
	}
	*selection = i;
    }

    return 0;
}

static int menu_reorder(MenuRec *rec, uint_t selection)
{
    return 0;
}

static int menu_hide(MenuRec *rec, WIMFepRec *wfep, WIMObjectRec *wobj, 
    IMObject *obj)
{
    int rc = 0;

    if (wobj->auxid) {
	if (WIMAuxHide(obj, wobj->auxid, wobj) != IMNoError) {
	    wfep->common.imerrno = IMCallbackError;
	    rc = 1;
	}
    }
    return rc;
}

static int menu_show(MenuRec *rec, WIMFepRec *wfep, WIMObjectRec *wobj, 
    IMObject *obj)
{
    int rc = 0;

    if (!rec->num_choices) {
	return 1;
    }

    /* NOTE: This is necessary because some auxinfo info is cached by XIM
     * the first time an auxiliary is drawn. Also because XIM doesn't allow
     * more than one auxiliary per IMObject. */

    /* If this menu is not current menu ... */
    if (wobj->curmenu != rec) {
	/* If there is a current menu ... */
	if (wobj->curmenu) {
	    /* Destroy auxiliary for current menu. */
	    WIMAuxDestroy(obj, wobj->auxid, wobj);
	}
	/* Try to make this menu the current menu. */
	wobj->curmenu = rec;
	if (WIMAuxCreate(obj, &(wobj->auxid), wobj) != IMNoError) {
	    wfep->common.imerrno = IMCallbackError;
	    wobj->auxid = NULL;
	    wobj->curmenu = NULL;
	    return 1;
	}
    }

    if (wobj->auxid) {
	if (WIMAuxDraw(obj, wobj->auxid, &(rec->auxinfo), wobj) != IMNoError) {
	    wfep->common.imerrno = IMCallbackError;
	    return 1;
	}
    }

    return 0;
}

/************************************************************************
 * Miscellaneous Functions
 ************************************************************************/

static int imstr_copy(char *str, uint_t len, IMSTR *ostr, uint_t *osiz)
{
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < len) {
	*osiz = len;
	ostr->str = (uchar_t *)realloc(ostr->str, *osiz);
    }

    memcpy(ostr->str, str, len);
    ostr->len = len;

    return err;
}

static int imstr_append(char *str, uint_t len, IMSTR *ostr, uint_t *osiz)
{
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < (len + ostr->len)) {
	*osiz = len + ostr->len;
	ostr->str = (uchar_t *)realloc(ostr->str, *osiz);
    }

    memcpy(ostr->str + ostr->len, str, len);
    ostr->len += len;

    return err;
}

static int imstratt_copy(char *str, uint_t len, char att, 
    IMSTRATT *ostr, uint_t *osiz)
{
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < len) {
	*osiz = len;
	ostr->str = (uchar_t *)realloc(ostr->str, *osiz);
	ostr->att = (uchar_t *)realloc(ostr->att, *osiz);
    }

    memcpy(ostr->str, str, len);
    memset(ostr->att, att, len);
    ostr->len = len;

    return err;
}

static int imstratt_append(char *str, uint_t len, char att, 
    IMSTRATT *ostr, uint_t *osiz)
{
    int err = 0;

    /* Realloc maximum size needed for conversion, if necessary. */
    if (*osiz < (len + ostr->len)) {
	*osiz = len + ostr->len;
	ostr->str = (uchar_t *)realloc(ostr->str, *osiz);
	ostr->att = (uchar_t *)realloc(ostr->att, *osiz);
    }

    memcpy(ostr->str + ostr->len, str, len);
    memset(ostr->att + ostr->len, att, len);
    ostr->len += len;

    return err;
}

static int find_loc(WIMFepRec *fep, char *language)
{
    int i;
    char *str;

    str = strdup(language);
    strtok(str, ".");
    for (i = 0; i < fep->numlocs; i++) {
	if (!strcmp(str, fep->feptbl[i].locale)) {
	    break;
	}
    }
    /* NOTE: If not found, might want to look for version beginning with
     * other case (i.e. lower-case vs upper-case). */
    free(str);

    return i;
}

ClistRec *find_clist(WIMFepRec *fep, char *label)
{
    int i;

    for (i = 0; i < fep->numclists; i++) {
	if (!strcmp(label, fep->clisttbl[i].label)) {
	    break;
	}
    }
    if (i >= fep->numclists) {
	/* Default clist is first in the list. */
	i = 0;
    }

    return &(fep->clisttbl[i]);
}

static int get_data_recs(WIMObjectRec *obj, FepDataRec **frecp,
        ObjDataRec **orecp)
{
    WIMFepRec *fep;
    ObjDataRec *orec;
    FepDataRec *frec;

    if (!obj) {
	return IMError;
    }

    fep = (WIMFepRec *)(obj->common.imfep);

    if (obj->curloc >= obj->numlocs) {
	return IMError;
    }

    /* Get Obj and FEP data records for current locale. */
    *frecp = frec = &(fep->feptbl[obj->curloc]);
    *orecp = orec = &(obj->objtbl[obj->curloc]);

    /* Allocate a FEP for this IM if not already done. */
    if (!(frec->fep)) {
	if (!(frec->fep = IMInitialize(fep->feptbl[obj->curloc].imlocale))) {
	    return IMError;
	}
    }

    /* Open a converter for this IM if not already done. */
    if (!(frec->cd)) {
	frec->cd = iconv_open(nl_langinfo(CODESET), 
		fep->feptbl[obj->curloc].code);
	if ((!frec->cd) || (frec->cd == -1)) {
	    frec->cd = NULL;
	    return IMError;
	}
    }

    /* Allocate an Obj for this IM if not already done. */
    if (!(orec->obj)) {
	/* NOTE: Since IMCreate can call a callback function, need to set
	 * orec->obj in all callbacks, to prevent infinite recursion. */
	if (!(orec->obj = IMCreate(frec->fep, &(obj->wimcb), obj))) {
	    return IMError;
	}

	/* If application supports selection lists, make appropriate Ioctl
	 * call. */
	if (obj->supp_selection) {
	    if (IMIoctl(orec->obj, IM_SupportSelection, NULL) != IMNoError) {
		fep->common.imerrno = frec->fep->imerrno;
	    }
	    /* Don't fail, just because child IM can't handle this. */
	}

    }
    
    return IMNoError;
}

static int wim_filter(WIMObjectRec *obj, uint_t keysym, uint_t state,
	IMSTR *str, uint_t *str_siz, ObjDataRec *orec, FepDataRec *frec)
{
    int selection;
    int oldloc;
    WIMFepRec *fep;
    /* NOTE: I am not passing these back to WIMFilter, for now. */
    ObjDataRec *neworec;
    FepDataRec *newfrec;
    int rc = IMInputNotUsed;

    fep = (WIMFepRec *)obj->common.imfep;

    switch (obj->state) {
    case BASE_STATE: /* Not in a selection mode. */
	if ((state == fep->locsel_modmask) && (keysym == fep->locsel_xk)) {
	    /* Locale selection initiation code  */
	    rc = IMInputUsed;
	    str->len = 0;
	    obj->state = LOCSEL_STATE;
	    if (menu_show(&(obj->locmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->clsel_modmask) && (keysym == fep->clsel_xk)) {
	    /* Character list selection initiation code  */
	    rc = IMInputUsed;
	    str->len = 0;
	    obj->state = CLSEL_STATE;
	    if (menu_show(&(obj->clistmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->csel_modmask) && (keysym == fep->csel_xk)) {
	    /* Character selection initiation code  */
	    rc = IMInputUsed;
	    str->len = 0;
	    obj->state = CSEL_STATE;
	    /* If this clist has not yet been created, fill the menu. */
	    if (!(fep->clist->chars)) {
		clist_fill(fep->clist, obj->supp_selection);
		/* NOTE: I should probably keep a copy of the clist record
		 * with each WIMobject. */
	    }
	    if (menu_show(&(fep->clist->menu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	    else { /* menu_show succeeded. */
		if (obj->supp_selection) {
		    preedit_init(obj, orec);
		    /* NOTE: Return value was not always 0. */
		}
	    }
	} 
	/* If not a special key sequence, leave return code and string
	 * unchanted. */
	break; /* BASE_STATE */

    case LOCSEL_STATE: /* In a locale selection mode. */
	rc = IMInputUsed;
	str->len = 0;
	if ((state == fep->locsel_modmask) && (keysym == fep->locsel_xk)) {
	    /* Can be used to return to base mode. */
	    if (!menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->clsel_modmask) && (keysym == fep->clsel_xk)) {
	    /* Can be used to get to clist selection menu. */
	    if (menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = CLSEL_STATE;
	    if (menu_show(&(obj->clistmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->csel_modmask) && (keysym == fep->csel_xk)) {
	    /* Can be used to get to character selection menu. */
	    if (menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = CSEL_STATE;
	    /* If this clist has not yet been created, fill the menu. */
	    if (!(fep->clist->chars)) {
		clist_fill(fep->clist, obj->supp_selection);
	    }
	    if (menu_show(&(fep->clist->menu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	    else {
		if (obj->supp_selection) {
		    preedit_init(obj, orec);
		    /* NOTE: Return value was not always 0. */
		}
	    }
	}
	else if (keysym == XK_Return) {
	    /* Return to base state. */
	    if (!menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		obj->state = BASE_STATE;
	    }
	}
	else { 
	    /* Not a selection character or a return. */
	    if (menu_filter(&(obj->locmenu), keysym, state, 
		fep, obj, orec->obj, &selection)) {
		/* This is an input error. */
	    }
	    else if (selection > -1) { 
		/* A valid selection was returned by menu_filter. */
		/* Try to load this locale. */
		oldloc = obj->curloc;
		obj->curloc = selection;
		if (get_data_recs(obj, &newfrec, &neworec) != IMError) {
		    orec = neworec;
		    frec = newfrec;
		    /* Change default locale for Fep. */
		    fep->defloc = obj->curloc;
		    /* Draw indicator for new locale. */
		    WIMIndicatorDraw(orec->obj, &(obj->indinfo), obj);
		    /* Put this locale at front of menu. */
		    /* menu_reorder(&(obj->locmenu), selection); */
		}
		else {
		    obj->curloc = oldloc;
		    WIMBeep(orec->obj, 50, obj);
		}
		if (!menu_hide(&(obj->locmenu), fep, obj, orec->obj)) {
		    obj->state = BASE_STATE;
		}
	    }
	    /* Otherwise, the keysym was used by menu_filter 
	     * (page-down, etc.). */
	}
	    
	break; /* LOCSEL_STATE */

    case CLSEL_STATE: /* In a char list selection mode. */
	rc = IMInputUsed;
	str->len = 0;
	if ((state == fep->locsel_modmask) && (keysym == fep->locsel_xk)) {
	    /* Can be used to get to loc selection menu. */
	    if (menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = LOCSEL_STATE;
	    if (menu_show(&(obj->locmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->clsel_modmask) && (keysym == fep->clsel_xk)) {
	    /* Can be used to get to return to base state. */
	    if (!menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->csel_modmask) && (keysym == fep->csel_xk)) {
	    /* Can be used to get to clist. */
	    if (menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = CSEL_STATE;
	    /* If this clist has not yet been created, fill the menu. */
	    if (!(fep->clist->chars)) {
		clist_fill(fep->clist, obj->supp_selection);
	    }
	    if (menu_show(&(fep->clist->menu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	    else {
		if (obj->supp_selection) {
		    preedit_init(obj, orec);
		    /* NOTE: Return value was not always 0. */
		}
	    }
	}
	else if (keysym == XK_Return) {
	    /* Return to base state. */
	    if (!menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		obj->state = BASE_STATE;
	    }
	}
	else { /* Not a control character or a return. */
	    if (menu_filter(&(obj->clistmenu), keysym, state, 
		fep, obj, orec->obj, &selection)) {
		/* This is an input error. */
	    }
	    else if (selection > -1) {
		/* A valid selection was returned by menu_filter. */
		fep->clist = &(fep->clisttbl[selection]);
		/* Put this clist at front of menu. */
		if (!menu_hide(&(obj->clistmenu), fep, obj, orec->obj)) {
		    /* Put up clist. */
		    obj->state = CSEL_STATE;
		    if (!(fep->clist->chars)) {
			clist_fill(fep->clist, obj->supp_selection);
		    }
		    if (menu_show(&(fep->clist->menu), fep, obj, orec->obj)) {
			/* menu_show failed. */
		        obj->state = BASE_STATE;
		    }
		}
	    }
	    /* Otherwise, the keysym was used by menu_filter 
	     * (page-down, etc.). */
	}

	break; /* CLSEL_STATE */

    case CSEL_STATE: /* In a character selection mode. */
	rc = IMInputUsed;
	str->len = 0;
	if ((state == fep->locsel_modmask) && (keysym == fep->locsel_xk)) {
	    /* Can be used to get to loc selection menu. */
	    if (obj->supp_selection) {
		preedit_commit(obj, str, str_siz, orec);
		/* preedit_init(obj, orec); */
	    }
	    if (menu_hide(&(fep->clist->menu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = LOCSEL_STATE;
	    if (menu_show(&(obj->locmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->clsel_modmask) && (keysym == fep->clsel_xk)) {
	    /* Can be used to get to clist selection menu. */
	    if (obj->supp_selection) {
		preedit_commit(obj, str, str_siz, orec);
		/* preedit_init(obj, orec); */
	    }
	    if (menu_hide(&(fep->clist->menu), fep, obj, orec->obj)) {
		break;
	    }
	    obj->state = CLSEL_STATE;
	    if (menu_show(&(obj->clistmenu), fep, obj, orec->obj)) {
		/* menu_show failed. */
		obj->state = BASE_STATE;
	    }
	}
	else if ((state == fep->csel_modmask) && (keysym == fep->csel_xk)) {
	    /* Can be used to get back to base state. */
	    if (obj->supp_selection) {
		preedit_commit(obj, str, str_siz, orec);
		/* preedit_init(obj, orec); */
	    }
	    if (!menu_hide(&(fep->clist->menu), fep, obj, orec->obj)) {
		obj->state = BASE_STATE;
	    }
	} 
	else if (keysym == XK_Return) {
	    /* Can be used to get back to base state. */
	    if (obj->supp_selection && !obj->textinfo.chglen &&
	      !obj->textinfo.text.len) {
		if (!menu_hide(&(fep->clist->menu), fep, obj, orec->obj)) {
		    obj->state = BASE_STATE;
		}
	    }
	    else {
	    /* Return can be used to commit string, without taking
	     * down menu. */
		preedit_commit(obj, str, str_siz, orec);
		preedit_init(obj, orec);
	    }
	}
	else { /* Not a return or a control character. */
	    if (menu_filter(&(fep->clist->menu), keysym, state, 
		fep, obj, orec->obj, &selection)) {
		/* This is an input error. */
	    }
	    else if (selection > -1) {
		/* A valid selection was returned by menu_filter. */
		imstr_copy(fep->clist->chars[selection], 
		    mblen(fep->clist->chars[selection], MB_CUR_MAX),
		    str, str_siz);
	    }
	}

	break; /* CSEL_STATE */
    } /* switch obj->state */

    return rc;
}

static int *get_locpath(IMLanguage language, char *suffix, char* pathname)
{
	char	*locpath, *locptr;
	int	len;

    *pathname = '\0';

    if (!language) {
	return IMError;
    }

    locpath = NULL;

    if (!__issetuid()) {
	locpath = getenv("LOCPATH");
    }

    if (!locpath || !*locpath) {
	locpath = IMDIRECTORY;
    }

    do {
	locptr = locpath;
	while (*locptr && *locptr != ':') locptr++;
	len = locptr - locpath;
	if (len == 0) {
	    locptr++;
	    len = sizeof (IMDIRECTORY) - 1;
	    locpath = IMDIRECTORY;
	}
	else if (*locptr && !*++locptr) {
	    locptr--;
	}
	memcpy(pathname, locpath, len);
	sprintf(&(pathname[len]), "/%s%s", language, suffix);
        /* Check if readable. */
	if (access(pathname, R_OK) >= 0) {
	    return IMNoError;
	}
	locpath = locptr;
    } while (*locpath);
    return IMError;
}

static int read_config(WIMFepRec *fep, char *path)
{
    uint_t feptbl_size = 32;	/* initial size of feptbl. */
    uint_t clisttbl_size = 32;	/* initial size of clistbl. */
    FILE *file;
    char *line;			/* pointer to current line in file */
    char *linebuff;		/* complete buffer contatining logical line */
    uint_t linebuff_siz = 4;	/* size of buffer in lines */
    uint_t nlines;		/* number of lines in current logical line. */
    uint_t len;
    char *keyword, *value;
    uint_t i;
    ClistRec *clist;

    /* Initialize selection key values. */
    fep->locsel_modmask = LOCSEL_MODMASK;
    fep->locsel_xk = LOCSEL_XK;
    fep->clsel_modmask = CLSEL_MODMASK;
    fep->clsel_xk = CLSEL_XK;
    fep->csel_modmask = CSEL_MODMASK;
    fep->csel_xk = CSEL_XK;

    /* Allocate initial FEP Table. */
    fep->feptbl = (FepDataRec *)malloc(feptbl_size * sizeof(FepDataRec));
    fep->numlocs = 0;

    /* Allocate initial clist table. */
    fep->clisttbl = (ClistRec *)malloc(clisttbl_size * sizeof(ClistRec));
    fep->numclists = 0;
    fep->clist = &(fep->clisttbl[0]);

    /* If can't find config file return an error. */
    if (!(file = fopen(path, "r"))) {
	return IMError;
    }

    /* Allocate initial (4 line) line buffer. */
    linebuff = malloc(linebuff_siz * MAX_CFG_LINE);

    /* Parse lines. */
    nlines = 1;
    line = linebuff;
    while (fgets(line, MAX_CFG_LINE - 1, file)) {
	
	/* Prune comments. */
	line[strcspn(line, "#")] = '\0';

	/* Process line continuations. */
	line += (strlen(line) - 2);
	if (!strcmp(line, "\\\n")) {
	    /* Newline is escaped, increment line counter,
	     * check buffer size, and realloc if necesssary. */
	    nlines++;
	    if (linebuff_siz < nlines) {
		linebuff_siz *= 2;
		len = line - linebuff;
		linebuff = realloc(linebuff, linebuff_siz * MAX_CFG_LINE);
		line = linebuff + len;
	    }
	    /* Get next (continuation) line. */
	    continue;
	}
	/* Reset line counter and pointer to front of line buff. */
	nlines = 1;
	line = linebuff;

	/* Skip white space. */
	keyword = linebuff + strspn(linebuff, " \t\n");

	/* Skip over keyword. */
	value = keyword + strcspn(keyword, " \t\n");

	/* Tokenize keyowrd. */
	*value = '\0';
	value++;

	/* Skip blanks to keyword value. */
	value += strspn(value, " \t\n");

	/* If this line is incomplete ... */
	if (!(*value)) {
	    /* Start over with next line. */
	    continue;
	}

	if (!strcmp(keyword, "IMSelKey")) {
	    if (sscanf(value, "%x%x", &(fep->locsel_modmask), 
		&(fep->locsel_xk)) != 2) {
		continue;
	    }
	}

	else if (!strcmp(keyword, "ClistSelKey")) {
	    if (sscanf(value, "%x%x", &(fep->clsel_modmask), 
		&(fep->clsel_xk)) != 2) {
		continue;
	    }
	}

	else if (!strcmp(keyword, "CharSelKey")) {
	    if (sscanf(value, "%x%x", &(fep->csel_modmask), 
		&(fep->csel_xk)) != 2) {
		continue;
	    }
	}

	else if (!strcmp(keyword, "IM")) {

	    /* If the table is not big enough, double it. */
	    if (fep->numlocs >= feptbl_size) {
		feptbl_size *= 2;
		fep->feptbl = (FepDataRec *)realloc(fep->feptbl, 
			feptbl_size * sizeof(FepDataRec));
	    }
	    parse_locline(fep, value);
	}

	else if (!strcmp(keyword, "Clist")) {

	    /* If the table is not big enough, double it. */
	    if (fep->numclists >= clisttbl_size) {
		clisttbl_size *= 2;
		fep->clisttbl = (ClistRec *)realloc(fep->clisttbl, 
			clisttbl_size * sizeof(ClistRec));
	    }
	    parse_clistline(fep, value);

	}
    }

    /* Do this at the end, after all reallocing is finished. */
    for (i = 0; i < fep->numclists; i ++) {
	clist = &(fep->clisttbl[i]);
	/* Initialize clist menu. This is filled only if used. */
	menu_init(&(clist->menu), clist->label, CLIST_NUM_COLS);
    }

    free(linebuff);

    /* Set default clist. */
    fep->clist = &(fep->clisttbl[0]);

    return IMNoError;
}

static int parse_locline(WIMFepRec *fep, char *line)
{
    FepDataRec *rec;
    char *field;

    /* NOTE: Might not want to create entries with identical
     * descriptions. (i.e. use first entry only). */

    /* The table is guaranteed to have at least one more empty entry. */
    rec = &(fep->feptbl[fep->numlocs]);

    /* Use strtok to parse the line. */
    if ((field = strtok(line, " \t\n"))) {
	memcpy(rec->description, field, MAX_CFG_FIELD);
	rec->description[MAX_CFG_FIELD - 1] = '\0';
    }
    if ((field = strtok(NULL, " \t\n"))) {
	memcpy(rec->locale, field, MAX_CFG_FIELD);
	rec->locale[MAX_CFG_FIELD - 1] = '\0';
    }
    if ((field = strtok(NULL, " \t\n"))) {
	memcpy(rec->imlocale, field, MAX_CFG_FIELD);
	rec->imlocale[MAX_CFG_FIELD - 1] = '\0';
    }
    if ((field = strtok(NULL, " \t\n"))) {
	memcpy(rec->code, field, MAX_CFG_FIELD);
	rec->code[MAX_CFG_FIELD - 1] = '\0';
    }

    /* Check that all fields were found before end of line. */
    if (!field) {
	return IMError;
    }

    fep->numlocs++;
    rec->fep = NULL;
    rec->cd = NULL;
    return IMNoError;
}

static int parse_clistline(WIMFepRec *fep, char *line)
{
    ClistRec *clist;
    CrangeRec *range;
    int clist_size;
    uint_t first, last;
    char *p, *q;
    int len;

    clist = &(fep->clisttbl[fep->numclists]);
    clist->chars = NULL;
    clist->charbuf = NULL;

    /* Copy clist label. */
    len = strcspn(line, " \t\n");
    if (!len) {
	/* If no label, just return. */
	return IMError;
    }
    p = line + len;
    len = MIN(MAX_CFG_FIELD - 1, len);
    memcpy(clist->label, line, len);
    clist->label[len] = '\0';

    /* Skip blanks. */
    p += strspn(p, " \t\n");

    clist_size = 16;
    clist->ranges = (CrangeRec *)malloc(clist_size * sizeof(CrangeRec));
    clist->numranges = 0;

    /* While can read a number... */
    while(sscanf(p, "%x", &first) == 1) {

	/* If the clist is not big enough, double it. */
	if (clist->numranges >= clist_size) {
	    clist_size *= 2;
	    clist->ranges = (CrangeRec *)realloc(clist->ranges, 
		clist_size * sizeof(CrangeRec));
	}

	range = &(clist->ranges[clist->numranges]);
	range->first = (wchar_t)first;

	/* Get pointer to range or sequence delimiter. */
	q = p + strcspn(p, ",-");
	if (*q) {
	    p = q + 1;
	}
	else {
	    p = q;
	}

	/* If this is a range delimiter, try to read end of range. */
	if ((*q == '-') && (sscanf(p, "%x", &last) == 1)) {
	    range->last = (wchar_t)last;
	    /* Skip to next delimiter. */
	    q = p + strcspn(p, ",-");
	    if (*q) {
		p = q + 1;
	    }
	    else {
		p = q;
	    }
	}
	else {
	    range->last = (wchar_t)first;
	}

	clist->numranges++;
    }

    /* If no ranges, just return. */
    if (!clist->numranges) {
	return IMError;
    }

    fep->numclists++;
    return IMNoError;
}

static int clist_fill(ClistRec *clist, int supp_selection)
{
    int i;
    int numchars;
    wchar_t wc;
    char *p;
    int len;
    int size = 128; /* Initial size of char list. */

    /* If clist is empty, don't do anything. */
    if (!clist->numranges) {
	return 0;
    }

    /* Buffer has space for file code plus space plus terminal NULL. */
    /* NOTE: Space is required because of Motif 1.2 bug. */
    clist->charbuf = (char *)malloc(size * (MB_CUR_MAX + 2));
    numchars = 0;
    for (i = 0; i < clist->numranges; i++) {
	for(wc = clist->ranges[i].first; wc <= clist->ranges[i].last; wc++) {
	    if (numchars >= size) {
		size *= 2;
		clist->charbuf = (char *)realloc(clist->charbuf, 
		    size * (MB_CUR_MAX + 2));
	    }
	    p = &(clist->charbuf[numchars * (MB_CUR_MAX + 2)]);
	    if (len = wctomb(p, wc)) {
		/* if (iswprint(w)) { */
		/* Check if this is a valid printable character in the
		 * charmap. */
		if ((!iswcntrl(wc)) && (isascii(*p) || (wcsid(wc) > 0))) { 
		    p[len] = '\0';
		    numchars++;
		}
	    }
	}
    }
    /* NOTE: Have to this at the end, because realloc will change
     * addresses. */
    clist->chars = (char **)malloc(numchars * sizeof(char *));
    for (i = 0; i < numchars; i++) {
	clist->chars[i] = &(clist->charbuf[i * (MB_CUR_MAX + 2)]);
    }

    /* NOTE: This is a hack to make sure that characters are always at least
     * the width of first character + space. Motif 1.2 bug causes the with of 
     * items in row-column to be the width of the first item. */
    if (numchars) {
        strcat(clist->chars[0], " ");
    }

    menu_fill(&(clist->menu), clist->chars, numchars);
    menu_supp_selection(&(clist->menu), supp_selection);
}
