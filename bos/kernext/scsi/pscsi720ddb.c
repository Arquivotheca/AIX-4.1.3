static char sccsid[] = "@(#)46	1.17  src/bos/kernext/scsi/pscsi720ddb.c, sysxscsi, bos41J, 9520A_all 5/16/95 12:12:24";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Bottom Half
 *
 * FUNCTIONS:
 * p720_chip_register_init  p720_chip_register_reset p720_config_adapter
 * p720_logerr              p720_read_reg            p720_write_reg
 * p720_read_POS            p720_write_POS           p720_strategy
 * p720_start               p720_alloc_STA
 * p720_unfreeze_qs	    p720_process_q_clr       p720_q_full
 * p720_free_STA            p720_alloc_TAG           
 * p720_alloc_TCE           p720_free_TCE
 * p720_alloc_resources     p720_free_resources      p720_iodone
 * p720_enq_active          p720_enq_wait            p720_enq_WFR
 * p720_enq_abort_bdr       p720_deq_active          p720_deq_wait
 * p720_deq_WFR             p720_deq_abort_bdr       p720_dump
 * p720_dumpstrt            p720_dump_intr           p720_dumpwrt
 * p720_dump_dev            p720_cdt_func            p720_fail_cmd
 * p720_delay               p720_intr                p720_issue_cmd
 * p720_check_wfr_queue     p720_cleanup_reset       p720_scsi_reset_received
 * p720_epow                p720_watchdog            p720_command_reset_scsi_bus
 * p720_verify_tag          p720_fail_free_resources p720_issue_abort_bdr
 * p720_scsi_interrupt      p720_prep_main_script    
 * p720_set_disconnect      p720_verify_neg_answer   p720_reset_iowait_jump
 * p720_restore_iowait_jump p720_update_dataptr      
 * p720_handle_extended_message 		     p720_patch_nondefault_sync
 * p720_issue_abort_script  p720_issue_bdr_script    p720_trace_1
 * p720_trace_2             p720_trace_3
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
/* NAME:        pscsi720ddb.c                                           */
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
#include <sys/pscsi720dd.h>
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
struct  p720_cdt_table    *p720_cdt = NULL;

/* global pointer for adapter structure                                 */
struct    adapter_def    adp_str;

int    adp_str_inited = FALSE;

/* include SCRIPTS-related data structures */
#include "pscsi720bss.h"

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_chip_register_init                                         */
/*                                                                        */
/* FUNCTION:  Initializes adapter chip registers.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called from the process or interrupt level.   */
/*                                                                        */
/* NOTES:  This routine is called to initialize the adapter chip reg-     */
/*         isters to the non-default values we use.                       */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      eaddr - effective address for pio, or NULL                        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      EIO         - pio error                                           */
/*      PSC_NO_ERR  - successful completion			          */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_chip_register_init(caddr_t eaddr)
{
    int old_level, ret_code;
    uchar called_from_top_half;

    ret_code = PSC_NO_ERR;
    called_from_top_half = FALSE;
    if (eaddr == NULL)
    {
        called_from_top_half = TRUE;
        old_level = i_disable(adp_str.ddi.int_prior);
        eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    }

    if (p720_write_reg((uint) SCNTL0, (char) SCNTL0_SIZE,
                       (uchar) SCNTL0_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) SCNTL1, (char) SCNTL1_SIZE,
                       (char) SCNTL1_EXC_OFF, eaddr))
        ret_code = EIO;
 
    else if (p720_write_reg((uint) SCNTL3, (char) SCNTL3_SIZE,
                       (char) SCNTL3_INIT_FAST, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) SIEN0, (char) SIEN0_SIZE, 
			(uchar) SIEN0_MASK, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) SIEN1, (char) SIEN1_SIZE, 
			(uchar) SIEN1_MASK, eaddr))
        ret_code = EIO;

    /* enable response to reselection and OR in chip id */
    else if (p720_write_reg((uint) SCID, (char) SCID_SIZE, 
                         SCID_INIT | adp_str.ddi.card_scsi_id, eaddr))
        ret_code = EIO;

    /* set the SCSI id that the chip responds to when being reselected */
    else if (p720_write_reg((uint) RESPID0, (char) RESPID0_SIZE, 
                       (uchar) (0x01 << adp_str.ddi.card_scsi_id), eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) SXFER, (char) SXFER_SIZE, 
			(uchar) SXFER_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) DMODE, (char) DMODE_SIZE,
                       (uchar) DMODE_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) DIEN, (char) DIEN_SIZE, (uchar) DIEN_MASK,
			eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) DCNTL, (char) DCNTL_SIZE,
                       (uchar) DCNTL_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) CTEST0, (char) CTEST0_SIZE,
                       (uchar) CTEST0_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) STIME0, (char) STIME0_SIZE,
                       (uchar) STIME0_INIT, eaddr))
        ret_code = EIO;

    else if (p720_write_reg((uint) STEST3, (char) STEST3_SIZE,
                       (uchar) STEST3_INIT, eaddr))
        ret_code = EIO;

    if (called_from_top_half)
    {
	BUSIO_DET(eaddr);
        i_enable(old_level);
    }

    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_chip_register_reset                                        */
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
/*      reset_flag - indicates the kind of reset to perform               */
/*      eaddr - effective address for pio                                 */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_chip_register_reset(int reset_flag, caddr_t eaddr)

{

    int     i, rc;
    uint    dma_addr;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_chip_register_reset routine.\n")
    TRACE_1 ("in chipR", reset_flag)
    if (reset_flag & PSC_RESET_CHIP)
    {
        /* First, make sure the chip is stopped by writing to */
        /* the ISTAT register.  This is needed to avoid a bus */
        /* time-out problem with the hardware.                */
        rc = p720_write_reg(ISTAT, ISTAT_SIZE, ABRT, eaddr);
        if (rc != REG_FAIL)
        {
            /* check to see when abort has occurred */
            for (i = 0; i < 30; i++)
            {
                p720_delay(1);
                /* look for DMA interrupt pending */
                rc = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
                if (!(rc & DIP))
                {
                    if (rc & SIP)
                        (void) p720_read_reg(SIST, SIST_SIZE, eaddr);
                    continue;
                }
                /* deassert abort cmd */
                (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr);

                /* read dma status registers to clear them */
                rc = p720_read_reg(DSTAT, DSTAT_SIZE, eaddr);
                if (!(rc & DABRT))
                {
                    (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x80, eaddr);
                    continue;
                }
                break;
            }
        }

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
        p720_config_adapter(eaddr);

        /* Assert a chip reset by writing to the ISTAT register */
        (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x40, eaddr);
        p720_delay(1);
        (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr);

        /* Call routine to re-initialize all chip registers  */
        (void) p720_chip_register_init(eaddr);

        /* re-initialize DMA channel */
        adp_str.channel_id = d_init(adp_str.ddi.dma_lvl,
                                    DMA_INIT, adp_str.ddi.bus_id);
        d_unmask(adp_str.channel_id);
    }

    if (reset_flag & PSC_RESET_DMA)
    {
        /* lock in the dma areas for STA, Table Indirect page, and SCRIPTS */
        dma_addr = adp_str.ddi.tcw_start_addr;
        d_master(adp_str.channel_id, DMA_NOHIDE, 
		 (char *) adp_str.STA[0], (size_t) PAGESIZE,
                 &adp_str.xmem_STA, (char *) dma_addr);

        /* execute d_master on the Table Indirect area */
        dma_addr = (uint) adp_str.IND_TABLE.dma_ptr;
        d_master(adp_str.channel_id, DMA_NOHIDE,
                    (char *) adp_str.IND_TABLE.system_ptr,
                    (size_t) PAGESIZE, &adp_str.xmem_MOV,
                    (caddr_t) dma_addr);

        /* execute d_master on the SCRIPTS area */
        dma_addr = (uint) adp_str.SCRIPTS[0].dma_ptr;
        d_master(adp_str.channel_id, DMA_NOHIDE,
                    (char *) adp_str.SCRIPTS[0].script_ptr, 
		    (size_t) PAGESIZE, &adp_str.xmem_SCR, 
		    (caddr_t) dma_addr);

	/* execute d_master on any scripts besides the iowait script. */
        for (i = 1; i < MAX_SCRIPTS; i++)
        {
            if (adp_str.SCRIPTS[i].script_ptr != NULL)
            {
                dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
                                    adp_str.SCRIPTS[i].TCE_index);
                d_master(adp_str.channel_id, DMA_NOHIDE, (char *)
			 adp_str.SCRIPTS[i].script_ptr, (size_t) PAGESIZE,
                         &adp_str.xmem_SCR, (char *) dma_addr);
            }
        }
    }
    TRACE_1 ("outchipR", 0)
    DEBUG_0 ("Leaving p720_chip_register_reset routine.\n")
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_config_adapter                                     */
/*                                                                      */
/* FUNCTION:    Adapter Configuration Routine                           */
/*                                                                      */
/*      This internal routine performs actions required to make         */
/*      the driver ready for the first open to the adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called from the process level or interrupt  */
/*      level.                                                          */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      Global_adapter_ddi                                              */
/*                                                                      */
/* INPUTS:                                                              */
/*      eaddr - effective address for pio, or NULL                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for unsuccessful operation                    */
/*                                                                      */
/************************************************************************/
int
p720_config_adapter(caddr_t eaddr)
{
    int     ret_code;
    int     IPL_timer_count;
    int     val0, val1;
    int     old_level;
    uchar   int_level, data, called_from_top_half;

    DEBUG_0 ("Entering p720_config_adapter routine.\n")
    ret_code = 0;       /* default to no error */

    /* check here to make sure planer board is ready */
    /* for POS register set-up.                      */
    IPL_timer_count = 0;

    /* loop waiting for either card ready or timeout */

    while (TRUE)
    {   /* loop as long as it takes */
        val0 = p720_read_POS((uint) POS0);
        val1 = p720_read_POS((uint) POS1);
        if ((val0 == -1) || (val1 == -1))
        {       /* permanent I/O error */
            ret_code = EIO;
            break;
        }
        else
        {       /* no I/O error */
            if (((uchar) val0 == POS0_VAL) && ((uchar) val1 == POS1_VAL))
                /* POS0 AND POS1 have both been set correctly */
                break;
            /* card ready for POS reg loading */
            else
            {   /* planer IPL is still going */
                if (IPL_timer_count >= IPL_MAX_SECS)
                {       /* timed-out waiting for planer diag */
                    p720_logerr(ERRID_SCSI_ERR1,
                               UNKNOWN_ADAPTER_ERROR, 10, 0,
                               NULL, FALSE);
                    ret_code = EIO;
                    break;
                }

                /* delay, then come back and check POS0 and   */
                /* POS1 again.                                */
                p720_delay(1);
                IPL_timer_count++;
            }   /* planer IPL is still going */
        }       /* no I/O error */
    }   /* loop as long as it takes */
    if (ret_code == 0)
    {   /* no errors so far, continue to initialize POS registers */
        /* Set up interrupt value for the chip */
        if (adp_str.ddi.int_lvl == 5)
            int_level = 0x80;
        else
            int_level = 0;
        /* We normally enable scsi parity (use 03 value).  This ifdef is */
	/* needed to compensate for a hw problem with very early versions */
 	/* of the planar io chip. */ 
#ifdef P720_DD1
        if (p720_write_POS((uint) POS2, (int_level | 0x01)))
            ret_code = EIO;
#else
        if (p720_write_POS((uint) POS2, (int_level | 0x03)))
            ret_code = EIO;
#endif
    }   /* no errors so far, continue to initialize POS registers */
    if (ret_code == 0)
    {
        data = p720_read_POS((uint) POS4);
        if (data == REG_FAIL)
            ret_code = EIO;
        else
            /* disable scsi fairness to work-around problem with some */
	    /* recent versions of the planar i/o chip that interfaces */
	    /* with the 720. */
            if (p720_write_POS((uint) POS4, ((data & 0xe0) |
                                            adp_str.ddi.dma_lvl)))
                ret_code = EIO;
    }

    called_from_top_half = FALSE;
    /* Clear any pending interrupts that may have been */
    /* left over from ipl. Generate the effective      */
    /* address and disable interrupts if necessary     */
    if (eaddr == NULL)
    {
        called_from_top_half = TRUE;
        old_level = i_disable(adp_str.ddi.int_prior);
        eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    }
        
    if (p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr) == REG_FAIL)
        ret_code = EIO;
    else if (p720_read_reg(DSTAT, DSTAT_SIZE, eaddr) == REG_FAIL)
        ret_code = EIO;
    else if (p720_read_reg(SIST, SIST_SIZE, eaddr) == REG_FAIL)
        ret_code = EIO;

    if (called_from_top_half)
    {
	BUSIO_DET(eaddr);
        i_enable(old_level);
    }

    DEBUG_0 ("Leaving p720_config_adapter routine.\n")
    return (ret_code);

}  /* end p720_config_adapter */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_read_POS                                           */
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
/************************************************************************/
int
p720_read_POS(
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

    DEBUG_1 ("Entering p720_read_POS routine.REG=0x%x\n", offset);

    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
    old_level = i_disable(INTMAX);

    while ((count < PIO_RETRY_COUNT) && ((val1 != val2) || (rc != 0)))
    {   /* bus miscompare, retry loop */
        rc = BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
                       &val1);
        rc |= BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
                        &val2);
        count++;
    }

    if (count >= PIO_RETRY_COUNT)
    {   /* log as unrecoverable I/O bus problem */
        p720_logerr(ERRID_SCSI_ERR1, PIO_RD_DATA_ERR, 35,
                   offset, NULL, FALSE);
        ret_code = -1;
    }
    else if (count > 1)
    /* if we went into the while loop on an error, count will be inc'd */
    {       /* log as recovered I/O bus problem */
        p720_logerr(ERRID_SCSI_ERR2, PIO_RD_DATA_ERR, 40, offset, NULL, FALSE);
        ret_code = val1;    /* no error in bus access */
        DEBUG_1 ("In read_POS, count = 0x%x\n", count);
    }
    else
        ret_code = val1;    /* no error in bus access */

    DEBUG_0 ("Leaving p720_read_POS routine.\n")
    i_enable(old_level);
    IOCC_DET(iocc_addr);
    return (ret_code);
}  /* end p720_read_POS */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_write_POS                                          */
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
/************************************************************************/
int
p720_write_POS(
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

    DEBUG_2 ("Entering p720_write_POS routine.REG=0x%x 0x%x\n", offset, data)
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
    {   /* bus miscompare, retry loop */
        /* load register */
        BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
                  data_byte[1]);
        rc = BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + offset),
                       &val1);
        count++;
    }

    if (count >= PIO_RETRY_COUNT)
    {   /* log as unrecoverable I/O bus problem */
        p720_logerr(ERRID_SCSI_ERR1, PIO_WR_DATA_ERR, 45,
                   offset, NULL, FALSE);
        ret_code = -1;
    }
    else
        if (count > 0)
            /* if we went into the while loop on an error, count will be
               inc'd */
        {       /* log as recovered I/O bus problem */
            p720_logerr(ERRID_SCSI_ERR2, PIO_WR_DATA_ERR, 50,
                       offset, NULL, FALSE);
            DEBUG_1 ("In write_POS, count = 0x%x\n", count);
        }

    DEBUG_0 ("Leaving p720_write_POS routine.\n")
    i_enable(old_level);
    IOCC_DET(iocc_addr);
    return (ret_code);

}  /* end p720_write_POS */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_write_reg_disable                                  */
/*                                                                      */
/* FUNCTION:    Pinned routine to disable interrutps and call write_reg */
/*                                                                      */
/*      This routine provides an interface to the p720_write_reg        */
/*      routine for functions not running at the adapter's interrupt    */
/*      level.                                                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is intended to be called when interrupts are not disabled, */
/*      or are at a less-favored level than the adapter's level.        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected POS reg                        */
/*      reg_size - size of register in bytes                            */
/*      data    - value to be loaded                                    */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values,      */
/*      which are simply a pass-through of the return from write_reg.   */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/************************************************************************/
int
p720_write_reg_disable(
              uint offset,
              char reg_size,
              int data)
{
    int old_level, ret_code;
    caddr_t eaddr;

    old_level = i_disable(adp_str.ddi.int_prior);
    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    ret_code = p720_write_reg(offset, reg_size, data, eaddr);
    BUSIO_DET(eaddr);
    i_enable(old_level);
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_logerr                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Error Logging Routine                    */
/*                                                                      */
/*      This routine provides a common point through which all driver   */
/*      error logging passes.                                           */
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
/*      com_ptr - pointer to the command structure most likely          */
/*		  associated with the error				*/
/*      read_regs - flag indicating whether to read chip registers      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_logerr(
           int errid,
           int add_hw_status,
           int errnum,
           int data1,
           struct cmd_info * com_ptr,
           uchar read_regs)
{
    struct error_log_def log;
    caddr_t iocc_addr;
    int     cmd_length, cmd_filler;

    DEBUG_0 ("Entering p720_logerr routine.\n")
    if (adp_str.errlog_enable)
    {   /* set up info and log */
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
                   POS4), (char *) &log.data.un.card2.pos4_val);
        BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) +
                   POS5), (char *) &log.data.un.card2.pos5_val);
        IOCC_DET(iocc_addr);

        if (read_regs)
        {       /* on an interrupt level, read these regs */
            iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
            BUS_GETCX(iocc_addr + ISTAT, (char *) &log.data.un.card2.istat_val);
            BUS_GETCX(iocc_addr + SSTAT0, (char *) &log.data.un.card2.sstat0_val);
            BUS_GETCX(iocc_addr + SSTAT1, (char *) &log.data.un.card2.sstat1_val);
            BUS_GETCX(iocc_addr + SSTAT2, (char *) &log.data.un.card2.sstat2_val);
            BUS_GETCX(iocc_addr + SDID, (char *) &log.data.un.card2.sdid_val);
            BUS_GETCX(iocc_addr + SCID, (char *) &log.data.un.card2.scid_val);
            BUS_GETCX(iocc_addr + CTEST3, (char *) &log.data.un.card2.ctest3_val);

            /* The following lines are commented since a direct read of these */
	    /* registers will clear pending interrupts.  To log the contents  */
	    /* of these 3 registers, we would have to pass their values in.   */
	    /* This is a possible future enhancement.                         */
/*
            BUS_GETCX(iocc_addr + DSTAT, (char *) &log.data.un.card2.dstat_val);
            BUS_GETCX(iocc_addr + SIST0, (char *) &log.data.un.card2.sist0_val);
            BUS_GETCX(iocc_addr + SIST1, (char *) &log.data.un.card2.sist1_val);
*/
            BUS_GETCX(iocc_addr + SSID, (char *) &log.data.un.card2.ssid_val);
            BUS_GETCX(iocc_addr + SXFER, (char *) &log.data.un.card2.sxfer_val);
            BUS_GETCX(iocc_addr + SCNTL2, (char *) &log.data.un.card2.scntl2_val);
            BUS_GETCX(iocc_addr + SCNTL3, (char *) &log.data.un.card2.scntl3_val);
            BUS_GETCX(iocc_addr + GPREG, (char *) &log.data.un.card2.gpreg_val);
            BUS_GETCX(iocc_addr + SCNTL1, (char *) &log.data.un.card2.scntl1_val);
            BUS_GETCX(iocc_addr + SBCL, (char *) &log.data.un.card2.sbcl_val);
            BUS_GETCX(iocc_addr + SFBR, (char *) &log.data.un.card2.sfbr_val);
            BUS_GETLX((long *) (iocc_addr + SCRATCHA), 
		   (long *) &log.data.un.card2.scratcha_val);
            BUS_GETLX((long *) (iocc_addr + DBC),
                      (long *) &log.data.un.card2.dbc_val);
            BUS_GETLX((long *) (iocc_addr + DSP),
                      (long *) &log.data.un.card2.dsp_val);
            BUS_GETLX((long *) (iocc_addr + DSPS),
                      (long *) &log.data.un.card2.dsps_val);
            BUSIO_DET(iocc_addr);

            log.data.un.card2.scratcha_val = word_reverse(
                                           log.data.un.card2.scratcha_val);
            log.data.un.card2.dbc_val = word_reverse(
                                           log.data.un.card2.dbc_val);
            log.data.un.card2.dsp_val = word_reverse(
                                           log.data.un.card2.dsp_val);
            log.data.un.card2.dsps_val = word_reverse(
                                           log.data.un.card2.dsps_val);
            
            if (com_ptr != NULL) 
 	    {
		if (com_ptr->bp != NULL)
	        {
                    cmd_length = (int) com_ptr->bp->scsi_command.scsi_length;
                    log.data.un.card2.scsi_cmd[0] = com_ptr->bp->
				scsi_command.scsi_cmd.scsi_op_code;
                    log.data.un.card2.scsi_cmd[1] = com_ptr->bp->
				scsi_command.scsi_cmd.lun;
                    for (cmd_filler = 2; cmd_filler < cmd_length; cmd_filler++)
                        log.data.un.card2.scsi_cmd[cmd_filler] =
                                com_ptr->bp->scsi_command.scsi_cmd.scsi_bytes
				[cmd_filler - 2];
	   	}
		if (com_ptr->device != NULL)
		{
                    log.data.un.card2.target_id = com_ptr->device->scsi_id;
                    log.data.un.card2.queue_state = 
					    com_ptr->device->queue_state;
                    log.data.un.card2.cmd_activity_state =
                                            com_ptr->device->device_state;
                    log.data.un.card2.resource_state = 
					    com_ptr->resource_state;
		}
            }
        }

        /* log the error here */
        errsave(&log, sizeof(struct error_log_def));
    }
    DEBUG_0 ("Leaving p720_logerr routine.\n")

}  /* end p720_logerr */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_read_reg                                           */
/*                                                                      */
/* FUNCTION:    Access the specified register.                          */
/*                                                                      */
/*      This routine reads from a selected adapter                      */
/*      register and returns the data, performing                       */
/*      appropriate error detection and recovery.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled, to read a single register; 8,24 or 32 bit  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected register                       */
/*      reg_size - size of register in bytes                            */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 32-bits of data if good or recovered error        */
/*                  = 32-bits of -1 if permanent I/O error encountered  */
/*                                                                      */
/************************************************************************/
int
p720_read_reg(
             uint offset,
             char reg_size,
	     caddr_t eaddr)
{
    int     ret_code, pio_rc, retry_count;
    int     val1;
    short   vals1;
    char    valc1;

    DEBUG_1 ("Entering p720_read_reg routine.REG=0x%x\n", offset);

    ASSERT (eaddr != NULL);
    if (reg_size == 1)
        pio_rc = BUS_GETCX((eaddr + offset), &valc1);
    else if (reg_size != 2)
        pio_rc = BUS_GETLX((long *)(eaddr + offset), (long *) &val1);
    else
        pio_rc = BUS_GETSX((short *)(eaddr + offset), (short *) &vals1);

    retry_count = 0;
    while ((retry_count < PIO_RETRY_COUNT) && (pio_rc != 0))
    {
        if ((offset == DSTAT) ||        /* Don't retry these registers */
            (offset == SIST0) ||        /* because rereading them corrupts */
            (offset == SIST1) ||        /* the value originally in them. */
            (offset == SIST))           /* We will just log the error. */
            break;
        else
            retry_count++;
        if (reg_size == 1)
            pio_rc = BUS_GETCX((eaddr + offset), &valc1);
        else if (reg_size != 2)
            pio_rc = BUS_GETLX((long *)(eaddr + offset), (long *) &val1);
        else
            pio_rc = BUS_GETSX((short *)(eaddr + offset), (short *) &vals1);
    }

    if (pio_rc != 0)
    {   
        struct pio_except infop;

	ASSERT(pio_rc == EXCEPT_IO);
	getexcept((struct except *) &infop);
        /* log as unrecoverable I/O bus problem */
        if ((infop.pio_csr & PIO_EXCP_MASK) == PIO_EXCP_DPRTY)
            p720_logerr(ERRID_SCSI_ERR1, PIO_RD_DATA_ERR, 15,
                       offset, NULL, FALSE);
        else
            p720_logerr(ERRID_SCSI_ERR1, PIO_RD_OTHR_ERR, 15,
                       offset, NULL, FALSE);
        ret_code = REG_FAIL;
    }
    else
    {
        if (retry_count > 0)
        {
            p720_logerr(ERRID_SCSI_ERR2, PIO_RD_OTHR_ERR, 20,
                       offset, NULL, FALSE);
    	    TRACE_1 ("read-reg err", offset)
        }
        if (reg_size == 1)
            ret_code = valc1;
        else if (reg_size == 4)
            ret_code = val1;
        else if (reg_size == 2)
            ret_code = vals1;
        else        /* reg size = 3 */
            ret_code = (val1 >> 8) & 0x00ffffff;
    }
    DEBUG_0 ("Leaving p720_read_reg routine.\n")

    return (ret_code);
}  /* end p720_read_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_write_reg                                          */
/*                                                                      */
/* FUNCTION:    Store passed data in specified register.                */
/*                                                                      */
/*      This routine loads a selected adapter register with the         */
/*      passed data value, performing appropriate error detection       */
/*      and recovery.                                                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      offset  - offset to the selected POS reg                        */
/*      reg_size - size of register in bytes                            */
/*      data    - value to be loaded                                    */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/************************************************************************/
int
p720_write_reg(
              uint offset,
              char reg_size,
              int data, 
	      caddr_t eaddr)
{
    int     ret_code, pio_rc, retry_count;
     
    DEBUG_2 ("Entering p720_write_reg routine.REG=0x%x 0x%x\n", offset, data)

    ASSERT(eaddr != NULL);

    if (reg_size == 1)
        pio_rc = BUS_PUTCX((eaddr + offset), (char) data);
    else if (reg_size != 2)
        pio_rc = BUS_PUTLX((long *)(eaddr + offset), data);
    else
        pio_rc = BUS_PUTSX((short *)(eaddr + offset), (ushort) data);

    retry_count = 0;
    while ((retry_count < PIO_RETRY_COUNT) && (pio_rc != 0))
    {
        retry_count++;
        if (reg_size == 1)
            pio_rc = BUS_PUTCX((eaddr + offset), (char) data);
        else if (reg_size != 2)
            pio_rc = BUS_PUTLX((long *)(eaddr + offset), data);
        else
            pio_rc = BUS_PUTSX((short *)(eaddr + offset), (ushort) data);
    }
    ret_code = 0;
    if (pio_rc != 0)
    {   
        struct pio_except infop;

	ASSERT(pio_rc == EXCEPT_IO);
	getexcept((struct except *) &infop);
        /* log as unrecoverable I/O bus problem */
        if ((infop.pio_csr & PIO_EXCP_MASK) == PIO_EXCP_DPRTY)
            p720_logerr(ERRID_SCSI_ERR1, PIO_WR_DATA_ERR,
                       25, offset, NULL, FALSE);
        else
            p720_logerr(ERRID_SCSI_ERR1, PIO_WR_OTHR_ERR,
                       25, offset, NULL, FALSE);

        ret_code = REG_FAIL;
    }
    else if (retry_count > 0)
    {
        p720_logerr(ERRID_SCSI_ERR2, PIO_WR_OTHR_ERR, 30,
                   offset, NULL, FALSE);
    }
    DEBUG_0 ("Leaving p720_write_reg routine.\n")
    return (ret_code);

}  /* end p720_write_reg */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_strategy                                           */
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
/************************************************************************/
int
p720_strategy(
             struct sc_buf * bp)
{
    int     ret_code;
    dev_t   devno;
    int     dev_index;
    int     new_pri;
    struct dev_info *dev_ptr;

    DEBUG_0 ("Entering p720_strategy routine \n")

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

        /* Disable interrupt to keep this single threaded. */
        new_pri = i_disable(adp_str.ddi.int_prior);
        dev_ptr = adp_str.device_queue_hash[dev_index];
        TRACE_2 ("in strat", (int) bp, (int) dev_ptr)

        /* miscellaneous validation of request */
	/* a check that if we are stopping, we will only accept commands */
	/* to get through a contingent allegiance condition */
        if ((dev_ptr != NULL) &&
            (bp->bufstruct.b_bcount <= adp_str.max_request)
            && ((dev_ptr->queue_state & STOPPING_MASK) != STOPPING)
            && (bp->resvd1 == 0x0))
        {
	    TRACE_1 ("qstate", dev_ptr->queue_state)
            /* passed check of device structure info */

	    /* ACTIVE and WAIT_FLUSH case */
	    if (dev_ptr->queue_state & ACTIVE_or_WAIT_FLUSH)
	    {
                /* if we get here, device is not halted anymore */
                /* set normal state */
                bp->bufstruct.av_forw = NULL;   /* only accept one cmd */
                bp->bufstruct.b_work = 0;
                /* Initialize the following flag to the "no */
                /* error" state.                            */
                bp->bufstruct.b_error = 0;
                bp->bufstruct.b_flags &= ~B_ERROR;
                bp->bufstruct.b_resid = 0;
                bp->status_validity = 0;
                bp->general_card_status = 0;
                bp->scsi_status = 0;
		bp->resvd7 = FALSE;  	    /* not expedited */

                /* Queue this command to the device's pending      */
                /* queue for processing by p720_start.             */
                p720_start(bp, dev_ptr);
	    }
	    /* case HALTED: */
            else if (dev_ptr->queue_state & HALTED)
                /* The HALTED flag indicates all the queue was    */
                /* flushed when we informed the head of a prior   */
                /* error.  Do not continue the processing if the  */
                /* caller has not set the SC_RESUME flage.        */
	    {
                if (bp->flags & SC_RESUME)
                {
                    /* initialize bp */
                    bp->bufstruct.av_forw = NULL;   /* only accept one cmd */
                    bp->bufstruct.b_work = 0;
                    /* Initialize the following flag to the "no */
                    /* error" state.                            */
                    bp->bufstruct.b_error = 0;
                    bp->bufstruct.b_flags &= ~B_ERROR;
                    bp->bufstruct.b_resid = 0;
                    bp->status_validity = 0;
                    bp->general_card_status = 0;
                    bp->scsi_status = 0;
		    bp->resvd7 = FALSE;  	    /* not expedited */
                    dev_ptr->queue_state = ACTIVE;
                    p720_start(bp, dev_ptr);
                }
                else
                {   /* Check of queue_state and RESUME flag failed */
                    /* iodone B_ERROR and ENXIO */
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    bp->status_validity = 0;
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
                }
            }
            else if (dev_ptr->queue_state & HALTED_CAC)
	    /* The HALTED_CAC condition indicates the driver is in 
	     * the initial state of processing a contingent allegiance 
	     * condition for a cmd. queueing target.  The queues for
	     * this target lun are frozen pending action by the head
	     * device driver.  First, we require a SC_RESUME.  With the
	     * SC_RESUME, we will either expedite an untagged SCSI cmd
	     * to the target, or process a SC_Q_CLR.
	     */
	    {
		if (bp->flags & SC_RESUME)
		{
		    if (bp->flags & SC_Q_CLR)
		    {
		        p720_process_q_clr(dev_ptr, bp);
		    }
		    else
		    {
			/* initialize the bp */
                        bp->bufstruct.av_forw = NULL;   /* accept one cmd */
                        bp->bufstruct.b_work = 0;
                        bp->bufstruct.b_error = 0;      /* no errors */
                        bp->bufstruct.b_flags &= ~B_ERROR;
                        bp->bufstruct.b_resid = 0;
                        bp->status_validity = 0;
                        bp->general_card_status = 0;
                        bp->scsi_status = 0;

			/* mark the bp as being expedited */
			bp->resvd7 = TRUE;
		        
			/* in this state, can't send a tagged cmd */
			if (bp->q_tag_msg != SC_NO_Q)
			{
                            bp->bufstruct.b_error = ENXIO;
                    	    bp->bufstruct.b_flags |= B_ERROR;
                    	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    	    iodone((struct buf *) bp);
                    	    ret_code = ENXIO;
			}
			else 
			{
			    /* change state to wait_info by leaving the */
			    /* value of the STOPPING bit alone, turning */
			    /* all others off except the WAIT_INFO bit. */
			    if (dev_ptr->queue_state & STOPPING)
			        dev_ptr->queue_state = WAIT_INFO | STOPPING;
			    else
			        dev_ptr->queue_state = WAIT_INFO;

			    p720_start(bp, dev_ptr);
			}
	 	    }	
		} /* end if SC_RESUME while in HALTED_CAC */
		else   /* check of queue_state &  SC_RESUME failed */
		{
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
		}
	    }
	    else if (dev_ptr->queue_state & WAIT_INFO)
	    /* WAIT_INFO case:
	     * The driver is waiting for a command to process the queue
	     * during error recovery of a CAC:  either Q_CLR to clear all
	     * the remaining commands on the queue, or Q_RESUME, to restart
	     * the queue, keeping all commands active.  A command may be
	     * present in the sc_buf in the Q_RESUME case.
	     */
	    {
		if (bp->flags & SC_Q_CLR)
		{
		    /* the queues must be cleared and the bp iodoned */
		    p720_process_q_clr(dev_ptr, bp);
		}
		else if (bp->flags & SC_Q_RESUME)
		{
		    /* the queues have to be resumed */

    	 	    /* adjust the queue state, the CAC processing is finished */ 
    		    if (dev_ptr->queue_state & STOPPING)
			dev_ptr->queue_state = ACTIVE | STOPPING;
    		    else
			dev_ptr->queue_state = ACTIVE;

		    /* check for a command associated with the sc_buf */
		    if (bp->scsi_command.scsi_length != 0)
		    {
                        /* initialize bp */
                        bp->bufstruct.av_forw = NULL;   /* only accept 1 cmd */
                        bp->bufstruct.b_work = 0;
                        /* Initialize the following flag to the "no */
                        /* error" state.                            */
                        bp->bufstruct.b_error = 0;
                        bp->bufstruct.b_flags &= ~B_ERROR;
                        bp->bufstruct.b_resid = 0;
                        bp->status_validity = 0;
                        bp->general_card_status = 0;
                        bp->scsi_status = 0;
                        bp->resvd7 = FALSE;         /* not expedited */
			(void) p720_unfreeze_qs(dev_ptr, bp, FALSE, NULL);
		    }
		    else
		    {
			(void) p720_unfreeze_qs(dev_ptr, NULL, TRUE, NULL);
                        bp->bufstruct.b_error = 0;      /* no errors */
                        bp->bufstruct.b_flags &= ~B_ERROR;
                        iodone((struct buf *) bp);
		    }
		}
		else 
		{
		    /* the bp->flags value is inconsistent w/ queue_state */
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
		}
            }
	    else    /* we should never get here */
	    {
                    bp->bufstruct.b_error = ENXIO;
                    bp->bufstruct.b_flags |= B_ERROR;
                    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                    bp->status_validity = 0;
                    iodone((struct buf *) bp);
                    ret_code = ENXIO;
		    TRACE_1("default", 0)
	    }

        }
        else
        {       /* device structure validation failed */
	    TRACE_1("valid fail", 0)
            bp->bufstruct.b_error = EIO;
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->status_validity = SC_ADAPTER_ERROR;
            bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
            iodone((struct buf *) bp);
            ret_code = EIO;
        }

        TRACE_1 ("out strat", ret_code)
        i_enable(new_pri);
    }
    else
    {   /* scsi chip has not been initialized or opened */
        bp->bufstruct.b_error = EIO;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
        bp->status_validity = SC_ADAPTER_ERROR;
        bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
        iodone((struct buf *) bp);
        ret_code = EIO;
    }

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, devno);
    DEBUG_0 ("Leaving p720_strategy routine \n")
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_start                                              */
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
/************************************************************************/
void
p720_start(
	  struct sc_buf *bp,
          struct dev_info * dev_ptr)
{
    int     tag, i;
    uint  * target_script;
    struct scsi_id_info *sid_ptr;
    uchar   chk_disconnect;
    caddr_t eaddr;

    DEBUG_0 ("Entering p720_start routine.\n")
    TRACE_1 ("in strt ", (int) dev_ptr)
    sid_ptr = dev_ptr->sid_ptr;

