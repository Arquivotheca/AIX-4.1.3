static char sccsid[] = "@(#)15  1.5  src/bos/usr/lib/nls/loc/imt/tfep/tedstj.c, libtw, bos411, 9428A410j 4/21/94 02:17:31";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: tedstj.c
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
/* MODULE NAME:        TedStj                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Simplify-Tsang-Jye Mode       */
/*                                                                            */
/*                                                                            */
/*                     StjCandidates   : Find Satisfied Candidates            */
/*                                                                            */
/*                     StjSelect       : Process Input Key On Cand. List Box  */
/*                                                                            */
/*                     StjListBox      : Process PageUp/PageDown On List Box  */
/*                                                                            */
/*                     StjGetCandidate : Send Selected Cand. To Output Buffer */
/*                                                                            */
/*                     StjEraseAux     : Close Candidate List Box             */
/*                                                                            */
/*                     StrokeCandidates: Find Satisfied Candidates            */
/*                                                                            */
/*                     StrokeSelect    : Process Input Key On Cand. List Box  */
/*                                                                            */
/*                     StrokeListBox   : Process PageUp/PageDown On List Box  */
/*                                                                            */
/*                     StrokeGetCandidate: Process Selected Candidate         */
/*                                                                            */
/*                     itoa            : Make integer Convret To String       */
/*                                                                            */
/*                                                                            */
/* MODULE TYPE:        C                                                      */
/*                                                                            */
/* COMPILER:           AIX C                                                  */
/*                                                                            */
/* AUTHOR:             Terry Chou                                             */
/*                                                                            */
/* STATUS:             Chinese Input Method Version 1.0                       */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                     Modified by Jim Roung (IBM-T)                          */
/*  V410  06/14/93'    Modified by Debby Tseng (Mirrors) for Input Method     */
/*                     Learning and TSANG JYE duplicate                       */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include "ted.h"
#include "tedinit.h"
#include "tedacc.h"
#include "msgextrn.h"    /* @big5 */
unsigned char flag=0;;  /* V410 */

/******************************************************************************/
/* FUNCTION    : StjCandidates                                                */
/* DESCRIPTION : Fill 10 stj candidates to auxbuf if there are candidates     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StjCandidates(fepcb)
FEPCB *fepcb;
{

      if (fepcb->starpos !=0 )                   /* Simplify Tsange Jye V410 */
        AccessDictionary(fepcb);


    if ( fepcb->ret == FOUND_CAND )
    {
       if (fepcb->stjstruct.allcandno == 1)   /*  only one candidate       */
       {

       /* ===============>>> Marked By Jim Roung <<<====================
          memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
          if ( (fepcb->stjstruct.stjcand->euc[0] == EUC_BYTE1) &&
               (fepcb->Lang_Type == codesetinfo[0].code_type) )
             fepcb->edendacsz = 4;
          else
             fepcb->edendacsz = 2;

          fepcb->ret = FOUND_WORD;
       /*==================>>> The End of Marked <<<======================*/

          memset(fepcb->preinpbuf, NULL, strlen(fepcb->preinpbuf));      /*  V410 */
          strncpy(fepcb->preinpbuf,fepcb->curinpbuf,strlen(fepcb->curinpbuf)); /* V410 */
          StjGetCandidate(fepcb,0x30);  /* Added By Jim Roung   */
          TjEraseAllRadical(fepcb);
       }
       else     /* more than one candidate selected     */
       {
                            /* check learning flag                      V410 */
           if ( ((fepcb->learning) &&                               /*  V410 */
               (fepcb->starpos !=0))                                /*  V410 */
               && !flag)                                            /*  V410 */
           {                                                        /*  V410 */
              access_learn_data(fepcb);                             /*  V410 */
           }                                                        /*  V410 */

          fepcb->imode.ind2 = STJ_SELECT_ON;
          memset(fepcb->preinpbuf, NULL, strlen(fepcb->preinpbuf)); /*  V410 */
          strncpy(fepcb->preinpbuf,fepcb->curinpbuf,strlen(fepcb->curinpbuf));
          StjListBox(fepcb, BEGINNING);
       }
    }
    else if ( fepcb->ret == NOT_FOUND )
         {
            fepcb->indchfg=ON;         /* set indictor flag on    */
            fepcb->imode.ind5=ERROR2;  /* error occur             */
            fepcb->isbeep = ON;
            fepcb->echocrps = 0;
            fepcb->eccrpsch = ON;
            fepcb->inputlen = 0;
         }
    else          /* debby */
                if ( fepcb->ret == FOUND_WORD )       /* the word found  */
                {
                   memset(fepcb->edendbuf,NULL,strlen(fepcb->preedbuf));
                   strcpy(fepcb->edendbuf,fepcb->preedbuf);  /* buffering */
                   fepcb->edendacsz = strlen(fepcb->edendbuf);
                   memset(fepcb->preinpbuf, NULL, strlen(fepcb->preinpbuf)); /* V410 */
                   strcpy(fepcb->preinpbuf,fepcb->curinpbuf); /* buffering */
                   TjEraseAllRadical(fepcb);
                }
  return;
}

