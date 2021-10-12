static char sccsid[] = "@(#)53  1.3  src/bos/usr/lib/nls/loc/imk/kfep/KIMCreate.c, libkr, bos411, 9428A410j 3/23/93 20:34:54";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		KIMCreate.c
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
 *  Module:       KIMCreate.c
 *
 *  Description:  Korean Input Method Create
 *
 *  Functions:    KIMCreate()
 * 		  KIMmakeprofile()
 * 		  KIMtranspro()
 * 		  KIMfreedictname()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include Header
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <unistd.h>
#include <im.h>
#include <imP.h>
#include <imlang.h>
#include "kfep.h"
#include "kimerrno.h"         /* Korean Input Method error defs  */
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/

IMObject	KIMCreate(imfep, imcallback, udata)
KIMFEP 		imfep;
IMCallback 	*imcallback;
caddr_t 	udata;
{
  /***********************/
  /* external references */
  /***********************/
  extern caddr_t    malloc();
  extern int	    kedControl();
  extern InputMode  kedGetInputMode();

  /*******************/  
  /* local variables */
  /*******************/  
  KIMOBJ  	obj;
  KIMED		*kimed;
  int     	arg;
  register int  i;
  int     	ret;
  char    	**aux_str;
  char    	**aux_atr;
  int     	textmaxlen;
  int		auxformat;
  AuxSize	auxsize;
  unsigned char *sysdict;
  int		cberrflag = FALSE;
  int		KIMmakeprofile();
  void		KIMtranspro();
  void		KIMfreedictname();
  kedprofile	prostr;
  kimprofile	kimpro;
  InputMode	imode;

  /*************************/
  /* initialize error code */
  /*************************/
  imfep->common.imerrno = IMNoError;

  /*************************************************/
  /* allocate Korean input method object	     */
  /*************************************************/
  obj = (KIMOBJ)malloc(sizeof(KIMobj));

  /**************************************/
  /* string buffer size.		*/
  /**************************************/
  textmaxlen = imcallback->textmaxwidth;

  /*************************************************/
  /* string/attribute buffers for Text Info. */
  /*************************************************/
  obj->textinfo.text.str = (unsigned char *)malloc(textmaxlen);
  obj->textinfo.text.att =(unsigned char *)malloc(textmaxlen);
  obj->textinfo.maxwidth = 	KIM_TXT_MINWIDTH;
  obj->textinfo.cur 	 = 	KIM_TXT_CURFIRST;
  obj->textinfo.chgtop 	 = 	KIM_TXT_CURFIRST;
  obj->textinfo.chglen 	 = 	KIM_TXT_NOCHANGED_LEN;
  obj->textinfo.text.len = 	KIM_TXT_EMTY;
  memset(obj->textinfo.text.str, NULL, textmaxlen);
  memset(obj->textinfo.text.att, NULL, textmaxlen);

  /*************************************/
  /* Callback uses bellow data.        */
  /* Allocates here.		       */
  /* They are displayed in Aux window. */
  /*************************************/
  obj->auxinfo.message.text = 
	(IMSTRATT *)malloc(sizeof(IMSTRATT) * KIM_MSGTXT_MAXROW); 
  for(i = 0; i < KIM_MSGTXT_MAXROW; i++)  {
	obj->auxinfo.message.text[i].str = (unsigned char*)malloc(KIM_MSGTXT_MAXCOL);
	obj->auxinfo.message.text[i].att = (unsigned char*)malloc(KIM_MSGTXT_MAXCOL);
  }
  /**************************************/
  /* Initialize here.			*/
  /**************************************/
  for(i = 0; i < KIM_MSGTXT_MAXROW; i++) {
	obj->auxinfo.message.text[i].len = KIM_MSGTXT_EMTY;
	memset(obj->auxinfo.message.text[i].str, NULL, KIM_MSGTXT_MAXCOL);
	memset(obj->auxinfo.message.text[i].att, NULL, KIM_MSGTXT_MAXCOL);
  }
  obj->auxinfo.message.maxwidth = 	KIM_MSGTXT_MINWIDTH;
  obj->auxinfo.message.nline    = 	KIM_MSGTXT_EMTY;
  obj->auxinfo.message.cursor  	= 	KIM_MSG_CUROFF;
  obj->auxinfo.message.cur_row 	= 	KIM_MSG_ROWPOS_FIRST;
  obj->auxinfo.message.cur_col 	= 	KIM_MSG_COLPOS_FIRST;
  obj->auxinfo.title.len 	=	0 ;
  obj->auxinfo.title.str	= 	NULL ;
  obj->auxinfo.button = 		IM_NONE;
  obj->auxinfo.selection.panel_row = 	KIM_NOPANEL_ROW;
  obj->auxinfo.selection.panel_col = 	KIM_NOPANEL_COL;
  obj->auxinfo.selection.panel = 	KIM_NOPANEL;
  obj->auxinfo.hint = 			0;
  obj->auxinfo.status = 		IMAuxHidden;

  obj->auxid = 				NULL;
  obj->auxidflag = 			KIM_AUXID_NOTSETTED;
  obj->auxstate = 			KIM_AUXEMTY;

  /*************************************/
  /* string buffer for GetString ioctl */
  /*************************************/
  obj->string.str = 	(unsigned char *)malloc(textmaxlen);
  obj->string.len = 	KIM_STRBUF_EMTY;
  memset(obj->string.str, NULL, textmaxlen);

  /*************************************/
  /* indicator string buffer */
  /*************************************/
  obj->indstr.str = (unsigned char *)malloc(KIM_INDSTR_MAXLEN);
  obj->indstr.len = KIM_INDSTR_EMTY;
  memset(obj->indstr.str, NULL, KIM_INDSTR_MAXLEN);

  /*************************************/
  /* output buffer for Process */
  /*************************************/
  obj->outstr = (unsigned char *)malloc(textmaxlen);
  memset(obj->outstr, NULL, textmaxlen);

  obj->output.data =(caddr_t)malloc(textmaxlen);
  obj->output.len = 0;
  obj->output.siz = textmaxlen;
  memset(obj->output.data, NULL, textmaxlen);
  /*************************************/
  /* object initialization common part */
  /*************************************/
  obj->imobject.imfep = 	(IMFep)imfep;
  obj->imobject.cb = 		imcallback;
  obj->imobject.udata = 	udata;

  /********************************************/
  /* Bellow field describes indicator status. */
  /********************************************/
  /* Bellow info reinitialized at kedInit. */
  /********************************************/
  obj->indinfo.size = 	IMHalfWidth;
  obj->indinfo.insert = IMReplaceMode;
  obj->indinfo.unique = KIM_SH_ALPHA|KIM_SH_HANJAOFF;

  /********************************************/
  /* Bellow field describes FEP status.	      */
  /********************************************/
  obj->q_state.mode = 		IMSuppressedMode;
  obj->q_state.text = 		OFF;
  obj->q_state.aux = 		OFF;
  obj->q_state.indicator = 	OFF;
  obj->q_state.beep = 		KIM_NOBEEP;

  /**********************************************************/
  /* profiling                                              */
  /* retrieve information from profile, making KIM profile  */
  /* structure                                              */
  /**********************************************************/
  if(KIMmakeprofile(&kimpro) == TRUE) {
        /*************************************/
	/* The dictionary file was found.     */
        /*************************************/

        /*************************************************/
        /* make profile for KIMED from profile structure */
        /*************************************************/
        KIMtranspro(&kimpro, &prostr);

        /**************************/
        /* initilization 	  */
        /**************************/
  	auxformat 	  = 	KIM_AUX_LONGFORMAT;
	auxsize.itemnum   = 	KIM_AUXROWMAX;
	auxsize.itemsize  = 	KIM_AUXCOLMAX;

        /**************************/
        /* initilization KIMED 	  */
        /**************************/
  	ret = kedInit(textmaxlen, auxformat, auxsize, &prostr); 

        if(ret == KP_ERR)
            imfep->common.imerrno = KIMEDError;

#ifdef DEBUG
        if(ret > 0)
            dump_keid(ret);
#endif DEBUG

    } /* end of makeprofile == TRUE */
    else {
        /*************************************/
	/* The dictionary file was not found. */
        /*************************************/
        ret = KP_ERR;
        imfep->common.imerrno = KIMDictError;
    }

  /***************************************/
  /* free allocated for dictionary names */
  /***************************************/
  KIMfreedictname(&kimpro);

  /**********************************/
  /* Make init.			    */
  /**********************************/
  kimed = (KIMED*) ret;

  /**********************************/
  /* KIMED successfully initialized */
  /**********************************/
  if(ret != KP_ERR) {
	/***************************/
	/* save return from editor */
	/***************************/
      	obj->kimed = ret;

        /**************************************************************/
        /* set initial mode according to profile specification 	      */
        /**************************************************************/
        imode = kedGetInputMode(kimed);

	/************/
	/* basemode */
	/************/
        if(kimpro.initbase == KIM_ALPHANUM) imode.basemode = MD_ENG;
        else if(kimpro.initbase == KIM_HANGEUL) {
		imode.basemode = MD_HAN;
		imode.supnormode = MD_NORM;
	}
        else  {
		imode.basemode = MD_JAMO;
		imode.supnormode = MD_NORM;
	}

	/****************/
	/* size 	*/
	/****************/
        if(kimpro.initsize == KIM_BANJA)  imode.sizemode = MD_BANJA; 
        else {
		imode.sizemode = MD_JEONJA;
		imode.supnormode = MD_NORM;
	}

	/************/
	/* hanja    */
	/************/
        if(kimpro.inithanja == KIM_HANJAON) {
		imode.hjmode = MD_HJON;
		imode.supnormode = MD_NORM;
	}
        else imode.hjmode = MD_HJOFF;

	/************/
	/* insrep   */
	/************/
	if (kimpro.initinsrep == KIM_INSERT) imode.insrepmode = MD_INSERT;
	else imode.insrepmode = MD_REPLACE;

        /**************************************************************/
        /* set initial mode according to profile specification 	      */
        /**************************************************************/
        kedSetInputMode(kimed, imode);
	kedSetInterState(kimed, imode);

	/******************************/
	/* make indicator information */
	/* Here draw indicator first  */
	/******************************/
	makeindinfo(obj);
	makequerystate(obj);
        if(obj->q_state.mode == IMNormalMode) {
	  if(imcallback->indicatordraw) 
	    if((*(imcallback->indicatordraw))
		  (obj, &(obj->indinfo), udata) == IMError) 
	    {
		  imfep->common.imerrno = IMCallbackError;
		  ret = KP_ERR; /* pretend error from editor */
		  if (ret == KP_ERR) 
			cberrflag = TRUE;	
	     }
	} 
  } /* end of ret != KP_ERR */

  /***************************************************************/
  /* if error occurs, do clean up                                */
  /* until this point, "ret" is set by following three reasons   */
  /* 1. result of kedInit                                        */
  /* 2. result of KIMmakeprofile with dictionary access error    */
  /* 3. result of indicatordraw callback                         */
  /* above are distinguished via "imerrno" set at the point      */
  /* where particular error is detected                          */
  /***************************************************************/

  if (cberrflag == TRUE) { 
	/******************************************************/
	/* Free all kimed's resources allocated so far        */
	/* Now, The Kimed structure is sucessfully allocated. */
	/* But cannot continue, because Callback error.       */
	/******************************************************/
	kedClose(kimed);
  }

  if(ret == KP_ERR) {
	/***************************************/
	/* Free all resources allocated so far */
	/* Now, not necessary deallocate kimed */
	/* Because kimed is aleady freed or    */
	/* allocated.			       */
	/***************************************/

	/***************************************/
	/* string/attribute buffers for Text Info. */
	/***************************************/
	free(obj->textinfo.text.str);
	free(obj->textinfo.text.att);

	/***************************************/
	/* buffers for Aux Info. */
	/***************************************/
	for(i = 0; i < KIM_MSGTXT_MAXROW; i++) {
	  free(obj->auxinfo.message.text[i].str);
	  free(obj->auxinfo.message.text[i].att);
	}
	free(obj->auxinfo.message.text); 

	/***************************************/
	/* string buffer for GetString ioctl */
	/***************************************/
	free(obj->string.str);

	/***************************************/
	/* indicator string buffer */
	/***************************************/
	free(obj->indstr.str);

	/***************************************/
	/* output buffer for process */
	/***************************************/
	free(obj->outstr);

        free((caddr_t)obj->output.data);

	/***************************************/
	/* KIM object structure */ 
	/***************************************/
	free(obj);

	/****************/
	/* inform error */
	/****************/
	obj = NULL;

  } /* end of if KP_ERR */

  /*******************/
  /* return IMobject */
  /*******************/
  return((IMObject)obj);
} /* end of create */

