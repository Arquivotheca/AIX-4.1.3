static char sccsid[] = "@(#)73	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhimproce.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:39:02";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMFilter
 *		ZHIMLookup
 *		ZHIMProcess
 *		zhimFilter
 *		zhimLookup
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
*       Module Name      : ZHIMProcess                                  *
*                                                                       *
*       Description Name : Chinese Input Method Process                 *
*                                                                       *
*       Function         : ZHIMProcess : Process the Keyboard event and *
*                          terminal display (CallBack)                  *
*                                                                       *
*************************************************************************/


/* ********************** Include File ******************************** */

#include "chinese.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/*-------------------------------------------------------------------*/
/***************  Refer to external function  ************************/
/*-------------------------------------------------------------------*/

extern EchoBufChanged ZHedIsEchoBufChanged();
extern AuxSize  ZHedGetAuxSize();
extern InputMode ZHedGetInputMode();
extern int      ZHedIsAuxBufChanged();
extern int      ZHedControl();
extern int      ZHedIsBeepRequested();
extern int      ZHedGetEchoBufLen();
extern int      ZHedGetEchoCurPos();
extern int      ZHedGetFixBufLen();
extern int      ZHedIsAuxBufUsedNow();
extern int      ZHedSetInputMode();
extern void     ZHedSetWarning();
extern int      beepmake();
extern int      indicatordraw();
extern void     querystate();
extern void     indlinemake();
extern int      auxdestroy();
extern int      auxhide();
extern int      auxcreate();
extern int      auxdraw();
extern void     auxinfomake();
extern void     text_proc_1();
extern void     text_proc_2();
extern void     textinfomake();
extern int      makedisplaystr();
extern void     textinfoinit();

static void     zhimLookup();


/* ********************************************************************** *
 *
 * ZHIMProcess() ...                                                    * *
 *
********************************************************************** */

int             ZHIMProcess(imobj, keysym, state, str, len)
   ZHIMOBJ          imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{
   /* check is modifier key or not ?       */
   if (IsModifierKey(keysym)) 
      *str = *(caddr_t*)len = 0;

   if (state & Mod5Mask)                         /* opearated with Mod5Mask */
   {
      if (IsKeypadKey(keysym))                   /* Is Keypas key or not ?   */
         state ^= ShiftMask;
      state &= ~Mod5Mask;

   }                                             /* if */
   state &= ZHIM_VALID_BITS;

   imobj->output.len = 0;                        /* initialization        */

   /************************************************************************/
   /* If the AIX Chinese Input Method not uses the key routine, it use an  */
   /* internal Chinese Input Method keysym file to map keysym/modifier     */
   /* to a string.                                                         */
   /************************************************************************/
   if (zhimFilter(imobj, keysym, state, &imobj->output) == IMInputNotUsed)
      zhimLookup(imobj, keysym, state, &imobj->output);

   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return imobj->state_aux_text;

}                                                /* ZHIMProcess() */


/* ********************************************************************** *
 *                                                                        *
 * ZHIMLookup() ...                                                       *
 *         The ZHIMLookup subroutine is used to map a keysym/state pair   *
 *         to a localized string. It uses an internal Chinese input method*
 *         keysym file to map a keysym/modifier to a string. The string   *
 *         returned is encoded in the same code set as the Chinese name   *
 *         passed to the ZHIMInitialize subroutine.                       *
 *                                                                        *
 ************************************************************************ */

int             ZHIMLookup(imobj, keysym, state, str, len)
   ZHIMOBJ          imobj;
   uint            keysym;
   uint            state;
   caddr_t        *str;
   uint           *len;
{

   if (state & Mod5Mask)
   {
      if (IsKeypadKey(keysym))
         state ^= ShiftMask;
      state &= ~Mod5Mask;

   }                                             /* if */
   state &= ZHIM_VALID_BITS;

   imobj->output.len = 0;                        /* initialization       */
   zhimLookup(imobj, keysym, state, &imobj->output);
   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return *len != 0 ? IMReturnString : IMReturnNothing;

}                                                /* ZHIMLookup() */


