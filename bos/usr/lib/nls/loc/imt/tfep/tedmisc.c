static char sccsid[] = "@(#)12  1.3  src/bos/usr/lib/nls/loc/imt/tfep/tedmisc.c, libtw, bos411, 9428A410j 4/21/94 02:01:12";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TedGetAuxCurPos
 *              TedGetAuxSize
 *              TedGetAuxType
 *              TedGetEchoBufLen
 *              TedGetEchoCurPos
 *              TedGetFixBufLen
 *              TedGetIndMessage
 *              TedGetInputMode
 *              TedIsAuxBufChanged
 *              TedIsAuxBufUsedNow
 *              TedIsAuxCurPosChanged
 *              TedIsBeepRequested
 *              TedIsEchoBufChanged
 *              TedIsEchoCurPosChanged
 *              TedIsInputModeChanged
 *              TedSetInputMode
 *              TedSetWarning
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

/******************** START OF MODULE SPECIFICATIONS ********************/
/*                                                                      */
/* MODULE NAME:         TedMisc.c                                       */
/*                                                                      */
/* DESCRIPTIVE NAME:    Chinese Input Method Miscelaneous               */
/*                                                                      */
/* FUNCTION:                                                            */
/*                      TedGetEchoBufLen : Return Echo Buffer Length.   */
/*                                                                      */
/*                      TedGetFixBufLen : Return Fix Buffer Length.     */
/*                                                                      */
/*                      TedGetAuxSize : Return Aux Active size.         */
/*                                                                      */
/*                      TedIsEchoBufChanged : Return Echo Buffer        */
/*                                            Change Flag.              */
/*                                                                      */
/*                      TedIsAuxBufChanged : Return Aux Buffer          */
/*                                           Change Flag.               */
/*                                                                      */
/*                      TedIsInputModeChanged : Return Input Mode       */
/*                                              Change Flag Structure.  */
/*                                                                      */
/*                      TedIsCurPosChanged : Return Cursor Position     */
/*                                           Change Flag.               */
/*                                                                      */
/*                      TedIsAuxCurPosChanged : Return Aux Cursor       */
/*                                              Position Change Flag.   */
/*                                                                      */
/*                      TedIsBeepRequested : Return Beep Requested      */
/*                                           Flag.                      */
/*                                                                      */
/*                      TedGetEchoCurPos : Return Echo Buffer Cursor    */
/*                                         Position.                    */
/*                                                                      */
/*                      TedGetAuxCurPos : Return Aux Cursor Posstion.   */
/*                                                                      */
/*                      TedGetAuxType : Return Aux Type.                */
/*                                                                      */
/*                      TedGetInputMode : Return Input Mode Structure.  */
/*                                                                      */
/*                      TedSetInputMode : Set Input Mode Structure.     */
/*                                                                      */
/*                      TedGetIndMessage : Return Indicator Message.    */
/*                                                                      */
/*                      TedIsAuxBufUsedNow : Return Aux Buffer Used     */
/*                                           Flag.                      */
/*                                                                      */
/*                      TedSetWarning : Reset Beep sound and error      */
/*                                      msg to user - Added by Jim R.   */
/*                                                                      */
/*                      TedGetLangType : Return Lang Type -add by debby */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/* MODULE TYPE:         C                                               */
/*                                                                      */
/* COMPILER:            AIX C                                           */
/*                                                                      */
/* STATUS:              Chinese Input Method Version 1.0                */
/*                                                                      */
/* CHANGE ACTIVITY:                                                     */
/*                 Sept/18/'91 -> Jim R. add one addtional API          */
/*                                - TedSetWarning()                     */
/*                                                                      */
/*                 10/22/'93   -> Debby add one addtional API           */
/*                                - TedGetLangType()                    */
/*                                                                      */
/******************** END OF SPECIFICATIONS *****************************/

#include "ted.h"
#include "tedinit.h"
#include "tedacc.h"
#include "msgextrn.h"    /* @big5 */

int     TedGetEchoBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->echoacsz);
}

int     TedGetFixBufLen(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->edendacsz);
}

