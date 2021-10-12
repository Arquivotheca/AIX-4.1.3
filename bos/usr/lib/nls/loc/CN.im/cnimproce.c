static char sccsid[] = "@(#)28	1.2  src/bos/usr/lib/nls/loc/CN.im/cnimproce.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:32";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMFilter
 *		CNIMLookup
 *		CNIMProcess
 *		cnimFilter
 *		cnimLookup
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
*       Module Name      : CNIMProcess                                  *
*                                                                       *
*       Description Name : Chinese Input Method Process                 *
*                                                                       *
*       Function         : CNIMProcess : Process the Keyboard event and *
*                          terminal display (CallBack)                  *
*                                                                       *
*************************************************************************/


/* ********************** Include File ******************************** */


#include "chinese.h"
#include "stdlib.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <X11/keysym.h>

/*-------------------------------------------------------------------*/
/***************  Refer to external function  ************************/
/*-------------------------------------------------------------------*/

extern EchoBufChanged CNedIsEchoBufChanged();
extern AuxSize  CNedGetAuxSize();
extern InputMode CNedGetInputMode();
extern int      CNedIsAuxBufChanged();
extern int      CNedControl();
extern int      CNedIsBeepRequested();
extern int      CNedGetEchoBufLen();
extern int      CNedGetEchoCurPos();
extern int      CNedGetFixBufLen();
extern int      CNedIsAuxBufUsedNow();
extern int      CNedSetInputMode();
extern void     CNedSetWarning();
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

static void     cnimLookup();


/* ********************************************************************** *
 *
 * CNIMProcess() ...                                                    * *
 *
********************************************************************** */

int             CNIMProcess(imobj, keysym, state, str, len)
   CNIMOBJ          imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{
   /* check is modifier key or not ?       */
   if (IsModifierKey(keysym)){ 
      *str = 0; *len = 0;}

   if (state & Mod5Mask)                         /* opearated with Mod5Mask */
   {
      if (IsKeypadKey(keysym))                   /* Is Keypas key or not ?   */
         state ^= ShiftMask;
      state &= ~Mod5Mask;

   }                                             /* if */
   state &= CNIM_VALID_BITS;

   imobj->output.len = 0;                        /* initialization        */

   /************************************************************************/
   /* If the AIX Chinese Input Method not uses the key routine, it use an  */
   /* internal Chinese Input Method keysym file to map keysym/modifier     */
   /* to a string.                                                         */
   /************************************************************************/
   if (cnimFilter(imobj, keysym, state, &imobj->output) == IMInputNotUsed)
      cnimLookup(imobj, keysym, state, &imobj->output);

   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return imobj->state_aux_text;

}                                                /* CNIMProcess() */


/* ********************************************************************** *
 *                                                                        *
 * CNIMLookup() ...                                                       *
 *         The CNIMLookup subroutine is used to map a keysym/state pair   *
 *         to a localized string. It uses an internal Chinese input method*
 *         keysym file to map a keysym/modifier to a string. The string   *
 *         returned is encoded in the same code set as the Chinese name   *
 *         passed to the CNIMInitialize subroutine.                       *
 *                                                                        *
 ************************************************************************ */

int             CNIMLookup(imobj, keysym, state, str, len)
   CNIMOBJ          imobj;
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
   state &= CNIM_VALID_BITS;

   imobj->output.len = 0;                        /* initialization       */
   cnimLookup(imobj, keysym, state, &imobj->output);
   *len = makedisplaystr(imobj);
   *str = imobj->convert_str;
   return *len != 0 ? IMReturnString : IMReturnNothing;

}                                                /* CNIMLookup() */


/* *********************************************************************
 *                                                                     *
 * CNIMFilter() ...                                                    *
 *        Process a keyboard event and determine if the AIX Chinese    *
 *        Input Method uses the key event.                             *
 *                                                                     *
********************************************************************** */

int             CNIMFilter(imobj, keysym, state, str, len)
   CNIMOBJ         imobj;
   uint            keysym, state;
   caddr_t        *str;
   uint           *len;
{

   uint            ret_code;

   if (IsModifierKey(keysym)){
      *str = 0;  *len = 0;}
   if (state & Mod5Mask)
   {
      if (IsKeypadKey(keysym))
         state ^= ShiftMask;
         state &= ~Mod5Mask;

   }                                             /* if */
   state &= CNIM_VALID_BITS;
   imobj->output.len = 0;                        /* initialization       */
   ret_code = cnimFilter(imobj, keysym, state, &imobj->output);

   if(erase_flag == OFF)  /* For decimal IM (Alt + Keypad) -> erase previous data */
   {
      *len = makedisplaystr(imobj);     /* No Alt + Keypad data in buffer       */
      *str = imobj->convert_str;
   }
   else{ /* erase_flag == ON */    
       *len = 0; *str = 0;}                 /* erase data in buffer (Alt + Keypad   */

   return ret_code;

}                                                /* CNIMFilter() */



/* ********************************************************************
 *                                                                    *
 * cnimFilter() ...                                                   *
 *        Process a keyboard event and determine if the AIX Chinese    *
 *        Input Method uses the key event.                             *
 *                                                                    *
 **********************************************************************/

