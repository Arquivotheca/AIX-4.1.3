static char sccsid[] = "@(#)09  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedic.c, libtw, bos411, 9428A410j 4/21/94 01:59:21";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: tedic.c
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
 *
 */

/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         TedIC                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Internal-Code Input Method source file        */
/*                                                                            */
/* FUNCTION:            IcMain : Entry Procedure for IC. Input Method         */
/*                                                                            */
/*                      IcProcess : State Transition Procedure                */
/*                                                                            */
/*                      IcInitial : Initial Internal-Code State               */
/*                                                                            */
/*                      IcRadicalInput : Input Procedure for IC. Radicals     */
/*                                                                            */
/*                      IcEraseCurrentRadical : Erase Internal-Code Radical   */
/*                                                                            */
/*                      IcEraseAllRadical : Erase All Internal-Code Radicals  */
/*                                                                            */
/*                      IcKeyToRadical : Translate Input ASCII to IC.         */
/*                                         Radical EUC Code                   */
/*                                                                            */
/*                      IcIsEucCodeRange : Test Input key is in Euc Code Range*/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                      IcIsRadical : Test whether input key can translate    */
/*                                      into radical.                         */
/*                                                                            */
/*                      IcTranslator : Convert Radical to Euc Code            */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Terry Chou                                            */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                  (1). April/17, 1991 By Jim Roung - IBM(Taiwan)            */
/*                                                                            */
/*                       * Modified IcIsRadical() -                           */
/*                         Let user can type in those following characters :  */
/*                         !,",#,$,%,&,',(,),~, in Internal_Code mode.        */
/*                                                                            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

#include "ted.h"
#include "tedinit.h"
#include "tedacc.h"
#include "msgextrn.h"             /* @big5 */

unsigned char *IcKeyToRadical();

/******************************************************************************/
/* FUNCTION    : IcMain                                                       */
/* DESCRIPTION : Entry of Internal-Code Input Method                          */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcMain(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   /***********************/
   /*  Test for PIMI key  */
   /***********************/

   switch( key )
   {
      case ALPH_NUM_KEY:
        AnInitial(fepcb);
        break;

      case TSANG_JYE_KEY:
        TjInitial(fepcb);
        break;

      case PHONETIC_KEY:
        PhInitial(fepcb);
        break;

      case INTERNAL_CODE_KEY:
        IcInitial(fepcb);
        break;

      case FULL_HALF_KEY :
        if ( fepcb->imode.ind1 == FULL )
           fepcb->imode.ind1 = HALF ;
        else
           fepcb->imode.ind1 = FULL ;
        fepcb->indchfg = ON ;
        break;

      default :                             /* go to Internal_Code input  */
             IcProcess(fepcb,key);          /* radical state              */
             break;
   }
   return;
}

