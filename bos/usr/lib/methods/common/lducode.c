static char sccsid[] = "@(#)75	1.1  src/bos/usr/lib/methods/common/lducode.c, cfgmethods, bos411, 9428A410j 8/2/92 15:09:09";
/*
 * COMPONENT_NAME: (CFGMETH) cfgampx
 *
 * FUNCTIONS: load_ucode(), swap_bytes()                    
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*�����������������������������������������������������������������������Ŀ
�                                                                         �
�                                                                         �
� PURPOSE: Loads tasks on to ARTIC adapters using the machine device      �
�          driver                                                         �
�                                                                         �
� ALGORITHM DESCRIPTION: Described under each function                    �
�                                                                         �
� EXTERNAL VARIABLES USED: None                                           �
�                                                                         �
� PUBLIC VARIABLES DECLARED: None                                         �
�                                                                         �
� PUBLIC SUBROUTINES DECLARED: load_diag_task                             �
�                                                                         �
� EXTERNAL ROUTINES CALLED:                                               �
�                             getbuffers                                  �
�                             issuecmd                                    �
�                             intwait                                     �
�                             readmem                                     �
�                             writemem                                    �
�                                                                         �
��������������������������������������������������������������������������*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include "lducode.h"
#include <sys/errno.h>
#include <sys/mdio.h>

/*����������������������������������������������Ŀ
 � Macros used to access fields in task header   �
 ������������������������������������������������*/

#define TASKNUM(x)                    *((uchar *)&x[4])
#define CMDSEG(x)                     *((ushort *)&x[12])
#define INITSEG(x)                    *((ushort *)&x[16])
#define DATASEG(x)                    *((ushort *)&x[18])
#define STACKSEG(x)                   *((ushort *)&x[20])

/*����������������������������������������������Ŀ
 � Size of xxxxxxxx.com file headers             �
 ������������������������������������������������*/
#define TASK_HEADER_SIZE              28

/*����������������������������������������������Ŀ
 � Size of data packets read from task file and  �
 � transferred to card                           �
 ������������������������������������������������*/
#define XFERSIZE                      128

/*����������������������������������������������Ŀ
 � ROS Command codes                             �
 ������������������������������������������������*/
#define REQUEST_TASK_LOAD             0x01
#define START_TASK                    0x05

/*����������������������������������������������Ŀ
 � Return codes from load_ucode                  �
 ������������������������������������������������*/
#define SUCCESS                        0
#define E_FILE_OPEN                   -1
#define E_GET_BUFFERS                 -2
#define E_FILE_READ                   -3
#define E_REQ_TASK_LOAD               -4
#define E_NO_INTERRUPT                -5
#define E_READ_BUSMEM                 -6
#define E_WRITE_BUSMEM                -7
#define E_RTL_CMD                     -8
#define E_ISSUECMD                    -9
#define E_TASK_HEADER                 -10
#define E_RESET                       -11
#define E_TASK_ALREADY_LOADED         -12

void swap_bytes(word)
/*��������������������������������������������������������������Ŀ
 � Swaps the two bytes pointed to by the word parameter.         �
 � Used to convert data into Intel format for use on the cards   �
 �                                                               �
 � INPUTS: address of word to swap                               �
 �                                                               �
 � OUTPUTS: none                                                 �
 �                                                               �
 ����������������������������������������������������������������*/
ushort *word;    /* Word to be swapped */
{
   uchar *p, tmp;

   p = (uchar *) word;
   tmp = *p;
   *p = *(p+1);
   *(p+1) = tmp;
}

int load_ucode(fd, pos, fname, tasknum)
/*�������������������������������������������������������������������������Ŀ
 � Loads the task indicated by fname unto a specific card. The routine will �
 � fail if tasknum is  already loaded on the card. Also note that this will �
 � only load tasks in .COM format.                                          �
 �                                                                          �
 � INPUTS: file descriptor for machine device driver                        �
 �         POS register info of target adapter                              �
 �         path name of task (microcode)                                    �
 �         task number to load task as                                      �
 �                                                                          �
 � OUTPUTS: return code       0       no errors                             �
 �                            < 0     Error                                 �
 ���������������������������������������������������������������������������*/
