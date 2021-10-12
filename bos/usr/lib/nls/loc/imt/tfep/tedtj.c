static char sccsid[] = "@(#)16  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedtj.c, libtw, bos411, 9428A410j 4/21/94 02:19:51";
/*
 * COMPONENT_NAME :     LIBTW
 *
 * FUNCTIONS :          tedtj.c
 *
 * ORIGINS :            27
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
/* MODULE NAME:         TedTj                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Tsang-Jye Input Method source file            */
/*                                                                            */
/* FUNCTION:            TjMain : Entry Procedure for Tsang-Jye Input Method   */
/*                                                                            */
/*                      TjProcess : State Transition Procedure                */
/*                                                                            */
/*                      TjInitial : Initial Tsang-Jye State                   */
/*                                                                            */
/*                      TjStarInput : Input Procedure for STAR Character      */
/*                                                                            */
/*                      TjRadicalInput : Input Procedure for Tsang-Jye        */
/*                                         Radicals                           */
/*                                                                            */
/*                      TjEraseCurrentRadical : Erase Tsang-Jye Radical       */
/*                                                                            */
/*                      TjEraseAllRadical : Erase All Tsang-Jye Radicals      */
/*                                                                            */
/*                      TjKeyToRadical : Translate Input ASCII to Tsang-Jye   */
/*                                         Radical EUC Code                   */
/*                                                                            */
/*                      TjIsRadical : Test whether input key can translate    */
/*                                      into radical.                         */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Andrew Wu                                             */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

#include "ted.h"
#include "tedinit.h"
#include "tedacc.h"
#include "msgextrn.h"     /* @big5 */

/******************************************************************************/
/* FUNCTION    : TjMain                                                       */
/* DESCRIPTION : Entry of Tsang_Jye & Simplified Tsang_Jye                    */
/*               Input Method.                                                */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/


