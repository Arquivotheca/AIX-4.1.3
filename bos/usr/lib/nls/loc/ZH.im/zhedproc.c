static char sccsid[] = "@(#)61	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedproc.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:53";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AltKeypadProcess
 *		AnInitial
 *		AnMain
 *		AnProcess
 *		IsAltNumKey
 *		ZHedFilter
 *		ZHedLookup
 *		get_state
 *		isasciikeysym
 *		iskeypadkeysym
 *		isspnumkeysym
 *		makeoutputbuffer
 *		mapkeysymtokey
 *		maptobase
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
/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         ZHedProc                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Process source file       */
/*                                                                            */
/* FUNCTION:            ZHedFilter      : Chinese Input Method Editor Filter  */
/*                                                                            */
/*                      ZHedLookup      : Find & Copy Keysym to Output Buffer */
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
/************************ END OF SPECIFICATION ********************************/

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "chinese.h"
#include "zhedud.h"


#define iskeypadkeysym(k)       (XK_KP_Multiply <= (k) && (k) <= XK_KP_9)
#define isspnumkeysym(k)        (XK_KP_0 <= (k) && (k) <= XK_KP_9)
#define isasciikeysym(k)        (XK_space <= (k) && (k) <= XK_asciitilde)
#define XK_Alph_Num             0xaff47
#define XK_Pinyin               0xaff49
#define XK_English_Chinese      0xaff50
#define XK_ABC_Code             0xaff51
#define XK_Tsang_Jye            0xaff52
#define XK_Full_Half            0xaff53
#define XK_Convert              0xaff54
#define XK_Legend               0xaff55
#define XK_User_Defined         0xaff56
#define XK_Ctrl_Right           0xaff57
#define XK_Ctrl_Home            0xaff58
#define XK_Non_Convert          0xaff59
#define XK_IMED_Set_Option      0xaff60