/******************************************************************************/
/* FUNCTION    : StjListBox                                                   */
/* DESCRIPTION : Process PageUp and PageDown On STJ Candidate List Box.       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
StjListBox(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code (PGUP_KEY/PGDN_KEY)     */
{
/* static char title[]={0xf6,0xfc,0xcd,0xf8,0xd4,0xbf,0xef,0xee,0xf2,0xd9,0xf0,
                 0xe5,0x20,0x20,0xde,0xb7,0xef,0xf5};          @big5 */

/* static char Tj_title[]={0x20,0x20,0x20,0x20,0xd4,0xbf,0xef,0xee,0xf2,0xd9,0xf0,
                 0xe5,0x20,0x20,0xde,0xb7,0xef,0xf5};             V410 @big5 */

/* static char line[]={0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1  }; @big5 */

/* static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
              0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
              0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
              0x20,0x39,0x20,0x20 };                                       @big5 */

/* static char bottom1[]={0xcc,0xbd,0xd6,0xbc,0x28,0x45,0x73,0x63,0x29,
                   0x20,0x20,0x20,0xe1,0xad,0xe8,0xa2,0x28,0xa2,0xb0,0x29};  @big5 */

/* static char Tj_bottom1[]={0xcc,0xbd,0xd6,0xbc,0x28,0x45,0x73,0x63,0x29,  V410 @big5 */
/*                 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}; V410 @big5 */

/*   static char bottom2[]={0xd0,0xa9,0xd3,0xf7,0x28,0x50,0x67,0x55,0x70,
                   0x29,0x20,0x20,0xc8,0xb9,0xd3,0xf7,0x28,0x50,0x67,0x44,
                   0x6e,0x29};                                          /* @big5 */

   unsigned char   **toauxs;           /* Pointer To Aux. String             */
   unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
   unsigned char   *tostring;          /* Pointer To Aux. String             */
   unsigned char   *toattribute;       /* Pointer To Aux. Attribute          */
   unsigned char   buffer[4];
   StjCand *stj;
   int i;
   int j;
   int len;
   unsigned int k,m;
   unsigned char *temp;
   static int stj_dictionary_index=0,stj_learn_index;              /* V410 */
   static int pre_stj_dictionary_index=0;                          /* V410 */
   static int aft_stj_dictionary_index=0;                          /* V410 */

   len = Stj_title_LEN+sizeof(buffer);                             /* @big5 */
   if (((fepcb->learning)                                          /* V410  */
       && (fepcb->starpos !=0)) &&                                 /* V410  */
       !flag )                                                     /* V410  */
   {                                                               /* V410  */
      switch ( key )                                               /* V410  */
      {                                                            /* V410  */
        case BEGINNING:                                            /* V410  */
            fepcb->stjstruct.headcandno = 0;                       /* V410  */
            if (fepcb->stjstruct.allcandno < 10)                   /* V410  */
            {                                                      /* V410  */
                for (i=0;i<fepcb->stjstruct.allcandno;i++)         /* V410  */
                   fepcb->learnstruct.list_index[i] =              /* V410  */
                                   fepcb->learnstruct.index[i];    /* V410  */
                stj_learn_index =fepcb->stjstruct.allcandno;       /* V410  */
                fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno - 1; /*V410*/
                stj_dictionary_index = stj_learn_index;            /* V410  */
            }                                                      /* V410  */
            else                                                   /* V410  */
            {                                                      /* V410  */
                for (i=0;i<LIST_NO ;i++ )                          /* V410  */
                  fepcb->learnstruct.list_index[i] =               /* V410  */
                                   fepcb->learnstruct.index[i];    /* V410  */
                fepcb->stjstruct.tailcandno = 9;                   /* V410  */
                stj_learn_index = 10;                              /* V410  */
                stj_dictionary_index =0 ;                          /* V410  */
            }                                                      /* V410  */
            pre_stj_dictionary_index = 0;                          /* V410  */
            aft_stj_dictionary_index = 0;                          /* V410  */
            break;                                                 /* V410  */

         case PGUP_KEY:                                            /* V410  */
            if (fepcb->stjstruct.headcandno == 0)                  /* V410  */
            {                                                      /* V410  */
                fepcb->imode.ind5=ERROR1;  /* error occur             V410  */
                fepcb->indchfg=ON;         /* set indictor flag on    V410  */
                fepcb->isbeep = ON;                                /* V410  */
                return;                                            /* V410  */
            }                                                      /* V410  */
            else                                                   /* V410  */
            {                                                      /* V410  */
              if (fepcb->stjstruct.headcandno > 9)                 /* V410  */
              {                          /* non-first page            V410  */
                 i = 0;                                            /* V410  */
                 if ((fepcb->stjstruct.headcandno-10) < LEARN_NO)   /* V410  */
                 {                        /* get order from LEARN file V410  */
                   for ( ;i<LIST_NO;i++)                            /* V410  */
                   {                                                /* V410  */
                      if ((i+fepcb->stjstruct.headcandno-10) == LEARN_NO) /* V410 */
                        break;                                      /* V410  */
                      fepcb->learnstruct.list_index[i] =            /* V410  */
                       fepcb->learnstruct.index[i+fepcb->stjstruct.headcandno-10]; /* V410  */
                   }                                                /* V410  */
                   if (i == LIST_NO)                                /* V410  */
                   {                                                /* V410  */
                     pre_stj_dictionary_index = 0;                  /* V410  */
                     aft_stj_dictionary_index = 0;                  /* V410  */
                   }                                                /* V410  */
                 }                                                  /* V410  */
                 if (i < LIST_NO)                                   /* V410  */
                 {                        /* get order from dictionary V410  */
                    k=9;                                            /* V410  */
                    aft_stj_dictionary_index = pre_stj_dictionary_index;  /* V410 */
                    for (stj_dictionary_index=pre_stj_dictionary_index -1 ;/* V410  */
                         stj_dictionary_index >= 0;                 /* V410  */
                         stj_dictionary_index-- )                   /* V410  */
                    {                                               /* V410  */
                                                                    /* V410  */
                       for (j=0;j<LEARN_NO ;j++ )                   /* V410  */
                         if (stj_dictionary_index ==
                                 fepcb->learnstruct.index[j])       /* V410  */
                              break;                                /* V410  */
                       if (j == LEARN_NO)                           /* V410  */
                       {                                            /* V410  */
                           fepcb->learnstruct.list_index[k] =
                                 stj_dictionary_index;              /* V410  */
                           if (k == 0)                              /* V410  */
                              break;                                /* V410  */
                           k--;                                     /* V410  */
                       }                                            /* V410  */
                    } /* endfor */                                  /* V410  */
                    pre_stj_dictionary_index = stj_dictionary_index ; /* V410 */
                 }                                                  /* V410  */
                 fepcb->stjstruct.headcandno = fepcb->stjstruct.headcandno - 10; /*V410 */
                 fepcb->stjstruct.tailcandno = fepcb->stjstruct.headcandno + 9;  /* V410*/
                 stj_learn_index -= 10;                             /* V410  */
              }                                                     /* V410  */
              else                                                  /* V410  */
              {                           /* display first page        V410  */
                fepcb->stjstruct.headcandno = 0;                    /* V410  */
                aft_stj_dictionary_index = 0;                       /* V410  */
                pre_stj_dictionary_index = 0 ;                      /* V410  */
                if (fepcb->stjstruct.allcandno < 10 )               /* V410  */
                {                                                   /* V410  */
                   for (i=0;i<fepcb->stjstruct.allcandno ;i++ )     /* V410  */
                     fepcb->learnstruct.list_index[i] =             /* V410  */
                                   fepcb->learnstruct.index[i];     /* V410  */
                   fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno-1; /* V410  */
                   stj_learn_index = fepcb->stjstruct.allcandno;      /* V410 */
                }                                                     /* V410 */
                else                                                  /* V410 */
                {                                                     /* V410 */
                     for (i=0;i<LIST_NO ;i++ )                        /* V410 */
                       fepcb->learnstruct.list_index[i] =             /* V410 */
                                      fepcb->learnstruct.index[i];    /* V410 */
                   fepcb->stjstruct.tailcandno = 9;                   /* V410 */
                   stj_learn_index = 10;                              /* V410 */
                }                                                     /* V410 */
              }                                                       /* V410 */
            }                                                         /* V410 */
            break;                                                    /* V410 */

         case PGDN_KEY:                                               /* V410 */
            if (fepcb->stjstruct.tailcandno == (fepcb->stjstruct.allcandno - 1)) /* V410 */
            {                                                         /* V410 */
                fepcb->imode.ind5=ERROR1;  /* error occur                V410 */
                fepcb->indchfg=ON;         /* set indictor flag on       V410 */
                fepcb->isbeep = ON;                                   /* V410 */
                return;                                               /* V410 */
            }                                                         /* V410 */
            else                                                      /* V410 */
            {                                                         /* V410 */
              fepcb->stjstruct.headcandno = fepcb->stjstruct.tailcandno + 1; /* V410 */
              if ((fepcb->stjstruct.headcandno+9) >= (fepcb->stjstruct.allcandno-1)) /* V410*/
              {                             /* display last page         V410 */
                  i=0;                                                /* V410 */
                  if ((fepcb->stjstruct.headcandno) < LEARN_NO)       /* V410 */
                  {                                                   /* V410 */
                    for (;i<LIST_NO;i++)                              /* V410 */
                    {                                                 /* V410 */
                      if ((i+fepcb->stjstruct.headcandno) == LEARN_NO)/* V410 */
                         break;                                       /* V410 */
                     fepcb->learnstruct.list_index[i] =               /* V410 */
                      fepcb->learnstruct.index[i+fepcb->stjstruct.headcandno];  /* V410 */
                    }                                                 /* V410 */
                  }                                                   /* V410 */

                  if(i<LIST_NO)                                       /* V410 */
                  {                                                   /* V410 */
                    pre_stj_dictionary_index = aft_stj_dictionary_index ; /* V410 */
                    for (stj_dictionary_index=aft_stj_dictionary_index;   /* V410  */
                      stj_dictionary_index<=fepcb->stjstruct.allcandno ;  /* V410  */
                      stj_dictionary_index++ )                        /* V410  */
                    {                                                 /* V410  */
                      for (j=0;j<LEARN_NO ;j++ )                      /* V410  */
                        if (stj_dictionary_index ==
                               fepcb->learnstruct.index[j])           /* V410  */
                           break;                                     /* V410  */
                      if (j == LEARN_NO)                              /* V410  */
                         fepcb->learnstruct.list_index[i++] =
                                               stj_dictionary_index;  /* V410  */
                      if (i == LIST_NO)                               /* V410  */
                        break;                                        /* V410  */
                     } /* endfor */                                   /* V410  */
                  aft_stj_dictionary_index = stj_dictionary_index;    /* V410 */
                  }                                                   /* V410  */
                  stj_learn_index = stj_learn_index +                 /* V410  */
                                    fepcb->stjstruct.allcandno -      /* V410  */
                                    fepcb->stjstruct.headcandno;      /* V410  */
                  fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno - 1; /*V410*/

              }                                                       /* V410 */
              else                                                    /* V410 */
              {                                                       /* V410 */
                  i=0;                                                /* V410 */
                  if ((fepcb->stjstruct.headcandno) < LEARN_NO)       /* V410 */
                  {                                                   /* V410 */
                     for ( ;i<LIST_NO ;i++)                           /* V410 */
                     {                                                /* V410 */
                      if ((i+fepcb->stjstruct.headcandno) == LEARN_NO)/* V410 */
                         break;                                       /* V410 */
                      fepcb->learnstruct.list_index[i] =              /* V410 */
                      fepcb->learnstruct.index[i+fepcb->stjstruct.headcandno];  /* V410 */
                     }                                                /* V410 */
                  }                                                   /* V410 */
                  if (i < LIST_NO)                                    /* V410 */
                  {                                                   /* V410 */
                     pre_stj_dictionary_index = aft_stj_dictionary_index ;  /* V410 */
                     for (stj_dictionary_index=aft_stj_dictionary_index; /* V410  */
                     stj_dictionary_index<=fepcb->stjstruct.allcandno ;/* V410  */
                     stj_dictionary_index++ )                          /* V410  */
                    {                                                 /* V410  */
                      for (j=0;j<LEARN_NO ;j++ )                      /* V410  */
                        if (stj_dictionary_index ==
                               fepcb->learnstruct.index[j])           /* V410  */
                           break;                                     /* V410  */
                      if (j == LEARN_NO)                              /* V410  */
                         fepcb->learnstruct.list_index[i++] =
                                           stj_dictionary_index;      /* V410  */
                      if (i == LIST_NO)                               /* V410  */
                        break;                                        /* V410  */
                     } /* endfor */                                   /* V410  */
                  aft_stj_dictionary_index = stj_dictionary_index+1;  /* V410 */
                  }                                                   /* V410 */
                  stj_learn_index +=10;                               /* V410  */
                  fepcb->stjstruct.tailcandno = fepcb->stjstruct.headcandno + 9; /* V410*/
              }                                                       /* V410 */
            }                                                         /* V410 */
            break;                                                    /* V410 */

         case RESTORE:                                                /* V410 */
            pre_stj_dictionary_index = 0;                             /* V410 */
            aft_stj_dictionary_index = fepcb->stjstruct.tailcandno+1; /* V410 */
            break;                                                    /* V410 */
      }                                                               /* V410 */
   }                                                                  /* V410 */
   else                                                               /* V410 */
   {                                                                  /* V410 */
     switch ( key )
     {
        case BEGINNING:
           fepcb->stjstruct.headcandno = 0;
           if (fepcb->stjstruct.allcandno < 10)
               fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno - 1;
           else
               fepcb->stjstruct.tailcandno = 9;
           break;

        case PGUP_KEY:
           if (fepcb->stjstruct.headcandno == 0)
           {
               fepcb->imode.ind5=ERROR1;  /* error occur             */
               fepcb->indchfg=ON;         /* set indictor flag on    */
               fepcb->isbeep = ON;
               return;
           }
           if (fepcb->stjstruct.headcandno > 9)
           {
               fepcb->stjstruct.headcandno = fepcb->stjstruct.headcandno - 10;
               fepcb->stjstruct.tailcandno = fepcb->stjstruct.headcandno + 9;
           }
           else
           {
               fepcb->stjstruct.headcandno = 0;
               if (fepcb->stjstruct.allcandno < 10)
                   fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno - 1;
               else
                   fepcb->stjstruct.tailcandno = 9;
           }
           break;

        case PGDN_KEY:
           if (fepcb->stjstruct.tailcandno == (fepcb->stjstruct.allcandno - 1))
           {
               fepcb->imode.ind5=ERROR1;  /* error occur             */
               fepcb->indchfg=ON;         /* set indictor flag on    */
               fepcb->isbeep = ON;
               return;
           }
           fepcb->stjstruct.headcandno = fepcb->stjstruct.tailcandno + 1;
           if ((fepcb->stjstruct.headcandno+9) >= (fepcb->stjstruct.allcandno-1))
              fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno - 1;
           else
              fepcb->stjstruct.tailcandno = fepcb->stjstruct.headcandno + 9;
           break;

        case RESTORE:
           break;
     }
   }                                                                   /* V410 */

   toauxs = fepcb->auxbufs;       /*   Clear Aux Buffer                      */
   toauxa = fepcb->auxbufa;

   for(k = 0; k < 14; k++) /* Added By Jim Roung ------ April/4/92 */
   {
       temp = fepcb->auxbufs[k];
       for(m = 0; m < fepcb->auxacsz.itemsize; memcpy(temp," ",1),
           temp++,m++);
   }/* for */   /* End of modification ----------------------------- */
   if (fepcb->auxacsz.itemnum != 0)
      for( i=0 ; i< fepcb->auxacsz.itemnum -2; i++ )
           memset(*toauxs++, ' ', fepcb->auxacsz.itemsize);

   fepcb->auxacsz.itemsize = len;
   fepcb->auxacsz.itemlen =  len;                              /* @big5   */
   fepcb->auxacsz.itemnum = AUXROWMAX;

                                        /* Fill Title Message              */
   if (fepcb->starpos != 0)                                     /* V410 */
   {                                    /* Simplify Tsang Jye      V410 */
      memcpy(fepcb->auxbufs[0], Stj_title, Stj_title_LEN);      /* @big5 */
      tostring = fepcb->auxbufs[0]+Stj_title_LEN;               /* @big5 */
   }                                                            /* V410 */
   else                                                         /* V410 */
   {                                    /* Tsang Jye Duplicate     V410 */
      memcpy(fepcb->auxbufs[0], Tj_title, Tj_title_LEN);        /* V410 */
      tostring = fepcb->auxbufs[0]+Tj_title_LEN;                /* V410 */
   }                                                            /* V410 */

   itoa(fepcb->stjstruct.allcandno-fepcb->stjstruct.tailcandno-1, buffer
        , sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line Message                 */
   memccpy(fepcb->auxbufs[1], Stj_line, "", len);                    /* @big5 */

   if ( ((fepcb->learning)                                           /* V410 */
       && (fepcb->starpos !=0))                                      /* V410 */
       && !flag)                                                     /* V410 */
   {                                                                 /* V410 */
     for (i=0;
     (i<LIST_NO) && (i <= fepcb->stjstruct.tailcandno - fepcb->stjstruct.headcandno);
     i++ )                                                           /* V410 */
     {                                                               /* V410 */
      tostring = fepcb->auxbufs[i+2];                                /* V410 */
                                      /* Fill Number                    V410 */
      for(j=i*4; j<i*4+4; j++)                                       /* V410 */
         *tostring++ = *(Stj_digit+j);                          /* V410 @big5*/
      stj = fepcb->stjstruct.stjcand + fepcb->learnstruct.list_index[i]; /* V410 */

      if ((stj->euc[0] == EUC_BYTE1 ) &&    /*  Fill Data             V410 */
          (fepcb->Lang_Type == codesetinfo[0].code_type))            /* @big5*/
         memcpy(tostring, stj->euc, sizeof(stj->euc));               /* V410 */
      else                                                           /* V410 */
         memcpy(tostring, stj->euc, strlen(stj->euc));               /* V410 */
     } /* endfor */                                                  /* V410 */
   }                                                                 /* V410 */
   else                                                              /* V410 */
   {                                                                 /* V410 */
   stj = fepcb->stjstruct.stjcand + fepcb->stjstruct.headcandno;
   for (i=0; i <= fepcb->stjstruct.tailcandno-fepcb->stjstruct.headcandno; i++)
   {
      tostring = fepcb->auxbufs[i+2];
                                      /* Fill Number                        */
      for(j=i*4; j<i*4+4; j++)
         *tostring++ = *(Stj_digit+j);                              /* @big5 */
      if ((stj->euc[0] == EUC_BYTE1 ) &&  /*  Fill Data                    */
          (fepcb->Lang_Type == codesetinfo[0].code_type))            /* @big5*/
         memcpy(tostring, stj->euc, sizeof(stj->euc));
      else
         memcpy(tostring, stj->euc, strlen(stj->euc));
      stj++;
   }
   }                                                                 /* V410 */

   if (key == BEGINNING || key == RESTORE || key == PGUP_KEY || key == PGDN_KEY)
   {
      for (i=fepcb->auxacsz.itemnum-2; i<fepcb->auxacsz.itemnum; i++)
        memset(fepcb->auxbufs[i], ' ', fepcb->auxacsz.itemsize);
                                           /* Fill Bottom Message         */
     if (fepcb->starpos != 0)                                     /* V410 */
     {                                    /* Simplify Tsang Jye      V410 */
        memcpy(fepcb->auxbufs[AUXROWMAX-2], Stj_bottom1, Stj_bottom1_LEN);
                                                                  /* @big5 */
     }                                                            /* V410 */
     else                                                         /* V410 */
     {                                   /* Tsang Jye Duplicate      V410 */
        memcpy(fepcb->auxbufs[AUXROWMAX-2], Tj_bottom1, Tj_bottom1_LEN); /* V410 */
     }                                                            /* V410 */
                                           /* Fill Bottom Message          */
      memcpy(fepcb->auxbufs[AUXROWMAX-1], Stj_bottom2, Stj_bottom2_LEN);
                                                                   /* @big5 */
   }
   fepcb->auxchfg=ON;
   fepcb->auxuse=USE;

}

/******************************************************************************/
/* FUNCTION    : StjSelect                                                    */
/* DESCRIPTION : Process input key on candidate list box.                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StjSelect(fepcb,key)
FEPCB *fepcb;
{
   switch( key )
   {
     case ADD_KEY :
         if (fepcb->starpos != 0)    /* duplicate Tsang-Jye not support it V410 */
           StrokeCandidates(fepcb);
         else
         {
           fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm        V410*/
           fepcb->indchfg = ON;       /*                                   V410*/
           fepcb->imode.ind5 = ERROR1; /* For Error Message display        V410*/
         }
         break;

      case PGUP_KEY:
         StjListBox(fepcb, key);
         break;

      case PGDN_KEY:
         StjListBox(fepcb, key);
         break;

      case ESC_KEY:
         StjEraseAux(fepcb);
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
         if (fepcb->stjstruct.headcandno+key-0x30 >
              fepcb->stjstruct.allcandno-1)
         {
            fepcb->indchfg = ON;        /* Added By Jim Roung    */
            fepcb->imode.ind5 = ERROR1; /* For Error Msg display */
            fepcb->isbeep = BEEP_ON;
         }
         else
         {
            if ((fepcb->learning) &&                                  /* V410 */
                 (fepcb->starpos))                                    /* V410 */
              StjUpdateLearningData(fepcb,key);                       /* V410 */
            StjGetCandidate(fepcb, key);
            TjEraseAllRadical(fepcb);
            StjEraseAux(fepcb);
         }
         break;

      default :
         fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm        */
         fepcb->indchfg = ON;        /* Added by Jim Roung               */
         fepcb->imode.ind5 = ERROR1; /* For Error Message display        */
         break;
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

/******************************************************************************/
/* FUNCTION    : StjGetCandidate                                              */
/* DESCRIPTION : Send Selected Candidate To Output Buffer.                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StjGetCandidate(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   StjCand *stj;
   int no,i;
   int len;
   char *ptr;
/* char radical[]={0xc7,0xf3,0xd5,0xfc,0xa1,0xa8};                      @big5 */
   unsigned char euccode[3],code[3];                               /*   @big5 */
   size_t        in_count,out_count;                               /*   @big5 */


   no = key - 0x30;
   if ((fepcb->learning) && (!flag) && (fepcb->starpos !=0))            /* V410 */
     stj = fepcb->stjstruct.stjcand+ fepcb->learnstruct.list_index[no]; /* V410 */
   else                                                                 /* V410 */
     stj = fepcb->stjstruct.stjcand + fepcb->stjstruct.headcandno + no;
   if ((stj->euc[0] == EUC_BYTE1) &&
          (fepcb->Lang_Type == codesetinfo[0].code_type))            /* @big5*/
      len = 4;
   else
      len = 2;                           /*   Fill Edend Buffer for Output  */


   memset(fepcb->edendbuf, NULL, strlen(fepcb->edendbuf));
   strncpy(fepcb->edendbuf, stj->euc, len);
   fepcb->edendacsz = len;

   if (fepcb-> starpos !=0 )                    /* Simplify Tsange Jye @big5 */
   {                                                                /* @big5  */
   memset(fepcb->radeucbuf, NULL, 20);   /*   Fill Radical Buffer for Ind.  */

   for (i=0; i<strlen(stj_radical);i++)                             /* @big5 */
     *(fepcb->radeucbuf+i)=stj_radical[i];                          /* @big5 */

   for (i=0 ; i<5 ;i++)
   {
/*   if (stj->key[i] == ' ')                                           @big5 */
     if ( (stj->key[i] == ' ') || (stj->key[i] == '\0') )           /* @big5 */
        i=5;
     else
     {
        ptr=TjKeyToRadical(stj->key[i]);

        if (fepcb->Lang_Type != codesetinfo[0].code_type)            /* @big5*/
        {                                                            /* @big5*/
           euccode[0]=*ptr;                                          /* @big5*/
           euccode[1]=*(ptr+1);                                      /* @big5*/
           in_count =2;                                              /* @big5*/
           out_count =2;                                             /* @big5*/
           StrCodeConvert(fepcb->iconv_flag,euccode,code,
                                              &in_count,&out_count); /* @big5*/
           ptr = code ;                                              /* @big5*/
        }                                                            /* @big5*/
        strncpy(fepcb->radeucbuf+strlen(stj_radical)+2*i, ptr, 2);  /* @big5 */
     }
   }
   fepcb->indchfg = ON;                                              /* @big5 */
   fepcb->imode.ind5 = RADICAL;                                      /* @big5*/

   }                                                                 /* @big5  */
   else                                                              /* @big5  */
   {                                                                 /* @big5 */
     fepcb->imode.ind5 = BLANK;                                      /* @big5 */
     fepcb->indchfg = OFF;                                           /* @big5 */
   }                                                                 /* @big5 */
/* fepcb->indchfg = ON;                                                 @big5 */
/* fepcb->imode.ind5 = RADICAL;                                         @big5 */
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->ret = FOUND_WORD;
}