int             cnimFilter(imobj, keysym, state, imb)
   CNIMOBJ         imobj;
   uint            keysym;
   uint            state;
   IMBuffer       *imb;
{

   /*---------------------------------------------------------------- */
   /* -------------------  Local variable  -------------------------- */
   /*---------------------------------------------------------------- */

   int             ret_code;
   int             CNed_ret;
   AuxSize         aux_size;
   IMFep           cnimfep;
   int             cnedID;
   InputMode       imode;

   /*---------------------------------------------------------------- */
   /* ------------------  Initialization  --------------------------- */
   /*---------------------------------------------------------------- */

   erase_flag = OFF;
   cnimfep = imobj->imobject.imfep;
   cnedID = imobj->cnedinfo.CNedID;
   imobj->state_aux_text = IMTextAndAuxiliaryOff;
   if (CNedGetEchoBufLen(imobj->cnedinfo.CNedID) == 0)
      imobj->textstate = TEXT_OFF;               /* text area isn't created      */
    else
      imobj->textstate = TEXT_ON;                /* text area is created         */

   textinfoinit(imobj);

   CNed_ret = CNedFilter(cnedID, ((CNIMFEP) cnimfep)->keymap, keysym, state, imb);

   /*---------------------------------------------------------------- */
   /* -----------  process the text area -> Echo Buffer  ------------ */
   /*---------------------------------------------------------------- */

   if (CNedGetEchoBufLen(imobj->cnedinfo.CNedID))
   {
      erase_flag = ON;
      imobj->state_aux_text = IMTextOn;
   }

   if (CNedIsEchoBufChanged(imobj->cnedinfo.CNedID).flag ||
       CNedIsEchoCurPosChanged(imobj->cnedinfo.CNedID))
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

   aux_size = CNedGetAuxSize(imobj->cnedinfo.CNedID);
   if ((aux_size.itemnum > 0 && aux_size.itemsize > 0)  /* check the Aux area exist or not ?    */
       && CNedIsAuxBufUsedNow(imobj->cnedinfo.CNedID))
      imobj->state_aux_text = (imobj->state_aux_text == IMTextOn) ?     /* both Aux area and text area exist */
                                    IMTextAndAuxiliaryOn : IMAuxiliaryOn;
   if (CNedIsAuxBufChanged(imobj->cnedinfo.CNedID) ||      /* Check Aux area/Aux cursor change ??  */
       CNedIsAuxCurPosChanged(imobj->cnedinfo.CNedID))
   {
      auxinfomake(imobj);                        /* Make Aux area        */
      if (CNedIsAuxBufUsedNow(imobj->cnedinfo.CNedID))     /* Aux. area is displayed or not => Now */
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

   }                                             /* if CNedIsAuxBufChanged() and CNedIsAuxCurPosChanged() */
   /*---------------------------------------------------------------- */
   /* -------------- process the Indicator line ------------------    */
   /*---------------------------------------------------------------- */


/* =================================================================== */
/* -> if the key belongs to ModiferKey group and Text and Aux area is  */
/*    ON, then display error message and make a beep sound.            */

      if(IsModifierKey(keysym))
      {
         imode = CNedGetInputMode(imobj->cnedinfo.CNedID);
      /* if((imobj->state_aux_text == IMTextOn) || (imobj->state_aux_text == IMTextAndAuxiliaryOn))*/
         if((imode.ind0 == PINYIN_MODE) && (imobj->state_aux_text == IMTextAndAuxiliaryOn)) 
         {
            imode.ind5 = ERROR1;
            CNedSetInputMode(imobj->cnedinfo.CNedID,imode);
            CNedSetWarning(imobj->cnedinfo.CNedID);
         }
     }

   if (CNedIsInputModeChanged(imobj->cnedinfo.CNedID))     /* Check Indicator is changed or not    */
   {
      indlinemake(imobj);                        /* make indicator information   */
      querystate(imobj);
      if (CNedGetInputMode(imobj->cnedinfo.CNedID).ind3 == NORMAL_MODE)
         indicatordraw(imobj);
   }                                             /* if */
   /* --------------------------------------------------------------- */
   /* ------------- process the Beep is ON/OFF ------------------     */
   /*---------------------------------------------------------------- */

   if (CNedIsBeepRequested(imobj->cnedinfo.CNedID))
      beepmake(imobj);

   /*---------------------------------------------------------------- */
   /* ---------- Final Return Code  -----------                 */
   /*---------------------------------------------------------------- */

   if (CNed_ret == CNedProcError)
      return IMError;

   return CNed_ret == IMED_USED ? IMInputUsed : IMInputNotUsed;

}                                                /* cnimFilter() */

/* ********************************************************************
 *                                                                    *
 * cnimLookup() ...                                                   *
 *                                                                    *
 **********************************************************************/

static void     cnimLookup(imobj, keysym, state, imb)
   CNIMOBJ         imobj;
   uint            keysym, state;
   IMBuffer       *imb;
{

   IMFep           cnimfep;
   int             cnedID;

   cnimfep = imobj->imobject.imfep;
   cnedID = imobj->cnedinfo.CNedID;
   CNedLookup(cnedID, ((CNIMFEP) cnimfep)->keymap, keysym, state, imb);

}                                                /* cnimLookup() */
