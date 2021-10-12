static char sccsid[] = "@(#)12	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedfssacc.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:22";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: FssAccessDictionary
 *		FssFillCandidates
 *		FssFreeCandidates
 *		FssJmFillCandidates
 *		FssJmLoadSysFileMI
 *		FssLoadSysFileMI
 *		FssLoadUsrFileMI
 *		FssUsrDictFileChange
 *		FssValidDict
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
/* MODULE NAME:        CNedFssAcc                                          */
/*                                                                         */
/* DESCRIPTIVE NAME:   Chinese Input Method Access Dictionary              */
/*                                                                         */
/* FUNCTION:           FssAccessDictionary  : Access File                   */
/*                                                                         */
/*                     FssUsrDictFileChange : Check FSS User File Change   */
/*                                                                         */
/*                     FssCompareRadicals   : Compare FSS Radicals         */
/*                                                                         */
/*                     FssLoadSysFileMI     : Load MI of FSS System File   */
/*                                                                         */
/*                     FssLoadUsrFileMI     : Load MI of FSS User File     */
/*                                                                         */
/*                     FssFillCandidates    : Find FSS System File Cand.   */
/*                                           Find FSS User File Cand.      */
/*                                                                         */
/*                     FssFreeCandidates    : Free FSS Candidates          */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cnedinit.h"
#include "cned.h"
#include "cnedacc.h"
#include "dictionary.h"

int FssAccessDictionary();
int FssUsrDictFileChange();
int FssLoadSysFileMI();
int FssLoadUsrFileMI();
int FssJmLoadSysFileMI();
int FssFillCandidates();
int FssFreeCandidates();
int FssValidDict();               

unsigned char FSSJM[] = "工了以在有地一上不是中国同民为这我的要和产发人经主";


