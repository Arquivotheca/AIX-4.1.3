static char sccsid[] = "@(#)52	1.3  src/bos/usr/lib/nls/loc/ZH.im/zhedacc.c, ils-zh_CN, bos41J, 9513A_all 3/26/95 05:49:29";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AccessDictionary
 *		AccessLDictionary
 *		EnFillCandidates
 *		EnFreeCandidates
 *		EnLoadSysFileMI
 *		EnLoadUsrFileMI
 *		EnUsrDictFileChange
 *		EnValidDict
 *		LeFillCandidates
 *		LeFreeCandidates
 *		LeLoadSysFileMI
 *		LeLoadUsrFileMI
 *		LeUsrDictFileChange
 *		LeValidDict
 *		PyFillCandidates
 *		PyFreeCandidates
 *		PyLoadSysFileMI
 *		PyLoadUsrFileMI
 *		PyUsrDictFileChange
 *		PyValidDict
 *		TjFillSysCandidates
 *		TjFillUserCandidates
 *		TjFreeCandidates
 *		TjLoadSysFileMI
 *		TjLoadUsrFileMI
 *		TjUsrDictFileChange
 *		TjValidDict
 *		itoa
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
/********************* START OF MODULE SPECIFICATION ***********************/
/*                                                                         */
/* MODULE NAME:        ZHedAcc                                             */
/*                                                                         */
/* DESCRIPTIVE NAME:   Chinese Input Method Access Dictionary              */
/*                                                                         */
/* FUNCTION:           AccessDictionary    : Access File                   */
/*                                                                         */
/*                     AccessLDictionary   : Access Legend Dictionary File */
/*                                                                         */
/*                     PyUsrDictFileChange : Check PY User File Change     */
/*                                                                         */
/*                     PyCompareRadicals   : Compare PY Radicals           */
/*                                                                         */
/*                     PyLoadSysFileMI     : Load MI of PY System File     */
/*                                                                         */
/*                     PyLoadUsrFileMI     : Load MI of PY User File       */
/*                                                                         */
/*                     PyFillCandidates    : Find PY System File Cand.     */
/*                                           Find PY User File Cand.       */
/*                                                                         */
/*                     PyFreeCandidates    : Free PY Candidates            */
/*                                                                         */
/*                     TjUsrDictFileChange : Check Tj User File Change     */
/*                                                                         */
/*                     TjCompareRadicals   : Compare Tj Radicals           */
/*                                                                         */
/*                     TjLoadSysFileMI     : Load MI of Tj System File     */
/*                                                                         */
/*                     TjLoadUsrFileMI     : Load MI of Tj User File       */
/*                                                                         */
/*                     TjFillSysCandidates : Find Tj System File Cand.     */
/*                                                                         */
/*                     TjFillUserCandidates: Find Tj User File Cand.       */
/*                                                                         */
/*                     TjFreeCandidates    : Free Tj Candidates            */
/*                                                                         */
/*                     EnUsrDictFileChange : Check EN User File Change     */
/*                                                                         */
/*                     EnCompareRadicals   : Compare EN Radicals           */
/*                                                                         */
/*                     EnLoadSysFileMI     : Load MI of EN System File     */
/*                                                                         */
/*                     EnLoadUsrFileMI     : Load MI of EN User File       */
/*                                                                         */
/*                     EnFillCandidates    : Find EN System File Cand.     */
/*                                           Find EN User File Cand.       */
/*                                                                         */
/*                     EnFreeCandidates    : Free EN Candidates            */
/*                                                                         */
/*                     LeUsrDictFileChange : Check LE User File Change     */
/*                                                                         */
/*                     LeLoadSysFileMI     : Load MI of LE System File     */
/*                                                                         */
/*                     LeLoadUsrFileMI     : Load MI of LE User File       */
/*                                                                         */
/*                     LeFillCandidates    : Find LE System File Cand.     */
/*                                           Find LE User File Cand.       */
/*                                                                         */
/*                     LeFreeCandidates    : Free LE Candidates            */
/*                                                                         */
/* MODULE TYPE:        C                                                   */
/*                                                                         */
/* COMPILER:           AIX C                                               */
/*                                                                         */
/* AUTHOR:             Tang Bosong, WuJian                                 */
/*                                                                         */
/* STATUS:             Chinese Input Method Version 1.0                    */
/*                                                                         */
/* CHANGE ACTIVITY:                                                        */
/*                                                                         */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include "zhedinit.h"
#include "zhed.h"
#include "zhedacc.h"
#include "dictionary.h"

#define FOUND  1

int AccessDictionary();
int PyUsrDictFileChange();
int PyLoadSysFileMI();
int PyLoadUsrFileMI();
int PyFillCandidates();
int PyFreeCandidates();
int PyValidDict();               
int TjUsrDictFileChange();
int TjLoadSysFileMI();
int TjLoadUsrFileMI();
int TjFillSysCandidates();
int TjFillUserCandidates();
int TjFreeCandidates();
int TjValidDict();               
int EnUsrDictFileChange();
int EnLoadSysFileMI();
int EnLoadUsrFileMI();
int EnFillCandidates();
int EnFreeCandidates();
int EnValidDict();               
int AccessLDictionary();
int LeUsrDictFileChange();
int LeLoadSysFileMI();
int LeLoadUsrFileMI();
int LeFillCandidates();
int LeFreeCandidates();
int LeValidDict();               


