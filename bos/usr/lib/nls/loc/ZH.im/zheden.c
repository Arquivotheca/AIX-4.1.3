static char sccsid[] = "@(#)57	1.2  src/bos/usr/lib/nls/loc/ZH.im/zheden.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:38:54";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: EnCalItemSize
 *		EnCandidates
 *		EnCloseAux
 *		EnEraseAllRadical
 *		EnFilter
 *		EnGetCandidate
 *		EnInitial
 *		EnIsRadical
 *		EnListBox
 *		EnMain
 *		EnRadicalInput
 *		EnSelect
 *		EnShowCursor
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
/************************* START OF MODULE SPECIFICATION **********************/
/*                                                                            */
/* MODULE NAME:        zhedEn                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For English_Chinese Mode          */
/*                                                                            */
/* FUNCTION:           EnMain           : Entry Point Of English_Chinese Input*/
/*                                        Method                              */
/*                                                                            */
/*                     EnFilter         : English_Chinese Input Method filter */
/*                                                                            */
/*                     EnInitial        : Initialization                      */
/*                                                                            */
/*                     EnEraseAllRadical: Erase All Radicals                  */
/*                                                                            */
/*                     EnCandidates     : Find Satisfied Candidates           */
/*                                                                            */
/*                     EnRadicalInput   : Replace/Insert Radicals To Echo Buf */
/*                                                                            */
/*                     EnSelect         : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     EnListBox        : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     EnCalItemSize    : Caculate Item Size                  */
/*                                                                            */
/*                     EnGetCandidate   : Send Selected Cand. To Output Buffer*/
/*                                                                            */
/*                     EnCloseAux       : Close Candidate List Box            */
/*                                                                            */
/* MODULE TYPE:        C                                                      */
/*                                                                            */
/* COMPILER:           AIX C                                                  */
/*                                                                            */
/* AUTHOR:             Tang Bosong, WuJian                                    */
/*                                                                            */
/* STATUS:             Chinese Input Method Version 1.0                       */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
#include  "zhed.h"
#include  "zhedacc.h"
#include  "chinese.h"
#include  "zhedud.h"

