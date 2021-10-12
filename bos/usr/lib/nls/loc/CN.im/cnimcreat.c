static char sccsid[] = "@(#)22	1.2  src/bos/usr/lib/nls/loc/CN.im/cnimcreat.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:28";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMCreate
 *		CNIMfreedictname
 *		CNIMmakeprofile
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
/* MODULE NAME:        CNIMCreate                                      */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Object Creation            */
/*                                                                     */
/* FUNCTION:           CNIMCreate   :  Create Object                   */
/*                                                                     */
/* CNIMmakeprofile  : Make CNIM Profile                                */
/*                                                                     */
/* CNIMfreedictname : Free Dict. Name                                  */
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
#include "cnedud.h"
#include <sys/ldr.h>

/* ---------------------------------------------------------------------- */
/* External function reference                                            */
/* ---------------------------------------------------------------------- */
extern int      indlinemake();
extern int      querystate();

/*-----------------------------------------------------------------------*
*       Beginning of procedure
*-----------------------------------------------------------------------*/
CNIMOBJ        CNIMCreate(imfep, imcallback, udata)
   CNIMFEP         imfep;
   IMCallback     *imcallback;
   caddr_t         udata;
{
   extern InputMode CNedGetInputMode();

   /*******************/
   /* local variables */
   /*******************/
   int             CNIMmakeprofile();
   void            CNIMfreedictname();
   CNIMOBJ         obj;
   CNimProfile     cnimpro;
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
   obj = (CNIMOBJ) malloc(sizeof(CNIMobj));

   /***************************************/
   /* allocate additional data structures */
   /***************************************/

   textmaxlen = imcallback->textmaxwidth;

   /* string/attribute buffers for Text Info. */
   obj->textinfo.text.str = (unsigned char *) malloc(textmaxlen);
   obj->textinfo.text.att = (unsigned char *) malloc(textmaxlen);

   obj->auxinfo.message.text =                /* buffers for Aux Information */
                                 (IMSTRATT *) malloc(sizeof(IMSTRATT) * CNIM_AUXROWMAX);

   for (i = 0; i < CNIM_AUXROWMAX; i++)       /* string and attribute buffer */
   {
      obj->auxinfo.message.text[i].str = (unsigned char *)
                                    malloc(CNIM_AUXCOLMAX);
      obj->auxinfo.message.text[i].att = (unsigned char *)
                                    malloc(CNIM_AUXCOLMAX);
   }

   obj->string.str = (unsigned char *) malloc(textmaxlen);
                                              /* string buffer for GetString */


   obj->indstr.str = (unsigned char *) malloc(CNIM_INDSTRMAXLEN);  
                                              /* indicator string buffer */

/* when the convert key is pressed, the final EUC will place into this buffer */

   obj->convert_str = (caddr_t) malloc(textmaxlen);

   obj->output.data = (caddr_t) malloc(CNIM_INDSTRMAXLEN);
   obj->output.len = 0;
   obj->output.siz = CNIM_INDSTRMAXLEN;

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

   for (i = 0; i < CNIM_AUXROWMAX; i++)           /* buffer for aux  */
   {
      obj->auxinfo.message.text[i].len = 0;
      memset(obj->auxinfo.message.text[i].str, NULL, CNIM_AUXCOLMAX);
      memset(obj->auxinfo.message.text[i].att, NULL, CNIM_AUXCOLMAX);
   }

   memset(obj->string.str, NULL, textmaxlen);
                                        /* string buffer for GetString ioctl */

   memset(obj->indstr.str, NULL, CNIM_INDSTRMAXLEN);
                                        /* indicator string buffer */

   /* The final EUC result after pressing press convert key   */

   memset(obj->convert_str, NULL, textmaxlen);

   /***************************************************************/
   /* allocate buffers between CNIMED                             */
   /* since CNIMED generates string,                              */
   /***************************************************************/
   /* echo buffer, fix buffers */
   obj->cnedinfo.echobufs = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->cnedinfo.echobufa = (unsigned char *) malloc(imcallback->textmaxwidth);
   obj->cnedinfo.fixbuf = (unsigned char *) malloc(imcallback->textmaxwidth);

   /* aux string buffer */
   obj->cnedinfo.auxbufs = (char **) malloc(sizeof(caddr_t) * CNIM_AUXROWMAX);
   aux_str = obj->cnedinfo.auxbufs;
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      *(aux_str)++ = (unsigned char *) malloc(CNIM_AUXCOLMAX);

   /* aux attribute buffer */
   obj->cnedinfo.auxbufa = (char **) malloc(sizeof(caddr_t) * CNIM_AUXROWMAX);
   aux_atr = obj->cnedinfo.auxbufa;
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      *(aux_atr)++ = (unsigned char *) malloc(CNIM_AUXCOLMAX);

   /*****************************************/
   /* initialize information between CNIMED */
   /*****************************************/
   /* echo buffer and fix buffer initial */
   memset(obj->cnedinfo.echobufs, NULL, imcallback->textmaxwidth);
   memset(obj->cnedinfo.echobufa, NULL, imcallback->textmaxwidth);
   memset(obj->cnedinfo.fixbuf, NULL, imcallback->textmaxwidth);
   obj->cnedinfo.echobufsize = imcallback->textmaxwidth;

   aux_str = obj->cnedinfo.auxbufs;               /* aux string buffer */
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      memset(*(aux_str)++, NULL, CNIM_AUXCOLMAX);

   aux_atr = obj->cnedinfo.auxbufa;               /* aux attribute buffer */
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      memset(*(aux_atr)++, NULL, CNIM_AUXCOLMAX);
   obj->cnedinfo.auxsize.itemnum = CNIM_AUXROWMAX;
   obj->cnedinfo.auxsize.itemsize = CNIM_AUXCOLMAX;

   /**********************************************************/
   /* profiling                                              */
   /* retrieve information from profile, making CNIM profile */
   /**********************************************************/
   if (CNIMmakeprofile(&cnimpro) == TRUE)
   {
      /************************/
      /* CNIMED initialized   */
      /************************/
      auxformat = LONGAUX;
      /* make profile for CNIMED from profile structure */
      
      /* call CNedInit to initial fepcb    */
      ret = CNedInit(obj->cnedinfo.echobufs,
                    obj->cnedinfo.echobufa,
                    obj->cnedinfo.fixbuf,
                    obj->cnedinfo.echobufsize,
                    obj->cnedinfo.auxbufs,
                    obj->cnedinfo.auxbufa,
                    &(obj->cnedinfo.auxsize),
                    auxformat,
                    &cnimpro);
      if (ret == ERROR)
         imfep->common.imerrno = CNIMEDError;

   } else
   {
      ret = ERROR;
      imfep->common.imerrno = CNIMDictError;
   }

   /***************************************/
   /* free allocated for dictionary names */
   /***************************************/
   CNIMfreedictname(&cnimpro);

   /***********************************/
   /* CNIMED successfully initialized */
   /***********************************/
   if (ret != ERROR)
   {
      obj->cnedinfo.CNedID = ret;

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
   /* 1. result of CNedInit                                       */
   /* 2. result of CNIMmakeprofile with dictionary access error    */
   /* 3. result of indicatordraw callback                         */
   /***************************************************************/
   if (ret == ERROR)
   {                                             /* error returned */
      /***************************************/
      /* free all resources allocated so far */
      /***************************************/
      free(obj->cnedinfo.echobufs);
      free(obj->cnedinfo.echobufa);
      free(obj->cnedinfo.fixbuf);

      aux_str = obj->cnedinfo.auxbufs;            /* aux string buffers */
      for (i = 0; i < CNIM_AUXROWMAX; i++)
         free(*aux_str++);
      free(obj->cnedinfo.auxbufs);

      aux_atr = obj->cnedinfo.auxbufa;            /* aux attribute buffers  */
      for (i = 0; i < CNIM_AUXROWMAX; i++)
         free(*aux_atr++);
      free(obj->cnedinfo.auxbufa);

      free(obj->textinfo.text.str);              /* string buffers for Text Info. */
      free(obj->textinfo.text.att);

      for (i = 0; i < CNIM_AUXROWMAX; i++)        /* buffers for Aux Info. */
      {
         free(obj->auxinfo.message.text[i].str);
         free(obj->auxinfo.message.text[i].att);
      }
      free(obj->auxinfo.message.text);

      free(obj->string.str);                     /* string buffer for GetString ioctl */

      free(obj->indstr.str);                     /* indicator string buffer */

      free(obj);                                 /* CNIM object structure */
      obj = NULL;
   }
   return (obj);
}                                                /* end of create */

/*-----------------------------------------------------------------------*
*       Beginning of CNIMmakeprofile
*           retrieve profile information from CNIM profile file
*           or set to defaults
*       return FLASE if dictionary is not found, TRUE otherwise
*-----------------------------------------------------------------------*/
static int      CNIMmakeprofile(pro)
   CNimProfile     *pro;
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
   pro->dictname.Py_sys_dict = NULL;
   pro->dictname.Le_sys_dict = NULL;
   pro->dictname.Fss_sys_dict = NULL;
   pro->dictname.Fssjm_sys_dict = NULL;
   pro->dictname.Fs_sys_dict = NULL;
   pro->dictname.Fsph_sys_dict = NULL;
   pro->dictname.Abc_sys_dict_cwd = NULL;
   pro->dictname.Abc_sys_dict_ovl = NULL;
   pro->dictname.En_usr_dict = NULL;
   pro->dictname.Py_usr_dict = NULL;
   pro->dictname.Le_usr_dict = NULL;
   pro->dictname.Fss_usr_dict = NULL;
   pro->dictname.Fs_usr_dict = NULL;
   pro->dictname.Fsph_usr_dict = NULL;
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
            if (!strcmp(temp, CNIM_INIT_AN))
               pro->initchar = ALPH_NUM_MODE;
            else
            if (!strcmp(temp, CNIM_INIT_EN))
               pro->initchar = ENGLISH_CHINESE_MODE;
            else
            if (!strcmp(temp, CNIM_INIT_PY))
               pro->initchar = PINYIN_MODE;
            else
            if (!strcmp(temp, CNIM_INIT_ABC))
               pro->initchar = ABC_MODE;
            else                                 /* default value */
            if (!strcmp(temp, CNIM_INIT_UD))
               pro->initchar = USER_DEFINED_MODE;
            else
            if (!strcmp(temp, CNIM_INIT_FSS))
               pro->initchar = FIVESTROKE_STYLE_MODE;
            else
            if (!strcmp(temp, CNIM_INIT_FS))
               pro->initchar = FIVESTROKE_MODE;
            else
               pro->initchar = ALPH_NUM_MODE;
         }
         temp = low(getoptvalue(INITSIZE));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, CNIM_INIT_HALF))
               pro->initsize = HALF;
            else
               pro->initsize = FULL;
         }
         temp = low(getoptvalue(INITBEEP));
         if (temp)
         {                                       /* specified option found */
            if (!strcmp(temp, CNIM_INIT_BEEP_ON))
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
   pro->dictname.Py_sys_dict = FindSysUserDict(PY_SYS_DICT, READ_ONLY);
   pro->dictname.Le_sys_dict = FindSysUserDict(LE_SYS_DICT, READ_ONLY);
   pro->dictname.Fss_sys_dict = FindSysUserDict(FSS_SYS_DICT, READ_ONLY);
   pro->dictname.Fssjm_sys_dict = FindSysUserDict(FSSJM_SYS_DICT, READ_ONLY);
   pro->dictname.Fs_sys_dict = FindSysUserDict(FS_SYS_DICT, READ_ONLY);
   pro->dictname.Fsph_sys_dict = FindSysUserDict(FSPH_SYS_DICT, READ_ONLY);
   pro->dictname.Abc_sys_dict_cwd = FindSysUserDict(ABC_SYS_DICT_CWD, READ_ONLY);
   pro->dictname.Abc_sys_dict_ovl = FindSysUserDict(ABC_SYS_DICT_OVL, READ_ONLY);
   pro->dictname.En_usr_dict = FindSysUserDict(EN_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.Py_usr_dict = FindSysUserDict(PY_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Le_usr_dict = FindSysUserDict(LE_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Fss_usr_dict = FindSysUserDict(FSS_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Fs_usr_dict = FindSysUserDict(FS_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Fsph_usr_dict = FindSysUserDict(FSPH_USER_DICT,
                                              READ_ONLY | WRITE_ONLY);
   pro->dictname.Abc_usr_rem = FindSysUserDict(ABC_USER_REM,
                                               READ_ONLY | WRITE_ONLY);
   pro->dictname.Abc_usr_dict = FindSysUserDict(ABC_USER_DICT,
                                               READ_ONLY | WRITE_ONLY);

   /******************************************************************/
   /*  User Defined Input Method module don't provide temporarily    */
   /******************************************************************/
   pro->ud_im_name = UdFindIM();
   if ((pro->ud_im_func = load(pro->ud_im_name, 1, "")) <= 0 )
      pro->ud_im_func = NULL; 

   if ( pro->dictname.Py_sys_dict && pro->dictname.En_sys_dict&&pro->dictname.Fss_sys_dict && pro->dictname.Fssjm_sys_dict && pro->dictname.Fs_sys_dict && pro->dictname.Abc_sys_dict_cwd 
&& pro->dictname.Abc_sys_dict_ovl )
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
*       beginning of CNIMfreedictname
*          free memory to hold dictionary names which is
*          allocated by dictionary find functions
*-----------------------------------------------------------------------*/
static void     CNIMfreedictname(pro)
   CNimProfile     *pro;
{
   if (pro->dictname.En_sys_dict)
      free(pro->dictname.En_sys_dict);
   if (pro->dictname.Py_sys_dict)
      free(pro->dictname.Py_sys_dict);
   if (pro->dictname.Le_sys_dict)
      free(pro->dictname.Le_sys_dict);
   if (pro->dictname.Fss_sys_dict)
      free(pro->dictname.Fss_sys_dict);
   if (pro->dictname.Fssjm_sys_dict)
      free(pro->dictname.Fssjm_sys_dict);
   if (pro->dictname.Fs_sys_dict)
      free(pro->dictname.Fs_sys_dict);
   if (pro->dictname.Fsph_sys_dict)
      free(pro->dictname.Fsph_sys_dict);
   if (pro->dictname.Abc_sys_dict_cwd)
      free(pro->dictname.Abc_sys_dict_cwd);
   if (pro->dictname.Abc_sys_dict_ovl)
      free(pro->dictname.Abc_sys_dict_ovl);
   if (pro->dictname.En_usr_dict)
      free(pro->dictname.En_usr_dict);
   if (pro->dictname.Py_usr_dict)
      free(pro->dictname.Py_usr_dict);
   if (pro->dictname.Le_usr_dict)
      free(pro->dictname.Le_usr_dict);
   if (pro->dictname.Fss_usr_dict)
      free(pro->dictname.Fss_usr_dict);
   if (pro->dictname.Fs_usr_dict)
      free(pro->dictname.Fs_usr_dict);
   if (pro->dictname.Fsph_usr_dict)
      free(pro->dictname.Fsph_usr_dict);
   if (pro->dictname.Abc_usr_dict)
      free(pro->dictname.Abc_usr_dict);
   if (pro->dictname.Ud_dict)
      free(pro->dictname.Ud_dict);
}
