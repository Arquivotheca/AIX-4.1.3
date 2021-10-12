static char sccsid[] = "@(#)69	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhimdisp.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:08";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: auxcreate
 *		auxdestroy
 *		auxdraw
 *		auxhide
 *		auxinfomake
 *		beepmake
 *		getstringmake
 *		indicatordraw
 *		indicatorhide
 *		indlinemake
 *		makedisplaystr
 *		querystate
 *		text_proc_1
 *		text_proc_2
 *		textdraw
 *		texthide
 *		textinfoinit
 *		textinfomake
 *		textstart
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
/*************************************************************************
 *                                                                       *
 *       Module Name      : zhimdisp                                     *
 *                                                                       *
 *       Description Name : Chinese Input Method -> display routines     *
 *                                                                       *
 *       Function         : zhimdisp -> call CallBack routine to display *
 *                                                                       *
 *************************************************************************/


/* --------------------------------------------------------------------- */
/* *********************  Include File  ******************************** */
/* --------------------------------------------------------------------- */

#include "chinese.h"

/* --------------------------------------------------------------------- */
/* *************  Refer to external function  ************************** */
/* --------------------------------------------------------------------- */
extern EchoBufChanged ZHedIsEchoBufChanged();
extern AuxSize  ZHedGetAuxSize();
extern InputMode ZHedGetInputMode();
extern AuxCurPos ZHedGetAuxCurPos();
extern int      ZHedIsAuxBufChanged();
extern int      ZHedControl();
extern int      ZHedIsBeepRequested();
extern int      ZHedGetEchoBufLen();
extern int      ZHedGetEchoCurPos();
extern int      ZHedGetFixBufLen();

/* ----------------------  Global Variable  -------------------------- */
int             offset;

/*---------------------------------------------------------------------*/
/* ************************* beepmake() ******************************* */
/*---------------------------------------------------------------------*/

