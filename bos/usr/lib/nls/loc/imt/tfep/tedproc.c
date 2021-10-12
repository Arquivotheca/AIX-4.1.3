static char sccsid[] = "@(#)14  1.6  src/bos/usr/lib/nls/loc/imt/tfep/tedproc.c, libtw, bos41J, 9523B_all 6/6/95 13:11:54";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: tedproc.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         TedProc                                               */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Process source file       */
/*                                                                            */
/* FUNCTION:            TedFilter       : Chinese Input Method Editor Filter  */
/*                                                                            */
/*                      TedLookup       : Find & Copy Keysym to Output Buffer */
/*                                                                            */
/*                      AnMain          : Entry Point of Alphnum Input Method */
/*                                                                            */
/*                      AnProcess       : Process Each Input Key              */
/*                                                                            */
/*                      AnInitial       : Initialization                      */
/*                                                                            */
/*                      maptobase       : map keysym to suitable keysym       */
/*                                                                            */
/*                      get_state       : Get The Current Internal Processing */
/*                                                                            */
/*                      AltKeypadProcess: Process Alt+Keypad Input Method     */
/*                                                                            */
/*                      makeoutputbuffer: Copy Fixed Buffer to Output Buffer  */
/*                                                                            */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Benson Lu, Terry Chou                                 */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                      Oct/24/1991 : Jim Roung                               */
/*                                                                            */
/*                                    Modified the keysym value.              */
/*                                    (Japanese ask us can't use their        */
/*                                     keysym name, so, I modified they       */
/*                                     to Hex. value)                         */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
#include "stdio.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "taiwan.h"
#include "message.h" /* @big5 */

#define iskeypadkeysym(k)       (XK_KP_Multiply <= (k) && (k) <= XK_KP_9)
#define isspnumkeysym(k)        (XK_KP_0 <= (k) && (k) <= XK_KP_9)
#define isasciikeysym(k)        (XK_space <= (k) && (k) <= XK_asciitilde)
#define XK_Phonetic             0xff48  /* Modified By Jim Roung        */
#define XK_Alph_Num             0xaff50
#define XK_Tsang_Jye            0xff47
#define XK_Internal_Code        0xff4a
#define XK_Full_Half            0xff42
#define XK_Convert              0xaff53
#define XK_Non_Convert          0xaff52
#define XK_Ctrl_Right           0xaff55
#define XK_Ctrl_Home            0xaff54
#define XK_ZH_Switch            0xaff59 	    /* Round Robin */
#define XK_ZH_Toggle            0xaff60		    /* ZH/Eng Toggle */
#define XK_Alt_Shift            0xaffe3             /* @big5 */

/******************************************************************************/
/* FUNCTION    : mapkeysymtokey                                               */
/* DESCRIPTION : convert keysym to key                                        */
/* INPUT       : keysym, state                                                */
/* OUTPUT      : key                                                          */
/******************************************************************************/
static unsigned char mapkeysymtokey(keysym,fepcb) /* @big5 */
unsigned int keysym;
FEPCB        *fepcb;         /* FEP Control Block    @big5 */
{
     unsigned int  current_mode;                  /* @big5 */

     switch( keysym )
     {
        case XK_Alph_Num        : return(ALPH_NUM_KEY);
                                  break;
        case XK_Phonetic        : return(PHONETIC_KEY);
                                  break;
        case XK_Tsang_Jye       : return(TSANG_JYE_KEY);
                                  break;
        case XK_Internal_Code   : return(INTERNAL_CODE_KEY);
                                  break;
        case XK_Full_Half       : return(FULL_HALF_KEY);
                                  break;
        case XK_Convert         : return(CONVERT_KEY);
                                  break;
        case XK_Non_Convert     : return(NON_CONVERT_KEY);
                                  break;
        case XK_BackSpace       : return(BACK_SPACE_KEY);
                                  break;
        case XK_Escape          : return(ESC_KEY);
                                  break;
        case XK_Home            : return(HOME_KEY);
                                  break;
        case XK_Left            : return(LEFT_ARROW_KEY);
                                  break;
        case XK_Right           : return(RIGHT_ARROW_KEY);
                                  break;
        case XK_Ctrl_Right      : return(CTRL_RIGHT_KEY);
                                  break;
        case XK_Ctrl_Home       : return(CTRL_HOME_KEY);
                                  break;
        case XK_Insert          : return(INSERT_KEY);
                                  break;
        case XK_Delete          : return(DELETE_KEY);
                                  break;
        case XK_Prior           : return(PGUP_KEY);
                                  break;
        case XK_Next            : return(PGDN_KEY);
                                  break;
        case XK_Up              : return(UP_KEY);
                                  break;
        case XK_Down            : return(DOWN_KEY);
                                  break;
        case XK_Return          : return(RETURN_KEY);
                                  break;
        case XK_Num_Lock        : return(NUM_LOCK_KEY);
                                  break;
        case XK_Alt_Shift       :                                  /* @big5 */
                                  current_mode = get_state(fepcb); /* @big5 */
                                  if (current_mode == MODE_NUMBER -1) /* @big5 */
                                     current_mode = 0;             /* @big5 */
                                  else                             /* @big5 */
                                     current_mode ++;              /* @big5 */
                                  return(MODE_KEY[current_mode]);  /* @big5 */
                                  break;                           /* @big5 */
        default :
                     if (isasciikeysym(keysym))
                         return(keysym);
                     else if (iskeypadkeysym(keysym))
                              return(keysym - XK_KP_Multiply + XK_asterisk);
                          else
                              return(NO_USE_KEY);
     }
}

