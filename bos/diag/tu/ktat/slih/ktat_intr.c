static char sccsid[] = "@(#)78	1.2  src/bos/diag/tu/ktat/slih/ktat_intr.c, tu_ktat, bos41J, 9522A_all 5/24/95 10:57:44";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: ktat_interrupt
 *		swap32
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

/*** header files ***/
#include <sys/adspace.h> 
#include <sys/ioacc.h> 
#include <sys/types.h>
#include <sys/sleep.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <stdio.h>

#include "kent_defs.h"
#include "kent_tu_type.h"
#include <sys/diagex.h>

#ifdef DEBUG_P
#define	DEBUGP	printf
#else
#define DEBUGP
#endif

/******************************************************************************
*
* NAME:  ktat_interrupt
*
* FUNCTION:  Interrupt handler for the klickitat adapter.
*
* INPUT PARAMETERS:     diagex_handle = handle returned for diagex_open
*                       data         =  data passed to handler during
*                                       initialization.
*
* EXECUTION ENVIRONMENT:  Interrupt
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: diagex_io_read, diagex_io_write
*
******************************************************************************/

int ktat_interrupt(diag_struc_t *diagex_handle, struct int_data *data_area)
{
  int   data, work1, i = 0;
  int   j, k, ioa, rc;
  
/*  DEBUGP("\nEntering Interrupt Handler with data_area = %x\n  ", data_area); */
/*  DEBUGP("diagex_handler = %x\n", diagex_handle);  */

  /*************************************************
  * Need slight delay to catch multiple interrupts.
  **************************************************/
  for (j = 0, k = 0; j < 3000; j++) {
  	k+=j;
  }

/*
  DEBUGP("Adapter slot_num = %x\n ", diagex_handle->dds.slot_num);
  DEBUGP("Adapter bus_intr_lvl = %x ", diagex_handle->dds.bus_intr_lvl);
  DEBUGP("Adapter bus_io_addr = %x\n ", diagex_handle->dds.bus_io_addr);
  DEBUGP("Sleep word = %x\n ", diagex_handle->sleep_word);
  DEBUGP("Sleep flag = %x\n ", diagex_handle->sleep_flag);
*/

/*****************************************************
*   Get value of interrupt status register - csr0
*****************************************************/

  rc = diag_io_write(diagex_handle, IOLONG, csr0, rap, (void *)NULL, INTRKMEM);
  rc = diag_io_read(diagex_handle, IOLONG, rdp, &data, (void *)NULL, INTRKMEM);

  work1 = swap32(data);
  DEBUGP ("Interrupt Status Register Contents = %x\n", work1);
/*
  data_area->status = work1;
  diagex_handle-> flag_word = 0;
*/
  if ((work1 & INTR_MSK) == INTR_MSK)
  {
/***********************************************************
*       An Interrupt for this card has occured, process it.
***********************************************************/
/*
    data_area->intr_count++;
    if((work1 & TINT_INTR) == TINT_INTR)
      data_area->xmit_count++;
    if((work1 & RINT_INTR) == RINT_INTR)
      data_area->rec_count++;
*/
    data_area->pending = work1;
    diagex_handle-> flag_word = INTR_MSK;

/*  reset the interrupt(s)   */

    rc = diag_io_write(diagex_handle, IOLONG, rdp, data, (void *)NULL, INTRKMEM);


/****************************************************
*         Wake up sleeping application
****************************************************/

    if (diagex_handle->sleep_flag)
    {
      e_wakeup((int *)&diagex_handle->sleep_word);
    }
/*    DEBUGP("Exiting Interrupt Handler\n"); */

    return (INTR_SUCC);
  }
  else
  {
    return (INTR_FAIL);
  }

} /* end ktat_intr */



swap32(data)
  int data;
{
  int s_data;
  s_data = ((data & 0xff000000) >> 24) | ((data & 0x00ff0000) >> 8) |
           ((data & 0x0000ff00) << 8) | ((data & 0x000000ff) << 24);
    return(s_data);
}

