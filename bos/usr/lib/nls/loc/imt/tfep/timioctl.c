static char sccsid[] = "@(#)22  1.4  src/bos/usr/lib/nls/loc/imt/tfep/timioctl.c, libtw, bos411, 9428A410j 4/21/94 02:29:42";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: LongForm
 *              ShortForm
 *              TIMIoctl
 *              p00
 *              p01
 *              p02
 *              p03
 *              p04
 *              p05
 *              p06
 *              p07
 *              p08
 *              p09
 *              p10
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
/************************************************************************
*                                                                       *
*       Module Name      : TIMIoctl                                     *
*                                                                       *
*       Description Name : Chinese Input Method Ioctl                   *
*                                                                       *
*       Function         : TIMIoctl -> control IOCTL sub-call           *
*                                                                       *
*       Module Type      : C                                            *
*                                                                       *
*       Compiler         : AIX C                                        *
*                                                                       *
*       Author           : Jim Roung                                    *
*                                                                       *
*       Status           : Chinese Input Method Version 1.0             *
*                                                                       *
*       Change Activity  :                                              *
*                                                                       *
************************************************************************/

/* ---------------  Include Files  --------------------------------- */
#include "taiwan.h"
#include  "msgextrn.h"   /* @big5 */

/* ***************  External reference Functions   ******************* */

extern InputMode TedGetInputMode();
extern int      indicatordraw();
extern int      auxdraw();
extern int      textdraw();
extern void     getstringmake();
extern void     textinfomake();
extern void     auxinfomake();
extern int      querystate();
extern char    *TedGetIndMessage();
extern int      indicatorhide();
extern int      indlinemake();

/* ------------------   The define constant  ---------------------------- */

#define Refresh                 0
#define GetString               1
#define Clear                   2
#define Reset                   3
#define ChangeLength            4
#define QueryState              5
#define QueryText               6
#define QueryAuxiliary          7
#define QueryIndicator          8
#define QueryIndicatorString    9
#define ChangeMode             10

#define RESET_AUX               0
#define CHANGE_LENGTH           RESET_AUX+1
#define IMInvalidArgument       0

/* -----------------  Global Variables  --------------------------- */

TIMOBJ          Obj;
char           *Arg;

/*--------------  The declaration of state table  -------------------*/

int             p00();
int             p01();
int             p02();
int             p03();
int             p04();
int             p05();
int             p06();
int             p07();
int             p08();
int             p09();
int             p10();
int             LongForm();
int             ShortForm();

/* =========>>> Set up the event table <<<=============== */

int             (*ioctl_action[11]) () =
{
   /* Refresh GetStr Clear Reset ChgLen Q-State Q-Text Q-Aux Q-Ind Q-Ind_Str ChgMode */
        p00,    p01,  p02,  p03,   p04,   p05,   p06,   p07,  p08,   p09,      p10
};

/* -------------------------------------------------------------------- */
/*********************  Ioctl main function   **************************/
/* -------------------------------------------------------------------- */

/***********************************************************************
*                                                                      *
* Function : TIMIoctl()                                                *
* Description : Porcess the IM_Ioctl sub-calls.                        *
* External Reference :                                                 *
* Input  :                                                             *
*         (1). imobj -> Input-Method Object data structure (TIMOBJ)    *
*         (2). operation -> What's kind of Ioctl operation.            *
*         (3). arg -> optional parameter, depend on what's kind of     *
*                     Ioctl operation is being used.                   *
* Output :                                                             *
*         (1). IMNoError                                               *
*         (2). IMError                                                 *
*         (3). IMInvalidParameter                                      *
*                                                                      *
************************************************************************/

