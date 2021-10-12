static char sccsid[] = "@(#)64	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedud.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:59";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: BackSpaceCB
 *		CursorCB
 *		ErrorBeepCB
 *		NoWordCB
 *		NonConvertCB
 *		UdCloseAuxCB
 *		UdEraseAllRadicalCB
 *		UdEraseCurRadicalCB
 *		UdFreeCandCB
 *		UdGetCandCB
 *		UdInitialCB
 *		UdListBoxCB
 *		UdOtherInitialCB
 *		UdRadicalInputCB
 *		UdShowCursorCB
 *		halffullCB
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
/* MODULE NAME:        zhedud(CallBack)                                       */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For User Defined CallBack Mode    */
/*                                                                            */
/* FUNCTION:           OtherInitialCB      : Initialization other IM          */
/*                                                                            */
/*                     UdInitialCB         : Initialize User_Defined Input    */
/*                                           Method                           */
/*                                                                            */
/*                     UdEraseCurRadicalCB : Erase Current Radicals           */
/*                                                                            */
/*                     UdEraseAllRadicalCB : Erase all radicals at            */
/*                                           Off_The_Spot.                    */
/*                                                                            */
/*                     UdRadicalInputCB    : Replace/Insert Radicals To Echo  */
/*                                           Buf                              */
/*                                                                            */
/*                     BackspaceCB         : Erase previouse radical          */
/*                                                                            */
/*                     NonConvertCB        : Process No Convert key Mode      */
/*                                                                            */
/*                     ErrorBeepCB         : Display error message and beep on*/
/*                                                                            */
/*                     UdListBoxCB         : Process UD List Box              */
/*                                                                            */
/*                     UdGetCandCB         : Send Selected Cand. To Output    */
/*                                           Buffer                           */
/*                                                                            */
/*                     UdCloseAuxCB        : Close Candidate List Box         */
/*                                                                            */
/*                     UdFreeCandCB        : Free alloacted memory for User   */
/*                                           Defined Condidates               */
/*                                                                            */
/*                     CursorCB            : Process cursor in the Pre-edit   */
/*                                                                            */
/*                     NoWordCB            : Process no word error            */
/*                                                                            */
/*                     UdShowCursorCB      : Initialized                      */
/*                                                                            */
/*                     halffullCB          : Process Full/Half key            */
/*                                                                            */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/

#include  "zhed.h"
#include  "zhedacc.h"
#include  "zhedud.h"
#include  "chinese.h"
#include  "im.h"
#include  "imP.h"

extern FEPCB      *fep;

/******************************************************************************/
/* FUNCTION    : UdOtherInitialCB                                             */
/* DESCRIPTION : Entry Point of Other Input Method.                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :  key                                                         */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

int  UdOtherInitialCB(key)
unsigned short    key;
{

   switch( key )
   {
      case ALPHA_NUM_KEY :
           UdFreeCandCB();             /* Free User Defined  Candidates      */
                                       /* Buffer                             */
           AnInitial(fep);           /* Initialize Alph_Num Input Method   */
           break;

      case PINYIN_KEY :
           UdFreeCandCB();             /* Free User Defined Candidates       */
                                       /* Buffer                             */
           PyInitial(fep);           /* Initialize Pinyin Input Method     */
           break;

      case ENGLISH_CHINESE_KEY :
           UdFreeCandCB();             /* Free User Defined Candidates       */
                                       /* Buffer                             */
           EnInitial(fep);           /* Initialize English_Chinese Input   */
                                       /* Method                             */
           break;

      case ABC_KEY :
           UdFreeCandCB();             /* Free User Defined Candidates       */
                                       /* Buffer                             */
           AbcInitial(fep);          /* Initialize ABC Input Method        */
           break;

      case USER_DEFINED_KEY :
           UdFreeCandCB();             /* Free User Defined Candidates       */
                                       /* Buffer                             */
           (*udimcomm->UserDefinedInitial)();/* Initialize User_Defined Input*/
                                       /* Method                             */
           break;

      case TSANG_JYE_KEY :
           UdFreeCandCB();             /* Free User Defined Candidates       */
                                       /* Buffer                             */
           TjInitial(fep);           /* Initialize Tsang_Jye Input Method */
           break;

      case FULL_HALF_KEY :             /* Set FULL/HALF Mode                 */
           fep->indchfg = TRUE;

           if ( fep->inputlen > 0 )
           {
              fep->imode.ind5 = ERROR1;
              fep->isbeep = ON;
              break;
           }

           if( fep->imode.ind1 == HALF )
             fep->imode.ind1 = FULL;
           else
             fep->imode.ind1 = HALF;
           break;
   }

}