/**************************************************************************/
/* FUNCTION    : AccessDictionary                                         */
/* DESCRIPTION : Fill word for Pinyin Input Method, fill candidates for   */
/*               for English and ABC Input Method.                        */
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
   int PyUsrDictFileChange();
   int PyCompareRadicals();
   int PyUsrDictFileChange();

   switch ( fepcb->imode.ind0 )
   {
      case ENGLISH_CHINESE_MODE:
         if ( fepcb->fd.enusrfd != NULL )
         {                           /*  have en user file  */
            if ( (temp=EnUsrDictFileChange(fepcb)) == FALSE )
            {
               EnFreeCandidates(fepcb);
               EnFillCandidates(fepcb, USR); /* find en usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  EnFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                        EnFreeCandidates(fepcb);
               EnFillCandidates(fepcb, SYS); /* find en sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  EnFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                  fepcb->ret=FOUND_CAND;
               else
                  fepcb->ret=NOT_FOUND;
            }
            else
            {
               if ( temp == ERROR )
                  return;
               else
               {
                  EnFreeCandidates(fepcb);
                  EnFillCandidates(fepcb, USR);
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     EnFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          EnFreeCandidates(fepcb);
                  EnFillCandidates(fepcb, SYS);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     EnFreeCandidates(fepcb);
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
         {                   /*  no en usr file  */
            EnFreeCandidates(fepcb);
            EnFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               EnFreeCandidates(fepcb);
         }
         break;

      case PINYIN_MODE:
         if ( fepcb->fd.pyusrfd != NULL )
         {                           /*  have py user file  */
            if ( (temp=PyUsrDictFileChange(fepcb)) == FALSE )
            {
               PyFreeCandidates(fepcb);
               PyFillCandidates(fepcb, USR, 0); /* find py usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                        PyFreeCandidates(fepcb);
               
               PyFillCandidates(fepcb, SYS, COMM); /* find py sys file cand. */
               if ( fepcb->ret == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               ret2 = fepcb->ret;
               if ( fepcb->imode.ind8 == CHINESE_GB || fepcb->imode.ind8 == CHINESE_GLOBLE )
               {
                  PyFillCandidates(fepcb, SYS, GB);
                  if ( fepcb->ret == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  if ( fepcb->ret == FOUND_CAND )
                     ret2 = fepcb->ret;
               }
               if ( fepcb->imode.ind8 == CHINESE_CNS || fepcb->imode.ind8 == CHINESE_GLOBLE )
               {
                  PyFillCandidates(fepcb, SYS, CNS);
                  if ( fepcb->ret == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  if ( fepcb->ret == FOUND_CAND )
                     ret2 = fepcb->ret;
               }
               if ( fepcb->imode.ind8 == CHINESE_GLOBLE )
               {
                   PyFillCandidates(fepcb, SYS, JK);
                   if ( fepcb->ret == ERROR )
                   {
                      PyFreeCandidates(fepcb);
                      return;
                   }
                   if ( fepcb->ret == FOUND_CAND )
                      ret2 = fepcb->ret;
               }
               if ( ret2 == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                  fepcb->ret=FOUND_CAND;
               else
                  fepcb->ret=NOT_FOUND;
            }
            else
            {
               if ( temp == ERROR )
                  return;
               else
               {
                  PyFreeCandidates(fepcb);
                  PyFillCandidates(fepcb, USR, 0);
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          PyFreeCandidates(fepcb);

                  PyFillCandidates(fepcb, SYS, COMM);
                  if ( fepcb->ret == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  ret2 = fepcb->ret;
                  if ( fepcb->imode.ind8 == CHINESE_GB || fepcb->imode.ind8 == CHINESE_GLOBLE )
                  {
                     PyFillCandidates(fepcb, SYS, GB);
                     if ( fepcb->ret == ERROR )
                     {
                        PyFreeCandidates(fepcb);
                        return;
                     }
                     if ( fepcb->ret == FOUND_CAND )
                        ret2 = fepcb->ret;
                  }
                  if ( fepcb->imode.ind8 == CHINESE_CNS || fepcb->imode.ind8 == CHINESE_GLOBLE )
                  {
                     PyFillCandidates(fepcb, SYS, CNS);
                     if ( fepcb->ret == ERROR )
                     {
                        PyFreeCandidates(fepcb);
                        return;
                     }
                     if ( fepcb->ret == FOUND_CAND )
                        ret2 = fepcb->ret;
                  }
                  if ( fepcb->imode.ind8 == CHINESE_GLOBLE )
                  {
                      PyFillCandidates(fepcb, SYS, JK);
                      if ( fepcb->ret == ERROR )
                      {
                         PyFreeCandidates(fepcb);
                         return;
                      }
                      if ( fepcb->ret == FOUND_CAND )
                         ret2 = fepcb->ret;
                  }

                  if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no py usr file  */
            PyFreeCandidates(fepcb);
            PyFillCandidates(fepcb, SYS, COMM);
            if ( fepcb->ret == ERROR )
            {
               PyFreeCandidates(fepcb);
               return;
            }
            ret2 = fepcb->ret;
            if ( fepcb->imode.ind8 == CHINESE_GB || fepcb->imode.ind8 == CHINESE_GLOBLE )
            {
               PyFillCandidates(fepcb, SYS, GB);
               if ( fepcb->ret == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               if ( fepcb->ret == FOUND_CAND )
                  ret2 = fepcb->ret;
            }
            if ( fepcb->imode.ind8 == CHINESE_CNS || fepcb->imode.ind8 == CHINESE_GLOBLE )
            {
               PyFillCandidates(fepcb, SYS, CNS);
               if ( fepcb->ret == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               if ( fepcb->ret == FOUND_CAND )
                  ret2 = fepcb->ret;
            }
            if ( fepcb->imode.ind8 == CHINESE_GLOBLE )
            {
               PyFillCandidates(fepcb, SYS, JK);
               if ( fepcb->ret == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               if ( fepcb->ret == FOUND_CAND )
                  ret2 = fepcb->ret;
            }
            fepcb->ret = ret2;
         }
         break;

      case TSANG_JYE_MODE:
         if ( fepcb->fd.tjusrfd != NULL )
         {                           /*  have tj user file  */
            if ( (temp=TjUsrDictFileChange(fepcb)) == FALSE )
            {
               TjFreeCandidates(fepcb);
               TjFillUserCandidates(fepcb); /* find tj usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  TjFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                        TjFreeCandidates(fepcb);
               TjFillSysCandidates(fepcb); /* find tj sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  TjFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                  fepcb->ret=FOUND_CAND;
               else
                  fepcb->ret=NOT_FOUND;
            }
            else
            {
               if ( temp == ERROR )
                  return;
               else
               {
                  TjFreeCandidates(fepcb);
                  TjFillUserCandidates(fepcb);
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     TjFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          TjFreeCandidates(fepcb);
                  TjFillSysCandidates(fepcb);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     TjFreeCandidates(fepcb);
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
         {                   /*  no tj usr file  */
            TjFreeCandidates(fepcb);
            TjFillSysCandidates(fepcb);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               TjFreeCandidates(fepcb);
         }
         break;
   }
}

/**************************************************************************/
/* FUNCTION    : AccessLDictionary                                        */
/* DESCRIPTION : Fill word for Pinyin Legend Input Method.                */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

AccessLDictionary(fepcb)
FEPCB *fepcb;
{
   int temp;
   int ret1;
   int ret2;
   int LeUsrDictFileChange();

   if ( fepcb->fd.leusrfd != NULL )
   {                                        /*  have legend user file  */
      if ( (temp=LeUsrDictFileChange(fepcb)) != ERROR )
      {
         LeFreeCandidates(fepcb);
         LeFillCandidates(fepcb, USR); /* find legend usr file cand. */
         ret1=fepcb->ret;
         if ( ret1 == ERROR )
         {
             LeFreeCandidates(fepcb);
             return;
          }
          else if ( ret1 == NOT_FOUND )
                  LeFreeCandidates(fepcb);
          LeFillCandidates(fepcb, SYS); /* find legend sys file cand. */
          ret2=fepcb->ret;
          if ( ret2 == ERROR )
          {
              LeFreeCandidates(fepcb);
              return;
           }
           if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
              fepcb->ret=FOUND_CAND;
           else
              fepcb->ret=NOT_FOUND;
           }
      else
      {
         return;
      }
   }
   else
   {                   /*  no legend usr file  */
      LeFreeCandidates(fepcb);
      LeFillCandidates(fepcb, SYS);
      if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
         LeFreeCandidates(fepcb);
    }
}


/**************************************************************************/
/* FUNCTION    : PyUsrDictFileChange                                      */
/* DESCRIPTION : Change Pyonetic user dictionary file whether it change   */
/*               or not, if it change, reload master index of file        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PyUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int PyLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.pyusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.pytime, p) )
      return(FALSE);      /*   file had not been changed   */
   else
   {                      /*   file had been changed       */
      if ( PyLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.pytime, p);
         return(TRUE);
      }
   }
}


/******************************************************************************/
/* FUNCTION    : PyValidDict                                                  */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Pinyin user dictionary                          */
/*               FALSE = invalid Pinyin user dictionary                       */
/******************************************************************************/

PyValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *pydictinfo;
   char *p;
   extern char *ctime();
   int cnt, Index, length;
   unsigned char *addr, *buf;
   unsigned int code;
   unsigned short word;

   stat(fepcb->fname.pyusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.pyusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.pyusrfd, 0, 0);
   pydictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(pydictinfo, sizeof(struct dictinfo), 1, fepcb->fd.pyusrfd) == 0 
           && ferror(fepcb->fd.pyusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(pydictinfo->d_mtime)), p) )
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 26)&&(cnt < 5); Index++ ) 
         {
            if ((length = fepcb->mi.pyusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.pyusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.pyusrfd, (long)addr, 0) != 0 )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.pyusrfd) == 0 
                    && ferror(fepcb->fd.pyusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }
            code = *(unsigned int *)buf;
            word = *(unsigned short *)(buf + 4);
            if((code & 0x80000000) == 0 && (word & 0x8000))
            {
               cnt++; 
               continue;
            }
            else 
               return(FALSE);
         }
      }
   }
   return(TRUE);
}

/**************************************************************************/
/* FUNCTION    : PyLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Pinyin user file                    */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PyLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.pyusrfd == NULL )
   {
      if ( (fepcb->fd.pyusrfd=fopen(fepcb->fname.pyusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial zhed  */
         stat(fepcb->fname.pyusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.pytime, p);
      }
   }
   else
   {
      free(fepcb->mi.pyusrmi);
   }

   fseek(fepcb->fd.pyusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */

   memset(fepcb->mi.pyusrmi, NULL, PY_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.pyusrmi, PY_USR_MI_LEN, sizeof(MI),
      fepcb->fd.pyusrfd) == 0 && ferror(fepcb->fd.pyusrfd) != 0 )
      return(ERROR);
   else
   {
      if (PyValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.pyusrmi);
          fepcb->fd.pyusrfd = NULL;
          memset(fepcb->ctime.pytime, NULL, strlen(fepcb->ctime.pytime));
          return(ERROR);
      }
   }
}