int             TIMIoctl(imobj, operation, arg)
   TIMOBJ          imobj;
   int             operation;
   char           *arg;
{
   int             state;
   int             ret_code;

   switch (operation)                            /* dispatch what's kind Ioctl calls are issued */
   {
   case IM_Refresh:
      state = Refresh;
      break;
   case IM_GetString:
      state = GetString;
      break;
   case IM_Clear:
      state = Clear;
      break;
   case IM_Reset:
      state = Reset;
      break;
   case IM_ChangeLength:
      state = ChangeLength;
      break;
   case IM_QueryState:
      state = QueryState;
      break;
   case IM_QueryText:
      state = QueryText;
      break;
   case IM_QueryAuxiliary:
      state = QueryAuxiliary;
      break;
   case IM_QueryIndicator:
      state = QueryIndicator;
      break;
   case IM_QueryIndicatorString:
      state = QueryIndicatorString;
      break;
   case IM_ChangeMode:
      state = ChangeMode;
      break;
   default:
      return IMError;
      break;
   }                                             /* switch */

   Obj = imobj;
   Arg = arg;
   ret_code = (*ioctl_action[state]) ();

   if (ret_code != IMNoError)
      imobj->imobject.imfep->imerrno = ret_code;

   return (ret_code);

}                                                /* TIMIoctl() */

/*----------------------------------------------------------------------*/
/* ------------------  process IM_Refresh  ----------------------------- */
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p00()                                                      *
* Description : Porcess the IM_Refresh :                                *
*               - Redraw the indicator line.                            *
*               - Redraw the text area.                                 *
*               - Redraw the Auxiliary area.                            *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p00()
{
   int             retcode;

   if (Obj->querystate.mode == IMNormalMode)
      retcode = indicatordraw(Obj);              /* redraw the indicator */
   if (TedGetEchoBufLen(Obj->tedinfo.TedID) > 0)
      retcode = textdraw(Obj);                   /* redraw text area     */
   if (TedIsAuxBufUsedNow(Obj->tedinfo.TedID))
      retcode = auxdraw(Obj);                    /* redraw aux area      */

   return (retcode);

}                                                /* p00() */

/*----------------------------------------------------------------------*/
/*-------------------  process IM_GetString  ---------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p01()                                                      *
* Description : Porcess the IM_GetString                                *
*               - To get the current pre-editing string.                *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/


int             p01()
{
   int             retcode;


   Obj->textstate = (TedGetEchoBufLen(Obj->tedinfo.TedID) == 0) ?
                                 TEXT_OFF : TEXT_ON;
   getstringmake(Obj);
   *(IMSTR *) Arg = Obj->string;

   if (Obj->textstate == TEXT_ON)                /* clear text area      */
   {
      textinfomake(Obj);
      retcode = texthide(Obj);
   }                                             /* if */
   if (Obj->auxstate == USE)                     /* clear Aux area       */
   {
      auxinfomake(Obj);
      retcode = auxhide(Obj);
      if ((retcode = auxdestroy(Obj)) == IMNoError)
         Obj->auxIDflag = FALSE;
   }                                             /* if */
   return (retcode);

}                                                /* p01() */

/*----------------------------------------------------------------------*/
/*-------------------  process IM_Clear  -------------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p02()                                                      *
* Description : Porcess the IM_Clear                                    *
*               - To clear the text area if it exist.                   *
*               - To clear the Aux area if it exist.
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p02()
{
   int             retcode;

   Obj->textstate = (TedGetEchoBufLen(Obj->tedinfo.TedID) == 0) ?
                                 TEXT_OFF : TEXT_ON;
   TedClear(Obj->tedinfo.TedID);
   textinfomake(Obj); auxinfomake(Obj);
   if (Arg != NULL)
   {
      if (Obj->textstate == TEXT_ON)             /* Clear Text Area */
         retcode = texthide(Obj);
      if (Obj->auxstate == USE)                  /* Clear Aux Area  */
      {
         retcode = auxhide(Obj);
         if ((retcode = auxdestroy(Obj)) == IMNoError)
            Obj->auxIDflag = FALSE;
      }                                          /* if */
   }                                             /* if */
   Obj->auxstate = NOTUSE;
   /* Clear Indicator Area */
/*
   if (TedIsInputModeChanged(Obj->tedinfo.TedID))
   {
      indlinemake(Obj);
      querystate(Obj);
      if (TedGetInputMode(Obj->tedinfo.TedID).ind3 == NORMAL_MODE)
         retcode = indicatordraw(Obj);
   }*/                                             /* clear the error condition on indicator line */
   return (retcode);

}                                                /* p02() */

