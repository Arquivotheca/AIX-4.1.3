static char sccsid[] = "@(#)26	1.2  src/bos/usr/lib/nls/loc/CN.im/cnimioctl.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:30";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMIoctl
 *		LongForm
 *		ShortForm
 *		p00
 *		p01
 *		p02
 *		p03
 *		p04
 *		p05
 *		p06
 *		p07
 *		p08
 *		p09
 *		p10
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
/************************************************************************
*                                                                       *
*       Module Name      : CNIMIoctl                                    *
*                                                                       *
*       Description Name : Chinese Input Method Ioctl                   *
*                                                                       *
*       Function         : CNIMIoctl -> control IOCTL sub-call          *
*                                                                       *
*                                                                       *
************************************************************************/

/* ---------------  Include Files  --------------------------------- */
#include "chinese.h"

/* ***************  External reference Functions   ******************* */

extern InputMode CNedGetInputMode();
extern int      indicatordraw();
extern int      auxdraw();
extern int      textdraw();
extern void     getstringmake();
extern void     textinfomake();
extern void     auxinfomake();
extern int      querystate();
extern char    *CNedGetIndMessage();
extern int      indicatorhide();
extern int      indlinemake();
extern char    *CNedGetIndLegendL();
extern char    *CNedGetIndLegendS();

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

CNIMOBJ          Obj;
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
* Function : CNIMIoctl()                                                *
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

