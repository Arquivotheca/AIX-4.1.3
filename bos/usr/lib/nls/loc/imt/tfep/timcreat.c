static char sccsid[] = "@(#)18  1.4.1.1  src/bos/usr/lib/nls/loc/imt/tfep/timcreat.c, libtw, bos411, 9431A411a 7/26/94 17:13:26";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TIMCreate
 *              TIMfreedictname
 *              TIMmakeprofile
 *              low
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION *******************/
/*                                                                     */
/* MODULE NAME:        TIMCreate                                       */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Object Creation            */
/*                                                                     */
/* FUNCTION:           TIMCreate   :  Create Object                    */
/*                                                                     */
/* TIMmakeprofile  : Make TIM Profile                                  */
/*                                                                     */
/* TIMfreedictname : Free Dict. Name                                   */
/*                                                                     */
/* low  :  Convert Lower                                               */
/*                                                                     */
/* MODULE TYPE:        C                                               */
/*                                                                     */
/* COMPILER:           AIX C                                           */
/*                                                                     */
/* AUTHOR:             Terry Chou                                      */
/*                                                                     */
/* STATUS:             Chinese Input Method Version 1.0                */
/*                                                                     */
/* CHANGE ACTIVITY:                                                    */
/********************* END OF SPECIFICATIONS ****************************/

/*-----------------------------------------------------------------------*
*       Include files
*-----------------------------------------------------------------------*/
#include <langinfo.h>      /* @g5 */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <imlang.h>
#include "taiwan.h"
#include "message.h"        /* @big5 */

/* ---------------------------------------------------------------------- */
/* External function reference                                            */
/* ---------------------------------------------------------------------- */
extern int      indlinemake();
extern int      querystate();