/**************************************************************************/
/* FUNCTION    : PyLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Pinyin system file                  */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PyLoadSysFileMI(fepcb)
FEPCB *fepcb;
{
                           /* Load master index of Pinyin Common system file */
   if ( (fepcb->fd.pysysfd[COMM]=fopen(fepcb->fname.pysysfname[COMM], "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.pysysmi[COMM], NULL, PY_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.pysysfd[COMM], sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.pysysmi[COMM], PY_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.pysysfd[COMM]) == 0 && ferror(fepcb->fd.pysysfd[COMM]) != 0 )
      return(ERROR);
                           /* Load master index of Pinyin GB system file */
   if ( (fepcb->fd.pysysfd[GB]=fopen(fepcb->fname.pysysfname[GB], "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.pysysmi[GB], NULL, PY_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.pysysfd[GB], sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.pysysmi[GB], PY_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.pysysfd[GB]) == 0 && ferror(fepcb->fd.pysysfd[GB]) != 0 )
      return(ERROR);
                           /* Load master index of Pinyin CNS system file */
   if ( (fepcb->fd.pysysfd[CNS]=fopen(fepcb->fname.pysysfname[CNS], "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.pysysmi[CNS], NULL, PY_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.pysysfd[CNS], sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.pysysmi[CNS], PY_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.pysysfd[CNS]) == 0 && ferror(fepcb->fd.pysysfd[CNS]) != 0 )
      return(ERROR);
                           /* Load master index of Pinyin JK system file */
   if ( (fepcb->fd.pysysfd[JK]=fopen(fepcb->fname.pysysfname[JK], "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.pysysmi[JK], NULL, PY_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.pysysfd[JK], sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.pysysmi[JK], PY_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.pysysfd[JK]) == 0 && ferror(fepcb->fd.pysysfd[JK]) != 0 )
      return(ERROR);

   return(OK);
}

/**************************************************************************/
/* FUNCTION    : PyFillCandidates                                         */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

PyFillCandidates( fepcb, flag, torr )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
   int             torr;               /* COMM, GB, CNS Or JK                */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[8];         /* Echo Buffer                        */
   unsigned char   *miptr;             /* Pointer To Master Index            */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code;               /* Code Of Radicals                   */
   unsigned short  a_code;             /* A Code/Word Of Dictionary          */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             allcandno;          /* Number of Candidates               */
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             found = NOT_FOUND;

   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<6; cnt++ )  echobuf[cnt] = '\0';

                                       /* Compress The Content Of Echo Buffer*/
   code = 0;
   for( cnt=0; cnt<fepcb->echoacsz; echobufptr ++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      if ( cnt >= 1 ) 
         code |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
   }

   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.pyusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.pyusrmi[echobuf[0] - 'a'].length) == 0)
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.pyusrmi[echobuf[0] - 'a'].index;
      fepcb->pystruct.cand = (unsigned char *)malloc(spcounter=1);
      memset(fepcb->pystruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else   /* flag = SYS */
   {
      fdptr = (FILE *)fepcb->fd.pysysfd[torr];
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.pysysmi[torr][echobuf[0] - 'a'].length) == 0)
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.pysysmi[torr][echobuf[0] - 'a'].index;
      if( *fepcb->pystruct.cand == '\0' )
         spcounter = 1;
      else
         spcounter = strlen( fepcb->pystruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != 0 )
   {
      fepcb->ret = ERROR;
      return;
   }

   drbuf = (unsigned char *)malloc(drlength);
   if( fread(drbuf, drlength, 1, fdptr) != 1 && ferror(fdptr) != 0)
   {
      free(drbuf);
      fepcb->ret = ERROR;
      return;
   }
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;      /* Jan.12 95 Modified By B.S.Tang     */

   while( (drbegin <= drend) && ((drend - drbegin) != 2) ) 
   {
      if ( code == *(unsigned int *)drbuf )
      {
         chptr = &drbuf[4];
         found = FOUND;
         break;
      }  
      drmiddle=(drbegin + drend)/2;
      drmiddle= drmiddle & 0xFFFFFFFE;
      chptr= &drbuf[drmiddle];

      if ( (*chptr & 0x80) != 0 )
      {
         while ( (*chptr++ & 0x80) && (chptr < &drbuf[drlength]) );
         if ( chptr == &drbuf[drlength] )
         {
            drend = drmiddle;
            continue;
         }
         --chptr;
      }
      if ( (*(chptr -3 ) & 0xe0) == 0xe0 )
         r_code = *(unsigned int *)chptr;
      else 
      {
         if ( (*(chptr - 3) & 0x80) == 0 )
         {
            r_code = *(unsigned int *)(chptr - 3);
            chptr -= 3;
         }
         else
         { 
            for ( cnt = 2; (cnt >= 0) && (*(chptr - cnt)&0x80); cnt--);
            r_code = *(unsigned int *)(chptr - cnt);
            chptr -= cnt;
         }
      }  

      if ( code > r_code )
      {
         drbegin = drmiddle;
         continue;
      }
      if ( code < r_code )
      {
         drend = drmiddle;
         continue;
      }

      if ( code == r_code)                /* Candidates Be Found */
      {
         chptr += 4;
         found = FOUND;
         break;
      }
   }
   if ( found == FOUND )
   {
         candptr = chptr;
         while ( (*chptr & 0x80) && (chptr < &drbuf[drlength]) )
            chptr ++;
         allcandno = (chptr - candptr) / 3;
         fepcb->pystruct.cand = 
               (unsigned char *)realloc(fepcb->pystruct.cand, (spcounter + (chptr - candptr)));
         chptr = (unsigned char *)fepcb->pystruct.cand;
         for(cnt = 0; cnt < (spcounter - 1); chptr++, cnt++);

         strncpy(chptr, candptr, allcandno * 3);
         *(chptr + allcandno * 3) = '\0';
         fepcb->pystruct.allcandno += allcandno;
         free(drbuf);
         fepcb->ret = FOUND_CAND;
         return;
   }
   free(drbuf);
   fepcb->ret = NOT_FOUND;
   return;
}


/***************************************************************************/
/* FUNCTION    : PyFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for Pinyin candidates               */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

PyFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->pystruct.cand != NULL )
   {
      /****************************/
      /*   initial py structure   */
      /****************************/
      free(fepcb->pystruct.cand);
      fepcb->pystruct.cand=NULL;
      fepcb->pystruct.curptr=NULL;
      fepcb->pystruct.allcandno=0;
      fepcb->pystruct.more=0;

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


/**************************************************************************/
/* FUNCTION    : LeUsrDictFileChange                                      */
/* DESCRIPTION : Change Legend user dictionary file whether it change     */
/*               or not, if it change, reload master index of file        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

LeUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int LeLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.leusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.letime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( LeLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.letime, p);
         return(TRUE);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : LeLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Legend system file                  */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

LeLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.lesysfd=fopen(fepcb->fname.lesysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.lesysmi, NULL, LE_SYS_MI_LEN * sizeof(MI));
   fseek(fepcb->fd.lesysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */

   if ( fread((char *)fepcb->mi.lesysmi, LE_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.lesysfd) == 0 && ferror(fepcb->fd.lesysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}


/******************************************************************************/
/* FUNCTION    : LeValidDict                                                  */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Legned user dictionary                          */
/*               FALSE = invalid Legned user dictionary                       */
/******************************************************************************/

LeValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *ledictinfo;
   char *p;
   extern char *ctime();
   int cnt, Index, length;
   unsigned char *addr, *buf;
   unsigned short  code;
   unsigned short  word;

   stat(fepcb->fname.leusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.leusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.leusrfd, 0, 0);
   ledictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(ledictinfo, sizeof(struct dictinfo), 1, fepcb->fd.leusrfd) == 0 
           && ferror(fepcb->fd.leusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(ledictinfo->d_mtime)), p) )
         return(FALSE);
      else 
      {
         for( cnt = 0; (Index < 0x52) && (cnt < 5); Index++ ) 
         {
            if ((length = fepcb->mi.leusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.leusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.leusrfd, (long)addr, 0) != 0 )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.leusrfd) != 1 
                    && ferror(fepcb->fd.leusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }
            code = (unsigned short )buf;
            word = (unsigned short )(buf + 2);
            if((code & 0x8000) == 0 && (word & 0x8000)) 
            { 
               cnt++;
               continue;
            }
            else 
            {
               free(buf);
               return(FALSE);
            }
         }
      }
   }
   return(TRUE);
}

/**************************************************************************/
/* FUNCTION    : LeLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Legned user file                    */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

LeLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.leusrfd == NULL )
   {
      if ( (fepcb->fd.leusrfd=fopen(fepcb->fname.leusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial zhed  */
         stat(fepcb->fname.leusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.letime, p);
      }
   }
   else
      free(fepcb->mi.leusrmi);

   fseek(fepcb->fd.leusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */

   memset(fepcb->mi.leusrmi, NULL, LE_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.leusrmi, LE_USR_MI_LEN, sizeof(MI),
      fepcb->fd.leusrfd) == 0 && ferror(fepcb->fd.leusrfd) != 0 )
      return(ERROR);
   else
   {
      if (LeValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.leusrmi);
          fepcb->fd.leusrfd = NULL;
          memset(fepcb->ctime.letime, NULL, strlen(fepcb->ctime.letime));
          return(ERROR);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : LeFillCandidates                                         */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

LeFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[2];         /* Echo Buffer                        */
   unsigned char   *miptr;             /* Pointer To Master Index            */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned short  le_index;           /* Index Of Legend Radicals           */
   unsigned short  code;               /* Code Of Radicals                   */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             drbegin;            
   int             drmiddle;
   int             drend;
   wchar_t         le_code;            

   echobufptr = (unsigned char *)fepcb->edendbuf;
   mbtowc(&le_code, echobufptr, 3);
   le_index = le_code >> 8;
   code = le_code & 0xff;
                                       /* Compress The Content Of Echo Buffer*/
   
   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.leusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.leusrmi[le_index - 0x4e].length) == 0)
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.leusrmi[le_index - 0x4e].index;
      fepcb->lestruct.cand = (unsigned char *)malloc(spcounter=2);
      memset(fepcb->lestruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else   /* flag = SYS */
   {
      fdptr = (FILE *)fepcb->fd.lesysfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.lesysmi[le_index - 0x4e].length) == 0)
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.lesysmi[le_index - 0x4e].index;
      if( *fepcb->lestruct.cand == '\0' )
         spcounter = 2;
      else
         spcounter = strlen(fepcb->lestruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != 0 )
   {
      fepcb->ret = ERROR;
      return;
   }

   drbuf = (unsigned char *)malloc(drlength);
   if( fread(drbuf, drlength, 1, fdptr) != 1 && ferror(fdptr) != 0)
   {
      free(drbuf);
      fepcb->ret = ERROR;
      return;
   }

   drbegin = 0;
   drend = drlength;
   drmiddle = 0;      /* Jan.12 95 Modified By B.S.Tang     */

   while( (drbegin <= drend) && ((drend - drbegin) > 2) ) 
   {
      drmiddle=(drbegin + drend)/2;
      chptr= &drbuf[drmiddle];
      while( (*chptr-- != MASK) && (chptr > &drbuf[0]) );
      if ( chptr == &drbuf[0] )
         r_code = *(unsigned short *)chptr;
      else
      {
         chptr += 2;
         r_code = *(unsigned short *)chptr ;
      }

      if ( code > r_code )              /* Compare Input Code With Group Code */
      {
         drbegin = drmiddle ;
         continue;
      }
      if ( code < r_code )
      {
         drend = drmiddle ;
         continue;
      }
      if ( code == r_code) {
         chptr = chptr + 2; 
         while ( LOOP )              /* Fill Satisfied Cand. to Cand. Buffer */
         {
            if( (*chptr & MASK) == MASK ) 
            {
               *candptr++ = *chptr;
               *candptr = '\0';
               break;                /* End Of This Group Or Not     */
            }
 
            spcounter += 3;
            fepcb->lestruct.cand = 
               (unsigned char *)realloc(fepcb->lestruct.cand, spcounter);
            candptr = (unsigned char *)fepcb->lestruct.cand;
            for( cnt=2; cnt<(spcounter-3); cnt++, candptr++ );
            *candptr++ = *chptr++;
            *candptr++ = *chptr++;
            *candptr++ = *chptr;

            if( (*chptr & 0x80) == 0 )    /* End Of One Phrase Or Not     */
               fepcb->lestruct.allcandno++;

            *candptr = '\0';
            chptr++;
         }

         free(drbuf);
         fepcb->ret = FOUND_CAND;
         return;
      }
   }

   free(drbuf);
   fepcb->ret = NOT_FOUND;
   return;
}


/***************************************************************************/
/* FUNCTION    : LeFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for Pinyin Legend candidates        */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

LeFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->lestruct.cand != NULL )
   {
      /****************************/
      /*   initial le structure   */
      /****************************/
      free(fepcb->lestruct.cand);
      fepcb->lestruct.cand=NULL;
      fepcb->lestruct.curptr=NULL;
      fepcb->lestruct.allcandno=0;
      fepcb->lestruct.more=0;

   }
}


/**************************************************************************/
/* FUNCTION    : EnUsrDictFileChange                                      */
/* DESCRIPTION : Change English_Chinese user dictionary file whether it   */
/*               change or not, if it change, reload master index of file */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

EnUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int EnLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.enusrfname, &info);
   p = ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.entime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( EnLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.entime, p);
         return(TRUE);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : EnLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of English_Chinese system file         */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

EnLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.ensysfd=fopen(fepcb->fname.ensysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.ensysmi, NULL, EN_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.ensysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.ensysmi, EN_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.ensysfd) == 0 && ferror(fepcb->fd.ensysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}


/******************************************************************************/
/* FUNCTION    : EnValidDict                                                  */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid English_Chinese user dictionary                 */
/*               FALSE = invalid English_Chinese user dictionary              */
/******************************************************************************/

EnValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *endictinfo;
   char *p;
   extern char *ctime();
   int cnt, i, Index, length;
   unsigned char *addr, *buf, *buf1;
   unsigned short code, word;


   stat(fepcb->fname.enusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.enusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.enusrfd, 0, 0);
   endictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(endictinfo, sizeof(struct dictinfo), 1, fepcb->fd.enusrfd) == 0 
           && ferror(fepcb->fd.enusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(endictinfo->d_mtime)), p) )
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 26) && (cnt < 5); Index++ ) 
         {
            if ((length = fepcb->mi.enusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.enusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.enusrfd, (long)addr, 0) != 0)
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.enusrfd) == 0 
                    && ferror(fepcb->fd.enusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }
            code = *(unsigned short *)buf;
            for( i = 1; i < 4; i++)
            {
               word = *(unsigned short *)(buf + 6 * i);
               if((word & 0x8000) == 0)
                   continue;
               else
                   break;
            }
            if((code & 0x8000) == 0 && (word & 0x8000)) 
            {
               cnt++;
               continue;
            }
            else 
            {
               free(buf);
               return(FALSE);
            }
         }
      }
   }
   return(TRUE);
}

