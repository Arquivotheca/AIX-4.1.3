static char sccsid[] = "@(#)03	1.8.1.4  src/bos/usr/lib/nls/loc/jim/jfep/JIMCreate.c, libKJI, bos411, 9428A410j 6/10/94 14:48:44";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMCreate.c
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <imlang.h>
#include <limits.h>
#include <ctype.h>
#include "imjimP.h"		/* Japanese Input Method header file */
#include "jimerrno.h"           /* Japanese Input Method error defs  */

/*-----------------------------------------------------------------------*
*	Beginning of JIMmakeprofile
*	    retrieve profile information from JIM profile file
*	    or set to defaults
*       return FLASE if dictionary is not found, TRUE otherwise
*-----------------------------------------------------------------------*/
static int	JIMmakeprofile(jimprofile *pro) 
{
    extern int getprofile();
    extern char *getoptvalue();
    extern void closeprofile();
    extern char *searchprofile();
    extern char **findsysdict();
    int	   i;
    extern char *findusrdict();
    extern char *findadjdict();
    char *temp;
    char *proname;
    char *pnum;
    int  inum, numerr;

    /*******************************/ 
    /* set every field to defaults */ 
    /*******************************/ 
    pro->initchar = JIM_ALPHA;
    pro->initsize = JIM_SINGLE;
    pro->initrkc = JIM_RKCOFF;
    pro->conversion = JIM_MPHRASE;
    pro->kjbeep  = JIM_BEEPON;
    pro->learning = JIM_LEARNON;
    pro->dictname.usr = NULL;
    pro->dictname.adj = NULL;
    pro->kuten = JIM_KUTEN83;
    pro->shmatnum   = JIM_SHNUMDEF;
    pro->udload     = JIM_UDLOADON;
    pro->numberinput= JIM_JISKUTEN;
    pro->modereset  = JIM_RESETOFF;
    pro->alphanum   = JIM_ALPHAON;
    pro->supportselection = 0;

    /*************************************************/
    /* first, search profile file in following order */
    /*************************************************/
    proname = searchprofile(R_OK);

    /***************************************************/
    /* if file exists, retrieve information from it,   */
    /* updating information in structure	       */
    /***************************************************/
    if(proname) { 
	if(getprofile(proname)) { 
#ifdef PROFILEDEBUG 
fprintf(stderr, "profile name = %s\n", proname);
printlist(); /* in jplist.c with DEBUG option */
#endif /* PROFILEDEBUG */
	    temp = getoptvalue(JIM_INITCHAR);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITALPHA))
		    pro->initchar = JIM_ALPHA;
		else if(!strcmp(temp, JIM_INITKATA))
		    pro->initchar = JIM_KATA;
		else if(!strcmp(temp, JIM_INITHIRA))
		    pro->initchar = JIM_HIRA;
		else /* default anyway */
		    pro->initchar = JIM_ALPHA;
	    }
	    temp = getoptvalue(JIM_INITSIZE);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITDBL))
		    pro->initsize = JIM_DOUBLE;
		else
		    pro->initsize = JIM_SINGLE;
	    }
	    temp = getoptvalue(JIM_INITRKC);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITON))
		    pro->initrkc = JIM_RKCON;
		else
		    pro->initrkc = JIM_RKCOFF;
	    }
	    temp = getoptvalue(JIM_PROCONV);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_CONVWORD))
		    pro->conversion = JIM_WORD;
		else if(!strcmp(temp, JIM_CONVSPHRASE))
		    pro->conversion = JIM_SPHRASE;
		else if(!strcmp(temp, JIM_CONVLOOKAHEAD))
		    pro->conversion = JIM_LOOKAHEAD;
		else /* default anyway */
		    pro->conversion = JIM_MPHRASE;
	    }
	    temp = getoptvalue(JIM_PROBEEP);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITOFF))
		    pro->kjbeep = JIM_BEEPOFF;
		else
		    pro->kjbeep = JIM_BEEPON;
	    }
	    temp = getoptvalue(JIM_PROLEARN);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITOFF))
		    pro->learning = JIM_LEARNOFF;
		else
		    pro->learning = JIM_LEARNON;
	    }
	    temp = getoptvalue(JIM_PROALPHA);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_INITOFF))
		    pro->alphanum = JIM_ALPHAOFF;
		else
		    pro->alphanum = JIM_ALPHAON;
	    }
	    temp = getoptvalue(JIM_PROKUTEN);
	    if(temp) { /* specified option found */
		if(!strcmp(temp, JIM_JIS78))
		    pro->kuten = JIM_KUTEN78;
		else
		    pro->kuten = JIM_KUTEN83;
	    }

            temp = getoptvalue(JIM_PROSHNUM);
            if(temp) { /* specified option found */
		pnum = temp;
		inum = *pnum;
		numerr = 0;
		while ( inum != 0 ) {
		    if ( !isdigit( inum ) ) {
			numerr++;
			break;
		    }
		    inum = *(pnum++);
		}
		if ( ! numerr ) {
		    inum = atoi( temp );
		    if ( inum > UCHAR_MAX )
			inum = UCHAR_MAX;
                    pro->shmatnum = inum;
		}
		else
                    pro->shmatnum = JIM_SHNUMDEF;
            }
            temp = getoptvalue(JIM_PROUDLOAD);
            if(temp) { /* specified option found */
                if(!strcmp(temp, JIM_INITOFF))
                    pro->udload = JIM_UDLOADOFF;
                else
                    pro->udload = JIM_UDLOADON;
            }
            temp = getoptvalue(JIM_PRONUMBER);
            if(temp) { /* specified option found */
                if(!strcmp(temp, JIM_KANJIINPUT))
                    pro->numberinput = JIM_IBMKANJI;
                else
                    pro->numberinput = JIM_JISKUTEN;
            }
            temp = getoptvalue(JIM_PRORESET);
            if(temp) { /* specified option found */
                if(!strcmp(temp, JIM_INITON))
                    pro->modereset = JIM_RESETON;
                else
                    pro->modereset = JIM_RESETOFF;
            }
            temp = getoptvalue(JIM_SUPPORTSELECTION);
            if(temp) { /* specified option found */
                if(!strcmp(temp, JIM_INITON))
                    pro->supportselection = JIM_SELECTIONON;
                else if(!strcmp(temp, JIM_INITOFF))
                    pro->supportselection = JIM_SELECTIONOFF;
                else
                    pro->supportselection = 0;
            }
            temp = getoptvalue("doublequote");
            if(temp) { /* specified option found */
		int high, low;
		if(sscanf(temp, "%02x%02x", &high, &low) == 2){
		    Set_tdc_tbl(0x22, high, low);
		}
	    }
            temp = getoptvalue("singlequote");
            if(temp) { /* specified option found */
		int high, low;
		if(sscanf(temp, "%02x%02x", &high, &low) == 2){
		    Set_tdc_tbl(0x27, high, low);
		}
	    }

	    closeprofile();
	} /* end of getprofile */
	free(proname);
    } /* end of proname */

    /*************************************************/
    /* find what dictionaries                        */
    /* when no file is found, inform caller of error */
    /*************************************************/
    pro->dictname.sys = findsysdict(R_OK);
    if( !pro->dictname.sys[0] )
	return(FALSE);  	/* Dictionary Not Found */
    if( !(pro->dictname.usr = findusrdict(R_OK|W_OK)))
    	if( !(pro->dictname.usr = findusrdict(R_OK))) 
	    return(FALSE);  	/* Dictionary Not Found */
    if( !(pro->dictname.adj = findadjdict(R_OK|W_OK)))
    	if( !(pro->dictname.adj = findadjdict(R_OK)))
	    return(FALSE);  	/* Dictionary Not Found */