/******************************************************************************/
/* FUNCTION    : maptonextmode                                                */
/* DESCRIPTION : using keysym/state, determines the next mode, uses round     */
/*                    robin approach.					      */
/* INPUT       : mode, keysym, state                                          */
/* OUTPUT      : keysym                                                       */
/******************************************************************************/

static unsigned int maptonextmode(mode, keysym, state)
int		  mode;		  /* IM Mode				      */
unsigned int      keysym;         /* Interpretation Of A Key Press            */
unsigned int      state;          /* Status Of Key                            */
{
      switch(mode)
      {
          case PROCESSOFF         : return(keysym);
          case ALPH_NUM_MODE      : return(XK_Phonetic);
          case PHONETIC_MODE      : return(XK_Tsang_Jye);
          case TSANG_JYE_MODE     : return(XK_Internal_Code);
          case INTERNAL_CODE_MODE : return(XK_Alph_Num);
       }
   }

/******************************************************************************/
/* FUNCTION    : maptobase                                                    */
/* DESCRIPTION : map keysym to suitable keysym                                */
/* INPUT       : immap, keysym, state                                         */
/* OUTPUT      : keysym                                                       */
/******************************************************************************/
static unsigned int     maptobase(IMKeymap *immap, unsigned int keysym,
        unsigned int state, FEPCB *fepcb)
{
	int mode;
        _IMMapKeysym(immap, &keysym, &state);
        if (state) {
                keysym = XK_VoidSymbol;
		return keysym;
	}
	mode = get_state(fepcb);

	if ( keysym == XK_ZH_Switch )
		keysym = maptonextmode( mode, keysym, state);

	else if ( mode == ALPH_NUM_MODE ) {

		if ( keysym == XK_ZH_Toggle )
		switch(fepcb->prev_mode) {
		  case PHONETIC_MODE      : keysym = XK_Phonetic; break;
		  case TSANG_JYE_MODE     : keysym = XK_Tsang_Jye; break;
		  case INTERNAL_CODE_MODE : keysym = XK_Internal_Code; break;
		}
	}
	else if ( keysym == XK_space ) /* && mode != ALPH_NUM_MODE */
		keysym = XK_Convert;

	else if ( keysym == XK_less )  /* && mode != ALPH_NUM_MODE */
		keysym = XK_Non_Convert;

	else if ( keysym == XK_ZH_Toggle ) /* && mode != ALPH_NUM_MODE */
		keysym = XK_Alph_Num;

        return keysym;
}