/******************************************************************************/
/* FUNCTION    : EnMain                                                       */
/* DESCRIPTION : Entry Point of English_Chinese Input Method.                 */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   if( fepcb->imode.ind2 == ENGLISH_CHINESE_SELECT_ON )
   {
      EnSelect(fepcb, key);            /* Select Candidate At English_Chinese*/
                                       /* Candidate List Box                 */
      return;
   }


   switch( key )
   {
      case ALPHA_NUM_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
           break;

      case PINYIN_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           PyInitial(fepcb);           /* Initialize Pinyin Input Method     */
           break;

      case ENGLISH_CHINESE_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           EnInitial(fepcb);           /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case ABC_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           AbcInitial(fepcb);          /* Initialize ABC Input Method        */
           break;

      case USER_DEFINED_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           if(udimcomm->ret == UD_FOUND)
              (*udimcomm->UserDefinedInitial)(udimcomm); 
                                       /* Initialize User_Defined Input      */
                                       /* Method                             */
           else
              return;
           break;

      case TSANG_JYE_KEY :
           EnFreeCandidates(fepcb);    /* Free English_Chinese Candidates    */
                                       /* Buffer                             */
           TjInitial(fepcb);           /* Initialize Tsang_Jye Input Method */
           break;

      case FULL_HALF_KEY :             /* Set FULL/HALF Mode                 */
           fepcb->indchfg = TRUE;

           if ( fepcb->inputlen > 0 ) 
           {
              fepcb->imode.ind5 = ERROR1;
              fepcb->isbeep = ON;        
              break;
           }
           if( fepcb->imode.ind1 == HALF )
             fepcb->imode.ind1 = FULL;
           else
             fepcb->imode.ind1 = HALF;
           break;

      default :
           EnFilter(fepcb, key);       /* Process English_Chinese Input       */
                                       /* Method                              */
           break;
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : EnFilter                                                     */
/* DESCRIPTION : Filters the English_Chinese radicals and special keys and    */
/*               goes into appropriatly process.                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnFilter(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned short  cnt;                /* Byte Counter                       */

   if (fepcb->imode.ind1 == FULL)
   {
       AnProcess(fepcb, key);
       return;
   }

   /****************************************/
   /*  Process English_Chinese  radical    */
   /****************************************/
   if ( EnIsRadical(key) )
   {
      if( (fepcb->inputlen >= MAX_EN_INPUT_LEN ) && (fepcb->echocrps >= 24 ) )
                                       /* Jan.12 95 Modified By B.S.Tang     */
      {                                /* Input len overflow error           */
         EnEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
         fepcb->imode.ind5 = ERROR1;   /* Display error message              */
         fepcb->isbeep = ON;           /* sound a beep                       */
      }
      else
         EnRadicalInput( fepcb,key );  /* Input English_Chinese radial       */

      EnShowCursor(fepcb);
      return;
   }

   if( fepcb->inputlen == 0 )
   {
      AnProcess(fepcb, key);           /* Return Key To AIX System           */
      return;
   }

   switch( key )
   {
      case BACK_SPACE_KEY :
      /* Erase the input radical in the preceding area   */
         if( fepcb->echocrps > 0 )
         {
            for( cnt = fepcb->echocrps-1; cnt <= fepcb->echoacsz-1; cnt++ )
            {
               *(fepcb->echobufs+cnt) = *(fepcb->echobufs+cnt+1);
               *(fepcb->echobufa+cnt) = *(fepcb->echobufa+cnt+1);
            }
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = fepcb->echocrps-1;
            fepcb->echochfg.chlenbytes = fepcb->echoacsz-(fepcb->echocrps-1);
            fepcb->echocrps -= 1;
            fepcb->eccrpsch = ON;
            fepcb->echoacsz -= 1;
            fepcb->echoover += 1;
            fepcb->inputlen -= 1;
         }
         break;

      case CONVERT_KEY :
         EnCandidates(fepcb);          /* Find Satisfied Candidates          */
         break;

      case NON_CONVERT_KEY :
         for( cnt=0; cnt<fepcb->echoacsz; cnt++ )
           *(fepcb->edendbuf+cnt) = *(fepcb->echobufs+cnt);

         fepcb->edendacsz = fepcb->echoacsz;
         EnEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
                                       /* Copy Echo Buffer To Edit_End Buffer*/
         fepcb->ret = FOUND_WORD;
         break;

      case ESC_KEY :
         EnEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
         break;

      case LEFT_ARROW_KEY : 
                  /* Move the cursor in the preceding area */
         if( fepcb->echocrps > 0 )
         {
            fepcb->echocrps -= 1;
            fepcb->eccrpsch = ON;
         }
         break;

      case RIGHT_ARROW_KEY :
                  /* Move the cursor in the preceding area */
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            fepcb->echocrps += 1;
            fepcb->eccrpsch = ON;
         }
         break;

      case INSERT_KEY :
          /* Insert the radical in the preceding area   */
         if( fepcb->imode.ind4 == INSERT_MODE )
           fepcb->imode.ind4 = REPLACE_MODE;
         else
           fepcb->imode.ind4 = INSERT_MODE;
         break;

      case DELETE_KEY :
          /* Delete the radical in the preceding area   */
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            for( cnt = fepcb->echocrps; cnt <= fepcb->echoacsz-1; cnt++ )
            {
               *(fepcb->echobufs+cnt) = *(fepcb->echobufs+cnt+1);
            }
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = fepcb->echocrps;
            fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
            fepcb->echoacsz -= 1;
            fepcb->echoover += 1;
            fepcb->inputlen -= 1;
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
         fepcb->indchfg = ON;         
         fepcb->imode.ind5 = ERROR1;
         break;
   }

   EnShowCursor(fepcb);
   return;
}

/******************************************************************************/
/* FUNCTION    : EnShowCursor                                                 */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
EnShowCursor(fepcb)
FEPCB *fepcb;
{
   if (fepcb->eccrpsch == ON)         /*  for  Showing  Cursor   */
   {
      memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
      if (fepcb->echocrps < fepcb->echoacsz)
      {
         memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
         fepcb->echobufa[fepcb->echocrps] = 1;
         fepcb->echochfg.flag = ON;
         fepcb->echochfg.chtoppos = 0;
         fepcb->echochfg.chlenbytes = 1;
      }
   }
}