/*-----------------------------------------------------------------------*
*       Beginning of procedure
*-----------------------------------------------------------------------*/
IMObject        TIMCreate(imfep, imcallback, udata)
   TIMFEP          imfep;
   IMCallback     *imcallback;
   caddr_t         udata;
{
   extern InputMode TedGetInputMode();

   /*******************/
   /* local variables */
   /*******************/
   int             TIMmakeprofile();
   void            TIMfreedictname();
   TIMOBJ          obj;
   TimProfile      timpro;
   int             arg;
   int             i;
   int             auxformat;
   int             ret;
   char          **aux_str;
   char          **aux_atr;
   InputMode       imode;
   int             textmaxlen;

   /*************************/
   /* initialize error code */
   /*************************/
   imfep->common.imerrno = IMNoError;

   /*************************************************/
   /* allocate Japanese input method object          */
   /*************************************************/
   obj = (TIMOBJ) malloc(sizeof(TIMobj));

   /***************************************/
   /* allocate additional data structures */
   /***************************************/

   textmaxlen = imcallback->textmaxwidth;

   /* string/attribute buffers for Text Info. */
   obj->textinfo.text.str = (unsigned char *) malloc(textmaxlen);
   obj->textinfo.text.att = (unsigned char *) malloc(textmaxlen);

   obj->auxinfo.message.text =                   /* buffers for Aux Information */
                                 (IMSTRATT *) malloc(sizeof(IMSTRATT) * TIM_AUXROWMAX);

   for (i = 0; i < TIM_AUXROWMAX; i++)           /* string and attribute buffer */
   {
      obj->auxinfo.message.text[i].str = (unsigned char *)
                                    malloc(TIM_AUXCOLMAX);
      obj->auxinfo.message.text[i].att = (unsigned char *)
                                    malloc(TIM_AUXCOLMAX);
   }

/*!!!   add by terry chou     !!!*/
   obj->auxinfo.selection.panel = malloc(sizeof(IMPanel));
/*!!!   end                   !!!*/

   obj->string.str = (unsigned char *) malloc(textmaxlen);      /* string buffer for GetString */


   obj->indstr.str = (unsigned char *) malloc(TIM_INDSTRMAXLEN);        /* indicator string buffer */

   /* when the convert key is pressed, the final EUC will place into this buffer                                               */

   obj->convert_str = (caddr_t) malloc(textmaxlen);

   obj->output.data = (caddr_t) malloc(TIM_INDSTRMAXLEN);
   obj->output.len = 0;
   obj->output.siz = TIM_INDSTRMAXLEN;

   /******************************/
   /* initialize allocated above */
   /******************************/
   obj->imobject.imfep = (IMFep) imfep;  /* object initialization common part */
   obj->imobject.cb = imcallback;
   obj->imobject.udata = udata;

   obj->textinfo.maxwidth = 0;          /* object initialization chinese part */
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
   obj->auxinfo.selection.panel->maxwidth = 0;
   obj->auxinfo.selection.panel->item_row =0;
   obj->auxinfo.selection.panel->item_col = 0;
   obj->auxinfo.selection.panel->item = NULL;
   obj->auxinfo.hint = 0;
   obj->auxinfo.status = IMAuxHidden;
   obj->string.len = 0;
   obj->indstr.len = 0;
   obj->indinfo.size = 0;
   obj->indinfo.insert = 0;
   obj->indinfo.unique = 0;
   obj->querystate.mode = 0;
   obj->querystate.text = 0;
   obj->querystate.aux = 0;
   obj->querystate.indicator = 0;
   obj->querystate.beep = 0;
   obj->auxID = 0;
   obj->auxIDflag = FALSE;
   obj->textstate = 0;
   obj->auxstate = NOTUSE;

   memset(obj->textinfo.text.str, NULL, textmaxlen);    /* buffer for text */
   memset(obj->textinfo.text.att, NULL, textmaxlen);

   for (i = 0; i < TIM_AUXROWMAX; i++)           /* buffer for aux  */
   {
      obj->auxinfo.message.text[i].len = 0;
      memset(obj->auxinfo.message.text[i].str, NULL, TIM_AUXCOLMAX);
      memset(obj->auxinfo.message.text[i].att, NULL, TIM_AUXCOLMAX);
   }

   memset(obj->string.str, NULL, textmaxlen);    /* string buffer for GetString ioctl */

   memset(obj->indstr.str, NULL, TIM_INDSTRMAXLEN);     /* indicator string buffer */

   /* The final EUC result after pressing press convert key   */

   memset(obj->convert_str, NULL, textmaxlen);

   /***************************************************************/
   /* allocate buffers between JIMED                              */
   /* since TIMED generates string,                               */
   /***************************************************************/
   /* echo buffer, fix buffers */
   obj->tedinfo.echobufs = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->tedinfo.echobufa = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->tedinfo.fixbuf = (unsigned char *) malloc(imcallback->textmaxwidth);

   /* aux string buffer */
   obj->tedinfo.auxbufs = (char **) malloc(sizeof(caddr_t) * TIM_AUXROWMAX);
   aux_str = obj->tedinfo.auxbufs;
   for (i = 0; i < TIM_AUXROWMAX; i++)
      *(aux_str)++ = (unsigned char *) malloc(TIM_AUXCOLMAX);

   /* aux attribute buffer */
   obj->tedinfo.auxbufa = (char **) malloc(sizeof(caddr_t) * TIM_AUXROWMAX);
   aux_atr = obj->tedinfo.auxbufa;
   for (i = 0; i < TIM_AUXROWMAX; i++)
      *(aux_atr)++ = (unsigned char *) malloc(TIM_AUXCOLMAX);

   /****************************************/
   /* initialize information between TIMED */
   /****************************************/
   /* echo buffer and fix buffer initial */
   memset(obj->tedinfo.echobufs, NULL, imcallback->textmaxwidth);
   memset(obj->tedinfo.echobufa, NULL, imcallback->textmaxwidth);
   memset(obj->tedinfo.fixbuf, NULL, imcallback->textmaxwidth);
   obj->tedinfo.echobufsize = imcallback->textmaxwidth;

   aux_str = obj->tedinfo.auxbufs;               /* aux string buffer */
   for (i = 0; i < TIM_AUXROWMAX; i++)
      memset(*(aux_str)++, NULL, TIM_AUXCOLMAX);

   aux_atr = obj->tedinfo.auxbufa;               /* aux attribute buffer */
   for (i = 0; i < TIM_AUXROWMAX; i++)
      memset(*(aux_atr)++, NULL, TIM_AUXCOLMAX);
   obj->tedinfo.auxsize.itemnum = TIM_AUXROWMAX;
   obj->tedinfo.auxsize.itemsize = TIM_AUXCOLMAX;

   /**********************************************************/
   /* profiling                                              */
   /* retrieve information from profile, making TIM profile  */
   /**********************************************************/
   if (TIMmakeprofile(&timpro) == TRUE)
   {
      /************************/
      /* TIMED initialized   */
      /************************/
      auxformat = LONGAUX;
      /* make profile for TIMED from profile structure */

      /* call TedInit to initial fepcb    */
      ret = TedInit(obj->tedinfo.echobufs,
                    obj->tedinfo.echobufa,
                    obj->tedinfo.fixbuf,
                    obj->tedinfo.echobufsize,
                    obj->tedinfo.auxbufs,
                    obj->tedinfo.auxbufa,
                    &(obj->tedinfo.auxsize),
                    auxformat,
                    &timpro);
      if (ret == ERROR)
         imfep->common.imerrno = TIMEDError;

   } else
   {
      ret = ERROR;
      imfep->common.imerrno = TIMDictError;
   }

   /***************************************/
   /* free allocated for dictionary names */
   /***************************************/
   TIMfreedictname(&timpro);

   /**********************************/
   /* TIMED successfully initialized */
   /**********************************/
   if (ret != ERROR)
   {
      obj->tedinfo.TedID = ret;

      /******************************/
      /* make indicator information */
      /******************************/
      indlinemake(obj);
      querystate(obj);
      if (obj->querystate.mode == IMNormalMode)
         indicatordraw(obj);
   }
   /***************************************************************/
   /* if error occurs, do clean up                                */
   /* until this point, "ret" is set by following three reasons   */
   /* 1. result of TedInit                                        */
   /* 2. result of TIMmakeprofile with dictionary access error    */
   /* 3. result of indicatordraw callback                         */
   /***************************************************************/
   if (ret == ERROR)
   {                                             /* error returned */
      /***************************************/
      /* free all resources allocated so far */
      /***************************************/
      free(obj->tedinfo.echobufs);
      free(obj->tedinfo.echobufa);
      free(obj->tedinfo.fixbuf);

      aux_str = obj->tedinfo.auxbufs;            /* aux string buffers */
      for (i = 0; i < TIM_AUXROWMAX; i++)
         free(*aux_str++);
      free(obj->tedinfo.auxbufs);

      aux_atr = obj->tedinfo.auxbufa;            /* aux attribute buffers  */
      for (i = 0; i < TIM_AUXROWMAX; i++)
         free(*aux_atr++);
      free(obj->tedinfo.auxbufa);

      free(obj->textinfo.text.str);              /* string buffers for Text Info. */
      free(obj->textinfo.text.att);

      for (i = 0; i < TIM_AUXROWMAX; i++)        /* buffers for Aux Info. */
      {
         free(obj->auxinfo.message.text[i].str);
         free(obj->auxinfo.message.text[i].att);
      }
      free(obj->auxinfo.message.text);

      if (obj->auxinfo.selection.panel)               /* big5 debby */
        free(obj->auxinfo.selection.panel);           /* big5 debby */

      free(obj->string.str);                     /* string buffer for GetString ioctl */

      if(obj->output.data) free(obj->output.data); /* big5 debby */

      free(obj->indstr.str);                     /* indicator string buffer */
      free(obj->convert_str);                    /* big5 debby */

      free(obj);                                 /* TIM object structure */
      obj = NULL;
   }
   return (obj);
}                                                /* end of create */

