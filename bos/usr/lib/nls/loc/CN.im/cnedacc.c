static char sccsid[] = "@(#)03	1.3  src/bos/usr/lib/nls/loc/CN.im/cnedacc.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:18";
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
/* MODULE NAME:        CNedAcc                                             */
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
/************************ END OF SPECIFICATION *****************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cnedinit.h"
#include "cned.h"
#include "cnedacc.h"
#include "dictionary.h"

int AccessDictionary();
int PyUsrDictFileChange();
int PyLoadSysFileMI();
int PyLoadUsrFileMI();
int PyFillCandidates();
int PyFreeCandidates();
int PyValidDict();               
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
                 /* Check the EN user file change TRUE or FALSE  */
            {
               EnFreeCandidates(fepcb);      /* Free allocated memory for     */
                                             /* English Chinese Candidates    */
               EnFillCandidates(fepcb, USR); /* find en usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  EnFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND ) 
                                 /* Not Find EN user file Candidates    */
                        EnFreeCandidates(fepcb);
               EnFillCandidates(fepcb, SYS); /* find en sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  EnFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find EN user file and sys file Candidates    */
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
                  EnFreeCandidates(fepcb);   /* Free allocated memory for     */
                                             /* English Chinese Candidates    */
                  EnFillCandidates(fepcb, USR); /* find en usr file cand. */
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
                      /* Find EN user file and sys file Candidates    */
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no en usr file , find en sys file  */
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
                 /* Check the PY user file change TRUE or FALSE  */
            {
               PyFreeCandidates(fepcb);      /* Free allocated memory for */
                                             /* PinYin candidates         */
               PyFillCandidates(fepcb, USR); /* find py usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  PyFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND )
                            /* Not Find PY user file Candidates    */
                        PyFreeCandidates(fepcb);
               PyFillCandidates(fepcb, SYS); /* find py sys file cand. */
               ret2=fepcb->ret;
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
                  PyFillCandidates(fepcb, USR);
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          PyFreeCandidates(fepcb);
                  PyFillCandidates(fepcb, SYS);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     PyFreeCandidates(fepcb);
                     return;
                  }
                  if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find PY user file and sys file Candidates    */
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no py usr file  */
            PyFreeCandidates(fepcb);
            PyFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               PyFreeCandidates(fepcb);
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
                 /* Check the Le user file change TRUE or FALSE  */
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
                      /* Find LE user file and sys file Candidates    */
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
   p=ctime(&info.st_mtime);    /* Creat time               */
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
   p=ctime(&info.st_mtime);         /* creat time         */
   if ( fepcb->fd.pyusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.pyusrfd, 0, 0);    /* Read user dictinoary head structure */
   pydictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(pydictinfo, sizeof(struct dictinfo), 1, fepcb->fd.pyusrfd) == 0 
           && ferror(fepcb->fd.pyusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(pydictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 26)&&(cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
         {
            if ((length = fepcb->mi.pyusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.pyusrmi[Index].index;  /* Py Index address   */
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.pyusrfd, (long)addr, 0) != 0)
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
        /* Check the Pinyin User file code and word has valid format or not */
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
      {    /*  load time to fepcb when input method initial cned  */
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
          /*  Check dictionary has a valid dictionary format               */
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

   if ( (fepcb->fd.pysysfd=fopen(fepcb->fname.pysysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.pysysmi, NULL, PY_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.pysysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.pysysmi, PY_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.pysysfd) == 0 && ferror(fepcb->fd.pysysfd) != 0 )
      return(ERROR);
   else
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

PyFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
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
   int             allcandno;          /* Number of all Cand.                */
   int             drbegin;            
   int             drmiddle;
   int             drend;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<6; cnt++ )  echobuf[cnt] = NULL;

   /***********************************************************************/
   /* Calculate PinYin dictionary file radical code                       */
   /***********************************************************************/
   code = 0;
   for( cnt=0; cnt<fepcb->echoacsz; echobufptr ++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      if ( cnt >= 1 ) 
         code |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
   }

   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/
   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.pyusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.pyusrmi[echobuf[0] - 'a'].length) == 0)
                                      /* Check the dictionary length */
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.pyusrmi[echobuf[0] - 'a'].index;
                                     /* Calculate offset of adictionary racord*/
      fepcb->pystruct.cand = (unsigned char *)malloc(spcounter=1);
      memset(fepcb->pystruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else   /* flag = SYS */
   {
      fdptr = (FILE *)fepcb->fd.pysysfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.pysysmi[echobuf[0] - 'a'].length) == 0)
                                      /* Check the dictionary length */
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.pysysmi[echobuf[0] - 'a'].index;
                                     /* Calculate offset of adictionary racord*/
      if( *fepcb->pystruct.cand == NULL )
         spcounter = 1;
      else
         spcounter = strlen( fepcb->pystruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != NULL )
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

  /*************************************************************************/
  /* Adopt Bisect Algorithm to search the code of radical in the dictionary*/
  /* record, and Compress The Content Of Echo Buffer                       */
  /*************************************************************************/
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;     /* Jan.12 95 Modified By B.S.Tang  */
   while( (drbegin <= drend) && ((drend - drbegin) != 2) ) 
   {
      drmiddle=(drbegin + drend)/2;
      drmiddle= drmiddle & 0xFFFFFFFE;
      chptr= &drbuf[drmiddle];
      do      /* Move the pointer to point the candidates            */
      {
         a_code =*(unsigned short *)chptr;
         chptr -= 2;
      } while ( (a_code & 0x8000) && (chptr > &drbuf[0]) );

      if ( chptr == &drbuf[0] )
      {
         r_code = *(unsigned int *)chptr;
         chptr -= 2;
      }  
      else
      {
         for(a_code=*(unsigned short *)chptr ;((a_code &0x8000 ) == 0) && (chptr > &drbuf[0]); )
         {
            chptr -= 2;
            a_code =*(unsigned short *)chptr;
         }
         r_code = *(unsigned int *)(chptr + 2);
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

     /* Candidates Be Found, and Fill satisfied Candidates to Cand. Buffer */
      if ( code == r_code) 
      {
         chptr += 6;
         candptr = chptr;
         while ( (*chptr & 0x80) && (chptr < &drbuf[drlength]) )
            chptr ++;
         allcandno = (chptr - candptr) / 2;
         fepcb->pystruct.cand = 
               (unsigned char *)realloc(fepcb->pystruct.cand, (spcounter + (chptr - candptr)));
         chptr = (unsigned char *)fepcb->pystruct.cand;
         for(cnt = 0; cnt < (spcounter - 1); chptr++, cnt++);

         strncpy(chptr, candptr, allcandno * 2);
         *(chptr + allcandno * 2) = NULL;
         fepcb->pystruct.allcandno += allcandno;
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
   fseek(fepcb->fd.leusrfd, 0, 0);    /* Read user dictinoary head structure */
   ledictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(ledictinfo, sizeof(struct dictinfo), 1, fepcb->fd.leusrfd) == 0 
           && ferror(fepcb->fd.leusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(ledictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = 0; (Index < 85) && (cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
         {
            if ((length = fepcb->mi.leusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.leusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.leusrfd, (long)addr, 0) != 0)
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
        /* Check the Legend User file code and word has valid format or not */
            code = *(unsigned short *)buf;
            word = *(unsigned short *)(buf + 2);
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
       /* Check user dictionary file */
   {
      if ( (fepcb->fd.leusrfd=fopen(fepcb->fname.leusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial cned  */
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
   unsigned short  code;               /* Code Of Radicals                   */
   unsigned int    a_code;             /* A Code/Word Of Dictionary          */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             drbegin;            
   int             drmiddle;
   int             drend;


   echobufptr = (unsigned char *)fepcb->edendbuf;
   echobuf[0] = NULL;
   echobuf[1] = NULL;

   /***********************************************************************/
   /* Calculate Legend dictionary file radical code                       */
   /***********************************************************************/
   code = 0;
   for( cnt=0; cnt<fepcb->edendacsz; echobufptr++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      if ( cnt >= 1 ) 
         code |= echobuf[cnt] & 0x7F;    /* Calculate the code            */
   }

   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/
   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.leusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.leusrmi[echobuf[0] - 0xb0].length) == 0)
                                      /* Check the dictionary length */
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.leusrmi[echobuf[0] - 0xb0].index;
                                     /* Calculate offset of adictionary racord*/
      fepcb->lestruct.cand = (unsigned char *)malloc(spcounter=1);
      memset(fepcb->lestruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else   /* flag = SYS */
   {
      fdptr = (FILE *)fepcb->fd.lesysfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.lesysmi[echobuf[0] - 0xb0].length) == 0)
      {
         fepcb->ret = NOT_FOUND;
         return;
      }
      droffset = (int)fepcb->mi.lesysmi[echobuf[0] - 0xb0].index;
      if( *fepcb->lestruct.cand == NULL )
         spcounter = 1;
      else
         spcounter = strlen(fepcb->lestruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != NULL )
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

  /*************************************************************************/
  /* Adopt Bisect Algorithm to search the code of radical in the dictionary*/
  /* record, Compress The Content Of Echo Buffer                           */
  /*************************************************************************/
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;

   while( (drbegin <= drend) && ((drend - drbegin) > 2) ) 
   {
      drmiddle=(drbegin + drend)/2;
      chptr= &drbuf[drmiddle];
      while( (chptr > &drbuf[0]) && (*chptr !=0) )
         chptr--;
      if ( chptr == &drbuf[0] )
         r_code = *(unsigned short *)chptr;
      else
         r_code = *(unsigned short *)chptr ;

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
            if( (*chptr == NULL) || (chptr > (drbuf + drlength - 1)) )
               break;                /* End Of This Group Or Not     */

            if( (*chptr & 0x80) == NULL )    /* End Of One Phrase Or Not     */
               fepcb->lestruct.allcandno++;

            fepcb->lestruct.cand = 
               (unsigned char *)realloc(fepcb->lestruct.cand, ++spcounter);
            candptr = (unsigned char *)fepcb->lestruct.cand;
            for( cnt=1; cnt<(spcounter-1); cnt++, candptr++ );
            *candptr++ = *chptr;
            *candptr = NULL;
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
         /* Access the En user dictionary file head */
   endictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(endictinfo, sizeof(struct dictinfo), 1, fepcb->fd.enusrfd) == 0 
           && ferror(fepcb->fd.enusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(endictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 26) && (cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
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

        /* Check the Pinyin User file code and word has valid format or not */
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
      {    /*  load time to fepcb when input method initial cned  */
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
      echobuf[cnt] = NULL;   
   for( cnt = 0; cnt < 24 && *echobufptr != NULL; cnt++)
      echobuf[cnt] = *echobufptr++;
 
   /***********************************************************************/
   /* Calculate English_Chinese dictionary file radical code              */
   /***********************************************************************/
   com = 0;
   code[0] = (unsigned short *)calloc(3, sizeof(short));
   for( i = 1, cnt=0; echobuf[i] != NULL; code[++cnt] = (unsigned short *)calloc(3, sizeof(short)))
   {
      for(cdcount = 0; cdcount < 8 && echobuf[i + cdcount] != NULL; cdcount++) 
      {
         if(echobuf[i + cdcount] >= 'A' && echobuf[i + cdcount] <= 'Z')
            echobuf[i + cdcount] = echobuf[i + cdcount] - 'A' + 'a';
         if(echobuf[i + cdcount] == '\'')
            echobuf[i + cdcount] = 'z' + 1;
         if(echobuf[i + cdcount] == '-')
            echobuf[i + cdcount] = 'z' + 2;
         if(echobuf[i + cdcount] == '.')
            echobuf[i + cdcount] = 'z' + 3;
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


   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/
   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.enusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if(echobuf[0] >= 'A' && echobuf[0] <= 'Z')
      {
         if ((drlength = fepcb->mi.enusrmi[echobuf[0] - 'A'].length) == 0)
         {
            fepcb->ret = NOT_FOUND;
            return;
         }
         else
            droffset = (int)fepcb->mi.enusrmi[echobuf[0] - 'A'].index;
      }
      else                         /* echobuf[0] >= 'a' && echobuf[0] <= 'z' */
      {
          if ((drlength = fepcb->mi.enusrmi[echobuf[0] - 'a'].length) == 0)
          {
             fepcb->ret = NOT_FOUND;
             return;
          }
          else
             droffset = (int)fepcb->mi.enusrmi[echobuf[0] - 'a'].index;
      }
      spcounter = 1;
      fepcb->enstruct.cand = (unsigned char *)malloc(spcounter);
      memset(fepcb->enstruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else                            /* flag = SYS                            */
   {
      fdptr = (FILE *)fepcb->fd.ensysfd;
      fseek(fdptr, 0, SEEK_SET);
      if(echobuf[0] >= 'A' && echobuf[0] <= 'Z')
      {
         if ((drlength = fepcb->mi.ensysmi[echobuf[0] - 'A'].length) == 0)
         {
            fepcb->ret = NOT_FOUND;
            return;
         }
         else
            droffset = (int)fepcb->mi.ensysmi[echobuf[0] - 'A'].index;
      }
      else                         /* echobuf[0] >= 'a' && echobuf[0] <= 'z' */
      {
          if ((drlength = fepcb->mi.ensysmi[echobuf[0] - 'a'].length) == 0)
          {
             fepcb->ret = NOT_FOUND;
             return;
          }
          else
             droffset = (int)fepcb->mi.ensysmi[echobuf[0] - 'a'].index;
      }
      if( *fepcb->enstruct.cand == NULL )
         spcounter = 1;
      else
         spcounter = strlen( fepcb->enstruct.cand) + 1;
   }

   echobufptr = (unsigned char *)echobuf;

   if( fseek(fdptr, droffset, 0) != NULL )
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

  /*************************************************************************/
  /* Adopt Bisect Algorithm to search the code of radical in the dictionary*/
  /* record, Compress The Content Of Echo Buffer                           */
  /*************************************************************************/
   number = cnt?cnt:1;
   drbegin = 0;
   drend = drlength;
   drmiddle = 0;     /* Jan.12 95 Modified By B.S.Tang  */

   while( (drbegin <= drend) && ((drend - drbegin) > 2) ) 
   {
      drmiddle=(drbegin + drend)/2;
      drmiddle= drmiddle & 0xFFFFFFFE;
      chptr= &drbuf[drmiddle];
      do {
         chptr -= 2;
         r_code =*(unsigned short *)chptr;
      } while (r_code & 0x8000);

      while (((r_code & 0x8000) == 0 ) && (chptr >= drbuf))
      {
         chptr -= 2;
         r_code = *(unsigned short *)chptr;
      }
      chptr += 2;
      r_code = *(unsigned short *)chptr;
      
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
          if( cou == 0)
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
   /******************************************************************/
   /* Candidates Be Found ,and fill Satisfied Cand. to Cand. Buffer  */
   /******************************************************************/
   if ( found == 1)       
   {
      while ( LOOP )  
      {
         if ( chptr <= (drbuf + drlength - 1) )
            r_code = *(unsigned short *)chptr;
         else
            break;

         if( (r_code & 0x8000) == NULL )      /* End Of This Group Or Not */
            break;               
 
         spcounter += 2;
         fepcb->enstruct.cand = 
            (unsigned char *)realloc(fepcb->enstruct.cand, spcounter);
         candptr = (unsigned char *)fepcb->enstruct.cand;
         for( cnt=1; cnt<(spcounter-2); cnt++, candptr++ );
         *candptr++ = *chptr++;
         *candptr++ = *chptr;

         if( (*chptr & 0x80) == NULL )    /* End Of One Phrase Or Not     */
            fepcb->enstruct.allcandno++;

         *candptr = NULL;
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
