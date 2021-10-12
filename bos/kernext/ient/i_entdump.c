static char sccsid[] = "@(#)13  1.4  src/bos/kernext/ient/i_entdump.c, sysxient, bos411, 9428A410j 6/12/94 05:06:24";
/*============================================================================.
||                                                                           ||
||   COMPONENT_NAME: SYSXIENT                                                ||
||                                                                           ||
||   FUNCTIONS:                                                              ||
||              ient_dump                                                    ||
||              ient_dump_init                                               ||
||              ient_ru_start                                                ||
||                                                                           ||
||   ORIGINS: 27                                                             ||
||                                                                           ||
||                                                                           ||
||   (C) COPYRIGHT International Business Machines Corp. 1993                ||
||   All Rights Reserved                                                     ||
||   Licensed Materials - Property of IBM                                    ||
||   US Government Users Restricted Rights - Use, duplication or             ||
||   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.       ||
||                                                                           ||
.============================================================================*/

/*============================================================================.
||                                                                           ||
|| Ethernet dump support.                                                    ||
||                                                                           ||
|| This is a polling driver that takes over the 82596 chip and provides two  ||
|| entry points:                                                             ||
||                                                                           ||
||    - d_complete fails                                                     ||
||    - multiple tight loops that might not complete if the 82596 is         ||
||      not working properly                                                 ||
||    - port command returnd and error during channel attention              ||
||                                                                           ||
.============================================================================*/

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
#include <sys/systemcfg.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <netinet/if_ether.h>

#include "i_entdds.h"
#include "i_entmac.h"
#include "i_enthw.h"
#include "i_entsupp.h"
#include "i_ent.h"
#include "i_enterrids.h"

#define  MAX_POLL_RU_START  100
#define  FORCE_DELAY()                   \
{                                        \
    volatile char   *cp = 0;             \
    volatile int    x, n;                \
    n = 10000;                           \
    while (--n >= 0)                     \
            x = *cp;                     \
}

extern ient_dev_ctl_t *p_dev_ctl;

static int ient_dump_init();
static int ient_dmp_recv(ndd_t *p_ndd, caddr_t arg);
static int ient_dev_start(ulong ioa);
static int ient_dmp_xmit(ndd_t *p_ndd, struct mbuf *p_mbuf);


/*============================================================================.
||                                                                           ||
|| NAME: ient_dump                                                           ||
||                                                                           ||
|| FUNCTION: Adapter Driver Dump Routine                                     ||
||                                                                           ||
||      This routine handles requests for dumping data to a previously       ||
||      opened device.                                                       ||
||                                                                           ||
|| EXECUTION ENVIRONMENT:                                                    ||
||      This routine is called when there is certain to be limited           ||
||      functionality available in the system.  However, system              ||
||      dma kernel services are available.  The passed data is already       ||
||      pinned by the caller.  There are no interrupt or timer kernel        ||
||      services available.  This routine should run at INTMAX level.        ||
||                                                                           ||
|| NOTES:                                                                    ||
||      Any lack of resources, or error in attempting to run any             ||
||      command, is considered fatal in the context of the dump.             ||
||      It is assumed that normal operation will not continue once           ||
||      this routine has been executed.                                      ||
||                                                                           ||
|| INPUTS:                                                                   ||
||    p_ndd   - pointer to ndd struct                                        ||
||      cmd   - parameter specifying the dump operation                      ||
||      arg   - pointer to command specific structure                        ||
||                                                                           ||
|| RETURN VALUE DESCRIPTION:                                                 ||
||      A zero will be returned on successful completion, otherwise,         ||
||      one of the errno values listed below will be given.                  ||
||                                                                           ||
|| ERROR DESCRIPTION:                                                        ||
||      The following errno values may be returned:                          ||
||          0         - successful completion                                ||
||          EINVAL    - invalid request                                      ||
||          ENETDOWN  - the adapter cannot connect to the network            ||
||          ETIMEDOUT - the DUMPREAD option timed-out                        ||
||                                                                           ||
.============================================================================*/