    /* if NOT (epow or
     *    restart_in_progress and bp->flags says to delay the cmd or
     *	  flushing or starving or blocked and not expedited cmd)
     * then process the bp
     * else temporarily put it aside
     */
    if ((adp_str.epow_state != EPOW_PENDING) &&
	(!(sid_ptr->restart_in_prog && (bp->flags & SC_DELAY_CMD))) &&
	((!(dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED)) || bp->resvd7))
    {   
        /* Attempt to allocate the resource that are  */
        /* needed. i.e. TCE's, small xfer area, etc.  */
        tag = p720_alloc_resources(bp, dev_ptr);
        TRACE_3("strt", (int) tag, (int) bp, (int) bp->bufstruct.b_un.b_addr)
        TRACE_3("strt", (int) bp->bufstruct.b_blkno, (int) bp->bufstruct.b_bcount,
                (int) bp->scsi_command.scsi_cmd.scsi_op_code)
	if (tag <= PSC_ALLOC_FAILED)
        {  /* Resources cannot be allocated */
           p720_enq_WFR(bp);
	   dev_ptr->flags |= BLOCKED;
           return;
        }

       if (adp_str.DEVICE_ACTIVE_head == NULL)
       {   /* There's no commands currently active */
 	   /* Write system trace to indicate start of command */
	   /* to adapter */
       	   DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, adp_str.devno,
          	bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
            	bp->bufstruct.b_bcount);

	   /* get an effective address for pio */
	   eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

           if (!(sid_ptr->negotiate_flag))
           {   /* Negotiation not required, SCRIPT already patched */
               /* Call routine to start command at the chip */
               chk_disconnect = bp->scsi_command.flags & SC_NODISC;
               if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                   p720_set_disconnect(dev_ptr, chk_disconnect);
               if ((dev_ptr->sid_ptr->tag_flag != tag) || 
                   (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
	           (dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                   p720_patch_tag_changes(&adp_str.command[tag], 
			bp->q_tag_msg);
               p720_prep_main_script(&adp_str.command[tag],
                  (uint *) adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
		script_ptr, dev_ptr->sid_ptr->dma_script_ptr);
               ISSUE_MAIN_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, eaddr);
               dev_ptr->device_state = CMD_IN_PROGRESS;
           }
           else if (bp->scsi_command.flags & SC_ASYNC)
           {   /* Negotiation not required, known ASYNC.   */
               /* Patch the SCRIPT for async transfers.    */
	        target_script = (uint *) adp_str.SCRIPTS
		[dev_ptr->sid_ptr->script_index].script_ptr;
 		TRACE_1 ("start: async", 0)
                for (i=0; i<4; i++)
                {
                    target_script[R_sxfer_patch_Used[i]] =
                            word_reverse(SXFER_ASYNC_MOVE);
                    target_script[R_scntl1_patch_Used[i]] =
                            word_reverse(SCNTL1_EOFF_MOVE);
                    target_script[R_scntl3_patch_Used[i]] =
                            word_reverse(SCNTL3_SLOW_MOVE);
		}

                /* Call routine to start command at the chip */
                chk_disconnect = bp->scsi_command.flags & SC_NODISC;

                if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                    p720_set_disconnect(dev_ptr, chk_disconnect);

                if ((dev_ptr->sid_ptr->tag_flag != tag) || 
                    (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
	               	(dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                    p720_patch_tag_changes(&adp_str.command[tag], 
                    	bp->q_tag_msg);

                p720_prep_main_script(&adp_str.command[tag], (uint *)
		    adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr,
                            dev_ptr->sid_ptr->dma_script_ptr);

                ISSUE_MAIN_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, eaddr);
                dev_ptr->device_state = CMD_IN_PROGRESS;
                sid_ptr->async_device = TRUE;
                sid_ptr->negotiate_flag = FALSE;
            }
            else        /* Need to issue negotiate command */
            {
                /* Call routine to start negotiate at the chip */
                chk_disconnect = bp->scsi_command.flags & SC_NODISC;
                if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) 
			!= 0)
                    p720_set_disconnect(dev_ptr, chk_disconnect);

                if ((dev_ptr->sid_ptr->tag_flag != tag) || 
                    (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
	               	(dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                    p720_patch_tag_changes(&adp_str.command[tag], 
                    	bp->q_tag_msg);
                ISSUE_NEGOTIATE_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, 
					  eaddr);
                dev_ptr->device_state = NEGOTIATE_IN_PROGRESS;
            }

            BUSIO_DET(eaddr);

            /* Start the watchdog timer for this command. We know it is the */
	    /* only active command for the target id/lun, since there are   */
	    /* no active commands for the entire adapter. */
            if (bp->timeout_value == 0)
                dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
            else
                dev_ptr->dev_watchdog.dog.restart = (ulong)bp->timeout_value+1;
            w_start(&(dev_ptr->dev_watchdog.dog));

            /* Add to the DEVICE ACTIVE QUEUE and command active queue */
            p720_enq_active(dev_ptr, &adp_str.command[tag]);
            DEBUG_0 ("Issuing command\n")
            DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, adp_str.devno);
	    TRACE_1("started cmd", (int) &adp_str.command[tag])
        }
        else if (adp_str.DEVICE_WAITING_head == NULL)
        {
            /* Add to the DEVICE WAITING QUEUE */

	    /* get an effective address for pio */
	    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

            /* Use sigp try to signal the chip that a  */
            /* command is waiting.                     */
	    TRACE_1 ("sigp start ", (int) &adp_str.command[tag])
	    if (p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr))
	    {
		/* a failure by write_reg means we were unsuccessful */
		/* after multiple pio attempts.  Something is very   */
		/* wrong.  Mark the current bp, then call            */
		/* p720_cleanup_reset to reset the chip and bus, and */
		/* to return this and any other bps. */
                bp->bufstruct.b_error = EIO;
                bp->bufstruct.b_flags |= B_ERROR;
                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                bp->status_validity = SC_ADAPTER_ERROR;
                bp->general_card_status = SC_ADAPTER_HDW_FAILURE;
                p720_enq_wait(&adp_str.command[tag]);
                p720_cleanup_reset(REG_ERR, eaddr);
	        BUSIO_DET(eaddr);
		TRACE_1 ("sigp fail", (int) bp)
                return;
            }
            else
            {
  		BUSIO_DET(eaddr);
                p720_enq_wait(&adp_str.command[tag]);
            }
        }
        else
        {   /* There's other device waiting, so add to queue */
            /* Add to the DEVICE WAITING QUEUE */
            p720_enq_wait(&adp_str.command[tag]);
            DEBUG_0 ("Other device waiting\n")
        }
    }
    else
    {   /* Don't want to try to allocate resources */
        /* Add to the BP SAVE QUEUE */
	TRACE_1 ("not trying", (int) dev_ptr)
#ifdef P720_TRACE
	if (sid_ptr->restart_in_prog && (bp->flags & SC_DELAY_CMD))
	    TRACE_1("restarting", (int) sid_ptr)
	if ((dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED) && !bp->resvd7)
	    TRACE_2("flags", dev_ptr->flags, bp->resvd7)
#endif
	if (dev_ptr->bp_save_tail == NULL)
	    dev_ptr->bp_save_head = bp;
	else
	    dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;
	dev_ptr->bp_save_tail = bp;
        TRACE_1 ("bp_save", (int) dev_ptr->bp_save_head)
    }
    TRACE_1 ("out strt", (int) dev_ptr)
    DEBUG_0 ("Leaving p720_start routine.\n")
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_alloc_STA                                                  */
/*                                                                        */
/* FUNCTION:  Allocates a small transfer area for a small data xfer.      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine assumes interrupts are disabled.                     */
/*                                                                        */
/* NOTES:  This routine is called when allocating resources for a command.*/
/*         This routine obtains a transfer area of size 256 bytes from    */
/*         the memory page this dd has set aside for data transfers in-   */
/*         volving small amounts.                                         */
/*         Assuming there are no more than 32 STA's.                      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    com_ptr - pointer to command structure        			  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    index  - index in STA page (able to allocate STA).                  */
/*    PSC_NO_STA_FREE - Unable to allocate STA.                           */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_alloc_STA(
              struct cmd_info * com_ptr)
{

    int index;

    DEBUG_0 ("Entering p720_alloc_STA routine.\n")
    /* check if an STA is free */
    index = p720_getfree (adp_str.STA_alloc[0]);
    if (index < NUM_STA)
    {   /* one is free - allocate it for this command */
	com_ptr->resource_index = index;
	com_ptr->STA_addr = (uint) adp_str.STA[index];
	com_ptr->resource_state = STA_RESOURCES_USED;
	ALLOC(adp_str.STA_alloc[0], index)
	DEBUG_0 ("Leaving p720_alloc_STA routine.\n")
        return (index);
    }

    DEBUG_0 ("Leaving p720_alloc_STA routine.\n")
    return (PSC_NO_STA_FREE);     /* No STA's availible at this time */

}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_free_STA                                                   */
/*                                                                        */
/* FUNCTION:  Frees a small transfer area.                                */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled when this routine is called.        */
/*                                                                        */
/* NOTES:  This routine frees a small transfer area for later use.        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    com_ptr - pointer to command structure        			  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_free_STA(
             struct cmd_info * com_ptr)
{
    DEBUG_0 ("Entering p720_free_STA routine.\n")
    FREE(adp_str.STA_alloc[0], com_ptr->resource_index)
    com_ptr->resource_index = 0;
    com_ptr->STA_addr = 0;
    DEBUG_0 ("Leaving p720_free_STA routine.\n")
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_alloc_TAG                                                  */
/*                                                                        */
/* FUNCTION:  Allocates a command tag					  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* NOTES:  This routine allocates a command tag. 	                  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd_info structure - pointer to command information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    TRUE  - Able to allocate TAG.                                       */
/*    FALSE - Unable to allocate TAG.                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_alloc_TAG(
   		struct sc_buf * bp)
{
  int i, index, tag;

  tag = PSC_NO_TAG_FREE;

  /* Check if it is a command with command queuing or not */
  if ( bp->q_tag_msg != SC_NO_Q )
  {  /* Command queuing, so allocate from pool */
      for (i = START_Q_TAGS; i < TAG_TABLE_SIZE; i++)
      {
	  index = p720_getfree (adp_str.TAG_alloc[i]);
     	  if (index < PSC_WORDSIZE)
     	  {  /* a free tag was found - so allocate it */
		ALLOC(adp_str.TAG_alloc[i], index)
        	tag = i * PSC_WORDSIZE + index;
        	TRACE_1 ("Tag Q!", (int) tag)
		adp_str.command[tag].in_use = TRUE;
	        break;
     	  }
      }
  }
  else
  {  /* No command queuing so allocate the dedicated command struct */
     index = INDEX(bp->scsi_command.scsi_id,
                       (bp->scsi_command.scsi_cmd.lun) >> 5);
     if (!IN_USE(adp_str.TAG_alloc[index / PSC_WORDSIZE], 
		index % PSC_WORDSIZE)) 
     {
	ALLOC(adp_str.TAG_alloc[index / PSC_WORDSIZE], index % PSC_WORDSIZE)
	tag = index;
	adp_str.command[tag].in_use = TRUE;
        TRACE_1 ("Tag NQ!", (int) tag)
     }
  }
#ifdef P720_TRACE
  if (tag == PSC_NO_TAG_FREE)
	TRACE_1("No Tag free", (int) bp->q_tag_msg)
#endif

  DEBUG_0 ("Leaving p720_alloc_TAG routine.\n")
  return (tag);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_alloc_TCE                                                  */
/*                                                                        */
/* FUNCTION:  Allocate TCEs for data transfer.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* NOTES:  This routine is called when allocating resources for a command.*/
/*         Resources consist of either a single TCE allocated from the    */
/*         small TCE area or more TCEs which are allocated from the large */
/*         TCE area.                                                      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    sc_buf  - pointer to scsi structure from caller                     */
/*    com_ptr - pointer to command structure                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    index - Able to allocate TCE(s), returns bit index.                 */
/*    PSC_NO_TCE_FREE - Unable to allocate TCE(s).      		  */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_alloc_TCE(
              struct sc_buf * bp,
              struct cmd_info * com_ptr)
{
    int     cur_bit, cur_word;
    int     index, index1;
    int     keep_looking = TRUE;
    int     mask, free_blk_len_mask;
    int     tmp_table;
    int     baddr, num_TCE, num_large_TCE, i, j;

    DEBUG_0 ("Entering p720_alloc_TCE routine.\n")

    /* Compute the number of TCEs required using the  */
    /* buffer address and byte count from the scsi    */
    /* command.                                       */
    baddr = (int) bp->bufstruct.b_un.b_addr;
    num_TCE = (((baddr & (PAGESIZE - 1)) + bp->bufstruct.b_bcount - 1) /
               PAGESIZE) + 1;
    DEBUG_1 ("Number of TCEs = %d\n", num_TCE)
    if (num_TCE == 1)
    {   /* A TCE is allocated from the small TCE area */
	for (i=0 ; i < TCE_TABLE_SIZE/PSC_WORDSIZE; i++)
	{
	    if ( (index = p720_getfree (adp_str.TCE_alloc[i])) < PSC_WORDSIZE)
            { /* a free TCE was found - allocate it */
		index1 = i * PSC_WORDSIZE + index;  /* which bit in array */

                com_ptr->dma_addr =
                    (uint) DMA_ADDR(adp_str.ddi.tcw_start_addr, index1) +
                    (uint) (baddr & (PAGESIZE - 1));
                com_ptr->resource_index = index1;
                com_ptr->TCE_count = 1;

                ALLOC(adp_str.TCE_alloc[i], index)
                com_ptr->resource_state = SMALL_TCE_RESOURCES_USED;
                DEBUG_0 ("Leaving p720_alloc_TCE routine.\n")
                TRACE_3 ("sTCE", com_ptr->resource_index, com_ptr->TCE_count, 
			index1)
                return (index1);      /* TCE allocated */
            }
	}
    }

    /* The TCEs are allocated from the large TCE area */
    /* Note that this allows spill-over into the large area when the small */
    /* area becomes filled. */

    if ((adp_str.blocked_bp != NULL) && (adp_str.blocked_bp != bp))
    {
   	/* a previous request from the larget TCE area */
	/* remains unsatisfied, so we can't process    */
	/* this request.  Mark the work field and return */
	bp->bufstruct.b_work = LARGE_TCE_RESOURCES_USED;
	TRACE_1("blocked by", (int) adp_str.blocked_bp)
	return (PSC_NO_TCE_FREE);
    }

    /* Compute number of large TCEs required      */
    num_large_TCE = num_TCE / LARGE_TCE_SIZE;
    if (num_TCE % LARGE_TCE_SIZE)
        num_large_TCE++;
    TRACE_2 ("in TCELG", (int) num_large_TCE, bp->bufstruct.b_bcount)

    /* initialize the loop variables - start at first free TCE (index) */
    cur_word = 0;
    index = p720_getfree (adp_str.large_TCE_alloc[cur_word]);

    mask = 0xFFFFFFFF >> (index+1);
    free_blk_len_mask = (~(adp_str.large_TCE_alloc[cur_word])) & mask;
    /* free_blk_len_mask has a one at the bit after the end of the free */
    /* block that starts at index. */
    TRACE_2("inx fmsk", index, free_blk_len_mask)

    /* Scan the alloc table for a free block of resources */
    /* index is the first free slot of the block */
    /* index1 is one past the last free slot of the block */
    while (keep_looking) 
    {
        /* free_blk_len_mask - the first n '0's will remain '0'             */
        /*            the rest of word will be inverted after the first '1' */
        /* ex - free_blk_len_mask (0000 1111 0000) == 0000 0000 1111        */
        /* free_blk_len_mask is used to find the length of the free block   */
	cur_bit = p720_getfree (free_blk_len_mask);
	index1 = cur_word * PSC_WORDSIZE + cur_bit;

        /* make sure it didn't overflow */
        if (index1 > adp_str.large_req_end) {
            index1 = adp_str.large_req_end;
            keep_looking = FALSE;
        }

	if ( index1 - index >= num_large_TCE)
	{
	    /* The TCEs were found and a pointer to   */
	    /* first is stored in the dev_info struct. */
	    com_ptr->dma_addr =
	       (uint) DMA_ADDR2(adp_str.large_tce_start_addr, index) +
	       (uint) (baddr & (PAGESIZE - 1));
	    com_ptr->resource_index = index;
	    com_ptr->TCE_count = num_large_TCE;

	    /* allocate it and return where it starts.  Outside loop */
	    /* is for number of words, inner loop is for bits in the word */
	    index1 = index % PSC_WORDSIZE;
	    for ( i=index / PSC_WORDSIZE; i <= cur_word; i++ )
	    {
	        for ( j=index1; j<PSC_WORDSIZE && num_large_TCE != 0; 
		    j++, num_large_TCE--)
	           ALLOC(adp_str.large_TCE_alloc[i],j)
	        index1 = 0;	/* reset index1 after first longword filled */
	    }
	    com_ptr->resource_state = LARGE_TCE_RESOURCES_USED;
	    TRACE_2 ("out TCE ", com_ptr->resource_index, com_ptr->TCE_count)
	    return (index);
       	}
       	else if (free_blk_len_mask == 0)
       	{
	    /* we haven't reached the end of an unallocated block,   */
	    /* but so far the block is too small.  Advance to the    */
	    /* next word in the table to see if there enough         */
	    /* contiguous resources.                                 */
	    cur_word++;
            free_blk_len_mask = ~adp_str.large_TCE_alloc [cur_word];
	}
	else 
	{
	    /* the current block is not big enough */
	    /* see if there is another             */

            if (cur_bit >= PSC_WORDSIZE) {
                /* advance to the next longword */
                cur_word++;
                tmp_table = adp_str.large_TCE_alloc [cur_word];
	    }
	    else
   	        /* there may still be some unallocated resources in the */
	      	/* current longword.  Mask out up to cur_bit. */
	        tmp_table = adp_str.large_TCE_alloc [cur_word] & 
	 		(0xFFFFFFFF >> cur_bit);

	    cur_bit = p720_getfree(tmp_table);
	    mask = 0xFFFFFFFF >> (cur_bit+1);
            free_blk_len_mask = (~tmp_table) & mask;
	    index = cur_word * PSC_WORDSIZE + cur_bit;
	}
    } /* while */

    /* large allocation has failed */
    adp_str.blocked_bp = bp;
    bp->bufstruct.b_work = LARGE_TCE_RESOURCES_USED;

    TRACE_1 ("NO TCEs!", (int) com_ptr)
    TRACE_1 ("out TCE ", 0)
    DEBUG_0 ("Leaving p720_alloc_TCE routine.\n")
    return (PSC_NO_TCE_FREE);     /* Not enough TCEs available */
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_free_TCE                                                   */
/*                                                                        */
/* FUNCTION:  Free TCEs after command completion.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
/*                                                                        */
/* NOTES:  This routine is called when freeing resources for a command.   */
/*         The used TCEs will be freed to the proper table.               */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    com_ptr - pointer to command structure                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_free_TCE(
             struct cmd_info * com_ptr)
{

    int     i;
    int     word, bit;

    DEBUG_0 ("Entering p720_free_TCE routine.\n")
    if (com_ptr->resource_state == SMALL_TCE_RESOURCES_USED)
    {   /* A TCE is allocated from the small TCE area */
        /* Return this TCE to the small TCE area.      */
        FREE(adp_str.TCE_alloc[com_ptr->resource_index/PSC_WORDSIZE],
	     com_ptr->resource_index % PSC_WORDSIZE)
    }
    else
    {   /* The TCEs are allocated from the large TCE area */
        /* Return the TCEs to the large TCE area.     */
	TRACE_2("index cnt", com_ptr->resource_index, com_ptr->TCE_count)
	word = com_ptr->resource_index / PSC_WORDSIZE; 
	bit = com_ptr->resource_index % PSC_WORDSIZE; 
	while (com_ptr->TCE_count > 0)
	{
            for ( ; bit < PSC_WORDSIZE && com_ptr->TCE_count > 0;
                          bit++, com_ptr->TCE_count--)
		FREE(adp_str.large_TCE_alloc[word], bit) 
	    /* advance to the next long word in the alloc table */
	    bit = 0; 
	    word++;
        }
    }
    /* Clear the TCE save area in the cmd_info struct. */
    com_ptr->resource_index = 0;
    com_ptr->TCE_count = 0;
    DEBUG_0 ("Leaving p720_free_TCE routine.\n")
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:        p720_alloc_resources                                      */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to allocate resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
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
/*    sc_buf structure - pointer to sc_buf received from caller.          */
/*    dev_info structure - pointer to device information structure        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    tag             - Allocation of resources successful.               */
/*    PSC_NO_TAG_FREE - A command tag could not be allocated.             */
/*    PSC_NO_TCE_FREE - Allocation unsuccessful due to a lack of TCEs.    */
/*    PSC_NO_STA_FREE - A small transfer error was unavailable            */
/*                                                                        */
/**************************************************************************/
int
p720_alloc_resources(
    struct sc_buf *bp,
                    struct dev_info * dev_ptr)
{
    struct buf *next_bp;
    uint    local_dma_addr;
    int i, tag;
    struct cmd_info *com_ptr;

    DEBUG_0 ("Entering p720_allocate_resource routine.\n")
    TRACE_1 ("in allc ", (int) dev_ptr)

    /* So, try to allocate a cmd_info structure, i.e. a TAG */
    tag = p720_alloc_TAG (bp);
    if (tag == PSC_NO_TAG_FREE)
    {
       DEBUG_0 ("Leaving psc_allocate_resource routine.\n")
       TRACE_1 ("out allc", PSC_NO_TAG_FREE)
       return(PSC_NO_TAG_FREE);
    }

    /* Miscellaneous initialization in the cmd_info structure */
    com_ptr = &adp_str.command[tag];
    com_ptr->bp = bp;
    com_ptr->preempt_counter = PSC_MAX_PREEMPTS;
    com_ptr->active_fwd = NULL;
    com_ptr->active_bkwd = NULL;
    com_ptr->device = dev_ptr;
    com_ptr->next_dev_cmd = NULL;
    com_ptr->flags = 0;

    if ((bp->bufstruct.b_bcount > ST_SIZE) ||
        ((bp->bufstruct.b_bcount > 0) &&
        (bp->bufstruct.b_xmemd.aspace_id != XMEM_GLOBAL)))
    {
        /* Must be a normal data transfer if execution */
        /* reached here.                               */
        if (p720_alloc_TCE(bp, com_ptr) == PSC_NO_TCE_FREE)
        {   /* Not enough TCE's available */
            DEBUG_0 ("Leaving p720_allocate_resource routine.\n")
            TRACE_1 ("out allc", PSC_NO_TCE_FREE)
            p720_FREE_TAG(com_ptr);
            return (PSC_NO_TCE_FREE);
        }

        /* The required TCE's are free if execution reaches here. */
        /* Setup and call d_master for both spanned and */
        /* unspanned commands.                          */
        if (bp->bp == NULL)
        {   /* Non-spanned command */
            d_master((int) adp_str.channel_id, 
                     ((bp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
                     ((bp->bufstruct.b_flags & B_NOHIDE) ?
                      DMA_WRITE_ONLY : 0), bp->bufstruct.b_un.b_addr,
                     (size_t) bp->bufstruct.b_bcount,
                     &bp->bufstruct.b_xmemd,
                     (char *) com_ptr->dma_addr);
        }
        else
        {   /* must be a spanned command */
            next_bp = (struct buf *) bp->bp;
            local_dma_addr = com_ptr->dma_addr;
            while (next_bp != NULL)
            {
                d_master((int) adp_str.channel_id, 
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
        } /* else */
        DEBUG_0 ("Leaving p720_allocate_resource routine.\n")
        TRACE_1 ("out allc", PSC_NO_ERR)
        return (tag);
    } /* if */

    if (bp->bufstruct.b_bcount == 0)
    {
        /* there is not data to transfer, thus no   */
        /* need to allocate resources.              */
        com_ptr->resource_state = NO_RESOURCES_USED;

        DEBUG_0 ("Leaving p720_allocate_resource routine.\n")
        TRACE_1 ("out allc", PSC_NO_ERR)
        return (tag);
    }
    /* must allocate area from the small transfer area */
    if (p720_alloc_STA(com_ptr) != PSC_NO_STA_FREE)
    {   /* An STA area was free */
        /* Calculate the dma address of the STA and    */
        /* save in the device's info structure.        */
        com_ptr->dma_addr =
            (uint) (DMA_ADDR(adp_str.ddi.tcw_start_addr, 0)) +
            ((uint) (adp_str.STA[com_ptr->resource_index]) -
             ((uint) (adp_str.STA[0]) & ~(PAGESIZE - 1)));

        /* For writes, data needs to be written to the */
        /* small transfer area, so that is done here.  */
        if (!(bp->bufstruct.b_flags & B_READ))
        {
            /* The data is in kernal space so do a byte copy */
            for (i = 0; i < bp->bufstruct.b_bcount; i++)
                *(adp_str.STA[com_ptr->resource_index] + i) =
                    *(bp->bufstruct.b_un.b_addr + i);
        }
        DEBUG_0 ("Leaving p720_allocate_resource routine.\n")
        TRACE_1 ("out allc", PSC_NO_ERR)
        return (tag);
    }
    DEBUG_0 ("Leaving p720_allocate_resource routine.\n")
    TRACE_1 ("out allc", PSC_FAILED)
    p720_FREE_TAG(com_ptr);
    return (PSC_NO_STA_FREE);
}  /* p720_alloc_resources */

/**************************************************************************/
/*                                                                        */
/* NAME:        p720_free_resources                                       */
/*                                                                        */
/* FUNCTION:    Calls the appropriate routines to free all resources.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts must be disabled.                                      */
/*                                                                        */
/* NOTES:  This routine is called to the determine what resources are to  */
/*         be freed.  Pointers and save locations are initialized         */
/*         to the unused state and a d_complete is performed for those    */
/*         commands that require it.  (6/94: d_complete is no longer      */
/*	   called, it does not appear needed for the 720 integrated on    */
/*         PowerPc boxes).					          */
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
/*    PSC_NO_ERR     - Deallocation of resources successful.              */
/*    PSC_COPY_ERROR - Error occurred during data copy.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_free_resources(
                   struct cmd_info * com_ptr,
                   uchar copy_required)
{
/*    struct sc_buf *bp; 		*/
/*    struct buf *next_bp; 		*/
/*    uint    local_dma_addr; 		*/
/*    int     ret_code, ret_code2, i;   */
    int ret_code;

    DEBUG_0 ("Entering p720_free_resources routine.\n")
    TRACE_1 ("in free ", (int) com_ptr)
    ret_code = PSC_NO_ERR;      /* default to successful */

    /* On PowerPC boxes, d_complete only serves to check the channel
     * status register.  For cases where the CSR is set to indicate
     * an error, we will already have found out about the error and
     * processed it as a result of reading the SCSI reset/status register.
     * Therefore, the d_complete-related processing below is unnecessary,
     * and has been commented out.  If the NCR 720 is to be supported on
     * a POWER or POWER2 box, this code needs to be reenabled.
     */ 

/*  (bp is only needed for STA case)
*   ## Obtain pointer to the command to process. ##
*   bp = com_ptr->bp;
*/

    switch (com_ptr->resource_state)
    {
        /* There's no resources to free, so return. */
    case SMALL_TCE_RESOURCES_USED:
    case LARGE_TCE_RESOURCES_USED:
/*
*       ## process case of completion here.    *##
*       ## Setup and call d_complete for both spanned and *##
*       ## unspanned commands.                            *##
*       if (bp->bp == NULL)
*       {       ## Non-spanned command *##
*           ret_code = d_complete((int) adp_str.channel_id,
*                                 DMA_TYPE |
*                                 ((bp->bufstruct.b_flags & B_READ) ?
*                                  DMA_READ : 0) |
*                                 ((bp->bufstruct.b_flags & B_NOHIDE) ?
*                                  DMA_WRITE_ONLY : 0),
*                                 bp->bufstruct.b_un.b_addr,
*                                 (size_t) bp->bufstruct.b_bcount,
*                                 &bp->bufstruct.b_xmemd,
*                                 (char *) com_ptr->dma_addr);
*       }
*       else
*       {       ## must be a spanned command *##
*           next_bp = (struct buf *) bp->bp;
*           local_dma_addr = com_ptr->dma_addr;
*           while (next_bp != NULL)
*           {
*               ret_code2 = d_complete((int) adp_str.channel_id,
*                                      DMA_TYPE |
*                                      ((bp->bufstruct.b_flags & B_READ) ?
*                                       DMA_READ : 0) |
*                                      ((next_bp->b_flags & B_NOHIDE)
*                                       ? DMA_WRITE_ONLY : 0),
*                                      next_bp->b_un.b_addr,
*                                      (size_t) next_bp->b_bcount,
*                                      &next_bp->b_xmemd,
*                                      (char *) local_dma_addr);
*               if (ret_code2 != 0)
*               {   ## Was there a DMA type error?   *##
*                   if (ret_code2 == DMA_SYSTEM)
*                       ## if a system error has occurred *##
*                       ret_code = DMA_SYSTEM;
*                   else
*                       ret_code = PSC_DMA_ERROR;
*               }
*               local_dma_addr += (uint) next_bp->b_bcount;
*               next_bp = (struct buf *) next_bp->av_forw;
*           }
*       }
*/
        p720_free_TCE(com_ptr);
        break;
    case STA_RESOURCES_USED:
/*
*       ## Call d_complete on the small xfer that took place *##
*       ret_code = d_complete(adp_str.channel_id, DMA_TYPE,
*                             adp_str.STA[0], (size_t) PAGESIZE,
*                             &adp_str.xmem_STA,
*                             (char *) adp_str.ddi.tcw_start_addr);
*       if (ret_code != 0)
*       {    ## If an error occurred *##
*           if (ret_code != DMA_SYSTEM)
*               ret_code = PSC_DMA_ERROR;
*           ## Attempt to do another d_master to clean *##
*           ## up the error.                           *##
*           d_master(adp_str.channel_id, DMA_TYPE,
*                    adp_str.STA[0], (size_t) PAGESIZE,
*                    &adp_str.xmem_STA,
*                    (char *) adp_str.ddi.tcw_start_addr);
*           (void) d_complete(adp_str.channel_id, DMA_TYPE,
*                    adp_str.STA[0], (size_t) PAGESIZE, &adp_str.xmem_STA,
*                    (char *) adp_str.ddi.tcw_start_addr);
*       }
*       else
*       {  ## no error detected - good completion *##
*/
	{
            struct sc_buf *bp; /* temporary variable */
	    int i;             /* loop variable */

	    bp = com_ptr->bp;

            /* For reads, data needs to be read from the   */
            /* small transfer area, so that is done here.  */
            if ((bp->bufstruct.b_flags & B_READ) && (copy_required))
            {
                if (bp->bufstruct.b_xmemd.aspace_id == XMEM_GLOBAL)
                {       /* The data is in kernal space so do a  */
                    /* byte copy.                           */
                    for (i = 0; i < bp->bufstruct.b_bcount; i++)
                        *(bp->bufstruct.b_un.b_addr + i) =
                            *(adp_str.STA[com_ptr->resource_index] + i);
                }
                else
                {
                    /* use xmemout to copy the data out to user space */
                    ret_code = xmemout(
                                    adp_str.STA[com_ptr->resource_index],
                                       bp->bufstruct.b_un.b_addr,
                                       bp->bufstruct.b_bcount,
                                       &bp->bufstruct.b_xmemd);
                    if (ret_code != XMEM_SUCC)
                    {
                        p720_logerr(ERRID_SCSI_ERR1,
                                   XMEM_COPY_ERROR, 80,
                                   0, com_ptr, FALSE);
                        ret_code = PSC_COPY_ERROR;
                    }
                    else
                    {
                        ret_code = PSC_NO_ERR;
                    }
                }
            }
	}
/* (commented out as part of the d_complete call)
*       }
*/
        p720_free_STA(com_ptr);
        break;
    case NO_RESOURCES_USED:
    default:
        /* No action is required for these cases */
        break;
    }
    DEBUG_0 ("Leaving p720_allocate_resource routine.\n")

    com_ptr->resource_state = 0;
    p720_FREE_TAG(com_ptr);
    TRACE_1 ("out free", ret_code)
    return (ret_code);
}  /* p720_free_resources */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_iodone                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Iodone Routine                           */
/*                                                                      */
/*      This routine handles completion of commands initiated through   */
/*      the p720_ioctl routine.                                         */
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
/************************************************************************/
int
p720_iodone(struct sc_buf * bp)
{
    DEBUG_0 ("Entering p720_iodone routine.\n")
    TRACE_1 ("iodone  ", (int) bp)
    e_wakeup(&bp->bufstruct.b_event);
    DEBUG_0 ("Leaving p720_iodone routine.\n")
}

/**************************************************************************/
/*                                                                        */
/* NAME: p720_enq_active, p720_enq_wait, p720_enq_wait_resources,         */
/*       p720_enq_abort_bdr                                               */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Utility routines to handle queuing.                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adp_str structure - adapter information structure.                  */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_info structure - pointer to device information structure        */
/*    cmd_info structure - pointer to command information structure        */
/*    sc_buf structure - pointer to scsi command information structure    */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* ERROR DESCRIPTION:  The following errno values may be returned:        */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
p720_enq_active(
               struct dev_info * dev_ptr,
               struct cmd_info * com_ptr)
{
    DEBUG_1 ("p720_enq_active dev_ptr 0x%x\n", dev_ptr)
    TRACE_3 ("enqA", (int) com_ptr->tag, (int) com_ptr, 
                     (int) dev_ptr->active_head)
    if (dev_ptr->active_head == NULL)  /* only ENQ for first command */
    {
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
    }

    if (dev_ptr->active_head == NULL)
    {
        com_ptr->active_fwd = NULL;
        com_ptr->active_bkwd = NULL;
        dev_ptr->active_head = com_ptr;
        dev_ptr->active_tail = com_ptr;
    }
    else
    {
        com_ptr->active_bkwd = dev_ptr->active_tail;
        com_ptr->active_fwd = NULL;
        dev_ptr->active_tail->active_fwd = com_ptr;
        dev_ptr->active_tail = com_ptr;
    }
    TRACE_2 ("out enqA", (int) dev_ptr, (int)dev_ptr->active_head)
}

/************************************************************************/
void
p720_enq_wait(
             struct cmd_info * com_ptr)
{
    struct dev_info * dev_ptr;

    DEBUG_1 ("p720_enq_wait dev_ptr 0x%x\n", com_ptr->device)
    TRACE_2 ("in enqW", (int) com_ptr, (int) com_ptr->device)
    dev_ptr = com_ptr->device;
    if (adp_str.DEVICE_WAITING_head == NULL)
    {
         adp_str.DEVICE_WAITING_head = com_ptr;
    }
    else
    {
        adp_str.DEVICE_WAITING_tail->active_fwd = com_ptr;
    }
    com_ptr->active_fwd = NULL;
    adp_str.DEVICE_WAITING_tail = com_ptr;

    /* enqueue on chain of waiting commands attached to a particular dev_ptr */
    if (dev_ptr->wait_head == NULL)
    {
	dev_ptr->wait_head = com_ptr;
    }
    else
    {
	dev_ptr->wait_tail->next_dev_cmd = com_ptr; 
    }
    com_ptr->next_dev_cmd = NULL;
    dev_ptr->wait_tail = com_ptr;

    TRACE_2 ("out enqW", (int) dev_ptr->wait_head, (int) dev_ptr->wait_tail)
    TRACE_2 ("out enqW", (int) com_ptr, (int) com_ptr->device)
}

/************************************************************************/
void
p720_enq_WFR(
	    struct sc_buf * bp)
{
    DEBUG_1 ("p720_enq_wfr bp 0x%x\n", bp)
    TRACE_1 ("in enqWFR ", (int) bp)
    if (adp_str.REQUEST_WFR_head == NULL)
    {
         adp_str.REQUEST_WFR_head = bp;
         adp_str.REQUEST_WFR_tail = bp;
    }
    else
    {
         adp_str.REQUEST_WFR_tail->bufstruct.av_forw = (struct buf *)bp;
         adp_str.REQUEST_WFR_tail = bp;
    }
    TRACE_1 ("out eqWR", 0)
}

/************************************************************************/
void
p720_enq_abort_bdr(
                  struct dev_info * dev_ptr)
{
    DEBUG_1 ("p720_enq_abort_bdr dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in enqB ", (int) dev_ptr)
    dev_ptr->queue_state &= STOPPING;
    dev_ptr->queue_state |= WAIT_FLUSH;
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
/* NAME: p720_deq_active, p720_deq_wait, p720_deq_wfr, p720_deq_abort_bdr */
/*                                                                        */
/* FUNCTION:                                                              */
/*      Utility routines to handle queuing.                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* ERROR DESCRIPTION:  The following errno values may be returned:        */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
p720_deq_active(
               struct dev_info * dev_ptr,
               struct cmd_info * com_ptr)
{

    DEBUG_1 ("p720_deq_active dev_ptr 0x%x\n", dev_ptr)
    TRACE_2 ("in deqA ", (int) dev_ptr, (int) com_ptr)
    TRACE_1 ("act head", (int) dev_ptr->active_head)

    if (dev_ptr->active_head == dev_ptr->active_tail) /* only one */
    {
        dev_ptr->active_head = NULL;
        dev_ptr->active_tail = NULL;
    }
    else if (dev_ptr->active_head == com_ptr)  /* first one */
    {
        dev_ptr->active_head = com_ptr->active_fwd;
	com_ptr->active_fwd->active_bkwd = com_ptr->active_bkwd;
    }
    else if (dev_ptr->active_tail == com_ptr) /* last one */
    {
	dev_ptr->active_tail = com_ptr->active_bkwd;
	com_ptr->active_bkwd->active_fwd = com_ptr->active_fwd;
    }
    else        /* in the middle */
    {
 	com_ptr->active_bkwd->active_fwd = com_ptr->active_fwd;
	com_ptr->active_fwd->active_bkwd = com_ptr->active_bkwd;
    }
    com_ptr->active_fwd = NULL;
    com_ptr->active_bkwd = NULL;

    /* remove dev_ptr if this was the last command */
    TRACE_1 ("act head", (int) dev_ptr->active_head)
    if (dev_ptr->active_head == NULL)
    {
        if (adp_str.DEVICE_ACTIVE_head == adp_str.DEVICE_ACTIVE_tail)
        {
            adp_str.DEVICE_ACTIVE_head = NULL;
            adp_str.DEVICE_ACTIVE_tail = NULL;
        }
        else if (adp_str.DEVICE_ACTIVE_head == dev_ptr)      /* first one */
        {
            adp_str.DEVICE_ACTIVE_head = dev_ptr->DEVICE_ACTIVE_fwd;
            dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
                                                dev_ptr->DEVICE_ACTIVE_bkwd;
        }
        else if (adp_str.DEVICE_ACTIVE_tail == dev_ptr)  /* last one */
        {
            adp_str.DEVICE_ACTIVE_tail = dev_ptr->DEVICE_ACTIVE_bkwd;
            dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
                                             dev_ptr->DEVICE_ACTIVE_fwd;
        }
        else        /* in the middle */
        {
            dev_ptr->DEVICE_ACTIVE_bkwd->DEVICE_ACTIVE_fwd =
                                             dev_ptr->DEVICE_ACTIVE_fwd;
            dev_ptr->DEVICE_ACTIVE_fwd->DEVICE_ACTIVE_bkwd =
                                            dev_ptr->DEVICE_ACTIVE_bkwd;
        }
        dev_ptr->DEVICE_ACTIVE_fwd = NULL;
        dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
        dev_ptr->device_state = NOTHING_IN_PROGRESS;
    }
    TRACE_1 ("out deqA", (int) dev_ptr)
}

/************************************************************************/
void
p720_deq_wait(
             struct cmd_info * com_ptr)
{
    struct dev_info * dev_ptr;
    struct cmd_info * cur_com_ptr, * prev_com_ptr;

    DEBUG_1 ("p720_deq_wait dev_ptr 0x%x\n", com_ptr)
    TRACE_2 ("in deqW ", (int) com_ptr, (int) adp_str.DEVICE_WAITING_head)
    dev_ptr = com_ptr->device;
    TRACE_2 ("dev wait", (int) dev_ptr->wait_head, 
		      (int) dev_ptr->wait_tail)

    /* dequeue cmd_info from wait queue anchored in dev_info structure */
    if (dev_ptr->wait_head == dev_ptr->wait_tail)
    {
	dev_ptr->wait_head = NULL;
	dev_ptr->wait_tail = NULL;
    }
    else
    {
	dev_ptr->wait_head = com_ptr->next_dev_cmd;
    }
    com_ptr->next_dev_cmd = NULL;

    /* dequeue command from global queue */
    /* only element in the list */
    if (adp_str.DEVICE_WAITING_head == adp_str.DEVICE_WAITING_tail)
    {
        adp_str.DEVICE_WAITING_head = NULL;
        adp_str.DEVICE_WAITING_tail = NULL;
    }
    /* first element in the list */
    else if (com_ptr == adp_str.DEVICE_WAITING_head)
    {
        adp_str.DEVICE_WAITING_head = com_ptr->active_fwd;
    }
    /* not the first element in the list */
    else
    {
        cur_com_ptr = adp_str.DEVICE_WAITING_head;
        while (cur_com_ptr != com_ptr)
        {
            prev_com_ptr = cur_com_ptr;
            cur_com_ptr = cur_com_ptr->active_fwd;
        }
        /* the element has been found - remove it from chain */
        prev_com_ptr->active_fwd = com_ptr->active_fwd;

        /* if this is the last element in the wait q, reset  */
        /* the tail pointer */
	if (com_ptr == adp_str.DEVICE_WAITING_tail)
	    adp_str.DEVICE_WAITING_tail = prev_com_ptr;
    }
    com_ptr->active_fwd = NULL;
    TRACE_2 ("out devw", (int) dev_ptr->wait_head, 
		      (int) dev_ptr->wait_tail)
    TRACE_2 ("out deqW", (int) com_ptr, (int) adp_str.DEVICE_WAITING_head)
}

/************************************************************************/
void
p720_deq_WFR(
	    struct sc_buf * bp,
	    struct sc_buf * prev_bp)
{
    DEBUG_1 ("p720_deq_wfr bp 0x%x\n", bp)
    TRACE_2 ("deqWFR ", (int) bp, (int) prev_bp)

    if (prev_bp == NULL)   /* bp is first in list */
    {
         adp_str.REQUEST_WFR_head =
		(struct sc_buf *) bp->bufstruct.av_forw;
    }
    else 		  /* in middle or end of list */
    {
        prev_bp->bufstruct.av_forw = bp->bufstruct.av_forw;
    }

    if (adp_str.REQUEST_WFR_tail == bp)   /* bp is at end of list */
            adp_str.REQUEST_WFR_tail = prev_bp;
	    
    bp->bufstruct.av_forw = NULL;

    if (adp_str.blocked_bp == bp)
	adp_str.blocked_bp = NULL;
}

/************************************************************************/
void
p720_deq_abort_bdr(
                  struct dev_info * dev_ptr)
{

    DEBUG_1 ("p720_deq_abort_bdr dev_ptr 0x%x\n", dev_ptr)
    TRACE_1 ("in deqB ", (int) dev_ptr)
    if (adp_str.ABORT_BDR_head == adp_str.ABORT_BDR_tail)
    {
        adp_str.ABORT_BDR_head = NULL;
        adp_str.ABORT_BDR_tail = NULL;
    }
    else if (adp_str.ABORT_BDR_head == dev_ptr)  /* first one */
    {
        adp_str.ABORT_BDR_head = dev_ptr->ABORT_BDR_fwd;
        dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd = dev_ptr->ABORT_BDR_bkwd;
    }
    else if (adp_str.ABORT_BDR_tail == dev_ptr)      /* last one */
    {
        adp_str.ABORT_BDR_tail = dev_ptr->ABORT_BDR_bkwd;
        dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd = dev_ptr->ABORT_BDR_fwd;
    }
    else        /* in the middle */
    {
        dev_ptr->ABORT_BDR_bkwd->ABORT_BDR_fwd = dev_ptr->ABORT_BDR_fwd;
        dev_ptr->ABORT_BDR_fwd->ABORT_BDR_bkwd = dev_ptr->ABORT_BDR_bkwd;
    }
    dev_ptr->ABORT_BDR_fwd = NULL;
    dev_ptr->ABORT_BDR_bkwd = NULL;
    TRACE_1 ("out deqB", (int) dev_ptr)
}

/*************************************************************************/
/*                                                                      */
/* NAME:        p720_intr                                               */
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
/*      cmd_info - driver structure which holds all information related  */
/* 		  to a particular command			        */
/*      dev_info - driver structure which holds information related to  */
/*                 a particular device                                  */
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
/************************************************************************/
int
p720_intr(struct intr * handler)
{
    struct sc_buf *bp, *tbp;
    struct dev_info *dev_ptr, *tmp_dev_ptr;
    struct cmd_info *com_ptr;
    int     save_isr, save_dsr; /* save status registers */
    int	    tag;
    int     scsi_stat, dma_stat;
    int     command_issued_flag, check_for_disconnect;
    int     issue_abrt_bdr;     /* are we doing abort/bdr */
    int     dev_index;
    caddr_t eaddr;
    int     start_new_job;      /* 1=start new job up */
    int     save_sist;
    int	    i;  		/* loop variable */
    uint  * target_script;      /* used to patch script */
    uchar   save_stat;
    register int rc;

    DEBUG_0 ("Entering p720_intr routine\n")
    TRACE_1 ("in intr", 0)

    /* check and make sure its a scsi interrupt */
    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    rc = p720_read_reg(SCSIRS, SCSIRS_SIZE, eaddr);
    DEBUG_1 ("p720_intr: SCSIRS = 0x%x\n", rc);
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, adp_str.devno);
    /* channel check and associated interrupts */
    if (!((rc == REG_FAIL) || (rc & 0x000000F0)))
    {
        /* Check to see if this is our interrupt */
        if (!(rc & 0x00000001))
        {
            /* Check to see if a command is active */
            if (adp_str.DEVICE_ACTIVE_head != NULL)
            {
		/* d_complete_ppc only checks the channel status register.
		 * We have already done the equivalent by reading the
		 * SCSIRS register (CCHK would be active if there was an
		 * error stored in the CSR).  Comment out the d_complete logic.
		 */
/*
*               rc = d_complete((int) adp_str.channel_id, DMA_TYPE,
*                               (char *) adp_str.IND_TABLE.system_ptr,
*                               (size_t) PAGESIZE, &adp_str.xmem_MOV, 
*			        (char *) adp_str.IND_TABLE.dma_ptr);
*
*               #* if we got a system dma error, we need to kill *#
*               #* all jobs on all devices.                      *#
*               if (rc == DMA_SUCC)
*               {
*/
                    /* read the ISTAT register to see which interrupt */
                    /* caused us to be called                         */
                    save_isr = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
                    tag = p720_read_reg(SCRATCHA0, SCRATCHA0_SIZE, eaddr);
		    TRACE_1 ("tag is ", tag)
                    DEBUG_1 ("ISTAT is 0x%x\n", save_isr);
                    if ((save_isr != REG_FAIL) && (tag != REG_FAIL))
                    {
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
                            save_dsr = p720_read_reg(DSTAT, DSTAT_SIZE, 
							eaddr);
                            DEBUG_1 ("DSTAT is 0x%x\n", save_dsr);
                            if (save_dsr == REG_FAIL)
                            {
                                DEBUG_0 ("p720_intr: bad DSTAT\n")
                                TRACE_1 ("bad dsta", 0)
                                TRACE_1 ("out intr", 0)
                                p720_cleanup_reset(REG_ERR, eaddr);
            			BUSIO_DET(eaddr);
                                i_reset(&(adp_str.intr_struct));
                                DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 
					adp_str.devno);
                                return (INTR_SUCC);
                            }
                            
                            /**********************************************/
                            /*         PLANNED SCRIPT INTERRUPT           */
                            /**********************************************/
                            if (save_dsr & SIR)
                            {
                                /* DSPS contains the value which identifies */
                                /* the specific planned SCRIPTS interrupt.  */
                                dma_stat = word_reverse(p720_read_reg(DSPS,
                                                        DSPS_SIZE, eaddr));
                                DEBUG_1 ("PLANNED INTR is 0x%x\n", dma_stat);
                                if (dma_stat == REG_FAIL)
                                {
                                    DEBUG_0 ("p720_intr: bad reg read DSPS\n")
                                    TRACE_1 ("bad dsps", 0)
                                    TRACE_1 ("out intr", 0)
                                    p720_cleanup_reset(REG_ERR, eaddr);
            			    BUSIO_DET(eaddr);
                                    i_reset(&(adp_str.intr_struct));
                                    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR,
                                            0, adp_str.devno);
                                    return (INTR_SUCC);
                                }

 				if (adp_str.command[tag].in_use == TRUE)
				{
				    dev_ptr = adp_str.command[tag].device;
				    com_ptr = &adp_str.command[tag];
				    bp = com_ptr->bp;
                                    TRACE_3("dint", (int) tag, (int) dma_stat, 
                                                (int) bp)
                                    TRACE_3("dint", (int) tag, (int) com_ptr, 
                                                (int) dev_ptr)
				}	
				else if ((dma_stat != A_suspended) &&
				        (dma_stat != A_unknown_reselect_id) &&
				        (dma_stat != A_uninitialized_reselect))
				/* tag is not valid or needed for these */
				/* interrupts that come from the iowait */
				/* part of the script. */
				{
                                    TRACE_2 ("bad tag", (int) tag, 
							(int) dma_stat)
                                    p720_logerr(ERRID_SCSI_ERR2,
                                        ADAPTER_INT_ERROR, 85, 0, NULL, TRUE);
				    p720_command_reset_scsi_bus(eaddr);
                                    TRACE_1 ("out intr", 0)
                                    i_reset(&(adp_str.intr_struct));
                                    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 
					adp_str.devno);
                                    return (INTR_SUCC);
				}

				/***************************************/
                                /*    SWITCH ON TYPE OF INTERRUPT      */
				/***************************************/
                                switch (dma_stat)
                                {       /* completed command */
                                    case A_check_next_io:
                                    TRACE_1 ("chk next", (int) dev_ptr)
                                    case A_check_next_io_data:
                                    /* the device is about to disc. from */
                                    /* the bus. and a save DP has just   */
                                    /* been sent. Move any data xferred  */
                                    /* into the buffer and adjust the    */
                                    /* data pointer within our buffer to */
                                    /* show the buffer size has changed. */
                                    TRACE_2 ("chk nxtd", (int) dev_ptr,
							(int) bp)
                                    if (dma_stat == A_check_next_io_data)
                                    {
                                        bp->bufstruct.b_resid = 0;
                                    }
                                    else if (com_ptr->flags & RESID_SUBTRACT)
                                    {
                                        bp->bufstruct.b_resid
                                            -= com_ptr->bytes_moved;
                                    }

                                    /* Turn off resid subtract */
                                    com_ptr->flags &= ~RESID_SUBTRACT;
                                    dev_ptr = NULL;
                                    start_new_job = TRUE;
                                    break;

                                case A_io_done:
                                case A_io_done_after_data:
                                    TRACE_2 ("A_iodone", (int) dev_ptr, 
							 (int) bp)
                                    DEBUG_0 ("p720_intr: Current I/O done\n")
                                    w_stop(&dev_ptr->dev_watchdog.dog);
                                    /* reset completion flags */
                                    if (dma_stat == A_io_done_after_data)
                                    {
                                        bp->bufstruct.b_resid = 0;
                                        /* resid = 0 */
                                    }
                                    else if (com_ptr->flags & RESID_SUBTRACT)
                                    {
                                        bp->bufstruct.b_resid
                                            -= com_ptr->bytes_moved;
                                        /* do subtraction */
                                    }

                                    if ((adp_str.ABORT_BDR_head == dev_ptr) ||
                                         (dev_ptr->ABORT_BDR_bkwd != NULL))
                                        /* there is an abort or bdr to queued */
                                        issue_abrt_bdr = TRUE;
                                    else
                                        /* there isn't an abort or bdr queued */
                                        issue_abrt_bdr = FALSE;

                                    if (issue_abrt_bdr)
                                    {
					/* expedite an abort/bdr for this 
					 * device, we will iodone the bp when
					 * when we flush the queues. 
					 */
                                        TRACE_1 ("A_iodBDR", (int) dev_ptr)
                                        start_new_job =
                                            p720_issue_abort_bdr(dev_ptr,
                                                bp, TRUE, save_isr &
					        CONNECTED, eaddr);
					break;
                                    }
				    /* otherwise issue a command as soon as */
				    /* possible to keep the chip busy, and  */
				    /* process the bp. */

                                    /* get status byte because issue will
                                       destroy it */
                                    save_stat = GET_CMD_STATUS_BYTE(
                                            adp_str.SCRIPTS[dev_ptr->
                                            sid_ptr->script_index].script_ptr);
                                    DEBUG_1 ("STATUS BUFFER = 0x%x\n",
                                              save_stat);

				    /* special trace hook to note that */
				    /* command is done */
                    	  	    DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0,
					adp_str.devno, bp);

                                    if (save_stat == SC_GOOD_STATUS)
                                    {   /* scsi status is ok */

                                        command_issued_flag = p720_issue_cmd(
						eaddr);

					/* if the first command finished, */
					if (dev_ptr->active_head == com_ptr)
					{
					    /* no need for starvation */
					    if (dev_ptr->flags & STARVATION) 
					    {
					        dev_ptr->flags &= ~STARVATION;
					        /* release any held cmds */
						(void) p720_unfreeze_qs(dev_ptr, 
							NULL, FALSE, eaddr);
 						if (!command_issued_flag)
                                                    command_issued_flag =
                                                        p720_issue_cmd(eaddr);
					    }

					    dev_ptr->dev_watchdog.save_time = 0;
					    /* start the timer for any next 
					     * cmd (issue_cmd above will not 
					     * have started it since com_ptr 
					     * was still queued when issue_cmd 
				             * was called). 
					     */
					    if (com_ptr->active_fwd != NULL)
					    {
						tbp = com_ptr->active_fwd->bp;
	    					if (tbp->timeout_value == 0)
                				    dev_ptr->dev_watchdog.
							dog.restart = (ulong) 0;
	   					else
                				    dev_ptr->dev_watchdog.
						    dog.restart = (ulong) 
							tbp->timeout_value + 1;
            				        w_start(
						    &dev_ptr->dev_watchdog.dog);
					    }
					}
					else /* other than first command */
					{
					    dev_ptr->active_head->
					               preempt_counter--;
					    if (dev_ptr->active_head->
						    preempt_counter == 0)
					    {
						dev_ptr->flags |= STARVATION;
						p720_freeze_qs(dev_ptr);
						/* move pending cmds and bps */
					    }
            				    w_start(&dev_ptr->dev_watchdog.dog);
					}

					p720_deq_active(dev_ptr, com_ptr);

					/* bp should only be null for abort or
					 * bdr ioctls, in which case we should
					 * not get this interrupt.  Also, the
					 * original code included a check
					 * for B_ERROR being previously set.
					 * It appears if this was previously
					 * set, we want to issue an abort or
					 * bdr (in which case we would have
					 * done so above and not gotten here),
					 * or else have aborted the chip and
					 * reset the scsi bus, clearing any
					 * SIR interrupts first.
					 */

                                        TRACE_1 ("A_iodgds", (int) dev_ptr)
                                        /* set scsi status in sc_buf */
                                        bp->bufstruct.b_error = 0;
                                        bp->status_validity = 0;
                                        bp->scsi_status = SC_GOOD_STATUS;
                                        dev_ptr->flags |= RETRY_ERROR;
                                        /* free resources */
                                        rc = p720_free_resources(com_ptr,TRUE);
                                        if (rc == PSC_NO_ERR)
                                        {
                                            iodone((struct buf *) bp);
                                            if ((dev_ptr->queue_state
                                                           & STOPPING) &&
                                                (dev_ptr->active_head == NULL))
					    {
                                                e_wakeup(&dev_ptr->stop_event);
                                            }
                                        }
                                        else
                                        {
					    /* fail free assumes cmd  */
					    /* is on the active queue */
					    p720_enq_active(dev_ptr, com_ptr);

                                            command_issued_flag = 
					       p720_fail_free_resources(dev_ptr,
						  bp, save_isr & CONNECTED,
					 	  command_issued_flag, eaddr);
                                        }
				    }
                                    else    /* status is not okay */
                                    {
					TRACE_1 ("bad status", save_stat)
                                        dev_ptr->flags |= RETRY_ERROR;

				        if ((dev_ptr->active_head !=
					     dev_ptr->active_tail) && 
					    ((save_stat == SC_CHECK_CONDITION)
					  || (save_stat == 
						       SC_COMMAND_TERMINATED)))
					{
					    /* handle starvation and preempt */

                                            /* if the first command finished */
                                            if (dev_ptr->active_head == com_ptr)
                                            {
                                                /* no need for starvation */
                                                if (dev_ptr->flags 
							  & STARVATION)
						{
                                                    dev_ptr->flags &= 
							     ~STARVATION;
					            /* release any held cmds */
						    (void) p720_unfreeze_qs(
						     dev_ptr, NULL, FALSE, eaddr);
						}

					        dev_ptr->dev_watchdog.save_time 
							= 0;
					        /* start the timer for any next 
					         * command 
					         */
					        if (com_ptr->active_fwd != NULL)
					        {
						    tbp = 
						        com_ptr->active_fwd->bp;
	    					    if (tbp->timeout_value == 0)
                				        dev_ptr->dev_watchdog.
							dog.restart = (ulong) 0;
	   					    else
                				        dev_ptr->dev_watchdog.
						        dog.restart = (ulong) 
							tbp->timeout_value + 1;
            				            w_start(
						    &dev_ptr->dev_watchdog.dog);
					        }
                                            }
                                            else /* other than first command */
                                            {
                                                dev_ptr->active_head->
                                                       preempt_counter--;
                                                if (dev_ptr->active_head->
                                                    preempt_counter == 0)
                                                {
                                                    dev_ptr->flags |= STARVATION;
                                                    p720_freeze_qs(dev_ptr);
                                                /* move pending cmds and bps */
                                                }
            				        w_start(
						    &dev_ptr->dev_watchdog.dog);
                                            }

                                            p720_deq_active(dev_ptr, com_ptr);

					    /* set bp */
					    bp->adap_q_status = 
							SC_DID_NOT_CLEAR_Q;
                                            bp->scsi_status = save_stat;
                                            bp->status_validity = SC_SCSI_ERROR;
                                            bp->bufstruct.b_flags |= B_ERROR;
                                            bp->bufstruct.b_error = EIO;

					    /* set queue state */
					    if (dev_ptr->queue_state & STOPPING)
					        dev_ptr->queue_state = 
						    STOPPING_or_HALTED_CAC;
					    else
					        dev_ptr->queue_state = 
							HALTED_CAC;

					    if (!(dev_ptr->flags & STARVATION))
					    {
					       /* freeze queues if not already 
						* frozen due to starvation.
						*/
						p720_freeze_qs(dev_ptr);
					    }

                                            command_issued_flag = 
						p720_issue_cmd(eaddr);

                                            /* free resources and */
					    /* return the bp */
                                            rc = p720_free_resources(com_ptr,
							TRUE);
                                            if (rc == PSC_NO_ERR)
                                            {
                                                iodone((struct buf *) bp);
						/* we don't check stopping, */
						/* we will resolve CAC 1st. */
                                            }
                                            else
                                            {
					        bp->adap_q_status = 0;
					        /* fail free assumes cmd  */
					        /* is on the active queue */
					        p720_enq_active(dev_ptr,com_ptr);
                                                command_issued_flag = 
						    p720_fail_free_resources(
						      dev_ptr, bp, save_isr &
						      CONNECTED,
						      command_issued_flag,
						      eaddr);
                                            }
					} /* end if CAC */
					else if (save_stat == SC_QUEUE_FULL)
					{
					    TRACE_1 ("iodon qfull", 
								(int) com_ptr)
					    /* deq_active and enq_wait,
						   trying to delay */
					    p720_deq_active(dev_ptr, com_ptr);
					    p720_q_full(dev_ptr,
								 com_ptr);
                                            command_issued_flag = 
						p720_issue_cmd(eaddr);
					}
					else 
					{
					    /* not a CAC.  If queueing the  
					     * target has probably flushed its
					     * queue, but we will issue an abort
					     * to be sure.  Otherwise we just
					     * call fail cmd to clean up.
					     */ 
                                            TRACE_2 ("chk noq", save_stat,
                                                         (int) dev_ptr)

                                            /* set scsi stat in sc_buf */
                                            bp->scsi_status = save_stat;
                                            bp->status_validity = SC_SCSI_ERROR;
                                            bp->bufstruct.b_flags |= B_ERROR;
                                            bp->bufstruct.b_error = EIO;

					    p720_enq_abort_bdr(dev_ptr);
					    dev_ptr->flags |= SCSI_ABORT;
					    /* issue_abort_bdr returns whether
					     * we should start a new job.  It
					     * will determine if we actually 
					     * need to send an abort, or 
					     * whether a call to fail_cmd is
					     * sufficient.
					     */	
					    command_issued_flag = 
						!p720_issue_abort_bdr(dev_ptr,
                                                bp, FALSE, 
					        save_isr & CONNECTED, eaddr);
						
 					    /* if no abort or bdr was issued
					     * see if we can issue a regular
					     * command.
					     */
					    if (!command_issued_flag)
						command_issued_flag =
						    p720_issue_cmd(eaddr);

					} 
                                    }       /* end else bad status */

    				    if (adp_str.REQUEST_WFR_head != NULL)
                                        p720_check_wfr_queue(
                                               command_issued_flag, eaddr);
			    	    else if (!command_issued_flag && 
				        (adp_str.DEVICE_ACTIVE_head != NULL))
				    {
				   	p720_write_reg(DSP, DSP_SIZE, 
					    word_reverse(
						adp_str.SCRIPTS[0].dma_ptr),
						eaddr);
				    }
                                    break;

                                case A_suspended:
                                    TRACE_1 ("suspended", 0)
                                    dev_ptr = NULL;
                                    start_new_job = TRUE;
                                    break;

                                case A_sync_neg_done:
                                    TRACE_1 ("neg done", (int) dev_ptr)
                                    /* sync complete and successful */
                                    dev_ptr->sid_ptr->negotiate_flag = FALSE;
                                    dev_ptr->sid_ptr->async_device = FALSE;

                                    DEBUG_0 ("p720_intr: SYNC_NEG_DONE \n")
                                    /* supports synch negotiation */
                                    if ((rc = p720_verify_neg_answer((uint *)
                                        adp_str.SCRIPTS[dev_ptr->sid_ptr->
					script_index].script_ptr,
                                        dev_ptr)) == PSC_NO_ERR)
				    {
                                        /* setup script to issue the */
					/* command already tagged to */
				        /* this device queue  */
                                        p720_prep_main_script( 
					    com_ptr, (uint *) adp_str.SCRIPTS
                                            [dev_ptr->sid_ptr->script_index].
                                            script_ptr, dev_ptr->sid_ptr->
					    dma_script_ptr);

                                         ISSUE_MAIN_AFTER_NEG(dev_ptr->
						sid_ptr->dma_script_ptr, eaddr);
                                         dev_ptr->device_state = CMD_IN_PROGRESS;
                                    }
				    else  /* verify_neg_answer rc */
				    {     /* is non-zero, we must */
					  /* issue a msg reject.  */
                                        if (rc == PSC_FAILED)
                                        {
                                            /* need to issue msg reject */
                                            /* First, prep the script   */
                                            p720_prep_main_script( com_ptr, 
                                                (uint *) adp_str.SCRIPTS[
					        dev_ptr->sid_ptr->script_index].
                                                script_ptr, dev_ptr->sid_ptr->
						dma_script_ptr);
                                            dev_ptr->device_state =
                                                    CMD_IN_PROGRESS;
                                            rc = p720_write_reg((uint) DSP,
                                                (char) DSP_SIZE, word_reverse(
                                                dev_ptr->sid_ptr->dma_script_ptr
					        + Ent_reject_target_sync),
						eaddr);
                                   	} 
                                        if (rc == REG_FAIL)
                                        {
					    p720_cleanup_reset(REG_ERR, eaddr);
					}	
				    }
                                    break;

                                case A_sync_msg_reject:
                                    TRACE_1 ("sync rej", (int) dev_ptr)
                                    /* we tried to negotiate for synch mode,
                                       but failed. ASYNC mode patched into
                                       the script. */
                                    dev_ptr->sid_ptr->negotiate_flag = FALSE;
                                    dev_ptr->sid_ptr->async_device = TRUE;
                                    p720_prep_main_script(com_ptr, 
                                        (uint *) adp_str.SCRIPTS[dev_ptr->
					sid_ptr->script_index].script_ptr,
                                        dev_ptr->sid_ptr->dma_script_ptr);
                                    dev_ptr->device_state = CMD_IN_PROGRESS;
                                    DEBUG_0 ("p720_intr: NEG_FAILED \n")

			            target_script = (uint *) adp_str.SCRIPTS
			                [dev_ptr->sid_ptr->script_index].
					script_ptr;
                                    /* temporarily use rc as loop counter */
 				    /* as we patch the SCRIPT for Async.  */
			            TRACE_1 ("msg_rj: async", 0)
                                    for (rc=0; rc<4; rc++)
                                    {
                                        target_script[R_sxfer_patch_Used[rc]] =
                                            word_reverse(SXFER_ASYNC_MOVE);
                                        target_script[R_scntl1_patch_Used[rc]] =
                                            word_reverse(SCNTL1_EOFF_MOVE);
                                        target_script[R_scntl3_patch_Used[rc]] =
                                            word_reverse(SCNTL3_SLOW_MOVE);
		                    }

                                    /* and restart script */
                                    rc = p720_read_reg(DSP, DSP_SIZE, 
							eaddr);
                                    if (rc == REG_FAIL)
                                    {
                                        p720_cleanup_reset(REG_ERR, eaddr);
                                        break;
                                    }
                                    rc = p720_write_reg(DSP, DSP_SIZE, rc,
							eaddr);
                                    if (rc == REG_FAIL)
                                    {
                                        p720_cleanup_reset(REG_ERR, eaddr);
                                    }
                                    break;

                                case A_ext_msg:
                                case A_modify_data_ptr:
                                case A_target_sync_sent:
                                    DEBUG_0 ("p720_intr: mod. data ptr \n")
                                    TRACE_1 ("ext msg ", (int) dev_ptr)
                                    if (p720_handle_extended_messages(
                                          (uint *) adp_str.SCRIPTS
                                          [dev_ptr->sid_ptr->script_index].
                                          script_ptr, (uint *) dev_ptr->
					  sid_ptr->dma_script_ptr, 
					  com_ptr, dma_stat, eaddr) 
					  == REG_FAIL)
                                    {   /* register error */
                                        p720_cleanup_reset(REG_ERR, eaddr);
                                    }
                                    break;

                                case A_abort_io_complete:
                                    TRACE_1 ("ABORT done", (int) dev_ptr)
                                    w_stop(&dev_ptr->dev_watchdog.dog);
				    dev_ptr->device_state =
						NOTHING_IN_PROGRESS;
                                    /* flush the aborted LUN */
				    p720_fail_cmd(dev_ptr);

                                    if (dev_ptr->ioctl_wakeup == TRUE)
                                    {
                                        dev_ptr->ioctl_errno = PSC_NO_ERR;
                                        dev_ptr->ioctl_wakeup = FALSE;
                                        e_wakeup(&dev_ptr->ioctl_event);
                                    }
                                    start_new_job = TRUE;
                                    break;

                                case A_bdr_io_complete:
                                    TRACE_1 ("BDR done", (int) dev_ptr)
                                    w_stop(&dev_ptr->dev_watchdog.dog);
				    dev_ptr->device_state =
						NOTHING_IN_PROGRESS;

				    /* reflect the hard reset for the id */
                                    dev_ptr->sid_ptr->negotiate_flag = TRUE;
                                    dev_ptr->sid_ptr->restart_in_prog = TRUE;

				    /* flush all the LUNs which had active 
				     * commands.  For LUNS that had only
				     * pending commands, freeze the qs if
				     * needed until the restart delay is
				     * completed.
				     */
    				    dev_index = dev_ptr->scsi_id << 3;
    				    for (i = 0; i< MAX_LUNS; i++)
    				    {
        			        if ((tmp_dev_ptr = adp_str.
					    device_queue_hash[dev_index + i]) 
					    != NULL)
        			 	{
            				    tmp_dev_ptr->flags &= 
						~SCSI_BDR_or_ABORT;
					    if (tmp_dev_ptr == dev_ptr) 
					    {
					        if (dev_ptr->active_head != 
						       dev_ptr->active_tail)
						    p720_fail_cmd(dev_ptr);
						else
						{
						    (void) p720_free_resources(
							   com_ptr, FALSE);
						    p720_deq_active(dev_ptr, 
								    com_ptr);
						    p720_check_qs(dev_ptr);
						}
					    }
					    else  /* another lun */
					    {
					        if (tmp_dev_ptr->active_head 
						            != NULL)
	    				            p720_fail_cmd(tmp_dev_ptr);
						else
						    p720_check_qs(tmp_dev_ptr);
					    }

					    /* finally, handle any queued
					     * aborts, and wakeup any 
					     * ioctl sleepers.  The issue_abrt_
				             * bdr variable indicates whether
					     * an abort was pending.
					     */
					    if ((adp_str.ABORT_BDR_head ==
					      tmp_dev_ptr) || (tmp_dev_ptr->
					      ABORT_BDR_bkwd != NULL))
					    {
					        p720_deq_abort_bdr(tmp_dev_ptr);
					        issue_abrt_bdr = TRUE;
					    }
				            else
						issue_abrt_bdr = FALSE;
						
                                            if (tmp_dev_ptr->ioctl_wakeup 
							== TRUE)
                                    	    {
					        if ((tmp_dev_ptr == dev_ptr) &&
						    (issue_abrt_bdr == FALSE))
                                        	    dev_ptr->ioctl_errno = 
							   PSC_NO_ERR;
						else
                                        	    tmp_dev_ptr->ioctl_errno = 
							   EIO;

                                                tmp_dev_ptr->ioctl_wakeup =
							   FALSE;
                                                e_wakeup(&tmp_dev_ptr->
							   ioctl_event);
                                            }
        				}
    				    } /* end of loop checking all luns */

				    dev_ptr->sid_ptr->bdring_lun = NULL;

                                    adp_str.restart_watchdog.dog.restart =
                                            adp_str.ddi.cmd_delay + 1;
                                    w_start(&(adp_str.restart_watchdog.dog));
                                    start_new_job = TRUE;
                                    break;

                                case A_neg_select_failed:
                                case A_cmd_select_atn_failed:
                                    TRACE_1 ("Cneg bad", (int) dev_ptr)
                                    TRACE_1 ("Cneg tag", (int) tag)
                                    /* We got beaten to the bus, probably by */
                                    /* a cmd completion.  Put the script onto*/
                                    /* waiting to execute queue and go to the*/
                                    /* WAIT SCRIPT. This isn't an error but a*/
                                    /* change of status.                     */

                                    DEBUG_0 ("p720_intr: select_atn_failed \n")
                                    w_stop(&dev_ptr->dev_watchdog.dog);
                                    p720_deq_active(dev_ptr, com_ptr);
                                    /* place this dev on front of WAIT Q */
                                    if (adp_str.DEVICE_WAITING_head == NULL)
                                    {
                                        adp_str.DEVICE_WAITING_head = com_ptr;
                                        adp_str.DEVICE_WAITING_tail = com_ptr;
                                        com_ptr->active_fwd = NULL;
					com_ptr->next_dev_cmd = NULL;
					dev_ptr->wait_tail = com_ptr;
                                    }
                                    else        /* list not empty */
                                    {
					com_ptr->next_dev_cmd = 
						dev_ptr->wait_head;
                                        com_ptr->active_fwd =
                                            adp_str.DEVICE_WAITING_head;
                                        adp_str.DEVICE_WAITING_head = com_ptr;
					if (dev_ptr->wait_tail == NULL)
					    dev_ptr->wait_tail = com_ptr;
                                    }
				    dev_ptr->wait_head = com_ptr;
                                    /* if no commands are outstanding, the */
                                    /* scsi bus is hung. Attempt a scsi bus */
                                    /* reset to clear up this condition.   */
                                    if (adp_str.DEVICE_ACTIVE_head == NULL)
                                    {
                                        p720_command_reset_scsi_bus(eaddr);
                                    }
                                    else
                                    {
					/* restart command timer */
					if (dev_ptr->active_head != NULL)
            				    w_start(&dev_ptr-> dev_watchdog.dog);

                                        /* restart IOWAIT routine */
                                        if (p720_write_reg(DSP, DSP_SIZE,
                                            word_reverse(adp_str.SCRIPTS[0].
                                            dma_ptr), eaddr) == REG_FAIL)
                                            p720_cleanup_reset(REG_ERR, eaddr);
                                    }
                                    break;

                                case A_abort_select_failed:
                                    /* someone beat us to the bus. */
                                    /* So we have to */
                                    /* remember to reissue the abort again */
                                    /* when the bus frees up.  */
                                case A_bdr_select_failed:
                                    DEBUG_0 ("p720_intr: ABRTBDR_SEL_FAILED \n")
                                    TRACE_1 ("ABneg bad", (int) dev_ptr)
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

				    /* deq_active the unstarted BDR */
                                    p720_deq_active(dev_ptr, com_ptr);

				    /* if there are outstanding cmds for */
				    /* lun (aborts) or id (bdrs), we need */
				    /* to restore the iowait jump */
				    if (dma_stat == A_abort_select_failed)
				    {
					dev_ptr->device_state &= 
							~ABORT_IN_PROGRESS;
                                        if (dev_ptr->active_head != NULL)
                                            p720_restore_iowait_jump( (uint *)
                                                adp_str.SCRIPTS[0].script_ptr,
                                                dev_ptr, dev_ptr->sid_ptr->
						dma_script_ptr);
                                    }
                                    else
                                    {   
					dev_ptr->device_state &= 
							~BDR_IN_PROGRESS;
    				        dev_index = dev_ptr->scsi_id << 3;
    				        for (i = 0; i< MAX_LUNS; i++)
    				        {
        				    if (((tmp_dev_ptr = adp_str.
						device_queue_hash[dev_index 
						+ i]) != NULL) &&
						(tmp_dev_ptr->active_head
						 != NULL))
        				    {
                                                p720_restore_iowait_jump(
						  (uint *) adp_str.SCRIPTS[0].
						  script_ptr, tmp_dev_ptr, 
					          tmp_dev_ptr->sid_ptr->
						  dma_script_ptr);
        				    }
    					}
                                    }
                                    /* put device on front of abort queue   */
                                    if (adp_str.ABORT_BDR_head == NULL)
                                    {
                                        dev_ptr->ABORT_BDR_fwd = NULL;
                                        dev_ptr->ABORT_BDR_bkwd = NULL;
                                        adp_str.ABORT_BDR_head = dev_ptr;
                                        adp_str.ABORT_BDR_tail = dev_ptr;
                                    }
                                    else        /* list not empty */
                                    {
                                        adp_str.ABORT_BDR_head->ABORT_BDR_bkwd
                                          = dev_ptr;
                                        dev_ptr->ABORT_BDR_bkwd = NULL;
                                        dev_ptr->ABORT_BDR_fwd =
                                           adp_str.ABORT_BDR_head;
                                        adp_str.ABORT_BDR_head = dev_ptr;
                                    }

				    /* Use i to hold whether no commands are active */
				    if (adp_str.DEVICE_ACTIVE_head == NULL)
					i = TRUE;
				    else
					i = FALSE;

    				    /*
     				     * If there is a bp associated with the abort/bdr,
     				     * place it back on the active queue, to be handled
     				     * when the abort/bdr (or bus reset) is resolved.
     				     */

    				    if (com_ptr->bp)
        				p720_enq_active(dev_ptr, com_ptr);

                                    /* if no commands are outstanding, the */
                                    /* scsi bus is hung. Attempt a scsi bus */
                                    /* reset to clear up this condition.   */
                                    if (i || (dev_ptr->retry_count >=
                                         PSC_RETRY_COUNT))
                                    {
                                        p720_command_reset_scsi_bus(eaddr);
                                    }
                                    else
                                    {
                                        /* restart IOWAIT routine */
                                        if (p720_write_reg(DSP, DSP_SIZE,
                                            word_reverse(adp_str.SCRIPTS[0].
                                            dma_ptr), eaddr) == REG_FAIL)
                                            p720_cleanup_reset(REG_ERR, eaddr);
                                    }
                                    break;

                                    /* for any phase or unknown message */
                                    /* error do an abort */
                                case A_phase_error:
                                    TRACE_1 ("Aphaserr", (int) dev_ptr)
                                case A_err_not_ext_msg:
                                    TRACE_1 ("not ext ", (int) dev_ptr)
                                case A_unknown_msg:
                                    DEBUG_0 ("p720_intr: MSG_ERR Phas_error\n")
                                    TRACE_1 ("unk msg ", (int) dev_ptr)
                                    w_stop(&dev_ptr->dev_watchdog.dog);

                                    if ((bp != NULL) &&
                                        (!(bp->bufstruct.b_flags & B_ERROR)))
                                    {   /* there's no previous error so log */
                                        /* this one.                        */
                                        switch (dma_stat)
                                        {       /* for command logging */
                                        case A_phase_error:
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                        PHASE_ERROR, 90, 0,
                                                        com_ptr, TRUE);
                                            break;
                                        case A_err_not_ext_msg:
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                        ERROR_NOT_EXTENT, 90,
                                                        0, com_ptr, TRUE);
                                            break;
                                        case A_unknown_msg:
#ifdef P720_TRACE
 					    target_script = (uint *) adp_str.
						SCRIPTS[dev_ptr->
						sid_ptr->script_index].
						script_ptr;
         				    TRACE_1 ("unk msg2", (target_script
					       [Ent_cmd_msg_in_buf / 4] >> 24))
#endif
                                        default:
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                        UNKNOWN_MESSAGE, 90,
                                                        0, com_ptr, TRUE);
                                            break;
                                        }
                                    }

                                    if ((dev_ptr->device_state
                                         == ABORT_IN_PROGRESS) ||
                                        (dev_ptr->device_state
                                         == BDR_IN_PROGRESS))
                                    {
                                        /* remove device from active queue */
                                        /* if there is a command tagged to
                                           this */
                                        if (bp != NULL)
                                        {
                                            if (!(bp->bufstruct.b_flags &
                                                  B_ERROR))
                                            {   /* no previous error */
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
                                        p720_command_reset_scsi_bus(eaddr);
                                        break;
                                    }
				    /* at this point in the processing, we got 
				     * an error for a command other than a bdr 
				     * or abort, and logged it if needed.  Now 
				     * we issue an abort, unless a BDR is 
				     * already pending, in which case we issue 
				     * the BDR.
				    */
        			    dev_index = INDEX(dev_ptr->scsi_id, 
					              dev_ptr->lun_id);
				    if (!(dev_ptr->flags & SCSI_BDR))
				    {
                                        dev_ptr->device_state =
                                                      ABORT_IN_PROGRESS;
					/* note we are still on the active q */
                                        if (p720_issue_abort_script(
                                           (uint *) adp_str.SCRIPTS
                                           [dev_ptr->sid_ptr->script_index].
					   script_ptr, (uint *) 
					   dev_ptr->sid_ptr->dma_script_ptr, 
					   com_ptr, dev_index, save_isr
					   & CONNECTED, eaddr) == REG_FAIL)
                                        {
                                            p720_cleanup_reset(REG_ERR, eaddr);
                                            break;
                                        }
                                        dev_ptr->flags |= SCSI_ABORT;
                                        dev_ptr->dev_watchdog.dog.restart =
                                                          LONGWAIT;
                                        dev_ptr->flags &= ~RETRY_ERROR;
                                        dev_ptr->retry_count = 1;
                                        w_start(&dev_ptr->dev_watchdog.dog);
                                        /* mark the bp */
                                        if ((bp != NULL)
                                            && (!(bp->bufstruct.b_flags &
                                                  B_ERROR)))
                                        {
                                            bp->bufstruct.b_resid =
                                                    bp->bufstruct.b_bcount;
                                            bp->status_validity = 
						    SC_ADAPTER_ERROR;
                                            bp->general_card_status =
                                                    SC_SCSI_BUS_FAULT;
                                            bp->bufstruct.b_flags |= B_ERROR;
                                            bp->bufstruct.b_error = EIO;
                                        }
			
				 	/* if an abort was already queued */
					/* then dequeue it. */
                                        if ((adp_str.ABORT_BDR_head == dev_ptr)
					     || (dev_ptr->ABORT_BDR_bkwd 
								!= NULL))
					    p720_deq_abort_bdr(dev_ptr);
				    }
				    else /* a BDR has been requested */
				    {
					if (dev_ptr != 
					    dev_ptr->sid_ptr->bdring_lun)
					{
					    dev_ptr =
					        dev_ptr->sid_ptr->bdring_lun;
            					com_ptr = &adp_str.command[
						    adp_str.ddi.card_scsi_id 
						    << 3];
            					com_ptr->bp = NULL;
            					com_ptr->device = dev_ptr;
            					p720_enq_active(dev_ptr, 
								com_ptr);
        				}

        				if (p720_issue_bdr_script((uint *) 
					    adp_str.SCRIPTS[dev_ptr->sid_ptr->
					    script_index].script_ptr, (uint *) 
					    dev_ptr->sid_ptr->dma_script_ptr,
                 			    dev_index, com_ptr, save_isr & 
					    CONNECTED, eaddr) == REG_FAIL)
        				{
            				    p720_cleanup_reset(REG_ERR, eaddr);
            				    TRACE_1 ("outissB", REG_FAIL)
            				    break;
        				}
        				dev_ptr->device_state = 
						BDR_IN_PROGRESS;
        				dev_ptr->dev_watchdog.dog.restart = 
						LONGWAIT;
        				dev_ptr->flags &= ~RETRY_ERROR;
        				dev_ptr->retry_count = 1;
        				w_start(&dev_ptr->dev_watchdog.dog);
        				p720_deq_abort_bdr(dev_ptr);
				    }

				    /* any ioctl sleeper will be wakened at */
				    /* the completion of the abort or bdr.  */

                                    break;

                                case A_unknown_reselect_id:
                                    DEBUG_0 ("p720_intr: unknwn_reselect.\n")
                                    TRACE_1 ("unkn id ", 0)
                                    /* a device reselected us, but we can't*/
                                    /* figure out who it is, including the */
                                    /* possibility that the id is the same */
                                    /* as our own.                         */
            			    p720_logerr(ERRID_SCSI_ERR10, 
					       UNKNOWN_RESELECT_ID, 103, 0, 
					       NULL, TRUE);
            			    p720_command_reset_scsi_bus(eaddr);
                                    break;

                                case A_uninitialized_reselect:
                                    /* an unconfigured device is trying to */
                                    /* select us (spurious interrupt) or a */
                                    /* device which was aborted or BDR'ed  */
                                    /* is trying to reselect us and is to  */
                                    /* trying finish off a cmd.            */
                                    DEBUG_0 ("p720_intr: uninit_reselect.\n")
                                    TRACE_1 ("bad resl", 0)
                                    p720_logerr(ERRID_SCSI_ERR10,
                                               UNINITIALIZED_RESELECT, 105,
                                               0, NULL, TRUE);
                                    p720_command_reset_scsi_bus(eaddr);
                                    break;

                                default:
                                    /* unknown interrupt flag.  This should*/
                                    /* not happen. If it does, we have a   */
                                    /* fatal error which we must back out  */
                                    /* of.                                 */
                                    DEBUG_0 ("p720_intr: SCRIPT err dflt \n")
                                    TRACE_1 ("default ", (int) dev_ptr)
                                    p720_logerr(ERRID_SCSI_ERR2,
                                               ADAPTER_INT_ERROR, 110, 0,
                                               NULL, TRUE);
                                    rc = p720_read_reg(DSP, DSP_SIZE, 
								eaddr);
				    TRACE_1 ("default ", rc)
                                    (void) p720_write_reg(DSP, DSP_SIZE, rc,
							  eaddr);
                                    break;

                                }       /* end script switch */
                            }   /* end script interrupt (SIR) */
                            /******************************************/
                            /*      ABORT CAUSED THIS INTERRUPT       */
                            /******************************************/
                            else if (save_dsr & DABRT)
                            {
                                DEBUG_0 ("p720_intr: Abort caused int\n")
                                /* interrupt cleared up above */
                                TRACE_1 ("abrt int", 0)
				/* With the use of SIGP, the interrupt */
				/* handler no longer expects to see an */
				/* ABRT interrupt.  Just assume the    */
				/* chip has been stopped gracefully... */
                                p720_logerr(ERRID_SCSI_ERR2,
                                   ADAPTER_INT_ERROR, 115, 0, NULL, TRUE);
                                dev_ptr = NULL;
                                start_new_job = TRUE;
                            }
                            /**************************************/
                            /*    ILLEGAL INSTRUCTION DETECTED    */
                            /**************************************/
                            else if (save_dsr & IID)
                            {
                                DEBUG_0 ("p720_intr: Ig Inst int\n")
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
                                rc = p720_read_reg(DCMD,DCMD_SIZE, eaddr);
                                TRACE_1 ("igl inst", rc)
                                if (rc == REG_FAIL)
                                    p720_cleanup_reset(REG_ERR, eaddr);
                                else
                                {
                                    if (rc == 0x48)
                                    {
                                        if ((rc = p720_read_reg(DSP, DSP_SIZE, 
						  eaddr)) == REG_FAIL)
                                            p720_cleanup_reset(REG_ERR, eaddr);
                                        else if (p720_write_reg(DSP, DSP_SIZE, 
						  rc, eaddr) == REG_FAIL)
                                            p720_cleanup_reset(REG_ERR, eaddr);
                                    }
                                    else
                                    {
                                        dev_ptr = NULL;
                                        start_new_job = TRUE;
                                        p720_logerr(ERRID_SCSI_ERR2,
                                            ADAPTER_INT_ERROR, 120, 0, NULL, 
					    TRUE);
                                    }
                                }
                            }
                            else if (save_dsr & BF)
                            /**************************************/
                            /*      HOST BUS FAULT INTERRUPT      */
                            /*          (should not occur)        */
                            /**************************************/
                            {  
                                p720_logerr(ERRID_SCSI_ERR2,
                                      ADAPTER_INT_ERROR, 125, 0, NULL, TRUE);
                                p720_cleanup_reset(HBUS_ERR, eaddr);
                            }
                            /**************************************/
                            /*        HOST PARITY INTERRUPT       */
                            /*      WATCH DOG TIMER INTERRUPT     */
                            /*        SINGLE STEP INTERRUPT       */
                            /*         (none should occur)        */
                            /**************************************/
                            else if ((save_dsr & HPE) || (save_dsr & WTD) 
				  || (save_dsr & SSI))
                            {
                                if (save_dsr & HPE)
                                {
                                    DEBUG_0 ("p720_intr: hp\n")
                                    TRACE_1 ("host parity", 0)
                                    p720_logerr(ERRID_SCSI_ERR2,
                                      ADAPTER_INT_ERROR, 126, 0, NULL, TRUE);
                                }
                                else if (save_dsr & WTD)
                                {
                                    DEBUG_0 ("p720_intr: wtd\n")
                                    TRACE_1 ("watch tm", 0)
                                    p720_logerr(ERRID_SCSI_ERR2,
                                      ADAPTER_INT_ERROR, 127, 0, NULL, TRUE);
                                }
				else
                                {
                                    DEBUG_0 ("p720_intr: wtd\n")
                                    TRACE_1 ("watch tm", 0)
                                    p720_logerr(ERRID_SCSI_ERR2,
                                      ADAPTER_INT_ERROR, 128, 0, NULL, TRUE);
				}

                                dev_ptr = NULL;
                                start_new_job = TRUE;
                            }
                        }       /* if ISR & DIP */
                        /*******************************************/
                        /*        THIS IS A SCSI INTERRUPT         */
                        /*******************************************/
                        /* in case of DIP and SIP will fall here */
                        else if (save_isr & SIP)
                        {
                            /* read scsi status register 0 for cause int */
                            rc = p720_read_reg(SIST, SIST_SIZE, eaddr);
                            DEBUG_1 ("CHIP INTR: rc is 0x%x\n", rc);
                            if (rc == REG_FAIL)
                            {
                                /* clean up errors and clear interrupts */
                                p720_cleanup_reset(REG_ERR, eaddr);
                            }
                            else
                            {
                                /* Arrange scsi_stat and mask */
			        /* out the non-fatal interrupts.   */
                                scsi_stat = (word_reverse(
                                        (ulong) rc) >> 16) & SIEN_MASK;
                                DEBUG_1 ("CHIP INTR is 0x%x\n", scsi_stat)
                                dev_ptr = NULL;
                                check_for_disconnect = TRUE;
				com_ptr = &adp_str.command[tag];
                                TRACE_3("sist", (int) tag, (int) scsi_stat, 0)
                                TRACE_3("sist", (int) tag, (int) com_ptr, 
                                                (int) com_ptr->device)
                            scsi_int:
                                switch (scsi_stat)
                                {

                                    case PHASE_MIS:
                                        DEBUG_0 ("p720_intr: PHASE_MIS\n")
                                        if (p720_verify_tag(com_ptr, eaddr))
                                        {
					    dev_ptr = com_ptr->device;
                                            TRACE_1 ("phase ms", (int) dev_ptr)
                                            p720_update_dataptr(
                                               adp_str.SCRIPTS[dev_ptr->
                                               sid_ptr->script_index].
					       script_ptr, dev_ptr, tag, eaddr);
                                            /* clear DMA and SCSI FIFOs */
                                            if ((p720_write_reg(CTEST3,
                                                CTEST3_SIZE, 0x04, eaddr) 
					        == REG_FAIL) ||
                                               (p720_write_reg(STEST3,
                                                STEST3_SIZE, 0x92, eaddr) 
					        == REG_FAIL))
                                            {
                                                p720_cleanup_reset(REG_ERR,
								   eaddr);
                                                break;
                                            }

                                            ISSUE_MAIN_AFTER_NEG(dev_ptr->
					     sid_ptr->dma_script_ptr, eaddr);
                                        }
                                        else
                                        {
                                            TRACE_1 ("phase ms", 0)
                                            DEBUG_0 ("p720_intr: PH_MIS /n")
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                        PHASE_ERROR, 130, 0,
                                                        NULL, TRUE);
                                            p720_cleanup_reset(PHASE_ERR, 
							       eaddr);
                                        }
                                        break;

                                    case SCSI_SEL_TIMEOUT:
                                        DEBUG_1 ("p720_intr: sel to 0x%x\n",
                                                   scsi_stat)
                                        if (p720_verify_tag(com_ptr, eaddr))
                                        {
					    dev_ptr = com_ptr->device;
                                            TRACE_1 ("sel time out",
                                                      (int) dev_ptr)
                                            if (check_for_disconnect)
						/* issue an abort if there is
						 * more than 1 active command
						 */
                                                start_new_job =
                                                    p720_scsi_interrupt(
							com_ptr,
                                                        dev_ptr, scsi_stat,
                                                        ((dev_ptr->active_head 
							!= dev_ptr->active_tail)
							 | DISC_PENDING),
							eaddr);
                                            else
                                                start_new_job =
                                                    p720_scsi_interrupt(
						       com_ptr, dev_ptr, 
						       scsi_stat,
                                                       (dev_ptr->active_head 
						       != dev_ptr->active_tail),
                                                       eaddr);
                                        }
                                        else
                                        {
                                            DEBUG_0 ("p720_intr:s to dev\n")
                                            TRACE_1 ("sel time out", 0)
                                            /* clean active queue */
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                    UNEXPECTED_SELECT_TIMEOUT,
                                                    135, 0, NULL, TRUE);
                                            p720_cleanup_reset(DEVICE_ERR,
							       eaddr);
                                        }
                                        break;

                                    case SCSI_GROSS_ERR:
                                        DEBUG_1 ("p720_intr:gross 0x%x\n",
                                                    scsi_stat)
                                    case SCSI_PARITY_ERR:
                                        DEBUG_1 ("p720_intr:parity 0x%x\n",
                                                    scsi_stat);
                                        if (p720_verify_tag(com_ptr, eaddr))
                                        {
					    dev_ptr = com_ptr->device;
                                            TRACE_1 ("par err ", (int) dev_ptr)
                                            start_new_job =
                                                p720_scsi_interrupt(com_ptr,
						    dev_ptr, scsi_stat, 
						    TRUE, eaddr);
                                        }
                                        else
                                        {
                                            DEBUG_0 ("p720_intr: no dev\n")
                                            TRACE_1 ("par err ", 0)
                                            /* clean active queue */
                                            if (scsi_stat ==
                                                 SCSI_GROSS_ERR)
                                            {
                                                p720_logerr(ERRID_SCSI_ERR10,
                                                        GROSS_ERROR, 140, 0, 
						        NULL, TRUE);
                                                p720_cleanup_reset(DEVICE_ERR,
							       eaddr);
                                            }
                                            else    /* parity error */
                                            {
                                                p720_logerr(ERRID_SCSI_ERR10,
                                                        BAD_PARITY_DETECTED,
                                                        140, 0, NULL, TRUE);
                                                p720_cleanup_reset(HOST_ERR,
							       eaddr);
                                            }
                                        }
                                        break;

                                    case SCSI_UNEXP_DISC:
                                        DEBUG_1 ("p720_intr: unexp 0x%x\n",
                                                        scsi_stat);
					if ((save_isr & CONNECTED) &&
					    ((word_reverse(rc) >> 16) & SCSI_RESELECTED))
					{
                			    p720_sel_glitch(com_ptr, save_isr, eaddr);
                			    start_new_job = FALSE;
               				} /* end if chip glitch */

                                        else if (p720_verify_tag(com_ptr, eaddr))
                                        {
					    dev_ptr = com_ptr->device;
                                            TRACE_1 ("uex dis ", (int) dev_ptr)
                                            start_new_job =
                                                p720_scsi_interrupt(
						    com_ptr,
                                                    dev_ptr, scsi_stat,
                                                    TRUE, eaddr);
                                        }
                                        else
                                        {
                                            DEBUG_0 ("p720_intr:  no dev\n")
                                            TRACE_1 ("uex dis ", (int) dev_ptr)
                                            /* clean active queue */
                                            p720_logerr(ERRID_SCSI_ERR10,
                                                    UNEXPECTED_BUS_FREE,
                                                    140, 0, NULL, TRUE);
                                            p720_cleanup_reset(DISC_ERR,
							       eaddr);
                                        }
                                        break;

                                    case SCSI_RST:
                                        DEBUG_0 ("p720_intr: int reset.\n")
                                        TRACE_1 ("RESET!!!", 0)
                                        w_stop(&(adp_str.reset_watchdog.dog));
                                        /* clean active queue */
                                        p720_scsi_reset_received(eaddr);
                                        p720_logerr(ERRID_SCSI_ERR10,
                                                SCSI_BUS_RESET, 145, 0, NULL, 
						TRUE);
                                        break;

                                    default:
                                        DEBUG_1 ("p720_intr:int dflt0x%x\n",
                                                scsi_stat)
                                        TRACE_1 ("scsi dft", scsi_stat)
                                        /* if here we probably have 2  */
                                        /* interrupt bits set.  Other  */
                                        /* case would be if at least   */
                			/* 1 of the masked interrupt   */
				  	/* bits is set.  We process    */
    					/* the most severe interrupt   */
				        /* and continue processing.    */
                                        if (scsi_stat & SCSI_RST)
                                        {
                                            /* on reset throw away the */
                                            /* others and execute */
                                            scsi_stat = SCSI_RST;
                                            goto scsi_int;
                                        }

				        /* chip glitch work-around */
					if ((scsi_stat & SCSI_UNEXP_DISC) &&
					    ((word_reverse(rc) >> 16) & SCSI_RESELECTED) &&
					    (save_isr & CONNECTED))
					{
                			    TRACE_1("dft glitch", scsi_stat)
                			    scsi_stat = SCSI_UNEXP_DISC;
                			    goto scsi_int;
             				}

                                        if (scsi_stat & SCSI_UNEXP_DISC)
                                        {
                                            check_for_disconnect = FALSE;
                                            scsi_stat = (scsi_stat &
                                                    ~SCSI_UNEXP_DISC);
                                            goto scsi_int;
                                        }
                                        if ((scsi_stat & SCSI_PARITY_ERR)
					  || (scsi_stat & SCSI_GROSS_ERR))
                                        {   /* insure that a        */
                                            /* SCSI_PARITY_ERR or   */
                                            /* SCSI_GROSS_ERR gets  */
                                            /* processed as SCSI_   */
                                            /* PARITY_ERR case.     */
                                            scsi_stat = SCSI_PARITY_ERR;
                                            goto scsi_int;
                                        }
                                        if (scsi_stat & PHASE_MIS)
                                        {
                                            scsi_stat = PHASE_MIS;
                                            goto scsi_int;
                                        }
                                        if (scsi_stat & SCSI_SEL_TIMEOUT)
                                        {
                                            scsi_stat = SCSI_SEL_TIMEOUT;
                                            goto scsi_int;
                                        }
                                        /* We should never get here.   */
                                        p720_logerr(ERRID_SCSI_ERR2,
                                                    UNKNOWN_ADAPTER_ERROR,
                                                    160, 0, NULL, TRUE);
                                        start_new_job = TRUE;
                                        break;
                                }       /* end of scsi interrupt switch */
                            }
                        }   /* end else if end of SCSI interrupt (SIP) */
                            /* if DIP or SIP bits of ISTAT are not set  */
                            /* this implies that this error caused by   */
                            /* spurious interrupt log soft error        */
                        else
                        {
                            DEBUG_0 ("p720_intr: Spurious error \n")
                            TRACE_1 ("spurious", save_isr)
                            p720_logerr(ERRID_SCSI_ERR2, ADAPTER_INT_ERROR,
                                       170, 0, NULL, TRUE);
                        }
                        /*********************************************/
                        if (start_new_job)
                        {
                            command_issued_flag = p720_issue_cmd(eaddr);
    			    if (adp_str.REQUEST_WFR_head != NULL)
                                p720_check_wfr_queue(command_issued_flag,
				                     eaddr);
			    else if (!command_issued_flag && 
				     (adp_str.DEVICE_ACTIVE_head != NULL))
				p720_write_reg(DSP, DSP_SIZE, word_reverse(
					       adp_str.SCRIPTS[0].dma_ptr),
					       eaddr);
                        }
                    }
                    else
                    {   /* read of the ISTAT failed */
                        TRACE_1 ("bad Rist", 0)
                        TRACE_1 ("out intr", 0)
                        p720_cleanup_reset(REG_ERR, eaddr);
                    }
/* code to handle failed d_complete has been commented out.
*               }
*               else
*               {       #* DMA error *#
*                   DEBUG_0 ("Leaving p720_intr: DMA FAILED \n")
*                   TRACE_1 ("dma err ", 0)
*                   TRACE_1 ("out intr", 0)
*                   p720_logerr(ERRID_SCSI_ERR2, DMA_ERROR, 175,
*                              (uint) rc, NULL, TRUE);
*                   (void) p720_read_reg(DSTAT, DSTAT_SIZE, eaddr);
*                   (void) p720_read_reg(SIST, SIST_SIZE, eaddr);
*                   p720_cleanup_reset(DMA_ERR, eaddr);
*               }
*/
            }
            else
            {   /* an interrupt was received with no command active */
                DEBUG_1 ("Leaving p720_intr: rc=%d\n", rc);
                /* If an epow has occurred, a reset may have been sent */
                /* by the epow handler.  If there are commands this    */
                /* will be handled by the scsi reset cleanup.  If not, */
                /* it will hit this code and have the watchdog reset.  */
                save_isr = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
                TRACE_1 ("no cmd", (int) save_isr)
                (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr);
                if (save_isr & DIP)
                {
                    (void) p720_read_reg(DSTAT, DSTAT_SIZE, eaddr);
                }
                if (save_isr & SIP)
                {
                    save_sist = p720_read_reg(SIST, SIST_SIZE, eaddr);
		    TRACE_1("sip", (int) save_sist)
                    if (save_sist == REG_FAIL)
                    {
                        DEBUG_0 ("p720_intr: bad read of SIST\n")
			TRACE_1 ("bad Rsist", 0)
			TRACE_1 ("out intr", 0)
			p720_cleanup_reset(REG_ERR, eaddr);
		    }
                    else if ((word_reverse((ulong) save_sist) >> 16) & SCSI_RST)
                    {
                        w_stop(&(adp_str.reset_watchdog.dog));
                        /* clean active queue */
                        p720_scsi_reset_received(eaddr);
                        p720_logerr(ERRID_SCSI_ERR10, SCSI_BUS_RESET,
                                   177, 0, NULL, TRUE);
                    }
                }
            }
        }
        else
        {       /* this is not our interrupt */
            DEBUG_1 ("Leaving p720_intr: not our int. SCSIRS 0x%x\n", rc);
            TRACE_1 ("not ours", 0)
            TRACE_1 ("out intr", 0)
	    BUSIO_DET(eaddr);
            DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, adp_str.devno);
            return (INTR_FAIL);
        }
    }
    else        /* register fail or channel check in SCSIRS */
    {
        DEBUG_1 ("Leaving p720_intr: SCSIRS error 0x%x\n", rc);
        TRACE_1 ("bad SCIR", rc)
        if (rc == REG_FAIL)
            p720_logerr(ERRID_SCSI_ERR2, PIO_RD_DATA_ERR, 180,
                       SCSIRS, NULL, TRUE);
        else
            p720_logerr(ERRID_SCSI_ERR2, CHANNEL_CHK_ERROR, 180,
                       SCSIRS, NULL, TRUE);
        (void) p720_write_reg(SCSIRS, SCSIRS_SIZE, (rc | 0x1000000),
			      eaddr);
        p720_cleanup_reset(REG_ERR, eaddr);
    }
    DEBUG_0 ("Leaving p720_intr\n")
    TRACE_1 ("out intr", 0);
    BUSIO_DET(eaddr);
    i_reset(&(adp_str.intr_struct));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, adp_str.devno);
    return (INTR_SUCC);
}  /* end of p720_intr      */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_issue_cmd                                          */
/*                                                                      */
/* FUNCTION:    issues a waiting command, or ABORT/BDR command          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      int return value                                                */
/*      TRUE - command issued all is well                               */
/*      FALSE - command not issued all is not well                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*                                                                      */
/************************************************************************/
int
p720_issue_cmd(caddr_t eaddr)
{
    struct dev_info *dev_ptr;
    struct sc_buf *bp;
    struct cmd_info *com_ptr;
    uint  * target_script;   /* used in patching SCRIPT */
    int     dev_index;  /* index into device hash table */
    int     i;  /* loop counter */
    uchar   chk_disconnect;

    DEBUG_0 ("Entering p720_issue_cmd routine\n")
    TRACE_1 ("in issue", 0)

    /* if abort/bdr queue not empty then go to work  */
    dev_ptr = adp_str.ABORT_BDR_head;
    /* loop while Q not empty */
    while ((dev_ptr != NULL) && (adp_str.epow_state != EPOW_PENDING))
    {
        if (dev_ptr->flags & SCSI_BDR)
        {
	    /* this dev_ptr could be on the queue for an abort to a lun. */
	    /* access the dev_ptr associated with the bdr. */
 	    dev_ptr = dev_ptr->sid_ptr->bdring_lun;

            dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);

            dev_ptr->device_state = BDR_IN_PROGRESS;
            com_ptr = &adp_str.command[adp_str.ddi.card_scsi_id << 3];

	    /* Note the assumption that an item on the abort/bdr queue has no 
	     * associated command structure - is this true for command 
	     */
            com_ptr->bp = NULL;
            com_ptr->device = dev_ptr;
  	    com_ptr->resource_state = NO_RESOURCES_USED;
            com_ptr->in_use = TRUE;

            if (p720_issue_bdr_script((uint *) adp_str.SCRIPTS
                    [dev_ptr->sid_ptr->script_index].script_ptr,
                    (uint *) dev_ptr->sid_ptr->dma_script_ptr,
                    dev_index, com_ptr, FALSE, eaddr) == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
                TRACE_1 ("out issu", 0)
                return (TRUE);
            }

            TRACE_1 ("issue bd", (int) dev_ptr)
            p720_deq_abort_bdr(dev_ptr);
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

            p720_enq_active(dev_ptr, com_ptr);

            /* the timer is restarted from it's last */
            /* point to eventually time out the cmd  */
            /* if all else fails.                    */
            DEBUG_0 ("p720_issue_cmd: leaving TRUE\n")
            TRACE_1 ("out issu", (int) dev_ptr)
            return (TRUE);      /* cmds issued */
        }   /* end if a BDR */
	else /* else an abort */
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
            dev_ptr->device_state = ABORT_IN_PROGRESS;
            com_ptr = &adp_str.command[adp_str.ddi.card_scsi_id << 3];

            com_ptr->bp = NULL;
            com_ptr->device = dev_ptr;
  	    com_ptr->resource_state = NO_RESOURCES_USED;
            com_ptr->in_use = TRUE;
            dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);

            if (p720_issue_abort_script((uint *) adp_str.SCRIPTS
                [dev_ptr->sid_ptr->script_index].script_ptr,
                (uint *) dev_ptr->sid_ptr->dma_script_ptr,
                com_ptr, dev_index, FALSE, eaddr) == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
                return (TRUE);
            }

	    /* dequeue from the abort_bdr q, unless a BDR is pending and */
	    /* it isn't anchored on this dev_ptr */
	    if ((!(dev_ptr->flags & SCSI_BDR)) ||
	      ((dev_ptr->flags & SCSI_BDR) && (dev_ptr->sid_ptr->bdring_lun != 
					dev_ptr)))
                p720_deq_abort_bdr(dev_ptr);
            p720_enq_active(dev_ptr, com_ptr);
            w_start(&dev_ptr->dev_watchdog.dog);
            DEBUG_0 ("p720_issue_cmd: leaving TRUE\n")
            return (TRUE);  /* cmds issued */
        }   /* end else an abort */
        dev_ptr = adp_str.ABORT_BDR_head;
    }   /* end while loop */

    /* is there something on the waiting queue */
    com_ptr = adp_str.DEVICE_WAITING_head;
                                 
    if ((com_ptr != NULL) && (adp_str.epow_state != EPOW_PENDING))
    {
        bp = com_ptr->bp;
	dev_ptr = com_ptr->device;

	/* trace hook to indicate new command for adapter */
        DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, adp_str.devno,
            bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
            bp->bufstruct.b_bcount);

        /* if we haven't negotiated */
        if (!(dev_ptr->sid_ptr->negotiate_flag))
        {
            chk_disconnect = bp->scsi_command.flags & SC_NODISC;
            if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                p720_set_disconnect(dev_ptr, chk_disconnect);

            if ((dev_ptr->sid_ptr->tag_flag != com_ptr->tag) || 
                    (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
	            (dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                p720_patch_tag_changes(com_ptr, bp->q_tag_msg);
            
            p720_prep_main_script(com_ptr, (uint *) adp_str.SCRIPTS[
		    dev_ptr->sid_ptr->script_index].script_ptr, 
		    dev_ptr->sid_ptr->dma_script_ptr);
            ISSUE_MAIN_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, eaddr);
            dev_ptr->device_state = CMD_IN_PROGRESS;
        }
        else if (!(bp->scsi_command.flags & SC_ASYNC))
        {
            /* setup chip to exec the negotiate script */
            chk_disconnect = bp->scsi_command.flags & SC_NODISC;
            if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                p720_set_disconnect(dev_ptr, chk_disconnect);

            if ((dev_ptr->sid_ptr->tag_flag != com_ptr->tag) || 
                    (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
                    (dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                p720_patch_tag_changes(com_ptr, bp->q_tag_msg);

            ISSUE_NEGOTIATE_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr,
		  eaddr);
            dev_ptr->device_state = NEGOTIATE_IN_PROGRESS;
        }
        else
        {
            /* Patch the SCRIPT for async transfers. */
          
	    target_script = (uint *) adp_str.SCRIPTS
	        [dev_ptr->sid_ptr->script_index].script_ptr;
            TRACE_1 ("iss cmd asy", 0)
            for (i=0; i<4; i++)
            {
                target_script[R_sxfer_patch_Used[i]] =
                    word_reverse(SXFER_ASYNC_MOVE);
                target_script[R_scntl1_patch_Used[i]] =
                    word_reverse(SCNTL1_EOFF_MOVE);
                target_script[R_scntl3_patch_Used[i]] =
                    word_reverse(SCNTL3_SLOW_MOVE);
	    }

            chk_disconnect = bp->scsi_command.flags & SC_NODISC;
            if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                p720_set_disconnect(dev_ptr, chk_disconnect);

            if ((dev_ptr->sid_ptr->tag_flag != com_ptr->tag) || 
                (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
	        (dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                p720_patch_tag_changes(com_ptr, bp->q_tag_msg);

            p720_prep_main_script( com_ptr,
                    (uint *) adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
                    script_ptr, dev_ptr->sid_ptr->dma_script_ptr);
            ISSUE_MAIN_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, eaddr);
            dev_ptr->device_state = CMD_IN_PROGRESS;
        }
        TRACE_1 ("issue cd", (int) dev_ptr)
        p720_deq_wait(com_ptr);

	/* if this will be the only active command for the lun, start the */
	/* command timer. */
	if (dev_ptr->active_head == NULL)
	{
	    if (bp->timeout_value == 0)
                dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
	    else
                dev_ptr->dev_watchdog.dog.restart = 
				(ulong) bp->timeout_value + 1;
            w_start(&dev_ptr->dev_watchdog.dog);
	}
        p720_enq_active(dev_ptr, com_ptr);
        DEBUG_0 ("Leaving p720_issue_cmd with TRUE\n")
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, adp_str.devno);
        TRACE_1 ("out issu", (int) dev_ptr)
        return (TRUE);  /* command issued */
    }   /* if */
    /* no command to issue.  restart io wait and return             */
    DEBUG_0 ("Leaving p720_issue_cmd with FALSE\n")
    return (FALSE);     /* command not issued */
}  /* p720_issue_cmd */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_prep_main_script                                   */
/*                                                                      */
/* FUNCTION:    Adapter Script Command Loading Routine                  */
/*      This will write the SCSI command bytes, found by looking        */
/*      at the sc_buf to be executed, to the buffer in the script.  It  */
/*	will record this address and the number of bytes to be trans-   */
/*      fered (if any) to the table indirect page.                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      dev_info structure                                              */
/*      sc_buf structure                                                */
/*                                                                      */
/* INPUTS:                                                              */
/*      *com_ptr - pointer to the cmd_info structure associated with the */
/*              sc_buf which contains the SCSI command.                 */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be loaded with the actual SCSI command to be sent to */
/*              the device.  This is the pointer to the TOP             */
/*              of the script, not to the "main" portion of the script. */
/*	script_dma_addr - the dma address of the script pointed to by   */
/*		script_vir_addr						*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_prep_main_script(
		     struct cmd_info * com_ptr,
                     uint * script_vir_addr,
                     uint script_dma_addr)
{

    char   *script_byte_ptr;
    struct move_info *move_info_ptr;
    uchar   scsi_command, tag_index;
    ulong cmd_byte_length;
    int     i;
    struct sc_buf * bp;

    bp = com_ptr->bp;
    /* Entering p720_prep_main_script */
    script_byte_ptr = (char *) script_vir_addr;

    TRACE_1 ("in prepm", (int) script_vir_addr)

    /* Load the byte length for the scsi cmds. */
    cmd_byte_length = (ulong) bp->scsi_command.scsi_length;

    /* Load the scsi command into the command buffer */
    scsi_command = bp->scsi_command.scsi_cmd.scsi_op_code;

    script_byte_ptr[Ent_cmd_buf] = scsi_command;
    script_byte_ptr[Ent_cmd_buf + 1] = bp->scsi_command.scsi_cmd.lun;
    for (i = 2; i < cmd_byte_length; i++)
        script_byte_ptr[Ent_cmd_buf + i] =
            bp->scsi_command.scsi_cmd.scsi_bytes[i - 2];
 
    DEBUG_0 ("SCSI cmd_info ^^^^^^^^^^^^^^^^^^^^^^^^^^^ SCSI COMMAND\n")
    DEBUG_2 ("op_code is       0x%x  for ID  0x%x\n",
              bp->scsi_command.scsi_cmd.scsi_op_code,
              bp->scsi_command.scsi_id);
    if (bp == NULL)
        com_ptr->max_disconnect = 0;
    else if (bp->bufstruct.b_bcount == 0)
        com_ptr->max_disconnect = 0x00100000;
    else
        com_ptr->max_disconnect = bp->bufstruct.b_bcount;

    DEBUG_1 ("The value set in max_disconnect is %x\n",
              com_ptr->max_disconnect);

    tag_index = com_ptr->tag;

    /* write the move-related information into the table indirect area */
    move_info_ptr = (struct move_info *) (
		((ulong) adp_str.IND_TABLE.system_ptr + (16 * tag_index))); 

    move_info_ptr->data_buffer_addr = word_reverse(com_ptr->dma_addr);
    move_info_ptr->data_byte_count = word_reverse(com_ptr->max_disconnect);

    move_info_ptr->cmd_buffer_addr = word_reverse(script_dma_addr + 
							Ent_cmd_buf);
    move_info_ptr->cmd_byte_count = word_reverse(cmd_byte_length);
    TRACE_2 ("cmd_info", move_info_ptr->cmd_byte_count,
                         move_info_ptr->cmd_buffer_addr);
    TRACE_3("prep", (int) com_ptr->tag, (int) script_vir_addr, 
                    (int) move_info_ptr)
    TRACE_3("prep", (int) move_info_ptr->data_byte_count, (int)
                          move_info_ptr->data_buffer_addr, (int) bp)

    /* Turn off resid subract flag, indicate command preparation complete */
    com_ptr->flags = PREP_MAIN_COMPLETE;
    com_ptr->bytes_moved = bp->bufstruct.b_bcount;
    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
    TRACE_1 ("outprepm", 0)
    return;
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_set_disconnect                                     */
/*                                                                      */
/* FUNCTION:    Adapter Script Identify Message Initialization Routine  */
/*      This function goes into the main script and patches the identify*/
/*      message to have the disconnect bit either off or on depending   */
/*      on the flags value from the command of SC_NODISC.               */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
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
p720_set_disconnect(
                   struct dev_info * dev_ptr,
                   uchar chk_disconnect)
{
    uchar  *script_ptr;

    TRACE_2 ("set disc", (int) dev_ptr, chk_disconnect)
    script_ptr = (uchar *) adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
			script_ptr;

    /* store an indicator of the patched value: */
    /*     0x80 means no disconnect privilege, */
    /*     0x00 means disconnect privilege	*/
    dev_ptr->sid_ptr->disconnect_flag = chk_disconnect;

    /* toggle bit 6 (disconnect privilege */
    script_ptr[Ent_identify_msg_buf] ^= 0x40;
    script_ptr[Ent_sync_msg_out_buf] ^= 0x40;
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_patch_tag_changes                                  */
/*                                                                      */
/* FUNCTION:    Patches Scripts to reflect changes to the tag 		*/
/*     	This function is called when it is determined that the tag has  */
/*	changed since the script was last patched.  This may also imply */
/* 	that the lun is different than what is patched.                 */
/*	Four items are handled:						*/
/*	1. the lun is patched into identify messages if it has changed  */
/*	2. if the lun changed, patch the jump to the reconnect point    */
/*	3. the identify messages are patched to handle a q_tag message	*/
/*	   and/or a new tag.  Note the whole message is patched, we 	*/
/*	   could get smarter and decide whether we just need to 	*/
/*	   change the tag itself.					*/
/*	4. the tag is patched into register move instructions		*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      com_ptr - pointer to the command information structure for      */
/*              the particular script that needs to be altered.         */
/*      q_tag_msg - the queue tag message from the sc_buf               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_patch_tag_changes(struct cmd_info * com_ptr, uchar q_tag_msg)
{
    ulong  *script_ptr;
    ulong   id_bit_mask;
    uchar lun;
    int     i, offset_to_jump;
    struct dev_info * dev_ptr;
    
    dev_ptr = com_ptr->device;
    script_ptr = (ulong *) 
		adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr;
    TRACE_3 ("ptag", (int) com_ptr->tag, (int) script_ptr, (int) dev_ptr)

    /* record the new tag being patched */
    dev_ptr->sid_ptr->tag_flag = com_ptr->tag;
    dev_ptr->sid_ptr->tag_msg_flag = q_tag_msg;
    ASSERT(q_tag_msg == SC_NO_Q || q_tag_msg == SC_SIMPLE_Q || 
	   q_tag_msg == SC_HEAD_OF_Q || q_tag_msg == SC_ORDERED_Q);

    /* determine if script is being patched for a different lun */
    if (dev_ptr->sid_ptr->lun_flag ^ dev_ptr->lun_id)
    {
        TRACE_2 ("new lun", dev_ptr->lun_id, (int) script_ptr);
	/* record the new lun being patched */
	dev_ptr->sid_ptr->lun_flag = dev_ptr->lun_id;
        lun = com_ptr->bp->scsi_command.scsi_cmd.lun >> 5;

	/* patch the lun in identify messages */
	id_bit_mask = (lun << 24);
        script_ptr[(Ent_identify_msg_buf / 4)] &= 0xF0000000;
        script_ptr[(Ent_sync_msg_out_buf / 4)] &= 0xF0000000;
        script_ptr[(Ent_identify_msg_buf / 4)] |= id_bit_mask;
        script_ptr[(Ent_sync_msg_out_buf / 4)] |= id_bit_mask;

        /* patch the jump to the script reconnect point */
        switch (dev_ptr->scsi_id)
        {
        case 0:
            offset_to_jump = E_scsi_0_lun_Used[lun];
            break;
        case 1:
            offset_to_jump = E_scsi_1_lun_Used[lun];
            break;
        case 2:
            offset_to_jump = E_scsi_2_lun_Used[lun];
            break;
        case 3:
            offset_to_jump = E_scsi_3_lun_Used[lun];
            break;
        case 4:
            offset_to_jump = E_scsi_4_lun_Used[lun];
            break;
        case 5:
            offset_to_jump = E_scsi_5_lun_Used[lun];
            break;
        case 6:
            offset_to_jump = E_scsi_6_lun_Used[lun];
            break;
        case 7:
            offset_to_jump = E_scsi_7_lun_Used[lun];
            break;
        default:
            break;
        }
	
	/* modify the iowait script */
	adp_str.SCRIPTS[0].script_ptr[offset_to_jump] = 
                word_reverse(dev_ptr->sid_ptr->dma_script_ptr + 
		Ent_script_reconnect_point);
    }
    else /* lun unchanged */
    {
        script_ptr[(Ent_identify_msg_buf / 4)] &= 0xFF000000;
        script_ptr[(Ent_sync_msg_out_buf / 4)] &= 0xFF000000;
    }

    /* complete the patches related to the message out phase */
    if (q_tag_msg != SC_NO_Q)
    {
 	id_bit_mask = 0;
        id_bit_mask |= (((ulong) (q_tag_msg + 0x1F) << 16) |
		       ((ulong) (com_ptr->tag) << 8)); 
        script_ptr[(Ent_identify_msg_buf / 4)] |= id_bit_mask;
	id_bit_mask |= 0x00000001;
        script_ptr[(Ent_sync_msg_out_buf / 4)] |= id_bit_mask;

        script_ptr[(Ent_sync_msg_out_buf / 4) + 1] = 0x03011908;

	/* patch number of bytes to move in message out - word already */
	/* reversed. */
        script_ptr[(Ent_message_loop / 4)] = 0x0300000E;	
        script_ptr[(Ent_start_sync_msg_out / 4)] =  0x0800000E;
    }
    else /* no q tag */
    {
	/* identify_msg_buf already cleared out above, and identify byte */
	/* has been patched.  Now patch the buf associated with the SDTR */
	id_bit_mask = 0x00010301;
        script_ptr[(Ent_sync_msg_out_buf / 4)] |= id_bit_mask;
	id_bit_mask = 0x19080000;
        script_ptr[(Ent_sync_msg_out_buf / 4) + 1] = id_bit_mask;

        script_ptr[(Ent_message_loop / 4)] = word_reverse((ulong) 0x0E000001);	
        script_ptr[(Ent_start_sync_msg_out / 4)] = 
					     word_reverse((ulong) 0x0E000006);	
    }

    /* Finally, patch the tag into the register move instructions */
    for (i = 0; i < S_COUNT(R_tag_patch_Used); i++)
    {
        script_ptr[R_tag_patch_Used[i]] &= word_reverse(0xffff00ff);
        script_ptr[R_tag_patch_Used[i]] |= 
 			word_reverse(((ulong) com_ptr->tag) << 8);
    }
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_update_dataptr                                     */
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
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called by the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be changed so that the dma'd information coming from */
/*              from the target device will go to a new memory location.*/
/*      *dev_ptr - pointer to the dev_info structure, used by this      */
/*              function to determine if the device is async            */
/*              by the driver head.                                     */
/*	tag - index into cmd_info array, accesses information for the    */
/*		command associated with the save data pointers msg.	*/
/*	eaddr - effective address for pio				*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_update_dataptr(
                   ulong * script_vir_addr,
                   struct dev_info * dev_ptr,
                   ulong tag,
		   caddr_t eaddr)
{
    ulong   orig_byte_transfer_request;
    ulong   dma_fifo_bytes, dbc_bytes, left_over_bytes;
    ulong new_dma_offset;
    struct sc_buf *bp;
    struct cmd_info *com_ptr;
    struct move_info *move_info_ptr;
    uchar tag_index;
    register uchar sstat0_sav, sstat2_sav;

    DEBUG_0 ("Entering p720_update_dataptr\n")

    tag_index = (uchar) tag;

    /* write the move-related information into the table indirect area */
    move_info_ptr = (struct move_info *) (
		((ulong) adp_str.IND_TABLE.system_ptr + (16 * tag_index))); 
    TRACE_2 ("updatep", tag, (int) move_info_ptr);

    dbc_bytes = (word_reverse((ulong) 
		     p720_read_reg((uint) DBC, (char) DBC_SIZE, eaddr)) >> 8);
    orig_byte_transfer_request = (word_reverse(move_info_ptr->data_byte_count));
    DEBUG_1 ("Orig bytes is %d\n", orig_byte_transfer_request)
    TRACE_3 ("updp", tag, (int) move_info_ptr, (int) orig_byte_transfer_request)
    bp = adp_str.command[tag].bp;
    if (!(bp->bufstruct.b_flags & B_READ))
    {   /* must be a send data command */
        dma_fifo_bytes = (((p720_read_reg((uint) DFIFO, (char) DFIFO_SIZE,
			    eaddr) & 0x7F) - (dbc_bytes & 0x7F)) & 0x7F);
        left_over_bytes = dma_fifo_bytes + dbc_bytes;
        sstat0_sav = p720_read_reg((uint) SSTAT0, (char) SSTAT0_SIZE, eaddr);
        sstat2_sav = p720_read_reg((uint) SSTAT2, (char) SSTAT2_SIZE, eaddr);
        if (sstat0_sav & 0x20)
            left_over_bytes++;
        if ((sstat0_sav & 0x40) && !(dev_ptr->sid_ptr->async_device))
            left_over_bytes++;
        if (sstat2_sav & 0x20)
            left_over_bytes++;
        if ((sstat2_sav & 0x40) && !(dev_ptr->sid_ptr->async_device))
            left_over_bytes++;
    }
    else
        left_over_bytes = dbc_bytes;
    new_dma_offset = bp->bufstruct.b_bcount - left_over_bytes;

    DEBUG_0 ("************************************************\n")
    DEBUG_1 ("orig req = 0x%x\n", orig_byte_transfer_request);
    DEBUG_1 ("left over = 0x%x\n", left_over_bytes);
    DEBUG_1 ("new dma off = 0x%x\n", new_dma_offset);
    DEBUG_1 ("max_disconnect BEFORE change = 0x%x\n", com_ptr->max_disconnect);

    com_ptr = &adp_str.command[tag];
    if ((com_ptr->max_disconnect > left_over_bytes) &&
        (left_over_bytes > 0))
        com_ptr->max_disconnect = left_over_bytes;

    DEBUG_1 ("max_disconnect AFTER change = 0x%x\n", com_ptr->max_disconnect);
    DEBUG_0 ("************************************************\n")

    move_info_ptr->data_buffer_addr = word_reverse(com_ptr->dma_addr +
						   new_dma_offset);
    move_info_ptr->data_byte_count = word_reverse(com_ptr->max_disconnect);
    TRACE_3("updt", (int) move_info_ptr->data_byte_count,
                          move_info_ptr->data_buffer_addr, (int) bp)

    com_ptr->bytes_moved = orig_byte_transfer_request - left_over_bytes;
    if (bp->scsi_command.flags & SC_NODISC)
    {
        /* This is a no disconnect device that needs the byte  */
        /* count subtraction done here.                        */
        bp->bufstruct.b_resid -= com_ptr->bytes_moved;
    }
    else
    {
        com_ptr->flags |= RESID_SUBTRACT;
    }
    DEBUG_1 ("NEW dma address is 0x%x\n", com_ptr->dma_addr + new_dma_offset)
    DEBUG_0 ("Leaving p720_update_dataptr\n")
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_check_wfr_queue                                    */
/*                                                                      */
/* FUNCTION:    Examine the waiting for resources queue                 */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* NOTES:                                                               */
/*  this is called to see if a cmd which is waiting for resources       */
/*  may be satisfied                                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      command_issued_flag - FALSE means no commands issued yet        */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_check_wfr_queue(uchar command_issued_flag, caddr_t eaddr)
{
    struct sc_buf *bp, *prev_bp, *current_bp;
    struct sc_buf *failed_bps_head, *failed_bps_tail;
    struct dev_info *dev_ptr;
    int tag;

    DEBUG_1 ("Entering p720_check_wfr_queue: flag=0x%x\n",
              command_issued_flag)
    TRACE_1 ("in chkwfrq", command_issued_flag)

    prev_bp = NULL;
    failed_bps_head = NULL;
    bp = adp_str.REQUEST_WFR_head;
    /* loop while Q not empty */
    while (bp != NULL)
    {
	dev_ptr = adp_str.device_queue_hash[
        INDEX(bp->scsi_command.scsi_id, (bp->scsi_command.scsi_cmd.lun) >> 5)];
 
	if (((bp == adp_str.blocked_bp) || 
	     (bp->bufstruct.b_work != LARGE_TCE_RESOURCES_USED))
	     || (adp_str.blocked_bp == NULL))
	/* if either the 1st bp needing large resources, or a bp not */
	/* needing large resources, or the 1st bp needing large resources */
	/* was already satisfied, then try to acquire resources    */
	{
	    tag = p720_alloc_resources(bp, dev_ptr);
            if (tag > PSC_ALLOC_FAILED) 
            {   /* allocation successful */

		/* save bp and advance in the list, before dequeueing */
	 	current_bp = bp;
	    	bp = (struct sc_buf *) bp->bufstruct.av_forw;
	
                p720_deq_WFR(current_bp, prev_bp);
	        dev_ptr->flags &= ~BLOCKED;

		/* place the allocated cmd on the wait queue */
                p720_enq_wait(&adp_str.command[tag]);

                if (command_issued_flag == FALSE)
                    command_issued_flag = p720_issue_cmd(eaddr);

		/* process the bp_save queue */
		if ((current_bp = p720_process_bp_save(dev_ptr)) != NULL)
		{
		    /* one of the sc_bufs on the bp_save queue failed */
		    /* allocation.  We will append it to the wfr_q    */
		    /* after we complete the current walk thru the q. */
		    /* Here we build a list, using failed_bps_head    */
		    /* and failed_bps_tail. */
		    if (failed_bps_head == NULL)
		    {
			failed_bps_tail = current_bp;
		    }
		    current_bp->bufstruct.av_forw = (struct buf *) 
					failed_bps_head;
		    failed_bps_head = current_bp;
		}

		/* bp has already been adjusted, we don't adjust prev_bp */
		/* since we dequeued an element.  Continue at top of loop */
		continue;
            }
	}

	/* we either didn't try, or failed, allocation.  Move to the next */
	/* element in the wfr_q. */
	prev_bp = bp;
	bp = (struct sc_buf *) bp->bufstruct.av_forw;	
    }   /* end while */

    /* if any allocation attempts failed after processing bp_save queues, */
    /* append the newly blocked bps onto the wfr_q.   */
    if (failed_bps_head != NULL)
    {
	TRACE_1 ("append wfr", (int) failed_bps_head)
	if (adp_str.REQUEST_WFR_head == NULL)
	{
	    adp_str.REQUEST_WFR_head = failed_bps_head;
	}
	else
	{
	    adp_str.REQUEST_WFR_tail->bufstruct.av_forw = 
				(struct buf *) failed_bps_head;
	}
	adp_str.REQUEST_WFR_tail = failed_bps_tail;
    }

    /* if the chip has not been previously restarted, and we could */
    /* expect a legal reselection, restart the iowait script. */
    if ((command_issued_flag == FALSE) &&
        (adp_str.DEVICE_ACTIVE_head != NULL))
    {
        p720_write_reg(DSP, DSP_SIZE, word_reverse(
                           adp_str.SCRIPTS[0].dma_ptr), eaddr);
        DEBUG_0 ("p720_check_wfr_queue: Issuing IOWAIT\n")
    }
    DEBUG_0 ("leaving p720_check_wfr_queue\n")
    TRACE_1 ("out chkwfrq", 0)
}  /* end of p720_check_wfr_queue */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_cleanup_reset                                      */
/*                                                                      */
/* FUNCTION:    Cleans up the active queues due to a register error or  */
/*              a dma error.                                            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts should be disabled.                                  */
/*                                                                      */
/* NOTES:                                                               */
/*  This is called when an error occurrs that requires a bus reset to   */
/*  be issued.  All commands will be cleaned up upon completion of the  */
/*  reset by p720_scsi_reset_received.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      unint  err_type                                                 */
/*            1=dma 2=register 3=phase 4=device 5=disc 6=host 7=host bus*/
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_cleanup_reset(int err_type, caddr_t eaddr)
{
    struct dev_info *dev_ptr;   /* device structure */
    struct cmd_info *com_ptr;   /* device structure */
    struct sc_buf *bp;

    DEBUG_1 ("Entering p720_cleanup_reset routine error = 0x%x\n", err_type);
    TRACE_2 ("in clres", err_type, (int) eaddr)
    ASSERT(eaddr != NULL);

    /* loop while devices are on Active queue */
    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
        /* this error may have been caused by a timeout or epow */
        /* or a glitch on the reset line                        */
	com_ptr = dev_ptr->active_head;
        while (com_ptr != NULL)
        {
	    bp = com_ptr->bp;
            /* if active cmd status not already set */
            if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR)))
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
                case HBUS_ERR:
                default:
                    bp->general_card_status = SC_HOST_IO_BUS_ERR;
                    break;
                }
                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                bp->bufstruct.b_flags |= B_ERROR;
                bp->bufstruct.b_error = EIO;
            }
	    com_ptr = com_ptr->active_fwd;
        }
        if ((adp_str.ABORT_BDR_head == dev_ptr) ||
            (dev_ptr->ABORT_BDR_bkwd != NULL))
            p720_deq_abort_bdr(dev_ptr);
        if (dev_ptr->ioctl_wakeup == TRUE)
        {
            dev_ptr->ioctl_errno = EIO;
            dev_ptr->ioctl_wakeup = FALSE;
            e_wakeup(&dev_ptr->ioctl_event);
        }
        dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
    }   /* end of while loop */
    if ((err_type == REG_ERR) || (err_type == HBUS_ERR))
    {
        p720_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA, eaddr);
        p720_command_reset_scsi_bus(eaddr);
    }
    else if (err_type == DMA_ERR)
    {
        p720_command_reset_scsi_bus(eaddr);
        p720_chip_register_reset(PSC_RESET_DMA, eaddr);
    }
    else if (err_type == DISC_ERR)
    {
        /* clear DMA and SCSI FIFOs */
        (void) p720_write_reg(CTEST3, CTEST3_SIZE, 0x04, eaddr);
        (void) p720_write_reg(STEST3, STEST3_SIZE, 0x92, eaddr);
        p720_command_reset_scsi_bus(eaddr);
    }
    else
    {
        p720_command_reset_scsi_bus(eaddr);
    }
    TRACE_1 ("out clre", err_type)
    DEBUG_0 ("leaving p720_cleanup_reset\n")
}  /* end p720_cleanup_reset */