/**************************************************************************/
/* FUNCTION    : EnLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of English_Chinese user file           */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

EnLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.enusrfd == NULL )
   {
      if ( (fepcb->fd.enusrfd=fopen(fepcb->fname.enusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial zhed  */
         stat(fepcb->fname.enusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.entime, p);
      }
   }
   else
   {
      free(fepcb->mi.enusrmi);
   }

   fseek(fepcb->fd.enusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */


   memset(fepcb->mi.enusrmi, NULL, EN_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.enusrmi, EN_USR_MI_LEN, sizeof(MI),
      fepcb->fd.enusrfd) == 0 && ferror(fepcb->fd.enusrfd) != 0 )
      return(ERROR);
   else
   {
      if (EnValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.enusrmi);
          fepcb->fd.enusrfd = NULL;
          memset(fepcb->ctime.entime, NULL, strlen(fepcb->ctime.entime));
          return(ERROR);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : EnFillCandidates                                         */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

EnFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[24];         /* Echo Buffer                        */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   unsigned int    cdcount;            /* Code Counter                       */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned short  *code[3];           /* Code Of Radicals                   */
   unsigned short  r_code;             /* Code Of Dictionary Record          */
   int             m;
   int             n;
   int             i;
   int             found;
   unsigned short  com;                /* Code flag                          */
   unsigned short  number;
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             cou;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<24; cnt++ )
      echobuf[cnt] = '\0';   
   for( cnt = 0; cnt < 24 && *echobufptr != '\0'; cnt++)
      echobuf[cnt] = *echobufptr++;
 
                                     /* Compress The Content Of Echo Buffer*/
   com = 0;
   code[0] = (unsigned short *)calloc(3, sizeof(short));
   for( i = 1, cnt=0; echobuf[i] != '\0'; code[++cnt] = (unsigned short *)calloc(3, sizeof(short)))
   {
      for(cdcount = 0; cdcount < 8 && echobuf[i + cdcount] != '\0'; cdcount++) 
      {
         if(cdcount >= 0 && cdcount <= 2)
            code[cnt][0] |= (echobuf[i + cdcount] - 'a' + 1) << (2 - cdcount) * 5;
         if(cdcount >= 3 && cdcount <= 5)
            code[cnt][1] |= (echobuf[i + cdcount] - 'a' + 1) << (5 - cdcount) * 5;
         if(cdcount >= 6 && cdcount < 8)
         {
            code[cnt][2] |= (echobuf[i + cdcount] - 'a' + 1) << (8 - cdcount) * 5;
            if(cdcount == 7 && echobuf[i + cdcount + 1])
            {
               code[cnt][2] |= 0x0001;
               com = 0x0004;
             }
          }
      }
      i += cdcount;
   }


   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.enusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if(echobuf[0] >= 'a' && echobuf[0] <= 'z')
      {                       /* echobuf[0] >= 'a' && echobuf[0] <= 'z' */
          if ((drlength = fepcb->mi.enusrmi[echobuf[0] - 'a'].length) == 0)
          {
             fepcb->ret = NOT_FOUND;
             return;
          }
          else
             droffset = (int)fepcb->mi.enusrmi[echobuf[0] - 'a'].index;
      }
      spcounter = 2;
      fepcb->enstruct.cand = (unsigned char *)malloc(spcounter);
      memset(fepcb->enstruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else                            /* flag = SYS                            */
   {
      fdptr = (FILE *)fepcb->fd.ensysfd;
      fseek(fdptr, 0, SEEK_SET);
      if(echobuf[0] >= 'a' && echobuf[0] <= 'z')
      {                          /* echobuf[0] >= 'a' && echobuf[0] <= 'z' */
          if ((drlength = fepcb->mi.ensysmi[echobuf[0] - 'a'].length) == 0)
          {
             fepcb->ret = NOT_FOUND;
             return;
          }
          else
             droffset = (int)fepcb->mi.ensysmi[echobuf[0] - 'a'].index;
      }
      if( *fepcb->enstruct.cand == '\0' )
         spcounter = 2;
      else
         spcounter = strlen( fepcb->enstruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != 0 )
   {
      fepcb->ret = ERROR;
      return;
   }

   drbuf = (unsigned char *)malloc(drlength);
   if( fread(drbuf, drlength, 1, fdptr) != 1 && ferror(fdptr) != 0)
   {
      free(drbuf);
      fepcb->ret = ERROR;
      return;
   }
   number = cnt?cnt:1;
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;      /* Jan.12 95 Modified By B.S.Tang     */


   while( (drbegin <= drend) && ((drend - drbegin) > 3) ) 
   {
      drmiddle=(drbegin + drend)/2;
      chptr= &drbuf[drmiddle];

      while( (*chptr-- != MASK) && (chptr > drbuf) );
      if( chptr == &drbuf[0] )
          r_code = *(unsigned short *)chptr;
      else
      {
          chptr += 2;
          r_code = *(unsigned short *)chptr;
      }

      for(n = 0, cou = 1; n < number; n++)
      {
         found = 0;
         for(m = 0; m < 3 && ((r_code & 0x8000) == 0); m++)
         {
             if(code[n][m] != r_code)
             {
                 found = cou = 0;
                 break;
             }
             else
             {
                 found = 1;
                 chptr += 2;
                 r_code = *(unsigned short *)chptr;
             }
         }
         if(cou == 0)
             break;
      }

      if(found == 1)
         break;
      if ((r_code & 0x8000) != 0)
          r_code = 0;

      if ( code[n][m] > r_code )
         drbegin = drmiddle;
      if ( code[n][m] < r_code )
         drend = drmiddle;
   }

   if ( found == 1)                /* Candidates Be Found */
   {
      while ( LOOP )              /* Fill Satisfied Cand. to Cand. Buffer */
      {
         if( (*chptr & MASK) == MASK )      /* End Of This Group Or Not */
         {
            *candptr++ = *chptr;
            *candptr = '\0';
            break;               
         }
 
         spcounter += 3;
         fepcb->enstruct.cand = 
            (unsigned char *)realloc(fepcb->enstruct.cand, spcounter);
         candptr = (unsigned char *)fepcb->enstruct.cand;
         for( cnt=2; cnt<(spcounter-3); cnt++, candptr++ );
         *candptr++ = *chptr++;
         *candptr++ = *chptr++;
         *candptr++ = *chptr;

         if( (*chptr & 0x80) == 0 )    /* End Of One Phrase Or Not     */
            fepcb->enstruct.allcandno++;

         *candptr = '\0';
         chptr++;
      }

      free(drbuf);
      fepcb->ret = FOUND_CAND;
      return;
   }
   else
   {
      free(drbuf);
      fepcb->ret = NOT_FOUND;
      return;
    }
}



/***************************************************************************/
/* FUNCTION    : EnFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for English_Chinese candidates      */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

EnFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->enstruct.cand != NULL )
   {
      /****************************/
      /*   initial en structure   */
      /****************************/
      free(fepcb->enstruct.cand);
      fepcb->enstruct.cand=NULL;
      fepcb->enstruct.curptr=NULL;
      fepcb->enstruct.allcandno=0;
      fepcb->enstruct.more=0;

   }
}

/**************************************************************************/
/* FUNCTION    : TjUsrDictFileChange                                      */
/* DESCRIPTION : Change Tsang Jye user dictionary file whether it change  */
/*               or not, if it change, reload master index of file        */
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
   int TjLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.tjusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.tjtime, p) )
      return(FALSE);      /*   file had not been changed   */
   else
   {                      /*   file had been changed       */
      if ( TjLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.tjtime, p);
         return(TRUE);
      }
   }
}