int
ient_dump(ndd_t *p_ndd, int cmd, caddr_t arg)
{
    int       rc = 0;
    struct    dmp_query dump_ptr;


    TRACE_DBG(HKWD_IENT_OTHER, "DonB", (ulong) p_ndd, cmd, (ulong) arg);

    /*
    **  Process dump command
    */

    switch (cmd)
    {
        case DUMPQUERY:
            dump_ptr.min_tsize = ENT_MIN_MTU;
            dump_ptr.max_tsize = ENT_MAX_MTU;
            bcopy(&dump_ptr, arg, sizeof(struct dmp_query));

            TRACE_DBG(HKWD_IENT_OTHER, "Don1", dump_ptr.min_tsize,
                      dump_ptr.max_tsize, 0);
        break;

        case DUMPSTART:
            TRACE_DBG(HKWD_IENT_OTHER, "Don2", p_dev_ctl->device_state, 0, 0);
            if (p_dev_ctl->device_state != OPENED)
            {
                TRACE_DBG(HKWD_IENT_OTHER, "Don3", ENETDOWN, 0, 0);
                return(ENETDOWN);
            }

            ient_dump_init();                   /* Setup for the dump */
    
            NDD.ndd_output = ient_dmp_xmit;
            break;
    
        case DUMPREAD:
            if ((rc = ient_dmp_recv(p_ndd, arg)) != 0)
            {
                TRACE_DBG(HKWD_IENT_OTHER, "Don4", rc, 0, 0);
            }
            /*
            ** Forced interrupts disable between xmit and receive.
            */
            break;
    
        case DUMPEND:
        case DUMPINIT:
        case DUMPTERM:
            break;
    
        default:
            rc = EINVAL;
            break;
    
    }

    TRACE_DBG(HKWD_IENT_OTHER, "DonE", rc, 0, 0);
    return(rc);

}


/*============================================================================.
||                                                                           ||
|| NAME: ient_dump_init                                                      ||
||                                                                           ||
|| FUNCTION: Ethernet driver dump init routine.                              ||
||                                                                           ||
|| EXECUTION ENVIRONMENT: Process environment                                ||
||                                                                           ||
|| NOTES: Re-initialize the adapter for dumps so that we don't get stuck on  ||
||        ethernet.                                                          ||
||                                                                           ||
|| CALLED FROM: ient_dump                                                    ||
||                                                                           ||
|| CALLS TO:   ient_setpos                                                   ||
||             port                                                          ||
||             init_sms                                                      ||
||             ient_xmit_setup                                               ||
||             ient_recv_setup                                               ||
||             ient_dev_start                                                ||
||             DELAYMS                                                       ||
||             WAIT_FOR_COMMAND                                              ||
||             CHANNEL_ATTENTION                                             ||
||             START_CU                                                      ||
|| INPUT: none.                                                              ||
||                                                                           ||
|| RETURNS:  0 or errno                                                      ||
||                                                                           ||
.============================================================================*/