/**************************************************************************/
/* FUNCTION    : FssAccessDictionary                                      */
/* DESCRIPTION : Fill word for Five Stroke Style Input Method, fill       */
/*               candidates for Five Stroke Style Input Method.           */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : fepcb                                                    */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssAccessDictionary(fepcb)
FEPCB *fepcb;
{
   int temp;
   int ret1;
   int ret2;
   int FssUsrDictFileChange();
   int FssCompareRadicals();
   int FssUsrDictFileChange();

   if ( fepcb->imode.ind0 == FIVESTROKE_STYLE_MODE)
   {
         if ( fepcb->fd.fssusrfd != NULL )
         {                           /*  have fss user file  */
            if ( (temp=FssUsrDictFileChange(fepcb)) == FALSE )
                 /* Check the Fss user file change TRUE or FALSE  */
            {
               FssFreeCandidates(fepcb);     /* Free allocated memory for     */
                                             /* Five Stroke Style Candidates  */
               FssFillCandidates(fepcb, USR); /* find fss usr file cand. */
               ret1=fepcb->ret;
               if ( ret1 == ERROR )
               {
                  FssFreeCandidates(fepcb);
                  return;
               }
               else if ( ret1 == NOT_FOUND ) 
                                 /* Not Find FSS user file Candidates    */
                        FssFreeCandidates(fepcb);
               if(fepcb->flag == ON)
                  FssJmFillCandidates(fepcb, SYS);
               else
                  FssFillCandidates(fepcb, SYS); /* find fss sys file cand. */
               ret2=fepcb->ret;
               if ( ret2 == ERROR )
               {
                  FssFreeCandidates(fepcb);
                  return;
               }
               if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FSS user file and sys file Candidates    */
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
                  FssFreeCandidates(fepcb);  /* Free allocated memory for     */
                                             /* Five Stroke Style Candidates  */
                  FssFillCandidates(fepcb, USR); /* find fss usr file cand. */
                  ret1=fepcb->ret;
                  if ( ret1 == ERROR )
                  {
                     FssFreeCandidates(fepcb);
                     return;
                  }
                  else if ( ret1 == NOT_FOUND )
                          FssFreeCandidates(fepcb);
                  if(fepcb->flag == ON)
                     FssJmFillCandidates(fepcb, SYS);
                  else
                     FssFillCandidates(fepcb, SYS);
                  ret2=fepcb->ret;
                  if ( ret2 == ERROR )
                  {
                     FssFreeCandidates(fepcb);
                     return;
                  }
                  if ( ret1 == FOUND_CAND || ret2 == FOUND_CAND )
                      /* Find FSS user file and sys file Candidates    */
                     fepcb->ret=FOUND_CAND;
                  else
                     fepcb->ret=NOT_FOUND;
               }
            }
         }
         else
         {                   /*  no fss usr file , find fss sys file  */
            FssFreeCandidates(fepcb);
            if(fepcb->flag == ON)
               FssJmFillCandidates(fepcb, SYS);
            else
               FssFillCandidates(fepcb, SYS);
            if ( fepcb->ret == ERROR || fepcb->ret == NOT_FOUND )
               FssFreeCandidates(fepcb);
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
/* FUNCTION    : FssUsrDictFileChange                                     */
/* DESCRIPTION : Change Five Stroke Style user dictionary file whether it */
/*               change or not, if it change, reload master index of file */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssUsrDictFileChange(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   int FssLoadUsrFileMi();
   extern char *ctime();

   stat(fepcb->fname.fssusrfname, &info);
   p = ctime(&info.st_mtime);
   if ( !strcmp(fepcb->ctime.fsstime, p) )
      return(FALSE);
   else
   {         /*   file had been changed   */
      if ( FssLoadUsrFileMI(fepcb) == ERROR )
      {
         fepcb->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fepcb->ctime.fsstime, p);
         return(TRUE);
      }
   }
}

/**************************************************************************/
/* FUNCTION    : FssLoadSysFileMI                                         */
/* DESCRIPTION : Load master index of Five Stroke Style system file       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.fsssysfd=fopen(fepcb->fname.fsssysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.fsssysmi, NULL, FSS_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.fsssysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.fsssysmi, FSS_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.fsssysfd) == 0 && ferror(fepcb->fd.fsssysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}


/**************************************************************************/
/* FUNCTION    : FssJmLoadSysFileMI                                       */
/* DESCRIPTION : Load master index of Five Stroke Simple Style system file*/
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssJmLoadSysFileMI(fepcb)
FEPCB *fepcb;
{

   if ( (fepcb->fd.fssjmsysfd=fopen(fepcb->fname.fssjmsysfname, "r")) == NULL )
      return(ERROR);
   memset(fepcb->mi.fssjmsysmi, NULL, FSS_SYS_MI_LEN * sizeof(MI));

   fseek(fepcb->fd.fssjmsysfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */
   if ( fread((char *)fepcb->mi.fssjmsysmi, FSS_SYS_MI_LEN, sizeof(MI),
      fepcb->fd.fssjmsysfd) == 0 && ferror(fepcb->fd.fssjmsysfd) != 0 )
      return(ERROR);
   else
      return(OK);
}

/******************************************************************************/
/* FUNCTION    : FssValidDict                                                 */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : TRUE = valid Five Stroke Style user dictionary               */
/*               FALSE = invalid Five Stroke STile user dictionary            */
/******************************************************************************/

FssValidDict(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                   */
{
   struct stat info;
   struct dictinfo *fssdictinfo;
   char *p;
   extern char *ctime();
   int cnt, i, Index, length;
   unsigned char *addr, *buf, *buf1;
   unsigned short code, word;


   stat(fepcb->fname.fssusrfname, &info);
   p=ctime(&info.st_mtime);
   if ( fepcb->fd.fssusrfd == NULL )
      return(FALSE);
   fseek(fepcb->fd.fssusrfd, 0, 0); 
         /* Access the Fss user dictionary file head */
   fssdictinfo = (struct dictinfo *)malloc(sizeof(struct dictinfo));
   if ( fread(fssdictinfo, sizeof(struct dictinfo), 1, fepcb->fd.fssusrfd) == 0 
           && ferror(fepcb->fd.fssusrfd) != 0 )
      return(FALSE);
   else 
   {
      if( strcmp(ctime(&(fssdictinfo->d_mtime)), p) )
                           /* Check the time of last data modification    */
         return(FALSE);
      else 
      {
         for( cnt = Index = 0; (Index < 25) && (cnt < 5); Index++ ) 
               /* Check the user files has valid dictionary format or not  */ 
         {
            if ((length = fepcb->mi.fssusrmi[Index].length) == 0)
               continue;
            addr = fepcb->mi.fssusrmi[Index].index;
            buf = (unsigned char *)malloc(length);
            if( fseek(fepcb->fd.fssusrfd, (long)addr, 0) != NULL )
            {
               free(buf);
               return(FALSE);
            }
            if(fread(buf, length, 1, fepcb->fd.fssusrfd) == 0 
                    && ferror(fepcb->fd.fssusrfd) != 0)
            {
               free(buf);
               return(FALSE);
            }

        /* Check the Five Stroke Style User file code and word has valid format or not */
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
/* FUNCTION    : FssLoadUsrFileMI                                         */
/* DESCRIPTION : Load master index of Five Stroke Style user file         */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssLoadUsrFileMI(fepcb)
FEPCB *fepcb;
{
   struct stat info;
   char *p;
   extern char *ctime();

   if ( fepcb->fd.fssusrfd == NULL )
   {
      if ( (fepcb->fd.fssusrfd=fopen(fepcb->fname.fssusrfname, "r")) == NULL )
         return(ERROR);
      else
      {    /*  load time to fepcb when input method initial cned  */
         stat(fepcb->fname.fssusrfname, &info);
         p=ctime(&info.st_mtime);
         strcpy(fepcb->ctime.fsstime, p);
      }
   }
   else
   {
      free(fepcb->mi.fssusrmi);
   }

   fseek(fepcb->fd.fssusrfd, sizeof(struct dictinfo), 0);   
                                             /* fd point head of Index table */


   memset(fepcb->mi.fssusrmi, NULL, FSS_USR_MI_LEN * sizeof(MI));
   if ( fread((char *)fepcb->mi.fssusrmi, FSS_USR_MI_LEN, sizeof(MI),
      fepcb->fd.fssusrfd) == 0 && ferror(fepcb->fd.fssusrfd) != 0 )
      return(ERROR);
   else
   {
      if (FssValidDict(fepcb) == TRUE) 
          return(OK);
      else
      {
          free(fepcb->mi.fssusrmi);
          fepcb->fd.fssusrfd = NULL;
          memset(fepcb->ctime.fsstime, NULL, strlen(fepcb->ctime.fsstime));
          return(ERROR);
      }
   }
}


/**************************************************************************/
/* FUNCTION    : FssFillCandidates                                        */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[4];         /* Echo Buffer                        */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code;               /* Code Of Radicals                   */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   int             found = 0;
   int             drbegin;            
   int             drmiddle;
   int             drend;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<4; cnt++ )
      echobuf[cnt] = NULL;   
 
   /***********************************************************************/
   /* Calculate Five Stroke Style dictionary file radical code            */
   /***********************************************************************/
   code = 0;
   for( cnt=0; cnt<fepcb->echoacsz; echobufptr++, cnt++ )
   {
      echobuf[cnt] = *echobufptr;
      code |= 0xff000000;
      if ( cnt >= 1 )
         code |= (echobuf[cnt] - 'a' + 1) << ( 3 - cnt ) * 5;
   }



   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/
   if( flag == USR )
   {
      fdptr = (FILE *)fepcb->fd.fssusrfd;
      fseek(fdptr, 0, SEEK_SET);
      if ((drlength = fepcb->mi.fssusrmi[echobuf[0] - 'a'].length) == 0)
      {
           fepcb->ret = NOT_FOUND;
           return;
      }
      else
             droffset = (int)fepcb->mi.fssusrmi[echobuf[0] - 'a'].index;
      spcounter = 1;
      fepcb->fssstruct.cand = (unsigned char *)malloc(spcounter);
      memset(fepcb->fssstruct.cand, NULL, 1);
      fepcb->ret = NULL;
   }
   else                            /* flag = SYS                            */
   {
      fdptr = (FILE *)fepcb->fd.fsssysfd;
      fseek(fdptr, 0, SEEK_SET);
                               /* echobuf[0] >= 'a' && echobuf[0] <= 'y' */
      if ((drlength = fepcb->mi.fsssysmi[echobuf[0] - 'a'].length) == 0)
      {
          fepcb->ret = NOT_FOUND;
          return;
      }
      else
          droffset = (int)fepcb->mi.fsssysmi[echobuf[0] - 'a'].index;
      if( *fepcb->fssstruct.cand == NULL )
         spcounter = 1;
      else
         spcounter = strlen( fepcb->fssstruct.cand) + 1;
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

   while( (drbegin <= drend) && ((drend - drbegin) > 2) ) 
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
         if ( chptr <= (drbuf + drlength - 1) )
            r_code = *(unsigned int *)chptr;
         else
            break;

         if( (r_code & 0xff000000) == 0xff000000 )/* End Of This Group Or Not */
            break;               
 
         spcounter += 2;
         fepcb->fssstruct.cand = 
            (unsigned char *)realloc(fepcb->fssstruct.cand, spcounter);
         candptr = (unsigned char *)fepcb->fssstruct.cand;
         for( cnt=1; cnt<(spcounter-2); cnt++, candptr++ );
         *candptr++ = *chptr++;
         *candptr++ = *chptr;

         if( (*chptr & 0x80) == NULL )    /* End Of One Phrase Or Not     */
            fepcb->fssstruct.allcandno++;

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



/**************************************************************************/
/* FUNCTION    : FssJmFillCandidates                                      */
/* DESCRIPTION : Search satisfied candidates and fill them to buffer.     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb = FEP control block                                */
/*               flag = USR or SYS                                        */
/* OUTPUT      : fepcb = FEP control block                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

FssJmFillCandidates( fepcb, flag )
   FEPCB           *fepcb;             /* FEP Control Block                  */
   int             flag;               /* User Or System                     */
{
   unsigned char   *echobufptr;        /* Pointer To Echo Buffer             */
   unsigned char   echobuf[4];         /* Echo Buffer                        */
   int             droffset;           /* Offset Of A Dictionary Record      */
   int             drlength;           /* Length Of A Dictionary Record      */
   unsigned char   *candptr;           /* Pointer To Candidate               */
   FILE            *fdptr;             /* Pointer To Dictionary File         */
   unsigned char   *drbuf;             /* DR Buffer                          */
   unsigned char   *chptr;             /* Character Pointer                  */
   unsigned char   *chword;            /* Character Pointer                  */
   short           cnt;                /* Loop Counter                       */
   int             cnt1;               /* Loop Counter                       */
   int             cnt2;               /* Loop Counter                       */
   unsigned int    spcounter;          /* Space Counter                      */
   unsigned int    code;               /* Code Of Radicals                   */
   unsigned int    MaskCode;           /* Mask Code Of Radicals              */
   unsigned int    EndCode;            /* End Code Of Radicals               */
   unsigned int    r_code;             /* Code Of Dictionary Record          */
   unsigned int    a_code;             /* Code Of Dictionary Record          */
   unsigned int    w_code;             /* Code Of Dictionary Record          */
   int             found = 0, z_flag = 0, indexnumber = 0, index = 0, len;
   int             drbegin;            
   int             drmiddle;
   int             drend;


   echobufptr = (unsigned char *)fepcb->echobufs;
   for( cnt=0; cnt<4; cnt++ )
      echobuf[cnt] = NULL;   
 
   echobuf[0] = *echobufptr;
   if(fepcb->echoacsz == 1)
   {
      chword = &FSSJM[0];
      if(echobuf[0] == 'z' )
      {
         fepcb->fssstruct.cand =
              (unsigned char *)realloc(fepcb->fssstruct.cand, 101);
         candptr = (unsigned char *)fepcb->fssstruct.cand;
         fepcb->fssstruct.allcandno = 25;

         for(cnt = 0; cnt < 25; cnt++)
         {
             *candptr++ = *chword++;
             *candptr++ = *chword++;
             *candptr++ = ' ';
             *candptr++ = 0x61 + cnt;
         }
         *candptr = '\0';
         fepcb->ret = FOUND_CAND;
         return;
      }
      else
      {
         fepcb->fssstruct.cand =
             (unsigned char *)realloc(fepcb->fssstruct.cand, 2);
         candptr = (unsigned char *)fepcb->fssstruct.cand;
         *candptr++ = chword[(echobuf[0] - 'a')*2];
         *candptr++ = chword[(echobuf[0] - 'a')*2+1];
         fepcb->fssstruct.allcandno = 1;
         fepcb->ret = FOUND_CAND;
         return;
      }

   }

   /***********************************************************************/
   /* Calculate Five Stroke Style dictionary file radical code            */
   /***********************************************************************/
   code = 0;
   MaskCode = 0;
   EndCode = 0;
   for( cnt = 1; cnt < fepcb->echoacsz; cnt++ )
   {
       echobufptr++;
       echobuf[cnt] = *echobufptr;
       if (echobuf[cnt] == 'z')
       {
           code |= 0 << (6 - cnt) * 5;
           MaskCode |= 0 << (6 - cnt) * 5;
           z_flag = 1;
       }
       else
       {
           code |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
           MaskCode |= 0x1F << (6 - cnt) * 5;
       }
       EndCode |= (echobuf[cnt] - 'a' + 1) << ( 6 - cnt ) * 5;
   }


   /*********************************************************************/
   /* Check the current dictionary file is User file or Sys file        */
   /*********************************************************************/
   fdptr = (FILE *)fepcb->fd.fssjmsysfd;
   fseek(fdptr, 0, SEEK_SET);
                             /* echobuf[0] >= 'a' && echobuf[0] <= 'y' */
   if(echobuf[0] == 'z')
      indexnumber = 25;
   else
      indexnumber = 1;
   for(cnt = 0; cnt < indexnumber; cnt++)
   {
       if(indexnumber == 1)
          index = echobuf[0] - 'a';
       else
          index = cnt;

       if (((drlength = fepcb->mi.fssjmsysmi[index].length) == 0) && (indexnumber == 1))
       {
           fepcb->ret = NOT_FOUND;
           return;
       }
       else
           droffset = (int)fepcb->mi.fssjmsysmi[index].index;
       if( *fepcb->fssstruct.cand == NULL )
           spcounter = 1;
       else
           spcounter = strlen( fepcb->fssstruct.cand) + 1;

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

       chptr=&drbuf[0];
       for(len = 0; len < drlength;)
       {
            r_code = *(unsigned int *)chptr;
            a_code = r_code&MaskCode;
            if ( a_code > EndCode )
                break;
            if(a_code == code)
            {
                switch(fepcb->echoacsz)
                {
                case 2:
                    if ( r_code & 0x40 )
                    {
                        chptr = chptr + 4 + (r_code & 3) * 2;
                        len = len + 4 + (r_code & 3) * 2;
                        spcounter += 5;
                        fepcb->fssstruct.cand =
                           (unsigned char *)realloc(fepcb->fssstruct.cand, spcounter+1);
                        candptr = (unsigned char *)fepcb->fssstruct.cand;
                        for( cnt1=1; cnt1<(spcounter-5); cnt1++, candptr++ );
                        *candptr++ = *chptr++;
                        *candptr++ = *chptr++;
                        for(len+=2;(*(unsigned short *)chptr)&0x8000;chptr+=2,len+=2);
                        *candptr++ = ' ';
                        if (echobuf[0] == 'z')
                            *candptr++ = cnt + 'a';
                        else
                            *candptr++ = echobuf[0];
                        w_code = (r_code >> 25) & 0x1F;
                        *candptr++ = w_code + 0x60;
                        *candptr = '\0';
                        fepcb->fssstruct.allcandno ++;
                        found = 1;
                    }
                    else
                        for(chptr+=4,len+=4; (*(unsigned short *)chptr)&0x8000; chptr+=2, len+=2);
                    break;
                case 3:
                    if ( r_code & 4 )
                    {
                        if ( r_code & 8 )
                            for ( chptr += 4, len += 4; (*(unsigned short *)chptr)&0x8000; )
                            {
                                spcounter += 6;
                                fepcb->fssstruct.cand =
                               (unsigned char *)realloc(fepcb->fssstruct.cand, spcounter+1);
                                candptr = (unsigned char *)fepcb->fssstruct.cand;
                                for( cnt1=1; cnt1<(spcounter-6); cnt1++, candptr++ );
                                 *candptr++ = *chptr++;
                                 *candptr++ = *chptr++;
                                 len += 2;
                                 *candptr++ = ' ';
                                 if (echobuf[0] == 'z')
                                     *candptr++ = cnt + 'a';
                                 else
                                     *candptr++ = echobuf[0];
                                 for(cnt1=0; ((w_code = (r_code >> (5-cnt1) * 5) & 0x1F) != 0)&&(cnt1<2); cnt1++)
                                     *candptr++ = w_code + 0x60;
                                 fepcb->fssstruct.allcandno ++;
                                 *candptr = '\0';
                                 found = 1;
                            }
                        else
                        {
                            chptr = chptr + 4 + (r_code & 3) * 2;
                            len = len + 4 + (r_code & 3) * 2;
                            spcounter += 6;
                            fepcb->fssstruct.cand =
                             (unsigned char *)realloc(fepcb->fssstruct.cand, spcounter+1);
                            candptr = (unsigned char *)fepcb->fssstruct.cand;
                            for( cnt1=1; cnt1<(spcounter-6); cnt1++, candptr++);
                            *candptr++ = *chptr++;
                            *candptr++ = *chptr++;
                            for(len+=2; (*(unsigned short *)chptr)&0x8000; chptr+=2,len+=2);
                            *candptr++ = ' ';
                            if (echobuf[0] == 'z')
                                *candptr++ = cnt + 'a';
                            else
                                *candptr++ = echobuf[0];
                            for(cnt1=0; ((w_code = (r_code >> (5-cnt1) * 5) & 0x1F) != 0)&&(cnt1<2); cnt1++)
                            *candptr++ = w_code + 0x60;
                            *candptr = '\0';
                            fepcb->fssstruct.allcandno ++;
                            found = 1;
                        }
                    }
                    else
                        for(chptr+=4,len+=4; (*(unsigned short *)chptr)&0x8000; chptr+=2, len+=2);
                    break;
                case 4:
                    for ( chptr += 4, len += 4;((*(unsigned short *)chptr)&0x8000) && ( len < drlength); )
                    {
                       spcounter += 7;
                       fepcb->fssstruct.cand =
                      (unsigned char *)realloc(fepcb->fssstruct.cand, spcounter+1);
                       candptr = (unsigned char *)fepcb->fssstruct.cand;
                       for( cnt1=1; cnt1<(spcounter-7); cnt1++, candptr++);
                       *candptr++ = *chptr++;
                       *candptr++ = *chptr++;
                       len += 2;
                       *candptr++ = ' ';
                       if (echobuf[0] == 'z')
                           *candptr++ = cnt + 'a';
                       else
                           *candptr++ = echobuf[0];
                       for(cnt2=0; ((w_code = (r_code >> (5-cnt2) * 5) & 0x1F) != 0)&&(cnt2<3); cnt2++)
                           *candptr++ = w_code + 0x60;
                       for (; cnt2 < 3; cnt2++)
                           *candptr++ = ' ';
                       *candptr = '\0';
                       fepcb->fssstruct.allcandno ++;
                       found = 1;
                    }
               }
           }
           else
               for(chptr+=4,len+=4; (*(unsigned short *)chptr)&0x8000; chptr+=2, len+=2 );
       }
      free(drbuf);
   }
   if(found == 1)
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



/***************************************************************************/
/* FUNCTION    : FssFreeCandidates                                         */
/* DESCRIPTION : Free allocated memory for Five Stroke Style candidates    */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

FssFreeCandidates(fepcb)
FEPCB *fepcb;
{
   if ( fepcb->fssstruct.cand != NULL )
   {
      /****************************/
      /*   initial fss structure  */
      /****************************/
      free(fepcb->fssstruct.cand);
      fepcb->fssstruct.cand=NULL;
      fepcb->fssstruct.curptr=NULL;
      fepcb->fssstruct.allcandno=0;
      fepcb->fssstruct.more=0;

   }
}