/******************************************************************************/
/* FUNCTION    : UdShowCursorCB                                               */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fep                                                        */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
int  UdShowCursorCB()
{
   if (fep->eccrpsch == ON)         /*  for  Showing  Cursor   */
   {
      memset(fep->echobufa, REVERSE_ATTR, fep->echoacsz);
      if (fep->echocrps < fep->echoacsz)
      {
         memset(fep->echobufa, REVERSE_ATTR, fep->echoacsz);
         fep->echobufa[fep->echocrps] = 1;
         fep->echochfg.flag = ON;
         fep->echochfg.chtoppos = 0;
         fep->echochfg.chlenbytes = 1;
      }
   }
}


/******************************************************************************/
/* FUNCTION    : halffullCB                                                   */
/* DESCRIPTION : Process Full/Half key.                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fep                                                        */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
int  halffullCB(key)
unsigned short       key;
{
         AnProcess(fep, key);
}
     

/******************************************************************************/
/* FUNCTION    : BackSpaceCB                                                  */
/* DESCRIPTION : Process backspace key                                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  BackSpaceCB(key)
unsigned short key;
{
   unsigned short  cnt;                /* Byte Counter                       */

   if( fep->echocrps > 0 )
   {
       for( cnt = fep->echocrps-1; cnt <= fep->echoacsz-1; cnt++ )
       {
          *(fep->echobufs+cnt) = *(fep->echobufs+cnt+1);
          *(fep->echobufa+cnt) = *(fep->echobufa+cnt+1);
       }
       fep->echochfg.flag = ON;
       fep->echochfg.chtoppos = fep->echocrps-1;
       fep->echochfg.chlenbytes = fep->echoacsz-(fep->echocrps-1);
       fep->echocrps -= 1;
       fep->eccrpsch = ON;
       fep->echoacsz -= 1;
       fep->echoover += 1;
       fep->inputlen -= 1;
    }
    else
    {
       AnProcess(fep, key);
       return;
    }

 }

/******************************************************************************/
/* FUNCTION    : NonConvertCB                                                 */
/* DESCRIPTION : Process No Convert key's mode                                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  NonConvertCB()
{
   unsigned short  cnt;                /* Byte Counter                       */

   for( cnt = 0; cnt < fep->echoacsz; cnt++)
        *( fep->edendbuf + cnt ) = *( fep->echobufs + cnt);
   fep->edendacsz = fep->echoacsz;
   UdEraseAllRadicalCB();
   fep->ret = FOUND_WORD;
}


/******************************************************************************/
/* FUNCTION    : CursorCB                                                     */
/* DESCRIPTION : Move cursor in the Pre-edit                                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :        flag                                                  */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int   CursorCB(key, flag)
  unsigned short  key;                /*  A key  which the user pressed      */
                                       /* (Ascii Code)                       */
  unsigned short  flag;                /*  A key  which the user pressed      */
                                       /* (Ascii Code)                       */
{
    switch(flag)
    {
      case LEFT_ARROW_FLAG :
         if( fep->echocrps > 0 && fep->inputlen > 0 )
         {
            fep->echocrps -= 1;
            fep->eccrpsch = ON;
            break;
         }
         else
         {
            AnProcess(fep, key);
            return;
         }

      case RIGHT_ARROW_FLAG :
         if( fep->echocrps < fep->echoacsz  && fep->inputlen > 0)
         {
            fep->echocrps += 1;
            fep->eccrpsch = ON;
            break;
         }
         else
         {
            AnProcess(fep, key);
            return;
         }
   }

}