/*----------------------------------------------------------------------*/
/*-----------------  process IM_Reset  ---------------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p03()                                                      *
* Description : Porcess the IM_Reset                                    *
*               - To clear the Aux area if it exist.                    *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p03()
{
   int             retcode;

   TedControl(Obj->tedinfo.TedID, RESET_AUX, Arg);
   auxinfomake(Obj);
   if (Arg != NULL)                              /* process the Callback procedure       */
   {
      if (Obj->auxstate == USE)
      {
         retcode = auxhide(Obj);
         if ((retcode = auxdestroy(Obj)) == IMNoError)
            Obj->auxIDflag == FALSE;
      }                                          /* if */
   }                                             /* if */
   Obj->auxstate = NOTUSE;
   return (retcode);

}                                                /* p03 */

/*----------------------------------------------------------------------*/
/*---------------  process IM_ChangeLength  ----------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p04()                                                      *
* Description : Porcess the IM_ChangeLength                             *
*               - To chnage the max. length of text area.               *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p04()
{
   int             retcode;

   if ((int) Arg > Obj->imobject.cb->textmaxwidth)
   {
      Obj->imobject.imfep->imerrno = IMCallbackError;
      retcode = IMInvalidParameter;
   }
    /* if */
   else
   if ((int) Arg < TedGetEchoBufLen(Obj->tedinfo.TedID))
   {
      Obj->imobject.imfep->imerrno = IMCallbackError;
      retcode = IMInvalidParameter;
   }
    /* if */
   else
      TedControl(Obj->tedinfo.TedID, CHANGE_LENGTH, Arg);

   return (retcode);

}                                                /* p04 */

/*----------------------------------------------------------------------*/
/*-------------------  process IM_QueryState  --------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p05()                                                      *
* Description : Porcess the IM_QueryState                               *
*               - To check the status of AIX Input Method.              *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p05()
{

                   querystate(Obj);

   ((IMQueryState *) Arg)->mode = Obj->querystate.mode;
   ((IMQueryState *) Arg)->text = Obj->querystate.text;
   ((IMQueryState *) Arg)->aux = Obj->querystate.aux;
   ((IMQueryState *) Arg)->indicator = Obj->querystate.indicator;
   ((IMQueryState *) Arg)->beep = Obj->querystate.beep;

   return (IMNoError);

}                                                /* p05() */

/*----------------------------------------------------------------------*/
/*------------------  process IM_QueryText  ----------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p06()                                                      *
* Description : Porcess the IM_QueryText                                *
*               - To get the related information about text area        *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p06()
{
   IMQueryText    *temp_ptr;

   temp_ptr = (IMQueryText *) Arg;
   temp_ptr->textinfo = &(Obj->textinfo);
   return (IMNoError);

}                                                /* p06() */

/*----------------------------------------------------------------------*/
/*----------------  process  IM_QueryAuxiliary  ------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p07()                                                      *
* Description : Porcess the IM_QueryAuxiliary                           *
*               - To get the related information about Aux area, if     *
*                 it is showing now.                                    *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p07()
{

                   auxinfomake(Obj);
   if ((Obj->auxIDflag == TRUE) && (((IMQueryAuxiliary *) Arg)->auxid == Obj->auxID))
   {
      ((IMQueryAuxiliary *) Arg)->auxinfo = &(Obj->auxinfo);
      return (IMNoError);
   }
    /* if */
   else
   {
      Obj->imobject.imfep->imerrno = IMCallbackError;
      return (IMInvalidParameter);
   }                                             /* else */

}                                                /* p07() */

/*----------------------------------------------------------------------*/
/*----------------  process IM_QueryIndicator  -------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p08()                                                      *
* Description : Porcess the IM_QueryIndicator                           *
*               - To get the current indicator value if it exist.       *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p08()
{


   ((IMQueryIndicator *) Arg)->indicatorinfo = (Obj->querystate.mode == IMNormalMode) ?
                                               &(Obj->indinfo) : NULL;
   return (IMNoError);

}                                                /* p08() */

/*----------------------------------------------------------------------*/
/*---------------  process IM_QueryIndicatorString  --------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p09()                                                      *
* Description : Porcess the IM_QueryIndicatorString                     *
*               - To get the formatted and final indicator corresponding*
*                 to the current indicator value.                       *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

/* 5bChin2bHalf6bPhon28b*Msg */

