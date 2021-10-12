static char sccsid[] = "@(#)04  1.6  src/bos/usr/lib/nls/loc/imt/tfep/tedacc.c, libtw, bos411, 9428A410j 6/14/94 20:35:44";
/*
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: Tedacc.c
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

/********************* START OF MODULE SPECIFICATION ***********************/
/*                                                                         */
/* MODULE NAME:        TedAcc                                              */
/*                                                                         */
/* DESCRIPTIVE NAME:   Chinese Input Method Access Dictionary              */
/*                                                                         */
/* FUNCTION:           AccessDictionary    : Access File                   */
/*                                                                         */
/*                     TjUsrDictFileChange : Check TJ User File Change     */
/*                                                                         */
/*                     PhUsrDictFileChange : Check PH User File Change     */
/*                                                                         */
/*                     TjCompareRadicals   : Compare TJ Radicals           */
/*                                                                         */
/*                     PhCompareRadicals   : Compare PH Radicals           */
/*                                                                         */
/*                     TjLoadSysFileMI     : Load MI of TJ System File     */
/*                                                                         */
/*                     PhLoadSysFileMI     : Load MI of PH System File     */
/*                                                                         */
/*                     TjLoadUsrFileMI     : Load MI of TJ User File       */
/*                                                                         */
/*                     PhLoadUsrFileMI     : Load MI of Ph User File       */
/*                                                                         */
/*                     TjFillSysCandidates : Find TJ System File Cand.     */
/*                                                                         */
/*                     TjFillUsrCandidates : Find TJ User File Cand.       */
/*                                                                         */
/*                     PhFillCandidates    : Find PH System File Cand.     */
/*                                           Find PH User File Cand.       */
/*                                            (  by Mei Lin  )             */
/*                     StrokeCmp           : Compare Stroke                */
/*                                                                         */
/*                     StjFreeCandidates   : Free STJ Candidates           */
/*                                                                         */
/*                     PhFreeCandidates    : Free PH Candidates            */
/*                                                                         */
/* MODULE TYPE:        C                                                   */
/*                                                                         */
/* COMPILER:           AIX C                                               */
/*                                                                         */
/* AUTHOR:             Terry Chou                                          */
/*                                                                         */
/* STATUS:             Chinese Input Method Version 1.0                    */
/*                                                                         */
/* CHANGE ACTIVITY:                                                        */
/*                                                                         */
/*  V410  06/18/93'    Modified by Debby Tseng (Mirrors) for TSANGE JYE    */
/*                     CODE duplicate                                      */
/*                                                                         */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tedinit.h"
#include "ted.h"
#include "tedacc.h"

int AccessDictionary();
int TjUsrDictFileChange();
int PhUsrDictFileChange();
int TjCompareRadicals();
int PhCompareRadicals();
int TjLoadSysFileMI();
int PhLoadSysFileMI();
int TjLoadUsrFileMI();
int PhLoadUsrFileMI();
int TjFillSysCandidates();
int TjFillUsrCandidates();
int PhFillCandidates();
int StrokeCmp();
int StjFreeCandidates();
int PhFreeCandidates();
int PhGetNextRRN();               /* Benson Lu */
char *PhGetNextIndex();           /* Benson Lu */
int  PhValidDict();               /* Benson Lu */
extern StrCodeConvert(iconv_t ,
                      unsigned char *,
                      unsigned char *,
                      size_t,
                      size_t);                                        /* @big5*/
   size_t        in_count;                                            /* @big5*/
   size_t        out_count;                                           /* @big5*/

