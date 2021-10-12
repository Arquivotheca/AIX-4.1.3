static char sccsid[] = "@(#)24  1.5  src/bos/usr/lib/nls/loc/imt/tfep/timproce.c, libtw, bos411, 9431A411a 7/26/94 13:07:42";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: timproce.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************
*                                                                       *
*       Module Name      : TIMProcess                                   *
*                                                                       *
*       Description Name : Chinese Input Method Process                 *
*                                                                       *
*       Function         : TIMProcess : Process the Keyboard event and  *
*                          terminal display (CallBack)                  *
*                                                                       *
*       Module Type      : C                                            *
*                                                                       *
*       Compiler         : AIX C                                        *
*                                                                       *
*       Author           : Jim Roung                                    *
*                                                                       *
*       Status           : Chinese Input Method Version 1.0             *
*                                                                       *
*       Chnage Activity  :                                              *
*                      18/12/90                                         *
*                      Create one new API - TedIsAuxBufUsedNow()        *
*                                                                       *
*                      June/6/91:                                       *
*                                                                       *
*                      For IMFilte() and IMLookUp()                     *
*                                                                       *
*************************************************************************/


/* ***************************************************************** Include File ***************************************************************** */


#include "taiwan.h"
#include "stdlib.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <X11/keysym.h>



/*-------------------------------------------------------------------*/
/***************  Refer to external function  ************************/
/*-------------------------------------------------------------------*/

extern EchoBufChanged TedIsEchoBufChanged();
extern AuxSize  TedGetAuxSize();
extern InputMode TedGetInputMode();
extern int      TedIsAuxBufChanged();
extern int      TedControl();
extern int      TedIsBeepRequested();
extern int      TedGetEchoBufLen();
extern int      TedGetEchoCurPos();
extern int      TedGetFixBufLen();
extern int      TedIsAuxBufUsedNow();
extern int      TedSetInputMode();
extern void     TedSetWarning();        /* New API , Added By Jim R. 9/18/'91   */
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

static void     timLookup();


/* ********************************************************************** *
 *
 * TIMProcess() ...                                                * *
 *
********************************************************************** */

int             TIMProcess(imobj, keysym, state, str, len)
   TIMOBJ          imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{

   if (IsModifierKey(keysym))                    /* check is modifier key or not ?       */
   {
      *str = *len = 0;
/*    return imobj->state_aux_text;     ==> For IC internal usage       */

   }                                             /* if */
   if (state & Mod5Mask)                         /* opearated with Mod5Mask              */
   {
      if (IsKeypadKey(keysym))                   /* Is Keypas key or not ?               */
         state ^= ShiftMask;
      state &= ~Mod5Mask;

   }                                             /* if */
   state &= TIM_VALID_BITS;

   imobj->output.len = 0;                        /* intialization        */
   if (timFilter(imobj, keysym, state, &imobj->output) == IMInputNotUsed)
      timLookup(imobj, keysym, state, &imobj->output);
   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return imobj->state_aux_text;

}                                                /* TIMProcess() */


/* ********************************************************************** *
 *
 * TIMLookup() ...                                                 * *
 *
********************************************************************** */

int             TIMLookup(imobj, keysym, state, str, len)
   TIMOBJ          imobj;
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
   state &= TIM_VALID_BITS;

   imobj->output.len = 0;                        /* initialization       */
   timLookup(imobj, keysym, state, &imobj->output);
   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return *len != 0 ? IMReturnString : IMReturnNothing;

}                                                /* TIMLookup() */


/* *********************************************************************
 *                                                                     *
 * TIMFilter() ...                                                     *
 *                                                                     *
********************************************************************** */

int             TIMFilter(imobj, keysym, state, str, len)
   TIMOBJ          imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{

   uint            ret_code;

   if (IsModifierKey(keysym))
   {
      *str = *len = 0;
/*    return IMInputNotUsed;    => For IC internal usage        */

   }                                             /* if */
   if (state & Mod5Mask)
   {
      if (IsKeypadKey(keysym))
         state ^= ShiftMask;
      state &= ~Mod5Mask;

   }                                             /* if */
   state &= TIM_VALID_BITS;
   imobj->output.len = 0;                        /* initialization       */
   ret_code = timFilter(imobj, keysym, state, &imobj->output);

   if(erase_flag == OFF)  /* For decimal IM (Alt + Keypad) -> erase previous data */
   {
      *len = makedisplaystr(imobj);     /* No Alt + Keypad data in buffer       */
      *str = imobj->convert_str;
   }
   else /* erase_flag == ON     */
       *len = *str = 0;                 /* erase data in buffer (Alt + Keypad   */

   return ret_code;

}                                                /* TIMFilter() */



/* ********************************************************************
 *                                                                    *
 * timFilter() ...                                                    *
 *                                                                    *
 **********************************************************************/