/************************************************************************/
/*                                                                      */
/* NAME:        p720_scsi_reset_received                                */
/*                                                                      */
/* FUNCTION:    Cleans up the active queues after a scsi bus reset has  */
/*              occurred.                                               */
/*                                                                      */
/* EXECUTION ENVIRONMENT:						*/
/*  Called from interrupt handler.					*/
/*                                                                      */
/* NOTES:                                                               */
/*  After a scsi bus reset occurs, all active commands are cleaned up.  */
/*  In addition, all resources are freed and all queues are searched to */
/*  ensure command restart occurs without error.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      eaddr - effective address for pio                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_scsi_reset_received(caddr_t eaddr)
{
    struct dev_info *dev_ptr;   /* device structure */
    struct dev_info *tmp_dev_ptr;   /* device structure used for bdrs */
    struct cmd_info  *com_ptr;   /* command structure */
    struct sc_buf *bp, *tmp_bp;
    int     i, dev_index;

    DEBUG_0 ("Entering p720_scsi_reset_received routine\n")
    TRACE_1 ("in reset rcv", 0)

    /* loop while devices are on Active queue */
    while ((dev_ptr = adp_str.DEVICE_ACTIVE_head) != NULL)
    {
        /* this error may have been caused by a timeout or epow */
        /* or a glitch on the reset line                        */
        w_stop(&dev_ptr->dev_watchdog.dog);

	dev_ptr->device_state = NOTHING_IN_PROGRESS;

	/* process all active commands for the device */
        while ((com_ptr = dev_ptr->active_head) != NULL)
        {
            /* free resources for this command sta's, tce's */
            (void) p720_free_resources(com_ptr, FALSE);
            if ((bp = com_ptr->bp) != NULL)
            {
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

            dev_ptr->active_head = com_ptr->active_fwd;
	    com_ptr->active_fwd = NULL;
	    com_ptr->active_bkwd = NULL;
        }
	dev_ptr->active_tail = NULL;

        if ((adp_str.ABORT_BDR_head == dev_ptr) ||
            (dev_ptr->ABORT_BDR_bkwd != NULL))
            p720_deq_abort_bdr(dev_ptr);

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

        if (dev_ptr->flags & SCSI_BDR)
	{
	    /* make sure all luns are marked, as the bdr won't be issued */
            dev_index = dev_ptr->scsi_id << 3;
	    for (i=0; i < MAX_LUNS; i++)
	    {
                if ((tmp_dev_ptr = adp_str.device_queue_hash[dev_index + i])
                        != NULL)
                {
		    /* luns with active commands will be handled by the */
		    /* outmost while loop */
                    if (tmp_dev_ptr->active_head == NULL)
		    {
			if (tmp_dev_ptr == dev_ptr)
			    p720_fail_cmd(dev_ptr);
		        else
			/* a different lun, with no active cmds */
			{
			   tmp_dev_ptr->flags = RETRY_ERROR;
			   tmp_dev_ptr->queue_state &= STOPPING;
			   tmp_dev_ptr->queue_state |= ACTIVE;
		        }
		    }
		    else  /* mark flags so we don't hit this code again */
		        tmp_dev_ptr->flags &= ~SCSI_BDR;	
                }
            }
	}
	else
            p720_fail_cmd(dev_ptr);  /* return pending cmds */ 

        adp_str.DEVICE_ACTIVE_head = dev_ptr->DEVICE_ACTIVE_fwd;
        dev_ptr->DEVICE_ACTIVE_fwd = NULL;
        dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
    }   /* end of while loop */

    adp_str.DEVICE_ACTIVE_tail = NULL;

    /* Cleanup the abort/bdr queue */
    while ((dev_ptr = adp_str.ABORT_BDR_head) != NULL)
    {
	p720_deq_abort_bdr(dev_ptr);
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
	if (dev_ptr->flags & SCSI_BDR)
	{
	    /* no luns had active commands, if they did the flag value */
	    /* would have been reset above.  Reset the flag and queue state */
            dev_index = dev_ptr->scsi_id << 3;
	    for (i=0; i < MAX_LUNS; i++)
	    {
                if ((tmp_dev_ptr = adp_str.device_queue_hash[dev_index + i])
                        != NULL)
                {
	   	    tmp_dev_ptr->flags = RETRY_ERROR;
		    tmp_dev_ptr->queue_state &= STOPPING;
		    tmp_dev_ptr->queue_state |= ACTIVE;
		}
	    }
	}
	else /* either an abort, or a bdr for an id which had active cmds */
	    p720_fail_cmd(dev_ptr);
    }

/* Finding a command or bp at this point means there were no
* active commands (we would have called fail_cmd and returned them).
* So this code needs to find any pending commands and bps, free any associated
* resources, and park them on the correct save queue.
*/
    com_ptr = adp_str.DEVICE_WAITING_head;
    while (com_ptr != NULL)
    {
	if (com_ptr->bp->flags & SC_DELAY_CMD)
	{
	    dev_ptr = com_ptr->device;
	    /* find the next command before extracting. Upon exit of */
	    /* the loop we point to the end of a sublist about to be */
	    /* moved to the command save queue */
	    while ((com_ptr->active_fwd != NULL) &&
		   (dev_ptr == com_ptr->active_fwd->device))
	    {
		com_ptr = com_ptr->active_fwd;
	    }
	    com_ptr = com_ptr->active_fwd;
	    p720_freeze_qs(dev_ptr);
	}
	else
	    com_ptr = com_ptr->active_fwd;
    }

    bp = adp_str.REQUEST_WFR_head;
    while (bp != NULL)
    {
	if (bp->flags & SC_DELAY_CMD)
	{
	    tmp_bp = bp;
	    bp = (struct sc_buf *) bp->bufstruct.av_forw;
	    p720_freeze_qs(adp_str.device_queue_hash[INDEX(tmp_bp->
	       scsi_command.scsi_id, (tmp_bp->scsi_command.scsi_cmd.lun) >> 5)]);
	}
	else
	    bp = (struct sc_buf *) bp->bufstruct.av_forw;
    }

    /* clear the DMA and scsi fifo's on the chip */
    if ((p720_write_reg(CTEST3, CTEST3_SIZE, 0x04, eaddr) == REG_FAIL) ||
        (p720_write_reg(STEST3, STEST3_SIZE, 0x92, eaddr) == REG_FAIL))
    {   /* if there's a failure to clear the fifo, reset the chip */
        p720_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA, eaddr);
    }

    /* For all active target ids, note that a hard reset has occured */
    for (i = 0; i < MAX_SCRIPTS; i++)
    {
        if (adp_str.sid_info[i].dma_script_ptr != NULL)
        {
            adp_str.sid_info[i].negotiate_flag = TRUE;
            adp_str.sid_info[i].restart_in_prog = TRUE;
        }
    }
    adp_str.restart_watchdog.dog.restart =
        adp_str.ddi.cmd_delay + 1;
    w_start(&(adp_str.restart_watchdog.dog));
    TRACE_1 ("out resetrcv", 0)
    DEBUG_0 ("leaving p720_scsi_reset_received\n")
}  /* end p720_scsi_reset_received */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_check_qs                                                   */
/*                                                                        */
/* FUNCTION:  Examines pending queues after a hard reset.                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.					  */
/*                                                                        */
/* NOTES:  This routine checks the wait and wfr queue to see if elements  */
/* for a particular dev_ptr need to be extracted due to a hard reset.     */
/* The queue state is set to ACTIVE.					  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      tmp_dev_ptr - pointer to device structure for the reset target    */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      none                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_check_qs(struct dev_info * tmp_dev_ptr)
{
    struct cmd_info * com_ptr;
    struct sc_buf * bp;

    TRACE_1("checkqs", (int) tmp_dev_ptr)
    /* change queue state to ACTIVE */
    if (tmp_dev_ptr->queue_state & STOPPING)
    {
	/* if in fact there are still cmds in our queues, p720_stop_dev
         * is just going to go back to sleep.
         */
        e_wakeup(&tmp_dev_ptr->stop_event);
	tmp_dev_ptr->queue_state = ACTIVE | STOPPING;
    }
    else
    {
        tmp_dev_ptr->queue_state = ACTIVE;
    }

    com_ptr = tmp_dev_ptr->wait_head;
    while (com_ptr != NULL)
    {
	if (com_ptr->bp->flags & SC_DELAY_CMD)
	{
	    p720_freeze_qs(tmp_dev_ptr);
	    break;
	}
	else
	    com_ptr = com_ptr->next_dev_cmd;
    }

    if (tmp_dev_ptr->flags & BLOCKED)
    {
        bp = adp_str.REQUEST_WFR_head;
        while (bp != NULL)
        {
	    if (adp_str.device_queue_hash[INDEX(bp->scsi_command.scsi_id, 
		  (bp->scsi_command.scsi_cmd.lun) >> 5)] == tmp_dev_ptr)
	    {
	        /* found the blocked bp, freeze q if needed */
		if (bp->flags & SC_DELAY_CMD)
		    p720_freeze_qs(tmp_dev_ptr);
		break;
	    }
	    else
	    {
		bp = (struct sc_buf *) bp->bufstruct.av_forw;
	    }
	}
    }
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_unfreeze_qs                                        */
/*                                                                      */
/* FUNCTION:    Resumes processing of commands held on cmd_save or      */
/*      bp_save queue, sending a command to the chip if appropriate.    */
/*                                                                      */
/*      This function resumes the queues, handling the bp input		*/
/*      parameter if it is not null by issuing it or placing it in the  */
/*      correct queue.  						*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure with frozen queues        */
/*      bp - pointer to any sc_buf with a command to issue after the    */
/*	     queues are unfrozen.					*/
/*      check_chip - flag indicating whether it is necessary to try to  */
/*	     start the chip on an unfrozen command or bp.		*/
/*      eaddr - effective address for pio, or NULL                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      indication whether chip should be checked again                 */
/*                                                                      */
/************************************************************************/
int
p720_unfreeze_qs(struct dev_info *dev_ptr, struct sc_buf *bp, 
		 uchar check_chip, caddr_t eaddr)
{
    struct cmd_info *com_ptr;
    struct sc_buf *failed_bp;
    int ret_code = TRUE;
    uchar called_from_strategy = FALSE;

    TRACE_2("unfreez", (int) dev_ptr, (int) bp)
    if ((!(dev_ptr->flags & STARVATION)) && (dev_ptr->cmd_save_head != NULL))
    {
	/* place elements on the command save queue back on the wait queue */

	/* Update the wait list pointers */
	TRACE_2 ("cmd save", (int) dev_ptr->cmd_save_head, 
			     (int) dev_ptr->cmd_save_tail)
	com_ptr = dev_ptr->cmd_save_head;
	while (com_ptr != NULL)
	{
	    com_ptr->active_fwd = com_ptr->next_dev_cmd;
            com_ptr = com_ptr->next_dev_cmd;
	}

	/* mark the front of the sublist */
  	if (adp_str.DEVICE_WAITING_tail == NULL)
	    adp_str.DEVICE_WAITING_head = dev_ptr->cmd_save_head;
	else
	    adp_str.DEVICE_WAITING_tail->active_fwd = dev_ptr->cmd_save_head;

	/* mark the end of the list */
	adp_str.DEVICE_WAITING_tail = dev_ptr->cmd_save_tail;

	/* adjust the anchor pointers in the dev info structure */
	dev_ptr->wait_head = dev_ptr->cmd_save_head;
	dev_ptr->wait_tail = dev_ptr->cmd_save_tail;
	dev_ptr->cmd_save_head = NULL;
	dev_ptr->cmd_save_tail = NULL;
    }

    if (dev_ptr->bp_save_head != NULL)
    {
	/* process bp save will move the bps to the wait q if appropriate */
	failed_bp = p720_process_bp_save(dev_ptr); 
  	if (failed_bp != NULL)
	    p720_enq_WFR(failed_bp);
    }
        
    /* Finally, handle any command passed in */
    if (bp != NULL)
    {
	/* before we call start, need to make sure that this command can't */
	/* be started on the chip ahead of other commands for the target.  */
	/* This can happen if the active queue is empty. */
	if ((dev_ptr->bp_save_head != NULL) || (dev_ptr->flags & BLOCKED))
	{
	    dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;
	    dev_ptr->bp_save_tail = bp;
	}
	else if ((dev_ptr->wait_head == NULL) || 
		 (adp_str.DEVICE_ACTIVE_head != NULL))
	{
	    p720_start(bp, dev_ptr);
	    /* if the next cmd to be issued for this device is not bp, */
	    /* it is likely that the chip needs to be signaled that there */
	    /* is new work (start will not have done so).  Go ahead and */
	    /* do it here. */
	    if ((dev_ptr->wait_head != NULL) && (dev_ptr->wait_head->bp != bp))
	    {
	        if (eaddr == NULL)
	        {
		    called_from_strategy = TRUE;
                    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    	        }
	        TRACE_1 ("sigp unfrz", (int) dev_ptr->wait_head)
	        (void) p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr);
                if (called_from_strategy)
		    BUSIO_DET(eaddr);
	    }
	}
	else 
	{    /* calling start in this case could send the bp in front of */
	     /* the other commands for the same target.  So we first call */
	     /* issue command (to start the chip), then call start to */
	     /* handle the bp.  Note that since the active queue is empty,*/
	     /* it is safe to call issue command. */
	    if (eaddr == NULL)
	    {
		called_from_strategy = TRUE;
                eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    	    }
	    (void) p720_issue_cmd(eaddr);
            if (called_from_strategy)
		BUSIO_DET(eaddr);
	    p720_start(bp, dev_ptr);
	}
    }
    else if (check_chip)
    {
	/* routine was called from the top-half with a null bp.  We need to
	 * see whether we should try to issue a command directly or signal
	 * the chip that there is work.
	 */
	if (adp_str.DEVICE_WAITING_head != NULL)
	{
	    ret_code = FALSE;
            eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	    if (adp_str.DEVICE_ACTIVE_head == NULL)
	        (void) p720_issue_cmd(eaddr);
	    else
	        (void) p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr);
            BUSIO_DET(eaddr);
	}
    }
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_process_q_clr                                      */
/*                                                                      */
/* FUNCTION:    Handles the SC_Q_CLR flag by sending an abort.          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure with contingent alleg.    */
/*      bp - pointer to sc_buf to iodone after the queues are flushed.  */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_process_q_clr(struct dev_info *dev_ptr, struct sc_buf *bp)
{
    caddr_t eaddr;   /* effective address for pio */

    TRACE_2 ("q_clear", (int) dev_ptr, (int) bp)
    dev_ptr->flags |= SCSI_ABORT;
    p720_enq_abort_bdr(dev_ptr);

    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

    if (adp_str.DEVICE_ACTIVE_head == NULL)
        (void) p720_issue_cmd(eaddr);
    else if (adp_str.DEVICE_WAITING_head == NULL)
        (void) p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr);
    BUSIO_DET(eaddr);

    /* put the bp on the bp_save queue, it will be iodoned at */
    /* the completion of the abort. */
    bp->bufstruct.av_forw = NULL;
    if (dev_ptr->bp_save_head == NULL)
        dev_ptr->bp_save_head = bp;
    else
	dev_ptr->bp_save_tail->bufstruct.av_forw = (struct buf *) bp;
    dev_ptr->bp_save_tail = bp;
    TRACE_1("out clr", (int) dev_ptr)
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_q_full                                             */
/*                                                                      */
/* FUNCTION:    Handles the SC_QUEUE_FULL SCSI status.           r      */
/*                                                                      */
/*      This function places the failed command back on the wait queue, */
/*      and restarts the command timer.                                 */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the cmd.    */
/*      com_ptr - pointer to the command which received the status      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_q_full(struct dev_info *dev_ptr, struct cmd_info *com_ptr)
{
    struct cmd_info *trail_ptr, *work_ptr;
    struct sc_buf * bp;

   /* this function handles a queue full status by placing the unsuccessful
    * command back on to the wait queue.  The command is placed in front of
    * any other commands on the wait queue for the same target lun.  The 
    * hope is that during the time we spend processing the iodone interrupt
    * where we discovered the queue full status, the target has had an
    * opportunity to arbitrate for the bus to begin a reselection.  Additional
    * processing that could be done here if this simple approach does not work
    * well during testing would be to move all commands for the target to the
    * end of the wait_q, or perhaps better (but more complicated), remove them
    * from the wait_q and delay resubmitting them.
    */
    TRACE_2 ("q_full", (int) dev_ptr, (int) com_ptr)
    if (dev_ptr->wait_head == NULL)
    {
	/* place at end of the wait queue */
     	p720_enq_wait(com_ptr);
    }
    else  /* there is already a command for this target on the wait q */
    {
	trail_ptr = NULL;
 	work_ptr = adp_str.DEVICE_WAITING_head;
	while(work_ptr != dev_ptr->wait_head)
        {
	    trail_ptr = work_ptr;
	    work_ptr = work_ptr->active_fwd;
	}

    	if (trail_ptr == NULL)   /* command is first in list */
    	{
            adp_str.DEVICE_WAITING_head = com_ptr;
    	}
    	else 		  /* in middle or end of list */
    	{
            trail_ptr->active_fwd = com_ptr;
    	}

	dev_ptr->wait_head = com_ptr;
	com_ptr->next_dev_cmd = work_ptr;
	com_ptr->active_fwd = work_ptr;
    }

    /* now restart the timer for the active commands */
    if (dev_ptr->active_head != NULL)
    {
	bp = dev_ptr->active_head->bp;
	if (bp->timeout_value == 0)
            dev_ptr->dev_watchdog.dog.restart = (ulong) 0;
	else
            dev_ptr->dev_watchdog.dog.restart = (ulong) bp->timeout_value + 1;
        w_start(&dev_ptr->dev_watchdog.dog);
    }
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_freeze_qs                                          */
/*                                                                      */
/* FUNCTION:    Moves commands to the cmd_save and bp_save queues.      */
/*                                                                      */
/*      This function processes the pending queues after an error       */
/*      occurs.                                                         */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the cmd.    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
p720_freeze_qs(struct dev_info *dev_ptr)
{
    struct cmd_info *trail_ptr, *work_ptr, *target_ptr;
    struct sc_buf *prev_bp, *bp;
    struct dev_info *cur_dev_ptr;
    int dev_index;

    /* WAIT QUEUE */
    TRACE_1 ("freezeq", (int) dev_ptr)
    if (dev_ptr->wait_head != NULL)
    {
        /* move items from the wait_q to the cmd_save q */

        trail_ptr = NULL;
        work_ptr = adp_str.DEVICE_WAITING_head;
        target_ptr = dev_ptr->wait_head;

        /* loop until last element is found */
        while (target_ptr != NULL)
        {
    	    while(work_ptr != target_ptr)
            {
	        trail_ptr = work_ptr;
	        work_ptr = work_ptr->active_fwd;
	    }

	    /* work_ptr now equals the target pointer, which is at the head
	     * of a sublist to be extracted.  Now locate the end of the sublist,
	     * ie. the next element not for the same device (which could be 
	     * null)
	     */
            while ((work_ptr->next_dev_cmd != NULL) &&
	        (work_ptr->active_fwd == work_ptr->next_dev_cmd))
	    {
	        work_ptr = work_ptr->active_fwd;
	    }

            /* at this point trail_ptr is before the first element, and work_ptr
	     * is at the last command to be extracted in the current sublist 
	     */

            TRACE_2 ("sublist", (int) trail_ptr, (int) work_ptr)
	    /* identify the next target (might be null) */
	    target_ptr = work_ptr->next_dev_cmd;

	    if (target_ptr != NULL)
            {  /* we are going to stay in the loop, mark the front of the */
	       /* intermediate sublist */

	        /* was the first element at the head of the list ? */
	        if (trail_ptr == NULL)
	            adp_str.DEVICE_WAITING_head = work_ptr->active_fwd;
	        else
	            trail_ptr->active_fwd = work_ptr->active_fwd;
	    }
        }
        TRACE_2 ("sublist", (int) trail_ptr, (int) work_ptr)

        /* on exit of the loop, trail_ptr preceeds the last sublist extracted */
        if (adp_str.DEVICE_WAITING_tail == dev_ptr->wait_tail)
        {
	    adp_str.DEVICE_WAITING_tail = trail_ptr;
        }

        /* mark the front of the final sublist */
        if (trail_ptr == NULL)
            adp_str.DEVICE_WAITING_head = work_ptr->active_fwd;
        else
            trail_ptr->active_fwd = work_ptr->active_fwd;

        /* extracted chain is already linked, just adjust the head pointers */
        /* note that active_fwd for the extracted list is not meaningful    */
        dev_ptr->cmd_save_head = dev_ptr->wait_head; 
        dev_ptr->cmd_save_tail = dev_ptr->wait_tail; 
        dev_ptr->wait_head = NULL;
        dev_ptr->wait_tail = NULL;
    }

    TRACE_2("adp str", (int) adp_str.DEVICE_WAITING_head,
                       (int) adp_str.DEVICE_WAITING_tail)
    TRACE_2("cmd sv", (int) dev_ptr->cmd_save_head, 
		      (int) dev_ptr->cmd_save_tail)

    /* WFR QUEUE */
    if (dev_ptr->flags & BLOCKED)
    {
	/* find the blocked bp and place it at the front of the bp_save queue */
        prev_bp = NULL;
        bp = adp_str.REQUEST_WFR_head;
        while (bp != NULL)
        {
            dev_index = INDEX(bp->scsi_command.scsi_id,
                          (bp->scsi_command.scsi_cmd.lun) >> 5);
            cur_dev_ptr = adp_str.device_queue_hash[dev_index];
            if (cur_dev_ptr == dev_ptr)
            {   /* this request belongs to the device */
                p720_deq_WFR (bp, prev_bp);
                break;  /* only one command on WFR queue; we found */
                        /* it so break out the the while loop      */
            }
            else
            {   /* this request doesn't belong to the device - skip it */
                prev_bp = bp;
                bp = (struct sc_buf *) bp->bufstruct.av_forw;
            }
        }  /* end while */

	/* insert the found bp */
	bp->bufstruct.av_forw = (struct buf *) dev_ptr->bp_save_head;
        dev_ptr->bp_save_head = bp;
	if (dev_ptr->bp_save_tail == NULL)
	    dev_ptr->bp_save_tail = bp;
	dev_ptr->flags &= ~BLOCKED;
    }

}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_process_bp_save                                    */
/*                                                                      */
/* FUNCTION:    Moves commands to the cmd_save and bp_save queues.      */
/*                                                                      */
/*      This function sees if an element on the bp_save queue has       */
/*      become unblocked, and attempts allocation if so.  If resource   */
/*      allocation fails, a pointer to the failing bp is returned (no   */
/*      additional elements on the bp_save q are processed).            */
/* 									*/
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which must be called with           */
/*      interrupts disabled.                                            */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      dev_ptr - pointer to device structure associated w/ the save_q  */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      sc_buf pointer or NULL.  If not NULL, this points to a sc_buf   */
/*      for which resource allocation failed.                           */
/*                                                                      */
/************************************************************************/
struct sc_buf *
p720_process_bp_save(struct dev_info *dev_ptr)
{
    struct sc_buf *ret_bp, *bp;
    struct scsi_id_info *sid_ptr;
    int tag;
	
    ret_bp = NULL;
    TRACE_1 ("proc bpsave", (int) dev_ptr)

    /* If an epow is pending, we don't need to be processing this.  */
    if (adp_str.epow_state != EPOW_PENDING)
    {
	sid_ptr = dev_ptr->sid_ptr;
        while ((bp = dev_ptr->bp_save_head) != NULL)
        {
            TRACE_1 ("bpsave", (int) dev_ptr->bp_save_head)
	    /* if we don't need to delay due to a restart AND
	     *   the bp is expedited if we're blocking, starving, or flushing
	     *   the queue, AND
	     *   the bp is expedited if we are processing a CAC
	     */
	    if ( (!(sid_ptr->restart_in_prog && 
		   (bp->flags & SC_DELAY_CMD)))       &&
	         ((!(dev_ptr->flags & FLUSH_or_STARVE_or_BLOCKED)) || 
		    bp->resvd7)        && 
		 ((!(dev_ptr->queue_state & WAIT_INFO_or_HALTED_CAC)) || 
		    bp->resvd7))
	    {
		p720_DEQ_BP_SAVE(dev_ptr)
		bp->bufstruct.av_forw = NULL;
                if (dev_ptr->bp_save_head == NULL)
                    dev_ptr->bp_save_tail = NULL;
		TRACE_1("deq", (int) dev_ptr)
        	tag = p720_alloc_resources(bp, dev_ptr);
		if (tag > PSC_ALLOC_FAILED)
        	{  /* Resources were allocated */
		    dev_ptr->flags &= ~BLOCKED;
                    p720_enq_wait(&adp_str.command[tag]);
		}
		else
		{
		    /* resource allocation failed, so marked the */
		    /* dev_ptr as blocked, return the bp so that */
		    /* the caller can place it on the wfr queue, */
		    /* and stop processing the bp_save queue.    */
		    dev_ptr->flags |= BLOCKED;
		    ret_bp = bp;	
		    break;
		}
	    }
	    else /* can't process queue, so exit loop */
		break;
	} /* end while */
    }  /* end if */
    
    return (ret_bp);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_epow                                                       */
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
/**************************************************************************/
int
p720_epow(struct intr * handler)
{
    int     old_pri1, old_pri2, i;
    caddr_t eaddr;
    struct dev_info * dev_ptr;
    struct sc_buf * ret_bp;

    DEBUG_0 ("Entering p720_epow routine.\n")
    if ((handler->flags & EPOW_SUSPEND) ||
        ((handler->flags & EPOW_BATTERY) &&
         (!(adp_str.ddi.battery_backed))))
    {
        DEBUG_0 ("p720_epow: suspend\n")
        adp_str.epow_state = EPOW_PENDING;
        eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
        p720_command_reset_scsi_bus(eaddr);
	BUSIO_DET(eaddr);
    }   /* endif */
    else  /* restart again after an EPOW has occurred */
    {
        if (handler->flags & EPOW_RESUME)
        {
            DEBUG_0 ("p720_epow: resume\n")
            /* handle a resume command */
            /* disable to int level during this */
            old_pri1 = i_disable(adp_str.ddi.int_prior);

            /* disable to close window around the test */
            old_pri2 = i_disable(INTEPOW);

            if (((!(handler->flags & EPOW_BATTERY)) &&
                 (!(handler->flags & EPOW_SUSPEND))) &&
                (adp_str.epow_state == EPOW_PENDING))
            {
                adp_str.epow_state = 0; /* reset epow state */
                i_enable(old_pri2);     /* return to lower priority */
		for (i = 0; i < MAX_DEVICES; i++)
		{
		    if (((dev_ptr = adp_str.device_queue_hash[i]) != NULL) &&
			(dev_ptr->bp_save_head != NULL))
		    {
			if ((ret_bp = p720_process_bp_save(dev_ptr)) != NULL)
			    p720_enq_WFR(ret_bp);
		    }
		}
            }
            else
            {   /* either a SUSPEND has re-occurred, or this */
                /* adap was not put in epow pending state.   */
                /* for these cases--leave adapter as is      */
                i_enable(old_pri2);     /* return to lower priority */
            }
            i_enable(old_pri1); /* re-enable */
        }       /* end if */
    }   /* end else */

    DEBUG_0 ("leaving epow: intr_succeeded\n")
    return (INTR_SUCC);
}  /* end p720_epow */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_watchdog                                           	  */
/*                                                                        */
/* FUNCTION:  Command timer routine.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine runs at INTTIMER.                                    */
/*                                                                        */
/* NOTES:  This routine is called when a command has timed out.  A BDR is */
/*         issued in an attempt to cleanup the device when a normal cmd   */
/*         times-out.  If an abort or bdr times-out the bus is reset.     */
/*	   The other time-outs handled by this routine are for the amount */
/*	   of time it takes to reset the scsi bus, and for the restart    */
/*         delay.							  */
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
/**************************************************************************/
void
p720_watchdog(struct watchdog * w)
{
    struct dev_info *dev_ptr, *tmp_dev_ptr;
    struct cmd_info *com_ptr;
    struct sc_buf *bp;
    struct timer *tdog;
    int     i, old_pri;
    uchar   check_chip;
    int dev_index, lun;
    caddr_t eaddr;
    int     base_time;    /* save the largest timeout value on the */
                          /* active list; used to detect real t_o  */

    DEBUG_0 ("Entering p720_watchdog\n")
    old_pri = i_disable(adp_str.ddi.int_prior);

    tdog = (struct timer *) w;
    TRACE_1 ("in watch", tdog->timer_id)
    if (tdog->timer_id == PSC_RESET_TMR)
    {
        w_stop(&adp_str.reset_watchdog.dog);
        /* if we failed in causing the scsi bus to reset.  This   */
        /* guarantees that the chip is hung, so reset the chip.   */
        eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
        p720_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA, eaddr);
        p720_command_reset_scsi_bus(eaddr);
        BUSIO_DET(eaddr);
    }
    else if (tdog->timer_id == PSC_RESTART_TMR)
    {
	TRACE_1 ("restart timer", 0);
        w_stop(&adp_str.restart_watchdog.dog);
        if (adp_str.epow_state == EPOW_PENDING)
        {       /* an epow is still pending, restart timer */
            adp_str.restart_watchdog.dog.restart = adp_str.ddi.cmd_delay + 1;
            w_start(&(adp_str.restart_watchdog.dog));
        }
        else
        {
            /* loop through hash table restarting each device */
	    check_chip = TRUE;
            eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
            for (i = 0; i < MAX_SCRIPTS; i++)
            {
                if ((adp_str.sid_info[i].dma_script_ptr != NULL) &&
                    (adp_str.sid_info[i].restart_in_prog))
                {
                    adp_str.sid_info[i].restart_in_prog = FALSE;
		    dev_index = adp_str.sid_info[i].scsi_id << 3;
		    /* for each lun, unfreeze its queues */
	            for (lun = 0; lun < MAX_LUNS; lun++)
		    {
			if ((dev_ptr = adp_str.device_queue_hash[
				dev_index + lun]) != NULL)
			{
			    check_chip = p720_unfreeze_qs(dev_ptr, 
					NULL, check_chip, eaddr);
			}
		    } /* end for loop */
                }
            } /* end for loop */
            BUSIO_DET(eaddr);
        } /* end else not EPOW */
    }
    else  /* PSC_COMMAND_TMR */
    {
        /* if here a possibly fatal command timeout occurred */ 
	dev_ptr = (struct dev_info *) ((uint) (tdog)); 
	TRACE_2("cmd tmr", (int) dev_ptr, (int) dev_ptr->device_state)
	w_stop(&dev_ptr->dev_watchdog.dog);
	dev_ptr->flags |= CAUSED_TIMEOUT;

        /* is an abort currently running, or is a bdr pending or
         * currently running?  (the pending bdr case in included in
         * case we wrote sigp to issue a bdr and the chip is hung.
         */
        if ((dev_ptr->device_state == ABORT_IN_PROGRESS) ||
	    (dev_ptr->flags & SCSI_BDR))
        {
            DEBUG_0 ("p720_watchdog: ABORT/BDR in PROGRESS\n")
            if (((bp = dev_ptr->active_head->bp) != NULL) &&
		(!(bp->bufstruct.b_flags & B_ERROR)))
            {
                bp->status_validity = SC_ADAPTER_ERROR;
                bp->general_card_status = SC_CMD_TIMEOUT;
                bp->scsi_status = SC_GOOD_STATUS;
                bp->bufstruct.b_flags |= B_ERROR;
                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
                bp->bufstruct.b_error = EIO;
            }
            eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
            p720_command_reset_scsi_bus(eaddr);       /* reset scsi bus */
	    BUSIO_DET(eaddr);
        }
        else    /* regular command */
        {
            DEBUG_0 ("p720_watchdog: regular cmd \n")
	    TRACE_1 ("regular cmd", (int) dev_ptr)

            if (((bp = dev_ptr->active_head->bp) == NULL) ||
                (dev_ptr->device_state == NOTHING_IN_PROGRESS))
	    {
		/* this should not occur, a null bp indicates an abort */
		/* or bdr.  Log the error and exit. */
                p720_logerr(ERRID_SCSI_ERR6, UNKNOWN_ADAPTER_ERROR, 183,
                   0, dev_ptr->active_head, TRUE);
                TRACE_1 ("out watch", -1)
                i_enable(old_pri);
                return;
	    }

	    /* is there more than one active command (are we queuing?) */
	    if (dev_ptr->active_head != dev_ptr->active_tail) 
	    {
                /* determine if this is a valid timeout.          */
                /* walk the list of commands on this device's     */
                /* active queue to ensure that no cmd on the list */
                /* has a timeout value greater than that at the   */
                /* head of the list. If a greater value exists,   */
                /* an erroneous timeout may have occurred as a    */
                /* result of the device reordering commands.      */
                base_time = bp->timeout_value;
                com_ptr = dev_ptr->active_head->active_fwd;
		while(com_ptr != NULL)
	        {
                    if (com_ptr->bp != NULL ) 
	            {
                        if ( com_ptr->bp->timeout_value > base_time )
                            base_time = com_ptr->bp->timeout_value;
			else if (com_ptr->bp->timeout_value == 0)
			{
			    base_time = 0;
			    break;
			}
		    }
		    com_ptr = com_ptr->active_fwd;
                } /* end while */

                /* if a command on the active list has a timeout    */
                /* value greater than that of the head command,     */
                /* then restart the wdog with the difference of     */
                /* of the two timeout values; this was not a true   */
                /* timeout.   save_time is used so the code does    */
                /* restart the timer for the same command more than */
                /* once.  Without using save_time, the timer would  */
                /* again expire and we would find the same command  */
                /* with a timeout value greater than the head and   */
                /* perpetually restart the timer.                   */
	        /* Restart the timer with a zero value if a cmd on  */
		/* the active list has a timeout value of zero.     */
                if (((base_time > bp->timeout_value) && (base_time >
                         dev_ptr->dev_watchdog.save_time)) || 
		     (base_time == 0)) 
		{
		    if (base_time)
                        dev_ptr->dev_watchdog.dog.restart =
                            base_time - bp->timeout_value + 1;
		    else
                        dev_ptr->dev_watchdog.dog.restart = 0;
                    dev_ptr->dev_watchdog.save_time = base_time;
	            dev_ptr->flags &= ~CAUSED_TIMEOUT;
                    w_start(&dev_ptr->dev_watchdog.dog);
		    TRACE_1("false to", dev_ptr->dev_watchdog.dog.restart)
                    i_enable(old_pri);
                    return;
                }
                else 
		{  /* a real timeout has been detected */
                    /* reset save_time */
                    dev_ptr->dev_watchdog.save_time = 0;
                }
	    } /* end if command queuing device */

	    /* at this point we know we that a valid bp has actually */
	    /* timed-out.  We mark the bp, and then set up for a bdr */

            if (dev_ptr->ioctl_wakeup)
            {
                e_wakeup(&dev_ptr->ioctl_event);
                dev_ptr->ioctl_errno = ETIMEDOUT;
                dev_ptr->ioctl_wakeup = FALSE;
            }

            bp->status_validity = SC_ADAPTER_ERROR;
            bp->general_card_status = SC_CMD_TIMEOUT;
            bp->scsi_status = SC_GOOD_STATUS;
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->bufstruct.b_error = EIO;

	    /* if a bdr is not already queued, set up for one and signal */
	    /* the chip */
	    if (!(dev_ptr->flags & SCSI_BDR))
	    {
                dev_ptr->flags &= ~RETRY_ERROR;
                dev_ptr->retry_count = 1;
                if (!(dev_ptr->flags & SCSI_ABORT))
                    p720_enq_abort_bdr(dev_ptr);

    	        /* mark all LUNs for the scsi id */
    	        dev_index = dev_ptr->scsi_id << 3;
    	        for (i = 0; i< MAX_LUNS; i++)
    	        {
        	    if ((tmp_dev_ptr = adp_str.device_queue_hash[dev_index + i]) 
			!= NULL)
        	    {
            	        tmp_dev_ptr->flags |= SCSI_BDR;
		        tmp_dev_ptr->queue_state &= STOPPING;
		        tmp_dev_ptr->queue_state |= WAIT_FLUSH;
        	    }
    	        }

	        dev_ptr->sid_ptr->bdring_lun = dev_ptr;

                /* signal the chip that there is new work */
                eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	        if (p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr))
                    p720_command_reset_scsi_bus(eaddr);
                else
                {   /* we start timing the command here, even though     */
		    /* at this point we are only trying to stop the chip */
		    /* The timer guards against the chip being hung, at  */
		    /* the risk of leading to a bus reset if a device    */
		    /* at this point is on the bus for 5 seconds and the */
		    /* SIGP therefore has no effect. */
                    dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
                    w_start(&dev_ptr->dev_watchdog.dog);
                }
		BUSIO_DET(eaddr);

	    } /* if no BDR already queued */
        } /* else regular command */
    }   /* else command timer */
    TRACE_1 ("outwatch", 0)
    i_enable(old_pri);
    DEBUG_0 ("Leaving p720_watchdog\n")
}  /* p720_watchdog */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_command_reset_scsi_bus                                     */
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
/* INPUTS:                                                                */
/*	eaddr - effective address for pio				  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      none                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_command_reset_scsi_bus(caddr_t eaddr)
{
    uchar reset_chip;
    int     rc, i;
    int     old_pri;

    DEBUG_0 ("Entering p720_comand_reset_scsi_bus\n")
    TRACE_1 ("in busrs", (int) eaddr)
    old_pri = i_disable(adp_str.ddi.int_prior);

    /* assert the SIOP abort */
    rc = p720_write_reg(ISTAT, ISTAT_SIZE, 0x80, eaddr);  
    if (rc != REG_FAIL)
    {
        reset_chip = TRUE;
        /* check to see when abort has occurred */
        for (i = 0; i < 30; i++)
        {
            p720_delay(1);
            /* deassert abort cmd */
            rc = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
            if (!(rc & DIP))
            {
                if (rc & SIP)
                    (void) p720_read_reg(SIST, SIST_SIZE, eaddr);
                continue;
            }
            /* deassert abort cmd */
            rc = p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr);
            if (rc == REG_FAIL)
            {
                break;
            }

            /* read the scsi and dma status registers to clear them */
            rc = p720_read_reg(DSTAT, DSTAT_SIZE, eaddr);
            if (rc == REG_FAIL)
            {
                break;
            }

            if (!(rc & DABRT))
            {
                rc = p720_write_reg(ISTAT, ISTAT_SIZE, 0x80, eaddr);
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
        p720_chip_register_reset(PSC_RESET_CHIP | PSC_RESET_DMA, eaddr);
    }

    /* attempt to assert the reset */
    /* write SCNTL1 to assert scsi reset */
    /* assert the SCSI reset signal */
    (void) p720_write_reg(SCNTL1, SCNTL1_SIZE, (SCNTL1_EXC_OFF | 0x08),
		 	  eaddr);
    p720_delay(30);

    /* write SCNTL1 to deassert scsi reset */
    (void) p720_write_reg(SCNTL1, SCNTL1_SIZE, SCNTL1_EXC_OFF, eaddr);
    w_start(&(adp_str.reset_watchdog.dog));
    TRACE_1 ("out busr", 0)
    i_enable(old_pri);
    DEBUG_0 ("LEAVING p720_command_reset_scsi_bus\n")
}  /* p720_command_reset_scsi_bus */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_fail_free_resources                                        */
/*                                                                        */
/* FUNCTION:  Handles the case where free resources fails                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      called by the interrupt handler                                   */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_info structure whose command experienced a copy failure       */
/*      sc_buf structure associated with the STA command                  */
/* 	flag indicating whether the chip is connected to the scsi bus     */
/* 	effective address for pio					  */
/*     									  */
/* RETURN VALUE DESCRIPTION:                                              */
/*      flag indicating whether a command was issued                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_fail_free_resources(
                        struct dev_info * dev_ptr,
                        struct sc_buf * bp, uchar connected,
                        int cmd_issued, caddr_t eaddr)
{
    int ret_code;

    DEBUG_1 ("Entering p720_fail_free_resources error= 0x%x\n", err_code);
    TRACE_1 ("in failf", (int) dev_ptr)

    /* PSC_COPY_ERROR is the only way free resources can fail */
    if (!(bp->bufstruct.b_flags & B_ERROR))
    {   /* no previous error */
        bp->status_validity = 0;
        bp->bufstruct.b_error = EFAULT;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
    }

    dev_ptr->flags |= SCSI_ABORT;
    p720_enq_abort_bdr(dev_ptr);

    /* if a command hasn't been issued yet, call issue_abort_bdr */
    if (!cmd_issued)
    {   /* issue_abort_bdr returns whether a new job can be started */
        ret_code = !(p720_issue_abort_bdr (dev_ptr, bp, FALSE, 
		     connected, eaddr));
    }
    else
	ret_code = cmd_issued;

    DEBUG_0 ("leaving p720_fail_free_resources\n")
    TRACE_1 ("out faif", 0)
    return (ret_code);
}  /* p720_fail_free_resources */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_issue_abort_bdr                                            */
/*                                                                        */
/* FUNCTION:  Handles the issueing of bdr/aborts for interrupts           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Interrupts should be disabled.                                    */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_info structure for target of abort or bdr                     */
/*      sc_buf structure (if any) to be marked				  */
/* 	flag indicating whether routine is called from A_iodone path of   */
/*	     the interrupt handler, before status was recorded.		  */
/*	flag indicating whether the chip is connected to the scsi bus     */
/* 	effective address for pio					  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      indication of whether a new job should be started:                */
/*      TRUE: start new job   FALSE: chip is busy                         */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_issue_abort_bdr(
                    struct dev_info * dev_ptr,
                    struct sc_buf *bp,
                    uint iodone_flag, uchar connected, caddr_t eaddr)
{
    struct cmd_info * com_ptr;
    int     dev_index;

    DEBUG_0 ("Entering p720_issue_abort_bdr\n")
    TRACE_1 ("in issAB", (int) dev_ptr)

    /* set the status if not previously marked */
    if (!(bp->bufstruct.b_flags & B_ERROR))
    {
	/* if called after an iodone, need to get the status */
	if (iodone_flag)
        {
            bp->scsi_status = GET_CMD_STATUS_BYTE(
                adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr);
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
        else  /* not iodone flag */
        {
            bp->status_validity = 0;
            bp->scsi_status = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
            bp->bufstruct.b_error = ENXIO;
            bp->bufstruct.b_flags |= B_ERROR;
        }
    }

    if (dev_ptr->flags & SCSI_BDR)
    {
	/* access the dev_ptr the bdr is anchored upon, to ensure the 
	 * correct ioctl sleeper (if any) will be identified 
	 */
	if (dev_ptr != dev_ptr->sid_ptr->bdring_lun)
	{
	    dev_ptr = dev_ptr->sid_ptr->bdring_lun;
	}

        com_ptr = &adp_str.command[adp_str.ddi.card_scsi_id << 3];
        com_ptr->bp = NULL;
        com_ptr->device = dev_ptr;
	com_ptr->resource_state = NO_RESOURCES_USED;
	com_ptr->in_use = TRUE;
        p720_enq_active(dev_ptr, com_ptr);

        dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);

        if (p720_issue_bdr_script(
                (uint *) adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
		script_ptr, (uint *) dev_ptr->sid_ptr->dma_script_ptr, 
		dev_index, com_ptr, connected, eaddr) == REG_FAIL)
        {
            p720_cleanup_reset(REG_ERR, eaddr);
            TRACE_1 ("outissB", REG_FAIL)
            return (FALSE);
        }
 
        dev_ptr->device_state = BDR_IN_PROGRESS;
        dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
        dev_ptr->flags &= ~RETRY_ERROR;
        dev_ptr->retry_count = 1;
        w_start(&dev_ptr->dev_watchdog.dog);
        p720_deq_abort_bdr(dev_ptr);
        TRACE_1 ("outissB", 0)
        return (FALSE);
    }
    else        /* abort was requested */
    {
        /* remove device from ABRT/BDR queue */
        p720_deq_abort_bdr(dev_ptr);
	
	/* if there are still active commands (assuming this one is
	 * still on the queue), then issue the abort.
         */
        if (dev_ptr->active_head != dev_ptr->active_tail)
	{
	    dev_ptr->device_state = ABORT_IN_PROGRESS;
            com_ptr = &adp_str.command[adp_str.ddi.card_scsi_id << 3];
            com_ptr->bp = NULL;
            com_ptr->device = dev_ptr;
	    com_ptr->resource_state = NO_RESOURCES_USED;
	    com_ptr->in_use = TRUE;
            dev_index = INDEX(dev_ptr->scsi_id, dev_ptr->lun_id);
	    if (p720_issue_abort_script((uint *) 
		adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr, 
		(uint *) dev_ptr->sid_ptr->dma_script_ptr, 
		com_ptr, dev_index, connected, eaddr) == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
		return(FALSE);
            }
            dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
            dev_ptr->flags &= ~RETRY_ERROR;
            dev_ptr->retry_count = 1;
            w_start(&dev_ptr->dev_watchdog.dog);
            TRACE_1("out issabrt", 0)
	    return (FALSE);
	}
	else /* no other active commands */
	{
            p720_fail_cmd(dev_ptr);

            /* if IOCTL wakeup flag is TRUE then call wakeup */
            /* and set IOCTL status to EIO                   */
            if (dev_ptr->ioctl_wakeup == TRUE)
            {
                dev_ptr->ioctl_errno = EIO;
                dev_ptr->ioctl_wakeup = FALSE;
                e_wakeup(&dev_ptr->ioctl_event);
            }
	} /* end else no other active commands */
    }   /* end else  abort */
    DEBUG_0 ("Leaving p720_issue_abort_bdr\n")
    TRACE_1 ("outissAB", 4)
    return (TRUE);
}  /* p720_issue_abort_bdr */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_verify_tag                                                 */
/*                                                                        */
/* FUNCTION:  Finds the dev_info whose script addr is in the DSP          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      called by the interrupt handler                             	  */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:     	                                                          */
/*      com_ptr - pointer to the command structure associated w/ the tag  */
/*      eaddr - effective address for pio                                 */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      TRUE    tag is valid                                              */
/*      FALSE   tag appears invalid given dsp or i/o errors               */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int 
p720_verify_tag(struct cmd_info * com_ptr, caddr_t eaddr)
{
    int     dsp;

    /* theory is get script dma addr from DSP       */
    /* and then compare it with the information     */
    /* accessible via the candidate command struct. */

    dsp = word_reverse(p720_read_reg(DSP, DSP_SIZE, eaddr));
    TRACE_1 ("fnd_info", dsp)

    /* if bad pio, return */
    if (dsp == REG_FAIL)
        return (FALSE);

    /* if dsp is within the main script associated with the candidate */
    /* command structure, assume we have the correct tag. */
    if ((com_ptr->device != NULL) && 
	(dsp >= (com_ptr->device->sid_ptr->dma_script_ptr + 
		Ent_scripts_entry_point)) && 
	(dsp <= (com_ptr->device->sid_ptr->dma_script_ptr + 
		Ent_cmd_msg_in_buf)))
    {
        return (TRUE);
    }
    TRACE_3("vtag", (int) com_ptr, (int) dsp, 0)
    return (FALSE);
}  /* end of p720_verify_tag */