/**************************************************************************/
/* FUNCTION    : AccessDictionary                                         */
/* DESCRIPTION : Fill word for Tsang-Jye Input Method, fill candidates    */
/*               for Phonetic and Simplify-Tsang-Jye Input Method.        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

AccessDictionary(fepcb)
FEPCB *fepcb;
{
   int temp;
   int ret1;
   int ret2;
   int TjUsrDictFileChange();
   int TjCompareRadicals();
   int TjFillUsrCandidates();
   int PhUsrDictFileChange();
   int PhCompareRadicals();
   int PhUsrDictFileChange();

   if (fepcb->learning)                /* Clear learning Structure Debby     */
   {                                                      /*  Debby          */
      reset_index(fepcb);                                 /*  Debby          */
      reset_list_index(fepcb);                            /*  Debby          */
   }                                                      /*  Debby          */

   if ( fepcb->imode.ind0 == TSANG_JYE_MODE )
   {
      if ( *(fepcb->curinpbuf) == 'x' ) /* relate with TJ user file */
      {
         if ( fepcb->fd.tjusrfd != NULL)  /*  TJ user file exist  */
         {
            if ( (temp=TjUsrDictFileChange(fepcb)) == FALSE )
            {
                        /* TJ user file does not change */
               if ( TjCompareRadicals(fepcb) )
               {
                  if ( fepcb->starpos == 0 )      /*  TJ   */
                     fepcb->ret=FOUND_WORD;
                  else
                  {                              /*  STJ   */
                     if ( fepcb->stjstruct.stjcand != NULL )
                        fepcb->ret=FOUND_CAND;
                     else
                        TjFillUsrCandidates(fepcb);
                  }
               }
               else
                  TjFillUsrCandidates(fepcb);
            }
            else
            {
               if ( temp == ERROR )
                  return;
               else          /*  temp=TRUE  */
                  TjFillUsrCandidates(fepcb);
            }
         }
         else
            fepcb->ret=NOT_FOUND;
      }
      else               /*  relate with  TJ system file  */
      {
         if ( TjCompareRadicals(fepcb) )
         {              /*  TJ system file does not change  */
            if ( fepcb->starpos == 0 )
            {
               if (fepcb->stjstruct.stjcand != NULL)                  /* V410 */
                 fepcb->ret = FOUND_CAND;                             /* V410 */
               else                                                   /* V410 */
               {                                                      /* V410 */
                  TjFillSysCandidates(fepcb);                         /* V410 */
      /*          fepcb->ret=FOUND_WORD;                                 V410 */
               }                                                      /* V410 */
            }
            else
            {
               if ( fepcb->stjstruct.stjcand != NULL )
                  fepcb->ret=FOUND_CAND;
               else
                  TjFillSysCandidates(fepcb);
            }
         }
         else
            TjFillSysCandidates(fepcb);
      }
   }
   else     /*  PHONETIC_MODE   */
   {
      if ( fepcb->fd.phusrfd != NULL )
      {                           /*  have ph user file  */
         if ( (temp=PhUsrDictFileChange(fepcb)) == FALSE )
         {
            if ((PhCompareRadicals(fepcb)) &&
               (fepcb->phstruct.phcand != NULL))
            {                         /* input buffer the same as  */
               fepcb->ret=FOUND_CAND; /* last input buffer         */
               return;
            }
            else
            {
               PhFreeCandidates(fepcb);
               PhFillCandidates(fepcb, USR); /* find ph usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  PhFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                        PhFreeCandidates(fepcb);
               PhFillCandidates(fepcb, SYS); /* find ph sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  PhFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                  fepcb->ret=FOUND_CAND;
               else
                  fepcb->ret=NOT_FOUND;
            }
         }
         else
         {
            if ( temp == ERROR )
               return;
            else
            {
               PhFreeCandidates(fepcb);
               PhFillCandidates(fepcb, USR);
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  PhFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                       PhFreeCandidates(fepcb);
               PhFillCandidates(fepcb, SYS);
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  PhFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                  fepcb->ret=FOUND_CAND;
               else
                  fepcb->ret=NOT_FOUND;
            }
         }
      }
      else
      {                   /*  no ph usr file  */
         if ((PhCompareRadicals(fepcb)) &&
              (fepcb->phstruct.phcand != NULL))
         {                            /* input buffer the same as  */
            fepcb->ret=FOUND_CAND;   /* last input buffer         */
            return;
         }
         else
         {
            PhFreeCandidates(fepcb);
            PhFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               PhFreeCandidates(fepcb);
         }
      }
   }
}

/**************************************************************************/
/* FUNCTION    : TjUsrDictFileChange                                      */
/* DESCRIPTION : Check Tsang-Jye user dictionary file whether it changes  */
/*               or not if it change, reload master index of file         */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int TjLoadUsrFileMI();
   extern char *ctime();

   stat(fepcb->fname.tjusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.tjtime,p) )
      return(FALSE);
   else
   {              /*  file had been changed  */
      if ( TjLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.tjtime,p);
         return(TRUE);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : PhUsrDictFileChange                                      */
/* DESCRIPTION : Change Phonetic user dictionary file whether it change   */
/*               or not, if it change, reload master index of file        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PhUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int PhLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.phusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.phtime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( PhLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.phtime, p);
         return(TRUE);
      }
   }
}

/***************************************************************************/
/* FUNCTION    : TjCompareRadicals                                         */
/* DESCRIPTION : Compare current input buffer with previous input buffer   */
/*               ,if it the same as previous buffer, return TRUE, else     */
/*               return FALSE                                              */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : TRUE, FALSE                                               */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

TjCompareRadicals(fepcb)
FEPCB *fepcb;
{
   if ( !strcmp(fepcb->curinpbuf,fepcb->preinpbuf) )
      return(TRUE);
   else
      return(FALSE);
}

/***************************************************************************/
/* FUNCTION    : PhCompareRadicals                                         */
/* DESCRIPTION : Compare echo buffer with ctrl_right buffer, if it the     */
/*               same as ctrl_righ buffer, return TRUE, else return FALSE  */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : TRUE, FALSE                                               */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

PhCompareRadicals(fepcb)
FEPCB *fepcb;
{
   if ( !strcmp(fepcb->ctrl_r_buf,fepcb->echobufs) )
      return(TRUE);
   else
      return(FALSE);
}


/**************************************************************************/
/* FUNCTION    : PhLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Phonetic system file                */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PhLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.phsysfd=fopen(fepcb->fname.phsysfname, "r")) == NULL )
      return(ERROR);
   fepcb->mi.phsysmi=(unsigned char *)calloc(PH_SYS_MI_LEN, sizeof(char));
   memset(fepcb->mi.phsysmi, NULL, PH_SYS_MI_LEN);

   if ( fread((char *)fepcb->mi.phsysmi, PH_SYS_MI_LEN, 1,
      fepcb->fd.phsysfd) == 0 && ferror(fepcb->fd.phsysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}

/******************************************************************************/
/* FUNCTION    : PhGetNextRRN                                                 */
/* DESCRIPTION : get rrn from Master Index                                    */
/* INPUT       : mi                                                           */
/*               ptr                                                          */
/* OUTPUT      : rrn                                                          */
/******************************************************************************/

int PhGetNextRRN(mi, ptr)
char *mi;                              /* pointer to the Master Index         */
int *ptr;                              /* point to the position of MI         */
{
   int rrn;                            /* the value of rrn                    */
   int count;                          /* loop counter                        */
   unsigned short miend;

   miend = PH_MI_END_MARK;
   /* skip index key */
   while (LOOP)
   {
       if (strncmp(mi+(*ptr), (char *)&miend, 2) == 0)
          return(-1);                  /* End mark of MI                      */
       else if (*(mi+(*ptr)) & 0x80)
          (*ptr)++;                    /* skip to next byte                   */
       else
          break;                       /* End of index key                    */
   }
   /* get the rrn value , the rrn occupy RRN_LENGTH bytes */
   for (count = 0; count < RRN_LENGTH; count++)
   {
       rrn <<= 8;                      /* shift left 8 bits                   */
       rrn |= *(mi+(++(*ptr)));
   }
   (*ptr)++;                           /* skip to next byte                   */
   return(rrn);                        /* value of rrn                        */
}

/******************************************************************************/
/*FUNCTION    : PhGetNextIndex                                                */
/*DESCRIPTION : get next index key of MI                                      */
/*INPUT       : dict = the dictionary                                         */
/*              ptr = the start point of MI                                   */
/*OUTPUT      : index_key = the index key of MI                               */
/*NOTE        : the value of ptr must return                                  */
/******************************************************************************/

char * PhGetNextIndex(mi, ptr)
char *mi;                              /* pointer to Master Index             */
int *ptr;                              /* pointer to the position of MI       */
{
   static char index_key[PHONETIC_LEN];/* index key buffer                    */
   int count = 0;                      /* counter                             */
   char *miptr;                        /* pointer to MI                       */
   unsigned short miend;

   miend = PH_MI_END_MARK;
   (void) memset(index_key, NULL, PHONETIC_LEN); /* clear index_key           */
   miptr = mi;                         /* point to the beginning of MI        */
   if ((*(miptr+*ptr)!= 0)&&(strncmp((miptr+(*ptr)),&miend, 2) != 0))
   {
      while ((*ptr < PH_USR_MI_LEN) && (*(miptr+(*ptr)) & 0x80))
         index_key[count++] = *(miptr+(*ptr)++); /* get next byte             */
      /* set the 7th bit of last byte to on */
      index_key[count++] = *(miptr+(*ptr)++) | 0x80;
      *ptr += RRN_LENGTH;              /* skip rrn                            */
   }
   else
   {
      /* set the index_key to be end mark of MI */
      (void) memcpy(index_key,(char *)&miend, 2);
   }
   if(miptr != NULL) miptr = NULL;      /* for xaltu tools      */
   return ((char *) index_key);        /* return index key                    */
}

/******************************************************************************/
/* FUNCTION    : PhValidDict                                                  */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid phonetic user dictionary                        */
/*               FALSE = invalid phonetic user dictionary                     */
/******************************************************************************/

PhValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   char *PhGetNextIndex();
   int ptr;
   int num;
   long int max_rrn;
   long int rrn;
   char *indexkey_ptr;
   unsigned short miend;

   miend = PH_MI_END_MARK;
   ptr =0;
   indexkey_ptr = PhGetNextIndex(fepcb->mi.phusrmi, &ptr);
   if (strncmp(indexkey_ptr,(char *)&miend, 2) == 0)
   {
      num=2;
      while (num < PH_USR_MI_LEN)
           if (fepcb->mi.phusrmi[num++] != NULL)
            return(FALSE); /* Invalid dictionary format */
      return(TRUE);
   }
   while (strncmp(indexkey_ptr, &miend,2) != 0)
   {
       if (strlen(indexkey_ptr) > MAX_PHONETIC_NUM)
          return(FALSE);  /* Invalid dictionary format */
       num=0;
       while (*(indexkey_ptr+num))
       {
         if ((*(indexkey_ptr+num) >= 0xc7) && (*(indexkey_ptr+num) <= 0xf0))
            num++;
         else
            return(FALSE);  /* Invalid dictionary format */
       }
       indexkey_ptr = PhGetNextIndex(fepcb->mi.phusrmi, &ptr);
   }
   ptr = 0;
   max_rrn=0;

   while ((rrn=PhGetNextRRN(fepcb->mi.phusrmi,&ptr)) > 0)
   {
       if ((rrn - PH_USR_MI_LEN) % PH_DR_MAX_LEN)
          return(FALSE);
       if ( max_rrn < rrn) max_rrn = rrn;
   }
   if (max_rrn < PH_USR_MI_LEN) return(FALSE);
   return(TRUE);
}

