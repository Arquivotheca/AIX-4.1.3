static char sccsid[] = "@(#)19	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedud.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:54";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: UdCalItemSize
 *		UdCandidates
 *		UdCloseAux
 *		UdEraseAllRadical
 *		UdFilter
 *		UdFreeCandidates
 *		UdGetCandidate
 *		UdInitial
 *		UdListBox
 *		UdMain
 *		UdRadicalInput
 *		UdSelect
 *		UdShowCursor
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
/* MODULE NAME:        cnedud                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For User Defined Mode             */
/*                                                                            */
/* FUNCTION:           UdMain           : Entry Point Of User Defined Input   */
/*                                        Method                              */
/*                                                                            */
/*                     UdFilter         : User Defined Input Method filter    */
/*                                                                            */
/*                     UdInitial        : Initialization                      */
/*                                                                            */
/*                     UdEraseAllRadical: Erase All Radicals                  */
/*                                                                            */
/*                     UdRadicalInput   : Replace/Insert Radicals To Echo Buf */
/*                                                                            */
/*                     UdSelect         : Process Input Key On Cand. List Box */
/*                                                                            */
/*                     UdListBox        : Process PageUp/PageDown On List Box */
/*                                                                            */
/*                     UdCalItemSize    : Caculate maximum size of all cand.  */
/*                                                                            */
/*                     UdCandidates     : Find Satisfied Candidates           */
/*                                                                            */
/*                     UdGetCandidate   : Send Selected Cand. To Output Buffer*/
/*                                                                            */
/*                     UdCloseAux       : Close Candidate List Box            */
/*                                                                            */
/*                     UdFreeCandidates : Free alloacted memory for User      */
/*                                        Defined Condidates                  */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/

#include  "cned.h"
#include  "cnedacc.h"
#include  "cnedud.h"
#include  "chinese.h"

extern UdimCommon *udimcomm;

/******************************************************************************/
/* FUNCTION    : UdMain                                                       */
/* DESCRIPTION : Entry Point of User defined Input Method.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* a key which the user pressed       */
                                       /* (Ascii Code)                       */
{
   if( fepcb->imode.ind2 == USER_DEFINED_SELECT_ON )
   {
      UdSelect(fepcb, key);            /* Select Candidate At User Defined  */
                                       /* Candidate List Box                */
      return;
   }

   switch( key )
   {
      case ALPHA_NUM_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined  Candidates      */
                                       /* Buffer                             */
           AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
           break;

      case PINYIN_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined Candidates       */
                                       /* Buffer                             */
           PyInitial(fepcb);           /* Initialize Pinyin Input Method     */
           break;

      case ENGLISH_CHINESE_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined Candidates       */
                                       /* Buffer                             */
           EnInitial(fepcb);           /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case ABC_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined Candidates       */
                                       /* Buffer                             */
           AbcInitial(fepcb);          /* Initialize ABC Input Method        */
           break;

      case USER_DEFINED_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined Candidates       */
                                       /* Buffer                             */
           UdInitial(fepcb);           /* Initialize User_Defined Input      */
                                       /* Method                             */
           break;

      case ROW_COLUMN_KEY :
           UdFreeCandidates(fepcb);    /* Free User Defined Candidates       */
                                       /* Buffer                             */
           RcInitial(fepcb);           /* Initialize Row_Column Input Method */
           break;

      case FULL_HALF_KEY :             /* Set FULL/HALF Mode                 */
           fepcb->indchfg = TRUE;

           if( fepcb->imode.ind1 == HALF )
             fepcb->imode.ind1 = FULL;
           else
             fepcb->imode.ind1 = HALF;
           break;

      default :
           UdFilter(fepcb, key);       /* Process User Defined Input Method  */
           break;
   }

   return;
}

