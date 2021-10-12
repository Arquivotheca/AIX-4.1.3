static char sccsid[] = "@(#)23  1.10  src/bos/kernext/scsi/pscsiddb.c, sysxscsi, bos41J, 9520A_all 5/16/95 18:25:31";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Bottom Half
 *
 * FUNCTIONS:
 * psc_chip_register_init  psc_chip_register_reset  psc_config_adapter
 * psc_logerr              psc_read_reg             psc_write_reg
 * psc_read_POS            psc_write_POS            psc_strategy
 * psc_start               psc_alloc_STA
 * psc_free_STA            psc_alloc_TCW            psc_free_TCW
 * psc_alloc_resources     psc_free_resources       psc_iodone
 * psc_enq_active          psc_enq_wait             psc_enq_wait_resources
 * psc_enq_abort_bdr       psc_deq_active           psc_deq_wait
 * psc_deq_wait_resources  psc_deq_abort_bdr        psc_dump
 * psc_dumpstrt            psc_dump_intr            psc_dumpwrt
 * psc_dump_dev            psc_cdt_func             psc_fail_cmd
 * psc_delay               psc_intr                 psc_issue_cmd
 * psc_check_wait_queue    psc_cleanup_reset        psc_cleanup_reset_error
 * psc_epow                psc_command_watchdog     psc_command_reset_scsi_bus
 * psc_find_devinfo        psc_fail_free_resources  psc_issue_abort_bdr
 * psc_scsi_parity_error   psc_prep_main_script     psc_patch_async_switch_int
 * psc_set_disconnect      psc_patch_io_wait_int_on psc_patch_io_wait_int_off
 * psc_reselect_router     psc_verify_neg_answer    psc_reset_iowait_jump
 * psc_restore_iowait_jump psc_update_dataptr       psc_handle_extended_message
 * psc_issue_abort_script  psc_issue_bdr_script     psc_trace_1
 * psc_trace_2             psc_trace_3
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        pscsiddb.c                                              */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Bottom Half Source File         */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/scsi.h>
#include <sys/pscsidd.h>
/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Global pageable device driver static data areas                      */
/************************************************************************/
extern ULONG PSC_SCRIPT[];

/************************************************************************/
/* Global pinned device driver static data areas                        */
/************************************************************************/
/* global static structure to hold the driver's EPOW handler struct     */
struct    intr    epow_struct;

/* global driver component dump table pointer                           */
struct  psc_cdt_table    *psc_cdt = NULL;

/* global pointer for adapter structure                                 */
struct    adapter_def    adp_str;

int    adp_str_inited = FALSE;

ULONG	A_io_wait_Used[] = {
	0x00000003
};

ULONG	A_abort_msg_error_Used[] = {
	0x00000207
};

ULONG	A_abort_select_failed_Used[] = {
	0x0000020d
};

ULONG	A_abort_io_complete_Used[] = {
	0x0000020b
};

ULONG	A_unknown_reselect_id_Used[] = {
	0x00000015
};

ULONG	A_uninitialized_reselect_Used[] = {
	0x00000029,	0x0000003d,
	0x00000051,	0x00000065,
	0x00000079,	0x0000008d,
	0x000000a1,	0x000000b5
};

ULONG	R_cmd_bytes_out_count_Used[] = {
	0x000000c6
};

ULONG	R_data_byte_count_Used[] = {
	0x000000f4,	0x000000fc
};

ULONG	R_dummy_int_Used[] = {
	0x00000175
};

ULONG	R_ext_msg_size_Used[] = {
	0x0000012a
};

ULONG	data_addr_Used[] = {
	0x000000f5,	0x000000fd
};

ULONG	scsi_0_lun_Used[] = {
	0x00000019,	0x0000001b,
	0x0000001d,	0x0000001f,
	0x00000021,	0x00000023,
	0x00000025,	0x00000027
};

ULONG	scsi_1_lun_Used[] = {
	0x0000002d,	0x0000002f,
	0x00000031,	0x00000033,
	0x00000035,	0x00000037,
	0x00000039,	0x0000003b
};

ULONG	scsi_2_lun_Used[] = {
	0x00000041,	0x00000043,
	0x00000045,	0x00000047,
	0x00000049,	0x0000004b,
	0x0000004d,	0x0000004f
};

ULONG	scsi_3_lun_Used[] = {
	0x00000055,	0x00000057,
	0x00000059,	0x0000005b,
	0x0000005d,	0x0000005f,
	0x00000061,	0x00000063
};

ULONG	scsi_4_lun_Used[] = {
	0x00000069,	0x0000006b,
	0x0000006d,	0x0000006f,
	0x00000071,	0x00000073,
	0x00000075,	0x00000077
};

ULONG	scsi_5_lun_Used[] = {
	0x0000007d,	0x0000007f,
	0x00000081,	0x00000083,
	0x00000085,	0x00000087,
	0x00000089,	0x0000008b
};

ULONG	scsi_6_lun_Used[] = {
	0x00000091,	0x00000093,
	0x00000095,	0x00000097,
	0x00000099,	0x0000009b,
	0x0000009d,	0x0000009f
};

ULONG	scsi_7_lun_Used[] = {
	0x000000a5,	0x000000a7,
	0x000000a9,	0x000000ab,
	0x000000ad,	0x000000af,
	0x000000b1,	0x000000b3
};

ULONG	INSTRUCTIONS	= 0x00000119;

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_chip_register_init                                          */
/*                                                                        */
/* FUNCTION:  Initializes adapter chip registers.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to initialize the adapter chip reg-     */
/*         isters to before command processing is to begin.               */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      None                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_chip_register_init()
{

    int     rc;

    rc = psc_write_reg((uint) SCNTL0, (char) SCNTL0_SIZE,
		       (uchar) SCNTL0_INIT);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) SCNTL1, (char) SCNTL1_SIZE,
		       (char) SCNTL1_INIT);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) SIEN, (char) SIEN_SIZE, (uchar) SIEN_MASK);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) SCID, (char) SCID_SIZE,
		       (uchar) (0x01 << adp_str.ddi.card_scsi_id));
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) SXFER, (char) SXFER_SIZE, (uchar) SYNC_VAL);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) DMODE, (char) DMODE_SIZE,
		       (uchar) DMODE_INIT);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) DIEN, (char) DIEN_SIZE, (uchar) DIEN_MASK);
    if (rc != 0)
	return (EIO);

    rc = psc_write_reg((uint) DCNTL, (char) DCNTL_SIZE,
		       (uchar) DCNTL_INIT);
    if (rc != 0)
	return (EIO);

    return (PSC_NO_ERR);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_chip_register_reset                                         */
/*                                                                        */
/* FUNCTION:  Resets SCSI chip.                                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by the interrupt handler.         */
/*                                                                        */
/* NOTES:  This routine is called to reset the adapter chip after a       */
/*         catastrophic error has occurred.                               */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      None                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      d_mask          d_clear                                           */
/*      d_master                                                          */
/*                                                                        */
/**************************************************************************/
void
psc_chip_register_reset(int reset_flag)

{

    int     i, dma_addr;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_chip_register_reset routine.\n")
    TRACE_1 ("in chipR", reset_flag)
    if (reset_flag & PSC_RESET_CHIP)
    {
	/* Reset SCSI in Component reset register   */
	iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
	(void) BUS_PUTLX((long *) (iocc_addr + PSC_COMP_RESET),
		          PSC_COMP_OFF);
	(void) BUS_PUTLX((long *) (iocc_addr + PSC_COMP_RESET),
	                  PSC_COMP_ON);
	IOCC_DET(iocc_addr);

	/* Reset the DMA channel for the chip */
	d_mask(adp_str.channel_id);
	d_clear(adp_str.channel_id);    /* free DMA channel */

	/* reset the POS registers for the chip */
	psc_config_adapter();

	/* Disable dma and SCSI interrupts from the chip */
	(void) psc_write_reg(SIEN, SIEN_SIZE, 0x00);
	(void) psc_write_reg(DIEN, DIEN_SIZE, 0x00);

	/* Assert a chip reset by writing to the DCNTL register */
	(void) psc_write_reg(DCNTL, DCNTL_SIZE, 0x01);
	psc_delay(1);
	(void) psc_write_reg(DCNTL, DCNTL_SIZE, 0x00);

	/* Call routine to re-initialize all chip registers  */
	(void) psc_chip_register_init();

	/* re-initialize DMA channel */
	adp_str.channel_id = d_init(adp_str.ddi.dma_lvl,
				    DMA_INIT, adp_str.ddi.bus_id);
	d_unmask(adp_str.channel_id);
    }

    if (reset_flag & PSC_RESET_DMA)
    {
	/* lock in the dma areas for STA and SCRIPTS */
	dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
			    adp_str.STA_tcw);
	d_master(adp_str.channel_id, DMA_NOHIDE, adp_str.
		 STA[0].sta_ptr, (size_t) PAGESIZE,
		 &adp_str.xmem_STA, (char *) dma_addr);

	dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
			    adp_str.STA_tcw + 1);
	d_master(adp_str.channel_id, DMA_NOHIDE, (char *)
		 adp_str.SCRIPTS[0].script_ptr, (size_t) PAGESIZE,
		 &adp_str.xmem_SCR, (char *) dma_addr);
	for (i = 0; i < MAX_SCRIPTS; i++)
	{
	    if (adp_str.SCRIPTS[i].script_ptr != NULL)
	    {
		dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
				    adp_str.SCRIPTS[i].TCW_index);
		d_master(adp_str.channel_id, DMA_NOHIDE, (char *)
			 adp_str.SCRIPTS[i].script_ptr,
			 (size_t) PAGESIZE,
			 &adp_str.xmem_SCR, (char *) dma_addr);
	    }
	}
    }
    TRACE_1 ("outchipR", 0)
    DEBUG_0 ("Leaving psc_chip_register_reset routine.\n")
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_config_adapter                                      */
/*                                                                      */
/* FUNCTION:    Adapter Configuration Routine                           */
/*                                                                      */
/*      This internal routine performs actions required to make         */
/*      the driver ready for the first open to the adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine must be called from the process level only.        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      Global_adapter_ddi                                              */
/*                                                                      */
/* INPUTS:                                                              */
/*      None                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for unsuccessful operation                    */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      psc_delay                                                       */
/*                                                                      */
/************************************************************************/
int
psc_config_adapter()
{
    int     ret_code;
    int     IPL_timer_count;
    int     val0, val1;
    uchar   int_level, data;

    DEBUG_0 ("Entering psc_config_adapter routine.\n")
    ret_code = 0;       /* default to no error */

    /* check here to make sure planer board is ready */
    /* for POS register set-up.                      */
    IPL_timer_count = 0;

    /* loop waiting for either card ready or timeout */

    while (TRUE)
    {	/* loop as long as it takes */
	val0 = psc_read_POS((uint) POS0);
	val1 = psc_read_POS((uint) POS1);
	if ((val0 == -1) || (val1 == -1))
	{	/* permanent I/O error */
	    ret_code = EIO;
	    break;
	}
	else
	{	/* no I/O error */
	    if (((uchar) val0 == POS0_VAL) && ((uchar) val1 == POS1_VAL))
		/* POS0 AND POS1 have both been set correctly */
		break;
	    /* card ready for POS reg loading */
	    else
	    {	/* planer IPL is still going */
		if (IPL_timer_count >= IPL_MAX_SECS)
		{	/* timed-out waiting for planer diag */
		    psc_logerr(ERRID_SCSI_ERR1,
			       UNKNOWN_ADAPTER_ERROR, 10, 0,
			       NULL, FALSE);
		    ret_code = EIO;
		    break;
		}
		/* */
		/* delay, then come back and check POS0 and   */
		/* POS1 again.                                */
		/* */
		psc_delay(1);
		IPL_timer_count++;
	    }	/* planer IPL is still going */
	}       /* no I/O error */
    }	/* loop as long as it takes */
    if (ret_code == 0)
    {	/* no errors so far, continue to initialize POS registers */
	/* Set up interrupt value for the chip */
	if (adp_str.ddi.int_lvl == 5)
	    int_level = 0x80;
	else
	    int_level = 0;
	if (psc_write_POS((uint) POS2, (int_level | 0x03)))
	    ret_code = EIO;
    }	/* no errors so far, continue to initialize POS registers */
    if (ret_code == 0)
    {
	data = psc_read_POS((uint) POS4);
	if (data == REG_FAIL)
	    ret_code = EIO;
	else
	    if (psc_write_POS((uint) POS4, ((data & 0xf0) |
					    adp_str.ddi.dma_lvl)))
		ret_code = EIO;
    }
    /* Clear any pending interrupts that may have been */
    /* left over from ipl.                             */
    if (psc_write_reg(ISTAT, ISTAT_SIZE, 0x00) == REG_FAIL)
	ret_code = EIO;
    else
	if (psc_read_reg(DSTAT, DSTAT_SIZE) == REG_FAIL)
	    ret_code = EIO;
	else
	    if (psc_read_reg(SSTAT0, SSTAT0_SIZE) == REG_FAIL)
		ret_code = EIO;

    DEBUG_0 ("Leaving psc_config_adapter routine.\n")
    return (ret_code);

}  /* end psc_config_adapter */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_logerr                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Error Logging Routine                    */
/*                                                                      */
/*      This routine provides a common point through which all driver   */
/*      error logging passes.                                           */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      error_log_def - adapter driver error logging information        */
/*                      structure                                       */
/*                                                                      */
/* INPUTS:                                                              */
/*      errid   - the error log unique id for this entry                */
/*      add_hw_status - additional hardware status for this entry       */
/*      errnum  - a unique value which identifies which error is        */
/*                causing this call to log the error                    */
/*      data1   - additional, error dependent data                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      errsave                                                         */
/*                                                                      */
/************************************************************************/
void
psc_logerr(
	   int errid,
	   int add_hw_status,
	   int errnum,
	   int data1,
	   struct dev_info * dev_ptr,
	   uchar read_regs)
{
    struct error_log_def log;
    caddr_t iocc_addr;
    int     cmd_length, cmd_filler;

    DEBUG_0 ("Entering psc_logerr routine.\n")
    if (adp_str.errlog_enable)
    {	/* set up info and log */
	bzero(&log, sizeof(struct error_log_def));
	log.errhead.error_id = (uint) errid;
	bcopy(&adp_str.ddi.resource_name[0], &log.errhead.
	      resource_name[0], 8);
	log.data.diag_validity = 0;
	log.data.diag_stat = 0;
	if (add_hw_status != 0)
	{
	    log.data.ahs_validity = 0x01;
	    log.data.ahs = (uchar) add_hw_status;
	}
	log.data.un.card2.errnum = (uint) errnum;

	/* see if additional data is needed due to error id */
	if (((uint) errid == (uint) ERRID_SCSI_ERR7) ||
	    ((uint) errid == (uint) ERRID_SCSI_ERR8))
	    log.data.sys_rc = (uint) data1;

	/* indicate which register caused this error logging to be called */
	if ((((uint) errid == (uint) ERRID_SCSI_ERR1) ||
	     ((uint) errid == (uint) ERRID_SCSI_ERR2) ||
	     ((uint) errid == (uint) ERRID_SCSI_ERR10)) &&
	    (data1 != 0))
	    log.data.un.card2.reg_validity = data1;

	/* see if additional data is needed due to add_hw_status value */
	if ((uint) add_hw_status & DMA_ERROR)
	    log.data.sys_rc = (uint) data1;
	/* Get POS register data, directly read the regs   */
	/* here to avoid another call to error log         */
	/* routine due to error in read functions.         */
	iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS0), (char *) &log.data.un.card2.pos0_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS1), (char *) &log.data.un.card2.pos1_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS2), (char *) &log.data.un.card2.pos2_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS6), (char *) &log.data.un.card2.pos6_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS3), (char *) &log.data.un.card2.pos3_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS4), (char *) &log.data.un.card2.pos4_val);
	BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
		   POS5), (char *) &log.data.un.card2.pos5_val);
	IOCC_DET(iocc_addr);

	if (read_regs)
	{	/* on an interrupt level, read these regs */
	    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	    BUS_GETCX(iocc_addr + ISTAT, (char *) &log.data.un.card2.istat_val);
	    BUS_GETCX(iocc_addr + SSTAT1, (char *) &log.data.un.card2.sstat1_val);
	    BUS_GETCX(iocc_addr + SSTAT2, (char *) &log.data.un.card2.sstat2_val);
	    BUS_GETCX(iocc_addr + SDID, (char *) &log.data.un.card2.sdid_val);
	    BUS_GETCX(iocc_addr + SCID, (char *) &log.data.un.card2.scid_val);
	    BUS_GETLX((long *) (iocc_addr + DBC), 
		      (long *) &log.data.un.card2.dbc_val);
	    BUS_GETLX((long *) (iocc_addr + DSP), 
		      (long *) &log.data.un.card2.dsp_val);
	    BUS_GETLX((long *) (iocc_addr + DSPS), 
		      (long *) &log.data.un.card2.dsps_val);
	    BUSIO_DET(iocc_addr);

	    log.data.un.card2.dbc_val = word_reverse(
					   log.data.un.card2.dbc_val);
	    log.data.un.card2.dsp_val = word_reverse(
					   log.data.un.card2.dsp_val);
	    log.data.un.card2.dsps_val = word_reverse(
					   log.data.un.card2.dsps_val);
	    if ((dev_ptr != NULL) &&
		(dev_ptr->active != NULL))
	    {	/* found valid ptr to the job the MOST */
		/* likely caused us to go here */
		cmd_length = (int) dev_ptr->active->scsi_command.scsi_length;
		log.data.un.card2.scsi_cmd[0] = dev_ptr->active->scsi_command.
		    scsi_cmd.scsi_op_code;
		log.data.un.card2.scsi_cmd[1] = dev_ptr->active->scsi_command.
		    scsi_cmd.lun;
		for (cmd_filler = 2; cmd_filler < cmd_length; cmd_filler++)
		    log.data.un.card2.scsi_cmd[cmd_filler] =
			dev_ptr->active->scsi_command.scsi_cmd.
			scsi_bytes[cmd_filler - 2];
		log.data.un.card2.target_id = dev_ptr->scsi_id;
		log.data.un.card2.queue_state = dev_ptr->queue_state;
		log.data.un.card2.cmd_activity_state =
					       dev_ptr->cmd_activity_state;
		log.data.un.card2.resource_state = dev_ptr->resource_state;
	    }
	}

	/* log the error here */
	errsave(&log, sizeof(struct error_log_def));
    }
    DEBUG_0 ("Leaving psc_logerr routine.\n")

}  /* end psc_logerr */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_read_reg                                            */
/*                                                                      */
/* FUNCTION:    Access the specified register.                          */
/*                                                                      */
/*      This routine reads from a selected adapter                      */
/*      register and returns the data, performing                       */
/*      appropriate error detection and recovery.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to read a single register; 8,24 or 32 bit                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected register                       */
/*      reg_size - size of register in bytes                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 32-bits of data if good or recovered error        */
/*                  = 32-bits of -1 if permanent I/O error encountered  */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
psc_read_reg(
	     uint offset,
	     char reg_size)
{
    int     ret_code, pio_rc, retry_count;
    int     val1, old_level;
    char    valc1;
    caddr_t iocc_addr;

    retry_count = 0;
    DEBUG_1 ("Entering psc_read_reg routine.REG=0x%x\n", offset);
    /* disable our interupts while reading registers */
    /* to prevent our interrupts killing the read    */
    old_level = i_disable(INTMAX);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    if (reg_size == 1)
	pio_rc = BUS_GETCX((iocc_addr + offset), &valc1);
    else
	pio_rc = BUS_GETLX((long *)(iocc_addr + offset),(long *) &val1);
    while ((retry_count < PIO_RETRY_COUNT) && (pio_rc != 0))
    {
	if ((offset == ISTAT) ||	/* Don't want to retry these */
	    (offset == DSTAT) ||	/* registers because rereading */
	    (offset == SSTAT0) ||	/* them corrupts the value */
	    (offset == SSTAT1) ||	/* orginally in them.  We will */
	    (offset == SSTAT2))		/* just log the error */
	    break;
	else
	    retry_count++;
	if (reg_size == 1)
	    pio_rc = BUS_GETCX((iocc_addr + offset), &valc1);
	else
	    pio_rc = BUS_GETLX((long *)(iocc_addr + offset), (long *) &val1);
    }

    if (pio_rc != 0)
    {
        struct pio_except infop;

        ASSERT(pio_rc == EXCEPT_IO);
        getexcept((struct except *) &infop);
        /* log as unrecoverable I/O bus problem */
        if ((infop.pio_csr & PIO_EXCP_MASK) == PIO_EXCP_DPRTY)
            psc_logerr(ERRID_SCSI_ERR1, PIO_RD_DATA_ERR, 15,
                       offset, NULL, FALSE);
        else
	    psc_logerr(ERRID_SCSI_ERR1, PIO_RD_OTHR_ERR, 15,
                       offset, NULL, FALSE);
        ret_code = REG_FAIL;
    }
    else
    {
	if (retry_count > 0)
	{
	    psc_logerr(ERRID_SCSI_ERR2, PIO_RD_OTHR_ERR, 20,
		       offset, NULL, FALSE);
	    TRACE_1 ("read-reg err", offset)
	}
	if (reg_size == 1)
	    ret_code = valc1;
	else
	    if (reg_size == 4)
		ret_code = val1;
	    else	/* reg size = 3 */
		ret_code = (val1 >> 8) & 0x00ffffff;
    }
    DEBUG_0 ("Leaving psc_read_reg routine.\n")
    BUSIO_DET(iocc_addr);
    i_enable(old_level);
    return (ret_code);
}  /* end psc_read_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_write_reg                                           */
/*                                                                      */
/* FUNCTION:    Store passed data in specified register.                */
/*                                                                      */
/*      This routine loads a selected adapter register with the         */
/*      passed data value, performing appropriate error detection       */
/*      and recovery.                                                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to load a single register.                                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected POS reg                        */
/*      reg_size - size of register in bytes                            */
/*      data    - value to be loaded                                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
psc_write_reg(
	      uint offset,
	      char reg_size,
	      int data)
{
    int     ret_code, pio_rc, retry_count;
    int     old_level;
    caddr_t iocc_addr;

    ret_code = 0;
    retry_count = 0;

    if (offset == SXFER)
    {
      TRACE_1 ("w SXFER ", data)
    }

    DEBUG_2 ("Entering psc_write_reg routine.REG=0x%x 0x%x\n", offset, data)
    /* disable our interupts while writing registers */
    /* to prevent our interrupts killing the read    */
            old_level = i_disable(INTMAX);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    if (reg_size == 1)
	pio_rc = BUS_PUTCX((iocc_addr + offset), (char) data);
    else
	pio_rc = BUS_PUTLX((long *)(iocc_addr + offset), data);
    while ((retry_count < PIO_RETRY_COUNT) && (pio_rc != 0))
    {
	retry_count++;
	if (reg_size == 1)
	    pio_rc = BUS_PUTCX((iocc_addr + offset), (char) data);
	else
	    pio_rc = BUS_PUTLX((long *)(iocc_addr + offset), data);
    }
    if (pio_rc != 0)
    {
        struct pio_except infop;

        ASSERT(pio_rc == EXCEPT_IO);
        getexcept((struct except *) &infop);
        /* log as unrecoverable I/O bus problem */
        if ((infop.pio_csr & PIO_EXCP_MASK) == PIO_EXCP_DPRTY)
	    psc_logerr(ERRID_SCSI_ERR1, PIO_WR_DATA_ERR,
                       25, offset, NULL, FALSE);
        else
	    psc_logerr(ERRID_SCSI_ERR1, PIO_WR_OTHR_ERR,
                       25, offset, NULL, FALSE);

        ret_code = REG_FAIL;
    }
    else
        if (retry_count > 0)
        {
            psc_logerr(ERRID_SCSI_ERR2, PIO_WR_OTHR_ERR, 30,
		       offset, NULL, FALSE);
	}
    DEBUG_0 ("Leaving psc_write_reg routine.\n")
    BUSIO_DET(iocc_addr);
    i_enable(old_level);
    return (ret_code);

}  /* end psc_write_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_read_POS                                            */
/*                                                                      */
/* FUNCTION:    Access the specified register.                          */
/*                                                                      */
/*      This routine reads from a selected adapter POS                  */
/*      register and returns the data, performing                       */
/*      appropriate error detection and recovery.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to read a single register 8 bit POS                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected register                       */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 32-bits of data if good or recovered error        */
/*                  = 32-bits of -1 if permanent I/O error encountered  */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
psc_read_POS(
	     uint offset)
{
    int     count, ret_code, rc;
    int     old_level;
    char    val1, val2;
    caddr_t iocc_addr;

    ret_code = 0;
    rc = 0;
    count = 0;
    val1 = 0x1;
    val2 = 0x0;

    DEBUG_1 ("Entering psc_read_POS routine.REG=0x%x\n", offset);

    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
    old_level = i_disable(INTMAX);

    while ((count < PIO_RETRY_COUNT) && ((val1 != val2) || (rc != 0)))
    {	/* bus miscompare, retry loop */
	rc = BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
		       &val1);
	rc |= BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
			&val2);
	count++;
    }

    if (count >= PIO_RETRY_COUNT)
    {	/* log as unrecoverable I/O bus problem */
	psc_logerr(ERRID_SCSI_ERR1, PIO_RD_DATA_ERR, 35,
		   offset, NULL, FALSE);
	ret_code = -1;
    }
    else
	if (count > 1)
	    /* if we went into the while loop on an error, count will be
	       inc'd */
	{	/* log as recovered I/O bus problem */
	    psc_logerr(ERRID_SCSI_ERR2, PIO_RD_DATA_ERR, 40,
		       offset, NULL, FALSE);
	    ret_code = val1;	/* no error in bus access */
	    DEBUG_1 ("In read_POS, count = 0x%x\n", count);
	}
	else
	    ret_code = val1;	/* no error in bus access */

    DEBUG_0 ("Leaving psc_read_POS routine.\n")
    i_enable(old_level);
    IOCC_DET(iocc_addr);
    return (ret_code);
}  /* end psc_read_POS */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_write_POS                                           */
/*                                                                      */
/* FUNCTION:    Store passed data in specified POS register.            */
/*                                                                      */
/*      This routine loads a selected adapter POS reg with the          */
/*      passed data value, performing appropriate error detection       */
/*      and recovery.                                                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to load a single POS register.                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected POS reg                        */
/*      data    - value to be loaded                                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
psc_write_POS(
	      uint offset,
	      int data)

{
    int     count, ret_code, rc, i, j;
    int     temp, old_level;
    char    data_byte[5], val1;
    caddr_t iocc_addr;

    ret_code = 0;
    count = 0;
    val1 = 0x0;

    DEBUG_2 ("Entering psc_write_POS routine.REG=0x%x 0x%x\n", offset, data)
    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);

    old_level = i_disable(INTMAX);
    /* strip out each byte so we can load the register */
    /* byte by byte (32 bits = 4 bytes)                */
    j = 4;
    for (i = 1; i < 5; i++)
    {
	temp = data << ((j - 1) * 8);
	temp &= 0xFF000000;
	temp = temp >> 24;
	data_byte[i] = (char) (temp & 0xFF);
	j--;
    }

    /* load register  */
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
	      data_byte[1]);

    /* left justify the value so that we can compare it */
    /* to the value we get from BUS_GETSTRX             */
    rc = BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
		   &val1);
    /************************************************************/
    /* There has been problems with BUS_PUT not putting values  */
    /* correctly on the first try, so lets compare what was     */
    /* written to what the data is. If it is incorrect then try */
    /* a few more times to put it correctly                     */
    /************************************************************/
    while ((count < PIO_RETRY_COUNT) && ((val1 != data_byte[1]) ||
					 (rc != 0)))
    {	/* bus miscompare, retry loop */
	/* load register */
	BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
		  data_byte[1]);
	rc = BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
		       &val1);
	count++;
    }

    if (count >= PIO_RETRY_COUNT)
    {	/* log as unrecoverable I/O bus problem */
	psc_logerr(ERRID_SCSI_ERR1, PIO_WR_DATA_ERR, 45,
		   offset, NULL, FALSE);
	ret_code = -1;
    }
    else
	if (count > 0)
	    /* if we went into the while loop on an error, count will be
	       inc'd */
	{	/* log as recovered I/O bus problem */
	    psc_logerr(ERRID_SCSI_ERR2, PIO_WR_DATA_ERR, 50,
		       offset, NULL, FALSE);
	    DEBUG_1 ("In write_POS, count = 0x%x\n", count);
	}

    DEBUG_0 ("Leaving psc_write_POS routine.\n")
    i_enable(old_level);
    IOCC_DET(iocc_addr);
    return (ret_code);

}  /* end psc_write_POS */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_strategy                                            */
/*                                                                      */
/* FUNCTION: Scsi Chip Device Driver Strategy Routine                   */
/*                                                                      */
/*      This routine will accept commands from the device heads         */
/*      and queue them up to the appropriate device structure for       */
/*      execution.  The command, in the form of scbufs, contains        */
/*      a bufstruct at the beginning of the structure and pertinent     */
/*      data appended after cannot exceed the maximum transfer size     */
/*      allowed.  Note that the av_forw field of the bufstruct MUST     */
/*      be NULL as chained commands are not supported.                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - scsi chip unique data structure                   */
/*      scbuf - input command structure.                                */
/*                                                                      */
/* INPUTS:                                                              */
/*      bp - command buffer pointer                                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0 - For good completion                                         */
/*      ERRNO - value otherwise                                         */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      EIO - Adapter not defined or initialized                        */
/*          - Adapter not opened                                        */
/*          - Device not started                                        */
/*          - Device is being stopped                                   */
/*      ENXIO - Device queue is currently awaiting a resume cmd         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable, i_enable                                             */
/*                                                                      */
/************************************************************************/
int
psc_strategy(
	     struct sc_buf * bp)
{
    int     ret_code;
    dev_t   devno;
    int     dev_index;
    int     new_pri;
    struct dev_info *dev_ptr;

    DEBUG_0 ("Entering psc_strategy routine \n")

    ret_code = 0;
    devno = bp->bufstruct.b_dev;
    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_STRATEGY, ret_code, devno, bp,
	    bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
	    bp->bufstruct.b_bcount);

    if ((adp_str_inited) && (adp_str.opened))
    {
	/* passed the init and the open test */
	/* get index into device table for this device */
	dev_index = INDEX(bp->scsi_command.scsi_id,
			  (bp->scsi_command.scsi_cmd.lun) >> 5);
	dev_ptr = adp_str.device_queue_hash[dev_index];
	TRACE_2 ("in strat", (int) bp, (int) dev_ptr)

	/* Disable interrupt to keep this single threaded. */
	new_pri = i_disable(adp_str.ddi.int_prior);

	/* miscellaneous validation of request */
	if ((dev_ptr != NULL) &&
	    (bp->bufstruct.b_bcount <= adp_str.max_request)
	    && (dev_ptr->queue_state != STOPPING)
	    && (bp->resvd1 == 0x0))
	{
	    /* passed check of device structure info */

	    /* The HALTED flag is set when the adapter driver */
	    /* encounters an error.  Do not continue the pro- */
	    /* cessing if the caller has not set RESUME flag. */
	    if ((dev_ptr->queue_state != HALTED) ||
		((dev_ptr->queue_state == HALTED)
		 && (bp->flags & SC_RESUME)))
	    {
		/* passed queue_state and RESUME tests */

		/* if we get here, device is not halted anymore */
		/* set normal state */
		dev_ptr->queue_state = ACTIVE;
		bp->bufstruct.av_forw = NULL;	/* only accept one cmd */
		bp->bufstruct.b_work = 0;
		/* Initialize the following flag to the "no */
		/* error" state.                            */
		bp->bufstruct.b_error = 0;
		bp->bufstruct.b_flags &= ~B_ERROR;
		bp->bufstruct.b_resid = 0;
		bp->status_validity = 0;
		bp->general_card_status = 0;
		bp->scsi_status = 0;

		/* Queue this command to the device's pending      */
		/* queue for processing by psc_start.              */
		if (dev_ptr->head_pend == NULL)
		{	/* queue is empty  */
		    dev_ptr->head_pend = bp;
		    dev_ptr->tail_pend = bp;
		    if ((!(dev_ptr->restart_in_prog)) &&
			(!(dev_ptr->flags & CHK_SCSI_ABDR)) &&
			(dev_ptr->active == NULL))
			psc_start(dev_ptr);
		}
		else
		{	/* pending queue not empty */
		    /* point last cmd's av_forw at the new request */
		    dev_ptr->tail_pend->bufstruct.av_forw = (struct buf *) bp;
		    dev_ptr->tail_pend = bp;
		}
	    }
	    else
	    {	/* check queue_state and RESUME flag failed */
		bp->bufstruct.b_error = ENXIO;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->status_validity = 0;
		iodone((struct buf *) bp);
		ret_code = ENXIO;
	    }
	}
	else
	{	/* device structure validation failed */
	    bp->bufstruct.b_error = EIO;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->status_validity = SC_ADAPTER_ERROR;
	    bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	    iodone((struct buf *) bp);
	    ret_code = EIO;
	}
    }
    else
    {	/* scsi chip has not been initialized or opened */
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	bp->status_validity = SC_ADAPTER_ERROR;
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	iodone((struct buf *) bp);
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, EIO, devno);
	TRACE_1 ("out strat", EIO)
	return (EIO);
    }

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, devno);
    DEBUG_0 ("Leaving psc_strategy routine \n")
    TRACE_1 ("out strat", 0)
    i_enable(new_pri);
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_start                                               */
/*                                                                      */
/* FUNCTION: Scsi Chip Device Driver Start Command Routine              */
/*                                                                      */
/*      This routine is called from the strategy routine when a command */
/*      becomes available for a device that does not currently have a   */
/*      command in the process of executing.  Resources are allocated,  */
/*      QUEUE, and an attempt is made to suspend the chip to get this   */
/*      command running.                                                */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process or interrupt       */
/*      handler.  It can page fault only if called under a process      */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - scsi chip unique data structure                   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_index - index to device information                         */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      NONE                                                            */
/*                                                                      */
/* ERROR DESCRIPTION                                                    */
/*      NONE                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_start                                                         */
/*                                                                      */
/************************************************************************/
void
psc_start(
	  struct dev_info * dev_ptr)
{
    struct sc_buf *bp;
    int     ret_code, i, dev_index;
    uchar   istat_val, chk_disconnect;
    caddr_t iocc_addr;
    register int rc;