/******************************************************************************/
/* FUNCTION    : IcProcess                                                    */
/* DESCRIPTION : Processing each key for Internal_Code Input Method.          */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcProcess(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   unsigned int outrange;                                 /* @big5 */
   /**********************************/
   /* process Internal_Code radicals */
   /**********************************/
   if ( IcIsRadical(&key) )
   {
      if ( fepcb->inputlen >= max_ic_input_len )           /* @big5 */
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

      /***********************************************/
      /*  If error message was displayed last time   */
      /***********************************************/

      if ( fepcb->echoacsz != 0 && fepcb->inputlen == 0 )
         IcEraseAllRadical(fepcb);   /* Erase error radical */

      IcRadicalInput(fepcb,key);     /* Input Internal_Code radical. */
      return;
   }

   /*****************************/
   /* Process special input key */
   /*****************************/

   switch( key )
   {
      case NON_CONVERT_KEY :
        return;                           /* No action                */

      case CONVERT_KEY :
        if ( fepcb->echoacsz == 0)
        {
           fepcb->indchfg = ON;         /* Added by Jim Roung           */
           fepcb->imode.ind5 = ERROR1;
           fepcb->isbeep = ON;
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
           if ((fepcb->inputlen+1)/2 == fepcb->inputlen/2)
           {
               memset(fepcb->preedbuf, NULL, strlen(fepcb->preedbuf));
               IcTranslator(fepcb->curinpbuf, fepcb->preedbuf);
/*             if (fepcb->Lang_Type == BIG5_CODE)                /* @big5 */
/*                outrange = IcIsBig5CodeRange(fepcb->preedbuf); /* @big5 */
/*             else                                              /* @big5 */
/*             if (fepcb->Lang_Type == EUC_CODE)                 /* @big5 */
/*                outrange = IcIsEucCodeRange(fepcb->preedbuf) ; /* @big5 */
/*             outrange = IcIsCodeRange(fepcb->preedbuf,fepcb->Lang_Type) ; */
               if ( IcIsCodeRange(fepcb->preedbuf,fepcb->Lang_Type) ) /* @big5 */
               {
                   memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
                   strcpy(fepcb->edendbuf, fepcb->preedbuf,
                                                strlen(fepcb->preedbuf));
                   fepcb->edendacsz = strlen(fepcb->edendbuf);
                   IcEraseAllRadical(fepcb);
                   fepcb->ret = FOUND_WORD;
                   return;
               }
           }

           fepcb->indchfg = ON ;            /*  the word not found  */
           fepcb->imode.ind5 = ERROR2;
           fepcb->echocrps = 0;
           fepcb->eccrpsch = ON;
           fepcb->echobufa[fepcb->echocrps] |= UNDERLINE_ATTR;
           fepcb->echobufa[fepcb->echocrps+1] |= UNDERLINE_ATTR;
           fepcb->echochfg.flag = ON;
           fepcb->echochfg.chtoppos = 0;
           fepcb->echochfg.chlenbytes = 2;
           fepcb->isbeep   = ON;
           fepcb->inputlen = 0;
        }
        return;

      case BACK_SPACE_KEY :
        if ( fepcb->inputlen > 0 )    /* Has radicals Over-The-Spot */
        {
           IcEraseCurrentRadical(fepcb);
           return;
        }
        break;

      case ESC_KEY :
        if ( fepcb->inputlen > 0 )  /* Has radical Over-The-Spot */
        {
           IcEraseAllRadical(fepcb);
           return;
        }
        break;

   }

   /***************************/
   /* Process other input key */
   /***************************/

   if ( fepcb->inputlen == 0 )
   {
      IcEraseAllRadical(fepcb);     /* Erase radical buffer     */
      AnProcess(fepcb, key);        /* return key to AIX system */
   }
   else
   {
      fepcb->indchfg = ON;      /* added by Jim Roung           */
      fepcb->imode.ind5 = ERROR1;
      fepcb->isbeep  = ON;        /* sound a beep             */
   }
   return;
}

/******************************************************************************/
/* FUNCTION    : IcInitial                                                    */
/* DESCRIPTION : Initialize Internal_Code Input Method Environment            */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcInitial(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->imode.ind0 != INTERNAL_CODE_MODE )  /* set indicator flag */
   {
      fepcb->indchfg = ON ;
      fepcb->imode.ind0 = INTERNAL_CODE_MODE;
   }

   memset(fepcb->preinpbuf, NULL, strlen(fepcb->preinpbuf));
   fepcb->imode.ind2 = SELECT_OFF;

   IcEraseAllRadical(fepcb);                   /* erase radicals    */
}