int             CNIMIoctl(imobj, operation, arg)
   CNIMOBJ          imobj;
   int             operation;
   char           *arg;
{
   int             state;
   int             ret_code;

   switch (operation)   
                          /* dispatch what's kind Ioctl calls are issued */
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

}                                                /* CNIMIoctl() */

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
   if (CNedGetEchoBufLen(Obj->cnedinfo.CNedID) > 0)
      retcode = textdraw(Obj);                   /* redraw text area     */
   if (CNedIsAuxBufUsedNow(Obj->cnedinfo.CNedID))
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


   Obj->textstate = (CNedGetEchoBufLen(Obj->cnedinfo.CNedID) == 0) ?
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

   Obj->textstate = (CNedGetEchoBufLen(Obj->cnedinfo.CNedID) == 0) ?
                                 TEXT_OFF : TEXT_ON;
   CNedClear(Obj->cnedinfo.CNedID);
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
   if (CNedIsInputModeChanged(Obj->cnedinfo.CNedID))
   {
      indlinemake(Obj);
      querystate(Obj);
      if (CNedGetInputMode(Obj->cnedinfo.CNedID).ind3 == NORMAL_MODE)
         retcode = indicatordraw(Obj);
   }                                             /* clear the error condition on indicator line */
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

   CNedControl(Obj->cnedinfo.CNedID, RESET_AUX, Arg);
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
*               - To change the max. length of text area.               *
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
   if ((int) Arg < CNedGetEchoBufLen(Obj->cnedinfo.CNedID))
   {
      Obj->imobject.imfep->imerrno = IMCallbackError;
      retcode = IMInvalidParameter;
   }
    /* if */
   else
      CNedControl(Obj->cnedinfo.CNedID, CHANGE_LENGTH, Arg);
     /* Dispatch appropriate routine based on command specified */

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

   /* Check the status of AIX Method  */
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


        /*  Get the current indicator value if it exist.       */
   ((IMQueryIndicator *) Arg)->indicatorinfo = (Obj->querystate.mode == IMNormalMode) ? &(Obj->indinfo) : NULL;
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
/*-----------------  process IM_ChangeMode  ----------------------------*/
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

   input_mode = CNedGetInputMode(Obj->cnedinfo.CNedID);

   if ((int) Arg == IMNormalMode)   
                                 /* AP want to change mode to Normal mode */
   {
      if (input_mode.ind3 != NORMAL_MODE)
      {
         input_mode.ind3 = NORMAL_MODE;
         CNedSetInputMode(Obj->cnedinfo.CNedID, input_mode);
         input_mode = CNedGetInputMode(Obj->cnedinfo.CNedID);
         indlinemake(Obj);
         querystate(Obj);
         retcode = indicatordraw(Obj);
      }                                          /* if */
   }
    /* if */
   else                                      
                            /* AP want to change mode to Suppressed Mode    */
   if ((int) Arg == IMSuppressedMode)
   {
      if (input_mode.ind3 != SUPPRESSED_MODE)
      {
         input_mode.ind3 = SUPPRESSED_MODE;
         CNedSetInputMode(Obj->cnedinfo.CNedID, input_mode);
         input_mode = CNedGetInputMode(Obj->cnedinfo.CNedID);
         indlinemake(Obj);
         querystate(Obj);
         retcode = indicatorhide(Obj);
      }                                          /* if */
   }
    /* if */
   else
      return IMError;                 /* invalid mode specified       */

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


   /* Get the CNIM Indicator string */
   temp_ind = indicator = Obj->indstr.str;
   temp_arg = (IMQueryIndicatorString *) Arg;

   for ( j=0; j< IND_LONG_FORM; memcpy(temp_ind," ",1),temp_ind++,j++);

  /*******************************************************************/
  /* Select the input method's running mode , and set the name of it */
  /* into the indicator area 					     */ 
  /*******************************************************************/
   switch (Obj->indinfo.unique & IM_MASK) 
   {      
   case ALPH_NUM_MODE:
      memcpy(indicator, ALPHA_NUM_EUC, sizeof(ALPHA_NUM_EUC) - 1);
      break;
   case ROW_COLUMN_MODE:
   case PINYIN_MODE:
   case ENGLISH_CHINESE_MODE:
   case FIVESTROKE_STYLE_MODE:
   case FIVESTROKE_MODE:
   case ABC_MODE:
   case USER_DEFINED_MODE:
      memcpy(indicator, CHINESE_EUC, sizeof(CHINESE_EUC) - 1);
      break;
/*   case CAPS_LOCK_MODE:
      memcpy(indicator, CAPS_LOCK_EUC, sizeof(CAPS_LOCK_EUC) - 1);
      break;
*/
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the next display information */
   indicator = indicator + sizeof(CHINESE_EUC) - 1;

   switch (Obj->indinfo.size)
   {
   case HALF:
      memcpy(indicator, HALF_SIZE_EUC, sizeof(HALF_SIZE_EUC) - 1);
      break;
   case FULL:
      memcpy(indicator, FULL_SIZE_EUC, sizeof(FULL_SIZE_EUC) - 1);
      break;
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the input mode name */
   indicator = indicator + sizeof(FULL_SIZE_EUC) - 1;

  /*******************************************************************/
  /* Select the input method, and fill it into indicator line        */
  /*******************************************************************/
   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, BLANK_EUC, sizeof(BLANK_EUC) - 1);
      break;
   case ROW_COLUMN_MODE:
      memcpy(indicator, ROW_COLUMN_EUC, sizeof(ROW_COLUMN_EUC) - 1);
      break;
   case PINYIN_MODE:
      memcpy(indicator, PINYIN_EUC, sizeof(PINYIN_EUC) - 1);
      break;
   case ENGLISH_CHINESE_MODE:
      memcpy(indicator, ENGLISH_CHINESE_EUC, sizeof(ENGLISH_CHINESE_EUC) - 1);
      break;
   case FIVESTROKE_STYLE_MODE:
      memcpy(indicator, FIVESTROKE_STYLE_EUC, sizeof(FIVESTROKE_STYLE_EUC) - 1);
      break;
   case FIVESTROKE_MODE:
      memcpy(indicator, FIVESTROKE_EUC, sizeof(FIVESTROKE_EUC) - 1);
      break;
   case ABC_MODE:
      memcpy(indicator, ABC_EUC, sizeof(ABC_EUC)-1);
      break;
   case USER_DEFINED_MODE:
      memcpy(indicator, USER_DEFINED_EUC, sizeof(USER_DEFINED_EUC)-1);
      break;
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the next display information */
   indicator = indicator + sizeof(ENGLISH_CHINESE_EUC) - 1;

   indicator += 20;
   ind_msg = CNedGetIndMessage(Obj->cnedinfo.CNedID);/* Get Indicator message */

   memcpy(indicator, ind_msg, strlen(ind_msg));
                                   /* Set the message into the indicator area */

   /* In the pinyin mode , set the name of the legend into the indicator area */
   if( (Obj->indinfo.unique & IM_MASK) == PINYIN_MODE )
   {
       indicator += 15;
       ind_msg = CNedGetIndLegendL(Obj->cnedinfo.CNedID);

       memcpy(indicator, ind_msg, strlen(ind_msg));
   }
   if( (Obj->indinfo.unique & IM_MASK) == FIVESTROKE_STYLE_MODE )
   {
       indicator += 15;
       ind_msg = CNedGetIndLegendL(Obj->cnedinfo.CNedID);

       memcpy(indicator, ind_msg, strlen(ind_msg));
   }
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

   /* Get the CNIM Indicator string */
   temp_ind = indicator = Obj->indstr.str;
   temp_arg = (IMQueryIndicatorString *) Arg;

   for(j=0; j<IND_SHORT_FORM;memcpy(temp_ind," ",1),temp_ind++,j++);

  /*******************************************************************/
  /* Select the input method's running mode , and set the name of it */
  /* into the indicator area 					     */ 
  /*******************************************************************/
   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, S_ALPHA_NUM_EUC, sizeof(S_ALPHA_NUM_EUC) - 1);
      break;
   case ROW_COLUMN_MODE:
   case PINYIN_MODE:
   case ENGLISH_CHINESE_MODE:
   case FIVESTROKE_STYLE_MODE:
   case FIVESTROKE_MODE:
   case ABC_MODE:
   case USER_DEFINED_MODE:
      memcpy(indicator, S_CHINESE_EUC, sizeof(S_CHINESE_EUC) - 1);
      break;
