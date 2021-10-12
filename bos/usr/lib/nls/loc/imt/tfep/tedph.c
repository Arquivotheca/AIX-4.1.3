static char sccsid[] = "@(#)13  1.4.1.1  src/bos/usr/lib/nls/loc/imt/tfep/tedph.c, libtw, bos411, 9431A411a 7/26/94 17:21:10";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: teph.c
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

/************************* START OF MODULE SPECIFICATION **********************/
/*                                                                            */
/* MODULE NAME:        TedPh                                                  */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Phonetic Mode                 */
/*                                                                            */
/* FUNCTION:           PhMain           : Entry Point Of Phonetic Input Method*/
/*                                                                            */
/*                     PhProcess        : Process Each Input Key              */
/*                                                                            */
/*                     PhInitial        : Initialization                      */
/*                                                                            */
/*                     PhKeyToRadical   : Convert Key To Phonetic Radical     */
/*                                                                            */
/*                     PhEraseAllRadical: Erase All Radicals                  */
/*                                                                            */
/*                     PhCandidates     : Find Satisfied Candidates           */
/*                                                                            */
/*                     PhRadicalInput   : Replace/Insert Radicals To Echo Buf */
/*                                                                            */
/*                     PhCopyLastRadical: Replace/Insert Previous Radical Buf */
/*                                                                            */
/*                     PhSelect         : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     PhListBox        : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     PhCalItemSize    : Caculate Item Size                  */
/*                                                                            */
/*                     PhGetCandidate   : Send Selected Cand. To Output Buffer*/
/*                                                                            */
/*                     PhEraseAux       : Close Candidate List Box            */
/*                                                                            */
/* MODULE TYPE:        C                                                      */
/*                                                                            */
/* COMPILER:           AIX C                                                  */
/*                                                                            */
/* AUTHOR:             Mei Lin            modify by Terry Chou                */
/*                                        modifed by Beson Lu                 */
/*                                        -> PhListBx()                       */
/*                                                                            */
/* STATUS:             Chinese Input Method Version 1.0                       */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/*  V410  06/14/93'    Modified by Debby Tseng (Mirrors) for Input Method     */
/*                     Learning                                               */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/
#include  "tedinit.h"
#include  "ted.h"
#include  "tedacc.h"
#include  "msgextrn.h"    /* @big5 */
extern StrCodeConvert(iconv_t ,
                      unsigned char *,
                      unsigned char *,
                      size_t,
                      size_t);                              /* @big5 */

/******************************************************************************/
/* FUNCTION    : PhMain                                                       */
/* DESCRIPTION : Entry Point of Phonetic Input Method.                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   if( fepcb->imode.ind2 == PHONETIC_SELECT_ON )
   {
      PhSelect(fepcb, key);            /* Select Candidate At Candidate List */
                                       /* Box                                */
      return;
   }

   switch( key )
   {
      case ALPH_NUM_KEY :
           PhFreeCandidates(fepcb);    /* Free Phonetic Candidates Buffer    */
           AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
           break;

      case PHONETIC_KEY :
           PhFreeCandidates(fepcb);    /* Free Phonetic Candidates Buffer    */
           PhInitial(fepcb);           /* Initialize Phonetic Input Method   */
           break;

      case TSANG_JYE_KEY :
           PhFreeCandidates(fepcb);    /* Free Phonetic Candidates Buffer    */
           TjInitial(fepcb);           /* Initialize Tsang_Jye Input Method  */
           break;

      case INTERNAL_CODE_KEY :
           PhFreeCandidates(fepcb);    /* Free Phonetic Candidates Buffer    */
           IcInitial(fepcb);
           break;

      case FULL_HALF_KEY :             /* Set FULL/HALF Mode                 */
           fepcb->indchfg = TRUE;

           if( fepcb->imode.ind1 == HALF )
             fepcb->imode.ind1 = FULL;
           else
             fepcb->imode.ind1 = HALF;
           break;

      default :
           PhProcess(fepcb, key);      /* Process Phonetic Input Method      */
           break;
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : PhProcess                                                    */
/* DESCRIPTION : Process each input key for Phonetic Input Method.            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhProcess(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned short  cnt;                /* Byte Counter                       */
   unsigned char   phcode[3];          /* Phonetic Code                      */
   unsigned char   code[3];            /* @big5 */
   unsigned char   euccode[3];         /* @big5 */
   size_t          in_count,out_count; /* @big5 */

/*   PhKeyToRadical(key, phcode);      /* Convert Key To Phonetic Radical If */
     PhKeyToRadical(fepcb,key, phcode);/* Convert Key To Phonetic Radical If */
                                       /* @big5                              */
                                       /* It Is In Phonetic Radical Range    */

   if( phcode[0] != NULL )             /* Key Is In Phonetic Code Range      */
   {
      PhRadicalInput(fepcb, phcode);   /* Replace/Insert Key To Echo Buffer  */
      PhShowCursor(fepcb);             /*  by terry  */
      return;
   }

   if( key == CTRL_RIGHT_KEY )
   {
      if (strlen(fepcb->ctrl_r_buf) != 0)
      {
         PhCopyLastRadical(fepcb);       /* Ctrl_Right_Buf To Echo Buffer */
         PhShowCursor(fepcb);            /*  by terry  */
      }
      else
      {
         if (fepcb->inputlen == 0 )
             AnProcess(fepcb, key);         /* Return Key To AIX System           */
         else
         {
            fepcb->isbeep = ON;
            fepcb->indchfg = ON;
            fepcb->imode.ind5 = ERROR1;
         }
      }
      return;
   }

   if( fepcb->inputlen == 0 )
   {
      if( key == CONVERT_KEY || key == NON_CONVERT_KEY )
      {
        fepcb->isbeep = BEEP_ON;       /* Notify FEP To Beep To Alarm        */
        fepcb->indchfg = ON;           /* added by Jim Roung    */
        fepcb->imode.ind5 = ERROR1;
      }
      else
        AnProcess(fepcb, key);         /* Return Key To AIX System           */
      return;
   }

   switch( key )
   {
      case BACK_SPACE_KEY :
         if( fepcb->echocrps > 0 )
         {
            for( cnt = fepcb->echocrps-2; cnt <= fepcb->echoacsz-1; cnt++ )
            {
               *(fepcb->echobufs+cnt) = *(fepcb->echobufs+cnt+2);
               *(fepcb->echobufa+cnt) = *(fepcb->echobufa+cnt+2);
            }
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = fepcb->echocrps-2;
            fepcb->echochfg.chlenbytes = fepcb->echoacsz-(fepcb->echocrps-2);
            fepcb->echocrps -= 2;
            fepcb->eccrpsch = ON;
            fepcb->echoacsz -= 2;
            fepcb->echoover += 2;
            fepcb->inputlen -= 1;
         }
         break;

      case CONVERT_KEY :
         PhCandidates(fepcb);          /* Find Satisfied Candidates          */
         break;

      case NON_CONVERT_KEY :
         if (fepcb->Lang_Type != codesetinfo[0].code_type)        /* @big5 */
           for( cnt=0; cnt<fepcb->echoacsz; cnt=cnt+2 )           /* @big5 */
           {                                                      /* @big5 */
              in_count = 2;                                       /* @big5 */
              out_count = 2;                                      /* @big5 */
              euccode[0] = *(fepcb->echobufs+cnt);
              euccode[1] = *(fepcb->echobufs+cnt+1);
              StrCodeConvert(fepcb->iconv_flag,euccode,
                                        code, &in_count,&out_count);/* @big5 */

              *(fepcb->edendbuf+cnt) = code[0];                   /* @big5 */
              *(fepcb->edendbuf+cnt+1) = code[1];                 /* @big5 */
           }                                                      /* @big5 */
         else                                                     /* @big5 */
           for( cnt=0; cnt<fepcb->echoacsz; cnt++ )
              *(fepcb->edendbuf+cnt) = *(fepcb->echobufs+cnt);

         fepcb->edendacsz = fepcb->echoacsz;
         PhEraseAllRadical(fepcb);     /* Clear All Radicals At Over_The_Spot*/
                                       /* Copy Echo Buffer To Edit_End Buffer*/
         fepcb->ret = FOUND_WORD;
         break;

      case ESC_KEY :
         PhEraseAllRadical(fepcb);     /* Clear All Radicals At Over_The_Spot*/
         break;

      case LEFT_ARROW_KEY :
         if( fepcb->echocrps > 0 )
         {
            fepcb->echocrps -= 2;
            fepcb->eccrpsch = ON;
         }
         break;

      case RIGHT_ARROW_KEY :
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            fepcb->echocrps += 2;
            fepcb->eccrpsch = ON;
         }
         break;

      case INSERT_KEY :
         if( fepcb->imode.ind4 == INSERT_MODE )
           fepcb->imode.ind4 = REPLACE_MODE;
         else
           fepcb->imode.ind4 = INSERT_MODE;
         break;

      case DELETE_KEY :
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            for( cnt = fepcb->echocrps; cnt <= fepcb->echoacsz-1; cnt++ )
            {
               *(fepcb->echobufs+cnt) = *(fepcb->echobufs+cnt+2);
      /*       *(fepcb->echobufa+cnt) = *(fepcb->echobufa+cnt+2);  by terry */
            }
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = fepcb->echocrps;
            fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
            fepcb->echoacsz -= 2;
            fepcb->echoover += 2;
            fepcb->inputlen -= 1;
         }
         else
         {
           fepcb->isbeep = BEEP_ON;
           fepcb->indchfg = ON;         /* added by Jim Roung   */
           fepcb->imode.ind5 = ERROR1;
         }
         break;

      case CTRL_HOME_KEY :
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            for( cnt = fepcb->echocrps; cnt <= fepcb->echoacsz-1; cnt++ )
            {
               *(fepcb->echobufs+cnt) = NULL;
               *(fepcb->echobufa+cnt) = NULL;
            }
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = fepcb->echocrps;
            fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
            fepcb->echoover += (fepcb->echoacsz-fepcb->echocrps);
            fepcb->inputlen -= (fepcb->echoacsz-fepcb->echocrps)/2;
            fepcb->echoacsz = fepcb->echocrps;
         }
         else
         {
           fepcb->isbeep = BEEP_ON;
           fepcb->indchfg = ON;
           fepcb->imode.ind5 = ERROR1;
         }
         break;

      default :
         fepcb->isbeep = BEEP_ON;      /* Notify FEP To Beep To Alarm        */
         fepcb->indchfg = ON;          /* Added by Jim Roung                 */
         fepcb->imode.ind5 = ERROR1;
         break;
   }

   PhShowCursor(fepcb);
   return;
}

