static char sccsid[] = "@(#)44  1.20  src/bos/kernext/ient/i_entopen.c, sysxient, bos411, 9439C411e 9/30/94 21:48:22";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_open
 *              init_sms
 *              ient_start
 *              ient_xmit_setup
 *              ient_recv_setup
 *              ient_setpos
 *              ient_start_ru
 *              ient_action
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/

#include <stddef.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/systemcfg.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>


#include "i_entdds.h"
#include "i_entmac.h"
#include "i_enthw.h"
#include "i_entsupp.h"
#include "i_ent.h"
#include "i_enterrids.h"

extern int ient_slih();
extern void ient_dmp_add();
extern struct cdt *ient_cdt_func();
extern ient_dev_ctl_t *p_dev_ctl;
extern ethernet_all_mib_t ent_mib_status;
extern ient_trace_t ient_trace_tbl;
void ient_setpos();


/*****************************************************************************/
/*
 * NAME:     ient_open
 *
 * FUNCTION: Integrated Ethernet Device Driver's Open Entry Point
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM: NS user by using the ndd_open field in the NDD on the NDD chain.
 *
 * CALLS TO:  bcopy
 *            bzero
 *            dmp_add
 *            ient_dmp_add  (i_entutil.c)
 *            ient_openclean
 *            ient_setup
 *            ient_start
 *            io_att
 *            io_det
 *            lockl
 *            pincode
 *            unlockl
 *            unpincode
 *
 * INPUT:    p_ndd       - pointer to the ndd structure in the kernel.
 *
 * RETURNS:  0 (success) or errno
 */
/*****************************************************************************/

int
ient_open(ndd_t *p_ndd)        /* pointer to the ndd in the dev_ctl area */
{
    int rc, oldpri;
    ulong io_addr, ioccaddr, save_ndd_flags;
    char pos2val;

    TRACE_BOTH(HKWD_IENT_OTHER, "OpnB", (ulong)p_ndd, 0, 0);

    /* Obtain a non-blocking lock during open of device driver */

    if (lockl((int *)&WRK.lock_anchor, LOCK_NDELAY) != LOCK_SUCC) {
        TRACE_BOTH(HKWD_IENT_OTHER, "Opn1", EBUSY, 0, 0);
        return(EBUSY);
    }

    if (p_dev_ctl->device_state != CLOSED)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Opn2", p_dev_ctl->device_state, EBUSY, 0);
        (void) unlockl((int *)&WRK.lock_anchor);
        return(EBUSY);
    }

    /*
    ** Set the device state and release interrupts.
    */
    p_dev_ctl->device_state = OPEN_PENDING;

    if (pincode(ient_slih) != 0)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Opn3", ENOMEM, 0, 0);
        (void) unlockl((int *)&WRK.lock_anchor);
        return(ENOMEM);
    }

    /* save the start time for statistics */
    WRK.ndd_stime = lbolt;
    WRK.dev_stime = lbolt;

    WRK.sysmem = 0;

    save_ndd_flags = p_ndd->ndd_flags;

    p_ndd->ndd_flags = NDD_BROADCAST | NDD_SIMPLEX;

#ifdef DEBUG
    p_ndd->ndd_flags |= NDD_DEBUG;
#endif

    /*
    ** Initialize all the stats and MIB structures.
    */
    bzero(&ENTSTATS, sizeof(struct ent_genstats));
    bzero(&NDD.ndd_genstats, sizeof(struct ndd_genstats));
    bzero(&DEVSTATS, sizeof(struct ient_stats));
    bzero(&MIB, sizeof(ethernet_all_mib_t));

    bcopy (ETH_MIB_Intel82596, MIB.Generic_mib.ifExtnsEntry.chipset,
        CHIPSETLENGTH);

    if (rc = ient_setup())
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Opn4", rc, 0, 0);
        ient_openclean();
        p_ndd->ndd_flags = p_ndd->ndd_flags;
        p_dev_ctl->device_state = CLOSED;
        (void) unlockl((int *)&WRK.lock_anchor);
        (void) unpincode(ient_slih);
        return(rc);
    }

    if (rc = ient_start())
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Opn5", rc, 0, 0);
        io_addr = (ulong) io_att((DDS.bus_id | IOCC_SELECT), 0);
        ioccaddr = io_addr + (POS_REG_OFFSET + (DDS.slot << 16));
        BUS_GETCX((caddr_t) ioccaddr, &pos2val);
        BUS_PUTCX((caddr_t) ioccaddr, (pos2val & 0xFE));

        io_det(io_addr);
        ient_openclean();
        p_ndd->ndd_flags = p_ndd->ndd_flags;
        p_dev_ctl->device_state = CLOSED;
        (void) unlockl((int *)&WRK.lock_anchor);
        (void) unpincode(ient_slih);
        return(rc);
    }

    WRK.sleep_anchor = EVENT_NULL;

    /*
    ** update the device state and send a status block to demuxer
    ** Remember that this part code should executed only if rc was 0.
    ** If you add any code that changes, change this part of code accordingly.
    */
    p_ndd->ndd_flags |= (NDD_RUNNING | NDD_UP);

    ient_dmp_add("Dev CS", (char *) p_dev_ctl, sizeof(ient_dev_ctl_t));
    ient_dmp_add("Dev Wrk", (char *) &WRK, sizeof(ient_wrk_t));
    ient_dmp_add("Cfg DDS", (char *) &DDS, sizeof(ient_dds_t));
    ient_dmp_add("Trace", (char *) &ient_trace_tbl, TRACE_TABLE_SIZE);

    if (rc = dmp_add(ient_cdt_func))
    {
        /*
        ** If dmp_add fails we will go ahead and ignore this and continue
        ** on with the open.  Besides, the chance of this failing is slim.
        */
        TRACE_SYS(HKWD_IENT_OTHER, "Opn6", rc, 0, 0);
    }

    (void) unlockl((int *)&WRK.lock_anchor);

    TRACE_BOTH(HKWD_IENT_OTHER, "OpnE", rc, 0, 0);

    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_setup
 *
 * FUNCTION: Initialize the Intel 82596 Ethernet Coprocessor and MicroChannel
 *           structures.
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM: ient_open
 *
 * CALLS TO:  d_init
 *            ient_logerr (i_enterr.c)
 *            panic 
 *            xmalloc
 *            ient_logerr
 *            ient_setpos
 *
 * INPUT:    none.
 *
 * RETURNS:  0 (success) or error.
 */