/**************************************************************************/
/*                                                                        */
/* NAME:        p720_fail_cmd                                             */
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
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  The following are the return values:        */
/*      none                                                              */
/*                                                                        */
/**************************************************************************/
void
p720_fail_cmd(
             struct dev_info * dev_ptr)
{
    struct sc_buf *bp, *prev_bp;
    int     old_pri;
    int     dev_index;
    struct dev_info * cur_dev_ptr;
    struct cmd_info * com_ptr;


    DEBUG_0 ("Entering p720_fail_cmd routine.\n")

    old_pri = i_disable(adp_str.ddi.int_prior);
    TRACE_1 ("in failC", (int) dev_ptr)

    /* return all commands on the active queue */
    while (dev_ptr->active_head != NULL)
    {
        bp = dev_ptr->active_head->bp;
        com_ptr = dev_ptr->active_head;

	/* check if this buf has been marked as an error */
        if ((bp != NULL) && (!(bp->bufstruct.b_flags & B_ERROR)))
	{ /* set b_flags B_ERROR flag */
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	    bp->status_validity = 0;
	}

        (void) p720_free_resources(com_ptr, FALSE);

        if (bp != NULL)
            iodone((struct buf *) bp);

	p720_deq_active(dev_ptr, com_ptr);
    }   /* while */

