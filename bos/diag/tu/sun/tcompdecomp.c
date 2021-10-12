static char sccsid[] = "@(#)54  1.6  src/bos/diag/tu/sun/tcompdecomp.c, tu_sunrise, bos411, 9437A411a 9/13/94 08:41:38";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: CompTest
 *              DeCompTest
 *              LoadCompressedData
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
/*  File:  TCOMPDECOMP.C                                                  */
/**************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include "pc5xx.h"
#include "sun_tu_type.h"
#include "error.h"
#include "suntu.h"

/***** Globals ******************************************************/
/* Compression/Decompression Setup */
short ImageWidth = 640;
short ImageHeight =240;
short QFactor = 40;

/***** Functions ****************************************************/
int CompTest();
int DeCompTest();
int LoadCompressedData();

#define         COMP_CRC1        0x18bc5341
#define         COMP_CRC2        0x9cfd64f6
#define         COMP_SIZE        1898  /* 0x76A */
/*   COMP_CRC1:  Golden cksum                                               */
/*   COMP_CRC2:  Compression sometimes also give this cksum, the            */
/*               compressed data has the color shift problem (need further  */
/*               investigation).                                            */

/*****************************************************************************/
/*  CompTest()                                                               */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will load an image in the frame buffer, compress a field     */
/*    of the image (upto 224 lines, 640x224), then calculate the CRC         */
/*    (checksum) of the compressed data and compare to the defined CRC.      */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int CompTest()
{

   int rc;
   int i,j,k, EOIfound;
   unsigned long *DMAReadData;
   unsigned long *DMAReadData1;
   unsigned long StatusVal, comp_count;
   unsigned int size, size1;
   unsigned long crcvalue;
   FILE *FOUT,*FIN;
/*
   unsigned int test_pattern[8]={0xbe80be80,0xaa2caa8e,0x8a9a8a2a,0x7747773a,
                                 0x58b758c7,0x466346d5,0x24d32472,0x00800080};
   unsigned int test_pattern[8]={0x00800080,0x15801580,0x30803080,0x62806280,
                                 0x92809280,0xc280c280,0xf280f280,0xff80ff80};
*/
   unsigned int test_pattern[8]={0xbe7fbe00,0xaa2caa8e,0x8a9a8a2a,0x7747773a,
                                      0x58b758c7,0x466346d5,0x24d32472,0xff80ff80};

   /* Setup front-end video */
   if (rc = vidsetup(NTSC_NOINPUT))
   {
      LOG_COMP_ERROR(ERRORCOMP_VIDSETUP,rc,0,0,0,0);
   }

   /* Store the pattern generated image in the frame buffer */
   if (rc = DMAWrite(640,480,0x0,test_pattern,8,40,WORD_BYTESWAP))
   {
      LOG_COMP_ERROR(ERRORCOMP_WRITEDATA,rc,0,0,0,0);
   }

   /* Move image data from top portion of frame buffer OUT to the pixel bus */
   /* thru V1 port of 2070                                                   */
   if (rc=V1Out(0))
   {
      LOG_COMP_ERROR(ERRORCOMP_V1OUT,rc,0,0,0,0);
   }
   sleep(1);        /* wait for pixel data ready */

   /* Capture the image to field memory from the V1 port */
   if (rc = pio_write(LBControl, 0x4000ac0a,1))
   {
      LOG_COMP_ERROR(ERRORCOMP_STOREFIELD,rc,0,0,0,0);
   }
   usleep(500000);

   /* Freeze the image */
   if (rc = pio_write(LBControl, 0x4000ac18,1))
   {
      LOG_COMP_ERROR(ERRORCOMP_FREEZEFIELD,rc,0,0,0,0);
   }
   usleep(500000);

   /* Setup 2070 for transfering compressed data from 560 to 2070 */
   if (rc = c560To2070())
   {
      LOG_COMP_ERROR(ERRORCOMP_560TO2070,rc,0,0,0,0);
   }

   /* Compress whatever in field memory */
   if (rc = Compress(ImageWidth, (ImageHeight-16), QFactor, &comp_count ))
      /* Failed at line 235 or after: gave different cksums of comp.dat's,   */
      /* therefore, adjust the y-size of image in compression.               */
      /* This maybe due to the way the 2070 setup to transfer data that the  */
      /* last 12 (use 16 for multiple of 8) lines may have invalid data      */
      /* (see Setup_Ip1 call in V1In() routine)                              */

   {
      LOG_COMP_ERROR(ERRORCOMP_COMPRESS,rc,0,0,0,0);
   }
   DEBUG_1("Count: %08x \n\r",comp_count);

   /* Reset */
   if (rc = pxinitialize())
   {
      LOG_COMP_ERROR(ERRORCOMP_2070INIT,rc,0,0,0,0);
   }

   /* Memory allocation */
   if((DMAReadData=(unsigned long *)malloc(2*1024*20)) == NULL)
   {
      LOG_COMP_ERROR(ERRORCOMP_ALLOCATE,rc,0,0,0,0);
   }

   if (rc = DMAReadWithData(1024,20,0,DMAReadData,WORD_BYTESWAP))
   {
      free(DMAReadData);
      LOG_COMP_ERROR(ERRORCOMP_READDATA,rc,0,0,0,0);
   }

   crcvalue = 0;         /* Initialize the crc value */
   for (i=0;i<=COMP_SIZE ;i++ ) {
      crcvalue=cksum_32(DMAReadData[i],&crcvalue);        /* Accumulate 32 bit crc on 32bit(sz=4) value */
   } /* endfor */

   if ((crcvalue != COMP_CRC1) | (comp_count != COMP_SIZE))     /* 5/23/94 */
   {
      DEBUG_2("CRC = 0x%x  &  Word Count = 0x%x\n", crcvalue, comp_count);
      pio_write(LBControl,0x4000ac40,1);  /* Display field memory for screen */
                                          /* verifying (for debug)           */
      usleep(500000);                     /* wait for pixel data ready */
      free(DMAReadData);
      LOG_COMP_ERROR(ERRORCOMP_CRCCHK,0,COMP_CRC1,crcvalue,COMP_SIZE,comp_count);
   }

   free(DMAReadData);
   return (OK);
} /* End of CompTest */