/*
** Re-initialize the adapter for dumps so that we don't get stuck on ethernet
** driver produced crashes.
*/
static int
ient_dump_init()
{
    ulong   ioa, busy, value;
    int     i, rc;
    struct  ias     *ias;           /* IAS command structure */
    int     drc = 0;
    uchar   *net_addr;
    struct  acbl     *cbl_address;
    struct  cfg  *cfg;
    int ticks, limit;
    ulong clk_addr = 0;


    TRACE_DBG(HKWD_IENT_OTHER, "DdiB", (ulong)p_dev_ctl, 0, 0);

    /*
    ** Since a dump is starting, we must suspend all current dump activity
    ** and provide whatever setup of the device that is required.  First,
    ** purge the xmit queues and then the receive queues.
    */

    /* Get access to the I/O bus to access I/O registers */
    if (WRK.machine != MACH_MEM_BASED)
        ioa = (void *) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);


    WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_ABORT | CMD_RUC_ABORT);
    CHANNEL_ATTENTION();

    FORCE_DELAY();

    /*
    ** Reset the 82596.
    ** This makes the CU and RU units idle until a channel attention.
    */
    ient_setpos();

    DELAYMS(10);

    /* Get access to the I/O bus to access I/O registers */
    if (WRK.machine != MACH_MEM_BASED)
        ioa = (void *) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    port(ioa, PORT_RESET);


    /*
    ** Steal real driver's memory for dump driver use.
    ** Zap everything that is out there... Bummer!!!  Also we are going to
    ** use the d_master'd channel.
    */

    init_sms(ioa);

    /*
    ** Now setup the send and receive buffers.  Notice that we set
    ** WRK.restart to TRUE so that we don't mangle the pointers.
    */

    WRK.restart = TRUE;          /* Do this so that we do not mess up the */
                                 /* pointers. */

    ient_xmit_setup(ioa, XMIT_BUFFERS);
    ient_recv_setup(ioa, ADP_RCV_QSIZE);

    WRK.restart = FALSE;         /* Reset */


    WRITE_LONG(WRK.acbl_ptr[0].next_cb, 0xFFFFFFFF);
    WRITE_LONG(WRK.ncbl_ptr[0].next_cb, 0xFFFFFFFF);

    WRK.action_que_active = 0x00;
    WRK.control_pending = FALSE;

    /*========================================================================.
    ||                                                                       ||
    || Initialization Process:                                               ||
    ||                                                                       ||
    || The initialization procedure begins when a Channel Attention signal   ||
    || is asserted after RESET.  The 82596 uses the address of the double    ||
    || word that contains the SCP as a default.  Before the CA signal is     ||
    || asserted this default address (00FFFFF4h) can be changed to any other ||
    || available address by asserting the PORT pin and providing the desired ||
    || address over the D31-D4 pins of the address bus.                      ||
    ||                                                                       ||
    || Relocate the default SCP using a port access.                         ||
    ||                                                                       ||
    || Note: the SCP must be on a 16-byte boundary. This is necessary        ||
    ||       because the low-order four bits are used to distinguish the     ||
    ||       port access command.                                            ||
    ||       SCP is on a page boundary.                                      ||
    ||                                                                       ||
    .========================================================================*/

    port(ioa, (int)WRK.scp_addr | PORT_SCP);
    WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_NOP);
    DELAYMS(10);
    CHANNEL_ATTENTION();

    DELAYMS(10);

    READ_LONG(WRK.iscp_ptr->busy, &busy);

    if (busy)
    {
        ient_setpos();
        port(ioa, PORT_RESET);
        DELAYMS(10);
        port(ioa, (int)WRK.scp_addr | PORT_SCP);
        WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_NOP);
        CHANNEL_ATTENTION();
        DELAYMS(10);

        if (busy)
        {
            if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
            TRACE_BOTH(HKWD_IENT_ERR, "Ddi1", busy, 0, 0);
            return(EIO);
        }
    }

    /*
    **  Issue an Individual Address Setup action command
    **  to load the 82596 with its IA address
    */

    ias = (struct ias *) &(WRK.acbl_ptr[0].csc);
    WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | IAS_CMD);
    net_addr = (uchar *) WRK.ent_addr;

    /*
    **  We can not use a COPY_NADR here because these
    **   writes may be to memory or to ASIC ram
    */
    for(i = 0; i <= 5; i++) {
       WRITE_CHAR(ias->iaddr[i], net_addr[i]);
    }

    cbl_address = &WRK.acbl_addr[0];

    if (WRK.machine != MACH_MEM_BASED)
    {
        vm_cflush(WRK.sysmem, WRK.alloc_size);
    }

    COMMAND_QUIESCE(drc)
    if (drc)
    {
        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
        return(ENETDOWN);
    }

    WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, cbl_address);
    START_CU();
    DELAYMS(10);
    WAIT_FOR_COMMAND(drc);
    if (drc)
    {
        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
        return(ENETDOWN);
    }

    /*
    ** Configure the 82596.
    */

    cfg = (struct cfg *)&WRK.acbl_ptr[0].csc;

    /*
    ** Write out the new configuration.
    */
    WRITE_CHAR(cfg->byte_count, CFG_BYTE_COUNT);
    WRITE_CHAR(cfg->fifo_limit, CFG_FIFO_LIMIT);
    WRITE_CHAR(cfg->save_bf, CFG_BAD_FRAMES);
    WRITE_CHAR(cfg->loopback, CFG_LOOPBACK);
    WRITE_CHAR(cfg->linear_pri, CFG_LINEAR);
    WRITE_CHAR(cfg->spacing, CFG_SPACING);
    WRITE_CHAR(cfg->slot_time_low, CFG_SLOT_LOW);
    WRITE_CHAR(cfg->slot_time_up, CFG_SLOT_HIGH);
    WRITE_CHAR(cfg->promiscuous, CFG_PROMISCUOUS);
    WRITE_CHAR(cfg->carrier_sense, CFG_CSF);
    WRITE_CHAR(cfg->frame_len, CFG_FRAME_LEN);
    WRITE_CHAR(cfg->preamble, 0xDB);
    WRITE_CHAR(cfg->dcr_num, CFG_RESERVED);
    WRITE_CHAR(cfg->dcr_slot, CFG_RESERVED);

    WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CFG_CMD);

    if (WRK.machine != MACH_MEM_BASED)
    {
        vm_cflush(WRK.sysmem, WRK.alloc_size);
    }

    START_CU();
    DELAYMS(10);
    WAIT_FOR_COMMAND(drc);
    if (drc)
    {
        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
        return(ENETDOWN);
    }

    if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
    return(drc);

    TRACE_DBG(HKWD_IENT_OTHER, "DdiE", drc, 0, 0);
}


