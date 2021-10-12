static char sccsid[] = "@(#)90  1.3  src/bos/diag/tu/artic/diagld.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:08";
/* COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
  FUNCTIONS:  load_diag_task(), swap_bytes(), window_size()

  PURPOSE: Loader for Diagnostic tasks

  ATTRIBUTE: Reentrant

  INPUTS: file descriptor for ARTIC diagnostic device driver
          path name of diagnostic task

  OUTPUTS: return code          0       no errors

                                < 0     Error. (See return code #defines below)



  ALGORITHM DESCRIPTION: Described under each function


  EXTERNAL VARIABLES USED: None

  PUBLIC VARIABLES DECLARED: None

  PUBLIC SUBROUTINES DECLARED: load_diag_task

  EXTERNAL ROUTINES CALLED:   fopen
                              fclose
                              fseek
                              icagetparms
                              icaintreg
                              icaintdereg
                              icaoutbuf
                              icainbuf
                              icasecstatbuf
                              icaissuecmd
                              icaintwait
                              icareadmem
                              icawritemem

  THIS ROUTINE CALLED BY: Diagnostics application and Hardware Exerciser

  PROCESSOR/COMPILER USED:
      AIX 3.2 CC Compiler

  MACROS USED: None

  ASSEMBLER SUBROUTINES CALLED:

  SPECIAL NOTES:

  CHANGE ACTIVITY:

 WHO DATE     ITEM
 --- -------- ----------------------------------------------------
              none


***********************************************************************/


/* Macros used to access fields in the diagnostic tasks' headers */
#define TASKNUM(x)                    *((uchar *)&x[4])
#define CMDSEG(x)                     *((ushort *)&x[12])
#define INITSEG(x)                    *((ushort *)&x[16])
#define DATASEG(x)                    *((ushort *)&x[18])
#define STACKSEG(x)                   *((ushort *)&x[20])

/* Task number to load diagnostic task as */
#define DIAG_TASK_NUM                 0

/* Size of the diagnostic tasks header */
#define TASK_HEADER_SIZE              28

/* Size of data packets read from diag task file and xferred to card */
#define XFERSIZE                      2048

/* Formats for icareadmem and icawritemem calls */
#define FORMAT_PAGEOFF                0xFF
#define FORMAT_SEGOFF                 0x00

/* ROS Command codes */
#define REQUEST_TASK_LOAD             0x01
#define START_TASK                    0x05

/* Return codes from load_diag_task()  */
#define SUCCESS                        0
#define INVALID_CARD_NUM              -1
#define CANNOT_UPDATE_IB              -2
#define CANNOT_OPEN_DIAG_TASK_FILE    -3
#define CANNOT_REGISTER_FOR_CARD_INT  -4
#define CANNOT_GET_INPUT_BUFFER       -5
#define CANNOT_GET_OUTPUT_BUFFER      -6
#define CANNOT_GET_SS_BUFFER          -7
#define CANNOT_READ_DIAG_TASK_FILE    -8
#define REQUEST_TASK_LOAD_ERROR       -9
#define NO_INTERRUPT_FROM_ROS         -10
#define CANNOT_READ_ROS_STATUS        -11
#define ERROR_FROM_RTL_CMD            -12
#define CANNOT_READ_ROS_REPLY         -13
#define CANNOT_COPY_DIAG_TASK_TO_CARD -14
#define START_TASK_ERROR              -15
#define INVALID_TASK_HEADER           -16

/* Default maxtask, maxpri, maxtime and maxqueue values */
#define DEFAULT_IB_VALS               0x10

#include <stdio.h>
#include <sys/types.h>
#include <artictst.h>

void swap_bytes(word)
/* Swaps the two bytes pointed to by the word parameter        */
/* Used to convert data into Intel format for use on the cards */
ushort *word;
{
   uchar *p, tmp;

   p = (uchar *) word;
   tmp = *p;
   *p = *(p+1);
   *(p+1) = tmp;
}

ulong window_size(size_code)
/* Converts the SSW size code (in the ICAPARMS structure) to the actual */
/* window size                                                          */
uchar size_code;
{
  switch(size_code) {
     case 0:
        return(0x2000);     /* 8K window  */
     case 1:
        return(0x4000);     /* 16K window */
     case 2:
        return(0x8000);     /* 32K window */
     case 3:
        return(0x10000);    /* 64K window */
     default:
        return(0x2000);
  }

}


static uchar xferbuf[XFERSIZE];   /* Used to xfer data from diag file  */
                                  /* to card                           */