int fd;           /* File descriptor of machine device driver */
POS_INFO *pos;    /* POS reg info of target adapter           */
char *fname;      /* Name of task to load                     */
uchar tasknum;    /* Tasknumber to load task as               */
{
   uchar ros_stat[2];                /* Status returned by ROS            */
   FILE *dfh;                        /* Task file handle                  */
   BUFFER_ADDRS bufs;                /* Task's buffer addresses           */
   char parmbuf[8];                  /* Used to get diag file size        */
   char taskhdbuf[TASK_HEADER_SIZE]; /* Used relocate task header         */
   uint numwritten, numread;         /* Byte counts during load           */
   ushort start_seg;                 /* Segment on card to load task      */
   uchar primstat;                   /* Task's primary status byte        */
   uchar load_page;                  /* Page/off representation of        */
   uint load_offset;                 /* task's load segment               */
   uchar xferbuf[XFERSIZE];          /* Used to xfer data from task file  */
                                     /* to card                           */

   /*�����������������Ŀ
    � Open task file   �
    �������������������*/
   if ((dfh = fopen(fname,"rb")) == NULL)
      return(E_FILE_OPEN);

   /*�������������������������������Ŀ
    � Get task's primary status byte �
    ���������������������������������*/
   if (getprimstat(fd, pos, tasknum, &primstat))
   {
      fclose(dfh);
      return(E_READ_BUSMEM);
   }

   /*��������������������������������������������������������Ŀ
    �   If task is already loaded return error if tasknum > 0 �
    �   else reset the adapter and continue                   �
    ����������������������������������������������������������*/
   if (primstat != 0x00)
   {
      if (tasknum != 0)
      {
         fclose(dfh);
         return(E_TASK_ALREADY_LOADED);
      }
      else
      {
         if (reset(fd, pos))
         {
            fclose(dfh);
            return(E_RESET);
         }
      }
   }

   /*������������������������������������������������������Ŀ
    � Get task's input, output and secondary status buffers �
    ��������������������������������������������������������*/
   if (getbuffers(fd, pos, tasknum, &bufs))
   {
      fclose(dfh);
      return(E_GET_BUFFERS);
   }

   /*����������������������������������Ŀ
    � Indicate task number to be loaded �
    ������������������������������������*/
   parmbuf[0] = tasknum;

   /*��������������������Ŀ
    � Get task file size  �
    ����������������������*/
   if (fread(&parmbuf[1],1,4,dfh) != 4)
   {
      fclose(dfh);
      return(E_FILE_READ);
   }

   /*�����������������������������������������Ŀ
    � Issue Request Task Load Commmand to ROS  �
    �������������������������������������������*/
   if (issuecmd(fd, pos, tasknum, REQUEST_TASK_LOAD, 1000, 5, parmbuf))
   {
      fclose(dfh);
      return(E_ISSUECMD);
   }

   /*��������������������������������Ŀ
    � Wait for ROS to interrupt back  �
    ����������������������������������*/
   if (intwait(fd, pos, tasknum, 1000))
   {
      fclose(dfh);
      return(E_NO_INTERRUPT);
   }

   /*������������������������������������������Ŀ
    � Check ROS's return code from RTL Command  �
    ��������������������������������������������*/
   if (readmem(fd, pos, bufs.sspage, bufs.ssoffset, 1, ros_stat))
   {
      fclose(dfh);
      return(E_READ_BUSMEM);
   }

   if (ros_stat[0])
   {
      fclose(dfh);
      return(E_RTL_CMD);
   }

   /*������������������������������������������������Ŀ
    � Read starting segment from ROS input buffer     �
    ��������������������������������������������������*/
   if (readmem(fd, pos, bufs.inpage, bufs.inoffset, 2, &start_seg))
   {
      fclose(dfh);
      return(E_READ_BUSMEM);
   }

   /*������������������������������������������Ŀ
    � Reset file ptr to beginning of task file  �
    ��������������������������������������������*/
   fseek(dfh, 0L, 0);

   /*�������������������Ŀ
    � Read task header   �
    ���������������������*/
   numread = fread(taskhdbuf,1,TASK_HEADER_SIZE,dfh);
   if (numread < TASK_HEADER_SIZE)
   {
      fclose(dfh);
      return(E_FILE_READ);
   }

   /*������������������������������Ŀ
    � Do relocation on task header  �
    ��������������������������������*/
   TASKNUM(taskhdbuf) = tasknum;
   CMDSEG(taskhdbuf) = start_seg;
   INITSEG(taskhdbuf) = start_seg;
   DATASEG(taskhdbuf) = start_seg;
   STACKSEG(taskhdbuf) = start_seg;

   /*�������������������������������������������Ŀ
    � Get page and offset of task start address  �
    ���������������������������������������������*/
   swap_bytes(&start_seg);
   load_page = (start_seg << 4)/pos->win_size;
   load_offset = (start_seg << 4) - (load_page * pos->win_size);

   /*���������������������������Ŀ
    � Write task header to card  �
    �����������������������������*/
   if (writemem(fd, pos, load_page, load_offset, TASK_HEADER_SIZE, taskhdbuf))
   {
      fclose(dfh);
      return(E_WRITE_BUSMEM);
   }

   /*����������������������������Ŀ
    � Write rest of task to card  �
    ������������������������������*/
   numwritten = TASK_HEADER_SIZE;
   load_offset += numwritten;
   while (1)
   {
      numread = fread(xferbuf,1,128,dfh);
      if (numread > 0)
      {
         /*��������������������������������������������������Ŀ
          �Adjust page and offset if we cross a page boundary �
          ����������������������������������������������������*/
         if (load_offset >= pos->win_size)
         {
            load_offset -= pos->win_size;
            ++load_page;
         }

         if (writemem(fd, pos, load_page, load_offset, numread,  xferbuf))
         {
            fclose(dfh);
            return(E_WRITE_BUSMEM);
         }
         numwritten += numread;
         load_offset += numread;
      }
      else
         break;
   }

   /*�������������������������������������������������������Ŀ
    � Verify the right number of bytes were written to card  �
    ���������������������������������������������������������*/
   swap_bytes((ushort *) (parmbuf+1));
   if (*((ushort *) (parmbuf+1)) != numwritten)
   {
      fclose(dfh);
      return(E_TASK_HEADER);
   }

   /*���������������������������������Ŀ
    � Issue Start Task Command to ROS  �
    �����������������������������������*/
   if (issuecmd(fd, pos, tasknum, START_TASK, 1000, 1, &tasknum))
   {
      fclose(dfh);
      return(E_ISSUECMD);
   }

   /*������������������������������������������Ŀ
    � Wait for started task to interrupt back   �
    � NOTE: no need to check command status in  �
    � secondary status buffer because if the    �
    � command is executed successfully the task �
    � will interrupt back                       �
    ��������������������������������������������*/
   if (intwait(fd, pos, tasknum, 1000))
   {
      fclose(dfh);
      return(E_NO_INTERRUPT);
   }

   /*��������������������������Ŀ
    � Indicate successful load  �
    ����������������������������*/
   fclose(dfh);
   return(SUCCESS);
}


