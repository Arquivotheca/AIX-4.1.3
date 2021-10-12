static char sccsid[] = "@(#)27	1.3  src/bos/diag/tu/sun/Decompinit.c, tu_sunrise, bos411, 9437A411a 5/27/94 13:41:46";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: Decompress
 *		FillFIFO
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
/*  File:  decompinit.c                                                   */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include "pc5xx.h"
#include "makehuff.h"
#include "error.h"
#include "suntu.h"

int FillFIFO( void );
extern short PixelMode;

/*****************************************************************************/
/*  Decompress()                                                             */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will decompress the compressed data that is in the frame     */
/*    buffer and output the decompressed data image to the field memory.    */
/*                                                                           */
/*  INPUT:    ImageWidth = Width of the raw image                            */
/*            ImageHeight= Height of the raw image                           */
/*            QFactor    = Compress Q factor                                 */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int Decompress(short ImageWidth, short ImageHeight, short QFactor)
{
    int           rc;
    unsigned int    Flags;
    clock_t startTime;
    struct tms tmsstart, tmsnow;

   /* Setup for Decompression */
    PixelMode = _422;      /* set up color mode for the driver */
    Direction = DECOMPRESS;
    if (rc = CODECSetup(ImageWidth, ImageHeight, QFactor))
       return(ERRORDECOMP_SETUP);

    /* Setup Local Bus Control reg for Decompression */
#ifdef CIF  /* If using CIF image */
    if (rc = pio_write (LBControl, 0x4000a060, 1)) return(rc);
#else
    if (rc = pio_write (LBControl, 0x4000a040, 1)) return(rc);
#endif

    /* Setup 550 for decompression and fill FIFO */
    if (rc = WriteCL5xxRegister(_DRQ_Mask, 0x0, 0 )) return(rc);
    if (rc = WriteCL5xxRegister(_Flags, 0x1, 0)) return(rc);    /* this clears the late flag */
    if (rc = FillFIFO())                       /*  Pre-Fill FIFO to 3/4 */
       return(ERRORDECOMP_FILLFIFO);
    if (rc = WriteCL5xxRegister(_Flags, 0x1, 0)) return(rc);    /* this clears the late flag */
    if (rc = WriteCL5xxRegister(_IRQ1_Mask, 0x0800, 0)) return(rc);
    if (rc = WriteCL5xxRegister(_DRQ_Mask, 0x2002, 0)) return(rc);
    if (rc = WriteCL5xxRegister(_HV_Enable, 0x1, 0)) return(rc);

    if (rc = WriteCL5xxRegister(_Start, 0x1, 0 )) return(rc);       /* Ok....Go !! */

    startTime=times(&tmsstart);
    while(1) {
      if (rc = ReadCL5xxRegister(0x9014, &Flags,0)) return(rc);
      Flags &= 0x00002000;
      if (Flags) {
        if (rc = WriteCL5xxRegister(_Start, 0x0, 0 )) return(rc);
        break;
      }
      if ((times(&tmsnow)-startTime) >= CODEC_TIMEOUT)
          return(ERRORDECOMP_TIMEOUT);
    }
    return(OK);
} /* End of Decompress() */


#define FILLSIZE                96              /* 3/4 of FIFO */
int FillFIFO()
{
  int rc, i;
  unsigned int val1, val2, val, Flags;

#ifdef CL560B
  for (i=0;i<FILLSIZE ;i++ ) {
    if (rc = pio_read(HIU_4, &val1, 1)) return(rc);
    if (rc = pio_write (CDbaseaddr, val, 1)) return(rc);
  }
#else
  do {
    if (rc = pio_read(HIU_4, &val1, 1)) return(rc);
    if (rc = pio_read(HIU_4, &val2, 1)) return(rc);
    if (rc = ReadCL5xxRegister(0x9014, &Flags, 0)) return(rc);
    while (!(Flags & 0x2000)){
      usleep(20);
      if (rc = ReadCL5xxRegister(0x9014, &Flags, 0)) return(rc);
    }
    if (rc = pio_write (CDbaseaddr, val1, 1)) return(rc);
    if (rc = pio_write (CDbaseaddr, val2, 1)) return(rc);
    usleep(20);
    if (rc = ReadCL5xxRegister(0x9014, &Flags, 0)) return(rc);
    Flags &= 00000010;
  } while (!Flags);
#endif

  return(OK);
}    /* End of FillFIFO */

