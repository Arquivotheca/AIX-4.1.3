static char sccsid[] = "@(#)60	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedmisc.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:51";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHedGetAuxCurPos
 *		ZHedGetAuxSize
 *		ZHedGetAuxType
 *		ZHedGetEchoBufLen
 *		ZHedGetEchoCurPos
 *		ZHedGetFixBufLen
 *		ZHedGetIndLegendL
 *		ZHedGetIndLegendS
 *		ZHedGetIndMessage
 *		ZHedGetInputMode
 *		ZHedIsAuxBufChanged
 *		ZHedIsAuxBufUsedNow
 *		ZHedIsAuxCurPosChanged
 *		ZHedIsBeepRequested
 *		ZHedIsEchoBufChanged
 *		ZHedIsEchoCurPosChanged
 *		ZHedIsInputModeChanged
 *		ZHedSetInputMode
 *		ZHedSetWarning
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
/******************** START OF MODULE SPECIFICATIONS ********************/
/*                                                                      */
/* MODULE NAME:         ZHedMisc.c                                      */
/*                                                                      */
/* DESCRIPTIVE NAME:    Chinese Input Method Miscelaneous               */
/*                                                                      */
/* FUNCTION:                                                            */
/*                      ZHedGetEchoBufLen : Return Echo Buffer Length.  */
/*                                                                      */
/*                      ZHedGetFixBufLen : Return Fix Buffer Length.    */
/*                                                                      */
/*                      ZHedGetAuxSize : Return Aux Active size.        */
/*                                                                      */
/*                      ZHedIsEchoBufChanged : Return Echo Buffer       */
/*                                            Change Flag.              */
/*                                                                      */
/*                      ZHedIsAuxBufChanged : Return Aux Buffer         */
/*                                           Change Flag.               */
/*                                                                      */
/*                      ZHedIsInputModeChanged : Return Input Mode      */
/*                                              Change Flag Structure.  */
/*                                                                      */
/*                      ZHedIsCurPosChanged : Return Cursor Position    */
/*                                           Change Flag.               */
/*                                                                      */
/*                      ZHedIsAuxCurPosChanged : Return Aux Cursor      */
/*                                              Position Change Flag.   */
/*                                                                      */
/*                      ZHedIsBeepRequested : Return Beep Requested     */
/*                                           Flag.                      */
/*                                                                      */
/*                      ZHedGetEchoCurPos : Return Echo Buffer Cursor   */
/*                                         Position.                    */
/*                                                                      */
/*                      ZHedGetAuxCurPos : Return Aux Cursor Posstion.  */
/*                                                                      */
/*                      ZHedGetAuxType : Return Aux Type.               */
/*                                                                      */
/*                      ZHedGetInputMode : Return Input Mode Structure. */
/*                                                                      */
/*                      ZHedSetInputMode : Set Input Mode Structure.    */
/*                                                                      */
/*                      ZHedGetIndMessage : Return Indicator Message.   */
/*                                                                      */
/*                      ZHedGetIndLegend : Return Indicator Legend.     */
/*                                                                      */
/*                      ZHedIsAuxBufUsedNow : Return Aux Buffer Used    */
/*                                           Flag.                      */
/*                                                                      */
/*                      ZHedSetWarning : Reset Beep sound and error     */
/*                                      msg to user                     */
/*                                                                      */
/*                                                                      */
/******************** END OF SPECIFICATIONS *****************************/
#include "chinese.h"
#include "zhedacc.h"

int     ZHedGetEchoBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->echoacsz);
}

int     ZHedGetFixBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->edendacsz);
}

AuxSize ZHedGetAuxSize(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->auxacsz);
}

EchoBufChanged   ZHedIsEchoBufChanged(fepcb)
FEPCB   *fepcb;
{
   return(fepcb->echochfg);
}

int     ZHedIsAuxBufChanged(fepcb)
FEPCB  *fepcb ;
{
   if ( fepcb->auxchfg )
      return(TRUE);
   else
      return(FALSE);
}

int      ZHedIsInputModeChanged(fepcb)
FEPCB   *fepcb;
{
   return( fepcb->indchfg );
}

int      ZHedIsEchoCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
    if ( fepcb->eccrpsch )
       return( TRUE );
    else
       return( FALSE );
}

int     ZHedIsAuxCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxcrpsch )
      return( TRUE );
   else
      return( FALSE );
}

int ZHedIsBeepRequested(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->beepallowed == ON )
   {
      if ( fepcb->isbeep )
         return( TRUE );
       else
         return( FALSE);
   }
   else /* requesting beep is not allowed */
       return( FALSE );
}

int     ZHedGetEchoCurPos(fepcb)
FEPCB  *fepcb ;
{
   return( fepcb->echocrps );
}

AuxCurPos   ZHedGetAuxCurPos(fepcb)
FEPCB      *fepcb ;
{
   return( fepcb->auxcrps );
}

int     ZHedGetAuxType(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxuse == USE )
      return ( Aux_Candidate_List );
   else
      return ( Aux_No_Used );
}

InputMode  ZHedGetInputMode(fepcb)
FEPCB     *fepcb ;
{
   return(fepcb->imode);
}