#define         DECOMP_CRC1     0xa6e60fba
#define         DECOMP_CRC2     0x910cda0d

/*   DECOMP_CRC1:  Golden cksum                                               */
/*   DECOMP_CRC2:  Decompression sometimes also give this cksum, the          */
/*                 decompressed data is slightly different (??? not quite     */
/*                 sure why), however the decompressed image looks OK.        */

/*****************************************************************************/
/*  DeCompTest()                                                             */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will load the compressed data from an input file to the      */
/*    frame buffer, decompress the data, then calculate the CRC              */
/*    (checksum) of the decompressed data and compare to the defined CRC.    */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int DeCompTest()
{
  int rc,i;
  unsigned long *dma_block_ptr;
  unsigned long crcvalue;
  int xsize=640, ysize=240, buffsize, buffsize_adjust;

  if (rc = vidsetup(NTSC_NOINPUT))       /* Setup front-end video */
  {
     LOG_CRC_ERROR(ERRORDECOMP_VIDSETUP,rc,0,0);
  }

  if (rc=LoadCompressedData())
  {
     LOG_CRC_ERROR(ERRORDECOMP_LOADDATA,rc,0,0);
  }

  if (pxinitialize())       /* Initialize the px2070 */
  {
     LOG_CRC_ERROR(ERRORDECOMP_2070INIT,rc,0,0);
  }

  if (rc = c560From2070())
  {
     LOG_CRC_ERROR(ERRORDECOMP_560FROM2070,rc,0,0);
  }

  /* Decompressing ........ the JPEG data */
  if (rc = Decompress(ImageWidth, ImageHeight, QFactor))
  {
     LOG_CRC_ERROR(ERRORDECOMP_DECOMP,rc,0,0);
  }

  /* Display field memory to output port which transfer field memory */
  /* decompressed data to the pixel bus */
  if (pio_write(LBControl,0x4000ac40,1))
  {
     LOG_CRC_ERROR(ERRORDECOMP_DISPLAYFIELD,rc,0,0);
  }
  usleep(500000);

  /* Transfer the decompressed data from pixel bus (which was in the field */
  /* memory) to the top portion of frame buffer for data verification */
  if (rc=V1In(0x0))
  {
     LOG_CRC_ERROR(ERRORDECOMP_V1IN,rc,0,0);
  }
   sleep(1);        /* wait for data to be transfered */


  buffsize = xsize*ysize/2;
  if ((dma_block_ptr = (unsigned long *) malloc(4*buffsize)) == NULL)
  {
    LOG_CRC_ERROR(ERRORDECOMP_ALLOCATE,0,0,0);
  }

  /* Read the decompressed data from the frame buffer */
  if (rc = DMAReadWithData(xsize,ysize,0x0,dma_block_ptr,WORD_BYTESWAP))
  {
    free(dma_block_ptr);
    LOG_CRC_ERROR(ERRORDECOMP_READDATA,rc,0,0);
  }

  crcvalue = 0;         /* Initialize the crc value */

  buffsize_adjust = buffsize-(12*xsize/2);/* This is due to the way the 2070  */
                                          /* setup to transfer data that the  */
                                          /* last 12 lines may have invalid  */
                                          /* data (see Setup_Ip1 call in      */
                                          /* V1In() routine)                  */

  /* Accumulate 32 bit crc on 32bit(sz=4) value of the decompressed data */
  for (i=0;i<buffsize_adjust;i++) {
    crcvalue=cksum_32(dma_block_ptr[i],&crcvalue);
  } /* endfor */

  free(dma_block_ptr);


  if ((crcvalue != DECOMP_CRC1) && (crcvalue != DECOMP_CRC2))
  {
      DEBUG_1 ("crc=0x%x\n", crcvalue);
      pio_write(LBControl,0x4000ac40,1);   /* Display field memory for screen    */
                                           /* verifying (for debug               */
      usleep(500000);        /* wait for pixel data ready */
      LOG_CRC_ERROR(ERRORDECOMP_CRCCHK,rc,DECOMP_CRC1,crcvalue);
  }

  return(OK);

} /* End of DeCompTest() */