/*****************************************************************************/

int
ient_setup()
{

    int rc = 0;

    TRACE_BOTH(HKWD_IENT_OTHER, "OsuB", (ulong)p_dev_ctl, 0, 0);

    /*
    ** Setup a cross memory descriptor to the shared memory system.
    */

    WRK.xbuf_xd.aspace_id = XMEM_GLOBAL;

    if (WRK.machine == MACH_MEM_BASED)
    {
        WRK.alloc_size =
        sizeof(struct scp) +
        sizeof(struct iscp) +
        sizeof(struct scb) +
        sizeof(struct xcbl) * XMIT_BUFFERS +
        sizeof(struct acbl) * 2 +
        sizeof(struct xmit_buffer) * XMIT_BUFFERS +
        sizeof(struct tbd) * XMIT_BUFFERS +
        sizeof(struct recv_buffer) * ADP_RCV_QSIZE +
        sizeof(struct rfd) * ADP_RCV_QSIZE +
        sizeof(struct rbd) * ADP_RCV_QSIZE +
				PAGESIZE*2;	/* !!!+ 0x100; */
    }
    else
    {
        WRK.alloc_size =
        sizeof(struct xmit_buffer) * XMIT_BUFFERS +
        sizeof(struct recv_buffer) * ADP_RCV_QSIZE +
				PAGESIZE*2;	/* !!!+ 0x100; */
        if (DDS.bus_mem_addr == NULL)
        {
            /* Something didn't get passed correctly notify caller and
            ** return error.
            */
            TRACE_BOTH(HKWD_IENT_ERR, "Osu1", WRK.sysmem, EIO, 0);
            return(EIO);
        }

    }

    /*
    ** Now xmalloc the memory structure aligned on a page boundary using
    ** pinned heap space.
    */

    WRK.sysmem = (ulong) xmalloc(WRK.alloc_size, PGSHIFT, pinned_heap);
    if (WRK.sysmem == NULL)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Osu2", WRK.sysmem, ENOMEM, 0);
        return(ENOMEM);
    }

    WRK.sysmem_end = WRK.sysmem;

    if (WRK.machine == MACH_MEM_BASED)
    {
        WRK.scp_ptr = (struct scp *) WRK.sysmem;
        WRK.scp_addr = (struct scp *)SMTOIOADDR(WRK.scp_ptr);
    }
    else
    {
        WRK.asicram = DDS.bus_mem_addr;
        WRK.asicram_end = WRK.asicram;

        WRK.scp_ptr = (struct scp *) WRK.asicram_end;
        WRK.scp_addr = (struct scp *)((ulong)WRK.scp_ptr & ARAM_MASK);

    }

    /*
    **  divide malloc'd memory between the SCP, ISCP, SCB and the CBL's
    */

    WRK.iscp_ptr = (struct iscp *)(WRK.scp_ptr  + 1);
    WRK.iscp_addr = (struct iscp *)(WRK.scp_addr + 1);
    WRK.scb_ptr  = (struct scb *)(WRK.iscp_ptr + 1);
    WRK.scb_addr = (struct scb *)(WRK.iscp_addr + 1);
    WRK.xcbl_ptr = (struct xcbl *)(WRK.scb_ptr  + 1);
    WRK.xcbl_addr = (struct xcbl *)(WRK.scb_addr + 1);
    WRK.acbl_ptr = (struct acbl *)(WRK.xcbl_ptr + XMIT_BUFFERS);
    WRK.acbl_addr = (struct acbl *)(WRK.xcbl_addr + XMIT_BUFFERS);

    /* the following is a dedicated cbl for no-ops */
    WRK.ncbl_ptr = (struct acbl *)(WRK.acbl_ptr + 1);
    WRK.ncbl_addr = (struct acbl *)(WRK.acbl_addr + 1);

    if (WRK.machine == MACH_MEM_BASED)
        WRK.sysmem_end = (ulong) (WRK.ncbl_ptr + 1);
    else
        WRK.asicram_end = (ulong) (WRK.ncbl_ptr + 1);


    /*
    ** Initalize the POS register structure to their values.
    */

    WRK.pos_reg[POS_3] = 0x00;
    WRK.pos_reg[POS_5] = 0xC0;
    WRK.pos_reg[POS_6] = 0x00;

    /*
    **  set DMA arbitration level and ethernet fairness in POS 4
    **  set interrupt level, ethernet card enable, parity
    **  and select feedback exception in POS 2
    */

    if (WRK.machine == MACH_MEM_BASED)
    {
        WRK.pos_reg[POS_2] =  POS2_CARD_ENABLE | POS2_PARITY_MEM_BASED |
                              POS2_CSF_MEM_BASED;
        WRK.pos_reg[POS_4] =  DDS.dma_arbit_lvl;

    }
    else
    {
        WRK.pos_reg[POS_2] =  POS2_CARD_ENABLE | POS2_INT_ENABLE |
                              POS2_82596_IO_BASED |  POS2_PARITY_IO_BASED |
                              POS2_CSF_IO_BASED | POS2_82596_IO_BASED;

        WRK.pos_reg[POS_4] =  DDS.dma_arbit_lvl << POS4_ARBSHIFT_IO_BASED |
                              POS4_FAIR_IO_BASED;
    }

    /*
    **  use the configuration value to determine the bus interrupt level.
    **  GBG - PPC only uses 5 and 7.
    */

    switch (DDS.intr_level) {
    case 5:
        WRK.pos_reg[POS_2] |= POS2_IRQ5;
        break;
    case 7:
        WRK.pos_reg[POS_2] |= POS2_IRQ7;
        break;
    case 9:
        WRK.pos_reg[POS_2] |= POS2_IRQ9;
        break;
    case 10:
        WRK.pos_reg[POS_2] |= POS2_IRQ10;
        break;
    case 11:
        WRK.pos_reg[POS_2] |= POS2_IRQ11;
        break;
    case 12:
        WRK.pos_reg[POS_2] |= POS2_IRQ12;
        break;
    default:
        panic("ethdd:  invalid interrupt level");
    }


    if (WRK.channel_allocated == FALSE)
    {
        /*  allocate a DMA channel */
        if ((WRK.dma_channel = d_init(DDS.dma_arbit_lvl,
               MICRO_CHANNEL_DMA, (DDS.bus_id | BUSMEM_IO_SELECT))) != DMA_FAIL)
        {
            WRK.channel_allocated = TRUE;
        }
        else
        {
            rc = EFAULT;

            /* log the error */
            TRACE_BOTH(HKWD_IENT_ERR, "Osu3", WRK.dma_channel, rc, 0);
            ient_logerr(ERRID_IENT_SOFTFAIL, __LINE__,__FILE__,
                        DDS.dma_arbit_lvl, WRK.dma_channel,
                        (DDS.bus_id | BUSMEM_IO_SELECT));
        }
    }

    WRK.dump_started = 0;

    TRACE_BOTH(HKWD_IENT_OTHER, "OsuE", rc, (ulong)p_dev_ctl, 0);
    return(rc);
}