/******************************************************************************/
/* FUNCTION    : UdEraseCurRadicalCB                                          */
/* DESCRIPTION : Erase Radical                                                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fep                                                        */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
int  UdEraseCurRadicalCB()
{
   unsigned short     cnt;

   if( fep->echocrps < fep->echoacsz )
   {
       for( cnt = fep->echocrps; cnt <= fep->echoacsz-1; cnt++ )
       {
            *(fep->echobufs+cnt) = *(fep->echobufs+cnt+1);
       }
       fep->echochfg.flag = ON;
       fep->echochfg.chtoppos = fep->echocrps;
       fep->echochfg.chlenbytes = fep->echoacsz-fep->echocrps;
       fep->echoacsz -= 1;
       fep->echoover += 1;
       fep->inputlen -= 1;
   }
   else
       ErrorBeepCB();

}


/******************************************************************************/
/* FUNCTION    : UdEraseAllRadicalCB                                          */
/* DESCRIPTION : Erase all radicals at  Off_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fep                                                        */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

int  UdEraseAllRadicalCB()
{
                                       /* Clear Echo Buffer                  */
   (void)memset(fep->echobufs, NULL, fep->echosize);
   (void)memset(fep->echobufa, NULL, fep->echosize);

   fep->echochfg.flag = TRUE;
   fep->echochfg.chtoppos = 0;
   fep->echochfg.chlenbytes = fep->echoacsz;
   fep->echocrps = 0;
   fep->eccrpsch = ON;
   fep->echoacsz = 0;
   fep->echoover = fep->echosize;
   fep->inputlen = 0;
}



/******************************************************************************/
/* FUNCTION    : ErrorBeepCB                                                  */
/* DESCRIPTION : Display error message and beep on                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
int  ErrorBeepCB()
{
   fep->isbeep = BEEP_ON;    /* Notify FEP To Beep To Alarm        */
   fep->indchfg = ON;         
   fep->imode.ind5 = ERROR1;
}


/******************************************************************************/
/* FUNCTION    : UdInitialCB                                                  */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  UdInitialCB()
{
      /* Erase All Radicals At Over_The_Spot and  Clear Edit_End Buffer   */
      UdEraseAllRadicalCB();

      if ( fep->edendacsz == 0 )
      {
         (void)memset(fep->edendbuf, NULL, fep->echosize);
         fep->edendacsz = 0;
      }
      if ( fep->auxchfg != ON )
         fep->auxchfg = OFF;

      fep->auxuse = NOTUSE;
      fep->auxcrpsch = OFF;
      fep->auxacsz.itemsize = 0;
      fep->auxacsz.itemnum = 0;
      fep->indchfg = ON;
      fep->imode.ind0 = USER_DEFINED_MODE;
      fep->imode.ind2 = SELECT_OFF;
      fep->imode.ind4 = REPLACE_MODE;
      fep->imode.ind5 = BLANK;
      fep->imode.ind6 = INPUT_NO_SELECTED;
      fep->isbeep = BEEP_OFF;
      fep->inputlen = 0;
}



/******************************************************************************/
/* FUNCTION    : NoWordCB                                                     */
/* DESCRIPTION : Display no word error message and beep on                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  NoWordCB()
{
        fep->indchfg = ON;
        fep->imode.ind5 = ERROR2;  /* Display Error Message           */
        fep->isbeep = ON;
        fep->echocrps = 0;
        fep->eccrpsch = ON;
}

/******************************************************************************/
/* FUNCTION    : UdRadicalInputCB                                             */
/* DESCRIPTION : Inputs the key which the user pressed to echo buffer.        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :        key                                                   */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

int  UdRadicalInputCB(key, flag)
unsigned short  key;       /*  A key which user pressed( ASCII    */
                           /* code )                             */
