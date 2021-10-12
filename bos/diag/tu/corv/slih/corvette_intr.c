static char sccsid[] = "@(#)19  1.3  src/bos/diag/tu/corv/slih/corvette_intr.c, tu_corv, bos411, 9428A410j 9/1/93 20:20:38";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: corvette_interrupt
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/intr.h>

#ifdef DIAGEX10
#include "diagex.h"
#elif DIAGEX20
#include "diagex20.h"
#elif DIAGNOSTICS
#include <sys/diagex.h>
#define DIAGEX20 1
#endif


int corvette_interrupt(diagex_handle, data)
unsigned long diagex_handle;
char *data;
{
     unsigned char Interrupt_Status_Register;
     unsigned char Basic_Status_Register;
     int device;

#ifdef DIAGEX10
     Interrupt_Status_Register = diag_io_read(diagex_handle, IOCHAR, 6);
     Basic_Status_Register = diag_io_read(diagex_handle, IOCHAR, 7);
#elif DIAGEX20
     diag_io_read(diagex_handle, IOCHAR, 6, &Interrupt_Status_Register, NULL, INTRKMEM);
     diag_io_read(diagex_handle, IOCHAR, 7, &Basic_Status_Register, NULL, INTRKMEM);
#endif

#ifdef DEBUG
     printf("BSR %x\n",Basic_Status_Register);
#endif

     if (Basic_Status_Register & 0x02) {

      if (*(data+6)==0xf)    /*- force device = 15 in run_self_test function -*/
       device = 0xf;
      else
       device = Interrupt_Status_Register & 0x0f;

      *(data+3) = *(data+2);
      *(data+2) = *(data+1);
      *(data+1) = *data;
      *data = Interrupt_Status_Register;
      *(data+7) = (*(data+7) + 1);

#ifdef DEBUG
     printf("ISR %x\n",Interrupt_Status_Register);
     printf("data: %0x\n",*data);
#endif

#ifdef DIAGEX10
     diag_io_write(diagex_handle, IOCHAR, 4, (0xe0 | device));
#elif DIAGEX20
     diag_io_write(diagex_handle, IOCHAR, 4, (0xe0 | device), NULL, INTRKMEM);
#endif

#ifdef DEBUG
     printf("BSR %x\n",Basic_Status_Register);
#endif

     return INTR_SUCC;
     }
     else
     return INTR_FAIL;
}