/******************************************************************************/
/* FUNCTION    : EnIsRadical                                                  */
/* DESCRIPTION : Test the input key is a English_Chinese radical.            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
EnIsRadical(key)
unsigned short  key;                   /* Ascii Code                         */
{

   if (  key >= 'a' && key <= 'z' )  
      return(TRUE);
   else
      return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : EnInitial                                                    */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{

   EnEraseAllRadical(fepcb);           /* Erase All Radicals At Off_The_Spot */

                                       /* Clear Edit_End Buffer              */
   if ( fepcb->edendacsz == 0 )
   {
      (void)memset(fepcb->edendbuf, NULL, fepcb->echosize);
      fepcb->edendacsz = 0;
   }
   if ( fepcb->auxchfg != ON )
      fepcb->auxchfg = OFF;
   fepcb->auxuse = NOTUSE;
   fepcb->auxcrpsch = OFF;
   fepcb->auxacsz.itemsize = 0;
   fepcb->auxacsz.itemnum = 0;
   fepcb->indchfg = ON;
   fepcb->imode.ind0 = ENGLISH_CHINESE_MODE;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->imode.ind4 = REPLACE_MODE;
   fepcb->imode.ind5 = BLANK;
   fepcb->imode.ind6 = INPUT_NO_SELECTED;
   fepcb->isbeep = BEEP_OFF;
   fepcb->inputlen = 0;
}


/******************************************************************************/
/* FUNCTION    : EnEraseAllRadical                                            */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnEraseAllRadical(fepcb)
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
/* FUNCTION    : EnCandidates                                                 */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;

   AccessDictionary(fepcb);            /* To Find Satisfied Candidates And   */
                                       /* To Show Candidate List Box         */
   if( fepcb->ret == FOUND_CAND )
   {
                                       /* Clear English_Chinese Input Buffer */
      (void)memset(fepcb->eninbuf, NULL, strlen(fepcb->eninbuf));
                                       /* Copy Echo Buffer to English_Chinese*/
                                       /* Input Buffer                       */
      for( i=0; i<fepcb->echoacsz; i++ )
        *(fepcb->eninbuf+i) = *(fepcb->echobufs+i);

      if (fepcb->enstruct.allcandno == 1)   /*  only one candidate       */
      {
         itemsize = 0;
         memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
         ptr = fepcb->edendbuf;
         candptr = fepcb->enstruct.cand;
         do {                             /* Fill Phrase                */
            itemsize ++;
            *ptr++ = (*candptr | 0x80);
         }  while( (*candptr++ & 0x80) != 0 );
         fepcb->edendacsz = itemsize;
         fepcb->ret = FOUND_WORD;
         EnEraseAllRadical(fepcb);
         return;
      }
      fepcb->imode.ind2 = ENGLISH_CHINESE_SELECT_ON;
      fepcb->auxchfg=ON;
      fepcb->auxuse = USE;
      fepcb->enstruct.curptr = fepcb->enstruct.cand;
      fepcb->enstruct.more = fepcb->enstruct.allcandno;

      EnListBox(fepcb, BEGINNING);     /* Fill Candidates To Aux. Buffer     */
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
/* FUNCTION    : EnRadicalInput                                               */
/* DESCRIPTION : Replace/Insert raidcal to echo buffer.                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnRadicalInput(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* A key which user pressed( ASCII    */
                                       /* code )                             */
{
   short           i;                  /* Loop Counter                       */


   if( fepcb->imode.ind4 == REPLACE_MODE )
   {
      if( (fepcb->echocrps < 24 && fepcb->inputlen == 24)
          || (fepcb->inputlen < 24) )
      {
                                       /* Replace Radical To Echo Buffer     */
         *(fepcb->echobufs+fepcb->echocrps) = key;
         *(fepcb->echobufa+fepcb->echocrps) = REVERSE_ATTR;

         fepcb->echochfg.flag = ON;  /* Update Internal Informations       */
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = 1;

         if( fepcb->echocrps >= fepcb->echoacsz )
         {
            fepcb->inputlen ++;
            fepcb->echoacsz += 1;
            fepcb->echoover -= 1;
         }
         fepcb->echocrps += 1;
         fepcb->eccrpsch = ON;
      }
      else
      {
        fepcb->isbeep = BEEP_ON;       /* Notify FEP To Beep To Alarm        */
        fepcb->indchfg = ON;
        fepcb->imode.ind5 = ERROR1;
      }
   }
   else  /* Insert Mode */
   {
      if( (fepcb->echocrps < 24 && fepcb->inputlen == 24)
          || (fepcb->inputlen < 24) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+1; i>=fepcb->echocrps+1; i-- )
         {
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-1);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-1);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fepcb->echobufs+fepcb->echocrps) = key;

         if (fepcb->inputlen < 24)
         {
            fepcb->echoacsz += 1;
            fepcb->inputlen++;
         }
                                    /* Update Internal Informations       */
         fepcb->echochfg.flag = ON;
         fepcb->echochfg.chtoppos = fepcb->echocrps;
         fepcb->echochfg.chlenbytes = fepcb->echoacsz-fepcb->echocrps;
         fepcb->echocrps += 1;
         fepcb->eccrpsch = ON;
         fepcb->echoover -= 1;
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
/* FUNCTION    : EnSelect                                                     */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnSelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   switch( key )
   {
      case PGUP_KEY :
             EnListBox(fepcb, key);    /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_KEY :
             EnListBox(fepcb, key);    /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case ESC_KEY :
             EnCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             if(fepcb->imode.ind6 == INPUT_SELECTED)
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             break;

      case DELETE_KEY :
             EnCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             EnEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot  */
             if(fepcb->imode.ind6 == INPUT_SELECTED)
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
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
             if ( fepcb->imode.ind6 == INPUT_NO_SELECTED  )
             {
                EnGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                if (fepcb->ret == FOUND_WORD) 
                     fepcb->imode.ind6 = INPUT_SELECTED;
             }
             else
             {
                EnCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                EnEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot*/
                EnFilter(fepcb, key);  
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             }
             break;

      case ALT_NUM_KEY0 :            /* Duplicate Re_Select A Candidate      */
      case ALT_NUM_KEY1 :
      case ALT_NUM_KEY2 :
      case ALT_NUM_KEY3 :
      case ALT_NUM_KEY4 :
      case ALT_NUM_KEY5 :
      case ALT_NUM_KEY6 :
      case ALT_NUM_KEY7 :
      case ALT_NUM_KEY8 :
      case ALT_NUM_KEY9 :
             if ( fepcb->imode.ind6 == INPUT_SELECTED  )
             {
                key &= 0xff;     /* Convert ALT_NUM_KEY to NUM_KEY           */
                EnGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                fepcb->imode.ind6 = INPUT_SELECTED;
             }
             else
             {
                fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm      */
                fepcb->indchfg = ON;
                fepcb->imode.ind5 = ERROR1;
             }
             break;

      default :
             if ( fepcb->imode.ind6 == INPUT_NO_SELECTED )
             {
                fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm      */
                fepcb->indchfg = ON;
                fepcb->imode.ind5 = ERROR1;
                break;
             }
             else
             {
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
                EnCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                EnEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot*/
                EnMain(fepcb, key);
             }
  }
}

/******************************************************************************/
/* FUNCTION    : EnListBox                                                    */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnListBox(fepcb, key)
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

   static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
              0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
              0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
              0x20,0x39,0x20,0x20};


   candptr = fepcb->enstruct.curptr;

   switch( key )
   {
      case PGUP_KEY:
         if( fepcb->enstruct.curptr == fepcb->enstruct.cand )
         {
             fepcb->isbeep = BEEP_ON;  
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         else
         {
            if ( (fepcb->enstruct.allcandno - fepcb->enstruct.more) <= 20 )
            {  
               fepcb->enstruct.curptr = fepcb->enstruct.cand;
               candptr = fepcb->enstruct.cand;
               fepcb->enstruct.more = fepcb->enstruct.allcandno-10 ;
            }
            else
            {
               for( i=0; i<11; )
               {
                  if( (*(--candptr) & 0x80) == '\0' ) i++;
               }
               fepcb->enstruct.curptr = ++candptr;
               if ( fepcb->enstruct.more == 0 )
                   fepcb->enstruct.more = tempno;
               else
                   fepcb->enstruct.more += 10;
            }

         }
         break;

      case PGDN_KEY:
         if( fepcb->enstruct.more > 0 )
         {
            for( i=0; i<10 && *candptr != MASK; )
            {
               if( (*(candptr++) & 0x80) == '\0') i++;
            }

            fepcb->enstruct.curptr = candptr;

            if( fepcb->enstruct.more > 10 )
              fepcb->enstruct.more -= 10;
            else
            {
              tempno = fepcb->enstruct.more;
              fepcb->enstruct.more = 0;
            }
         }
         else
         {
             fepcb->isbeep = BEEP_ON;
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         break;

      default:                         /* Beginning Of The Selection         */
         if( fepcb->enstruct.more >= 10 )
           fepcb->enstruct.more -= 10;
         else
         {
           tempno = fepcb->enstruct.more;
           fepcb->enstruct.more = 0;
         }
         len = strlen(ENTITLE)+4;   /* Length Of Title & Bottom Of Cand.  */
         fepcb->auxacsz.itemsize = len;
         EnCalItemSize(fepcb);        /* Caculate Maximum Size Of All Cand. */
         break;
   }

   toauxs = fepcb->auxbufs;
   toauxa = fepcb->auxbufa;
                                                   /*  clear aux buffer   */
   if (fepcb->auxacsz.itemnum != 0)
      for( i = 0 ; i< fepcb->auxacsz.itemnum-2; i++ )
          memset( *toauxs++,' ',fepcb->auxacsz.itemsize+1);

   toauxs = fepcb->auxbufs;
   toauxa = fepcb->auxbufa;

   for(k = 0; k < 8 ; k++)
   {
       temp = fepcb->auxbufs[k];
       for(m = 0; m < fepcb->auxacsz.itemsize; memcpy(temp," ",1),
           temp++,m++);
   }/* for */              
                                       /* Fill Title                      */
   memcpy(fepcb->auxbufs[0], ENTITLE, strlen(ENTITLE));
   tostring = fepcb->auxbufs[0]+strlen(ENTITLE);

   itoa(fepcb->enstruct.more, buffer, sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], LINE, "", fepcb->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; item<10 && *candptr!=MASK; item++ )
   {
      itemsize = 0;
      tostring = fepcb->auxbufs[item+2];
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(digit+j);
      }

      do{                           /* Fill Phrase                        */
         itemsize++;
         *tostring++ = ( *candptr | 0x80);
        }while( (*candptr++ & 0x80) != '\0');
   }
                                            
   memcpy(fepcb->auxbufs[AUXROWMAX-2], BOTTOM1, strlen(BOTTOM1));
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], BOTTOM2, strlen(BOTTOM2));

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}

