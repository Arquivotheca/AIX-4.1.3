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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³                                                                         ³
³                                                                         ³
³ PURPOSE: Loads tasks on to ARTIC adapters using the machine device      ³
³          driver                                                         ³
³                                                                         ³
³ ALGORITHM DESCRIPTION: Described under each function                    ³
³                                                                         ³
³ EXTERNAL VARIABLES USED: None                                           ³
³                                                                         ³
³ PUBLIC VARIABLES DECLARED: None                                         ³
³                                                                         ³
³ PUBLIC SUBROUTINES DECLARED: load_diag_task                             ³
³                                                                         ³
³ EXTERNAL ROUTINES CALLED:                                               ³
³                             getbuffers                                  ³
³                             issuecmd                                    ³
³                             intwait                                     ³
³                             readmem                                     ³
³                             writemem                                    ³
³                                                                         ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include "lducode.h"
#include <sys/errno.h>
#include <sys/mdio.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Macros used to access fields in task header   ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

#define TASKNUM(x)                    *((uchar *)&x[4])
#define CMDSEG(x)                     *((ushort *)&x[12])
#define INITSEG(x)                    *((ushort *)&x[16])
#define DATASEG(x)                    *((ushort *)&x[18])
#define STACKSEG(x)                   *((ushort *)&x[20])

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Size of xxxxxxxx.com file headers             ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#define TASK_HEADER_SIZE              28

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Size of data packets read from task file and  ³
 ³ transferred to card                           ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#define XFERSIZE                      128

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ ROS Command codes                             ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
#define REQUEST_TASK_LOAD             0x01
#define START_TASK                    0x05

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Return codes from load_ucode                  ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
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
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Swaps the two bytes pointed to by the word parameter.         ³
 ³ Used to convert data into Intel format for use on the cards   ³
 ³                                                               ³
 ³ INPUTS: address of word to swap                               ³
 ³                                                               ³
 ³ OUTPUTS: none                                                 ³
 ³                                                               ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
ushort *word;    /* Word to be swapped */
{
   uchar *p, tmp;

   p = (uchar *) word;
   tmp = *p;
   *p = *(p+1);
   *(p+1) = tmp;
}

int load_ucode(fd, pos, fname, tasknum)
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 ³ Loads the task indicated by fname unto a specific card. The routine will ³
 ³ fail if tasknum is  already loaded on the card. Also note that this will ³
 ³ only load tasks in .COM format.                                          ³
 ³                                                                          ³
 ³ INPUTS: file descriptor for machine device driver                        ³
 ³         POS register info of target adapter                              ³
 ³         path name of task (microcode)                                    ³
 ³         task number to load task as                                      ³
 ³                                                                          ³
 ³ OUTPUTS: return code       0       no errors                             ³
 ³                            < 0     Error                                 ³
 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
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

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Open task file   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if ((dfh = fopen(fname,"rb")) == NULL)
      return(E_FILE_OPEN);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get task's primary status byte ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (getprimstat(fd, pos, tasknum, &primstat))
   {
      fclose(dfh);
      return(E_READ_BUSMEM);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³   If task is already loaded return error if tasknum > 0 ³
    ³   else reset the adapter and continue                   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
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

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get task's input, output and secondary status buffers ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (getbuffers(fd, pos, tasknum, &bufs))
   {
      fclose(dfh);
      return(E_GET_BUFFERS);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Indicate task number to be loaded ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   parmbuf[0] = tasknum;

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get task file size  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (fread(&parmbuf[1],1,4,dfh) != 4)
   {
      fclose(dfh);
      return(E_FILE_READ);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Issue Request Task Load Commmand to ROS  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (issuecmd(fd, pos, tasknum, REQUEST_TASK_LOAD, 1000, 5, parmbuf))
   {
      fclose(dfh);
      return(E_ISSUECMD);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Wait for ROS to interrupt back  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (intwait(fd, pos, tasknum, 1000))
   {
      fclose(dfh);
      return(E_NO_INTERRUPT);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Check ROS's return code from RTL Command  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
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

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Read starting segment from ROS input buffer     ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (readmem(fd, pos, bufs.inpage, bufs.inoffset, 2, &start_seg))
   {
      fclose(dfh);
      return(E_READ_BUSMEM);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Reset file ptr to beginning of task file  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   fseek(dfh, 0L, 0);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Read task header   ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   numread = fread(taskhdbuf,1,TASK_HEADER_SIZE,dfh);
   if (numread < TASK_HEADER_SIZE)
   {
      fclose(dfh);
      return(E_FILE_READ);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Do relocation on task header  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   TASKNUM(taskhdbuf) = tasknum;
   CMDSEG(taskhdbuf) = start_seg;
   INITSEG(taskhdbuf) = start_seg;
   DATASEG(taskhdbuf) = start_seg;
   STACKSEG(taskhdbuf) = start_seg;

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Get page and offset of task start address  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   swap_bytes(&start_seg);
   load_page = (start_seg << 4)/pos->win_size;
   load_offset = (start_seg << 4) - (load_page * pos->win_size);

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Write task header to card  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (writemem(fd, pos, load_page, load_offset, TASK_HEADER_SIZE, taskhdbuf))
   {
      fclose(dfh);
      return(E_WRITE_BUSMEM);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Write rest of task to card  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   numwritten = TASK_HEADER_SIZE;
   load_offset += numwritten;
   while (1)
   {
      numread = fread(xferbuf,1,128,dfh);
      if (numread > 0)
      {
         /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
          ³Adjust page and offset if we cross a page boundary ³
          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
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

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Verify the right number of bytes were written to card  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   swap_bytes((ushort *) (parmbuf+1));
   if (*((ushort *) (parmbuf+1)) != numwritten)
   {
      fclose(dfh);
      return(E_TASK_HEADER);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Issue Start Task Command to ROS  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (issuecmd(fd, pos, tasknum, START_TASK, 1000, 1, &tasknum))
   {
      fclose(dfh);
      return(E_ISSUECMD);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Wait for started task to interrupt back   ³
    ³ NOTE: no need to check command status in  ³
    ³ secondary status buffer because if the    ³
    ³ command is executed successfully the task ³
    ³ will interrupt back                       ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   if (intwait(fd, pos, tasknum, 1000))
   {
      fclose(dfh);
      return(E_NO_INTERRUPT);
   }

   /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ³ Indicate successful load  ³
    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
   fclose(dfh);
   return(SUCCESS);
}