unsigned short     flag;
{
unsigned short     i;                  /* Loop Counter                       */

   fep->imode.ind4 = flag;
   if( fep->imode.ind4 == REPLACE_MODE )
   {
      if( (fep->echocrps < udimcomm->maxlen && fep->inputlen == udimcomm->maxlen)
          || (fep->inputlen < udimcomm->maxlen) )
      {
                                        /* Replace Radical To Echo Buffer     */

         *(fep->echobufs+fep->echocrps) = key;
         *(fep->echobufa+fep->echocrps) = REVERSE_ATTR;

         fep->echochfg.flag = ON;  /* Update Internal Informations       */
         fep->echochfg.chtoppos = fep->echocrps;
         fep->echochfg.chlenbytes = 1;

         if( fep->echocrps >= fep->echoacsz )
         {
            fep->inputlen ++;
            fep->echoacsz += 1;
            fep->echoover -= 1;
         }
         fep->echocrps += 1;
         fep->eccrpsch = ON;
         udimcomm->inputlen = fep->inputlen;
      }
      else
         ErrorBeepCB();
   }
   else  /* Insert Mode */
   {
      if( (fep->echocrps < udimcomm->maxlen && fep->inputlen == udimcomm->maxlen)
          || (fep->inputlen < udimcomm->maxlen) )
      {
                                       /* Shift Content Of Echo Buffer Right */
         for( i=fep->echoacsz+1; i>=fep->echocrps+1; i-- )
         {
            *(fep->echobufs+i) = *(fep->echobufs+i-1);
            *(fep->echobufa+i) = *(fep->echobufa+i-1);
         }
                                       /* Insert Radical To Echo Buffer      */
         *(fep->echobufs+fep->echocrps) = key;

         if (fep->inputlen < udimcomm->maxlen)
         {
            fep->echoacsz += 1;
            fep->inputlen++;
         }
                                    /* Update Internal Informations       */
         fep->echochfg.flag = ON;
         fep->echochfg.chtoppos = fep->echocrps;
         fep->echochfg.chlenbytes = fep->echoacsz-fep->echocrps;
         fep->echocrps += 1;
         fep->eccrpsch = ON;
         fep->echoover -= 1;
         udimcomm->inputlen = fep->inputlen;
      }
      else
         ErrorBeepCB();
   }
}


