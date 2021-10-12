static char sccsid[] = "@(#)11	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedfss.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:34:47";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: FssCalItemSize
 *		FssCandidates
 *		FssCloseAux
 *		FssEraseAllRadical
 *		FssFilter
 *		FssGetCandidate
 *		FssInitial
 *		FssIsRadical
 *		FssLeSelect
 *		FssListBox
 *		FssMain
 *		FssRadicalInput
 *		FssSelect
 *		FssShowCursor
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
/* MODULE NAME:        cnedFss                                                */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Five Stroke Style Mode        */
/*                                                                            */
/* FUNCTION:           FssMain          : Entry Point Of Five Stroke Style    */
/*                                        Input Method                        */
/*                                                                            */
/*                     FssFilter        : Five Stroke Style Input Method      */
/*                                        filter                              */
/*                                                                            */
/*                     FssInitial       : Initialization                      */
/*                                                                            */
/*                     FssEraseAllRadical: Erase All Radicals                 */
/*                                                                            */
/*                     FssCandidates    : Find Satisfied Candidates           */
/*                                                                            */
/*                     FssRadicalInput  : Replace/Insert Radicals To Echo Buf */
/*                                                                            */
/*                     FssSelect        : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     FssListBox       : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     FssCalItemSize   : Caculate Item Size                  */
/*                                                                            */
/*                     FssGetCandidate  : Send Selected Cand. To Output Buffer*/
/*                                                                            */
/*                     FssCloseAux      : Close Candidate List Box            */
/*                                                                            */
/* MODULE TYPE:        C                                                      */
/*                                                                            */
/* COMPILER:           AIX C                                                  */
/*                                                                            */
/* AUTHOR:             Tang Bosong                                            */
/*                                                                            */
/* STATUS:             Chinese Input Method Version 1.0                       */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

#include  "cnedinit.h"
#include  "cned.h"
#include  "cnedacc.h"