TjMain(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{

   /**************************************/
   /*  For simplied Tsang-Jye selection  */
   /**************************************/

   if ( fepcb->imode.ind2 == STJ_SELECT_ON )
   {
      StjSelect(fepcb,key);           /* the simplified Tsang_Jye candidate  */
      return;                         /* selection list box is displayed     */
   }                                  /*  now                                */

   if ( fepcb->imode.ind2 == STROKE_SELECT_ON )
   {
      StrokeSelect(fepcb,key);       /* the simplified Tsang_Jye stroke     */
      return;                        /* selection list box is displayed     */
   }                                 /*  now                                */

   /***********************/
   /*  Test for PIMI key  */
   /***********************/

   switch( key )                 /* testing input shift state key of   */
   {                                             /* of ALPH_NUM_KEY,   */
      case ALPH_NUM_KEY  :                       /*    TSANG_JYE_KEY,  */
             StjFreeCandidates(fepcb);           /*    PHONETIC_KEY,   */
             AnInitial(fepcb);                   /*    INTERNAL_CODE_KEY   */
             break;
                                               /*    FULL_HALF_KEY   */
      case TSANG_JYE_KEY :
             TjInitial(fepcb);
             break;

      case PHONETIC_KEY  :
             StjFreeCandidates(fepcb);   /* Free STJ Candidates Buffer    */
             PhInitial(fepcb);
             break;

      case INTERNAL_CODE_KEY :
             StjFreeCandidates(fepcb);   /* Free STJ Candidates Buffer    */
             IcInitial(fepcb);
             break;

      case FULL_HALF_KEY :
             if ( fepcb->imode.ind1 == FULL )
                fepcb->imode.ind1 = HALF ;
             else
                fepcb->imode.ind1 = FULL ;
             fepcb->indchfg = ON ;
             break;
      default :                             /* go to Tsang_Jye input  */
             TjProcess(fepcb,key);          /* radical state          */
             break;
   }
   return;
}

/******************************************************************************/
/* FUNCTION    : TjProcess                                                    */
/* DESCRIPTION : Processing each key for Tsang_Jye Input Method.              */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjProcess(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   /********************************************/
   /* Process '*' input for Simplied Tsang-Jye */
   /********************************************/
   if ( (key == '*' || key == ':') && fepcb->starpos== 0 )
   {
      switch ( fepcb->inputlen )
      {
         case 0  :
                TjEraseAllRadical(fepcb);  /* Ease all radical         */
                AnProcess(fepcb,key);      /* Return key to AIX system */
                break;
         case 1  :
         case 2  :
                TjStarInput(fepcb);        /*Input "*" radical for     */
                break;                     /* Simplied Tsang-Jye.      */
         default :
                fepcb->indchfg = ON;
                fepcb->imode.ind5 = ERROR1;
                fepcb->isbeep = ON ;       /* sound a beep */
                break;
      }
      return;
    }
   /******************************/
   /* process Tsang-Jye radicals */
   /******************************/
   if ( TjIsRadical(key) == TRUE )
   {
      if ( fepcb->starpos == 0 )     /* Tsang_Jye radical input mode */
      {
         if ( fepcb->inputlen >= MAX_TJ_INPUT_LEN )
         {                               /* Input len overflow error */
            fepcb->inputlen = 0 ;
            fepcb->echocrps = 0 ;        /* cursor change position   */
            fepcb->eccrpsch = ON;        /* cursor change flag       */
            fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
            fepcb->echobufa[fepcb->echocrps+1] |= UNDERLINE_ATTR;
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = 0;
            fepcb->echochfg.chlenbytes = 2;
            fepcb->indchfg = ON ;        /* indicator change flag on */
            fepcb->imode.ind5 = ERROR1;  /* display error message    */
            fepcb->isbeep   = ON;        /* sound a beep             */
            return;
         }
      }
      else                  /* Simplied Tsang-Jye radical input mode */
      {
         if ( fepcb->inputlen >= MAX_STJ_INPUT_LEN )
         {                               /* Input len overflow error */
            fepcb->isbeep   = ON;        /* sound a beep             */
            fepcb->inputlen = 0 ;
            fepcb->echocrps = 0 ;        /* cursor change position   */
            fepcb->eccrpsch = ON;
            fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
            fepcb->echobufa[fepcb->echocrps+1] |= UNDERLINE_ATTR;
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = 0;
            fepcb->echochfg.chlenbytes = 2;
            fepcb->indchfg = ON ;        /* indicator change flag on */
            fepcb->imode.ind5 = ERROR1;  /* display error message    */
            return;
         }
      }

      /***********************************************/
      /*  If error message was displayed last time   */
      /***********************************************/

      if ( fepcb->echoacsz != 0 && fepcb->inputlen == 0 )
      {
         TjEraseAllRadical(fepcb);   /* Ease error radical */
      }

      TjRadicalInput(fepcb,key);     /* Input Tsang-Jye radical. */
      return;
   }

   /*****************************/
   /* Process special input key */
   /*****************************/

   switch( key )
   {
      case NON_CONVERT_KEY :
                fepcb->isbeep = ON;            /* It's better to take some     */
                fepcb->indchfg = ON;           /* action. (Beep and Error msg  */
                fepcb->imode.ind5 = ERROR1;    /* Added by Jim Roung           */
             return;                           /* No action                */
      case CONVERT_KEY :
             if ( fepcb->echoacsz == 0)
             {
                fepcb->isbeep = ON;
                fepcb->indchfg = ON;            /* added by Jim Roung   */
                fepcb->imode.ind5 = ERROR1;
                return;
             }

             if ( fepcb->inputlen == 0 )
             {
                if ( fepcb->imode.ind5 == BLANK )
                {
                   fepcb->indchfg = ON ;        /* indicator change flag on */
                   fepcb->imode.ind5 = ERROR2;  /* display error message    */
                   fepcb->isbeep = ON;          /* sound a beep             */
                }
             }
             else
             {
                if ( fepcb->starpos != 0 )            /* Simplify Tsang Jye */
                {
                   StjCandidates(fepcb);
                   return;
                }

                AccessDictionary(fepcb);              /* Tsang Jye       */

                if ( fepcb->ret == FOUND_WORD )       /* the word found  */
                {
                   memset(fepcb->edendbuf,NULL,strlen(fepcb->preedbuf));
                   strcpy(fepcb->edendbuf,fepcb->preedbuf);  /* buffering */
                   fepcb->edendacsz = strlen(fepcb->edendbuf);
                   memset(fepcb->preinpbuf, NULL, strlen(fepcb->preinpbuf)); /* V410 */
                   strcpy(fepcb->preinpbuf,fepcb->curinpbuf); /* buffering */
                   TjEraseAllRadical(fepcb);
                }
                else
                {
                   if (fepcb->ret == FOUND_CAND)                     /* V410 */
                   {                   /* The TSANG JYE CODE duplicate  V410 */
                      StjCandidates(fepcb);                          /* V410 */
                      return;                                        /* V410 */
                   }                                                 /* V410 */
                   else                                              /* V410 */
                   {                                                 /* V410 */
                     fepcb->indchfg = ON ;            /*  the word not found  */
                     fepcb->imode.ind5 = ERROR2;
                     fepcb->echocrps = 0;
                     fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
                     fepcb->echobufa[fepcb->echocrps+1] |= UNDERLINE_ATTR;
                     fepcb->echochfg.flag = ON;
                     fepcb->echochfg.chtoppos = 0;
                     fepcb->echochfg.chlenbytes = 2;
                     fepcb->eccrpsch = ON;
                     fepcb->isbeep   = ON;
                     fepcb->inputlen = 0;
                   }                                                 /* V410 */
                }
             }
             return;

      case BACK_SPACE_KEY :
                if ( fepcb->inputlen > 0 )    /* Has radicals Over-The-Spot */
                {
                   TjEraseCurrentRadical(fepcb);
                   return;
                }
                break;
      case ESC_KEY :
                if ( fepcb->inputlen > 0 )  /* Has radical Over-The-Spot */
                {
                   TjEraseAllRadical(fepcb);
                   return;
                 }
                 break;
   }

   /***************************/
   /* Process other input key */
   /***************************/

   if ( fepcb->inputlen == 0 )
   {
      TjEraseAllRadical(fepcb);    /* Erase radical buffer     */
      AnProcess(fepcb,key);        /* return key to AIX system */
   }
   else{
      fepcb->isbeep  = ON;         /* sound a beep             */
      fepcb->indchfg = ON;
      fepcb->imode.ind5 = ERROR1;  /* added by Jim Roung       */
   }
   return;
}

/******************************************************************************/
/* FUNCTION    : TjInitial                                                    */
/* DESCRIPTION : Initialize Tsang Jye Input Method Environment                */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjInitial(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->imode.ind0 != TSANG_JYE_MODE )  /* set indicator flag */
   {
      fepcb->indchfg = ON ;
      fepcb->imode.ind0 = TSANG_JYE_MODE;
   }

   fepcb->imode.ind2 = SELECT_OFF;

   TjEraseAllRadical(fepcb);                   /* erase radicals    */
}

