static char sccsid[] = "@(#)20  1.3  src/bos/usr/lib/nls/loc/imt/tfep/timdisp.c, libtw, bos411, 9428A410j 4/21/94 02:29:14";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: auxcreate
 *              auxdestroy
 *              auxdraw
 *              auxhide
 *              auxinfomake
 *              beepmake
 *              getstringmake
 *              indicatordraw
 *              indicatorhide
 *              indlinemake
 *              makedisplaystr
 *              querystate
 *              text_proc_1
 *              text_proc_2
 *              textdraw
 *              texthide
 *              textinfoinit
 *              textinfomake
 *              textstart
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
 *       Module Name      : timdisp                                      *
 *                                                                       *
 *       Description Name : Chinese Input Method -> display routines     *
 *                                                                       *
 *       Function         : timdisp -> call CallBack routine to display  *
 *                                                                       *
 *       Module Type      : C                                            *
 *                                                                       *
 *       Compiler         : AIX C                                        *
 *                                                                       *
 *       Author           : Jim Roung                                    *
 *                                                                       *
 *       Status           : Chinese Input Methid Version 1.0             *
 *                                                                       *
 *       Change Activity  : T.B.W.                                       *
 *************************************************************************/


/* --------------------------------------------------------------------- */
/* *********************  Include File  ******************************** */
/* --------------------------------------------------------------------- */

#include "taiwan.h"
#include "tedacc.h"                                         /* @big5 */

/* --------------------------------------------------------------------- */
/* *************  Refer to external function  ************************** */
/* --------------------------------------------------------------------- */
extern EchoBufChanged TedIsEchoBufChanged();
extern AuxSize  TedGetAuxSize();
extern InputMode TedGetInputMode();
extern AuxCurPos TedGetAuxCurPos();
extern int      TedIsAuxBufChanged();
extern int      TedControl();
extern int      TedIsBeepRequested();
extern int      TedGetEchoBufLen();
extern int      TedGetEchoCurPos();
extern int      TedGetFixBufLen();
extern int      TedGetLangType();                           /* @big5 */
extern StrCodeConvert(iconv_t,
                      unsigned char *,
                      unsigned char *,
                      size_t,
                      size_t);                              /* @big5 */

/* ----------------------  Global Variable  -------------------------- */
int             offset;

/*---------------------------------------------------------------------*/
/* ************************* beepmake() ******************************* */
/*---------------------------------------------------------------------*/

