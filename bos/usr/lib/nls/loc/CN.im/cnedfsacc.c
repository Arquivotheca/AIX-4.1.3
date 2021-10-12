static char sccsid[] = "@(#)10	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedfsacc.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:20";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: FsAccessDictionary
 *		FsFillCandidates
 *		FsFreeCandidates
 *		FsLoadSysFileMI
 *		FsLoadUsrFileMI
 *		FsPhAccessDictionary
 *		FsPhFillCandidates
 *		FsPhLoadSysFileMI
 *		FsPhLoadUsrFileMI
 *		FsPhUsrDictFileChange
 *		FsPhValidDict
 *		FsUsrDictFileChange
 *		FsValidDict
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
/* MODULE NAME:        CNedFsAcc                                           */
/*                                                                         */
/* DESCRIPTIVE NAME:   Chinese Input Method Access Dictionary              */
/*                                                                         */
/* FUNCTION:           FsAccessDictionary  : Access File                   */
/*                                                                         */
/*                     FsUsrDictFileChange : Check FS User File Change     */
/*                                                                         */
/*                     FsCompareRadicals   : Compare FS Radicals           */
/*                                                                         */
/*                     FsLoadSysFileMI     : Load MI of FS System File     */
/*                                                                         */
/*                     FsLoadUsrFileMI     : Load MI of FS User File       */
/*                                                                         */
/*                     FsFillCandidates    : Find FS System File Cand.     */
/*                                           Find FS User File Cand.       */
/*                                                                         */
/*                     FsFreeCandidates    : Free FS Candidates            */
/*                                                                         */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cnedinit.h"
#include "cned.h"
#include "cnedacc.h"
#include "dictionary.h"

#define  ITEMLEN1    1024
#define  WORD        1

int FsAccessDictionary();
int FsUsrDictFileChange();
int FsLoadSysFileMI();
int FsLoadUsrFileMI();
int FsFillCandidates();
int FsValidDict();               
int FsFreeCandidates();
int FsPhAccessDictionary();
int FsPhLoadSysFileMI();
int FsPhLoadUsrFileMI();
int FsPhFillCandidates();
int FsPhUsrDictFileChange();
int FsPhValidDict();               


/**************************************************************************/
/* FUNCTION    : FsAccessDictionary                                       */
/* DESCRIPTION : Fill word for Five Stroke  Input Method, fill            */
/*               candidates for Five Stroke Input Method.                 */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsAccessDictionary(fepcb)
FEPCB *fepcb;
{
   int temp;
   int ret1;
   int ret2;
   int FsUsrDictFileChange();
   int FsCompareRadicals();
   int FsUsrDictFileChange();

   if ( fepcb->imode.ind0 == FIVESTROKE_MODE)
   {
        if ( fepcb->fd.fsusrfd != NULL )
         {                           /*  have fs user file  */
            if ( (temp=FsUsrDictFileChange(fepcb)) == FALSE )
                 /* Check the Fs user file change TRUE or FALSE  */
            {
               FsFreeCandidates(fepcb);     /* Free allocated memory for     */
                                             /* Five Stroke Candidates  */
               FsFillCandidates(fepcb, USR); /* find fs usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  FsFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND ) 
                                 /* Not Find FS user file Candidates    */
                        FsFreeCandidates(fepcb);
               FsFillCandidates(fepcb, SYS); /* find fs sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  FsFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FS user file and sys file Candidates    */
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
                  FsFreeCandidates(fepcb);  /* Free allocated memory for     */
                                             /* Five Stroke Candidates  */
                  FsFillCandidates(fepcb, USR); /* find fs usr file cand. */
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     FsFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          FsFreeCandidates(fepcb);
                  FsFillCandidates(fepcb, SYS);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     FsFreeCandidates(fepcb);
                     return;
                  }
                  if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FS user file and sys file Candidates    */
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no fss usr file , find fss sys file  */
            FsFreeCandidates(fepcb);
            if(fepcb->flag == ON)
               FsPhFillCandidates(fepcb, SYS);
            else
               FsFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               FsFreeCandidates(fepcb);
         }
   }
}