/******************************************************************************/
/* FUNCTION    : StjEraseAux                                                  */
/* DESCRIPTION : Disappear candidate list box and return to radical input mode*/
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StjEraseAux(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
   unsigned char   **toauxs;           /* Pointer To Aux. String             */
   unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
   int i;


   toauxs = fepcb->auxbufs;
   toauxa = fepcb->auxbufa;
   for( i = 0 ; i< AUXROWMAX ; i++ )
   {
      memset( *(toauxs++),NULL,AUXCOLMAX);
   }
   fepcb->auxacsz.itemnum = 0;
   fepcb->auxacsz.itemlen = 0;                                 /* @big5   */
   fepcb->auxacsz.itemsize = 0;
   fepcb->auxuse = NOTUSE;
   fepcb->imode.ind2 = SELECT_OFF;
   fepcb->auxchfg = ON;
   flag = 0; /* V410 */
}

/******************************************************************************/
/* FUNCTION    : StrokeCandidates                                             */
/* DESCRIPTION : Fill 10 stroke candidates to auxbuf if there are candidates  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StrokeCandidates(fepcb)
FEPCB *fepcb;
{

   fepcb->imode.ind2 = STROKE_SELECT_ON;
   flag = 1 ;                                                         /* V410 */
   if (fepcb->strokestruct.headcandno == 0)
      StrokeListBox(fepcb, BEGINNING);
   else
      StrokeListBox(fepcb, RESTORE);
}