int             beepmake(imobj)
   TIMOBJ          imobj;
{
   if (imobj->imobject.cb->beep)                 /* call CallBack -> Beep to make beep on */
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
   TIMOBJ          imobj;
{                                                /* Draw the status line (indicator line)                */

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
   TIMOBJ          imobj;
{
   InputMode       input_mode;

   input_mode = TedGetInputMode(imobj->tedinfo.TedID);

   imobj->querystate.mode = (input_mode.ind3 == NORMAL_MODE) ?
                                 IMNormalMode : IMSuppressedMode;
   imobj->querystate.text = (uint) TedIsEchoBufChanged(imobj->tedinfo.TedID).flag;
   imobj->querystate.aux = (uint) TedIsAuxBufChanged(imobj->tedinfo.TedID);

   imobj->querystate.indicator = (uint) TedIsInputModeChanged(imobj->tedinfo.TedID);
   imobj->querystate.beep = (TedIsBeepRequested(imobj->tedinfo.TedID))
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
   TIMOBJ          imobj;
{
   InputMode       input_mode;

   input_mode = TedGetInputMode(imobj->tedinfo.TedID);

   imobj->indinfo.size = (input_mode.ind1 == HALF) ? IMHalfWidth : IMFullWidth;
   imobj->indinfo.insert = (input_mode.ind4 == INSERT_MODE) ?
                                 IMInsertMode : IMReplaceMode;
   imobj->indinfo.unique = IND_INITIALIZE;

   switch (input_mode.ind0)
   {
   case ALPH_NUM_MODE:
      imobj->indinfo.unique |= IND_ALPHA_NUM;
      break;
   case PHONETIC_MODE:
      imobj->indinfo.unique |= IND_PHONETIC;
      break;
   case TSANG_JYE_MODE:
      imobj->indinfo.unique |= IND_TSANG_JYE;
      break;
   case INTERNAL_CODE_MODE:
      imobj->indinfo.unique |= IND_INTER_CODE;
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
   TIMOBJ          imobj;
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
      {
         return (IMNoError);
       }
}                                                /* auxdestroy() */

/*---------------------------------------------------------------------*/
/* ********************  AuxHide()  *********************************** */
/*---------------------------------------------------------------------*/

int             auxhide(imobj)
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
{
   AuxSize         aux_size;
   AuxCurPos       aux_cur_pos;
   int             aux_type;
   caddr_t        *temp_ptr;
   int             i, j;
   caddr_t        *temp_atr;                            /* @big5 */
   /* fill in Aux area structure   */
   aux_size = TedGetAuxSize(imobj->tedinfo.TedID);      /* check aux size and  */
   aux_cur_pos = TedGetAuxCurPos(imobj->tedinfo.TedID); /* aux cursor position */

   imobj->auxinfo.title.len = 0;
   imobj->auxinfo.title.str = NULL;
   imobj->auxinfo.selection.panel_row = 0;
   imobj->auxinfo.selection.panel_col = 0;
   imobj->auxinfo.button = IM_NONE;

   aux_type = TedGetAuxType(imobj->tedinfo.TedID);
   imobj->auxinfo.hint = (aux_type == Aux_Candidate_List) ?
                                 IM_AtTheEvent : IM_UpperRight;
   if (imobj->auxstate == USE)                   /* Aux Area exist before */
      if (TedIsAuxBufUsedNow(imobj->tedinfo.TedID))
         imobj->auxinfo.status = (TedIsAuxBufChanged(imobj->tedinfo.TedID)) ?
                                       IMAuxUpdated : IMAuxShowing;
      else
         imobj->auxinfo.status = IMAuxHidden;
   else
      imobj->auxinfo.status = (TedIsAuxBufUsedNow(imobj->tedinfo.TedID)) ?
                                    IMAuxShown : IMAuxHiding;

   /* ======= Fill in the message field  ====== */

   if (TedIsAuxBufUsedNow(imobj->tedinfo.TedID))
   {
      temp_ptr = imobj->tedinfo.auxbufs;
      temp_atr = imobj->tedinfo.auxbufa;                            /* @big5 */

      for (i = 0; i < aux_size.itemnum; i++)
      {
/*      memcpy(imobj->auxinfo.message.text[i].str, *temp_ptr++, aux_size.itemsize); */
                                                                    /* @big5 */
        imobj->auxinfo.message.text[i].len = aux_size.itemlen ;     /* @big5 */
        memcpy(imobj->auxinfo.message.text[i].att, *temp_atr++, aux_size.itemlen);
                                                                    /* @big5 */
        memcpy(imobj->auxinfo.message.text[i].str, *temp_ptr++, aux_size.itemlen);
                                                                    /* @big5 */
      }                                                             /* @big5 */


/*    temp_ptr = imobj->tedinfo.auxbufa;                               @big5 */
/*    for (i = 0; i < aux_size.itemnum; i++)                           @big5 */
/*    {                                                                @big5 */
/*       imobj->auxinfo.message.text[i].len = aux_size.itemsize;       @big5 */
/*       memcpy(imobj->auxinfo.message.text[i].att, *temp_ptr++, aux_size.itemsize);*/
/*    }                                                                @big5 */

      /* ========  Fill in IMMessage data structure  ===== */

      imobj->auxinfo.message.maxwidth = aux_size.itemsize;
      imobj->auxinfo.message.nline = aux_size.itemnum;

      /* === Decide there is cursor in Aux area or not === */

      imobj->auxinfo.message.cursor = ((aux_cur_pos.rowpos >= 0) &&
                                       (aux_cur_pos.colpos >= 0)) ?
                                    TRUE : FALSE;
      imobj->auxinfo.message.cur_row = aux_cur_pos.rowpos;
      imobj->auxinfo.message.cur_col = aux_cur_pos.colpos;

   }                                             /* if -> TedIsAuxBufUsedNow() */
}                                                /* auxinfomake() */

/*---------------------------------------------------------------------*/
/* **************** Porcess the text callback function **************** */
/*---------------------------------------------------------------------*/

/* IMED get key event and convert it to corresponding EUC. Meanwhile, IMED put that EUC into Echo buf. This function will process
 * the contents of Echo buf. Finally, it will call "TextDraw" to draw the contents of Echo buf                                              */

/*---------------------------------------------------------------------*/

void            text_proc_1(imobj)
   TIMOBJ          imobj;
{

   offset = imobj->textinfo.maxwidth;

   if (imobj->textstate == TEXT_OFF)             /* So far, no text */
      textstart(imobj);
   textdraw(imobj);

}                                                /* text_proc_1() */
/*---------------------------------------------------------------------*/

/* IMED receive the "convert key" and copy data from Echo buf into Fix buf. Meanwhile, reset the length of echo buf to zero. This
 * function will call "TextHide" callback to hide the text       */
/*---------------------------------------------------------------------*/

void            text_proc_2(imobj)
   TIMOBJ          imobj;
{

   if (imobj->textstate == TEXT_ON)
      texthide(imobj);

}                                                /* text_proc_2() */

/*---------------------------------------------------------------------*/
/* *************** Fill in textinfo structure ************************* */
/*---------------------------------------------------------------------*/

void            textinfomake(imobj)
   TIMOBJ          imobj;
{
   int             echo_len, i;
   char           *ptr;
   int             lang_type;                                        /* @big5 */
   unsigned char   code[80];                                         /* @big5 */
   size_t          in_count,out_count;                               /* @big5 */
   iconv_t         iconv_flag;                                       /* @big5 */


   echo_len = TedGetEchoBufLen(imobj->tedinfo.TedID);
   imobj->textinfo.text.len = echo_len;          /* fill in IMSTRATT struct      */
   ptr = imobj->textinfo.text.str;
   /* Call iconv to convert echobuffer to BIG5 code  by DEBBY  */
   lang_type= TedGetLangType(imobj->tedinfo.TedID);                   /* @big5 */
   if (lang_type != codesetinfo[0].code_type)                         /* @big5 */
   {                                                                  /* @big5 */
      iconv_flag = TedGetIconv(imobj->tedinfo.TedID);                 /* @big5 */
      in_count = echo_len;                                            /* @big5 */
      out_count = echo_len;                                           /* @big5 */
      StrCodeConvert(iconv_flag,imobj->tedinfo.echobufs,
                     code, &in_count,&out_count);                     /* @big5 */
      out_count = echo_len - out_count;                               /* @big5 */
      memcpy(ptr, code, echo_len);                                    /* @big5 */
   }                                                                  /* @big5 */
   else                                                               /* @big5 */
     memcpy(ptr, imobj->tedinfo.echobufs, echo_len);
   ptr = imobj->textinfo.text.att;               /* IMed have to fill in the attribute */
   memcpy(ptr, imobj->tedinfo.echobufa, echo_len);
   imobj->textinfo.maxwidth = echo_len;
   imobj->textinfo.cur = TedGetEchoCurPos(imobj->tedinfo.TedID);
   imobj->textinfo.chgtop = TedIsEchoBufChanged(imobj->tedinfo.TedID).chtoppos;
   imobj->textinfo.chglen = TedIsEchoBufChanged(imobj->tedinfo.TedID).chlenbytes;

}                                                /* textinfomake() */

/*---------------------------------------------------------------------*/
/*********************  makeDisplayStr()  ******************************/
/*---------------------------------------------------------------------*/

int             makedisplaystr(imobj)
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
{
   imobj->textinfo.cur = 0;
   imobj->textinfo.chgtop = 0;
   imobj->textinfo.chglen = 0;

}                                                /* textinfoinit() */

/*---------------------------------------------------------------------*/
/* *******************  TextStart  ************************************ */
/*---------------------------------------------------------------------*/

int             textstart(imobj)
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
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
   TIMOBJ          imobj;
{ /* Hide text area for future active     */
   if (imobj->imobject.cb->texthide)
   {
      if (HideText(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      }
    /* if */
      else
      {
         return (IMNoError);
       }
    }
}                                                /* texthide() */

/*---------------------------------------------------------------------*/
/* ******************  GetString()  *********************************** */
/*---------------------------------------------------------------------*/

void            getstringmake(imobj)
   TIMOBJ          imobj;
{

   int             len;

   len = TedGetFixBufLen(imobj->tedinfo.TedID);
   memcpy(imobj->string.str, imobj->output.data, len);
   imobj->string.len = len;


}                                                /* getstringmake() */

/*---------------------------------------------------------------------*/
/* ******************  Indicator Hide() ******************************* */
/*---------------------------------------------------------------------*/


int             indicatorhide(imobj)
   TIMOBJ          imobj;
{
   if (imobj->imobject.cb->indicatorhide)
      if (HideIndicator(imobj) == IMError)
      {
         imobj->imobject.imfep->imerrno = IMCallbackError;
         return (IMError);
      } else
         return (IMNoError);

}                                                /* indicatorhide() */