/*-----------------------------------------------------------------------*
*       Beginning of KIMmakeprofile
*           retrieve profile information from KIM profile file
*           or set to defaults
*       return FLASE if dictionary is not found, TRUE otherwise
*-----------------------------------------------------------------------*/
static int KIMmakeprofile(pro)
kimprofile *pro;
{
    extern int getprofile();
    extern char *getoptvalue();
    extern void closeprofile();
    extern char *searchprofile();
    extern char *findsysdict();
    extern char *findusrdict();
    char *temp;
    char *proname;

    /*******************************/
    /* set every field to defaults */
    /*******************************/
    pro->initbase 	= KIM_ALPHANUM;
    pro->initsize 	= KIM_BANJA;
    pro->inithanja 	= KIM_HANJAOFF;
    pro->initbeep  	= KIM_BEEPON;
    pro->initlearning 	= KIM_LEARNOFF;
    pro->initinsrep	= KIM_REPLACE;
    pro->dictname.sys 	= NULL;
    pro->dictname.usr 	= NULL;
    pro->initacm        = KIM_ACMOFF;

    /*************************************************/
    /* first, search profile file in following order */
    /*************************************************/
    proname = searchprofile(R_OK);

    /***************************************************/
    /* if file exists, retrieve information from it,   */
    /* updating information in structure               */
    /***************************************************/
    if(proname) {
        if(getprofile(proname)) {
            temp = getoptvalue(KIM_INITBASE);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_ALPHANUM))
                    pro->initbase = KIM_ALPHANUM;
                else if(!strcmp(temp, KIM_I_HANGEUL))
                    pro->initbase = KIM_HANGEUL;
                else /* default anyway */
                    pro->initbase = KIM_JAMO;
            }

            temp = getoptvalue(KIM_INITSIZE);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_BANJA))
                    pro->initsize = KIM_BANJA;
                else
                    pro->initsize = KIM_JEONJA;
            }

            temp = getoptvalue(KIM_INITHANJA);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_HJON))
                    pro->inithanja = KIM_HANJAON;
                else
                    pro->inithanja = KIM_HANJAOFF;
            }

            temp = getoptvalue(KIM_INITBEEP);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_BPOFF))
                    pro->initbeep = KIM_BEEPOFF;
                else
                    pro->initbeep = KIM_BEEPON;
            }

            temp = getoptvalue(KIM_INITINSREP);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_INSERT))
                    pro->initinsrep = KIM_INSERT;
                else
                    pro->initinsrep = KIM_REPLACE;
            }

            temp = getoptvalue(KIM_INITLEARN);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_LEARNON))
                    pro->initlearning = KIM_LEARNON;
                else
                    pro->initlearning = KIM_LEARNOFF;
            }

            temp = getoptvalue(KIM_INITACM);
            if(temp) { 
    		/**************************/
		/* specified option found */
    		/**************************/
                if(!strcmp(temp, KIM_I_ACMON))
                    pro->initacm = KIM_ACMON;
                else
                    pro->initacm = KIM_ACMOFF;
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
    pro->dictname.usr = findusrdict(R_OK | W_OK);

#ifdef DICTDEBUG
/*******
fprintf(stderr, "sys name = %s\n", pro->dictname.sys);
fprintf(stderr, "usr name = %s\n", pro->dictname.usr);
*******/
#endif DICTDEBUG

    if(pro->dictname.sys != NULL) 
    	/************************/
    	/* dictionary found     */
    	/************************/
	return (TRUE);
    else 
    	/************************/
	/* dictionary not found */
    	/************************/
	return(FALSE);  

} /* end of KIMmakeprofile */