/******************************************************************************/
/* FUNCTION    : TjValidDict                                                  */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Tsang Jye user dictionary                       */
/*               FALSE = invalid Tsang Jye user dictionary                    */
/******************************************************************************/

TjValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *tjdictinfo;
   char *p;
   extern char *ctime();
   int cnt, Index, length;
   unsigned char *addr, *buf;
   unsigned int code;
   unsigned short word;

   stat(fepcb->fname.tjusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.tjusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.tjusrfd, 0, 0);
   tjdictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(tjdictinfo, sizeof(struct dictinfo), 1, fepcb->fd.tjusrfd) == 0 
           && ferror(fepcb->fd.tjusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(tjdictinfo->d_mtime)), p) )
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 25)&&(cnt < 5); Index++ ) 
         {
            if ((length = fepcb->mi.tjusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.tjusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.tjusrfd, (long)addr, 0) != 0 )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.tjusrfd) == 0 
                    && ferror(fepcb->fd.tjusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }
            code = *(unsigned int *)buf;
            word = *(unsigned short *)(buf + 4);
            if((code & 0x80000000) == 0 && (word & 0x8000))
            {
               cnt++; 
               continue;
            }
            else 
               return(FALSE);
         }
      }
   }
   return(TRUE);
}

/**************************************************************************/
/* FUNCTION    : TjLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Tsang Jye user file                 */
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
   char *p;
   extern char *ctime();

   if ( fepcb->fd.tjusrfd == NULL )
   {
      if ( (fepcb->fd.tjusrfd=fopen(fepcb->fname.tjusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial zhed  */
         stat(fepcb->fname.tjusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.tjtime, p);
      }
   }
   else
   {
      free(fepcb->mi.tjusrmi);
   }

   fseek(fepcb->fd.tjusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */

   memset(fepcb->mi.tjusrmi, NULL, TJ_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.tjusrmi, TJ_USR_MI_LEN, sizeof(MI),
      fepcb->fd.tjusrfd) == 0 && ferror(fepcb->fd.tjusrfd) != 0 )
      return(ERROR);
   else
   {
      if (TjValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.tjusrmi);
          fepcb->fd.tjusrfd = NULL;
          memset(fepcb->ctime.tjtime, NULL, strlen(fepcb->ctime.tjtime));
          return(ERROR);
      }
   }
}