int             p09()
{
   IMQueryIndicatorString *temp_arg;
   int             retcode;

   temp_arg = (IMQueryIndicatorString *) Arg;

   if (temp_arg->format == IMLongForm)
      retcode = LongForm();
   else
   if (temp_arg->format == IMShortForm)
      retcode = ShortForm();
   else
   {
      Obj->imobject.imfep->imerrno = IMCallbackError;
      retcode = IMInvalidParameter;
   }
   return retcode;

}                                                /* p09() */


/*----------------------------------------------------------------------*/
/*-----------------  process IM_ChnageMode  ----------------------------*/
/*----------------------------------------------------------------------*/

/************************************************************************
*                                                                       *
* Function : p10()                                                      *
* Description : Porcess the IM_ChangeMode                               *
*               - To set the processing mode                            *
*                 Normal-Mode / Suppressed-Mode                         *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*         2. IMError                                                    *
*         3. IMInvalidParameter                                         *
*                                                                       *
*************************************************************************/

int             p10()
{

   InputMode       input_mode;
   int             retcode;

   input_mode = TedGetInputMode(Obj->tedinfo.TedID);

   if ((int) Arg == IMNormalMode)                /* AP want to change mode to Normal mode */
   {
      if (input_mode.ind3 != NORMAL_MODE)
      {
         input_mode.ind3 = NORMAL_MODE;
         TedSetInputMode(Obj->tedinfo.TedID, input_mode);
         input_mode = TedGetInputMode(Obj->tedinfo.TedID);
         indlinemake(Obj);
         querystate(Obj);
         retcode = indicatordraw(Obj);
      }                                          /* if */
   }
    /* if */
   else                                          /* AP want to change mode to Suppressed Mode    */
   if ((int) Arg == IMSuppressedMode)
   {
      if (input_mode.ind3 != SUPPRESSED_MODE)
      {
         input_mode.ind3 = SUPPRESSED_MODE;
         TedSetInputMode(Obj->tedinfo.TedID, input_mode);
         input_mode = TedGetInputMode(Obj->tedinfo.TedID);
         indlinemake(Obj);
         querystate(Obj);
         retcode = indicatorhide(Obj);
      }                                          /* if */
   }
    /* if */
   else
      return IMError;                            /* invalid mode specified       */

   return (retcode);

}                                                /* p10() */

/************************************************************************
*                                                                       *
* Function : LongForm()                                                 *
* Description : Porcess the long format of indicator line.              *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*                                                                       *
*************************************************************************/

int             LongForm()
{

   char           *indicator, *ind_msg,*temp_ind;
   IMQueryIndicatorString *temp_arg;
   int             j;
   int             lang_type;                             /* @big5 */


   temp_ind = indicator = Obj->indstr.str;
   temp_arg = (IMQueryIndicatorString *) Arg;


   for ( j=0; j< IND_LONG_FORM; memcpy(temp_ind," ",1),temp_ind++,j++);

   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, ALPHA_NUM_EUC, ALPHA_NUM_LEN);   /* @big5 */
      break;
   case PHONETIC_MODE:
   case TSANG_JYE_MODE:
   case INTERNAL_CODE_MODE:
      memcpy(indicator, CHINESE_EUC, CHINESE_LEN);       /* @big5 */
      break;
   case CAPS_LOCK_MODE:
      memcpy(indicator, CAPS_LOCK_EUC, CAPS_LOCK_LEN );  /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + CHINESE_LEN;                  /* @big5 */
   memcpy(indicator,CODE_FLAG,strlen(CODE_FLAG));        /* @big5 */
   indicator = indicator + strlen(CODE_FLAG);            /* @big5 */

   switch (Obj->indinfo.size)
   {
   case HALF:
      memcpy(indicator, HALF_SIZE_EUC, HALF_SIZE_LEN);   /* @big5 */
      break;
   case FULL:
      memcpy(indicator, FULL_SIZE_EUC, FULL_SIZE_LEN);   /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + FULL_SIZE_LEN;                /* @big5 */

   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, BLANK_EUC, BLANK_LEN );          /* @big5 */
      break;
   case PHONETIC_MODE:
      memcpy(indicator, PHONETIC_EUC, L_PHONETIC_LEN);    /* @big5 */
      break;
   case TSANG_JYE_MODE:
      memcpy(indicator, TSANG_JYE_EUC, TSANG_JYE_LEN);   /* @big5 */
      break;
   case INTERNAL_CODE_MODE:
      memcpy(indicator, INTERNAL_CODE_EUC, INTERNAL_CODE_LEN);  /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + TSANG_JYE_LEN;                /* @big5 */