int load_diag_task(fd, fname)
/* Loads the diagnostic task indicated by fname unto a specific card.  */
/* Note that these tasks are always loaded as task 0. The routine will */
/* fail if RCM is already loaded on the card. Also note that this will */
/* only load diagnostic tasks in .COM format                           */
int fd;           /* File descriptor of DAYTONA device driver */
char *fname;      /* Name of file with diagnostic task        */
{
   uchar ros_stat[2];                /* Status returned by ROS            */
   FILE *dfh;                        /* Diag task file handle             */
   ICABUFFER inbuf, outbuf, ssbuf;   /* ROS's buffer addresses            */
   char parmbuf[8];                  /* Used to get diag file size        */
   char taskhdbuf[TASK_HEADER_SIZE]; /* Used relocate task header         */
   ulong numwritten, numread;         /* Byte counts during load           */
   ICAPARMS ib_parms;                /* Used to update IB                 */
   ICAPARMS parms;                   /* Used to verify card number        */
   ushort start_seg;                 /* Segment on card to load task      */
   uchar tasknum;                    /* Diagnostic task number            */

   /* Get Parms for Task 0 */
   if (icagetparms(fd, &parms))
      return(INVALID_CARD_NUM);

   /* Update IB */
   ib_parms.maxtask = ib_parms.maxpri = ib_parms.maxqueue =
   ib_parms.maxtime = DEFAULT_IB_VALS;
   if (icasendconfig(fd, &ib_parms))
      return(CANNOT_UPDATE_IB);

   /* Open diagnostic task file */
   if ((dfh = fopen(fname,"rb")) == NULL)
      return(CANNOT_OPEN_DIAG_TASK_FILE);

   /* Register for ROS interrupts */
   if (icaintreg(fd)) {
      fclose(dfh);
      return(CANNOT_REGISTER_FOR_CARD_INT);
   }

   /* Get task 0's input, output and secondary status buffers */
   if (icainbuf(fd, &inbuf)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_GET_INPUT_BUFFER);
   }

   if (icaoutbuf(fd, &outbuf)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_GET_OUTPUT_BUFFER);
   }

   if (icasecstatbuf(fd, &ssbuf)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_GET_SS_BUFFER);
   }

   /* Indicate task number to be loaded */
   parmbuf[0] = DIAG_TASK_NUM;

   /* Get diagnostic task file size */
   if (fread(&parmbuf[1],1,4,dfh) != 4) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_READ_DIAG_TASK_FILE);
   }

   /* Issue Request Task Load Commmand to ROS */
   if (icaissuecmd(fd, REQUEST_TASK_LOAD, 5, 1000, parmbuf)) {
      fclose(dfh);
      icaintdereg(fd);
      return(REQUEST_TASK_LOAD_ERROR);
   }

   /* Wait for ROS to interrupt back */
   if (icaintwait(fd, 1000)) {
      fclose(dfh);
      icaintdereg(fd);
      return(NO_INTERRUPT_FROM_ROS);
   }

   /* Check ROS's return code from RTL Command */
   if (icareadmem(fd, 1, (ushort) ssbuf.page, ssbuf.offset, FORMAT_PAGEOFF,
                  ros_stat)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_READ_ROS_STATUS);
   }

   if (ros_stat[0]) {
      fclose(dfh);
      icaintdereg(fd);
      return(ERROR_FROM_RTL_CMD);
   }

   /* Get the start segment of the task to be loaded */
   if (icareadmem(fd, 2, (ushort) inbuf.page, inbuf.offset,
                  FORMAT_PAGEOFF, &start_seg)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_READ_ROS_REPLY);
   }

   /* Reset diag file ptr to beginning of file */
   fseek(dfh, 0L, 0);

   /* Read task header */
   numread = fread(taskhdbuf,1,TASK_HEADER_SIZE,dfh);
   if (numread < TASK_HEADER_SIZE) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_READ_DIAG_TASK_FILE);
   }

   /* Do relocation on task header */
   TASKNUM(taskhdbuf) = DIAG_TASK_NUM;
   CMDSEG(taskhdbuf) = start_seg;
   INITSEG(taskhdbuf) = start_seg;
   DATASEG(taskhdbuf) = start_seg;
   STACKSEG(taskhdbuf) = start_seg;

   /* Return page and offset of task start address */
   swap_bytes(&start_seg);

   /* Write task header to card */
   if (icawritemem(fd, TASK_HEADER_SIZE, start_seg, 0x0000,FORMAT_SEGOFF,
       taskhdbuf)) {
      fclose(dfh);
      icaintdereg(fd);
      return(CANNOT_COPY_DIAG_TASK_TO_CARD);
   }

   /* Write rest of task to card */
   numwritten = TASK_HEADER_SIZE;

   while (1) {
      numread = fread(xferbuf,1,XFERSIZE,dfh);
      if (numread > 0) {
         if(icawritemem(fd, numread, start_seg, numwritten,
                        FORMAT_SEGOFF, xferbuf)) {
            fclose(dfh);
            icaintdereg(fd);
            return(CANNOT_COPY_DIAG_TASK_TO_CARD);
         }
         numwritten += numread;
      }
      else
         break;
   }

   /* Verify the right number of bytes were written to card */
   swap_bytes((ushort *) (parmbuf+1));
   if (*((ushort *) (parmbuf+1)) != numwritten) {
      fclose(dfh);
      icaintdereg(fd);
      return(INVALID_TASK_HEADER);
   }

   /* Issue Start Task Command to ROS */
   tasknum = DIAG_TASK_NUM;
   if (icaissuecmd(fd, START_TASK, 1, 1000, &tasknum)) {
      fclose(dfh);
      icaintdereg(fd);
      return(START_TASK_ERROR);
   }

   /* Wait for started task to interrupt back   */
   /* NOTE: no need to check command status in  */
   /* secondary status buffer because if the    */
   /* command is executed successfully the task */
   /* will interrupt back                       */
   if (icaintwait(fd, 1000)) {
      fclose(dfh);
      icaintdereg(fd);
      return(NO_INTERRUPT_FROM_ROS);
   }

   /* Indicate successful load */
   fclose(dfh);
   icaintdereg(fd);
   return(SUCCESS);
}