int             beepmake(imobj)
   ZHIMOBJ         imobj;
{
   if (imobj->imobject.cb->beep)   
                                /* call CallBack -> Beep to make beep on */
      if (MakeBeep(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);
}                                                /* beepmake() */

/*---------------------------------------------------------------------*/
/* ********************* IndicatorDraw()  ***************************** */
/*---------------------------------------------------------------------*/

int             indicatordraw(imobj)
   ZHIMOBJ         imobj;
{                 
               /* Draw the status line (indicator line)    */

   if (imobj->imobject.cb->indicatordraw)        /* CallBack -> IndicatorDraw */
      if (DrawIndicator(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* indicatordraw() */

/*---------------------------------------------------------------------*/
/* *********************  Query State  ******************************** */
/*---------------------------------------------------------------------*/

void            querystate(imobj)
   ZHIMOBJ         imobj;
{
   InputMode       input_mode;

   input_mode = ZHedGetInputMode(imobj->zhedinfo.ZHedID);

   imobj->querystate.mode = (input_mode.ind3 == NORMAL_MODE) ?
                                 IMNormalMode : IMSuppressedMode;
   imobj->querystate.text = (uint) ZHedIsEchoBufChanged(imobj->zhedinfo.ZHedID).flag;
   imobj->querystate.aux = (uint) ZHedIsAuxBufChanged(imobj->zhedinfo.ZHedID);
   imobj->querystate.indicator = (uint) ZHedIsInputModeChanged(imobj->zhedinfo.ZHedID);
   imobj->querystate.beep = (ZHedIsBeepRequested(imobj->zhedinfo.ZHedID))
                                 ? BEEP_ON : BEEP_OFF;
}                                                /* querystate() */

/*---------------------------------------------------------------------*/
/* *****************  make the indicator line  *********************** */
/* */
/* Fill in the information for indicator line. Ex:                     */
/* English/Chinese, Full/Half and/or error condition.                  */
/* */
/*---------------------------------------------------------------------*/

void            indlinemake(imobj)
   ZHIMOBJ         imobj;
{
   InputMode       input_mode;

   input_mode = ZHedGetInputMode(imobj->zhedinfo.ZHedID);

   imobj->indinfo.size = (input_mode.ind1 == HALF) ? IMHalfWidth : IMFullWidth;
   imobj->indinfo.insert = (input_mode.ind4 == INSERT_MODE) ?
                                 IMInsertMode : IMReplaceMode;
   imobj->indinfo.unique = IND_INITIALIZE;

   switch (input_mode.ind0)
   {                    /*    Fill the input mode        */
   case ALPH_NUM_MODE:
      imobj->indinfo.unique |= IND_ALPHA_NUM;
      break;
   case PINYIN_MODE:
      imobj->indinfo.unique |= IND_PINYIN;
      break;
   case TSANG_JYE_MODE:
      imobj->indinfo.unique |= IND_TSANG_JYE;
      break;
   case ENGLISH_CHINESE_MODE:
      imobj->indinfo.unique |= IND_ENGLISH_CHINESE;
      break;
   case ABC_MODE:
      imobj->indinfo.unique |= IND_ABC;
      break;
   case USER_DEFINED_MODE:
      imobj->indinfo.unique |= IND_USER_DEFINED;
      break;
   default:
      break;
   }                                             /* switch */

   switch (input_mode.ind5)
   {
   case IND_BLANK:
      imobj->indinfo.unique |= IND_BLANK;
      break;
   case ERROR1:
      imobj->indinfo.unique |= IND_ERROR_MSG;
      break;
   case ERROR2:
      imobj->indinfo.unique |= IND_NO_WORD;
      break;
   case RADICAL:
      imobj->indinfo.unique |= IND_RADICAL;
   default:
      break;
   }                                             /* switch */

}                                                /* indlinemake() */

/*---------------------------------------------------------------------*/
/* **********************  AuxDestroy()  ****************************** */
/*---------------------------------------------------------------------*/

int             auxdestroy(imobj)
   ZHIMOBJ         imobj;
{
   /* Destroy the Aux. area        */

   if (imobj->imobject.cb->auxdestroy)
      if (DestroyAux(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* auxdestroy() */

/*---------------------------------------------------------------------*/
/* ********************  AuxHide()  *********************************** */
/*---------------------------------------------------------------------*/

int             auxhide(imobj)
   ZHIMOBJ         imobj;
{                                                /* Hide the Aux. area for future active */

   if (imobj->imobject.cb->auxhide)
      if (HideAux(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* auxhide() */

/*---------------------------------------------------------------------*/
/* ************************  AuxCreate()  ***************************** */
/*---------------------------------------------------------------------*/

int             auxcreate(imobj)
   ZHIMOBJ         imobj;
{                                                /* Create the Aux. area         */

   if (imobj->imobject.cb->auxcreate)
      if (CreateAux(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* auxcreate() */

/*---------------------------------------------------------------------*/
/* *********************  AuxDraw() *********************************** */
/*---------------------------------------------------------------------*/

int             auxdraw(imobj)
   ZHIMOBJ         imobj;
{                                                /* Draw the Aux area    */

   if (imobj->imobject.cb->auxdraw)
      if (DrawAux(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* auxdraw() */

/*---------------------------------------------------------------------*/
/* ********************* Auxinfomake()  ******************************* */
/*---------------------------------------------------------------------*/

void            auxinfomake(imobj)
   ZHIMOBJ         imobj;
{
   AuxSize         aux_size;
   AuxCurPos       aux_cur_pos;
   int             aux_type;
   caddr_t        *temp_ptr;
   int             i, j;
   /* fill in Aux area structure   */
   aux_size = ZHedGetAuxSize(imobj->zhedinfo.ZHedID);      /* check aux size and  */
   aux_cur_pos = ZHedGetAuxCurPos(imobj->zhedinfo.ZHedID); /* aux cursor position */

   imobj->auxinfo.title.len = 0;
   imobj->auxinfo.title.str = NULL;
   imobj->auxinfo.selection.panel_row = 0;
   imobj->auxinfo.selection.panel_col = 0;
   imobj->auxinfo.button = IM_NONE;

   aux_type = ZHedGetAuxType(imobj->zhedinfo.ZHedID);
   imobj->auxinfo.hint = (aux_type == Aux_Candidate_List) ?
                                 IM_AtTheEvent : IM_UpperRight;
   if (imobj->auxstate == USE)                   /* Aux Area exist before */
      if (ZHedIsAuxBufUsedNow(imobj->zhedinfo.ZHedID))
         imobj->auxinfo.status = (ZHedIsAuxBufChanged(imobj->zhedinfo.ZHedID)) ?
                                       IMAuxUpdated : IMAuxShowing;
      else
         imobj->auxinfo.status = IMAuxHidden;
   else
      imobj->auxinfo.status = (ZHedIsAuxBufUsedNow(imobj->zhedinfo.ZHedID)) ?
                                    IMAuxShown : IMAuxHiding;

   /* ======= Fill in the message field  ====== */

   if (ZHedIsAuxBufUsedNow(imobj->zhedinfo.ZHedID))
   {
      temp_ptr = imobj->zhedinfo.auxbufs;
      for (i = 0; i < aux_size.itemnum; i++)
         memcpy(imobj->auxinfo.message.text[i].str, *temp_ptr++, aux_size.itemsize);
      temp_ptr = imobj->zhedinfo.auxbufa;
      for (i = 0; i < aux_size.itemnum; i++)
      {
         imobj->auxinfo.message.text[i].len = aux_size.itemsize;
         memcpy(imobj->auxinfo.message.text[i].att, *temp_ptr++, aux_size.itemsize);
      }

      /* ========  Fill in IMMessage data structure  ===== */

      imobj->auxinfo.message.maxwidth = aux_size.itemsize;
      imobj->auxinfo.message.nline = aux_size.itemnum;

      /* === Decide there is cursor in Aux area or not === */

      imobj->auxinfo.message.cursor = ((aux_cur_pos.rowpos >= 0) &&
                                       (aux_cur_pos.colpos >= 0)) ?
                                    TRUE : FALSE;
      imobj->auxinfo.message.cur_row = aux_cur_pos.rowpos;
      imobj->auxinfo.message.cur_col = aux_cur_pos.colpos;

   }                                           /* if -> ZHedIsAuxBufUsedNow() */
}                                              /* auxinfomake() */

/*---------------------------------------------------------------------*/
/* **************** Porcess the text callback function **************** */
/*---------------------------------------------------------------------*/

/* IMED get key event and convert it to corresponding UTF. Meanwhile, *
 * IMED put that UTF into Echo buf. This function will process the    *
 * contents of Echo buf. Finally, it will call "TextDraw" to draw the *
 * contents of Echo buf                                               */
/*---------------------------------------------------------------------*/

void            text_proc_1(imobj)
   ZHIMOBJ         imobj;
{

   offset = imobj->textinfo.maxwidth;

   if (imobj->textstate == TEXT_OFF)             /* So far, no text */
      textstart(imobj);
   textdraw(imobj);

}                                                /* text_proc_1() */
/*---------------------------------------------------------------------*/

/* IMED receive the "convert key" and copy data from Echo buf into Fix buf.
 * Meanwhile, reset the length of echo buf to zero. This
 * function will call "TextHide" callback to hide the text       */
/*---------------------------------------------------------------------*/

void            text_proc_2(imobj)
   ZHIMOBJ         imobj;
{

   if (imobj->textstate == TEXT_ON)
      texthide(imobj);

}                                                /* text_proc_2() */

/*---------------------------------------------------------------------*/
/* *************** Fill in textinfo structure ************************* */
/*---------------------------------------------------------------------*/

void            textinfomake(imobj)
   ZHIMOBJ         imobj;
{
   int             echo_len, i;
   char           *ptr;

   echo_len = ZHedGetEchoBufLen(imobj->zhedinfo.ZHedID);
   imobj->textinfo.text.len = echo_len;          /* fill in IMSTRATT struct */
   ptr = imobj->textinfo.text.str;
   memcpy(ptr, imobj->zhedinfo.echobufs, echo_len);
   ptr = imobj->textinfo.text.att;               /* IMed have to fill in the attribute */
   memcpy(ptr, imobj->zhedinfo.echobufa, echo_len);
   imobj->textinfo.maxwidth = echo_len;
   imobj->textinfo.cur = ZHedGetEchoCurPos(imobj->zhedinfo.ZHedID);
   imobj->textinfo.chgtop = ZHedIsEchoBufChanged(imobj->zhedinfo.ZHedID).chtoppos;
   imobj->textinfo.chglen = ZHedIsEchoBufChanged(imobj->zhedinfo.ZHedID).chlenbytes;

}                                                /* textinfomake() */

/*---------------------------------------------------------------------*/
/*********************  makeDisplayStr()  ******************************/
/*---------------------------------------------------------------------*/

int             makedisplaystr(imobj)
   ZHIMOBJ         imobj;
{
   int             ret_len = 0;
   /* whnever the IMed get something in FixBuf, this routine will be executed */
   ret_len = imobj->output.len;
   memcpy(imobj->convert_str, imobj->output.data, imobj->output.len);
   return (ret_len);

}                                                /* makeDisplayStr() */

/*---------------------------------------------------------------------*/
/* *********** IMTextInfo structure initialization  ******************* */
/*---------------------------------------------------------------------*/

void            textinfoinit(imobj)
   ZHIMOBJ         imobj;
{
   imobj->textinfo.cur = 0;
   imobj->textinfo.chgtop = 0;
   imobj->textinfo.chglen = 0;
}                                                /* textinfoinit() */

/*---------------------------------------------------------------------*/
/* *******************  TextStart  ************************************ */
/*---------------------------------------------------------------------*/

int             textstart(imobj)
   ZHIMOBJ         imobj;
{                                                /* start the Text (echo) area (before drawing)  */

   if (imobj->imobject.cb->textstart)
      if (StartText(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* textstatr() */


/*---------------------------------------------------------------------*/
/* ************************  TextDraw()  ****************************** */
/*---------------------------------------------------------------------*/

int             textdraw(imobj)
   ZHIMOBJ         imobj;
{                                                /* draw text area (echo area)   */

   if (imobj->imobject.cb->textdraw)
      if (DrawText(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);

}                                                /* textdraw() */

/*---------------------------------------------------------------------*/
/*********************  TextHide()  ************************************/
/*---------------------------------------------------------------------*/

int             texthide(imobj)
   ZHIMOBJ         imobj;
{                                                /* Hide text area for future active     */
   if (imobj->imobject.cb->texthide)
      if (HideText(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
         return (IMNoError);
}                                                /* texthide() */

/*---------------------------------------------------------------------*/
/* ******************  GetString()  *********************************** */
/*---------------------------------------------------------------------*/

void            getstringmake(imobj)
   ZHIMOBJ         imobj;
{

   int             len;

   len = ZHedGetFixBufLen(imobj->zhedinfo.ZHedID);
   memcpy(imobj->string.str, imobj->output.data, len);
   imobj->string.len = len;


}                                                /* getstringmake() */

/*---------------------------------------------------------------------*/
/* ******************  Indicator Hide() ******************************* */
/*---------------------------------------------------------------------*/


int             indicatorhide(imobj)
   ZHIMOBJ         imobj;
{
   if (imobj->imobject.cb->indicatorhide)
      if (HideIndicator(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      } else
         return (IMNoError);

}                                                /* indicatorhide() */
