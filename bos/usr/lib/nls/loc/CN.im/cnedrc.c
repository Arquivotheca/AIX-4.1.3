static char sccsid[] = "@(#)18	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedrc.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:52";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: RcEraseAllRadical
 *		RcEraseCurrentRadical
 *		RcFilter
 *		RcInitial
 *		RcIsEucCodeRange
 *		RcIsRadical
 *		RcMain
 *		RcRadicalInput
 *		RcTranslator
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
/* MODULE NAME:         CNedRC                                                */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Row/Column Input Method source file           */
/*                                                                            */
/* FUNCTION:            RcMain : Entry Procedure for RC. Input Method         */
/*                                                                            */
/*                      RcFilter : State Transition Procedure                 */
/*                                                                            */
/*                      RcInitial : Initial Row/Column State                  */
/*                                                                            */
/*                      RcRadicalInput : Input Procedure for RC. Radicals     */
/*                                                                            */
/*                      RcEraseCurrentRadical : Erase Row/Column Radical      */
/*                                                                            */
/*                      RcEraseAllRadical : Erase All Row/Column Radicals     */
/*                                                                            */
/*                      RcIsEucCodeRange : Test Input key is in Euc Code Range*/
/*                                                                            */
/*                      RcIsRadical : Test whether input key can translate    */
/*                                      into radical.                         */
/*                                                                            */
/*                      RcTranslator : Convert Radical to Euc Code            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

#include "cned.h"
#include "cnedinit.h"
#include "cnedacc.h"

/******************************************************************************/
/* FUNCTION    : RcMain                                                       */
/* DESCRIPTION : Entry of Row/Column Input Method                             */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcMain(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   /***********************/
   /*  Test for PIMI key  */
   /***********************/

   switch( key )
   {
      case ALPHA_NUM_KEY:
        AnInitial(fepcb);                /* Nothing would do                  */
        break;

      case ENGLISH_CHINESE_KEY:
        EnInitial(fepcb);                /* Change to English_Chinese initial */
        break;                           /* Status                            */

      case PINYIN_KEY:                   /* Change to Pinyin initial          */
        PyInitial(fepcb);                /* status                            */
        break;

      case ABC_KEY:                      /* Change to Abc Initial status      */
        AbcInitial(fepcb);
        break;

      case USER_DEFINED_KEY:             /* Change to User Defined Initial    */
        UdInitial(fepcb);                /* status                            */
        break;

      case ROW_COLUMN_KEY:               /* Change to Row/column Initial      */
        RcInitial(fepcb);                /* status                            */
        break;

      case FIVESTROKE_STYLE_KEY :
        FssInitial(fepcb);               /* Initialize Five Stroke Style Input*/
                                         /* Method                            */
        break;

      case FIVESTROKE_KEY :
           FsInitial(fepcb);          /* Initialize Five Stroke  Input   */
                                       /* Method                             */
           break;

      case FULL_HALF_KEY :
        fepcb->indchfg = ON ;
        if ( fepcb->inputlen > 0 )
        {
           fepcb->imode.ind5 = ERROR1;     /* display error message    */
           fepcb->isbeep   = ON;           /* sound a beep             */
           break;
        }
        if ( fepcb->imode.ind1 == FULL )   /* If indicator is FULL then */
           fepcb->imode.ind1 = HALF ;      /* to be HALF                */
        else
           fepcb->imode.ind1 = FULL ;      /* Else set to be FULL       */
        break;

      default :                             /* go to Row/Column input  */
             RcFilter(fepcb,key);           /* radical state           */
             break;
   }
   return;
}

