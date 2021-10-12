static char sccsid[] = "@(#)47	1.3  src/bos/usr/lib/nls/loc/ZH.im/zhedabcacc.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:32:24";
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
/* MODULE NAME:        ZHedAbcAcc                                          */
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
#include <fcntl.h>
#include <errno.h>
#include <sys/mode.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "zhedinit.h"
#include "zhed.h"
#include "zhedacc.h"
#include "cjk.h"
#include "data.h"
#include "extern.h"
/* #include "abc.h" */
/* #include "abcpy.h" */
 

extern struct FILEHEAD; 

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
     /* Open ABC System Dictionary  of Simplified Chinese        */
     if ( (fepcb->fd.abcsysfd[ABCCWD_S]=fopen(fepcb->fname.abcsysfname[ABCCWD_S], "r")) == NULL )
        return(ERROR);
     fepcb->mi.abcsysmi=(unsigned char *)calloc(ABC_MI_LEN, sizeof(char));
     memset(fepcb->mi.abcsysmi, NULL, ABC_MI_LEN);

     /* Read Master Index of ABC System Dictionary */
     if ( fread((char *)fepcb->mi.abcsysmi, ABC_MI_LEN, 1,
          fepcb->fd.abcsysfd[ABCCWD_S]) == 0 && ferror(fepcb->fd.abcsysfd[ABCCWD_S]) != 0 )
        return(ERROR);
     memcpy((char *)&ndx.body_start, (char *)fepcb->mi.abcsysmi,ABC_MI_LEN);


     /* Open ABC System Dictionary of Traditional Chinese          */
     if ( (fepcb->fd.abcsysfd[ABCCWD_T]=fopen(fepcb->fname.abcsysfname[ABCCWD_T], "r")) == NULL )
        return(ERROR);


     /* Open ABC System OVL Dictionary          */
     if ( (fepcb->fd.abcsysfd[ABCOVL]=fopen(fepcb->fname.abcsysfname[ABCOVL], "r")) == NULL )
        return(ERROR);
     if ( fread(&head, sizeof(struct FILEHEAD), 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     cisu.t_bf_start = (WORD *)malloc(16);     /* apply single tone area */
     memset(cisu.t_bf_start, 0, 16);           /* clear single tone area */

     if ( fread((char *)cisu.t_bf_start, 16, 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     cisu.t_bf1 = (WORD *)malloc(cisu.t_bf_start[1]*2);   /* apply single tone area */
     cisu.t_bf2 = (WORD *)malloc(cisu.t_bf_start[2]*2);   /* apply single tone area */
     cisu.bx = (WORD *)malloc(cisu.t_bf_start[3]*2);
     cisu.attr = (BYTE *)malloc(cisu.t_bf_start[4]*2);

     memset(cisu.t_bf1, 0, cisu.t_bf_start[1]*2);          /* clear single tone area */
     memset(cisu.t_bf2, 0, cisu.t_bf_start[2]*2);          /* clear single tone area */
     memset(cisu.bx, 0, cisu.t_bf_start[3]*2);
     memset(cisu.attr, 0, cisu.t_bf_start[4]*2);

     if ( fread((char *)cisu.t_bf1, cisu.t_bf_start[1]*2, 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     if ( fread((char *)cisu.t_bf2, cisu.t_bf_start[2]*2, 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     if ( fread((char *)cisu.bx, cisu.t_bf_start[3]*2, 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     if ( fread((char *)cisu.attr, cisu.t_bf_start[4]*2, 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     fseek(fepcb->fd.abcsysfd[ABCOVL], 16, 1);     /* pointer move from current position */

     if ( fread((char *)jp_index, sizeof(jp_index), 1,
          fepcb->fd.abcsysfd[ABCOVL]) == 0 && ferror(fepcb->fd.abcsysfd[ABCOVL]) != 0 )
        return(ERROR);

     if ( fread((char *)qp_index, sizeof(qp_index), 1,
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

   if (AbcUsrDictCheckCreate(fepcb) == ERROR)
      return(ERROR);

   if ( (fepcb->fd.abcusrfd[USRREM]=open(fepcb->fname.abcusrfname[USRREM], O_RDWR)) == -1 )
      return(ERROR);

   if ((i= read(fepcb->fd.abcusrfd[USRREM], &tmmr, TMMR_REAL_LENGTH) == -1)
          && errno != 0 )
      return(ERROR);

                                 /* move the pointer to the paremeter area */
   lseek(fepcb->fd.abcusrfd[USRREM], TMMR_REAL_LENGTH, 0);
   if ( read(fepcb->fd.abcusrfd[USRREM], buffer, PAREMETER_LENGTH) != PAREMETER_LENGTH)
                                 /* read the paremeters to the buffer */
   {
      for (i = 0; i < PAREMETER_LENGTH; i++)
         buffer[i] = 0;

      buffer[0] = territory;             /* transfer the peremeters */
      buffer[1] = auto_mode;
      buffer[2] = bdd_flag;
      buffer[3] = CHINESE_GB;

      if ( write(fepcb->fd.abcusrfd[USRREM], buffer, PAREMETER_LENGTH) != PAREMETER_LENGTH )   /* writer the file */
         return(ERROR);
   }

   territory = buffer[0];                 /* transfer the paremeter */
   auto_mode = buffer[1];
   bdd_flag = buffer[2];
   fepcb->imode.ind8 = buffer[3];
                        /*  load time to fepcb when input method initial zhed  */
   stat(fepcb->fname.abcusrfname[USRREM], &info);
   p=(unsigned char*)ctime(&info.st_mtime); /* (char) is added by Ms. Dong */
   strcpy(fepcb->ctime.abctime[USRREM], p);
    
   if ( (fepcb->fd.abcusrfd[USR]=open(fepcb->fname.abcusrfname[USR], O_RDWR)) == -1 )
      return(ERROR);
   else
   {    /*  load time to fepcb when input method initial zhed  */
      stat(fepcb->fname.abcusrfname[USR], &info);
      p=(unsigned char*)ctime(&info.st_mtime); /* (char) is added by Ms. Dong */
      strcpy(fepcb->ctime.abctime[USR], p);
   }

   fepcb->mi.abcusrmi=(unsigned char *)calloc(ABC_MI_LEN, sizeof(char));
   memset(fepcb->mi.abcusrmi, NULL, ABC_MI_LEN);
   lseek(fepcb->fd.abcusrfd[USR], 0xa000l, 0);

   if ( read(fepcb->fd.abcusrfd[USR], (char *)fepcb->mi.abcusrmi, ABC_MI_LEN) == -1
        && errno != 0 )
      return(ERROR);
   memcpy((char *)&kzk_ndx.body_start, (char *)fepcb->mi.abcusrmi,ABC_MI_LEN);

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
                 0,       /* MULU_START_LOW                        */
                 0x1800,  /* MULU_LENGTH_MAX   SIZELIB+SIZELIB_KZK */
                 0x10,    /* MULU_TRUE_LENGTH  10H                 */
                 0xA,     /* MULU_RECORD_LENGTH  10                */
                 0,       /* DATA_START_HI  0                      */
                 0x1800,  /* DATA_START_LOW DW SIZE LIB_W          */
                 0x20     /* DATA_RECORD_LENGTH 32                 */
      };

      int fd, i,count;
      unsigned char *rem_area, *p;
      unsigned short *pp;

      rem_area = (unsigned char *)calloc(TMMR_LIB_LENGTH+100, sizeof(char));
      memset(rem_area, NULL, TMMR_LIB_LENGTH);

      if ( ( fd = open(fepcb->fname.abcusrfname[USRREM], O_RDONLY) ) != -1 )
      {
         read(fd, rem_area, TMMR_LIB_LENGTH);
         if ((rem_area[CHECK_POINT]=='T')&& (rem_area[CHECK_POINT+1]=='X')
                                         && (rem_area[CHECK_POINT+2]=='L')
                                         && (rem_area[CHECK_POINT+3]=='N'))
             close(fd);
         else
             close(fd), fd = -1;
      }

      if (fd == -1)
      {
          if ( ( fd = open(fepcb->fname.abcusrfname[USRREM], O_WRONLY|O_CREAT|O_TRUNC, 0644) ) == -1 )
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

             if ( write(fd, rem_area, TMMR_LIB_LENGTH) == -1 && errno != 0 )
                 return(ERROR);

             for (i = 4; i < PAREMETER_LENGTH; i++)
                 rem_area[i] = 0;       

             rem_area[0] = territory = 1; 
             rem_area[1] = auto_mode= 1;
             rem_area[2] = bdd_flag= 1;
             rem_area[3] = fepcb->imode.ind8 = CHINESE_GB;
             rem_area[0] = territory = 1;   /* Transfer the parameters */

             if ( write(fd, rem_area, PAREMETER_LENGTH) == -1 && errno != 0 )
                 return(ERROR);
             close(fd);
          }
      }

/*                                                                    */
/* Check or create TMMR.REM file is over. Now, deel with USER.REM.    */
/*                                                                    */
      if ( ( fd = open(fepcb->fname.abcusrfname[USR], O_RDONLY) ) != -1 )
      {
         lseek(fd, LENGTH_OF_USER, 0);
         read(fd, rem_area, ABC_MI_LEN);

         if ((rem_area[CHECK_POINT2]=='T')&&(rem_area[CHECK_POINT2+1]=='X')
                                         &&(rem_area[CHECK_POINT2+2]=='L')
                                         && (rem_area[CHECK_POINT2+3]=='N'))
             close(fd);
         else
             close(fd), fd = -1;
      }

      if (fd == -1)
      {
          if ( ( fd = open(fepcb->fname.abcusrfname[USR], O_WRONLY|O_CREAT|O_TRUNC, 0644) ) == -1 )
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
                  if ( write(fd, rem_area, 0x1000) == -1 && errno != 0 )
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

              if ( write(fd, &user_file_head, ABC_MI_LEN) == -1 && errno != 0 )
                   return(ERROR);
              close(fd);
          }
      }

    free(rem_area);
    return(OK);
}