/**************************************************************************/
/* FUNCTION    : TjLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Tsang Jye system file               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.tjsysfd=fopen(fepcb->fname.tjsysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.tjsysmi, NULL, TJ_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.tjsysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.tjsysmi, TJ_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.tjsysfd) == 0 && ferror(fepcb->fd.tjsysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}

/**************************************************************************/
/* FUNCTION    : TjFillSysCandidates                                      */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjFillSysCandidates( fepcb )
   FEPCB           *fepcb;             /* FEP Control Block                  */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[8];         /* Echo Buffer                        */
   unsigned char   *miptr;             /* Pointer To Master Index            */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code;               /* Code Of Radicals                   */
   unsigned short  a_code;             /* A Code/Word Of Dictionary          */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             allcandno;          /* Number of Candidates               */
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             found = NOT_FOUND;

   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<8; cnt++ )  echobuf[cnt] = '\0';

                                       /* Compress The Content Of Echo Buffer*/
   code = 0;
   for( cnt=0; cnt<fepcb->echoacsz; echobufptr ++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      if ( cnt >= 1 ) 
         code |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
   }

   fdptr = (FILE *)fepcb->fd.tjsysfd;
   fseek(fdptr, 0, SEEK_SET);
   if ((drlength = fepcb->mi.tjsysmi[echobuf[0] - 'a'].length) == 0)
   {
      fepcb->ret = NOT_FOUND;
      return;
   }

   droffset = (int)fepcb->mi.tjsysmi[echobuf[0] - 'a'].index;
   if( *fepcb->tjstruct.cand == '\0' )
       spcounter = 1;
   else
       spcounter = strlen( fepcb->tjstruct.cand) + 1;

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != 0 )
   {
      fepcb->ret = ERROR;
      return;
   }

   drbuf = (unsigned char *)malloc(drlength);
   if( fread(drbuf, drlength, 1, fdptr) != 1 && ferror(fdptr) != 0)
   {
      free(drbuf);
      fepcb->ret = ERROR;
      return;
   }
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;      /* Jan.12 95 Modified By B.S.Tang     */

   while( (drbegin <= drend) && ((drend - drbegin) != 7) ) 
   {
      if ( code == *(unsigned int *)drbuf )
      {
         chptr = &drbuf[4];
         found = FOUND;
         break;
      }  
      drmiddle=(drbegin + drend)/2;
      drmiddle= (( drmiddle / 7) * 7 );
      chptr= &drbuf[drmiddle];

      if ( (*chptr & 0x80) == 0 )
            r_code = *(unsigned int *)chptr;
      if ( code > r_code )
      {
         drbegin = drmiddle;
         continue;
      }
      if ( code < r_code )
      {
         drend = drmiddle;
         continue;
      }

      if ( code == r_code)                /* Candidates Be Found */
      {
         chptr += 4;
         found = FOUND;
         break;
      }
   }
   if ( found == FOUND )
   {
         candptr = chptr;
         while ( *chptr & 0x80 )
            chptr ++;
         allcandno = (chptr - candptr) / 3;
         fepcb->tjstruct.cand = 
               (unsigned char *)realloc(fepcb->tjstruct.cand, (spcounter + (chptr - candptr)));
         chptr = (unsigned char *)fepcb->tjstruct.cand;
         for(cnt = 0; cnt < (spcounter - 1); chptr++, cnt++);

         strncpy(chptr, candptr, allcandno * 3);
         *(chptr + allcandno * 3) = '\0';
         fepcb->tjstruct.allcandno += allcandno;
         free(drbuf);
         fepcb->ret = FOUND_CAND;
         return;
   }
   free(drbuf);
   fepcb->ret = NOT_FOUND;
   return;
}


