static char sccsid[] = "@(#)98	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedabcacc.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:12";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AbcLoadSysFileMI
 *		AbcLoadUsrFile
 *		AbcUsrDictCheckCreate
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
/* MODULE NAME:        CNedAbcAcc                                          */
/*                                                                         */
/* DESCRIPTIVE NAME:   Abc Input Method Access Dictionary                  */
/*                                                                         */
/* FUNCTION:           AccessDictionary      : Access File                 */
/*                                                                         */
/*                     AbcLoadSysFileMI      : Load MI of Abc System File  */
/*                                                                         */
/*                     AbcLoadUsrFile        : Load Abc User File          */
/*                                                                         */
/*                     AbcUsrDictCheckCreate : Check Abc Input Method User */
/*                                             Dictionary Files Exist or   */
/*                                             Not, If no, Creat Them      */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cnedinit.h"
#include "cned.h"
#include "cnedacc.h"
#include "cjk.h"
#include "data.h"
#include "extern.h"
/* #include "abc.h" */
 
/**************************************************************************/
/* FUNCTION    : AbcLoadSysFileMI                                         */
/* DESCRIPTION : Load master index of Abc system file and other tables    */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

AbcLoadSysFileMI(fepcb)
FEPCB *fepcb;
{
     /* Open ABC System Dictionary          */
     if ( (fepcb->fd.abcsysfd[ABCCWD]=fopen(fepcb->fname.abcsysfname[ABCCWD], "r")) == NULL )
        return(ERROR);
     fepcb->mi.abcsysmi=(unsigned char *)calloc(ABC_MI_LEN, sizeof(char));
     memset(fepcb->mi.abcsysmi, NULL, ABC_MI_LEN);

     /* Read Master Index of ABC System Dictionary */
     if ( fread((char *)fepcb->mi.abcsysmi, ABC_MI_LEN, 1, 
          fepcb->fd.abcsysfd[ABCCWD]) == 0 && ferror(fepcb->fd.abcsysfd[ABCCWD]) != 0 )
        return(ERROR);
     memcpy((char *)&ndx.body_start, (char *)fepcb->mi.abcsysmi,ABC_MI_LEN);

     /* Open ABC System OVL Dictionary          */
     if ( (fepcb->fd.abcsysfd[ABCOVL]=fopen(fepcb->fname.abcsysfname[ABCOVL], "r")) == NULL )
        return(ERROR);
     fseek(fepcb->fd.abcsysfd[ABCOVL], PTZ_LIB_START_POINTER, 0);

     cisu = (struct TBF *)calloc(PTZ_LIB_LENGTH, sizeof(char));
     memset(cisu, NULL, PTZ_LIB_LENGTH);

     /* Read morpheme information */
     if ( fread((char *)cisu, PTZ_LIB_LENGTH, 1, 
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     fseek(fepcb->fd.abcsysfd[ABCOVL], PD_START_POINTER, 0);

     /* Read frequency information */
     if ( fread((char *)pindu.pd_bf0, PD_LENGTH, 1, 
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     fseek(fepcb->fd.abcsysfd[ABCOVL], SPBX_START_POINTER, 0);

     /* Read basic stroke information */
     if ( fread((char *)spbx_tab, SPBX_LENGTH, 1, 
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);
 
     return(OK);
}


/**************************************************************************/
/* FUNCTION    : AbcLoadUsrFile                                           */
/* DESCRIPTION : Load master index of Abc user file and peremeters        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

AbcLoadUsrFile(fepcb)
FEPCB *fepcb;
{
   unsigned char   buffer[PAREMETER_LENGTH];
   struct stat     info;
   char            *p;
   int             i;

   if (AbcUsrDictCheckCreate(fepcb) != ERROR)
   {
     if ( (fepcb->fd.abcusrfd[USRREM]=fopen(fepcb->fname.abcusrfname[USRREM], "r+")) == NULL )
          return(ERROR);

     if ( fread(&tmmr, TMMR_REAL_LENGTH, 1, fepcb->fd.abcusrfd[USRREM]) == 0 
            && ferror(fepcb->fd.abcusrfd[USRREM]) != 0 )
        return(ERROR);

                                   /* move the pointer to the paremeter area */
     fseek(fepcb->fd.abcusrfd[USRREM], TMMR_REAL_LENGTH, 0); 
     if ( fread(buffer, PAREMETER_LENGTH, 1, fepcb->fd.abcusrfd[USRREM]) == 0)   
                                   /* read the paremeters to the buffer */
     {
        for (i = 0; i < PAREMETER_LENGTH; i++)
           buffer[i] = 0;

        buffer[0] = IfTopMost;             /* transfer the peremeters */
        buffer[1] = auto_mode;
        buffer[2] = bdd_flag;
        buffer[3] = cbx_flag;

        if ( fwrite(buffer, PAREMETER_LENGTH, 1,
             fepcb->fd.abcusrfd[USRREM]) == 0 )   /* writer the file */
             return(ERROR);
     }
     else
         if ( ferror(fepcb->fd.abcusrfd[USRREM]) != 0 )
            return(ERROR);

     IfTopMost = buffer[0];                 /* transfer the paremeter */
     auto_mode = buffer[1];
     bdd_flag = buffer[2];
     cbx_flag = buffer[3];
                        /* Load time to fepcb when input method initial zhed */
     stat(fepcb->fname.abcusrfname[USRREM], &info);
     p = (unsigned char*)ctime(&info.st_mtime);
     strcpy(fepcb->ctime.abctime[USRREM], p);

     if ( (fepcb->fd.abcusrfd[USR]=fopen(fepcb->fname.abcusrfname[USR], "r+")) == NULL )
          return(ERROR);
     else
     {               /* Load time to fepcb when input method initial zhed */
         stat(fepcb->fname.abcusrfname[USRREM], &info);
         p = (unsigned char*)ctime(&info.st_mtime);
         strcpy(fepcb->ctime.abctime[USRREM], p);
     }

     fepcb->mi.abcusrmi=(unsigned char *)calloc(ABC_MI_LEN, sizeof(char));
     memset(fepcb->mi.abcusrmi, NULL, ABC_MI_LEN);
     fseek(fepcb->fd.abcusrfd[USR], 0xa000l, 0);

     if ( fread((char *)fepcb->mi.abcusrmi, ABC_MI_LEN, 1, 
          fepcb->fd.abcusrfd[USR]) == 0 && ferror(fepcb->fd.abcusrfd[USR]) != 0 )
        return(ERROR);
     memcpy((char *)&kzk_ndx.body_start, (char *)fepcb->mi.abcusrmi,ABC_MI_LEN);

   }
   return(OK);
}


/**************************************************************************/
/* FUNCTION    : AbcUsrDictCheckCreate                                    */
/* DESCRIPTION : Check Abc Input Method User Dictionary Files Exist or Not*/
/*               If no, Create Them                                       */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : fepcb                                                    */
/* OUTPUT      : OK, ERROR                                                */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/

AbcUsrDictCheckCreate(fepcb)
FEPCB *fepcb;
{
      struct INDEX user_file_head;
      struct M_NDX mulu_head={
                 0,
                 0,       /* MULU_START_LOW                目录读写低字节 */
                 0x1800,  /* MULU_LENGTH_MAX   SIZELIB+SIZELIB_KZK   目录最大长度=缓冲池的长度    */
                 0x10,    /* MULU_TRUE_LENGTH  10H                目录的实际长度,开始只有参数.    */
                 0xA,     /* MULU_RECORD_LENGTH  10  目录每条记录的长度。 */
                 0,       /* DATA_START_HI  0  整个用户文件限制在64K之内.  */
                 0x1800,  /* DATA_START_LOW DW SIZE LIB_W      目录区域和数据区互相衔接.          */
                 0x20     /* DATA_RECORD_LENGTH 32 每个记录的长度.  */
      };

      FILE *fp;
      int i,count;
      unsigned char *rem_area, *p;
      unsigned short *pp;

      rem_area = (unsigned char *)calloc(TMMR_LIB_LENGTH+100, sizeof(char));
      memset(rem_area, NULL, TMMR_LIB_LENGTH);

      if ( ( fp = fopen(fepcb->fname.abcusrfname[USRREM], "r") ) != NULL )
      {
         fread(rem_area, TMMR_LIB_LENGTH, 1, fp);
         if ((rem_area[CHECK_POINT]=='T')&& (rem_area[CHECK_POINT+1]=='X')
                                         && (rem_area[CHECK_POINT+2]=='L')
                                         && (rem_area[CHECK_POINT+3]=='N'))
             fclose(fp);
         else
             fclose(fp), fp = NULL;
      }

      if (fp == NULL)
      {
          if ( ( fp = fopen(fepcb->fname.abcusrfname[USRREM], "w") ) == NULL )
             return(ERROR);
          else
          {
             for (i = 0; i < TMMR_LIB_LENGTH; i++)
                 rem_area[i] = 0;                  /* Init the temp rem */
                                                   /* area by zero.*/
             rem_area[CHECK_POINT] = 'T';          /* give Mark! */
             rem_area[CHECK_POINT+1] = 'X';
             rem_area[CHECK_POINT+2] = 'L';
             rem_area[CHECK_POINT+3] = 'N';

             if ( fwrite(rem_area, TMMR_LIB_LENGTH, 1, fp) == 0 && ferror(fp) != 0 )
                 return(ERROR);

             for (i = 0; i < PAREMETER_LENGTH; i++)
                 rem_area[i] = 0;             
	     /* transfer the peremeters */
             rem_area[0] = IfTopMost= 0; 
             rem_area[1] = auto_mode= 1;
             rem_area[2] = bdd_flag= 1;
             rem_area[3] = cbx_flag= 0;

             if ( fwrite(rem_area, PAREMETER_LENGTH, 1, fp) == 0 && ferror(fp) != 0 )
                 return(ERROR);
             fflush(fp);
             fclose(fp);
          }
      }

/*                                                                    */
/* Check or create TMMR.REM file is over. Now, deel with USER.REM.    */
/*                                                                    */
      if ( ( fp = fopen(fepcb->fname.abcusrfname[USR], "r") ) != NULL )
      {
         fseek(fp, LENGTH_OF_USER, 0);
         fread(rem_area, ABC_MI_LEN, 1, fp);

         if ((rem_area[CHECK_POINT2]=='T')&&(rem_area[CHECK_POINT2+1]=='X')
                                         &&(rem_area[CHECK_POINT2+2]=='L')
                                         && (rem_area[CHECK_POINT2+3]=='N'))
             fclose(fp);
         else
             fclose(fp), fp = NULL;
      }

      if (fp == NULL)
      {
          if ( ( fp = fopen(fepcb->fname.abcusrfname[USR], "w") ) == NULL )
             return(ERROR);
          else
          {
             for (i = 0; i < TMMR_LIB_LENGTH; i++)
                 rem_area[i]=0;                  /* Init the temp rem */
                                                 /* area by zero. */
                           /* First, write file para for force remenber. */

              p = (unsigned char *)&mulu_head.mulu_start_hi;
              for (i = 0; i < 16; i++) 
                  rem_area[i] = p[i];

/* Init force rem file */

              for (i = 0; i < LENGTH_OF_USER / 0x1000; i++)
                  if ( fwrite(rem_area, 0x1000, 1, fp) == 0 && ferror(fp) != 0 )
                       return(ERROR);

/* Init user dictionary file */
              p = (unsigned char *)&user_file_head.body_start;
              for (i = 0; i < sizeof(user_file_head); i++) 
                  p[i] = 0;

              user_file_head.body_start = ABC_MI_LEN / 16;
              user_file_head.ttl_length = ABC_MI_LEN / 16;
              user_file_head.body_length = 0;
              user_file_head.index_start = 3;
              user_file_head.index_length = ABC_MI_LEN / 16 - 3;
              user_file_head.unused1 = 0x2000;
              p[CHECK_POINT2] = 'T';               /* give Mark!     */
              p[CHECK_POINT2+1] = 'X';
              p[CHECK_POINT2+2] = 'L';
              p[CHECK_POINT2+3] = 'N';

              if ( fwrite(&user_file_head, ABC_MI_LEN, 1, fp) == 0 && ferror(fp) != 0 )
                   return(ERROR);
              fflush(fp);
              fclose(fp);
          }
      }

    free(rem_area);
    return(OK);
}  