    DEBUG_0 ("Entering psc_start routine.\n")
    TRACE_1 ("in strt ", (int) dev_ptr)
    bp = dev_ptr->head_pend;

    if (adp_str.epow_state != EPOW_PENDING)
    {	/* non-EPOW state */
	/* Attempt to allocate the resource that are  */
	/* needed. i.e. TCW's, small xfer area, etc.  */
	ret_code = psc_alloc_resources(dev_ptr);
	if (ret_code == PSC_NO_ERR)
	{  /* Resources were allocated successfully */
	    if (adp_str.DEVICE_ACTIVE_head == NULL)
	    {	/* There's no commands currently active */
                /* Write system trace to indicate start of command */
                /* to adapter */
                DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, adp_str.devno,
                        bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
                        bp->bufstruct.b_bcount);

		if (!(dev_ptr->negotiate_flag))
		{	/* Negotiation not required, Assume SYNC */
		    /* Call routine to start command at the chip */
		    if (dev_ptr->async_device)
		    {
			if (SET_SXFER(0x00) == REG_FAIL)
			{
			    bp->bufstruct.b_error = EIO;
			    bp->bufstruct.b_flags |= B_ERROR;
			    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
			    bp->status_validity = SC_ADAPTER_ERROR;
			    bp->general_card_status = SC_ADAPTER_HDW_FAILURE;
			    (void) psc_free_resources(dev_ptr, FALSE);
			    iodone((struct buf *) bp);
			    dev_ptr->active = NULL;
			    /* Fail the rest of the pending queue. */
			    if (dev_ptr->head_pend != NULL)
				psc_fail_cmd(dev_ptr, 1);
			    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART,
				    EIO, adp_str.devno);
			    return;
			}
		    }
		    else
			if ((dev_ptr->agreed_xfer !=
			     DEFAULT_MIN_PHASE) ||
			    (dev_ptr->agreed_req_ack <
			     DEFAULT_BYTE_BUF))
			{
			    if (SET_SXFER(dev_ptr->special_xfer_val)
					 == REG_FAIL)
			    {
				bp->bufstruct.b_error = EIO;
				bp->bufstruct.b_flags |= B_ERROR;
				bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
				bp->status_validity = SC_ADAPTER_ERROR;
				bp->general_card_status =
						    SC_ADAPTER_HDW_FAILURE;
				(void) psc_free_resources(dev_ptr, FALSE);
				iodone((struct buf *) bp);
				dev_ptr->active = NULL;
				/* Fail the rest of the pending queue. */
				if (dev_ptr->head_pend != NULL)
				    psc_fail_cmd(dev_ptr, 1);
				DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART,
					EIO, adp_str.devno);
				return;
			    }
			}
		    chk_disconnect = bp->scsi_command.flags & SC_NODISC;
		    if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
			psc_set_disconnect(dev_ptr, chk_disconnect);
		    psc_prep_main_script((uint *)adp_str.SCRIPTS[0].script_ptr,
		   (uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
					 dev_ptr, dev_ptr->script_dma_addr);
		    ISSUE_MAIN_TO_DEVICE(dev_ptr->script_dma_addr);
		    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
		}
		else
		    if (bp->scsi_command.flags & SC_ASYNC)
		    {	/* Negotiation not required, known ASYNC */
			/* Call routine to set asynch in script.     */
			if (SET_SXFER(0x00) == REG_FAIL)
			{
			    bp->bufstruct.b_error = EIO;
			    bp->bufstruct.b_flags |= B_ERROR;
			    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
			    bp->status_validity = SC_ADAPTER_ERROR;
			    bp->general_card_status = SC_ADAPTER_HDW_FAILURE;
			    (void) psc_free_resources(dev_ptr, FALSE);
			    iodone((struct buf *) bp);
			    dev_ptr->active = NULL;
			    /* Fail the rest of the pending queue. */
			    if (dev_ptr->head_pend != NULL)
				psc_fail_cmd(dev_ptr, 1);
			    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART,
				    EIO, adp_str.devno);
			    return;
			}
			/* Call routine to start command at the chip */
			dev_index = INDEX(dev_ptr->scsi_id,
					  dev_ptr->lun_id);
			psc_patch_async_switch_int((uint *) adp_str.SCRIPTS
			   [dev_ptr->cmd_script_ptr].script_ptr, dev_index);
			chk_disconnect = bp->scsi_command.flags & SC_NODISC;
			if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
			    psc_set_disconnect(dev_ptr, chk_disconnect);
			psc_prep_main_script((uint *)
			   adp_str.SCRIPTS[0].script_ptr, (uint *)
			   adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
			   dev_ptr, dev_ptr->script_dma_addr);
			ISSUE_MAIN_TO_DEVICE(dev_ptr->script_dma_addr);
			dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
			dev_ptr->async_device = TRUE;
			dev_ptr->negotiate_flag = FALSE;
		    }
		    else	/* Need to issue negotiate command */
		    {
			/* Call routine to start negotiate at the chip */
			chk_disconnect = bp->scsi_command.flags & SC_NODISC;
			if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
			    psc_set_disconnect(dev_ptr, chk_disconnect);
			ISSUE_NEGOTIATE_TO_DEVICE(dev_ptr->script_dma_addr);
			dev_ptr->cmd_activity_state = NEGOTIATE_IN_PROGRESS;
		    }
		/* Start the watchdog timer for this command */
                if (bp->timeout_value == 0)
                      dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
                else
                      dev_ptr->dev_watchdog.dog.restart = 
                                (ulong)bp->timeout_value+1;
		w_start(&(dev_ptr->dev_watchdog.dog));

		/* Add to the DEVICE ACTIVE QUEUE */
		psc_enq_active(dev_ptr);
		DEBUG_0 ("Issuing command\n")
                DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, adp_str.devno);
	    }
	    else
		if (adp_str.DEVICE_WAITING_head == NULL)
		{
		    /* Add to the DEVICE WAITING QUEUE */
		    /* A read of the istat register indicates that */
		    /* the scsi chip is currently not busy on the  */
		    /* scsi bus, so try to signal the chip that a  */
		    /* command is waiting.                         */
		    iocc_addr =
			BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
		    psc_patch_iowait_int_on();
		    for (i = 0; i < PIO_RETRY_COUNT; i++)
		    {
			rc = BUS_GETCX(iocc_addr + ISTAT, (char *)&istat_val);
                        if ((rc == 0) && !(istat_val & CONN_INT_TEST))
                          /* Successful register read, & chip not connected */
                        {
                            rc = BUS_PUTCX(iocc_addr + ISTAT, 0x80);
                            if (rc == 0)
			    {
				/* Start a timer for the chip suspend */
                                TRACE_1("ab istat", istat_val)
				w_start(&(adp_str.adap_watchdog.dog));
				break;
                            }
                        }
                        if (rc == 0)
                        {       /* no pio errors to retry, so */
                            break;
                        }
                    }
                    BUSIO_DET(iocc_addr);
                    if (rc != 0)
                    {
			bp->bufstruct.b_error = EIO;
			bp->bufstruct.b_flags |= B_ERROR;
			bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
			bp->status_validity = SC_ADAPTER_ERROR;
			bp->general_card_status = SC_ADAPTER_HDW_FAILURE;
			(void) psc_free_resources(dev_ptr, FALSE);
			iodone((struct buf *) bp);
			dev_ptr->active = NULL;
			/* Fail the rest of the pending queue. */
			if (dev_ptr->head_pend != NULL)
			    psc_fail_cmd(dev_ptr, 1);
			return;
		    }
		    else
		    {
			psc_enq_wait(dev_ptr);
		    }
		}
		else
		{	/* There's other device waiting, so add to queue */
		    /* Add to the DEVICE WAITING QUEUE */
		    psc_enq_wait(dev_ptr);
		    DEBUG_0 ("Other device waiting\n")
		}
	}
	else
	{   /* Resources could not be allocated */
	    /* Add to the DEVICE WAITING FOR RESOURCES QUEUE */
	    psc_enq_wait_resources(dev_ptr);
	    DEBUG_0 ("Could not allocate resources\n")
	}
    }
    else
    {	/* An EPOW is occuring */
	DEBUG_0 ("EPOW!!!\n")
    }
    TRACE_1 ("out strt", (int) dev_ptr)
    DEBUG_0 ("Leaving psc_start routine.\n")
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_alloc_STA                                                   */
/*                                                                        */
/* FUNCTION:  Allocates a small transfer area for a small data xfer.      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called when allocating resources for a command.*/
/*         This routine obtains a transfer area of size 256 bytes from    */
/*         the memory page this dd has set aside for data transfers in-   */
/*         volving small amounts.                                         */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    TRUE  - Able to allocate STA.                                       */
/*    FALSE - Unable to allocate STA.                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_alloc_STA(
	      struct dev_info * dev_ptr)
{

    int     i;

    DEBUG_0 ("Entering psc_alloc_STA routine.\n")
    for (i = 0; i < NUM_STA; i++)
    {	/* Loop through the STA area to find a free */
	if (adp_str.STA[i].in_use == STA_UNUSED)
	{  /* A free STA area has been found */
	    dev_ptr->STA_index = i;
	    dev_ptr->STA_addr = (uint) adp_str.STA[i].sta_ptr;
	    dev_ptr->resource_state = STA_RESOURCES_USED;
	    adp_str.STA[i].in_use = STA_IN_USE;
	    DEBUG_0 ("Leaving psc_alloc_STA routine.\n")
	    return (TRUE);
	}
    }
    DEBUG_0 ("Leaving psc_alloc_STA routine.\n")
    return (FALSE);     /* No STA's availible at this time */
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_free_STA                                                    */
/*                                                                        */
/* FUNCTION:  Frees a small transfer area.                                */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine frees a small transfer area for later use.        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    TRUE  - Able to allocate STA.                                       */
/*    FALSE - Unable to allocate STA.                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
psc_free_STA(
	     struct dev_info * dev_ptr)
{
    DEBUG_0 ("Entering psc_free_STA routine.\n")
    adp_str.STA[dev_ptr->STA_index].in_use = STA_UNUSED;
    dev_ptr->STA_index = 0;
    dev_ptr->STA_addr = 0;
    DEBUG_0 ("Leaving psc_free_STA routine.\n")
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_alloc_TCW                                                   */
/*                                                                        */
/* FUNCTION:  Allocate TCWs for data transfer.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called when allocating resources for a command.*/
/*         Resources consist of either a single TCW allocated from the    */
/*         small TCW area or more TCWs which are allocated from the large */
/*         TCW area.                                                      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*    sc_buf structure   - pointer to scsi command structure.             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    TRUE  - Able to allocate TCW(s).                                    */
/*    FALSE - Unable to allocate TCW(s).                                  */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_alloc_TCW(
	      struct sc_buf * bp,
	      struct dev_info * dev_ptr)
{

    uchar   found, wrap, end_search, alloc_val;
    int     baddr, num_TCW, num_large_TCW, t_ptr, i;

    DEBUG_0 ("Entering psc_alloc_TCW routine.\n")
    found = FALSE;
    /* Set up value to use as "in use" flag for debug */
    /* purposes.                                      */
    alloc_val = INDEX(bp->scsi_command.scsi_id,
		      (bp->scsi_command.scsi_cmd.lun) >> 5);
    /* Compute the number of TCWs required using the  */
    /* buffer address and byte count from the scsi    */
    /* command.                                       */
    baddr = (int) bp->bufstruct.b_un.b_addr;
    num_TCW = (((baddr & (PAGESIZE - 1)) + bp->bufstruct.b_bcount - 1) /
	       PAGESIZE) + 1;
    DEBUG_1 ("Number of TCWs = %d\n", num_TCW)
    if (num_TCW == 1)
    {	/* A TCW is allocated from the small TCW area */
	if ((adp_str.num_4K_tcws_in_use + adp_str.num_scripts_created)
	     >= (NUM_4K_TCWS - 1))
	    return (FALSE);
	/* The small TCW area is searched from start  */
	/* of the circular pointer.                   */
	TRACE_2 ("in TCWSM", (int) dev_ptr, bp->bufstruct.b_bcount)
	t_ptr = adp_str.next_4K_req;
	do
	{
	    if (adp_str.TCW_alloc_table[t_ptr] == TCW_UNUSED)
	    {
		/* Setup a pointer to the TCW small    */
		/* xfer array in the dev_info structure */
		dev_ptr->dma_addr =
		    (uint) DMA_ADDR(adp_str.ddi.tcw_start_addr, t_ptr) +
		    (uint) (baddr & (PAGESIZE - 1));
		dev_ptr->TCW_index = t_ptr;
		dev_ptr->TCW_count = 1;
		/* Mark TCW as "in use" with alloc_val */
		adp_str.TCW_alloc_table[t_ptr] = alloc_val;
		found = TRUE;
	    }
	    t_ptr++;
	    if (t_ptr > adp_str.ending_4K_TCW)
		t_ptr = adp_str.begin_4K_TCW;
	} while (!(found) && (t_ptr != adp_str.next_4K_req));
	if (found)
	{
	    adp_str.next_4K_req = t_ptr;
	    DEBUG_0 ("Leaving psc_alloc_TCW routine.\n")
	    dev_ptr->resource_state = SMALL_TCW_RESOURCES_USED;
	    TRACE_2 ("out TCW ", dev_ptr->TCW_index,
		             dev_ptr->TCW_count)
	    adp_str.num_4K_tcws_in_use++;
	    return (TRUE);	/* TCW allocated */
	}
    }
    else
    {	/* The TCWs are allocated from the large TCW area */
	/* The large TCW area is searched from start- */
	/* at a circular pointer.  The number of TCWs */
	/* required are then searched for starting at */
	/* the first free large TCW.                  */

	/* Compute number of large TCWs required      */
	TRACE_2 ("in TCWLG", (int) dev_ptr, bp->bufstruct.b_bcount)
	num_large_TCW = num_TCW / LARGE_TCW_SIZE;
	if (num_TCW % LARGE_TCW_SIZE)
	    num_large_TCW++;
	t_ptr = adp_str.next_large_req;
	wrap = FALSE;
	end_search = FALSE;
	do
	{
	    /* Is there enough room to search     */
	    if ((wrap) &&
		((t_ptr + num_large_TCW) > adp_str.next_large_req))
	    {	/* the search has wrapped around */
		end_search = TRUE;
	    }
	    else
		if ((t_ptr + num_large_TCW) > (adp_str.large_req_end + 1))
		{
		    t_ptr = adp_str.large_req_begin;
		    wrap = TRUE;
		    continue;
		}
	    found = TRUE;
	    /* Loop through the large TCW     */
	    /* table to determine if there is */
	    /* the required number of conti-  */
	    /* guous TCWs.                    */
	    for (i = t_ptr; i < (t_ptr + num_large_TCW); i++)
	    {
		if (adp_str.large_TCW_table[i] != TCW_UNUSED)
		{
		    found = FALSE;
		    t_ptr = i + 1;
		    break;
		}
	    }
	} while (!(found) && !(end_search));
	if (found)
	{
	    /* The TCWs were found and a pointer to   */
	    /* first is stored in the dev_info struct. */
	    dev_ptr->dma_addr =
		(uint) DMA_ADDR2(adp_str.large_tcw_start_addr, t_ptr) +
		(uint) (baddr & (PAGESIZE - 1));
	    dev_ptr->TCW_index = t_ptr;
	    dev_ptr->TCW_count = num_large_TCW;
	    /* Loop through large TCWs and mark in use */
	    for (i = 0; i < num_large_TCW; i++)
	    {
		adp_str.large_TCW_table[t_ptr + i] = alloc_val;
	    }
	    if ((t_ptr + num_large_TCW) > adp_str.large_req_end)
		adp_str.next_large_req = adp_str.large_req_begin;
	    else
		adp_str.next_large_req = t_ptr + num_large_TCW;
	    dev_ptr->resource_state = LARGE_TCW_RESOURCES_USED;
	    TRACE_2 ("out TCW ", dev_ptr->TCW_index,
		    dev_ptr->TCW_count)
	    return (TRUE);
	}
    }

    TRACE_1 ("NO TCWs!", (int) dev_ptr)
    TRACE_1 ("out TCW ", 0)
    DEBUG_0 ("Leaving psc_alloc_TCW routine.\n")
    return (FALSE);     /* Not enough TCWs available */
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_free_TCW                                                    */
/*                                                                        */
/* FUNCTION:  Free TCWs after command completion.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called when freeing resources for a command.   */
/*         The used TCWs will be freed to the proper table.               */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
psc_free_TCW(
	     struct dev_info * dev_ptr)
{

    int     i;

    DEBUG_0 ("Entering psc_free_TCW routine.\n")
    if (dev_ptr->resource_state == SMALL_TCW_RESOURCES_USED)
    {	/* A TCW is allocated from the small TCW area */
	/* Return this TCW to the small TCW area.      */
	adp_str.TCW_alloc_table[dev_ptr->TCW_index] = TCW_UNUSED;
	adp_str.num_4K_tcws_in_use--;
    }
    else
    {	/* The TCWs are allocated from the large TCW area */
	/* Return the TCWs to the large TCW area.     */
	for (i = 0; i < dev_ptr->TCW_count; i++)
	{
	    adp_str.large_TCW_table[dev_ptr->TCW_index + i] = TCW_UNUSED;
	}
    }
    /* Clear the TCW save area in the dev_info struct. */
    dev_ptr->TCW_index = 0;
    dev_ptr->TCW_count = 0;
    DEBUG_0 ("Leaving psc_free_TCW routine.\n")
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:        psc_alloc_resources                                       */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to allocate resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to the determine what resources are re- */
/*         quired for allocation to take place.  Upon completion,         */
/*         d_master will have been called, and the DMA address is saved   */
/*         along with other needed information in the device structure.   */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    NO_ERR       -  Allocation of resources successful.                 */
/*    PSC_FAILED   -  Allocation of resources unsuccessful.               */
/*    PSC_COPY_ERROR - Error occurred during data copy.                   */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_alloc_resources(
		    struct dev_info * dev_ptr)
{
    struct sc_buf *bp;
    struct buf *next_bp;
    uint    local_dma_addr;
    int     i;

    DEBUG_0 ("Entering psc_allocate_resource routine.\n")
    TRACE_1 ("in allc ", (int) dev_ptr)
    /* Obtain pointer to the command to process. */
    bp = dev_ptr->head_pend;

    if ((bp->bufstruct.b_bcount > ST_SIZE) ||
	((bp->bufstruct.b_bcount > 0) &&
	(bp->bufstruct.b_xmemd.aspace_id != XMEM_GLOBAL)))
    {
	/* Must be a normal data transfer if execution */
	/* reached here.                               */
	if (psc_alloc_TCW(bp, dev_ptr))
	{   /* The required TCW's are free */
	    /* Move command from the pending device queue */
	    /* to the device active queue.                */
	    dev_ptr->active = bp;
	    dev_ptr->head_pend = (struct sc_buf *) bp->bufstruct.av_forw;
	    if (dev_ptr->head_pend == NULL)
		dev_ptr->tail_pend = NULL;

	    /* Setup and call d_master for both spanned and */
	    /* unspanned commands.                          */
	    if (bp->bp == NULL)
	    {	/* Non-spanned command */
		d_master((int) adp_str.channel_id, DMA_TYPE |
			 ((bp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
			 ((bp->bufstruct.b_flags & B_NOHIDE) ?
			  DMA_WRITE_ONLY : 0), bp->bufstruct.b_un.b_addr,
			 (size_t) bp->bufstruct.b_bcount,
			 &bp->bufstruct.b_xmemd,
			 (char *) dev_ptr->dma_addr);
	    }
	    else
	    {	/* must be a spanned command */
		next_bp = (struct buf *) bp->bp;
		local_dma_addr = dev_ptr->dma_addr;
		while (next_bp != NULL)
		{
		    d_master((int) adp_str.channel_id, DMA_TYPE |
			     ((bp->bufstruct.b_flags & B_READ) ?
			      DMA_READ : 0) |
			     ((next_bp->b_flags & B_NOHIDE) ?
			      DMA_WRITE_ONLY : 0),
			     next_bp->b_un.b_addr,
			     (size_t) next_bp->b_bcount,
			     &next_bp->b_xmemd,
			     (char *) local_dma_addr);

		    local_dma_addr += (uint) next_bp->b_bcount;
		    next_bp = (struct buf *) next_bp->av_forw;
		}
	    }
	    DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
	    TRACE_1 ("out allc", PSC_NO_ERR)
	    return (PSC_NO_ERR);
	}
	else
	{   /* Not enough TCW's available */
	    DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
	    TRACE_1 ("out allc", PSC_FAILED)
	    return (PSC_FAILED);
	}
    }
    if (bp->bufstruct.b_bcount == 0)
    {
	/* there is not data to transfer, thus no   */
	/* need to allocate resources.              */
	dev_ptr->resource_state = NO_RESOURCES_USED;
	/* Move command from the pending device queue */
	/* to the device active queue.                */
	dev_ptr->active = bp;
	dev_ptr->head_pend = (struct sc_buf *) bp->bufstruct.av_forw;
	if (dev_ptr->head_pend == NULL)
	    dev_ptr->tail_pend = NULL;
	DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
	TRACE_1 ("out allc", PSC_NO_ERR)
	return (PSC_NO_ERR);
    }
    /* must allocate area from the small transfer area */
    if (psc_alloc_STA(dev_ptr))
    {	/* An STA area was free */
	/* Calculate the dma address of the STA and    */
	/* save in the device's info structure.        */
	dev_ptr->dma_addr =
	    (uint) (DMA_ADDR(adp_str.ddi.tcw_start_addr, 0)) +
	    ((uint) (adp_str.STA[dev_ptr->STA_index].sta_ptr) -
	     ((uint) (adp_str.STA[0].sta_ptr) & ~(PAGESIZE - 1)));

	/* Move command from the pending device queue */
	/* to the device active queue.                */
	dev_ptr->active = bp;
	dev_ptr->head_pend = (struct sc_buf *) bp->bufstruct.av_forw;
	if (dev_ptr->head_pend == NULL)
	    dev_ptr->tail_pend = NULL;

	/* For writes, data needs to be written to the */
	/* small transfer area, so that is done here.  */
	if (!(bp->bufstruct.b_flags & B_READ))
	{
	    /* The data is in kernal space so do a byte copy */
	    for (i = 0; i < bp->bufstruct.b_bcount; i++)
		*(adp_str.STA[dev_ptr->STA_index].sta_ptr + i) =
		    *(bp->bufstruct.b_un.b_addr + i);
	}
	DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
	TRACE_1 ("out allc", PSC_NO_ERR)
	return (PSC_NO_ERR);
    }
    DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
    TRACE_1 ("out allc", PSC_FAILED)
    return (PSC_FAILED);
}  /* psc_alloc_resources */

/**************************************************************************/
/*                                                                        */
/* NAME:        psc_free_resources                                        */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to free all resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to the determine what resources are to  */
/*         be freed.  Pointers and save locations are initialized         */
/*         to the unused state and a d_complete is performed for those    */
/*         commands that require it.                                      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*    copy_required - flag to test for copy to a STA:                     */
/*                    TRUE - perform the copy to the user/kernel buffer.  */
/*                    FALSE - A previous error occurred so don't do copy. */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    NO_ERR         - Deallocation of resources successful.              */
/*    PSC_COPY_ERROR - Error occurred during data copy.                   */
/*    PSC_DMA_ERROR  - Error occurred during d_complete.                  */
/*    DMA_SYSTEM     - A DMA system error was detected.                   */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_free_resources(
		   struct dev_info * dev_ptr,
		   uchar copy_required)
{
    struct sc_buf *bp;
    struct buf *next_bp;
    uint    local_dma_addr;
    int     ret_code, ret_code2, i;

    DEBUG_0 ("Entering psc_free_resources routine.\n")
    TRACE_1 ("in free ", (int) dev_ptr)
    ret_code = PSC_NO_ERR;      /* default to successful */

    /* Obtain pointer to the command to process. */
    bp = dev_ptr->active;

    switch (dev_ptr->resource_state)
    {
	/* There's no resources to free, so return. */
    case SMALL_TCW_RESOURCES_USED:
    case LARGE_TCW_RESOURCES_USED:
	/* process case of completion here.    */
	/* Setup and call d_complete for both spanned and */
	/* unspanned commands.                            */
	if (bp->bp == NULL)
	{	/* Non-spanned command */
	    ret_code = d_complete((int) adp_str.channel_id,
				  DMA_TYPE |
				  ((bp->bufstruct.b_flags & B_READ) ?
				   DMA_READ : 0) |
				  ((bp->bufstruct.b_flags & B_NOHIDE) ?
				   DMA_WRITE_ONLY : 0),
				  bp->bufstruct.b_un.b_addr,
				  (size_t) bp->bufstruct.b_bcount,
				  &bp->bufstruct.b_xmemd,
				  (char *) dev_ptr->dma_addr);
	}
	else
	{	/* must be a spanned command */
	    next_bp = (struct buf *) bp->bp;
	    local_dma_addr = dev_ptr->dma_addr;
	    while (next_bp != NULL)
	    {
		ret_code2 = d_complete((int) adp_str.channel_id,
				       DMA_TYPE |
				       ((bp->bufstruct.b_flags & B_READ) ?
					DMA_READ : 0) |
				       ((next_bp->b_flags & B_NOHIDE)
					? DMA_WRITE_ONLY : 0),
				       next_bp->b_un.b_addr,
				       (size_t) next_bp->b_bcount,
				       &next_bp->b_xmemd,
				       (char *) local_dma_addr);
		if (ret_code2 != 0)
		{   /* Was there a DMA type error?   */
		    if (ret_code2 == DMA_SYSTEM)
			/* if a system error has occurred */
			ret_code = DMA_SYSTEM;
		    else
			ret_code = PSC_DMA_ERROR;
		}
		local_dma_addr += (uint) next_bp->b_bcount;
		next_bp = (struct buf *) next_bp->av_forw;
	    }
	}
	psc_free_TCW(dev_ptr);
	dev_ptr->resource_state = 0;
	break;
    case STA_RESOURCES_USED:
	/* Call d_complete on the small xfer that took place */
	ret_code = d_complete(adp_str.channel_id, DMA_TYPE,
			      adp_str.STA[0].sta_ptr, (size_t) PAGESIZE,
			      &adp_str.xmem_STA,
			      (char *) DMA_ADDR(adp_str.ddi.tcw_start_addr,
				       adp_str.STA_tcw));
	if (ret_code != 0)
	{    /* If an error occurred */
	    if (ret_code != DMA_SYSTEM)
		ret_code = PSC_DMA_ERROR;
	    /* Attempt to do another d_master to clean */
	    /* up the error.                           */
	    d_master(adp_str.channel_id, DMA_TYPE,
		     adp_str.STA[0].sta_ptr, (size_t) PAGESIZE,
		     &adp_str.xmem_STA,
		     (char *) DMA_ADDR(adp_str.ddi.tcw_start_addr,
			      adp_str.STA_tcw));
		     (void) d_complete(adp_str.channel_id, DMA_TYPE,
				       adp_str.STA[0].sta_ptr,
				       (size_t) PAGESIZE, &adp_str.xmem_STA,
			      (char *) DMA_ADDR(adp_str.ddi.tcw_start_addr,
				       adp_str.STA_tcw));
	}
	else
	{  /* no error detected - good completion */
	    /* For reads, data needs to be read from the   */
	    /* small transfer area, so that is done here.  */
	    if ((bp->bufstruct.b_flags & B_READ) && (copy_required))
	    {
		if (bp->bufstruct.b_xmemd.aspace_id == XMEM_GLOBAL)
		{	/* The data is in kernal space so do a  */
		    /* byte copy.                           */
		    for (i = 0; i < bp->bufstruct.b_bcount; i++)
			*(bp->bufstruct.b_un.b_addr + i) =
			    *(adp_str.STA[dev_ptr->STA_index].sta_ptr + i);
		}
		else
		{
		    /* use xmemout to copy the data out to user space */
		    ret_code = xmemout(
				    adp_str.STA[dev_ptr->STA_index].sta_ptr,
				       bp->bufstruct.b_un.b_addr,
				       bp->bufstruct.b_bcount,
				       &bp->bufstruct.b_xmemd);
		    if (ret_code != XMEM_SUCC)
		    {
			psc_logerr(ERRID_SCSI_ERR1,
				   XMEM_COPY_ERROR, 80,
				   0, dev_ptr, FALSE);
			ret_code = PSC_COPY_ERROR;
		    }
		    else
		    {
			ret_code = PSC_NO_ERR;
		    }
		}
	    }
	}
	psc_free_STA(dev_ptr);
	dev_ptr->resource_state = 0;
	break;
    case NO_RESOURCES_USED:
    default:
	/* No action is required for these cases */
	dev_ptr->resource_state = 0;
	break;
    }
    DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
    TRACE_1 ("out free", ret_code)
    return (ret_code);
}  /* psc_free_resources */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_iodone                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Iodone Routine                           */
/*                                                                      */
/*      This routine handles completion of commands initiated through   */
/*      the psc_ioctl routine.                                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the IODONE interrupt level, so it can      */
/*      be interrupted by the interrupt handler.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      bp      - pointer to the passed sc_buf                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
int
psc_iodone(struct sc_buf * bp)
{
    DEBUG_0 ("Entering psc_iodone routine.\n")
    TRACE_1 ("iodone  ", (int) bp)
    e_wakeup(&bp->bufstruct.b_event);
    DEBUG_0 ("Leaving psc_iodone routine.\n")
}

/**************************************************************************/
/*                                                                        */
/* NAME: psc_enq_active, psc_enq_wait, psc_enq_wait_resources,            */
/*       psc_enq_abort_bdr                                                */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Utility routines to handle queuing of device structures to each   */
/*      of the queues in use.                                             */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* ERROR DESCRIPTION:  The following errno values may be returned:        */
/*      none                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
psc_enq_active(
	       struct dev_info * dev_ptr)
{
    DEBUG_1 ("psc_enq_active dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in enqA ", (int) dev_ptr)
    if (adp_str.DEVICE_ACTIVE_head == NULL)
    {
	dev_ptr->DEVICE_ACTIVE_fwd = NULL;
	dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
	adp_str.DEVICE_ACTIVE_head = dev_ptr;
	adp_str.DEVICE_ACTIVE_tail = dev_ptr;
    }
    else
    {
	dev_ptr->DEVICE_ACTIVE_bkwd = adp_str.DEVICE_ACTIVE_tail;
	dev_ptr->DEVICE_ACTIVE_fwd = NULL;
	adp_str.DEVICE_ACTIVE_tail->DEVICE_ACTIVE_fwd = dev_ptr;
	adp_str.DEVICE_ACTIVE_tail = dev_ptr;
    }
    TRACE_1 ("out enqA", (int) dev_ptr)
}

/************************************************************************/
void
psc_enq_wait(
	     struct dev_info * dev_ptr)
{
    DEBUG_1 ("psc_enq_wait dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in enqW ", (int) dev_ptr)
    if (adp_str.DEVICE_WAITING_head == NULL)
    {
	 adp_str.DEVICE_WAITING_head = dev_ptr;
    }
    else
    {
	adp_str.DEVICE_WAITING_tail->DEVICE_WAITING_fwd = dev_ptr;
    }
    dev_ptr->DEVICE_WAITING_fwd = NULL;
    adp_str.DEVICE_WAITING_tail = dev_ptr;
    TRACE_1 ("out enqW", (int) dev_ptr)
}

/************************************************************************/
void
psc_enq_wait_resources(
		       struct dev_info * dev_ptr)
{
    DEBUG_1 ("psc_enq_wait_resources dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in enqWR", (int) dev_ptr)
    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_head == NULL)
    {
	adp_str.DEVICE_WAITING_FOR_RESOURCES_head = dev_ptr;
    }
    else
    {
	adp_str.DEVICE_WAITING_FOR_RESOURCES_tail->
	    DEVICE_WAITING_FOR_RESOURCES_fwd = dev_ptr;
    }
    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd = NULL;
    adp_str.DEVICE_WAITING_FOR_RESOURCES_tail = dev_ptr;
    TRACE_1 ("out eqWR", (int) dev_ptr)
}

/************************************************************************/
void
psc_enq_abort_bdr(
		  struct dev_info * dev_ptr)
{
    DEBUG_1 ("psc_enq_abort_bdr dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in enqB ", (int) dev_ptr)
    if (adp_str.ABORT_BDR_head == NULL)
    {
	dev_ptr->ABORT_BDR_fwd = NULL;
	dev_ptr->ABORT_BDR_bkwd = NULL;
	adp_str.ABORT_BDR_head = dev_ptr;
	adp_str.ABORT_BDR_tail = dev_ptr;
    }
    else
    {
	dev_ptr->ABORT_BDR_bkwd = adp_str.ABORT_BDR_tail;
	dev_ptr->ABORT_BDR_fwd = NULL;
	adp_str.ABORT_BDR_tail->ABORT_BDR_fwd = dev_ptr;
	adp_str.ABORT_BDR_tail = dev_ptr;
    }
    TRACE_1 ("out enqB", (int) dev_ptr)
}

/**************************************************************************/
/*                                                                        */
/* NAME: psc_deq_active, psc_deq_wait, psc_deq_wait_resources,            */
/*       psc_deq_abort_bdr                                                */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Utility routines to handle dequeueing device structures from      */
/*      each of the queues in use.                                        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* ERROR DESCRIPTION:  The following errno values may be returned:        */
/*      none                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
psc_deq_active(
	       struct dev_info * dev_ptr)
{

    DEBUG_1 ("psc_deq_active dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in deqA ", (int) dev_ptr)
    if (adp_str.DEVICE_ACTIVE_head == adp_str.DEVICE_ACTIVE_tail)
    {
	adp_str.DEVICE_ACTIVE_head = NULL;
	adp_str.DEVICE_ACTIVE_tail = NULL;
    }
    else
	if (adp_str.DEVICE_ACTIVE_head == dev_ptr)	/* first one */
	{
	    adp_str.DEVICE_ACTIVE_head = dev_ptr->DEVICE_ACTIVE_fwd;
	    dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
						dev_ptr->DEVICE_ACTIVE_bkwd;
	}
	else
	    if (adp_str.DEVICE_ACTIVE_tail == dev_ptr)	/* last one */
	    {
		adp_str.DEVICE_ACTIVE_tail = dev_ptr->DEVICE_ACTIVE_bkwd;
		dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
						 dev_ptr->DEVICE_ACTIVE_fwd;
	    }
	    else	/* in the middle */
	    {
		dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
						 dev_ptr->DEVICE_ACTIVE_fwd;
		dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
						dev_ptr->DEVICE_ACTIVE_bkwd;
	    }
    dev_ptr->DEVICE_ACTIVE_fwd = NULL;
    dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
    dev_ptr->cmd_activity_state = 0;
    TRACE_1 ("out deqA", (int) dev_ptr)
}

/************************************************************************/
void
psc_deq_wait(
	     struct dev_info * dev_ptr)
{