/******************************************************************************/
/* FUNCTION    : PhShowCursor                                                 */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
PhShowCursor(fepcb)         /*    by   terry     */
FEPCB *fepcb;
{
   if (fepcb->eccrpsch == ON)         /*  for  Showing  Cursor   */
   {
      memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
      if (fepcb->echocrps < fepcb->echoacsz)
      {
         memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
         fepcb->echobufa[fepcb->echocrps] = 1;
         fepcb->echobufa[fepcb->echocrps+1] = 1;
/*       fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
         fepcb->echobufa[fepcb->echocrps+1] |= UNDERLINE_ATTR;  */
         fepcb->echochfg.flag = ON;
         fepcb->echochfg.chtoppos = 0;
         fepcb->echochfg.chlenbytes = 2;
      }
   }
}

/******************************************************************************/
/* FUNCTION    : PhInitial                                                    */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{

   PhEraseAllRadical(fepcb);           /* Erase All Radicals At Over_The_Spot*/

                                       /* Clear Edit_End Buffer              */
   (void)memset(fepcb->edendbuf, NULL, fepcb->echosize);

   fepcb->edendacsz = 0;
   fepcb->auxchfg = OFF;
   fepcb->auxuse = NOTUSE;
   fepcb->auxcrpsch = OFF;
   fepcb->auxacsz.itemsize = 0;
   fepcb->auxacsz.itemlen =  0;                            /* @big5   */
   fepcb->auxacsz.itemnum = 0;
   fepcb->indchfg = ON;
   fepcb->imode.ind0 = PHONETIC_MODE;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->imode.ind4 = REPLACE_MODE;
   fepcb->imode.ind5 = BLANK;
   fepcb->isbeep = BEEP_OFF;
   fepcb->inputlen = 0;
   fepcb->starpos = 0;
}

/******************************************************************************/
/* FUNCTION    : PhKeyToRadical                                               */
/* DESCRIPTION : Convert key to phonetic radical if it is in phonetic radical */
/*               range.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key                                                          */
/* OUTPUT      : phcode                                                       */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

/* unsigned char ph_radical[] = { 0xa5,0xd0, 0xa5,0xc7, 0xa5,0xc8, 0xa5,0xc9,
                               0xa5,0xca, 0xa5,0xcb, 0xa5,0xcc, 0xa5,0xcd,
                               0xa5,0xce, 0xa5,0xcf, 0xa5,0xe9, 0xa5,0xe7,
                               0xa5,0xe5, 0xa5,0xeb, 0xa5,0xd4, 0xa5,0xdc,
                               0xa5,0xdd, 0xa5,0xde, 0xa5,0xd9, 0xa5,0xdf,
                               0xa5,0xe0, 0xa5,0xe1, 0xa5,0xee, 0xa5,0xe8,
                               0xa5,0xda, 0xa5,0xdb, 0xa5,0xd2, 0xa5,0xd5,
                               0xa5,0xea, 0xa5,0xd6, 0xa5,0xd8, 0xa5,0xe6,
                               0xa5,0xd3, 0xa5,0xe4, 0xa5,0xd7, 0xa5,0xe3,
                               0xa5,0xd1, 0xa5,0xe2, 0xa5,0xef, 0xa5,0xf0,
                               0xa5,0xec };                       @big5 */

/*--------------support 101 keyboard layout ------------------------@big5----*/
/*                                0          1          2          3     */
unsigned char ph_IBM_radical[] = { 0xa5,0xd0, 0xa5,0xc7, 0xa5,0xc8, 0xa5,0xc9,
/*                                4          5          6          7     */
                               0xa5,0xca, 0xa5,0xcb, 0xa5,0xcc, 0xa5,0xcd,
/*                                8          9          a          b     */
                               0xa5,0xce, 0xa5,0xcf, 0xa5,0xe9, 0xa5,0xe7,
/*                                c          d          e          f     */
                               0xa5,0xe5, 0xa5,0xeb, 0xa5,0xd4, 0xa5,0xdc,
/*                                g          h          i          j     */
                               0xa5,0xdd, 0xa5,0xde, 0xa5,0xd9, 0xa5,0xdf,
/*                                k          l          m          n     */
                               0xa5,0xe0, 0xa5,0xe1, 0xa5,0xee, 0xa5,0xe8,
/*                                o          p          q          r     */
                               0xa5,0xda, 0xa5,0xdb, 0xa5,0xd2, 0xa5,0xd5,
/*                                s          t          u          v     */
                               0xa5,0xea, 0xa5,0xd6, 0xa5,0xd8, 0xa5,0xe6,
/*                                w          x          y          z     */
                               0xa5,0xd3, 0xa5,0xe4, 0xa5,0xd7, 0xa5,0xe3,
/*                                -          ;          ,          .     */
                               0xa5,0xd1, 0xa5,0xe2, 0xa5,0xef, 0xa5,0xf0,
/*                                //                                     */
                               0xa5,0xec };