/******************************************************************************/
/* FUNCTION    : UdFilter                                                     */
/* DESCRIPTION : Passes every inputed key event to the user defined filter    */
/*               routine (UserDefinedFilter() ) and goed into appropriatly    */
/*               process according to return value of the user defined filter */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdFilter(fepcb, key )
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key ;                  /* A key  which the user pressed      */
                                       /* (Ascii Code)                       */
{
   int         i,  flag;               /* A flag of the key                  */
   unsigned short  cnt;                /* Byte Counter                       */

   if (fepcb->imode.ind1 == FULL)
   {
       AnProcess(fepcb, key);
       return;
   }

   /*************************************/
   /*  Process User Defined  radical    */
   /*************************************/
   flag = (*udimcomm->UserDefinedFilter)( key );


   switch( flag )
   {
      case IS_RADICAL_FLAG :
         if( fepcb->inputlen >= udimcomm->inputmaxlen )
         {                         /* Input len overflow error           */
             UdEraseAllRadical(fepcb);
                                   /* Clear All Radicals At Over_The_Spot */
             fepcb->imode.ind5 = ERROR1;
                                   /* Display error message              */
             fepcb->isbeep = ON;   /* sound a beep                       */
         }
         else
             UdRadicalInput( fepcb,key );
                                    /* Input User Defined  radial         */
         break;

      case BACK_SPACE_FLAG :
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
         else
         {
            AnProcess(fepcb, key);
            return;
         }
         break;

      case CONVERT_FLAG :
         if( fepcb->inputlen > 0 )
         {
             UdCandidates(fepcb);
         }
         else
         {
             AnProcess(fepcb, key);
             return;
         }
         break;
 
      case NON_CONVERT_FLAG :
         for( cnt = 0; cnt < fepcb->echoacsz; cnt++)
              *( fepcb->edendbuf + cnt ) = *( fepcb->echobufs + cnt);
         fepcb->edendacsz = fepcb->echoacsz;
         UdEraseAllRadical(fepcb);
         fepcb->ret = FOUND_WORD;
         break;

      case CLEAR_RADICAL_FLAG :
         if( fepcb->inputlen > 0 )
         {
             UdEraseAllRadical(fepcb);   
                             /* Clear All Radicals At Over_The_Spot */
             break;
         }
         else
         {
             AnProcess(fepcb, key);
             return;
         }


      case LEFT_ARROW_FLAG :
         if( fepcb->echocrps > 0 && fepcb->inputlen > 0 )
         {
            fepcb->echocrps -= 1;
            fepcb->eccrpsch = ON;
            break;
         }
         else
         {
             AnProcess(fepcb, key);
             return;
         }

      case RIGHT_ARROW_FLAG :
         if( fepcb->echocrps < fepcb->echoacsz  && fepcb->inputlen > 0)
         {
            fepcb->echocrps += 1;
            fepcb->eccrpsch = ON;
            break;
         }
         else
         {
             AnProcess(fepcb, key);
             return;
         }

      case INSERT_FLAG :
         if( fepcb->imode.ind4 == INSERT_MODE )
           fepcb->imode.ind4 = REPLACE_MODE;
         else
           fepcb->imode.ind4 = INSERT_MODE;
         break;

      case DELETE_FLAG :
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

      case ERROR_FLAG :
         if(fepcb->inputlen == 0)
         {
            AnProcess(fepcb, key);  /* Return Key To AIX System           */
            return;
         }
         else
         {
            fepcb->isbeep = BEEP_ON;    /* Notify FEP To Beep To Alarm        */
            fepcb->indchfg = ON;         
            fepcb->imode.ind5 = ERROR1;
            break;
         }

   }

   UdShowCursor(fepcb);
   return;
}


/******************************************************************************/
/* FUNCTION    : UdShowCursor                                                 */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
UdShowCursor(fepcb)
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
/* FUNCTION    : UdInitial                                                    */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   if(udimcomm->ret == UD_FOUND)
   {
      UdEraseAllRadical(fepcb);        /* Erase All Radicals At Over_The_Spot */

                                       /* Clear Edit_End Buffer              */
      (void)memset(fepcb->edendbuf, NULL, fepcb->echosize);

      fepcb->edendacsz = 0;
      fepcb->auxchfg = OFF;
      fepcb->auxuse = NOTUSE;
      fepcb->auxcrpsch = OFF;
      fepcb->auxacsz.itemsize = 0;
      fepcb->auxacsz.itemnum = 0;
      fepcb->indchfg = ON;
      fepcb->imode.ind0 = USER_DEFINED_MODE;
      fepcb->imode.ind2 = SELECT_OFF;
      fepcb->imode.ind4 = REPLACE_MODE;
      fepcb->imode.ind5 = BLANK;
      fepcb->imode.ind6 = INPUT_NO_SELECTED;
      fepcb->isbeep = BEEP_OFF;
      fepcb->inputlen = 0;
/*   fepcb->starpos = 0;    */
   }
   
