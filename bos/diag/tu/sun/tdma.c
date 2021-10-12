static char sccsid[] = "@(#)39  1.9  src/bos/diag/tu/sun/tdma.c, tu_sunrise, bos411, 9437A411a 9/13/94 08:44:32";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: DMAComplete
 *              DMARead
 *              DMAReadWithData
 *              DMASetup
 *              DMAWrite
 *              DMAWriteWithData
 *              FrameBufferDMATest
 *              StartDMA
 *              EMC_DMATest
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
/*****************************************************************************

Function(s) - DMA Test

Module Name :  tdma.c

DMA test writes/reads/compares byte values to check all bit positions
in the entire frame buffer.

*****************************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/diagex.h>
#include "sun_tu_type.h"
#include "error.h"
#include "suntu.h"
#include "px2070.h"
#include "iodvp.h"
#include "sun_slih.h"

#define         BUFFSIZE_READ   261632  /* for image size 1024x511 */
#define         BUFFSIZE_WRITE  261632

unsigned long *dma_block_ptr;
unsigned long phys_addr;

extern diag_struc_t * handle;  /* handle for Sunrise       */
extern struct sun_intr_data sun_data;

/*****************************************************************************/
/*  StartDMA                                                                 */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Setup Pixel and Miami to do DMA and start DMA operation.               */
/*                                                                           */
/*  INPUT:    bus_address - Pointer to the host data buffer to be transfered.*/
/*            size      - Number of bytes to be transfered.                  */
/*            dir       - Direction of DMA transfer:                         */
/*                        0 - from Card to Host                              */
/*                        1 - from Host to Card                              */
/*                                                                           */
/*  OUTPUT:   return value     - OK if DMA transfer sucessful, ERROR if NOT. */
/*****************************************************************************/
int StartDMA(ulong bus_address, int size, int dir)
{
  int i,rc;
  unsigned int val;

  /* Setup Miami to do DMA & start Busmaster DMA transfer */

  if (rc=pio_write (MIbaseaddr+_CAR1, DMAbaseaddr, 1)) return(rc);
  /* Set the physical address to SAR reg */
  if (rc=pio_write (MIbaseaddr+_SAR1, bus_address, 1)) return(rc);
  /* Set number of byte count for transfer */
  if (rc=pio_write (MIbaseaddr+_BCR1, size, 1)) return(rc);

  /* Set the Channel Control reg to start DMA */
  /* CCR = 000010001x1 (Binary), where x=direction bit */

  /* STARTS DMA .... */
  if (rc=pio_write (MIbaseaddr+_CCR1, (0x45 | (dir<<1)), 1)) return(rc);

  /* Wait for DMA to finish */
  if (rc = diag_watch4intr(handle, 0x1, 5))
  {
     diag_dma_flush(handle,bus_address);
     if (rc=DMAComplete (bus_address)) return(ERRORDMA_COMPLETE);
     DEBUG_1 ("MiamiIntrStatus = 0x%x\n", sun_data.MiamiIntrStatus);
     return(ERRORDMA_WAIT4INTR);  /* Wait for Miami interrupt */
  }

  if (rc = diag_dma_flush(handle,bus_address)) return(rc);

  return(OK);

} /* End of StartDMA */

/*****************************************************************************/
/*  DMASetup()                                                               */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Read data (by DMA) from the frame buffer and store it in the buffer    */
/*                                                                           */
/*  INPUT:    *buffer_ptr  - pointer to user buffer.                         */
/*            *bus_address - ptr to the physical address returned from       */
/*                           the 'diag_dma_master' call.                     */
/*            size      - Number of bytes to be transfered.                  */
/*            dir       - Direction of DMA transfer:                         */
/*                        0 - from Card to Host                              */
/*                        1 - from Host to Card                              */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int DMASetup (ulong *buffer_ptr, ulong *bus_address, int size, int dir)
{
  int rc;

   if (dir) {
   rc = diag_dma_master(handle,DMA_WRITE_ONLY,buffer_ptr,
                           size, bus_address);
   } else {
   rc = diag_dma_master(handle,(DMA_READ | DMA_NOHIDE),buffer_ptr,
                           size, bus_address);
   } /* endif */
   if (rc) {
     return (rc);
   } /* endif */
   return (OK);

}