/*-----------------------------------------------------------------------*
*       Beginning of TIMmakeprofile
*           retrieve profile information from JIM profile file
*           or set to defaults
*       return FLASE if dictionary is not found, TRUE otherwise
*-----------------------------------------------------------------------*/
static int      TIMmakeprofile(pro)
   TimProfile     *pro;
{
   extern int      openprofile();
   extern char    *getoptvalue();
   extern void     closeprofile();
   extern char    *searchprofile();
   extern char    *FindSysUserDict();
   extern char    *low();
   char           *temp;
   char           *proname;
   unsigned       char i;       /* @big5 */
   iconv_t        iconv_flag;   /* @big5 */

   /*******************************/
   /* set every field to defaults */
   /*******************************/
   pro->initchar = ALPH_NUM_MODE;
   pro->initsize = HALF;
   pro->initbeep = BEEP_ON;
   pro->initselection = SELECTION_ON; /* V4.1 */
   pro->initlearn = LEARN_OFF;        /* V4.1 */
   pro->initkeyboard = IBM_LAYOUT;                               /* @big5 */
   pro->initcodeset = codesetinfo[0].code_type;       /* V4.1 */ /* @big5 */
   pro->iconv_flag = 0;                                          /* @big5 */
/*   pro->initcodeset = EUC_CODE;                     /* V4.1 */ /* @big5 */
   pro->dictname.TJ_sys_dict = NULL;
   pro->dictname.PH_sys_dict = NULL;
   pro->dictname.TJ_usr_dict = NULL;
   pro->dictname.PH_usr_dict = NULL;
   pro->learnfile = NULL;             /* V410 */


   /*************************************************/
   /* first, search profile file in following order */
   /*************************************************/
   proname = searchprofile(READ_ONLY);

   /***************************************************/
   /* if file exists, retrieve information from it,   */
   /* updating information in structure        */
   /***************************************************/
   if (proname)
   {
      if (openprofile(proname))
      {
         temp = low(getoptvalue(INITCHAR));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, TIM_INIT_AN))
               pro->initchar = ALPH_NUM_MODE;
            else
            if (!strcmp(temp, TIM_INIT_TJ))
               pro->initchar = TSANG_JYE_MODE;
            else
            if (!strcmp(temp, TIM_INIT_PH))
               pro->initchar = PHONETIC_MODE;
            else                                 /* default value */
               pro->initchar = ALPH_NUM_MODE;
         }
         temp = low(getoptvalue(INITSIZE));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, TIM_INIT_HALF))
               pro->initsize = HALF;
            else
               pro->initsize = FULL;
         }
         temp = low(getoptvalue(INITBEEP));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, TIM_INIT_BEEP_ON))
               pro->initbeep = BEEP_ON;
            else
               pro->initbeep = BEEP_OFF;
         }
         temp = low(getoptvalue(INITSELECTION)); /* ------ V4.1 -------- */
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, TIM_INIT_SELECTION_ON))
               pro->initselection = SELECTION_ON;
            else
               pro->initselection = SELECTION_OFF;
         }
 /*      temp = low(getoptvalue(INITLEARN));                  V410 */
 /*      if(temp)                                             V410 */
 /*      {                                                    V410 */
 /*         if (!strcmp(temp, TIM_INIT_LEARN_ON))             V410 */
 /*            pro->initlearn = LEARN_ON;                     V410 */
 /*         else                                              V410 */
 /*            pro->initlearn = LEARN_OFF;                    V410 */
 /*      }                                                    V410 */

         temp = getoptvalue(INITLEARN);                    /* V410 */
         if (temp)                                         /* V410 */
          if (access(temp, READ_ONLY|WRITE_ONLY) <= 0)     /* V410 */
          {                                                /* V410 */
             pro->learnfile = malloc((unsigned char)strlen(temp) + 1);/* @V410 */
             strcpy(pro->learnfile,temp);                             /* @V410 */
             pro->initlearn = LEARN_ON;                    /* V410 */
          }                                                /* V410 */