AuxSize TedGetAuxSize(fepcb)
FEPCB  *fepcb;
{
   return(fepcb->auxacsz);
}

EchoBufChanged   TedIsEchoBufChanged(fepcb)
FEPCB   *fepcb;
{
   return(fepcb->echochfg);
}

int     TedIsAuxBufChanged(fepcb)
FEPCB  *fepcb ;
{
   if ( fepcb->auxchfg )
      return(TRUE);
   else
      return(FALSE);
}

int      TedIsInputModeChanged(fepcb)
FEPCB   *fepcb;
{
   return( fepcb->indchfg );
}

int      TedIsEchoCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
    if ( fepcb->eccrpsch )
       return( TRUE );
    else
       return( FALSE );
}

int     TedIsAuxCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxcrpsch )
      return( TRUE );
   else
      return( FALSE );
}

int TedIsBeepRequested(fepcb)
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

int     TedGetEchoCurPos(fepcb)
FEPCB  *fepcb ;
{
   return( fepcb->echocrps );
}

AuxCurPos   TedGetAuxCurPos(fepcb)
FEPCB      *fepcb ;
{
   return( fepcb->auxcrps );
}

int     TedGetAuxType(fepcb)
FEPCB   *fepcb ;
{
   if ( fepcb->auxuse == USE )
      return ( Aux_Candidate_List );
   else
      return ( Aux_No_Used );
}

InputMode  TedGetInputMode(fepcb)
FEPCB     *fepcb ;
{
   return(fepcb->imode);
}

int      TedSetInputMode(fepcb,imode)
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
      case PHONETIC_MODE :
      case TSANG_JYE_MODE :
      case CAPS_LOCK_MODE :
      case INTERNAL_CODE_MODE :
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
      case PHONETIC_MODE :
             if ( fepcb->imode.ind0 != PHONETIC_MODE )
             {
                fepcb->imode.ind0 = PHONETIC_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case TSANG_JYE_MODE :
             if ( fepcb->imode.ind0 != TSANG_JYE_MODE )
             {
                fepcb->imode.ind0 = TSANG_JYE_MODE ;
                fepcb->indchfg = ON ;
             }
             break ;
      case INTERNAL_CODE_MODE :
             if ( fepcb->imode.ind0 != INTERNAL_CODE_MODE)
             {
                fepcb->imode.ind0 = INTERNAL_CODE_MODE;
                fepcb->indchfg = ON ;
             }
             break ;

             break;
      case CAPS_LOCK_MODE : /* Jim Roung added this case for Caps-Lock */
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

char  *TedGetIndMessage(fepcb)
FEPCB *fepcb;
{
   static unsigned char blank_str[]= {
              0x00};                     /* Added by Jim Roung          */
/* static unsigned char error1_str[]= {
              0xa1,0xee,0xd1,0xba,0xf2,0xe3,
              0xf5,0xef,0x00 };                    added by Jim Roung @big5 */

 /*static unsigned char error1_str[]= {
              0xa1,0xee,0xf2,0xe3,
              0xeb,0xa8,0x00 };      ===>   marked by Jim Roung  <=== */

/* static unsigned char error2_str[]= {
              0xa1,0xee,0xca,0xf4,
              0xc7,0xf3,0x00 };                 big5 */

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
      case RADICAL :
             return(fepcb->radeucbuf);
      default:
             return( NULL );
   }
}

int TedIsAuxBufUsedNow(fepcb)
FEPCB *fepcb;
{
   return( fepcb->auxuse );
}

/* =================>>> This API is added by Jim R. <<<===============  */


void TedSetWarning(fepcb)
FEPCB *fepcb;
{
   fepcb->indchfg = fepcb->isbeep = ON;


}

int TedGetLangType(fepcb)                                   /* @big5 */
FEPCB *fepcb;                                               /* @big5 */
{                                                           /* @big5 */
   return(fepcb->Lang_Type);                                /* @big5 */
}                                                           /* @big5 */

int TedGetIconv(fepcb)                                      /* @big5 */
FEPCB *fepcb;                                               /* @big5 */
{                                                           /* @big5 */
   return(fepcb->iconv_flag);                               /* @big5 */
}                                                           /* @big5 */