/*****************************************************************************/
/*  DMAComplete()                                                            */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Call 'diag_dma_complete' to clean up DMA mess.                         */
/*                                                                           */
/*  INPUT:   *bus_address - ptr to the physical address                      */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int DMAComplete (ulong bus_address)
{
  int rc;
  if (rc = diag_dma_complete(handle,bus_address))
  {
     return(rc);
  }
  return  (OK);
}

#ifdef INUSE   /* NOT need this function for now */
/*****************************************************************************/
/*  DMARead()                                                                */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Read data (by DMA) from the 2070 frame buffer at a given starting      */
/*    address and write data to a file if request.                           */
/*                                                                           */
/*  INPUT:    xsize - row size of a frame.                                   */
/*            ysize - col size of a frame.                                   */
/*            fb_startaddr - starting address.                               */
/*            file_wanted  - '1' if want to write the read data to a file    */
/*                                                                           */
/*  OUTPUT:   return value     - OK if DMA read sucessful, ERROR if NOT.     */
/*****************************************************************************/
int DMARead(int xsize, int ysize, unsigned int fb_startaddr,
            int file_wanted, unsigned int lbctrl_mask)
{
  int i, dma_block_size, BuffSize;
  int rc;
  unsigned int val;
  FILE *FILEOUT;

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  if ((dma_block_ptr = (unsigned long *)malloc(dma_block_size)) == NULL)
     return(ERRORDMA);
  for (i=0;i<BuffSize;dma_block_ptr[i++]=i*3);    /* initialize host buffer */
  if (rc = DMASetup (dma_block_ptr, &phys_addr, dma_block_size, 0))
  {   free(dma_block_ptr);
      return(ERRORDMA);
  }
  /* Enable DMA, Micro Channel interrupt, Multiplexed Streaming Data thru Miami */
   pio_mcwrite (_SCP, 0x3, 1);
   pio_read(MIbaseaddr+_PROC_CFG,&val,1);
   val = val | 0xc00;
   pio_write (MIbaseaddr+_PROC_CFG,val,1);
   /* Clear the CBSP register */
   pio_write (MIbaseaddr+_CBSP, 0, 1);
   if (rc = pio_read(LBControl,&val,1)) return(rc);
   val = (val & 0xfffff3ff) | lbctrl_mask;
   if (rc = pio_write (LBControl,val,1)) return(rc);

   sun_data.CodecIntrStatus = 0;
   sun_data.ScalarIntrStatus = 0;
   sun_data.MiamiIntrStatus = 0;

  pxinitialize();
  UpdateGen();

  pxDMARead(xsize, ysize, fb_startaddr);
   usleep(30000);

/*  Start DMA ... */
  sun_data.dataarea[INTR_MODE] = WAIT4INTR; /* Set to WAIT4INTR */
  startTime=times(&tmsstart);
  if (rc = StartDMA (phys_addr, dma_block_size, 0))
  { free(dma_block_ptr);
    return(ERRORDMA);
  }
  endTime = times(&tmsend);

  DEBUG_1("DMA Bandwidth: %7.4f\r\n",
        (((double)4*BuffSize)/((endTime-startTime)/
         (double)sysconf(_SC_CLK_TCK)))/(double)1000000);

  if (file_wanted == 1) {
    FILEOUT = fopen("/tmp/dmatohost.out", "w");
    for (i=0;i<BuffSize; i++)
      fprintf(FILEOUT, "%08x\n", dma_block_ptr[i]);
    fclose(FILEOUT);
  }

  /* Disable Micro Channel interrupt thru Miami */
   pio_mcwrite (_SCP, 0x0, 1);
   pio_read(MIbaseaddr+_PROC_CFG,&val,1);
   val = val & 0xFBFF;
   pio_write (MIbaseaddr+_PROC_CFG,val,1);

   if (rc= DMAComplete (phys_addr))
   {  free(dma_block_ptr);
      return(ERRORDMA);
   }
   free(dma_block_ptr);

  return(OK);
} /* End of DMARead */
#endif