/******************************************************************************/
/* FUNCTION    : TedFilter                                                    */
/* DESCRIPTION : Entry Point of Chinese Input Method Editor                   */
/* EXTERNAL REFERENCES :                                                      */
/* INPUT       : fepcb, keymap, keysym, state, imb                            */
/* OUTPUT      : IMED_USED                                                    */
/*               IMED_NOTUSED                                                 */
/*               ERROR                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int TedFilter(fepcb, keymap, keysym, state, imb)
FEPCB             *fepcb;         /* FEP Control Block                        */
IMKeymap          *keymap;        /* Symbols Of Keyboard Mapping              */
unsigned int      keysym;         /* Interpretation Of A Key Press            */
unsigned int      state;          /* Status Of Key                            */
IMBuffer          *imb;
{
   unsigned char  key;

   fepcb->echochfg.flag = OFF ;          /*  Clear all flag   */
   fepcb->auxchfg  = OFF ;
   fepcb->indchfg  = OFF ;
   fepcb->eccrpsch = OFF ;
   fepcb->auxcrpsch = OFF ;
   fepcb->isbeep = OFF;
   fepcb->ret = IMED_USED;

   if ( fepcb->imode.ind5 != BLANK )       /* Erase error message            */
   {
      fepcb->imode.ind5 = BLANK;
      fepcb->indchfg = ON;
   }
                                           /* convert to suitable keysym  */
   keysym = maptobase(keymap, keysym, state, fepcb);
   key = mapkeysymtokey(keysym,fepcb);        /* @big5 */
               /*  the key maybe use and send it to suitable state  */
   if (AltKeypadProcess(fepcb, keysym, state))  /* Alt+Keypad Process */
      fepcb->ret = IMED_USED;
   else
   {
      if (fepcb->edendacsz != 0)
          makeoutputbuffer(fepcb, imb);
      if (IsModifierKey(keysym))
          return(IMED_NOTUSED);

      switch(get_state(fepcb))
      {
          case PROCESSOFF :
             fepcb->ret = IMED_NOTUSED;
             break;

          case ALPH_NUM_MODE :
             AnMain(fepcb, key);
             break;

         case PHONETIC_MODE :
             if (iskeypadkeysym(keysym) &&
                   (fepcb->imode.ind2 != PHONETIC_SELECT_ON))
             {
                 if (fepcb->echoacsz != 0)
                 {
                    key = NO_USE_KEY;
                    PhMain(fepcb, key);
                 }
                 else
                    AnMain(fepcb, key);
             }
             else
                 PhMain(fepcb, key);
             break;

          case TSANG_JYE_MODE :
             TjMain(fepcb, key);
             break;

          case INTERNAL_CODE_MODE :
             IcMain(fepcb, key);
             break;
       }
   }


   if (fepcb->edendacsz != 0)         /*  copy data to imobj->output buffer */
   {
      makeoutputbuffer(fepcb, imb);
   }

   if (fepcb->ret == IMED_NOTUSED)     /*  return to TimProcess or TimFilter */
   {
       return(IMED_NOTUSED);
   }
   else if (fepcb->ret == ERROR)
            return(ERROR);
        else
            return(IMED_USED);

}

/******************************************************************************/
/* FUNCTION    : makeoutputbuffer                                             */
/* DESCRIPTION : copy fixed buffer to output buffer                           */
/* INPUT       : keysym, state                                                */
/* OUTPUT      : fepcb                                                        */
/******************************************************************************/
makeoutputbuffer(fepcb, imb)
FEPCB *fepcb;
IMBuffer *imb;
{

   if ((get_state(fepcb) == TSANG_JYE_MODE) && (fepcb->inputlen == 0) &&
                         (fepcb->echoacsz != 0))
       TjEraseAllRadical(fepcb);     /* echo buffer on window    */
   memcpy(&imb->data[imb->len], fepcb->edendbuf, fepcb->edendacsz);
   imb->len += fepcb->edendacsz;
   memset(fepcb->edendbuf, NULL, fepcb->edendacsz);
   fepcb->edendacsz = 0;
   return;
}