/******************************************************************************/
/* FUNCTION    : RcFilter                                                     */
/* DESCRIPTION : Processing each key for Row/Column Input Method.             */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcFilter(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
     /* Input mode is FULL ,  Processing for ASCII mode    */
   if (fepcb->imode.ind1 == FULL)
   {
       AnProcess(fepcb, key);
       return;
   }

   /*******************************/
   /* process Row/Column radicals */
   /*******************************/
   if ( RcIsRadical(key) )
   {
      if ( fepcb->inputlen > MAX_RC_INPUT_LEN )
      {                               /* Input len overflow error */
         fepcb->inputlen = 0 ;
         fepcb->echocrps = 0 ;        /* cursor change position   */
         fepcb->eccrpsch = ON;        /* cursor change flag       */
         fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
         fepcb->echochfg.flag = ON;
         fepcb->echochfg.chtoppos = 0;
         fepcb->echochfg.chlenbytes = 1;
         fepcb->indchfg = ON ;        /* indicator change flag on */
         fepcb->imode.ind5 = ERROR1;  /* display error message    */
         fepcb->isbeep   = ON;        /* sound a beep             */
         return;
      }

      /***********************************************/
      /*  If error message was displayed last time   */
      /***********************************************/

      if ( fepcb->echoacsz != 0 && fepcb->inputlen == 0 )
         RcEraseAllRadical(fepcb);   /* Erase error radical */

      RcRadicalInput(fepcb,key);     /* Input Row/Column radical. */

      if ( fepcb->inputlen == MAX_RC_INPUT_LEN ) 
      {
         memset(fepcb->rcinbuf, NULL, strlen(fepcb->rcinbuf));
         RcTranslator(fepcb->echobufs, fepcb->rcinbuf);
         if ( RcIsEucCodeRange(fepcb->rcinbuf) )
         {
            memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
            strcpy(fepcb->edendbuf, fepcb->rcinbuf, strlen(fepcb->rcinbuf));
            fepcb->edendacsz = strlen(fepcb->edendbuf);
            RcEraseAllRadical(fepcb);
            fepcb->ret = FOUND_WORD;
            return;
         }
         else
         {

            fepcb->indchfg = ON ;            /*  the word not found  */
            fepcb->imode.ind5 = ERROR2;
            fepcb->echocrps = 0;
            fepcb->eccrpsch = ON;
            fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = 0;
            fepcb->echochfg.chlenbytes = 1;
            fepcb->isbeep   = ON;
            fepcb->inputlen = 0;
         }
      }
      return;
   }

   /*****************************/
   /* Process special input key */
   /*****************************/

   switch( key )
   {
      case NON_CONVERT_KEY :
        return;                           /* No action                */

      case BACK_SPACE_KEY :
        if ( fepcb->inputlen > 0 )    /* Has radicals Off-The-Spot */
        {
           RcEraseCurrentRadical(fepcb);
           return;
        }
        break;

      case ESC_KEY :
        if ( fepcb->inputlen > 0 )  /* Has radical Off-The-Spot */
        {
           RcEraseAllRadical(fepcb);
           return;
        }
        break;

   }

   /***************************/
   /* Process other input key */
   /***************************/

   if ( fepcb->inputlen == 0 )
   {
      RcEraseAllRadical(fepcb);     /* Erase radical buffer     */
      AnProcess(fepcb, key);        /* return key to AIX system */
   }
   else
   {
      fepcb->indchfg = ON;     
      fepcb->imode.ind5 = ERROR1;
      fepcb->isbeep  = ON;          /* sound a beep             */
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : RcInitial                                                    */
/* DESCRIPTION : Initialize Row/Column Input Method Environment               */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcInitial(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->imode.ind0 != ROW_COLUMN_MODE )  /* set indicator flag */
   {
      fepcb->indchfg = ON ;
      fepcb->imode.ind0 = ROW_COLUMN_MODE;
   }

   fepcb->imode.ind2 = SELECT_OFF;

   RcEraseAllRadical(fepcb);                   /* erase radicals    */
}

/******************************************************************************/
/* FUNCTION    : RcRadicalInput                                               */
/* DESCRIPTION : Input Row/Column Radical to echo_buffer                      */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcRadicalInput(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{

   unsigned char *rad_str;

  /* Pressed key to echo buffer     */
   fepcb->inputlen = fepcb->inputlen + 1;
   fepcb->echoover = fepcb->echoover - 1;
   *(fepcb->echobufs+fepcb->echocrps) = key;
   *(fepcb->echobufa+fepcb->echocrps) = REVERSE_ATTR;
   fepcb->echochfg.flag = ON;               /* fill echochfg structure */
   fepcb->echochfg.chtoppos = fepcb->echoacsz;
   fepcb->echochfg.chlenbytes = 1;
   fepcb->echoacsz = fepcb->echoacsz + 1;
   fepcb->echocrps = fepcb->echocrps + 1;
   fepcb->eccrpsch = ON;
}

/******************************************************************************/
/* FUNCTION    : RcEraseCurrentRadical                                        */
/* DESCRIPTION : Erase the preceding input radical                            */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcEraseCurrentRadical(fepcb)
FEPCB *fepcb;
{
   /* Erase the input radical in the preceding area */
   fepcb->inputlen = fepcb->inputlen - 1;
   memset(&fepcb->echobufs[fepcb->echoacsz-1],NULL,1);
   memset(&fepcb->echobufa[fepcb->echoacsz-1],NULL,1);
   fepcb->echoacsz = fepcb->echoacsz - 1;
   fepcb->echoover = fepcb->echoover + 1;
   fepcb->echochfg.flag = ON;     /* fill echochfg structure  */
   fepcb->echochfg.chtoppos = fepcb->echoacsz;
   fepcb->echochfg.chlenbytes = 1;
   fepcb->echocrps = fepcb->echocrps - 1;
   fepcb->eccrpsch = ON;
}

/******************************************************************************/
/* FUNCTION    : RcEraseAllRadical                                            */
/* DESCRIPTION : Erase all Row/Column input radicals                          */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcEraseAllRadical(fepcb)
FEPCB *fepcb;
{

   /*************************************************/
   /*   Test if there is some radical Off_the_spot  */
   /*************************************************/

   if ( fepcb->echoacsz > 0 )       /* echo buffer is not empty  */
   {
      fepcb->inputlen=0;
      memset(fepcb->echobufs,NULL,fepcb->echosize);
      memset(fepcb->echobufa,NULL,fepcb->echosize);
      fepcb->echoover = fepcb->echosize;
      fepcb->echochfg.flag = ON;      /* fill echochfg structure  */
      fepcb->echochfg.chtoppos = 0;
      fepcb->echochfg.chlenbytes = fepcb->echoacsz;
      fepcb->echoacsz = 0;
      fepcb->echocrps = 0;
      fepcb->eccrpsch = ON;
   }
}


/******************************************************************************/
/* FUNCTION    : RcTranslator                                                 */
/* DESCRIPTION : Convert Input Radical to Euc Code                            */
/* INPUT       : string = input radical buffer                                */
/*               hzcode =  chinese character buffer                           */
/* OUTPUT      : Chinese character code ( euc format )                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcTranslator(string, hzcode)
unsigned char string[];
unsigned char hzcode[];
{
   hzcode[0] = (string[0] - '0') * 10 + (string[1] - '0') + 0xa0;
   hzcode[1] = (string[2] - '0') * 10 + (string[3] - '0') + 0xa0;
}

/******************************************************************************/
/* FUNCTION    : RcIsEucCodeRange                                             */
/* DESCRIPTION : Test Input key is in Euc Code Range (0xa1a1 -- 0xfefe)       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcIsEucCodeRange(ptr)
unsigned char *ptr;
{
   if (strlen(ptr) == 2)
   {
      if ((*ptr<0xa1 || *ptr>0xfe) || (*(ptr+1)<0xa1 || *(ptr+1)>0xfe))
         return FALSE;
   }
   return TRUE;
}


/******************************************************************************/
/* FUNCTION    : RcIsRadical                                                  */
/* DESCRIPTION : Test Input key is a Row/Column Radical                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

RcIsRadical(key)
unsigned char key;
{

   return((key >= 0x30 && key <= 0x39) ? TRUE : FALSE);

}