int      ZHedSetInputMode(fepcb,imode)
FEPCB   *fepcb ;
InputMode       imode;
{
   extern  void    set_imode(), set_indicator();

   /***************************************************/
   /* first, check if all fields contain valid values */
   /***************************************************/

   switch ( imode.ind0 )
   {
      case ALPH_NUM_MODE :
      case TSANG_JYE_MODE :
      case PINYIN_MODE :
      case ENGLISH_CHINESE_MODE :
      case ABC_MODE :
      case USER_DEFINED_MODE:
      case CAPS_LOCK_MODE :
             break ;
      default :
             return (ERROR) ;
   }

   switch ( imode.ind1 )
   {
      case HALF :
      case FULL :
             break ;
      default :
             return ( ERROR ) ;
   }

   switch ( imode.ind4 )
   {
      case INSERT_MODE :
      case REPLACE_MODE :
             break ;
      default :
             return( ERROR );
   }

   switch ( imode.ind0 )
   {
      case ALPH_NUM_MODE :
             if ( fepcb->imode.ind0 != ALPH_NUM_MODE )
             {
                fepcb->imode.ind0 = ALPH_NUM_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case TSANG_JYE_MODE :
             if ( fepcb->imode.ind0 != TSANG_JYE_MODE)
             {
                fepcb->imode.ind0 = TSANG_JYE_MODE;
                fepcb->indchfg = ON ;
             }
             break ;
      case PINYIN_MODE :
             if ( fepcb->imode.ind0 != PINYIN_MODE )
             {
                fepcb->imode.ind0 = PINYIN_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case ENGLISH_CHINESE_MODE :
             if ( fepcb->imode.ind0 != ENGLISH_CHINESE_MODE )
             {
                fepcb->imode.ind0 = ENGLISH_CHINESE_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case ABC_MODE :
             if ( fepcb->imode.ind0 != ABC_MODE)
             {
                fepcb->imode.ind0 = ABC_MODE;
                fepcb->indchfg = ON ;
             }
             break ;
      case USER_DEFINED_MODE :
             if ( fepcb->imode.ind0 != USER_DEFINED_MODE)
             {
                fepcb->imode.ind0 = USER_DEFINED_MODE;
                fepcb->indchfg = ON ;
             }
             break ;
      case CAPS_LOCK_MODE : 
             if(fepcb->imode.ind0 != CAPS_LOCK_MODE)
             {
                fepcb->imode.ind0 = CAPS_LOCK_MODE;
                fepcb->indchfg = ON ;
             }
             break;
   } /* end case ind0 */

   switch ( imode.ind1 )
   {
      case HALF :
             if ( fepcb->imode.ind1 != HALF )
             {
                fepcb->imode.ind1 = HALF ;
                fepcb->indchfg = ON ;
             }
             break ;
      case FULL :
             if ( fepcb->imode.ind1 != FULL )
             {
                fepcb->imode.ind1 = FULL ;
                fepcb->indchfg = ON ;
             }
             break ;
   } /* end switch ind1 */

   switch( imode.ind4 )
   {
      case INSERT_MODE :
             if ( fepcb->imode.ind4 != INSERT_MODE )
                  fepcb->imode.ind4 = INSERT_MODE ;
             break ;

      case REPLACE_MODE :
             if ( fepcb->imode.ind4 != REPLACE_MODE )
                  fepcb->imode.ind4 = REPLACE_MODE ;
             break ;

   } /* end switch ind3 */


   switch(imode.ind3)
   {
      case NORMAL_MODE:
           if(fepcb->imode.ind3 != NORMAL_MODE)
              fepcb->imode.ind3 = NORMAL_MODE;
           break;
      case SUPPRESSED_MODE:
           if(fepcb->imode.ind3 != SUPPRESSED_MODE)
              fepcb->imode.ind3 = SUPPRESSED_MODE;
           break;
      default:
           break;
   }

/* ============================================ */

   fepcb->imode.ind5 = imode.ind5;


   return (OK) ;

}

char  *ZHedGetIndMessage(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 

   unsigned char error1_str[12];
   unsigned char error2_str[9];

   strncpy (error1_str, ERROR_MSG_UTF, 11);
   error1_str[11] = '\0';
   strncpy (error2_str, NO_WORD_UTF, 8);
   error2_str[8] = '\0';
   switch(fepcb->imode.ind5)
   {
      case BLANK   :
             return(blank_str);
      case ERROR1  :
             if(fepcb->isbeep == ON)    /* For resize case, when mwm resize window      */
                return(error1_str);     /* the echo and aux area should be cleared      */
             else                       /* So, the error message should be also clearwd */
                 return(blank_str);     /* Normally, the isbeep flag is ON, message     */
      case ERROR2  :                    /* should be filled in, ==> It's Ok !           */
             if(fepcb->isbeep == ON)    /* Resize case, the isbeep flag is OFF, then,   */
                return(error2_str);     /* the error message didn't need to be filled   */
             else                       /* in. ======> Modified and written by Jim      */
                 return(blank_str);
      default:
             return( NULL );
   }
}

char  *ZHedGetIndLegendL(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 

   unsigned char legend_str[9];

   strncpy(legend_str, LEGEND_UTF, 8);
   legend_str[8] = '\0';
   if(fepcb->imode.ind7 == LEGEND_ON)
   {
      return(legend_str);
   }
   else
      return(blank_str);
}

char  *ZHedGetIndLegendS(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 

   unsigned char legend_str[9];

   strncpy(legend_str, S_LEGEND_UTF, 8);
   legend_str[8] = '\0';
   if(fepcb->imode.ind7 == LEGEND_ON)
   {
      return(legend_str);
   }
   else
      return(blank_str);
}
int ZHedIsAuxBufUsedNow(fepcb)
FEPCB *fepcb;
{
   return( fepcb->auxuse );
}


void ZHedSetWarning(fepcb)
FEPCB *fepcb;
{
   fepcb->indchfg = fepcb->isbeep = ON;
}