/**************************************************************************/
/* FUNCTION    : FsPhAccessDictionary                                     */
/* DESCRIPTION : Fill word for Five Stroke  Input Method, fill            */
/*               candidates for Five Stroke Input Method.                 */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsPhAccessDictionary(fepcb)
FEPCB *fepcb;
{
   int temp;
   int ret1;
   int ret2;
   int FsPhUsrDictFileChange();
   int FsCompareRadicals();
   int FsPhUsrDictFileChange();

   if ( fepcb->imode.ind0 == FIVESTROKE_MODE)
   {
        if ( fepcb->fd.fsphusrfd != NULL )
         {                           /*  have fs user file  */
            if ( (temp=FsPhUsrDictFileChange(fepcb)) == FALSE )
                 /* Check the Fs user file change TRUE or FALSE  */
            {
               FsFreeCandidates(fepcb);     /* Free allocated memory for     */
                                             /* Five Stroke Candidates  */
               FsPhFillCandidates(fepcb, USR); /* find fs usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  FsFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND ) 
                                 /* Not Find FS user file Candidates    */
                        FsFreeCandidates(fepcb);
               FsPhFillCandidates(fepcb, SYS); /* find fs sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  FsFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FS user file and sys file Candidates    */
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
                  FsFreeCandidates(fepcb);  /* Free allocated memory for     */
                                             /* Five Stroke Candidates  */
                  FsPhFillCandidates(fepcb, USR); /* find fs usr file cand. */
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     FsFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          FsFreeCandidates(fepcb);
                  FsPhFillCandidates(fepcb, SYS);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     FsFreeCandidates(fepcb);
                     return;
                  }
                  if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FS user file and sys file Candidates    */
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no fss usr file , find fss sys file  */
            FsFreeCandidates(fepcb);
            FsPhFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               FsFreeCandidates(fepcb);
         }
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

/**************************************************************************/
/* FUNCTION    : FsUsrDictFileChange                                      */
/* DESCRIPTION : Change Five Stroke user dictionary file whether it       */
/*               change or not, if it change, reload master index of file */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int FsLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.fsusrfname, &info);
   p = ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.fstime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( FsLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.fstime, p);
         return(TRUE);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : FsPhUsrDictFileChange                                    */
/* DESCRIPTION : Change Five Stroke user dictionary file whether it       */
/*               change or not, if it change, reload master index of file */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsPhUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int FsPhLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.fsphusrfname, &info);
   p = ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.fstime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( FsPhLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.fstime, p);
         return(TRUE);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : FsLoadSysFileMI                                          */
/* DESCRIPTION : Load master index of Five Stroke Style system file       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.fssysfd=fopen(fepcb->fname.fssysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.fssysmi, NULL, FS_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.fssysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.fssysmi, FS_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.fssysfd) == 0 && ferror(fepcb->fd.fssysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}


/**************************************************************************/
/* FUNCTION    : FsPhLoadSysFileMI                                        */
/* DESCRIPTION : Load master index of Five Stroke Style system file       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsPhLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.fsphsysfd=fopen(fepcb->fname.fsphsysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.fsphsysmi, NULL, FS_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.fsphsysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.fsphsysmi, FS_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.fsphsysfd) == 0 && ferror(fepcb->fd.fsphsysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}

/******************************************************************************/
/* FUNCTION    : FsValidDict                                                 */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Five Stroke user dictionary                     */
/*               FALSE = invalid Five Stroke STile user dictionary            */
/******************************************************************************/

FsValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *fsdictinfo;
   char *p;
   extern char *ctime();
   int cnt, i, Index, length;
   unsigned char *addr, *buf, *buf1;
   unsigned short code, word;


   stat(fepcb->fname.fsusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.fsusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.fsusrfd, 0, 0); 
         /* Access the Fs user dictionary file head */
   fsdictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(fsdictinfo, sizeof(struct dictinfo), 1, fepcb->fd.fsusrfd) == 0 
           && ferror(fepcb->fd.fsusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(fsdictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 5) && (cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
         {
            if ((length = fepcb->mi.fsusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.fsusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.fsusrfd, (long)addr, 0) != NULL )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.fsusrfd) == 0 
                    && ferror(fepcb->fd.fsusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }

        /* Check the Five Stroke User file code and word has valid format or not */
            code = *(unsigned short *)buf;
            word = *(unsigned short *)(buf + 4);
            if((code & 0xff000000) == 0xff000000 && (word & 0x8000)) 
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

/******************************************************************************/
/* FUNCTION    : FsPhValidDict                                                */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Five Stroke user dictionary                     */
/*               FALSE = invalid Five Stroke STile user dictionary            */
/******************************************************************************/

FsPhValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *fsdictinfo;
   char *p;
   extern char *ctime();
   int cnt, i, Index, length;
   unsigned char *addr, *buf, *buf1;
   unsigned short code, word;


   stat(fepcb->fname.fsphusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.fsphusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.fsphusrfd, 0, 0); 
         /* Access the Fs user dictionary file head */
   fsdictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(fsdictinfo, sizeof(struct dictinfo), 1, fepcb->fd.fsphusrfd) == 0 
           && ferror(fepcb->fd.fsphusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(fsdictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 5) && (cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
         {
            if ((length = fepcb->mi.fsphusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.fsphusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.fsphusrfd, (long)addr, 0) != NULL )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.fsphusrfd) == 0 
                    && ferror(fepcb->fd.fsphusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }

        /* Check the Five Stroke User file code and word has valid format or not */
            code = *(unsigned short *)buf;
            word = *(unsigned short *)(buf + 4);
            if((code & 0xff000000) == 0xff000000 && (word & 0x8000)) 
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
/* FUNCTION    : FsLoadUsrFileMI                                          */
/* DESCRIPTION : Load master index of Five Stroke user file               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.fsusrfd == NULL )
   {
      if ( (fepcb->fd.fsusrfd=fopen(fepcb->fname.fsusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial cned  */
         stat(fepcb->fname.fsusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.fstime, p);
      }
   }
   else
   {
      free(fepcb->mi.fsusrmi);
   }

   fseek(fepcb->fd.fsusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */


   memset(fepcb->mi.fsusrmi, NULL, FS_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.fsusrmi, FS_USR_MI_LEN, sizeof(MI),
      fepcb->fd.fsusrfd) == 0 && ferror(fepcb->fd.fsusrfd) != 0 )
      return(ERROR);
   else
   {
      if (FsValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.fsusrmi);
          fepcb->fd.fsusrfd = NULL;
          memset(fepcb->ctime.fstime, NULL, strlen(fepcb->ctime.fstime));
          return(ERROR);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : FsPhLoadUsrFileMI                                        */
/* DESCRIPTION : Load master index of Five Stroke user file               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsPhLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.fsphusrfd == NULL )
   {
      if ( (fepcb->fd.fsphusrfd=fopen(fepcb->fname.fsphusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial cned  */
         stat(fepcb->fname.fsphusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.fstime, p);
      }
   }
   else
   {
      free(fepcb->mi.fsphusrmi);
   }

   fseek(fepcb->fd.fsphusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */


   memset(fepcb->mi.fsphusrmi, NULL, FS_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.fsphusrmi, FS_USR_MI_LEN, sizeof(MI),
      fepcb->fd.fsphusrfd) == 0 && ferror(fepcb->fd.fsphusrfd) != 0 )
      return(ERROR);
   else
   {
      if (FsPhValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.fsphusrmi);
          fepcb->fd.fsphusrfd = NULL;
          memset(fepcb->ctime.fstime, NULL, strlen(fepcb->ctime.fstime));
          return(ERROR);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : FsFillCandidates                                        */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[5];         /* Echo Buffer                        */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   drbuf1[ITEMLEN1];   /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   short           cnt1;               /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code = 0;           /* Code Of Radicals                   */
   unsigned int    MaskCode = 0;       /* Mask Code Of Radicals              */
   unsigned int    EndCode = 0;        /* End Code Of Radicals               */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             found = 0;
   int             comm = 0;
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             number=0, indexnumber, index = 0;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<5; cnt++ )
      echobuf[cnt] = NULL;   
 
   /***********************************************************************/
   /* Calculate Five Stroke dictionary file radical code                  */
   /***********************************************************************/
   code |= 0xff000000;
   EndCode |= 0xff000000;
   MaskCode |= 0xff000000;
   echobuf[0] = *echobufptr;
   if(echobuf[0] == '6' && fepcb->echoacsz == 1)
   {
      code |= 0 << ( 8 * 3);
      comm = 1;
      EndCode |= 0xffffff;
   }
   else
   {
      for( cnt = 1; cnt < fepcb->echoacsz; cnt++ )
      {
         echobufptr++;
         echobuf[cnt] = *echobufptr;
         if( echobuf[cnt] == '6')
         {
             code |= 0 << ( 8 - cnt ) * 3;
             MaskCode |= 0 << ( 8 - cnt ) * 3;
             comm = 1;
         }
         else
         {
            code |= (echobuf[cnt] - '1' + 1) << ( 8 - cnt ) * 3;
            MaskCode |= 7 << ( 8 - cnt ) * 3;
         }
         EndCode |= (echobuf[cnt] - '1' + 1) << ( 8 - cnt ) * 3;
      }
    }


   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/

   if( echobuf[0] == '6' )
   {
       for( indexnumber = 0; indexnumber < 5; indexnumber++)
       {
            if( flag == USR )
            {
               fdptr = (FILE *)fepcb->fd.fssusrfd;
               fseek(fdptr, 0, SEEK_SET);
               if ((drlength = fepcb->mi.fsusrmi[indexnumber].length) == 0)
               {
                    continue;
                    index ++;
               }
               if(index == 5)
               {
                    fepcb->ret = NOT_FOUND;
                    return;
               }
               else
                      droffset = (int)fepcb->mi.fsusrmi[indexnumber].index;
               spcounter = 1;
               fepcb->fsstruct.cand = (unsigned char *)malloc(spcounter);
               memset(fepcb->fsstruct.cand, NULL, 1);
               fepcb->ret = NULL;
            }
            else                            /* flag = SYS */
            {
               fdptr = (FILE *)fepcb->fd.fssysfd;
               fseek(fdptr, 0, SEEK_SET);
                               /* echobuf[0] >= '1' && echobuf[0] <= '6' */
               if ((drlength = fepcb->mi.fssysmi[indexnumber].length) == 0)
               {
                    continue;
                    index ++;
               }
               if(index == 5)
               {
                   fepcb->ret = NOT_FOUND;
                   return;
               }
               else
                   droffset = (int)fepcb->mi.fssysmi[indexnumber].index;
               if( *fepcb->fsstruct.cand == NULL )
                  spcounter = 1;
               else
                  spcounter = strlen( fepcb->fsstruct.cand) + 1;
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

            if( comm == 1)
            {
                chptr = &drbuf[0];
                for(cnt = number = 0; ( cnt < drlength) && 
                   ( chptr < (drbuf + drlength)); )
                {
                    r_code = *(unsigned int *)chptr;
                    if(( r_code & MaskCode) == code)
                    {
                       chptr = chptr + 4;
                       cnt = cnt + 4;
                       while ( LOOP )
                       {
                          if ( ((*chptr & 0xff) != 0xff) && (chptr < (drbuf + drlength)) )
                                 r_code = *(unsigned int *)chptr;
                          else
                             break;

                          if( (r_code & 0xff000000) == 0xff000000 )
                             break;

                          spcounter += 2;
                          fepcb->fsstruct.cand =
                            (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                          candptr = (unsigned char *)fepcb->fsstruct.cand;
                          for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                          *candptr++ = *chptr++;
                          *candptr++ = *chptr;
                          cnt += 2;

                          /* End Of One Phrase Or Not */
                          if( (*chptr & 0x80) == NULL )
                             fepcb->fsstruct.allcandno++;

                          *candptr = NULL; 
                          chptr++;
                          number = WORD;
                       }

                    }
                    else
                    {
                       chptr = chptr + 2;
                       cnt = cnt + 2;
                    }
                }
                free(drbuf);
            }
            else
            {

  /*************************************************************************/
  /* Adopt Bisect Algorithm to search the code of radical in the dictionary*/
  /* record, Compress The Content Of Echo Buffer                           */
  /*************************************************************************/
                drbegin = 0;
                drend = drlength;

                while( (drbegin <= drend) && ((drend - drbegin) != 2) )
                {
                     drmiddle=(drbegin + drend)/2;
                     drmiddle= drmiddle & 0xFFFFFFFE;
                     chptr= &drbuf[drmiddle];
                     while( (*chptr-- != 0xff) && (chptr > &drbuf[0]) );
                     if( chptr == &drbuf[0] )
                         r_code = *(unsigned int *)chptr;
                     else
                     {
                         chptr += 1;
                         r_code = *(unsigned int *)chptr;
                     }
                     if( code > r_code )
                     {
                         drbegin = drmiddle;
                         continue;
                     }
                     if( code < r_code )
                     {
                         drend = drmiddle;
                         continue;
                     }
                     if( code == r_code )
                     {
                         chptr = chptr + 4;
                         found = 1;
                         break;
                     }

                  }
   /******************************************************************/
   /* Candidates Be Found ,and fill Satisfied Cand. to Cand. Buffer  */
   /******************************************************************/
                  if ( found == 1)
                  {
                     while ( LOOP )
                     {
                        if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                           r_code = *(unsigned int *)chptr;
                        else
                           break;

                        if( (r_code & 0xff000000) == 0xff000000 )
                             break;

                        spcounter += 2;
                        fepcb->fsstruct.cand =
                          (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                        candptr = (unsigned char *)fepcb->fsstruct.cand;
                        for( cnt=1; cnt<(spcounter-2); cnt++, candptr++ );
                        *candptr++ = *chptr++;
                        *candptr++ = *chptr;

                        if( (*chptr & 0x80) == NULL )
                                            /* End Of One Phrase Or Not   */
                           fepcb->fsstruct.allcandno++;

                        *candptr = NULL;
                        chptr++;
                        number = WORD;
                     }
                  }
            }
       }
       if(number == WORD)
       {
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
   else
   {
       if( flag == USR )
       {
          fdptr = (FILE *)fepcb->fd.fsusrfd;
          fseek(fdptr, 0, SEEK_SET);
          if ((drlength = fepcb->mi.fsusrmi[echobuf[0] - '1'].length) == 0)
          {
               fepcb->ret = NOT_FOUND;
               return;
          }
          else
                 droffset = (int)fepcb->mi.fsusrmi[echobuf[0] - '1'].index;
          spcounter = 1;
          fepcb->fsstruct.cand = (unsigned char *)malloc(spcounter);
          memset(fepcb->fsstruct.cand, NULL, 1);
          fepcb->ret = NULL;
       }
       else                    /* flag = SYS                            */
       {
          fdptr = (FILE *)fepcb->fd.fssysfd;
          fseek(fdptr, 0, SEEK_SET);
                               /* echobuf[0] >= '1' && echobuf[0] <= '6' */
          if ((drlength = fepcb->mi.fssysmi[echobuf[0] - '1'].length) == 0)
          {
              fepcb->ret = NOT_FOUND;
              return;
          }
          else
              droffset = (int)fepcb->mi.fssysmi[echobuf[0] - '1'].index;
          if( *fepcb->fsstruct.cand == NULL )
             spcounter = 1;
          else
             spcounter = strlen( fepcb->fsstruct.cand) + 1;
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

       if( comm == 1)
       {
           chptr = &drbuf[0];
           for(cnt = 0; ( cnt < drlength) && ( chptr < &drbuf[drlength]);)
           {
               r_code = *(unsigned int *)chptr;
               if(( r_code & MaskCode) == code)
               {
                  chptr = chptr + 4;
                  cnt = cnt + 4;
                  while ( LOOP )
                  {
                     if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                        r_code = *(unsigned int *)chptr;
                     else
                        break;

                     spcounter += 2;
                     fepcb->fsstruct.cand =
                       (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                     candptr = (unsigned char *)fepcb->fsstruct.cand;
                     for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                     *candptr++ = *chptr++;
                     *candptr++ = *chptr;
                     cnt += 2;

                     if( (*chptr & 0x80) == NULL )/* End Of One Phrase Or Not */
                        fepcb->fsstruct.allcandno++;

                     *candptr = NULL;
                     chptr++;
                  }

                  number = WORD;
               }
               else
               {
                  chptr = chptr + 2;
                  cnt = cnt + 2;
               }
           }
           if(number == WORD)
           {
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
       else
       {

  /*************************************************************************/
  /* Adopt Bisect Algorithm to search the code of radical in the dictionary*/
  /* record, Compress The Content Of Echo Buffer                           */
  /*************************************************************************/
           drbegin = 0;
           drend = drlength;

           while( (drbegin <= drend) && ((drend - drbegin) != 2) ) 
           {
                drmiddle=(drbegin + drend)/2;
                drmiddle= drmiddle & 0xFFFFFFFE;
                chptr= &drbuf[drmiddle];
                while( (*chptr-- != 0xff) && (chptr > &drbuf[0]) );
                if( chptr == &drbuf[0] )
                    r_code = *(unsigned int *)chptr;
                else
                {
                    chptr += 1;
                    r_code = *(unsigned int *)chptr;
                }
                if( code > r_code )
                {
                    drbegin = drmiddle;
                    continue;
                }
                if( code < r_code )
                {
                    drend = drmiddle;
                    continue;
                }
                if( code == r_code )
                {
                    chptr = chptr + 4;
                    found = 1;
                    break;
                }

             }
   /******************************************************************/
   /* Candidates Be Found ,and fill Satisfied Cand. to Cand. Buffer  */
   /******************************************************************/
             if ( found == 1)       
             {
                while ( LOOP )  
                {
                   if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                      r_code = *(unsigned int *)chptr;
                   else
                      break;

                   if( (r_code & 0xff000000) == 0xff000000 )
                       break;               

                   spcounter += 2;
                   fepcb->fsstruct.cand = 
                     (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                   candptr = (unsigned char *)fepcb->fsstruct.cand;
                   for( cnt=1; cnt<(spcounter-2); cnt++, candptr++ );
                   *candptr++ = *chptr++;
                   *candptr++ = *chptr;

                   if( (*chptr & 0x80) == NULL )/* End Of One Phrase Or Not   */
                      fepcb->fsstruct.allcandno++;

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
    }
}


/**************************************************************************/
/* FUNCTION    : FsPhFillCandidates                                       */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FsPhFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[9];         /* Echo Buffer                        */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   drbuf1[ITEMLEN1];   /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   short           cnt1;               /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code = 0;           /* Code Of Radicals                   */
   unsigned int    MaskCode = 0;       /* Mask Code Of Radicals              */
   unsigned int    EndCode = 0;        /* End Code Of Radicals               */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             found = 0;
   int             comm = 0;
   int             drbegin;            
   int             drmiddle;
   int             drend;
   int             number=0, indexnumber, index = 0;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<9; cnt++ )
      echobuf[cnt] = NULL;   
 
   /***********************************************************************/
   /* Calculate Five Stroke dictionary file radical code                  */
   /***********************************************************************/
   code |= 0xff000000;
   EndCode |= 0xff000000;
   MaskCode |= 0xff000000;
   echobufptr ++;
   echobuf[1] = *echobufptr;
   if(echobuf[1] == '6' && fepcb->echoacsz == 2)
   {
      code |= 0 << ( 8 * 3);
      comm = 1;
      EndCode |= 0xffffff;
   }
   else
   {
      for( cnt = 2; cnt < fepcb->echoacsz; cnt++ )
      {
         echobufptr++;
         echobuf[cnt] = *echobufptr;
         if( echobuf[cnt] == '6')
         {
             code |= 0 << ( 10 - cnt ) * 3;
             MaskCode |= 0 << ( 10 - cnt ) * 3;
             comm = 1;
         }
         else
         {
            code |= (echobuf[cnt] - '1' + 1) << ( 10 - cnt ) * 3;
            MaskCode |= 7 << ( 10 - cnt ) * 3;
         }
         EndCode |= (echobuf[cnt] - '1' + 1) << ( 10 - cnt ) * 3;
      }
    }


   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/

   if( echobuf[1] == '6' )
   {
       for( indexnumber = 0; indexnumber < 5; indexnumber++)
       {
            if( flag == USR )
            {
               fdptr = (FILE *)fepcb->fd.fsphusrfd;
               fseek(fdptr, 0, SEEK_SET);
               if ((drlength = fepcb->mi.fsphusrmi[indexnumber].length) == 0)
               {
                    continue;
                    index ++;
               }
               if(index == 5)
               {
                    fepcb->ret = NOT_FOUND;
                    return;
               }
               else
                      droffset = (int)fepcb->mi.fsphusrmi[indexnumber].index;
               spcounter = 1;
               fepcb->fsstruct.cand = (unsigned char *)malloc(spcounter);
               memset(fepcb->fsstruct.cand, NULL, 1);
               fepcb->ret = NULL;
            }
            else                            /* flag = SYS */
            {
               fdptr = (FILE *)fepcb->fd.fsphsysfd;
               fseek(fdptr, 0, SEEK_SET);
                               /* echobuf[1] >= '1' && echobuf[1] <= '6' */
               if ((drlength = fepcb->mi.fsphsysmi[indexnumber].length) == 0)
               {
                    continue;
                    index ++;
               }
               if(index == 5)
               {
                   fepcb->ret = NOT_FOUND;
                   return;
               }
               else
                   droffset = (int)fepcb->mi.fsphsysmi[indexnumber].index;
               if( *fepcb->fsstruct.cand == NULL )
                  spcounter = 1;
               else
                  spcounter = strlen( fepcb->fsstruct.cand) + 1;
            }

            echobufptr = (unsigned char *)echobuf;
            echobufptr++;

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

            if( comm == 1)
            {
                chptr = &drbuf[0];
                for(cnt = number = 0; ( cnt < drlength) && 
                   ( chptr < (drbuf + drlength)); )
                {
                    r_code = *(unsigned int *)chptr;
                    if(( r_code & MaskCode) == code)
                    {
                       chptr = chptr + 4;
                       cnt = cnt + 4;
                       while ( LOOP )
                       {
                          if ( ((*chptr & 0xff) != 0xff) && (chptr < (drbuf + drlength)) )
                                 r_code = *(unsigned int *)chptr;
                          else
                             break;

                          if( (r_code & 0xff000000) == 0xff000000 )
                             break;

                          spcounter += 2;
                          fepcb->fsstruct.cand =
                            (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                          candptr = (unsigned char *)fepcb->fsstruct.cand;
                          for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                          *candptr++ = *chptr++;
                          *candptr++ = *chptr;
                          cnt += 2;

                          /* End Of One Phrase Or Not */
                          if( (*chptr & 0x80) == NULL )
                             fepcb->fsstruct.allcandno++;

                          *candptr = NULL; 
                          chptr++;
                          number = WORD;
                       }

                    }
                    else
                    {
                       chptr = chptr + 2;
                       cnt = cnt + 2;
                    }
                }
                free(drbuf);
            }
            else
            {
                chptr = &drbuf[0];
                for(cnt = 0; ( cnt < drlength) && ( chptr < &drbuf[drlength]);)
                {
                    r_code = *(unsigned int *)chptr;
                    if( r_code  == code)
                    {
                       chptr = chptr + 4;
                       cnt = cnt + 4;
                       while ( LOOP )
                       {
                          if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                             r_code = *(unsigned int *)chptr;
                          else
                             break;

                          spcounter += 2;
                          fepcb->fsstruct.cand =
                            (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                          candptr = (unsigned char *)fepcb->fsstruct.cand;
                          for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                          *candptr++ = *chptr++;
                          *candptr++ = *chptr;
                          cnt += 2;

                          if( (*chptr & 0x80) == NULL )/* End Of One Phrase Or Not */
                             fepcb->fsstruct.allcandno++;

                          *candptr = NULL;
                          chptr++;
                       }

                       number = WORD;
                    }
                    else
                    {
                       chptr = chptr + 2;
                       cnt = cnt + 2;
                    }
                }
                free(drbuf);

            }
       }
       if(number == WORD)
       {
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
   else
   {
       if( flag == USR )
       {
          fdptr = (FILE *)fepcb->fd.fsphusrfd;
          fseek(fdptr, 0, SEEK_SET);
          if ((drlength = fepcb->mi.fsphusrmi[echobuf[1] - '1'].length) == 0)
          {
               fepcb->ret = NOT_FOUND;
               return;
          }
          else
                 droffset = (int)fepcb->mi.fsphusrmi[echobuf[1] - '1'].index;
          spcounter = 1;
          fepcb->fsstruct.cand = (unsigned char *)malloc(spcounter);
          memset(fepcb->fsstruct.cand, NULL, 1);
          fepcb->ret = NULL;
       }
       else                    /* flag = SYS                            */
       {
          fdptr = (FILE *)fepcb->fd.fsphsysfd;
          fseek(fdptr, 0, SEEK_SET);
                               /* echobuf[1] >= '1' && echobuf[1] <= '6' */
          if ((drlength = fepcb->mi.fsphsysmi[echobuf[1] - '1'].length) == 0)
          {
              fepcb->ret = NOT_FOUND;
              return;
          }
          else
              droffset = (int)fepcb->mi.fsphsysmi[echobuf[1] - '1'].index;
          if( *fepcb->fsstruct.cand == NULL )
             spcounter = 1;
          else
             spcounter = strlen( fepcb->fsstruct.cand) + 1;
       }

       echobufptr = (unsigned char *)echobuf;
       echobufptr ++;

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

       if( comm == 1)
       {
           chptr = &drbuf[0];
           for(cnt = 0; ( cnt < drlength) && ( chptr < &drbuf[drlength]);)
           {
               r_code = *(unsigned int *)chptr;
               if(( r_code & MaskCode) == code)
               {
                  chptr = chptr + 4;
                  cnt = cnt + 4;
                  while ( LOOP )
                  {
                     if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                        r_code = *(unsigned int *)chptr;
                     else
                        break;

                     spcounter += 2;
                     fepcb->fsstruct.cand =
                       (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                     candptr = (unsigned char *)fepcb->fsstruct.cand;
                     for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                     *candptr++ = *chptr++;
                     *candptr++ = *chptr;
                     cnt += 2;

                     if( (*chptr & 0x80) == NULL )/* End Of One Phrase Or Not */
                        fepcb->fsstruct.allcandno++;

                     *candptr = NULL;
                     chptr++;
                  }

                  number = WORD;
               }
               else
               {
                  chptr = chptr + 2;
                  cnt = cnt + 2;
               }
           }
           if(number == WORD)
           {
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
       else
       {
           chptr = &drbuf[0];
           for(cnt = 0; ( cnt < drlength) && ( chptr < &drbuf[drlength]);)
           {
               r_code = *(unsigned int *)chptr;
               if( r_code  == code)
               {
                  chptr = chptr + 4;
                  cnt = cnt + 4;
                  while ( LOOP )
                  {
                     if ( ((*chptr & 0xff) != 0xff) && (chptr < (&drbuf[drlength])) )
                        r_code = *(unsigned int *)chptr;
                     else
                        break;

                     spcounter += 2;
                     fepcb->fsstruct.cand =
                       (unsigned char *)realloc(fepcb->fsstruct.cand, spcounter);
                     candptr = (unsigned char *)fepcb->fsstruct.cand;
                     for( cnt1=1; cnt1<(spcounter-2); cnt1++, candptr++ );
                     *candptr++ = *chptr++;
                     *candptr++ = *chptr;
                     cnt += 2;

                     if( (*chptr & 0x80) == NULL )/* End Of One Phrase Or Not */
                        fepcb->fsstruct.allcandno++;

                     *candptr = NULL;
                     chptr++;
                  }

                  number = WORD;
               }
               else
               {
                  chptr = chptr + 2;
                  cnt = cnt + 2;
               }
           }
           if(number == WORD)
           {
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
    }
}




/***************************************************************************/
/* FUNCTION    : FsFreeCandidates                                          */
/* DESCRIPTION : Free allocated memory for Five Stroke candidates          */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

FsFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->fsstruct.cand != NULL )
   {
      /****************************/
      /*   initial fs structure  */
      /****************************/
      free(fepcb->fsstruct.cand);
      fepcb->fsstruct.cand=NULL;
      fepcb->fsstruct.curptr=NULL;
      fepcb->fsstruct.allcandno=0;
      fepcb->fsstruct.more=0;

   }
}