/*   if(udimcomm->ret == UD_NOT_FOUND)   */
   else
      return;
}



/******************************************************************************/
/* FUNCTION    : UdCandidates                                                 */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdCandidates(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   short           i,itemsize;         /* Loop Counter                       */
   unsigned char  *candptr,*ptr;
   int         ret;                    /* A flag of the convert              */

         (void)memset(fepcb->udinbuf, NULL, strlen(fepcb->udinbuf));
                    /*Copy Echo Buffer to User Defined Input Buffer*/          
         for( i=0; i<fepcb->echoacsz; i++ )
             *(fepcb->udinbuf+i) = *(fepcb->echobufs+i);
         udimcomm->radicbuf = fepcb->udinbuf;
         ret = (*udimcomm->UserDefinedConvert)(udimcomm);
         if( ret == FOUND_CANDIDATE )
         {
             fepcb->udstruct.cand = udimcomm->candbuf;
             fepcb->udstruct.allcandno = udimcomm->allcandno;
             if (fepcb->udstruct.allcandno == 1)   /*  only one candidate    */
             {
                 itemsize = 0;
                 memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
                 ptr = fepcb->edendbuf;
                 candptr = fepcb->udstruct.cand;
                 do {                        /* Fill Phrase                */
                     itemsize ++;
                     *ptr++ = (*candptr | 0x80);
                 }  while( (*candptr++ & 0x80) != NULL );
                 fepcb->edendacsz = itemsize;
                 fepcb->ret = FOUND_WORD;
                 UdEraseAllRadical(fepcb);
                 return;
              }
             fepcb->imode.ind2 = USER_DEFINED_SELECT_ON;
             fepcb->auxchfg=ON;
             fepcb->auxuse = USE;
             fepcb->udstruct.curptr = fepcb->udstruct.cand;
             fepcb->udstruct.more = fepcb->udstruct.allcandno;
             UdListBox(fepcb, BEGINNING); /* Fill Candidates To Aux. Buffer  */
         }
         else
         {
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR2;  /* Display Error Message           */
             fepcb->isbeep = ON;
             fepcb->echocrps = 0;
             fepcb->eccrpsch = ON;
         }
}

