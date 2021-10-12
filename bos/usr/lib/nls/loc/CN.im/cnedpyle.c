static char sccsid[] = "@(#)17	1.3  src/bos/usr/lib/nls/loc/CN.im/cnedpyle.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:26";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: LeCalItemSize
 *		LeCandidates
 *		LeGetCandidate
 *		LeListBox
 *		LeSelect
 *		PyCalItemSize
 *		PyCandidates
 *		PyCloseAux
 *		PyEraseAllRadical
 *		PyFilter
 *		PyGetCandidate
 *		PyInitial
 *		PyIsRadical
 *		PyListBox
 *		PyMain
 *		PyRadicalInput
 *		PySelect
 *		PyShowCursor
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
/* MODULE NAME:        cnedpyle                                               */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Pinyin Mode                   */
/*                                                                            */
/* FUNCTION:           PyMain           : Entry Point Of Pinyin Input Method  */
/*                                                                            */
/*                     PyFilter         : Pinyin Input Method filter          */
/*                                                                            */
/*                     PyInitial        : Initialization                      */
/*                                                                            */
/*                     PyEraseAllRadical: Erase All Radicals                  */
/*                                                                            */
/*                     PyCandidates     : Find Satisfied Candidates           */
/*                                                                            */
/*                     PyRadicalInput   : Replace/Insert Radicals To Echo Buf */
/*                                                                            */
/*                     PySelect         : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     PyListBox        : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     PyCalItemSize    : Caculate Item Size                  */
/*                                                                            */
/*                     PyGetCandidate   : Send Selected Cand. To Output Buffer*/
/*                                                                            */
/*                     PyCloseAux       : Close Candidate List Box            */
/*                                                                            */
/*                     LeCandidates     : Find Satisfied Candidates           */
/*                                                                            */
/*                     LeSelect         : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     LeListBox        : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     LeCalItemSize    : Caculate Item Size                  */
/*                                                                            */
/*                     LeGetCandidate   : Send Selected Cand. To Output Buffer*/
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

#include  "cnedinit.h"
#include  "cned.h"
#include  "cnedacc.h"