/*                                0          1          2          3     */
unsigned char ph_101_radical[] = { 0xa5,0xe4, 0xa5,0xc7, 0xa5,0xcb, 0xa5,0xef,
/*                                4          5          6          7     */
                               0xa5,0xf0, 0xa5,0xd5, 0xa5,0xee, 0xa5,0xec,
/*                                8          9          a          b     */
                               0xa5,0xdc, 0xa5,0xe0, 0xa5,0xc9, 0xa5,0xd8,
/*                                c          d          e          f     */
                               0xa5,0xd1, 0xa5,0xd0, 0xa5,0xcf, 0xa5,0xd3,
/*                                g          h          i          j     */
                               0xa5,0xd7, 0xa5,0xda, 0xa5,0xdd, 0xa5,0xea,
/*                                k          l          m          n     */
                               0xa5,0xde, 0xa5,0xe2, 0xa5,0xeb, 0xa5,0xdb,
/*                                o          p          q          r     */
                               0xa5,0xe1, 0xa5,0xe5, 0xa5,0xc8, 0xa5,0xd2,
/*                                s          t          u          v     */
                               0xa5,0xcd, 0xa5,0xd6, 0xa5,0xe9, 0xa5,0xd4,
/*                                w          x          y          z     */
                               0xa5,0xcc, 0xa5,0xce, 0xa5,0xd9, 0xa5,0xca,
/*                                -          ;          ,          .     */
                               0xa5,0xe8, 0xa5,0xe6, 0xa5,0xdf, 0xa5,0xe3,
/*                                //                                     */
                               0xa5,0xe7 };

/*--------------support 101 keyboard layout ------------------------@et------*/

/*                                0          1          2          3     */
unsigned char ph_et_radical[] = { 0xa5,0xe6, 0xa5,0xec, 0xa5,0xee, 0xa5,0xef,
/*                                4          5          6          7     */
                               0xa5,0xf0, 0x0 ,0x0 , 0x0 ,0x0 , 0xa5,0xd3,
/*                                8          9          a          b     */
                               0xa5,0xe4, 0xa5,0xe5, 0xa5,0xdc, 0xa5,0xc7,
/*                                c          d          e          f     */
                               0xa5,0xd4, 0xa5,0xcb, 0xa5,0xe9, 0xa5,0xca,
/*                                g          h          i          j     */
                               0xa5,0xd2, 0xa5,0xd1, 0xa5,0xe0, 0xa5,0xd8,
/*                                k          l          m          n     */
                               0xa5,0xd0, 0xa5,0xce, 0xa5,0xc9, 0xa5,0xcd,
/*                                o          p          q          r     */
                               0xa5,0xdd, 0xa5,0xc8, 0xa5,0xe1, 0xa5,0xde,
/*                                s          t          u          v     */
                               0xa5,0xdb, 0xa5,0xcc, 0xa5,0xeb, 0xa5,0xcf,
/*                                w          x          y          z     */
                               0xa5,0xdf, 0xa5,0xea, 0xa5,0xe3, 0xa5,0xe2,
/*                                -          ;          ,          .     */
                               0xa5,0xe7, 0xa5,0xd9, 0xa5,0xd5, 0xa5,0xd6,
/*                                //          :         ^                */
                               0xa5,0xd7, 0xa5,0xda, 0xa5,0xe8 };
/*--------------support Etan keyboard layout ------------------------@et------*/

PhKeyToRadical(fepcb,key, phcode)
FEPCB           *fepcb;                /* FEP Control Block        @big5     */
unsigned short  key;                   /* Ascii Code                         */
unsigned char   *phcode;               /* Phonetic Radical (EUC Code)        */
{
   unsigned char   *codeptr;           /* Pointer To Phonetic Code           */
   unsigned short  locate;             /* Mapping Location                   */
   unsigned short  i;                  /* Loop Counter                       */
   unsigned char   *rdptr;             /* Pointer To Radical Mapping Table   */
   unsigned char   *ph_radical;        /*                           @big5    */

   if (fepcb->keylayout == IBM_LAYOUT)                          /*  @big5    */
     ph_radical = (unsigned char *)ph_IBM_radical;              /*  @big5    */
   else if (fepcb->keylayout == NON_IBM_LAYOUT)                 /*  @et      */
     ph_radical = (unsigned char *)ph_101_radical;              /*  @big5    */
   else if (fepcb->keylayout == ET_LAYOUT)                      /*  @et      */
     ph_radical = (unsigned char *)ph_et_radical;               /*  @et      */

   codeptr = phcode;
   rdptr = (unsigned char *)ph_radical;
   for( i=0; i<3; i++ )                /* Clear Phonetic Code Buffer         */
      *(codeptr+i) = NULL;
                                       /* Caculate Mapping Location          */
  /* if( key == 0x7e )                     A
      locate = 0;
   else if( key <= 0x39 && key >= 0x30)*/
   if( key <= 0x39 && key >= 0x30)
      locate = (key-0x30)*2;
   else if( key <= 0x29 && key >= 0x21 )
      locate = (key-0x20)*2;
   else if( key <= 0x5a && key >= 0x41 )
      locate = (key-0x41+10)*2;
   else if( key <= 0x7a && key >= 0x61 )
      locate = (key-0x61+10)*2;
   else if( key == 0x3d || key == 0x2d )
      locate = (10+26)*2;
   else if( key == 0x2b || key == 0x3b )
      locate = (10+26+1)*2;
   else if( key == 0x3c || key == 0x2c )
      locate = (10+26+2)*2;
   else if( key == 0x3e || key == 0x2e )
      locate = (10+26+3)*2;
   else if( key == 0x3f || key == 0x2f )
      locate = (10+26+4)*2;
   else if( key == 0x2a || key == 0x3a ) /* support etan keylayout : @et */
      locate = (10+26+5)*2;
   else if( key == 0x5e || key == 0x7e ) /* support etan keylayout ^ @et */
      locate = (10+26+6)*2;
   else
      return;                          /* Key Is Not In Phinetic Code Range  */

   for( i=locate; i<locate+2; i++ )    /* Get Phonetic Code                  */
        *codeptr++ = *(rdptr+i);
}


/******************************************************************************/
/* FUNCTION    : PhEraseAllRadical                                            */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhEraseAllRadical(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
                                       /* Clear Echo Buffer                  */
   (void)memset(fepcb->echobufs, NULL, fepcb->echosize);
   (void)memset(fepcb->echobufa, NULL, fepcb->echosize);

   fepcb->echochfg.flag = TRUE;
   fepcb->echochfg.chtoppos = 0;
   fepcb->echochfg.chlenbytes = fepcb->echoacsz;
   fepcb->echocrps = 0;
   fepcb->eccrpsch = ON;
   fepcb->echoacsz = 0;
   fepcb->echoover = fepcb->echosize;
   fepcb->inputlen = 0;
}