/******************************************************************************/
/* FUNCTION    : FssMain                                                      */
/* DESCRIPTION : Entry Point of Five Stroke Style Input Method.               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   if( fepcb->imode.ind2 == FIVESTROKE_STYLE_SELECT_ON )
   {
      FssSelect(fepcb, key);           /* Select Candidate At Five Stroke    */
                                       /* Style Candidate List Box           */
      return;
   }

   if (fepcb->imode.ind2 ==LEGEND_SELECT_ON )
   {
      FssLeSelect(fepcb,key);          /* Select Candidate At Legend         */
                                       /* Candidate List Box                 */
      return;
   }


   switch( key )
   {
      case ALPHA_NUM_KEY :
           FssFreeCandidates(fepcb);   /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
           break;

      case PINYIN_KEY :
           FssFreeCandidates(fepcb);   /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           PyInitial(fepcb);           /* Initialize Pinyin Input Method     */
           break;

      case ENGLISH_CHINESE_KEY :
           FssFreeCandidates(fepcb);   /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           EnInitial(fepcb);           /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case FIVESTROKE_STYLE_KEY :
           FssFreeCandidates(fepcb);   /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           FssInitial(fepcb);          /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case FIVESTROKE_KEY :
           FssFreeCandidates(fepcb);   /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           FsInitial(fepcb);          /* Initialize Five Stroke  Input   */
                                       /* Method                             */
           break;

      case ABC_KEY :
           FssFreeCandidates(fepcb);    /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           AbcInitial(fepcb);          /* Initialize ABC Input Method        */
           break;

      case USER_DEFINED_KEY :
           FssFreeCandidates(fepcb);    /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           UdInitial(fepcb);           /* Initialize User_Defined Input      */
                                       /* Method                             */
           break;

      case ROW_COLUMN_KEY :
           FssFreeCandidates(fepcb);    /* Free Five Stroke Style Candidates */
                                       /* Buffer                             */
           RcInitial(fepcb);           /* Initialize Row_Column Input Method */
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

      case LEGEND_SWITCH_KEY :        /* Set LEGEND ON/OFF Mode             */
           fepcb->indchfg = TRUE;

           if( fepcb->imode.ind7 == LEGEND_ON )
             fepcb->imode.ind7 = LEGEND_OFF;
           else
             if( fepcb->fd.lesysfd || fepcb->fd.leusrfd )
               fepcb->imode.ind7 = LEGEND_ON;
           break;

      default :
           FssFilter(fepcb, key);       /* Process Five Stroke Style Input    */
                                       /* Method                              */
           break;
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : FssFilter                                                    */
/* DESCRIPTION : Filters the Five Stroke Style radicals and special keys and  */
/*               goes into appropriatly process.                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssFilter(fepcb, key)
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
   /*  Process Five Stroke Style  radical  */
   /****************************************/
   if ( FssIsRadical(key) )
   {

      if( fepcb->inputlen > MAX_FSS_INPUT_LEN )
      {                                /* Input len overflow error           */
         FssEraseAllRadical(fepcb);    /* Clear All Radicals At Off_The_Spot */
         fepcb->imode.ind5 = ERROR1;   /* Display error message              */
         fepcb->isbeep = BEEP_ON;      /* sound a beep                       */

      }
      else
      {

         if ( fepcb->echoacsz != 0 && fepcb->inputlen == 0 )
              FssEraseAllRadical(fepcb);   /* Erase error radical            */

         FssRadicalInput( fepcb,key );  /* Input Five Stroke Style radial    */


         if( key == 'z')
             fepcb->flag = ON;

         if( fepcb->inputlen == MAX_FSS_INPUT_LEN)
             FssCandidates(fepcb);      /* Find Satisfied Candidates          */
      }

         FssShowCursor(fepcb);
         return;
   }

   if( fepcb->inputlen == 0 )
   {
      FssEraseAllRadical(fepcb);   /* Erase error radical              */
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
         fepcb->flag = ON;
         FssCandidates(fepcb);         /* Find Satisfied Candidates          */
         break;

      case NON_CONVERT_KEY :
         for( cnt=0; cnt<fepcb->echoacsz; cnt++ )
           *(fepcb->edendbuf+cnt) = *(fepcb->echobufs+cnt);

         fepcb->edendacsz = fepcb->echoacsz;
         FssEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
                                       /* Copy Echo Buffer To Edit_End Buffer*/
         fepcb->ret = FOUND_WORD;
         break;

      case ESC_KEY :
         FssEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
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

   FssShowCursor(fepcb);
   return;
}

/******************************************************************************/
/* FUNCTION    : FssShowCursor                                                */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
FssShowCursor(fepcb)
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
/* FUNCTION    : FssIsRadical                                                 */
/* DESCRIPTION : Test the input key is a Five Stroke Style radical.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
FssIsRadical(key)
unsigned short  key;                   /* Ascii Code                         */
{

   if (  key >= 'a' && key <= 'z' )  
      return(TRUE);
   else
      return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : FssInitial                                                   */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{

   FssEraseAllRadical(fepcb);          /* Erase All Radicals At Off_The_Spot */
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
   fepcb->flag = OFF;
   fepcb->imode.ind0 = FIVESTROKE_STYLE_MODE;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->imode.ind4 = REPLACE_MODE;
   fepcb->imode.ind5 = BLANK;
   fepcb->imode.ind6 = INPUT_NO_SELECTED;
   fepcb->isbeep = BEEP_OFF;
   fepcb->inputlen = 0;
}


/******************************************************************************/
/* FUNCTION    : FssEraseAllRadical                                           */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssEraseAllRadical(fepcb)
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
/* FUNCTION    : FssCandidates                                                */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;

   FssAccessDictionary(fepcb);         /* To Find Satisfied Candidates And   */
                                       /* To Show Candidate List Box         */
   if( fepcb->ret == FOUND_CAND )
   {
                                       /* Clear Input Buffer */
      (void)memset(fepcb->fssinbuf, NULL, strlen(fepcb->fssinbuf));
                                       /* Copy Echo Buffer to Input Buffer   */
      for( i=0; i<fepcb->echoacsz; i++ )
        *(fepcb->fssinbuf+i) = *(fepcb->echobufs+i);

      if (fepcb->fssstruct.allcandno == 1)   /*  only one candidate       */
      {
         itemsize = 0;
         memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
         ptr = fepcb->edendbuf;
         candptr = fepcb->fssstruct.cand;
         if( fepcb->flag == ON)
         {
             strncpy(ptr, candptr, 2);
             fepcb->edendacsz = 2;
         }
         else
         {
             do {                             /* Fill Phrase                */
                itemsize ++;
                *ptr++ = (*candptr | 0x80);
             }  while( (*candptr++ & 0x80) != NULL );
             fepcb->edendacsz = itemsize;
         }
         fepcb->ret = FOUND_WORD;
         fepcb->flag = OFF;
         FssEraseAllRadical(fepcb);
         if ( (fepcb->edendacsz == 2) && (fepcb->imode.ind7 == LEGEND_ON) )
         {
            LeCandidates(fepcb);
            if ( fepcb->ret == NOT_FOUND ) {
               fepcb->imode.ind6 = INPUT_NO_SELECTED;
               FssEraseAllRadical(fepcb);
            }
         }
         return;
      }
      fepcb->imode.ind2 = FIVESTROKE_STYLE_SELECT_ON;
      fepcb->auxchfg=ON;
      fepcb->auxuse = USE;
      fepcb->fssstruct.curptr = fepcb->fssstruct.cand;
      fepcb->fssstruct.more = fepcb->fssstruct.allcandno;

      FssListBox(fepcb, BEGINNING);     /* Fill Candidates To Aux. Buffer     */
   }
   else
   {
      fepcb->indchfg = ON;
      if(fepcb->flag == ON)
         fepcb->flag = OFF;
      fepcb->imode.ind5 = ERROR2; 
      fepcb->isbeep = ON;
      fepcb->echocrps = 0;
      fepcb->eccrpsch = ON;
      fepcb->inputlen = 0;
   }
}


/******************************************************************************/
/* FUNCTION    : FssRadicalInput                                              */
/* DESCRIPTION : Replace/Insert raidcal to echo buffer.                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssRadicalInput(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* A key which user pressed( ASCII    */
                                       /* code )                             */
{
   short           i;                  /* Loop Counter                       */


   if( fepcb->imode.ind4 == REPLACE_MODE )
   {
      if( (fepcb->echocrps < 4 && fepcb->inputlen == 4)
          || (fepcb->inputlen < 4) )
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
      if( (fepcb->echocrps < 4 && fepcb->inputlen == 4)
          || (fepcb->inputlen < 4) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+1; i>=fepcb->echocrps+1; i-- )
         {
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-1);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-1);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fepcb->echobufs+fepcb->echocrps) = key;

         if (fepcb->inputlen < 4)
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
/* FUNCTION    : FssSelect                                                    */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssSelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   switch( key )
   {
      case PGUP_KEY :
             FssListBox(fepcb, key);   /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_KEY :
             FssListBox(fepcb, key);    /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case ESC_KEY :
             FssCloseAux(fepcb);       /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             if(fepcb->imode.ind6 == INPUT_SELECTED)
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             break;

      case DELETE_KEY :
             FssCloseAux(fepcb);       /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             FssEraseAllRadical(fepcb);/* Clear All Radical At Off_the_Spot  */
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
                FssGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                if ((fepcb->ret == FOUND_WORD) && (fepcb->imode.ind7==LEGEND_OFF))
                     fepcb->imode.ind6 = INPUT_SELECTED;
             }
             else
             {
                FssCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                FssEraseAllRadical(fepcb);/* Clear All Radical At Off_the_Spot*/
                FssFilter(fepcb, key);  
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
                FssGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                                 /* Jan.12 95 Modified By B.S.Tang           */
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
                FssCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                fepcb->flag = OFF;
                FssEraseAllRadical(fepcb);/* Clear All Radical At Off_the_Spot*/
                FssMain(fepcb, key);  
             }
  }
}