/******************************************************************************/
/* FUNCTION    : PyMain                                                       */
/* DESCRIPTION : Entry Point of Pinyin Input Method.                          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   if( fepcb->imode.ind2 == PINYIN_SELECT_ON )
   {
      PySelect(fepcb, key);            /* Select Candidate At Pinyin        */
                                       /* Candidate List Box                */
      return;
   }

   if (fepcb->imode.ind2 ==LEGEND_SELECT_ON )
                                       /* Jan. 12 95 Modified By B.S.Tang   */
   {
      LeSelect(fepcb,key);             /* Select Candidate At Legend         */
                                       /* Candidate List Box                 */
      return;
   }

   switch( key )
   {
      case ALPHA_NUM_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
           break;

      case PINYIN_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           PyInitial(fepcb);           /* Initialize Pinyin Input Method     */
           break;

      case ENGLISH_CHINESE_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           EnInitial(fepcb);           /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case ABC_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           AbcInitial(fepcb);          /* Initialize ABC Input Method        */
           break;

      case USER_DEFINED_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           UdInitial(fepcb);           /* Initialize User_Defined Input      */
                                       /* Method                             */
           break;

      case ROW_COLUMN_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           RcInitial(fepcb);           /* Initialize Row_Column Input Method */
           break;

      case FIVESTROKE_STYLE_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           FssInitial(fepcb);          /* Initialize Five Stroke Style Input */
                                       /* Method                             */
           break;

      case FIVESTROKE_KEY :
           PyFreeCandidates(fepcb);    /* Free Pinyin Candidates Buffer      */
           FsInitial(fepcb);          /* Initialize Five Stroke  Input   */
                                       /* Method                             */
           break;

      case FULL_HALF_KEY :             /* Set FULL/HALF Mode                 */
           fepcb->indchfg = TRUE;
           if(fepcb->inputlen > 0 )
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
           PyFilter(fepcb, key);       /* Process Pinyin Input Method        */
           break;
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : PyFilter                                                     */
/* DESCRIPTION : Filters the Pinyin radicals and special keys and goes into   */
/*               appropriatly process.                                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyFilter(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned short  cnt;                /* Byte Counter                       */

   if (fepcb->imode.ind1 == FULL)
   { 
       AnProcess(fepcb, key);
       return;
   }

   /******************************/
   /*  Process Pinyin radical    */
   /******************************/
   if ( PyIsRadical(key) )
   {
      if( (fepcb->inputlen >= MAX_PY_INPUT_LEN ) && (fepcb->echocrps >= 8))
                                      /* Jan. 12 95 Modified By Tang Bosong  */
      {                                /* Input len overflow error           */
         PyEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
         fepcb->imode.ind5 = ERROR1;   /* Display error message              */
         fepcb->isbeep = ON;           /* sound a beep                       */
      }
      else
         PyRadicalInput( fepcb,key );  /* Input Pinyin radial                */

      PyShowCursor(fepcb);            
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
         PyCandidates(fepcb);          /* Find Satisfied Candidates          */
         break;

      case NON_CONVERT_KEY :
         for( cnt=0; cnt<fepcb->echoacsz; cnt++ )
           *(fepcb->edendbuf+cnt) = *(fepcb->echobufs+cnt);

         fepcb->edendacsz = fepcb->echoacsz;
         PyEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
                                       /* Copy Echo Buffer To Edit_End Buffer*/
         fepcb->ret = FOUND_WORD;
         break;

      case ESC_KEY :
         PyEraseAllRadical(fepcb);     /* Clear All Radicals At Off_The_Spot */
         break;

      case LEFT_ARROW_KEY :
         if( fepcb->echocrps > 0 )
         {
            fepcb->echocrps -= 1;
            fepcb->eccrpsch = ON;
         }
         break;

      case RIGHT_ARROW_KEY :
         if( fepcb->echocrps < fepcb->echoacsz )
         {
            fepcb->echocrps += 1;
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

   PyShowCursor(fepcb);
   return;
}

/******************************************************************************/
/* FUNCTION    : PyShowCursor                                                 */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
PyShowCursor(fepcb)         /*    by   terry     */
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
/* FUNCTION    : PyIsRadical                                                  */
/* DESCRIPTION : Test the inpput key is a Pinyin radical.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
PyIsRadical(key)
unsigned short  key;                   /* Ascii Code                         */
{
   if ( key < 0x61 || key > 'z' )
      return(FALSE);
   else
      return(TRUE);
}

/******************************************************************************/
/* FUNCTION    : PyInitial                                                    */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{

   PyEraseAllRadical(fepcb);           /* Erase All Radicals At Off_The_Spot */

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
   fepcb->imode.ind0 = PINYIN_MODE;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->imode.ind4 = REPLACE_MODE;
   fepcb->imode.ind5 = BLANK;
   fepcb->imode.ind6 = INPUT_NO_SELECTED;
   fepcb->isbeep = BEEP_OFF;
   fepcb->inputlen = 0;
/*   fepcb->starpos = 0; */
}


/******************************************************************************/
/* FUNCTION    : PyEraseAllRadical                                            */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyEraseAllRadical(fepcb)
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
/* FUNCTION    : PyCandidates                                                 */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;

   AccessDictionary(fepcb);            /* To Find Satisfied Candidates And   */
                                       /* To Show Candidate List Box         */
   if( fepcb->ret == FOUND_CAND )
   {
                                       /* Clear Pinyin Iput Buffer           */
      (void)memset(fepcb->pyinbuf, NULL, strlen(fepcb->pyinbuf));
                                       /* Copy Echo Buffer to Pinyin Input   */
                                       /* Buffer                             */
      for( i=0; i<fepcb->echoacsz; i++ )
        *(fepcb->pyinbuf+i) = *(fepcb->echobufs+i);

      if (fepcb->pystruct.allcandno == 1)   /*  only one candidate       */
      {
         itemsize = 0;
         memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
         ptr = fepcb->edendbuf;
         candptr = fepcb->pystruct.cand;
         strncpy(ptr, candptr, 2);
         fepcb->edendacsz = 2;
         fepcb->ret = FOUND_WORD;
         PyEraseAllRadical(fepcb);
         if ( fepcb->imode.ind7 == LEGEND_ON )
         {
            LeCandidates(fepcb);
            if ( fepcb->ret == NOT_FOUND ) {
               fepcb->imode.ind6 = INPUT_NO_SELECTED;
               PyEraseAllRadical(fepcb);
            }
         }
         return;
      }
      fepcb->imode.ind2 = PINYIN_SELECT_ON;
      fepcb->auxchfg=ON;
      fepcb->auxuse = USE;
      fepcb->pystruct.curptr = fepcb->pystruct.cand;
      fepcb->pystruct.more = fepcb->pystruct.allcandno;

      PyListBox(fepcb, BEGINNING);     /* Fill Candidates To Aux. Buffer     */
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
/* FUNCTION    : PyRadicalInput                                               */
/* DESCRIPTION : Replace/Insert raidcal to echo buffer.                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyRadicalInput(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* A key which user pressed( ASCII    */
                                       /* code )                             */
{
   short           i;                  /* Loop Counter                       */
   unsigned char   *codeptr;           /* Pointer To Pinyin Radical          */

   /*codeptr = key;      Point To Pinyin Radical            */

   if( fepcb->imode.ind4 == REPLACE_MODE )
   {
      if( (fepcb->echocrps < 8 && fepcb->inputlen == 8)
          || (fepcb->inputlen < 8) )
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
      if( (fepcb->echocrps < 8 && fepcb->inputlen == 8)
          || (fepcb->inputlen < 8) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+1; i>=fepcb->echocrps+1; i-- )
         {
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-1);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-1);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fepcb->echobufs+fepcb->echocrps) = key;

         if (fepcb->inputlen < 8)
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
/* FUNCTION    : PySelect                                                     */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PySelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   switch( key )
   {
      case PGUP_KEY :
             PyListBox(fepcb, key);    /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_KEY :
             PyListBox(fepcb, key);    /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case ESC_KEY :
             PyCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             if(fepcb->imode.ind6 == INPUT_SELECTED)
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
     /*      PyShowCursor(fepcb);     */
             break;

      case DELETE_KEY :
             PyCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             PyEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot  */
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
                PyGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                if ( (fepcb->ret == FOUND_WORD) && (fepcb->imode.ind7 == LEGEND_OFF) )
                   fepcb->imode.ind6 = INPUT_SELECTED;
             }
             else
             {
                PyCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                PyEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot*/
                PyFilter(fepcb, key);  
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
                PyGetCandidate(fepcb, key);
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
                PyCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                PyEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot*/
                PyMain(fepcb, key);  
             }
  }
}

/******************************************************************************/
/* FUNCTION    : PyListBox                                                    */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyListBox(fepcb, key)
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

   static char title[]={0xc6,0xb4,0xd2,0xf4,0xd1,0xa1,0xd4,0xf1,0x20,0x20
                        ,0x20,0x20,0x20,0x20,0xca,0xa3,0xd3,0xe0};

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

   candptr = fepcb->pystruct.curptr;

   switch( key )
   {
      case PGUP_KEY:
         if( fepcb->pystruct.curptr == fepcb->pystruct.cand )
         {
             fepcb->isbeep = BEEP_ON;              /*   by terry  */
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         else
         {
            if ( (fepcb->pystruct.allcandno - fepcb->pystruct.more) <= 20 )
            {      /* <=20 , original value < 10  */
               fepcb->pystruct.curptr = fepcb->pystruct.cand;
               candptr = fepcb->pystruct.cand;
               fepcb->pystruct.more = fepcb->pystruct.allcandno-10 ;
            }
            else
            {
               candptr -= 20;
               fepcb->pystruct.curptr = candptr;
               if ( fepcb->pystruct.more == 0 )
                   fepcb->pystruct.more = tempno;
               else
                   fepcb->pystruct.more += 10;
            }

         }
         break;

      case PGDN_KEY:
         if( fepcb->pystruct.more > 0 )
         {
            for( i=0; i<20 && (*candptr & 0x80 ) != NULL; i++, candptr++ );

            fepcb->pystruct.curptr = candptr;

            if( fepcb->pystruct.more > 10 ) 
              fepcb->pystruct.more -= 10;
            else
            {
              tempno = fepcb->pystruct.more;
              fepcb->pystruct.more = 0;
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
         if( fepcb->pystruct.more >= 10 )
           fepcb->pystruct.more -= 10;
         else
         {
           tempno = fepcb->pystruct.more;
           fepcb->pystruct.more = 0;
         }           
         len = sizeof(title)+4;   /* Length Of Title & Bottom Of Cand.  */
         fepcb->auxacsz.itemsize = len;
         if( fepcb->imode.ind0 == PINYIN_MODE )
             PyCalItemSize(fepcb);   /* Caculate Maximum Size Of All Cand. */
         if( fepcb->imode.ind0 == FIVESTROKE_STYLE_MODE )
             FssCalItemSize(fepcb);   /* Caculate Maximum Size Of All Cand. */
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

   itoa(fepcb->pystruct.more, buffer, sizeof(buffer)); 
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], line, "", fepcb->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; item<10 && *candptr != PY_GP_END_MARK; item++ )
   {
      itemsize = 0;
      tostring = fepcb->auxbufs[item+2];
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(digit+j);
      }

      *tostring++ = *candptr++;     /* Fill Phrase                        */
      *tostring++ = *candptr++;
   }
                                            
   memcpy(fepcb->auxbufs[AUXROWMAX-2], bottom1, sizeof(bottom1));
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], bottom2, sizeof(bottom2));

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}