/******************************************************************************/
/* FUNCTION    : mapkeysymtokey                                               */
/* DESCRIPTION : convert keysym to key                                        */
/* INPUT       : keysym, state                                                */
/* OUTPUT      : key                                                          */
/******************************************************************************/
static unsigned char mapkeysymtokey(keysym)
unsigned int keysym;
{
     switch( keysym )
     {
        case XK_Alph_Num        : return(ALPHA_NUM_KEY);
                                  break;
        case XK_Tsang_Jye       : return(TSANG_JYE_KEY);
                                  break;
        case XK_Pinyin          : return(PINYIN_KEY);
                                  break;
        case XK_Legend          : return(LEGEND_SWITCH_KEY);
                                  break;
        case XK_English_Chinese : return(ENGLISH_CHINESE_KEY);
                                  break;
        case XK_ABC_Code        : return(ABC_KEY);
                                  break;
        case XK_User_Defined    : return(USER_DEFINED_KEY);
                                  break;
        case XK_Full_Half       : return(FULL_HALF_KEY);
                                  break;
        case XK_Convert         : return(CONVERT_KEY);
                                  break;
        case XK_Non_Convert     : return(NON_CONVERT_KEY);
                                  break;
        case XK_IMED_Set_Option : return(IMED_SET_OPTION_KEY);
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
        case XK_End             : return(END_KEY);
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
/* FUNCTION    : maptobase                                                    */
/* DESCRIPTION : map keysym to suitable keysym                                */
/* INPUT       : immap, keysym, state                                         */
/* OUTPUT      : keysym                                                       */
/******************************************************************************/
static unsigned int     maptobase(IMKeymap *immap, unsigned int keysym,
        unsigned int state)
{
        _IMMapKeysym(immap, &keysym, &state);
        if (state && (state != ALT_STATE) && (state != 4))
                keysym = XK_VoidSymbol;
        return keysym;
}

/******************************************************************************/
/* FUNCTION    : ZHedFilter                                                   */
/* DESCRIPTION : Entry Point of Chinese Input Method Editor                   */
/* EXTERNAL REFERENCES :                                                      */
/* INPUT       : fepcb, keymap, keysym, state, imb                            */
/* OUTPUT      : IMED_USED                                                    */
/*               IMED_NOTUSED                                                 */
/*               ERROR                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int ZHedFilter(fepcb, keymap, keysym, state, imb)
FEPCB             *fepcb;         /* FEP Control Block                        */
IMKeymap          *keymap;        /* Symbols Of Keyboard Mapping              */
unsigned int      keysym;         /* Interpretation Of A Key Press            */
unsigned int      state;          /* Status Of Key                            */
IMBuffer          *imb;
{
   unsigned short  key;


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
   keysym = maptobase(keymap, keysym, state);

   if (IsAltNumKey(keysym, state))
      key = keysym | 0xFF00;
   else
      key = mapkeysymtokey(keysym);
               /*  the key maybe use and send it to suitable state  */
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

          case TSANG_JYE_MODE :
             TjMain(fepcb, key); 
             break;

          case PINYIN_MODE :
             PyMain(fepcb, key);
             break;

          case ENGLISH_CHINESE_MODE :
             EnMain(fepcb, key);
             break;

          case ABC_MODE :
             AbcMain(fepcb, key);  
             break;

          case USER_DEFINED_MODE :
             (*udimcomm->UserDefinedMain)(udimcomm, key);
             break;
       }

   if (fepcb->edendacsz != 0)         /*  copy data to imobj->output buffer */
      makeoutputbuffer(fepcb, imb);

   if (fepcb->ret == IMED_NOTUSED)    /*  return to ZHimProcess or ZHimFilter */
       return(IMED_NOTUSED);
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
/* CALLED      : ZHedProcess                                                  */
/* CALL        :                                                              */
/******************************************************************************/
AnMain(fepcb, key)
FEPCB     *fepcb ;                           /* FEP control block             */
unsigned char key;                           /* the pseudo code of stroke key */
{
   switch(key)
   {
      case ALPHA_NUM_KEY :                   /* nothing would do              */
           break;

      case TSANG_JYE_KEY :                   /* Change to TSANG_JYE initial  */
           TjInitial(fepcb);                 /* status                        */
           break;

      case PINYIN_KEY:
           PyInitial(fepcb);                 /* change to pinyin initial      */
           break;                            /* status                        */

      case ENGLISH_CHINESE_KEY :
           EnInitial(fepcb);                 /* change to english_chinese     */
           break;                            /*  initial status               */

      case ABC_KEY :       
           AbcInitial(fepcb);                /* Change to Abc initial         */
           break;                            /* status                        */

      case USER_DEFINED_KEY :  
           if(udimcomm->ret == UD_FOUND)
             (*udimcomm->UserDefinedInitial)(udimcomm);
                                             /* Change to User Defined initial*/
                                             /* status                        */
           else
             return;
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
/* DESCRIPTION : Initialize Alph_Num_Code Input Method Environment            */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/*                                                                            */
/******************************************************************************/
AnInitial(fepcb)
FEPCB *fepcb;                       /* FEP Control Block                      */
{
   if( fepcb->imode.ind0 != ALPH_NUM_MODE )
   {
      fepcb->indchfg = ON ;         /* set the indicator would be changed     */
      fepcb->imode.ind0 = ALPH_NUM_MODE;  /* set input mode to be ALPH_NUM    */
   }
   fepcb->imode.ind2 = SELECT_OFF;
     PyEraseAllRadical(fepcb);
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
   if(fepcb->imode.ind0 == TSANG_JYE_MODE)
      return(TSANG_JYE_MODE);
   if(fepcb->imode.ind0 == PINYIN_MODE)
      return(PINYIN_MODE);            /* input mode is pinyin mode            */
   if(fepcb->imode.ind0 == ENGLISH_CHINESE_MODE)
      return (ENGLISH_CHINESE_MODE);  /* input mode is english_chinese mode   */
   if(fepcb->imode.ind0 ==ABC_MODE)
      return(ABC_MODE);               /* input mode is abc mode               */
   if(fepcb->imode.ind0 == USER_DEFINED_MODE)
      return(USER_DEFINED_MODE);      /* input mode is user defined mode      */
   else
      return (ALPH_NUM_MODE);         /* input mode is alphabtic numeric mode */
}

/*-----------------------------------------------------------------------*/
/* definition of euc code of printable characters from 0x20 to 0x7e      */
/*-----------------------------------------------------------------------*/
unsigned int full[] = {
/* blank    !        "        #        $        %        &        '      */
   0xe38080,0xefbc81,0xefbc82,0xefbc83,0xefbc84,0xefbc85,0xefbc86,0xefbc87,
/* (        )        *        +        ,        -        .        /      */
   0xefbc88,0xefbc89,0xefbc8a,0xefbc8b,0xefbc8c,0xefbc8d,0xefbc8e,0xefbc8f,
/* 0        1        2        3        4        5        6        7      */
   0xefbc90,0xefbc91,0xefbc92,0xefbc93,0xefbc94,0xefbc95,0xefbc96,0xefbc97,
/* 8        9        :        ;        <        =        >        ?      */
   0xefbc98,0xefbc99,0xefbc9a,0xefbc9b,0xefbc9c,0xefbc9d,0xefbc9e,0xefbc9f,
/* @        A        B        C	       D        E        F        G      */
   0xefbca0,0xefbca1,0xefbca2,0xefbca3,0xefbca4,0xefbca5,0xefbca6,0xefbca7,
/* H        I        J        K        L        M        N        O      */
   0xefbca8,0xefbca9,0xefbcaa,0xefbcab,0xefbcac,0xefbcad,0xefbcae,0xefbcaf,
/* P        Q        R        S        T        U        V        W      */
   0xefbcb0,0xefbcb1,0xefbcb2,0xefbcb3,0xefbcb4,0xefbcb5,0xefbcb6,0xefbcb7,
/* X        Y        Z        [        \        ]        ^        _      */
   0xefbcb8,0xefbcb9,0xefbcba,0xefbcbb,0xefbcbc,0xefbcbd,0xefbcbe,0xefbcbf,
/* `        a        b        c        d        e        f        g      */
   0xefbd80,0xefbd81,0xefbd82,0xefbd83,0xefbd84,0xefbd85,0xefbd86,0xefbd87,
/* h        i        j        k        l        m        n        o      */
   0xefbd88,0xefbd89,0xefbd8a,0xefbd8b,0xefbd8c,0xefbd8d,0xefbd8e,0xefbd8f,
/* p        q        r        s        t        u        v        w      */
   0xefbd90,0xefbd91,0xefbd92,0xefbd93,0xefbd94,0xefbd95,0xefbd96,0xefbd97,
/* x        y        z        {        |        }        ~               */
   0xefbd98,0xefbd99,0xefbd9a,0xefbd9b,0xefbd9c,0xefbd9d,0xefbfa300 };


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
   unsigned int *por;

   if ( (fepcb->imode.ind1 == FULL)
       && (key >= 0x20 && key <= 0x7e))
   {
      char str[3];

     /* conver sbcs to dbcs*/      
      por = &full[key - 0x20];
      str[0] = *((unsigned char *)por + 1);
      str[1] = *((unsigned char *)por + 2);
      str[2] = *((unsigned char *)por + 3);
      memcpy(fepcb->edendbuf, str, 3);
      fepcb->edendacsz = 3;
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
   static unsigned char rcbuf1[3]={'\0','\0','\0'};
   static unsigned char rcbuf2[4]={'\0','\0','\0','\0'};

   ret = FALSE;
   if ((state == ALT_STATE) && isspnumkeysym(keysym))
   {
      if ( (fepcb->echoacsz == 0) && (fepcb->inputlen == 0) )
      {
         if (cnt1<3)                                     /*  3 radical    */
            rcbuf1[cnt1++]=keysym - XK_KP_0 + 0x30;
         if (cnt1 == 3)
         {
            rcbuf2[cnt2++] = atoi(rcbuf1)%256;
            memset(rcbuf1, NULL, cnt1);
            cnt1 = 0;
 /*           if (RcIsEucCodeRange(rcbuf2) || (cnt2 == 2 && rcbuf2[0] != 0x8E)
              || (cnt2 == 1 && rcbuf2[0] == 0x00)  ||
                 (cnt2 == 4 && rcbuf2[0] == 0x8E))     check range  
            {
               strcpy(fepcb->edendbuf, rcbuf2, cnt2);
               fepcb->edendacsz = cnt2;
               memset(rcbuf2, NULL, cnt2);
               cnt2 = 0;
            }*/
         }
         ret = TRUE;
      }
   }
   else if ((cnt1 != 0) || (cnt2 != 0))
        {
           if (cnt1 != 0)
           {
              rcbuf2[cnt2++] = atoi(rcbuf1)%256;
              memset(rcbuf1, NULL, cnt1);
              cnt1 = 0;
           }
/*
           if (RcIsEucCodeRange(rcbuf2) || (cnt2 == 2 && rcbuf2[0] != 0x8e)
               || (cnt2 == 1 && rcbuf2[0] == 0x00)  ||
               (cnt2 == 4 && rcbuf2[0] == 0x8E))
           {
              strcpy(fepcb->edendbuf, rcbuf2, cnt2);
              fepcb->edendacsz = cnt2;
              memset(rcbuf1, NULL, cnt1);
              memset(rcbuf2, NULL, cnt2);
              cnt1 = 0;
              cnt2 = 0;
           }
           else
           {
              if ((keysym != XK_Alt_R) && ((cnt2 == 1) || (cnt2 == 3)))
              {
                 memset(rcbuf1, NULL, cnt1);
                 memset(rcbuf2, NULL, cnt2);
                 cnt1 = 0;
                 cnt2 = 0;
              }
           }
*/
        }
    return (ret);
}

/******************************************************************************/
/* FUNCTION    : IsAltNumKey                                                  */
/* DESCRIPTION : Process alt+numeric key stroke for Duplicate Re_Select       */
/* INPUT       : keysym, state                                                */
/* OUTPUT      : TRUE or FLASE                                                */
/******************************************************************************/
IsAltNumKey(keysym,state)
unsigned int      keysym;         /* Interpretation Of A Key Press            */
unsigned int      state;          /* Status Of Key                            */
{
   if ((state == ALT_STATE) && (0x30<=(keysym) && (keysym)<=0x39))
      return(TRUE);
   else
      return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : ZHedLookup                                                   */
/* DESCRIPTION : find keysym and copy it to output buffer                     */
/* INPUT       : fepcb, keymap, keysym, state, imb                            */
/* OUTPUT      :                                                              */
/******************************************************************************/
void    ZHedLookup(fepcb, keymap, keysym, state, imb)
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