int             timFilter(imobj, keysym, state, imb)
   TIMOBJ          imobj;
   uint            keysym;
   uint            state;
   IMBuffer       *imb;
{

   /*---------------------------------------------------------------- */
   /* -------------------  Local variable  -------------------------- */
   /*---------------------------------------------------------------- */

   int             ret_code;
   int             Ted_ret;
   AuxSize         aux_size;
   IMFep           timfep;
   int             tedID;
   InputMode       imode;

   /*---------------------------------------------------------------- */
   /* ------------------  Initialization  --------------------------- */
   /*---------------------------------------------------------------- */

   erase_flag = OFF;
   timfep = imobj->imobject.imfep;
   tedID = imobj->tedinfo.TedID;
   imobj->state_aux_text = IMTextAndAuxiliaryOff;
   if (TedGetEchoBufLen(imobj->tedinfo.TedID) == 0)
      imobj->textstate = TEXT_OFF;               /* text area isn't created      */
   else
      imobj->textstate = TEXT_ON;                /* text area is created         */

   Ted_ret = TedFilter(tedID, ((TIMFEP) timfep)->keymap, keysym, state, imb);

   /*---------------------------------------------------------------- */
   /* -----------  process the text area -> Echo Buffer  ------------ */
   /*---------------------------------------------------------------- */

   if (TedGetEchoBufLen(imobj->tedinfo.TedID))
   {
      imobj->state_aux_text = IMTextOn;
      erase_flag = ON;
   }

   if (TedIsEchoBufChanged(imobj->tedinfo.TedID).flag ||
       TedIsEchoCurPosChanged(imobj->tedinfo.TedID))
   {
      if (imobj->state_aux_text == IMTextOn)     /* there are some text data           */
      {
         textinfoinit(imobj);                  
         textinfomake(imobj);                    /* prepare the echo buf to display    */
         text_proc_1(imobj);
      } else                                     /* convert key is pressed        */
         text_proc_2(imobj);                     /* echo_buf->fix_buf, to call    */
      /* 'texthide' to hide text.      */
   }                                             /* if echo-buf changed or echo-cursor postion changed */
   /*
   /* ----------- process the Auxiliary area -> Aux buffer  --------- */
   /*---------------------------------------------------------------- */

   aux_size = TedGetAuxSize(imobj->tedinfo.TedID);
   if ((aux_size.itemnum > 0 && aux_size.itemsize > 0)  /* check the Aux area exist or not ?    */
       && TedIsAuxBufUsedNow(imobj->tedinfo.TedID))
      imobj->state_aux_text = (imobj->state_aux_text == IMTextOn) ?     /* both Aux area and text area exist    */
                                    IMTextAndAuxiliaryOn : IMAuxiliaryOn;
   if (TedIsAuxBufChanged(imobj->tedinfo.TedID) ||      /* Check Aux area/Aux cursor change ??  */
       TedIsAuxCurPosChanged(imobj->tedinfo.TedID))
   {
      if (imobj->state_aux_text == IMTextAndAuxiliaryOn) 
          auxinfomake(imobj);                        /* Make Aux area        */
      if (TedIsAuxBufUsedNow(imobj->tedinfo.TedID))     /* Aux. area is displayed or not => Now */
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
          auxhide(imobj);    
         auxdestroy(imobj);                      /* destroy this Aux area                */
         imobj->auxstate = NOTUSE;               /* set necessary flag           */
         imobj->auxIDflag = FALSE;
      }                                          /* else */

   }                                             /* if TedIsAuxBufChanged() and TedIsAuxCurPosChanged() */
   /*---------------------------------------------------------------- */
   /* -------------- process the Indicator line ------------------    */
   /*---------------------------------------------------------------- */


/* =================================================================== */
/* -> if the key belongs to ModiferKey group and Text and Aux area is  */
/*    ON, then display error message and make a beep sound.            */

      if(IsModifierKey(keysym))
      {
         imode = TedGetInputMode(imobj->tedinfo.TedID);
      /* if((imobj->state_aux_text == IMTextOn) || (imobj->state_aux_text == IMTextAndAuxiliaryOn))*/
         if((imode.ind0 == PHONETIC_MODE) && (imobj->state_aux_text == IMTextAndAuxiliaryOn))
         {  /* marked by Jim R. Nov/19/1991 -> Simplied TJ (*)  */
            imode.ind5 = ERROR1;
            TedSetInputMode(imobj->tedinfo.TedID,imode);
            TedSetWarning(imobj->tedinfo.TedID);
         }
     }
/* ===> Jim R Sept/18/'91 , Press ModifierKey => Beep! and Error Msg <===       */

   if (TedIsInputModeChanged(imobj->tedinfo.TedID))     /* Check Indicator is changed or not    */
   {
      indlinemake(imobj);                        /* make indicator information   */
      querystate(imobj);
      if (TedGetInputMode(imobj->tedinfo.TedID).ind3 == NORMAL_MODE)
         indicatordraw(imobj);
   }                                             /* if */
   /* --------------------------------------------------------------- */
   /* ------------- process the Beep is ON/OFF ------------------     */
   /*---------------------------------------------------------------- */

   if (TedIsBeepRequested(imobj->tedinfo.TedID))
      beepmake(imobj);

   /*---------------------------------------------------------------- */
   /* ---------- Final Return Code  -----------                 */
   /*---------------------------------------------------------------- */

   if (Ted_ret == TedProcError)
      return IMError;

   return Ted_ret == IMED_USED ? IMInputUsed : IMInputNotUsed;

}                                                /* timFilter() */

/* ********************************************************************
 *                                                                    *
 * timLookup() ...                                                    *
 *                                                                    *
 **********************************************************************/

static void     timLookup(imobj, keysym, state, imb)
   TIMOBJ          imobj;
   uint            keysym, state;
   IMBuffer       *imb;
{

   IMFep           timfep;
   int             tedID;

   timfep = imobj->imobject.imfep;
   tedID = imobj->tedinfo.TedID;
   TedLookup(tedID, ((TIMFEP) timfep)->keymap, keysym, state, imb);

}                                                /* timLookup() */