/******************************************************************************/
/* FUNCTION    : PhCandidates                                                 */
/* DESCRIPTION : Find satisfied candidates.                                   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer      V410   */

   AccessDictionary(fepcb);            /* To Find Satisfied Candidates And   */
                                       /* To Show Candidate List Box         */

   if( fepcb->ret == FOUND_CAND )
   {
                                       /* Clear Ctrl_Right_Buf               */
      (void)memset(fepcb->ctrl_r_buf, NULL, strlen(fepcb->ctrl_r_buf));

                                       /* Copy Echo Buffer to Ctrl_Right_Buf */
      for( i=0; i<fepcb->echoacsz; i++ )
        *(fepcb->ctrl_r_buf+i) = *(fepcb->echobufs+i);

      if (fepcb->phstruct.allcandno == 1)   /*  only one candidate       */
      {                                     /*  by  terry                */
         itemsize = 0;
         memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
         ptr = fepcb->edendbuf;
         candptr = fepcb->phstruct.phcand;

         do {                             /* Fill Phrase                */
         itemsize ++;
         *ptr++ = (*candptr | 0x80);
         }  while( (*candptr++ & 0x80) != NULL );

         if (fepcb->Lang_Type != codesetinfo[0].code_type)        /* @big5 */
         {                                                        /* @big5 */
            itemsize ++;                                          /* @big5 */
            *ptr++ = *candptr++;                                  /* @big5 */
         }                                                        /* @big5 */
         fepcb->edendacsz = itemsize;
         fepcb->ret = FOUND_WORD;
         PhEraseAllRadical(fepcb);
         return;
      }
      if (fepcb->learning)                                   /*  @V410 */
      {                                                      /*  @V410 */
        echobufptr = (unsigned char *)fepcb->echobufs;       /*  @V410 */
        for( i=0; i<fepcb->echoacsz/2; i++ )                 /*  @V410 */
        {                                                    /*  @V410 */
          fepcb->curinpbuf[i] = *(++echobufptr);             /*  @V410 */
          echobufptr ++;                                     /*  @V410 */
        }                                                    /*  @V410 */
        access_learn_data(fepcb);                            /*  @V410 */
      }                                                      /*  @V410 */
      fepcb->imode.ind2 = PHONETIC_SELECT_ON;
      fepcb->auxchfg=ON;
      fepcb->auxuse = USE;
      fepcb->phstruct.curptr = fepcb->phstruct.phcand;
      fepcb->phstruct.more = fepcb->phstruct.allcandno;

      PhListBox(fepcb, BEGINNING);     /* Fill Candidates To Aux. Buffer     */
   }
   else
   {
      fepcb->indchfg = ON;
      fepcb->imode.ind5 = ERROR2;      /* Display Error Message              */
      fepcb->isbeep = ON;
      fepcb->echocrps = 0;
      fepcb->eccrpsch = ON;
   }
}

/******************************************************************************/
/* FUNCTION    : PhRadicalInput                                               */
/* DESCRIPTION : Replace/Insert raidcal to echo buffer.                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, phcode                                                */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhRadicalInput(fepcb, phcode)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned char   *phcode;               /* Phonetic Radical (EUC Code)        */
{
   short           i;                  /* Loop Counter                       */
   unsigned char   *codeptr;           /* Pointer To Phonetic Radical        */

   codeptr = phcode;                   /* Point To Phonetic Radical          */

   if( fepcb->imode.ind4 == REPLACE_MODE )
   {
      if( (fepcb->echocrps/2 < 14 && fepcb->inputlen == 14)
          || (fepcb->inputlen <14) )                  /*  by terry */
      {
                                       /* Replace Radical To Echo Buffer     */
         *(fepcb->echobufs+fepcb->echocrps) = *codeptr++;
         *(fepcb->echobufs+fepcb->echocrps+1) = *codeptr;
         *(fepcb->echobufa+fepcb->echocrps) = REVERSE_ATTR;
         *(fepcb->echobufa+fepcb->echocrps+1) = REVERSE_ATTR;

         fepcb->echochfg.flag = ON;  /* Update Internal Informations       */
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = 2;

         if( fepcb->echocrps >= fepcb->echoacsz )
         {
            fepcb->inputlen ++;
            fepcb->echoacsz += 2;
            fepcb->echoover -= 2;
         }
         fepcb->echocrps += 2;
         fepcb->eccrpsch = ON;
      }
      else
      {
        fepcb->isbeep = BEEP_ON;       /* Notify FEP To Beep To Alarm        */
        fepcb->indchfg = ON;           /* Added by Jim Roung => error msg    */
        fepcb->imode.ind5 = ERROR1;
      }
   }
   else  /* Insert Mode */
   {
      if( (fepcb->echocrps/2 < 14 && fepcb->inputlen == 14)
          || (fepcb->inputlen <14) )                       /*  by terry */
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+1; i>=fepcb->echocrps+2; i-- )
         {
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-2);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-2);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fepcb->echobufs+fepcb->echocrps) = *codeptr++;
         *(fepcb->echobufs+fepcb->echocrps+1) = *codeptr;

         if (fepcb->inputlen < 14)          /*  by  terry  */
         {
            fepcb->echoacsz += 2;
            fepcb->inputlen++;
         }
                                    /* Update Internal Informations       */
         fepcb->echochfg.flag = ON;
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
         fepcb->echocrps += 2;
         fepcb->eccrpsch = ON;
         fepcb->echoover -= 2;
      }
      else
      {
         fepcb->isbeep = BEEP_ON;       /* Notify FEP To Beep To Alarm        */
         fepcb->indchfg = ON;
         fepcb->imode.ind5 = ERROR1;
      }
   }
}

/******************************************************************************/
/* FUNCTION    : PhCopyLastRadical                                            */
/* DESCRIPTION : Replace/Insert previous radical buffer to echo buffer.       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhCopyLastRadical(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           len;                /* Length Of Ctrl_Right_Buf           */
   short           i;                  /* Loop Counter                       */

   len = strlen(fepcb->ctrl_r_buf);

   if( fepcb->imode.ind4 == REPLACE_MODE )
   {                                                 /*  by  terry  */
      if( (fepcb->echocrps/2+len/2 <= 14) || (fepcb->inputlen+len/2 <= 14) )
      {
         for( i=0; i<len; i++ )        /* Replace Ctrl_Right_Buf To Echo Buf.*/
         {
           *(fepcb->echobufs+fepcb->echocrps+i) = *(fepcb->ctrl_r_buf+i);
           *(fepcb->echobufa+fepcb->echocrps+i) = REVERSE_ATTR;
         }
         fepcb->echochfg.flag = ON;  /* Update Internal Informations       */
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = len;

         if( fepcb->echocrps >= fepcb->echoacsz )
         {
            fepcb->inputlen += len/2;
            fepcb->echoacsz += len;
            fepcb->echoover -= len;
         }
         else if( fepcb->echocrps+len >= fepcb->echoacsz )
              {
                 fepcb->inputlen += (fepcb->echocrps+len-fepcb->echoacsz)/2;
                 fepcb->echoover -= (fepcb->echocrps+len-fepcb->echoacsz);
                 fepcb->echoacsz = fepcb->echocrps+len;
              }

         fepcb->echocrps += len;
         fepcb->eccrpsch = ON;
      }
      else
      {
         fepcb->isbeep = BEEP_ON;
         fepcb->indchfg = ON;
         fepcb->imode.ind5 = ERROR1;
      }
   }
   else  /* Insert Mode */
   {                                           /*  by  terry  */
      if( (fepcb->echocrps/2+len/2 <= 14) || (fepcb->inputlen+len/2 <= 14) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+len+1; i>=fepcb->echocrps+len; i-- )
         {                                          /*   by terry  */
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-len);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-len);
         }

         for( i=0; i<len; i++ )        /* Insert Ctrl_Right_Buf To Echo Buf. */
           *(fepcb->echobufs+fepcb->echocrps+i) = *(fepcb->ctrl_r_buf+i);

         fepcb->inputlen += len/2;        /*      by terry      */
         fepcb->echoacsz += len;
         if (fepcb->inputlen > 14)
         {
             fepcb->inputlen = 14;        /*  update echo buffer lenght */
             fepcb->echoacsz = 14*2;
         }

         fepcb->echochfg.flag = ON;  /* Update Internal Informations       */
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
         fepcb->echocrps += len;
         fepcb->eccrpsch = ON;
         fepcb->echoover -= len;
      }
      else
      {
         fepcb->isbeep = BEEP_ON;       /* Notify FEP To Beep To Alarm        */
         fepcb->indchfg = ON;
         fepcb->imode.ind5 = ERROR1;
      }
   }
}