    DEBUG_1 ("psc_deq_wait dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in deqW ", (int) dev_ptr)
    if (adp_str.DEVICE_WAITING_head == adp_str.DEVICE_WAITING_tail)
    {
	adp_str.DEVICE_WAITING_head = NULL;
	adp_str.DEVICE_WAITING_tail = NULL;
    }
    else
    {
	adp_str.DEVICE_WAITING_head = dev_ptr->DEVICE_WAITING_fwd;
    }
    dev_ptr->DEVICE_WAITING_fwd = NULL;
    TRACE_1 ("out deqW", (int) dev_ptr)
}

/************************************************************************/
void
psc_deq_wait_resources(
		       struct dev_info * dev_ptr)
{

    DEBUG_1 ("psc_deq_wait_resources dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in deqWR", (int) dev_ptr)
    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_head ==
	adp_str.DEVICE_WAITING_FOR_RESOURCES_tail)
    {
	adp_str.DEVICE_WAITING_FOR_RESOURCES_head = NULL;
	adp_str.DEVICE_WAITING_FOR_RESOURCES_tail = NULL;
    }
    else
    {
	adp_str.DEVICE_WAITING_FOR_RESOURCES_head =
	    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
    }
    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd = NULL;
    TRACE_1 ("out dqWR", (int) dev_ptr)
}

/************************************************************************/
void
psc_deq_abort_bdr(
		  struct dev_info * dev_ptr)
{

    DEBUG_1 ("psc_deq_abort_bdr dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in deqB ", (int) dev_ptr)
    if (adp_str.ABORT_BDR_head == adp_str.ABORT_BDR_tail)
    {
	adp_str.ABORT_BDR_head = NULL;
	adp_str.ABORT_BDR_tail = NULL;
    }
    else
	if (adp_str.ABORT_BDR_head == dev_ptr)	/* first one */
	{
	    adp_str.ABORT_BDR_head = dev_ptr->ABORT_BDR_fwd;
	    dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd = dev_ptr->ABORT_BDR_bkwd;
	}
	else
	    if (adp_str.ABORT_BDR_tail == dev_ptr)	/* last one */
	    {
		adp_str.ABORT_BDR_tail = dev_ptr->ABORT_BDR_bkwd;
		dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd =
						     dev_ptr->ABORT_BDR_fwd;
	    }
	    else	/* in the middle */
	    {
		dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd =
						     dev_ptr->ABORT_BDR_fwd;
		dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd =
						    dev_ptr->ABORT_BDR_bkwd;
	    }
    dev_ptr->ABORT_BDR_fwd = NULL;
    dev_ptr->ABORT_BDR_bkwd = NULL;
    TRACE_1 ("out deqB", (int) dev_ptr)
}

/**************************************************************************/
/*                                                                        */
/* NAME: psc_dump                                                         */
/*                                                                        */
/* FUNCTION: Determine what type of dump operation is being sought        */
/*                                                                        */
/*      Allocate necessary segment registers and parse type of            */
/*      dump operation.  Call the specified routine.                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine must be called in the interrupt environment.  It     */
/*      can not page fault and is pinned.                                 */
/*                                                                        */
/* (NOTES:) This routine handles the following operations :               */
/*      DUMPINIT   - initializes bus attached disk as dump device         */
/*      DUMPSTART  - prepares device for dump                             */
/*      DUMPQUERY  - returns the maximum and minimum number of bytes that */
/*                   can be transferred in a single DUMPWRITE command     */
/*      DUMPWRITE  - performs write to disk                               */
/*      DUMPEND    - cleans up device on end of dump                      */
/*      DUMPTERM   - terminates the bus attached disk as dump device      */
/*                                                                        */
/* (DATA STRUCTURES:) uio             - structure containing information  */
/*                                      about the data to transfer        */
/*                    adapter_def    - adapter info structure             */
/*                    dev_info       - device info structure              */
/*                    sc_buf         - i/o request struct between driver  */
/*                                     and device                         */
/*                    dmp_query       - queried transfer information is   */
/*                                      returned                          */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - device major/minor number                               */
/*      uiop    - pointer to uio structure for data for the               */
/*                specified operation code                                */
/*      cmd     - Type of dump operation being requested                  */
/*      arg     - Pointer to dump query structure or buf_struct for cmd   */
/*      chan    - unused                                                  */
/*      ext     - unused                                                  */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*      psc_dumpstrt                                                      */
/*      psc_dumpwrt                                                       */
/*      psc_dumpend                                                       */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*                                                                        */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*      and the caller is left responsible to recover from the error.     */
/*                                                                        */
/* RETURNS:                                                               */
/*      EBUSY     -  request sense inuse                                  */
/*      EINVAL    -  Invalid iovec argumenmt,or invalid cmd               */
/*      EIO       -  I/O error, quiesce or scsi writefailed.              */
/*      ENOMEM    -  Unable to allocate resources                         */
/*      ENXIO     -  Not inited as dump device                            */
/*      ETIMEDOUT - adapter not responding                                */
/*                                                                        */
/**************************************************************************/
int
psc_dump(
	 dev_t devno,
	 struct uio * uiop,
	 int cmd,
	 int arg,
	 int chan,
	 int ext)
{
    /****************************/
    /* Misc. Variables       */
    /****************************/
    struct sc_buf *scbuf_ptr;
    struct dmp_query *dmp_qry_ptr;
    struct dev_info *dev_ptr;
    int     dev_index;
    int     ret_code;
    char    first_time;
    /****************************/
    DEBUG_0 ("Entering psc_dump\n");
    TRACE_1 ("in psc_dump", 0);

    /* check to make sure adapter is inited and opened */
    ret_code = PSC_NO_ERR;
    if ((!adp_str_inited) || (!adp_str.opened))
	return (ENXIO);
    switch (cmd)
    {
    case DUMPINIT:
	adp_str.dump_inited = TRUE;
	adp_str.dump_started = FALSE;
	TRACE_1 ("DUMPINIT", (int) &arg)
	break;
    case DUMPQUERY:
	dmp_qry_ptr = (struct dmp_query *) arg;
	dmp_qry_ptr->min_tsize = 512;
	dmp_qry_ptr->max_tsize = adp_str.max_request;
	TRACE_1 ("DUMPQUERY", (int) &arg)
	break;
    case DUMPSTART:
	TRACE_1 ("DUMPSTART", (int) &arg)
	break;
    case DUMPWRITE:
	i_disable(INTMAX);
	scbuf_ptr = (struct sc_buf *) arg;
	TRACE_1 ("DUMPWRITE", (int) &arg)
	psc_dumpstrt(scbuf_ptr);
	first_time = TRUE;
	while ((adp_str.DEVICE_ACTIVE_head != NULL) && (ret_code == 0))
	{
	    ret_code = psc_dump_intr(adp_str.DEVICE_ACTIVE_head,
			first_time);
	    first_time = FALSE;
	}
	adp_str.dump_started = TRUE;
	if (ret_code == 0)
	    ret_code = psc_dumpwrt(scbuf_ptr);
	TRACE_1 ("rc_dmpwrt", ret_code)
	dev_index = INDEX(scbuf_ptr->scsi_command.scsi_id,
			  (scbuf_ptr->scsi_command.scsi_cmd.lun) >> 5);
	dev_ptr = adp_str.device_queue_hash[dev_index];
	dev_ptr->head_pend = scbuf_ptr;
	if (ret_code == 0)
	    ret_code = psc_dump_intr(dev_ptr, FALSE);
	break;
    case DUMPEND:
	TRACE_1 ("DUMPEND", (int) &arg)
	adp_str.dump_started = FALSE;
	/* enable interrupts */
	i_enable(INTMAX);
	break;
    case DUMPTERM:
	TRACE_1 ("DUMPTERM", (int) &arg)
	if ((!adp_str.dump_inited) || (!adp_str.dump_started))
	    ret_code = EINVAL;
	else
	    adp_str.dump_started = FALSE;
	break;
    default:
	ret_code = EINVAL;
    }
    DEBUG_0 ("Leaving psc_dump\n")
    TRACE_1 ("out psc_dump", ret_code)
    return (ret_code);
}  /* end psc_dump */

/**************************************************************************/
/*                                                                        */
/* NAME: psc_dumpstrt                                                     */
/*                                                                        */
/* FUNCTION: Notify device of pending dump                                */
/*                                                                        */
/*        Prepare device for dump, fail existing commands on all devices  */
/*        including the dump device. Set up the device queue to except    */
/*        dump commands.                                                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine must be called in the interrupt environment.  It     */
/*      can not page fault and is pinned                                  */
/*                                                                        */
/* (DATA STRUCTURES:)                                                     */
/*                    adapter_def    - adapter info structure             */
/*                    sc_buf    - controller info structure               */
/*                    dev_info    - device info structure                 */
/*                                                                        */
/* INPUTS:                                                                */
/*      bp       -  buf scruct for the pending dump command               */
/*                                                                        */
/* CALLED BY:                                                             */
/*      psc_dump                                                          */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      psc_free_resources                                                */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*      and the caller is left responsible to recover from the error.     */
/*                                                                        */
/* RETURNS:                                                               */
/*      EIO       -  Failed quiesce or scsi write                         */
/*      ENOMEM    -  Unable to allocate resources                         */
/*      ETIMEDOUT - adapter not responding                                */
/*                                                                        */
/**************************************************************************/
int
psc_dumpstrt(
	     struct sc_buf * bp)
{
    struct dev_info *dev_ptr;
    int dev_index, ret_code;

    DEBUG_0 ("Entering psc_dumpstrt\n")
    TRACE_1 ("in dumpstrt", (int) bp)
    ret_code = 0;

    if (!adp_str.dump_inited)
	return (EINVAL);
    dev_index = INDEX(bp->scsi_command.scsi_id,
		      (bp->scsi_command.scsi_cmd.lun) >> 5);
    TRACE_1 ("dev_index :", dev_index)
    dev_ptr = adp_str.device_queue_hash[dev_index];
    dev_ptr->head_pend = bp;

    /* if dump dev already started return to caller */
    if (adp_str.dump_started)
	return (PSC_NO_ERR);

    /***********************************************************/
    /* Clear all abort_bdr queues. Then enter abort commands   */
    /* all active device queues.                               */
    /***********************************************************/
    adp_str.ABORT_BDR_head = NULL;
    adp_str.ABORT_BDR_tail = NULL;
    adp_str.DEVICE_WAITING_FOR_RESOURCES_head = NULL;
    dev_ptr = adp_str.DEVICE_WAITING_head;
    while (dev_ptr != NULL)
    {
	psc_deq_wait(dev_ptr);
	(void) psc_free_resources(dev_ptr, FALSE);
	if (dev_ptr->head_pend == NULL)
	{	/* queue is empty  */
	    dev_ptr->head_pend = dev_ptr->active;
	    dev_ptr->tail_pend = dev_ptr->active;
	}
	else
	{	/* pending queue not empty */
	    /* point first cmd's av_forw at the new request */
	    dev_ptr->active->bufstruct.av_forw =
					 (struct buf *) dev_ptr->head_pend;
	    dev_ptr->head_pend = dev_ptr->active;
	}
	dev_ptr->active = NULL;
	dev_ptr = adp_str.DEVICE_WAITING_head;
    }
    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
	/***************************************************/
	/* put an abort command on the ABORT_BDR queue for */
	/* this device	                                   */
	/***************************************************/
	dev_ptr->flags |= SCSI_ABORT;
	psc_enq_abort_bdr(dev_ptr);
	dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
    }
    DEBUG_0 ("Leaving psc_dumpstrt\n")
    TRACE_1 ("out dumpstrt", ret_code)
    return (ret_code);
}  /* end psc_dumpstrt */

/**************************************************************************/
/*                                                                        */
/* NAME: psc_dump_intr                                                    */
/*                                                                        */
/* FUNCTION: Handle polling for interrupts for dump routines              */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine may be call in the interrupt or process environment. */
/*      It can not page fault and is pinned.                              */
/*                                                                        */
/* (DATA STRUCTURES:)                                                     */
/*                    adapter_def    - adapter info structure             */
/*                    sc_buf    - controller info structure               */
/*                    dev_info    - device info structure                 */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/*                                                                        */
/* CALLED BY:                                                             */
/*      psc_dumpwrt, psc_dumpstrt                                         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      psc_free_resources          psc_read_reg       psc_delay          */
/*      psc_update_dataptr          psc_write_reg      psc_find_devinfo   */
/*      psc_verify_neg_answer       psc_deq_active     psc_fail_cmd       */
/*                                                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      word_reverse                                                      */
/* 	IOCC_ATT							  */
/* 	IOCC_DET							  */
/*                                                                        */
/* RETURNS:                                                               */
/*    EIO - register failures                                             */
/*    ETIMEDOUT - command time outs                                       */
/*                                                                        */
/**************************************************************************/
int
psc_dump_intr(struct dev_info * dev_ptr, char abort_chip)

{
    int dev_index, ret_code, loop, i, j, scsi_intr, dma_intr;
    int rc, save_isr, save_stat, dma_stat, save_interrupt;
    int start_new_job;   /* 1=start new job up */
    int multiple_intr, time_out_value, command_issued_flag;
    uchar istat_val;
    caddr_t iocc_addr;
    struct sc_buf *bp;

    DEBUG_0 ("Entering psc_dump_intr\n")
    TRACE_1 ("in dumpintr", abort_chip)
    if (abort_chip)
    {
        /* A read of the istat register indicates that */
        /* the scsi chip is currently not busy on the  */
        /* scsi bus, so try to signal the chip that a  */
        /* command is waiting.                         */
        iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
        psc_patch_iowait_int_on();
        for (j = 0; j < PIO_RETRY_COUNT; j++)
        {
            rc = BUS_GETCX(iocc_addr + ISTAT, (char *) &istat_val);
            TRACE_1 ("dmp istat", istat_val )
            if ((rc == 0) && !(istat_val & CONN_INT_TEST))
            {   /* The register read is successful & chip isn't busy */
                TRACE_1 ("abrt chip", 0 )
                rc = BUS_PUTCX(iocc_addr + ISTAT, 0x80);
            }
            if (rc == 0)
            {       /* no pio errors, so */
                break;
            }
        }
        BUSIO_DET(iocc_addr);
    }
    loop = TRUE;
    i = 0;
    /* note timeout_value of 0 not supported in the dump context */
    time_out_value = dev_ptr->active->timeout_value;
    if (time_out_value == 0)
	time_out_value = 10;
    while ((loop) && (++i < (time_out_value * 1000)))
    {
	/* delay for 1000th of a second */
	psc_delay(1000);
	start_new_job = FALSE;
	 
	/* if not our interrupt go around again */
	save_isr = psc_read_reg(ISTAT, ISTAT_SIZE);
	if (!(save_isr & (SIP | DIP)))
	    continue;
	else
	{
	    if (save_isr & SIP)	/* Planned SCSI interrupt */
	    {
		scsi_intr = psc_read_reg(SSTAT0, SSTAT0_SIZE);
		save_interrupt = scsi_intr;
		TRACE_1 ("dmp scsi", scsi_intr)
	        if (SET_SXFER(SYNC_VAL) == REG_FAIL)
		{
		    loop = FALSE;
		    ret_code = EIO;
		    continue;
		}

		multiple_intr = FALSE;
		do
		{
		    switch (scsi_intr)
		    {
		    case PHASE_MIS:
			TRACE_1 ("duPHASE_MIS", 0)
			DEBUG_0 ("psc_dump: PHASE_MIS\n")
			if ((dev_ptr = psc_find_devinfo(0)) !=
			     NULL)
			{
			    TRACE_1 ("dphse ms", (int) dev_ptr)
			    psc_update_dataptr(adp_str.
				           SCRIPTS[dev_ptr->cmd_script_ptr].
					   script_ptr, dev_ptr);
			    /* clear the DMA and SCSI FIFO */
			    if (psc_write_reg(DFIFO,
					      DFIFO_SIZE,
					      0x40) == REG_FAIL)
			    {
				loop = FALSE;
				multiple_intr = FALSE;
				ret_code = EIO;
				break;
			    }
			}
			else
			{
			    TRACE_1 ("no dev_ptr", 0)
			    DEBUG_0 ("PHS_MIS no dev_ptr\n")
			    psc_write_reg(DSP, DSP_SIZE,
					  word_reverse(adp_str.SCRIPTS[0].
						       dma_ptr));
			    start_new_job = FALSE;
			    multiple_intr = FALSE;
			    break;
			}
			start_new_job = FALSE;
			ISSUE_MAIN_AFTER_NEG(dev_ptr->script_dma_addr);
			multiple_intr = FALSE;
			break;

		    case SCSI_UNEXP_DISC:
			DEBUG_1 ("psc_dumpstrt: scsi unexp disc. 0x%x\n",
					 save_interrupt)
			dev_ptr = psc_find_devinfo(0);
			TRACE_1 ("du_UNEXP", (int) dev_ptr)

                        if ((dev_ptr != NULL) &&
                            (dev_ptr->cmd_activity_state ==
                            ABORT_IN_PROGRESS))
                        {
                            ret_code = PSC_NO_ERR;
                            start_new_job = TRUE;
                            psc_deq_active(dev_ptr);
                        }
                        else
                        {
                            ret_code = ENXIO;
                        }
                        loop = FALSE;
                        multiple_intr = FALSE;
                        break;

		    default:
			TRACE_1 ("du_default", scsi_intr)
			if (scsi_intr & PHASE_MIS)
			{
			    scsi_intr = PHASE_MIS;
			    multiple_intr = TRUE;
			}
                        else
                            if (scsi_intr & SCSI_UNEXP_DISC)
                            {
                                scsi_intr = SCSI_UNEXP_DISC;
                                multiple_intr = TRUE;
                            }

			break;
		    }	/* end switch */
		} while (multiple_intr);
	    }	/* End Planned SCSI interrupt */
	    else
		if (save_isr & DIP)	/* DMA interrupt */
		{
                    if (save_isr & 0x80)
                    {   /* if a chip abort has occurred */
                        /* clear the interrupt */
                        if (psc_write_reg(ISTAT, ISTAT_SIZE,(uchar) 0x00) 
				== REG_FAIL)
                        {
                            loop = FALSE;
			    ret_code = EIO;
                            TRACE_1 ("dmp bistat", 0)
			    continue;
                        }
                    }

		    save_interrupt = psc_read_reg(DSTAT, DSTAT_SIZE);
		    TRACE_1 ("dump dip", save_interrupt)
		    if (save_interrupt & SIR)   /* SCRIPTS interrupt */
		    {
		        dma_stat = word_reverse(psc_read_reg(DSPS, DSPS_SIZE));
		        dma_intr = 0x0000FFFF & dma_stat;
		        dev_index = ((dma_stat & 0xFFFF0000) >> 16);
		        dev_ptr = adp_str.device_queue_hash[dev_index];
		        switch (dma_intr)
		        {
		        case A_set_special_sync:
			    DEBUG_0 ("psc_dump_intr: A_set_special_sync \n")
			    TRACE_1 ("du_spc sync", (int) dev_ptr)
			    /* set xfer mode in chip to SYNC,   */
		            if (SET_SXFER(dev_ptr->special_xfer_val) 
					== REG_FAIL)
			    {
				ret_code = EIO;
				loop = FALSE;
				break;
			    }
			    /* and restart script */
			    rc = psc_read_reg(DSP, DSP_SIZE);
			    if (rc == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			        break;
			    }
			    if (psc_write_reg(DSP, DSP_SIZE, rc) == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			    }
			    break;

		        case A_change_to_async:
			    DEBUG_0 ("psc_intr: A_change_to_async \n");
			    TRACE_1 ("du_chg2async", 0);
			    /* set xfer mode in chip to ASYNC */
			    if (SET_SXFER(0x00) == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			        break;
			    }
			    rc = psc_read_reg(DSP, DSP_SIZE);
			    if (rc == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			        break;
			    }
			    if (psc_write_reg(DSP, DSP_SIZE, rc) == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			        break;
			    }
			    break;

  		        case A_sync_neg_done: 
			    /* sync complete and successful */
			    TRACE_1 ("du_syncneg", (int) dev_ptr)
			    /* supports sync negotiation */
			    dev_ptr->negotiate_flag = FALSE;
			    /* sync complete and successful */
			    dev_ptr->async_device = FALSE;
                            if (SET_SXFER(SYNC_VAL)
                                == REG_FAIL)
                            {
                                ret_code = EIO;
                                loop = FALSE;
                                break;
                            }

			    DEBUG_0 ("psc_dump: SYNC_NEG_DONE \n")
			    /* supports synch negotiation */
			    if (rc = psc_verify_neg_answer(
				(uint *) adp_str.SCRIPTS[dev_ptr->
				cmd_script_ptr].script_ptr,
				dev_ptr, dev_index))
			    {
				if (rc == PSC_FAILED)
                                {  /* need to send a message reject,   */
			           /* due to the way negotiation ended */
				   /* First need to patch the cmd      */
				    if ((bp = dev_ptr->active) != NULL)
				    {
				        psc_prep_main_script(
					    (uint *) adp_str.SCRIPTS
					    [0].script_ptr, (uint *)
					    adp_str.SCRIPTS
					     [dev_ptr->cmd_script_ptr].
					     script_ptr, dev_ptr,
					    dev_ptr->script_dma_addr);
					dev_ptr->cmd_activity_state =
						    CMD_IN_PROGRESS;
				    }
				    rc = psc_write_reg((uint) DSP,
				        (char) DSP_SIZE, word_reverse(
				        dev_ptr->script_dma_addr +
				        Ent_reject_target_sync ));
				}
				if (rc == REG_FAIL)
				{
				    ret_code = EIO;
				    loop = FALSE;
				}
			    }
			    else /* negotiation completed okay */
			    {
			        /* setup script to issue the command    */
			        /* already tagged to this device queue  */
                                if ((bp = dev_ptr->active) != NULL)
                                {
                                    psc_prep_main_script(
                                        (uint *) adp_str.SCRIPTS[0].script_ptr
,
                                        (uint *) adp_str.SCRIPTS
                                        [dev_ptr->cmd_script_ptr].script_ptr,
                                        dev_ptr, dev_ptr->script_dma_addr);
                                    ISSUE_MAIN_AFTER_NEG(dev_ptr->
                                        script_dma_addr);
                                    dev_ptr->cmd_activity_state =
                                                    CMD_IN_PROGRESS;
                                }
                                else
                                {
                                    start_new_job = TRUE;
                                }

                                ret_code = PSC_NO_ERR;
                                loop = TRUE;
			    }
			    break;

		        case A_io_done:
		        case A_io_done_after_data:
		    	    DEBUG_0 ("psc_dump: Current I/O done\n")
			    TRACE_1 ("du_A_iodone", (int) dev_ptr)
                            if (SET_SXFER(SYNC_VAL) == REG_FAIL)
                            {
                                ret_code = EIO;
                                loop = FALSE;
                                break;
                            }

			    /* reset completion flags */
			    bp = dev_ptr->active;
			    if (dma_intr == A_io_done_after_data)
			    {
			        dev_ptr->active->bufstruct.b_resid = 0;
			        /* resid = 0 */
			    }
			    else
			        if (dev_ptr->flags & RESID_SUBTRACT)
			        {
				    dev_ptr->active->bufstruct.b_resid -=
				        dev_ptr->bytes_moved;
				    /* do subtraction */
			        }
			    save_stat = GET_CMD_STATUS_BYTE(adp_str.
					   SCRIPTS[dev_ptr->cmd_script_ptr].
							script_ptr);
			    DEBUG_1 ("STATUS BUFFER = 0x%x\n", save_stat)
			    start_new_job = TRUE;
			    if (save_stat == SC_GOOD_STATUS)
			    {	/* scsi status is ok */
			        if ((bp != NULL) &&
				    (!(bp->bufstruct.b_flags & B_ERROR)))
			        {
				    TRACE_1 ("A_iodgds", (int) dev_ptr)
				    /* set scsi status in sc_buf */
				    bp->bufstruct.b_error = 0;
				    bp->status_validity = 0;
				    bp->scsi_status = SC_GOOD_STATUS;
				    /* free resources */
				    rc = psc_free_resources(dev_ptr, TRUE);
			        }
			    }
			    else	/* status is not okay */
			    {
			        if (adp_str.dump_started)
			        {
				    loop = FALSE;
				    ret_code = EIO;
				    break;
			        }
			        if ((bp != NULL) &&
				    (!(bp->bufstruct.b_flags &
				       B_ERROR)))
			        {
				    TRACE_1 ("diodchk", (int) dev_ptr);
				    /* set scsi stat iuf */
				    bp->scsi_status = save_stat;
				    bp->status_validity = SC_SCSI_ERROR;
				    bp->bufstruct.b_error = EIO;
				    bp->bufstruct.b_flags |= B_ERROR;
				    rc = psc_free_resources(dev_ptr, FALSE);
			        }
			        else
				    if (bp != NULL)
				    {	/* previous error */
				        TRACE_1 ("A_iodpe2", (int) dev_ptr)
				        (void) psc_free_resources(dev_ptr, 
					    FALSE);
				    }
			    }	/* end else */
			    if ((adp_str.ABORT_BDR_head == dev_ptr) ||
			        (dev_ptr->ABORT_BDR_bkwd != NULL))
			    {
			        psc_deq_abort_bdr(dev_ptr);
		            }	 
			    dev_ptr->active = NULL;
			    psc_deq_active(dev_ptr);
			    /* finished this phase of xfer */
			    loop = FALSE;
			    ret_code = PSC_NO_ERR;
			    break;
			    /* tell controller to keep going */

		        case A_io_wait:
			    TRACE_1 ("io wait ", (int) dev_ptr)
			    psc_patch_iowait_int_off();
			    rc = psc_write_reg(DSP, DSP_SIZE,
				  word_reverse(adp_str.SCRIPTS[0].dma_ptr));
			    start_new_job = FALSE;
			    if (rc == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			    }
			    break;

		        case A_abort_io_complete:
		        case A_bdr_io_complete:
			    TRACE_1 ("dbdr cmp", (int) dev_ptr);
			    dev_ptr->cmd_activity_state = 0;
			    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
			    {
			        ret_code = EIO;
			        loop = FALSE;
			        break;
			    }
			    /* remove device from active queue */
			    /* if there is a command tagged to this */
			    if ((bp = dev_ptr->active) != NULL)
			    {
			        if (!(bp->bufstruct.b_flags & B_ERROR))
			        {   /* no previous error */
				    bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
				    bp->bufstruct.b_flags |= B_ERROR;
				    bp->bufstruct.b_error = ENXIO;
			        }
			        rc = psc_free_resources(dev_ptr, FALSE);
			    }
			    dev_ptr->flags = RETRY_ERROR;
			    dev_ptr->active = NULL;
			    /* iodone pending commands and go issue a new one */
			    if (dev_ptr->head_pend != NULL)
			        psc_fail_cmd(dev_ptr, 1);
			    psc_deq_active(dev_ptr);
			    start_new_job = TRUE;
			    loop = FALSE;
			    ret_code = PSC_NO_ERR;
			    break;

		        case A_check_next_io:
		        case A_check_next_io_data:
			    save_stat = GET_CMD_STATUS_BYTE(adp_str.
					   SCRIPTS[dev_ptr->cmd_script_ptr].
							script_ptr);
			    /**********************************************/
			    /* the device is about to disc. from the bus. */
			    /* and a save DP has just been sent.  Move any */
			    /* dataxferred into the buffer and adjust the */
			    /* data pointer within our buffer to show     */
			    /* buffer size has changed.                   */
			    /**********************************************/
			    if (dma_intr == A_check_next_io_data)
			    {
			        dev_ptr->active->bufstruct.b_resid = 0;
			        /* resid = 0 */
			    }
			    else
			        if (dev_ptr->flags & RESID_SUBTRACT)
			        {
				    dev_ptr->active->bufstruct.b_resid -=
				        dev_ptr->bytes_moved;
			        }
			    /* Turn off resid subtract */
			    dev_ptr->flags &= ~RESID_SUBTRACT;
			    TRACE_1 ("dchk nxt", (int) dev_ptr)
			    if (SET_SXFER(SYNC_VAL) ==  REG_FAIL)
			    {
			        TRACE_1 ("du_regfail", 0)
			        loop = FALSE;
			        ret_code = EIO;
				continue;
			    }
			    else
			    {
			        dev_ptr = NULL;
			        start_new_job = TRUE;
			    }

			    break;

		        case A_abort_select_failed:
			    TRACE_1 ("d_abtselfail", 0)
                            if (SET_SXFER(SYNC_VAL) == REG_FAIL)
                            {
                                ret_code = EIO;
                                loop = FALSE;
                                break;
                            }
			    dev_ptr = adp_str.DEVICE_ACTIVE_head;
			    while (dev_ptr != NULL)
			    {
			        psc_restore_iowait_jump((uint *)
					      adp_str.SCRIPTS[0].script_ptr,
					 dev_ptr, dev_ptr->script_dma_addr);
			        dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
			    }
			    psc_patch_iowait_int_off();
			    psc_write_reg(DSP, DSP_SIZE, word_reverse(
					       adp_str.SCRIPTS[0].dma_ptr));
			    start_new_job = FALSE;
			    break;

		        default:
			    TRACE_1 ("d_default2", dma_intr)
			    start_new_job = TRUE;
                            if (SET_SXFER(SYNC_VAL) == REG_FAIL)
                            {
                                ret_code = EIO;
                                loop = FALSE;
                                break;
                            }
			    break;
		        }	/* End switch */
	   	    } /* End of if SIR */
		    /******************************************/
		    /* ABORT CAUSED THIS INTERRUPT    */
		    /******************************************/
		    if (save_interrupt & DABRT)
		    {
			DEBUG_0 ("psc_dump_intr: Abort caused interrupt\n")
			rc = psc_read_reg(ISTAT, ISTAT_SIZE);
			TRACE_1 ("du_abrt int", rc)
			/* A read of the istat register verifies that */
			/* the scsi chip is currently not busy on the */
			/* scsi bus, so we can issue the new command. */
			if (!(rc & CONNECTED))
			{
			    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
			    {
				loop = FALSE;
				ret_code = REG_FAIL;
				continue;
			    }
			    else
			    {
				dev_ptr = NULL;
				start_new_job = TRUE;
			    }
			}
			else
			{
			    (void) psc_read_reg(DSTAT, DSTAT_SIZE);
			    /* restart iowait script */
			    psc_patch_iowait_int_off();
			    rc = psc_write_reg(DSP, DSP_SIZE,
				  word_reverse(adp_str.SCRIPTS[0].dma_ptr));
			    start_new_job = FALSE;
			    if (rc == REG_FAIL)
			    {
				loop = FALSE;
				ret_code = REG_FAIL;
			    }
			}
		    }
		}	/* End DMA interrupt */
	}
	if (start_new_job)
	{
	    command_issued_flag = psc_issue_cmd();
            TRACE_1 ("strt job", command_issued_flag)
	    if ((command_issued_flag == FALSE) &&
		(adp_str.DEVICE_ACTIVE_head != NULL))
	    {
		dev_ptr = adp_str.DEVICE_ACTIVE_head;
		while (dev_ptr != NULL)
		{
		    psc_restore_iowait_jump((uint *)
					adp_str.SCRIPTS[0].script_ptr,
					dev_ptr, dev_ptr->script_dma_addr);
		    dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
		}
		psc_patch_iowait_int_off();
		psc_write_reg(DSP, DSP_SIZE, word_reverse(
					       adp_str.SCRIPTS[0].dma_ptr));
	    }
	}
    }	/* while */
    /****************************************************************/
    /* if the command did not complete within the time specified in */
    /* the command, then free the resources and return an error     */
    /****************************************************************/
    if (i >= (time_out_value * 1000))
    {
	ret_code = ETIMEDOUT;
    }
    if ((ret_code != 0) && (dev_ptr != NULL))
    {
	(void) psc_free_resources(dev_ptr, FALSE);
	dev_ptr->active = NULL;
	psc_deq_active(dev_ptr);
    }
    DEBUG_0 ("Leaving psc_dumpintr\n")
    TRACE_1 ("out dintr", ret_code)
    return (ret_code);
}  /* end psc_dump_intr */