/******************************************************************************/
/* FUNCTION    : StrokeListBox                                                */
/* DESCRIPTION : Process PageUp and PageDown On Stoke Candidate List Box.     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StrokeListBox(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code (PGUP_KEY/PGDN_KEY)     */
{
/*   static char title[]={0xe1,0xad,0xe8,0xa2,0xf2,0xd9,0xf0,0xe5,0x20,0x20,
                        0x20,0x20,0x20,0x20,0xde,0xb7,0xef,0xf5};     @big5 */

/*   static char line[]={0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,
                       0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1,0xa2,0xb1  }; @big5 */

/*   static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
                 0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,
                 0x20,0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,
                 0x20,0x20,0x39,0x20,0x20};
                                                                           @big5 */
/*   static char bottom1[]={0xcc,0xbd,0xd6,0xbc,0x28,0x45,0x73,0x63,0x29}; @big5 */

/*   static char bottom2[]={0xc4,0xb8,0xd3,0xf7,0x28,0x50,0x67,0x55,0x70,0x29,
                          0x20,0x20,0xc4,0xb6,0xd3,0xf7,0x28,0x50,0x67,0x44,
                          0x6e,0x29};                                    @big5 */

/*   static char strokechar[]={0xe8,0xa2};                               @big5 */

   unsigned char   **toauxs;           /* Pointer To Aux. String             */
   unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
   unsigned char   *tostring;          /* Pointer To Aux. String             */
   unsigned char   *toattribute;       /* Pointer To Aux. Attribute          */
   unsigned char   buffer[4];
   StrokeCand *stroke;
   int i;
   int j;
   int len;
   unsigned int k,m;
   unsigned char *temp;

   len = stroke_title_LEN+sizeof(buffer);                    /* @big5 */
   switch ( key )
   {
      case BEGINNING:
         fepcb->strokestruct.headcandno = 0;
         if (fepcb->strokestruct.allcandno < 10)
             fepcb->strokestruct.tailcandno = fepcb->strokestruct.allcandno - 1;
         else
             fepcb->strokestruct.tailcandno = 9;
         break;

      case PGUP_KEY:
         if (fepcb->strokestruct.headcandno == 0)
         {
             fepcb->imode.ind5=ERROR1;  /* error occur             */
             fepcb->indchfg=ON;         /* set indictor flag on    */
             fepcb->isbeep = ON;
             return;
         }
         if (fepcb->strokestruct.headcandno > 9)
         {
            fepcb->strokestruct.headcandno = fepcb->strokestruct.headcandno-10;
            fepcb->strokestruct.tailcandno = fepcb->strokestruct.headcandno+9;
         }
         else
         {
            fepcb->strokestruct.headcandno = 0;
            if (fepcb->strokestruct.allcandno < 10)
               fepcb->strokestruct.tailcandno = fepcb->strokestruct.allcandno-1;
            else
               fepcb->strokestruct.tailcandno = 9;
         }
         break;

      case PGDN_KEY:
         if (fepcb->strokestruct.tailcandno==(fepcb->strokestruct.allcandno-1))
         {
             fepcb->imode.ind5=ERROR1;  /* error occur             */
             fepcb->indchfg=ON;         /* set indictor flag on    */
             fepcb->isbeep = ON;
             return;
         }
         fepcb->strokestruct.headcandno = fepcb->strokestruct.tailcandno + 1;
         if ((fepcb->strokestruct.headcandno+9) >=
                                   (fepcb->strokestruct.allcandno-1))
            fepcb->strokestruct.tailcandno = fepcb->strokestruct.allcandno - 1;
         else
            fepcb->strokestruct.tailcandno = fepcb->strokestruct.headcandno + 9;
         break;

      case RESTORE:
         break;
   }

   toauxs = fepcb->auxbufs;       /*   Clear Aux Buffer                      */
   toauxa = fepcb->auxbufa;

   for(k = 0; k< 14; k++)
   {
       temp = fepcb->auxbufs[k];
       for(m = 0; m < fepcb->auxacsz.itemsize; memcpy(temp," ",1),
           temp++,m++);
   }/* for */
   if (fepcb->auxacsz.itemnum != 0)
      for( i=0 ; i< fepcb->auxacsz.itemnum -2; i++ )
          memset(*toauxs++, ' ', fepcb->auxacsz.itemsize);

   fepcb->auxacsz.itemsize = len;
   fepcb->auxacsz.itemlen = len;                               /* @big5   */
   fepcb->auxacsz.itemnum = AUXROWMAX;

                                       /* Fill Title Message                 */
   memcpy(fepcb->auxbufs[0], stroke_title, stroke_title_LEN);    /* @big5 */
   tostring = fepcb->auxbufs[0]+stroke_title_LEN;                /* @big5 */

   itoa(fepcb->strokestruct.allcandno-fepcb->strokestruct.tailcandno-1, buffer
        ,sizeof(buffer));
   memcpy(tostring, buffer, sizeof(buffer));

                                       /* Fill Line Message                 */
   memccpy(fepcb->auxbufs[1], stroke_line, "", len);                /* @big5 */


   stroke = fepcb->strokestruct.strokecand + fepcb->strokestruct.headcandno;
   for (i=0; i <= fepcb->strokestruct.tailcandno-fepcb->strokestruct.headcandno;
         i++)
   {
      tostring = fepcb->auxbufs[i+2];
                                    /* Fill Number                        */
      for(j=i*4; j<i*4+4; j++)
         *tostring++ = *(stroke_digit+j);                          /* @big5 */
      itoa(stroke->stroke, buffer, sizeof(buffer));  /* Fill Data           */
      memcpy(tostring-2, buffer, sizeof(buffer));
      memcpy(tostring+sizeof(buffer)-1, strokechar, strokechar_LEN);
      stroke++;
   }

   if (key == BEGINNING || key == RESTORE || key == PGDN_KEY || key == PGUP_KEY )
   {
      for (i=fepcb->auxacsz.itemnum-2; i<fepcb->auxacsz.itemnum; i++)
        memset(fepcb->auxbufs[i], ' ', fepcb->auxacsz.itemsize);
                                           /* Fill Bottom Message         */
      memcpy(fepcb->auxbufs[AUXROWMAX-2], stroke_bottom1, stroke_bottom1_LEN);
                                                                 /* @big5 */
                                           /* Fill Bottom Message          */
      memcpy(fepcb->auxbufs[AUXROWMAX-1], stroke_bottom2, stroke_bottom2_LEN);
                                                                 /* @big5 */

   }

   fepcb->auxchfg=ON;
   fepcb->auxuse=USE;

}
/******************************************************************************/
/* FUNCTION    : StrokeSelect                                                 */
/* DESCRIPTION : Process input key on stroke candidate list box.              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StrokeSelect(fepcb,key)
FEPCB *fepcb;
{
   switch( key )
   {
      case PGUP_KEY:
         StrokeListBox(fepcb, key);
         break;

      case PGDN_KEY:
         StrokeListBox(fepcb, key);
         break;

      case ESC_KEY:
         fepcb->imode.ind2 = STJ_SELECT_ON;
         StjListBox(fepcb,RESTORE);
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
         if (fepcb->strokestruct.headcandno+key-0x30 >
                     fepcb->strokestruct.allcandno-1)
            fepcb->isbeep = BEEP_ON;
         else
         {
            StrokeGetCandidate(fepcb, key);
            if(fepcb->stjstruct.tailcandno == 1)/* User press "+" for stroke    */
            {                                   /* selection, if only one cand  */
               StjGetCandidate(fepcb,0x30);     /* match this stroke, then that */
               TjEraseAllRadical(fepcb);        /* can directly be displayed on */
               StjEraseAux(fepcb);              /* screen. (the Aux area needn't*/
            }                                   /* be displayed again)          */
            else                                /* Added by Jim Roung July/29/91*/
                StjListBox(fepcb, RESTORE);
         }
         break;

      default :
         fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm        */
         fepcb->indchfg = ON;        /* Added By Jim Roung    */
         fepcb->imode.ind5 = ERROR1; /* For Error Msg display */
         break;
   }
}