    /* return all commands on the wait queue for this device */
    while (dev_ptr->wait_head != NULL)
    {
        bp = dev_ptr->wait_head->bp;
        com_ptr = dev_ptr->wait_head;

        /* Set b_flags B_ERROR flag if not already set.      */
	/* (on certain error flows, we might mark a bp,      */
	/* put it on the wait_q, and initiate an action      */
	/* that eventually results in fail_cmd being called) */
        if (!(bp->bufstruct.b_flags & B_ERROR))
        {
            bp->bufstruct.b_flags |= B_ERROR;
            bp->bufstruct.b_error = ENXIO;
            bp->status_validity = 0;
            bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	}

        (void) p720_free_resources(com_ptr, FALSE);
	p720_deq_wait (com_ptr);
        iodone((struct buf *) bp);
    }   /* while */

    /* return all commands on the cmd_save queue for this device */
    while (dev_ptr->cmd_save_head != NULL)
    {
        bp = dev_ptr->cmd_save_head->bp;
        com_ptr = dev_ptr->cmd_save_head;
        /* set b_flags B_ERROR flag */
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_error = ENXIO;
        bp->status_validity = 0;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
        (void) p720_free_resources(com_ptr, FALSE);
	p720_DEQ_CMD_SAVE(dev_ptr)
        iodone((struct buf *) bp);
    }
    dev_ptr->cmd_save_tail = NULL;