/*************************************************************************/
/*                                                                       */
/* NAME: psc_dumpwrt                                                     */
/*                                                                       */
/* FUNCTION: Write to the dump device.                                   */
/*                                                                       */
/*          Issue as many scsi commands as neccessry to write uiop's     */
/*      data to device using the SCSI write command.  If any errors      */
/*      the fail and return with error.                                  */
/*      Actual division of labor is the dumpwrt monitors the looping     */
/*      and checks for timeouts and interrupts.  psc_dump_dev actually   */
/*      builds the command and sends it to the device.  Returning status */
/*      when it is complete.                                             */
/*                                                                       */
/* EXECUTION ENVIRONMENT:                                                */
/*                                                                       */
/*      This routine can  be called in the interrupt environment.  It    */
/*      can not page fault and is pinned.                                */
/*                                                                       */
/* (DATA STRUCTURES:)                                                    */
/*                    adapter_def    - adapter info structure            */
/*                    sc_buf    - controller info structure              */
/*                    dev_info    - device info structure                */
/*                                                                       */
/* INPUTS:                                                               */
/*      bp       -  buf scruct for the pending dump command              */
/*                                                                       */
/* CALLED BY:                                                            */
/*      psc_dump                                                         */
/*                                                                       */
/* INTERNAL PROCEDURES CALLED:                                           */
/*      psc_dump_dev     psc_dump_intr                                   */
/*                                                                       */
/* EXTERNAL PROCEDURES CALLED:                                           */
/*                                                                       */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is return- */
/*      ed and the caller is left responsible to recover from the error. */
/*                                                                       */
/* RETURNS:                                                              */
/*      EBUSY     -  request sense inuse                                 */
/*      EINVAL    -  Invalid iovec argument                              */
/*      EIO       -  I/O error                                           */
/*      ENOMEM    -  Unable to allocate resources                        */
/*      ENXIO     -  Not inited as dump device                           */
/*      ETIMEDOUT - timed out because no interrupt                       */
/*                                                                       */
/*************************************************************************/
int
psc_dumpwrt(
	    struct sc_buf * bp)
{
    /**********************/
    /* Misc. Variables    */
    /**********************/
    struct dev_info *dev_ptr;
    int     dev_index, ret_code;
    /*********************************************/
    /* We will have to poll for interrupts, and  */
    /* handle them in the apropriate ways        */
    /*********************************************/
    DEBUG_0 ("Entering psc_dumpwrt\n")
    TRACE_1 ("in dumpwrt", 0)

    if ((!adp_str.dump_inited) || (!adp_str.dump_started))
    {
	(void) psc_read_reg(SCSIRS, SCSIRS_SIZE);
	return (EINVAL);
    }
    dev_index = INDEX(bp->scsi_command.scsi_id,
		      (bp->scsi_command.scsi_cmd.lun) >> 5);
    dev_ptr = adp_str.device_queue_hash[dev_index];
    if (dev_ptr == NULL)
    {
	(void) psc_read_reg(SCSIRS, SCSIRS_SIZE);
	return (ENXIO);
    }
    ret_code = psc_dump_dev(dev_ptr);	/* send a cmd to the disk */
    if (ret_code)
	return (ret_code);

    DEBUG_0 ("Leaving psc_dumpwrt\n")
    TRACE_1 ("out dumpwrt", 0)
    return (PSC_NO_ERR);
}  /* end psc_dumpwrt */