/*       temp = low(getoptvalue(INITCODESET_1));       V4.1   @big5 */
/*       if(temp)                                             @big5 */
/*       {                                                    @big5 */
/*          if (!strcmp(temp, TIM_INIT_BIG5))                 @big5 */
/*             pro->initcodeset = BIG5;                       @big5 */
/*       }                                                    @big5 */

         /*---------- get keyboard layout ------------------- @big5 */
         temp = low(getoptvalue(INITKEYLAYOUT));           /* @big5 */
         if (temp)                                         /* @big5 */
            if (!strcmp(temp, TIM_INIT_NON_IBM_LAYOUT))    /* @big5 */
               pro->initkeyboard = NON_IBM_LAYOUT;         /* @big5 */
            else if (!strcmp(temp, TIM_INIT_ET_LAYOUT))    /* @et   */
               pro->initkeyboard = ET_LAYOUT;              /* @et   */

         /* ----------------------------------- V4.1 -----------------------*/

         closeprofile();
      }
      free(proname);
   }                                             /* end of proname */
    /******************************************************************/
    /* get learning file name GETENV function                    @big5*/
    /*****************************************************************/
    temp = searchlearnfile(READ_ONLY|WRITE_ONLY);             /* @big5 */
    if (temp)                                                 /* @big5 */
    {                                                         /* @big5 */
      pro->initlearn = LEARN_ON;                              /* @big5 */
      pro->learnfile = temp;                                  /* @big5 */
    }                                                         /* @big5 */

    /******************************************************************/
    /* get keyboard layout from GETENV function                  @big5*/
    /*****************************************************************/
    temp = (char *) low(getenv(KEYLAYOUT));                   /* @big5 */
    if (temp)                                                 /* @big5 */
    {                                                         /* @big5 */
      if (!strcmp(temp, TIM_INIT_NON_IBM_LAYOUT))             /* @big5 */
         pro->initkeyboard = NON_IBM_LAYOUT;                  /* @big5 */
      else if (!strcmp(temp, TIM_INIT_ET_LAYOUT))             /* @et   */
         pro->initkeyboard = ET_LAYOUT;                       /* @et   */
    }                                                         /* @big5 */

   /*********************************************************************/
   /* Get CODESET Value                                            big5 */
   /*********************************************************************/
   setlocale(LC_ALL,"");                                       /* @big5 */
   for (i=1;i<CODE_SET_NO ;i++)                                /* @big5 */
   if ((strcmp(nl_langinfo (CODESET), codesetinfo[i].name)) == 0)/*@big5*/