/******************************************************************************/
/* FUNCTION    : IcRadicalInput                                               */
/* DESCRIPTION : Input Internal_Code Radical to echo_buffer                   */
/* INPUT       : fepcb = FEP control block                                    */
/*               key   = 1 byte unsigned char                                 */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcRadicalInput(fepcb,key)
FEPCB *fepcb;
unsigned char key;
{
   unsigned char *rad_str;

   rad_str = IcKeyToRadical(key);           /* get EUC code */
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
/* FUNCTION    : IcEraseCurrentRadical                                        */
/* DESCRIPTION : Erase the preceding input radical                            */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcEraseCurrentRadical(fepcb)
FEPCB *fepcb;
{
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
/* FUNCTION    : IcEraseAllRadical                                            */
/* DESCRIPTION : Erase all Internal_Code input radicals                       */
/* INPUT       : fepcb = FEP control block                                    */
/* OUTPUT      : fepcb = FEP control block                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcEraseAllRadical(fepcb)
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
}

/******************************************************************************/
/* FUNCTION    : IcKeyToRadical                                               */
/* DESCRIPTION : GET Internal_Code Radical EUC code of input key              */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : pointer to the EUC code string                               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

unsigned char *IcKeyToRadical(key)
unsigned char  key;
{
   /* Internal_Code radicals EUC code string  */
   static unsigned short ic_tab[]={
           0xa4a1,0xa4a2,0xa4a3,0xa4a4,0xa4a5,
           0xa4a6,0xa4a7,0xa4a8,0xa4a9,0xa4aa,
           0xa4c1,0xa4c2,0xa4c3,0xa4c4,0xa4c5,0xa4c6};

   static unsigned char ptr[3];
   unsigned short total;
   int    i ;

   if (key>=0x30 && key<=0x39)
   {
      i = key - 0x30;           /* 0,1,2,3,4,5,6,7,8,9 */
      total = ic_tab[i];
   }
   else if (key>=0x61 && key<=0x66)
        {
           i = key - 0x61 + 10;
           total = ic_tab[i];
        }
   ptr[0] = total / 256;
   ptr[1] = total % 256;
   ptr[2] = '\0';
   return(ptr);
}

/******************************************************************************/
/* FUNCTION    : IcTranslator                                                 */
/* DESCRIPTION : Convert Input Radical to Euc Code                            */
/* INPUT       : temp = input radical buffer                                  */
/*               euc =  euc buffer                                            */
/* OUTPUT      : euc                                                          */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcTranslator(temp, euc)
unsigned char temp[];
unsigned char euc[];
{
   int i,j,end;

   end=strlen(temp)/2;
   for(i=0;i<end;i++)
   {
      for (j=i*2;j<i*2+2;j++)
      {
         if (temp[j]>='0' && temp[j]<='9')
            temp[j]=temp[j]-'0';
         else if (temp[j]>='a' && temp[j]<='z')
                 temp[j]=temp[j]-'a'+10;
      }
      euc[i]=temp[i*2]*16+temp[i*2+1];
   }
}

/******************************************************************************/
/* FUNCTION    : IcIsEucCodeRange                                             */
/* DESCRIPTION : Test Input key is in Euc Code Range                          */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcIsEucCodeRange(ptr)
unsigned char *ptr;
{
   if (strlen(ptr) == 1)
   {
      if ((*ptr<0xa1 || *ptr>0xc2) && (*ptr<0xc4 || *ptr>0xfd) && (*ptr!=0x8e))
         return TRUE;
   }
   else if (strlen(ptr) == 2)
   {                       /*   A1A2 -- C2C1  --> A1A1 -- C2C1 ( K. C. Lee ) */
      /*if ( ((*ptr==0xa1) && (*(ptr+1)>=0xa2 && *(ptr+1)<=0xfe))*/
      if ( ((*ptr==0xa1) && (*(ptr+1)>=0xa1 && *(ptr+1)<=0xfe))  /* K. C. Lee */
         || ((*ptr>=0xa2 && *ptr<=0xc1) && (*(ptr+1)>=0xa1 && *(ptr+1)<=0xfe))
         || ((*ptr==0xc2) && (*(ptr+1)>=0xa1 && *(ptr+1)<=0xc1)) )
         return TRUE;
                                     /*   C4A1 -- FDCB   */
      if ( ((*ptr>=0xc4 && *ptr<=0xfc) && (*(ptr+1)>=0xa1 && *(ptr+1)<=0xfe))
         || ((*ptr>=0xfd) && (*(ptr+1)>=0xa1 && *(ptr+1)<=0xcb)) )
         return TRUE;
   }
   else if (strlen(ptr) == 4)
        {                           /*  8EA2A1A1 -- 8EA2F2C4  */
           if ((*ptr==0x8e) && (*(ptr+1)==0xa2))
           {
              if ( ((*(ptr+2)>=0xa1 && *(ptr+2)<=0xf1) &&
                    (*(ptr+3)>=0xa1 && *(ptr+3)<=0xfe)) ||
                   ((*(ptr+2)==0xf2) && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xc4)) )
              return TRUE;
           }
           if ((*ptr==0x8e) && (*(ptr+1)==0xa3))                     /* V410 */
           {                                                         /* V410 */
                                   /* 8EA3A1A1 -- 8EA3E2C6              V410 */
              if ( ((*(ptr+2)>=0xa1 && *(ptr+2)<=0xe1) &&            /* V410 */
                    (*(ptr+3)>=0xa1 && *(ptr+3)<=0xfe)) ||           /* V410 */
                   ((*(ptr+2)==0xe2) && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xc6)) )  /* V410 */
              return TRUE;                                           /* V410 */
           }                        /*                                  V410 */

           if ((*ptr==0x8e) && (*(ptr+1)==0xa4))                     /* V410 */
           {                       /* 8EA4A1A1 -- 8EA4EEDC              V410 */
              if ( ((*(ptr+2)>=0xa1 && *(ptr+2)<=0xed) &&            /* V410 */
                    (*(ptr+3)>=0xa1 && *(ptr+3)<=0xfe)) ||           /* V410 */
                   ((*(ptr+2)==0xee) && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xdc)) )  /* V410 */
              return TRUE;                                           /* V410 */
           }                                                         /* V410 */
                                    /*  8EACA1A1 -- 8EACE2FE            V410 */
           if ((*ptr==0x8e) && (*(ptr+1)==0xac))
           {
              if ( (*(ptr+2)>=0xa1 && *(ptr+2)<=0xe2)
                  && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xfe) )
              return TRUE;
           }
           /*** Added by K. C. Lee ***/
           /*** Code set 13 for IBM unit symbol 8EADA1A1 -- 8EADA4CB ***/
           if ((*ptr==0X8e) && (*(ptr+1)==0xad))
           {
              if ((*(ptr+2)>=0xa1 && *(ptr+2)<=0xa3)
                  && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xfe))
              {
                 return TRUE;
              }
              if ((*(ptr+2)==0xa4)
                 && (*(ptr+3)>=0xa1 && *(ptr+3)<=0xcb))
              {
                 return TRUE;
              }
           }
        }
    return FALSE;
}