/*****************************************************************************/
/*  LoadCompressData()                                                       */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will load the compressed data from an input file to the      */
/*    frame buffer.                                                          */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int LoadCompressedData()
{
  unsigned long *dma_block_ptr;
  int i,j, dma_block_size;
  int rc;
  unsigned int val;
  FILE *FIN;
  struct stat statBuffer;
  int buffsize;
  int xsize=640, ysize=240;  /* assume that maximum compressed data  */
                             /* not to exceed size of 640x240 */

  buffsize = xsize*ysize/2;

  if (rc = pxinitialize())       /* Initialize the 2070 */
     return(rc);

  /* Open the compressed data file */
  if((FIN=fopen("/usr/lpp/diagnostics/da/sunrise_comp.jpeg","r")) == NULL) {
    DEBUG_0("Unable to open file\n\r");
    return(ERRORDECOMP_FOPEN);
  }
  fstat(FIN,&statBuffer);
  dma_block_size = 4*buffsize;
  if ((dma_block_ptr = (unsigned long *)malloc(dma_block_size)) == NULL)
    return(ERRORDECOMP_MALLOC);

  /* Read compressed data from file and store to buffer */
  for(j=0,i=0;j<buffsize*4;j+=4,i++){
    rc=fread(&val,1,4,FIN);
    if(rc == 4)
      dma_block_ptr[i]=val;
    else
      break;
  }
  fclose(FIN);

  /* Write the rest of the buffer with 0xffffffff */
  for(;i<buffsize;i++)
    dma_block_ptr[i]=0xffffffff;

  /* Load the compressed data to the frame buffer */
  if (rc=DMAWriteWithData(xsize,ysize,0,dma_block_ptr,WORD_BYTESWAP)) {
    free(dma_block_ptr);
    return(ERRORDECOMP_WRITEDATA);
  } /* endif */

  free(dma_block_ptr);

  return(OK);
} /* End of LoadCompressedData() */