/******************************************************************************/
/* FUNCTION    : FssListBox                                                   */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssListBox(fepcb, key)
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

   static char title[]={0xce,0xe5,0xb1,0xca,0xd7,0xd6,0xd0,0xcd,0xd1,0xa1,0xd4,
                        0xf1,0x20,0x20,0xca,0xa3,0xd3,0xe0};

   static char line[]={0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                      0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                      0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                      0xa9,0xa5,0xa9,0xa5,0xa2,0xa5,0xa9,0xa5,0xa9,0xa5  };

   static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
              0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
              0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
              0x20,0x39,0x20,0x20};

   static char bottom1[]={0xc8,0xa1,0xcf,0xfb,0x28,0x45,0x73,0x63,0x29};

   static char bottom2[]={0xc9,0xcf,0xd2,0xb3,0x28,0x50,0x67,0x55,0x70,
                          0x29,0x20,0x20,0xcf,0xc2,0xd2,0xb3,0x28,0x50,0x67,
                          0x44,0x6e,0x29};


   candptr = fepcb->fssstruct.curptr;

   switch( key )
   {
      case PGUP_KEY:
         if( fepcb->fssstruct.curptr == fepcb->fssstruct.cand )
         {
             fepcb->isbeep = BEEP_ON;  
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         else
         {
            if ( (fepcb->fssstruct.allcandno - fepcb->fssstruct.more) <= 20 )
            {  
               fepcb->fssstruct.curptr = fepcb->fssstruct.cand;
               candptr = fepcb->fssstruct.cand;
               fepcb->fssstruct.more = fepcb->fssstruct.allcandno-10 ;
            }
            else
            {
               if(fepcb->flag == ON)
               {
                  for( i=0; i<11 && *candptr!= NULL; i++)
                  {
                      while((*candptr & 0x80 ) == 0)
                            candptr--;
                      candptr -= 2;
                  }

/*                  candptr -= 20;             */
                  fepcb->fssstruct.curptr = ++candptr;
                  if ( fepcb->fssstruct.more == 0 )
                      fepcb->fssstruct.more = tempno;
                  else
                      fepcb->fssstruct.more += 10;
               }
               else
               {
                  for( i=0; i<11; )
                  {
                     if( (*(--candptr) & 0x80) == NULL ) i++;
                  }
                  fepcb->fssstruct.curptr = ++candptr;
                  if ( fepcb->fssstruct.more == 0 )
                      fepcb->fssstruct.more = tempno;
                  else
                      fepcb->fssstruct.more += 10;
               }
            }

         }
         break;

      case PGDN_KEY:
         if( fepcb->fssstruct.more > 0 )
         {
            if(fepcb->flag == ON)
            {
               for( i=0; i<10 && *candptr!= NULL; i++)
               {
                   candptr += 2;
                   while((*candptr & 0x80 ) == 0)
                         candptr++;
               }

               fepcb->fssstruct.curptr = candptr;

               if( fepcb->fssstruct.more > 10 )
                 fepcb->fssstruct.more -= 10;
               else
               {
                 tempno = fepcb->fssstruct.more;
                 fepcb->fssstruct.more = 0;
               }
            }
            else
            {
               for( i=0; i<10 && *candptr != NULL; )
               {
                  if( (*(candptr++) & 0x80) == NULL) i++;
               }

               fepcb->fssstruct.curptr = candptr;

               if( fepcb->fssstruct.more > 10 )
                 fepcb->fssstruct.more -= 10;
               else
               {
                 tempno = fepcb->fssstruct.more;
                 fepcb->fssstruct.more = 0;
               }
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
         if( fepcb->fssstruct.more >= 10 )
           fepcb->fssstruct.more -= 10;
         else
         {
           tempno = fepcb->fssstruct.more;
           fepcb->fssstruct.more = 0;
         }
         len = sizeof(title)+4;   /* Length Of Title & Bottom Of Cand.  */
         fepcb->auxacsz.itemsize = len;
         FssCalItemSize(fepcb);        /* Caculate Maximum Size Of All Cand. */
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
   memcpy(fepcb->auxbufs[0], title, sizeof(title));
   tostring = fepcb->auxbufs[0]+sizeof(title);

   itoa(fepcb->fssstruct.more, buffer, sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], line, "", fepcb->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; (item<10) && (*candptr!=FSS_GP_END_MARK) ; item++ )
   {
      itemsize = 0;
      tostring = fepcb->auxbufs[item+2];
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(digit+j);
      }

      if(fepcb->flag == ON)
      {
         *tostring++ = *candptr++; /* Fill Phrase                        */
         *tostring++ = *candptr++; /* Fill Phrase                        */
         itemsize += 2;
         while(((*candptr & 0x80 ) == 0) && (*candptr != NULL))
         {
             itemsize++;
             *tostring++ = *candptr++;
         }
      }
      else
      {
        do{                           /* Fill Phrase                        */
           itemsize++;
           *tostring++ = ( *candptr | 0x80);
          }while( (*candptr++ & 0x80) != NULL);
      }
   }
                                            
   memcpy(fepcb->auxbufs[AUXROWMAX-2], bottom1, sizeof(bottom1));
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], bottom2, sizeof(bottom2));

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}

