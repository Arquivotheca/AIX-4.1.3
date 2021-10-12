static char sccsid[] = "@(#)49	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedabcfio.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:28";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AbcUsrDictFileChange
 *		ReadAIabcOvl
 *		read_a_page
 *		read_data
 *		read_mulu
 *		writefile
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
/* MODULE NAME:        zhedabcfio                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method File Input/Output For ABC IM      */
/*                                                                            */
/* FUNCTION:           read_a_page      : Read System Or User Dictionary      */
/*                                                                            */
/*                     read_mulu        : Read The User Definition Index      */
/*                                        From The $(HOME)/.abcusrrem         */
/*                                                                            */
/*                     read_data        : Read The Record Correspond To The   */
/*                                        Code                                */
/*                                                                            */
/*                     writefile        : Write File In Disk                  */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "cjk.h"
#include "zhedinit.h"
#include "data.h"
#include "extern.h"
#include "zhed.h"

#define TMMR_REAL_LENGTH               0x1800
#define CHINA_CNS    2


extern WORD lib_w[];
extern BYTE *openbuf;
extern BYTE *openbuf_kzk;
extern WORD *kzk_lib_w;
extern BYTE country_flag;

int  last_flag;
LONG last_start_ps;
WORD last_size;
extern FEPCB *fep;


/******************************************************************************/
/* FUNCTION    : read_a_page                                                  */
/* DESCRIPTION : Read System Or User Dictionary                               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : file_flag = 0  System Dictionary                             */
/*                           1  User Dictionary                               */
/*               start_ps    From Where to Begain Reading                     */
/*               size        Bytes to Be Read                                 */
/*               where       Buffer                                           */
/* OUTPUT      : 0 is Fault                                                   */
/*               1 is OK                                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
read_a_page(file_flag, start_ps, size, where)
int  file_flag;
LONG start_ps;
WORD size;
LPSTR where;
{
    FILE *fp;
    int  fd;

    if ((last_flag==file_flag)&&(last_start_ps==start_ps)&&(last_size==size))
        return(1);                  /* This Read Is The Same As Last One     */
     
    if ( file_flag < 2 )
    {
        fd = fep->fd.abcusrfd[file_flag];
        lseek(fd,start_ps,0);
        if(read(fd, where, size)==-1 && errno != 0)
            return(0);
    }
    else
    {
        if ( country_flag == CHINA_CNS )
            fp = fep->fd.abcsysfd[ABCCWD_T];
        else
            fp = fep->fd.abcsysfd[ABCCWD_S];
        fseek(fp,start_ps,0);

        if(fread(where, size, 1, fp)==0 && ferror(fp) != 0)
            return(0);
    }

    last_flag=file_flag;
    last_start_ps=start_ps;         /* Save Parameters of The Read         */
    last_size=size;
    return(1);
}


/******************************************************************************/
/* FUNCTION    : read_mulu                                                    */
/* DESCRIPTION : Read The User Definition Index From The $(HOME)/.abcusrrem   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : FALSE is Fault                                               */
/*               TRUE is OK                                                   */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
read_mulu()
{
    int op_count;

    unsigned short buf[100];

    lseek(fep->fd.abcusrfd[USR],0,0);

    op_count=read(fep->fd.abcusrfd[USR],lib_w,16);
    if (op_count==-1)
        return(FALSE);        /*error*/
    mulu_true_length=lib_w[3];
    if ( (mulu_true_length-16) == 0 )
        return(TRUE);

    op_count=read(fep->fd.abcusrfd[USR],&lib_w[8],mulu_true_length-16);
    if (op_count==-1)
        return(FALSE);         /*error*/
    return(TRUE);
}


/******************************************************************************/
/* FUNCTION    : read_data                                                    */
/* DESCRIPTION : Read The Record Correspond to The Code                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : rec_cnt : Record Count to Be Read                            */
/* OUTPUT      : FALSE is Fault                                               */
/*               TRUE is OK                                                   */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
read_data(rec_cnt)
int rec_cnt;
{
    int op_count;
    lseek(fep->fd.abcusrfd[USR],(data_start+rec_cnt*data_record_length), 0);
    op_count = read(fep->fd.abcusrfd[USR], (LPSTR)&out_svw, data_record_length);
    if ( op_count == -1 )
        return(FALSE);

    out_svw[0] = out_svw[0] * 2 - '0';
    if (out_svw[0]<2)
        return(FALSE);
    if ((out_svw[0]-0x30)>30)
        return(FALSE);

    return(TRUE);

}


/******************************************************************************/
/* FUNCTION    : writefile                                                    */
/* DESCRIPTION : Write File in Disk                                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : distance: Distance Where Be Writting                         */
/*             : p       : Buffer Pointer to Be Writen                        */
/*             : count   : Count to Be Writen                                 */
/* OUTPUT      : FALSE is Fault                                               */
/*               TRUE is OK                                                   */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
writefile(distance,p,count)
LONG distance;
WORD *p;
int count;
{
    struct stat info;
    extern char *ctime();
    int write_c;

    lseek(fep->fd.abcusrfd[USRREM],distance,0);
    write_c=write(fep->fd.abcusrfd[USRREM], (LPSTR)p, count);
    if (write_c == -1 )
        return(FALSE);

    fstat(fep->fd.abcusrfd[USRREM], &info);
    strcpy(fep->ctime.abctime[USRREM], ctime(&info.st_mtime));
    return(TRUE);
}

/******************************************************************************/
/* FUNCTION    : ReadAIabcOvl                                                 */
/* DESCRIPTION : Read AIABC.ovl file                                          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/*             : pos     : position where to start read.                      */
/*             : buffer  : where the table is loaded.                         */
/*             : read_size: count to Be Writen                                */
/* OUTPUT      : FALSE is Fault                                               */
/*               TRUE is OK                                                   */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
ReadAIabcOvl( pos, buffer, read_size )
long  pos;
char  *buffer;
unsigned short read_size;
{
    int op_count;

    fseek(fep->fd.abcsysfd[ABCOVL], pos, 0);
    op_count = fread((LPSTR)buffer,read_size,1,fep->fd.abcsysfd[ABCOVL]);
    if ( op_count != 1 )
        return(FALSE);
    else
        return 1;
}


/**************************************************************************/
/* FUNCTION    : AbcUsrDictFileChange                                     */
/* DESCRIPTION : Change ABC user dictionary file whether it change or     */
/*               not, if it change, reload some struct of the file        */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : file_flag, distance, size, buffer                        */
/* OUTPUT      : TRUE, FALSE, ERROR                                       */
/* CALLED      :                                                          */
/* CALL        :                                                          */
/**************************************************************************/
AbcUsrDictFileChange(file_flag, distance, size, buffer)
int file_flag;
int distance;
int size;
unsigned char *buffer;
{
   struct stat info;
   char *p;
   extern char *ctime();

   stat(fep->fname.abcusrfname[file_flag], &info);
   p=ctime(&info.st_mtime);
   if ( !strcmp(fep->ctime.abctime[file_flag], p) )
      return(FALSE);      /*   file had not been changed   */
   else
   {                      /*   file had been changed       */
      if ( file_flag == USR )
      {
         close(fep->fd.abcusrfd[file_flag]);
         fep->fd.abcusrfd[file_flag] = open(fep->fname.abcusrfname[file_flag], O_RDWR);
      }
      if ( read_a_page(file_flag, distance, size, buffer) == ERROR )
      {
         fep->ret=ERROR;
         return(ERROR);
      }
      else
      {
         strcpy(fep->ctime.abctime[file_flag], p);
         return(TRUE);
      }
   }
}