/******************************************************************************/
/* FUNCTION    : PyCalItemSize                                                */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   candptr = fepcb->pystruct.cand;
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
/* FUNCTION    : PyGetCandidate                                               */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyGetCandidate(fepcb, key)
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
   strncpy(fepcb->edendbuf, getptr, 2);
   fepcb->edendacsz = 2;

   PyEraseAllRadical(fepcb);
   if ( fepcb->imode.ind7 == LEGEND_ON )   
   {
      LeCandidates(fepcb);  
      if ( fepcb->ret == NOT_FOUND ) {
         fepcb->imode.ind6 = INPUT_SELECTED;
         PyEraseAllRadical(fepcb);
      }
      else         /* Jan.12 95 Modified By B.S.Tang   */
         fepcb->imode.ind6 = INPUT_NO_SELECTED;
   }

   fepcb->ret = FOUND_WORD;
}

/******************************************************************************/
/* FUNCTION    : PyCloseAux                                                   */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

PyCloseAux(fepcb)
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
/* FUNCTION    : LeCandidates                                                 */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

LeCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;

   AccessLDictionary(fepcb);           /* To Find Satisfied Candidates And   */
                                       /* To Show Candidate List Box         */
   if( fepcb->ret == FOUND_CAND )
   {

      fepcb->imode.ind2 = LEGEND_SELECT_ON;
      fepcb->auxchfg=ON;
      fepcb->auxuse = USE;
      fepcb->lestruct.curptr = fepcb->lestruct.cand;
      fepcb->lestruct.more = fepcb->lestruct.allcandno;

      LeListBox(fepcb, BEGINNING);     /* Fill Candidates To Aux. Buffer     */
   } 
}