/*
   for (j = 0; j < 15; memcpy(indicator, DB_BLANK, sizeof(DB_BLANK) - 1), indicator += 2, j++);
*/
/*   indicator += 30;      debby */
     indicator += 20;   /* debby */
   ind_msg = TedGetIndMessage(Obj->tedinfo.TedID);

   memcpy(indicator, ind_msg, strlen(ind_msg));
/*
   indicator += strlen(ind_msg);
   memcpy(indicator, NULL, 1);
*/
   Obj->indstr.len = IND_LONG_FORM;

   temp_arg->indicator.len = Obj->indstr.len;
   temp_arg->indicator.str = Obj->indstr.str;

   return IMNoError;

}                                                /* LongForm() */

/************************************************************************
*                                                                       *
* Function : ShortForm()                                                *
* Description : Porcess the short format of indicator line.             *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A -> Using the global variables.                            *
* Output :                                                              *
*         1. IMNoError                                                  *
*                                                                       *
*************************************************************************/

int             ShortForm()
{

   char           *indicator, *ind_msg,*temp_ind;
   IMQueryIndicatorString *temp_arg;
   int             j;

   temp_ind = indicator = Obj->indstr.str;
   temp_arg = (IMQueryIndicatorString *) Arg;

   for(j=0; j<IND_SHORT_FORM;memcpy(temp_ind," ",1),temp_ind++,j++);

   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, S_ALPHA_NUM_EUC, S_ALPHA_NUM_LEN);     /* @big5 */
      break;
   case PHONETIC_MODE:
   case TSANG_JYE_MODE:
   case INTERNAL_CODE_MODE:
      memcpy(indicator, S_CHINESE_EUC, S_CHINESE_LEN);         /* @big5 */
      break;
   case CAPS_LOCK_MODE:
      memcpy(indicator, S_CAPS_LOCK_EUC, S_CAPS_LOCK_LEN);     /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + S_CHINESE_LEN;                      /* @big5 */
   memcpy(indicator,S_CODE_FLAG,strlen(S_CODE_FLAG));          /* @big5 */
   indicator = indicator + strlen(S_CODE_FLAG);                /* @big5 */


   switch (Obj->indinfo.size)
   {
   case HALF:
      memcpy(indicator, S_HALF_SIZE_EUC, S_HALF_SIZE_LEN);     /* @big5 */
      break;
   case FULL:
      memcpy(indicator, S_FULL_SIZE_EUC, S_FULL_SIZE_LEN);     /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + S_FULL_SIZE_LEN;                   /* @big5 */

   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, S_BLANK_EUC, S_BLANK_LEN);            /* @big5 */
      break;
   case PHONETIC_MODE:
      memcpy(indicator, S_PHONETIC_EUC, S_PHONETIC_LEN);      /* @big5 */
      break;
   case TSANG_JYE_MODE:
      memcpy(indicator, S_TSANG_JYE_EUC, S_TSANG_JYE_LEN);    /* @big5 */
      break;
   case INTERNAL_CODE_MODE:
      memcpy(indicator, S_INTERNAL_CODE_EUC, S_INTERNAL_CODE_LEN); /* @big5 */
      break;
   default:
      break;
   }                                             /* switch */

   indicator = indicator + S_TSANG_JYE_LEN;                   /* @big5 */
/*
   for (j = 0; j < 19; memcpy(indicator, " ", 1), indicator += 1, j++);
*/
/* indicator += 20;   debby */
/* indicator += 14;   /* debby */
   indicator += 10;   /* debby */
   ind_msg = TedGetIndMessage(Obj->tedinfo.TedID);

   memcpy(indicator, ind_msg, strlen(ind_msg));
/*
   indicator += strlen(ind_msg);
   memcpy(indicator, NULL, 1);
*/
   Obj->indstr.len = IND_SHORT_FORM;

   temp_arg->indicator.len = Obj->indstr.len;
   temp_arg->indicator.str = Obj->indstr.str;

   return IMNoError;

}                                                /* ShortForm() */