#ifdef DBG_MSG
for(i=0; pro->dictname.sys[i] != NULL;i++ )
    fprintf(stderr, "jfep:JIMCreate():sys name[%d] = [%s]\n", i,pro->dictname.sys[i]);
fprintf(stderr, "jfep:JIMCreate():usr name = [%s]\n", pro->dictname.usr);
fprintf(stderr, "jfep:JIMCreate():adj name = [%s]\n", pro->dictname.adj);
#endif

    return(TRUE);  /* dictionary found */
} /* end of JIMmakeprofile */

/*-----------------------------------------------------------------------*
*       beginning of JIMfreedictname
*          free memory to hold dictionary names which is
*          allocated by dictionary find functions
*-----------------------------------------------------------------------*/
static void JIMfreedictname(jimprofile *pro, jedprofile *jedp)
{
    int	i;

    for( i = 0;i < SDICT_NUM; i++ ) { 
        if(pro->dictname.sys[i] == NULL)
	    break;
        else
	    free(pro->dictname.sys[i]);
    }
    if(pro->dictname.usr)
	free(pro->dictname.usr);
    if(pro->dictname.adj)
	free(pro->dictname.adj);
} /* end of JIMfreedictname */

/*-----------------------------------------------------------------------*
*	Beginning of JIMtranspro
*	    make JIMED profile translting information from
*	    JIMED profile structure 
*-----------------------------------------------------------------------*/
static void JIMtranspro(jimprofile *jimp, jedprofile *jedp)
{
    int	i;

    /* conversion mode */
    if(jimp->conversion == JIM_WORD)
	jedp->convmode = KP_CONV_WORD;
    else if(jimp->conversion == JIM_SPHRASE)
	jedp->convmode = KP_CONV_SPHRASE;
    else if(jimp->conversion == JIM_MPHRASE)
	jedp->convmode = KP_CONV_MPHRASE;
    else
	jedp->convmode = KP_CONV_LOOKAHEAD;
    /* beep */
    if(jimp->kjbeep == JIM_BEEPON)
	jedp->kjbeep = KP_BEEPON;
    else
	jedp->kjbeep = KP_BEEPOFF;
    /* learning */
    if(jimp->learning == JIM_LEARNON)
	jedp->learn = KP_LEARNON;
    else
	jedp->learn = KP_LEARNOFF;
    /* alphanum */
    if(jimp->alphanum == JIM_ALPHAON)
	jedp->alphanum = KP_ALPHAON;
    else
	jedp->alphanum = KP_ALPHAOFF;
    /* dictionary names */
    for( i = 0; jimp->dictname.sys[i] != NULL; i++ )
	jedp->dictstru.sys[i]  = jimp->dictname.sys[i];
    jedp->dictstru.user = jimp->dictname.usr;
    jedp->dictstru.adj = jimp->dictname.adj;

#if DBG_MSG
for( i = 0; jedp->dictstru.sys[i] != NULL; i++ )
  printf("jfep:JIMCreate():jedp->dictstru.sys[%d] = [%s]\n", i, jedp->dictstru.sys[i] );
printf("jfep:JIMCreate():jedp->dictstru.user = [%s]\n", jedp->dictstru.user );
printf("jfep:JIMCreate():jedp->dictstru.adj  = [%s]\n", jedp->dictstru.adj  );
#endif

    if(jimp->kuten == JIM_KUTEN78 )
	jedp->kuten = KP_KUTEN78;
    else
	jedp->kuten = KP_KUTEN83;
    /* shmatnum */
    jedp->shmatnum = jimp->shmatnum;
    /* udload */
    if(jimp->udload == JIM_UDLOADON)
        jedp->udload = KP_UDLOADON;
    else
        jedp->udload = KP_UDLOADOFF;
    /* numberinput */
    if(jimp->numberinput == JIM_JISKUTEN )
        jedp->numberinput = KP_JISKUTEN;
    else
        jedp->numberinput = KP_IBMKANJI;
    /* modereset */
    if(jimp->modereset == JIM_RESETON)
        jedp->modereset = KP_RESETON;
    else
        jedp->modereset = KP_RESETOFF;


} /* end of JIMtranspro */