    /* return any sc_buf on the wait for resources queue for this device */
    if (dev_ptr->flags & BLOCKED)
    {
        /* there is a bp on the queue for this device, need to find it */
	prev_bp = NULL;	
	bp = adp_str.REQUEST_WFR_head;
        while (bp != NULL)
        {
            dev_index = INDEX(bp->scsi_command.scsi_id,
                          (bp->scsi_command.scsi_cmd.lun) >> 5);
            cur_dev_ptr = adp_str.device_queue_hash[dev_index];
	    if (cur_dev_ptr == dev_ptr) 
	    {   /* this request belongs to the device */
                /* set b_flags B_ERROR flag */
                bp->bufstruct.b_flags |= B_ERROR;
                bp->bufstruct.b_error = ENXIO;
                bp->status_validity = 0;
                bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
   	        p720_deq_WFR (bp, prev_bp);
                iodone((struct buf *) bp);
	        break;	/* only one command on WFR queue; we found */
			/* it so break out the the while loop      */
	    }
	    else
	    {   /* this request doesn't belong to the device - skip it */
	        prev_bp = bp;
	        bp = (struct sc_buf *) bp->bufstruct.av_forw;
	    }
        }   /* while */
    }

    /* return all commands on the bp_save queue for this device */
    while (dev_ptr->bp_save_head != NULL)
    {
	TRACE_1 ("bp_save", (int) dev_ptr->bp_save_head)
	bp = dev_ptr->bp_save_head;
        /* set b_flags B_ERROR flag */
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_error = ENXIO;
        bp->status_validity = 0;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount;
	p720_DEQ_BP_SAVE(dev_ptr)
	iodone((struct buf *) bp);
    }
    dev_ptr->bp_save_tail = NULL;