/*****************************************************************************/
/*
 * NAME:     ient_start
 *
 * FUNCTION: Initialize and activate the Intel 82596 Ethernet Coprocessor.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM: ient_open
 *              ient_restart
 *
 * CALLS TO:    cache_inval
 *              d_cflush
 *              d_master
 *              ient_action
 *              ient_logerr (i_enterr.c)
 *              ient_recv_setup
 *              ient_setpos
 *              ient_start_ru
 *              ient_xmit_setup
 *              init_sms
 *              io_att
 *              io_det
 *              i_disable
 *              i_enable
 *              i_init
 *              port  (i_entutil.c)
 *              w_init
 *
 * INPUT:    none.
 *
 * RETURNS:  0 (success) or error.
 */
/*****************************************************************************/

int
ient_start()
{
    int    bus, ioa, iocc;
    int    i, j;          /* Loop Counter */
    uchar  temp;
    int    oldpri, rc = 0;
    ulong  busy;
    ulong  value;


    TRACE_BOTH(HKWD_IENT_OTHER, "OstB", (ulong)p_dev_ctl, 0, 0);

    ient_setpos();

    DELAYMS(10);

    /* Get access to the I/O bus to access I/O registers */
    if (WRK.machine != MACH_MEM_BASED)
        ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    port(ioa, PORT_RESET);


    if (!(WRK.restart))
    {
        d_master(WRK.dma_channel, DMA_NOHIDE, (caddr_t) WRK.sysmem,
                 WRK.alloc_size, (struct xmem *) &WRK.xbuf_xd,
                 (caddr_t) WRK.dma_base);

    }

    /* build the Shared Memory Structure */
    if ((rc = init_sms(ioa)) != 0)
    {
        /* log the error */
        ient_logerr(ERRID_IENT_SOFTFAIL, __LINE__, __FILE__, rc,0,0);

        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);

        TRACE_BOTH(HKWD_IENT_ERR, "Ost1", rc, 0, 0);
        return(rc);
    }


    /*
    ** Setup control structures and buffers for xmit, receive, action commands.
    */
    ient_recv_setup(ioa, ADP_RCV_QSIZE);
    ient_xmit_setup(ioa, XMIT_BUFFERS);

    WRITE_LONG(WRK.acbl_ptr[0].next_cb, 0xFFFFFFFF);
    WRITE_LONG(WRK.ncbl_ptr[0].next_cb, 0xFFFFFFFF);

    WRK.action_que_active = 0x00;
    WRK.control_pending = FALSE;

    if (__power_rs())
    {
        if (rc = d_cflush(WRK.dma_channel, WRK.sysmem, WRK.alloc_size,
                             (char *) WRK.dma_base))
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Ost2", INTR_FAIL, EIO, 0);
            if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
            return(rc);                    /* d_cflush failure */
        }
    }

    /*
    ** Initialization Process:
    **
    ** The initialization procedure begins when a Channel Attention signal
    ** is asserted after RESET.  The 82596 uses the address of the double
    ** word that contains the SCP as a default.  Before the CA signal is
    ** asserted this default address (00FFFFF4h) can be changed to any other
    ** available address by asserting the PORT pin and providing the desired
    ** address over the D31-D4 pins of the address bus.
    **
    ** Relocate the default SCP using a port access.
    ** Note: the SCP must be on a 16-byte boundary. This is necessary because
    ** the low-order four bits are used to distinguish the port access command.
    ** SCP is on a page boundary
    */


    port(ioa, (int)WRK.scp_addr | PORT_SCP);
    WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT | CMD_CUC_ABORT);
    DELAYMS(10);
    CHANNEL_ATTENTION();

    DELAYMS(100);

    /*
    **  Register the interrupt handler to the kernel
    */

    if (!WRK.intr_registered)
    {
        if ((rc = i_init((struct intr *)(&(IHS)))) == INTR_FAIL)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Ost3", INTR_FAIL, EIO, 0);
            if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
            return(EIO);
        }
        else
        {
            /* Initialize watchdog timer and set flags */
            w_init((struct watchdog *)(&(WDT)));

            WRK.timr_registered = TRUE;
            WRK.intr_registered = TRUE;
        }
    }

    if (__power_rsc())
        cache_inval(WRK.iscp_ptr, sizeof(struct iscp));

    oldpri = i_disable(DDS.intr_priority);

    READ_LONG(WRK.iscp_ptr->busy, &busy);
    if (busy)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Ost4", INTR_FAIL, EIO, 0);
        ient_setpos();
        port(ioa, PORT_RESET);
        DELAYMS(100);
        port(ioa, (int)WRK.scp_addr | PORT_SCP);
        WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT);
        CHANNEL_ATTENTION();
        DELAYMS(100);
    }

    if (__power_rsc())
        cache_inval(WRK.iscp_ptr, sizeof(struct iscp));

    READ_LONG(WRK.iscp_ptr->busy, &busy);
    if (busy)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Ost5", INTR_FAIL, EIO, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__,
            rc, WRK.sysmem, WRK.dma_channel);

        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
        (void) i_enable(oldpri);
        return(EIO);
    }

    rc = ient_action(CFG_CMD, TRUE);

    /*
    **  wait, then check for errors
    */
    DELAYMS(100);

    READ_LONG(WRK.acbl_ptr[0].csc, &value);

    if (!(value & CB_OK))
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Ost6", value, EFAULT, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0x1829, value,
                    p_dev_ctl);
        rc = EFAULT;
    }

    /*
    **  Issue an Individual Address Setup action command
    **  to load the 82596 with its IA address
    */

    rc = ient_action(IAS_CMD, TRUE);

    /*
    **  wait, then check for errors
    */
    DELAYMS(100);

    READ_LONG(WRK.acbl_ptr[0].csc, &value);

    if (!(value & CB_OK))
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Ost7", value, EFAULT, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0x1829, value,
                    p_dev_ctl);
        rc = EFAULT;
    }

    /*
    ** If multicast addresses are set (restart), execute a MCS
    */

    if ((WRK.alt_count != 0) && (WRK.alt_count < MAX_MULTI))
    {
        rc = ient_action(MCS_CMD, TRUE);

        /*
        **  wait, then check for errors
        */

        DELAYMS(100);

        READ_LONG(WRK.acbl_ptr[0].csc, &value);

        if (!(value & CB_OK))
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Ost8", value, EFAULT, 0);
            ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0x1829,
                        value, p_dev_ctl);
            rc = EFAULT;
        }
    }

    rc = ient_start_ru(ioa);

    /*
    ** update the device state.
    */

    p_dev_ctl->device_state = OPENED;

    if (WRK.machine != MACH_MEM_BASED) io_det(ioa);

    TRACE_BOTH(HKWD_IENT_OTHER, "OstE", rc, 0, 0);

    (void) i_enable(oldpri);

    return(rc);
}