/******************************************************************************/
/* FUNCTION    : TjStarInput                                                  */
/* DESCRIPTION : Input '*' to Radical_Buffer                                  */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjStarInput(fepcb)
FEPCB *fepcb;
{
   fepcb->curinpbuf[fepcb->inputlen] = '*';
   fepcb->inputlen++;
/* fepcb->echobufs[fepcb->echoacsz] = 0xa1  ;       input '*' EUC code @big5 */
/* fepcb->echobufs[fepcb->echoacsz+1] = 0xee;                          @big5 */
   memcpy(&(fepcb->echobufs[fepcb->echoacsz]),Star_code,2);       /*   @big5 */
   memset(&fepcb->echobufa[fepcb->echoacsz],REVERSE_ATTR,2);
   fepcb->echoover = fepcb->echoover - 2;
   fepcb->echochfg.flag = ON;
   fepcb->echochfg.chtoppos = fepcb->echoacsz;
   fepcb->echochfg.chlenbytes = 2;
   fepcb->echoacsz = fepcb->echoacsz + 2;
   fepcb->echocrps = fepcb->echocrps + 2 ;
   fepcb->eccrpsch = ON;
   fepcb->starpos  = fepcb->inputlen - 1;
}

/******************************************************************************/
/* FUNCTION    : TjRadicalInput                                               */
/* DESCRIPTION : Input Tsang-Jye Radical to echo_buffer                       */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjRadicalInput(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   extern   unsigned char *TjKeyToRadical();

   unsigned char *rad_str;

   rad_str = TjKeyToRadical(key);           /* get EUC code */
   if ( key >= 0x41 && key <=0x5a )
     key = key + 0x20;
   fepcb->curinpbuf[fepcb->inputlen] = key; /* fill low case letter */
   fepcb->inputlen = fepcb->inputlen + 1;
   memcpy(&fepcb->echobufs[fepcb->echoacsz],rad_str,2);
   memset(&fepcb->echobufa[fepcb->echoacsz],REVERSE_ATTR,2);
   fepcb->echoover = fepcb->echoover - 2;
   fepcb->echochfg.flag = ON;               /* fill echochfg structure */
   fepcb->echochfg.chtoppos = fepcb->echoacsz;
   fepcb->echochfg.chlenbytes = 2;
   fepcb->echoacsz = fepcb->echoacsz + 2;
   fepcb->echocrps = fepcb->echocrps + 2;
   fepcb->eccrpsch = ON;
}