/*****************************************************************************/
/*  DMAWrite()                                                               */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Write data (by DMA) from a given pattern to the frame buffer at a      */
/*    given starting address.                                                */
/*                                                                           */
/*  INPUT:    xsize        - row size of a frame.                            */
/*            ysize        - col size of a frame.                            */
/*            fb_startaddr - starting address.                               */
/*            *data        - ptr to the array of data patterns.              */
/*            array_size   - array size of data[].                           */
/*            number_rep   - number of times to repeat the data pattern      */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int DMAWrite(int xsize, int ysize, unsigned int fb_startaddr,
             unsigned int *data, int array_size, int number_rep,
             unsigned int lbctrl_mask)
{
  int i=0, j, k, dma_block_size, BuffSize;
  int rc;
  unsigned int val;
  FILE *FILEOUT;

  /* Reset 2070 */
  if (rc = pxinitialize()) return(rc);

  if (rc = pio_read(LBControl,&val,1)) return(rc);
  val = (val & 0xfffff3ff) | lbctrl_mask;
  if (rc = pio_write (LBControl,val,1)) return(rc);

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  if ((dma_block_ptr = (unsigned long *)malloc(dma_block_size)) == NULL)
     return(ERRORDMA_MALLOC);

  while (i<BuffSize) {
     for (j=0;j<array_size ;j++ ) {
       for (k=0;k<number_rep ;k++ ) {
          dma_block_ptr[i++] = data[j];
          if (i>=BuffSize) {            /* check to make sure 'i' not         */
                                        /* overrun the 'dma_block_ptr'        */
            k=number_rep; j=array_size; /* if yes, exit                       */
          } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endwhile */

  if (rc = DMASetup (dma_block_ptr, &phys_addr, dma_block_size, 1))
  {   free(dma_block_ptr);
      return(ERRORDMA_SETUP);
  }

  /* Enable DMA, Micro Channel interrupt, Multiplexed Streaming Data thru Miami */
   if (rc=pio_mcwrite (_SCP, 0x3, 1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }
   if (rc=pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }
   val = val | 0xc00;
   if (rc=pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }

   /* Clear the CBSP register */
   if (rc=pio_write (MIbaseaddr+_CBSP, 0, 1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }
   sun_data.CodecIntrStatus = 0;
   sun_data.ScalarIntrStatus = 0;
   sun_data.MiamiIntrStatus = 0;

   if (rc=pxDMAWrite(xsize, ysize, fb_startaddr))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(ERRORDMA_PXWRITE);
   }

   sun_data.dataarea[INTR_MODE] = WAIT4INTR; /* Set to WAIT4INTR */
   /* Start DMA ..... */
   if (rc = StartDMA (phys_addr, dma_block_size, 1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(ERRORDMA_START);
   }
   /* Disable Micro Channel interrupt thru Miami */
   if (pio_mcwrite (_SCP, 0x0, 1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }
   if (pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }
   val = val & 0xFBFF;
   if (pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       free(dma_block_ptr);
       return(rc);
   }

   if (rc= DMAComplete (phys_addr))
   {   free(dma_block_ptr);
       return(ERRORDMA_COMPLETE);
   }

   free(dma_block_ptr);

  return(OK);
} /* End of DMAWrite */


/*****************************************************************************/
/*  DMAReadWithData()                                                        */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Read data (by DMA) from the frame buffer and store it in the buffer    */
/*                                                                           */
/*  INPUT:    xsize        - row size of a frame.                            */
/*            ysize        - col size of a frame.                            */
/*            fb_startaddr - starting address.                               */
/*            *data        - ptr to the read data.                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int DMAReadWithData(int xsize, int ysize, unsigned int fb_startaddr,
                    unsigned long *DMAdata, unsigned int lbctrl_mask)
{
  int i, dma_block_size, BuffSize;
  int rc;
  unsigned int val;
  FILE *FILEOUT;

  /* Reset 2070 */
  if (rc = pxinitialize()) return(rc);

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  for (i=0;i<BuffSize;DMAdata[i++]=i*3);    /* initialize host buffer */
  if (rc = DMASetup (DMAdata, &phys_addr, dma_block_size, 0))
     return(ERRORDMA_SETUP);

  /* Enable DMA, Micro Channel interrupt, Multiplexed Streaming Data thru Miami */
   if (rc=pio_mcwrite (_SCP, 0x3, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   if (rc=pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   val = val | 0xc00;
   if (rc=pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   /* Clear the CBSP register */
   if (rc=pio_write (MIbaseaddr+_CBSP, 0, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   if (rc = pio_read(LBControl,&val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   val = (val & 0xfffff3ff) | lbctrl_mask;
   if (rc = pio_write (LBControl,val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   sun_data.CodecIntrStatus = 0;
   sun_data.ScalarIntrStatus = 0;
   sun_data.MiamiIntrStatus = 0;

  if (rc=pxDMARead(xsize, ysize, fb_startaddr))
   {   DMAComplete (phys_addr);
       return(ERRORDMA_PXREAD);
   }
  usleep(30000);

  sun_data.dataarea[INTR_MODE] = WAIT4INTR; /* Set to WAIT4INTR */

  if (rc = StartDMA (phys_addr, dma_block_size, 0))
   {   DMAComplete (phys_addr);
       return(ERRORDMA_START);
   }

   /* Disable Micro Channel interrupt thru Miami */
   if (rc = pio_mcwrite (_SCP, 0x0, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   if (rc = pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   val = val & 0xFBFF;
   if (rc = pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   if (rc= DMAComplete (phys_addr))
      return(ERRORDMA_COMPLETE);

  return(OK);
} /* DMAReadWithData */

/*****************************************************************************/
/*  DMAWriteWithData()                                                       */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Write data (by DMA) from a given buffer to the frame buffer at a       */
/*    given starting address.                                                */
/*                                                                           */
/*  INPUT:    xsize        - row size of a frame.                            */
/*            ysize        - col size of a frame.                            */
/*            fb_startaddr - starting address.                               */
/*            *data        - ptr to the data buffer.                         */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int DMAWriteWithData(int xsize, int ysize, unsigned int fb_startaddr,
             unsigned long *data, unsigned int lbctrl_mask)
{
  int i=0, j, k, dma_block_size, BuffSize;
  int rc;
  unsigned int val;
  FILE *FILEOUT;

  /* Reset 2070 */
  if (rc = pxinitialize()) return(rc);

  if (rc = pio_read(LBControl,&val,1)) return(rc);
  val = (val & 0xfffff3ff) | lbctrl_mask;
  if (rc = pio_write (LBControl,val,1)) return(rc);

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;

  if (rc = DMASetup (data, &phys_addr, dma_block_size, 1))
      return(ERRORDMA_SETUP);

  /* Enable DMA, Micro Channel interrupt, Multiplexed Streaming Data thru Miami */
   if (rc=pio_mcwrite (_SCP, 0x3, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   if (rc=pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   val = val | 0xc00;
   if (rc=pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   /* Clear the CBSP register */
   if (rc=pio_write (MIbaseaddr+_CBSP, 0, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   sun_data.CodecIntrStatus = 0;
   sun_data.ScalarIntrStatus = 0;
   sun_data.MiamiIntrStatus = 0;

   if (rc=pxDMAWrite(xsize, ysize, fb_startaddr))
   {   DMAComplete (phys_addr);
       return(ERRORDMA_PXWRITE);
   }

  sun_data.dataarea[INTR_MODE] = WAIT4INTR; /* Set to WAIT4INTR */
  /* Start DMA ... */
  if (rc = StartDMA (phys_addr, dma_block_size, 1))
   {   DMAComplete (phys_addr);
       return(ERRORDMA_START);
   }
   /* Disable Micro Channel interrupt thru Miami */
   if (pio_mcwrite (_SCP, 0x0, 1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   if (pio_read(MIbaseaddr+_PROC_CFG,&val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }
   val = val & 0xFBFF;
   if (pio_write (MIbaseaddr+_PROC_CFG,val,1))
   {   DMAComplete (phys_addr);
       return(rc);
   }

   if (rc= DMAComplete (phys_addr))
      return(ERRORDMA_COMPLETE);

  return(OK);
} /* DMAWriteWithData */

/*****************************************************************************/
/*  FrameBufferDMATest()                                                     */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Write/Read/Verify the 2070 frame buffer by DMA operation.              */
/*    Also test the HALF WORD and NO byte swap modes.                        */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if DMA verify sucessful, ERROR if NOT.   */
/*****************************************************************************/
#define         PAT_SIZE        5
int FrameBufferDMATest()
{
  int zero_data=0;
  int i, j, k, dma_block_size, BuffSize;
  unsigned long *DMAReadData;
  int rc;
  int xsize=1024;      /* Setup to test the entire frame buffer 1024x510 */
  int ysize=510;
  int number_rep=1;
  unsigned int expected_test_pattern[PAT_SIZE];
  unsigned int test_pattern[PAT_SIZE]={0x11223344,0x55667788,
                                       0x99aabbcc,0xddeeff00,0x10101010};

  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
     LOG_MEM_ERROR(ERRORDMA_2070INIT,rc,0,0,0);
  }

  /* DMA Write the top portion (start address 0) of frame buffer */
  /* with the given pattern */
  if (rc = DMAWrite(xsize,ysize,0,test_pattern,PAT_SIZE,number_rep,WORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORDMA_WRITEDATA,rc,0,0,0);
  }

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  if ((DMAReadData = (unsigned long *)malloc(dma_block_size)) == NULL)
  {
     LOG_MEM_ERROR(ERRORDMA_ALLOCATE,0,0,0,0);
  }

  /* DMA read back the data from the frame buffer */
  if (rc = DMAReadWithData(xsize,ysize,0,DMAReadData,WORD_BYTESWAP))
  {
     free(DMAReadData);
     LOG_MEM_ERROR(ERRORDMA_READDATA,rc,0,0,0);
  } /* endif */

  /* Verify data */
  i = 0;
  while (i<BuffSize) {
     for (j=0;j<PAT_SIZE ;j++ ) {
       for (k=0;k<number_rep ;k++ ) {
          if (DMAReadData[i++] != test_pattern[j]) {
              DEBUG_3 ("Failed compare test: expected=0x%x, received=0x%x, with i=%d\n",
                      test_pattern[j], DMAReadData[i-1], i-1);
              free(DMAReadData);
              LOG_MEM_ERROR(ERRORDMA_COMPARE,0,test_pattern[j],DMAReadData[i],i);
            } /* endif */
          if (i>=BuffSize) {            /* check to make sure 'i' not         */
                                        /* overrun the 'DMAReadData[]'        */
            k=number_rep; j=PAT_SIZE; /* if yes, exit                       */
          } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endwhile */

/* Setup to test the HALF WORD byte swap and NO byte swap modes in DMA */
  xsize = 640;  /* test just 2 lines of data */
  ysize = 2;
  BuffSize = (xsize*ysize)/2;

/* 1. Test the HALF WORD byte swap mode */
/* Clear the system storage and frame memory */
  for (i=0;i<BuffSize ;i++ ) {
    DMAReadData[i] = 0;
  } /* endfor */
  /* Write the frame buffer (start address 0x0) with 0 */
  if (rc = DMAWrite(xsize,ysize,0x0,&zero_data,1,1,WORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORDMA_WRITEZERO,rc,0,0,0);
  }

  /* DMA Write the top portion (start address 0) of frame buffer */
  /* with the given pattern in Half Word Byte Swap mode */
  if (rc = DMAWrite(xsize,ysize,0,test_pattern,PAT_SIZE,number_rep,HALFWORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORDMA_WRITEDATA,rc,0,0,0);
  }

  /* DMA read back the data from the frame buffer */
  /* use WORD_BYTESWAP mode due to micro channel data swap */
  if (rc = DMAReadWithData(xsize,ysize,0,DMAReadData,WORD_BYTESWAP))
  {
     free(DMAReadData);
     LOG_MEM_ERROR(ERRORDMA_READDATA,rc,0,0,0);
  } /* endif */

/* Calculate the expected test patterns due to HALF WORD byte swapping mode */
  for (i=0;i<PAT_SIZE;i++ ) {
    expected_test_pattern[i] = ((test_pattern[i] & 0xff) << 16) |
                               ((test_pattern[i] & 0xff00) << 16) |
                               ((test_pattern[i] & 0xff0000) >> 16) |
                               ((test_pattern[i] & 0xff000000) >> 16);
  } /* endfor */

  /* Verify data */
  i = 0;
  while (i<BuffSize) {
     for (j=0;j<PAT_SIZE ;j++ ) {
       for (k=0;k<number_rep ;k++ ) {
          if (DMAReadData[i++] != expected_test_pattern[j]) {
              printf ("Failed compare test in HALFWORD BYTE SWAP: expected=0x%x, received=0x%x, with i=%d\n",
                       expected_test_pattern[j], DMAReadData[i-1], i-1);
              free(DMAReadData);
              LOG_MEM_ERROR(ERRORDMA_HALFWORDSWAP,0,expected_test_pattern[j],
                            DMAReadData[i-1],i-1);
            } /* endif */
          if (i>=BuffSize) {            /* check to make sure 'i' not         */
                                        /* overrun the 'DMAReadData[]'        */
            k=number_rep; j=PAT_SIZE; /* if yes, exit                       */
          } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endwhile */

/* 2. Test NO byte swap mode */
/* Clear the system storage and frame memory */
  for (i=0;i<BuffSize ;i++ ) {
    DMAReadData[i] = 0;
  } /* endfor */
  /* Write the frame buffer (start address 0x0) with 0 */
  if (rc = DMAWrite(xsize,ysize,0x0,&zero_data,1,1,WORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORDMA_WRITEZERO,rc,0,0,0);
  }

  /* DMA Write the top portion (start address 0) of frame buffer */
  /* with the given pattern in NO Byte Swap mode */
  if (rc = DMAWrite(xsize,ysize,0,test_pattern,PAT_SIZE,number_rep,NO_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORDMA_WRITEDATA,rc,0,0,0);
  }

  /* DMA read back the data from the frame buffer */
  /* use WORD_BYTESWAP mode due to micro channel data swap */
  if (rc = DMAReadWithData(xsize,ysize,0,DMAReadData,WORD_BYTESWAP))
  {
     free(DMAReadData);
     LOG_MEM_ERROR(ERRORDMA_READDATA,rc,0,0,0);
  } /* endif */

/* Calculate the expected test patterns due to NO byte swapping mode */
  for (i=0;i<PAT_SIZE;i++ ) {
    expected_test_pattern[i] = ((test_pattern[i] & 0xff) << 8) |
                               ((test_pattern[i] & 0xff00) >> 8) |
                               ((test_pattern[i] & 0xff0000) << 8) |
                               ((test_pattern[i] & 0xff000000) >> 8);
  } /* endfor */

  /* Verify data */
  i = 0;
  while (i<BuffSize) {
     for (j=0;j<PAT_SIZE ;j++ ) {
       for (k=0;k<number_rep ;k++ ) {
          if (DMAReadData[i++] != expected_test_pattern[j]) {
              printf ("Failed compare test in NO BYTE SWAP: expected=0x%x, received=0x%x, with i=%d\n",
                       expected_test_pattern[j], DMAReadData[i-1], i-1);
              free(DMAReadData);
              LOG_MEM_ERROR(ERRORDMA_NOSWAP,0,expected_test_pattern[j],
                            DMAReadData[i-1],i-1);
            } /* endif */
          if (i>=BuffSize) {            /* check to make sure 'i' not         */
                                        /* overrun the 'DMAReadData[]'        */
            k=number_rep; j=PAT_SIZE; /* if yes, exit                       */
          } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endwhile */

  free(DMAReadData);
  return(OK);
} /* End of FrameBufferDMATest */

/*****************************************************************************/
/*  EMC_DMATest()                                                            */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Write/Read/Verify the 2070 frame buffer by DMA operation and also      */
/*    display the image from the frame buffer to the output video port.      */
/*    This is for EMC testing purpose.                                       */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if DMA verify sucessful, ERROR if NOT.   */
/*****************************************************************************/
#define         EMC_PAT_SIZE        8
#define         EMC_DMA_CRC         0x67166EDE
int EMC_DMATest()
{
  int i, j, k, dma_block_size, BuffSize;
  unsigned long *DMAReadData;
  int rc;
  int xsize=640;      /* Setup to test the entire frame buffer 640x480 */
  int ysize=480;
  int number_rep=40;
  unsigned long crcvalue;
  unsigned int test_pattern[EMC_PAT_SIZE]=
                              {0xbe7fbe00,0xaa2caa8e,0x8a9a8a2a,0x7747773a,
                               0x58b758c7,0x466346d5,0x24d32472,0xff80ff80};
   /* Setup front-end video */
   if (rc = vidsetup(NTSC_NOINPUT))
   {
      LOG_CRC_ERROR(ERROREMCDMA_VIDSETUP,rc,0,0);
   }

  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
     LOG_CRC_ERROR(ERROREMCDMA_2070INIT,rc,0,0);
  }

  /* DMA Write the top portion (start address 0) of frame buffer */
  /* with the given pattern */
  if (rc = DMAWrite(xsize,ysize,0,test_pattern,EMC_PAT_SIZE,number_rep,WORD_BYTESWAP))
  {
     LOG_CRC_ERROR(ERROREMCDMA_WRITEDATA,rc,0,0);
  }

   /* Move image data from top portion of frame buffer OUT to the pixel bus */
   /* thru V1 port of 2070                                                   */
   if (rc=V1Out(0))
   {
      LOG_CRC_ERROR(ERROREMCDMA_V1OUT,rc,0,0);
   }
   sleep(1);        /* wait for pixel data ready */

   /* Capture the image to field memory from the V1 port */
/* may not need this
   if (rc = pio_write(LBControl, 0x4000ac0a,1))
   {
      LOG_CRC_ERROR(ERROREMCDMA_STOREFIELD,rc,0,0);
   }
   usleep(500000);
*/

  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  if ((DMAReadData = (unsigned long *)malloc(dma_block_size)) == NULL)
  {
     LOG_CRC_ERROR(ERROREMCDMA_ALLOCATE,0,0,0);
  }

  /* DMA read back the data from the frame buffer */
  if (rc = DMAReadWithData(xsize,ysize,0,DMAReadData,WORD_BYTESWAP)) {
     free(DMAReadData);
     LOG_CRC_ERROR(ERROREMCDMA_READDATA,rc,0,0);
  } /* endif */

  /* Accumulate 32 bit crc on 32bit(sz=4) value of the decompressed data */
  for (i=0;i<BuffSize;i++) {
    crcvalue=cksum_32(DMAReadData[i],&crcvalue);
  } /* endfor */

  printf ("EMC_DMA crcvalue = 0x%x \n", crcvalue);

/*  For EMC, NOT checking for error for now
  if (crcvalue != EMC_DMA_CRC)
  {
      DEBUG_1 ("crc=0x%x\n", crcvalue);
      LOG_CRC_ERROR(ERROREMCDMA_CRCCHK,rc,EMC_DMA_CRC,crcvalue);
  }
*/

  free(DMAReadData);
  return(OK);
} /* End of EMC_DMATest */