/**************************************************************************/
/* FUNCTION    : PhLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Phonetic user file                  */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PhLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.phusrfd == NULL )
   {
      if ( (fepcb->fd.phusrfd=fopen(fepcb->fname.phusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial ted  */
         stat(fepcb->fname.phusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.phtime, p);
      }
   }
   else
   {
      free(fepcb->mi.phusrmi);
      fseek(fepcb->fd.phusrfd, 0, 0);   /* fd point head of file */
   }

   fepcb->mi.phusrmi=(char *)calloc(PH_USR_MI_LEN, sizeof(char));
   memset(fepcb->mi.phusrmi, NULL, PH_USR_MI_LEN);
   if ( fread((char *)fepcb->mi.phusrmi, PH_USR_MI_LEN, 1,
      fepcb->fd.phusrfd) == 0 && ferror(fepcb->fd.phusrfd) != 0 )
      return(ERROR);
   else
   {
      if (PhValidDict(fepcb) == TRUE)              /* Benson Lu  */
          return(OK);
      else
      {
          free(fepcb->mi.phusrmi);
          fepcb->mi.phusrmi = NULL;
          fepcb->fd.phusrfd = NULL;
          memset(fepcb->ctime.phtime, NULL, strlen(fepcb->ctime.phtime));
          return(ERROR);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : TjLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Tsang-Jye system file               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjLoadSysFileMI(fepcb)
FEPCB *fepcb;
{
   unsigned char *string;
   unsigned char *str;
   int i;

   if ( (fepcb->fd.tjsysfd=fopen(fepcb->fname.tjsysfname, "r")) == NULL )
      return(ERROR);
   string=(unsigned char *)calloc(TJ_SYS_MI_LEN, sizeof(char));
        /*  load master index  */
   if ( fread((char *)string, TJ_SYS_MI_LEN, 1, fepcb->fd.tjsysfd) == 0
        && ferror(fepcb->fd.tjsysfd) != 0 )
   {
      free(string);
      return(ERROR);
   }
   else
   {
      str=string;       /*  convert and fill value to variable  */
      for (i=0; i<TJ_SYS_TOT_MI; i++)
      {
         fepcb->mi.tjsysmi[i].index=*(str)*256*256+*(str+1)*256+*(str+2);
         fepcb->mi.tjsysmi[i].count=*(str+3)*256+*(str+4);
         fepcb->mi.tjsysmi[i].stroke=*(str+5);
         str=str+6;
      }
      free(string);
      str = NULL;       /* for xaltu    */
      return(OK);
   }
}

/**************************************************************************/
/* FUNCTION    : TjValidDict                                              */
/* DESCRIPTION : Check dictionary has a valid dictionary format           */
/* INPUT       : dict                                                     */
/* OUTPUT      : TRUE, FALSE                                              */
/**************************************************************************/

TjValidDict(fepcb)
FEPCB *fepcb;
{
   int i;
   int total=0;

   for (i=1; i<TJ_USR_TOT_MI; i++)
       total += fepcb->mi.tjusrmi[i].count;
   if (total == fepcb->mi.tjusrmi[0].count*2)
       return(TRUE);
   else
       return(FALSE);
}

/**************************************************************************/
/* FUNCTION    : TjLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Tsang-Jye user file                 */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   unsigned char *string;
   unsigned char *str;
   char *p;
   int i;
   extern char *ctime();

   if ( fepcb->fd.tjusrfd == NULL )   /*  for fepcb initial  */
   {
      if ( (fepcb->fd.tjusrfd=fopen(fepcb->fname.tjusrfname, "r")) == NULL )
         return(ERROR);
      else
      {     /*  load time to fepcb when input method initial ted  */
         stat(fepcb->fname.tjusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.tjtime, p);
      }
   }
   else
      fseek(fepcb->fd.tjusrfd, 0, 0);
   string=(unsigned char *)calloc(TJ_USR_MI_LEN, sizeof(char));
         /*  load master index   */
   if ( fread((char *)string, TJ_USR_MI_LEN, 1, fepcb->fd.tjusrfd) == 0
        && ferror(fepcb->fd.tjusrfd) != 0 )
   {
      free(string);
      return(ERROR);
   }
   else
   {
      str=string;    /*  convert and fill value to variable  */
      for (i=0; i<TJ_USR_TOT_MI; i++)
      {
         fepcb->mi.tjusrmi[i].index=*(str)*256+*(str+1);
         fepcb->mi.tjusrmi[i].count=*(str+2)*256+*(str+3);
         str=str+4;
      }
      free(string);
      if (TjValidDict(fepcb) == TRUE)
          return(OK);
      else
      {
          fepcb->fd.tjusrfd = NULL;
          memset(fepcb->ctime.tjtime, NULL, strlen(fepcb->ctime.tjtime));
          for (i=0; i<TJ_USR_TOT_MI; i++)
              fepcb->mi.tjusrmi[i].index=fepcb->mi.tjusrmi[i].count=0;
          return(ERROR);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : PhFillCandidates                                         */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PhFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[15];        /* Echo Buffer                        */
   unsigned char   *miptr;             /* Pointer To Master Index            */
   long            *offsetptr;         /* Pointer To Offset Of DR            */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   gkeybuf[15];        /* Group Key Buffer                   */
   unsigned char   *drptr;             /* Pointer To DR Buffer               */
   unsigned char   *chptr;             /* Character Pointer                  */
   unsigned char   *schptr;            /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned short  miend;              /* End Mark Of MI                     */
   unsigned short  drend;              /* End Mark Of DR                     */
   unsigned char   gpend;              /* End Mark Of Group                  */
   unsigned char   code[69];           /* @big5                              */
   unsigned char   euccode[69];        /* @big5                              */
/* size_t          in_count;           /* @big5                              */
/* size_t          out_count=69;       /* @big5                              */
   unsigned int    pre_spcounter;
   unsigned char   *t;



   fepcb->learnstruct.mode = PHONETIC_L;                             /* V410 */
   miend = PH_MI_END_MARK;
   drend = PH_DR_END_MARK;
   gpend = PH_GP_END_MARK;
   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<15; cnt++ )  echobuf[cnt] = NULL;

                                       /* Compress The Content Of Echo Buffer*/
   for( cnt=0; cnt<fepcb->echoacsz/2; cnt++ )
   {
      echobuf[cnt] = *(++echobufptr);
      echobufptr ++;
   }

   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.phusrfd;
      fseek(fdptr, 0, SEEK_SET);
      miptr = (unsigned char *)fepcb->mi.phusrmi;
      fepcb->phstruct.phcand = (unsigned char *)malloc(spcounter=1);
      memset(fepcb->phstruct.phcand, NULL, 1);
      fepcb->ret = NULL;
   }
   else   /* flag = SYS */
   {
      fdptr = (FILE *)fepcb->fd.phsysfd;
      fseek(fdptr, 0, SEEK_SET);
      miptr = (unsigned char *)fepcb->mi.phsysmi;
      if( *fepcb->phstruct.phcand == NULL )
         spcounter = 1;
      else
         spcounter = strlen( fepcb->phstruct.phcand);
   }

   echobufptr = (unsigned char *)echobuf;

   if( miptr != NULL )
   {
                                    /* To Search The Range Of Key Until The*/
                                    /* Content Of Echo Buffer Is Less Than */
                                    /* Or Equal To Index Value             */
      while( *echobufptr >= (*miptr | 0x80) )
      {
         if( *echobufptr == (*miptr | 0x80) )
         {
            if( *(echobufptr+1) == NULL )
               break;

            if( (*miptr & 0x80) == 0x00 && *echobufptr == (*miptr | 0x80) )
            {
                miptr ++;
                miptr += sizeof(long);
                echobufptr = (unsigned char *)echobuf;
            }
            else
            {
               echobufptr ++;
               miptr ++;
            }
         }
         else
         {
            if( strncmp(miptr, &miend, 2) == OK )
            {
               fepcb->ret = NOT_FOUND;
               return;
            }
            else
            {                        /*  Get Next Index Of Master Index    */
               while( (*miptr & 0x80) == 0x80 ) miptr ++;
               miptr ++;
               miptr += sizeof(long);
               echobufptr = (unsigned char *)echobuf;
            }
         }
      }

      if( strncmp(miptr, &miend, 2) == OK )
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
                                    /* Get The Offset Of DR               */
      while( (*miptr & 0x80) == 0x80 ) miptr ++;
      offsetptr = (long *)++miptr;
      drptr = (unsigned char *)malloc(PH_DR_MAX_LEN);
      for( cnt=0; cnt<PH_DR_MAX_LEN; cnt++ )
        *(drptr+cnt) = NULL;

      if( fseek(fdptr, (long)*offsetptr, SEEK_SET) != NULL )
      {
         free(drptr);
         fepcb->ret = ERROR;
         return;
      }

      if( fread(drptr, PH_DR_MAX_LEN, 1, fdptr) != 1 && ferror(fdptr) != 0 )
      {
         free(drptr);
         fepcb->ret = ERROR;
         return;
      }

      chptr = drptr;
                                    /* Dictionary Record Is Found         */
      while( LOOP )                 /* Now, To Search Satisfied Group     */
      {
         for( cnt=0; cnt<15; cnt++ )  gkeybuf[cnt] = NULL;
         schptr = chptr;
         for( cnt=0; (*chptr & 0x80) != NULL; cnt++,chptr++ )
            gkeybuf[cnt] = *chptr;
         gkeybuf[cnt] = *chptr | 0x80;
                                    /* Compare Echo Buffer With Group Key */
         if( strlen(gkeybuf) == strlen(echobuf) &&
             strncmp(echobuf, gkeybuf, strlen(echobuf)) == OK )
         {
            if (fepcb->Lang_Type != codesetinfo[0].code_type)        /* @big5*/
            {                                                        /* @big5*/
               in_count = 0;                                         /* @big5*/
               chptr ++;                                             /* @big5*/
               while(LOOP)                                           /* @big5*/
              {                                                      /* @big5*/
                  if( *chptr == gpend ) break;  /* End of group */   /* @big5*/

                                            /* check plane 3,4 character */
                  while ( *chptr != gpend )                          /* @big5*/
                  {                                                  /* @big5*/

                    if ( *(chptr) == EUC_BYTE1)                      /* @big5*/
                    {                                                /* @big5*/
/*                    if ( (*(chptr+1) == PLANE3_EUC_BYTE2) ||          @V42 */
/*                         (*(chptr+1) == PLANE4_EUC_BYTE2) )           @V42 */
                      if ( (*(chptr+1) != PLANE2_EUC_BYTE2) &&       /* @V42 */
                           (*(chptr+1) != PLANEc_EUC_BYTE2) &&       /* @V42 */
                           (*(chptr+1) != PLANEd_EUC_BYTE2) )        /* @V42 */
                      {                                              /* @big5*/
                         while ( ((*chptr & 0x80) != NULL ) &&       /* @big5*/
                                 (*chptr != gpend) )                 /* @big5*/
                               chptr ++;                             /* @big5*/
                          if (*chptr != gpend)  chptr ++;            /* @big5*/
                          in_count = 0 ;                             /* @big5*/
                      }                                              /* @big5*/
                      else                                           /* @big5*/
                      {                                              /* @big5*/

                          memcpy((euccode+in_count),chptr,4);        /* @big5*/

                          if ( (euccode[3+in_count] & 0x80) == NULL) /* @big5*/
                          {                                          /* @big5*/
                             fepcb->phstruct.allcandno++;            /* @big5*/
                             euccode[3+in_count] = euccode[3+in_count] | 0x80 ;        /* @big5*/
                             in_count = in_count + 4;                /* @big5*/
                             out_count =69;                          /* @big5*/
                             StrCodeConvert(fepcb->iconv_flag,
                                         euccode,code,
                                         &in_count,&out_count);      /* @big5*/
                             out_count = 69 - out_count;             /* @big5*/
                             pre_spcounter = spcounter ;             /* @big5*/
                             spcounter = spcounter + out_count  ;    /* @big5*/
                             fepcb->phstruct.phcand =                /* @big5*/
                             (unsigned char *)realloc(fepcb->phstruct.phcand,spcounter);
                                                                     /* @big5*/
                             candptr = (unsigned char *)fepcb->phstruct.phcand;/*@big5 */
                             for( cnt=0; cnt<(pre_spcounter-1);cnt++,candptr++ );
                                                                     /*@big5 */
                             code[out_count-2] = code[out_count-2] & 0x7F;
                                                                     /*@big5*/
                             memcpy(candptr,code,out_count);         /*@big5 */
                             *(candptr+out_count) = NULL;            /*@big5 */
                             in_count = 0;                           /* @big5*/
                          }                                          /* @big5*/
                          else                                       /* @big5*/
                            in_count = in_count + 4;                 /* @big5*/
                          chptr+=4;                                  /* @big5*/
                      }                                              /* @big5*/
                   }                                                 /* @big5*/
                   else                                              /* @big5*/
                   {                                                 /* @big5*/
                      memcpy((euccode+in_count),chptr,2);            /* @big5*/

                      if ((euccode[1+in_count] & 0x80) == 0)         /* @big5*/            /* @big5*/
                      {                                              /* @big5*/
                        fepcb->phstruct.allcandno++;                 /* @big5*/
                        euccode[1+in_count] = euccode[1+in_count] | 0x80 ;             /* @big5*/
                        in_count = in_count +2;                      /* @big5*/
                        out_count = 69;                              /* @big5*/
                        StrCodeConvert(fepcb->iconv_flag,
                                     euccode,code,
                                     &in_count,&out_count);
                        out_count =69-out_count;                       /* @big5*/
                        pre_spcounter = spcounter ;                    /* @big5*/
                        spcounter = spcounter + out_count  ;           /* @big5*/
                        fepcb->phstruct.phcand =                       /* @big5*/
                        (unsigned char *)realloc(fepcb->phstruct.phcand,spcounter);
                                                                     /* @big5*/
                        candptr = (unsigned char *)fepcb->phstruct.phcand;/*@big5*/
                        for( cnt=0; cnt<(pre_spcounter-1); cnt++,candptr++ );
                                                                     /* @big5*/
                        code[out_count-2] = code[out_count-2] & 0x7F;/* @big5*/
                        memcpy(candptr,code,out_count);              /* @big5*/
                        *(candptr+out_count) = NULL;                 /* @big5*/
                        in_count = 0;                                /* @big5*/
                      }                                              /* @big5*/
                      else                                           /* @big5*/
                        in_count = in_count +2;                      /* @big5*/
                      chptr+=2;                                      /* @big5*/
                   }                                                 /* @big5*/
                   if (in_count > 69)                                /* @big5*/
                   {                                                 /* @big5*/
                      in_count = 0;                                  /* @big5*/
                      break; /* error */                             /* @big5*/
                   }                                                 /* @big5*/
                 }                                                   /* @big5*/
                                                                     /* @big5*/
               }                                                     /* @big5*/

              candptr = (unsigned char *)fepcb->phstruct.phcand;     /*@big5 */
              for( cnt=0; cnt<(spcounter-1); cnt++,candptr++ );      /*@big5 */
              *candptr = *chptr;                                     /*@big5 */
              fepcb->phstruct.phcand =                               /*  @big5 */
              (unsigned char *)realloc(fepcb->phstruct.phcand,++spcounter);/*  @big5 */
              candptr = (unsigned char *)fepcb->phstruct.phcand;     /*@big5 */
              for( cnt=0; cnt<(spcounter-1); cnt++,candptr++ );      /*@big5 */
              *candptr = NULL;                                       /*@big5 */
              free(drptr);                                           /*@big5 */
              if (fepcb->phstruct.allcandno == 0)                    /*@big5 */
                 fepcb->ret = NOT_FOUND;                             /*@big5 */
              else                                                   /*@big5 */
                 fepcb->ret = FOUND_CAND;                            /*@big5 */
              return;                                                /*@big5 */
            }                                                        /*@big5 */
            else                                                     /*@big5 */
            {                                                        /*@big5 */
              while( LOOP )          /* Fill Satisfied Cand. To Cand. Buffer*/
              {
                 chptr ++;
                                      /* End Of This Group Or Not           */
                 if( *chptr == gpend ) break;

                                      /* End Of One Phrase Or Not           */
                 if( (*chptr & 0x80) == NULL ) fepcb->phstruct.allcandno ++;

                 fepcb->phstruct.phcand =
                 (unsigned char *)realloc(fepcb->phstruct.phcand,++spcounter);
                 candptr = (unsigned char *)fepcb->phstruct.phcand;
                 for( cnt=1; cnt<(spcounter-1); cnt++,candptr++ );

                 *candptr++ = *chptr;
                 *candptr = NULL;
              }

              *candptr = *chptr;
              fepcb->phstruct.phcand =
              (unsigned char *)realloc(fepcb->phstruct.phcand,++spcounter);
              candptr = (unsigned char *)fepcb->phstruct.phcand;
              for( cnt=0; cnt<(spcounter-1); cnt++,candptr++ );
              *candptr = NULL;
              free(drptr);
              fepcb->ret = FOUND_CAND;
              return;
            }
         }
         else
         {
            echobufptr = (unsigned char *)echobuf;
            chptr = schptr;

            while( *echobufptr >= (*chptr | 0x80) )
            {
               if( *echobufptr > (*chptr | 0x80) ||
                   (*chptr & 0x80) == NULL || *(echobufptr+1) == NULL )
               {
                  while( LOOP )
                  {
                     if( strncmp(chptr, &drend, 2) == OK )
                     {
                        fepcb->ret = NOT_FOUND;
                        free(drptr);
                        return;
                     }

                     if( *chptr == gpend )
                     {
                        echobufptr = (unsigned char *)echobuf;
                        chptr ++;
                        break;
                     }
                     chptr ++;
                  }
                  break;
               }
               chptr ++;
               echobufptr ++;
            }

            if( *echobufptr < (*chptr | 0x80) )
            {
               free(drptr);
               fepcb->ret = NOT_FOUND;
               return;
            }
         }
      }
   }
}

/**************************************************************************/
/* FUNCTION    : TjFillUsrCandidates                                      */
/* DESCRIPTION : According user dictionary, fill word to fepcb for Tsang  */
/*               -Jye Input Method or fill candidates to fepcb for        */
/*               Simplify-Tsang-Jye Input Method                          */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjFillUsrCandidates(fepcb)
FEPCB *fepcb;
{
   int i;
   int j;
   int type;
   unsigned short index;
   unsigned short offset;
   unsigned short stroke;
   unsigned char *string;
   unsigned char *string1;
   unsigned char *end;
   unsigned char key[4];
   unsigned char inputkey[3];       /*  for Tsang-Jye input key  */
   short strokeno[STROKE_TOT_NO];
   StjCand *stjptr;
   StrokeCand *strokeptr;
   int StrokeCmp();
   int StjFreeCandidates();
   unsigned char code[4];          /* @big5 */
/*   size_t        out_count;        /* @big5 */
/*   size_t        in_count;         /* @big5 */

   if ( fepcb->starpos == 0 )      /*  Tsang-Jye input method  */
   {
      index=*(fepcb->curinpbuf+(fepcb->inputlen-1))-BASE;   /*  find index   */

      switch(fepcb->inputlen)      /*  find input key  */
      {
         case 5 :    /*  radical number  */
           memcpy(inputkey, fepcb->curinpbuf+1, 3);
           break;
         case 4 :
           memcpy(inputkey, fepcb->curinpbuf+1, 2);
           inputkey[2]=' ';
           break;
         case 3 :
           inputkey[0]=*(fepcb->curinpbuf+1);
           inputkey[1]=' ';
           inputkey[2]=' ';
           break;
         case 2 :
           memset(inputkey,' ',3);
           break;
         case 1 :
           fepcb->ret=NOT_FOUND;
           return;
           break;
      }
      type=TJ0;       /*  set type  */
   }
   else                  /*  Simplify-Tsang-Jye input method  */
   {
      for (i=0; i<STROKE_TOT_NO; i++)
          strokeno[i]=0;

            /*  set type and find index   */
      if ( fepcb->starpos == 2 )        /*   XB*   */
      {
         index=TJ_USR_TOT_MI-1;
         type=STJ1;
         fepcb->learnstruct.mode = ABSTART;                         /* V410 */
      }                          /*   X*B   */
      else if ( (fepcb->starpos==1) && (fepcb->inputlen == 3) )
           {
              index=*(fepcb->curinpbuf+fepcb->inputlen-1)-BASE;
              type=STJ2;
              fepcb->learnstruct.mode = ASTARTB;                    /* V410 */
           }
           else                  /*   X*    */
           {
              index=TJ_USR_TOT_MI-1;
              type=STJ3;
              fepcb->learnstruct.mode = ASTART;                     /* V410 */
           }
   }

   if ( fepcb->mi.tjusrmi[index].count == NO_WORD )
   {
      fepcb->ret=NOT_FOUND;
      return;
   }
   else
   {        /*  get offset in order to load which block  */
      offset=fepcb->mi.tjusrmi[index].index;
      string=(unsigned char *)calloc(TJ_USR_BLOCK_SIZE, sizeof(char));
      string1=string;
      fepcb->ret=NOT_FOUND;   /*  set NOT_FOUND   */

      if ( type != TJ0 )      /* for Simplify-Tsang-Jye */
      {
         StjFreeCandidates(fepcb);
         fepcb->stjstruct.stjcand=(StjCand *)calloc(
                    fepcb->mi.tjusrmi[index].count, sizeof(StjCand));
         stjptr=fepcb->stjstruct.stjcand;
      }

      while ( offset != 0 )   /*   have other block  */
      {
         string=string1;
         memset(string, NULL, TJ_USR_BLOCK_SIZE);
         fseek(fepcb->fd.tjusrfd, TJ_USR_BLOCK_SIZE*offset, 0);

         if ( fread((char *)string, TJ_USR_BLOCK_SIZE, 1,
                fepcb->fd.tjusrfd) == 0 && ferror(fepcb->fd.tjusrfd) != 0 )
         {
            fepcb->ret=ERROR;
            StjFreeCandidates(fepcb);
            free(string1);
            return;
         }

         offset=*(string+TJ_USR_BLOCK_SIZE-2)*256+  /* get offset */
                       *(string+TJ_USR_BLOCK_SIZE-1);
                                          /*  end point to tail of block  */
         end=string+(TJ_USR_BLOCK_SIZE-4-(*(string+
                TJ_USR_BLOCK_SIZE-4)*256+*(string+TJ_USR_BLOCK_SIZE-3)));

         while ( string != end )
         {
            stroke=*(string);     /*  get stroke from file */
            string++;
            memset(key, NULL, 4);
                                    /*  get key from file    */
            if ( (type == TJ0) || (type == STJ2) )
            {
               memcpy(key, string, 3);
               string=string+3;
            }
            else
            {
               memcpy(key, string, 4);
               string=string+4;
            }

            switch (type)
            {
               case TJ0 :

                 if ( !memcmp(key, inputkey, 3) )
                 {
                   /*  fill euc code to fepcb->preedbuf */
                    memset(fepcb->preedbuf, NULL , strlen(fepcb->preedbuf));
                    *(fepcb->preedbuf)=USR_FONT_EUC_BYTE1;
                    *(fepcb->preedbuf+1)=USR_FONT_EUC_BYTE2;
                    *(fepcb->preedbuf+2)=*(string);
                    *(fepcb->preedbuf+3)=*(string+1);
                    if (fepcb->Lang_Type != codesetinfo[0].code_type) /* @big5 */
                    {                                                 /* @big5 */
                       in_count = 4;                                  /* @big5 */
                       out_count = 2;                                 /* @big5 */
                       StrCodeConvert(fepcb->iconv_flag,
                                   fepcb->preedbuf,code, &in_count, &out_count);
                                                                      /* @big5 */
                       memset(fepcb->preedbuf,NULL, strlen(fepcb->preedbuf));
                                                                      /* @Debby*/
                       memcpy(fepcb->preedbuf,code,2);                /* @big5 */
                    }                                                 /* @big5 */

                    fepcb->ret=FOUND_WORD;
                    StjFreeCandidates(fepcb);
                    free(string1);
                    return;
                 }
                 else
                    string=string+2;
                 break;

              case STJ1 :   /*  in type XB* , check 'B'   */

                 if ( key[0] != *(fepcb->curinpbuf+1) )
                 {
                    string=string+2;
                    break;
                 }

              case STJ2 :   /*  type X*B  */

              case STJ3 :   /*  type X*   */

                fepcb->stjstruct.allcandno++;    /*  count stjcand number   */

                strokeno[stroke]=1;    /*  for counting stroke number   *
                /********************************/
                /*    fill stjcand structure    */
                /********************************/

                stjptr->stroke=stroke; /*  fill stroke of stjcand structure */

                memset(stjptr->key, NULL, 5);/* fill key of stjcand structure */
                stjptr->key[0]='x';
                if ( type == STJ2 )
                {
                   for (j=0; j<3; j++)
                   {
                      if (key[j] != ' ')
                         stjptr->key[j+1]=key[j];
                      else
                         break;
                   }
                   stjptr->key[j+1]=*(fepcb->curinpbuf+2);
                }
                else
                {
                   for (j=0; j<4; j++)
                   {
                      if (key[j] != ' ')
                         stjptr->key[j+1]=key[j];
                      else
                         break;
                   }
                }
                                  /*  fill euc of stjcand structure  */
                memset(stjptr->euc, NULL, 4);
                stjptr->euc[0]=USR_FONT_EUC_BYTE1;
                stjptr->euc[1]=USR_FONT_EUC_BYTE2;
                stjptr->euc[2]=*(string);
                stjptr->euc[3]=*(string+1);
                if (fepcb->Lang_Type != codesetinfo[0].code_type)  /* @big5 */
                {                                                 /* @big5 */
                   in_count = 4;                                  /* @big5 */
                   out_count = 2;                                 /* @big5 */
                   StrCodeConvert(fepcb->iconv_flag,
                              stjptr->euc,code, &in_count, &out_count);
                                                                  /* @big5 */
                   memset(stjptr->euc, NULL, 4);                  /* @Debby*/
                   memcpy(stjptr->euc,code,2);                    /* @big5 */
                }                                                 /* @big5 */

                string=string+2;
                fepcb->ret=FOUND_CAND;
                stjptr++;
                break;
            }  /*   end case  */
         }    /*  end while  */
      }     /*  end while  */

      free(string1);
      if ( fepcb->ret == NOT_FOUND )
      {
         StjFreeCandidates(fepcb);
         return;
      }

                    /* call qsort library call to sort fepcb->stjcand  */
      qsort( (char *)fepcb->stjstruct.stjcand,
         (unsigned)fepcb->stjstruct.allcandno,sizeof(StjCand),StrokeCmp);

      for (j=0; j<STROKE_TOT_NO; j++)      /*  count stroke number */
         fepcb->strokestruct.allcandno+=strokeno[j];

                     /*  allocate memory for strokecand structure  */
      fepcb->strokestruct.strokecand=(StrokeCand *)calloc(
             fepcb->strokestruct.allcandno, sizeof(StrokeCand));

      strokeptr=fepcb->strokestruct.strokecand;
      strokeptr->stroke=MAX_STROKE+1;   /*  set value  */

      stjptr=fepcb->stjstruct.stjcand;

      for ( i=0; i<fepcb->stjstruct.allcandno; i++ )
                                      /* fill stroke and rrn to strokecand */
      {
         if ( strokeptr->stroke != stjptr->stroke )
         {
            if ( i != 0 )
               strokeptr++;
               strokeptr->stroke=stjptr->stroke;
               strokeptr->rrn=i;
         }
         stjptr++;
      }

   }  /*  end if  */
}

/**************************************************************************/
/* FUNCTION    : StrokeCmp                                                */
/* DESCRIPTION : As parameter of qsort library call                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : the result of compare                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

StrokeCmp(char *s1, char *s2)
{
   StjCand *stj1;
   StjCand *stj2;
   unsigned short stroke1;
   unsigned short stroke2;

   stj1=(StjCand *)s1;
   stj2=(StjCand *)s2;
   stroke1=stj1->stroke;        /*   get stroke   */
   stroke2=stj2->stroke;
   if (stroke1 == stroke2)      /* if stroke equal, compare key */
      return (strcmp(stj1->key, stj2->key));
   else
      return((int)(stroke1-stroke2));
}

/**************************************************************************/
/* FUNCTION    : TjFillSysCandidates                                      */
/* DESCRIPTION : According system dictionary, fill word to fepcb for      */
/*               Tsang-Jye Input Method or fill candidates to fepcb for   */
/*               Simplify-Tsang-Jye Input Method                          */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb fepcb->ret = FOUND_WORD,FOUND_CAND,NOT_FOUND,ERROR */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjFillSysCandidates(fepcb)
FEPCB *fepcb;
{
   int i;
   int j;
   int type;
   int cnt;
   unsigned short index;
   unsigned short stroke;
   unsigned short count;
   long offset;
   unsigned char *string;
   unsigned char *string1;
   unsigned char *end;
   unsigned char key[4];
   unsigned char inputkey[3];    /*  for Simplify-Tsang-Jye  */
   StjCand *stjptr;
   StrokeCand *strokeptr;
   int StjFreeCandidates();
   int SameTjCodeNo;                                                  /* V410 */
   unsigned short PreTjStroke;                                        /* V410 */
   unsigned char PreTjEucCodeLen,PreTjEucCode[5];                     /* V410 */
   unsigned char code[4];                                             /* @big5*/

   string=string1=NULL;            /* debby */
   stjptr = (StjCand *) NULL;        /* debby */
   strokeptr = (StrokeCand *) NULL;  /* debby */
   if ( fepcb->starpos == 0 )      /*  Tsang-Jye input method  */
   {

      if (fepcb->inputlen == 1)         /*  find index  */
         index=(*(fepcb->curinpbuf)-BASE-1)*27;
      else
         index=(*(fepcb->curinpbuf)-BASE-1)*27+
                *(fepcb->curinpbuf+(fepcb->inputlen-1))-BASE;

      switch(fepcb->inputlen)    /*  find input key  */
      {
         case 5 :   /*  radical number  */
           memcpy(inputkey, fepcb->curinpbuf+1, 3);
           break;
         case 4 :
           memcpy(inputkey, fepcb->curinpbuf+1, 2);
           inputkey[2]=' ';
           break;
         case 3 :
           inputkey[0]=*(fepcb->curinpbuf+1);
           inputkey[1]=' ';
           inputkey[2]=' ';
           break;
         case 2 :
           memset(inputkey, ' ', 3);
           break;
         case 1 :
           memset(inputkey, ' ', 3);
           break;
      }
      type=TJ0;
   }
   else                       /*  Simplify-Tsang-Jye input method  */
   {    /*****************************/
        /*  set type and find index  */
        /*****************************/
      if ( fepcb->starpos == 2 )        /*   AB*   */
      {
         index=(*(fepcb->curinpbuf)-BASE)+ZZ_INDEX;
         type=STJ1;
         fepcb->learnstruct.mode = ABSTART;                         /* V410 */
      }                                /*   A*B   */
      else if ( (fepcb->starpos == 1) && (fepcb->inputlen == 3) )
           {
              index=(*(fepcb->curinpbuf)-BASE-1)*27+*(fepcb->curinpbuf+2)-BASE;
              type=STJ2;
              fepcb->learnstruct.mode = ASTARTB;                    /* V410 */
           }
           else                       /*   A*    */
           {
              index=(*(fepcb->curinpbuf)-BASE)+ZZ_INDEX;
              type=STJ3;
              fepcb->learnstruct.mode = ASTART;                     /* V410 */
           }
   }

   if ( fepcb->mi.tjsysmi[index].count == NO_WORD )
   {
      fepcb->ret=NOT_FOUND;
      return;
   }
   else
   {
      if ( fseek(fepcb->fd.tjsysfd, fepcb->mi.tjsysmi[index].index, 0) )
      {
         fepcb->ret=ERROR;
         return;
      }
      else
      {    /*  get offset in order to determine how long be loaded  */
         offset=fepcb->mi.tjsysmi[index+1].index-
                fepcb->mi.tjsysmi[index].index;
         string=(unsigned char *)calloc(offset+1, sizeof(char));
         string1=string;
         end=string+offset; /*   end point to tail of block  */

         if ( type != TJ0 ) /*  Simplify-Tsang-Jye input method  */
         {
            if(fepcb->stjstruct.stjcand != NULL ||
               fepcb->strokestruct.strokecand != NULL)
               StjFreeCandidates(fepcb);

            if(stjptr != NULL) free(stjptr);
            if(strokeptr != NULL) free(strokeptr);

                         /*   allocate memory for stj candidates   */
            fepcb->stjstruct.stjcand=(StjCand *)calloc(
                       fepcb->mi.tjsysmi[index].count, sizeof(StjCand));
                         /*   allocate memory for stroke candidates  */
            fepcb->strokestruct.strokecand=(StrokeCand *)calloc(
                       fepcb->mi.tjsysmi[index].stroke,sizeof(StrokeCand));
            stjptr=fepcb->stjstruct.stjcand;
            strokeptr=fepcb->strokestruct.strokecand;

            cnt=0;         /*   set initial values   */
            strokeptr->stroke=MAX_STROKE+1;
         }

         if ( fread((char *)string, offset, 1, fepcb->fd.tjsysfd) == 0
              && ferror(fepcb->fd.tjsysfd) != 0 )
         {
            fepcb->ret=ERROR;
            StjFreeCandidates(fepcb);
            free(string1);
         }
         else
         {
              /*   while(string1!=end) printf("%02X ",*(string1++));
                       <<<    testing    >>> */
            fepcb->ret=NOT_FOUND;
            SameTjCodeNo = 0 ;                                       /* V410 */

            while ( string != end )
            {
               stroke=*(string);     /*  get stroke  */

               count=*(string+1)*256+*(string+2);   /*  get count   */
               string=string+3;

               for ( i=0; i<count; i++)
               {                      /*  get key   */
                  if ( (type == TJ0) || (type == STJ2) )
                  {
                     memcpy(key, string, 3);
                     string=string+3;
                  }
                  else
                  {
                     memcpy(key, string, 4);
                     string=string+4;
                  }

                  switch (type)
                  {
                     case STJ1 :                                      /* V410 */

                       if ( key[0] != *(fepcb->curinpbuf+1) )         /* V410 */
                       {                                              /* V410 */
                          if ( *(string) == EUC_BYTE1 )               /* V410 */
                             string=string+4;                         /* V410 */
                          else                                        /* V410 */
                             string=string+2;                         /* V410 */
                          break;                                      /* V410 */
                       }                                              /* V410 */

                     case TJ0 :

                      if (type == TJ0)                               /* V410 */
                       if ( !memcmp(key, inputkey, 3) )
                       {
                          if (fepcb->Lang_Type != codesetinfo[0].code_type)/* @big5*/
                          if ((*string) == EUC_BYTE1 )                /* @big5*/
/*                          if ((*(string+1) == PLANE3_EUC_BYTE2 ) ||    @big5*/
/*                              (*(string+1) == PLANE4_EUC_BYTE2 ) )     @big5*/
                       if ( (*(string+1) != PLANE2_EUC_BYTE2) &&      /* @V42 */
                            (*(string+1) != PLANEc_EUC_BYTE2) &&      /* @V42 */
                            (*(string+1) != PLANEd_EUC_BYTE2) )       /* @V42 */
                               {                                      /* @big5*/
                                string=string+4;                      /* @big5*/
                                break;                                /* @big5*/
                               }                                      /* @big5*/
                          SameTjCodeNo ++;                            /* V410 */
                          if (SameTjCodeNo < 2 )      /* find first Tj   V410 */
                          {
                            if ( *(string) == EUC_BYTE1 )             /* V410 */
                            {                                         /* V410 */
                               memcpy(PreTjEucCode,string,4);         /* V410 */
                               string=string+4;                       /* V410 */
                            }                                         /* V410 */
                            else                                      /* V410 */
                            {                                         /* V410 */
                               memcpy(PreTjEucCode,string,2);         /* V410 */
                               string=string+2;                       /* V410 */
                             }                                        /* V410 */
                            PreTjStroke = stroke;                     /* V410 */
                            break;                                    /* V410 */
                          }                                           /* V410 */
                          else                                        /* V410 */
                          {                          /* find next Tj     V410 */
                             if (SameTjCodeNo == 2 )                  /* V410 */
                             {                       /* find second Tj   V410 */

                               /* free memory for stj candidates         V410 */
                               if(fepcb->stjstruct.stjcand != NULL ||  /*V410 */
                                  fepcb->strokestruct.strokecand != NULL)/*V410*/
                                  StjFreeCandidates(fepcb);           /* V410 */

                               if(stjptr != NULL) free(stjptr);       /* V410 */
                               if(strokeptr != NULL) free(strokeptr); /* V410 */

                               /*   allocate memory for stj candidates   V410 */
                               fepcb->stjstruct.stjcand=(StjCand *)calloc(
                                 fepcb->mi.tjsysmi[index].count, sizeof(StjCand));/* V410 */
                              /*   allocate memory for stroke candidates  V410 */
                               fepcb->strokestruct.strokecand=(StrokeCand *)calloc(
                               fepcb->mi.tjsysmi[index].stroke,sizeof(StrokeCand));/* V410 */

                          fepcb->strokestruct.allcandno=0;
                               fepcb->stjstruct.allcandno=0;          /* V410 */
                               stjptr=fepcb->stjstruct.stjcand;       /* V410 */
                               strokeptr=fepcb->strokestruct.strokecand;/* V410 */

                               cnt=0;         /*   set initial values    V410 */
                               strokeptr->stroke=MAX_STROKE+1;        /* V410 */

                               /* fill first Tj code data to StjCand     V410 */
                               fepcb->stjstruct.allcandno++;          /* V410 */
                               stjptr->stroke =PreTjStroke;           /* V410 */
                               memset(stjptr->key,NULL,5);            /* V410 */
                               memcpy(stjptr->key,fepcb->curinpbuf,fepcb->inputlen);/* V410 */
                               memset(stjptr->euc,NULL,4);            /* V410 */
                               if (PreTjEucCode[0] == EUC_BYTE1)      /* V410 */
                                 if (fepcb->Lang_Type != codesetinfo[0].code_type)
                                                                      /* @big5*/
                                 {                                    /* @big5*/
                                    in_count = 4;                     /* @big5*/
                                    out_count = 2;                    /* @big5*/
                                    StrCodeConvert(fepcb->iconv_flag,
                                                       PreTjEucCode,code,
                                                       &in_count,&out_count);
                                                                      /* @big5*/
                                    memcpy(stjptr->euc,code,2);       /* @big5*/
                                 }                                    /* @big5*/
                                 else                                 /* @big5*/
                                   memcpy(stjptr->euc,PreTjEucCode,4);/* V410 */
                               else                                   /* V410 */
                                 if (fepcb->Lang_Type != codesetinfo[0].code_type)
                                                                      /* @big5*/
                                 {                                    /* @big5*/
                                    in_count = 2;                     /* @big5*/
                                    out_count = 2;                    /* @big5*/
                                    StrCodeConvert(fepcb->iconv_flag,
                                                       PreTjEucCode,code,
                                                       &in_count,&out_count);
                                                                      /* @big5*/
                                    memcpy(stjptr->euc,code,2);       /* @big5*/
                                 }                                    /* @big5*/
                                 else                                 /* @big5*/
                                   memcpy(stjptr->euc,PreTjEucCode,2);/* V410 */

                       if ( strokeptr->stroke != PreTjStroke )
                       {
                          if ( cnt != 0 )
                             strokeptr++;
                          fepcb->strokestruct.allcandno++;
                          strokeptr->stroke=PreTjStroke;
                          strokeptr->rrn=cnt;
                       }
                               cnt++;                                 /* V410 */
                               stjptr++;                              /* V410 */
                             }                                        /* V410 */
                          }                                           /* V410 */
                       }                                              /* V410 */

/*                        memset(fepcb->preedbuf, NULL,                  V410 */
/*                        strlen(fepcb->preedbuf));                      V410 */
/*                        if ( *(string) == EUC_BYTE1 )                  V410 */
/*                        {                                              V410 */
/*                           memcpy(fepcb->preedbuf,string,4);           V410 */
/*                           string=string+4;                            V410 */
/*                        }                                              V410 */
/*                        else                                           V410 */
/*                        {                                              V410 */
/*                           memcpy(fepcb->preedbuf,string,2);           V410 */
/*                           string=string+2;                            V410 */
/*                        }                                              V410 */
/*                        fepcb->ret=FOUND_WORD;                         V410 */
/*                        StjFreeCandidates(fepcb);                      V410 */
/*                        free(string1);                                 V410 */
/*                        return;                                        V410 */
/*                     }                                                 V410 */
                       else
                       {
                          if ( *(string) == EUC_BYTE1 )
                             string=string+4;
                          else
                             string=string+2;
                          break;                                     /*  V410 */
                       }
/*                     break;                                            V410 */

/*                   case STJ1 :                                         V410 */
/*                                                                       V410 */
/*                     if ( key[0] != *(fepcb->curinpbuf+1) )            V410 */
/*                     {                                                 V410 */
/*                        if ( *(string) == EUC_BYTE1 )                  V410 */
/*                           string=string+4;                            V410 */
/*                        else                                           V410 */
/*                           string=string+2;                            V410 */
/*                        break;                                         V410 */
/*                     }                                                 V410 */

                     case STJ2 :

                     case STJ3 :
                       if (fepcb->Lang_Type != codesetinfo[0].code_type)/* @big5*/
                       if ((*string) == EUC_BYTE1)                 /* @big5*/
/*                       if ((*(string+1) == PLANE3_EUC_BYTE2 ) ||    @big5*/
/*                           (*(string+1) == PLANE4_EUC_BYTE2 ) )     @big5*/
                         if ( (*(string+1) != PLANE2_EUC_BYTE2) && /* @V42 */
                            (*(string+1) != PLANEc_EUC_BYTE2) &&   /* @V42 */
                            (*(string+1) != PLANEd_EUC_BYTE2) )    /* @V42 */
                            {                                      /* @big5*/
                             string=string+4;                      /* @big5*/
                             break;                                /* @big5*/
                            }                                      /* @big5*/

                       fepcb->stjstruct.allcandno++;
                       /******************************/
                       /*   fill stjcand structure   */
                       /******************************/

                                 /* fill stroke of stjcand structure */
                       stjptr->stroke=stroke;

                                 /* fill key of stjcand structure  */
                       memset(stjptr->key, NULL, 5);
                       stjptr->key[0]=*(fepcb->curinpbuf);

                       if (type == TJ0)                               /* V410 */
                          memcpy(stjptr->key,fepcb->curinpbuf,fepcb->inputlen); /* V410 */
                       else                                           /* V410 */
                        if ( type == STJ2 )
                        {
                          for (j=0; j<3; j++)
                          {
                             if (key[j] != ' ')
                                stjptr->key[j+1]=key[j];
                             else
                                break;
                          }
                          stjptr->key[j+1]=*(fepcb->curinpbuf+2);
                        }
                        else
                        {
                          for (j=0; j<4; j++)
                          {
                             if (key[j] != ' ')
                                stjptr->key[j+1]=key[j];
                             else
                                break;
                          }
                        }

                                /*  fill euc of stjcand structure  */
                       memset(stjptr->euc, NULL, 4);
                       if ( *(string) == EUC_BYTE1 )
                       {
                          if (fepcb->Lang_Type != codesetinfo[0].code_type)/* @big5*/
                          {                                    /* @big5*/
                             in_count = 4;                     /* @big5*/
                             out_count = 2;                    /* @big5*/
                             StrCodeConvert(fepcb->iconv_flag,
                                                string,code,
                                                &in_count,&out_count);
                                                               /* @big5*/
                             memcpy(stjptr->euc,code,2);       /* @big5*/
                          }                                    /* @big5*/
                          else                                 /* @big5*/
                            memcpy(stjptr->euc, string, 4);
                          string=string+4;
                       }
                       else
                       {
                          if (fepcb->Lang_Type != codesetinfo[0].code_type)
                                                              /* @big5*/
                          {                                    /* @big5*/
                             in_count = 2;                     /* @big5*/
                             out_count = 2;                    /* @big5*/
                             StrCodeConvert(fepcb->iconv_flag,
                                                string,code,
                                                &in_count,&out_count);
                                                               /* @big5*/
                             memcpy(stjptr->euc,code,2);       /* @big5*/
                          }                                    /* @big5*/
                          else                                 /* @big5*/
                             memcpy(stjptr->euc ,string, 2);
                          string=string+2;
                       }

                               /*  fill strokecand structure */
                       if ( strokeptr->stroke != stroke )
                       {
                          if ( cnt != 0 )
                             strokeptr++;
                          fepcb->strokestruct.allcandno++;
                          strokeptr->stroke=stroke;
                          strokeptr->rrn=cnt;
                       }
                       cnt++;
                       stjptr++;
                       fepcb->ret=FOUND_CAND;
                  }   /*  end switch  */
               }  /*  end for  */
            } /*  end while  */
            if (SameTjCodeNo == 1)                                    /* V410 */
            {                                                         /* V410 */
                memset(fepcb->preedbuf, NULL,                         /* V410 */
                strlen(fepcb->preedbuf));                             /* V410 */
                if ( PreTjEucCode[0] == EUC_BYTE1 )                   /* V410 */
                  if (fepcb->Lang_Type != codesetinfo[0].code_type)   /* @big5*/
                  {                                                   /* @big5*/
                     in_count = 4;                                    /* @big5*/
                     out_count = 2;                                   /* @big5*/
                     StrCodeConvert(fepcb->iconv_flag,
                                        PreTjEucCode,code,
                                        &in_count,&out_count);        /* @big5*/
                     memcpy(fepcb->preedbuf,code,2);                  /* @big5*/
                  }                                                   /* @big5*/
                  else                                                /* @big5*/
                     memcpy(fepcb->preedbuf,PreTjEucCode,4);          /* V410 */
                else                                                  /* V410 */
                  if (fepcb->Lang_Type != codesetinfo[0].code_type)   /* @big5*/
                  {                                                   /* @big5*/
                     in_count = 2;                                    /* @big5*/
                     out_count = 2;                                   /* @big5*/
                     StrCodeConvert(fepcb->iconv_flag,
                                        PreTjEucCode,code,
                                        &in_count,&out_count);        /* @big5*/
                     memcpy(fepcb->preedbuf,code,2);                  /* @big5*/
                  }                                                   /* @big5*/
                  else                                                /* @big5*/
                     memcpy(fepcb->preedbuf,PreTjEucCode,2);          /* V410 */
                fepcb->ret=FOUND_WORD;                                /* V410 */
                StjFreeCandidates(fepcb);                             /* V410 */
                free(string1);                                        /* V410 */
                return;                                               /* V410 */
            } /* end SameTjCodeNo                                        V410 */
         }  /*  end if  */

         free(string1);
         if ( fepcb->ret == NOT_FOUND )
            StjFreeCandidates(fepcb);
      }
   }
   if(string != NULL && string1 != NULL)
   {
      free(string1);
      string = NULL;
   }/*  for xaltu       */
   return;       /* debby */
}



/***************************************************************************/
/* FUNCTION    : StjFreeCandidates                                         */
/* DESCRIPTION : Free allocated memory for Simplify-Tsang-Jye candidates   */
/*               and Stroke candidates                                     */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

StjFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->stjstruct.stjcand != NULL )
   {
      /***************************/
      /*  initial stj structure  */
      /***************************/
      free(fepcb->stjstruct.stjcand);
      fepcb->stjstruct.stjcand=NULL;
      fepcb->stjstruct.allcandno=0;
      fepcb->stjstruct.headcandno=0;
      fepcb->stjstruct.curcandno=0;
      fepcb->stjstruct.tailcandno=0;
   }
   if(fepcb->strokestruct.strokecand != NULL)
   {
      /******************************/
      /*  initial stroke structure  */
      /******************************/
      free(fepcb->strokestruct.strokecand);
      fepcb->strokestruct.strokecand=NULL;
      fepcb->strokestruct.allcandno=0;
      fepcb->strokestruct.headcandno=0;
      fepcb->strokestruct.curcandno=0;
      fepcb->strokestruct.tailcandno=0;
   }
}


/***************************************************************************/
/* FUNCTION    : PhFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for Phonetic candidates             */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

PhFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->phstruct.phcand != NULL )
   {
      /****************************/
      /*   initial ph structure   */
      /****************************/
      free(fepcb->phstruct.phcand);
      fepcb->phstruct.phcand=NULL;
      fepcb->phstruct.curptr=NULL;
      fepcb->phstruct.allcandno=0;
      fepcb->phstruct.more=0;

   }
}