/*
 *	JIMCreate()
 */
JIMOBJ JIMCreate(imfep, imcallback, udata)
JIMFEP imfep;
IMCallback *imcallback;
caddr_t udata;
{
    extern int	      jedControl();
    extern InputMode  jedGetInputMode();

    JIMOBJ  obj;
    static      jedprofile      prostr;
    jimprofile	jimpro;
    int     arg;
    int     i;
    int     auxformat;
    int     ret;
    char    **aux_str;
    char    **aux_atr;
    InputMode imode;
    int     textmaxlen;
    int     auxcolmax;

    SetCurrentSDICTDATA(&(imfep->sdictdata));
    SetCurrentUDICTINFO(&(imfep->udictinfo));
    SetCurrentFDICTINFO(&(imfep->fdictinfo));

    /*************************/
    /* initialize error code */
    /*************************/
    imfep->common.imerrno = IMNoError;

    /*************************************************/
    /* allocate Japanese input method object	     */
    /*************************************************/
    obj = (JIMOBJ)malloc(sizeof(JIMobj));

    /***************************************/
    /* allocate additional data structures */	       
    /***************************************/
    /* determine text buffer size in byte			*/
    /* textmaxwidth is specified as "display width", in SJIS	*/
    /* case, it can be seen as "buffer size in byte", however	*/
    /* in EUC case, we need larger buffer such that it can	*/
    /* include some of control codes				*/
    if (imfep->codeset == JIM_SJISDBCS) {
	textmaxlen = imcallback->textmaxwidth;
	auxcolmax = JIM_AUXCOLMAX;
    }
    else if (imfep->codeset == JIM_SJISMIX) {
	textmaxlen = imcallback->textmaxwidth;
	auxcolmax = JIM_AUXCOLMAX;
    }
    else /* JIM_EUC : in this case, more bytes than width required */ {
	textmaxlen = imcallback->textmaxwidth * 2;
	auxcolmax = JIM_AUXCOLMAX * 2;
    }

    /* string/attribute buffers for Text Info. */
    obj->textinfo.text.str = malloc(textmaxlen);
    obj->textinfo.text.att = malloc(textmaxlen);

    /* buffers for Aux Info. */
    /* IMSTRATT array */
    obj->auxinfo.message.text = 
	  (IMSTRATT *)malloc(sizeof(IMSTRATT) * JIM_AUXROWMAX);
    /* string and attribute buffer */
    for(i = 0; i < JIM_AUXROWMAX; i++)  {
	obj->auxinfo.message.text[i].str = malloc(auxcolmax);
	obj->auxinfo.message.text[i].att = malloc(auxcolmax);
    }

    /* string buffer for GetString ioctl */
    obj->string.str = malloc(textmaxlen);

    /* indicator string buffer */
    obj->indstr.str = malloc(JIM_INDSTRMAXLEN);

    /* output buffer for Process */
    obj->outstr = malloc(textmaxlen);

    /******************************/
    /* initialize allocated above */
    /******************************/
    /* object initialization common part */
    obj->imobject.imfep = (IMFep)imfep;
    obj->imobject.cb = imcallback;
    obj->imobject.udata = udata;

    /* object initialization japanese part */
    obj->textinfo.maxwidth = 0;
    obj->textinfo.cur = 0;
    obj->textinfo.chgtop = 0;
    obj->textinfo.chglen = 0;
    obj->textinfo.text.len = 0;
    obj->auxinfo.title.len = 0;
    obj->auxinfo.title.str = NULL;
    obj->auxinfo.message.maxwidth = 0;
    obj->auxinfo.message.nline = 0;
    obj->auxinfo.message.cursor = FALSE;
    obj->auxinfo.message.cur_row = 0;
    obj->auxinfo.message.cur_col = 0;
    obj->auxinfo.button = IM_NONE;
    obj->auxinfo.selection.panel_row = 0;
    obj->auxinfo.selection.panel_col = 0;
    obj->auxinfo.selection.panel = NULL;
    obj->auxinfo.hint = 0;
    obj->auxinfo.status = IMAuxHidden;
    obj->string.len = 0;
    obj->indstr.len = 0;
    obj->indinfo.size = 0;
    obj->indinfo.insert = 0;
    obj->indinfo.unique = 0;
    obj->q_state.mode = 0;
    obj->q_state.text = 0;
    obj->q_state.aux = 0;
    obj->q_state.indicator = 0;
    obj->q_state.beep = 0;
    obj->auxid = 0;
    obj->auxidflag = FALSE;
    obj->auxstate = 0;
    obj->output.data = NULL;
    obj->output.siz = 0;

    /* buffer initializations */
    /* buffer for text */
    memset(obj->textinfo.text.str, '\0', textmaxlen);
    memset(obj->textinfo.text.att, '\0', textmaxlen);

    /* buffer for aux  */
    /* string and attribute buffer */
    for(i = 0; i < JIM_AUXROWMAX; i++) {
	obj->auxinfo.message.text->len = 0;
	memset(obj->auxinfo.message.text[i].str, '\0', auxcolmax);
	memset(obj->auxinfo.message.text[i].att, '\0', auxcolmax);
    }

    /* string buffer for GetString ioctl */
    memset(obj->string.str, '\0', textmaxlen);

    /* indicator string buffer */
    memset(obj->indstr.str, '\0', JIM_INDSTRMAXLEN);

    /* output buffer for Process */
    memset(obj->outstr, '\0', textmaxlen);

    /***************************************************************/
    /* allocate buffers between JIMED				   */
    /* since JIMED generates SJIS string, which 		   */
    /* has same lengths both in byte and column 		   */
    /* buffer lengths are based on value specified as textmaxwidth */
    /***************************************************************/
    /* echo, echo attribute */
    obj->jedinfo.echobufs = malloc(imcallback->textmaxwidth);
    obj->jedinfo.echobufa = malloc(imcallback->textmaxwidth);

    /* aux buffer */
    /* string buffer */
    obj->jedinfo.auxbufs = (char **)malloc(sizeof(caddr_t) * JIM_AUXROWMAX);
    aux_str = obj->jedinfo.auxbufs;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	*(aux_str)++ = malloc(auxcolmax);
    /* attribute buffer */
    obj->jedinfo.auxbufa = (char **)malloc(sizeof(caddr_t) * JIM_AUXROWMAX);
    aux_atr = obj->jedinfo.auxbufa;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	*(aux_atr)++ = malloc(auxcolmax);

    /****************************************/
    /* initialize information between JIMED */
    /****************************************/
    /* echo, echo attribute */
    memset(obj->jedinfo.echobufs, '\0', imcallback->textmaxwidth);
    memset(obj->jedinfo.echobufa, '\0', imcallback->textmaxwidth);
    obj->jedinfo.echosize = imcallback->textmaxwidth;

    /* aux buffer */
    /* string buffer */
    aux_str = obj->jedinfo.auxbufs;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	memset(*(aux_str)++, '\0', auxcolmax);
    /* attribute buffer */
    aux_atr = obj->jedinfo.auxbufa;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	memset(*(aux_atr)++, '\0', auxcolmax);
    obj->jedinfo.auxsize.itemnum = JIM_AUXROWMAX;
    obj->jedinfo.auxsize.itemsize = auxcolmax;

    /**********************************************************/
    /* profiling					      */
    /* retrieve information from profile, making JIM profile  */
    /* structure					      */
    /**********************************************************/
    if(JIMmakeprofile(&jimpro) == TRUE) {
	/**************************/
	/* have JIMED initialized */
	/**************************/
	auxformat = KP_LONGAUX;
	/* make profile for JIMED from profile structure */
	JIMtranspro(&jimpro, &prostr);

	/***********************************/
	/* set the max number mapped files */
	/***********************************/
	imfep->sdictdata.shmatcnt = jimpro.shmatnum;
	/* initilization */
	ret = jedInit(obj->jedinfo.echobufs,
		      obj->jedinfo.echobufa,
		      obj->jedinfo.echosize,
		      obj->jedinfo.auxbufs,
		      obj->jedinfo.auxbufa,
		      &(obj->jedinfo.auxsize),
		      auxformat,
		      &prostr);
	if(ret == KP_ERR)
	    imfep->common.imerrno = JIMEDError;

    } /* end of makeprofile == TRUE */
    else {
	ret = KP_ERR;
	imfep->common.imerrno = JIMDictError;
    }

    /***************************************/
    /* free allocated for dictionary names */
    /***************************************/
    JIMfreedictname(&jimpro, &prostr);

    /**********************************/
    /* JIMED successfully initialized */
    /**********************************/
    if(ret != KP_ERR) {
	/***************************/
	/* save return from editor */
	/***************************/
	obj->jedinfo.jeid = ret;

	/*******************************/
	/* set code set to DBCS or mix */
	/*******************************/
	if (imfep->codeset == JIM_SJISDBCS)
	    arg = KP_ONLYDBCS;
	else if (imfep->codeset == JIM_SJISMIX)
	    arg = KP_MIX;
	else 
	    arg = KP_MIX;

	jedControl(obj->jedinfo.jeid, KP_SETLANG, arg);

	/**************************************************************/
	/* set initial shift state according to profile specification */
	/**************************************************************/
	imode = jedGetInputMode(obj->jedinfo.jeid);
	if(jimpro.initchar == JIM_ALPHA)
	    imode.ind0 = KP_ALPHANUM;
	else if(jimpro.initchar == JIM_KATA)
	    imode.ind0 = KP_KATAKANA;
	else 
	    imode.ind0 = KP_HIRAGANA;
	if(jimpro.initsize == JIM_SINGLE) {
	    if(imfep->codeset != JIM_SJISDBCS)
		imode.ind1 = KP_SINGLE;
        }
	else
	    imode.ind1 = KP_DOUBLE;
	if(jimpro.initrkc == JIM_RKCON)
	    imode.ind2 = KP_ROMAJI_ON;
	else
	    imode.ind2 = KP_ROMAJI_OFF;
	jedSetInputMode(obj->jedinfo.jeid, imode);

	obj->modereset = jimpro.modereset;
	obj->supportselection = jimpro.supportselection;

	/******************************/
	/* make indicator information */
	/******************************/
	makeindinfo(obj);
	makequerystate(obj);
	if(obj->q_state.mode == IMNormalMode) {
	    if(imcallback->indicatordraw) 
		if((*(imcallback->indicatordraw))(obj, 
			  &(obj->indinfo), udata) == IMError) {
		    imfep->common.imerrno = IMCallbackError;
		    ret = KP_ERR; /* pretend error from editor */
		}
	}

       /***************************************/
       /* initialize registration information */
       /***************************************/
       obj->registration = NULL;

    } /* end of ret != KP_ERR */

    /***************************************************************/
    /* if error occurs, do clean up                                */
    /* until this point, "ret" is set by following three reasons   */
    /* 1. result of jedInit                                        */
    /* 2. result of JIMmakeprofile with dictionary access error    */
    /* 3. result of indicatordraw callback                         */
    /* above are distinguished via "imerrno" set at the point      */
    /* where particular error is detected                          */
    /***************************************************************/
    if(ret == KP_ERR) {/* error returned */
	/***************************************/
	/* free all resources allocated so far */
	/***************************************/
	/* echo, echo attribute */
	free(obj->jedinfo.echobufs);
	free(obj->jedinfo.echobufa);

	/* aux buffers */
	aux_str = obj->jedinfo.auxbufs;
	for(i = 0; i < JIM_AUXROWMAX; i++)
	    free(*aux_str++);
	free(obj->jedinfo.auxbufs);
	aux_atr = obj->jedinfo.auxbufa;
	for(i = 0; i < JIM_AUXROWMAX; i++)
	    free(*aux_atr++);
	free(obj->jedinfo.auxbufa);

	/* string/attribute buffers for Text Info. */
	free(obj->textinfo.text.str);
	free(obj->textinfo.text.att);

	/* buffers for Aux Info. */
	for(i = 0; i < JIM_AUXROWMAX; i++) {
	    free(obj->auxinfo.message.text[i].str);
	    free(obj->auxinfo.message.text[i].att);
	}
	free(obj->auxinfo.message.text); 

	/* string buffer for GetString ioctl */
	free(obj->string.str);

	/* indicator string buffer */
	free(obj->indstr.str);

	/* output buffer for process */
	free(obj->outstr);

	/* JIM object structure */ 
	free(obj);

	/****************/
	/* inform error */
	/****************/
	obj = NULL;

    } /* end of if KP_ERR */

    /*******************/
    /* return IMobject */
    /*******************/
    return(obj);
} /* end of create */