/*============================================================================.
||                                                                           ||
|| NAME: ient_dev_start                                                      ||
||                                                                           ||
|| FUNCTION: Ethernet driver 82596 device start routine.                     ||
||                                                                           ||
|| EXECUTION ENVIRONMENT: Process environment                                ||
||                                                                           ||
|| NOTES: This routine will start both the RU and CU to begin the receive    ||
||        and transmission of data between the host and the NFS server.      ||
||                                                                           ||
|| CALLED FROM: ient_dmp_xmit                                                ||
||                                                                           ||
|| CALLS TO:   COMMAND_QUIESCE                                               ||
||             CHANNEL_ATTENTION                                             ||
||             DELAYMS                                                       ||
||                                                                           ||
|| INPUT: Address of BUSIO                                                   ||
||                                                                           ||
|| RETURNS:  0 or errno                                                      ||
||                                                                           ||
.============================================================================*/

static int
ient_dev_start(ulong ioa)
{
    int   i = 0, drc = 0;
    int   k = 0;
    volatile ushort  status;

    TRACE_DBG(HKWD_IENT_OTHER, "DdsB", ioa, 0, 0);

    /*
    ** Start the RU first so it is ready for the acknowledgement packet coming
    ** back from the NFS server.
    */

    if (__power_rs())
        cache_inval(&WRK.scb_ptr->status, sizeof(ulong));

    READ_SHORT(WRK.scb_ptr->status, &status);
    while (!(status & STAT_RUS_READY))
    {
        if (status & (STAT_RNR | STAT_RUS_NO_RESOURCE | STAT_RUS_NO_RSC_RBD))
        {
            /*
            ** Receive queue filled while we were away.
            ** Since we were not looking for anything yet flush queue
            ** and reset.
            */
            TRACE_DBG(HKWD_IENT_ERR, "Dds1", status, 0, 0);
            WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT);
            CHANNEL_ATTENTION();
            COMMAND_QUIESCE(drc);
            WRK.restart = TRUE;
            ient_recv_setup(ioa, ADP_RCV_QSIZE);
        }

        WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
        CHANNEL_ATTENTION();
        DELAYMS(20);

        COMMAND_QUIESCE(drc);

        if (__power_rs())
            cache_inval(&WRK.scb_ptr->status, sizeof(ulong));

        READ_SHORT(WRK.scb_ptr->status, &status);

        if (i++ > MAX_POLL_RU_START)
        {
            TRACE_DBG(HKWD_IENT_ERR, "Dds2", status, 0, 0);
            return(EIO);
        }
    }

    /*
    ** Start the CU if it is not active, and then wait until it is idle.
    ** When it is back to idle then we have processed the command.
    */

    while (status & STAT_CUS_ACTIVE)
    {
        if ((WRK.machine == MACH_MEM_BASED) && (__power_rs()))
            cache_inval(&WRK.scb_ptr->status, sizeof(ulong));

        READ_SHORT(WRK.scb_ptr->status, &status);
        DELAYMS(50);
    }

    WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_START);
    if ((WRK.machine == MACH_MEM_BASED) && (__power_rs()))
        vm_cflush(&WRK.scb_ptr->command, sizeof(ulong));

    DELAYMS(1);

    CHANNEL_ATTENTION();

    while (k++ < MAX_POLL_RU_START)
    {
        TRACE_DBG(HKWD_IENT_OTHER, "Dds3", status, k, 0);
        if (!(status & 0x000F))                     /* Is CU idle? */
            break;

        if ((WRK.machine == MACH_MEM_BASED) && (__power_rs()))
            cache_inval(&WRK.scb_ptr->status, sizeof(ulong));

        READ_SHORT(WRK.scb_ptr->status, &status);
        DELAYMS(10);
    }
       
    TRACE_DBG(HKWD_IENT_OTHER, "DdsE", status, i, k);
    return(drc);
}