/**********************************************************************
*
*       beginning of KIMfreedictname
*          free memory to hold dictionary names which is
*          allocated by dictionary find functions
**********************************************************************/
static void KIMfreedictname(pro)
kimprofile *pro;
{
    if(pro->dictname.sys) free(pro->dictname.sys);
    if(pro->dictname.usr) free(pro->dictname.usr);
} /* end of KIMfreedictname */

/**********************************************************************
*
*       Beginning of KIMtranspro
*           make KIMED profile translting information from
*           KIMED profile structure
**********************************************************************/
static void KIMtranspro(kimp, kedp)
kimprofile *kimp;
kedprofile *kedp;
{
	/**************************/
    	/* learning 		  */
   	/**************************/
    	if (kimp->initlearning == KIM_LEARNON) kedp->learn = KP_LEARNON;
    	else kedp->learn = KP_LEARNOFF;

	/******************/
    	/* ACM 		  */
   	/******************/
    	if (kimp->initacm == KIM_ACMON) kedp->acm = KP_ACMON;
    	else kedp->acm = KP_ACMOFF;

   	/**************************/
    	/* dictionary names       */
   	/**************************/
    	kedp->dictstru.sys = kimp->dictname.sys;
    	kedp->dictstru.user = kimp->dictname.usr;
} 	/* end of KIMtranspro */