/******************************************************************************/
/* FUNCTION    : IcIsBig5CodeRange                                             */
/* DESCRIPTION : Test Input key is in big5 Code Range                         */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcIsBig5CodeRange(ptr)
unsigned char *ptr;
{
   if (strlen(ptr) == 1)
   {
      if ((*ptr<0x81 || *ptr>0xff))
         return TRUE;
   }
   else if (strlen(ptr) == 2)
   {
      if ( ( (*(ptr+1) >= 0x40) && (*(ptr+1) <= 0x7e) ) ||
           ( (*(ptr+1) >= 0x81) && (*(ptr+1) <= 0xa0) ) ||
           ( (*(ptr+1) >= 0xa1) && (*(ptr+1) <= 0xfe) ) )
         return TRUE;
   }

   return FALSE;
}

/******************************************************************************/
/* FUNCTION    : IcIsCodeRange                                                */
/* DESCRIPTION : Test Input key is in Code Range                              */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcIsCodeRange(ptr,codetype)
unsigned char *ptr;
unsigned int  codetype;
{
   unsigned char i;
   unsigned int x1,x2,y2;

   x1 = x2 = y2 = 0x00;
   if (strlen(ptr) == 1)
   {
      y2 = *ptr;
   }
   else if (strlen(ptr) == 2)
   {
      x2 = *ptr;
      y2 = *(ptr+1);
   }
   else if (strlen(ptr) == 3)
   {
      return FALSE;
   }
   else if (strlen(ptr) == 4)
   {
      x1 = (*ptr << 8) | *(ptr+1);
      x2 = *(ptr+2);
      y2 = *(ptr+3);
   }
   for (i=0;i<codesetinfo[codetype].code_range_no ;i++ )
   {
      if ( ( (x1 >= *(codesetinfo[codetype].code_range+i*6+0)) &&
             (x1 <= *(codesetinfo[codetype].code_range+i*6+1))) &&
           ( (x2 >= *(codesetinfo[codetype].code_range+i*6+2)) &&
             (x2 <= *(codesetinfo[codetype].code_range+i*6+4))) &&
           ( (y2 >= *(codesetinfo[codetype].code_range+i*6+3)) &&
             (y2 <= *(codesetinfo[codetype].code_range+i*6+5))) )
        {
         return TRUE;
        }
   }
   return FALSE;
}

/******************************************************************************/
/* FUNCTION    : IcIsRadical                                                  */
/* DESCRIPTION : Test Input key is a Internal_Code Radical                    */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IcIsRadical(key)
unsigned char *key;
{
   unsigned char ch;
   int  ret_code;       /* added by Jim Roung   */

   if ( (*key>=0x41 && *key<=0x46) || (*key>=0x61 && *key<=0x66) )
   {
      if (*key>=0x41 && *key<=0x46)
          *key += 0x20;
      ret_code = TRUE;
   }
   /* Added By Jim Roung - April/18, 1991       */
   else
        ret_code = (*key >= 0x30 && *key <= 0x39) ? TRUE : FALSE;

   return(ret_code);

/* =================>>> Marked By Jim Roung <<<=========================

   else if ( (*key>=0x30 && *key<=0x39) || (*key>=0x21 && *key<=0x29))
        || (*key == 0x7e)
        {
            if (*key>=0x21 && *key<=0x29)
               *key += 0x10;
            if (*key == 0x7e)
               *key = 0x30;
            return (TRUE);        Internal-Code input radicals
        }
        else
            return ( FALSE );

===================> The End of marked area <===========================*/

}
