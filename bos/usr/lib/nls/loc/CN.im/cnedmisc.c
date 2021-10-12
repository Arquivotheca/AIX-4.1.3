static char sccsid[] = "@(#)15	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedmisc.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:43";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNedGetAuxCurPos
 *		CNedGetAuxSize
 *		CNedGetAuxType
 *		CNedGetEchoBufLen
 *		CNedGetEchoCurPos
 *		CNedGetFixBufLen
 *		CNedGetIndLegendL
 *		CNedGetIndLegendS
 *		CNedGetIndMessage
 *		CNedGetInputMode
 *		CNedIsAuxBufChanged
 *		CNedIsAuxBufUsedNow
 *		CNedIsAuxCurPosChanged
 *		CNedIsBeepRequested
 *		CNedIsEchoBufChanged
 *		CNedIsEchoCurPosChanged
 *		CNedIsInputModeChanged
 *		CNedSetInputMode
 *		CNedSetWarning
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
/* MODULE NAME:         CNedMisc.c                                      */
/*                                                                      */
/* DESCRIPTIVE NAME:    Chinese Input Method Miscelaneous               */
/*                                                                      */
/* FUNCTION:                                                            */
/*                      CNedGetEchoBufLen : Return Echo Buffer Length.  */
/*                                                                      */
/*                      CNedGetFixBufLen : Return Fix Buffer Length.    */
/*                                                                      */
/*                      CNedGetAuxSize : Return Aux Active size.        */
/*                                                                      */
/*                      CNedIsEchoBufChanged : Return Echo Buffer       */
/*                                            Change Flag.              */
/*                                                                      */
/*                      CNedIsAuxBufChanged : Return Aux Buffer         */
/*                                           Change Flag.               */
/*                                                                      */
/*                      CNedIsInputModeChanged : Return Input Mode      */
/*                                              Change Flag Structure.  */
/*                                                                      */
/*                      CNedIsCurPosChanged : Return Cursor Position    */
/*                                           Change Flag.               */
/*                                                                      */
/*                      CNedIsAuxCurPosChanged : Return Aux Cursor      */
/*                                              Position Change Flag.   */
/*                                                                      */
/*                      CNedIsBeepRequested : Return Beep Requested     */
/*                                           Flag.                      */
/*                                                                      */
/*                      CNedGetEchoCurPos : Return Echo Buffer Cursor   */
/*                                         Position.                    */
/*                                                                      */
/*                      CNedGetAuxCurPos : Return Aux Cursor Posstion.  */
/*                                                                      */
/*                      CNedGetAuxType : Return Aux Type.               */
/*                                                                      */
/*                      CNedGetInputMode : Return Input Mode Structure. */
/*                                                                      */
/*                      CNedSetInputMode : Set Input Mode Structure.    */
/*                                                                      */
/*                      CNedGetIndMessage : Return Indicator Message.   */
/*                                                                      */
/*                      CNedGetIndLegend : Return Indicator Legend.     */
/*                                                                      */
/*                      CNedIsAuxBufUsedNow : Return Aux Buffer Used    */
/*                                           Flag.                      */
/*                                                                      */
/*                      CNedSetWarning : Reset Beep sound and error     */
/*                                      msg to user                     */
/*                                                                      */
/******************** END OF SPECIFICATIONS *****************************/
/*
#include "cned.h"
#include "cnedinit.h"
*/
#include "chinese.h"
#include "cnedacc.h"

int     CNedGetEchoBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->echoacsz);
}

int     CNedGetFixBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->edendacsz);
}

AuxSize CNedGetAuxSize(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->auxacsz);
}

EchoBufChanged   CNedIsEchoBufChanged(fepcb)
FEPCB   *fepcb;
{
   return(fepcb->echochfg);
}

int     CNedIsAuxBufChanged(fepcb)
FEPCB  *fepcb ;
{
   if ( fepcb->auxchfg )
      return(TRUE);
   else
      return(FALSE);
}

int      CNedIsInputModeChanged(fepcb)
FEPCB   *fepcb;
{
   return( fepcb->indchfg );
}

int      CNedIsEchoCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
    if ( fepcb->eccrpsch )
       return( TRUE );
    else
       return( FALSE );
}

int     CNedIsAuxCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxcrpsch )
      return( TRUE );
   else
      return( FALSE );
}

int CNedIsBeepRequested(fepcb)
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

int     CNedGetEchoCurPos(fepcb)
FEPCB  *fepcb ;
{
   return( fepcb->echocrps );
}

AuxCurPos   CNedGetAuxCurPos(fepcb)
FEPCB      *fepcb ;
{
   return( fepcb->auxcrps );
}

int     CNedGetAuxType(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxuse == USE )
      return ( Aux_Candidate_List );
   else
      return ( Aux_No_Used );
}

InputMode  CNedGetInputMode(fepcb)
FEPCB     *fepcb ;
{
   return(fepcb->imode);
}

int      CNedSetInputMode(fepcb,imode)
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
      case ROW_COLUMN_MODE :
      case PINYIN_MODE :
      case ENGLISH_CHINESE_MODE :
      case FIVESTROKE_STYLE_MODE :
      case FIVESTROKE_MODE :
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
      case ROW_COLUMN_MODE :
             if ( fepcb->imode.ind0 != ROW_COLUMN_MODE)
             {
                fepcb->imode.ind0 = ROW_COLUMN_MODE;
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
      case FIVESTROKE_STYLE_MODE :
             if ( fepcb->imode.ind0 != FIVESTROKE_STYLE_MODE )
             {
                fepcb->imode.ind0 = FIVESTROKE_STYLE_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case FIVESTROKE_MODE :
             if ( fepcb->imode.ind0 != FIVESTROKE_MODE )
             {
                fepcb->imode.ind0 = FIVESTROKE_MODE ;
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

char  *CNedGetIndMessage(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 
 /*  static unsigned char error1_str[]= {
              0xa1,0xee,0xd1,0xba,0xf2,0xe3,
              0xf5,0xef,0x00 };    

 static unsigned char error1_str[]= {
              0xa1,0xee,0xf2,0xe3,
              0xeb,0xa8,0x00 };       

   static unsigned char error2_str[]= {
              0xa1,0xee,0xca,0xf4,
              0xc7,0xf3,0x00 };*/

   unsigned char error1_str[9];
   unsigned char error2_str[7];

   strncpy (error1_str, ERROR_MSG_EUC, 8);
   error1_str[8] = NULL;
   strncpy (error2_str, NO_WORD_EUC, 6);
   error2_str[6] = NULL;
/*
   strncpy (error1_str, "°´¼ü´í", 6);
   strncpy (error2_str, "ÎÞ×Ö  ", 6);
*/
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

char  *CNedGetIndLegendL(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 

   unsigned char legend_str[7];

   strncpy(legend_str, LEGEND_EUC, 6);
   legend_str[6] = NULL;
   if(fepcb->imode.ind7 == LEGEND_ON)
   {
      return(legend_str);
   }
   else
      return(blank_str);
}

char  *CNedGetIndLegendS(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                 

   unsigned char legend_str[7];

   strncpy(legend_str, S_LEGEND_EUC, 6);
   legend_str[6] = NULL;
   if(fepcb->imode.ind7 == LEGEND_ON)
   {
      return(legend_str);
   }
   else
      return(blank_str);
}
int CNedIsAuxBufUsedNow(fepcb)
FEPCB *fepcb;
{
   return( fepcb->auxuse );
}


void CNedSetWarning(fepcb)
FEPCB *fepcb;
{
   fepcb->indchfg = fepcb->isbeep = ON;
}