/*****************************************************************************/
/*
 *
 * NAME: init_sms
 *
 * FUNCTION: Initializes the Shared Memory Structure, this includes the SCP,
 *           ISCP, SCP, and the CBL's
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM: ient_start
 *              ient_dump_init (i_entdump.c)
 *
 * CALLS TO:  bzero
 *
 * INPUT:     ioa   -  pointer to systems BUS I/O address space.
 *
 * RETURNS:   0 (success) or error.
 *
 */
/*****************************************************************************/

int
init_sms(ulong ioa)
{
    ulong   i;
    ulong   rc;

    TRACE_BOTH(HKWD_IENT_OTHER, "OsmB", (ulong)p_dev_ctl, WRK.sysmem, 0);

    bzero(WRK.sysmem, WRK.alloc_size);    /* clear the control block page */

    if (WRK.machine == MACH_MEM_BASED)
    {
        WRITE_LONG(WRK.scp_ptr->sysbus, SCP_CFG_MEM_BASED)
    }
    else
    {
        BUSPUTC(RAM_ADDR_LOW, ((ulong)WRK.asicram >> 12) & 0xFF);
        BUSPUTC(RAM_ADDR_HIGH, ((ulong)WRK.asicram >> 20) & 0xFF);

        DELAYMS(10);

        for (i = 0; i < PAGESIZE / sizeof (int); ++i)
            BUSPUTL((ulong *)WRK.asicram + i, 0);

        WRITE_LONG(WRK.scp_ptr->sysbus, SCP_CFG_IO_BASED);
    }
    

    /* Finish the scp and then initialize the ISCP and the SCB */

    WRITE_LONG_REV(WRK.scp_ptr->iscp_addr, WRK.iscp_addr);

    WRITE_LONG(WRK.iscp_ptr->busy, BUSY);
    WRITE_LONG_REV(WRK.iscp_ptr->scb_addr, WRK.scb_addr);

    WRITE_SHORT(WRK.scb_ptr->status, 0x0);
    WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT);
    WRITE_LONG(WRK.scb_ptr->cbl_addr, 0x0);
    WRITE_LONG(WRK.scb_ptr->rfa_addr, 0x0);

    TRACE_BOTH(HKWD_IENT_OTHER, "OsmE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 *
 * NAME:   ient_xmit_setup
 *
 * FUNCTION:  Sets up the transmit buffers and queues
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM:  ient_start
 *               ient_dump_init (i_entdump.c)
 *
 * CALLS TO:  d_align
 *
 * INPUTS:    ioa   -  pointer to system's BUS I/O address space.
 *            bufs  -  Number of buffers to allocate.
 * 
 * RETURNS:   0 (success) always
 *
 */
/*****************************************************************************/

int
ient_xmit_setup(ulong ioa, ulong bufs)
{
    ulong   cache_align, cache_size, i;
    ulong boundary;

    TRACE_BOTH(HKWD_IENT_OTHER, "OxsB", (ulong)p_dev_ctl, bufs, 0);

    /*
    ** Initialize the Transmit Command Block List (CBL)
    */

    for (i = 0; i < bufs; i++)
    {
        WRITE_LONG(WRK.xcbl_ptr[i].xmit.csc, NOP_CMD);    /* Set up a noop */
        WRITE_SHORT(WRK.xcbl_ptr[i].xmit.tcb_cnt , 0x00);
        WRITE_LONG_REV(WRK.xcbl_ptr[i].xmit.next_cb, &WRK.xcbl_addr[i+1]);
    }

    /*
    ** Set it up so the last one on the chain points to the first one.
    */
    WRITE_LONG(WRK.xcbl_ptr[i-1].xmit.csc, NOP_CMD);
    WRITE_SHORT(WRK.xcbl_ptr[i-1].xmit.tcb_cnt , 0x00);
    WRITE_LONG_REV(WRK.xcbl_ptr[i-1].xmit.next_cb, &WRK.xcbl_addr[0]);

    /*
    ** Allocate transmit buffers from system memory align the transmit buffers
    ** on cache line boundaries.
    */
    if (!WRK.restart)
    {
        cache_align = d_align();
	cache_size = PAGESIZE;

	WRK.sysmem_end = WRK.sysmem_end + cache_size;
	WRK.sysmem_end = WRK.sysmem_end & ~(cache_size-1);
     
        WRK.xmit_buf_ptr = (struct xmit_buffer *) WRK.sysmem_end;
        WRK.sysmem_end = (ulong)(WRK.xmit_buf_ptr + bufs);
        WRK.xmit_buf_addr = (struct xmit_buffer *)SMTOIOADDR(WRK.xmit_buf_ptr);

        /* allocate transmit buffers descriptors */
        if (WRK.machine == MACH_MEM_BASED)
        {
            WRK.tbd_ptr = (struct tbd *) WRK.sysmem_end;
            WRK.sysmem_end = (ulong)(WRK.tbd_ptr + bufs);
            WRK.tbd_addr = (struct tbd *)SMTOIOADDR(WRK.tbd_ptr);
        }
        else
        {
            WRK.tbd_ptr = (struct tbd *) WRK.asicram_end;
            WRK.asicram_end = (ulong)(WRK.tbd_ptr + bufs);
            WRK.tbd_addr = (struct tbd *)((ulong)WRK.tbd_ptr & ARAM_MASK);
        }
    }

    /*
    ** Initialize the transmit buffer descriptors and permanently link the
    ** xcbls to the buffers.
    */
    for (i = 0; i < bufs; i++) {
        WRITE_LONG_REV(WRK.xcbl_ptr[i].xmit.tbd, &WRK.tbd_addr[i]);
        WRITE_LONG_REV(WRK.tbd_ptr[i].next_tbd, 0xFFFFFFFF);
        WRITE_LONG_REV(WRK.tbd_ptr[i].tb_addr, &WRK.xmit_buf_addr[i].buf);
        WRITE_LONG(WRK.tbd_ptr[i].control, 0);
    }

    WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.xcbl_addr[0]);

    /*
    **  initialize flags and counters
    */
    WRK.timeout = FALSE;
    WRK.xmits_buffered = 0;
    WRK.xmits_started = 0;
    WRK.txq_first = (struct mbuf *) NULL;
    WRK.txq_last = (struct mbuf *) NULL;
    WRK.buffer_in = 0;

    TRACE_BOTH(HKWD_IENT_OTHER, "OxsE", 0, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME: ient_recv_setup
 *
 * FUNCTION:  Sets up the receive buffers and the receive queues
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM:  ient_start
 *               ient_dump_init (i_entdump.c)
 *               ient_dev_start (i_entdump.c)
 *
 * CALLS TO:  bzero
 *            vm_cflush
 *
 * INPUTS:   ioa    -  pointer to system's BUS I/O address space.
 *           bufs   -  number of buffers to allocate.
 *
 * RETURN:  0 (success) always
 *
 */
/*****************************************************************************/

int
ient_recv_setup(ulong ioa, ulong bufs)
{
    ulong   i;
    ulong   rc;
    ulong	cache_size, cache_align;

    TRACE_BOTH(HKWD_IENT_OTHER, "OrsB", (ulong)p_dev_ctl, bufs, 0);

    /*
    **  set-up the receive frame area (RFA)
    **  malloc a block of memory and initialize it to be the
    **  receive frame descriptors (RFD's) and receive buffers
    */

    if (!WRK.restart)
    {
        if (WRK.machine == MACH_MEM_BASED)
        {
            /* allocate RFDs and RBDs from system memory */
            WRK.rfd_ptr = (struct rfd *) WRK.sysmem_end;
            WRK.sysmem_end = (ulong)(WRK.rfd_ptr + bufs);
            WRK.rfd_addr = (struct rfd *)(SMTOIOADDR(WRK.rfd_ptr));
            WRK.rbd_ptr = (struct rbd *) WRK.sysmem_end;
            WRK.sysmem_end = (ulong)(WRK.rbd_ptr + bufs);
            WRK.rbd_addr = (struct rbd *)(SMTOIOADDR(WRK.rbd_ptr));

            cache_align = d_align();
            cache_size = PAGESIZE;

            WRK.sysmem_end = WRK.sysmem_end + cache_size;
            WRK.sysmem_end = WRK.sysmem_end & ~(cache_size-1);
        }
        else
        {
            /* allocate RFDs and RBDs from ASIC RAM */
            WRK.rfd_ptr = (struct rfd *) WRK.asicram_end;
            WRK.asicram_end = (ulong)(WRK.rfd_ptr + bufs);
            WRK.rfd_addr = (struct rfd *)((ulong)WRK.rfd_ptr & ARAM_MASK);
            WRK.rbd_ptr = (struct rbd *) WRK.asicram_end;
            WRK.asicram_end = (ulong)(WRK.rbd_ptr + bufs);
            WRK.rbd_addr = (struct rbd *)((ulong)WRK.rbd_ptr & ARAM_MASK);
        }

        /* allocate receive buffers from system memory */
        WRK.recv_buf_ptr = (struct recv_buffer *) WRK.sysmem_end;
        WRK.sysmem_end = (ulong)(WRK.recv_buf_ptr + bufs);
        WRK.recv_buf_addr =(struct recv_buffer *)(SMTOIOADDR(WRK.recv_buf_ptr));

    }

    WRK.recv_buffers = bufs;

    /* zero out the receive buffers */
    vm_cflush((caddr_t) &WRK.recv_buf_ptr[0],
                                     bufs * sizeof(struct recv_buffer));

    /*
    **  initialize the RFDs and RBDs
    */
    for (i = 0; i < bufs; i++)
    {
        /* initialize RFD fields */
        WRITE_LONG_REV(WRK.rfd_ptr[i].next_rfd, &WRK.rfd_addr[i+1]);
        WRITE_LONG_REV(WRK.rfd_ptr[i].rbd, 0xffffffff);
        WRITE_SHORT_REV(WRK.rfd_ptr[i].size, 0x00);
        WRITE_SHORT(WRK.rfd_ptr[i].count, 0);
        WRITE_LONG(WRK.rfd_ptr[i].csc, CB_SF);

        /* initialize RBD fields */
        WRITE_LONG_REV(WRK.rbd_ptr[i].next_rbd, &WRK.rbd_addr[i+1]);
        WRITE_LONG_REV(WRK.rbd_ptr[i].rb_addr, &WRK.recv_buf_addr[i]);
        WRITE_LONG_REV(WRK.rbd_ptr[i].size, sizeof(WRK.recv_buf_ptr[i].buf));
    }

    WRITE_LONG(WRK.rfd_ptr[i-1].csc, CB_EL | CB_SF);
    WRITE_LONG_REV(WRK.rbd_ptr[i-1].size, sizeof(WRK.recv_buf_ptr[i-1].buf)
                                                                   | RBD_EL);

    WRK.el_ptr = i - 1;
    WRK.end_fbl = i - 1;
    WRK.rbd_el_ptr = &WRK.rbd_ptr[i-1];


    /* make the RFA circular */
    WRITE_LONG_REV(WRK.rfd_ptr[i-1].next_rfd, &WRK.rfd_addr[0]);
    WRITE_LONG_REV(WRK.rbd_ptr[i-1].next_rbd, &WRK.rbd_addr[0]);

    /* make the SCB point to the head of the RFD list */
    WRITE_LONG_REV(WRK.scb_ptr->rfa_addr, &WRK.rfd_addr[0]);
    WRK.begin_fbl = 0;                      /* 1st free buffer */
    WRITE_LONG_REV(WRK.rfd_ptr[0].rbd, &WRK.rbd_addr[0]);
    TRACE_BOTH(HKWD_IENT_OTHER, "OrsE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:  ient_setpos
 *
 * FUNCTION: Populate the POS registers with the configuration
 *           information found in the DDS.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM: ient_start
 *              ient_dump_init (i_entdump.c)
 *
 * CALLS TO:  io_att
 *            iodet
 *
 * INPUTS:   none
 *
 * RETURNS:  N/A
 */
/*****************************************************************************/

void
ient_setpos()
{
    ulong   ioa, pos_reg_off;
    int     i;

    TRACE_BOTH(HKWD_IENT_OTHER, "OspB", (ulong)p_dev_ctl, 0, 0);

    /* attach to IO space */
    ioa = (ulong) io_att((DDS.bus_id | IOCC_SELECT), 0);
    pos_reg_off = ioa + POS_REG_OFFSET + (DDS.slot << 16);

    /*
    **  update the POS registers
    **  NOTE: cannot generate an exception when accessing POS
    */
    BUS_PUTCX((char *)(pos_reg_off + POS_6), WRK.pos_reg[POS_6]);
    BUS_PUTCX((char *)(pos_reg_off + POS_5), WRK.pos_reg[POS_5]);
    BUS_PUTCX((char *)(pos_reg_off + POS_4), WRK.pos_reg[POS_4]);
    BUS_PUTCX((char *)(pos_reg_off + POS_3), WRK.pos_reg[POS_4]);
    BUS_PUTCX((char *)(pos_reg_off + POS_2), WRK.pos_reg[POS_2]);

    io_det(ioa);                                   /* detach IO handle */

    TRACE_BOTH(HKWD_IENT_OTHER, "OspE", 0, 0, 0);
    return;

}

/*****************************************************************************/
/*
 *
 * NAME: ient_start_ru
 *
 * FUNCTION:  This routine starts the ethernet adapter's receive unit.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *
 * CALLED FROM: ient_start
 *              ient_RU_complete
 *
 * CALLS TO:   ient_logerr  (i_enterr.c)
 *
 * INPUTS:   ioa       -  pointer to system's BUS I/O address space.
 *
 * RETURNS:  N/A
 *
 */
/*****************************************************************************/

int
ient_start_ru(ulong ioa)
{
    int     i = 0, drc = 0;
    int     k = 0;
    ushort  status;
    ushort  cmd_value;

    TRACE_BOTH(HKWD_IENT_OTHER, "OsrB", (ulong)p_dev_ctl, 0, 0);

    ENTSTATS.start_rx++;

    WRITE_LONG(WRK.ncbl_ptr[0].csc, CB_EL | NOP_CMD);
    WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.ncbl_addr[0]);

    COMMAND_QUIESCE(drc);
    if (drc)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Osr1", NDD_ADAP_CHECK, 0x11, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__,
            0, drc, WRK.dma_channel);
        return(drc);
    }

    WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
    CHANNEL_ATTENTION();

    DELAYMS(10);

    COMMAND_QUIESCE(drc);
    if (drc)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Osr2", NDD_ADAP_CHECK, 0x12, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__,
            0, drc, WRK.dma_channel);
        return(drc);
    }

    READ_SHORT(WRK.scb_ptr->status, &status);
    while (!(status & STAT_RUS_READY))
    {
        WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
        CHANNEL_ATTENTION();

        COMMAND_QUIESCE(drc);
        if (drc)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Osr3", NDD_ADAP_CHECK, 0x13, 0);
            ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__,
                0, drc, WRK.dma_channel);
            return(drc);
        }

        READ_SHORT(WRK.scb_ptr->status, &status);

        if (i++ >= 10)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Osr4", 0, 0x14, 0);
            break;
        }
    }


    TRACE_BOTH(HKWD_IENT_OTHER, "OsrE", status, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 *
 * NAME:   ient_action
 *
 * FUNCTION:  Sets up and issues an action command to the Intel 82596 chip.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called from ient_slih.
 *         Action commands are processed one at a time no chaining.
 *
 * CALLED FROM: ient_action_done (i_entintr.c)
 *              ient_ioctl  (i_entctl.c)
 *              ient_start
 *              ient_xmit_done
 *              multi_add (i_entctl.c)
 *              multi_del (i_entctl.c)
 *
 * CALLS TO:    ient_logerr  (i_enterr.c)
 *              ient_restart (i_enterr.c)
 *              io_att
 *              io_det
 *              i_disable
 *              i_enable
 *
 * INPUTS:    cmd           - action command to be executed
 *            execute_now   - whether or not we have to do it now or later.
 *
 * RETURNS:  0 (success) or errno
 *
 */
/*****************************************************************************/

int
ient_action(ulong cmd, ulong execute_now)
{
    ulong   ipri;                   /* save old priority */
    ulong   i, j, k;
    int     mcount;
    ushort  cmd_value;
    struct  acbl     *cbl_address;
    uchar   *net_addr;
    nad_t   *p_nad;
    ulong   ioa;
    struct  cfg     *cfg;           /* configure structure */
    struct  cfg     usercfg;        /* config struct passed from caller */
    struct  ias     *ias;           /* IAS command structure */
    struct  mc      *mc;            /* MCS command structure */
    int     drc = 0;

    TRACE_BOTH(HKWD_IENT_OTHER, "AacB", p_dev_ctl, cmd, execute_now);

    if (WRK.machine != MACH_MEM_BASED)                     /* Attach to Bus */
        ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    ipri = i_disable(DDS.intr_priority);

    if (p_dev_ctl->device_state == OPENED)
        WRK.control_pending = TRUE;

    if ((!execute_now) && (WRK.xmits_buffered))
    {
        /*
        ** queue the command for later issuance
        */

        WRK.action_que_active = cmd;
        TRACE_BOTH(HKWD_IENT_ERR, "Aac1", execute_now, WRK.xmits_buffered, 0);
        i_enable(ipri);
        return(0);
    }
    else
    {
        /*
        ** Either this action was previously queued or there should be no
        ** queued actions.
        */
        WRK.action_que_active = 0;
    }

    /*
    ** set up the cbl address pointing to the acbl
    */
    cbl_address = &WRK.acbl_addr[0];

    switch (cmd) {
    case CFG_CMD: 
    {
        /*
        **  called by the DD at first start and
        **  activation, use default values
        */
        WRK.cur_cfg.byte_count    = CFG_BYTE_COUNT;
        WRK.cur_cfg.fifo_limit    = CFG_FIFO_LIMIT;
        WRK.cur_cfg.save_bf       = CFG_BAD_FRAMES;
        WRK.cur_cfg.loopback      = CFG_LOOPBACK;
        WRK.cur_cfg.linear_pri    = CFG_LINEAR;
        WRK.cur_cfg.spacing       = CFG_SPACING;
        WRK.cur_cfg.slot_time_low = CFG_SLOT_LOW;
        WRK.cur_cfg.slot_time_up  = CFG_SLOT_HIGH;
        WRK.cur_cfg.promiscuous   = CFG_PROMISCUOUS;
        WRK.cur_cfg.carrier_sense = CFG_CSF;
        WRK.cur_cfg.frame_len     = CFG_FRAME_LEN;
        WRK.cur_cfg.preamble      = CFG_PREAMBLE;
        WRK.cur_cfg.dcr_num       = CFG_RESERVED;
        WRK.cur_cfg.dcr_slot      = CFG_RESERVED;

        /*
        ** Do NOT put a break here.
        ** This case is intended to fall through to the
        ** INTERNAL_CFG_CMD case to set up the action.
         */
    }   

    case INTERNAL_CFG_CMD: 
    {
        cfg = (struct cfg *)&WRK.acbl_ptr[0].csc;

        /*
        ** Write out the new configuration.
        */
        WRITE_CHAR(cfg->byte_count, WRK.cur_cfg.byte_count);
        WRITE_CHAR(cfg->fifo_limit, WRK.cur_cfg.fifo_limit);
        WRITE_CHAR(cfg->save_bf, WRK.cur_cfg.save_bf & 0x40);
        WRITE_CHAR(cfg->loopback, WRK.cur_cfg.loopback);
        WRITE_CHAR(cfg->linear_pri, WRK.cur_cfg.linear_pri);
        WRITE_CHAR(cfg->spacing, WRK.cur_cfg.spacing);
        WRITE_CHAR(cfg->slot_time_low, WRK.cur_cfg.slot_time_low);
        WRITE_CHAR(cfg->slot_time_up, WRK.cur_cfg.slot_time_up);
        WRITE_CHAR(cfg->promiscuous, WRK.cur_cfg.promiscuous);
        WRITE_CHAR(cfg->carrier_sense, WRK.cur_cfg.carrier_sense);
        WRITE_CHAR(cfg->frame_len, WRK.cur_cfg.frame_len);
        WRITE_CHAR(cfg->preamble, WRK.cur_cfg.preamble);
        WRITE_CHAR(cfg->dcr_num, WRK.cur_cfg.dcr_num);
        WRITE_CHAR(cfg->dcr_slot, WRK.cur_cfg.dcr_slot);

        /*
        ** This is an action command from the upper layers,
        ** request an interrupt from the adapter.
        */
        
        WRITE_LONG(WRK.acbl_ptr[0].csc, CFG_CMD | CB_SUS | CB_INT | CB_EL);
     
        break;
    }
    case IAS_CMD: 
    {

        /*
        ** Inform the device which HW address to use as a source address.
        ** Will receive packets with this address as a destination address
        ** as well as other packets. ie. broadcast, MC etc.
        */

        ias = (struct ias *) &(WRK.acbl_ptr[0].csc);

        net_addr = (uchar *) WRK.ent_addr;

        /*
        **  We can not use a COPY_NADR here because these
        **   writes may be to memory or to ASIC ram
        */
        for(i = 0; i <= 5; i++) {
           WRITE_CHAR(ias->iaddr[i], net_addr[i]);
        }

        WRITE_LONG(WRK.acbl_ptr[0].csc, IAS_CMD | CB_SUS | CB_INT | CB_EL);
        WRK.wdt_setter = WDT_IAS;
        break;
    }

    case MCS_CMD: 
    {
        p_nad = (nad_t *) WRK.alt_list;
        mcount = WRK.alt_count;
        
        if (mcount > MAX_MULTI)
            mcount = MAX_MULTI;

        /*
        ** Point at csc and fill in the length bit and command
        */
        mc = (struct mc *) WRK.acbl_ptr;

        WRITE_SHORT_REV(mc->byte_count, mcount * 6);

        /*
        **   we can not use a COPY_NADR here because these
        **   writes may be to memory or to ASIC ram
        */
        for (i = 0, k=0; i < mcount; i++)
        {
            for (j = 0; j < ENT_NADR_LENGTH; j++)
            {
                WRITE_CHAR(mc->addr_list[k++], p_nad->nadr[j]);
            }
            p_nad = p_nad->link;
        }

        WRITE_LONG(mc->csc, MCS_CMD | CB_SUS | CB_INT | CB_EL);
        break;
    }

    default:
        /*
        ** treat all other commands as no-ops, using the noop cbl
        */
        cbl_address = &WRK.ncbl_addr[0];
        WRITE_LONG(WRK.ncbl_ptr[0].csc, CB_EL | CB_INT | NOP_CMD);
        TRACE_DBG(HKWD_IENT_OTHER, "Aac2", cbl_address, 0, 0);
    }

    /*
    **  ensure the CU is not in the acceptance phase
    */
    COMMAND_QUIESCE(drc);
    if (drc)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Aac3", NDD_ADAP_CHECK, 0, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0, drc,
                    WRK.dma_channel);
        ient_restart(NDD_ADAP_CHECK);
    }
    else
    {
        /*
        **  put the bus address of the action CBL into the SCB
        */
        WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, cbl_address);
        WRK.action_que_active = FALSE;
        
        TRACE_DBG(HKWD_IENT_OTHER, "Aac4", cbl_address, WRK.acbl_ptr[0].csc,
                  WRK.ncbl_ptr[0].csc);

        /*
        **  start watchdog timer
        **  who set the timer and the reason for setting WDT
        **  were set above
        */

        if ((p_dev_ctl->device_state == OPENED))
            w_start((struct watchdog *)&(WDT));   /* start the watchdog timer */
        
        else
            WRK.wdt_setter = WDT_INACTIVE;


        START_CU();                 /* start the command unit */
    }

    if (WRK.machine != MACH_MEM_BASED) io_det(ioa);

    i_enable(ipri);              /* enable interrupts */


    TRACE_BOTH(HKWD_IENT_OTHER, "AacE", drc, 0, 0);
    return(drc);
}