/******************************************************************************/
/* FUNCTION    : PhSelect                                                     */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhSelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   switch( key )
   {
      case PGUP_KEY :
             PhListBox(fepcb, key);    /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_KEY :
             PhListBox(fepcb, key);    /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case ESC_KEY :
             PhEraseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             PhShowCursor(fepcb);
             break;

      case NUM_KEY0 :                  /* Select A Candidate                 */
      case NUM_KEY1 :
      case NUM_KEY2 :
      case NUM_KEY3 :
      case NUM_KEY4 :
      case NUM_KEY5 :
      case NUM_KEY6 :
      case NUM_KEY7 :
      case NUM_KEY8 :
      case NUM_KEY9 :
             if (fepcb->learning)                                    /* V410 */
                PhUpdateLearningData(fepcb,key);                     /* V410 */
             PhGetCandidate(fepcb, key);/* Send Selected Candidate To Output */
                                        /* Buffer                            */
             break;

      default :
             fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm        */
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             break;
  }
}

/******************************************************************************/
/* FUNCTION    : PhListBox                                                    */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhListBox(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code (PGUP_KEY/PGDN_KEY)     */
{
   unsigned char   *candptr;           /* Pointer To Candidates              */
   unsigned char   **toauxs;           /* Pointer To Aux. String             */
   unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
   unsigned char   *tostring;          /* Pointer To Aux. String             */
   unsigned char   *toattribute;       /* Pointer To Aux. Attribute          */
   unsigned short  item;               /* Item Number                        */
   unsigned short  itemsize;           /* Item Size                          */
   unsigned short  no[3];              /* Buffer Of More Number              */
   unsigned short  i, j;               /* Loop Counter                       */
   static tempno;
   char buffer[4];
   int len;
   unsigned char *temp;
   unsigned short k,m;
   static unsigned short ph_dictionary_index,ph_learn_index;            /* V410 */
   static unsigned short pre_ph_dictionary_index,aft_ph_dictionary_index;  /* V410 */
   unsigned short  list_no;                                             /* V410 */

/*   static char title[]={0xce,0xc3,0xd3,0xf6,0xf2,0xd9,0xf0,0xe5,0x20,0x20
                        ,0x20,0x20,0x20,0x20,0xde,0xb7,0xef,0xf5};  @big5 */

/*   static char line[]={0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                      0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                      0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                      0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1  }; @big5*/

/*   static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
              0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
              0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
              0x20,0x39,0x20,0x20};          @big5 */

/*   static char bottom1[]={0xcc,0xbd,0xd6,0xbc,0x28,0x45,0x73,0x63,0x29}; @big5*/

/*   static char bottom2[]={0xd0,0xa9,0xd3,0xf7,0x28,0x50,0x67,0x55,0x70,
                          0x29,0x20,0x20,0xc8,0xb9,0xd3,0xf7,0x28,0x50,0x67,
                          0x44,0x6e,0x29};                                  @big5 */


   candptr = fepcb->phstruct.curptr;

   if (fepcb->learning)                                                /* V410 */
   {                                                                   /* V410 */
     switch( key )                                                     /* V410 */
     {                                                                 /* V410 */
        case PGUP_KEY:                                                 /* V410 */

              if ((ph_learn_index == 10) || (fepcb->phstruct.allcandno < 10))   /* V410 */
              {                                                        /* V410 */
                 fepcb->isbeep = BEEP_ON;                              /* V410 */
                 fepcb->indchfg = ON;                                  /* V410 */
                 fepcb->imode.ind5 = ERROR1;                           /* V410 */
                 return;                                               /* V410 */
              }                                                        /* V410 */
              else                                                     /* V410 */
              {                                                        /* V410 */
                 if ((fepcb->phstruct.allcandno - fepcb->phstruct.more )>=20) /* V410 */
                 {                                                     /* V410 */
                    if (((ph_learn_index - 20) < LEARN_NO)
                    && (ph_learn_index -20 >= 0) )                      /* V410 */
                    {                     /* get order from Learn file    V410 */
                      for (i=0;i<LIST_NO ;i++ )                        /* V410 */
                      {                                                /* V410 */
                        fepcb->learnstruct.list_index[i] =             /* V410 */
                           fepcb->learnstruct.index[i+ph_learn_index-20]; /* V410 */
                       } /* endfor */                                  /* V410 */
                       pre_ph_dictionary_index = 0;                    /* V410 */
                       aft_ph_dictionary_index = 0;                    /* V410 */
                       fepcb->phstruct.more += 10 ;                    /* V410 */
                    }                                                  /* V410 */
                    else                                               /* V410 */
                    {                     /* get order from dictionary    V410 */
                       k=9;                                            /* V410 */
                       aft_ph_dictionary_index = pre_ph_dictionary_index;
                       for (ph_dictionary_index=pre_ph_dictionary_index-1; /* V410 */
                       (int) ph_dictionary_index>=0;                    /* V410 */
                       ph_dictionary_index-- )                          /* V410 */
                       {                                                /* V410 */
                         for (j=0;j<LEARN_NO ;j++ )                     /* V410 */
                           if (ph_dictionary_index ==
                                        fepcb->learnstruct.index[j])    /* V410 */
                            break;                                      /* V410 */
                         if (j == LEARN_NO)                             /* V410 */
                         {
                            fepcb->learnstruct.list_index[k] =
                                                   ph_dictionary_index; /* V410 */
                            if ((int) k == 0 )                             /* V410 */
                              break;                                      /* V410 */
                            k--;
                         }
                       } /* endfor */                                   /* V410 */
                       pre_ph_dictionary_index = ph_dictionary_index;   /* V410 */
                       if (fepcb->phstruct.more == 0)                   /* V410 */
                          fepcb->phstruct.more = tempno;                /* V410 */
                       else                                             /* V410 */
                          fepcb->phstruct.more += 10;                   /* V410 */
                    }                                                   /* V410 */
                    ph_learn_index -= 10;                              /* V410 */
                 }                                                     /* V410 */
                 else                                                  /* V410 */
                 {                                                     /* V410 */
                    if ((int)(fepcb->phstruct.allcandno - ph_learn_index) <= 10)  /* V410 */
                    {                    /* get order from Learn file    V410 */
                      for (i=0;i<LIST_NO;i++ )                         /* V410 */
                      {                                                /* V410 */
                        fepcb->learnstruct.list_index[i] =             /* V410 */
                           fepcb->learnstruct.index[i];                /*V410 */
                       } /* endfor */                                  /* V410 */
                       pre_ph_dictionary_index = 0;                    /* V410 */
                       aft_ph_dictionary_index = 0;                    /* V410 */
                    }                                                  /* V410 */
                    fepcb->phstruct.more = fepcb->phstruct.allcandno-10;/* V410 */
                    ph_learn_index = 10;                               /* V410 */
                 }                                                     /* V410 */
                 list_no = 10 ;                                        /* V410 */
              }                                                        /* V410 */
           break;                                                      /* V410 */

        case PGDN_KEY:                                                 /* V410 */

              if (fepcb->phstruct.more <= 0 )                                           /* V410 */
              {                                                        /* V410 */
                 fepcb->isbeep = BEEP_ON;                              /* V410 */
                 fepcb->indchfg = ON;                                  /* V410 */
                 fepcb->imode.ind5 = ERROR1;                           /* V410 */
                 return;                                               /* V410 */
              }                                                        /* V410 */
              else                                                     /* V410 */
              {                                                        /* V410 */
                 if (fepcb->phstruct.more > 10)                        /* V410 */
                 {                                                     /* V410 */
                    if ((ph_learn_index + 10) <= LEARN_NO)             /* V410 */
                    {                     /* get order from Learn file    V410 */
                      for (i=0;i<LIST_NO ;i++ )                        /* V410 */
                      {                                                /* V410 */
                        fepcb->learnstruct.list_index[i] =             /* V410 */
                           fepcb->learnstruct.index[i+ph_learn_index]; /* V410 */
                       } /* endfor */                                  /* V410 */
                       pre_ph_dictionary_index = 0;                    /* V410 */
                       aft_ph_dictionary_index = 0;                    /* V410 */
                    }                                                  /* V410 */
                    else                                               /* V410 */
                    {                     /* get order from dictionary    V410 */
                       k=0;                                             /* V410 */
                       pre_ph_dictionary_index = aft_ph_dictionary_index;/* V410 */
                       for (ph_dictionary_index=aft_ph_dictionary_index;/* V410 */
                       ph_dictionary_index<=fepcb->phstruct.allcandno; /* V410 */
                       ph_dictionary_index++ )                          /* V410 */
                       {                                                /* V410 */
                         for (j=0;j<LEARN_NO ;j++ )                     /* V410 */
                           if (ph_dictionary_index ==
                                        fepcb->learnstruct.index[j])    /* V410 */
                            break;                                      /* V410 */
                         if (j == LEARN_NO)                             /* V410 */
                            fepcb->learnstruct.list_index[k++] =
                                                   ph_dictionary_index; /* V410 */
                         if (k == LIST_NO)                              /* V410 */
                            break;                                      /* V410 */
                       } /* endfor */                                   /* V410 */
                       aft_ph_dictionary_index = ph_dictionary_index+1; /* V410 */
                    }                                                  /* V410 */
                    fepcb->phstruct.more -= 10;                        /* V410 */
                    ph_learn_index += 10;                              /* V410 */
                    list_no = 10;                                      /* V410 */
                 }                                                     /* V410 */
                 else                                                  /* V410 */
                 {               /* fepcb->phstruct.more < 10             V410 */
                    if ((ph_learn_index + fepcb->phstruct.more) <= LEARN_NO)   /* V410 */
                    {                    /* get order from Learn file    V410 */
                      for (i=0;i<fepcb->phstruct.more;i++ )            /* V410 */
                      {                                                /* V410 */
                        fepcb->learnstruct.list_index[i] =             /* V410 */
                           fepcb->learnstruct.index[i+ph_learn_index]; /*V410 */
                       } /* endfor */                                  /* V410 */
                    }                                                  /* V410 */
                    else                                               /* V410 */
                    {                     /* get order from dictionary    V410 */
                       k=0;                                             /* V410 */
                       pre_ph_dictionary_index = aft_ph_dictionary_index;/* V410 */
                       for (ph_dictionary_index=aft_ph_dictionary_index;  /* V410 */
                       ph_dictionary_index<=fepcb->phstruct.allcandno ; /* V410 */
                       ph_dictionary_index++ )                          /* V410 */
                       {                                                /* V410 */
                         for (j=0;j<LEARN_NO ;j++ )                     /* V410 */
                           if (ph_dictionary_index ==
                                        fepcb->learnstruct.index[j])    /* V410 */
                            break;                                      /* V410 */
                         if (j == LEARN_NO)                             /* V410 */
                            fepcb->learnstruct.list_index[k++] =
                                                   ph_dictionary_index; /* V410 */
                         if (k == fepcb->phstruct.more)                 /* V410 */
                            break;                                      /* V410 */
                       } /* endfor */                                   /* V410 */
                       aft_ph_dictionary_index = ph_dictionary_index+1;   /* V410 */
                    }                                                   /* V410 */
                    tempno = fepcb->phstruct.more;                      /* V410 */
/*                  ph_learn_index += fepcb->phstruct.more;                V410 */
                    ph_learn_index += 10;                               /* V410 */
                    list_no = fepcb->phstruct.more;                     /* V410 */
                    fepcb->phstruct.more = 0;                           /* V410 */
                 }                                                     /* V410 */
              }                                                        /* V410 */

           break;                                                       /* V410 */

        default:                       /* Beginning Of The Selection      V410  */

           if (fepcb->phstruct.curptr == fepcb->phstruct.phcand)       /* V410 */
              if (fepcb->phstruct.more >=10)                           /* V410 */
              {                                                        /* V410 */
                 for (i=0;i<LIST_NO ;i++ )                             /* V410 */
                 {                                                     /* V410 */
                     fepcb->learnstruct.list_index[i] =                /* V410 */
                                    fepcb->learnstruct.index[i];       /* V410 */
                 } /* endfor */                                        /* V410 */
                 fepcb->phstruct.more -= 10;                           /* V410 */
                 ph_learn_index = 10;                                  /* V410 */
                 ph_dictionary_index =0 ;                              /* V410 */
                 list_no = 10 ;                                        /* V410 */
              }                                                        /* V410 */
              else                                                     /* V410 */
              {                                                        /* V410 */
                 for (i=0;i<fepcb->phstruct.allcandno;i++ )            /* V410 */
                 {                                                     /* V410 */
                     fepcb->learnstruct.list_index[i] =                /* V410 */
                                    fepcb->learnstruct.index[i];       /* V410 */
                 } /* endfor */                                        /* V410 */
                 tempno = fepcb->phstruct.more;                        /* V410 */
                 fepcb->phstruct.more = 0;                             /* V410 */
                 ph_learn_index = fepcb->phstruct.allcandno;           /* V410 */
                 ph_dictionary_index = 0;                              /* V410 */
                 pre_ph_dictionary_index = 0;                          /* V410 */
                 list_no = fepcb->phstruct.allcandno;                  /* V410 */
              }                                                        /* V410 */
           len = Ph_title_LEN+4;   /* Length Of Title & Bottom Of Cand.V410 @big5 */
           fepcb->auxacsz.itemsize = len;                              /* V410 */
           fepcb->auxacsz.itemlen =  len;                              /* @big5   */
           PhCalItemSize(fepcb);     /* Caculate Maximum Size Of All Cand. V410 */
           break;                                                      /* V410 */
     }                                                                 /* V410 */
   }                                                                   /* V410 */
   else                                                                /* V410 */
   {                                                                   /* V410 */
     switch( key )
     {
        case PGUP_KEY:
           if( fepcb->phstruct.curptr == fepcb->phstruct.phcand )
           {
               fepcb->isbeep = BEEP_ON;              /*   by terry  */
               fepcb->indchfg = ON;
               fepcb->imode.ind5 = ERROR1;
               return;
           }
           else
           {
              if ( (fepcb->phstruct.allcandno - fepcb->phstruct.more) <= 20 )
              {      /* <=20 , original value < 10 , !!!!! Benson Lu !!!!! */
                 fepcb->phstruct.curptr = fepcb->phstruct.phcand;
                 candptr = fepcb->phstruct.phcand;
                 fepcb->phstruct.more = fepcb->phstruct.allcandno-10 ;
              }
              else
              {
                   if (fepcb->Lang_Type != codesetinfo[0].code_type)   /* @big5 */
                      candptr--;                                       /* @big5 */

                 for( i=0; i<11; )
                 {

                    if( (*(--candptr) & 0x80) == NULL )
                    {
                      if (fepcb->Lang_Type != codesetinfo[0].code_type) /* @big5 */
                        candptr--;                                      /* @big5 */
                      i++;
                    }
                 }


              /*----------   Benson Lu Mark it ---------------------------

                 candptr = fepcb->phstruct.phcand;
                 --candptr;

               -------------------------------------------------------------  */

                   fepcb->phstruct.curptr = ++candptr;

                 if (fepcb->Lang_Type != codesetinfo[0].code_type) /* @big5 */
                 {                                                 /* @big5 */
                   fepcb->phstruct.curptr = ++candptr;             /* @big5 */
                   fepcb->phstruct.curptr = ++candptr;             /* @big5 */
                 }                                                 /* @big5 */

                 if ( fepcb->phstruct.more == 0 )
                     fepcb->phstruct.more = tempno;
                 else
                     fepcb->phstruct.more += 10;
              }

           }
           break;

        case PGDN_KEY:

           if( fepcb->phstruct.more > 0 )
           {
              for( i=0; i<10 && *candptr!=PH_GP_END_MARK; )
              {
                 if( (*(candptr++) & 0x80) == NULL )
                 {
                   if ((fepcb->Lang_Type != codesetinfo[0].code_type) && /* @big5 */
                     (*candptr!=PH_GP_END_MARK) )                        /* @big5 */
                      candptr++;                                         /* @big5 */
                   i++;
                 }
              }

              fepcb->phstruct.curptr = candptr;

              if( fepcb->phstruct.more > 10 )       /*   by  terry   */
                fepcb->phstruct.more -= 10;
              else
              {
                tempno = fepcb->phstruct.more;
                fepcb->phstruct.more = 0;
              }
           }
           else
           {
               fepcb->isbeep = BEEP_ON;              /*   by terry  */
               fepcb->indchfg = ON;
               fepcb->imode.ind5 = ERROR1;
               return;
           }
           break;

        default:                       /* Beginning Of The Selection         */

              if( fepcb->phstruct.more >= 10 )
                 fepcb->phstruct.more -= 10;
              else
              {
                 tempno = fepcb->phstruct.more;
                 fepcb->phstruct.more = 0;
               }                                        /*  by terry    */
           len = Ph_title_LEN+4;   /* Length Of Title & Bottom Of Cand. @big5 */
           fepcb->auxacsz.itemsize = len;
           fepcb->auxacsz.itemlen =  len;                          /* @big5   */
           PhCalItemSize(fepcb);        /* Caculate Maximum Size Of All Cand. */
           break;
     }
   }                                                                   /* V410 */
   toauxs = fepcb->auxbufs;
   toauxa = fepcb->auxbufa;
                                                   /*  clear aux buffer   */
   if (fepcb->auxacsz.itemnum != 0)                /*   by terry     */
      for( i = 0 ; i< fepcb->auxacsz.itemnum-2; i++ )
/*        memset( *toauxs++,' ',fepcb->auxacsz.itemsize+1);       @big5 */
          memset( *toauxs++,' ',fepcb->auxacsz.itemlen+1); /*     @big5 */

   toauxs = fepcb->auxbufs;
   toauxa = fepcb->auxbufa;

/* for(k = 0; k < 14 ; k++)    Added by Jim Roung                  @big5 */
/* {                                                               @big5 */
/*     temp = fepcb->auxbufs[k];                                   @big5 */
/*     for(m = 0; m < fepcb->auxacsz.itemsize; memcpy(temp," ",1), @big5 */
/*         temp++,m++);                                            @big5 */
/* }                        /* End of addition by Jim Roung        @big5 */
                                       /* Fill Title                      */
   memcpy(fepcb->auxbufs[0], Ph_title, Ph_title_LEN);   /*  by terry  @big5*/
   tostring = fepcb->auxbufs[0]+Ph_title_LEN;            /* @big5 */

   itoa(fepcb->phstruct.more, buffer, sizeof(buffer));  /*  by terry  */
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], Ph_line, "", fepcb->auxacsz.itemsize);   /* @big5 */

                                    /* Fill Candidates To Aux. Buffer     */

   if (fepcb->learning)                                              /* V410 */
   {                                                                 /* V410 */
      for (item=0;(item<10) && (item<list_no) ;item++ )              /* V410 */
      {                                                              /* V410 */
         itemsize = 0;                                               /* V410 */
         tostring = fepcb->auxbufs[item+2];                          /* V410 */
                                    /* Fill Number                      V410 */
         for( j=item*4; j<item*4+4; j++ )                            /* V410 */
         {                                                           /* V410 */
            itemsize ++;                                             /* V410 */
            *tostring++ = *(Ph_digit+j);                        /* V410 @big5*/
         }                                                           /* V410 */

         if (fepcb->Lang_Type == codesetinfo[0].code_type)           /* @big5 */
         {                                                           /* @big5 */
            candptr = fepcb->phstruct.phcand;                        /* V410 */
            for (i=0;i<fepcb->learnstruct.list_index[item]           /* V410 */
                  && *candptr != PH_GP_END_MARK;i++)                 /* V410 */
              while (( *candptr++ & 0x80) != NULL);                  /* V410 */
           if (item == 0)                                            /* V410 */
             fepcb->phstruct.curptr = candptr;                       /* V410 */
            do {                             /* Fill Phrase             V410 */
                itemsize ++;                                         /* V410 */
                 *tostring++ = (*candptr | 0x80);                    /* V410 */
            } while( (*candptr++ & 0x80) != NULL );                  /* V410 */
         }                                                           /* @big5*/
         else                                                        /* @big5*/
         {                                                           /* @big5*/
           candptr = fepcb->phstruct.phcand;                         /* @big5*/
           for (i=0;i<fepcb->learnstruct.list_index[item]            /* @big5*/
                    && *candptr != PH_GP_END_MARK;i++)               /* @big5*/
           {                                                         /* @big5*/
              while (( *candptr & 0x80) != NULL)                     /* @big5*/
                candptr = candptr +2;                                /* @big5*/
              candptr = candptr +2;                                  /* @big5*/
           }                                                         /* @big5*/
                                                                     /* @big5*/
           if (item == 0)                                            /* @big5*/
             fepcb->phstruct.curptr = candptr;                       /* @big5*/
           while ((*candptr & 0x80) != NULL )   /* Fill Phrase       /* @big5*/
           {                                                         /* @big5*/
              itemsize ++;                                           /* @big5*/
              *tostring++ = (*candptr | 0x80);                       /* @big5*/
              candptr++;                                             /* @big5*/
              itemsize ++;                                           /* @big5*/
              *tostring++ = *candptr ++ ;                            /* @big5*/
           }                                                         /* @big5*/
           itemsize++;                                               /* @big5*/
           *tostring++ = (*candptr | 0x80);                          /* @big5*/
           candptr++;                                                /* @big5*/
           itemsize ++;                                              /* @big5*/
           *tostring++ = *candptr++;                                 /* @big5*/
        }                                                            /* @big5*/
        if (itemsize > fepcb->auxacsz.itemlen)                       /* @big5 */
           fepcb->auxacsz.itemlen = itemsize ;                       /* @big5 */

      } /* endfor */                                                   /* V410 */
   }                                                                   /* V410 */
   else                                                                /* V410 */
   for( item=0; item<10 && *candptr!=PH_GP_END_MARK; item++ )
   {
      itemsize = 0;
      tostring = fepcb->auxbufs[item+2];
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(Ph_digit+j);                                /* @big5 */
      }

      if (fepcb->Lang_Type == codesetinfo[0].code_type)              /* @big5 */
      {
        do {                             /* Fill Phrase                        */
              itemsize ++;
              *tostring++ = (*candptr | 0x80);
           } while( (*candptr++ & 0x80) != NULL );
      }                                                              /* @big5 */
      else                                                           /* @big5 */
      {                                                              /* @big5 */
           while ((*candptr & 0x80) != NULL )   /* Fill Phrase       /* @big5 */
           {                                                         /* @big5 */
              itemsize ++;                                           /* @big5 */
              *tostring++ = (*candptr | 0x80);                       /* @big5 */
              candptr++;                                             /* @big5 */
              itemsize ++;                                           /* @big5 */
              *tostring++ = *candptr ++ ;                            /* @big5 */
           }                                                         /* @big5 */
           itemsize++;                                               /* @big5 */
           *tostring++ = (*candptr | 0x80);                          /* @big5 */
           candptr++;                                                /* @big5 */
           itemsize ++;                                              /* @big5 */
           *tostring++ = *candptr++;                                 /* @big5 */
      }                                                              /* @big5 */
      if (itemsize > fepcb->auxacsz.itemlen)                        /* @big5 */
         fepcb->auxacsz.itemlen = itemsize ;                        /* @big5 */
   }

                                            /* by terry  */
   memcpy(fepcb->auxbufs[AUXROWMAX-2], Ph_bottom1, Ph_bottom1_LEN);   /* @big5 */
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], Ph_bottom2, Ph_bottom2_LEN);   /* @big5 */

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}