/*   case CAPS_LOCK_MODE:
      memcpy(indicator, S_CAPS_LOCK_EUC, sizeof(S_CAPS_LOCK_EUC) - 1);
      break;
*/
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the next display information */
   indicator = indicator + sizeof(S_CHINESE_EUC) - 1;


   switch (Obj->indinfo.size)
   {
   case HALF:
      memcpy(indicator, S_HALF_SIZE_EUC, sizeof(S_HALF_SIZE_EUC) - 1);
      break;
   case FULL:
      memcpy(indicator, S_FULL_SIZE_EUC, sizeof(S_FULL_SIZE_EUC) - 1);
      break;
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the input mode name */
   indicator = indicator + sizeof(S_FULL_SIZE_EUC) - 1;

  /*******************************************************************/
  /* Select the input method, and fill it into indicator line        */
  /*******************************************************************/
   switch (Obj->indinfo.unique & IM_MASK)
   {
   case ALPH_NUM_MODE:
      memcpy(indicator, S_BLANK_EUC, sizeof(S_BLANK_EUC) - 1);
      break;
   case ROW_COLUMN_MODE:
      memcpy(indicator, S_ROW_COLUMN_EUC, sizeof(S_ROW_COLUMN_EUC) - 1);
      break;
   case PINYIN_MODE:
      memcpy(indicator, S_PINYIN_EUC, sizeof(S_PINYIN_EUC) - 1);
      break;
   case ENGLISH_CHINESE_MODE:
      memcpy(indicator, S_ENGLISH_CHINESE_EUC, sizeof(S_ENGLISH_CHINESE_EUC) - 1);
      break;
   case FIVESTROKE_STYLE_MODE:
      memcpy(indicator, S_FIVESTROKE_STYLE_EUC, sizeof(S_FIVESTROKE_STYLE_EUC) - 1);
      break;
   case FIVESTROKE_MODE:
      memcpy(indicator, S_FIVESTROKE_EUC, sizeof(S_FIVESTROKE_EUC) - 1);
      break;
   case ABC_MODE:
      memcpy(indicator, S_ABC_EUC, sizeof(S_ABC_EUC)-1);
      break;
   case USER_DEFINED_MODE:
      memcpy(indicator, S_USER_DEFINED_EUC, sizeof(S_USER_DEFINED_EUC) - 1);
      break;
   default:
      break;
   }                                             /* switch */

   /* Calculate the location of the next display information */
   indicator = indicator + sizeof(S_ENGLISH_CHINESE_EUC) - 1;
   indicator += 20;
   ind_msg = CNedGetIndMessage(Obj->cnedinfo.CNedID);

   memcpy(indicator, ind_msg, strlen(ind_msg));

   /* In the pinyin mode , set the name of the legend into the indicator area */
   if( (Obj->indinfo.unique & IM_MASK) == PINYIN_MODE )
   {
       indicator += 15;
       ind_msg = CNedGetIndLegendS(Obj->cnedinfo.CNedID);
       memcpy(indicator, ind_msg, strlen(ind_msg));
   }
   if( (Obj->indinfo.unique & IM_MASK) == FIVESTROKE_STYLE_MODE )
   {
       indicator += 15;
       ind_msg = CNedGetIndLegendS(Obj->cnedinfo.CNedID);
       memcpy(indicator, ind_msg, strlen(ind_msg));
   }
   Obj->indstr.len = IND_SHORT_FORM;

   temp_arg->indicator.len = Obj->indstr.len;
   temp_arg->indicator.str = Obj->indstr.str;

   return IMNoError;

}                                                /* ShortForm() */
