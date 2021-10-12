static char sccsid[] = "@(#)97  1.3  src/bos/diag/tu/artic/tu052.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:31";
/* COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS: tu052
 *
 */

#include <stdio.h>
#include <artictst.h>
#include <sys/param.h>

#define ADAPTER_TO_SU           0
#define SU_TO_ADAPTER           1
#define EXTENDED_BM_TEST        0x20


/*
 * NAME: tu052
 *
 * FUNCTION: Bus Master DMA Test Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will initiate the Bus Master DMA test unit.  This
 *          Test will pull data from system unit memory to adapter memory
 *          and push data from adapter memory to system unit memory. The
 *          Bus Master function is not presently supported by the Artic
 *          device driver; this function is included here to provide
 *          additional hardware testing for diagnostics. There is a
 *          special "diagnostic" device driver/config method that is
 *          required for this TU to execute properly. There is also a
 *          prerequisite that TU019 be executed before executing this test
 *          unit.
 *
 * RETURNS: The return code from the Bus Master DMA test.
 *
 */
unsigned char *bm_buffer;
unsigned char *bm_buffer2;

int tu052 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern int start_diag_tu();
        ICABUFFER ob;
        unsigned short src;
        unsigned long physaddr;
        int rc = 0;
        int i;
        void swap_long();

        if ((bm_buffer = (char *) malloc(256)) == NULL)
        {
           return(MALLOC_ERR);
        }

        if ((bm_buffer2 = (char *) malloc(256)) == NULL)
        {
           return(MALLOC_ERR);
        }


        /* Get diag tasks output buffer */
        if (src = icaoutbuf(fdes, &ob))
        {
           free(bm_buffer);
           free(bm_buffer2);
           return(DRV_ERR);
        }

        /* Initialize buffer prior to Busmaster xfer */
        for (i = 0; i < 256; i++)
        {
           bm_buffer[i] = i;
        }

        /* Set up the IOCC for the DMA xfer */
        if (src = icadmasetup(fdes, SU_TO_ADAPTER, bm_buffer, 256, &physaddr))
        {
           free(bm_buffer);
           free(bm_buffer2);
           return(DRV_ERR);
        }

        /* Send address of buffer to diag task */
        physaddr |= (unsigned long) ((unsigned long) bm_buffer & 0xFFF);
        swap_long(&physaddr);

        if (src = icawritemem(fdes, 4, ob.page, ob.offset, 0xFF, &physaddr))
        {
           rc = DRV_ERR;
        }
        else
           /* Execute test */
           rc = start_diag_tu(fdes, tucb_ptr, GATEARRAY_COM_PCODE, GATEP_ER);

        /* Release DMA resources */
        if (src = icadmarel(fdes))
        {
           rc = DRV_ERR;
        }

        if (rc)
        {
           free(bm_buffer);
           free(bm_buffer2);
           return(rc);
        }

        /* Initialize buffer prior to Busmaster xfer */
        memset(bm_buffer2, 0, 256);

        /* Set up the IOCC for the DMA xfer */
        if (src = icadmasetup(fdes, ADAPTER_TO_SU, bm_buffer2, 256, &physaddr))
        {
           free(bm_buffer);
           free(bm_buffer2);
           return(DRV_ERR);
        }

        /* Send address of buffer to diag task */
        physaddr |= (unsigned long) ((unsigned long) bm_buffer2 & 0xFFF);
        swap_long(&physaddr);

        if (src = icawritemem(fdes, 4, ob.page, ob.offset, 0xFF, &physaddr))
        {
           rc = DRV_ERR;
        }
        else
           /* Execute test */
           rc = start_diag_tu(fdes, tucb_ptr, EXTENDED_BM_TEST, GATEP_ER);

        /* Release DMA resources */
        if (src = icadmarel(fdes))
        {
           rc = DRV_ERR;
        }

        if (rc)
        {
           free(bm_buffer);
           free(bm_buffer2);
           return(rc);
        }


        /* Verify data received via Busmaster xfer */

        if (memcmp(bm_buffer, bm_buffer2, 256)) {
           rc = GATEP_ER;
        }

        free(bm_buffer);
        free(bm_buffer2);

        return(rc);
   }

/*
 * NAME: swap_long
 *
 * FUNCTION: Convert long integer from Big Endian to Little Endian format
 *
 * RETURNS: Void
 *
 */
void swap_long(dest)
unsigned long *dest;
   {
      char *p, c;

      p = (char *) dest;
      c = *p;
      *p = *(p+3);
      *(p+3) = c;
      c = *(p+1);
      *(p+1) = *(p+2);
      *(p+2) = c;
   }