/**************************************************************************/
/* FUNCTION    : TjFillUserCandidates                                     */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

TjFillUserCandidates( fepcb )
   FEPCB           *fepcb;             /* FEP Control Block                  */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[8];         /* Echo Buffer                        */
   unsigned char   *miptr;             /* Pointer To Master Index            */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code;               /* Code Of Radicals                   */
   unsigned short  a_code;             /* A Code/Word Of Dictionary          */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             allcandno;          /* Number of Candidates               */
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             found = NOT_FOUND;

   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<8; cnt++ )  echobuf[cnt] = '\0';

                                       /* Compress The Content Of Echo Buffer*/
   code = 0;
   for( cnt=0; cnt<fepcb->echoacsz; echobufptr ++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      if ( cnt >= 1 ) 
         code |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
   }

   fdptr = (FILE *)fepcb->fd.tjusrfd;
   fseek(fdptr, 0, SEEK_SET);
   if ((drlength = fepcb->mi.tjusrmi[echobuf[0] - 'a'].length) == 0)
   {
      fepcb->ret = NOT_FOUND;
      return;
   }
   droffset = (int)fepcb->mi.tjusrmi[echobuf[0] - 'a'].index;
   fepcb->tjstruct.cand = (unsigned char *)malloc(spcounter=1);
   memset(fepcb->tjstruct.cand, NULL, 1);
   fepcb->ret = NULL;

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != 0 )
   {
      fepcb->ret = ERROR;
      return;
   }

   drbuf = (unsigned char *)malloc(drlength);
   if( fread(drbuf, drlength, 1, fdptr) != 1 && ferror(fdptr) != 0)
   {
      free(drbuf);
      fepcb->ret = ERROR;
      return;
   }
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;      /* Jan.12 95 Modified By B.S.Tang     */

   while( (drbegin <= drend) && ((drend - drbegin) != 2) ) 
   {
      if ( code == *(unsigned int *)drbuf )
      {
         chptr = &drbuf[4];
         found = FOUND;
         break;
      }  
      drmiddle=(drbegin + drend)/2;
      drmiddle= drmiddle & 0xFFFFFFFE;
      chptr= &drbuf[drmiddle];

      if ( (*chptr & 0x80) != 0 )
      {
         while ( (*chptr++ & 0x80) && (chptr < &drbuf[drlength]) );
         if ( chptr == &drbuf[drlength] )
         {
            drend = drmiddle;
            continue;
         }
         --chptr;
      }
      if ( (*(chptr -3 ) & 0xe0) == 0xe0 )
         r_code = *(unsigned int *)chptr;
      else 
      {
         if ( (*(chptr - 3) & 0x80) == 0 )
         {
            r_code = *(unsigned int *)(chptr - 3);
            chptr -= 3;
         }
         else
         { 
            for ( cnt = 2; (cnt >= 0) && (*(chptr - cnt)&0x80); cnt--);
            r_code = *(unsigned int *)(chptr - cnt);
            chptr -= cnt;
         }
      }  

      if ( code > r_code )
      {
         drbegin = drmiddle;
         continue;
      }
      if ( code < r_code )
      {
         drend = drmiddle;
         continue;
      }

      if ( code == r_code)                /* Candidates Be Found */
      {
         chptr += 4;
         found = FOUND;
         break;
      }
   }
   if ( found == FOUND )
   {
         candptr = chptr;
         while ( (*chptr & 0x80) && (chptr < &drbuf[drlength]) )
            chptr ++;
         allcandno = (chptr - candptr) / 3;
         fepcb->tjstruct.cand = 
               (unsigned char *)realloc(fepcb->tjstruct.cand, (spcounter + (chptr - candptr)));
         chptr = (unsigned char *)fepcb->tjstruct.cand;
         for(cnt = 0; cnt < (spcounter - 1); chptr++, cnt++);

         strncpy(chptr, candptr, allcandno * 3);
         *(chptr + allcandno * 3) = '\0';
         fepcb->tjstruct.allcandno += allcandno;
         free(drbuf);
         fepcb->ret = FOUND_CAND;
         return;
   }
   free(drbuf);
   fepcb->ret = NOT_FOUND;
   return;
}

/***************************************************************************/
/* FUNCTION    : TjFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for Pinyin candidates               */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

TjFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->tjstruct.cand != NULL )
   {
      /****************************/
      /*   initial tj structure   */
      /****************************/
      free(fepcb->tjstruct.cand);
      fepcb->tjstruct.cand=NULL;
      fepcb->tjstruct.curptr=NULL;
      fepcb->tjstruct.allcandno=0;
      fepcb->tjstruct.more=0;

   }
}