/* *********************************************************************
 *                                                                     *
 * ZHIMFilter() ...                                                    *
 *        Process a keyboard event and determine if the AIX Chinese    *
 *        Input Method uses the key event.                             *
 *                                                                     *
********************************************************************** */

int             ZHIMFilter(imobj, keysym, state, str, len)
   ZHIMOBJ         imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{

   uint            ret_code;

   if (IsModifierKey(keysym))
      *str = *(caddr_t*)len = 0;
   if (state & Mod5Mask)
   {
      if (IsKeypadKey(keysym))
         state ^= ShiftMask;
         state &= ~Mod5Mask;

   }                                             /* if */
   state &= ZHIM_VALID_BITS;
   imobj->output.len = 0;                        /* initialization       */
   ret_code = zhimFilter(imobj, keysym, state, &imobj->output);

   if(erase_flag == OFF)  /* For decimal IM (Alt + Keypad) -> erase previous data */
   {
      *len = makedisplaystr(imobj);     /* No Alt + Keypad data in buffer       */
      *str = imobj->convert_str;
   }
   else /* erase_flag == ON */    
       *len = *(uint*)str = 0;                 /* erase data in buffer (Alt + Keypad   */

   return ret_code;

}                                                /* ZHIMFilter() */



/* ********************************************************************
 *                                                                    *
 * zhimFilter() ...                                                   *
 *        Process a keyboard event and determine if the AIX Chinese    *
 *        Input Method uses the key event.                             *
 *                                                                    *
 **********************************************************************/