/******************************************************************************/
/* FUNCTION    : PhCalItemSize                                                */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */


   candptr = fepcb->phstruct.phcand;
   pitemsize = 4;
   nitemsize = 4;                      /*      by  terry   */

   if ((fepcb->Lang_Type == codesetinfo[0].code_type))               /* @big5*/
     for( ;*candptr != NULL; candptr++ )
     {
        if (*candptr == EUC_BYTE1)           /*   by  terry  */
            nitemsize -= 2;
        nitemsize ++;

        if( (*candptr & 0x80) == NULL )
        {
           if( pitemsize < nitemsize )  pitemsize = nitemsize;
           nitemsize = 4;                   /*  by terry   */
        }
     }
   else                                                              /* @big5*/
   {                                                                 /* @big5*/
     while(*candptr != NULL)                                         /* @big5*/
     {                                                               /* @big5*/

        candptr = candptr +2;                                        /* @big5*/
        nitemsize = nitemsize +2;                                    /* @big5*/

        if( (*candptr & 0x80) == NULL )                              /* @big5*/
        {                                                            /* @big5*/
            candptr ++;                                              /* @big5*/
            nitemsize ++;                                            /* @big5*/
            nitemsize ++;                                            /* @big5*/
            if (*candptr != NULL)                                    /* @big5*/
              candptr ++;                                            /* @big5*/

           if( pitemsize < nitemsize )  pitemsize = nitemsize;       /* @big5*/
           nitemsize = 4;                   /*  by terry   */        /* @big5*/
        }                                                            /* @big5*/
                                                                     /* @big5*/
     }                                                               /* @big5*/
   }                                                                 /* @big5*/

   if( fepcb->auxacsz.itemsize < pitemsize )
     fepcb->auxacsz.itemsize = pitemsize;

}