/******************************************************************************/
/* FUNCTION    : FssCalItemSize                                               */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   candptr = fepcb->fssstruct.cand;
   pitemsize = 4;
   nitemsize = 4;                      

   for( ;*candptr != NULL; candptr++ )
   {
      if (*candptr == EUC_BYTE1)        
          nitemsize -= 2;
      nitemsize ++;

      if( (*candptr & 0x80) == NULL )
      {
         if( pitemsize < nitemsize )  pitemsize = nitemsize;
         nitemsize = 4;         
      }
   }

   if( fepcb->auxacsz.itemsize < pitemsize )
     fepcb->auxacsz.itemsize = pitemsize;
}

/******************************************************************************/
/* FUNCTION    : FssGetCandidate                                              */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssGetCandidate(fepcb, key)
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
   if (*ptr == 0x20 || *ptr == NULL)
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

   FssEraseAllRadical(fepcb);
   if ( (fepcb->edendacsz == 2) && (fepcb->imode.ind7 == LEGEND_ON) )
   {
        LeCandidates(fepcb);
        if ( fepcb->ret == NOT_FOUND ) {
           fepcb->imode.ind6 = INPUT_SELECTED;
           FssEraseAllRadical(fepcb);
        }
        else           /* Jan.12 95 Modified By B.S.Tang                    */
           fepcb->imode.ind6 = INPUT_NO_SELECTED;
   }
   if ( fepcb->edendacsz > 2)     /* Jan. 12 95 Modified by Tang */
       fepcb->imode.ind6 = INPUT_SELECTED;
   fepcb->ret = FOUND_WORD;
}


/******************************************************************************/
/* FUNCTION    : FssCloseAux                                                  */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssCloseAux(fepcb)
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


/******************************************************************************/
/* FUNCTION    : FssLeSelect                                                  */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FssLeSelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   switch( key )
   {
      case PGUP_KEY :
             LeListBox(fepcb, key);    /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_KEY :
             LeListBox(fepcb, key);    /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case ESC_KEY :
             FssCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             FssEraseAllRadical(fepcb);
             fepcb->ret = FOUND_WORD;
             break;

      case DELETE_KEY :
             FssCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             FssEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot  */
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
             LeGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
             if (fepcb->ret == FOUND_WORD) 
             {
                 FssCloseAux(fepcb);   /* Disappear Candidate List Box And   */
                 FssEraseAllRadical(fepcb); 
                 fepcb->flag = OFF;
                                       /* Clear All Radical At Off_the_Spot*/
             }
             break;

      default :
             FssCloseAux(fepcb);
             FssEraseAllRadical(fepcb);
             fepcb->flag = OFF;
             FssMain(fepcb, key);
  }
}