/******************************************************************************/
/* FUNCTION    : EnCalItemSize                                                */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   candptr = fepcb->enstruct.cand;
   pitemsize = 4;
   nitemsize = 4;                      

   for( ;*candptr != '\0'; candptr++ )
   {
      if (*candptr == UTF_BYTE1)        
          nitemsize -= 2;
      nitemsize ++;

      if( (*candptr & 0x80) == '\0' )
      {
         if( pitemsize < nitemsize )  pitemsize = nitemsize;
         nitemsize = 4;         
      }
   }

   if( fepcb->auxacsz.itemsize < pitemsize )
     fepcb->auxacsz.itemsize = pitemsize;
}

/******************************************************************************/
/* FUNCTION    : EnGetCandidate                                               */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnGetCandidate(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned char  *ptr, *getptr;       /* Pointer To Selected Candidate      */
   unsigned short  getnum;             /* Selected Number                    */
   unsigned short  i;                  /* Loop Counter                       */
   int len=0;

   getnum = key-0x30;                  /* Caculate Selected Number           */
   getptr = fepcb->auxbufs[getnum+2];
   getptr = getptr +4;
                                       /* Copy Selected Candidate To Edit-End*/
                                       /* Buffer                             */
   ptr = getptr;
   if (*ptr == 0x20 || *ptr == '\0')
   {
       fepcb->isbeep = BEEP_ON;
       fepcb->indchfg = ON;
       fepcb->imode.ind5 = ERROR1;
       return;
   }
   while (*ptr != 0x20 && *ptr != '\0')
   {
       len++;
       ptr++;
   }
   strncpy(fepcb->edendbuf, getptr, len);
   fepcb->edendacsz = len;

   EnEraseAllRadical(fepcb);
   fepcb->ret = FOUND_WORD;
}


/******************************************************************************/
/* FUNCTION    : EnCloseAux                                                   */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

EnCloseAux(fepcb)
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