    /* turn off BLOCKED, STARVE, etc. */
    dev_ptr->flags = RETRY_ERROR;
    dev_ptr->dev_watchdog.save_time = 0;

    /* change queue state to HALTED */
    if (dev_ptr->queue_state & STOPPING)
    {
        e_wakeup(&dev_ptr->stop_event);
	dev_ptr->queue_state = HALTED | STOPPING;
    }
    else
    {
        dev_ptr->queue_state = HALTED;
    }
    TRACE_1 ("out fail", (int) dev_ptr)
    i_enable(old_pri);
}  /* end p720_fail_cmd */

/*************************************************************************/
/*                                                                      */
/* NAME:        p720_delay                                              */
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
/************************************************************************/
void 
p720_delay(int delay)
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

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_scsi_interrupt                                             */
/*                                                                        */
/* FUNCTION:  Handles the scsi interrupts (besides RESET) when a command  */
/*      structure has been identified.                                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      Called by the interrupt handler                                   */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS:                                                                */
/*      struct cmd_info:  command associated with the error               */
/*      struct dev_info:  target id/lun associated with the error         */
/*      save_interrupt:   encodes which error occurred                    */
/*      issue_abrt_bdr:   bit 1: is an unexpected disconnect int. pending */
/*                               (only used for Selection Time-outs)      */
/*                        bit 0: should an abort be issued                */
/*      eaddr:		  effective address for pio                       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      flag indicating whether a new job should be started on the chip   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int
p720_scsi_interrupt(
                      struct cmd_info * com_ptr,
                      struct dev_info * dev_ptr,
                      int save_interrupt,
                      int issue_abrt_bdr,
		      caddr_t eaddr)
{
    struct sc_buf *bp;
    struct dev_info *tmp_dev_ptr;
    uchar   log_err, gen_err;
    int     i, istat_val, sist_val;
    uint    dev_index;

    DEBUG_0 ("p720_scsi_interrupt: \n")
    TRACE_2 ("in sint ", (int) dev_ptr, (int) com_ptr)
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
    case SCSI_SEL_TIMEOUT:
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
    /* to 10 usecs.  */
    if ((save_interrupt == SCSI_SEL_TIMEOUT) && 
	(issue_abrt_bdr & DISC_PENDING))
    {
        /* turn off bit indicating unexpected disconnect pending */
        issue_abrt_bdr &= ~DISC_PENDING;

        /* loop looking to clear the unexpected disconnect int.  */
        for (i = 0; i < 10; i++)
        {
            p720_delay(1);
            istat_val = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
            if (istat_val == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
                TRACE_1 ("out sint", (int) ISTAT)
                return (FALSE);
            }
            if (istat_val & SIP)
            {
                sist_val = p720_read_reg(SIST, SIST_SIZE, eaddr);
                if (sist_val == REG_FAIL)
                {
                    p720_cleanup_reset(REG_ERR, eaddr);
                    TRACE_1 ("out sint", (int) SIST)
                    return (FALSE);
                }
                if ((word_reverse((ulong) sist_val) >> 16) & SCSI_UNEXP_DISC)
                        break;
            }
        }  /* end for loop */
    }
    /* for unexpected disconnects, it is possible that a */
    /* selection timeout interrupt is stacked due to the */
    /* design of the chip.  Here, the interrupt is      */
    /* cleared and the error returned is set to repre-  */
    /* set a selection timeout.                         */
    if (save_interrupt == SCSI_UNEXP_DISC)
    {
        istat_val = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
        if (istat_val == REG_FAIL)
        {
            p720_cleanup_reset(REG_ERR, eaddr);
            TRACE_1 ("out sint", (int) ISTAT)
            return (FALSE);
        }
        if (istat_val & SIP)
        {
            sist_val = p720_read_reg(SIST, SIST_SIZE, eaddr);
            if (sist_val == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
                TRACE_1 ("out sint", (int) SIST)
                return (FALSE);
            }
            if ((word_reverse((ulong) sist_val) >> 16) & SCSI_SEL_TIMEOUT)
            {
                save_interrupt = SCSI_SEL_TIMEOUT;
                dev_ptr->flags |= SELECT_TIMEOUT;
                log_err = SC_NO_DEVICE_RESPONSE;
                gen_err = SC_NO_DEVICE_RESPONSE;
		if (dev_ptr->active_head == dev_ptr->active_tail)
                    issue_abrt_bdr = FALSE;
            }
        }

        if (save_interrupt == SCSI_UNEXP_DISC) {
            /* clear DMA and SCSI FIFOs */
            if ((p720_write_reg(CTEST3, CTEST3_SIZE, 0x04, eaddr) == REG_FAIL) ||
                 (p720_write_reg(STEST3, STEST3_SIZE, 0x92, eaddr) == REG_FAIL)) {
                 p720_cleanup_reset(REG_ERR, eaddr);
                 TRACE_1 ("out fifo", 0)
                 return (FALSE);
            }
            TRACE_1("clear fifos", 0);
        }

    }

    bp = com_ptr->bp;
    TRACE_1("bp is ", (int) bp)

    /* if we were requested to issue an abort (via the issue_abrt_bdr flag) */
    /* issue it if an abort or bdr is not already in progress */
    if (((dev_ptr->device_state != ABORT_IN_PROGRESS) &&
         (dev_ptr->device_state != BDR_IN_PROGRESS)) &&
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
            if (save_interrupt != SCSI_SEL_TIMEOUT)
                p720_logerr(ERRID_SCSI_ERR10, log_err,
                           190, 0, com_ptr, TRUE);
        }

        /* update wait script */
        p720_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr,
                              dev_index, FALSE);

        /* start the abort SCRIPT */
        if ((save_interrupt != SCSI_SEL_TIMEOUT) &&
            (save_interrupt != SCSI_UNEXP_DISC)) {
	    /* read ISTAT to see if we are connected to the scsi bus */
   	    if ((istat_val = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr)) == REG_FAIL)
            {
                p720_cleanup_reset(REG_ERR, eaddr);
                TRACE_1 ("out sint", (int) ISTAT)
                return (FALSE);
            }
	    istat_val &= CONNECTED;
	} else
	    istat_val = FALSE;

        if (p720_issue_abort_script((uint *) adp_str.SCRIPTS
	     [dev_ptr->sid_ptr->script_index].script_ptr,
             (uint *) dev_ptr->sid_ptr->dma_script_ptr, com_ptr, dev_index,
	     (uchar) istat_val, eaddr)
              == REG_FAIL)
        {
            p720_cleanup_reset(REG_ERR, eaddr);
            TRACE_1 ("out sint", 1)
            return (FALSE);
        }
        dev_ptr->device_state = ABORT_IN_PROGRESS;
        dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
        dev_ptr->flags |= SCSI_ABORT;
        dev_ptr->flags &= ~RETRY_ERROR;
        dev_ptr->retry_count = 1;
        w_start(&dev_ptr->dev_watchdog.dog);
        TRACE_1 ("out sint", 0)
        return (FALSE);
    }
    else
    {
        /* if we issued a bdr due to a command timeout and got */
        /* a scsi interrupt, or if the interrupt we got would */
	/* normally cause us to issue an abort, and we were already */
        /* trying to issue an abort, then reset the scsi bus. */
        TRACE_1("not abort", 0)
        if ((dev_ptr->flags & CAUSED_TIMEOUT) ||
	   ((dev_ptr->device_state == ABORT_IN_PROGRESS) && issue_abrt_bdr))
        {
            p720_command_reset_scsi_bus(eaddr);       /* reset scsi bus */
            return (FALSE);
        }

	/* if we issued a bdr ioctl and any of the luns have active */
	/* commands besides the bdr ioctl, reset the bus and exit.  */
        /* Otherwise, mark the other luns to indicate a bdr is no   */
	/* longer pending, and set their queue state and flags.     */
	if (dev_ptr->device_state == BDR_IN_PROGRESS)
	{
	    dev_index = dev_ptr->scsi_id << 3;
            for (i=0; i < MAX_LUNS; i++)
            {
                if ((tmp_dev_ptr = adp_str.device_queue_hash[dev_index + i])
                        != NULL)
                {
		    if (((tmp_dev_ptr != dev_ptr) &&
			 (tmp_dev_ptr->active_head != NULL)) ||
			((tmp_dev_ptr == dev_ptr) && (dev_ptr->active_head !=
			 dev_ptr->active_tail)))
		    {
                        p720_command_reset_scsi_bus(eaddr);
                        return (FALSE);
		    }
		    else if (tmp_dev_ptr != dev_ptr)
		    {   /* fail cmd handles dev_ptr below, after additional */
			/* processing */
			tmp_dev_ptr->flags &= ~SCSI_BDR;
			tmp_dev_ptr->queue_state &= STOPPING;
			tmp_dev_ptr->queue_state |= ACTIVE;
		    }
		}
	    }
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
                /* sync message reject, no log entry should be */
	        /* made. An error, however,     */
                /* should be returned to the calling process. */
                if ((save_interrupt != SCSI_SEL_TIMEOUT) &&
                      ((save_interrupt != SCSI_UNEXP_DISC) ||
                       (dev_ptr->device_state !=
                        NEGOTIATE_IN_PROGRESS)))
                {
                    p720_logerr(ERRID_SCSI_ERR10, log_err,
                               195, 0, com_ptr, TRUE);
                }
            }
            /* status already set and error logged */
            (void) p720_free_resources(com_ptr, FALSE);
            iodone((struct buf *) bp);
        }
        else if (save_interrupt != SCSI_SEL_TIMEOUT)
	     /* bp should be null only for abort/bdr ioctls */
        {
            p720_logerr(ERRID_SCSI_ERR10, log_err, 200, 0, com_ptr, TRUE);
        }

        dev_ptr->flags = RETRY_ERROR;
        if ((adp_str.ABORT_BDR_head == dev_ptr) ||
            (dev_ptr->ABORT_BDR_bkwd != NULL))
            /* remove any abort or bdr to queued */
            p720_deq_abort_bdr(dev_ptr);

        /* remove device from active queue */
        p720_deq_active(dev_ptr, com_ptr);
        p720_fail_cmd(dev_ptr);

        if (dev_ptr->ioctl_wakeup == TRUE)
        {
            if (save_interrupt != SCSI_SEL_TIMEOUT)
                dev_ptr->ioctl_errno = PSC_NO_ERR;
            else
                dev_ptr->ioctl_errno = EIO;
            dev_ptr->ioctl_wakeup = FALSE;
            e_wakeup(&dev_ptr->ioctl_event);
        }

    }
    TRACE_1 ("out sint", 0)
    return (TRUE);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_verify_neg_answer                                  */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function goes into the main script and patches the         */
/*      register move instructions that occur for every data phase.     */
/*      Scripts.  If negotiation has ended normally (either the target  */
/*      has requested asynchronous transfers, or else the target has    */
/*      requested a transfer period we can support), this function      */
/*      returns zero.  A non-zero return from this function indicates   */
/*      that the caller must reject the target's SDTR message, and that */
/*      the script has been patched for asynchronous transfers.         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Called from the interrupt handler.                              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *script_vir_addr - the virtual address of the script that needs */
/*              to be altered with an interrupt we are patching in      */
/*              ourselves.  This can be considered a form of code       */
/*              self-modification.                                      */
/*	*dev_ptr - address of the device info structure associated w/   */
/*		the target id/lun we are negotiating with		*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      PSC_NO_ERR - negotiation exchange completed without problems    */
/*      PSC_FAILED - caller needs to issue a Message Reject             */
/*                                                                      */
/************************************************************************/
int
p720_verify_neg_answer(
                      uint * script_vir_addr,
                      struct dev_info * dev_ptr)

{
    uchar   xfer_period, req_ack_offset;
    int     i, rc;

    /* Entering p720_verify_neg_answer */
    rc = (int) PSC_NO_ERR;
    xfer_period = (uchar) (script_vir_addr[Ent_extended_msg_buf / 4] >> 8);
    req_ack_offset = (uchar) (script_vir_addr[Ent_extended_msg_buf / 4]);
    TRACE_2 ("v negot ", (int) xfer_period, (int) req_ack_offset)
    if ((xfer_period != DEFAULT_MIN_PHASE) ||
        (req_ack_offset != DEFAULT_BYTE_BUF))
    {
        if (req_ack_offset == 0)
        {
            /* if req_ack_offset is zero, we must treat this like */
            /* an asynchronous device.                            */
            dev_ptr->sid_ptr->async_device = TRUE;

            /* Patch the SCRIPT for async transfers. */
            TRACE_1 ("vneg zero", 0)
            for (i=0; i<4; i++)
            {
                script_vir_addr[R_sxfer_patch_Used[i]] =
                        word_reverse(SXFER_ASYNC_MOVE);
                script_vir_addr[R_scntl1_patch_Used[i]] =
                        word_reverse(SCNTL1_EOFF_MOVE);
                script_vir_addr[R_scntl3_patch_Used[i]] =
                        word_reverse(SCNTL3_SLOW_MOVE);
	    }

        }
        else
        {
	    if (xfer_period <=  adp_str.xfer_max[NUM_720_SYNC_RATES - 1])
	    {
		p720_patch_nondefault_sync(script_vir_addr, xfer_period,
			req_ack_offset);
	    }
	    else  /* target's requested sync transfer period is slower */
		  /* than we can set the chip.  We must inform the     */
  		  /* target by initiating another exchange of SDTR     */
		  /* messages, this time using a zero req/ack offset   */
		  /* to force the exchange to be async.                */
	    {
                dev_ptr->sid_ptr->async_device = TRUE;

                /* Patch the SCRIPT for async transfers. */
                TRACE_1 ("vneg slow", 0)
                for (i=0; i<4; i++)
                {
                    script_vir_addr[R_sxfer_patch_Used[i]] =
                            word_reverse(SXFER_ASYNC_MOVE);
                    script_vir_addr[R_scntl1_patch_Used[i]] =
                            word_reverse(SCNTL1_EOFF_MOVE);
                    script_vir_addr[R_scntl3_patch_Used[i]] =
                            word_reverse(SCNTL3_SLOW_MOVE);
	        }
		rc = (int) PSC_FAILED;
	    }
        }
    }
    else
    {
  	/* Patch in values for SXFER, SCNTL1, and SCNTL3 */
        TRACE_1 ("vfy neg, dflts", 0)
        for (i=0; i<4; i++)
        {
            script_vir_addr[R_sxfer_patch_Used[i]] 
	        = word_reverse(SXFER_DEF_MOVE);
            script_vir_addr[R_scntl1_patch_Used[i]] 
		= word_reverse(SCNTL1_EOFF_MOVE);
            script_vir_addr[R_scntl3_patch_Used[i]] 
		= word_reverse(SCNTL3_FAST_MOVE);
	}
    }

    /* Leaving p720_verify_neg_answer */

    return (rc);

}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_reset_iowait_jump                                  */
/*                                                                      */
/* FUNCTION:    Adapter Script Jump Reset Routine                       */
/*      This function does a patch of the jump instruction within the   */
/*      IO_WAIT script.  It resets the jump back to the original default*/
/*      jump that existed before it was initialized to run a script.    */
/*      This function should be called before issuing an abort or bdr   */
/*      Script.  This action and function acts as a safety net          */
/*      against an illegal or spurious interrupt by a target device     */
/*      that is supposed to be inactive.                                */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process, as long as the chip    */
/*      is not running.                                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      dev_info_hash - encodes the target id and lun                   */
/*      all_luns - flag to indicate whether to clear the jump for all   */
/*		the luns (ie. for a BDR), or just for the lun encoded   */
/*		in dev_info_hash					*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_reset_iowait_jump(
                      uint * iowait_vir_addr,
                      int dev_info_hash,
		      uchar all_luns)
{
    int     scsi_id, offset_to_jump, orig_byte_jump_offset, i;
    ulong  *io_wait_ptr;
    uchar   lun;

    TRACE_1("clear jmp", dev_info_hash)
    io_wait_ptr = (ulong *) iowait_vir_addr;

    /* Are we reseting the jump for all luns? */
    /* If so, we want the jump address for lun 0 */
    /* to use as our base */
    if (all_luns)
	lun = 0x00;
    else
        lun = LUN(dev_info_hash);

    scsi_id = SID(dev_info_hash);
    TRACE_2 ("lun id ", lun, scsi_id)
    switch (scsi_id)
    {
    case 0:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[0] - 1;
        offset_to_jump = E_scsi_0_lun_Used[lun];
        break;
    case 1:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[1] - 1;
        offset_to_jump = E_scsi_1_lun_Used[lun];
        break;
    case 2:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[2] - 1;
        offset_to_jump = E_scsi_2_lun_Used[lun];
        break;
    case 3:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[3] - 1;
        offset_to_jump = E_scsi_3_lun_Used[lun];
        break;
    case 4:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[4] - 1;
        offset_to_jump = E_scsi_4_lun_Used[lun];
        break;
    case 5:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[5] - 1;
        offset_to_jump = E_scsi_5_lun_Used[lun];
        break;
    case 6:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[6] - 1;
        offset_to_jump = E_scsi_6_lun_Used[lun];
        break;
    case 7:
        orig_byte_jump_offset = A_uninitialized_reselect_Used[7] - 1;
        offset_to_jump = E_scsi_7_lun_Used[lun];
        break;
    default:
        break;

    }

    TRACE_2 ("otj obj", offset_to_jump, orig_byte_jump_offset)
    if (!all_luns)
        io_wait_ptr[offset_to_jump] =
            word_reverse((orig_byte_jump_offset * 4) + 
				(uint) adp_str.SCRIPTS[0].dma_ptr);
    else
    {
        /* Starting with lun 0, patch each jump address (every 2nd word) */
        for (i = 0; i < MAX_LUNS; i++)
	{
 	    TRACE_1 ("change", offset_to_jump + (2 * i))
 	    io_wait_ptr[offset_to_jump + (2 * i)] =
            word_reverse((orig_byte_jump_offset * 4) + 
				(uint) adp_str.SCRIPTS[0].dma_ptr);
	}
    }
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_restore_iowait_jump                                */
/*                                                                      */
/* FUNCTION:    Adapter Script Jump Setting Routine                     */
/*      This will patch the iowait script with the address required     */
/*      for the particular device.  This routine is called after a      */
/*      selection failure when trying to issue an abort or bdr to a     */
/*      device.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process, as long as the chip    */
/*      is not running.                                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*	None								*/
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      *dev_ptr - pointer to the dev_info structure which identifies   */
/*              the target scsi id and lun whose iowait jump is being   */
/*              restored.                                               */
/*      script_dma_addr - the dma address of the script associated w/   */
/*              dev_ptr.                                                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_restore_iowait_jump(
                        uint * iowait_vir_addr,
                        struct dev_info * dev_ptr,
                        uint script_dma_addr)
{
    ulong  *io_wait_ptr;
    int     offset_to_jump;
    uchar   lun;

    TRACE_1("rest jmp", (int) dev_ptr)
    io_wait_ptr = (ulong *) iowait_vir_addr;
    /*********************************************************************/
    /* Almost every define and address label in the SCRIPTS is in terms  */
    /* of word offsets except those used in JUMP commands.  These are in */
    /* terms of bytes.  All offsets are relative to the beginning address*/
    /* of the scripts, where the IO_WAIT script also resides.            */
    /*********************************************************************/
    lun = dev_ptr->lun_id;

    switch (dev_ptr->scsi_id)
    {
    case 0:
        offset_to_jump = E_scsi_0_lun_Used[lun];
        break;
    case 1:
        offset_to_jump = E_scsi_1_lun_Used[lun];
        break;
    case 2:
        offset_to_jump = E_scsi_2_lun_Used[lun];
        break;
    case 3:
        offset_to_jump = E_scsi_3_lun_Used[lun];
        break;
    case 4:
        offset_to_jump = E_scsi_4_lun_Used[lun];
        break;
    case 5:
        offset_to_jump = E_scsi_5_lun_Used[lun];
        break;
    case 6:
        offset_to_jump = E_scsi_6_lun_Used[lun];
        break;
    case 7:
        offset_to_jump = E_scsi_7_lun_Used[lun];
        break;
    default:
        break;
    }

    io_wait_ptr[offset_to_jump] = 
	word_reverse(script_dma_addr + Ent_script_reconnect_point);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_handle_extended_messages                           */
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
/*      This routine is called from the interrupt handler.              */
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
/*      *com_ptr - pointer to the cmd_info  structure which holds        */
/*              the sc_buf which contains the SCSI command given to us  */
/*              by the driver head.                                     */
/*      interrupt_flag - Interrupt that resulted in this call by the    */
/*              interrupt handler.                                      */
/*	eaddr - effective address for pio				*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A value of REG_FAIL indicates a pio error occurred, 0 otherwise */
/************************************************************************/
int
p720_handle_extended_messages(
                             uint * script_vir_addr,
                             uint * script_dma_addr,
                             struct cmd_info * com_ptr,
                             int interrupt_flag,
			     caddr_t eaddr)
{
    struct dev_info * dev_ptr;
    uint   *target_script;
    uint    scntl1_patch, scntl3_patch;
    uchar   ext_msg_opcode;
    uchar   special_xfer_val;
    ulong   ext_msg_length;
    int     data_ptr_offset;
    int     i, rc, dsp_val;

    DEBUG_0 ("Entering p720_handle_extended_messages\n")
    target_script = script_vir_addr;
    ext_msg_length = (target_script[Ent_extended_msg_buf / 4] >> 24);
    ext_msg_opcode = (target_script[Ent_extended_msg_buf / 4] >> 16);
    target_script[R_ext_msg_size_Used[0]] &= 0x000000FF;
    dev_ptr = com_ptr->device;
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
                if (com_ptr->bp->scsi_command.flags & SC_ASYNC)
                {
                    /* The target is trying to do sync negotiation */
                    /* but we do not want to do sync neg.  Thus    */
                    /* we will send a reject msg out and force it  */
                    /* into ASYNC mode                             */
                    dev_ptr->sid_ptr->async_device = TRUE;
		    dev_ptr->sid_ptr->negotiate_flag = FALSE;

                    /* Patch the SCRIPT for async transfers. */
 		    TRACE_1 ("h ext: af", 0)
                    for (i=0; i<4; i++)
                    {
                        target_script[R_sxfer_patch_Used[i]] =
                            word_reverse(SXFER_ASYNC_MOVE);
                        target_script[R_scntl1_patch_Used[i]] =
                            word_reverse(SCNTL1_EOFF_MOVE);
                        target_script[R_scntl3_patch_Used[i]] =
                            word_reverse(SCNTL3_SLOW_MOVE);
		    }

		    /* can't be sure script is prepped: */
		    if (!(com_ptr->flags & PREP_MAIN_COMPLETE))
                        p720_prep_main_script(com_ptr, (uint *) 
			    adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
                            script_ptr, dev_ptr->sid_ptr->dma_script_ptr);

                    /* go to script that sends msg reject */
                    /* for sync and then continues on     */
                    rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
                             word_reverse(script_dma_addr +
                             (Ent_reject_target_sync / 4)), eaddr);
                    return (rc);
                }
                else
                {
                    /* The target is trying to do sync negotiation */
                    /* Since we want to talk in sync mode, we now  */
                    /* set up the scripts in interrupt back into   */
                    /* this function after it has read in the      */
                    /* target's sync message.                      */
                    target_script[Ent_ext_msg_patch / 4] =
                        word_reverse(P720_INTR_PATCH);
                    target_script[(Ent_ext_msg_patch / 4) + 1] =
                        word_reverse(A_target_sync_sent); 
		}
            }
            else if (interrupt_flag == A_target_sync_sent)
            {
                /* being here tells us that we have previously patched */
                /* this interrupt in order to move in the target's     */
                /* sync request.  Now, we must examine the request     */
                /* to determine if it is higher, lower, or equal to    */
                /* what we wish to be the sync transfer rate. In this  */
                /* case the ext_msg_length is really the xfer period   */
                /* and the ext_msg_opcode is really the rec/ack offset */

                /* cleanup up patched interrupt.  Make it go */
                /* back to being a NOP instruction           */
                target_script[Ent_ext_msg_patch / 4] =
                    word_reverse(P720_NOP_PATCH);
                target_script[(Ent_ext_msg_patch / 4) + 1] =
                    word_reverse(script_dma_addr + Ent_ext_msg_patch);

		/* make sure the script is ready */
		if (!(com_ptr->flags & PREP_MAIN_COMPLETE))
                    p720_prep_main_script(com_ptr, (uint *) 
		    adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr,
                    dev_ptr->sid_ptr->dma_script_ptr);

		/* if we wanted to negotiate, we don't need to now */
		dev_ptr->sid_ptr->negotiate_flag = FALSE;

                if ((ext_msg_length != DEFAULT_MIN_PHASE) ||
                    (ext_msg_opcode != DEFAULT_BYTE_BUF))
                {
		    if ((ext_msg_opcode == 0) || (ext_msg_length >
		         adp_str.xfer_max[NUM_720_SYNC_RATES - 1]))
		    {
                        /* If the REQ/ACK offset is zero, or the    */
		        /* requested transfer period is slower than */
			/* we can set the chip, we will run async.  */
			/* Patch the chip to run asynchronously. */
 		        TRACE_1 ("h ext: sz", 0)
                        for (i=0; i<4; i++)
                        {
                            target_script[R_sxfer_patch_Used[i]] =
                                word_reverse(SXFER_ASYNC_MOVE);
                            target_script[R_scntl1_patch_Used[i]] =
                                word_reverse(SCNTL1_EOFF_MOVE);
                            target_script[R_scntl3_patch_Used[i]] =
                                word_reverse(SCNTL3_SLOW_MOVE);
		        }
                        dev_ptr->sid_ptr->async_device = TRUE;

			if (dev_ptr->flags & NEG_PHASE_2)
			{
                            /* we are finished negotiating */
                            dsp_val = p720_read_reg((uint) DSP,
                                       (char) DSP_SIZE, eaddr);
                            dev_ptr->flags &= ~NEG_PHASE_2;

                            if (dsp_val != REG_FAIL)
			    {
                                rc = p720_write_reg((uint) DSP, (char) 
				    DSP_SIZE, dsp_val, eaddr);
			    }
                            else
                                rc = REG_FAIL;
			}
			else /* send a SDTR msg indicating async */
			{
 			    /* Patch in the negotiation message */
                            target_script[(Ent_sync_msg_out_buf2 / 4)] =
                                (0x01030100 | (ulong) ext_msg_length);
                            target_script[(Ent_sync_msg_out_buf2 / 4) + 1] = 
				((ulong) 0x00 << 24);
                            dev_ptr->flags |= NEG_PHASE_2;
                            rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
                                   word_reverse( script_dma_addr +
                                   (Ent_renegotiate_sync / 4)), eaddr);
			}
                        return (rc);
		    }
		    else /* not asynchronous (and not our defaults) */
		    {
                        /* make sure that the asynch flag is turned off */
                        dev_ptr->sid_ptr->async_device = FALSE;

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
                                target_script[(Ent_sync_msg_out_buf2 / 4) + 1] 
					= ((ulong) DEFAULT_BYTE_BUF << 24);
				special_xfer_val = DEFAULT_BYTE_BUF;
                            }
                            else
			    {
                                target_script[(Ent_sync_msg_out_buf2 / 4) + 1]
					= ((ulong) ext_msg_opcode << 24);
				special_xfer_val = ext_msg_opcode;
			    }

			    /* examine the requested xfer period    */
			    if (ext_msg_length < DEFAULT_MIN_PHASE)
			    {
                                target_script[(Ent_sync_msg_out_buf2 / 4)]
					 = (0x01030100 | (ulong) 
							DEFAULT_MIN_PHASE);
                                /* special_xfer_val high bits are */
				/* already set appropriately. */
				scntl1_patch = SCNTL1_EOFF_MOVE;
				scntl3_patch = SCNTL3_FAST_MOVE;
			    }
			    else
			    {
                                /* We can accept the requested      */
				/* period.  Patch the period into   */
				/* the SDTR message we need to      */
				/* echo. Next, determine the SCNTL1 */
				/* SCNTL3, and SXFER values (TP2-0  */
				/* bits) we need to patch.          */
                                target_script[(Ent_sync_msg_out_buf2 / 4)] 
					 = (0x01030100 | 
						(ulong) ext_msg_length);
                                for (i = 0; i < NUM_720_SYNC_RATES; i++)
			        {
                                    if (ext_msg_length <= adp_str.xfer_max[i])
                                    {
				        if (ext_msg_length == 
 					        adp_str.xfer_max[i])
				            scntl1_patch = SCNTL1_EOFF_MOVE;
				        else
				    	    scntl1_patch = SCNTL1_EON_MOVE;

                                        if (i < 4)
				        {
                                            special_xfer_val |= (uchar) 
								(i << 5);
					    scntl3_patch = SCNTL3_FAST_MOVE;
				        }
				        else
				        {
                                            special_xfer_val |=
                                                (uchar) ((i - 4) << 5);
					    scntl3_patch = SCNTL3_SLOW_MOVE;
				        }
                                        break;
                                    }
			        } /* end for */
			    }

			    /* Patch in values for SXFER, SCNTL1, */
			    /* and SCNTL3 into the SCRIPT.        */
			    TRACE_1 ("h ext 1b", (int) special_xfer_val)
                            for (i=0; i<4; i++)
                            {
                                target_script[R_sxfer_patch_Used[i]] 
				    = word_reverse(SXFER_MOVE_MASK |
				        (special_xfer_val << 8));
                                target_script[R_scntl1_patch_Used[i]] 
				    = word_reverse(scntl1_patch);
                                target_script[R_scntl3_patch_Used[i]] 
				    = word_reverse(scntl3_patch);
		            }
			    dev_ptr->flags |= NEG_PHASE_2;
 			    /* restart the script at negotiation msg */
                            rc = p720_write_reg((uint) DSP, (char) DSP_SIZE, 
				     word_reverse(script_dma_addr +
                                     (Ent_renegotiate_sync / 4)), eaddr);
                            return (rc);
		        }
                        else  /* one of target's values is smaller than */
		 	{     /* ours.  First, patch the SCRIPT.        */
                            p720_patch_nondefault_sync(target_script,
			        ext_msg_length, ext_msg_opcode);
		                     
                            /* Next, either echo the message or end */
      			    if (!(dev_ptr->flags & NEG_PHASE_2))
                            {
 			        /* Patch in the negotiation message */
                                target_script[(Ent_sync_msg_out_buf2 / 4)] 
				     = (0x01030100 | (ulong) ext_msg_length);
                                target_script[(Ent_sync_msg_out_buf2 / 4) + 1]
				     = ((ulong) ext_msg_opcode << 24);

                                dev_ptr->flags |= NEG_PHASE_2;
                                rc = p720_write_reg((uint) DSP, (char) 
				    DSP_SIZE, word_reverse(script_dma_addr 
				    + (Ent_renegotiate_sync / 4)), eaddr);
			    }
			    else   /* NEG_PHASE_2 */
			    {
			        /* Value is agreed to, stop echoing    */
                                dev_ptr->flags &= ~NEG_PHASE_2;
                                dsp_val = p720_read_reg((uint) DSP,
                                           (char) DSP_SIZE, eaddr);
                                if (dsp_val != REG_FAIL)
				{
                                    rc = p720_write_reg((uint) DSP, 
				        (char) DSP_SIZE, dsp_val, eaddr);
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
                    dev_ptr->sid_ptr->async_device = FALSE;

		    /* Patch in values for SXFER, SCNTL1, and SCNTL3 */
                     
 		    TRACE_1 ("h ext df", 0)
                    for (i=0; i<4; i++)
                    {
                        target_script[R_sxfer_patch_Used[i]] 
			        = word_reverse(SXFER_DEF_MOVE);
                        target_script[R_scntl1_patch_Used[i]] 
				= word_reverse(SCNTL1_EOFF_MOVE);
                        target_script[R_scntl3_patch_Used[i]] 
				= word_reverse(SCNTL3_FAST_MOVE);
		    }

                    if (dev_ptr->flags & NEG_PHASE_2)
                    {       
                        dev_ptr->flags &= ~NEG_PHASE_2;
                        dsp_val = p720_read_reg((uint) DSP, (char) DSP_SIZE, 
					eaddr);
                        if (dsp_val != REG_FAIL)
			{
                            rc = p720_write_reg((uint) DSP, (char) DSP_SIZE, 
					dsp_val, eaddr);
			}
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
                        rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
                            word_reverse(script_dma_addr +
                            (Ent_renegotiate_sync / 4)), eaddr);
                    }
                    return (rc);
		}
	    }
            DEBUG_0 ("Extended msg was not sync negotiation\n")
            /* We will restart the SCRIPT in the code below, reading in */
            /* the rest of the extended message.			*/
        }
        else if (ext_msg_opcode == 0x00)  /* modify data pointer */
        {
            /******************************************************/
            /* continue execution of script to read in messages   */
            /* telling us about how they want the data pointers   */
            /* modified.  Then, we hit the interrupt we planted   */
            /* ourselves so we can do data pointer modification.  */
            /******************************************************/
            target_script[Ent_ext_msg_patch / 4] =
                    word_reverse(P720_INTR_PATCH);
            target_script[(Ent_ext_msg_patch / 4) + 1] = word_reverse(
		    A_modify_data_ptr);
        }
        else if (ext_msg_opcode == 0x03)  /* WDTR message */
        {
            /* go to script that sends msg reject */
            /* for sync and then continues on     */
            rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
                    word_reverse(script_dma_addr +
                    (Ent_reject_target_sync / 4)), eaddr);
            return (rc);
	}
	else /* unknown opcode */
        {
	    /* restart the chip */
            rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
                            word_reverse(script_dma_addr +
                            (Ent_unknown_msg_hdlr / 4)), eaddr);
	    return (rc);
        }

	/* restart the chip */
        rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
            word_reverse(script_dma_addr + (Ent_complete_ext_msg / 4)), eaddr);
    }
    else
    {
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
        /* cleanup up patched interrupt.  Make it go  */
        /* back to being a NOP instruction            */
        target_script[Ent_ext_msg_patch / 4] =
            word_reverse(P720_NOP_PATCH);
        target_script[(Ent_ext_msg_patch / 4) + 1] =
            word_reverse(script_dma_addr + Ent_ext_msg_patch);
        data_ptr_offset = target_script[Ent_extended_msg_buf / 4];
/* this needs to be fixed eventually, although it has never worked.
*       dev_ptr->dma_addr += data_ptr_offset;
*/
	/* restart script */
        rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
             word_reverse(script_dma_addr + (Ent_goto_cleanup / 4)), eaddr);
    }
    return (rc);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_patch_nondefault_sync                              */