/******************************************************************************/
/* FUNCTION    : PhGetCandidate                                               */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhGetCandidate(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned char  *ptr, *getptr;       /* Pointer To Selected Candidate      */
   unsigned short  getnum;             /* Selected Number                    */
   unsigned short  i;                  /* Loop Counter                       */
   int len=0;

   getnum = key-0x30;                  /* Caculate Selected Number           */
   getptr = fepcb->auxbufs[getnum+2];  /*  by  terry  */
   getptr = getptr +4;
                                       /* Copy Selected Candidate To Edit-End*/
                                       /* Buffer                             */
   ptr = getptr;
   if (*ptr == 0x20 || *ptr == NULL)                     /*   by  terry  */
   {
       fepcb->isbeep = BEEP_ON;
       fepcb->indchfg = ON;
       fepcb->imode.ind5 = ERROR1;
       return;
   }
   while (*ptr != 0x20 && *ptr != '\0')   /*  by terry  */
   {
      len++;
      ptr++;
   }
   strncpy(fepcb->edendbuf, getptr, len);   /* by terry  */
   fepcb->edendacsz = len;

   PhEraseAllRadical(fepcb);
   PhEraseAux(fepcb);

   fepcb->ret = FOUND_WORD;
}

/******************************************************************************/
/* FUNCTION    : PhEraseAux                                                   */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PhEraseAux(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   char            **toauxs;           /* Pointer To Aux. String Buffer      */
   char            **toauxa;           /* Pointer To Aux. Attribute Buffer   */
   int i;

   toauxs = fepcb->auxbufs;             /*  by terry  */
   toauxa = fepcb->auxbufa;
   for( i=0; i<AUXROWMAX; i++ )
   {
      (void)memset(*toauxs++, NULL, AUXCOLMAX);
      (void)memset(*toauxa++, NULL, AUXCOLMAX);
   }
   fepcb->auxacsz.itemnum = 0;
   fepcb->auxacsz.itemsize = 0;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->echocrps = 0;
   fepcb->eccrpsch = ON;
   fepcb->auxuse = NOTUSE;
   fepcb->auxchfg = ON;
}