/***************************************************************************/
/*                                                                         */
/* NAME: psc_dump_dev                                                      */
/*                                                                         */
/* FUNCTION: Send scsi write cmd to the dump device                        */
/*                                                                         */
/*      This routine fills in cmd for the write scsi command for           */
/*      all data in one iovec. It also fills in the scsi command block for */
/*      scsi write.   And Patches the script and executes it.              */
/*                                                                         */
/* EXECUTION ENVIRONMENT:                                                  */
/*                                                                         */
/*      This routine can  be called in the interrupt environment.  It      */
/*      can not page fault and is pinned.                                  */
/*                                                                         */
/* (DATA STRUCTURES:)                                                      */
/*                    adapter_def    - adapter info structure              */
/*                    sc_buf    - controller info structure                */
/*                    dev_info    - device info structure                  */
/*                                                                         */
/* INPUTS:                                                                 */
/*                                                                         */
/*    dev_info structure - pointer to device information structure         */
/*                                                                         */
/* CALLED BY:                                                              */
/*      psc_dumpwrt                                                        */
/*                                                                         */
/* INTERNAL PROCEDURES CALLED:                                             */
/*                                                                         */
/* EXTERNAL PROCEDURES CALLED:                                             */
/*                                                                         */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned  */
/*      and the caller is left responsible to recover from the error.      */
/*                                                                         */
/*                                                                         */
/* RETURNS:                                                                */
/*      EINVAL -  Invalid iovec argument                                   */
/*      EIO    -  I/O error                                                */
/*      ENOMEM -  Unable to allocate resources                             */
/*                                                                         */
/***************************************************************************/
int
psc_dump_dev(
	     struct dev_info * dev_ptr)
{
    DEBUG_0 ("Entering psc_dump_dev routine.\n")
    TRACE_1 ("in dump_dev", 0)
    if (psc_alloc_resources(dev_ptr) != PSC_NO_ERR)
	return (ENOMEM);
    if (adp_str.DEVICE_ACTIVE_head == NULL)
    {
        /* assumes that negotiation is unnecessary at this point */
        if (!dev_ptr->negotiate_flag)
        {
	    if (dev_ptr->async_device)
	    {
	        if (SET_SXFER(0x00) == REG_FAIL)
	        {
		    return (REG_FAIL);
	        }
	    }
	    else
	        if ((dev_ptr->agreed_xfer != DEFAULT_MIN_PHASE) ||
		    (dev_ptr->agreed_req_ack < DEFAULT_BYTE_BUF))
	        {
		    if (SET_SXFER(dev_ptr->special_xfer_val) == REG_FAIL)
		    {
		        return (REG_FAIL);
		    }
	        }
	    else
	    {
	        if (SET_SXFER(SYNC_VAL) == REG_FAIL)
	        {
		    return (REG_FAIL);
	        }
	    }
	    /* patch labels to script */
	    psc_prep_main_script((uint *) adp_str.SCRIPTS[0].script_ptr,
		(uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
			dev_ptr, dev_ptr->script_dma_addr);
	    /* start dump SCRIPT */
	    ISSUE_MAIN_TO_DEVICE(dev_ptr->script_dma_addr);
	    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	    psc_enq_active(dev_ptr);
	}
        else
        {
	    return (REG_FAIL);
        }
    }
    else
    {
	TRACE_1 ("dmp enqw", (int) dev_ptr)
	psc_enq_wait(dev_ptr);
    }
    DEBUG_0 ("Leaving psc_dump_dev routine.\n");
    TRACE_1 ("out dump_dev", 0);
    return (PSC_NO_ERR);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_cdt_func                                            */
/*                                                                      */
/* FUNCTION:    Adapter Driver Component Dump Table Routine             */
/*                                                                      */
/*      This routine builds the driver dump table during a system dump. */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, so it cannot malloc   */
/*      or free memory.                                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      cdt     - the structure used in the master dump table to keep   */
/*                track of component dump entries.                      */
/*                                                                      */
/* INPUTS:                                                              */
/*      arg     - =1 dump dd is starting to get dump table entries.     */
/*                =2 dump dd is finished getting the dump table entries.*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      Return code is a pointer to the struct cdt to be dumped for     */
/*      this component.                                                 */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      strcpy                                                          */
/*                                                                      */
/************************************************************************/
struct cdt *
psc_cdt_func(
	     int arg)
{
    DEBUG_0 ("Entering psc_cdt_func routine.\n");

    if (arg == 1)
    {
	/* only build the dump table on the initial dump call */

	/* init the table */
	bzero((char *) psc_cdt, sizeof(struct psc_cdt_table));

	/* init the head struct */
	psc_cdt->psc_cdt_head._cdt_magic = DMP_MAGIC;
	strcpy(psc_cdt->psc_cdt_head._cdt_name, "pscsi");
	/* _cdt_len is filled in below */

	/* now begin filling in elements */
	psc_cdt->psc_entry[0].d_segval = 0;
	strcpy(psc_cdt->psc_entry[0].d_name, "adp_str");
	psc_cdt->psc_entry[0].d_ptr = (char *) &adp_str;
	psc_cdt->psc_entry[0].d_len = (sizeof(struct adapter_def));

	/* fill in the actual table size */
	psc_cdt->psc_cdt_head._cdt_len = sizeof(struct cdt_head) +
                                         sizeof(struct cdt_entry);

    }
    return ((struct cdt *) psc_cdt);

}  /* end psc_cdt_func */

/**************************************************************************/
/*                                                                        */
/* NAME:        psc_fail_cmd                                              */
/*                                                                        */
/* FUNCTION:    Fail Active and Pending Commands Routine                  */
/*                                                                        */
/*      This internal routine is called by the interrupt handler          */
/*      to clear out pending and active commands for a particular         */
/*      device which has experienced some sort of failure which is        */
/*      not recoverable by the adapter driver.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called on priority levels greater        */
/*      than, or equal to that of the interrupt handler.                  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - adapter unique data structure (one per adapter)     */
/*      sc_buf  - input/output request struct used between the adapter    */
/*                driver and the calling SCSI device driver               */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_info structure - pointer to device information structure      */
/*      queue_type - which queue 0 = active 1= pending 2=both             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  The following are the return values:        */
/*      none                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      i_disable       i_enable                                          */
/*      iodone          w_stop                                            */
/*                                                                        */
/**************************************************************************/
void
psc_fail_cmd(
	     struct dev_info * dev_ptr,
	     int queue_type)
{
    struct sc_buf *bp;
    int     old_pri;

    DEBUG_0 ("Entering psc_fail_cmd routine.\n")
    TRACE_1 ("in failC", (int) dev_ptr)

    /* this assumes that for the failed command(s), the  */
    /* following are set previously:  sc_buf status,     */
    /* sc_buf resid value, and sc_buf b_error field.     */
    old_pri = i_disable(adp_str.ddi.int_prior);

    /* clean up active cmd queue */
    if ((queue_type != 1) &&	/* active queue or both */
	(dev_ptr->active != NULL))
    {
	bp = dev_ptr->active;
	/* set b_flags B_ERROR flag */
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_error = ENXIO;
	(void) psc_free_resources(dev_ptr, FALSE);
	iodone((struct buf *) bp);
	dev_ptr->flags = RETRY_ERROR;
	dev_ptr->active = NULL;	/* reset tail pointer */
    }	/* end of if */
    /* clean up pending cmd queue */
    if (queue_type != 0)
    {
	bp = dev_ptr->head_pend;
	while (bp != NULL)
	{
	    bp->status_validity = 0;
	    bp->bufstruct.b_flags |= B_ERROR;	/* set b_flags B_ERROR flag */
	    bp->bufstruct.b_error = ENXIO;	
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;

	    /* point to next sc_buf in pending chain, if any */
	    dev_ptr->head_pend = (struct sc_buf *) bp->bufstruct.av_forw;
	    iodone((struct buf *) bp);
	    bp = dev_ptr->head_pend;
	}	/* endwhile */
	dev_ptr->tail_pend = NULL;	/* reset tail pointer */
    }
    if (dev_ptr->queue_state == STOPPING)
    {
	e_wakeup(&dev_ptr->stop_event);
    }
    else
    {
	dev_ptr->queue_state = HALTED;
    }
    TRACE_1 ("out fail", (int) dev_ptr)
    i_enable(old_pri);
}  /* end psc_fail_cmd */

/*************************************************************************/
/*                                                                      */
/* NAME:        psc_delay                                               */
/*                                                                      */
/* FUNCTION:    provide variable delay in 1 usec increments             */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*     process or interrupt                                             */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      delay - number of usecs to delay                                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  IOCC_ATT, IOCC_DET, BUS_GETCX           */
/*                                                                      */
/************************************************************************/

void
psc_delay(int delay)
{
    caddr_t iocc_addr;
    int     i;
    uchar   no_val;

    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
    for (i = 0; i < delay; i++)
	/* GETCX due to hardware restriction */
	(void) BUS_GETCX((iocc_addr + IOCC_DELAY), (char *) &no_val);
    IOCC_DET(iocc_addr);
}

/*************************************************************************/
/*                                                                      */
/* NAME:        psc_intr                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Interrupt Handler                        */
/*                                                                      */
/*      This routine processes adapter interrupt conditions.            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, therefore, it must    */
/*      only perform operations on pinned data.                         */
/*                                                                      */
/* NOTES:  This routine handles interrupts which are caused by scsi     */
/*     scripts processing.  This entails processing the interrupts      */
/*     handling error cleanup and logging, and issuing outstanding      */
/*     commands for other devices                                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      dev_info - driver structure which holds information related to  */
/*                 a particular device and hence a command              */
/*      intr    - kernel interrupt information structure                */
/*                                                                      */
/* INPUTS:                                                              */
/*      handler - pointer to the intrpt handler structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      INTR_FAIL    interrupt was not processed                        */
/*      INTR_SUCC    interrupt was processed                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*     w_start                                                          */
/*     w_clear             i_reset                                      */
/*     iodone              e_wakeup                                     */
/*     bzero               d_complete                                   */
/*                                                                      */
/************************************************************************/
int
psc_intr(struct intr * handler)
{
    struct sc_buf *bp;
    struct dev_info *dev_ptr;
    int     save_isr, save_dsr;	/* save status registers */
    int     save_interrupt, dma_stat;
    int     command_issued_flag, check_for_disconnect;
    int     issue_abrt_bdr;	/* are we doing abort/bdr */
    int     dev_index;
    int     dma_addr;
    int     start_new_job;	/* 1=start new job up */
    uchar   save_stat, chip_abort_occurred;
    register int rc;

    DEBUG_0 ("Entering psc_intr routine\n")
    TRACE_1 ("in intr", 0)

    /* check and make sure its a scsi interrupt */
    rc = psc_read_reg(SCSIRS, SCSIRS_SIZE);
    DEBUG_1 ("psc_intr: SCSIRS = 0x%x\n", rc);
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, adp_str.devno);
    /* channel check and associated interrupts */
    if (!((rc == REG_FAIL) || (rc & 0x000000F0)))
    {
	/* Check to see if this is our interrupt */
	if (!(rc & 0x00000001))
	{
	    /* stop the adapter's watchdog timer for aborts */
	    w_stop(&adp_str.adap_watchdog.dog);

	    /* Check to see if a command is active */
	    if (adp_str.DEVICE_ACTIVE_head != NULL)
	    {
		dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
				    adp_str.STA_tcw + 1);
		rc = d_complete((int) adp_str.channel_id, DMA_TYPE,
				(char *) adp_str.SCRIPTS[0].script_ptr,
				(size_t) PAGESIZE,
				&adp_str.xmem_SCR, (char *) dma_addr);
		/* if we got a system dma error, we need to kill */
		/* all jobs on all devices.                      */
		if (rc == DMA_SUCC)
		{
		    /* read the ISTAT register to see which interrupt */
		    /* caused us to be called                         */
		    save_isr = psc_read_reg(ISTAT, ISTAT_SIZE);
		    DEBUG_1 ("ISTAT is 0x%x\n", save_isr);
		    if (save_isr != REG_FAIL)
		    {
			if (save_isr & 0x80)
			{	/* if a chip abort has occurred */
			    /* clear the interrupt */
			    chip_abort_occurred = TRUE;
			    if (psc_write_reg(ISTAT, ISTAT_SIZE,
				(uchar) 0x00) == REG_FAIL)
			    {
				TRACE_1 ("bad Wist", 0)
				TRACE_1 ("out intr", 0)
				psc_cleanup_reset(REG_ERR);
				i_reset(&(adp_str.intr_struct));
				DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 
					adp_str.devno);
				return (INTR_SUCC);
			    }
			}
			else
			{
			    chip_abort_occurred = FALSE;
			}
			/* initialize flags */
			start_new_job = FALSE;
			/********************************************/
			/* DMA INTERRUPT               */
			/********************************************/
			if ((save_isr & DIP) && (!(save_isr & SIP)))
			    /* DMA caused this interrupt */
			{
			    /* verify the interrupt and check for other
			       conditions */
			    rc = psc_read_reg(DSTAT, DSTAT_SIZE);
			    save_dsr = (rc & DIEN_CHK_MASK);
			    DEBUG_1 ("DSTAT is 0x%x\n", save_dsr);
			    if (rc == REG_FAIL)
			    {
				DEBUG_1 ("psc_intr: save_dsr = 0x%x\n",
					  save_dsr);
				TRACE_1 ("bad dsta", 0)
				TRACE_1 ("out intr", 0)
				psc_cleanup_reset(REG_ERR);
				i_reset(&(adp_str.intr_struct));
				DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 
					adp_str.devno);
				return (INTR_SUCC);
			    }
			    /* if a chip abort was set but another   */
			    /* interrupt won the race, clear the     */
			    /* stacked interrupt for which the abort */
			    /* has taken place.                      */
			    if ((chip_abort_occurred) && (!(save_dsr & DABRT)))
			    {
				(void) psc_read_reg(DSTAT, DSTAT_SIZE);
			    }
			    /**********************************************/
			    /*         PLANNED SCRIPT INTERRUPT           */
			    /**********************************************/
			    if (save_dsr & SIR)
			    {
				dma_stat = word_reverse(psc_read_reg(DSPS,
							DSPS_SIZE));
				DEBUG_1 ("PLANNED INTR is 0x%x\n", dma_stat);
				if (dma_stat == REG_FAIL)
				{
				    DEBUG_0 ("psc_intr: bad reg read DSPS\n")
				    TRACE_1 ("bad dsps", 0)
				    TRACE_1 ("out intr", 0)
				    psc_cleanup_reset(REG_ERR);
				    i_reset(&(adp_str.intr_struct));
				    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR,
					    0, adp_str.devno);
				    return (INTR_SUCC);
				}
				TRACE_1 ("dma_stat", dma_stat)
				/* this contains the value of the inter. */
				/* which was set by us in the SCRIPTS to */
				/* to tell us the  type of interupt, the */
				/* location of the interrupt within the */
				/* the SCRIPT, and the device which this */
				/* interrupted SCRIPTS is running.      */
				/* top 16 bits are location of script   */
				/* bottom 16 bits are type of interrupt. */

				/* get top 16 bits which is index into  */
				/* array to get pointer to dev_info     */
				dev_index = ((0xffff0000 & dma_stat) >> 16);
				/* if we get a bad dev_ptr get out */
				dev_ptr =
				    adp_str.device_queue_hash[dev_index];
				if (dev_ptr == NULL)
				{
				    if (((dma_stat & 0x0000ffff) != A_io_wait)
					&& ((dma_stat & 0x0000ffff) !=
					    A_unknown_reselect_id)
					&& ((dma_stat & 0x0000ffff) !=
					    A_uninitialized_reselect))
				    {
					DEBUG_0 ("psc_intr: got bad dev_ptr\n")
					TRACE_1 ("bad dptr", 0)
					TRACE_1 ("out intr", 0)
					psc_logerr(ERRID_SCSI_ERR2,
						   ADAPTER_INT_ERROR,
						   85, 0, NULL, TRUE);
					i_reset(&(adp_str.intr_struct));
					DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR,
						0, adp_str.devno);
					return (INTR_SUCC);
				    }
				}
				else
				{
				    if ((adp_str.ABORT_BDR_head == dev_ptr) ||
					(dev_ptr->ABORT_BDR_bkwd != NULL))
				      /* there is an abort or bdr to queued */
					issue_abrt_bdr = TRUE;

				    else
				      /* there isn't an abort or bdr queued */
					issue_abrt_bdr = FALSE;
				}
				/* get bottom 16 bits ( interrupt type) so we
				   can switch it */
				save_interrupt = 0x0000ffff & dma_stat;
				/******************************************/
				/*       SWITCH ON TYPE OF INTERRUPT      */
				/******************************************/
				switch (save_interrupt)
				{	/* completed command */
				case A_check_next_io:
				    TRACE_1 ("chk next", (int) dev_ptr)
				    case A_check_next_io_data:
				    /* the device is about to disc. from */
				    /* the bus. and a save DP has just   */
				    /* been sent. Move any data xferred  */
				    /* into the buffer and adjust the    */
				    /* data pointer within our buffer to */
				    /* show the buffer size has changed. */
				    TRACE_1 ("chk next dat", (int) dev_ptr)
				    if (save_interrupt == A_check_next_io_data)
				    {
					dev_ptr->active->bufstruct.b_resid = 0;
				    }
				    else
					if (dev_ptr->flags & RESID_SUBTRACT)
					{
					    dev_ptr->active->bufstruct.b_resid
						-= dev_ptr->bytes_moved;
					}
				    /* Turn off resid subtract */
				    dev_ptr->flags &= ~RESID_SUBTRACT;
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
				    }
				    else
				    {
					dev_ptr = NULL;
					start_new_job = TRUE;
				    }
				    break;

				case A_io_done:
				case A_io_done_after_data:
				    TRACE_1 ("A_iodone", (int) dev_ptr)
				    DEBUG_0 ("psc_intr: Current I/O done\n")
				            w_stop(&dev_ptr->dev_watchdog.dog);
				    if (SET_SXFER(SYNC_VAL) 
					== REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    /* reset completion flags */
				    if (save_interrupt == A_io_done_after_data)
				    {
					dev_ptr->active->bufstruct.b_resid = 0;
					/* resid = 0 */
				    }
				    else
					if (dev_ptr->flags & RESID_SUBTRACT)
					{
					    dev_ptr->active->bufstruct.b_resid
						-= dev_ptr->bytes_moved;
					    /* do subtraction */
					}
				    if (issue_abrt_bdr)
				    {
					TRACE_1 ("A_iodBDR", (int) dev_ptr)
					        start_new_job =
						psc_issue_abort_bdr(dev_ptr,
								    TRUE);
				    }
				    else	/* issue a cmd */
				    {
					/* issue a command and complete
					   previous command */
					/* get status byte because issue will
					   destroy it */
					save_stat = GET_CMD_STATUS_BYTE(
						adp_str.SCRIPTS[dev_ptr->
						cmd_script_ptr].script_ptr);
					DEBUG_1 ("STATUS BUFFER = 0x%x\n",
						  save_stat);

					bp = dev_ptr->active;
                                        /* special trace hook to note that */
                                        /* command is done */
                                        DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0,
                                                adp_str.devno, bp);

					command_issued_flag = psc_issue_cmd();
					if (save_stat == SC_GOOD_STATUS)
					{	/* scsi status is ok */
					    if ((bp != NULL) &&
						(!(bp->bufstruct.b_flags &
						 B_ERROR)))
					    {
						TRACE_1 ("A_iodgds", (int)
							 dev_ptr)
						/* set scsi status in sc_buf */
						bp->bufstruct.b_error = 0;
						bp->status_validity = 0;
						bp->scsi_status =
							     SC_GOOD_STATUS;
						/* free resources */
						rc = psc_free_resources(
							     dev_ptr,TRUE);
						if (rc != PSC_NO_ERR)
						{
						    if (psc_fail_free_resources
							(dev_ptr, bp, rc))
							break;
						    if (dev_ptr->queue_state
							== STOPPING)
							e_wakeup(
							 &dev_ptr->stop_event);
						    else
							dev_ptr->queue_state
							  = HALTED;
						    break;
						}
						else
						{
						    iodone((struct buf *) bp);
						    if ((dev_ptr->queue_state
							 == STOPPING) &&
							(dev_ptr->head_pend
							 == NULL))
							e_wakeup(
							 &dev_ptr->stop_event);
						}
					    }
					    else
						if (bp != NULL)
						{	/* previous error */
						    TRACE_1 ("A_iodper",
							      (int) dev_ptr)
						    (void) psc_free_resources(
							      dev_ptr, FALSE);
						    iodone((struct buf *)
							    dev_ptr->active);
						    if (dev_ptr->head_pend
							!= NULL)
							psc_fail_cmd(
							  dev_ptr, 1);
						    if (dev_ptr->queue_state
							== STOPPING)
							e_wakeup(
							 &dev_ptr->stop_event);
						    else
							dev_ptr->queue_state
							   = HALTED;
						}
					}
					else	/* status is not okay */
					{
					    if ((bp != NULL) &&
						(!(bp->bufstruct.b_flags
						   & B_ERROR)))
					    {
						TRACE_2 ("A_iodchk", save_stat,
							 (int) dev_ptr)
						/* set scsi stat in sc_buf */
						bp->scsi_status = save_stat;
						bp->status_validity =
							      SC_SCSI_ERROR;
						bp->bufstruct.b_flags |=
							      B_ERROR;
						bp->bufstruct.b_error = EIO;
						rc = psc_free_resources(
							      dev_ptr, FALSE);
						if (rc != PSC_NO_ERR)
						{
						    if (psc_fail_free_resources
							(dev_ptr, bp, rc))
							break;
						}
						else
						    iodone((struct buf *)
							    dev_ptr->active);
					    }
					    else
						if (bp != NULL)
						{	/* previous error */
						    TRACE_1 ("A_iodpe2",
							      (int) dev_ptr)
						    (void) psc_free_resources(
							     dev_ptr, FALSE);
						    iodone((struct buf *)
							    dev_ptr->active);
						}

					    if (dev_ptr->head_pend != NULL)
						psc_fail_cmd(dev_ptr, 1);
					    if (dev_ptr->queue_state ==
						STOPPING)
						e_wakeup(&dev_ptr->stop_event);
					    else
						dev_ptr->queue_state = HALTED;
					}	/* end else */
					dev_ptr->flags = RETRY_ERROR;
					dev_ptr->active = NULL;
					psc_deq_active(dev_ptr);
					psc_check_wait_queue(dev_ptr,
						       command_issued_flag);
				    }	/* end else  cmd */
				    break;

				case A_set_special_sync:
				    DEBUG_0 ("psc_intr: A_set_special_sync \n")
				    TRACE_1 ("spc sync", (int) dev_ptr)
				    /* set xfer mode in chip to SYNC,   */
			            if (SET_SXFER(dev_ptr->special_xfer_val)
						 == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }

				    /* and restart script */
				    rc = psc_read_reg(DSP, DSP_SIZE);
				    if (rc == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    if (psc_write_reg(DSP, DSP_SIZE, rc)
					 == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
				    }
				    break;

				case A_change_to_async:
				    DEBUG_0 ("psc_intr: A_change_to_async \n")
				    TRACE_1 ("chg asyn", (int) dev_ptr)
				    /* set xfer mode in chip to ASYNC */
				    if (SET_SXFER(0x00) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }

				    /* and restart script */
				    rc = psc_read_reg(DSP, DSP_SIZE);
				    if (rc == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    if (psc_write_reg(DSP, DSP_SIZE, rc)
					 == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    break;

				case A_io_wait:
				    TRACE_1 ("io wait ", (int) dev_ptr)
				    psc_patch_iowait_int_off();
				    rc = psc_write_reg(DSP, DSP_SIZE,
					  word_reverse(
					   adp_str.SCRIPTS[0].dma_ptr));
				    if (rc == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
				    }
				    break;

				case A_sync_neg_done:
				    TRACE_1 ("neg done", (int) dev_ptr)
				    /* sync complete and successful */
				    dev_ptr->negotiate_flag = FALSE;
				    dev_ptr->async_device = FALSE;
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }

				    DEBUG_0 ("psc_intr: SYNC_NEG_DONE \n")
				    /* supports synch negotiation */
				    if ((rc = psc_verify_neg_answer((uint *)
					adp_str.SCRIPTS
					[dev_ptr->cmd_script_ptr].script_ptr,
					dev_ptr, dev_index)) == PSC_NO_ERR)
				    {
				        w_stop(&dev_ptr->dev_watchdog.dog);

				        if (issue_abrt_bdr)
					/* something on abort queue */
					    start_new_job =
					           psc_issue_abort_bdr(dev_ptr,
							 FALSE);
				        else	
					/* nothing on abort queue */
				        {
					    /* setup script to issue the */
					    /* command already tagged to */
					    /* this device queue  */
					    if ((bp = dev_ptr->active) != NULL)
					    {
					        psc_prep_main_script(
						    (uint *) adp_str.SCRIPTS
						    [0].script_ptr, (uint *)
						    adp_str.SCRIPTS
						     [dev_ptr->cmd_script_ptr].
						     script_ptr, dev_ptr,
						    dev_ptr->script_dma_addr);
					        ISSUE_MAIN_AFTER_NEG(
						    dev_ptr->script_dma_addr);
                                                if (bp->timeout_value == 0)
                                                      dev_ptr->dev_watchdog.dog.
                                                        restart = (ulong) 0;
                                                else
                                                      dev_ptr->dev_watchdog.dog.
                                                        restart = (ulong)
                                                      bp->timeout_value + 1;
					        w_start(&dev_ptr->
						    dev_watchdog.dog);
					        dev_ptr->cmd_activity_state =
						    CMD_IN_PROGRESS;
					    }
					    else
					    {
					        start_new_job = TRUE;
					    }
					}
				    }
                                    else  /* verify_neg_answer rc is  */
                                    {     /* non-zero, we must either */
                                          /* issue a msg reject, or   */
                                          /* recover from the bad pio */
                                        if (rc == PSC_FAILED)
					{
					  /* need to issue msg reject */
					  /* First, prep the script   */
					    if ((bp = dev_ptr->active) != NULL)
					    {
					        psc_prep_main_script(
						    (uint *) adp_str.SCRIPTS
						    [0].script_ptr, (uint *)
						    adp_str.SCRIPTS
						     [dev_ptr->cmd_script_ptr].
						     script_ptr, dev_ptr,
						    dev_ptr->script_dma_addr);
                                                if (bp->timeout_value == 0)
                                                      dev_ptr->dev_watchdog.dog.
                                                        restart = (ulong) 0;
                                                else
                                                      dev_ptr->dev_watchdog.dog.
                                                        restart = (ulong)
                                                      bp->timeout_value + 1;
					        w_start(&dev_ptr->
						    dev_watchdog.dog);
					        dev_ptr->cmd_activity_state =
						    CMD_IN_PROGRESS;
					    }
                                            rc = psc_write_reg((uint) DSP,
                                                (char) DSP_SIZE, word_reverse(
                                                dev_ptr->script_dma_addr +
                                                Ent_reject_target_sync));
					}
                                        if (rc == REG_FAIL)
                                        {
                                            psc_cleanup_reset(REG_ERR);
                                        }
                                    }

				    break;

				case A_sync_msg_reject:
				    TRACE_1 ("sync rej", (int) dev_ptr) 
				    /* we tried to negotiate for synch  */
				    /* mode but failed. ASYNC mode set  */
				    /* in psc_patch_async               */
				    dev_ptr->negotiate_flag = FALSE;
				    dev_ptr->async_device = TRUE;
				    psc_prep_main_script((uint *)
					adp_str.SCRIPTS[0].script_ptr,
					(uint *) adp_str.SCRIPTS
					 [dev_ptr->cmd_script_ptr].script_ptr,
					dev_ptr, dev_ptr->script_dma_addr);
				    dev_ptr->cmd_activity_state =
							    CMD_IN_PROGRESS;
				    psc_patch_async_switch_int(
					(uint *) adp_str.SCRIPTS
					[dev_ptr->cmd_script_ptr].script_ptr,
					dev_index);
				    DEBUG_0 ("psc_intr: NEG_FAILED \n")
				    if (SET_SXFER(0x00) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    /* and restart script */
				    rc = psc_read_reg(DSP, DSP_SIZE);
				    if (rc == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    rc = psc_write_reg(DSP, DSP_SIZE, rc);
				    if (rc == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
				    }
				    break;

				case A_ext_msg:
				case A_modify_data_ptr:
				case A_target_sync_sent:
				    DEBUG_0 ("psc_intr: mod. data ptr \n")
				    TRACE_1 ("ext msg ", (int) dev_ptr)
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    if (psc_handle_extended_messages(
					      (uint *) adp_str.SCRIPTS
					       [dev_ptr->cmd_script_ptr].
					       script_ptr,
					      (uint *) dev_ptr->script_dma_addr,
					      dev_index, dev_ptr,
					      save_interrupt) == REG_FAIL)
				    {	/* register error */
					psc_cleanup_reset(REG_ERR);
				    }
				    break;

				case A_abort_io_complete:
				case A_bdr_io_complete:
				    DEBUG_0 ("psc_intr: ABRBDR_IO_COMPLETE \n")
				    TRACE_1 ("A/B done", (int) dev_ptr)
				    w_stop(&dev_ptr->dev_watchdog.dog);
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    /* remove device from active queue */
				    /* if there is a command tagged to this */
				    if ((bp = dev_ptr->active) != NULL)
				    {
					if (!(bp->bufstruct.b_flags & B_ERROR))
					{	/* no previous error */
					    bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
					    bp->bufstruct.b_flags |= B_ERROR;
					    bp->bufstruct.b_error = ENXIO;
					}
					rc = psc_free_resources(dev_ptr,
								FALSE);
					if (rc != PSC_NO_ERR)
					{
					    if (psc_fail_free_resources(
						 dev_ptr, bp, rc))
						break;
					}
					else
					    /* finish off active and pending */
					    /* commands */
					    iodone((struct buf *) bp);
				    }
				    dev_ptr->flags = RETRY_ERROR;
				    dev_ptr->active = NULL;
				    /* iodone pending commands and go */
				    /* issue a new one */
				    if (dev_ptr->head_pend != NULL)
					psc_fail_cmd(dev_ptr, 1);
				    if (dev_ptr->queue_state == STOPPING)
					e_wakeup(&dev_ptr->stop_event);
				    else
					dev_ptr->queue_state = HALTED;
				    if (dev_ptr->ioctl_wakeup == TRUE)
				    {
					dev_ptr->ioctl_errno = PSC_NO_ERR;
					dev_ptr->ioctl_wakeup = FALSE;
					e_wakeup(&dev_ptr->ioctl_event);
				    }
				    if (dev_ptr->flags & SCSI_BDR)
				    {
					dev_ptr->negotiate_flag = TRUE;
					dev_ptr->restart_in_prog = TRUE;
					adp_str.restart_watchdog.dog.restart =
					    adp_str.ddi.cmd_delay + 1;
					w_start(
					   &(adp_str.restart_watchdog.dog));
				    }
				    psc_deq_active(dev_ptr);
				    start_new_job = TRUE;
				    break;

				case A_neg_select_failed:
				case A_cmd_select_atn_failed:
				    TRACE_1 ("Cneg bad", (int) dev_ptr)
				    /* We got beaten to the bus, probably by */
				    /* a cmd completion.  Put the script onto*/
				    /* waiting to execute queue and go to the*/
				    /* WAIT SCRIPT. This isn't an error but a*/
				    /* change of status.                     */

				    DEBUG_0 ("psc_intr: select_atn_failed \n")
				    w_stop(&dev_ptr->dev_watchdog.dog);
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    psc_deq_active(dev_ptr);
				    /* place this dev on front of WAIT Q */
				    if (adp_str.DEVICE_WAITING_head == NULL)
				    {
					dev_ptr->DEVICE_WAITING_fwd = NULL;
					adp_str.DEVICE_WAITING_head = dev_ptr;
					adp_str.DEVICE_WAITING_tail = dev_ptr;
				    }
				    else	/* list not empty */
				    {
					dev_ptr->DEVICE_WAITING_fwd =
					    adp_str.DEVICE_WAITING_head;
					adp_str.DEVICE_WAITING_head = dev_ptr;
				    }
				    /* if no commands are outstanding, the */
				    /* scsi bus is hung. Attempt a scsi bus */
				    /* reset to clear up this condition.   */
				    if (adp_str.DEVICE_ACTIVE_head == NULL)
				    {
					psc_command_reset_scsi_bus();
				    }
				    else
				    {
					/* restart IOWAIT routine */
					if (psc_write_reg(DSP, DSP_SIZE,
					    word_reverse(adp_str.SCRIPTS[0].
						      dma_ptr)) == REG_FAIL)
					    psc_cleanup_reset(REG_ERR);
				    }
				    break;

				case A_abort_select_failed:
				    /* someone beat us to the bus. */
				    /* So we have to */
				    /* remember to reissue the abort again */
				    /* when the bus frees up.  */

				case A_bdr_select_failed:
				    DEBUG_0 ("psc_intr: ABRTBDR_SEL_FAILED \n")
				    TRACE_1 ("Aneg bad", (int) dev_ptr)
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    /* if there's an active command this */
				    /* abort/bdr was issued for, continue */
				    /* to time the active command.       */
				    if (dev_ptr->flags & RETRY_ERROR)
				    {
					w_stop(&dev_ptr->dev_watchdog.dog);
					dev_ptr->dev_watchdog.dog.restart =
					    LONGWAIT;
					dev_ptr->flags &= ~RETRY_ERROR;
					dev_ptr->retry_count = 1;
					w_start(&dev_ptr->dev_watchdog.dog);
				    }
				    if (dev_ptr->active == NULL)
				    {	/* no active command */
					psc_deq_active(dev_ptr);
				    }
				    else
				    {	/* restore the iowait pointer for  */
					/* this device.                    */
					psc_restore_iowait_jump( (uint *)
					      adp_str.SCRIPTS[0].script_ptr,
					 dev_ptr, dev_ptr->script_dma_addr);
				    }
				    /* put device on front of abort queue   */
				    if (adp_str.ABORT_BDR_head == NULL)
				    {
					dev_ptr->ABORT_BDR_fwd = NULL;
					dev_ptr->ABORT_BDR_bkwd = NULL;
					adp_str.ABORT_BDR_head = dev_ptr;
					adp_str.ABORT_BDR_tail = dev_ptr;
				    }
				    else	/* list not empty */
				    {
					adp_str.ABORT_BDR_head->ABORT_BDR_bkwd
					  = dev_ptr;
					dev_ptr->ABORT_BDR_bkwd = NULL;
					dev_ptr->ABORT_BDR_fwd =
					   adp_str.ABORT_BDR_head;
					adp_str.ABORT_BDR_head = dev_ptr;
				    }
				    /* if no commands are outstanding, the */
				    /* scsi bus is hung. Attempt a scsi bus */
				    /* reset to clear up this condition.   */
				    if ((adp_str.DEVICE_ACTIVE_head == NULL) ||
					(dev_ptr->retry_count >=
					 PSC_RETRY_COUNT))
				    {
					psc_command_reset_scsi_bus();
				    }
				    else
				    {
					/* restart IOWAIT routine */
					if (psc_write_reg(DSP, DSP_SIZE,
					    word_reverse(adp_str.SCRIPTS[0].
						      dma_ptr)) == REG_FAIL)
					    psc_cleanup_reset(REG_ERR);
				    }
				    break;

				    /* for any phase or unknown message */
				    /* error do an abort */
				case A_phase_error:
				    TRACE_1 ("Aphaserr", (int) dev_ptr)
				case A_err_not_ext_msg:
				    TRACE_1 ("not ext ", (int) dev_ptr)
				case A_unknown_msg:
				    DEBUG_0 ("psc_intr: MSG_ERR Phas_error\n")
				    TRACE_1 ("unk msg ", (int) dev_ptr)
				    w_stop(&dev_ptr->dev_watchdog.dog);
				    bp = dev_ptr->active;
				    if ((bp != NULL) &&
					(!(bp->bufstruct.b_flags & B_ERROR)))
				    {	/* there's no previous error so log */
					/* this one.                        */
					switch (save_interrupt)
					{	/* for command logging */
					case A_phase_error:
					    psc_logerr(ERRID_SCSI_ERR10,
							PHASE_ERROR, 90, 0,
							dev_ptr, TRUE);
					    break;
					case A_err_not_ext_msg:
					    psc_logerr(ERRID_SCSI_ERR10,
							ERROR_NOT_EXTENT, 90,
							0, dev_ptr, TRUE);
					    break;
					case A_unknown_msg:
					default:
					    psc_logerr(ERRID_SCSI_ERR10,
							UNKNOWN_MESSAGE, 90,
							0, dev_ptr, TRUE);
					    break;
					}
				    }
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }

				    if ((dev_ptr->cmd_activity_state
					 == ABORT_IN_PROGRESS) ||
					(dev_ptr->cmd_activity_state
					 == BDR_IN_PROGRESS))
				    {
					/* remove device from active queue */
					/* if there is a command tagged to
					   this */
					if (bp != NULL)
					{
					    if (!(bp->bufstruct.b_flags &
						  B_ERROR))
					    {	/* no previous error */
						bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
						bp->status_validity =
						     SC_ADAPTER_ERROR;
						bp->general_card_status =
						     SC_SCSI_BUS_FAULT;
						bp->bufstruct.b_flags |=
						     B_ERROR;
						bp->bufstruct.b_error = EIO;
					    }
					}
					psc_command_reset_scsi_bus();
					break;
				    }
				    /* update wait script */
				    psc_reset_iowait_jump((uint *)
					       adp_str.SCRIPTS[0].script_ptr,
					       dev_index);
				    /* issue an abort script call */
				    if (psc_issue_abort_script(
					 (uint *) adp_str.SCRIPTS
				         [dev_ptr->cmd_script_ptr].script_ptr,
					 (uint *) dev_ptr->script_dma_addr, 
					 dev_ptr, dev_index, save_isr 
					 & CONNECTED) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    dev_ptr->cmd_activity_state =
							  ABORT_IN_PROGRESS;
				    dev_ptr->dev_watchdog.dog.restart =
							  LONGWAIT;
				    dev_ptr->flags |= SCSI_ABORT;
				    dev_ptr->flags &= ~RETRY_ERROR;
				    dev_ptr->retry_count = 1;
				    w_start(&dev_ptr->dev_watchdog.dog);
				    /* log error */
				    if (bp != NULL)
				    {
					/* this error type requires an abort */
					/* to be sent to the offending device*/

					bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
					bp->status_validity = SC_ADAPTER_ERROR;
					bp->general_card_status =
						     SC_SCSI_BUS_FAULT;
					bp->bufstruct.b_flags |= B_ERROR;
					bp->bufstruct.b_error = EIO;
				    }
				    if (dev_ptr->ioctl_wakeup == TRUE)
				    {
					dev_ptr->ioctl_errno = EIO;
					dev_ptr->ioctl_wakeup = FALSE;
					e_wakeup(&dev_ptr->ioctl_event);
				    }
				    if (issue_abrt_bdr)
					psc_deq_abort_bdr(dev_ptr);
				    break;

				case A_bdr_msg_error:
				case A_abort_msg_error:
				    DEBUG_0 ("psc_intr:A_abort_err or A_bdr\n")
				    TRACE_1 ("Amsg err", (int) dev_ptr)
				    w_stop(&dev_ptr->dev_watchdog.dog);
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }

				    /* update wait script  */
				    psc_reset_iowait_jump( (uint *)
					     adp_str.SCRIPTS[0].script_ptr,
					     dev_index);
				    /* log the appropriate error */
				    if (dev_ptr->flags & SCSI_ABORT)
					psc_logerr(ERRID_SCSI_ERR10,
						    ABORT_FAILED, 95, 0,
						    dev_ptr, TRUE);
				    else
					psc_logerr(ERRID_SCSI_ERR10, BDR_FAILED,
						   95, 0, dev_ptr, TRUE);

				    /* if abort failed issue a BDR */
				    bp = dev_ptr->active;
				    if ((dev_ptr->flags & SCSI_ABORT) &&
					(bp != NULL))
				    {
					dev_ptr->flags |= SCSI_BDR;
					/* issue a bdr script call */
					if (psc_issue_bdr_script(
					     (uint *) adp_str.SCRIPTS
					      [dev_ptr->cmd_script_ptr].
					      script_ptr,
					     (uint *) dev_ptr->script_dma_addr,
					     dev_index) == REG_FAIL)
					{
					    psc_cleanup_reset(REG_ERR);
					    break;
					}
					dev_ptr->cmd_activity_state =
							    BDR_IN_PROGRESS;
					/* the timer is restarted from it's */
					/* last point to eventually time    */
					/* out the cmd if all else fails.  */
					dev_ptr->dev_watchdog.dog.restart =
					    LONGWAIT;
					dev_ptr->flags &= ~RETRY_ERROR;
					dev_ptr->retry_count = 1;
					w_start(&dev_ptr->dev_watchdog.dog);
					if (!(bp->bufstruct.b_flags & B_ERROR))
					{
					    bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
					    bp->status_validity =
						     SC_ADAPTER_ERROR;
					    bp->general_card_status =
						     SC_SCSI_BUS_FAULT;
					    bp->scsi_status = SC_GOOD_STATUS;
					    bp->bufstruct.b_flags |= B_ERROR;
					    bp->bufstruct.b_error = EIO;
					}
					break;
				    }
				    if (bp != NULL)
				    {
					bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
					bp->status_validity =
						     SC_ADAPTER_ERROR;
					bp->general_card_status =
						     SC_SCSI_BUS_FAULT;
					bp->scsi_status = SC_GOOD_STATUS;
					bp->bufstruct.b_flags |= B_ERROR;
					bp->bufstruct.b_error = EIO;
					if ((rc = psc_free_resources(dev_ptr,
					      TRUE)) != PSC_NO_ERR)
					{
					    if (psc_fail_free_resources(
						 dev_ptr, bp, rc))
						break;
					}
					else
					    iodone((struct buf *) bp);
				    }
				    dev_ptr->flags = RETRY_ERROR;
				    dev_ptr->active = NULL;
				    psc_deq_active(dev_ptr);
				    if (dev_ptr->head_pend != NULL)
					psc_fail_cmd(dev_ptr, 1);
				    if (dev_ptr->queue_state == STOPPING)
					e_wakeup(&dev_ptr->stop_event);
				    else
					dev_ptr->queue_state = HALTED;
				    if (dev_ptr->ioctl_wakeup == TRUE)
				    {
					dev_ptr->ioctl_errno = EIO;
					dev_ptr->ioctl_wakeup = FALSE;
					e_wakeup(&dev_ptr->ioctl_event);
				    }
				    if (issue_abrt_bdr)
					psc_deq_abort_bdr(dev_ptr);
				    start_new_job = TRUE;
				    break;

				case A_unexpected_status:
				    TRACE_1 ("unexp st", (int) dev_ptr)
				    /* we received status back almost im- */
				    /* mediately. It is probably a hw or  */
				    /* protocol error.  Fail the job and  */
				    /* start a new one.                   */
				    DEBUG_0 ("psc_intr: A_unexpect_status \n")
				    w_stop(&dev_ptr->dev_watchdog.dog);
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    psc_logerr(ERRID_SCSI_ERR10,
						UNEXPECTED_STATUS, 100, 0,
						dev_ptr, TRUE);
				    if ((bp = dev_ptr->active) != NULL)
				    {
					bp->scsi_status = GET_CMD_STATUS_BYTE(
							   adp_str.SCRIPTS
							    [dev_ptr->
							    cmd_script_ptr].
							    script_ptr);
					bp->bufstruct.b_resid =
						     bp->bufstruct.b_bcount;
					bp->status_validity =
						     SC_ADAPTER_ERROR;
					bp->general_card_status =
						     SC_SCSI_BUS_FAULT;
					bp->bufstruct.b_flags |= B_ERROR;
					bp->bufstruct.b_error = EIO;
					if ((rc = psc_free_resources(dev_ptr,
						   TRUE)) != PSC_NO_ERR)
					{
					    if (psc_fail_free_resources(
						 dev_ptr, bp, rc))
						break;
					}
					else
					    iodone((struct buf *) bp);
				    }
				    dev_ptr->flags = RETRY_ERROR;
				    dev_ptr->active = NULL;
				    psc_deq_active(dev_ptr);
				    if (dev_ptr->head_pend != NULL)
					psc_fail_cmd(dev_ptr, 1);
				    if (dev_ptr->queue_state == STOPPING)
					e_wakeup(&dev_ptr->stop_event);
				    else
					dev_ptr->queue_state = HALTED;
				    if (dev_ptr->ioctl_wakeup == TRUE)
				    {
					dev_ptr->ioctl_errno = EIO;
					dev_ptr->ioctl_wakeup = FALSE;
					e_wakeup(&dev_ptr->ioctl_event);
				    }
				    if (issue_abrt_bdr)
					psc_deq_abort_bdr(dev_ptr);
				    start_new_job = TRUE;
				    break;

				case A_unknown_reselect_id:
				    TRACE_1 ("unkn id ", (int) dev_ptr)
				    /* a device reselected us, but we can't*/
				    /* figure out who it really is.        */
				            psc_reselect_router();
				    break;

				case A_uninitialized_reselect:
				    /* an unconfigured device is trying to */
				    /* select us (spurious interrupt) or a */
				    /* device which was aborted or BDR'ed  */
				    /* is trying to reselect us and is to  */
				    /* trying finish off a cmd.            */
				    DEBUG_0 ("psc_intr: uninit_reselect.\n")
				    TRACE_1 ("bad resl", (int) dev_ptr)
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    psc_logerr(ERRID_SCSI_ERR10,
					       UNINITIALIZED_RESELECT, 105,
					       0, NULL, TRUE);
				    psc_command_reset_scsi_bus();
				    break;

				default:
				    /* unknown interrupt flag.  This should*/
				    /* not happen. If it does, we have a   */
				    /* fatal error which we must back out  */
				    /* of.                                 */
				    DEBUG_0 ("psc_intr: SCRIPT err dflt \n")
				    TRACE_1 ("default ", (int) dev_ptr)
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					psc_cleanup_reset(REG_ERR);
					break;
				    }
				    psc_logerr(ERRID_SCSI_ERR2,
					       ADAPTER_INT_ERROR, 110, 0,
					       NULL, TRUE);
				    start_new_job = TRUE;
				    break;

				}	/* end script switch */
			    }	/* end script interrupt (SIR) */
			    /******************************************/
			    /*      ABORT CAUSED THIS INTERRUPT       */
			    /******************************************/
			    else
				if (save_dsr & DABRT)
				{
				    DEBUG_0 ("psc_intr: Abort caused int\n")
				    TRACE_1 ("abrt int", 0)
				    /* interrupt cleared up above */
				    rc = psc_read_reg(ISTAT, ISTAT_SIZE);
				    /* A read of the istat register verifies*/
				    /* that the scsi chip is currently not  */
				    /* busy on the scsi bus, so we can issue*/
				    /* the new command. */
				    if (!(rc & CONNECTED))
				    {
					/* Read the DSP register to make  */
					/* sure that the abort took place */
					/* while executing the iowait     */
					/* script. If this is the case, or*/
					/* a scsi abort/BDR command is    */
					/* waiting to be issued, go ahead */
					/* and issue the command.         */
					/* Else, restart the script.      */
					dma_stat = word_reverse(
					     psc_read_reg(DSP, DSP_SIZE));
#ifdef PSC_TRACE
					rc = psc_read_reg(DCMD, DCMD_SIZE);
					TRACE_2("DSP DCM1", dma_stat, rc)
#endif
					if ((dma_stat >=
					     adp_str.SCRIPTS[0].dma_ptr) &&
					    (dma_stat < ((ulong)
					     adp_str.SCRIPTS[0].dma_ptr +
					     Ent_scripts_entry_point)))
                                        {
					    if (SET_SXFER(SYNC_VAL) 
						== REG_FAIL)
					    {
						psc_cleanup_reset(REG_ERR);
					    }
					    else
					    {
						dev_ptr = NULL;
						start_new_job = TRUE;
					    }
					}
					else /* restart script */
					{
					    psc_patch_iowait_int_off();
                                            dma_stat -= 8;
					    rc = psc_write_reg(DSP, DSP_SIZE,
						   word_reverse(dma_stat));
					    if (rc == REG_FAIL)
					    {
						psc_cleanup_reset(REG_ERR);
					    }
					}
				    }
				    else
				    {
					(void) psc_read_reg(DSTAT, DSTAT_SIZE);
					/* restart iowait script */
					psc_patch_iowait_int_off();
					dma_stat = word_reverse(
					     psc_read_reg(DSP, DSP_SIZE));
					dma_stat -= 8;
#ifdef PSC_TRACE
					rc = psc_read_reg(DCMD, DCMD_SIZE);
					TRACE_2("DSP DCMD", dma_stat, rc)
#endif
					rc = psc_write_reg(DSP, DSP_SIZE,
						 word_reverse(dma_stat));
					if (rc == REG_FAIL)
					{
					    psc_cleanup_reset(REG_ERR);
					}
				    }
				}
			    /**************************************/
			    /*      WATCH DOG TIMER INTERRUPT     */
			    /**************************************/
				else
				    if (save_dsr & WTD)
				    {
					DEBUG_0 ("psc_intr: Timeout int\n")
					TRACE_1 ("watch tm", 0)
					if (SET_SXFER(SYNC_VAL) 
						== REG_FAIL)
					    psc_cleanup_reset(REG_ERR);
					psc_logerr(ERRID_SCSI_ERR2,
						   ADAPTER_INT_ERROR, 115, 0,
						   NULL, TRUE);
					dev_ptr = NULL;
					start_new_job = TRUE;
				    }
			    /**************************************/
			    /*    ILLEGAL INSTRUCTION DETECTED    */
			    /**************************************/
				    else
					if (save_dsr & OPC)
					{
					    DEBUG_0 ("psc_intr: Ig Inst int\n")
					    TRACE_1 ("igl inst", 0)
					/* A check is made here to read   */
					/* the command that is currently  */
					/* executing on the scsi chip. If */
					/* the command is a "wait for dis-*/
					/* conect command" (0x48), then a */
					/* reselect occured during this   */
					/* command.  Due to a chip fault, */
					/* this is reflected as an illegal*/
					/* instruction interrupt.  In this*/
					/* case, the DSP holds the address*/
					/* of the interrupt instruction   */
					/* that should have occurred. So  */
					/* the script is restarted to     */
					/* cause this interrupt to occur. */
					/* Otherwise, this interrupt is an*/
					/* error and is logged as such.   */
					    rc = psc_read_reg(DCMD,DCMD_SIZE);
					    if (rc == REG_FAIL)
						psc_cleanup_reset(REG_ERR);
					    else
					    {
						if (rc == 0x48)
						{
						    rc = psc_read_reg(DSP,
							       DSP_SIZE);
						    if (rc == REG_FAIL)
						    {
							psc_cleanup_reset
							  (REG_ERR);
						    }
						    else
						    {
							rc = psc_write_reg
							 (DSP, DSP_SIZE, rc);
							if (rc == REG_FAIL)
							{
							    psc_cleanup_reset
							      (REG_ERR);
							}
						    }
						}
						else
						{
						if (SET_SXFER(SYNC_VAL) 
							== REG_FAIL)
						    psc_cleanup_reset(REG_ERR);
						else
						{
						    dev_ptr = NULL;
						    start_new_job = TRUE;
						}
						psc_logerr(ERRID_SCSI_ERR2,
							 ADAPTER_INT_ERROR,
							 120, 0, NULL, TRUE);
					        }
					    }
					}
			    /**************************************/
			    /*       SINGLE STEP INTERRUPT        */
			    /**************************************/
					else
					    if (save_dsr & SSI)
					    {
						DEBUG_0 ("psc_intr S Sp Int\n")
						TRACE_1 ("sngl stp", 0)
						if (SET_SXFER(SYNC_VAL) 
							== REG_FAIL)
						    psc_cleanup_reset(REG_ERR);
						psc_logerr(ERRID_SCSI_ERR2,
							   ADAPTER_INT_ERROR,
							   125, 0, NULL, TRUE);
						dev_ptr = NULL;
						start_new_job = TRUE;
					    }
			}	/* if ISR & DIP */
			/*******************************************/
			/*        THIS IS A SCSI INTERRUPT         */
			/*******************************************/
			/* in case of DIP and SIP will fall here */
			else
			    if (save_isr & SIP)
			    {
				/* read scsi status register 0 for cause int */
				rc = psc_read_reg(SSTAT0, SSTAT0_SIZE);
				save_interrupt = (rc & SIEN_MASK);
				DEBUG_1 ("CHIP INTR is 0x%x\n",
					  save_interrupt);
				if (rc == REG_FAIL)
				{
				    /* clean up errors and clear interrupts */
				    psc_cleanup_reset(REG_ERR);
				}
				else
				    if (SET_SXFER(SYNC_VAL) == REG_FAIL)
				    {
					/* clean up errors and clear ints */
					psc_cleanup_reset(REG_ERR);
				    }
				    else
				    {
					/* if a chip abort was set but    */
					/* another interrupt won the race,*/
					/* clear the stacked interrupt for*/
					/* which the abort has taken place*/
					if (chip_abort_occurred)
					    (void) psc_read_reg(DSTAT,
					      DSTAT_SIZE);
					dev_ptr = NULL;
					check_for_disconnect = TRUE;
				scsi_int:
					switch (save_interrupt)
					{

					case PHASE_MIS:
					    DEBUG_0 ("psc_intr: PHASE_MIS\n")
					    if ((dev_ptr = psc_find_devinfo(0))
						 != NULL)
					    {
						TRACE_1 ("phase ms", 
							      (int) dev_ptr)
						rc = psc_update_dataptr(
							 adp_str.SCRIPTS
							  [dev_ptr->
							  cmd_script_ptr].
							 script_ptr,
							 dev_ptr);
						if (rc == REG_FAIL)
						{
						    psc_cleanup_reset(REG_ERR);
						    break;
						}
						/* clear DMA and SCSI FIFOs */
						if (psc_write_reg(DFIFO,
						     DFIFO_SIZE, 0x40) ==
						     REG_FAIL)
						{
						    psc_cleanup_reset(REG_ERR);
						    break;
						}
						ISSUE_MAIN_AFTER_NEG(
						   dev_ptr->script_dma_addr);
					    }
					    else
					    {
						TRACE_1 ("phase ms", 0)
						DEBUG_0 ("psc_intr: PH_MIS /n")
						psc_cleanup_reset(PHASE_ERR);
						psc_logerr(ERRID_SCSI_ERR10,
							   PHASE_ERROR, 130, 0,
							   NULL, TRUE);
					    }
					    break;

					case SCSI_SEL_FAIL:
					    DEBUG_1 ("psc_intr: sel f 0x%x\n",
						       save_interrupt);
					    if ((dev_ptr = psc_find_devinfo(0))
						 != NULL)
					    {
						TRACE_1 ("sel fail", 
							      (int) dev_ptr)
						if (check_for_disconnect)
						    start_new_job =
							psc_scsi_parity_error(
							    dev_ptr,
							    save_interrupt,
							    (!dev_ptr->
							    negotiate_flag)
							    | 0x02);
						else
						    start_new_job =
							psc_scsi_parity_error(
							    dev_ptr,
							    save_interrupt,
							    FALSE);
					    }
					    else
					    {
						DEBUG_0 ("psc_intr:UNEXP_DC\n")
						TRACE_1 ("sel fail", 0)
						/* clean active queue */
						psc_cleanup_reset(DEVICE_ERR);
						psc_logerr(ERRID_SCSI_ERR10,
						  UNEXPECTED_SELECT_TIMEOUT,
							 135, 0, NULL, TRUE);
					    }
					    break;

					case SCSI_GROSS_ERR:
					    DEBUG_1 ("psc_intr:gross 0x%x\n",
						             save_interrupt);
					case SCSI_PARITY_ERR:
					    DEBUG_1 ("psc_intr:parity 0x%x\n",
						             save_interrupt);
					    if ((dev_ptr = psc_find_devinfo(0))
						 != NULL)
					    {
						TRACE_1 ("par err ", 
							      (int) dev_ptr)
						start_new_job =
						    psc_scsi_parity_error(
							dev_ptr,
							save_interrupt, TRUE);
					    }
					    else
					    {
						DEBUG_0 ("psc_intr: no dev\n")
						TRACE_1 ("par err ", 0)
						/* clean active queue */
						if (save_interrupt ==
						     SCSI_GROSS_ERR)
						{
						    psc_cleanup_reset(
						      DEVICE_ERR);
						    psc_logerr(ERRID_SCSI_ERR10,
							       GROSS_ERROR,
							       140, 0, NULL,
							       TRUE);
						}
						else	/* parity error */
						{
						    psc_cleanup_reset(
						      HOST_ERR);
						    psc_logerr(ERRID_SCSI_ERR10,
							BAD_PARITY_DETECTED,
							140, 0, NULL, TRUE);
						}
					    }
					    break;

					case SCSI_UNEXP_DISC:
					    DEBUG_1 ("psc_intr: unexp 0x%x\n",
						             save_interrupt);
					    if ((dev_ptr = psc_find_devinfo(0))
						  != NULL)
					    {
						TRACE_1 ("uex dis ", 
							      (int) dev_ptr)
						start_new_job =
						   psc_scsi_parity_error(
						     dev_ptr, save_interrupt,
						     FALSE);
					    }
					    else
					    {
						DEBUG_0 ("psc_intr:  no dev\n")
						TRACE_1 ("uex dis ", 
							      (int) dev_ptr)
						/* clean active queue */
						psc_cleanup_reset(DISC_ERR);
						psc_logerr(ERRID_SCSI_ERR10,
							UNEXPECTED_BUS_FREE,
							140, 0, NULL, TRUE);
					    }
					    break;

					case SCSI_RST:
					    DEBUG_0 ("psc_intr: int reset.\n")
					    TRACE_1 ("RESET!!!", 0)
					    w_stop(
						&(adp_str.reset_watchdog.dog));
					    /* clean active queue */
					    psc_cleanup_reset_error();
					    psc_logerr(ERRID_SCSI_ERR10,
						       SCSI_BUS_RESET,
						       145, 0, NULL, TRUE);
					    break;

					case SCSI_COMP:
					    /* if we get here something is
					       wrong */
					    /* int. should be disabled */
					    DEBUG_0 ("psc_intr: SCSI_COMP.\n")
					    TRACE_1 ("scsi cmp", 0)
						    psc_logerr(ERRID_SCSI_ERR2,
							ADAPTER_INT_ERROR,
						        150, 0, NULL, TRUE);
					    break;

					case SCSI_SEL:
					    /* if we get here something is */
					    /* wrong int should be disabled*/
					    DEBUG_0 ("psc_intr: SCSI_SEL\n")
					    TRACE_1 ("scsi sel", 0)
					    psc_logerr(ERRID_SCSI_ERR2,
						       ADAPTER_INT_ERROR,
						       155, 0, NULL, TRUE);
					    break;

					default:
					    DEBUG_1 ("psc_intr:int dflt0x%x\n",
						             save_interrupt)
					    TRACE_1 ("scsi dft", 0)
					    /* if here we probably have 2  */
					    /* interrupt bits set so mask  */
					    /* them and carry on           */
					    if (save_interrupt & SCSI_RST)
					    {
						/* on reset throw away the */
						/* others and execute */
						save_interrupt = SCSI_RST;
						goto scsi_int;
					    }
					    if (save_interrupt &
						 SCSI_UNEXP_DISC)
					    {
						check_for_disconnect = FALSE;
						save_interrupt =
						    (save_interrupt &
						     ~SCSI_UNEXP_DISC);
						goto scsi_int;
					    }
					    if (save_interrupt &
						 SCSI_PARITY_ERR)
					    {   /* insure that a        */
						/* SCSI_PARITY_ERR gets */
						/* processed            */
						save_interrupt =
						   SCSI_PARITY_ERR;
						goto scsi_int;
					    }
					    if (save_interrupt &
						 SCSI_GROSS_ERR)
					    {   /* insure that a        */
						/* SCSI_GROSS_ERR gets  */
						/* processed the same as*/
						/* a SCSI_PARITY_ERROR  */
						save_interrupt =
						   SCSI_PARITY_ERR;
						goto scsi_int;
					    }
					    psc_logerr(ERRID_SCSI_ERR2,
						       UNKNOWN_ADAPTER_ERROR,
						       160, 0, NULL, TRUE);
					    start_new_job = TRUE;
					    break;
					}	/* end of scsi interrupt
						   switch */
				    }
			    }	/* end else if end of SCSI interrupt (SIP) */
			    /* if DIP or SIP bits of ISTAT are not set  */
			    /* this implies that this error caused by   */
			    /* spurious interrupt log soft error        */
			    else
			    {
				DEBUG_0 ("psc_intr: Spurious error \n")
				TRACE_1 ("spurious", 0)
				psc_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
					   170, 0, NULL, TRUE);
				if (SET_SXFER(SYNC_VAL))
				    psc_cleanup_reset(REG_ERR);
			    }
			/*********************************************/
			if (start_new_job)
			{
			    command_issued_flag = psc_issue_cmd();
			    psc_check_wait_queue(dev_ptr,command_issued_flag);
			}
		    }
		    else
		    {	/* read of the ISTAT failed */
			TRACE_1 ("bad Rist", 0)
			TRACE_1 ("out intr", 0)
			psc_cleanup_reset(REG_ERR);
		    }
		}
		else
		{	/* DMA error */
		    DEBUG_0 ("Leaving psc_intr: DMA FAILED \n")
		    TRACE_1 ("dma err ", 0)
		    TRACE_1 ("out intr", 0)
		    psc_logerr(ERRID_SCSI_ERR2, DMA_ERROR, 175,
			       (uint) rc, NULL, TRUE);
		    (void) psc_read_reg(DSTAT, DSTAT_SIZE);
		    (void) psc_read_reg(SSTAT0, SSTAT0_SIZE);
		    psc_cleanup_reset(DMA_ERR);
		}
	    }
	    else
	    {	/* an interrupt was received with no command active */
		DEBUG_1 ("Leaving psc_intr: rc=%d\n", rc);
		TRACE_1 ("no cmd  ", 0)
		TRACE_1 ("out intr", 0)
		/* If an epow has occurred, a reset may have been sent */
		/* by the epow handler.  If there are commands this    */
		/* will be handled by the scsi reset cleanup.  If not, */
		/* it will hit this code and have the watchdog reset.  */
		save_isr = psc_read_reg(ISTAT, ISTAT_SIZE);
		(void) psc_write_reg(ISTAT, ISTAT_SIZE, 0x00);
		if (save_isr & DIP)
		{
		    (void) psc_read_reg(DSTAT, DSTAT_SIZE);
		}
		if (save_isr & SIP)
		{
		    save_stat = psc_read_reg(SSTAT0, SSTAT0_SIZE);
		    if (save_stat & SCSI_RST)
		    {
			w_stop(&(adp_str.reset_watchdog.dog));
			/* clean active queue */
			psc_cleanup_reset_error();
			psc_logerr(ERRID_SCSI_ERR10, SCSI_BUS_RESET,
				   177, 0, NULL, TRUE);
		    }
		}
	    }
	}
	else
	{	/* this is not our interrupt */
	    DEBUG_1 ("Leaving psc_intr: not our int. SCSIRS 0x%x\n", rc);
	    TRACE_1 ("not ours", 0)
	    TRACE_1 ("out intr", 0)
	    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, adp_str.devno);
	    return (INTR_FAIL);
	}
    }
    else	/* register fail or channel check in SCSIRS */
    {
	DEBUG_1 ("Leaving psc_intr: SCSIRS error 0x%x\n", rc);
	TRACE_1 ("bad SCIR", 0)
	TRACE_1 ("out intr", 0)
	if (rc == REG_FAIL)
	    psc_logerr(ERRID_SCSI_ERR2, PIO_RD_DATA_ERR, 180,
		       SCSIRS, NULL, TRUE);
	else
	    psc_logerr(ERRID_SCSI_ERR2, CHANNEL_CHK_ERROR, 180,
		       SCSIRS, NULL, TRUE);
	(void) psc_write_reg(SCSIRS, SCSIRS_SIZE, (rc | 0x1000000));
	/* make sure the adapter timers are stopped */
	w_stop(&(adp_str.adap_watchdog.dog));
	psc_cleanup_reset(REG_ERR);
    }
    DEBUG_0 ("Leaving psc_intr\n")
    TRACE_1 ("out intr", 0);
    i_reset(&(adp_str.intr_struct));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, adp_str.devno);
    return (INTR_SUCC);
}  /* end of psc_intr      */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_issue_cmd                                           */
/*                                                                      */
/* FUNCTION:    issues a waiting command, or ABORT/BDR command          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Called by the interrupt handler                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      none                                                            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      int return value                                                */
/*      TRUE - command issued all is well                               */
/*      FALSE - command not issued all is not well                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      iodone                                                          */
/************************************************************************/
int
psc_issue_cmd()
{
    struct dev_info *dev_ptr;
    struct sc_buf *bp;
    int     dev_index;	/* index into device hash table */
    uchar   chk_disconnect;

    DEBUG_0 ("Entering psc_issue_cmd routine\n")
    TRACE_1 ("in issue", 0)

    /* if abort/bdr queue not empty then go to work  */
    dev_ptr = adp_str.ABORT_BDR_head;
    /* loop while Q not empty */
    while ((dev_ptr != NULL) && (adp_str.epow_state != EPOW_PENDING))
    {
	if (dev_ptr->flags & SCSI_ABORT)
	{
	    if (dev_ptr->active != NULL)
	    {
		/* the timer is started only for aborts  */
		/* that have not previously been issued. */
		/* Otherwise, the current timer is left  */
		/* running.                              */
		if (dev_ptr->flags & RETRY_ERROR)
		{
		    w_stop(&dev_ptr->dev_watchdog.dog);
		    dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
		    dev_ptr->flags &= ~RETRY_ERROR;
		    dev_ptr->retry_count = 1;
		    w_start(&dev_ptr->dev_watchdog.dog);
		}
		else
		{
		    dev_ptr->retry_count++;
		}
		dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;
		dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
		/* update wait script */
		psc_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr,
				      dev_index);
		if (psc_issue_abort_script((uint *) adp_str.SCRIPTS
				       [dev_ptr->cmd_script_ptr].script_ptr,
					(uint *) dev_ptr->script_dma_addr, 
					dev_ptr, dev_index, FALSE) == REG_FAIL)
		{
		    psc_cleanup_reset(REG_ERR);
		    return (TRUE);
		}
		psc_deq_abort_bdr(dev_ptr);
		w_start(&dev_ptr->dev_watchdog.dog);
		DEBUG_0 ("psc_issue_cmd: leaving TRUE\n")
		return (TRUE);  /* cmds issued */
	    }
	    else  /* else don't issue abort one is already in progress */
	    {
		psc_deq_abort_bdr(dev_ptr);
		if (dev_ptr->ioctl_wakeup == TRUE)
		{
		    dev_ptr->cmd_activity_state = 0;
		    dev_ptr->ioctl_errno = PSC_NO_ERR;
		    dev_ptr->ioctl_wakeup = FALSE;
		    e_wakeup(&dev_ptr->ioctl_event);
		}
	    }
	}
	else  /* a BDR */
	{
	    dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
	    /* update wait script */
	    psc_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr, 
				  dev_index);
	    if (psc_issue_bdr_script((uint *) adp_str.SCRIPTS
				     [dev_ptr->cmd_script_ptr].script_ptr, 
				     (uint *) dev_ptr->script_dma_addr,
				     dev_index) == REG_FAIL)
	    {
		psc_cleanup_reset(REG_ERR);
		TRACE_1 ("out issu", 0)
		return (TRUE);
	    }
	    TRACE_1 ("issue bd", (int) dev_ptr)
	    psc_deq_abort_bdr(dev_ptr);
	    /* the timer is started only for aborts  */
	    /* that have not previously been issued. */
	    /* Otherwise, the current timer is left  */
	    /* running.                              */
	    if (dev_ptr->flags & RETRY_ERROR)
	    {
		w_stop(&dev_ptr->dev_watchdog.dog);
		dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
		dev_ptr->flags &= ~RETRY_ERROR;
		dev_ptr->retry_count = 1;
		w_start(&dev_ptr->dev_watchdog.dog);
	    }
	    else
	    {
		dev_ptr->retry_count++;
	    }
	    if (dev_ptr->active == NULL)
	    {
		psc_enq_active(dev_ptr);
	    }
	    dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;
	    /* the timer is restarted from it's last */
	    /* point to eventually time out the cmd  */
	    /* if all else fails.                    */
	    DEBUG_0 ("psc_issue_cmd: leaving TRUE\n")
	    TRACE_1 ("out issu", (int) dev_ptr)
	    return (TRUE);      /* cmds issued */
	}   /* end else a BDR */
	dev_ptr = adp_str.ABORT_BDR_head;
    }	/* end while loop */

    /* is there something on the waiting queue */
    dev_ptr = adp_str.DEVICE_WAITING_head;
 
    if ((dev_ptr != NULL) && (adp_str.epow_state != EPOW_PENDING))
    {
	bp = dev_ptr->active;

        /* trace hook to indicate new command for adapter */
        DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, adp_str.devno,
            bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
            bp->bufstruct.b_bcount);

	/* if we haven't negotiated */
	if (!(dev_ptr->negotiate_flag))
	{
	    if (dev_ptr->async_device)
	    {
		if (SET_SXFER(0x00) == REG_FAIL)
		{
		    psc_cleanup_reset(REG_ERR);
		    TRACE_1 ("out issu", 0)
                    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0,
                            adp_str.devno);
		    return (TRUE);
		}
	    }
	    else
		if ((dev_ptr->agreed_xfer != DEFAULT_MIN_PHASE) ||
		    (dev_ptr->agreed_req_ack < DEFAULT_BYTE_BUF))
		{
		    if (SET_SXFER(dev_ptr->special_xfer_val) == REG_FAIL)
		    {
			psc_cleanup_reset(REG_ERR);
			TRACE_1 ("out issu", 0)
                        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0,
                                adp_str.devno);
			return (TRUE);
		    }
		}
	    chk_disconnect = bp->scsi_command.flags & SC_NODISC;
	    if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
		psc_set_disconnect(dev_ptr, chk_disconnect);
	    if (!(dev_ptr->flags & PREP_MAIN_COMPLETE))
	    {
		psc_prep_main_script((uint *) adp_str.SCRIPTS[0].script_ptr,
			    (uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].
			    script_ptr, dev_ptr, dev_ptr->script_dma_addr);
	    }
	    psc_patch_iowait_int_off();
	    ISSUE_MAIN_TO_DEVICE(dev_ptr->script_dma_addr);
	    dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	}
	else
	{
	    if (!(dev_ptr->active->scsi_command.flags & SC_ASYNC))
	    {
		/* setup chip to exec the negotiate script */
		psc_patch_iowait_int_off();
		chk_disconnect = bp->scsi_command.flags & SC_NODISC;
		if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
		    psc_set_disconnect(dev_ptr, chk_disconnect);
		ISSUE_NEGOTIATE_TO_DEVICE(dev_ptr->script_dma_addr);
		dev_ptr->cmd_activity_state = NEGOTIATE_IN_PROGRESS;
	    }
	    else
	    {
		/* set async mode */
		if (SET_SXFER(0x00) == REG_FAIL)
		{
		    psc_cleanup_reset(REG_ERR);
		    TRACE_1 ("out issu", 0)
                    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, adp_str.devno);
		    return (TRUE);
		}
		chk_disconnect = bp->scsi_command.flags & SC_NODISC;
		if ((dev_ptr->disconnect_flag ^ chk_disconnect) != 0)
		    psc_set_disconnect(dev_ptr, chk_disconnect);
		dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
		psc_patch_async_switch_int((uint *) adp_str.SCRIPTS
			   [dev_ptr->cmd_script_ptr].script_ptr, dev_index);
		if (!(dev_ptr->flags & PREP_MAIN_COMPLETE))
		{
		    psc_prep_main_script(
			    (uint *) adp_str.SCRIPTS[0].script_ptr,
			    (uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].
			    script_ptr, dev_ptr, dev_ptr->script_dma_addr);
		}
		psc_patch_iowait_int_off();
		ISSUE_MAIN_TO_DEVICE(dev_ptr->script_dma_addr);
		dev_ptr->cmd_activity_state = CMD_IN_PROGRESS;
	    }
	}
	TRACE_1 ("issue cd", (int) dev_ptr)
	psc_deq_wait(dev_ptr);
	psc_enq_active(dev_ptr);
	if (bp->timeout_value == 0)
            dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
	else
            dev_ptr->dev_watchdog.dog.restart = (ulong) bp->timeout_value + 1;

	w_start(&dev_ptr->dev_watchdog.dog);
	DEBUG_0 ("Leaving psc_issue_cmd with TRUE\n")
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, adp_str.devno);
	TRACE_1 ("out issu", (int) dev_ptr)
	return (TRUE);  /* command issued */
    }	/* if */
    /* no command to issue.  restart io wait and return             */
    DEBUG_0 ("Leaving psc_issue_cmd with FALSE\n")
    return (FALSE);     /* command not issued */
}  /* psc_issue_cmd */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_check_wait_queue                                    */
/*                                                                      */
/* FUNCTION:    issue a waiting command or Abort/Bdr command            */
/*                                                                      */
/* NOTES:                                                               */
/*  this is called to see if a cmd which is waiting for resources       */
/*  may be satisified                                                   */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      *dev_ptr           - pointer to device structure                */
/*      issue_command_flag - FALSE= not issued                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      iodone                                                          */
/************************************************************************/
void
psc_check_wait_queue(struct dev_info * dev_ptr, uchar issue_command_flag)
{
    struct dev_info *next_ptr;
    struct sc_buf *bp;
    uchar   command_issued_flag;

    DEBUG_2 ("Entering psc_check_wait_queue: dev_ptr 0x%x flag=0x%x\n",
	      dev_ptr, issue_command_flag)
    TRACE_2 ("in chkwq", (int) dev_ptr, issue_command_flag)
    command_issued_flag = issue_command_flag;
    next_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;

    while (next_ptr != NULL)	/* loop while Q not empty */
    {
	/* acquire resources waiting command */
	if (psc_alloc_resources(next_ptr) == PSC_NO_ERR)
	{
	    psc_deq_wait_resources(next_ptr);
	    psc_enq_wait(next_ptr);
	    if (command_issued_flag == FALSE)
		command_issued_flag = psc_issue_cmd();
	}
	else
	    /* couldn't alloc necessary resources so quit */
            break;
	next_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;
    }	/* end while */
    if ((dev_ptr != NULL) && (dev_ptr->head_pend != NULL) &&
       (dev_ptr->active == NULL))
    {
	bp = dev_ptr->head_pend;
	/* if xfer count <= 4k or no devices waiting            */
	/* for resources then attempt to alloc resources        */
	/* for command sitting on  pending queue of this device */
	if ((bp->bufstruct.b_bcount <= 4096) ||
	    (adp_str.DEVICE_WAITING_FOR_RESOURCES_head == NULL))
	{
	    /* attempt to allocate resources  for dev. on pending queue */
	    if (psc_alloc_resources(dev_ptr) == PSC_NO_ERR)
	    {
		psc_enq_wait(dev_ptr);
		if (command_issued_flag == FALSE)
		    command_issued_flag = psc_issue_cmd();
	    }
	    else
                psc_enq_wait_resources(dev_ptr);
	}
	else
	    psc_enq_wait_resources(dev_ptr);
    }
    if ((command_issued_flag == FALSE) &&
	(adp_str.DEVICE_ACTIVE_head != NULL))
    {
	psc_patch_iowait_int_off();
	psc_write_reg(DSP, DSP_SIZE, word_reverse(
					       adp_str.SCRIPTS[0].dma_ptr));
	DEBUG_0 ("psc_check_wait_queue: Issuing IOWAIT\n")
    }
    DEBUG_0 ("leaving psc_check_wait_queue\n")
    TRACE_1 ("out chkwq", 0)
}  /* end of psc_check_wait_queue */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_cleanup_reset                                       */
/*                                                                      */
/* FUNCTION:    Cleans up the active queues due to a register error or  */
/*              a dma error.                                            */
/*                                                                      */
/* NOTES:                                                               */
/*  This is called when an error occurrs that requires a bus reset to   */
/*  be issued.  All commands will be cleaned up upon completion of the  */
/*  reset by psc_cleanup_reset_error.                                   */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      unint  err_type                                                 */
/*            1=dma 2=register                                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
void
psc_cleanup_reset(int err_type)
{
    struct dev_info *dev_ptr;	/* device structure */
    struct sc_buf *bp;

    DEBUG_1 ("Entering psc_cleanup_reset routine error = 0x%x\n", err_type);
    TRACE_1 ("in clres", err_type)

    /* loop while devices are on Active queue */
    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
	/* this error may have been caused by a timeout or epow */
	/* or a glitch on the reset line                        */
	if ((bp = dev_ptr->active) != NULL)
	{
	    /* if active cmd status not already set */
	    if (!(bp->bufstruct.b_flags & B_ERROR))
	    {
		/* set scsi status in sc_buf to say adapter error */
		bp->status_validity = SC_ADAPTER_ERROR;
		switch (err_type)
		{
		case PHASE_ERR:
		case DISC_ERR:
		    bp->general_card_status = SC_SCSI_BUS_FAULT;
		    break;
		case DEVICE_ERR:
		    bp->general_card_status = SC_NO_DEVICE_RESPONSE;
		    break;
		case HOST_ERR:
		case REG_ERR:
		case DMA_ERR:
		default:
		    bp->general_card_status = SC_HOST_IO_BUS_ERR;
		    break;
		}
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
	    }
	}
	if ((adp_str.ABORT_BDR_head == dev_ptr) ||
	    (dev_ptr->ABORT_BDR_bkwd != NULL))
	    psc_deq_abort_bdr(dev_ptr);
	if (dev_ptr->ioctl_wakeup == TRUE)
	{
	    dev_ptr->ioctl_errno = EIO;
	    dev_ptr->ioctl_wakeup = FALSE;
	    e_wakeup(&dev_ptr->ioctl_event);
	}
	dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
    }	/* end of while loop */
    if (err_type == REG_ERR)
    {
	psc_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA);
	psc_command_reset_scsi_bus();
    }
    else
	if (err_type == DMA_ERR)
	{
	    psc_command_reset_scsi_bus();
	    psc_chip_register_reset(PSC_RESET_DMA);
	}
	else
	{
	    psc_command_reset_scsi_bus();
	}
    TRACE_1 ("out clre", err_type)
    DEBUG_0 ("leaving psc_cleanup_reset\n")
}  /* end psc_cleanup_reset */