/******************************************************************************/
/* FUNCTION    : AnMain                                                       */
/* DESCRIPTION : Entry of AlphaNumerical Input Method                         */
/* EXTERNAL REFERENCES :                                                      */
/* INPUT       : fepcb = FEP control block                                    */
/*               key =  the pseudo code of stroke key                         */
/* OUTPUT      : fepcb or NONE                                                */
/* CALLED      : TedProcess                                                   */
/* CALL        :                                                              */
/******************************************************************************/
AnMain(fepcb, key)
FEPCB     *fepcb ;                           /* FEP control block             */
unsigned char key;                           /* the pseudo code of stroke key */
{
   switch(key)
   {
      case ALPH_NUM_KEY :                    /* nothing would do              */
           break;

      case PHONETIC_KEY:
           PhInitial(fepcb);                 /* change to phonetic initial    */
           break;                            /* status                        */

      case TSANG_JYE_KEY :
           TjInitial(fepcb);                 /* change to Tsang-Jye initial   */
           break;                            /* status                        */

      case INTERNAL_CODE_KEY :
           IcInitial(fepcb);
           break;

      case FULL_HALF_KEY :
           if (fepcb->imode.ind1 == HALF)    /* if indicator is HALF then     */
              fepcb->imode.ind1 = FULL;      /* set to be FULL mode           */
           else
              fepcb->imode.ind1 = HALF;      /* if indicator is FULL then     */
           fepcb->indchfg = ON;              /* set to be HALF mode           */
           break;

      default :
           AnProcess(fepcb, key);            /* Process key stroke for        */
           break;                            /* AlphNumerical Input Method    */
   }
}

/******************************************************************************/
/* FUNCTION    : AnInitial                                                    */
/* DESCRIPTION : Initialize Tsang Jye Input Method Environment                */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/*                                                                            */
/******************************************************************************/
AnInitial(fepcb)
FEPCB *fepcb;                       /* FEP Control Block                      */
{
   if( fepcb->imode.ind0 != ALPH_NUM_MODE )
   {
      fepcb->prev_mode = get_state(fepcb);
      fepcb->indchfg = ON ;         /* set the indicator would be changed     */
      fepcb->imode.ind0 = ALPH_NUM_MODE;  /* set input mode to be ALPH_NUM    */
   }
   fepcb->imode.ind2 = SELECT_OFF;
   TjEraseAllRadical(fepcb);
}

/******************************************************************************/
/* FUNCTION    : get_state                                                    */
/* DESCRIPTION : This routine determines the current internal processing      */
/*               state                                                        */
/* INPUT       : fepcb  = FEP control block                                   */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int     get_state( fepcb )
FEPCB   *fepcb ;                      /* FEP control block                    */
{
   if (fepcb->imode.ind3 != NORMAL_MODE)
      return(PROCESSOFF);             /* input mode is suppressed mode        */
   if(fepcb->imode.ind0 == PHONETIC_MODE)
      return(PHONETIC_MODE);          /* input mode is phonetic mode          */
   if(fepcb->imode.ind0 == TSANG_JYE_MODE)
      return (TSANG_JYE_MODE);        /* input mode is Tsang-Tje mode         */
   if(fepcb->imode.ind0 == INTERNAL_CODE_MODE)
      return(INTERNAL_CODE_MODE);
   else
      return (ALPH_NUM_MODE);         /* input mode is alphabtic numeric mode */
}

/*-----------------------------------------------------------------------*/
/* definition of euc code of printable characters from 0x20 to 0x7e      */
/*-----------------------------------------------------------------------*/
unsigned short full[] = {
/* blank ! " # $ % & ' ( ) * + , - . / */
   0xa1a1, 0xa1aa, 0xa1e7, 0xa1ec, 0xa2e3, 0xa2e8, 0xa1ed, 0xa1e5, 0xa1be,
   0xa1bf, 0xa1ee, 0xa2b0, 0xa1a2, 0xa2b1, 0xa1a6, 0xa2df,

/*   0 1 2 3 4 5 6 7 8 9    */
   0xa4a1,0xa4a2,0xa4a3,0xa4a4,0xa4a5,0xa4a6,0xa4a7,0xa4a8,0xa4a9,0xa4aa,

/* : ; < = > ? @ */
   0xa1a8, 0xa1a7, 0xa2b6, 0xa2b8, 0xa2b7, 0xa1a9, 0xa2e9,

/* A B C D E F G H I J K L M N O P Q R S T U V W X Y Z */
   0xa4c1,0xa4c2,0xa4c3,0xa4c4,0xa4c5,0xa4c6,0xa4c7,0xa4c8,0xa4c9,0xa4ca,
   0xa4cb,0xa4cc,0xa4cd,0xa4ce,0xa4cf,0xa4d0,0xa4d1,0xa4d2,0xa4d3,0xa4d4,
   0xa4d5,0xa4d6,0xa4d7,0xa4d8,0xa4d9,0xa4da,

/* [ \ ] ^ _ ` */
   0xa1c6, 0xa2e0, 0xa1c7, 0xffff, 0xa2a5, 0xa1ea,
/* a b c d e f g h i j k l m n o p q r s t u v w x y z */
   0xa4db,0xa4dc,0xa4dd,0xa4de,0xa4df,0xa4e0,0xa4e1,0xa4e2,0xa4e3,0xa4e4,
   0xa4e5,0xa4e6,0xa4e7,0xa4e8,0xa4e9,0xa4ea,0xa4eb,0xa4ec,0xa4ed,0xa4ee,
   0xa4ef,0xa4f0,0xa4f1,0xa4f2,0xa4f3,0xa4f4,

/* { | } ~ */
   0xa1c2, 0xa2de, 0xa1c3, 0xa2c4, };