/******************************************************************************/
/* FUNCTION    : UdListBoxCB                                                  */
/* DESCRIPTION : Process Candidate List Box.                                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

int  UdListBoxCB()
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
   unsigned short  pitemsize;          /* Item Size Of Previous Candidate    */
   unsigned short  nitemsize;          /* Item Size Of Next Candidate        */

   static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
              0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
              0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
              0x20,0x39,0x20,0x20};

   len = strlen(UDTITLE)+4;   /* Length Of Title & Bottom Of Cand.  */
   fep->auxacsz.itemsize = len;

   /* Calculate Maximum Size Of All Cand. */
   pitemsize = 4;
   nitemsize = 4;                      

   for( ;*candptr != NULL; candptr++ )
   {
      if (*candptr == UTF_BYTE1)        
          nitemsize -= 2;
      nitemsize ++;

      if( (*candptr & 0x80) == NULL )
      {
         if( pitemsize < nitemsize )  pitemsize = nitemsize;
         nitemsize = 4;         
      }
   }

   if( fep->auxacsz.itemsize < pitemsize )
     fep->auxacsz.itemsize = pitemsize;


   toauxs = fep->auxbufs;
   toauxa = fep->auxbufa;
                                                   /*  clear aux buffer   */
   if (fep->auxacsz.itemnum != 0)
      for( i = 0 ; i< fep->auxacsz.itemnum-2; i++ )
          memset( *toauxs++,' ',fep->auxacsz.itemsize+1);

   toauxs = fep->auxbufs;
   toauxa = fep->auxbufa;

   for(k = 0; k < 8 ; k++)
   {
       temp = fep->auxbufs[k];
       for(m = 0; m < fep->auxacsz.itemsize; memcpy(temp," ",1),
           temp++,m++);
   }/* for */              
                                       /* Fill Title                      */
   memcpy(fep->auxbufs[0], UDTITLE, strlen(UDTITLE));
   tostring = fep->auxbufs[0]+strlen(UDTITLE);

   itoa(udimcomm->more, buffer, sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line                       */
   memccpy(fep->auxbufs[1], LINE, "", fep->auxacsz.itemsize);

                                    /* Fill Candidates To Aux. Buffer     */
   for( item=0; item<10; item++ )
   {
      itemsize = 0;
      tostring = fep->auxbufs[item+2];
      candptr = udimcomm->candbuf[item];
      if(*candptr == '\0')
         break;
                                    /* Fill Number                        */
      for( j=item*4; j<item*4+4; j++ )
      {
         itemsize ++;
         *tostring++ = *(digit+j);
      }

      do{                           /* Fill Phrase                        */
         itemsize++;
         *tostring++ = ( *candptr | 0x80);
        }while( (*candptr++ != '\0') && (itemsize < (AUXCOLMAX - 4)));
   }
                                            
   memcpy(fep->auxbufs[AUXROWMAX-2], BOTTOM1, strlen(BOTTOM1));
                                           /* Fill Bottom Message          */
   memcpy(fep->auxbufs[AUXROWMAX-1], BOTTOM2, strlen(BOTTOM2));

   fep->auxacsz.itemnum = AUXROWMAX;
   fep->auxchfg = ON;
   fep->auxuse  = USE;
}


/******************************************************************************/
/* FUNCTION    : UdGetCandCB                                                  */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :        key                                                   */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  UdGetCandCB(udata)
unsigned char   *udata;
{
   unsigned char  *ptr, *getptr;       /* Pointer To Selected Candidate      */
   unsigned short  i;                  /* Loop Counter                       */
   unsigned short  getnum;             /* Selected Number                    */
   int len=0;

   ptr = udata;
   if(*ptr == 0x20 || *ptr == NULL)
   {
      ErrorBeepCB();
      return;
   }

   while(*ptr != 0x20 && *ptr != '\0')
   {
       len++;
       ptr++;
   }
   strncpy(fep->edendbuf, udata, len);
   fep->edendacsz = len;

   UdEraseAllRadicalCB();
   fep->ret = FOUND_WORD;

}

/******************************************************************************/
/* FUNCTION    : UdCloseAuxCB                                                 */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : fep                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int  UdCloseAuxCB()
{
   char            **toauxs;           /* Pointer To Aux. String Buffer      */
   char            **toauxa;           /* Pointer To Aux. Attribute Buffer   */
   int i;

   toauxs = fep->auxbufs;
   toauxa = fep->auxbufa;
   for( i=0; i<AUXROWMAX; i++ )
   {
      (void)memset(*toauxs++, NULL, AUXCOLMAX);
      (void)memset(*toauxa++, NULL, AUXCOLMAX);
   }
   fep->auxacsz.itemnum = 0;
   fep->auxacsz.itemsize = 0;
   fep->imode.ind2 = SELECT_OFF;
   fep->echocrps = 0;
   fep->eccrpsch = ON;
   fep->auxuse = NOTUSE;
   fep->auxchfg = ON;
}

/***************************************************************************/
/* FUNCTION    : UdFreeCandCB                                              */
/* DESCRIPTION : Free allocated memory for User Defined candidates         */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       :                                                           */
/* OUTPUT      : fep                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/
int  UdFreeCandCB()
{
   if ( fep->udstruct.cand != NULL )
   {
      /****************************/
      /*   initial ud structure   */
      /****************************/
      free(fep->udstruct.cand);
      fep->udstruct.cand=NULL;
      fep->udstruct.curptr=NULL;
      fep->udstruct.allcandno=0;
      fep->udstruct.more=0;

   }
}