/************************************************************************/
/*                                                                      */
/* NAME:        psc_cleanup_reset_error                                 */
/*                                                                      */
/* FUNCTION:    Cleans up the active queues after a scsi bus reset has  */
/*              occurred.                                               */
/*                                                                      */
/* NOTES:                                                               */
/*  After a scsi bus reset occurs, all active commands are cleaned up.  */
/*  In addition, all resources are freed and all queues are searched to */
/*  ensure command restart occurs without error.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
void
psc_cleanup_reset_error()
{
    struct dev_info *dev_ptr;	/* device structure */
    struct sc_buf *bp;
    int     i;

    DEBUG_0 ("Entering psc_cleanup_reset_error routine\n")
    TRACE_1 ("in rserr", 0)

    /* make sure the adapter timers are stopped */
    w_stop(&(adp_str.adap_watchdog.dog));
    /* loop while devices are on Active queue */
    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
	/* this error may have been caused by a timeout or epow */
	/* or a glitch on the reset line                        */
	w_stop(&dev_ptr->dev_watchdog.dog);
	if ((bp = dev_ptr->active) != NULL)
	{
	    /* free resources for this command sta's, tcw's */
	    (void) psc_free_resources(dev_ptr, FALSE);
	    /* if active cmd status not set */
	    if (!(bp->bufstruct.b_flags & B_ERROR))
	    {
		bp->status_validity = SC_ADAPTER_ERROR;
		bp->general_card_status = SC_SCSI_BUS_RESET;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
	    }
	    iodone((struct buf *) bp);
	}
	dev_ptr->active = NULL;
	dev_ptr->flags = RETRY_ERROR;
	psc_deq_active(dev_ptr);
	if ((adp_str.ABORT_BDR_head == dev_ptr) ||
	    (dev_ptr->ABORT_BDR_bkwd != NULL))
	    psc_deq_abort_bdr(dev_ptr);
	if (dev_ptr->head_pend != NULL)
	    psc_fail_cmd(dev_ptr, 1); /* kill pending cmds */
	if (dev_ptr->queue_state == STOPPING)
	    e_wakeup(&dev_ptr->stop_event);
	else
	    dev_ptr->queue_state = HALTED;
	if (dev_ptr->ioctl_wakeup == TRUE)
	{
	    if (dev_ptr->flags & CAUSED_TIMEOUT)
	    {
		dev_ptr->ioctl_errno = ETIMEDOUT;
	    }
	    else
	    {
		dev_ptr->ioctl_errno = EIO;
	    }
	    dev_ptr->ioctl_wakeup = FALSE;
	    e_wakeup(&dev_ptr->ioctl_event);
	}
	dev_ptr = adp_str.DEVICE_ACTIVE_head;
    }	/* end of while loop */

    dev_ptr = adp_str.DEVICE_WAITING_head;
    while (dev_ptr != NULL)
    {
	psc_deq_wait(dev_ptr);
	(void) psc_free_resources(dev_ptr, FALSE);
	if (dev_ptr->head_pend == NULL)
	{	/* queue is empty  */
	    dev_ptr->head_pend = dev_ptr->active;
	    dev_ptr->tail_pend = dev_ptr->active;
	}
	else
	{	/* pending queue not empty */
	    /* point first cmd's av_forw at the new request */
	    dev_ptr->active->bufstruct.av_forw =
					  (struct buf *) dev_ptr->head_pend;
	    dev_ptr->head_pend = dev_ptr->active;
	}
	dev_ptr->active = NULL;
	dev_ptr = adp_str.DEVICE_WAITING_head;
    }

    dev_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;
    while (dev_ptr != NULL)
    {
	psc_deq_wait_resources(dev_ptr);
	dev_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;
    }

    /* clear the DMA and scsi fifo's on the chip */
    if (psc_write_reg(DFIFO, DFIFO_SIZE, 0x40) == REG_FAIL)
    {	/* if there's a failure to clear the fifo, reset the chip */
	psc_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA);
    }
    /* loop through hash table setting the negotiation structs to TRUE */
    for (i = 0; i <= MAX_DEVICES; i++)
    {
	dev_ptr = adp_str.device_queue_hash[i];
	/* a check is made to determine if the device queue has */
	/* pending command that, due to the reset error, were   */
	/* moved to the waiting or waiting for resoures queues. */
	/* if so, they are taken care of here.                  */
	if (dev_ptr != NULL)
	{
	    dev_ptr->negotiate_flag = TRUE;
	    dev_ptr->restart_in_prog = TRUE;
	}
    }
    adp_str.restart_watchdog.dog.restart =
	adp_str.ddi.cmd_delay + 1;
    w_start(&(adp_str.restart_watchdog.dog));
    TRACE_1 ("out rser", 0)
    DEBUG_0 ("leaving psc_cleanup_reset_error\n")
}  /* end psc_cleanup_reset_error */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_epow                                                        */
/*                                                                        */
/* FUNCTION:  Scsi chip device driver suspend routine.                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called when an EPOW (Early Power Off Warning)  */
/*         has been detected.                                             */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - adapter unique data structure (one per adapter)     */
/*      intr    -  kernel interrupt handler information structure         */
/*                                                                        */
/* INPUTS:                                                                */
/*      handler - pointer to the intrpt handler structure                 */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      INTR_SUCC - interrupt was not processed                           */
/*      INTR_SUCC - interrupt was processed                               */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      i_disable  i_enable                                               */
/*                                                                        */
/**************************************************************************/
int
psc_epow(struct intr * handler)
{
    int     old_pri1, old_pri2;

    DEBUG_0 ("Entering psc_epow routine.\n")
    if ((handler->flags & EPOW_SUSPEND) ||
	((handler->flags & EPOW_BATTERY) &&
	 (!(adp_str.ddi.battery_backed))))
    {
	DEBUG_0 ("psc_epow: suspend\n")
	adp_str.epow_state = EPOW_PENDING;
	psc_command_reset_scsi_bus();
    }	/* endif */
    else  /* restart again after an EPOW has occurred */
    {
	if (handler->flags & EPOW_RESUME)
	{
	    DEBUG_0 ("psc_epow: resume\n")
	    /* handle a resume command */
	    /* disable to int level during this */
	    old_pri1 = i_disable(adp_str.ddi.int_prior);

	    /* disable to close window around the test */
	    old_pri2 = i_disable(INTEPOW);

	    if (((!(handler->flags & EPOW_BATTERY)) &&
		 (!(handler->flags & EPOW_SUSPEND))) &&
		(adp_str.epow_state == EPOW_PENDING))
	    {
		adp_str.epow_state = 0;	/* reset epow state */
		i_enable(old_pri2);	/* return to lower priority */
	    }
	    else
	    {   /* either a SUSPEND has re-occurred, or this */
		/* adap was not put in epow pending state.   */
		/* for these cases--leave adapter as is      */
		i_enable(old_pri2);	/* return to lower priority */
	    }
	    i_enable(old_pri1);	/* re-enable */
	}	/* end if */
    }	/* end else */

    DEBUG_0 ("leaving epow: intr_succeeded\n")
    return (INTR_SUCC);
}  /* end psc_epow */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_command_watchdog                                            */
/*                                                                        */
/* FUNCTION:  Command timer routine.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called when a command has timed out.  A BDR is */
/*         issued in an attempt to cleanup the device This routines exe-  */
/*         cutes to reset the bus during normal command executtion.       */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*  w - gives you the address of the dev_info structure                   */
/*                                                                        */
/* INPUTS: None                                                           */
/*         w - pointer to watch dog timer structure which timed out       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION: None                                         */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*        w_start  e_wakeup                                               */
/**************************************************************************/
void
psc_command_watchdog(struct watchdog * w)
{
    struct dev_info *dev_ptr;
    struct sc_buf *bp;
    struct timer *tdog;
    int     i, old_pri;
    uchar   issue_abrt_bdr;
    uchar   istat_val;
    caddr_t iocc_addr;
    register int rc;

    DEBUG_0 ("Entering psc_command_watchdog\n")
    old_pri = i_disable(adp_str.ddi.int_prior);

    tdog = (struct timer *) w;
    TRACE_1 ("in watch", tdog->timer_id)
    if (tdog->timer_id == PSC_SIOP_TMR)
    {
	w_stop(&adp_str.adap_watchdog.dog);
	/* if we failed in causing a chip abort, the chip is hung */
	/* reset the chip and DMA before doing a scsi bus reset.  */
	psc_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA);
	psc_command_reset_scsi_bus();
    }
    else
	if (tdog->timer_id == PSC_RESET_TMR)
	{
	    w_stop(&adp_str.reset_watchdog.dog);
	    /* if we failed in causing the scsi bus to reset.  This   */
	    /* guarantees that the chip is hung, so reset the chip.   */
	    psc_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA);
	    psc_command_reset_scsi_bus();
	}
	else
	    if (tdog->timer_id == PSC_RESTART_TMR)
	    {
                TRACE_1 ("restart timer", 0);
		w_stop(&adp_str.restart_watchdog.dog);
		if (adp_str.epow_state == EPOW_PENDING)
		{	/* an epow is still pending, restart timer */
		    adp_str.restart_watchdog.dog.restart =
			adp_str.ddi.cmd_delay + 1;
		    w_start(&(adp_str.restart_watchdog.dog));
		}
		else
		{
		    /* loop through hash table restarting each device */
		    for (i = 0; i <= MAX_DEVICES; i++)
		    {
			dev_ptr = adp_str.device_queue_hash[i];
			if ((dev_ptr != NULL) && (dev_ptr->restart_in_prog))
			{
			    dev_ptr->restart_in_prog = FALSE;
			    if (dev_ptr->head_pend != NULL)
				psc_start(dev_ptr);
			}
		    }
		}
	    }
	    else
	    {
		/* if here a fatal command timeout occurred */
		dev_ptr = (struct dev_info *) ((uint) (tdog));
		w_stop(&dev_ptr->dev_watchdog.dog);
		dev_ptr->flags |= CAUSED_TIMEOUT;
		if (dev_ptr->cmd_activity_state == 0)
		    return;
		if ((adp_str.ABORT_BDR_head == dev_ptr) ||
		    (dev_ptr->ABORT_BDR_bkwd != NULL))
		    issue_abrt_bdr = TRUE;
		else
		    issue_abrt_bdr = FALSE;
		TRACE_1 ("iss bdr", issue_abrt_bdr)
		/* is this is an abort or bdr */
		DEBUG_1 ("Command active = %d\n", dev_ptr->cmd_activity_state)
		DEBUG_1 ("issue_abrt_bdr = %d\n", issue_abrt_bdr)
		if ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) ||
		    (dev_ptr->cmd_activity_state == BDR_IN_PROGRESS) ||
		    (issue_abrt_bdr == TRUE))
		{
		    DEBUG_0 ("psc_command_watchdog: ABORT/BDR in PROGRESS\n")
		    if (((bp = dev_ptr->active) != NULL) &&
			(!(bp->bufstruct.b_flags & B_ERROR)))
		    {
			bp->status_validity = SC_ADAPTER_ERROR;
			bp->general_card_status = SC_CMD_TIMEOUT;
			bp->scsi_status = SC_GOOD_STATUS;
			bp->bufstruct.b_flags |= B_ERROR;
			bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
			bp->bufstruct.b_error = EIO;
		    }
		    psc_command_reset_scsi_bus();	/* reset scsi bus */
		}
		else	/* regular command */
		{
		    DEBUG_0 ("psc_command_watchdog: regular cmd \n")
                    TRACE_1 ("regular cmd", (int) dev_ptr)
		    if (issue_abrt_bdr)
			psc_deq_abort_bdr(dev_ptr);
		    if (dev_ptr->ioctl_wakeup)
		    {
			e_wakeup(&dev_ptr->ioctl_event);
			dev_ptr->ioctl_errno = ETIMEDOUT;
			dev_ptr->ioctl_wakeup = FALSE;
		    }
		    /* set up error's will come in through BDR */
		    /* after BDR */
		    if ((bp = dev_ptr->active) != NULL)
		    {
			bp->status_validity = SC_ADAPTER_ERROR;
			bp->general_card_status = SC_CMD_TIMEOUT;
			bp->scsi_status = SC_GOOD_STATUS;
			bp->bufstruct.b_flags |= B_ERROR;
			bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
			bp->bufstruct.b_error = EIO;
		    }
		    dev_ptr->flags |= SCSI_BDR;
		    dev_ptr->flags &= ~RETRY_ERROR;
		    dev_ptr->retry_count = 1;
		    psc_enq_abort_bdr(dev_ptr);
		    /* if dev is not connected                          */
		    /* to the scsi bus, then SUSPEND processing by chip */
		    /* (use a timer to keep an eye on things)           */

		    /* see if chip is waiting for reselection */
		    iocc_addr =
			BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
		    psc_patch_iowait_int_on();
		    for (i = 0; i < PIO_RETRY_COUNT; i++)
		    {
			rc = BUS_GETCX(iocc_addr + ISTAT, (char *) &istat_val);
                        if ((rc == 0) && !(istat_val & CONN_INT_TEST)) 
                          /* Successful register read, & chip not connected */
			{
			    rc = BUS_PUTCX(iocc_addr + ISTAT, 0x80);
			    if (rc == 0)
			    {
				/* Start a timer for the chip suspend */
				w_start(&(adp_str.adap_watchdog.dog));
				break;
			    }
			}
			if (rc == 0)
			{	/* no pio error to retry, so */
			    break;
			}
		    }
		    BUSIO_DET(iocc_addr);
		    if (rc != 0)
		    {
			psc_command_reset_scsi_bus();
		    }
		    else
		    {
			dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
			w_start(&dev_ptr->dev_watchdog.dog);
		    }
		}
	    }	/* else dev_ptr */
    TRACE_1 ("outwatch", 0)
    i_enable(old_pri);
    DEBUG_0 ("Leaving psc_command_watchdog\n")
}  /* psc_command_watchdog */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_command_reset_scsi_bus                                      */
/*                                                                        */
/* FUNCTION:  Resets the SCSI bus lines.                                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine will toggle the SCSI bus reset line for 30 micro- */
/*         seconds to assert a reset on the SCSI bus. This routines exe-  */
/*         cutes to reset the bus during normal command executtion.       */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS: None                                                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      PSC_NO_ERR - for good completion.                                 */
/*      ERRNO      - value otherwise                                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
psc_command_reset_scsi_bus()
{
    uchar   reset_chip;
    int     rc, i;
    int     old_pri;

    DEBUG_0 ("Entering psc_comand_reset_scsi_bus\n")
    TRACE_1 ("in busrs", 0)
    old_pri = i_disable(adp_str.ddi.int_prior);
    rc = psc_write_reg(ISTAT, ISTAT_SIZE, 0x80);  /* assert the SIOP abort */
    if (rc != REG_FAIL)
    {
	reset_chip = TRUE;
	/* check to see when abort has occurred */
	for (i = 0; i < 30; i++)
	{
	    psc_delay(1);
	    /* deassert abort cmd */
	    rc = psc_read_reg(ISTAT, ISTAT_SIZE);
	    if (!(rc & DIP))
	    {
		if (rc & SIP)
		    (void) psc_read_reg(SSTAT0, SSTAT0_SIZE);
		continue;
	    }
	    /* deassert abort cmd */
	    rc = psc_write_reg(ISTAT, ISTAT_SIZE, 0x00);
	    if (rc == REG_FAIL)
	    {
		break;
	    }

	    /* read the scsi and dma status registers to clear them */
	    rc = psc_read_reg(DSTAT, DSTAT_SIZE);
	    if (rc == REG_FAIL)
	    {
		break;
	    }

	    if (!(rc & DABRT))
	    {
		rc = psc_write_reg(ISTAT, ISTAT_SIZE, 0x80);
		continue;
	    }
	    reset_chip = FALSE;
	    break;
	}
    }
    /* handle fall through case */
    /* if we failed in causing a chip abort, the chip is hung. */
    /* reset the chip and DMA before doing a scsi bus reset.   */
    if ((reset_chip) || (rc == REG_FAIL))
    {
	TRACE_2 ("rbus", reset_chip, rc)
	psc_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA);
    }

    /* attempt to assert the reset */
    /* write SCNTL1 to assert scsi reset */
    /* assert the SCSI reset signal */
    (void) psc_write_reg(SCNTL1, SCNTL1_SIZE, (SCNTL1_INIT | 0x08));
    psc_delay(30);

    /* write SCNTL1 to deassert scsi reset */
    (void) psc_write_reg(SCNTL1, SCNTL1_SIZE, SCNTL1_INIT);

    w_start(&(adp_str.reset_watchdog.dog));
    TRACE_1 ("out busr", 0)
            i_enable(old_pri);
    DEBUG_0 ("LEAVING psc_command_reset_scsi_bus\n")
}  /* psc_command_reset_scsi_bus */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_find_devinfo                                                */
/*                                                                        */
/* FUNCTION:  Finds the dev_info whose script addr is in the DSP          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      may be called by interrupt or process                             */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS: None                                                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      struct dev_info *dev_ptr                                          */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      NULL    if address not found                                      */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
struct dev_info *
psc_find_devinfo(
		 int predefined_dsp)
{
    struct dev_info *dev_ptr;
    int     script_size;
    int     dsp;

    DEBUG_0 ("Entering psc_find_devinfo\n")

    /* theory is get script dma addr from DSP       */
    /* and then look at the dev_info active que     */
    /* and see if the address falls into this arena */
            script_size = INSTRUCTIONS * 8;
    if (predefined_dsp != 0)
	dsp = predefined_dsp;
    else
	dsp = word_reverse(psc_read_reg(DSP, DSP_SIZE));
    if (dsp == REG_FAIL)
	return (NULL);

    /* go through active queue and see if we can find our device */
    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
	if ((dsp >= (dev_ptr->script_dma_addr + Ent_scripts_entry_point)) &&
	    (dsp <= (dev_ptr->script_dma_addr + script_size)))
	{
	    DEBUG_1 ("leaving psc_find_devinfo: dev_ptr= 0x%x\n", dev_ptr);
	    return (dev_ptr);
	}
	dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;	/* go to next entry */
    }	/* while */
    TRACE_1 ("fnd_info", dsp)
    return (NULL);
}  /* end of psc_find_devinfo */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_fail_free_resources                                         */
/*                                                                        */
/* FUNCTION:  Handles the case where free resources fails                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      may be called by interrupt or process                             */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      sruct dev_info, struct sc_buf, int err_code                       */
/* RETURN VALUE DESCRIPTION:                                              */
/*      none                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
int
psc_fail_free_resources(
			struct dev_info * dev_ptr,
			struct sc_buf * bp,
			int err_code)
{
    DEBUG_1 ("Entering psc_fail_free_resources error= 0x%x\n", err_code);
    TRACE_1 ("in failf", 0)
    if (err_code == DMA_SYSTEM)
    {
	psc_cleanup_reset(DMA_ERR);
	psc_logerr(ERRID_SCSI_ERR2, DMA_ERROR, 185,
		   0, dev_ptr, TRUE);
	TRACE_1 ("out faif", 0)
	return (TRUE);
    }
    else
	if (err_code == PSC_DMA_ERROR)
	{
	    if (!(bp->bufstruct.b_flags & B_ERROR))
	    {	/* no previous error */
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->status_validity = SC_ADAPTER_ERROR;
		bp->general_card_status = SC_HOST_IO_BUS_ERR;
		bp->scsi_status = SC_GOOD_STATUS;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
		psc_logerr(ERRID_SCSI_ERR2, DMA_ERROR, 185,
			   0, dev_ptr, TRUE);
	    }
	    iodone((struct buf *) bp);
	    /* fail the rest of the pending queue */
	    if (dev_ptr->head_pend != NULL)
		psc_fail_cmd(dev_ptr, 1);
	}
	else	/* PSC_COPY_ERROR */
	{
	    if (!(bp->bufstruct.b_flags & B_ERROR))
	    {	/* no previous error */
		bp->status_validity = 0;
		bp->bufstruct.b_error = EFAULT;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    }
	    iodone((struct buf *) bp);
	    /* fail the rest of the pending queue */
	    if (dev_ptr->head_pend != NULL)
		psc_fail_cmd(dev_ptr, 1);
	}
    DEBUG_0 ("leaving psc_fail_free_resources\n")
    TRACE_1 ("out faif", 0)
    return (FALSE);
}  /* psc_fail_free_resources */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_issue_abort_bdr                                             */
/*                                                                        */
/* FUNCTION:  Handles the issueing of bdr/aborts for interrupts           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      may be called by interrupt or process                             */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      struct dev_info, struct sc_buf, int err_code                      */
/*      int iodone_flag     if =1 halt queue                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      none                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_issue_abort_bdr(
		    struct dev_info * dev_ptr,
		    uint iodone_flag)
{
    struct sc_buf *bp;
    int     dev_index;
    uint    rc;

    DEBUG_0 ("Entering psc_issue_abort_bdr\n")
    TRACE_1 ("in issAB", (int) dev_ptr)
    if (dev_ptr->flags & SCSI_BDR)
    {
	/* update wait script */
	dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
	psc_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr, 
			      dev_index);
	/* set the status */
	bp = dev_ptr->active;
	if ((iodone_flag) && (!(bp->bufstruct.b_flags & B_ERROR)))
	{
	    bp->scsi_status = GET_CMD_STATUS_BYTE(
		adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr);
	    if (bp->scsi_status != SC_GOOD_STATUS)
	    {
		bp->status_validity = SC_SCSI_ERROR;
		bp->bufstruct.b_error = EIO;
		bp->bufstruct.b_flags |= B_ERROR;
	    }
	    else
	    {
		bp->bufstruct.b_error = 0;
		bp->scsi_status = SC_GOOD_STATUS;
		bp->status_validity = 0;
	    }
	}
	else
	    if (!(iodone_flag) && (!(bp->bufstruct.b_flags & B_ERROR)))
	    {
		/* setup and issue the BDR */
		bp->status_validity = 0;
		bp->scsi_status = 0;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_error = ENXIO;
		bp->bufstruct.b_flags |= B_ERROR;
	    }
	if (psc_issue_bdr_script(
	      (uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
	      (uint *) dev_ptr->script_dma_addr, dev_index) == REG_FAIL)
	{
	    psc_cleanup_reset(REG_ERR);
	    TRACE_1 ("outissB", REG_FAIL)
	    return (FALSE);
	}
	dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;
	dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
	dev_ptr->flags &= ~RETRY_ERROR;
	dev_ptr->retry_count = 1;
	w_start(&dev_ptr->dev_watchdog.dog);
	psc_deq_abort_bdr(dev_ptr);
	TRACE_1 ("outissB", 0)
	return (FALSE);
    }
    else	/* abort was requested */
    {
	psc_deq_abort_bdr(dev_ptr);
	/* remove device from ABRT/BDR queue */
	bp = dev_ptr->active;
	if ((iodone_flag) && (!(bp->bufstruct.b_flags & B_ERROR)))
	{
	    bp->scsi_status =
		GET_CMD_STATUS_BYTE(
		 adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr);
	    if (bp->scsi_status != SC_GOOD_STATUS)
	    {
		bp->status_validity = SC_SCSI_ERROR;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
		rc = psc_free_resources(dev_ptr, FALSE);
		if ((rc != PSC_NO_ERR) &&
		    (psc_fail_free_resources(dev_ptr, bp, rc)))
		{
		    TRACE_1 ("outissAB", 1)
		            return (FALSE);
		}
	    }
	    else
	    {
		bp->bufstruct.b_error = 0;
		bp->scsi_status = SC_GOOD_STATUS;
		bp->status_validity = 0;
		rc = psc_free_resources(dev_ptr, TRUE);
		if ((rc != PSC_NO_ERR) &&
		    (psc_fail_free_resources(dev_ptr, bp, rc)))
		{
		    TRACE_1 ("outissAB", 2)
		            return (FALSE);
		}
	    }
	}
	else
	    if (!(iodone_flag) && (!(bp->bufstruct.b_flags & B_ERROR)))
	    {
		bp->status_validity = 0;
		bp->scsi_status = 0;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_error = ENXIO;
		bp->bufstruct.b_flags |= B_ERROR;
		rc = psc_free_resources(dev_ptr, FALSE);
		if ((rc != PSC_NO_ERR) &&
		    (psc_fail_free_resources(dev_ptr, bp, rc)))
		{
		    TRACE_1 ("outissAB", 3)
		            return (FALSE);
		}
	    }
	iodone((struct buf *) bp);
	dev_ptr->flags = RETRY_ERROR;
	dev_ptr->active = NULL;
	psc_deq_active(dev_ptr);
	/* fail and iodone pending cmds with ENXIO */
	if (dev_ptr->head_pend != NULL)
	    psc_fail_cmd(dev_ptr, 1);

	if (dev_ptr->queue_state == STOPPING)
	    e_wakeup(&dev_ptr->stop_event);
	else
	    dev_ptr->queue_state = HALTED;
	/* if IOCTL wakeup flag is TRUE then call wakeup */
	/* and set IOCTL status to EIO                   */
	if (dev_ptr->ioctl_wakeup == TRUE)
	{
	    dev_ptr->ioctl_errno = EIO;
	    dev_ptr->ioctl_wakeup = FALSE;
	    e_wakeup(&dev_ptr->ioctl_event);
	}
	/* remove device from ACTIVE queue */
    }	/* end else  abort */
    DEBUG_0 ("Leaving psc_issue_abort_bdr\n")
    TRACE_1 ("outissAB", 4)
    return (TRUE);
}  /* psc_issue_abort_bdr */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_scsi_parity_error                                           */
/*                                                                        */
/* FUNCTION:  Handles the scsi parity error function                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      may be called by interrupt or process                             */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      struct dev_info:  device/lun associated with the error            */
/*      save_interrupt:   encodes which error occurred                    */
/*      issue_abrt_bdr:   bit 1: is an unexpected disconnect int. pending */
/*                               (only used for Selection Time-outs)      */
/*                        bit 0: should an abort be issued                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      none                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_scsi_parity_error(
		      struct dev_info * dev_ptr,
		      int save_interrupt,
		      int issue_abrt_bdr)
{
    struct sc_buf *bp;
    uchar   log_err, gen_err;
    int     i, istat_val, sstat_val;
    uint    dev_index;

    DEBUG_0 ("psc_scsi_parity_error: \n")
    TRACE_2 ("in perr ", (int) dev_ptr, save_interrupt)
    w_stop(&dev_ptr->dev_watchdog.dog);
    switch (save_interrupt)
    {
    case SCSI_UNEXP_DISC:
	log_err = UNEXPECTED_BUS_FREE;
	gen_err = SC_SCSI_BUS_FAULT;
	break;
    case SCSI_GROSS_ERR:
	log_err = GROSS_ERROR;
	gen_err = SC_ADAPTER_HDW_FAILURE;
	break;
    case SCSI_PARITY_ERR:
	log_err = BAD_PARITY_DETECTED;
	gen_err = SC_HOST_IO_BUS_ERR;
	break;
    case SCSI_SEL_FAIL:
	dev_ptr->flags |= SELECT_TIMEOUT;
	log_err = SC_NO_DEVICE_RESPONSE;
	gen_err = SC_NO_DEVICE_RESPONSE;
	break;
    }
    /* for selection timeouts, the chip was designed to */
    /* return an unexpected disconnect after a selection */
    /* timeout.  So in the cases of a selection timeout */
    /* where the ISTAT shows another SIP interrupt wait- */
    /* ing, it can be assumed that this is the discon-  */
    /* nect that is expected.  An attempt is made to    */
    /* clear it here via a read loop of the istat for up */
    /* to 10 usecs.  If this fails, then exit the loop  */
    /* and issue an abort to the device.                */
    if (save_interrupt == SCSI_SEL_FAIL)
    {
        /* if the unexpected disconnect has not already been received */
        if (issue_abrt_bdr & 0x02)
        {
            /* turn off bit indicating unexpected disconnect pending */
            issue_abrt_bdr &= 0xFD;

            /* loop looking to clear the unexpected disconnect int.  */
	    for (i = 0; i < 10; i++)
	    {
	        psc_delay(1);
	        istat_val = psc_read_reg(ISTAT, ISTAT_SIZE);
	        if (istat_val == REG_FAIL)
	        {
		    psc_cleanup_reset(REG_ERR);
		    TRACE_1 ("out perr", 0)
		    return (FALSE);
	        }
	        if (istat_val & SIP)
	        {
		    sstat_val = psc_read_reg(SSTAT0, SSTAT0_SIZE);
		    if (sstat_val == REG_FAIL)
		    {
		        psc_cleanup_reset(REG_ERR);
		        TRACE_1 ("out perr", 0)
		        return (FALSE);
		    }
		    if (sstat_val & SCSI_UNEXP_DISC)
		        break;
		}
	    }
	}
    }
    /* for unexpected disconnects, it is possible that a */
    /* selection timeout interrupt is stacked due to the */
    /* design of the chip.  Here, the interrupt is      */
    /* cleared and the error returned is set to repre-  */
    /* set a selection timeout.                         */
    if (save_interrupt == SCSI_UNEXP_DISC)
    {
	istat_val = psc_read_reg(ISTAT, ISTAT_SIZE);
	if (istat_val == REG_FAIL)
	{
	    psc_cleanup_reset(REG_ERR);
	    TRACE_1 ("out perr", 0)
	    return (FALSE);
	}
	if (istat_val & SIP)
	{
	    sstat_val = psc_read_reg(SSTAT0, SSTAT0_SIZE);
	    if (sstat_val == REG_FAIL)
	    {
		psc_cleanup_reset(REG_ERR);
		TRACE_1 ("out perr", 0)
		return (FALSE);
	    }
	    if (sstat_val & SCSI_SEL_FAIL)
	    {
		save_interrupt = SCSI_SEL_FAIL;
		dev_ptr->flags |= SELECT_TIMEOUT;
		log_err = SC_NO_DEVICE_RESPONSE;
		gen_err = SC_NO_DEVICE_RESPONSE;
		issue_abrt_bdr = FALSE;
	    }
	}
    }
    bp = dev_ptr->active;
    /* It is determined which machine caused this */
    /* issue an abort if one is not in progress */
    if (((dev_ptr->cmd_activity_state != ABORT_IN_PROGRESS) &&
	 (dev_ptr->cmd_activity_state != BDR_IN_PROGRESS)) &&
	(issue_abrt_bdr))
    {
	dev_index = INDEX(bp->scsi_command.scsi_id,
			  (bp->scsi_command.scsi_cmd.lun) >> 5);

	if ((bp != NULL) &&
	    (!(bp->bufstruct.b_flags & B_ERROR)))
	{
	    /* set scsi stat in sc_buf and bytes   */
	    /* based on the error that got us here */
	    bp->status_validity = SC_ADAPTER_ERROR;
	    bp->general_card_status = gen_err;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_error = EIO;
	    if (save_interrupt != SCSI_SEL_FAIL)
		psc_logerr(ERRID_SCSI_ERR10, log_err,
			   190, 0, dev_ptr, TRUE);
	}

	/* read ISTAT to see if we are connected to the scsi bus */
   	if ((istat_val = psc_read_reg(ISTAT, ISTAT_SIZE)) == REG_FAIL)
        {
            psc_cleanup_reset(REG_ERR);
            TRACE_1 ("out perr", 0)
            return (FALSE);
        }

	/* update wait script */
	psc_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr,
			      dev_index);
	if (psc_issue_abort_script(
		  (uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
		  (uint *) dev_ptr->script_dma_addr, dev_ptr, dev_index, 
		  ((uchar) istat_val & CONNECTED))
		   == REG_FAIL)
	{
	    psc_cleanup_reset(REG_ERR);
	    TRACE_1 ("out perr", 0)
	    return (FALSE);
	}
	dev_ptr->cmd_activity_state = ABORT_IN_PROGRESS;
	dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
	dev_ptr->flags |= SCSI_ABORT;
	dev_ptr->flags &= ~RETRY_ERROR;
	dev_ptr->retry_count = 1;
	w_start(&dev_ptr->dev_watchdog.dog);
	TRACE_1 ("out perr", 0)
	return (FALSE);
    }
    else
    {	/* abort/bdr on command with no abort completed w/error */
	/* for command timeouts, a BDR is sent to clear the     */
	/* drive.  The expected result is an unexpected discon- */
	/* nect.  If this didn't happen or a selection timeout  */
	/* was recorded, reset the scsi bus.                    */
	if ((dev_ptr->flags & CAUSED_TIMEOUT) &&
	    ((save_interrupt != SCSI_UNEXP_DISC) ||
	     (dev_ptr->flags & SELECT_TIMEOUT)))
	{
	    psc_command_reset_scsi_bus();	/* reset scsi bus */
	    return (FALSE);
	}
	if (bp != NULL)
	{
	    if (!(bp->bufstruct.b_flags & B_ERROR))
	    {
		/* set scsi stat in sc_buf and bytes   */
		/* based on the error that got us here */
		bp->status_validity = SC_ADAPTER_ERROR;
		bp->general_card_status = gen_err;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_error = EIO;
		/* For the case of a scsi selction failure, or */
		/* an unexpected disconnect (bus free) occur- */
		/* ing during negotiation, or an unexpected   */
		/* disconnect (bus free) occurring after a    */
		/* sync message reject, or a bus free occuring */
		/* after an abort or bdr is issued to a device */
		/* no log entry should be made, These are all */
		/* legal scsi phases.  An error, however,     */
		/* should be returned to the calling process. */
		if (!((save_interrupt == SCSI_SEL_FAIL) ||
		      ((save_interrupt == SCSI_UNEXP_DISC) &&
		       (dev_ptr->cmd_activity_state ==
			NEGOTIATE_IN_PROGRESS)) ||
		      ((save_interrupt == SCSI_UNEXP_DISC) &&
		       (dev_ptr->cmd_activity_state ==
			BDR_IN_PROGRESS)) ||
		      ((save_interrupt == SCSI_UNEXP_DISC) &&
		       (dev_ptr->cmd_activity_state ==
			ABORT_IN_PROGRESS))))
		{
		    psc_logerr(ERRID_SCSI_ERR10, log_err,
			       195, 0, dev_ptr, TRUE);
		}
	    }
	    /* status already set and error logged */
	    (void) psc_free_resources(dev_ptr, FALSE);
	    iodone((struct buf *) bp);
	}
	else
	    if ((save_interrupt != SCSI_SEL_FAIL) &&
		(save_interrupt != SCSI_UNEXP_DISC))
	    {
		psc_logerr(ERRID_SCSI_ERR10, log_err,
			   200, 0, dev_ptr, TRUE);
	    }
	/* remove device from active queue */
	dev_ptr->flags = RETRY_ERROR;
	dev_ptr->active = NULL;
	if ((adp_str.ABORT_BDR_head == dev_ptr) ||
	    (dev_ptr->ABORT_BDR_bkwd != NULL))
	    /* there is an abort or bdr to queued */
	    psc_deq_abort_bdr(dev_ptr);
	psc_fail_cmd(dev_ptr, 1);
	if (dev_ptr->queue_state == STOPPING)
	    e_wakeup(&dev_ptr->stop_event);
	else
	    dev_ptr->queue_state = HALTED;
	if (dev_ptr->ioctl_wakeup == TRUE)
	{
	    if (save_interrupt != SCSI_SEL_FAIL)
		dev_ptr->ioctl_errno = PSC_NO_ERR;
	    else
		dev_ptr->ioctl_errno = EIO;
	    dev_ptr->ioctl_wakeup = FALSE;
	    e_wakeup(&dev_ptr->ioctl_event);
	}
	if (dev_ptr->cmd_activity_state == BDR_IN_PROGRESS)
	{
	    dev_ptr->negotiate_flag = TRUE;
	    dev_ptr->restart_in_prog = TRUE;
	    adp_str.restart_watchdog.dog.restart =
		adp_str.ddi.cmd_delay + 1;
	    w_start(&(adp_str.restart_watchdog.dog));
	}
	psc_deq_active(dev_ptr);
    }
    TRACE_1 ("out perr", 0)
    return (TRUE);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_prep_main_script                                    */