/*============================================================================.
||                                                                           ||
|| NAME: ient_sync_mem                                                       ||
||                                                                           ||
|| FUNCTION: Ethernet driver cache/memory/IOCC memory syncronization.        ||
||                                                                           ||
|| EXECUTION ENVIRONMENT: Interrupt Environment (INTMAX)                     ||
||                                                                           ||
|| NOTES: For the cache consistency models this is a NOP.                    ||
||                                                                           ||
|| CALLED FROM: ient_dmp_recv                                                ||
||                                                                           ||
|| CALLS TO:   cache_inval                                                   ||
||             d_complete                                                    ||
||             DELAYMS                                                       ||
||                                                                           ||
|| INPUT: Address and range of memory to sync.                               ||
||                                                                           ||
|| RETURNS:  0 or errno                                                      ||
||                                                                           ||
.============================================================================*/

int
ient_sync_mem(ulong mem_addr, ulong range, ulong ioa)
{
    int rc = 0;

    cache_inval(mem_addr, range);

    rc = d_complete(WRK.dma_channel, DMA_NOHIDE, WRK.sysmem,
                    WRK.alloc_size, (struct xmem *)&WRK.xbuf_xd,
                    (char *)WRK.dma_base);

    if (rc)
    {
        TRACE_DBG(HKWD_IENT_ERR, "Dsm2", WRK.dma_channel, rc, 0);
        if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
        return(ENETDOWN);
    }

    return(rc);
}



/*============================================================================.
||                                                                           ||
|| NAME: ient_dmp_xmit                                                       ||
||                                                                           ||
|| FUNCTION: Ethernet driver dump transmit routine.                          ||
||                                                                           ||
|| EXECUTION ENVIRONMENT: Process environment                                ||
||                                                                           ||
|| NOTES: Waits for an acknowledgement packet to come in from the host that  ||
||        is receiving the dump image.                                       ||
||                                                                           ||
|| CALLED FROM: NDD                                                          ||
||                                                                           ||
|| CALLS TO:   COMMAND_QUIESCE                                               ||
||             CHANNEL_ATTENTION                                             ||
||             DELAYMS                                                       ||
||                                                                           ||
|| INPUT: p_ndd - pointer to NDD structure                                   ||
||        mbuf_args - contains the pointer to struct dump_read               ||
||                                                                           ||
|| RETURNS:  0 or errno                                                      ||
||                                                                           ||
.============================================================================*/