/*****************************************************************************/
/*
 * NAME:   ient_openclean
 *
 * FUNCTION:  Clean up routine on open failure or on normal close.
 *
 * EXECUTION ENVIRONMENT:  Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM:  ient_open
 *               ient_close  (i_entclose.c)
 *
 * CALLS TO:   d_clear
 *             d_complete
 *             i_clear
 *             w_clear
 *             w_stop
 *             xmfree
 *
 * INPUTS:  none
 *
 * RETURNS:  0 (success) always.
 */
/*****************************************************************************/

int
ient_openclean()
{

    TRACE_BOTH(HKWD_IENT_OTHER, "OclB", (ulong)p_dev_ctl, 0, 0);

    /*
    ** Clear any errors that might have occurred on this channel.
    */

    if (WRK.channel_allocated == TRUE)
    {
        (void)d_complete(WRK.dma_channel,DMA_NOHIDE, (caddr_t) WRK.sysmem,
                    WRK.alloc_size, (struct xmem *) &WRK.xbuf_xd,
                    (caddr_t) WRK.dma_base);
    }
    /*
    ** Free up shared memory area allocated in ient_setup.
    */

    if (WRK.sysmem != NULL)
    {
        xmfree((void *)WRK.sysmem, pinned_heap);
    }
    
    /*
    **  free the DMA channel
    */

    if (WRK.channel_allocated == TRUE)
    {
        d_clear(WRK.dma_channel);
        WRK.channel_allocated = FALSE;
    }

    /* Clear the transmit watchdog timer */

    if (WRK.timr_registered)
    {
        if (WRK.wdt_setter != WDT_INACTIVE)
        {
            w_stop((struct watchdog *)&(WDT));
        }
        w_clear((struct watchdog *)&(WDT));
        WRK.timr_registered = FALSE;
    }

    /*
    **  Unregister the interrupt handler and
    **  and release dma channel (flush IO buffer before
    **  releasing channel )
    */

    if (WRK.intr_registered)
    {
        i_clear((struct intr *) p_dev_ctl);
        WRK.intr_registered = FALSE;
    }

    TRACE_BOTH(HKWD_IENT_OTHER, "OclE", 0, 0, 0);
    return (0);
}
