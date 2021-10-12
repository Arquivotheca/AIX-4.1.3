static char sccsid[] = "@(#)67	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhimcreat.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:32:57";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMCreate
 *		ZHIMfreedictname
 *		ZHIMmakeprofile
 *		low
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION *******************/
/*                                                                     */
/* MODULE NAME:        ZHIMCreate                                      */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Object Creation            */
/*                                                                     */
/* FUNCTION:           ZHIMCreate   :  Create Object                   */
/*                                                                     */
/* ZHIMmakeprofile  : Make ZHIM Profile                                */
/*                                                                     */
/* ZHIMfreedictname : Free Dict. Name                                  */
/*                                                                     */
/* low  :  Convert Lower                                               */
/*                                                                     */
/********************* END OF SPECIFICATIONS ****************************/

/*-----------------------------------------------------------------------*
*       Include files
*-----------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "imlang.h"
#include "chinese.h"
#include <sys/ldr.h>


/* ---------------------------------------------------------------------- */
/* External function reference                                            */
/* ---------------------------------------------------------------------- */
extern int      indlinemake();
extern int      querystate();

/*-----------------------------------------------------------------------*
*       Beginning of procedure
*-----------------------------------------------------------------------*/
ZHIMOBJ        ZHIMCreate(imfep, imcallback, udata)
   ZHIMFEP         imfep;
   IMCallback     *imcallback;
   caddr_t         udata;
{
   extern InputMode ZHedGetInputMode();

   /*******************/
   /* local variables */
   /*******************/
   int             ZHIMmakeprofile();
   void            ZHIMfreedictname();
   ZHIMOBJ         obj;
   ZHimProfile     zhimpro;
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
   /* allocate Chinese input method object          */
   /*************************************************/
   obj = (ZHIMOBJ) malloc(sizeof(ZHIMobj));

   /***************************************/
   /* allocate additional data structures */
   /***************************************/

   textmaxlen = imcallback->textmaxwidth;

   /* string/attribute buffers for Text Info. */
   obj->textinfo.text.str = (unsigned char *) malloc(textmaxlen);
   obj->textinfo.text.att = (unsigned char *) malloc(textmaxlen);

   obj->auxinfo.message.text =                /* buffers for Aux Information */
                                 (IMSTRATT *) malloc(sizeof(IMSTRATT) * ZHIM_AUXROWMAX);

   for (i = 0; i < ZHIM_AUXROWMAX; i++)       /* string and attribute buffer */
   {
      obj->auxinfo.message.text[i].str = (unsigned char *)
                                    malloc(ZHIM_AUXCOLMAX);
      obj->auxinfo.message.text[i].att = (unsigned char *)
                                    malloc(ZHIM_AUXCOLMAX);
   }

   obj->string.str = (unsigned char *) malloc(textmaxlen);
                                              /* string buffer for GetString */


   obj->indstr.str = (unsigned char *) malloc(ZHIM_INDSTRMAXLEN);  
                                              /* indicator string buffer */

/* when the convert key is pressed, the final UTF will place into this buffer */

   obj->convert_str = (caddr_t) malloc(textmaxlen);

   obj->output.data = (caddr_t) malloc(ZHIM_INDSTRMAXLEN);
   obj->output.len = 0;
   obj->output.siz = ZHIM_INDSTRMAXLEN;

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
   obj->auxinfo.selection.panel = NULL;
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

   for (i = 0; i < ZHIM_AUXROWMAX; i++)           /* buffer for aux  */
   {
      obj->auxinfo.message.text[i].len = 0;
      memset(obj->auxinfo.message.text[i].str, NULL, ZHIM_AUXCOLMAX);
      memset(obj->auxinfo.message.text[i].att, NULL, ZHIM_AUXCOLMAX);
   }

   memset(obj->string.str, NULL, textmaxlen);
                                        /* string buffer for GetString ioctl */

   memset(obj->indstr.str, NULL, ZHIM_INDSTRMAXLEN);
                                        /* indicator string buffer */

   /* The final UTF result after pressing press convert key   */

   memset(obj->convert_str, NULL, textmaxlen);

   /***************************************************************/
   /* allocate buffers between ZHIMED                             */
   /* since ZHIMED generates string,                              */
   /***************************************************************/
   /* echo buffer, fix buffers */
   obj->zhedinfo.echobufs = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->zhedinfo.echobufa = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->zhedinfo.fixbuf = (unsigned char *) malloc(imcallback->textmaxwidth);

   /* aux string buffer */
   obj->zhedinfo.auxbufs = (char **) malloc(sizeof(caddr_t) * ZHIM_AUXROWMAX);
   aux_str = obj->zhedinfo.auxbufs;
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      *(aux_str)++ = (unsigned char *) malloc(ZHIM_AUXCOLMAX);

   /* aux attribute buffer */
   obj->zhedinfo.auxbufa = (char **) malloc(sizeof(caddr_t) * ZHIM_AUXROWMAX);
   aux_atr = obj->zhedinfo.auxbufa;
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      *(aux_atr)++ = (unsigned char *) malloc(ZHIM_AUXCOLMAX);

   /*****************************************/
   /* initialize information between ZHIMED */
   /*****************************************/
   /* echo buffer and fix buffer initial */
   memset(obj->zhedinfo.echobufs, NULL, imcallback->textmaxwidth);
   memset(obj->zhedinfo.echobufa, NULL, imcallback->textmaxwidth);
   memset(obj->zhedinfo.fixbuf, NULL, imcallback->textmaxwidth);
   obj->zhedinfo.echobufsize = imcallback->textmaxwidth;

   aux_str = obj->zhedinfo.auxbufs;               /* aux string buffer */
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      memset(*(aux_str)++, NULL, ZHIM_AUXCOLMAX);

   aux_atr = obj->zhedinfo.auxbufa;               /* aux attribute buffer */
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      memset(*(aux_atr)++, NULL, ZHIM_AUXCOLMAX);
   obj->zhedinfo.auxsize.itemnum = ZHIM_AUXROWMAX;
   obj->zhedinfo.auxsize.itemsize = ZHIM_AUXCOLMAX;

   /**********************************************************/
   /* profiling                                              */
   /* retrieve information from profile, making ZHIM profile */
   /**********************************************************/
   if (ZHIMmakeprofile(&zhimpro) == TRUE)
   {
      /************************/
      /* ZHIMED initialized   */
      /************************/
      auxformat = LONGAUX;
      /* make profile for ZHIMED from profile structure */
      
      /* call ZHedInit to initial fepcb    */
      ret = ZHedInit(obj->zhedinfo.echobufs,
                    obj->zhedinfo.echobufa,
                    obj->zhedinfo.fixbuf,
                    obj->zhedinfo.echobufsize,
                    obj->zhedinfo.auxbufs,
                    obj->zhedinfo.auxbufa,
                    &(obj->zhedinfo.auxsize),
                    auxformat,
                    &zhimpro);
      if (ret == ERROR)
         imfep->common.imerrno = ZHIMEDError;

   } else
   {
      ret = ERROR;
      imfep->common.imerrno = ZHIMDictError;
   }

   /***************************************/
   /* free allocated for dictionary names */
   /***************************************/
   ZHIMfreedictname(&zhimpro);

   /***********************************/
   /* ZHIMED successfully initialized */
   /***********************************/
   if (ret != ERROR)
   {
      obj->zhedinfo.ZHedID = ret;

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
   /* 1. result of ZHedInit                                       */
   /* 2. result of ZHIMmakeprofile with dictionary access error    */
   /* 3. result of indicatordraw callback                         */
   /***************************************************************/
   if (ret == ERROR)
   {                                             /* error returned */
      /***************************************/
      /* free all resources allocated so far */
      /***************************************/
      free(obj->zhedinfo.echobufs);
      free(obj->zhedinfo.echobufa);
      free(obj->zhedinfo.fixbuf);

      aux_str = obj->zhedinfo.auxbufs;            /* aux string buffers */
      for (i = 0; i < ZHIM_AUXROWMAX; i++)
         free(*aux_str++);
      free(obj->zhedinfo.auxbufs);

      aux_atr = obj->zhedinfo.auxbufa;            /* aux attribute buffers  */
      for (i = 0; i < ZHIM_AUXROWMAX; i++)
         free(*aux_atr++);
      free(obj->zhedinfo.auxbufa);

      free(obj->textinfo.text.str);              /* string buffers for Text Info. */
      free(obj->textinfo.text.att);

      for (i = 0; i < ZHIM_AUXROWMAX; i++)        /* buffers for Aux Info. */
      {
         free(obj->auxinfo.message.text[i].str);
         free(obj->auxinfo.message.text[i].att);
      }
      free(obj->auxinfo.message.text);

      free(obj->string.str);                     /* string buffer for GetString ioctl */

      free(obj->indstr.str);                     /* indicator string buffer */

      free(obj);                                 /* ZHIM object structure */
      obj = NULL;
   }
   return (obj);
}                                                /* end of create */

/*-----------------------------------------------------------------------*
*       Beginning of ZHIMmakeprofile
*           retrieve profile information from ZHIM profile file
*           or set to defaults
*       return FLASE if dictionary is not found, TRUE otherwise
*-----------------------------------------------------------------------*/
static int      ZHIMmakeprofile(pro)
   ZHimProfile     *pro;
{
   extern int      openprofile();
   extern char    *getoptvalue();
   extern void     closeprofile();
   extern char    *searchprofile();
   extern char    *FindSysUserDict();
   extern char    *UdFindIM();
   extern char    *low();
   char           *temp;
   char           *proname;

   /*******************************/
   /* set every field to defaults */
   /*******************************/
   pro->initchar = ALPH_NUM_MODE;
   pro->initsize = HALF;
   pro->initbeep = BEEP_ON;
   pro->dictname.En_sys_dict = NULL;
   pro->dictname.Py_sys_dict_comm = NULL;
   pro->dictname.Py_sys_dict_gb = NULL;
   pro->dictname.Py_sys_dict_cns = NULL;
   pro->dictname.Py_sys_dict_jk = NULL;
   pro->dictname.Tj_sys_dict = NULL;
   pro->dictname.Le_sys_dict = NULL;
   pro->dictname.Abc_sys_dict_cwd_s = NULL;
   pro->dictname.Abc_sys_dict_cwd_t = NULL;
   pro->dictname.Abc_sys_dict_ovl = NULL;
   pro->dictname.En_usr_dict = NULL;
   pro->dictname.Py_usr_dict = NULL;
   pro->dictname.Tj_usr_dict = NULL;
   pro->dictname.Le_usr_dict = NULL;
   pro->dictname.Abc_usr_dict = NULL;
   pro->dictname.Ud_dict = NULL;

   /*************************************************/
   /* first, search profile file in following order */
   /*************************************************/
   proname = searchprofile(READ_ONLY);

   /***************************************************/
   /* if file exists, retrieve information from it,   */
   /* updating information in structure               */
   /***************************************************/
   if (proname)
   {
      if (openprofile(proname))
      {
         temp = low(getoptvalue(INITCHAR));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, ZHIM_INIT_AN))
               pro->initchar = ALPH_NUM_MODE;
            else
            if (!strcmp(temp, ZHIM_INIT_EN))
               pro->initchar = ENGLISH_CHINESE_MODE;
            else
            if (!strcmp(temp, ZHIM_INIT_PY))
               pro->initchar = PINYIN_MODE;
            else
            if (!strcmp(temp, ZHIM_INIT_TJ))
               pro->initchar = TSANG_JYE_MODE;
            else
            if (!strcmp(temp, ZHIM_INIT_ABC))
               pro->initchar = ABC_MODE;
            else                                 /* default value */
            if (!strcmp(temp, ZHIM_INIT_UD))
               pro->initchar = USER_DEFINED_MODE;
            else
               pro->initchar = ALPH_NUM_MODE;
         }
         temp = low(getoptvalue(INITSIZE));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, ZHIM_INIT_HALF))
               pro->initsize = HALF;
            else
               pro->initsize = FULL;
         }
         temp = low(getoptvalue(INITBEEP));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, ZHIM_INIT_BEEP_ON))
               pro->initbeep = BEEP_ON;
            else
               pro->initbeep = BEEP_OFF;
         }
         closeprofile();
      }
      free(proname);
   }                                             /* end of proname */
   /******************************************************/
   /* when system file is not found, inform caller error */
   /******************************************************/
   pro->dictname.En_sys_dict = FindSysUserDict(EN_SYS_DICT, READ_ONLY);
   pro->dictname.Py_sys_dict_comm = FindSysUserDict(PY_SYS_DICT_COMM, READ_ONLY);
   pro->dictname.Py_sys_dict_gb = FindSysUserDict(PY_SYS_DICT_GB, READ_ONLY);
   pro->dictname.Py_sys_dict_cns = FindSysUserDict(PY_SYS_DICT_CNS, READ_ONLY);
   pro->dictname.Py_sys_dict_jk = FindSysUserDict(PY_SYS_DICT_JK, READ_ONLY);
   pro->dictname.Tj_sys_dict = FindSysUserDict(TJ_SYS_DICT, READ_ONLY);
   pro->dictname.Le_sys_dict = FindSysUserDict(LE_SYS_DICT, READ_ONLY);
   pro->dictname.Abc_sys_dict_cwd_s = FindSysUserDict(ABC_SYS_DICT_CWD_S, READ_ONLY);
   pro->dictname.Abc_sys_dict_cwd_t = FindSysUserDict(ABC_SYS_DICT_CWD_T, READ_ONLY);
   pro->dictname.Abc_sys_dict_ovl = FindSysUserDict(ABC_SYS_DICT_OVL, READ_ONLY);
   pro->dictname.En_usr_dict = FindSysUserDict(EN_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.Py_usr_dict = FindSysUserDict(PY_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Tj_usr_dict = FindSysUserDict(TJ_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Le_usr_dict = FindSysUserDict(LE_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Abc_usr_rem = FindSysUserDict(ABC_USER_REM,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.Abc_usr_dict = FindSysUserDict(ABC_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.Ud_dict = FindSysUserDict(UD_DICT, READ_ONLY);

   /******************************************************************/
   /*  User Defined Input Method module don't provide temporarily    */
   /******************************************************************/
   pro->ud_im_name = UdFindIM();
   if ((pro->ud_im_func = load(pro->ud_im_name, 1, "")) <= 0 )
      pro->ud_im_func = NULL; 
   if ( pro->dictname.Py_sys_dict_comm && pro->dictname.Py_sys_dict_gb &&
        pro->dictname.Py_sys_dict_cns && pro->dictname.Py_sys_dict_jk && 
        pro->dictname.En_sys_dict && pro->dictname.Tj_sys_dict && 
        pro->dictname.Abc_sys_dict_cwd_s && pro->dictname.Abc_sys_dict_cwd_t &&
        pro->dictname.Abc_sys_dict_ovl)
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
*       beginning of ZHIMfreedictname
*          free memory to hold dictionary names which is
*          allocated by dictionary find functions
*-----------------------------------------------------------------------*/
static void     ZHIMfreedictname(pro)
   ZHimProfile     *pro;
{
   if (pro->dictname.En_sys_dict)
      free(pro->dictname.En_sys_dict);
   if (pro->dictname.Py_sys_dict_comm)
      free(pro->dictname.Py_sys_dict_comm);
   if (pro->dictname.Py_sys_dict_gb)
      free(pro->dictname.Py_sys_dict_gb);
   if (pro->dictname.Py_sys_dict_cns)
      free(pro->dictname.Py_sys_dict_cns);
   if (pro->dictname.Py_sys_dict_jk)
      free(pro->dictname.Py_sys_dict_jk);
   if (pro->dictname.Tj_sys_dict)
      free(pro->dictname.Tj_sys_dict);
   if (pro->dictname.Le_sys_dict)
      free(pro->dictname.Le_sys_dict);
   if (pro->dictname.Abc_sys_dict_cwd_s)
      free(pro->dictname.Abc_sys_dict_cwd_s);
   if (pro->dictname.Abc_sys_dict_cwd_t)
      free(pro->dictname.Abc_sys_dict_cwd_t);
   if (pro->dictname.Abc_sys_dict_ovl)
      free(pro->dictname.Abc_sys_dict_ovl);
   if (pro->dictname.En_usr_dict)
      free(pro->dictname.En_usr_dict);
   if (pro->dictname.Py_usr_dict)
      free(pro->dictname.Py_usr_dict);
   if (pro->dictname.Tj_usr_dict)
      free(pro->dictname.Tj_usr_dict);
   if (pro->dictname.Le_usr_dict)
      free(pro->dictname.Le_usr_dict);
   if (pro->dictname.Abc_usr_dict)
      free(pro->dictname.Abc_usr_dict);
   if (pro->dictname.Ud_dict)
      free(pro->dictname.Ud_dict);
}
