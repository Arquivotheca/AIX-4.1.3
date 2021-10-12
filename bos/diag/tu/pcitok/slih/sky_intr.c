static char sccsid[] = "@(#)34	1.3  src/bos/diag/tu/pcitok/slih/sky_intr.c, tu_pcitok, bos41J, 9519B_all 5/12/95 11:17:17";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: sky_interrupt
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
#include <sys/trcmacros.h>

#include <sys/diagex.h>
#include "sky_err_codes.h"
#include "getsky.h"
#include "skytools.h"

#include "sky_regs.h"
#include "sky_macros.h"

#ifdef DEBUG_P
#define	DEBUGP	printf
#else
#define DEBUGP
#endif

/******************************************************************************
*
* NAME:  sky_interrupt
*
* FUNCTION:  Interrupt handler for the skyline adapter.
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
int sky_interrupt(diag_struc_t *diagex_handle, char *data_area)
{
  ADAPTER_STRUCT   *adapter_info;
  ushort           SISR_status_reg, data, rc;
  ushort           cmd_reg, i;
  int              j, k, ioa;
  
  DEBUGP("\nEntering Interrupt Handler with data_area = %x  ", data_area);
  DEBUGP("diagex_handler = %x\n", diagex_handle);

  /****************************
  * Get access to adapter info
  *****************************/
  adapter_info = (ADAPTER_STRUCT *)data_area;

  DEBUGP("Adapter slot_num = %x\n ", diagex_handle->dds.slot_num);
  DEBUGP("Adapter bus_intr_lvl = %x ", diagex_handle->dds.bus_intr_lvl);
  DEBUGP("Adapter bus_io_addr = %x\n ", diagex_handle->dds.bus_io_addr);
  DEBUGP("number of interrupts = %d\n",adapter_info->num_interrupts);

  i = adapter_info->num_interrupts; 

  /******************************************
  * Get value of interrupt status register
  ******************************************/
  rc = diag_io_read(diagex_handle, IOSHORT, SISR, &data, NULL, INTRKMEM);

  SISR_status_reg = SWAP16(data);
  DEBUGP ("Interrupt Status Register Contents = %x\n", SISR_status_reg);

  if (SISR_status_reg & SISR_MSK) {
	diagex_handle->flag_word = 0;
  	/*****************************************************
         *  An Interrupt for this card has occured, process it.
        ******************************************************/
	while (SISR_status_reg) {
		if (SISR_status_reg & ADAPT_CHK) {
			if (!(adapter_info->int_pending & ADAPT_CHK))
			{
				adapter_info->int_type[i++] = ADAPT_CHK;
				adapter_info->int_pending |= ADAPT_CHK;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM,
			    SWAP16(~ADAPT_CHK), NULL, INTRKMEM);
			SISR_status_reg &= ~ADAPT_CHK;
			DEBUGP("Adapter Check Interrupt Occurred\n");
		} else if (SISR_status_reg & SRB_RSP) {
			if (!(adapter_info->int_pending & SRB_RSP))
			{
				adapter_info->int_type[i++] = SRB_RSP;
				adapter_info->int_pending |= SRB_RSP;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~SRB_RSP), NULL, INTRKMEM);
			SISR_status_reg &= ~SRB_RSP;
			diagex_handle->flag_word |= SRB_RSP;
			DEBUGP("SRB_RSP Interrupt Occurred\n");
		} else if (SISR_status_reg & MISR_INT) {
			if (!(adapter_info->int_pending & MISR_INT))
			{
				adapter_info->int_type[i++] = MISR_INT;
				adapter_info->int_pending |= MISR_INT;
			}

			rc = diag_io_read(diagex_handle, IOSHORT, MISR,
			    			&data, NULL, INTRKMEM);

			adapter_info->cmd_reg |= SWAP16(data);

			diag_io_write(diagex_handle, IOSHORT, MISR, 
					~data, NULL, INTRKMEM);

			SISR_status_reg &= ~MISR_INT;
			if ((adapter_info->cmd_reg & RECEIVE_MSK) &&
                            (adapter_info->cmd_reg & XMIT_DONE_MSK_2)) {
				diagex_handle->flag_word |= MISR_INT;
			}

			DEBUGP("MISR_INT Interrupt Occurred = 0x%08x\n",
				adapter_info->cmd_reg);
		} else if (SISR_status_reg & SCB_STAT) {
			if (!(adapter_info->int_pending & SCB_STAT))
			{
				adapter_info->int_type[i++] = SCB_STAT;
				adapter_info->int_pending |= SCB_STAT;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~SCB_STAT), NULL,INTRKMEM);
			SISR_status_reg &= ~SCB_STAT;
			DEBUGP("SCB_STAT Interrupt Occurred\n");
		} else if (SISR_status_reg & SCB_CTL) {
			if (!(adapter_info->int_pending & SCB_CTL))
			{
				adapter_info->int_type[i++] = SCB_CTL;
				adapter_info->int_pending |= SCB_CTL;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~SCB_CTL), NULL, INTRKMEM);
			SISR_status_reg &= ~SCB_CTL;
			DEBUGP("SCB_CTL Interrupt Occurred\n");
		} else if (SISR_status_reg & SCB_SIG) {
			if (!(adapter_info->int_pending & SCB_SIG))
			{
				adapter_info->int_type[i++] = SCB_SIG;
				adapter_info->int_pending |= SCB_SIG;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~SCB_SIG), NULL, INTRKMEM);
			SISR_status_reg &= ~SCB_SIG;
			DEBUGP("SCB_SIG Interrupt Occurred\n");
		} else if (SISR_status_reg & TIMER_EXP) {
			adapter_info->int_type[i++] = TIMER_EXP;
			adapter_info->int_pending |= TIMER_EXP;
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM,
				    SWAP16(~TIMER_EXP), NULL, INTRKMEM);
			SISR_status_reg &= ~TIMER_EXP;
			DEBUGP("TIMER_EXP Interrupt Occurred\n");
		} else if (SISR_status_reg & LAP_PRTY) {
			if (!(adapter_info->int_pending & LAP_PRTY))
			{
				adapter_info->int_type[i++] = LAP_PRTY;
				adapter_info->int_pending |= LAP_PRTY;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM,
				     SWAP16(~LAP_PRTY), NULL, INTRKMEM);
			SISR_status_reg &= ~LAP_PRTY;
			DEBUGP("LAP_PRTY Interrupt Occurred\n");
		} else if (SISR_status_reg & LAP_ACC) {
			if (!(adapter_info->int_pending & LAP_ACC))
			{
				adapter_info->int_type[i++] = LAP_ACC;
				adapter_info->int_pending |= LAP_ACC;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~LAP_ACC), NULL, INTRKMEM);
			SISR_status_reg &= ~LAP_ACC;
			DEBUGP("LAP_ACC Interrupt Occurred\n");
		} else if (SISR_status_reg & ARB_FREE) {
			if (!(adapter_info->int_pending & ARB_FREE))
			{
				adapter_info->int_type[i++] = ARB_FREE;
				adapter_info->int_pending |= ARB_FREE;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				     SWAP16(~ARB_FREE), NULL, INTRKMEM);
			SISR_status_reg &= ~ARB_FREE;
			DEBUGP("ARB_FREE Interrupt Occurred\n");
		} else if (SISR_status_reg & ARB_CMD) {
			if (!(adapter_info->int_pending & ARB_CMD))
			{
				adapter_info->int_type[i++] = ARB_CMD;
				adapter_info->int_pending |= ARB_CMD;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~ARB_CMD), NULL, INTRKMEM);
			diag_io_write(diagex_handle, IOSHORT, LISR_SUM, 
					SWAP16(0x02), NULL, INTRKMEM);
			SISR_status_reg &= ~ARB_CMD;
			diagex_handle->flag_word |= ARB_CMD;
			DEBUGP("ARB_CMD Interrupt Occurred\n");
		} else if (SISR_status_reg & TRB_RSP) {
			if (!(adapter_info->int_pending & TRB_RSP))
			{
				adapter_info->int_type[i++] = TRB_RSP;
				adapter_info->int_pending |= TRB_RSP;
			}
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				      SWAP16(~TRB_RSP), NULL, INTRKMEM);
			SISR_status_reg &= ~TRB_RSP;
			DEBUGP("TRB_RSP Interrupt Occurred\n");
		} else {
			if (i < 16)
				adapter_info->int_type[i++] = SISR_status_reg;
			diag_io_write(diagex_handle, IOSHORT, SISR_RUM, 
				     SWAP16(~SISR_MSK), NULL, INTRKMEM);
			SISR_status_reg &= ~SISR_MSK;
			DEBUGP("Unknown Interrupt Occurred\n");
		}

	} /* end while */

	adapter_info->num_interrupts = i;

	/*********************************
       	*  Wake up sleeping application
       	*********************************/
	if (diagex_handle->sleep_flag) {
		e_wakeup((int *)&diagex_handle->sleep_word);
	}
	DEBUGP("Exiting Interrupt Handler\n");

	return (INTR_SUCC);
  } else {
	return (INTR_FAIL);
  }

} /* end sky_intr */