static int
ient_dmp_xmit(ndd_t *p_ndd, struct mbuf *p_mbuf)
{
    ushort stat_value;
    ulong  csc_value, ioa, xcbl_status, status;
    int    bytes, save_bytes, offset, num_tries;
    struct ether_header *ie3_header;
    int    rc = 0;
    int    drc = 0;

    TRACE_DBG(HKWD_IENT_XMIT, "DtxB", (ulong)p_ndd, (ulong)p_mbuf, 0);

    bytes = p_mbuf->m_len;
    save_bytes = bytes;

    if (WRK.machine != MACH_MEM_BASED)
        ioa = (void *) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    /*
    ** If this is the first packet to be sent.  Set dump_started and then
    ** capture the DSAP value of the source.
    */
    if (WRK.dump_started == 0)
    {
        WRK.dump_started = TRUE;

        ie3_header = (struct ether_header *) p_mbuf->m_data;
        if (ie3_header->ether_type == ETHERTYPE_IP)
        {
            WRK.dump_dsap = (ulong) ie3_header->ether_type;
        }
        else
        {
            /* This is an 802.3 type of Ethernet */
            WRK.dump_dsap = (ulong)(*((caddr_t)(p_mbuf->m_data +
                                            sizeof(struct ie3_mac_hdr))));
        }
    }

    /*
    ** copy data into tranmit buffer and do processor cache flush
    */
    bcopy(MTOD(p_mbuf, caddr_t), &WRK.xmit_buf_ptr[0].buf, bytes);

    /*
    ** Pad short packet with garbage and fill out the TBD
    */
    if (bytes < ENT_MIN_MTU)
        bytes = ENT_MIN_MTU;

    TRACE_DBG(HKWD_IENT_ERR, "Dtx1", bytes, &WRK.xmit_buf_ptr[0].buf, 0);

    if (__power_rs())
         vm_cflush((caddr_t) &WRK.xmit_buf_ptr[0].buf, save_bytes);

    bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);

    WRITE_LONG(WRK.tbd_ptr[0].control, bytes << 16 | XMIT_EOF);
    WRITE_LONG(WRK.xcbl_ptr[0].xmit.csc, XMIT_CMD | CB_SF | CB_EL);
    WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.xcbl_addr[0]);

    WRK.xmits_buffered++;


    if ((WRK.machine == MACH_MEM_BASED) && (__power_rs()))
    {
        (void) d_cflush(WRK.dma_channel, WRK.sysmem, WRK.alloc_size,
                                                       (char *) WRK.dma_base);
    }

    ient_dev_start(ioa);    /* Start both the RU and CU now */
 
    num_tries = 0;

    while (WRK.xmits_buffered)
    {

        /*
        ** Insure that we do not pick up the old status on this loop from
        ** the cache and not from memory on the memory based machines.
        ** This will be a NOP on the PPC machines.
        */

        cache_inval(&WRK.xcbl_ptr[0].xmit, sizeof(struct xmit));

        READ_LONG(WRK.xcbl_ptr[0].xmit.csc, &xcbl_status);

        TRACE_DBG(HKWD_IENT_XMIT, "Dtx4", xcbl_status, WRK.xmits_buffered, 0);

        status = xcbl_status & STAT_FIELD_MASK;

        if (status & CB_COMP)
        {
            /*
            ** Transmit Command completed successfully.
            */
            if (__power_rs())
                ient_sync_mem(&WRK.xcbl_ptr[0].xmit, sizeof(struct xmit), ioa);

            num_tries = 0;
            WRK.xmits_buffered--;
        }
        else
        {
            if (++num_tries < 5)
            {
                TRACE_DBG(HKWD_IENT_XMIT, "Dtx5", xcbl_status, 0, 0);
                DELAYMS(10);
                continue;
            }
            else if (num_tries < 10)
            {
                TRACE_DBG(HKWD_IENT_XMIT, "Dtx6", xcbl_status, 0, 0);
                FORCE_DELAY();
                continue;
            }
            else
            {
                TRACE_DBG(HKWD_IENT_XMIT, "Dtx7", xcbl_status, 0, 0);
                WRK.xmits_buffered--;
                rc = ENETDOWN;
                break;
            }
        }
    }

    if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
    TRACE_DBG(HKWD_IENT_XMIT, "DtxE", rc, WRK.xmits_buffered, 0);
    return(rc);
}


/*============================================================================.
||                                                                           ||
|| NAME: ient_dmp_recv                                                       ||
||                                                                           ||
|| FUNCTION: Ethernet driver dump receive routine.                           ||
||                                                                           ||
|| EXECUTION ENVIRONMENT: Process environment                                ||
||                                                                           ||
|| NOTES: Waits for an acknowledgement packet to come in from the host that  ||
||        is receiving the dump image.                                       ||
||                                                                           ||
|| CALLED FROM: NDD                                                          ||
||                                                                           ||
|| CALLS TO:   COMMAND_QUIESCE                                               ||
||             CHANNEL_ATTENTION                                             ||
||             DELAYMS                                                       ||
||                                                                           ||
|| INPUT: p_ndd - pointer to NDD structure                                   ||
||        mbuf_args - contains the pointer to struct dump_read               ||
||                                                                           ||
|| RETURNS:  0 or errno                                                      ||
||                                                                           ||
.============================================================================*/