/*                                                                      */
/* FUNCTION:    Adapter Script Command Loading Routine                  */
/*      This will patch in the SCSI command bytes, found by looking     */
/*      at the sc_buf to be executed.  Then, it will patch in the       */
/*      number of bytes to be transfered (if any).  In addition, it     */
/*      will patch the jump within the IO_WAIT script to jump to        */
/*      this particular script when the target reselects the chip.      */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      dev_info structure                                              */
/*      sc_buf structure                                                */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      *script_vir_addr - the virtual address of the script just       */
/*              created and copied into memory.  The script that needs  */
/*              to be loaded with the actual SCSI command to be sent to */
/*              the device.  This is the pointer to the TOP             */
/*              of the script, not to the "main" portion of the script. */
/*      *dev_ptr - pointer to the dev_info structure which holds        */
/*              the sc_buf which contains the SCSI command given to us  */
/*              by the driver head.                                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_prep_main_script(
		     uint * iowait_vir_addr,
		     uint * script_vir_addr,
		     struct dev_info * dev_ptr,
		     uint script_dma_addr)
{

    ulong  *target_script, *io_wait_ptr;
    char   *script_byte_ptr;
    uchar   scsi_command, lun;
    ulong   cmd_byte_length, word_buffer;
    int     i, offset_to_jump;
    struct sc_buf *bp;

    /* Entering psc_prep_main_script */
    script_byte_ptr = (char *) script_vir_addr;
    target_script = (ulong *) script_vir_addr;
    io_wait_ptr = (ulong *) iowait_vir_addr;
    bp = dev_ptr->active;

    DEBUG_1 ("psc_prep_main: dma_addr= 0x%x\n", dev_ptr->dma_addr);
    TRACE_1 ("in prepm", target_script)

    /* Load the byte length for the scsi cmds.                            */
    cmd_byte_length = (ulong) bp->scsi_command.scsi_length;
    word_buffer = word_reverse(target_script[R_cmd_bytes_out_count_Used[0]]);
    word_buffer = ((word_buffer & 0xFF000000) | (cmd_byte_length));
    target_script[R_cmd_bytes_out_count_Used[0]] = word_reverse(word_buffer);

    /* Load the scsi command into the command buffer                      */
    scsi_command = bp->scsi_command.scsi_cmd.scsi_op_code;
    script_byte_ptr[Ent_cmd_buf] = scsi_command;

    script_byte_ptr[Ent_cmd_buf + 1] = bp->scsi_command.scsi_cmd.lun;
    for (i = 2; i < cmd_byte_length; i++)
	script_byte_ptr[Ent_cmd_buf + i] =
	    bp->scsi_command.scsi_cmd.scsi_bytes[i - 2];

    DEBUG_0 ("SCSI COMMAND ^^^^^^^^^^^^^^^^^^^^^^^^^^^ SCSI COMMAND\n")
    DEBUG_2 ("op_code is       0x%x  for ID  0x%x\n",
	      bp->scsi_command.scsi_cmd.scsi_op_code,
	      bp->scsi_command.scsi_id);
    DEBUG_0 ("SCSI COMMAND ^^^^^^^^^^^^^^^^^^^^^^^^^^^ SCSI COMMAND\n")
    if (dev_ptr->active == NULL)
    {
	dev_ptr->max_disconnect = 0;
	DEBUG_0 ("ERROR, ERROR, ERROR, ERROR, ERROR, ERROR\n")
    }
    else
    {
	if (bp->bufstruct.b_bcount == 0)
	    dev_ptr->max_disconnect = 0x00100000;
	else
	    dev_ptr->max_disconnect = bp->bufstruct.b_bcount;
    }

    DEBUG_1 ("The value set in max_disconnect is %x\n",
	      dev_ptr->max_disconnect);
    for (i = 0; i < 2; i++)
    {
	target_script[R_data_byte_count_Used[i]] &= word_reverse(0xff000000);
	target_script[R_data_byte_count_Used[i]] |= word_reverse(
						   dev_ptr->max_disconnect);
	/* Patch the dma address where the data in or out gets buffered. */
	target_script[data_addr_Used[i]] = word_reverse(dev_ptr->dma_addr);
    }
    /*********************************************************************/
    /* Almost every define and address label in the SCRIPTS is in terms  */
    /* of word offsets except those used in JUMP commands.  These are in */
    /* terms of bytes.  All offsets are relative to the beginning address*/
    /* of the scripts, where the IO_WAIT script also resides.            */
    /*********************************************************************/
    lun = bp->scsi_command.scsi_cmd.lun >> 5;
    switch (dev_ptr->scsi_id)
    {
    case 0:
	offset_to_jump = scsi_0_lun_Used[lun];
	break;
    case 1:
	offset_to_jump = scsi_1_lun_Used[lun];
	break;
    case 2:
	offset_to_jump = scsi_2_lun_Used[lun];
	break;
    case 3:
	offset_to_jump = scsi_3_lun_Used[lun];
	break;
    case 4:
	offset_to_jump = scsi_4_lun_Used[lun];
	break;
    case 5:
	offset_to_jump = scsi_5_lun_Used[lun];
	break;
    case 6:
	offset_to_jump = scsi_6_lun_Used[lun];
	break;
    case 7:
	offset_to_jump = scsi_7_lun_Used[lun];
	break;
    default:
	break;
    }
    io_wait_ptr[offset_to_jump] = word_reverse(script_dma_addr +
					       Ent_script_reconnect_point);
    /* Turn off resid subract flag */
    dev_ptr->flags &= ~RESID_SUBTRACT;
    dev_ptr->bytes_moved = bp->bufstruct.b_bcount;
    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
    dev_ptr->flags |= PREP_MAIN_COMPLETE;
    TRACE_1 ("outprepm", 0)
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_patch_async_switch_int                              */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches an interrupt*/
/*      command and the interrupt value A_change_to_async into the      */
/*      Scripts.  If this reserved memory location is not patched       */
/*      (because we have a synchronous device), then it does a simple   */
/*      NOP and drops into the portion of the main script that          */
/*      continues the SCSI command processing.                          */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered with an interrupt we are patching in      */
/*              ourselves.  This can be considered a form of code       */
/*              self-modification.                                      */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_patch_async_switch_int(
			   uint * script_vir_addr,
			   int dev_info_hash)
{
    ulong  *script_ptr;

    script_ptr = (ulong *) script_vir_addr;
    script_ptr[R_dummy_int_Used[0] - 1] = word_reverse(0x98080000);
    /* write in the int op code */
    script_ptr[R_dummy_int_Used[0]] = word_reverse((A_change_to_async |
					    ((ulong) dev_info_hash << 16)));
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_set_disconnect                                      */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches the identify*/
/*      message to have the disconnect bit either off or on depending   */
/*      on the flags value from the command of SC_NODISC.               */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to the device information structure for       */
/*              the particular script that needs to be altered.         */
/*                                                                      */
/*      chk_disconnect - flag used to determine if the device wants no  */
/*              disconnect to take place.                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_set_disconnect(
		   struct dev_info * dev_ptr,
		   uchar chk_disconnect)
{
    ulong  *script_ptr;
    ulong   id_bit_mask;

    script_ptr = (ulong *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr;
    dev_ptr->disconnect_flag = chk_disconnect;
    if (chk_disconnect & SC_NODISC)
	id_bit_mask = 0x80000000;
    else
	id_bit_mask = 0xC0000000;
    id_bit_mask |= (((ulong) dev_ptr->lun_id) << 24);
    script_ptr[(Ent_identify_msg_buf / 4)] = id_bit_mask;
    script_ptr[(Ent_identify_msg_buf / 4) + 1] = 0x00000000;
    script_ptr[(Ent_sync_msg_out_buf / 4)] = 0x00010301 | id_bit_mask;
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_patch_iowait_int_on                                 */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches an interrupt*/
/*      command and the interrupt value A_iowait_int into the           */
/*      Scripts.                                                        */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered with an interrupt we are patching in      */
/*              ourselves.  This can be considered a form of code       */
/*              self-modification.                                      */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_patch_iowait_int_on()
{
    ulong  *script_ptr;

    script_ptr = (ulong *) adp_str.SCRIPTS[0].script_ptr;
    script_ptr[A_io_wait_Used[0] - 1] = word_reverse(0x98080000);
    return;

}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_patch_iowait_int_off                                */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches an interrupt*/
/*      command and the interrupt value A_iowait_int into the           */
/*      Scripts.                                                        */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_patch_iowait_int_off()
{
    ulong  *script_ptr;

    script_ptr = (ulong *) adp_str.SCRIPTS[0].script_ptr;
    script_ptr[A_io_wait_Used[0] - 1] = word_reverse(0x80000000);
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_reselect_router                                     */
/*                                                                      */
/* FUNCTION:    Reselect router after an abort                          */
/*      This function routes the command to the proper place when an    */
/*      abort has taken place and we can't issue a new command.         */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine only be caused by the interrupt handler and        */
/*      can not page fault.                                             */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      None.                                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_reselect_router()
{
    uchar   targ_scsi_id;

    targ_scsi_id = psc_read_reg(SFBR, SFBR_SIZE);

    TRACE_1 ("resl rout", (uint) targ_scsi_id)

    if (targ_scsi_id != 0xff)
    {
	/* mask off our own scsi id */
	targ_scsi_id &= ~(0x01 << adp_str.ddi.card_scsi_id);

	/* choose the highest scsi id and restart the script */
	if (targ_scsi_id & 0x80)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_7)));
	else if (targ_scsi_id & 0x40)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_6)));
	else if (targ_scsi_id & 0x20)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_5)));
	else if (targ_scsi_id & 0x10)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_4)));
	else if (targ_scsi_id & 0x08)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_3)));
	else if (targ_scsi_id & 0x04)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_2)));
	else if (targ_scsi_id & 0x02)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_1)));
	else if (targ_scsi_id & 0x01)
	    targ_scsi_id = psc_write_reg(DSP, DSP_SIZE,
					 word_reverse(
		      ((uint) adp_str.SCRIPTS[0].dma_ptr + Ent_scsi_id_0)));
	else
	{
	    targ_scsi_id = 0;
	    psc_logerr(ERRID_SCSI_ERR10, UNKNOWN_RESELECT_ID,
		       205, 0, NULL, TRUE);
	    psc_command_reset_scsi_bus();
	}
    }

    if (targ_scsi_id == 0xff)
    {
	psc_cleanup_reset(REG_ERR);
    }
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_verify_neg_answer 					*/
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches an interrupt*/
/*      command and the interrupt value A_change_to_async or A_set_     */
/*      special_sync into the SCRIPTS.  Prior to calling this function, */
/*      SXFER is set with the value for default synchronous transfers.  */
/*      This function adjusts these values if necessary.  A return code */
/*      of PSC_FAILED indicates that the caller must reject the SDTR    */
/*      message, and that the chip has been set for async transfers.  A */
/*      return code of REG_FAIL indicates a pio error occurred.         */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered with an interrupt we are patching in      */
/*              ourselves.  This can be considered a form of code       */
/*              self-modification.                                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      PSC_NO_ERR - negotiation exchange completed without problems    */
/*      REG_FAILED - pio failure, caller needs to cleanup               */
/*      PSC_FAILED - caller needs to issue a Message Reject             */
/*                                                                      */
/************************************************************************/
int
psc_verify_neg_answer(
		      uint * script_vir_addr,
		      struct dev_info * dev_ptr,
		      int dev_info_hash)