/******************************************************************************/
/* FUNCTION    : AnProcess                                                    */
/* DESCRIPTION : Process each key stroke data for AlphaNumerical Input Method */
/* INPUT       : fepcb =                                                      */
/*               key =                                                        */
/* OUTPUT      : fepcb =                                                      */
/******************************************************************************/
AnProcess(fepcb, key)
FEPCB         *fepcb ;     /* FEP control block                               */
unsigned char key;         /* pseudo code of stroke key                       */
{
   unsigned char   code[4];            /* @big5                              */
   unsigned char   euccode[4];         /* @big5                              */
   size_t          in_count;           /* @big5                              */
   size_t          out_count=4;        /* @big5                              */

   if ( (fepcb->imode.ind1 == FULL)
       && (key >= 0x20 && key <= 0x7e))
   {
      if (key == 0x5e)
      {
         static char special_char[4]={0x8e ,0xad ,0xa1 ,0xa4};

         memcpy(fepcb->edendbuf, special_char, 4);
         fepcb->edendacsz = 4;
         if (fepcb->Lang_Type != codesetinfo[0].code_type)          /* @big5*/
         {                                                          /* @big5*/
           in_count = 4;                                            /* @big5*/
           out_count = 4;                                           /* @big5*/
           StrCodeConvert(fepcb->iconv_flag,
                          fepcb->edendbuf,code,
                          &in_count,&out_count);                    /* @big5*/
           fepcb->edendacsz = 4-out_count;                          /* @big5*/
           memcpy(fepcb->edendbuf, code,fepcb->edendacsz);          /* @big5*/
         }                                                          /* @big5*/
      }
      else
      {
         char str[2];

         str[0] = full[key - 0x20] / 256;     /*  conver sbcs to dbcs      */
         str[1] = full[key - 0x20] % 256;
         memcpy(fepcb->edendbuf, str, 2);
         fepcb->edendacsz = 2;
         if (fepcb->Lang_Type != codesetinfo[0].code_type)          /* @big5*/
         {                                                          /* @big5*/
           in_count = 2;                                            /* @big5*/
           out_count = 2;                                           /* @big5*/
           StrCodeConvert(fepcb->iconv_flag,
                          fepcb->edendbuf,code,
                          &in_count,&out_count);                    /* @big5*/
           fepcb->edendacsz = 2-out_count;                          /* @big5*/
           memcpy(fepcb->edendbuf, code,fepcb->edendacsz);          /* @big5*/
         }                                                          /* @big5*/
      }
      fepcb->ret = IMED_USED;
   }
   else
      fepcb->ret = IMED_NOTUSED;
   return;
}