/******************************************************************************/
/* FUNCTION    : TjEraseCurrentRadical                                        */
/* DESCRIPTION : Erase the preceding input radical                            */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjEraseCurrentRadical(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->curinpbuf[fepcb->inputlen-1] == '*' )
      fepcb->starpos = 0;        /* for simplied Tsang-Jye  */
   memset(&fepcb->curinpbuf[fepcb->inputlen-1],NULL,1);
   fepcb->inputlen = fepcb->inputlen - 1;
   memset(&fepcb->echobufs[fepcb->echoacsz-2],NULL,2);
   memset(&fepcb->echobufa[fepcb->echoacsz-2],NULL,2);
   fepcb->echoacsz = fepcb->echoacsz - 2;
   fepcb->echoover = fepcb->echoover + 2;
   fepcb->echochfg.flag = ON;     /* fill echochfg structure  */
   fepcb->echochfg.chtoppos = fepcb->echoacsz;
   fepcb->echochfg.chlenbytes = 2;
   fepcb->echocrps = fepcb->echocrps - 2;
   fepcb->eccrpsch = ON;
}

/******************************************************************************/
/* FUNCTION    : TjEraseAllRadical                                            */
/* DESCRIPTION : Erase all Tsang-Jye input radicals                           */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

TjEraseAllRadical(fepcb)
FEPCB *fepcb;
{

   /*************************************************/
   /*   Test if there is some radical Over_the_spot */
   /*************************************************/

   if ( fepcb->echoacsz > 0 )       /* echo buffer is not empty  */
   {
      fepcb->starpos = 0;
      memset(fepcb->curinpbuf,NULL,fepcb->echosize);
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
   /*   by wu
   if ( fepcb->imode.ind5 != BLANK )
   {
      fepcb->imode.ind5 = BLANK;
      fepcb->indchfg = ON;
   }
         by wu   */
}

/******************************************************************************/
/* FUNCTION    : TjKeyToRadical                                               */
/* DESCRIPTION : GET Tsang_Jye Radical EUC code of input key                  */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : pointer to the EUC code string                               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

unsigned char *TjKeyToRadical(key)
unsigned char  key;
{
   /* Tsang-Jye radicals EUC code string  */
   static unsigned short tj_w_tab[]={
           0xc5ca,0xc5cc,0xcfda,0xc5cd,0xc5d5,0xc5d6,0xc4c8,
           0xc8cc,0xc5c1,0xc4b2,0xc4cb,0xc4e3,0xc4a1,0xc4de,
           0xc4a9,0xc5c0,0xc5c3,0xc4c7,
           0xc4d3,0xc5bd,0xc4d4,0xc4cc,0xc6f0,0xf9c5,0xc4b3,
           0xd3ec};

   static unsigned char ptr[3];
   unsigned short total;
   int    i ;

   if ( TjIsRadical(key) == TRUE )
   {
      if ( key >= 0x41 && key <=0x5a )
      {
         i = key - 0x41;         /* input is upper case  */
         total = tj_w_tab[i];
      }
      else
      {
         i = key - 0x61;         /*  input is lower case */
         total = tj_w_tab[i];
      }
      ptr[0] = total / 256;
      ptr[1] = total % 256;
      ptr[2] = '\0';
      return(ptr);
   }
}

/******************************************************************************/
/* FUNCTION    : TjIsRadical                                                  */
/* DESCRIPTION : Test Input key is a Tsang Jye Radical                        */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

int TjIsRadical(key)
unsigned char key;
{
   if ( (key>=0x41 && key<=0x5a)||(key>=0x61 && key<=0x7a) )
      return ( TRUE );      /* only in Tsang-Jye input mode   */
   else                     /* could call the subroutine      */
      return ( FALSE );
}