/******************************************************************************/
/* FUNCTION    : StrokeGetCandidate                                           */
/* DESCRIPTION : Selected Candidate                                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

StrokeGetCandidate(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
   StrokeCand *stroke;
   int no;
   int i,j;                                                        /* V410  */

   no = key - 0x30;
   stroke = fepcb->strokestruct.strokecand + fepcb->strokestruct.headcandno+no;
   fepcb->stjstruct.headcandno = stroke->rrn;
   if (fepcb->stjstruct.headcandno+9 >= fepcb->stjstruct.allcandno-1)
   {                                                               /* V410  */
      if (((fepcb->learning)                                       /* V410  */
         && (fepcb->starpos !=0))                                  /* V410  */
         && !flag)                                                 /* V410  */
      {                                                            /* V410  */
        j=0;                     /*                                /* V410  */
        for (i=fepcb->stjstruct.headcandno;i<fepcb->stjstruct.allcandno ;i++ )
                                                                   /* V410  */
          fepcb->learnstruct.list_index[j++] = i;                  /* V410  */
       }                                                           /* V410  */
       fepcb->stjstruct.tailcandno = fepcb->stjstruct.allcandno-1;
   }                                                               /* V410  */
   else
   {                                                               /* V410  */
      if (((fepcb->learning)                                       /* V410  */
         && (fepcb->starpos !=0))                                  /* V410  */
         && !flag)                                                 /* V410  */
      {                                                            /* V410  */
        j=0;                                                       /* V410  */
        for (i=fepcb->stjstruct.headcandno;i<=fepcb->stjstruct.headcandno+9 ;i++ )
                                                                   /* V410  */
          fepcb->learnstruct.list_index[j++] = i;                  /* V410  */
       }                                                           /* V410  */
       fepcb->stjstruct.tailcandno = fepcb->stjstruct.headcandno+9;
   }                                                               /* V410  */

   fepcb->imode.ind2 = STJ_SELECT_ON;
}