/********************************************************************V410******/
/* FUNCTION    : PhUpdateLearningData                                V410     */
/* DESCRIPTION : Send Selected Candidate index to Learning index     V410     */
/* EXTERNAL REFERENCES:                                              V410     */
/* INPUT       : fepcb, key                                          V410     */
/* OUTPUT      : fepcb                                               V410     */
/* CALLED      :                                                     V410     */
/* CALL        :                                                     V410     */
/********************************************************************V410******/

PhUpdateLearningData(fepcb,key)                                   /* V410 */
FEPCB           *fepcb;                /* FEP Control Block          V410 */
unsigned short  key;                   /* Ascii Code                 V410 */
{                                                                 /* V410 */
   int no,i;                                                      /* V410 */
   unsigned short select_dictionary_index;                        /* V410 */
   unsigned short candidate_no,temp;                              /* V410 */
   unsigned char  *ptr, *getptr;    /* Pointer To Selected Candidate V410 */
   unsigned short  getnum;          /* Selected Number               V410 */

   getnum = key-0x30;                  /* Caculate Selected Number   V410 */
   getptr = fepcb->auxbufs[getnum+2];                            /*  V410 */
   getptr = getptr +4;                                           /*  V410 */
                                       /* Copy Selected Candidate To Edit-End*/
                                       /* Buffer                     V410 */
   ptr = getptr;                                                 /*  V410 */
   if (*ptr == 0x20 || *ptr == NULL)  /* invalid selected            V410 */
       return;                                                    /* V410 */

   no = key - 0x30;                                               /* V410 */
   select_dictionary_index = fepcb->learnstruct.list_index[no];   /* V410 */

   candidate_no = fepcb->phstruct.allcandno - fepcb->phstruct.more ; /* V410 */

   if (fepcb->phstruct.allcandno > 10 )                           /* V410 */
   {                                                              /* V410 */
     if (fepcb->phstruct.more == 0)                               /* V410 */
        candidate_no = candidate_no - (fepcb->phstruct.allcandno % 10) + no;
                                                                  /* V410 */
     else                                                         /* V410 */
        candidate_no = candidate_no - 10 + no ;                   /* V410 */
   }                                                              /* V410 */
   else                                                           /* V410 */
   {                                                              /* V410 */
     candidate_no = no ;                                          /* V410 */
   }                                                              /* V410 */

   if (select_dictionary_index != fepcb->learnstruct.index[0])    /* V410 */
   {                                                              /* V410 */
      if (candidate_no >= LEARN_NO)                               /* V410 */
         temp = LEARN_NO -1 ;                                     /* V410 */
      else                                                        /* V410 */
         temp = candidate_no;                                     /* V410 */
      for (i=temp;i>0 ;i-- )                                      /* V410 */
      {                                                           /* V410 */
          fepcb->learnstruct.index[i] = fepcb->learnstruct.index[i-1];  /* V410 */
      }                                                           /* V410 */
                                                                  /* V410 */
      fepcb->learnstruct.index[0] = select_dictionary_index;      /* V410 */
      update_learn_file(fepcb);                                   /* V410 */
   }                                                              /* V410 */
}                                                                 /* V410 */