/******************************************************************************/
/* FUNCTION    : UdEraseAllRadical                                            */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdEraseAllRadical(fepcb)
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
/* FUNCTION    : UdRadicalInput                                               */
/* DESCRIPTION : Inputs the key which the user pressed to echo buffer.        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdRadicalInput(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* A key which user pressed( ASCII    */
                                       /* code )                             */
{
   short           i;                  /* Loop Counter                       */

   if( fepcb->imode.ind4 == REPLACE_MODE )
   {
      if( (fepcb->echocrps < udimcomm->inputmaxlen && fepcb->inputlen == udimcomm->inputmaxlen)
          || (fepcb->inputlen < udimcomm->inputmaxlen) )
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
      if( (fepcb->echocrps < udimcomm->inputmaxlen && fepcb->inputlen == udimcomm->inputmaxlen)
          || (fepcb->inputlen < udimcomm->inputmaxlen) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fepcb->echoacsz+1; i>=fepcb->echocrps+1; i-- )
         {
            *(fepcb->echobufs+i) = *(fepcb->echobufs+i-1);
            *(fepcb->echobufa+i) = *(fepcb->echobufa+i-1);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fepcb->echobufs+fepcb->echocrps) = key;

         if (fepcb->inputlen < udimcomm->inputmaxlen)
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
/* FUNCTION    : UdSelect                                                     */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdSelect(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* A key which the user pressed       */
{
int             flag;                  /* A flag of the key                  */

   flag = (*udimcomm->UserDefinedSelect)( key );

   switch( flag )
   {
      case IS_RADICAL_FLAG :
             UdCloseAux(fepcb);      /* Disappear Candidate List Box And   */
             UdEraseAllRadical(fepcb);
                                     /* Clear All Radical At Over_the_Spot */
             UdFilter(fepcb, key);  
             break;

      case PGUP_FLAG :
             UdListBox(fepcb, flag);   /* Generate Previous 10 Candidates Of */
                                       /* Candidate List Box                 */
             break;

      case PGDN_FLAG :
             UdListBox(fepcb, flag);   /* Generate Next 10 Candidates Of     */
                                       /* Candidate List Box                 */
             break;

      case LISTBOX_OFF_FLAG :
             UdCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             if ( fepcb->imode.ind6 == INPUT_SELECTED  )
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             break;

      case DELETE_FLAG :
             UdCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                       /* Return To Radical Input Phase      */
             UdEraseAllRadical(fepcb); /* Clear All Radical At Over_the_Spot  */
             if ( fepcb->imode.ind6 == INPUT_SELECTED  )
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             break;

      case ISRESULT_NUM_FLAG :         /* Select A Candidate                 */
             if ( fepcb->imode.ind6 == INPUT_NO_SELECTED  )
             {
                UdGetCandidate(fepcb, key);
                                 /* Send Selected Candidate To Output Buffer */
                if (fepcb->ret == FOUND_WORD) 
                     fepcb->imode.ind6 = INPUT_SELECTED;
             }
             else
             {
                UdCloseAux(fepcb);      /* Disappear Candidate List Box And   */
                UdEraseAllRadical(fepcb);
                                        /* Clear All Radical At Over_the_Spot */
                UdFilter(fepcb, flag);  
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             }
             break;

      case ISRESULT_ALT_FLAG :        /* Duplicate Re_Select A Candidate      */
             if ( fepcb->imode.ind6 == INPUT_SELECTED  )
             {
                key &= 0xff;
                UdGetCandidate(fepcb, key);
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

      case ERROR_FLAG :
             if ( fepcb->imode.ind6 == INPUT_NO_SELECTED )
             {
                fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm      */
                fepcb->indchfg = ON;
                fepcb->imode.ind5 = ERROR1;
                break;
             }
             else
             {
                UdCloseAux(fepcb);     /* Disappear Candidate List Box And   */
                UdEraseAllRadical(fepcb);
                                       /* Clear All Radical At Over_the_Spot*/
                UdFilter(fepcb, key);  
                fepcb->imode.ind6 = INPUT_NO_SELECTED;
             }
   }
}



/******************************************************************************/
/* FUNCTION    : UdListBox                                                    */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, flag                                                  */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdListBox(fepcb, flag)
FEPCB           *fepcb;                /* FEP Control Block                  */
int             flag;                  /* Ascii Code (PGUP_KEY/PGDN_KEY)     */
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

   static char title[]={0xd3,0xc3,0xbb,0xa7,0xd1,0xa1,0xd4,0xf1,0x20,0x20
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


   candptr = fepcb->udstruct.curptr;

   switch( flag )
   {
      case PGUP_FLAG:
         if( fepcb->udstruct.curptr == fepcb->udstruct.cand )
         {
             fepcb->isbeep = BEEP_ON;  
             fepcb->indchfg = ON;
             fepcb->imode.ind5 = ERROR1;
             return;
         }
         else
         {
            if ( (fepcb->udstruct.allcandno - fepcb->udstruct.more) <= 20 )
            {  
               fepcb->udstruct.curptr = fepcb->udstruct.cand;
               candptr = fepcb->udstruct.cand;
               fepcb->udstruct.more = fepcb->udstruct.allcandno-10 ;
            }
            else
            {
               for( i=0; i<11; )
               {
                  if( (*(--candptr) & 0x80) == NULL ) i++;
               }
               fepcb->udstruct.curptr = ++candptr;
               if ( fepcb->udstruct.more == 0 )
                   fepcb->udstruct.more = tempno;
               else
                   fepcb->udstruct.more += 10;
            }

         }
         break;

      case PGDN_FLAG:
         if( fepcb->udstruct.more > 0 )
         {
            for( i=0; i<10 && *candptr != NULL; )
            {
               if( (*(candptr++) & 0x80) == NULL) i++;
            }

            fepcb->udstruct.curptr = candptr;

            if( fepcb->udstruct.more > 10 )
              fepcb->udstruct.more -= 10;
            else
            {
              tempno = fepcb->udstruct.more;
              fepcb->udstruct.more = 0;
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
         if( fepcb->udstruct.more >= 10 )
           fepcb->udstruct.more -= 10;
         else
         {
           tempno = fepcb->udstruct.more;
           fepcb->udstruct.more = 0;
         }
         len = sizeof(title)+4;   /* Length Of Title & Bottom Of Cand.  */
         fepcb->auxacsz.itemsize = len;
         UdCalItemSize(fepcb);        /* Caculate Maximum Size Of All Cand. */
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

   itoa(fepcb->udstruct.more, buffer, sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fepcb->auxbufs[1], line, "", fepcb->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; item<10 && *candptr!=EN_GP_END_MARK; item++ )
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
        }while( (*candptr++ & 0x80) != NULL);
   }
                                            
   memcpy(fepcb->auxbufs[AUXROWMAX-2], bottom1, sizeof(bottom1));
                                           /* Fill Bottom Message          */
   memcpy(fepcb->auxbufs[AUXROWMAX-1], bottom2, sizeof(bottom2));

   fepcb->auxacsz.itemnum = AUXROWMAX;
   fepcb->auxchfg = ON;
}


/******************************************************************************/
/* FUNCTION    : UdCalItemSize                                                */
/* DESCRIPTION : Caculate maximum size of all candidates.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdCalItemSize(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   *candptr;           /* Pointer To Candidate               */
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   candptr = fepcb->udstruct.cand;
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
/* FUNCTION    : UdGetCandidate                                               */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdGetCandidate(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   unsigned char  *ptr, *getptr;       /* Pointer To Selected Candidate      */
   unsigned short  i;                  /* Loop Counter                       */
   unsigned short  getnum;             /* Selected Number                    */
   int len=0;

   getnum = key - 0x30;                /* Calculate Selected Number          */
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
   while(*ptr != 0x20 && *ptr != '\0')
   {
       len++;
       ptr++;
   }
   strncpy(fepcb->edendbuf, getptr, len);
   fepcb->edendacsz = len;

   UdEraseAllRadical(fepcb);
   fepcb->ret = FOUND_WORD;

}

/******************************************************************************/
/* FUNCTION    : UdCloseAux                                                   */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

UdCloseAux(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   char            **toauxs;           /* Pointer To Aux. String Buffer      */
   char            **toauxa;           /* Pointer To Aux. Attribute Buffer   */
   int i;

   toauxs = fepcb->auxbufs;
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

/***************************************************************************/
/* FUNCTION    : UdFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for User Defined candidates         */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

UdFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->udstruct.cand != NULL )
   {
      /****************************/
      /*   initial ud structure   */
      /****************************/
      free(fepcb->udstruct.cand);
      fepcb->udstruct.cand=NULL;
      fepcb->udstruct.curptr=NULL;
      fepcb->udstruct.allcandno=0;
      fepcb->udstruct.more=0;

   }
}

/****************************************************************************/
/* FUNCTION    : itoa                                                       */
/* DESCRIPTION : integer convert to string                                  */
/* EXTERNAL REFERENCES:                                                     */
/* INPUT       : no, ptr, len                                               */
/* OUTPUT      :                                                            */
/* CALLED      :                                                            */
/* CALL        :                                                            */
/****************************************************************************/
/*
itoa(no, ptr, len)
int no;
char *ptr;
int len;
{
    long pow;
    int i,j;

    memset(ptr, ' ', len);
    if (no == 0)
    {
       *(ptr+len-1) = '0';
       return;
    }

    for (i=0; i<len; i++)
    {
       pow = 1;
       for (j=0; j<len-1-i; j++)
         pow = pow*10;
       *(ptr+i) = (no / pow)%10+48;
    }

    while (*ptr == '0')
       *ptr++ = ' ';
}
*/