/********************************************************************V410******/
/* FUNCTION    : StjUpdateLearningData                               V410     */
/* DESCRIPTION : Send Selected Candidate index to Learning index     V410     */
/* EXTERNAL REFERENCES:                                              V410     */
/* INPUT       : fepcb, key                                          V410     */
/* OUTPUT      : fepcb                                               V410     */
/* CALLED      :                                                     V410     */
/* CALL        :                                                     V410     */
/********************************************************************V410******/

StjUpdateLearningData(fepcb,key)                                      /* V410 */
FEPCB           *fepcb;                /* FEP Control Block          V410    */
unsigned short  key;                   /* Ascii Code                 V410    */
{                                                                 /* V410 */
   int no,i,j;                                                    /* V410 */
   unsigned short select_dictionary_index;                        /* V410 */
   unsigned short candidate_no,temp;                              /* V410 */

  no = key - 0x30;                                                 /* V410 */
  if (flag)                                                        /* V410 */
  {                                                                /* V410 */
     if (select_dictionary_index != fepcb->learnstruct.index[0])    /* V410 */
     {                                                              /* V410 */
        select_dictionary_index = fepcb->stjstruct.headcandno+no;   /* V410 */
        for (i=0;i<LEARN_NO ;i++ )                                  /* V410 */
          if (select_dictionary_index == fepcb->learnstruct.index[i]) /* V410 */
            break;                                                  /* V410 */
        if (i == LEARN_NO)                                          /* V410 */
           for (j=LEARN_NO -1 ;j> 0 ;j-- )                          /* V410 */
             fepcb->learnstruct.index[j] = fepcb->learnstruct.index[j-1]; /* V410 */
        else                                                        /* V410 */
           for (j=i;j>0 ;j-- )                                      /* V410 */
             fepcb->learnstruct.index[j] = fepcb->learnstruct.index[j-1];
                                                                    /* V410 */
        fepcb->learnstruct.index[0] = select_dictionary_index;      /* V410 */
        update_learn_file(fepcb);                                   /* V410 */
     }                                                              /* V410 */
  }                                                                 /* V410 */
  else                                                              /* V410 */
  {                                                                 /* V410 */
     select_dictionary_index = fepcb->learnstruct.list_index[no];   /* V410 */
     candidate_no = fepcb->stjstruct.headcandno + no;               /* V410 */
     if (select_dictionary_index != fepcb->learnstruct.index[0])    /* V410 */
     {                                                              /* V410 */
        candidate_no = fepcb->stjstruct.headcandno + no;            /* V410 */
        if (candidate_no > LEARN_NO)                                /* V410 */
           temp = LEARN_NO -1;                                      /* V410 */
        else                                                        /* V410 */
           temp = candidate_no;                                     /* V410 */
        for (i=temp;i>0 ;i-- )                                      /* V410 */
            fepcb->learnstruct.index[i] = fepcb->learnstruct.index[i-1];
                                                                    /* V410 */
        fepcb->learnstruct.index[0] = select_dictionary_index;      /* V410 */
        update_learn_file(fepcb);                                   /* V410 */
     }                                                              /* V410 */
   }                                                                /* V410 */
}                                                                   /* V410 */