{
    uchar   xfer_period, req_ack_offset;
    int     i, rc;

    /* Entering psc_verify_neg_answer */
    rc = (int) PSC_NO_ERR;
    xfer_period = (uchar) (script_vir_addr[Ent_extended_msg_buf / 4] >> 8);
    req_ack_offset = (uchar) (script_vir_addr[Ent_extended_msg_buf / 4]);
    TRACE_2 ("v negot ", (int) xfer_period, (int) req_ack_offset)
    if ((xfer_period != DEFAULT_MIN_PHASE) ||
	(req_ack_offset != DEFAULT_BYTE_BUF))
    {
	dev_ptr->agreed_xfer = xfer_period;
	dev_ptr->agreed_req_ack = req_ack_offset;
	if (req_ack_offset == 0)
	{
	    /* if req_ack_offset is zero, we must treat this like */
	    /* an asynchronous device.  			  */
	    dev_ptr->async_device = TRUE;

	    script_vir_addr[R_dummy_int_Used[0] - 1] =
		word_reverse(0x98080000);
	    /* write in the int op code */
	    script_vir_addr[R_dummy_int_Used[0]] =
		word_reverse((A_change_to_async |
			      ((ulong) dev_info_hash << 16)));

	    /* set the chip to transfer data in async mode */
	    rc = SET_SXFER(0x00);
	    dev_ptr->special_xfer_val = 0x00;
	}
	else
	{
	    if (xfer_period <= adp_str.xfer_max[NUM_700_SYNC_RATES - 1])
	    {
	        /* the special_xfer_val has to hold the req_ack offset as */
	        /* well as the variable which controls the transfer rate  */
	        for (i = 0; i < NUM_700_SYNC_RATES; i++)
		    if (xfer_period <= adp_str.xfer_max[i])
		    {
		        dev_ptr->special_xfer_val =
			    (uchar) ((i << 4) | req_ack_offset);
		        break;
		    }
	        rc = SET_SXFER(dev_ptr->special_xfer_val);

	        /* patch in special interrupt for sync devices */
	        script_vir_addr[R_dummy_int_Used[0] - 1] =
	    	    word_reverse(0x98080000);
	        /* write in the int op code */
	        script_vir_addr[R_dummy_int_Used[0]] =
		    word_reverse((A_set_special_sync |
			          ((ulong) dev_info_hash << 16)));
	    }
	    else  /* target's requested sync transfer period is slower */
                  /* than we can set the chip.                         */
            {
	        script_vir_addr[R_dummy_int_Used[0] - 1] =
		    word_reverse(0x98080000);
	        /* write in the int op code */
	        script_vir_addr[R_dummy_int_Used[0]] =
		    word_reverse((A_change_to_async |
			      ((ulong) dev_info_hash << 16)));

	        dev_ptr->special_xfer_val = 0x00;
                dev_ptr->async_device = TRUE;
                TRACE_1 ("vneg slow", 0)
                /* Set the chip for async transfers. */
		rc = SET_SXFER(0x00);
		if (rc != REG_FAIL)
		    rc = (int) PSC_FAILED;
	    }
	}
    }
    else
    {
	dev_ptr->agreed_xfer = DEFAULT_MIN_PHASE;
	dev_ptr->agreed_req_ack = DEFAULT_BYTE_BUF;
    }

    /* Leaving psc_verify_neg_answer */

    return (rc);

}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_reset_iowait_jump                                   */
/*                                                                      */
/* FUNCTION:    Adapter Script Jump Reset Routine                       */
/*      This function does a patch of the jump instruction within the   */
/*      IO_WAIT script.  It resets the jump back to the original default*/
/*      jump that existed before it was initialized to run a script.    */
/*      This function should be called before issuing an abort or bdr   */
/*      Script.  This is action and function acts as a safety net       */
/*      against an illegal or spurious interrupt by a target device     */
/*      that is supposed to be inactive.                                */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_reset_iowait_jump(
		      uint * iowait_vir_addr,
		      int dev_info_hash)
{
    int     scsi_id, offset_to_jump, orig_byte_jump_offset;
    ulong  *io_wait_ptr;
    uchar   lun;

    io_wait_ptr = (ulong *) iowait_vir_addr,
    lun = LUN(dev_info_hash);
    scsi_id = SID(dev_info_hash);
    switch (scsi_id)
    {
    case 0:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[0] - 1;
	offset_to_jump = scsi_0_lun_Used[lun];
	break;
    case 1:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[1] - 1;
	offset_to_jump = scsi_1_lun_Used[lun];
	break;
    case 2:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[2] - 1;
	offset_to_jump = scsi_2_lun_Used[lun];
	break;
    case 3:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[3] - 1;
	offset_to_jump = scsi_3_lun_Used[lun];
	break;
    case 4:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[4] - 1;
	offset_to_jump = scsi_4_lun_Used[lun];
	break;
    case 5:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[5] - 1;
	offset_to_jump = scsi_5_lun_Used[lun];
	break;
    case 6:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[6] - 1;
	offset_to_jump = scsi_6_lun_Used[lun];
	break;
    case 7:
	orig_byte_jump_offset = A_uninitialized_reselect_Used[7] - 1;
	offset_to_jump = scsi_7_lun_Used[lun];
	break;
    default:
	break;

    }

    io_wait_ptr[offset_to_jump] =
	word_reverse((orig_byte_jump_offset * 4) + 
                                (uint) adp_str.SCRIPTS[0].dma_ptr);

    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_restore_iowait_jump                                 */
/*                                                                      */
/* FUNCTION:    Adapter Script Command Loading Routine                  */
/*      This will patch the iowait script with the address required     */
/*      for the particular device.  This routine is called after a      */
/*      selection failure when trying to issue an abort or bdr to a     */
/*      device.                                                         */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      dev_info structure                                              */
/*      sc_buf structure                                                */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      script_dma_addr - the dma address of the script just            */
/*              created and copied into memory.                         */
/*      *dev_ptr - pointer to the dev_info structure which holds        */
/*              the sc_buf which contains the SCSI command given to us  */
/*              by the driver head.                                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_restore_iowait_jump(
			uint * iowait_vir_addr,
			struct dev_info * dev_ptr,
			uint script_dma_addr)
{
    struct sc_buf *bp;
    ulong  *io_wait_ptr;
    int     offset_to_jump;
    uchar   lun;

    io_wait_ptr = (ulong *) iowait_vir_addr;
    bp = dev_ptr->active;
    /*********************************************************************/
    /* Almost every define and address label in the SCRIPTS is in terms  */
    /* of word offsets except those used in JUMP commands.  These are in */
    /* terms of bytes.  All offsets are relative to the beginning address*/
    /* of the scripts, where the IO_WAIT script also resides.            */
    /*********************************************************************/
    lun = bp->scsi_command.scsi_cmd.lun >> 5;
    switch (dev_ptr->scsi_id)
    {
    case 0:
	offset_to_jump = scsi_0_lun_Used[lun];
	break;
    case 1:
	offset_to_jump = scsi_1_lun_Used[lun];
	break;
    case 2:
	offset_to_jump = scsi_2_lun_Used[lun];
	break;
    case 3:
	offset_to_jump = scsi_3_lun_Used[lun];
	break;
    case 4:
	offset_to_jump = scsi_4_lun_Used[lun];
	break;
    case 5:
	offset_to_jump = scsi_5_lun_Used[lun];
	break;
    case 6:
	offset_to_jump = scsi_6_lun_Used[lun];
	break;
    case 7:
	offset_to_jump = scsi_7_lun_Used[lun];
	break;
    default:
	break;
    }
    io_wait_ptr[offset_to_jump] = word_reverse(script_dma_addr +
					       Ent_script_reconnect_point);
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_update_dataptr                                      */
/*                                                                      */
/* FUNCTION:    Adapter Script Save Data Pointer                        */
/*      This routine is called when the chip signals us that to save    */
/*      our data pointers before it disconnects.  Usually, this means   */
/*      that the device is going to take a break from the data transfer */
/*      to get more more data from its cache.  Since our data buffer    */
/*      (where the dma'd data is going) is still pointing to the        */
/*      same place, we need to adjust it so that when the device        */
/*      eventually comes back with the rest of the data, we do not      */
/*      overwrite what we have already received from the device.        */
/*      The DBC registers hold how many bytes we have yet to transfer   */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be patched so that the dma'd information coming from */
/*              from the target device will go to a new memory location.*/
/*      *dev_ptr - pointer to the dev_info structure which holds        */
/*              the sc_buf which contains the SCSI command given to us  */
/*              by the driver head.                                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
int
psc_update_dataptr(
		   ulong * script_vir_addr,
		   struct dev_info * dev_ptr)
{
    int     i;
    ulong   orig_byte_transfer_request;
    ulong   dma_fifo_bytes, dbc_bytes, left_over_bytes;
    ulong   new_dma_offset, *target_script;
    struct sc_buf *bp;
    register uchar sstat1_sav;

    DEBUG_0 ("Entering psc_update_dataptr\n")

    target_script = (ulong *) script_vir_addr;
    dbc_bytes = (word_reverse((ulong) psc_read_reg((uint) DBC,
						   (char) DBC_SIZE)) >> 8);
    orig_byte_transfer_request = (word_reverse(target_script
						[R_data_byte_count_Used[0]])
						 & 0x00FFFFFF);
    bp = dev_ptr->active;
    if (!(bp->bufstruct.b_flags & B_READ))
    {	/* must be a send data command */
	dma_fifo_bytes = (((psc_read_reg((uint) DFIFO, (char) DFIFO_SIZE) &
			    0x3F) - (dbc_bytes & 0x3F)) & 0x3F);
	left_over_bytes = dma_fifo_bytes + dbc_bytes;
	sstat1_sav = psc_read_reg((uint) SSTAT1, (char) SSTAT1_SIZE);
	if (sstat1_sav & 0x20)
	    left_over_bytes++;
	if ((sstat1_sav & 0x40) && !(dev_ptr->async_device))
	    left_over_bytes++;
    }
    else
	left_over_bytes = dbc_bytes;
    new_dma_offset = bp->bufstruct.b_bcount - left_over_bytes;

    DEBUG_0 ("************************************************\n")
    DEBUG_1 ("orig req = 0x%x\n", orig_byte_transfer_request);
    DEBUG_1 ("left over = 0x%x\n", left_over_bytes);
    DEBUG_1 ("new dma off = 0x%x\n", new_dma_offset);
    DEBUG_1 ("max_disconnect BEFORE change = 0x%x\n", dev_ptr->max_disconnect);

    if ((dev_ptr->max_disconnect > left_over_bytes) &&
	(left_over_bytes > 0))
	dev_ptr->max_disconnect = left_over_bytes;

    DEBUG_1 ("max_disconnect AFTER change = 0x%x\n", dev_ptr->max_disconnect);
    DEBUG_0 ("************************************************\n")

    for     (i = 0; i < 2; i++)
    {
	target_script[R_data_byte_count_Used[i]] &= word_reverse(0xff000000);
	target_script[R_data_byte_count_Used[i]] |=
				       word_reverse(dev_ptr->max_disconnect);
	target_script[data_addr_Used[i]] = word_reverse(
				       dev_ptr->dma_addr + new_dma_offset);

    }
    dev_ptr->bytes_moved = orig_byte_transfer_request - left_over_bytes;
    if (bp->scsi_command.flags & SC_NODISC)
    {
	/* This is a no disconnect device that needs the byte  */
	/* count subtraction done here.  In addition, the      */
	/* SXFER reg should be set back to it's original value */
	if (dev_ptr->async_device)
	{
	    if (SET_SXFER(0x00) == REG_FAIL)
		return (REG_FAIL);
	}
	else
	    if ((dev_ptr->agreed_xfer != DEFAULT_MIN_PHASE) ||
		(dev_ptr->agreed_req_ack < DEFAULT_BYTE_BUF))
	    {
		if (SET_SXFER(dev_ptr->special_xfer_val) == REG_FAIL)
		    return (REG_FAIL);
	    }
	bp->bufstruct.b_resid -= dev_ptr->bytes_moved;
    }
    else
    {
	dev_ptr->flags |= RESID_SUBTRACT;
    }
    DEBUG_1 ("NEW dma address is 0x%x\n",
	      word_reverse(target_script[data_addr_Used[0]]));
    DEBUG_0 ("Leaving psc_update_dataptr\n")
    return (PSC_NO_ERR);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_handle_extended_messages                            */
/*                                                                      */
/* FUNCTION:    Adapter Script to Read and Process Extended Messages    */
/*      This function does is called for two cases:                     */
/*      First, an extended message was received before a command was    */
/*      sent, after a command was sent, or after data was received.     */
/*      We must read in the variable length extended message.  Thus     */
/*      we set up the byte count of how many message bytes be read      */
/*      in and complete the reading in of the messages.                 */
/*      Second, if the extended message is a command to modify the      */
/*      data pointer, then we must modify the dma address that the      */
/*      script will dma to when the script continues.                   */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be patched to read in the variable lengthed extended */
/*              messages coming in from the target.                     */
/*      *script_dma_addr - the dma address of the script that needs to  */
/*              be fed into the DSP register so that the chip will dma  */
/*              its instructions to itself.                             */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*      *dev_ptr - pointer to the dev_info structure which holds        */
/*              the sc_buf which contains the SCSI command given to us  */
/*              by the driver head.                                     */
/*      interrupt_flag - Interrupt that resulted in this call by the    */
/*              interrupt handler.                                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      0 for success.                                                  */
/************************************************************************/
int
psc_handle_extended_messages(
			     uint * script_vir_addr,
			     uint * script_dma_addr,
			     int dev_info_hash,
			     struct dev_info * dev_ptr,
			     int interrupt_flag)
{
    uint   *target_script;
    uchar   ext_msg_opcode;
    ulong   ext_msg_length;
    int     data_ptr_offset;
    int     i, rc, dsp_val;

    DEBUG_0 ("Entering psc_handle_extended_messages\n")
    target_script = script_vir_addr;
    ext_msg_length = (target_script[Ent_extended_msg_buf / 4] >> 24);
    ext_msg_opcode = (target_script[Ent_extended_msg_buf / 4] >> 16);
    target_script[R_ext_msg_size_Used[0]] &= 0x000000FF;
    if (ext_msg_length != 0)
        target_script[R_ext_msg_size_Used[0]] |= 
			word_reverse(ext_msg_length - 1);

    if (interrupt_flag != A_modify_data_ptr)
    {
        if ((ext_msg_opcode == 0x01) ||               /* SDTR message     */
            (ext_msg_opcode == 0x02) ||  /* SCSI-1 extended identify msg  */
            (interrupt_flag == A_target_sync_sent))  /* processing a SDTR */
	{
	    if ((ext_msg_opcode == 0x01) &&
		(interrupt_flag != A_target_sync_sent))
	    {
		if (dev_ptr->active->scsi_command.flags &
		    SC_ASYNC)
		{
		    /* The target is trying to do sync negotiation */
		    /* but we do not want to do sync neg.  Thus    */
		    /* we will send a reject msg out and force it  */
		    /* into ASYNC mode                             */
		    dev_ptr->async_device = TRUE;

                    TRACE_1 ("h ext: af", 0)
		    /* patch the int opcode for async     */
		    target_script[R_dummy_int_Used[0] - 1] =
			word_reverse(0x98080000);
		    target_script[R_dummy_int_Used[0]] =
			word_reverse((A_change_to_async |
				      ((ulong) dev_info_hash << 16)));

		    /* set the chip to async mode */
		    if ((rc = SET_SXFER(0x00)) != REG_FAIL)
		    {
		        dev_ptr->special_xfer_val = 0x00;

		        /* go to script that sends msg reject */
		        /* for sync and then continues on     */
		        rc = psc_write_reg((uint) DSP,
				       (char) DSP_SIZE,
				       word_reverse(script_dma_addr +
					     (Ent_reject_target_sync / 4)));
		    }
		    return (rc);
		}
		else
		{
		    /* The target is trying to do sync negotiation */
		    /* Since we want to talk in sync mode, we now  */
		    /* set up the scripts to interrupt back into   */
		    /* this function after it has read in the      */
		    /* target's sync message.                      */
		    target_script[Ent_ext_msg_patch / 4] =
			word_reverse(0x98080000);
		    target_script[(Ent_ext_msg_patch / 4) + 1] =
			word_reverse(A_target_sync_sent |
				     ((ulong) dev_info_hash << 16));
		}
	    }
	    else
	    {
		if (interrupt_flag == A_target_sync_sent)
		{
		    /* being here tells us that we have previously patched */
		    /* this interrupt in order to move in the target's     */
		    /* sync request.  Now, we must examine the request     */
		    /* to determine if it is higher, lower, or equal to    */
		    /* what we wish to be the sync transfer rate. In this  */
		    /* case the ext_msg_length is really the xfer period   */
		    /* and the ext_msg_opcode is really the req/ack offset */

		    /* cleanup up patched interrupt.  Make it go */
		    /* back to being a NOP instruction           */
		    target_script[Ent_ext_msg_patch / 4] =
			word_reverse(0x80000000);
		    target_script[(Ent_ext_msg_patch / 4) + 1] =
			word_reverse(script_dma_addr + Ent_ext_msg_patch);

		    /* cleanup up any special or asynch inter- */
		    /* rupt that may have been set for this    */
		    /* device.                                 */
		    target_script[R_dummy_int_Used[0] - 1] =
			word_reverse(0x80000000);
		    target_script[R_dummy_int_Used[0]] =
			0x00000000;

		    if ((ext_msg_length != DEFAULT_MIN_PHASE) ||
			(ext_msg_opcode != DEFAULT_BYTE_BUF))
		    {
			if ((ext_msg_opcode == 0) || (ext_msg_length >
			     adp_str.xfer_max[NUM_700_SYNC_RATES - 1]))
                            /* If the REQ/ACK offset is zero, or the    */
		            /* requested transfer period is slower than */
			    /* we can set the chip, we will run async.  */
			{
			    /* Patch the chip to run asynchronously. */
 		            TRACE_1 ("h ext: sz", 0)
			    if (SET_SXFER(0x00) == REG_FAIL)
				return (REG_FAIL);
		            /* patch the int opcode for async     */
		            target_script[R_dummy_int_Used[0] - 1] =
			        word_reverse(0x98080000);
		            target_script[R_dummy_int_Used[0]] =
			        word_reverse((A_change_to_async |
				      ((ulong) dev_info_hash << 16)));
                            dev_ptr->async_device = TRUE;

			    if (dev_ptr->flags & NEG_PHASE_2)
			    {
                                /* we are finished negotiating */
                                dsp_val = psc_read_reg((uint) DSP,
                                                   (char) DSP_SIZE);
                                dev_ptr->flags &= ~NEG_PHASE_2;
                                if (dsp_val != REG_FAIL)
				{
                                    rc = psc_write_reg((uint) DSP, 
					    (char) DSP_SIZE, dsp_val);
				}
                                else
                                    rc = REG_FAIL;
			    }
			    else /* send a SDTR msg indicating async */
			    {
 				/* Patch in the negotiation message */
                                target_script[(Ent_sync_msg_out_buf2 / 4)] =
                                    (0x01030100 | (ulong) ext_msg_length);
                                target_script[(Ent_sync_msg_out_buf2 / 4) 
				    + 1] = ((ulong) 0x00 << 24);
                                dev_ptr->flags |= NEG_PHASE_2;
                                rc = psc_write_reg((uint) DSP,
                                           (char) DSP_SIZE,
                                           word_reverse( script_dma_addr +
                                           (Ent_renegotiate_sync / 4)));
			    }
                            return (rc);
			}
			else /* not asynchronous (and not our defaults) */
			{
			    /* make sure that the asynch flag is turned off */
			    dev_ptr->async_device = FALSE;
			    if ((ext_msg_length < DEFAULT_MIN_PHASE) ||
				(ext_msg_opcode > DEFAULT_BYTE_BUF))
                            {
                                /* we cannot accept one of the target's */
                                /* values, and must go thru negotiation.*/
                                /* Patch negotiation message, echoing   */
                                /* any value we can support.            */

				/* examine the requested REQ/ACK offset */
                                /* Patch the REQ/ACK offset part of the */
			   	/* SDTR message we are constructing.    */
                                if (ext_msg_opcode > DEFAULT_BYTE_BUF)
 			        {
                                    target_script
                                        [(Ent_sync_msg_out_buf2 / 4) + 1] =
                                        ((ulong) DEFAULT_BYTE_BUF << 24);
				    dev_ptr->agreed_req_ack = 
				    	DEFAULT_BYTE_BUF;
                                }
                                else
			        {
                                    target_script
                                        [(Ent_sync_msg_out_buf2 / 4) + 1] =
                                        ((ulong) ext_msg_opcode << 24);
				    dev_ptr->agreed_req_ack = ext_msg_opcode;
				}

				/* examine the requested xfer period    */
			        if (ext_msg_length <= DEFAULT_MIN_PHASE)
			        {
                                    target_script
                                        [(Ent_sync_msg_out_buf2 / 4)] =
                                            (0x01030100 | 
					        (ulong) DEFAULT_MIN_PHASE);
			            dev_ptr->agreed_xfer = DEFAULT_MIN_PHASE;
			        }
			        else
			        {
                                    /* We can accept the requested       */
				    /* period.  Patch the period into    */
				    /* the SDTR message we need to echo, */
				    /* saving the index into xfer_max.   */
                                    target_script[(Ent_sync_msg_out_buf2 / 4)]
					 = (0x01030100 | 
						(ulong) ext_msg_length);
				    dev_ptr->agreed_xfer = ext_msg_length;
				    for (i=1; i < NUM_700_SYNC_RATES; i++)
                    			if (ext_msg_length <= 
						adp_str.xfer_max[i])
                        		    break;
				}

				dev_ptr->special_xfer_val =
                                        (uchar) ((i << 4) |
                                                 dev_ptr->agreed_req_ack);
				TRACE_1 ("h ext 1b", (int) 
						dev_ptr->special_xfer_val)
			        rc = SET_SXFER(dev_ptr->special_xfer_val);

				if ((dev_ptr->agreed_xfer != DEFAULT_MIN_PHASE)
				   || (dev_ptr->agreed_req_ack 
							  != DEFAULT_BYTE_BUF))
				{
			            /* patch in special interrupt for  */
			            /* sync devices                    */
			            script_vir_addr[R_dummy_int_Used[0] - 1] =
						  word_reverse(0x98080000);
			            /* write in the int op code */
			            script_vir_addr[R_dummy_int_Used[0]] =
				        word_reverse((A_set_special_sync |
					((ulong) dev_info_hash << 16)));
				}

				/* restart the script at negotiation msg */
				if (rc != REG_FAIL)
				{
			            dev_ptr->flags |= NEG_PHASE_2;
			            rc = psc_write_reg((uint) DSP,
					       (char) DSP_SIZE,
					       word_reverse(
						script_dma_addr +
					       (Ent_renegotiate_sync / 4)));
				}
			        return (rc);
			    }
			    else /* one of target's values is smaller than */
			    {    /* ours (and acceptable). First set SXFER */
			        for (i = 0; i < NUM_700_SYNC_RATES; i++)
				    if (ext_msg_length <= adp_str.xfer_max[i])
				    {
				        dev_ptr->special_xfer_val =
					    (uchar) ((i << 4) |
						 ext_msg_opcode);
				        break;
				    }
			        dev_ptr->agreed_xfer = (uchar) ext_msg_length;
			        dev_ptr->agreed_req_ack = ext_msg_opcode;

				if (SET_SXFER(dev_ptr->special_xfer_val)
				       == REG_FAIL)
				    return (REG_FAIL);

			        /* patch in special interrupt for  */
			        /* sync devices                    */
			        script_vir_addr[R_dummy_int_Used[0] - 1] =
						  word_reverse(0x98080000);
			        /* write in the int op code */
			        script_vir_addr[R_dummy_int_Used[0]] =
				        word_reverse((A_set_special_sync |
					((ulong) dev_info_hash << 16)));

                                /* Next, either echo the message or end */
      			        if (!(dev_ptr->flags & NEG_PHASE_2))
                                {
 				    /* Patch in the negotiation message */
                                    target_script[(Ent_sync_msg_out_buf2 / 4)] 
					 = (0x01030100 | 
						(ulong) ext_msg_length);
                                    target_script[(Ent_sync_msg_out_buf2 / 4) 
				         + 1] = ((ulong) ext_msg_opcode << 24);

                                    dev_ptr->flags |= NEG_PHASE_2;
                                    rc = psc_write_reg((uint) DSP, 
						(char) DSP_SIZE,
                                               word_reverse(script_dma_addr +
                                               (Ent_renegotiate_sync / 4)));
			        }
			        else   /* NEG_PHASE_2 */
			        {
			       	    /* Value is agreed to, stop echoing    */
                            	    dev_ptr->flags &= ~NEG_PHASE_2;
                                    dsp_val = psc_read_reg((uint) DSP,
                                                   (char) DSP_SIZE);
                                    if (dsp_val != REG_FAIL)
				    {
                                        rc = psc_write_reg((uint) DSP, 
					        (char) DSP_SIZE, dsp_val);
				    }
                                    else
                                        rc = REG_FAIL;
				}
				return (rc);
                            } /* end of else clause which handles cases when */
			      /* target's values are less than our defaults. */
			}
                    }
                    else  /* device is negotiating at our defaults */
   		    {
                        /* make sure that the asynch flag is turned off */
                        dev_ptr->async_device = FALSE;

			/* Set values for SXFER */
                     
 			TRACE_1 ("h ext df", 0)
			dev_ptr->agreed_xfer = DEFAULT_MIN_PHASE;
			dev_ptr->agreed_req_ack = DEFAULT_BYTE_BUF;
			if (SET_SXFER(SYNC_VAL))
			    return (REG_FAIL);

			if (dev_ptr->flags & NEG_PHASE_2)
			{
			    dev_ptr->flags &= ~NEG_PHASE_2;
			    dsp_val = psc_read_reg((uint) DSP,
						   (char) DSP_SIZE);
			    if (dsp_val != REG_FAIL)
				rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
						   dsp_val);
			    else
				rc = REG_FAIL;
			}
			else /* echo the SDTR msg using default values */
			{
			    /* Patch negotiation msgs */
			    target_script[(Ent_sync_msg_out_buf2 / 4)] =
				  (0x01030100 | (ulong) DEFAULT_MIN_PHASE);
			    target_script[(Ent_sync_msg_out_buf2 / 4) + 1] =
				  ((ulong) DEFAULT_BYTE_BUF << 24);
			    dev_ptr->flags |= NEG_PHASE_2;
			    rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
					       word_reverse(script_dma_addr +
					       (Ent_renegotiate_sync / 4)));
			}
			return (rc);
		    }
		}
	    }
            /* We will restart the SCRIPT in the code below, reading in */
            /* the rest of the extended message.			*/
	}
	else
	{
	    if (ext_msg_opcode == 0x00)  /* modify data pointer */
	    {
		/******************************************************/
		/* continue execution of script to read in messages   */
		/* telling us about how they want the data pointers   */
		/* modified.  Then, we hit the interrupt we planted   */
		/* ourselves so we can do data pointer modification.  */
		/******************************************************/
		target_script[Ent_ext_msg_patch / 4] =
						  word_reverse(0x98080000);
		target_script[(Ent_ext_msg_patch / 4) + 1] =
		    word_reverse(A_modify_data_ptr |
				 ((ulong) dev_info_hash << 16));
	    }
	    else
	    {
                if (ext_msg_opcode == 0x03)  /* WDTR message */
                {
                    /* go to script that sends msg reject */
                    /* for sync and then continues on     */
                    rc = psc_write_reg((uint) DSP,
                                       (char) DSP_SIZE,
                                       word_reverse(script_dma_addr +
                                             (Ent_reject_target_sync / 4)));
                    return (rc);
		}
		else       /* unknown opcode */
                {
		    rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
		       			word_reverse(script_dma_addr + 
					(Ent_unknown_msg_hdlr / 4)));
		    return (rc);
                }
	    }
        }
	/* restart the chip */
	rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
		word_reverse(script_dma_addr + (Ent_complete_ext_msg / 4)));
    }
    else
	/*******************************************************************/
	/* Being in this portion of the code implies that before, we had   */
	/* received notification of an extended message.  We then deter-   */
	/* mined that the extended message was a modify_data_ptr command.  */
	/* We read in the fourth byte offset and set up an interrupt to    */
	/* ourselves known as A_modify_data_ptr.  Now, we read what the    */
	/* data offset is that was sent by the target device and we add it */
	/* to our dma data pointer within the dev_struct.  Then, we con-   */
	/* tinue on with the script as before.                             */
	/*******************************************************************/
    {
	/* cleanup up patched interrupt.  Make it go  */
	/* back to being a NOP instruction	      */
	target_script[Ent_ext_msg_patch / 4] =
	    word_reverse(0x80000000);
	target_script[(Ent_ext_msg_patch / 4) + 1] =
	    word_reverse(script_dma_addr +
			 Ent_ext_msg_patch);
	data_ptr_offset = target_script[Ent_extended_msg_buf / 4];
	dev_ptr->dma_addr += data_ptr_offset;
	rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
		   word_reverse(script_dma_addr + (Ent_goto_cleanup / 4)));
    }
    return (rc);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_issue_abort_script                                  */
/*                                                                      */
/* FUNCTION:                                                            */
/*      This function will patch in the ABORT message into the message  */
/*      buffer and patch all interrupts within the abort_bdr script     */
/*      to reflect that this was an abort script executing if it ever   */
/*      hits an interrupt because of an error.                          */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered to show that this shared abort/bdr script */
/*              is being used as an abort script this time.             */
/*      *script_dma_addr - the dma address of the script that needs to  */
/*              be fed into the DSP register so that the chip will dma  */
/*              its instructions to itself.                             */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*      connected - indicates whether to try to select the device or    */
/*              whether we are already connected to it and should just  */
/*              try to raise ATN and send the abort message.  We start  */
/*              the script at a different point depending on the value. */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/************************************************************************/
int
psc_issue_abort_script(
		       uint * script_vir_addr,
		       uint * script_dma_addr,
		       struct dev_info * dev_ptr,
		       int dev_info_hash,
		       uchar connected)
{
    ulong   top_half_word, *target_script;
    ulong   id_bit_mask, abort_entry_point;
    int     rc;

    TRACE_2 ("iss abrt", (int) dev_ptr, (int) connected)

    target_script = (ulong *) script_vir_addr;
    top_half_word = ((ulong) dev_info_hash << 16);

    if (!connected)
    {
	id_bit_mask = 0x80000000;
        id_bit_mask |= (0x00060000 | (dev_ptr->lun_id << 24));
	abort_entry_point = Ent_abort_sequence;

        target_script[A_abort_select_failed_Used[0]] =
	    word_reverse(A_abort_select_failed | top_half_word);
    }
    else
    {
	id_bit_mask = 0x06000000;
	abort_entry_point = Ent_abort2_sequence;
    }

    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = id_bit_mask;
    target_script[A_abort_msg_error_Used[0]] =
	word_reverse(A_abort_msg_error | top_half_word);
    target_script[A_abort_io_complete_Used[0]] =
	word_reverse(A_abort_io_complete | top_half_word);

    TRACE_1 ("iss abrt", (int) word_reverse(script_dma_addr +
                                (abort_entry_point / 4)));
    rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
		  word_reverse(script_dma_addr + (abort_entry_point / 4)));
    return (rc);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_issue_bdr_script                                    */
/*                                                                      */
/* FUNCTION:                                                            */
/*      This function will patch in the BDR message into the message    */
/*      buffer and patch all interrupts within the abort_bdr script     */
/*      to reflect that this was a bdr script executing if it ever      */
/*      hits an interrupt because of an error.                          */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered to show that this shared abort/bdr script */
/*              is being used as a bdr script this time.                */
/*      *script_dma_addr - the dma address of the script that needs to  */
/*              be fed into the DSP register so that the chip will dma  */
/*              its instructions to itself.                             */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/************************************************************************/
int
psc_issue_bdr_script(
		     uint * script_vir_addr,
		     uint * script_dma_addr,
		     int dev_info_hash)
{

    ulong   top_half_word, *target_script;
    int     rc;

    target_script = (ulong *) script_vir_addr;
    top_half_word = ((ulong) dev_info_hash << 16);

    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = 0x0C000000;
    /* patch in a bdr interrupt */
    target_script[A_abort_msg_error_Used[0]] =
	word_reverse(A_bdr_msg_error | top_half_word);
    target_script[A_abort_io_complete_Used[0]] =
	word_reverse(A_bdr_io_complete | top_half_word);
    target_script[A_abort_select_failed_Used[0]] =
	word_reverse(A_bdr_select_failed | top_half_word);
    TRACE_1 ("iss bdr", (int) word_reverse(script_dma_addr +
                                (Ent_bdr_sequence / 4)));
    rc = psc_write_reg((uint) DSP, (char) DSP_SIZE,
		    word_reverse(script_dma_addr + (Ent_bdr_sequence / 4)));
    return (rc);
}

#ifdef PSC_TRACE
void
psc_trace_1(char *string, int data)
{
    int     i;

    if (adp_str.current_trace_line > (PSC_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 13; i++)
	adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.one_val.header1[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.one_val.data = data;
    adp_str.current_trace_line++;

    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[15] = '*';
}
#endif

#ifdef PSC_TRACE
void
psc_trace_2(char *string, int val1, int val2)
{
    int     i;

    if (adp_str.current_trace_line > (PSC_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 9; i++)
	adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.two_vals.header2[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.two_vals.data1 = val1;
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.two_vals.data2 = val2;
    adp_str.current_trace_line++;
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	 [adp_str.current_trace_line].un.header[15] = '*';
}
#endif

#ifdef PSC_TRACE
void
psc_trace_3(char *string, int data1, int data2, int data3)
{
    int     i;

    if (adp_str.current_trace_line > (PSC_TRACE_SIZE - 0x10))
    {
	adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 5; i++)
	adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.header3[i] = *(string + i);
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val1 = data1;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val2 = data2;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.three_vals.val3 = data3;
    adp_str.current_trace_line++;
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[0] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[1] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[2] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[3] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[4] = 'P';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[5] = 'O';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[6] = 'I';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[7] = 'N';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[8] = 'T';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[9] = 'E';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[10] = 'R';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[11] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[12] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[13] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[14] = '*';
    adp_str.trace_ptr->trace_buffer
	[adp_str.current_trace_line].un.header[15] = '*';
}
#endif