/*                                                                      */
/* FUNCTION:    Patch the register move instructions in the SCRIPT when */
/*      the target is negotiating to run with a synchronous transfer    */
/*      period and req/ack offset that are not our defaults, but that   */
/*      we can support.                                                 */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called from the interrupt handler.              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *target_script   - the virtual address of the script that needs */
/*              to be patched prior to any data phase transfers.        */
/*      xfer_pd - the synchronous transfer period factor sent by the    */
/*              target device.                                          */
/*              interrupt.                                              */
/*      req_ack - the req/ack offset sent by the target device          */
/*                                                                      */
/* RETURN VALUE:  none                                                  */
/*                                                                      */
/************************************************************************/
void
p720_patch_nondefault_sync(uint * target_script, uchar xfer_pd, uchar req_ack)
{
    int i;
    int scntl1_patch, scntl3_patch;
    char special_xfer_val;

    DEBUG_0("Entering p720_patch_nodefault_sync\n")
    TRACE_2 ("p nondef ", (int) xfer_pd, (int) req_ack)

    /* loop to determine chip transfer period closest to */
    /* requested transfer period, without being less than */
    /* the requested period.  If there is an exact match, */
    /* EXC should be off.  If not, we need EXC on so that */
    /* we will send at a speed slower than the target     */
    /* requested, and the set the chip to receive at a    */
    /* speed faster than the target requested.  SCNTL3 is */
    /* set depending on whether the requested period is   */
    /* less than 32 (5 Mbytes/sec).                       */
    for (i = 0; i < NUM_720_SYNC_RATES; i++)
    {
        if (xfer_pd <= adp_str.xfer_max[i])
        {
	    if (xfer_pd == adp_str.xfer_max[i])
	        scntl1_patch = SCNTL1_EOFF_MOVE;
	    else
	        scntl1_patch = SCNTL1_EON_MOVE;

            if (i < 4)
	    {
                special_xfer_val = (uchar) ((i << 5) | req_ack);
		scntl3_patch = SCNTL3_FAST_MOVE;
	    }
	    else
	    {
                special_xfer_val = (uchar) (((i - 4) << 5) | req_ack);
	        scntl3_patch = SCNTL3_SLOW_MOVE;
	    }
            break;
        }
    }   /* end for loop */

    /* Patch in values for SXFER, SCNTL1, and SCNTL3.  */
    TRACE_2 ("p nondef", special_xfer_val, scntl3_patch)
    for (i=0; i<4; i++)
    {
        target_script[R_sxfer_patch_Used[i]] = 
		word_reverse(SXFER_MOVE_MASK | (special_xfer_val << 8));
        target_script[R_scntl1_patch_Used[i]] = word_reverse(scntl1_patch);
        target_script[R_scntl3_patch_Used[i]] = word_reverse(scntl3_patch);
    }
    DEBUG_0("Leaving p720_patch_nodefault_sync\n")
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_issue_abort_script                                 */
/*                                                                      */
/* FUNCTION:                                                            */
/*      This function will patch in the ABORT message into the message  */
/*      buffer and patch all interrupts within the abort_bdr script.    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts must be disabled.                                    */
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
/*      dev_info_hash - encoded value of target id and lun              */
/*      *com_ptr - address of the command structure associated w/ the   */
/*              abort.                                                  */
/*      connected - indicates whether to try to select the device or    */
/*              whether we are already connected to it and should just  */
/*              try to raise ATN and send the abort message.  We start  */
/*              the script at a different point depending on the value. */
/*	eaddr - effective address for pio				*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      Passes through value returned from p720_write_reg               */
/************************************************************************/
int
p720_issue_abort_script(
                       uint * script_vir_addr,
                       uint * script_dma_addr,
                       struct cmd_info * com_ptr,
                       int dev_info_hash,
		       uchar connected,
		       caddr_t eaddr)
{
    struct scsi_id_info *sid_ptr;
    ulong   *target_script;
    ulong   id_bit_mask, abort_entry_point;
    int     rc;

    TRACE_2 ("iss abrt", (int) com_ptr, (int) connected)
    target_script = (ulong *) script_vir_addr;

    /* update wait script so that the LUN cannot reselect us */
    p720_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr,
        dev_info_hash, FALSE);

    /* force patching related to the LUN on the next command after */
    /* the abort. */
    sid_ptr = com_ptr->device->sid_ptr;
    sid_ptr->tag_flag = adp_str.ddi.card_scsi_id << 3;
    sid_ptr->lun_flag = 0xFF;

    if (!connected)
    {
        id_bit_mask = 0x80000000;
        id_bit_mask |= (0x00060000 | (com_ptr->device->lun_id << 24));
        abort_entry_point = Ent_abort_sequence;

        target_script[A_abort_select_failed_Used[0]] =
            /*word_reverse(A_abort_select_failed | top_half_word);*/
            word_reverse(A_abort_select_failed);
    }
    else
    {
        id_bit_mask = 0x06000000;
        abort_entry_point = Ent_abdr2_sequence;
    }

    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = id_bit_mask;
    target_script[A_abort_io_complete_Used[0]] =
        word_reverse(A_abort_io_complete);

    /* Finally, patch the tag into the register move instructions */
    for (rc = 0; rc < S_COUNT(R_abdr_tag_patch_Used); rc++)
    {
        target_script[R_abdr_tag_patch_Used[rc]] &= word_reverse(0xffff00ff);
        target_script[R_abdr_tag_patch_Used[rc]] |= 
 			word_reverse(((ulong) com_ptr->tag) << 8);
    }
    
    TRACE_1 ("iss abrt", (int) word_reverse(script_dma_addr + 
				(abort_entry_point / 4)));

    rc = p720_write_reg((uint) DSP, (char) DSP_SIZE,
           word_reverse(script_dma_addr + (abort_entry_point / 4)), eaddr);
    return (rc);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_issue_bdr_script                                   */
/*                                                                      */
/* FUNCTION:                                                            */
/*      This function will patch in the BDR message into the message    */
/*      buffer and patch all interrupts within the abort_bdr script.    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      Interrupts must be disabled.                                    */
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
/*      dev_index - encodes the target scsi id                          */
/*      *com_ptr - address of the command structure associated w/ the   */
/*              abort.                                                  */
/*      connected - indicates whether to try to select the device or    */
/*              whether we are already connected to it and should just  */
/*              try to raise ATN and send the abort message.  We start  */
/*              the script at a different point depending on the value. */
/*	eaddr - effective address for pio				*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      Passes through value returned from p720_write_reg               */
/************************************************************************/
int
p720_issue_bdr_script(
                     uint * script_vir_addr,
                     uint * script_dma_addr,
		     int dev_index,
                     struct cmd_info * com_ptr,
		     uchar connected, caddr_t eaddr)
{
    /* Note if I don't share buffer, wait_disconnect code in script, */
    /* and failed_selection handler, most of this code is not needed. */

    ulong *target_script;
    int     i;

    target_script = (ulong *) script_vir_addr;

    /* update wait script so that none of the LUNs can reselect us */
    p720_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr,
        dev_index, TRUE);

    /* force patching related to the LUN on the next command after */
    /* the BDR. */
    com_ptr->device->sid_ptr->tag_flag = adp_str.ddi.card_scsi_id << 3;
    com_ptr->device->sid_ptr->lun_flag = 0xFF;

    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = 0x0C000000;

    /* patch in a bdr interrupt */
    target_script[A_abort_io_complete_Used[0]] =
        word_reverse(A_bdr_io_complete);
    target_script[A_abort_select_failed_Used[0]] =
        word_reverse(A_bdr_select_failed);

    /* Finally, patch the tag into the register move instructions */
    for (i = 0; i < S_COUNT(R_abdr_tag_patch_Used); i++)
    {
        target_script[R_abdr_tag_patch_Used[i]] &= word_reverse(0xffff00ff);
        target_script[R_abdr_tag_patch_Used[i]] |= 
 			word_reverse(((ulong) com_ptr->tag) << 8);
    }

    TRACE_1 ("iss bdr", (int) word_reverse(script_dma_addr + 
				(Ent_bdr_sequence / 4)));
    if (!connected)
        i = p720_write_reg((uint) DSP, (char) DSP_SIZE,
             word_reverse(script_dma_addr + (Ent_bdr_sequence / 4)), eaddr);
    else
        i = p720_write_reg((uint) DSP, (char) DSP_SIZE,
             word_reverse(script_dma_addr + (Ent_abdr2_sequence / 4)), eaddr);

    return (i);
}
/**************************************************************************/
/*                                                                        */
/* NAME: p720_dump                                                        */
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
/*      p720_dumpstrt                                                     */
/*      p720_dumpwrt                                                      */
/*      p720_dumpend                                                      */
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
p720_dump(
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
    DEBUG_0 ("Entering p720_dump\n");
    TRACE_1 ("in p720_dump", 0);

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
        ret_code = p720_dumpstrt();
        first_time = TRUE;
        while ((adp_str.DEVICE_ACTIVE_head != NULL) && (ret_code == 0))
        {
            ret_code = p720_dump_intr(adp_str.DEVICE_ACTIVE_head, 
			first_time);
            first_time = FALSE;
        }
        adp_str.dump_started = TRUE;
        if (ret_code == 0)
            ret_code = p720_dumpwrt(scbuf_ptr);
        TRACE_1 ("rc_dmpwrt", ret_code)
        dev_index = INDEX(scbuf_ptr->scsi_command.scsi_id,
                          (scbuf_ptr->scsi_command.scsi_cmd.lun) >> 5);
        dev_ptr = adp_str.device_queue_hash[dev_index];
        if (ret_code == 0)
            ret_code = p720_dump_intr(dev_ptr, FALSE);
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
    DEBUG_0 ("Leaving p720_dump\n")
    TRACE_1 ("out p720_dump", ret_code)
    return (ret_code);
}  /* end p720_dump */

/**************************************************************************/
/*                                                                        */
/* NAME: p720_dumpstrt                                                    */
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
/*                                                                        */
/* CALLED BY:                                                             */
/*      p720_dump                                                         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      p720_freeze_qs      p720_enq_abrt_bdr                             */
/*                                                                        */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned */
/*      and the caller is left responsible to recover from the error.     */
/*                                                                        */
/* RETURNS:                                                               */
/*      PSC_NO_ERR - dump already started, or aborts queued w/o error     */
/*      EINVAL     - dump has not yet been initialized                    */
/*                                                                        */
/**************************************************************************/
int
p720_dumpstrt()
{
    struct dev_info *dev_ptr;
    struct cmd_info *com_ptr;
    int ret_code;

    DEBUG_0 ("Entering p720_dumpstrt\n")
    TRACE_1 ("in dumpstrt", 0)
    ret_code = PSC_NO_ERR;

    /* if dump dev already started return to caller */
    if (adp_str.dump_started)
        return (PSC_NO_ERR);

    if (!adp_str.dump_inited)
        return (EINVAL);

    /***********************************************************/
    /* Clear all abort_bdr queues. Then enter abort commands   */
    /* all active device queues.                               */
    /***********************************************************/
    adp_str.ABORT_BDR_head = NULL;
    adp_str.ABORT_BDR_tail = NULL;
    
    while ((com_ptr = adp_str.DEVICE_WAITING_head) != NULL)
    {
        p720_freeze_qs(com_ptr->device);
    }

    dev_ptr = adp_str.DEVICE_ACTIVE_head;
    while (dev_ptr != NULL)
    {
        /***************************************************/
        /* put an abort command on the ABORT_BDR queue for */
        /* this device                                     */
        /***************************************************/
        dev_ptr->flags |= SCSI_ABORT;
        p720_enq_abort_bdr(dev_ptr);
        dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
    }
    DEBUG_0 ("Leaving p720_dumpstrt\n")
    TRACE_1 ("out dumpstrt", ret_code)
    return (ret_code);
}  /* end p720_dumpstrt */

/**************************************************************************/
/*                                                                        */
/* NAME: p720_dump_intr                                                   */
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
/*    abort_chip - flag to indicate whether to signal chip                */
/*                                                                        */
/*                                                                        */
/* CALLED BY:                                                             */
/*      p720_dump                                                         */
/*                                                                        */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      p720_free_resources          p720_read_reg      p720_delay        */
/*      p720_update_dataptr          p720_write_reg     p720_verify_tag   */
/*      p720_verify_neg_answer       p720_deq_active    p720_fail_cmd     */
/*                                                                        */
/* RETURNS:                                                               */
/*    EIO - register failures                                             */
/*    ETIMEDOUT - command time outs                                       */
/*                                                                        */
/**************************************************************************/
int
p720_dump_intr(struct dev_info * dev_ptr, char abort_chip)

{
    int ret_code, loop, i, scsi_intr, dma_intr;
    int rc, save_isr, save_stat, dma_stat, save_interrupt;
    int start_new_job;   /* 1=start new job up */
    int time_out_value, command_issued_flag;
    int tag;
    caddr_t eaddr;
    struct cmd_info *com_ptr;
    struct sc_buf *bp;

    DEBUG_0 ("Entering p720_dump_intr\n")
    TRACE_1 ("in dumpintr", abort_chip)
    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

    if (abort_chip)
    {
	if (p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr))
	{
	    BUSIO_DET(eaddr);
	    return(EIO);
	}
    }
    com_ptr = NULL;
    loop = TRUE;
    i = 0;
    /* note timeout_value of 0 not supported in the dump context */
    time_out_value = dev_ptr->active_head->bp->timeout_value;
    while ((loop) && (++i < (time_out_value * 1000)))
    {
        /* delay for 1000th of a second */
        p720_delay(1000);
        start_new_job = FALSE;
                                  
        /* if not our interrupt go around again */
        save_isr = p720_read_reg(ISTAT, ISTAT_SIZE, eaddr);
        if (!(save_isr & (SIP | DIP)))
            continue;
        else 
        {
            tag = p720_read_reg(SCRATCHA0, SCRATCHA0_SIZE, eaddr);
            com_ptr = &adp_str.command[tag];
            TRACE_1 ("tag is ", tag)
            if (save_isr & SIP) /* Planned SCSI interrupt */
	    {
                rc = p720_read_reg(SIST, SIST_SIZE, eaddr);
                scsi_intr = (word_reverse((ulong) rc) >> 16) & SIEN_MASK;
                save_interrupt = scsi_intr;

	        TRACE_2 ("dmp scsi", scsi_intr, (int) com_ptr)
		/* we only deal with valid phase mismatches */
		if ((scsi_intr & PHASE_MIS) && p720_verify_tag(com_ptr, eaddr))
                {
                    DEBUG_0 ("p720_dump: PHASE_MIS\n")
		    dev_ptr = com_ptr->device;
                    TRACE_1 ("dphse ms", (int) dev_ptr)
                    p720_update_dataptr(adp_str.SCRIPTS[dev_ptr->
                        sid_ptr->script_index].script_ptr, dev_ptr, tag, eaddr);

                    /* clear the DMA and SCSI FIFO */
                    if ((p720_write_reg(CTEST3, CTEST3_SIZE, 0x04, eaddr) 
				== REG_FAIL) 
			|| (p720_write_reg(STEST3, STEST3_SIZE, 0x92, eaddr) 
				== REG_FAIL))
                    {
                        loop = FALSE;
                        ret_code = EIO;
                    }
                    else 
		    {
		        ISSUE_MAIN_AFTER_NEG(dev_ptr->sid_ptr->dma_script_ptr,
					     eaddr);
		    }
	    	}
		else
		{
		    /* a phase error for which we cannot determine the dev_ptr,
		     * or any scsi interrupt besides a phase error, is 
	             * considered fatal while taking a dump.
		     */
		    loop = FALSE;
		    ret_code = EIO;
		}
            }   /* End Planned SCSI interrupt */
            else if (save_isr & DIP)     /* DMA interrupt */
            {
                save_interrupt = p720_read_reg(DSTAT, DSTAT_SIZE, eaddr);
		TRACE_1 ("dump dip", save_interrupt)
                if (save_interrupt & SIR)   /* SCRIPTS interrupt */
                {
                    dma_stat = word_reverse(
				    p720_read_reg(DSPS, DSPS_SIZE, eaddr));
                    dma_intr = 0x0000FFFF & dma_stat;
                    dev_ptr = com_ptr->device;
                    bp = com_ptr->bp;

                    switch (dma_intr)
                    {
                        case A_io_done:
                        case A_io_done_after_data:
                            DEBUG_0 ("p720_dump: Current I/O done\n")
                            TRACE_2 ("duiodone", (int) dev_ptr, (int) bp)
                            /* reset completion flags */
                            if (dma_intr == A_io_done_after_data)
                            {
                                bp->bufstruct.b_resid = 0;
                                /* resid = 0 */
                            }
                            else if (com_ptr->flags & RESID_SUBTRACT)
                            {
                                bp->bufstruct.b_resid -= com_ptr->bytes_moved;
                                /* do subtraction */
                            }

                            save_stat = GET_CMD_STATUS_BYTE(adp_str.SCRIPTS
				[dev_ptr->sid_ptr->script_index].script_ptr);
                            DEBUG_1 ("STATUS BUFFER = 0x%x\n", save_stat)

                            start_new_job = TRUE;
                            if (save_stat == SC_GOOD_STATUS)
                            {       /* scsi status is ok */
                                TRACE_1 ("A_iodgds", (int) dev_ptr)
                                /* set scsi status in sc_buf */
                                bp->bufstruct.b_error = 0;
                                bp->status_validity = 0;
                                bp->scsi_status = SC_GOOD_STATUS;

                                p720_deq_active(dev_ptr, com_ptr);
                                /* free resources */
                                (void) p720_free_resources(com_ptr, TRUE);

				/* if this was the last command, no */
				/* longer need to send an abort. */
				if ((dev_ptr->active_head == NULL) &&
                                   ((adp_str.ABORT_BDR_head == dev_ptr) ||
                                   (dev_ptr->ABORT_BDR_bkwd != NULL)))
                                {
                                    p720_deq_abort_bdr(dev_ptr);
				    dev_ptr->flags &= ~SCSI_ABORT;
				    dev_ptr->queue_state = ACTIVE;
                                }

                                loop = FALSE;
                                ret_code = PSC_NO_ERR;
                            }
                            else    /* status is not okay, fail the dump */
                            {
                                loop = FALSE;
                                ret_code = EIO;

                                TRACE_1 ("diodchk", (int) dev_ptr);
                                /* set scsi stat iuf */
                                bp->scsi_status = save_stat;
                                bp->status_validity = SC_SCSI_ERROR;
                                bp->bufstruct.b_error = EIO;
                                bp->bufstruct.b_flags |= B_ERROR;
                                p720_deq_active(dev_ptr, com_ptr);
                                (void) p720_free_resources(com_ptr, FALSE);
                            }  /* end else bad status */
                            break;

                        case A_abort_io_complete:
                        case A_bdr_io_complete:
                            TRACE_1 ("dabdr cmp", (int) dev_ptr);
                            dev_ptr->device_state = NOTHING_IN_PROGRESS;

			    /* all commands are finished */
                            p720_fail_cmd(dev_ptr);
                            start_new_job = TRUE;
                            loop = FALSE;
                            ret_code = PSC_NO_ERR;
                            break;

                        case A_check_next_io:
                        case A_check_next_io_data:
                            TRACE_2 ("dchk nxt", (int) dev_ptr, (int) bp)
                            /**********************************************/
                            /* the device is about to disc. from the bus. */
                            /* and a save DP has just been sent.  Move any */
                            /* data xfered into the buffer and adjust the */
                            /* data pointer within our buffer to show     */
                            /* buffer size has changed.                   */
                            /**********************************************/
                            if (dma_intr == A_check_next_io_data)
                            {
                                bp->bufstruct.b_resid = 0;
                            }
                            else if (com_ptr->flags & RESID_SUBTRACT)
                            {
                                bp->bufstruct.b_resid -= com_ptr->bytes_moved;
                            }

                            /* Turn off resid subtract */
                            com_ptr->flags &= ~RESID_SUBTRACT;
                            dev_ptr = NULL;
                            start_new_job = TRUE;
                            break;

                        case A_suspended:
                            TRACE_1 ("suspended", 0)
                            dev_ptr = NULL;
                            start_new_job = TRUE;
                            break;

                        case A_abort_select_failed:
                            TRACE_1 ("d_abtsel", (int) dev_ptr)
                            /* deq_active the unstarted abort */
                            p720_deq_active(dev_ptr, com_ptr);

                            /* if there are outstanding cmds for
                             * the lun, we need
                             * to restore the iowait jump
			     */
                            dev_ptr->device_state &= ~ABORT_IN_PROGRESS;
                            if (dev_ptr->active_head != NULL)
			    {
                                p720_restore_iowait_jump( (uint *)
                                    adp_str.SCRIPTS[0].script_ptr, dev_ptr,
                                    dev_ptr->sid_ptr->dma_script_ptr);
                            }
                            /* put device on front of abort queue   */
                            if (adp_str.ABORT_BDR_head == NULL)
                            {
                                dev_ptr->ABORT_BDR_fwd = NULL;
                                dev_ptr->ABORT_BDR_bkwd = NULL;
                                adp_str.ABORT_BDR_head = dev_ptr;
                                adp_str.ABORT_BDR_tail = dev_ptr;
                            }
                            else        /* list not empty */
                            {
                                adp_str.ABORT_BDR_head->ABORT_BDR_bkwd
                                  = dev_ptr;
                                dev_ptr->ABORT_BDR_bkwd = NULL;
                                dev_ptr->ABORT_BDR_fwd = adp_str.ABORT_BDR_head;
                                adp_str.ABORT_BDR_head = dev_ptr;
                            }

                            (void) p720_write_reg(DSP, DSP_SIZE, word_reverse(
                                       adp_str.SCRIPTS[0].dma_ptr), eaddr);
                            start_new_job = FALSE;
                            break;

                        default:
                            TRACE_1 ("d_default2", dma_intr)
                            start_new_job = TRUE;
                            break;
                    }   /* End switch */
                } /* End of if SIR */
                /******************************************/
                /* ABORT CAUSED THIS INTERRUPT    */
                /******************************************/
                if (save_interrupt & DABRT)
                {
		    /* under the current design, this should not occur */
                    DEBUG_0 ("p720_dump_intr: Abort caused interrupt\n")

		    /* clear the abort from istat */
                    (void) p720_write_reg(ISTAT, ISTAT_SIZE, 0x00, eaddr);
                    dev_ptr = NULL;
                    start_new_job = TRUE;
                }
            }       /* End DMA interrupt */

            if (start_new_job)
            {
                command_issued_flag = p720_issue_cmd(eaddr);
	        TRACE_1 ("strt job", command_issued_flag)
                if ((command_issued_flag == FALSE) &&
                    (adp_str.DEVICE_ACTIVE_head != NULL))
                {
                    dev_ptr = adp_str.DEVICE_ACTIVE_head;
                    while (dev_ptr != NULL)
                    {
                        p720_restore_iowait_jump( (uint *) adp_str.SCRIPTS[0].
			    script_ptr, dev_ptr, dev_ptr->sid_ptr->
			    dma_script_ptr);
                        dev_ptr = dev_ptr->DEVICE_ACTIVE_fwd;
                    }
                    p720_write_reg(DSP, DSP_SIZE, word_reverse(adp_str.
				   SCRIPTS[0].dma_ptr), eaddr);
                }
            }
	}
    }   /* while */
    /****************************************************************/
    /* if the command did not complete within the time specified in */
    /* the command, then free the resources and return an error     */
    /****************************************************************/
    if (i >= (time_out_value * 1000))
    {
        ret_code = ETIMEDOUT;
    }

    /* if ret_code is not zero, we have a fatal error and expect */
    /* not to be called again to process any more commands */
    DEBUG_0 ("Leaving p720_dumpintr\n")
    TRACE_1 ("out dintr", ret_code)
    BUSIO_DET(eaddr);
    return (ret_code);
}  /* end p720_dump_intr */

/*************************************************************************/
/*                                                                       */
/* NAME: p720_dumpwrt                                                    */
/*                                                                       */
/* FUNCTION: Write to the dump device.                                   */
/*                                                                       */
/*          Issue as many scsi commands as neccessry to write uiop's     */
/*      data to device using the SCSI write command.  If any errors      */
/*      the fail and return with error.                                  */
/*      Actual division of labor is the dumpwrt monitors the looping     */
/*      and checks for timeouts and interrupts.  p720_dump_dev actually  */
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
/*      bp       -  buf struct for the pending dump command              */
/*                                                                       */
/* CALLED BY:                                                            */
/*      p720_dump                                                        */
/*                                                                       */
/* INTERNAL PROCEDURES CALLED:                                           */
/*      p720_dump_dev                                                    */
/*                                                                       */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is return- */
/*      ed and the caller is left responsible to recover from the error. */
/*                                                                       */
/* RETURNS:                                                              */
/*      EINVAL    -  Dump not inited or started                          */
/*      ENXIO     -  Dump device not opened                              */
/*      return values passed-through from p720_dump_dev                  */
/*                                                                       */
/*************************************************************************/
int
p720_dumpwrt(
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
    DEBUG_0 ("Entering p720_dumpwrt\n")
    TRACE_1 ("in dumpwrt", 0)

    if ((!adp_str.dump_inited) || (!adp_str.dump_started))
    {
        return (EINVAL);
    }
    dev_index = INDEX(bp->scsi_command.scsi_id,
                      (bp->scsi_command.scsi_cmd.lun) >> 5);
    dev_ptr = adp_str.device_queue_hash[dev_index];
    if (dev_ptr == NULL)
    {
        ret_code = ENXIO;
    }
    else 
        ret_code = p720_dump_dev(bp, dev_ptr);   /* send a cmd to the device */

    DEBUG_0 ("Leaving p720_dumpwrt\n")
    TRACE_1 ("out dumpwrt", 0)
    return (ret_code);
}  /* end p720_dumpwrt */

/***************************************************************************/
/*                                                                         */
/* NAME: p720_dump_dev                                                     */
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
/*    sc_buf structure - pointer to structure holding the scsi command     */
/*    dev_info structure - pointer to device information structure         */
/*                                                                         */
/* CALLED BY:                                                              */
/*      p720_dumpwrt                                                       */
/*                                                                         */
/* INTERNAL PROCEDURES CALLED:                                             */
/*                                                                         */
/* (RECOVERY OPERATION:) If an error occurs, the proper errno is returned  */
/*      and the caller is left responsible to recover from the error.      */
/*                                                                         */
/* RETURNS:                                                                */
/*      PSC_NO_ERROR                                                       */
/*      EIO    -  I/O error (device needs negotiation)                     */
/*      ENOMEM -  Unable to allocate resources                             */
/*                                                                         */
/***************************************************************************/
int
p720_dump_dev(
             struct sc_buf * bp,
             struct dev_info * dev_ptr)
{    
    caddr_t eaddr;
    uchar chk_disconnect;
    int tag;

    DEBUG_0 ("Entering p720_dump_dev routine.\n")
    TRACE_2 ("dump_dev", (int) bp, (int) dev_ptr)

    if ((tag = p720_alloc_resources(bp, dev_ptr)) <= PSC_ALLOC_FAILED)
        return (ENOMEM);

    if (adp_str.DEVICE_ACTIVE_head == NULL)
    {
        /* assumes that negotiation is unnecessary at this point */
        if (!dev_ptr->sid_ptr->negotiate_flag)
        {
            /* Call routine to start command at the chip */
            chk_disconnect = bp->scsi_command.flags & SC_NODISC;
            if ((dev_ptr->sid_ptr->disconnect_flag ^ chk_disconnect) != 0)
                p720_set_disconnect(dev_ptr, chk_disconnect);
            if ((dev_ptr->sid_ptr->tag_flag != tag) || 
                (dev_ptr->sid_ptr->lun_flag != dev_ptr->lun_id) ||
		(dev_ptr->sid_ptr->tag_msg_flag != bp->q_tag_msg))
                p720_patch_tag_changes(&adp_str.command[tag], bp->q_tag_msg);
            p720_prep_main_script(&adp_str.command[tag], (uint *) 
		    adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr, 
		    dev_ptr->sid_ptr->dma_script_ptr);

            eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
            ISSUE_MAIN_TO_DEVICE(dev_ptr->sid_ptr->dma_script_ptr, eaddr);
	    BUSIO_DET(eaddr);
            dev_ptr->device_state = CMD_IN_PROGRESS;

            /* Add to the DEVICE ACTIVE QUEUE and command active queue */
            p720_enq_active(dev_ptr, &adp_str.command[tag]);
        }
        else
        {
            return (EIO);
        }
    }
    else
    {
	TRACE_1 ("dmp enqw", (int) dev_ptr)
        p720_enq_wait(&adp_str.command[tag]);
    }
    DEBUG_0 ("Leaving p720_dump_dev routine.\n");
    TRACE_1 ("out dump_dev", 0);
    return (PSC_NO_ERR);
}

/************************************************************************
 *								
 * NAME:	p720_sel_glitch
 *						
 * FUNCTION:    Handles problem NCR chip has when target devices or a noisy
 *		bus cause a glitch on the Select line.  Determine the 
 *		appropriate point to restart the microcode.
 *							
 * EXECUTION ENVIRONMENT:			
 *	This routine is called with interrupts disabled. 
 *						
 * DATA STRUCTURES:			
 *	None
 *					
 * INPUTS:			
 *      com_ptr	   - pointer to command information structure
 *	save_isr   - holds the results of a prior pio to ISTAT
 *      eaddr - effective address for pio
 *				
 * RETURN VALUE DESCRIPTION:
 *	None
 *						
 * ERROR DESCRIPTION			
 *							
 ************************************************************************/
void
p720_sel_glitch(struct cmd_info *com_ptr, int save_isr, caddr_t eaddr)
{
    int dsp_val, rc;
    struct dev_info *dev_ptr = com_ptr->device;

    dsp_val = word_reverse(p720_read_reg(DSP, DSP_SIZE, eaddr));
    TRACE_1("chip glitch", dsp_val)

    /* if bad pio, return */
    if (dsp_val == REG_FAIL) {
        p720_cleanup_reset(REG_ERR, eaddr);
	return;
    }

    if (((dsp_val >= (ulong) adp_str.SCRIPTS[0].dma_ptr) &&
         (dsp_val <= ((ulong) adp_str.SCRIPTS[0].dma_ptr) + Ent_init_index)) || 
	 (dev_ptr == NULL)) {
        /* Restart chip at the "wait reselect" instruction 
         * if dsp is in the iowait script or null dev_ptr 
	 */
	TRACE_1("iowait", (int) dsp_val)
        /* restart IOWAIT routine */
        if (p720_write_reg(DSP, DSP_SIZE, word_reverse(adp_str.SCRIPTS[0].dma_ptr), 
		eaddr) == REG_FAIL)
            p720_cleanup_reset(REG_ERR, eaddr);

    } else { 
	/*
         * need to finish the last command, unless
         * a dma interrupt (probably a SIR) is
         * waiting
         */
        if (!(save_isr & DIP)) {
            uchar opcode;
            uchar * sptr;
            int restart_point;
            uint offset = dsp_val & 0x00000fff;

            /* If we are processing a selection attempt that lost
	     * out to the reselection, just continue.  This is indicated 
	     * if we are executing a select instruction, or if we are
	     * in the process of recovering from losing arbitration.
	     *
	     * If we are processing a Wait Disconnect instruction, we
	     * know the disconnect has completed, so we can just continue.
	     *
	     * If we are in the portion of the abort/bdr script that waits
	     * for the target to go bus free, we know this has occurred and
	     * can just restart the script to interrupt the abort/bdr completed.
	     *
	     * Otherwise we are somewhere in the main script, still doing the
	     * various processing that might occur after a legal disconnect.
	     * We cannot reliably know where to restart the SCRIPT since
	     * DSP is not reliable, so we have to give up on the command
	     * that just finished.  The script is restarted to process the
	     * reselection, the abort will go out after the reselection is
	     * serviced.
             */

            /*
             * Determine the opcode associated with the current
             * dsp value.  Remember that the script is byte-
             * swapped, so the opcode is 3 bytes from the
             * start of the instruction.
             */
            sptr = (uchar *) adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr;
            opcode = sptr[offset + 3];
	    TRACE_2("finish", (int) opcode, (int) dsp_val)

	    /* Now decide which case we have, and where to restart */
            if ((offset > Ent_failed_selection_hdlr) &&
	        (offset <= Ent_sync_negotiation)) 
		/* Either failed normal selection, or were attempting 
	         * selection for a negotiation.  In either case, let the 
	         * selection fail.  Note that is DSP=Ent_failed_selection_hdlr,
		 * we can't be sure whether a check_next_io interrupt is about
		 * to execute or whether we lost arbitration.  So that
		 * case is not handled here (we'll end up queuing an Abort below).
	         */
		restart_point = Ent_failed_selection_hdlr;
	    else if ((offset >= Ent_failed_sync_selection_hdlr) &&
		     (offset <= Ent_bdr_sequence))
	    {
		/* Either failed negotiation selection, or were attempting selection
		 * for a BDR.  Distinguish by looking at device state, then proceed
		 * with the correct kind of selection-failure interrupt.
		 */
		if (dev_ptr->device_state & BDR_IN_PROGRESS)
		    restart_point = Ent_failed_abort_bdr_selection_hdlr;
		else
		    restart_point = Ent_failed_sync_selection_hdlr;
	    }
	    else if ((offset >= Ent_failed_abort_bdr_selection_hdlr) &&
		     (offset <= Ent_cmd_msg_in_buf))
		restart_point = Ent_failed_abort_bdr_selection_hdlr;
            else if (opcode == 0x41) /* select instruction */
		restart_point = offset; 
            else if (opcode == 0x48) /* wait disconnect instruction */
		restart_point = offset + 8;
	    else if ((offset >= Ent_start_abdr2_msg_out_phase + 8) &&
		     (offset <= Ent_start_bdr_msg_out_phase))
		/* in the wait_for_bus_free portion of the abort/bdr script */
		restart_point = Ent_start_bdr_msg_out_phase - 8;
	    else {
		/* we are still executing instructions related to the
		 * command that disconnected prior to the glitched
		 * reselection.  Since we cannot rely on DSP to give
		 * an accurate indication where to restart the chip,
		 * queue up an abort (the bp will be marked after the
	         * abort), and process the reselection.
		 */

                dev_ptr->flags |= SCSI_ABORT;
                p720_enq_abort_bdr(dev_ptr);
		restart_point = -1;
	    }

 	    if (restart_point != -1)
                rc = p720_write_reg((uint) DSP, (char) DSP_SIZE, word_reverse(
			dev_ptr->sid_ptr->dma_script_ptr + restart_point), eaddr);
	    else
                rc = p720_write_reg(DSP, DSP_SIZE, word_reverse(adp_str.SCRIPTS[0].
                                            dma_ptr), eaddr);

            if (rc == REG_FAIL)
                p720_cleanup_reset(REG_ERR, eaddr);

        } else {
            /* dip pending too */
	    TRACE_1("dip pending", (int) dsp_val);
        }
    } /* end else not on wait reselect */
}


/************************************************************************/
/*                                                                      */
/* NAME:        p720_cdt_func                                           */
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
/************************************************************************/
struct cdt *
p720_cdt_func(
             int arg)
{
    DEBUG_0 ("Entering p720_cdt_func routine.\n");

    if (arg == 1)
    {
        /* only build the dump table on the initial dump call */

        /* init the table */
        bzero((char *) p720_cdt, sizeof(struct p720_cdt_table));

        /* init the head struct */
        p720_cdt->p720_cdt_head._cdt_magic = DMP_MAGIC;
        strcpy(p720_cdt->p720_cdt_head._cdt_name, "pscsi720");
        /* _cdt_len is filled in below */

        /* now begin filling in elements */
        p720_cdt->p720_entry[0].d_segval = 0;
        strcpy(p720_cdt->p720_entry[0].d_name, "adp_str");
        p720_cdt->p720_entry[0].d_ptr = (char *) &adp_str;
        p720_cdt->p720_entry[0].d_len = (sizeof(struct adapter_def));

        /* fill in the actual table size */
        p720_cdt->p720_cdt_head._cdt_len = sizeof(struct cdt_head) + 
					   sizeof(struct cdt_entry);
    }
    return ((struct cdt *) p720_cdt);

}  /* end p720_cdt_func */

#ifdef P720_TRACE
void
p720_trace_1(char *string, int data)
{
    int     i;

/*
    return;
*/
    if (adp_str.current_trace_line >= (P720_TRACE_SIZE - 0x10))
        adp_str.current_trace_line = 1;
    for (i = 0; i < 13; i++)
        adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.one_val.header1[i] = *(string + i);
    for (i = strlen(string); i < 12; i++)
    {
        adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.one_val.header1[i] = '\0';
    }
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.one_val.data = data;
    adp_str.current_trace_line++;

    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[0] = '1';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[1] = '1';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[2] = '1';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[3] = '1';
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

#ifdef P720_TRACE
void
p720_trace_2(char *string, int val1, int val2)
{
    int     i;

/*
    return;
*/
    if (adp_str.current_trace_line >= (P720_TRACE_SIZE - 0x10))
        adp_str.current_trace_line = 1;
    for (i = 0; i < 9; i++)
        adp_str.trace_ptr->trace_buffer
        [adp_str.current_trace_line].un.two_vals.header2[i] = *(string + i);
    for (i = strlen(string); i < 8; i++)
    {
        adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.two_vals.header2[i] = '\0';
    }
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.two_vals.data1 = val1;
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.two_vals.data2 = val2;
    adp_str.current_trace_line++;
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[0] = '2';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[1] = '2';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[2] = '2';
    adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.header[3] = '2';
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

#ifdef P720_TRACE
void
p720_trace_3(char *string, int data1, int data2, int data3)
{
    int     i;

    if (adp_str.current_trace_line > (P720_TRACE_SIZE - 0x10))
    {
        adp_str.current_trace_line = 1;
    }
    for (i = 0; i < 5; i++)
        adp_str.trace_ptr->trace_buffer
        [adp_str.current_trace_line].un.three_vals.header3[i] = *(string + i);
/*
    for (i = strlen(string); i < 4; i++)
    {
        adp_str.trace_ptr->trace_buffer
         [adp_str.current_trace_line].un.three_vals.header3[i] = '\0';
    }
*/
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