int             zhimFilter(imobj, keysym, state, imb)
   ZHIMOBJ         imobj;
   uint            keysym;
   uint            state;
   IMBuffer       *imb;
{

   /*---------------------------------------------------------------- */
   /* -------------------  Local variable  -------------------------- */
   /*---------------------------------------------------------------- */

   int             ret_code;
   int             ZHed_ret;
   AuxSize         aux_size;
   IMFep           zhimfep;
   int             zhedID;
   InputMode       imode;

   /*---------------------------------------------------------------- */
   /* ------------------  Initialization  --------------------------- */
   /*---------------------------------------------------------------- */

   erase_flag = OFF;
   zhimfep = imobj->imobject.imfep;
   zhedID = imobj->zhedinfo.ZHedID;
   imobj->state_aux_text = IMTextAndAuxiliaryOff;
   if (ZHedGetEchoBufLen(imobj->zhedinfo.ZHedID) == 0)
      imobj->textstate = TEXT_OFF;               /* text area isn't created      */
    else
      imobj->textstate = TEXT_ON;                /* text area is created         */

   textinfoinit(imobj);

   ZHed_ret = ZHedFilter(zhedID, ((ZHIMFEP) zhimfep)->keymap, keysym, state, imb);

   /*---------------------------------------------------------------- */
   /* -----------  process the text area -> Echo Buffer  ------------ */
   /*---------------------------------------------------------------- */

   if (ZHedGetEchoBufLen(imobj->zhedinfo.ZHedID))
   {
      erase_flag = ON;
      imobj->state_aux_text = IMTextOn;
   }

   if (ZHedIsEchoBufChanged(imobj->zhedinfo.ZHedID).flag ||
       ZHedIsEchoCurPosChanged(imobj->zhedinfo.ZHedID))
   {
      if (imobj->state_aux_text == IMTextOn)     /* there are some text data           */
      {
         textinfomake(imobj);                    /* prepare the echo buf to display    */
         text_proc_1(imobj);
      } else                                     /* convert key is pressed        */
         text_proc_2(imobj);                     /* echo_buf->fix_buf, to call    */
      /* 'texthide' to hide text.      */
   }                                             /* if echo-buf changed or echo-cursor postion changed */
   /* ----------- process the Auxiliary area -> Aux buffer  --------- */
   /*---------------------------------------------------------------- */

   aux_size = ZHedGetAuxSize(imobj->zhedinfo.ZHedID);
   if ((aux_size.itemnum > 0 && aux_size.itemsize > 0)  /* check the Aux area exist or not ?    */
       && ZHedIsAuxBufUsedNow(imobj->zhedinfo.ZHedID))
      imobj->state_aux_text = (imobj->state_aux_text == IMTextOn) ?     /* both Aux area and text area exist */
                                    IMTextAndAuxiliaryOn : IMAuxiliaryOn;
   if (ZHedIsAuxBufChanged(imobj->zhedinfo.ZHedID) ||      /* Check Aux area/Aux cursor change ??  */
       ZHedIsAuxCurPosChanged(imobj->zhedinfo.ZHedID))
   {
      auxinfomake(imobj);                        /* Make Aux area        */
      if (ZHedIsAuxBufUsedNow(imobj->zhedinfo.ZHedID))     /* Aux. area is displayed or not => Now */
      {
         if (imobj->auxstate == NOTUSE)          /* Aux area did be used before  */
         {
            auxcreate(imobj);                    /* Create Aux area              */
            imobj->auxIDflag = TRUE;
         }                                       /* if */
         auxdraw(imobj);                         /* Alreay created, just draw Aux area           */
         imobj->auxstate = USE;
      }
       /* if */
      else
      {
         auxhide(imobj);                         /* Aux area didn't be used ===> hide it         */
         auxdestroy(imobj);                      /* destroy this Aux area                */
         imobj->auxstate = NOTUSE;               /* set necessary flag           */
         imobj->auxIDflag = FALSE;
      }                                          /* else */

   }                                             /* if ZHedIsAuxBufChanged() and ZHedIsAuxCurPosChanged() */
   /*---------------------------------------------------------------- */
   /* -------------- process the Indicator line ------------------    */
   /*---------------------------------------------------------------- */


/* =================================================================== */
/* -> if the key belongs to ModiferKey group and Text and Aux area is  */
/*    ON, then display error message and make a beep sound.            */

      if(IsModifierKey(keysym))
      {
         imode = ZHedGetInputMode(imobj->zhedinfo.ZHedID);
      /* if((imobj->state_aux_text == IMTextOn) || (imobj->state_aux_text == IMTextAndAuxiliaryOn))*/
         if((imode.ind0 == PINYIN_MODE) && (imobj->state_aux_text == IMTextAndAuxiliaryOn)) 
         {
            imode.ind5 = ERROR1;
            ZHedSetInputMode(imobj->zhedinfo.ZHedID,imode);
            ZHedSetWarning(imobj->zhedinfo.ZHedID);
         }
     }

   if (ZHedIsInputModeChanged(imobj->zhedinfo.ZHedID))     /* Check Indicator is changed or not    */
   {
      indlinemake(imobj);                        /* make indicator information   */
      querystate(imobj);
      if (ZHedGetInputMode(imobj->zhedinfo.ZHedID).ind3 == NORMAL_MODE)
         indicatordraw(imobj);
   }                                             /* if */
   /* --------------------------------------------------------------- */
   /* ------------- process the Beep is ON/OFF ------------------     */
   /*---------------------------------------------------------------- */

   if (ZHedIsBeepRequested(imobj->zhedinfo.ZHedID))
      beepmake(imobj);

   /*---------------------------------------------------------------- */
   /* ---------- Final Return Code  -----------                 */
   /*---------------------------------------------------------------- */

   if (ZHed_ret == ZHedProcError)
      return IMError;

   return ZHed_ret == IMED_USED ? IMInputUsed : IMInputNotUsed;

}                                                /* zhimFilter() */

/* ********************************************************************
 *                                                                    *
 * zhimLookup() ...                                                   *
 *                                                                    *
 **********************************************************************/

static void     zhimLookup(imobj, keysym, state, imb)
   ZHIMOBJ         imobj;
   uint            keysym, state;
   IMBuffer       *imb;
{

   IMFep           zhimfep;
   int             zhedID;

   zhimfep = imobj->imobject.imfep;
   zhedID = imobj->zhedinfo.ZHedID;
   ZHedLookup(zhedID, ((ZHIMFEP) zhimfep)->keymap, keysym, state, imb);

}                                                /* zhimLookup() */