/******************************************************************************/
/* FUNCTION    : AltKeypadProcess                                             */
/* DESCRIPTION : Process alt+keypad key stroke for Internal Code Input Method */
/* INPUT       : fepcb, keysym, state                                         */
/* OUTPUT      : fepcb                                                        */
/******************************************************************************/
AltKeypadProcess(fepcb, keysym, state)
FEPCB         *fepcb;         /* FEP Control Block                        */
unsigned int  keysym;
unsigned int  state;
{
   int ret;
   static int     cnt1=0,cnt2=0;  /* alt+keypad internal code input method    */
   static unsigned char icbuf1[3]={'\0','\0','\0'};
   static unsigned char icbuf2[5]={'\0','\0','\0','\0','\0'};

   ret = FALSE;
   if ((state == ALT_STATE) && isspnumkeysym(keysym))
   {
      if ( (fepcb->echoacsz == 0) && (fepcb->inputlen == 0) )
      {
         if (cnt1<3)                                     /*  3 radical    */
            icbuf1[cnt1++]=keysym - XK_KP_0 + 0x30;
         if (cnt1 == 3)
         {
            icbuf2[cnt2++] = atoi(icbuf1)%256;
            memset(icbuf1, NULL, cnt1);
            cnt1 = 0;
/*          if (IcIsEucCodeRange(icbuf2) || (cnt2 == 2 && icbuf2[0] != 0x8E)
              || (cnt2 == 1 && icbuf2[0] == 0x00)  ||
                 (cnt2 == 4 && icbuf2[0] == 0x8E))      / *  check range  @big5 */

            if  ( (IcIsCodeRange(icbuf2,fepcb->Lang_Type)) ||
                  ((fepcb->Lang_Type == codesetinfo[0].code_type) &&
                   ((cnt2 == 2 && icbuf2[0] != 0x8E) ||
                    (cnt2 == 1 && icbuf2[0] == 0x00) ||
                    (cnt2 == 4 && icbuf2[0] == 0x8E)   )             ) ||
                  ((fepcb->Lang_Type != codesetinfo[0].code_type) &&
                   ((cnt2 == 2 )                     ||
                    (cnt2 == 1 && icbuf2[0] == 0x00)   )             ) ) /*@big5*/
            {
               strcpy(fepcb->edendbuf, icbuf2, cnt2);
               fepcb->edendacsz = cnt2;
               memset(icbuf2, NULL, cnt2);
               cnt2 = 0;
            }
         }
         ret = TRUE;
      }
   }
   else if ((cnt1 != 0) || (cnt2 != 0))
        {
           if (cnt1 != 0)
           {
              icbuf2[cnt2++] = atoi(icbuf1)%256;
              memset(icbuf1, NULL, cnt1);
              cnt1 = 0;
           }
/*         if (IcIsEucCodeRange(icbuf2) || (cnt2 == 2 && icbuf2[0] != 0x8e)
               || (cnt2 == 1 && icbuf2[0] == 0x00)  ||
               (cnt2 == 4 && icbuf2[0] == 0x8E))                 @big5 */
           if  ( (IcIsCodeRange(icbuf2,fepcb->Lang_Type)) ||
                 ((fepcb->Lang_Type == codesetinfo[0].code_type) &&
                  ((cnt2 == 2 && icbuf2[0] != 0x8E) ||
                   (cnt2 == 1 && icbuf2[0] == 0x00) ||
                   (cnt2 == 4 && icbuf2[0] == 0x8E)   )             ) ||
                 ((fepcb->Lang_Type != codesetinfo[0].code_type) &&
                  ((cnt2 == 2 )                     ||
                   (cnt2 == 1 && icbuf2[0] == 0x00)   )             ) ) /*@big5*/
           {
              strcpy(fepcb->edendbuf, icbuf2, cnt2);
              fepcb->edendacsz = cnt2;
              memset(icbuf1, NULL, cnt1);
              memset(icbuf2, NULL, cnt2);
              cnt1 = 0;
              cnt2 = 0;
           }
           else
           {
              if ((keysym != XK_Alt_R) && ((cnt2 == 1) || (cnt2 == 3)))
              {
                 memset(icbuf1, NULL, cnt1);
                 memset(icbuf2, NULL, cnt2);
                 cnt1 = 0;
                 cnt2 = 0;
              }
           }
        }
    return (ret);
}

/******************************************************************************/
/* FUNCTION    : TedLookup                                                    */
/* DESCRIPTION : find keysym and copy it to output buffer                     */
/* INPUT       : fepcb, keymap, keysym, state, imb                            */
/* OUTPUT      :                                                              */
/******************************************************************************/
void    TedLookup(fepcb, keymap, keysym, state, imb)
FEPCB    *fepcb;
IMKeymap *keymap;
unsigned int keysym;
unsigned int state;
IMBuffer *imb;
{
     _IMMapKeysym(keymap, &keysym, &state);
     _IMSimpleMapping(keymap, keysym, state, imb);
     return;
}