/* if ((strcmp("big5"               , codesetinfo[i].name)) == 0)
                                                               /* @big5 */
   {                                                           /* @big5 */
     pro->initcodeset = codesetinfo[i].code_type;              /* @big5 */
     if (iconv_flag = OpenIconv(codesetinfo[0].code_type,pro->initcodeset))
     {                                                         /* @big5 */
        pro->iconv_flag = iconv_flag;                          /* @big5 */
        Convert_Message(iconv_flag,pro->initcodeset);          /* @big5 */
     }                                                         /* @big5 */
     else                                                      /* @big5 */
       pro->initcodeset = codesetinfo[0].code_type;            /* @big5 */
                                                               /* @big5 */
   }                                                           /* @big5 */

   /******************************************************/
   /*                                                    */
   /* when system file is not found, inform caller error */
   /******************************************************/
   pro->dictname.TJ_sys_dict = FindSysUserDict(TJ_SYS_DICT, READ_ONLY);
   pro->dictname.PH_sys_dict = FindSysUserDict(PH_SYS_DICT, READ_ONLY);
   pro->dictname.TJ_usr_dict = FindSysUserDict(TJ_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.PH_usr_dict = FindSysUserDict(PH_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);
   if (pro->dictname.TJ_sys_dict && pro->dictname.PH_sys_dict)
      return (TRUE);                             /* dictionary found */
   else
      return (FALSE);                            /* dictionary not found */
}

char           *low(temp)
   char           *temp;
{
   char           *ptr;

   ptr = temp;
   while (*ptr != NULL)
   {
      *(ptr) = tolower(*(ptr));
      ptr++;
   }
   return temp;
}


/*-----------------------------------------------------------------------*
*       beginning of TIMfreedictname
*          free memory to hold dictionary names which is
*          allocated by dictionary find functions
*-----------------------------------------------------------------------*/
static void     TIMfreedictname(pro)
   TimProfile     *pro;
{
   if (pro->dictname.TJ_sys_dict)
      free(pro->dictname.TJ_sys_dict);
   if (pro->dictname.PH_sys_dict)
      free(pro->dictname.PH_sys_dict);
   if (pro->dictname.TJ_usr_dict)
      free(pro->dictname.TJ_usr_dict);
   if (pro->dictname.PH_usr_dict)
      free(pro->dictname.PH_usr_dict);
   if (pro->learnfile)                            /* @big5 */
      free(pro->learnfile);                       /* @big5 */
}