/******************************************************************************/
/* FUNCTION    : LeListBox                                                    */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

LeListBox(fepcb, key)
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


   static char title[]={0xc1,0xaa,0xcf,0xeb,0xd1,0xa1,0xd4,0xf1,0x20,0x20,
                        0x20,0x20,0x20,0x20,0xca,0xa3,0xd3,0xe0};

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

   candptr = fepcb->lestruct.curptr;

   switch( key )
   {
      case PGUP_KEY:
         if( fepcb->lestruct.curptr == fepcb->lestruct.cand )
         {
             fepcb->isbeep = BEEP_ON;              /*   by terry  */
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         else
         {
            if ( (fepcb->lestruct.allcandno - fepcb->lestruct.more) <= 20 )
            {      /* <=20 , original value < 10  */
               fepcb->lestruct.curptr = fepcb->lestruct.cand;
               candptr = fepcb->lestruct.cand;
               fepcb->lestruct.more = fepcb->lestruct.allcandno-10 ;
            }
            else
            {
               for( i=0; i<11; )
               {
                  if( (*(--candptr) & 0x80) == NULL ) i++;
               } 
               fepcb->lestruct.curptr = ++candptr;
               if ( fepcb->lestruct.more == 0 )
                   fepcb->lestruct.more = tempno;
               else
                   fepcb->lestruct.more += 10;
            }

         }
         break;

      case PGDN_KEY:
         if( fepcb->lestruct.more > 0 )
         {
            for( i=0; i<10 && *candptr!=NULL; )
            {
               if( (*(candptr++) & 0x80) == NULL ) i++;
            }

            fepcb->lestruct.curptr = candptr;

            if( fepcb->lestruct.more > 10 ) 
              fepcb->lestruct.more -= 10;
            else
            {
              tempno = fepcb->lestruct.more;
              fepcb->lestruct.more = 0;
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
         if( fepcb->lestruct.more >= 10 )
           fepcb->lestruct.more -= 10;
         else
         {
           tempno = fepcb->lestruct.more;
           fepcb->lestruct.more = 0;
         }                                
         len = sizeof(title)+4;   /* Length Of Title & Bottom Of Cand.  */
         fepcb->auxacsz.itemsize = len;
         PyCalItemSize(fepcb);        /* Caculate Maximum Size Of All Cand. */
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

   itoa(fepcb->lestruct.more, buffer, sizeof(buffer)); 
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], line, "", fepcb->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; item<10 && *candptr!=PY_GP_END_MARK; item++ )
   {
      itemsize = 0;
      tostring = fepcb->auxbufs[item+2];
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(digit+j);
      }

      do {                          /* Fill Phrase                        */
            itemsize ++;
            *tostring++ = (*candptr | 0x80);
         } while( (*candptr++ & 0x80) != NULL );
   }
                                            
   memcpy(fepcb->auxbufs[AUXROWMAX-2], bottom1, sizeof(bottom1));
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], bottom2, sizeof(bottom2));

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}

/******************************************************************************/
/* FUNCTION    : LeCalItemSize                                                */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

LeCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   candptr = fepcb->lestruct.cand;
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
/* FUNCTION    : LeGetCandidate                                               */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

LeGetCandidate(fepcb, key)
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

   fepcb->ret = FOUND_WORD;
}


/******************************************************************************/
/* FUNCTION    : LeSelect                                                     */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

LeSelect(fepcb, key)
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
             PyCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             PyEraseAllRadical(fepcb);
             fepcb->ret = FOUND_WORD;
             break;

      case DELETE_KEY :
             PyCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             PyEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot  */
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
                 PyCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                 PyEraseAllRadical(fepcb); /* Clear All Radical At Off_the_Spot*/
             }
             break;

      default :
             PyCloseAux(fepcb);
             PyEraseAllRadical(fepcb);
             PyMain(fepcb, key);
  }
}