static int
ient_dmp_recv(ndd_t *p_ndd, caddr_t mbuf_args)
{
    struct dump_read dump_args;          /* MBuf Pointer and Wait Time  */
    struct mbuf *p_mbuf;                 /* MBuf Pointer from dump_args */
    struct timestruc_t  current, timeout, temp_time;
    struct rbd *work_rbd;
    caddr_t work_buffer;
    int    rfd_nbr, rc, good_frame;
    uchar  value, tmp_rfd_data[NBR_DATA_BYTES_IN_RFD];
    ushort stat_value, size, have_packet, cnt_value, count;
    ulong  ioa, csc_value, tdsap;
    ulong  start_fbl, status, stat_reg = 0;

    TRACE_DBG(HKWD_IENT_OTHER, "DrdB", p_ndd, mbuf_args, 0);

    /* Get access to the I/O bus to access I/O registers */
    if (WRK.machine != MACH_MEM_BASED)
        ioa = (void *) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    /*
    ** Setup current time slot.
    */
    temp_time.tv_sec = dump_args.wait_time / MSEC_PER_SEC;
    temp_time.tv_nsec = (dump_args.wait_time % MSEC_PER_SEC) * NSEC_PER_MSEC;
    curtime(&current);
    ntimeradd(current, temp_time, timeout);

    /*
    ** Wait until we get a packet sent in.
    */

    have_packet = FALSE;
    rc = 0;
    start_fbl = WRK.begin_fbl;

    while (!have_packet)
    {
        if ((WRK.machine == MACH_MEM_BASED) && (__power_rs()))
            cache_inval(&WRK.rfd_ptr[WRK.begin_fbl].csc, sizeof(struct rfd));

        READ_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, &csc_value);

        TRACE_DBG(HKWD_IENT_ERR, "Drd1", csc_value, 0, 0);

        if (!(csc_value & CB_C))
        {
            /*
            ** Check to see if we have reached a timeout condition.
            */

            if (ntimercmp(current, timeout, >))
            {
                TRACE_DBG(HKWD_IENT_ERR, "Drd2", ENETDOWN, stat_value, 0);
                if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
                return(ETIMEDOUT);
            }
            curtime(&current);

            DELAYMS(10);
            continue;
        }

        if (__power_rs())
            ient_sync_mem(&WRK.rfd_ptr[WRK.begin_fbl], sizeof(struct rfd), ioa);

        READ_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, &work_rbd);

        if (WRK.machine == MACH_MEM_BASED)
        {
            work_rbd = IOADDRTOSM(work_rbd);
        }
        else
        {
            work_rbd = (long)work_rbd + (long)((long)&WRK.rbd_ptr[0] &
                                                           (long) (~ARAM_MASK));
        }

        READ_SHORT(WRK.rfd_ptr[WRK.begin_fbl].d_addr, tmp_rfd_data);


        /* Get the buffer address for later on  */
        READ_LONG_REV(work_rbd->rb_addr, &work_buffer);
        work_buffer = IOADDRTOSM(work_buffer);

        READ_SHORT(work_rbd->count, &cnt_value);
        count = reverse_short(cnt_value) & 0x3FFF;
        count += NBR_DATA_BYTES_IN_RFD;    /* Add bytes that are in rfd */

        TRACE_DBG(HKWD_IENT_ERR, "Drd3", count, work_buffer, work_rbd);

        if (count < 500)
        {
            /*
            ** It passed the first test now lets see if the DSAP's match.
            */

            tdsap = (ulong)(*((ushort *)(work_buffer +
                                            (ETHER_ADDRLEN * 2) - 
                                             NBR_DATA_BYTES_IN_RFD)));
            if (tdsap < ETHERMTU)
            {
                /* This is an 802.3 type of Ethernet */
                tdsap = (ulong)(*((caddr_t)(work_buffer +
                                            (ETHER_ADDRLEN * 2) -
                                             NBR_DATA_BYTES_IN_RFD + 1)));
            }

            TRACE_DBG(HKWD_IENT_ERR, "Drd4", tdsap, 0, 0);

            if (WRK.dump_dsap != tdsap)
            {
                WRITE_SHORT(WRK.rfd_ptr[WRK.begin_fbl].count, 0);
                WRITE_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, ALLONES);
                WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, CB_EL | CB_SF);
                WRITE_SHORT(WRK.rbd_ptr[WRK.begin_fbl].count, 0);
                WRITE_LONG_REV(WRK.rbd_ptr[WRK.begin_fbl].size,
                        sizeof(WRK.recv_buf_ptr[WRK.begin_fbl].buf) | RBD_EL);

                WRITE_LONG(WRK.rfd_ptr[WRK.end_fbl].csc, CB_SF);
                WRITE_LONG_REV(WRK.rbd_ptr[WRK.end_fbl].size,
                                sizeof(WRK.recv_buf_ptr[WRK.end_fbl].buf));

                WRK.end_fbl = WRK.begin_fbl;
                WRK.begin_fbl = ++WRK.begin_fbl % ADP_RCV_QSIZE;

                if (WRK.machine != MACH_MEM_BASED)
                {
                    vm_cflush(WRK.sysmem, WRK.alloc_size);
                }
                continue;
            }

            have_packet = TRUE;          /* Got one!!! */

            bcopy(mbuf_args, &dump_args, sizeof(struct dump_read));
            p_mbuf = dump_args.dump_bufread;
            p_mbuf->m_next = NULL;
            p_mbuf->m_flags |= M_PKTHDR;


            /*
            ** Build the mbuf to send back by copying the bytes stored in
            ** the receive frame descriptor and then the actual data that
            ** follows.
            */

            bcopy((char *) tmp_rfd_data, MTOD(p_mbuf,char *),
                                                NBR_DATA_BYTES_IN_RFD);

            bcopy((char *) work_buffer, MTOD(p_mbuf, char *)
                 + NBR_DATA_BYTES_IN_RFD, count - NBR_DATA_BYTES_IN_RFD);

            /*
            ** Adjust len and m_data as described in design document by
            ** passing just the address of the beginning of IP header and
            ** the length from that point on.
            **
            ** What dump doesn't want to know about:
            **
            **   ==========================================================
            **  ||  SRC ADDR (6)  ||  DEST ADDR (6)  ||  LEN/CONTROL (2)  ||
            **   ==========================================================
            **
            ** For 802.3 it doesn't want to know about the DSAP/MAC stuff.
            */

            count = count - sizeof(struct ether_header);
            p_mbuf->m_data = p_mbuf->m_data + sizeof(struct ether_header);

            if (WRK.dump_dsap < ETHERMTU)          /* 802.3 */
            {
                count = count - sizeof(struct ie2_llc_snaphdr);
                p_mbuf->m_data += sizeof(struct ie2_llc_snaphdr);
            }

            p_mbuf->m_pkthdr.len = p_mbuf->m_len = count;

            TRACE_DBG(HKWD_IENT_ERR, "Drd5", p_mbuf, p_mbuf->m_data, count);
        }

        WRITE_SHORT(WRK.rfd_ptr[WRK.begin_fbl].count, 0);
        WRITE_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, ALLONES);
        WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, CB_EL | CB_SF);
        WRITE_SHORT(WRK.rbd_ptr[WRK.begin_fbl].count, 0);
        WRITE_LONG_REV(WRK.rbd_ptr[WRK.begin_fbl].size,
                         sizeof(WRK.recv_buf_ptr[WRK.begin_fbl].buf) | RBD_EL);

        WRITE_LONG(WRK.rfd_ptr[WRK.end_fbl].csc, CB_SF);
        WRITE_LONG_REV(WRK.rbd_ptr[WRK.end_fbl].size,
                                sizeof(WRK.recv_buf_ptr[WRK.end_fbl].buf));


        WRK.end_fbl = WRK.begin_fbl;
        WRK.begin_fbl = ++WRK.begin_fbl % ADP_RCV_QSIZE;

        if (WRK.machine != MACH_MEM_BASED)
        {
            vm_cflush(WRK.sysmem, WRK.alloc_size);
        }
    }

    TRACE_DBG(HKWD_IENT_OTHER, "DrdE", rc, 0, 0);
    if (WRK.machine != MACH_MEM_BASED) io_det(ioa);
    return(rc);
}

