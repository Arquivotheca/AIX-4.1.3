static char sccsid[] = "@(#)26  1.5  src/bos/diag/tu/sun/Compinit.c, tu_sunrise, bos411, 9437A411a 8/1/94 16:07:58";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: Compress
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*  File:  COMPINIT.C          Version:   0.1                             */
/**************************************************************************/

#include <stdio.h>
#include <sys/times.h>
#include <malloc.h>
#include <stdlib.h>
#include "pc5xx.h"
#include "makehuff.h"
#include "error.h"
#include "suntu.h"

#undef HUFF_TABLES_THE_HARD_WAY   /* generate huffman from BitsNValues */

/*****************************************************************************/
/*  Compress()                                                               */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will compress the image data that is in the frame buffer     */
/*    and the compressed data will be stored to the frame buffer.            */
/*                                                                           */
/*  INPUT:    ImageWidth = Width of the raw image                            */
/*            ImageHeight= Height of the raw image                           */
/*            QFactor    = Compress Q factor                                 */
/*            count      = Words count of compressed data                    */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int Compress(short ImageWidth, short ImageHeight, short QFactor,
             unsigned int* count)
{
  int rc;
  unsigned int StatusVal, Dummy;
  unsigned int Flags, Flags1, Flags2;
  unsigned int CodecData1, CodecData2;
  unsigned int WordsLeft;
  int i;
  FILE *CodecFile;
  clock_t startTime,endTime;
  struct tms tmsstart, tmsnow;

/* Setup for compression */
  Direction = COMPRESS;  /* Global variable setup */
  if (rc = CODECSetup(ImageWidth, ImageHeight, QFactor))
     return(ERRORCOMP_SETUP);

/*  Initialization completed, setup interrupt mask */
#ifndef CL560B
  if (rc = WriteCL5xxRegister(_IRQ1_Mask, 0x0080, 0)) return(rc); /* empty */
  if (rc = WriteCL5xxRegister(_DRQ_Mask, 0x2040, 0 )) return(rc); /* codecNB & fifo1q */
#else
  if (rc = WriteCL5xxRegister(_DRQ_Mask, _FIFO_QFull, 0 )) return(rc);
  if (rc = WriteCL5xxRegister(_IRQ1_Mask, _FIFO_QFull, 0)) return(rc);
  if (rc = WriteCL5xxRegister(_IRQ2_Mask, _FIFO_HalfFull, 0)) return(rc);
  if (rc = WriteCL5xxRegister(_FRMEND_Enable, 0x2, 0       )) return(rc); /* Enable FEND at coder */
#endif

/* Clear the count register */
  if (rc = pio_read(CNTreg,&Dummy,1)) return(rc);

  if (rc = WriteCL5xxRegister(_HV_Enable, 0x1, 0)) return(rc);

/* Clear the Status/Interrupt bits */
   startTime=times(&tmsstart);
   while(1) {
     /* Clearing Status */
     if(rc=pio_read(LBStatus, &StatusVal, 1)) return(rc);
     StatusVal &= 0x00000038;
     if((StatusVal != 0x00000038)) {
        pio_read(0x1ffa0020,&Dummy,1);
        pio_write(0x1ffa2010,0,1);
        pio_write(LBStatus,0,1);
     }
     else
        break;
     if ((times(&tmsnow)-startTime) >= CODEC_TIMEOUT)
        return(ERRORCOMP_STATUSTO);
   }

   /* The VSync register somehow got corrupted when using an external power */
   /* supply as in stress lab, therefore failed the compression test.       */
   /* Re-writing to VSync reg. fixed the problem:       08/01/94            */
   if (rc = WriteCL5xxRegister(_VSync, 0x04,0)) return(rc);

   /*  Setup interrupt */
   if (rc = pio_write(LBControl, 0x4000ac1a,1)) return(rc);
   /* Starting Compression..........*/
   if (rc = WriteCL5xxRegister(_Start, 0x1, 0 )) return(rc);   /* Ok....Go !! */

   i = 0;
   do
   {
     /* Check for Vsync & CUBE550 (FIFO) interrupting */
     startTime=times(&tmsstart);
     while(1) {
       usleep(200);
       if(rc=pio_read(LBStatus, &StatusVal, 1)) return(rc);
       if ((StatusVal & 0x2) && !(StatusVal & 0x1)) {
           if (rc = pio_write(LBControl, 0x4000ec1a,1)) return(rc);
           /* Setting proper DMA Request Interrupt */
           if (rc = WriteCL5xxRegister(_DRQ_Mask, 0x6040, 0 )) return(rc);
           break;
       }
     if ((times(&tmsnow)-startTime) >= CODEC_TIMEOUT)
        return(ERRORCOMP_VSYNCTO);
     }  /* End of while */

     /* Wait for FIFO empty to indicate completion of transfering compressed */
     /* data from CUBE to Pixel                                              */
     startTime=times(&tmsstart);
     do {
       usleep(50);
       if (rc = ReadCL5xxRegister(0x9014, &Flags1,0)) return(rc);
       Flags1 &= 0x00000080;
       usleep(20);
       if (rc = ReadCL5xxRegister(0x9014, &Flags2,0)) return(rc);
       Flags2 &= 0x00000080;
       if ((times(&tmsnow)-startTime) >= CODEC_TIMEOUT)
          return(ERRORCOMP_FIFOTO);
     } while (!(Flags1 && Flags2));

     /* Dumping out the last word of data and the EOI (End of Image) marker */
     if (rc = ReadCL5xxRegister(0x0000,&CodecData1,0)) return(rc);
     if (rc = ReadCL5xxRegister(0x0000,&CodecData2,0)) return(rc);
     CodecData1 &= 0x0000ffff;
     CodecData2 &= 0x0000ffff;
     if (rc = pio_write(0x1eff0010,CodecData1,1)) return(rc);
     if (rc = pio_write(0x1eff0010,CodecData2,1)) return(rc);
     if (rc = pio_write(0x1eff0010,0xffff,1)) return(rc);
     if (rc = pio_write(0x1eff0010,0xd9ff,1)) return(rc);

     /* Read the count register for number of compressed 32-bit data words */
     if (rc = pio_read(CNTreg,count,1)) return(rc);
     *count = *count & 0x0000ffff;

     /* Reset the Huffman coder & interrupt mask */
     if (rc = WriteCL5xxRegister(0x9020, 0xffd0, 0 )) return(rc);  /* coder reset */
     if (rc = WriteCL5xxRegister(_DRQ_Mask, 0x2040, 0 )) return(rc);  /* codecNB & fifo1q */
     if (rc = pio_write(LBControl, 0x4000ac1a,1)) return(rc);
     i++;
   }  while (i < 1) ;  /* For single field compression */

   /* Stop compression engine */
   startTime=times(&tmsstart);
   do {
      if (rc = WriteCL5xxRegister(_Start, 0x0, 0 )) return(rc);
      usleep(100);
      if (rc = ReadCL5xxRegister(_Start, &Flags,0)) return(rc);
      Flags &= 0x0000ffff;
      if ((times(&tmsnow)-startTime) >= CODEC_TIMEOUT)
          return(ERRORCOMP_STOPTO);
   } while (Flags);

   usleep(100000);  /* Delay for data transfer settle */
   return(OK);

} /* End of Compress */
